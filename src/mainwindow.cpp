#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QSettings>
#include <QThread>
#include <QtEvents>

#include <QtXml>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

/** Search the configuration file */
QString searchConfigFile()
{
    QString binDir = QCoreApplication::applicationDirPath();
    QString appName = QCoreApplication::applicationName();
    QString fileName(appName + ".json");

    QStringList searchList;
    searchList.append(binDir);
    searchList.append(binDir + "/etc");
    searchList.append(binDir + "/../etc");
    searchList.append(binDir + "/../../etc");              // for development without shadow build
    searchList.append(binDir + "/../" + appName + "/etc"); // for development with shadow build
    searchList.append(binDir + "/../../" + appName + "/etc"); // for development with shadow build
    searchList.append(binDir + "/../../../" + appName + "/etc"); // for development with shadow build
    searchList.append(binDir + "/../../../../" + appName
                      + "/etc"); // for development with shadow build
    searchList.append(binDir + "/../../../../../" + appName
                      + "/etc"); // for development with shadow build
    searchList.append(QDir::rootPath() + "usr/local/etc");
    searchList.append(QDir::rootPath() + "etc/opt");
    searchList.append(QDir::rootPath() + "etc");

    foreach (QString dir, searchList) {
        QFile file(dir + "/" + fileName);
        if (file.exists()) {
            // found
            fileName = QDir(file.fileName()).canonicalPath();
            qDebug("Using config file %s", qPrintable(fileName));
            return fileName;
        }
    }

    // not found
    foreach (QString dir, searchList) {
        qWarning("%s/%s not found", qPrintable(dir), qPrintable(fileName));
    }
    qFatal("Cannot find config file %s", qPrintable(fileName));

    return nullptr;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //  State Machine
    currentState = stateInitApp;
    nextState = stateInitApp;
    lastState = stateInitApp;

    connect(&stateMachineTimer, &QTimer::timeout, this, &MainWindow::mainStateMachine);
    stateMachineTimer.setInterval(100);
    stateMachineTimer.start();

    //  Carga configuraciones almacenadas
    configUpdate = false;

    //  Serial Port
    checkPortTimer = new QTimer(this);
    serialBuffer = new QByteArray();

    //      Jig Command and Interface Editors
    commandEditor = new JigCommandEdit(this);
    interfaceEditor = new JigInterfaceEdit(this);

    //      JIGsaw
    jigsaw = new JigSaw(this);

    //  Application configurations
    QFile configFilePath(searchConfigFile());

    if (configFilePath.open(QIODevice::ReadOnly)) {
        QByteArray configFileData = configFilePath.readAll();

        readAppConfigFile(configFileData);
    } else {
        qWarning("Couldn't open config file.");
        QMessageBox::warning(this,
                             "Advertencia",
                             "El archivo de configuración tiene errores, o no fue encontrado.");
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "dbConexion");
    db.setHostName(dbHostName);
    db.setDatabaseName(dbDatabaseName);
    db.setUserName(dbUserName);
    db.setPassword(dbPassword);

    if (db.open()) {
        qDebug() << "DB OK";
    } else {
        qDebug() << "DB ERR";
        QMessageBox::warning(this, "Advertencia", "No fue posible conectarse con la base de datos.");
    }

    //  PicKit IPE parameters
    ipeAppState = Pickit::Off;
    ipeApp.setIpePath(jigsaw->getConfigIpePath());
    ipeApp.setPickitSerialNumber(jigsaw->getPickitSerialNumber());

    connect(&ipeApp, &Pickit::ipeOutput, [this](const QString &output) {
        this->outputInfo(output, normal);
    });
    connect(&ipeApp, &Pickit::ipeWarning, this, &MainWindow::ipeAppWarning);
    connect(&ipeApp, &Pickit::ipeMessage, this, &MainWindow::ipeAppMessage);

    ui->pushButtonStart->setEnabled(false);
    connect(ui->actionSalir, &QAction::triggered, this, &MainWindow::close);

    //loadConfig();
    ui->lineEdit_IPEPath->setText(jigsaw->getConfigIpePath());
    ui->lineEdit_hexPath->setText(jigsaw->getConfigHexPath());
    ui->lineEdit_PICKitBUR->setText(jigsaw->getPickitSerialNumber());

    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->pushButton_opcional1->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->pushButton_opcional1->addAction(ui->actionEditOptionalCommand);

    ui->progressBar->setValue(0);

    //  TimeOut Timer
    connect(&timerTimeOut, &QTimer::timeout, [this]() { this->timerTimeOutDone = true; });

    profileLoaded = false;
}

MainWindow::~MainWindow()
{
    ipeApp.close();
    delete ui;
}

void MainWindow::readAppConfigFile(QByteArray data)
{
    QJsonDocument configJsonDocument(QJsonDocument::fromJson(data));

    QJsonObject jsonObject = configJsonDocument.object();

    //  Database
    if (jsonObject.contains("database") && jsonObject["database"].isObject()) {
        QJsonObject databaseObject = jsonObject["database"].toObject();

        if (databaseObject.contains("hostName") && databaseObject["hostName"].isString()) {
            dbHostName = databaseObject["hostName"].toString();
        }

        if (databaseObject.contains("databaseName") && databaseObject["databaseName"].isString()) {
            dbDatabaseName = databaseObject["databaseName"].toString();
        }

        if (databaseObject.contains("userName") && databaseObject["userName"].isString()) {
            dbUserName = databaseObject["userName"].toString();
        }

        if (databaseObject.contains("password") && databaseObject["password"].isString()) {
            dbPassword = databaseObject["password"].toString();
        }
    } else {
        qWarning() << "Configuración de la base de datos no válida";
        QMessageBox::warning(this,
                             "Advertencia",
                             "La configuración de la base de datos tiene errores.");
    }

    //  API
    if (jsonObject.contains("api") && jsonObject["api"].isObject()) {
        QJsonObject apiObject = jsonObject["api"].toObject();

        if (apiObject.contains("hostName") && apiObject["hostName"].isString()) {
            apiHostName = apiObject["hostName"].toString();
        }

        if (apiObject.contains("userName") && apiObject["userName"].isString()) {
            apiUserName = apiObject["userName"].toString();
        }

        if (apiObject.contains("password") && apiObject["password"].isString()) {
            apiPassword = apiObject["password"].toString();
        }

        if (apiObject.contains("port")) {
            apiPort = static_cast<quint16>(apiObject["port"].toInt());
        }
    } else {
        qWarning() << "Configuración de la API no válida";
        QMessageBox::warning(this, "Advertencia", "La configuración de la API tiene errores.");
    }

    //  API para pruebas LORA
    device = new LoraServerApi;

    device->login(apiHostName, apiPort, apiUserName, apiPassword);
    device->setApplicationId("1");
    device->setDeviceProfileId("7292e595-8850-435a-b82f-32c06b40c97a");
}

