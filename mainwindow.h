#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
#include <cstdint>
#include <fstream>
#include <string>

#include <QMainWindow>
#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QNetworkInterface>
#include <QTcpSocket>
#include <QTcpServer>
#include <QScreen>
#include <QThread>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QRect>
#include <QTableView>
#include <QStandardItemModel>
#include <QHash>

#include "mouse.h"



struct mouseAction {
    QPoint pos;         // cursor position
    uint64_t action;    // list of actions supported are below
    uint64_t time;      // time when the action was taken
    uint64_t delay;     // time from the previous action

};

struct monitor {
    QString name;
    QRect geometry;
};


using namespace std;

class Mouse;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QString findExeDir(QString f);

public slots:


signals:

private slots:
    void on_updateButton_clicked();
    void on_tIntervalEdit_textChanged(const QString &arg1);
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void on_recordButton_clicked();

    void loadActions(string load);
    void saveActions(string save);
    void doActionList();
    void doAction(int index);
    void updateActionCounterLabel();
    void updateConnectionStatus(QAbstractSocket::SocketState s);
    void getMyMonitors();
    QJsonObject monitorDataToJSON();
    void jsonToMonitorData(QJsonDocument jDoc);
    void listMyMonitorsInUI();
    void drawMyMonitorsInUI();
    void listClientMonitorsInUI();
    void drawClientMonitorsInUI();
    void showNetworkInterfaces();
    void newConnection();
    void connectedToServer();
    void clearClientData();
    void processMouseMoved(QPoint p);


    // debug functions
    void showAllActions();

    void on_loadButton_clicked();
    void on_loadPathEdit_textChanged(const QString &arg1);
    void on_savePathEdit_textChanged(const QString &arg1);
    void on_saveButton_clicked();
    void on_repeatBox_toggled(bool checked);
    void on_repeatTimesEdit_textChanged(const QString &arg1);
    void on_listenButton_clicked();
    void on_connectButton_clicked();


    private:
    Ui::MainWindow *ui;

    QVector<mouseAction> actions;   // keep a list of actions recorded

    QTimer *timer;
    uint64_t timeZero;  // record starting time
    int index;
    bool recording;     // flags whether the app is currently recording or not
    int actionIndex;	// current action in the list being performed
    int repeatTotal;	// how many times the action list will be repeated
    int currRep;		// how many times the action list has been repeated so far

    // mouse hook stuff
    QThread mousePollThread;	// thread used to run the loop keeping track of the mouse position

    // directories
    QString exeDir;         // directory of this app executable
    QString tempFileDir;    // temporary save location to store actions being recorded
    QString fileLoaded;     // file containing actions currently loaded
    QString defFileToLoad;  // default file to load, latest loaded??

    // stuff for TCP socket
    QTcpServer *listenSocket;	// socket used to listen for connections from other clients
    QTcpSocket *serverSocket;	// socket used by the server to send data to the client
    QTcpSocket *clientSocket;	// socket used to connect to server
    quint16 port;			// port where socket will listen on

    // stuff for monitors
    QList<monitor> clientMonitors;	// monitor of the client connecting to the server instance
    QHash<QString, monitor> myMonitors;		// monitors on the current device
    QStandardItemModel *monitorModel;
    QTableView *monitorView;

};

#endif // MAINWINDOW_H



/* List of actions supported
 * 0 = undefined
 * 513 = Left mouse DOWN
 * 514 = Left mouse UP
 * 516 = Right mouse DOWN
 * 517 = Right mouse UP
 *
 */
