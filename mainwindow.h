#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "windows.h"
#include "Strsafe.h"

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

#include "mouse.h"

#define WM_MOUSEMOVE 0x0200     // to detect mouse support

typedef struct _HOOKSTRUCT
{
    int nType;
    HOOKPROC hkprc;
    HHOOK hhook;
    bool bInstalled;
} HOOKSTRUCT;


struct mouseAction {
    QPoint pos;         // cursor position
    uint64_t action;    // list of actions supported are below
    uint64_t time;      // time when the action was taken
    uint64_t delay;     // time from the previous action

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
    void doAction();

    // debug functions
    void showAllActions();



    void on_loadButton_clicked();
    void on_loadPathEdit_textChanged(const QString &arg1);
    void on_savePathEdit_textChanged(const QString &arg1);


    void on_saveButton_clicked();

private:
    Ui::MainWindow *ui;

    QVector<mouseAction> actions;   // keep a list of actions recorded

    QTimer *timer;
    uint64_t timeZero;  // record starting time
    int actionIndex;
    bool recording;     // flags whether the app is currently recording or not

    // mouse hook stuff
    HHOOK MouseHook;
    HOOKPROC hkprcSysMsg;

    // directories
    QString exeDir;         // directory of this app executable
    QString tempFileDir;    // temporary save location to store actions being recorded
    QString fileLoaded;     // file containing actions currently loaded
    QString defFileToLoad;  // default file to load, latest loaded??



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