void MainWindow::refreshTable()
{
    ui->tableWidget->clear();
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(5);

    QStringList headers;
    headers << "Prueba"
            << "Mínimo"
            << "Medido"
            << "Máximo"
            << "Estado";
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    int i = 0;
    foreach (JigCommand *cmd, jigsaw->commands) {
        ui->tableWidget->insertRow(i);

        ui->tableWidget->setItem(i, 0, cmd->commandItem);
        ui->tableWidget->setItem(i, 1, cmd->minItem);
        ui->tableWidget->setItem(i, 2, cmd->measuredItem);
        ui->tableWidget->setItem(i, 3, cmd->maxItem);
        ui->tableWidget->setItem(i, 4, cmd->resultItem);
        i++;
    }

    ui->tableWidget->resizeColumnToContents(0);
    ui->tableWidget->setColumnWidth(1, 125);
    ui->tableWidget->setMaximumWidth(
        ui->tableWidget->columnWidth(0) + ui->tableWidget->columnWidth(1)
        + ui->tableWidget->columnWidth(2) + ui->tableWidget->columnWidth(3)
        + ui->tableWidget->columnWidth(4) + 50);
}

void MainWindow::moveCommand(int from, int to)
{
    QList<QTableWidgetItem *> items;

    if ((from < 0) || (to < 0))
        return;

    if (to >= jigsaw->commands.size())
        return;

    jigsaw->commands.move(from, to);

    for (int i = 0; i < 5; i++)
        items << ui->tableWidget->takeItem(from, i);

    ui->tableWidget->removeRow(from);

    ui->tableWidget->insertRow(to);

    for (int i = 0; i < 5; i++)
        ui->tableWidget->setItem(to, i, items[i]);
}

