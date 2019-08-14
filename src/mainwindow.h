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

#include "jigcommand.h"
#include "jiginterface.h"
#include "jigsaw.h"
#include "loraserverapi.h"
#include "pickit.h"

#include "jigcommandedit.h"
#include "jiginterfaceedit.h"

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

    bool configUpdate;

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

protected slots:
    void refreshTable();

    void moveCommand(int from, int to);

private slots:
    void mainStateMachine();

    void ipeAppMessage();

    void ipeAppWarning();

    void on_lineEdit_numeroSerie_returnPressed();

    void on_pbIpe_clicked();

    void on_pbPicKit_clicked();

    void on_pbHex_clicked();

    void on_lineEdit_PICKitBUR_returnPressed();

    void outputInfo(QString info, infoType type);

    void on_pushButtonStart_clicked();

    void on_pushButtonStop_clicked();

    void on_actionLoad_triggered();

    void on_actionSave_triggered();

    void on_actionInsert_triggered();

    void on_actionEdit_triggered();

    void on_actionRemove_triggered();

    void on_tableWidget_itemChanged(QTableWidgetItem *item);

    void on_actionEditInterface_triggered();

    void on_actionMove_Up_triggered();

    void on_actionMove_Down_triggered();

    void on_actionDuplicate_triggered();

    void on_actionSaveAs_triggered();

    void on_actionMove_First_triggered();

    void on_actionMove_Last_triggered();

    void on_checkBox_stateChanged(int arg1);

    void on_actionAboutQt_triggered();

    void on_actionAbout_triggered();

    void on_actionEditOptionalCommand_triggered();

    void on_tableWidget_customContextMenuRequested(const QPoint &pos);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_lineEditTarpuqCode_editingFinished();

    void on_lineEditProjectName_editingFinished();

private:
    Ui::MainWindow *ui;
    JigCommandEdit *commandEditor;
    JigInterfaceEdit *interfaceEditor;

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
    QString eui;
    QString reportFails;
    QString reportComment;
    QString reportMeasure;

    int currentCommandIndex;

    bool globalCommandJump;
    bool runingCommand;

    QByteArray buildCommand;
    QByteArray commandToSend;

    QTimer timerTimeOut;
    bool timerTimeOutDone;

    QHash<QString, QByteArray> wildcard;

    JigCommand *currentCommand;

    testResult lastCommandResult;

    QString ifCommand;
    QString ifArguments;
    QString ifAnswer;
    QString ifAlias;
    JigInterface *interface;

    JigSaw *jigsaw;

    QString profilePath;

    int tempDutPid;
    int tempDutVid;
    bool tempDutIsPidVid;

    int tableRowReference;

    QElapsedTimer chrono;

    bool profileLoaded;
};

#endif // MAINWINDOW_H
