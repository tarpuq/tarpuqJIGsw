#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QFile>
#include <QProcess>
#include <QTimer>
#include <QSqlDatabase>
#include <QHash>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTableWidgetItem>
#include <QJSEngine>
#include <QMdiSubWindow>

#include "jigsynccommand.h"
#include "jiginterface.h"
#include "jigprofile.h"
#include "loraserverapi.h"
#include "pickit.h"
#include "commandeditdialog.h"
#include "jiginterfaceeditdialog.h"
#include "jiglogindialog.h"
#include "synccommandlistform.h"
#include "asynccommandlistform.h"
#include "dutsummaryform.h"
#include "console.h"
#include "duteditdialog.h"
#include "dutinfoform.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    enum states {
        stateInitApp,
        stateCheckProfile,
        stateIdle,
        stateReadyToStart,
        stateStart,
        stateReadCommand,
        stateDelayCommand,
        stateProcessCommand,
        stateApp,
        statePlugin,
        statePluginWait,
        stateIpeProgram,
        stateIpeWait,
        stateInterfaceOpen,
        stateInterfaceClose,
        stateUsartWrite,
        stateUsartRead, /****/
        stateProcessOutput,
        stateFinish,
        stateError
    };

    enum testResult { stepNone, stepOk, stepErr, stepTimeout };

    enum infoType { normal, ok, error };

    typedef struct
    {
        QString name;
        double min;
        double max;
        double measured;
        QString status;
    } testStruct;

    //  GPIO
    void writeToGPIO(quint8 number, bool state);
    int readFromGPIO(quint8 number, bool *state);

    //  A/D
    quint16 readFromADC(quint8 channel);

    //  USART
    void writeToUsart(QByteArray data);
    void readFromUsart(QByteArray *data);

    float voltage1H;
    float voltage1L;
    float voltage2;
    float voltage3;

    quint64 serialNumber;

    states lastState;
    states retryState;
    states nextState;
    states currentState;
    quint8 errors;
    quint8 timeoutCount;
    quint8 retries;

    Pickit::ipeStates ipeAppState;

    void readAppConfigFile(QByteArray data);

    QByteArray buildFrameToSend(JigSyncCommand *command);

    bool pushDatabaseRecord(DutInfoForm *dutInfo, qint64 timeElapsed);

public slots:
    void processAsyncCommand(JigSyncCommand *asyncCmd);

    void updateDutInfo();

    void setStateReadyToStart();

    void dutChanged(bool state);

private slots:
    void processSyncCommand();

    void ipeAppMessage();

    void ipeAppWarning();

    void on_pbHex_clicked();

    void outputInfo(QString info, infoType type);

    void debugInfo(QString info, infoType type);

    void on_pushButtonStart_clicked();

    void on_pushButtonStop_clicked();

    void on_actionLoad_triggered();

    void on_actionSave_triggered();

    void on_actionEditInterface_triggered();

    void on_actionSaveAs_triggered();

    void on_actionAboutQt_triggered();

    void on_actionAbout_triggered();

    void on_actionDebug_triggered();

    void on_actionShowCommandList_triggered();

    void on_actionShowCommandOptional_triggered();

    void on_actionConsole_triggered();

    void on_actionNew_triggered();

    void on_actionEditDut_triggered();

    void on_actionShowDutInformation_triggered();

    void on_actionShowMessages_triggered();

private:
    Ui::MainWindow *ui;
    CommandEditDialog *commandEditorDialog;
    InterfaceEditDialog *interfaceEditorDialog;
    DutEditDialog *dutEditorDialog;
    Console *console;
    LoginDialog *login;

    QTimer stateMachineTimer;
    QFile script;
    QJSEngine jsEngine;

    QString dbHostName;
    QString dbDatabaseName;
    QString dbUserName;
    QString dbPassword;
    QString apiHostName;
    QString apiUserName;
    QString apiPassword;
    quint16 apiPort = 0;

    Pickit ipeApp;

    int idProducto;

    //  Script
    QString args;
    QStringList list;

    QStringList scriptCommands;

    QTimer testInterface_t;

    int scriptCommandsSize;

    QTimer *checkPortTimer;
    QByteArray *serialBuffer;
    QString comPortName;

    uint8_t connectState;

    LoraServerApi *device;
    QStringList reportFails;
    QStringList reportComment;
    QStringList reportMeasure;

    int currentCommandIndex;

    bool globalCommandJump;
    bool runingCommand;

    QByteArray buildCommand;
    QByteArray commandToSend;

    QTimer timerTimeOut;
    bool timerTimeOutDone;

    QHash<QString, QByteArray> wildcard;

    testResult lastCommandResult;

    QString ifCommand;
    QString ifArguments;
    QString ifAnswer;
    QString ifAlias;
    JigInterface *interface;

    JigProfile *profile;
    JigSyncCommand *currentSyncCommand;

    QString profilePath;

    int tempDutPid;
    int tempDutVid;
    bool tempDutIsPidVid;

    int tableRowReference;

    QElapsedTimer chrono;

    bool profileLoaded;
    bool debugMode;

    QList<DutInfoForm *> *dutList;
    QList<bool> dutStatus;
};

#endif // MAINWINDOW_H