void MainWindow::mainStateMachine()
{
    bool state = false;
    QString cmdSend;
    QString cmdEcho;
    QString answer;
    QString rxAnswers;
    QString res2;
    QString insert_db;
    QString textToLabel;
    QStringList dataResponseList;
    QString formula;
    QJSValue evaluator;
    qint32 wildcardInt;
    int stateMachineTimerInterval = 100;
    int ans;

    qint64 tiempoOperacion = 0;

    bool saveDatabaseData = false;

    static bool dataOk = false;
    static bool onOk = false;

    QMessageBox::StandardButtons r;

    QByteArray array;

    int cmdEchoIndex;
    int answerIndex;

    double completed;

    //      Date and time into variables list
    do {
        wildcard["%D"] = QDateTime::currentDateTime().toString("dd/MM/yy HH:mm:ss").toLatin1();

        stateMachineTimer.stop();

        if (!profileLoaded) {
            currentState = stateCheckProfile;
            nextState = stateCheckProfile;
        }

        switch (currentState) {
        case stateInitApp:
            globalCommandJump = false;
            nextState = stateCheckProfile;
            break;
        case stateCheckProfile:
            ui->labelOutput->setText("Abra el perfil a utilizar");
            ui->labelOutput->setStyleSheet("background-color: normal;");

            if (profileLoaded)
                nextState = stateIdle;

            break;
        case stateIdle:
            if (lastState == stateCheckProfile) {
                ui->labelOutput->setText("Escanee o ingrese el número de serie");
                ui->labelOutput->setStyleSheet("background-color: normal;");
            }
            break;
        case stateReadyToStart:
            if (lastState == stateIdle) {
                ui->pushButtonStart->setEnabled(true);
                ui->pushButtonStart->setFocus();

                ui->labelOutput->setStyleSheet("background-color: normal;");
                ui->labelOutput->setText("Pulse 'Iniciar' para empezar");
            }
            break;
        case stateStart:
            ui->pushButtonStart->setEnabled(false);
            ui->progressBar->setValue(0);
            ui->statusBar->showMessage("Iniciando Pruebas");
            ui->labelOutput->setText("ESPERE");
            ui->labelOutput->setStyleSheet("background-color: normal;");
            ui->tableWidget->scrollToTop();

            reportFails.clear();
            reportComment.clear();
            reportMeasure.clear();
            chrono.restart();

            globalCommandJump = false;
            runingCommand = false;

            wildcard.clear();
            wildcard["%serialNumber"] = QByteArray::number(serialNumber);

            errors = 0;
            currentCommandIndex = 0;
            nextState = stateReadCommand;
            break;
        case stateReadCommand:
            runingCommand = false;

            completed = static_cast<double>(currentCommandIndex)
                        / static_cast<double>(jigsaw->commands.size());
            ui->progressBar->setValue(static_cast<int>(completed * 100));

            if (currentCommandIndex >= jigsaw->commands.size()) {
                nextState = stateFinish;
                break;
            }

            currentCommand = jigsaw->commands[currentCommandIndex];

            ui->tableWidget->scrollToItem(currentCommand->commandItem);

            ifCommand = currentCommand->getInterfaceCommand();
            ifArguments = currentCommand->getInterfaceArguments();
            ifAnswer = currentCommand->getInterfaceAnswer();
            ifAlias = currentCommand->getInterfaceName();
            interface = jigsaw->interfaces[ifAlias];

            if (currentCommand->commandItem->checkState() == Qt::Checked)
                currentCommand->setEnabled(true);
            else if (currentCommand->commandItem->checkState() == Qt::Unchecked)
                currentCommand->setEnabled(false);

            currentCommandIndex++;

            timeoutCount = 0;

            wildcard["%2db"] = "na";

            nextState = stateProcessCommand;

            //      Comprueba si hay demora
            if (currentCommand->getEnabled() && (currentCommand->getTimeDelay() > 0)) {
                currentCommand->setState(JigCommand::delay);
                nextState = stateDelayCommand;
            }

            //      Check if command has to be executed
            if (!currentCommand->getEnabled() || (!currentCommand->runOnJump && globalCommandJump)) {
                currentCommand->setState(JigCommand::jump);
                stateMachineTimerInterval = 10;
                nextState = stateReadCommand;
            }
            break;
        case stateDelayCommand:
            QThread::msleep(static_cast<unsigned long>(currentCommand->getTimeDelay()));
            nextState = stateProcessCommand;
            break;
        case stateProcessCommand:
            runingCommand = true;

            currentCommand->setState(JigCommand::running);
            outputInfo("*****************************************************", normal);
            if (!globalCommandJump) {
                outputInfo(QString("Ejecutando el comando %1:%2")
                               .arg(currentCommandIndex)
                               .arg(currentCommand->getId()),
                           normal);
            } else {
                outputInfo(QString("Ejecutando el comando obligatorio %1:%2")
                               .arg(currentCommandIndex)
                               .arg(currentCommand->getId()),
                           normal);
            }

            //      Show Notice
            if (!currentCommand->messageNotice.isEmpty())
                QMessageBox::information(this, "Atención", currentCommand->messageNotice);

            switch (interface->getType()) {
            case JigInterface::none:
                errors++;
                outputInfo("No está configurada la interfaz", error);
                globalCommandJump = true;
                nextState = stateReadCommand;
                break;
            case JigInterface::app:
                nextState = stateApp;
                break;
            case JigInterface::plugin:
                nextState = statePlugin;
                break;
            case JigInterface::usb:
                if (!ifCommand.compare("program"))
                    nextState = stateIpeProgram;
                break;
            case JigInterface::tty:
                if (!ifCommand.compare("open"))
                    nextState = stateInterfaceOpen;

                if (!ifCommand.compare("close"))
                    nextState = stateInterfaceClose;

                if (!ifCommand.compare("usartXfer")) {
                    nextState = stateUsartWrite;

                    dataOk = false;
                    onOk = false;
                }
                break;
            }

            if (currentCommand->getUseMeanFormula()) {
                formula = currentCommand->getMeanFormula();

                if (formula.contains('%')) {
                    if (formula.contains("%x")) {
                        if (wildcard.keys().contains("%x")) {
                            if (!wildcard["%x"].isEmpty()) {
                                wildcardInt = wildcard["%x"].at(1) & 0x000000FF;
                                wildcardInt |= (wildcard["%x"].at(0) << 8) & 0x0000FF00;

                                formula.replace("%x", QString::number(wildcardInt));
                            } else {
                                QMessageBox::critical(this, "Error", "El comodín está vacío.");
                                lastCommandResult = stepErr;
                                nextState = stateProcessOutput;
                                break;
                            }
                        } else if (formula.contains("%f")) {
                            if (!wildcard["%f"].isEmpty()) {
                                wildcardInt = wildcard["%f"].at(1) & 0x000000FF;
                                wildcardInt |= (wildcard["%x"].at(0) << 8) & 0x0000FF00;

                                formula.replace("%f", QString::number(wildcardInt));
                            } else {
                                QMessageBox::critical(this, "Error", "El comodín está vacío.");
                                lastCommandResult = stepErr;
                                nextState = stateProcessOutput;
                                break;
                            }
                        } else {
                            QMessageBox::critical(
                                this,
                                "Error",
                                "El comodín solicitado %x no fue encontrado, por favor revise si "
                                "es correcta la configuración del perfil, o si algún comando "
                                "previo no se está ejecutando.");
                            lastCommandResult = stepErr;
                            nextState = stateProcessOutput;
                            break;
                        }
                    }
                }

                evaluator = jsEngine.evaluate(formula);
                qDebug() << evaluator.toNumber();

                currentCommand->setMeasureParameters(evaluator.toNumber(),
                                                     currentCommand->deviation,
                                                     currentCommand->offset);
            }

            retryState = nextState;
            lastCommandResult = stepNone;

            timerTimeOutDone = false;
            timerTimeOut.stop();
            timerTimeOut.start(currentCommand->getTimeOut());
            break;
        case stateApp:
            if (!ifCommand.compare("dialog")) {
                r = QMessageBox::information(this,
                                             "Advertencia",
                                             currentCommand->getInterfaceArguments(),
                                             QMessageBox::Ok | QMessageBox::No);

                if (r == QMessageBox::Ok) {
                    lastCommandResult = stepOk;
                } else {
                    lastCommandResult = stepErr;
                }

                nextState = stateProcessOutput;
                break;
            }

            if (!ifCommand.compare("deadtime")) {
                QThread::msleep(static_cast<unsigned long>(currentCommand->getTimeOut()));

                lastCommandResult = stepOk;
                nextState = stateProcessOutput;
                break;
            }

            if (!ifCommand.compare("loraserverapi")) {
                if (currentCommand->getTxCommand().contains("create")) {
                    device->createDevice(
                        currentCommand
                            ->getTxArguments()); //  Esto debería salir de aqui y enviarlo a otro proceso
                    device->setKeys(currentCommand->getTxArguments(),
                                    "00000000000000000000000000000000",
                                    "abfae4d33b42258bcf742cc51178f1d4");
                    lastCommandResult = stepOk;
                }

                nextState = stateProcessOutput;
                break;
            }

            if (!ifCommand.compare("end")) {
                globalCommandJump = true;
                nextState = stateReadCommand;
                break;
            }

            if (timerTimeOutDone) {
                timeoutCount++;
                lastCommandResult = stepTimeout;
                nextState = stateProcessOutput;
                break;
            }
            break;
        case statePlugin:
            interface->setIfArguments(ifArguments, wildcard);
            interface->setAnswers(ifAnswer);

            if (interface->open() == 0) {
                lastCommandResult = stepOk;
                nextState = statePluginWait;
            } else {
                lastCommandResult = stepErr;
                nextState = stateProcessOutput;
            }
            break;
        case statePluginWait:
            if (interface->isDone()) {
                if (interface->getResult() == 0) {
                    if (!ifAnswer.isEmpty()) {
                        dataResponseList = interface->getAnswers();
                        if (ifAnswer.contains('%')) {
                            if (ifAnswer.contains("%x")) {
                                wildcard["%x"] = QByteArray::fromHex(
                                    dataResponseList.at(0).toLatin1());
                            }
                        }
                    }

                    lastCommandResult = stepOk;
                } else {
                    lastCommandResult = stepErr;
                }
                nextState = stateProcessOutput;
                break;
            }

            if (timerTimeOutDone) {
                timeoutCount++;
                lastCommandResult = stepTimeout;
                nextState = stateProcessOutput;
                break;
            }
            break;
        case stateIpeProgram:
            if (ipeApp.runPickit()) {
                outputInfo("Arrancando programador...", normal);
                ui->statusBar->showMessage("Arrancando programador");
                nextState = stateIpeWait;
            } else {
                outputInfo("Estado del programador desconocido...", error);
                errors++;
                globalCommandJump = true;
                lastCommandResult = stepErr;
                nextState = stateProcessOutput;
            }
            if (!ipeApp.isOpen()) {
                qDebug() << "IPE error";
                errors++;
                globalCommandJump = true;
                lastCommandResult = stepErr;
                nextState = stateProcessOutput;
            }
            break;
        case stateIpeWait:
            ipeAppState = ipeApp.state();

            if (ipeAppState == Pickit::ProgrammingComplete) {
                outputInfo("Programación OK", ok);
                ui->statusBar->showMessage("Programación Completa", 3000);
                ui->progressBar->setValue(6);
            }

            if (ipeAppState == Pickit::Success || ipeAppState == Pickit::Ready) {
                lastCommandResult = stepOk;
                nextState = stateProcessOutput;
            }
            if (ipeAppState == Pickit::Fail || ipeAppState == Pickit::Off) {
                errors++;
                lastCommandResult = stepErr;
                nextState = stateProcessOutput;
                outputInfo(ipeApp.lastError, error);
                ui->statusBar->showMessage("Error de programación");
            }

            if (timerTimeOutDone) {
                ipeApp.close();
                timeoutCount++;
                lastCommandResult = stepTimeout;
                nextState = stateProcessOutput;
                outputInfo("Error de programación - Tiempo excedido", error);
                ui->statusBar->showMessage("Error de programación - Tiempo excedido");
            }
            break;
        case stateInterfaceOpen:
            ans = interface->open();

            if (ans == 0) {
                lastCommandResult = stepOk;
                nextState = stateProcessOutput;
            } else if (ans == -1) {
                lastCommandResult = stepErr;
                nextState = stateProcessOutput;
            }

            if (timerTimeOutDone) {
                timeoutCount++;
                lastCommandResult = stepTimeout;
                nextState = stateProcessOutput;
                break;
            }
            break;
        case stateInterfaceClose:
            if (interface->isOpen()) {
                interface->close();
            } else if (globalCommandJump) {
                outputInfo(QString("Salta el comando por que la interfaz %1 no está abierta.")
                               .arg(currentCommand->getInterfaceName()),
                           infoType::normal);
                currentCommand->setState(JigCommand::jump);
                nextState = stateReadCommand;
                break;
            }

            lastCommandResult = stepOk;
            nextState = stateProcessOutput;
            break;
        case stateUsartWrite:
            if (timerTimeOutDone) {
                timeoutCount++;
                lastCommandResult = stepTimeout;
                nextState = stateProcessOutput;
                break;
            }

            interface->rxBuffer.clear();

            if (!interface->isOpen()) {
                if (globalCommandJump) {
                    outputInfo(QString("Salta el comando por que la interfaz %1 no está abierta.")
                                   .arg(currentCommand->getInterfaceName()),
                               infoType::normal);
                    currentCommand->setState(JigCommand::jump);
                    nextState = stateReadCommand;
                    break;
                }

                outputInfo(QString("No esta abierta la interface %1")
                               .arg(currentCommand->getInterfaceName()),
                           infoType::error);
                globalCommandJump = true;
                lastCommandResult = stepErr;
                nextState = stateProcessOutput;
                break;
            }

            if (currentCommand->isTxCommand()) {
                if (currentCommand->getTxCommand().contains(
                        "%")) { //        Wildcard for random variables
                    buildCommand = wildcard[currentCommand->getTxCommand()];
                } else {
                    buildCommand = currentCommand->getTxCommand().toLatin1();
                }

                if (currentCommand->isTxArguments()) {
                    buildCommand.append(" ");

                    if (currentCommand->getTxArguments().contains(
                            "%")) { //        Wildcard for random variables
                        buildCommand.append(wildcard[currentCommand->getTxArguments()]);
                    } else {
                        buildCommand.append(currentCommand->getTxArguments());
                    }
                }

                if (!currentCommand->getOnOk().isEmpty() || !currentCommand->getOnError().isEmpty()
                    || !currentCommand->getRxCommand().isEmpty()) {
                    qDebug() << "TX:" << buildCommand << "; WAIT:" << currentCommand->getOnOk();
                    outputInfo(QString("Tx: %1 [espero: %2]")
                                   .arg(QString::fromLatin1(buildCommand))
                                   .arg(currentCommand->getOnOk()),
                               normal);
                    nextState = stateUsartRead;
                } else {
                    outputInfo(QString("Tx: %1").arg(QString::fromLatin1(buildCommand)), normal);
                    lastCommandResult = stepOk;
                    nextState = stateProcessOutput;
                }

                commandToSend = buildCommand;

                if (currentCommand->getIsCrc16()) {
                }

                if (currentCommand->getIsCr()) {
                    commandToSend.append('\r');
                }

                if (currentCommand->getIsLf()) {
                    commandToSend.append('\n');
                }

                interface->write(commandToSend);
            } else if (!currentCommand->getRxCommand().isEmpty()) {
                nextState = stateUsartRead;
            } else {
                nextState = stateUsartRead;
            }
            break;
        case stateUsartRead:
            if (timerTimeOutDone) {
                timeoutCount++;
                lastCommandResult = stepTimeout;
                nextState = stateProcessOutput;
                break;
            }

            if (!interface->isOpen()) {
                outputInfo(QString("No esta abierta la interface %1")
                               .arg(currentCommand->getInterfaceName()),
                           infoType::error);
                globalCommandJump = true;
                lastCommandResult = stepErr;
                nextState = stateProcessOutput;
                break;
            }

            if (interface->dataReady()) {
                if (currentCommand->isTxCommand()) {
                    if (!currentCommand->getOnOk().isEmpty()
                        || !currentCommand->getOnError().isEmpty()) {
                        if (interface->rxBuffer.contains(
                                QString("\r" + currentCommand->getOnOk() + "\r\n").toLatin1())
                            || interface->rxBuffer.contains(
                                   QString("\r\n" + currentCommand->getOnOk() + "\r\n").toLatin1())) {
                            dataOk = true;
                            onOk = true;
                        }

                        if (interface->rxBuffer.contains(
                                QString("\r" + currentCommand->getOnError() + "\r\n").toLatin1())
                            || interface->rxBuffer.contains(
                                   QString("\r\n" + currentCommand->getOnError() + "\r\n")
                                       .toLatin1())) {
                            dataOk = true;
                        }
                    } else if (!currentCommand->getRxCommand().isEmpty()) {
                        if (interface->rxBuffer.contains(
                                QString(currentCommand->getRxCommand()).toLatin1())) {
                            dataOk = true;
                            onOk = true;
                        }
                    }
                } else if (currentCommand->isRxCommand()) {
                    if (interface->rxBuffer.contains(
                            QString(currentCommand->getRxCommand()).toLatin1())) {
                        dataOk = true;
                        onOk = true;
                    }
                } else {
                    if (interface->rxBuffer.contains(
                            QString(currentCommand->getOnOk() + "\r\n").toLatin1())) {
                        dataOk = true;
                        onOk = true;
                    }

                    if (interface->rxBuffer.contains(
                            QString(currentCommand->getOnError() + "\r\n").toLatin1())) {
                        dataOk = true;
                    }
                }
            }

            if (dataOk) {
                nextState = stateProcessOutput;
                cmdEchoIndex = -1;

                QByteArray::iterator it;
                QString dataReceivedClean = "";
                for (it = interface->rxBuffer.begin(); it < interface->rxBuffer.end(); it++) {
                    if ((*it == '\r') || (*it == '\n') || ((*it >= 0x20) && (*it < 0x7F))) {
                        dataReceivedClean.append(*it);
                    }
                }

                dataReceivedClean.remove(QRegExp("\n"));
                dataResponseList = dataReceivedClean.split("\r");

                outputInfo(QString("Rx: %1 [bytes: %2]")
                               .arg(dataResponseList.join(";"))
                               .arg(dataReceivedClean.size()),
                           normal);

                dataResponseList.removeAll("");

                if (dataResponseList.isEmpty()) {
                    errors++;
                    qDebug() << "error" << errors;
                    outputInfo("Se descartó la información recibida, posiblemente el tiempo de "
                               "espera fue muy corto.",
                               error);
                    globalCommandJump = true;
                    lastCommandResult = stepErr;
                    nextState = stateProcessOutput; //      Fall out error
                    break;
                }

                qDebug() << "RX:" << dataResponseList;

                if (currentCommand->isRxCommand()) {
                    answerIndex = dataResponseList.size() - 1;
                } else {
                    if (onOk) {
                        answerIndex = dataResponseList.indexOf(currentCommand->getOnOk());
                    } else {
                        answerIndex = dataResponseList.indexOf(currentCommand->getOnError());
                    }
                }

                if (answerIndex >= 0)
                    answer = dataResponseList.at(answerIndex);

                if (answer.isEmpty()) {
                    errors++;
                    qDebug() << "error" << errors;
                    lastCommandResult = stepErr;
                    nextState = stateUsartWrite;
                    break;
                }

                if (currentCommand->isTxCommand()) {
                    cmdEchoIndex = dataResponseList.indexOf(buildCommand);

                    if ((cmdEchoIndex >= 0) && (answerIndex > cmdEchoIndex)) {
                        cmdEcho = dataResponseList.at(cmdEchoIndex);
                        rxAnswers = dataResponseList.at(cmdEchoIndex + 1);
                    }
                }

                if (currentCommand->isRxCommand()) {
                    dataResponseList = answer.split(' ');

                    cmdEcho = dataResponseList.takeAt(0);
                    rxAnswers = dataResponseList.join(' ');

                    if (currentCommand->getRxCommand().compare(cmdEcho) == 0)
                        lastCommandResult = stepOk;
                }

                if (currentCommand->isRxAnswers()) {
                    if (currentCommand->getRxAnswers().contains('%')) { //      Variable comodín
                        wildcard[currentCommand->getRxAnswers()] = rxAnswers.toLatin1();
                        wildcard["%2db"] = rxAnswers.toLatin1();

                        if (!currentCommand->getRxAnswers().compare("%D")) {
                            QDateTime t1 = QDateTime::fromString(rxAnswers, "dd/MM/yyyy HH:mm:ss");
                            QDateTime t2 = QDateTime::currentDateTime();

                            qDebug() << t2.secsTo(t1);

                            if (abs(t2.secsTo(t1)) < 10) {
                                lastCommandResult = stepOk;
                            } else {
                                lastCommandResult = stepErr;
                            }
                        } else if (!currentCommand->getRxAnswers().compare("%measure")) {
                            if (!currentCommand->getMeasFormula().isEmpty()) {
                                formula = currentCommand->getMeasFormula();

                                if (formula.contains("%")) {      //  Wildcard
                                    if (formula.contains("%f")) { // floating
                                        formula.replace("%f", rxAnswers);
                                    }
                                }

                                evaluator = jsEngine.evaluate(formula);
                                qDebug() << evaluator.toNumber();

                                if (currentCommand->setMeasure(evaluator.toNumber())) {
                                    lastCommandResult = stepOk;
                                } else {
                                    lastCommandResult = stepErr;
                                }

                            } else if (currentCommand->setMeasure(rxAnswers.toDouble())) {
                                lastCommandResult = stepOk;
                            } else {
                                lastCommandResult = stepErr;
                            }
                        } else if (!currentCommand->getRxAnswers().compare("%serialNumber")) {
                            if (QString::number(serialNumber).compare(rxAnswers) == 0) {
                                lastCommandResult = stepOk;
                            } else {
                                lastCommandResult = stepErr;
                            }
                        } else if (!currentCommand->getRxAnswers().compare("%x")) {
                            wildcard[currentCommand->getRxAnswers()] = QByteArray::fromHex(
                                rxAnswers.toLatin1());
                        }
                    } else if (currentCommand->getRxAnswers().compare(rxAnswers) == 0) {
                        lastCommandResult = stepOk;
                    }
                }

                if (currentCommand->isOnOk() && !currentCommand->getOnOk().compare(answer)) {
                    if (lastCommandResult != stepErr)
                        lastCommandResult = stepOk;
                }

                if (currentCommand->isOnError() && !currentCommand->getOnError().compare(answer)) {
                    lastCommandResult = stepErr;

                    if (currentCommand->isRxAnswers() && !rxAnswers.isEmpty()) {
                        if (currentCommand->getRxAnswers().contains("%")) { //      Variable comodín

                            if (!currentCommand->getRxAnswers().compare("%measure")) {
                                currentCommand->measuredItem->setText(rxAnswers);
                                currentCommand->measuredItem->setTextColor("red");
                            }
                        }
                    }
                    break;
                }
            }
            break;
        case stateProcessOutput:

            switch (lastCommandResult) {
            case stepNone:
                break;
            case stepOk: //  OK
                if (!currentCommand->messageOnOk.isEmpty())
                    outputInfo(currentCommand->messageOnOk, ok);
                outputInfo("OK", infoType::ok);
                currentCommand->setState(JigCommand::ok);

                nextState = stateReadCommand;
                break;
            case stepErr: //  ERROR
                errors++;

                if (!currentCommand->messageOnError.isEmpty())
                    outputInfo(currentCommand->messageOnError, error);
                outputInfo("ERROR", infoType::error);
                currentCommand->setState(JigCommand::fail);

                if (currentCommand->end)
                    globalCommandJump = true;

                reportFails.append(QString::number(currentCommandIndex) + ":"
                                   + currentCommand->getId() + ";");
                reportComment.append(currentCommand->messageOnError + ";");
                reportMeasure.append(wildcard["%2db"] + ";");

                nextState = stateReadCommand;
                break;
            case stepTimeout:
                outputInfo("Se ha excedido el tiempo limite de ejecución de comando", normal);

                if (timeoutCount >= 3) {
                    outputInfo("TIMEOUT", error);
                    ui->statusBar->showMessage("Error en la prueba");

                    currentCommand->setState(JigCommand::fail);

                    globalCommandJump = true;
                    errors++;

                    nextState = stateReadCommand;
                } else {
                    outputInfo("Reintento...", normal);
                    timerTimeOutDone = false;
                    timerTimeOut.stop();
                    timerTimeOut.start(currentCommand->getTimeOut());

                    nextState = retryState;
                }
                break;
            }

            qDebug() << "";

            break;
        case stateFinish:
            tiempoOperacion = chrono.elapsed();
            ui->labelOutput->setText("ESPERE...");

            qDebug() << tiempoOperacion;

            //  Cierra los puertos seriales
            foreach (JigInterface *interface, jigsaw->interfaces) {
                if (interface->getType() == JigInterface::tty) {
                    interface->close();
                }
            }

            if (errors == 0) {
                state = true;
                ui->labelOutput->setText("PASA");
                ui->labelOutput->setStyleSheet("background-color: green;");
                reportFails.append("ok");
                reportComment.append("ok");
                reportMeasure.append("ok");
            } else {
                state = false;
                ui->labelOutput->setText("NO PASA");
                ui->labelOutput->setStyleSheet("background-color: red;");

                if (reportFails.isEmpty())
                    reportFails.append("fail");
                if (reportComment.isEmpty())
                    reportComment.append("fail");
                if (reportMeasure.isEmpty())
                    reportMeasure.append("fail");
            }

            saveDatabaseData = true; //  Guarda los datos en la base de datos

            ui->progressBar->setValue(100);
            ui->pushButtonStart->setEnabled(true);
            ui->lineEdit_numeroSerie->clear();
            ui->lineEdit_numeroSerie->setEnabled(true);

            globalCommandJump = false;

            nextState = stateIdle;
            break;
        case stateError:
            ui->labelOutput->setText("Desconecte el cable USB y reinicie");
            ui->labelOutput->setStyleSheet("background-color: red;");
            break;
        }

        //      Update last state
        lastState = currentState;
        if (nextState != stateInitApp)
            currentState = nextState;
    } while (globalCommandJump && !runingCommand);

    if (saveDatabaseData) {
        QSqlDatabase db = QSqlDatabase::database("dbConexion");
        QSqlQuery query(db);

        query.prepare("INSERT INTO RegistroJIG ("
                      "RegistroJIGProducto,"
                      "RegistroJIGNumeroSerie,"
                      "RegistroJIGEstado,"
                      "RegistroJIGFallas,"
                      "RegistroJIGObservaciones,"
                      "RegistroJIGMedicion,"
                      "RegistroJIGFecha,"
                      "RegistroJIGTiempoOperacion)"
                      "VALUES ("
                      ":idProducto,"
                      ":serialNumber,"
                      ":state,"
                      ":report,"
                      ":notes,"
                      ":measure,"
                      "CURRENT_TIMESTAMP(),"
                      ":tiempoOperacion)");
        query.bindValue(":idProducto", idProducto);
        query.bindValue(":serialNumber", wildcard["%serialNumber"]);
        query.bindValue(":state", state);
        query.bindValue(":report", reportFails);
        query.bindValue(":notes", reportComment);
        query.bindValue(":measure", reportMeasure);
        query.bindValue(":tiempoOperacion", tiempoOperacion);

        if (!query.exec()) {
            QMessageBox::warning(this,
                                 "Advertencia",
                                 "No fue posible registrar la información en la base de datos, por "
                                 "favor reinicie la aplicación y repita la prueba.");
            qDebug() << query.lastError().text();
        }
    }

    stateMachineTimer.setInterval(stateMachineTimerInterval);
    stateMachineTimer.start();
}

