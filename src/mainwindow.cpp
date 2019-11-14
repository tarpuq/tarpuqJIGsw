#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QSettings>
#include <QStyleFactory>
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

    //  Serial Port
    checkPortTimer = new QTimer(this);
    serialBuffer = new QByteArray();

    //      Editors and Validators
    commandEditor = new JigCommandEditDialog(this);
    interfaceEditor = new JigInterfaceEditDialog(this);
    login = new JigLoginDialog(this);

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
    db.setConnectOptions();
    db.setConnectOptions("MYSQL_OPT_CONNECT_TIMEOUT=2");

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

    ui->progressBar->setValue(0);

    //  TimeOut Timer
    connect(&timerTimeOut, &QTimer::timeout, [this]() { this->timerTimeOutDone = true; });

    connect(login, &JigLoginDialog::isLogin, [this](bool ans) {
        this->setEnabled(true);
        if (ans) {
            debugMode = true;
            QMessageBox::information(this, "Aviso", "Modo Debug activado");
        } else {
            debugMode = false;
            QMessageBox::information(this, "Aviso", "Contraseña incorrecta");
        }
    });
    connect(login, &JigLoginDialog::finished, [this]() { this->setEnabled(true); });
    connect(login, &JigLoginDialog::rejected, [this]() { this->setEnabled(true); });

    profileLoaded = false;

    //  CommandList
    commandListUi = new CommandList(this);
    commandListSubwindow = new QMdiSubWindow(this);
    commandListSubwindow->setAttribute(Qt::WA_DeleteOnClose, false);
    commandListSubwindow->setWidget(commandListUi);
    ui->mdiArea->addSubWindow(commandListSubwindow);
    commandListUi->setCommandList(&jigsaw->syncCommands);

    asyncCommandListUi = new AsyncCommandList(this);
    asyncCommandListSubwindow = new QMdiSubWindow(this);
    asyncCommandListSubwindow->setAttribute(Qt::WA_DeleteOnClose, false);
    asyncCommandListSubwindow->setWidget(asyncCommandListUi);
    ui->mdiArea->addSubWindow(asyncCommandListSubwindow);

    connect(commandListUi, &CommandList::commandEdited, [this](JigSyncCommand *cmd) {
        commandEditor->loadCommand(0, cmd);
        commandEditor->show();
    });

    connect(asyncCommandListUi,
            &AsyncCommandList::opcionalCommandExecuted,
            this,
            &MainWindow::processAsyncCommand);

    //  Style
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette darkPalette;
    QColor darkColor = QColor(45, 45, 45);
    QColor disabledColor = QColor(127, 127, 127);
    darkPalette.setColor(QPalette::Window, darkColor);
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(18, 18, 18));
    darkPalette.setColor(QPalette::AlternateBase, darkColor);
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
    darkPalette.setColor(QPalette::Button, darkColor);
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);

    qApp->setPalette(darkPalette);

    qApp->setStyleSheet(
        "QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
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

QByteArray MainWindow::buildFrameToSend(JigSyncCommand *command)
{
    QByteArray ans;
    QByteArray builded;

    if (!command->getTxCommand().isEmpty()) {
        //        Wildcard for random variables
        if (command->getTxCommand().contains("%")) {
            builded = wildcard[command->getTxCommand()];
        } else {
            builded = command->getTxCommand().toLatin1();
        }

        if (command->isTxArguments()) {
            builded.append(" ");

            //        Wildcard for random variables
            if (command->getTxArguments().contains("%")) {
                builded.append(wildcard[command->getTxArguments()]);
            } else {
                builded.append(command->getTxArguments());
            }
        }
    }

    return builded;
}

void MainWindow::processAsyncCommand(JigSyncCommand *asyncCmd)
{
    QString ifName;
    JigInterface *iface;

    ifName = asyncCmd->getInterfaceName();
    iface = jigsaw->interfaces[ifName];

    if (asyncCmd->getInterfaceCommand() == "open") {
        if (!iface->isOpen()) {
            int ans = iface->open();

            if (ans == 0) {
                qDebug() << "ok";
            } else if (ans == -1) {
                qDebug() << "already open";
                return;
            } else if (ans == -2) {
                QString errorMsg
                    = QString("No se puede encontrar la interfaz %1, revise si está conectada.")
                          .arg(ifName);
                QMessageBox::warning(this, "Advertencia", errorMsg);
                return;
            }
        }
    } else if (asyncCmd->getInterfaceCommand() == "usartXfer") {
        QByteArray bCommand;
        QByteArray command2Send;

        if (!iface->isOpen()) {
            QString errorMsg = QString("La interfaz %1, no está abierta.").arg(ifName);
            QMessageBox::warning(this, "Advertencia", errorMsg);
            return;
        }

        command2Send = buildFrameToSend(asyncCmd);

        if (asyncCmd->getIsCrc16()) {
        }

        if (asyncCmd->getIsCr()) {
            command2Send.append('\r');
        }

        if (asyncCmd->getIsLf()) {
            command2Send.append('\n');
        }

        iface->write(command2Send);
    } else {
        QMessageBox::warning(this, "Advertencia", "El comando asíncrono aún no está implementado.");
    }
}

void MainWindow::refreshTable()
{
    commandListUi->clearTable();

    foreach (JigSyncCommand *cmd, jigsaw->syncCommands) {
        commandListUi->insertCommand(cmd);
    }
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

    QBrush brush;

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
            commandListUi->scrollToTop();

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
                        / static_cast<double>(jigsaw->syncCommands.size());
            ui->progressBar->setValue(static_cast<int>(completed * 100));

            if (currentCommandIndex >= jigsaw->syncCommands.size()) {
                nextState = stateFinish;
                break;
            }

            currentSyncCommand = jigsaw->syncCommands[currentCommandIndex];

            commandListUi->scrollToItem(currentSyncCommand->nameItem);

            ifCommand = currentSyncCommand->getInterfaceCommand();
            ifArguments = currentSyncCommand->getInterfaceArguments();
            ifAnswer = currentSyncCommand->getInterfaceAnswer();
            ifAlias = currentSyncCommand->getInterfaceName();
            interface = jigsaw->interfaces[ifAlias];

            if (currentSyncCommand->nameItem->checkState() == Qt::Checked)
                currentSyncCommand->setEnabled(true);
            else if (currentSyncCommand->nameItem->checkState() == Qt::Unchecked)
                currentSyncCommand->setEnabled(false);

            currentCommandIndex++;

            timeoutCount = 0;

            wildcard["%2db"] = "na";

            nextState = stateProcessCommand;

            //      Comprueba si hay demora
            if (currentSyncCommand->getEnabled() && (currentSyncCommand->getTimeDelay() > 0)) {
                currentSyncCommand->setState(JigSyncCommand::delay);
                nextState = stateDelayCommand;
            }

            //      Check if command has to be executed
            if (!currentSyncCommand->getEnabled()
                || (!currentSyncCommand->runOnJump && globalCommandJump)) {
                currentSyncCommand->setState(JigSyncCommand::jump);
                stateMachineTimerInterval = 10;
                nextState = stateReadCommand;
            }
            break;
        case stateDelayCommand:
            QThread::msleep(static_cast<unsigned long>(currentSyncCommand->getTimeDelay()));
            nextState = stateProcessCommand;
            break;
        case stateProcessCommand:
            runingCommand = true;

            currentSyncCommand->setState(JigSyncCommand::running);
            outputInfo("*****************************************************", normal);
            if (!globalCommandJump) {
                outputInfo(QString("Ejecutando el comando %1:%2")
                               .arg(currentCommandIndex)
                               .arg(currentSyncCommand->getId()),
                           normal);
            } else {
                outputInfo(QString("Ejecutando el comando obligatorio %1:%2")
                               .arg(currentCommandIndex)
                               .arg(currentSyncCommand->getId()),
                           normal);
            }

            //      Show Notice
            if (!currentSyncCommand->messageNotice.isEmpty())
                QMessageBox::information(this, "Atención", currentSyncCommand->messageNotice);

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
                if (ifCommand == "program")
                    nextState = stateIpeProgram;
                break;
            case JigInterface::tty:
                if (ifCommand == "open")
                    nextState = stateInterfaceOpen;

                if (ifCommand == "close")
                    nextState = stateInterfaceClose;

                if (ifCommand == "usartXfer") {
                    nextState = stateUsartWrite;

                    dataOk = false;
                    onOk = false;
                }
                break;
            }

            if (currentSyncCommand->getUseMeanFormula()) {
                formula = currentSyncCommand->getMeanFormula();

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

                currentSyncCommand->setMeasureParameters(evaluator.toNumber(),
                                                         currentSyncCommand->deviation,
                                                         currentSyncCommand->offset);
            }

            retryState = nextState;
            lastCommandResult = stepNone;

            timerTimeOutDone = false;
            timerTimeOut.stop();
            timerTimeOut.start(currentSyncCommand->getTimeOut());
            break;
        case stateApp:
            if (ifCommand == "dialog") {
                r = QMessageBox::information(this,
                                             "Advertencia",
                                             currentSyncCommand->getInterfaceArguments(),
                                             QMessageBox::Ok | QMessageBox::No);

                if (r == QMessageBox::Ok) {
                    lastCommandResult = stepOk;
                } else {
                    lastCommandResult = stepErr;
                }

                nextState = stateProcessOutput;
                break;
            }

            if (ifCommand == "deadtime") {
                QThread::msleep(static_cast<unsigned long>(currentSyncCommand->getTimeOut()));

                lastCommandResult = stepOk;
                nextState = stateProcessOutput;
                break;
            }

            if (ifCommand == "loraserverapi") {
                if (currentSyncCommand->getTxCommand().contains("create")) {
                    device->createDevice(
                        currentSyncCommand
                            ->getTxArguments()); //  Esto debería salir de aqui y enviarlo a otro proceso
                    device->setKeys(currentSyncCommand->getTxArguments(),
                                    "00000000000000000000000000000000",
                                    "abfae4d33b42258bcf742cc51178f1d4");
                    lastCommandResult = stepOk;
                }

                nextState = stateProcessOutput;
                break;
            }

            if (ifCommand == "end") {
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
                               .arg(currentSyncCommand->getInterfaceName()),
                           infoType::normal);
                currentSyncCommand->setState(JigSyncCommand::jump);
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
                                   .arg(currentSyncCommand->getInterfaceName()),
                               infoType::normal);
                    currentSyncCommand->setState(JigSyncCommand::jump);
                    nextState = stateReadCommand;
                    break;
                }

                outputInfo(QString("No esta abierta la interface %1")
                               .arg(currentSyncCommand->getInterfaceName()),
                           infoType::error);
                globalCommandJump = true;
                lastCommandResult = stepErr;
                nextState = stateProcessOutput;
                break;
            }

            if (!currentSyncCommand->getTxCommand().isEmpty()) {
                buildCommand = buildFrameToSend(currentSyncCommand);

                commandToSend = buildCommand;

                if (currentSyncCommand->getIsCrc16()) {
                }

                if (currentSyncCommand->getIsCr()) {
                    commandToSend.append('\r');
                }

                if (currentSyncCommand->getIsLf()) {
                    commandToSend.append('\n');
                }

                interface->write(commandToSend);

                if (!currentSyncCommand->getOnOk().isEmpty()
                    || !currentSyncCommand->getOnError().isEmpty()
                    || !currentSyncCommand->getRxCommand().isEmpty()) {
                    qDebug() << "TX:" << buildCommand << "; WAIT:" << currentSyncCommand->getOnOk();
                    outputInfo(QString("Tx: %1 [espero: %2]")
                                   .arg(QString::fromLatin1(buildCommand))
                                   .arg(currentSyncCommand->getOnOk()),
                               normal);
                    nextState = stateUsartRead;
                } else {
                    outputInfo(QString("Tx: %1").arg(QString::fromLatin1(buildCommand)), normal);
                    lastCommandResult = stepOk;
                    nextState = stateProcessOutput;
                }

            } else if (!currentSyncCommand->getRxCommand().isEmpty()) {
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
                               .arg(currentSyncCommand->getInterfaceName()),
                           infoType::error);
                globalCommandJump = true;
                lastCommandResult = stepErr;
                nextState = stateProcessOutput;
                break;
            }

            if (interface->dataReady()) {
                if (currentSyncCommand->isTxCommand()) {
                    if (!currentSyncCommand->getOnOk().isEmpty()
                        || !currentSyncCommand->getOnError().isEmpty()) {
                        if (interface->rxBuffer.contains(
                                QString("\r" + currentSyncCommand->getOnOk() + "\r\n").toLatin1())
                            || interface->rxBuffer.contains(
                                QString("\r\n" + currentSyncCommand->getOnOk() + "\r\n").toLatin1())) {
                            dataOk = true;
                            onOk = true;
                        }

                        if (interface->rxBuffer.contains(
                                QString("\r" + currentSyncCommand->getOnError() + "\r\n").toLatin1())
                            || interface->rxBuffer.contains(
                                QString("\r\n" + currentSyncCommand->getOnError() + "\r\n")
                                    .toLatin1())) {
                            dataOk = true;
                        }
                    } else if (!currentSyncCommand->getRxCommand().isEmpty()) {
                        if (interface->rxBuffer.contains(
                                QString(currentSyncCommand->getRxCommand()).toLatin1())) {
                            dataOk = true;
                            onOk = true;
                        }
                    }
                } else if (currentSyncCommand->isRxCommand()) {
                    if (interface->rxBuffer.contains(
                            QString(currentSyncCommand->getRxCommand()).toLatin1())) {
                        dataOk = true;
                        onOk = true;
                    }
                } else {
                    if (interface->rxBuffer.contains(
                            QString(currentSyncCommand->getOnOk() + "\r\n").toLatin1())) {
                        dataOk = true;
                        onOk = true;
                    }

                    if (interface->rxBuffer.contains(
                            QString(currentSyncCommand->getOnError() + "\r\n").toLatin1())) {
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

                if (currentSyncCommand->isRxCommand()) {
                    int index = 0;
                    answerIndex = -1;
                    foreach (QString str, dataResponseList) {
                        if (str.contains(currentSyncCommand->getRxCommand())) {
                            answerIndex = index;
                            break;
                        }
                        index++;
                    }
                } else {
                    if (onOk) {
                        answerIndex = dataResponseList.indexOf(currentSyncCommand->getOnOk());
                    } else {
                        answerIndex = dataResponseList.indexOf(currentSyncCommand->getOnError());
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

                if (currentSyncCommand->isTxCommand()) {
                    cmdEchoIndex = dataResponseList.indexOf(buildCommand);

                    if ((cmdEchoIndex >= 0) && (answerIndex > cmdEchoIndex)) {
                        cmdEcho = dataResponseList.at(cmdEchoIndex);
                        rxAnswers = dataResponseList.at(cmdEchoIndex + 1);
                    }
                }

                if (currentSyncCommand->isRxCommand()) {
                    dataResponseList = answer.split(' ');

                    cmdEcho = dataResponseList.takeAt(0);
                    rxAnswers = dataResponseList.join(' ');

                    if (currentSyncCommand->getRxCommand() == cmdEcho)
                        lastCommandResult = stepOk;
                }

                if (currentSyncCommand->isRxAnswers()) {
                    qDebug() << "valid answer:" << rxAnswers;

                    if (currentSyncCommand->getRxAnswers().contains('%')) { //      Variable comodín
                        wildcard[currentSyncCommand->getRxAnswers()] = rxAnswers.toLatin1();
                        wildcard["%2db"] = rxAnswers.toLatin1();

                        if (currentSyncCommand->getRxAnswers() == "%D") {
                            QDateTime t1 = QDateTime::fromString(rxAnswers, "dd/MM/yyyy HH:mm:ss");
                            QDateTime t2 = QDateTime::currentDateTime();

                            qDebug() << "localTime" << t1 << "; deviceTime" << t2
                                     << "diff:" << t2.secsTo(t1);

                            if (abs(t2.secsTo(t1)) < 10) {
                                lastCommandResult = stepOk;
                            } else {
                                lastCommandResult = stepErr;
                            }
                        } else if (currentSyncCommand->getRxAnswers() == "%measure") {
                            if (!currentSyncCommand->getMeasFormula().isEmpty()) {
                                formula = currentSyncCommand->getMeasFormula();

                                if (formula.contains("%")) {      //  Wildcard
                                    if (formula.contains("%f")) { // floating
                                        formula.replace("%f", rxAnswers);
                                    }
                                }

                                evaluator = jsEngine.evaluate(formula);
                                qDebug() << evaluator.toNumber();

                                if (currentSyncCommand->setMeasure(evaluator.toNumber())) {
                                    lastCommandResult = stepOk;
                                } else {
                                    lastCommandResult = stepErr;
                                }

                            } else if (currentSyncCommand->setMeasure(rxAnswers.toDouble())) {
                                lastCommandResult = stepOk;
                            } else {
                                lastCommandResult = stepErr;
                            }
                        } else if (currentSyncCommand->getRxAnswers() == "%serialNumber") {
                            if (QString::number(serialNumber) == rxAnswers) {
                                lastCommandResult = stepOk;
                            } else {
                                lastCommandResult = stepErr;
                            }
                        } else if (currentSyncCommand->getRxAnswers() == "%x") {
                            wildcard[currentSyncCommand->getRxAnswers()] = QByteArray::fromHex(
                                rxAnswers.toLatin1());
                        }
                    } else if (currentSyncCommand->getRxAnswers() == rxAnswers) {
                        lastCommandResult = stepOk;
                    }
                }

                if (currentSyncCommand->isOnOk() && (currentSyncCommand->getOnOk() == answer)) {
                    if (lastCommandResult != stepErr)
                        lastCommandResult = stepOk;
                }

                if (currentSyncCommand->isOnError()
                    && (currentSyncCommand->getOnError() == answer)) {
                    lastCommandResult = stepErr;

                    if (currentSyncCommand->isRxAnswers() && !rxAnswers.isEmpty()) {
                        if (currentSyncCommand->getRxAnswers().contains(
                                "%")) { //      Variable comodín

                            if (currentSyncCommand->getRxAnswers() == "%measure") {
                                brush = currentSyncCommand->measuredItem->foreground();
                                currentSyncCommand->measuredItem->setText(rxAnswers);
                                brush.setColor(Qt::red);
                                currentSyncCommand->measuredItem->setForeground(brush);
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
                if (!currentSyncCommand->messageOnOk.isEmpty())
                    outputInfo(currentSyncCommand->messageOnOk, ok);
                outputInfo("OK", infoType::ok);
                currentSyncCommand->setState(JigSyncCommand::ok);

                nextState = stateReadCommand;
                break;
            case stepErr: //  ERROR
                errors++;

                if (!currentSyncCommand->messageOnError.isEmpty())
                    outputInfo(currentSyncCommand->messageOnError, error);
                outputInfo("ERROR", infoType::error);
                currentSyncCommand->setState(JigSyncCommand::fail);

                if (currentSyncCommand->end)
                    globalCommandJump = true;

                reportFails.append(QString::number(currentCommandIndex) + ":"
                                   + currentSyncCommand->getId() + ";");
                reportComment.append(currentSyncCommand->messageOnError + ";");
                reportMeasure.append(wildcard["%2db"] + ";");

                nextState = stateReadCommand;
                break;
            case stepTimeout:
                outputInfo("Se ha excedido el tiempo limite de ejecución de comando", normal);

                if (timeoutCount >= 3) {
                    outputInfo("TIMEOUT", error);
                    ui->statusBar->showMessage("Error en la prueba");

                    currentSyncCommand->setState(JigSyncCommand::fail);

                    globalCommandJump = true;
                    errors++;

                    nextState = stateReadCommand;
                } else {
                    outputInfo("Reintento...", normal);
                    timerTimeOutDone = false;
                    timerTimeOut.stop();
                    timerTimeOut.start(currentSyncCommand->getTimeOut());

                    nextState = retryState;
                }
                break;
            }

            qDebug() << "";

            break;
        case stateFinish:
            tiempoOperacion = chrono.elapsed();
            ui->labelOutput->setText("ESPERE...");

            qDebug() << "Tiempo de la prueba" << tiempoOperacion;

            foreach (JigInterface *interface, jigsaw->interfaces) {
                if (interface) {
                    if (interface->getType() == JigInterface::tty) {
                        if (interface->isOpen()) {
                            interface->close();
                        }
                    }
                } else {
                    QMessageBox::critical(this,
                                          "Error",
                                          "Se ha detectado una interfaz que no ha sido configurada,"
                                          "revisar el perfil de pruebas.");
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

    if (saveDatabaseData && !debugMode) {
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

        foreach (JigSyncCommand *cmd, jigsaw->syncCommands) {
            cmd->clearMeasure();
            cmd->setState(JigSyncCommand::pending);
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
        ipeApp.setHexPath(jigsaw->getConfigHexPath());
        ui->lineEdit_hexPath->setText(jigsaw->getConfigHexPath());
    }
}

void MainWindow::on_lineEdit_PICKitBUR_returnPressed()
{
    ui->lineEdit_PICKitBUR->setReadOnly(true);
    jigsaw->setPickitSerialNumber(ui->lineEdit_PICKitBUR->text());
    ipeApp.setPickitSerialNumber(jigsaw->getPickitSerialNumber());
}

void MainWindow::outputInfo(QString info, infoType type)
{
    QString color;
    switch (type) {
    case normal:
        color = "white";
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
    if (jigsaw->syncCommands.size() == 0) {
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

    //  Refresh buttons
    asyncCommandListUi->setCommandList(&jigsaw->asyncCommands);
    asyncCommandListUi->refreshButtons();

    //  Datos extra ajenos al archivo JSON pero necesarios a partir de ahora
    QSqlDatabase db = QSqlDatabase::database("dbConexion");
    QSqlQuery query(db);
    query.prepare("SELECT idProducto FROM produccionIDi.Producto WHERE ProductoCodigo = :code");
    query.bindValue(":code", productCode);

    if (query.exec()) {
        qDebug() << query.executedQuery();
        while (query.next())
            idProducto = query.value("idProducto").toInt();
    } else {
        QMessageBox::warning(this, "Advertencia", "No fue posible conectarse con la base de datos.");
    }

    commandEditor->setInterfaces(&jigsaw->interfaces);
    commandEditor->setCommands(jigsaw->commandList);
    commandEditor->setTtyCommandsList(JigInterfaceTty::getDefaultCommands());
    commandEditor->setUsbCommandsList(JigInterfacePickit::getDefaultCommands());
    commandEditor->setPluginCommandsList(JigInterfacePlugin::getDefaultCommands());

    commandListSubwindow->show();
    commandListUi->show();

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

void MainWindow::on_actionEditInterface_triggered()
{
    interfaceEditor->setInterfaces(jigsaw->interfaces.values());
    interfaceEditor->show();
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

void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this, "Acerca de Qt");
}

void MainWindow::on_actionAbout_triggered()
{
    QString text = QString("Creado en Tarpuq Cia. Ltda. \nAutor: Galo Guzmán\nVersión %1")
                       .arg(APP_VERSION);

    QMessageBox::about(this, "Acerca de TarpuqJIGsw", text);
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

void MainWindow::on_actionDebug_triggered()
{
    if (!debugMode) {
        this->setEnabled(false);
        login->setEnabled(true);
        login->show();
    } else {
        debugMode = false;
        QMessageBox::information(this, "Aviso", "Modo Debug desactivado");
    }
}

void MainWindow::on_actionShowCommandList_triggered()
{
    commandListSubwindow->show();
    commandListUi->show();
}

void MainWindow::on_actionShowCommandOptional_triggered()
{
    asyncCommandListSubwindow->show();
    asyncCommandListUi->show();
}
