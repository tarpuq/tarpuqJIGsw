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

    connect(&stateMachineTimer, &QTimer::timeout, this, &MainWindow::processSyncCommand);
    stateMachineTimer.setInterval(100);
    stateMachineTimer.start();

    //  Serial Port
    checkPortTimer = new QTimer(this);
    serialBuffer = new QByteArray();

    //      Editors and Validators
    commandEditorDialog = new CommandEditDialog(this);
    interfaceEditorDialog = new InterfaceEditDialog(this);
    dutEditorDialog = new DutEditDialog(this);
    console = new Console(this);
    login = new LoginDialog(this);

    //      JIGsaw
    profile = new JigProfile(this);
    dutEditorDialog->setProfile(profile);
    connect(dutEditorDialog,&DutEditDialog::dutUpdated,this,&MainWindow::updateDutInfo);

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
        debugInfo("DB OK", normal);
    } else {
        debugInfo("DB ERR", error);
        QMessageBox::warning(this, "Advertencia", "No fue posible conectarse con la base de datos.");
    }

    //  PicKit IPE parameters
    ipeAppState = Pickit::Off;

    connect(&ipeApp, &Pickit::ipeOutput, [this](const QString &output) {
        this->outputInfo(output, normal);
    });
    connect(&ipeApp, &Pickit::ipeWarning, this, &MainWindow::ipeAppWarning);
    connect(&ipeApp, &Pickit::ipeMessage, this, &MainWindow::ipeAppMessage);

    ui->pushButtonStart->setEnabled(false);
    connect(ui->actionSalir, &QAction::triggered, this, &MainWindow::close);

    ui->progressBar->setValue(0);

    //  TimeOut Timer
    connect(&timerTimeOut, &QTimer::timeout, [this]() { this->timerTimeOutDone = true; });

    connect(login, &LoginDialog::isLogin, [this](bool ans) {
        this->setEnabled(true);
        if (ans) {
            debugMode = true;
            ui->statusBar->showMessage("Debug");
            QMessageBox::information(this, "Aviso", "Modo Debug activado");
        } else {
            debugMode = false;
            ui->statusBar->clearMessage();
            QMessageBox::information(this, "Aviso", "Contraseña incorrecta");
        }
    });
    connect(login, &LoginDialog::finished, [this]() { this->setEnabled(true); });
    connect(login, &LoginDialog::rejected, [this]() { this->setEnabled(true); });

    profileLoaded = false;

    //  SyncCommandList
    ui->syncCommandListUi->setCommandList(&profile->syncCommands);
    connect(ui->syncCommandListUi, &SyncCommandListForm::commandEdited, [this](JigSyncCommand *cmd) {
        commandEditorDialog->loadCommand(0, cmd);
        commandEditorDialog->show();
    });

    //  ASyncCommandList
    connect(ui->asyncCommandListUi,
            &AsyncCommandListForm::opcionalCommandExecuted,
            this,
            &MainWindow::processAsyncCommand);

    //  Dut Summary Form
    connect(ui->dutSummaryUi,
            &DutSummaryForm::dutsReady,
            this,
            &MainWindow::setStateReadyToStart);

    connect(ui->dutSummaryUi,
            &DutSummaryForm::dutChanged,
            this,
            &MainWindow::dutChanged);

    foreach(QMdiSubWindow *subwindow, ui->mdiArea->subWindowList()){
        subwindow->setAttribute(Qt::WA_DeleteOnClose, false);
        subwindow->close();
    }

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

    //  UI first state
    ui->actionSave->setEnabled(false);
    ui->actionEditInterface->setEnabled(false);
    ui->actionEditDut->setEnabled(false);

    debugMode = false;
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

bool MainWindow::pushDatabaseRecord(DutInfoForm *dutInfo, qint64 timeElapsed)
{
    QSqlDatabase db = QSqlDatabase::database("dbConexion");
    QSqlQuery query(db);

    bool ans = true;

    bool state = false;
    if(dutInfo->getStatus() == 1){
        state = true;
    }

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
    query.bindValue(":serialNumber", dutInfo->getSerialNumber());
    query.bindValue(":state", state);
    query.bindValue(":report", dutInfo->getFails().join(";"));
    query.bindValue(":notes", dutInfo->getComments().join(";"));
    query.bindValue(":measure", dutInfo->getMeasures().join(";"));
    query.bindValue(":tiempoOperacion", timeElapsed);

    if (!query.exec()) {
        ans = false;
        debugInfo(query.lastError().text(), normal);
    }

    return ans;
}