void MainWindow::ipeAppMessage()
{
    Pickit::ipeStates state = ipeApp.state();

    if (state == Pickit::TargetFound) {
        outputInfo("Equipo detectado", normal);
        ui->statusBar->showMessage("Equipo detectado");
        ui->labelOutput->setStyleSheet("background-color: normal");
        ui->labelOutput->setText("Dispositivo detectado");
    }

    if (state == Pickit::TargetRemoved) {
        ui->labelOutput->setStyleSheet("background-color: normal;");
        ui->labelOutput->setText("Inserte una placa");
        ui->statusBar->showMessage("Equipo removido");
    }
}

void MainWindow::ipeAppWarning()
{
    Pickit::ipeStates state = ipeApp.state();

    if (state == Pickit::TargetNotFound) {
        outputInfo(ipeApp.lastError, error);
        ui->statusBar->showMessage("Error de programación");
    }
}

void MainWindow::on_lineEdit_numeroSerie_returnPressed()
{
    QString msg;
    quint64 tempSerialNumber = static_cast<quint64>(ui->lineEdit_numeroSerie->text().toLongLong());
    if (tempSerialNumber != 0) {
        msg = QString("Detectado Numero de serie %1\r").arg(tempSerialNumber);
        serialNumber = static_cast<quint64>(tempSerialNumber);
        eui = ui->lineEdit_numeroSerie->text();

        ui->tbConsole->clear();
        outputInfo(msg, normal);

        if (serialNumber != 0 && ui->lineEdit_numeroSerie->text().length() == 11)
            nextState = stateReadyToStart;

        foreach (JigCommand *cmd, jigsaw->commands) {
            cmd->clearMeasure();
            cmd->setState(JigCommand::pending);
        }
    }
}

void MainWindow::on_pbIpe_clicked()
{
    QString path;
    path = QFileDialog::getOpenFileName(this,
                                        "Ruta del script del IPE",
                                        "/opt/microchip/mplabx/",
                                        "IPE (ipecmd.sh | ipecmd.exe)");
    if (!path.isEmpty()) {
        jigsaw->setConfigIpePath(path);
        ui->lineEdit_IPEPath->setText(jigsaw->getConfigIpePath());
        ipeApp.setIpePath(jigsaw->getConfigIpePath());
        configUpdate = true;
    }
}

void MainWindow::on_pbPicKit_clicked()
{
    ui->lineEdit_PICKitBUR->setReadOnly(false);
}

void MainWindow::on_pbHex_clicked()
{
    QString path;
    path = QFileDialog::getOpenFileName(this,
                                        "Ruta del HEX",
                                        QDir::homePath(),
                                        "Archivos HEX (*.hex)");
    if (!path.isEmpty()) {
        jigsaw->setConfigHexPath(path);
        ui->lineEdit_hexPath->setText(jigsaw->getConfigHexPath());
        ipeApp.setHexPath(jigsaw->getConfigHexPath());
        configUpdate = true;
    }
}

void MainWindow::on_lineEdit_PICKitBUR_returnPressed()
{
    ui->lineEdit_PICKitBUR->setReadOnly(true);
    jigsaw->setPickitSerialNumber(ui->lineEdit_PICKitBUR->text());
    ipeApp.setPickitSerialNumber(jigsaw->getPickitSerialNumber());
    configUpdate = true;
}