void MainWindow::processAsyncCommand(JigSyncCommand *asyncCmd)
{
    QString ifName;
    QString ifCmd;
    JigInterface *iface;

    ifName = asyncCmd->getInterfaceName();
    ifCmd = asyncCmd->getInterfaceCommand();
    iface = profile->interfaces[ifName];

    if (ifCmd == "open") {
        if (!iface->isOpen()) {
            int ans = iface->open();

            if (ans == 0) {
                debugInfo("OK", normal);
            } else if (ans == -1) {
                debugInfo("Already open", normal);
                return;
            } else if (ans == -2) {
                QString errorMsg
                    = QString("No se puede encontrar la interfaz %1, revise si está conectada.")
                          .arg(ifName);
                QMessageBox::warning(this, "Advertencia", errorMsg);
                return;
            }
        }
    } else if (ifCmd == "usartXfer") {
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

void MainWindow::updateDutInfo()
{
    ui->lineEditTarpuqCode->setText(profile->getProductCode());
    ui->lineEditProjectName->setText(profile->getProductName());

    //  Temp
    dutList = ui->dutSummaryUi->createDutMatrix(profile->getPanelRows(),profile->getPanelCols());
}

void MainWindow::setStateReadyToStart()
{
    if(ui->dutSummaryUi->isReady()){
        nextState = stateReadyToStart;

        QString series = ui->dutSummaryUi->getSerialNumbersList().join(' ');

        QString msg = QString("Detectado número(s) de serie %1\r").arg(series);
        outputInfo(msg, normal);

        foreach (JigSyncCommand *cmd, profile->syncCommands) {
            cmd->clearMeasure();
            cmd->setStatus(JigSyncCommand::pending);
        }
    }
}

void MainWindow::dutChanged(bool state)
{
    if(!state){
        ui->dockWidgetContents->clearOutputMessage();
        nextState = stateCheckProfile;
    }
}

void MainWindow::processSyncCommand()
{
    bool state = false;
    QString cmdSend;
    QString cmdEcho;
    QString answer;
    QString rxAnswers;
    QStringList rxAnswersList;
    QString res2;
    QString insert_db;
    QString textToLabel;
    QStringList dataResponseList;
    QString formula;
    QJSValue evaluator;
    qint32 wildcardInt;
    int stateMachineTimerInterval = 10;
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
            ui->statusBar->showMessage("Abra el perfil a utilizar");

            if (profileLoaded)
                nextState = stateIdle;

            break;
        case stateIdle:
            if (lastState == stateCheckProfile) {
                ui->statusBar->showMessage("Escanee o ingrese el número de serie");
            }
            break;
        case stateReadyToStart:
            if (lastState == stateIdle) {
                ui->pushButtonStart->setEnabled(true);
                ui->pushButtonStart->setFocus();

                ui->statusBar->showMessage("Pulse 'Iniciar' para empezar");
            }
            break;
        case stateStart:
            ui->pushButtonStart->setEnabled(false);
            ui->progressBar->setValue(0);
            ui->statusBar->showMessage("Iniciando Pruebas");

            ui->syncCommandListUi->cleanUI();
            ui->syncCommandListUi->scrollToTop();

            ui->dutSummaryUi->cleanUI();

            ui->dockWidgetContents->clearOutputMessage();

            reportFails.clear();
            reportComment.clear();
            reportMeasure.clear();
            chrono.restart();

            globalCommandJump = false;
            runingCommand = false;

            wildcard.clear();
            wildcard["%serialNumber"] = QByteArray::number(serialNumber);

            dutStatus.clear();

            for(int i = 0; i < profile->getPanelAmount(); i++){
                dutStatus << true;
            }

            errors = 0;
            currentCommandIndex = 0;
            nextState = stateReadCommand;
            break;
        case stateReadCommand:
            runingCommand = false;

            completed = static_cast<double>(currentCommandIndex)
                        / static_cast<double>(profile->syncCommands.size());
            ui->progressBar->setValue(static_cast<int>(completed * 100));

            if (currentCommandIndex >= profile->syncCommands.size()) {
                nextState = stateFinish;
                break;
            }

            currentSyncCommand = profile->syncCommands[currentCommandIndex];

            ui->syncCommandListUi->scrollToItem(currentSyncCommand->treeItem);

            ifCommand = currentSyncCommand->getInterfaceCommand();
            ifArguments = currentSyncCommand->getInterfaceArguments();
            ifAnswer = currentSyncCommand->getInterfaceAnswer();
            ifAlias = currentSyncCommand->getInterfaceName();
            interface = profile->interfaces[ifAlias];

            currentSyncCommand->refreshState();

            currentCommandIndex++;

            timeoutCount = 0;

            wildcard["%2db"] = "na";

            nextState = stateProcessCommand;

            //      Comprueba si hay demora
            if (currentSyncCommand->getEnabled() && (currentSyncCommand->getTimeDelay() > 0)) {
                currentSyncCommand->setStatus(JigSyncCommand::delay);
                nextState = stateDelayCommand;
            }

            //      Check if command has to be executed
            if (!currentSyncCommand->getEnabled()
                || (!currentSyncCommand->runOnJump && globalCommandJump)) {
                currentSyncCommand->setStatus(JigSyncCommand::jump);
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

            currentSyncCommand->setStatus(JigSyncCommand::running);
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

                debugInfo(evaluator.toString(), normal);

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
                debugInfo("IPE error", normal);
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
            if(interface->isOpen()){
                interface->close();
            }

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
                currentSyncCommand->setStatus(JigSyncCommand::jump);
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

            interface->reset();

            if (!interface->isOpen()) {
                if (globalCommandJump) {
                    outputInfo(QString("Salta el comando por que la interfaz %1 no está abierta.")
                                   .arg(currentSyncCommand->getInterfaceName()),
                               infoType::normal);
                    currentSyncCommand->setStatus(JigSyncCommand::jump);
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

                    debugInfo("TX:" + commandToSend + "; WAIT:" + currentSyncCommand->getOnOk(), normal);

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
                                QString("\r\n" + currentSyncCommand->getOnError() + "\r\n").toLatin1())) {
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
                    debugInfo("Errores" + QString::number(errors), error);
                    outputInfo("Se descartó la información recibida, posiblemente el tiempo de "
                               "espera fue muy corto.",
                               error);
                    globalCommandJump = true;
                    lastCommandResult = stepErr;
                    nextState = stateProcessOutput; //      Fall out error
                    break;
                }

                debugInfo("RX: " + dataResponseList.join(' '),normal);

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
                    rxAnswersList = rxAnswers.split(' ');
                    rxAnswersList.removeAll("");
                    rxAnswers = rxAnswersList.join(' ');

                    debugInfo("Valid answer(s): " + rxAnswers, normal);

                    if (currentSyncCommand->getRxAnswers().contains('%')) { //      Variable comodín
                        wildcard[currentSyncCommand->getRxAnswers()] = rxAnswers.toLatin1();
                        wildcard["%2db"] = rxAnswers.toLatin1();
                    }

                    //  Temp
                    //  Check if number of PCBs is equal to Measures amount
                    if (rxAnswersList.size() <= profile->getPanelAmount()){
                        QStringList rep;
                        QList<bool> errs;

                        if(!currentSyncCommand->processAnswers(rxAnswersList, &wildcard, currentCommandIndex,&errs, &rep)){
                            lastCommandResult = stepOk;
                        } else {
                            lastCommandResult = stepErr;
                        }

                        for (int i = 0; i < errs.size(); i++) {
                            if(errs.at(i)){
                                dutList->at(i)->setError(true);
                                dutList->at(i)->putRegister(rep.at(0),rep.at(1),rep.at(2));
                            }
                        }
                    } else {
                        lastCommandResult = stepErr;
                    }
                }

                if (currentSyncCommand->isOnOk() && (currentSyncCommand->getOnOk() == answer)) {
                    if (lastCommandResult != stepErr)
                        lastCommandResult = stepOk;
                }

                if (currentSyncCommand->isOnError() && (currentSyncCommand->getOnError() == answer)) {
                    lastCommandResult = stepErr;

                    if (currentSyncCommand->isRxAnswers() && !rxAnswers.isEmpty()) {
                        if (currentSyncCommand->getRxAnswers().contains("%")) { //      Variable comodín
                            if (currentSyncCommand->getRxAnswers() == "%measure") {
                                currentSyncCommand->setMeasureError(rxAnswers);
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
                currentSyncCommand->setStatus(JigSyncCommand::ok);

                nextState = stateReadCommand;
                break;
            case stepErr: //  ERROR
                errors++;

                if (!currentSyncCommand->messageOnError.isEmpty())
                    outputInfo(currentSyncCommand->messageOnError, error);
                outputInfo("ERROR", infoType::error);
                currentSyncCommand->setStatus(JigSyncCommand::fail);

                if (currentSyncCommand->end)
                    globalCommandJump = true;

                reportFails << QString::number(currentCommandIndex) + ":" + currentSyncCommand->getId();
                reportComment << currentSyncCommand->messageOnError;
                reportMeasure << wildcard["%2db"];

                nextState = stateReadCommand;
                break;
            case stepTimeout:
                outputInfo("Se ha excedido el tiempo limite de ejecución de comando", normal);

                if (timeoutCount >= 3) {
                    outputInfo("TIMEOUT", error);
                    ui->statusBar->showMessage("Error en la prueba");

                    currentSyncCommand->setStatus(JigSyncCommand::fail);

                    globalCommandJump = true;
                    errors++;

                    for(int i = 0; i < profile->getPanelAmount(); i++){
                        dutList->at(i)->setError(true);
                    }

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
            ui->statusBar->showMessage("ESPERE...");

            debugInfo("Tiempo de la prueba" + QString::number(tiempoOperacion), normal);

            foreach (JigInterface *interface, profile->interfaces) {
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

            for(int i = 0; i < profile->getPanelAmount(); i++){
                if(!dutList->at(i)->getError()){
                    ui->dutSummaryUi->setDutStatus(i, 1);
                    dutList->at(i)->putRegister("ok","ok","ok");
                } else{
                    ui->dutSummaryUi->setDutStatus(i, 2);
                    dutList->at(i)->putRegister("fail","fail","fail");
                }
            }

            if (errors == 0) {
                state = true;
                ui->statusBar->showMessage("PASA",10000);
                reportFails << "ok";
                reportComment << "ok";
                reportMeasure << "ok";
            } else {
                state = false;
                ui->statusBar->showMessage("NO PASA",10000);

                if (reportFails.isEmpty())
                    reportFails << "fail";
                if (reportComment.isEmpty())
                    reportComment << "fail";
                if (reportMeasure.isEmpty())
                    reportMeasure << "fail";
            }

            saveDatabaseData = true; //  Guarda los datos en la base de datos

            ui->progressBar->setValue(100);
            ui->pushButtonStart->setEnabled(true);

            globalCommandJump = false;

            nextState = stateIdle;
            break;
        case stateError:
            ui->statusBar->showMessage("Desconecte el cable USB y reinicie");
            break;
        }

        //      Update last state
        lastState = currentState;
        if (nextState != stateInitApp)
            currentState = nextState;
    } while (globalCommandJump && !runingCommand);

    if (saveDatabaseData && !debugMode) {
        bool state = true;
        foreach(DutInfoForm *dutInfo, *dutList){
            state &= pushDatabaseRecord(dutInfo, tiempoOperacion);
        }

        if (!state){
            QMessageBox::warning(this,
                                 "Advertencia",
                                 "No fue posible registrar la información en la base de datos, por "
                                 "favor reinicie la aplicación y repita la prueba.");
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
        ui->statusBar->showMessage("Dispositivo detectado");
    }

    if (state == Pickit::TargetRemoved) {
        ui->statusBar->showMessage("Inserte una placa");
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

void MainWindow::on_pbHex_clicked()
{
    QString path;
    path = QFileDialog::getOpenFileName(this,
                                        "Ruta del HEX",
                                        QDir::homePath(),
                                        "Archivos HEX (*.hex)");
    if (!path.isEmpty()) {
        profile->setFwPath(path);
        ipeApp.setHexPath(profile->getFwPath());
    }
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

    QString text = QString("<font color =\"%1\">%2</font><br>").arg(color).arg(info);

    ui->dockWidgetContents->setOutputMessage(text);
}

void MainWindow::debugInfo(QString info, MainWindow::infoType type)
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

    QString text = QString("<font color =\"%1\">%2</font><br>").arg(color).arg(info);

    ui->dockWidgetContents->setDebugMessage(text);
}

void MainWindow::on_pushButtonStart_clicked()
{
    if (profile->availableInterfaces.contains("pickit")){
        JigInterfacePickit *pickit = static_cast<JigInterfacePickit *>(profile->interfaces["pickit"]);

        if (pickit->getIpePath().isEmpty()) {
            QMessageBox::warning(this, "Alerta", "Ruta del IPE no especificada");
            return;
        }

        if (pickit->getBur().isEmpty()) {
            QMessageBox::warning(this, "Alerta", "Numero de serie del programador no especificado");
            return;
        }
        if (!QFile(pickit->getIpePath()).exists()) {
            QMessageBox::warning(this, "Alerta", "La ruta de MPLAB X IPE es incorrecta");
            return;
        }
    }

    if (profile->getFwPath().isEmpty()) {
        QMessageBox::warning(this, "Alerta", "Ruta del HEX no especificada");
        return;
    }
    if (!ui->dutSummaryUi->isReady()) {
        QMessageBox::warning(this, "Alerta", "Uno o más de los números de serie está incompleto");
        return;
    }
    if (profile->syncCommands.size() == 0) {
        QMessageBox::warning(this, "Alerta", "No se ha cargado un perfil de pruebas");
        return;
    }

    outputInfo(QString("Iniciando pruebas del PCBA sn/%1").arg(serialNumber), normal);
    debugInfo("Iniciando test", normal);
    nextState = stateStart;

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
    QString homePath = QDir::homePath();

    path = QFileDialog::getOpenFileName(this,
                                        "Ruta del perfil",
                                        homePath,
                                        "Archivos json (*.json)");

    ui->dockWidgetContents->clearOutputMessage();

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

    res = profile->readProfile(data);

    if (res < 0) {
        //      Error
        QMessageBox::critical(this, "Error", "El archivo de configuración tiene errores.");
        return;
    }

    ui->actionSave->setEnabled(true);
    ui->actionEditInterface->setEnabled(true);
    ui->actionEditDut->setEnabled(true);

    if(profile->availableInterfaces.contains("pickit")){
        JigInterfacePickit *pickit = static_cast<JigInterfacePickit *>(profile->interfaces["pickit"]);

        ipeApp.setTargetName(pickit->getTarget());
        ipeApp.setPickitSerialNumber(pickit->getBur());
        ipeApp.setIpePath(pickit->getIpePath());
        ipeApp.setHexPath(profile->getFwPath());

        ipeApp.setMclrPinName(pickit->getMclrPinName());
    }

    QString productName = profile->getProductName();
    QString productCode = profile->getProductCode();

    //      Write
    if (!productName.isEmpty() && !productCode.isEmpty()) {
        ui->lbProfile->setText("Perfil actual:");
    }

    dutEditorDialog->refreshUI();
    ui->syncCommandListUi->refreshUI();

    //  Refresh buttons
    ui->asyncCommandListUi->setCommandList(&profile->asyncCommands);
    ui->asyncCommandListUi->refreshButtons();

    //  Datos extra ajenos al archivo JSON pero necesarios a partir de ahora
    QSqlDatabase db = QSqlDatabase::database("dbConexion");
    QSqlQuery query(db);
    query.prepare("SELECT idProducto FROM produccionIDi.Producto WHERE ProductoCodigo = :code");
    query.bindValue(":code", productCode);

    if (query.exec()) {
        debugInfo(query.executedQuery(), normal);
        while (query.next())
            idProducto = query.value("idProducto").toInt();
    } else {
        QMessageBox::warning(this, "Advertencia", "No fue posible conectarse con la base de datos.");
    }

    commandEditorDialog->setInterfaces(&profile->interfaces);

    ui->syncCommandListUi->show();
    ui->asyncCommandListUi->show();
    ui->dutSummaryUi->show();

    profilePath = path;
    profileLoaded = true;

    //  Create DutInfo
    updateDutInfo();
}

void MainWindow::on_actionSave_triggered()
{
    QByteArray data;
    int res;

    res = profile->writeProfile(&data);

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
    interfaceEditorDialog->setInterfaces(profile->interfaces.values());
    interfaceEditorDialog->show();
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
    QString msg;
    QTextStream stream(&msg);
    stream.setCodec("UTF-8");

    stream << "tarpuqJIGsw v" << APP_VERSION << "\n";
    stream << "Copyright " << (char)Qt::Key_copyright << " 2018-2020, Tarpuq Cia. Ltda. \n\n";
    stream << QString("Autor: Galo Guzmán G.\n");

    QMessageBox::about(this, "Acerca de TarpuqJIGsw", msg);
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
    ui->syncCommandListUi->show();
    ui->syncCommandListUi->setFocus();
}

void MainWindow::on_actionShowCommandOptional_triggered()
{
    ui->asyncCommandListUi->show();
    ui->asyncCommandListUi->setFocus();
}

void MainWindow::on_actionShowDutInformation_triggered()
{
    ui->dutSummaryUi->show();
    ui->dutSummaryUi->setFocus();
}

void MainWindow::on_actionConsole_triggered()
{
    console->show();
}

void MainWindow::on_actionNew_triggered()
{

}

void MainWindow::on_actionEditDut_triggered()
{
    dutEditorDialog->show();
}

void MainWindow::on_actionShowMessages_triggered()
{
    ui->dockWidget->show();
}