void MainWindow::outputInfo(QString info, infoType type)
{
    QString color;
    switch (type) {
    case normal:
        color = "black";
        break;
    case ok:
        color = "green";
        break;
    case error:
        color = "red";
        break;
    }

    auto textCursor = ui->tbConsole->textCursor();
    textCursor.movePosition(QTextCursor::End);
    ui->tbConsole->setTextCursor(textCursor);

    QString text = QString("<font color =\"%1\">%2</font><br>").arg(color).arg(info);
    ui->tbConsole->insertHtml(text);

    ui->tbConsole->verticalScrollBar()->setValue(ui->tbConsole->verticalScrollBar()->maximum());
}

void MainWindow::on_pushButtonStart_clicked()
{
    if (jigsaw->getConfigIpePath().isEmpty()) {
        QMessageBox::warning(this, "Alerta", "Ruta del IPE no especificada");
        return;
    }
    if (jigsaw->getConfigHexPath().isEmpty()) {
        QMessageBox::warning(this, "Alerta", "Ruta del HEX no especificada");
        return;
    }
    if (jigsaw->getPickitSerialNumber().isEmpty()) {
        QMessageBox::warning(this, "Alerta", "Numero de serie del programador no especificado");
        return;
    }
    if (!QFile(jigsaw->getConfigIpePath()).exists()) {
        QMessageBox::warning(this, "Alerta", "La ruta de MPLAB X IPE es incorrecta");
        return;
    }
    if (ui->lineEdit_numeroSerie->text().isEmpty()) {
        QMessageBox::warning(this, "Alerta", "Ingresa un número de serie para continuar");
        return;
    }
    if (jigsaw->commands.size() == 0) {
        QMessageBox::warning(this, "Alerta", "No se ha cargado un perfil de pruebas");
        return;
    }

    outputInfo(QString("Iniciando pruebas del PCBA sn/%1").arg(serialNumber), normal);
    qDebug() << "Iniciando test";
    nextState = stateStart;
    ui->lineEdit_numeroSerie->setEnabled(false);

    return;
}

void MainWindow::on_pushButtonStop_clicked()
{
    if ((nextState != stateInitApp) && (nextState != stateCheckProfile)
        && (nextState != stateIdle)) {
        ipeApp.close();
        errors++;
        globalCommandJump = true;

        reportFails.append("stop;");
        reportComment.append("stop;");
        reportMeasure.append("stop;");

        nextState = stateReadCommand;
    }
}

void MainWindow::on_actionLoad_triggered()
{
    int res;
    QString path;
    path = QFileDialog::getOpenFileName(this,
                                        "Ruta del perfil",
                                        QDir::homePath(),
                                        "Archivos json (*.json)");

    ui->tbConsole->clear();

    //      Read
    if (path.isEmpty())
        return;

    QFile configFile(path);

    if (!configFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open profile.");
        return;
    }

    QByteArray data = configFile.readAll();

    configFile.close();

    res = jigsaw->readProfile(data);

    if (res < 0) {
        //      Error
        QMessageBox::critical(this, "Error", "El archivo de configuración tiene errores.");
        return;
    }

    ipeApp.setTargetName(jigsaw->getPickitTarget());
    ipeApp.setPickitSerialNumber(jigsaw->getPickitSerialNumber());
    ipeApp.setIpePath(jigsaw->getConfigIpePath());
    ipeApp.setHexPath(jigsaw->getConfigHexPath());

    ipeApp.setMclrPinName(jigsaw->getPickitMclrPinName());

    ui->lineEdit_PICKitBUR->setText(jigsaw->getPickitSerialNumber());
    ui->lineEdit_IPEPath->setText(jigsaw->getConfigIpePath());
    ui->lineEdit_hexPath->setText(jigsaw->getConfigHexPath());

    QString productName = jigsaw->getProductName();
    QString productCode = jigsaw->getProductCode();

    //      Write
    if (!productName.isEmpty() && !productCode.isEmpty()) {
        ui->lbProfile->setText("Perfil actual:");
        ui->lineEditTarpuqCode->setText(productCode);
        ui->lineEditProjectName->setText(productName);
    }

    refreshTable();

    //  Datos extra ajenos al archivo JSON pero necesarios a partir de ahora
    QSqlDatabase db = QSqlDatabase::database("dbConexion");
    QSqlQuery query(db);
    query.prepare("SELECT idProducto FROM produccionIDi.Producto WHERE ProductoCodigo = :code");
    query.bindValue(":code", productCode);

    if (query.exec()) {
        qDebug() << query.executedQuery();
        while (query.next())
            idProducto = query.value(0).toInt();
    }

    commandEditor->setInterfaces(jigsaw->interfaces.keys());
    commandEditor->setCommands(jigsaw->commandList);

    profilePath = path;
    profileLoaded = true;
}

void MainWindow::on_actionSave_triggered()
{
    QByteArray data;
    int res;

    res = jigsaw->writeProfile(&data);

    if (res < 0) {
        //      Error
    }

    QFile saveFile(profilePath);

    saveFile.open(QIODevice::ReadWrite);
    saveFile.resize(0);
    saveFile.write(data);
    saveFile.close();
}

void MainWindow::on_actionInsert_triggered()
{
    JigCommand *command = new JigCommand(this);

    command->setOnOk("0");
    command->setOnError("1");
    command->setTimeOut(3000.0);
    command->setId("id");
    command->setCodeName("Nuevo Comando");

    jigsaw->commands.insert(tableRowReference, command);

    ui->tableWidget->insertRow(tableRowReference);
    ui->tableWidget->setItem(tableRowReference, 0, command->commandItem);
    ui->tableWidget->setItem(tableRowReference, 1, command->minItem);
    ui->tableWidget->setItem(tableRowReference, 2, command->measuredItem);
    ui->tableWidget->setItem(tableRowReference, 3, command->maxItem);
    ui->tableWidget->setItem(tableRowReference, 4, command->resultItem);
}

void MainWindow::on_actionEdit_triggered()
{
    commandEditor->loadCommand(tableRowReference, jigsaw->commands[tableRowReference]);
    commandEditor->show();
}

void MainWindow::on_actionRemove_triggered()
{
    jigsaw->commands[tableRowReference]->deleteLater();
    ui->tableWidget->removeRow(tableRowReference);
    jigsaw->commands.removeAt(tableRowReference);

    qDebug() << tableRowReference;
}

void MainWindow::on_actionDuplicate_triggered()
{
    JigCommand *duplicated = new JigCommand(this);
    JigCommand *original = jigsaw->commands[tableRowReference];
    *duplicated = *original;

    jigsaw->commands.insert(tableRowReference, duplicated);

    ui->tableWidget->insertRow(tableRowReference);
    ui->tableWidget->setItem(tableRowReference, 0, duplicated->commandItem);
    ui->tableWidget->setItem(tableRowReference, 1, duplicated->minItem);
    ui->tableWidget->setItem(tableRowReference, 2, duplicated->measuredItem);
    ui->tableWidget->setItem(tableRowReference, 3, duplicated->maxItem);
    ui->tableWidget->setItem(tableRowReference, 4, duplicated->resultItem);
}

void MainWindow::on_actionEditInterface_triggered()
{
    interfaceEditor->setInterfaces(jigsaw->interfaces.values());
    interfaceEditor->show();
}

void MainWindow::on_actionMove_Up_triggered()
{
    moveCommand(tableRowReference, tableRowReference - 1);
}

void MainWindow::on_actionMove_Down_triggered()
{
    moveCommand(tableRowReference, tableRowReference + 1);
}

void MainWindow::on_actionMove_First_triggered()
{
    moveCommand(tableRowReference, 0);
}

void MainWindow::on_actionMove_Last_triggered()
{
    moveCommand(tableRowReference, jigsaw->commands.size() - 1);
}

void MainWindow::on_actionSaveAs_triggered()
{
    QString path;
    path = QFileDialog::getSaveFileName(this,
                                        "Ruta del perfil",
                                        QDir::homePath(),
                                        "Archivos json (*.json)");

    if (!path.isEmpty()) {
        profilePath = path;
        on_actionSave_triggered();
    }
}

void MainWindow::on_tableWidget_itemChanged(QTableWidgetItem *item)
{
    if (item->column() == 0) {
        if (item->checkState() == Qt::Checked) {
            jigsaw->commands[item->row()]->setEnabled(true);
        } else {
            jigsaw->commands[item->row()]->setEnabled(false);
        }
    }
}

void MainWindow::on_checkBox_stateChanged(int arg1)
{
    qDebug() << arg1;
    if (arg1 == Qt::Unchecked) {
        foreach (JigCommand *cmd, jigsaw->commands) {
            cmd->setEnabled(false);
        }
    } else if (arg1 == Qt::Checked) {
        foreach (JigCommand *cmd, jigsaw->commands) {
            cmd->setEnabled(true);
        }
    }
}

void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this, "Acerca de Qt");
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,
                       "Acerca de TarpuqJIGsw",
                       "Creado en Tarpuq Cia. Ltda. \nAutor: Galo Guzmán");
}

void MainWindow::on_actionEditOptionalCommand_triggered()
{
    QMessageBox::information(this, "Editar comandos opcionales", "Proximamente");
}

void MainWindow::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    QMenu menu(this);
    QModelIndex cell = ui->tableWidget->indexAt(pos);
    tableRowReference = cell.row();

    if (cell.isValid()) {
        menu.addAction(ui->actionEdit);
        menu.addAction(ui->actionInsert);
        menu.addAction(ui->actionRemove);
        menu.addSeparator();
        menu.addAction(ui->actionDuplicate);
        menu.addAction(ui->actionMove_Up);
        menu.addAction(ui->actionMove_Down);
        menu.addAction(ui->actionMove_First);
        menu.addAction(ui->actionMove_Last);
        menu.exec(ui->tableWidget->mapToGlobal(pos));
    }
}

void MainWindow::on_pushButton_clicked()
{
    ui->lineEditTarpuqCode->setReadOnly(false);
}

void MainWindow::on_pushButton_2_clicked()
{
    ui->lineEditProjectName->setReadOnly(false);
}

void MainWindow::on_lineEditTarpuqCode_editingFinished()
{
    ui->lineEditTarpuqCode->setReadOnly(true);
}

void MainWindow::on_lineEditProjectName_editingFinished()
{
    ui->lineEditProjectName->setReadOnly(true);
}
