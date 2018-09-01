#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    exeDir = findExeDir(QCoreApplication::arguments().at(0));
    tempFileDir = exeDir + QString("\\tmpfile.txt");
//	defFileToLoad = "C:\\Users\\Davide\\Desktop\\MouseActions\\CoalDung.txt";
    defFileToLoad = tempFileDir;

    recording = false;
    timeZero = 0;
    actionIndex = 0;
    repeatTotal = 1;
    currRep = 0;

    // initialise timer
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(doActionList()));


    // initialise stuff in UI
    this->setWindowTitle("MoveMouse");
    this->setFixedSize(this->size());
    ui->stopButton->setDisabled(true);
    ui->startButton->setDisabled(false);
    ui->currActEdit->setEnabled(false);
    ui->totalActEdit->setEnabled(false);
    ui->console->setEnabled(false);
    ui->saveButton->setEnabled(false);
    ui->loadButton->setEnabled(false);
    ui->repeatTimesEdit->setEnabled(false);

    ui->repeatTimesEdit->setValidator(new QIntValidator(0, 9999999, this));


    // initialise UI
    // put text in the various edits, labels and buttons
    ui->console->setText("No file loaded");
    ui->repeatTimesEdit->setText(QString::number(repeatTotal));
    ui->loadPathEdit->setText(defFileToLoad);

    // initialise the UI view for the monitors, put in the monitors of the current device
    getMyMonitors();	// make sure this runs before anything requiring the myMonitors list
    listMyMonitorsInUI();
    drawMyMonitorsInUI();		// TODO: implement this function




    // initialise data for mouse cursor
//	Mouse::tmpActFile = tempFileDir;
//	loadActions(tempFileDir.toStdString());


    // initialise stuff for network operations
    showNetworkInterfaces();
    port = 60000;
    listenSocket = new QTcpServer(this);
    connect(listenSocket, SIGNAL(newConnection()), this, SLOT(newConnection()));


    // debug    ===============================================================
    //    qDebug() << "argc: " << QCoreApplication::arguments().size();
    //    for (int i = 0; i < QCoreApplication::arguments().size(); i++) {
    //        qDebug() << i << "\targv: " << QCoreApplication::arguments().at(i);
    //    }

    qDebug() << "exeDir: " << exeDir;
    qDebug() << "tempFileDir; " << tempFileDir;


    // initialise mouse things
    Mouse *mouseObj = new Mouse();
    connect(mouseObj, SIGNAL(mouseMoved(QPoint)), this, SLOT(processMouseMoved(QPoint)));

    Mouse::startMouseTracking(mouseObj);


//    monitorDataToJSON();
//    jsonToMonitorData(QJsonDocument(monitorDataToJSON()));


}

MainWindow::~MainWindow() {
    mousePollThread.quit();
    mousePollThread.terminate();    // that is ok, the application is being terminated
    timer->stop();
    delete timer;
    delete ui;

}

QString MainWindow::findExeDir(QString f) {
    int t;
    t = (int)f.toStdString().find_last_of('\\');
    return QString(f.toStdString().substr(0, t).c_str());

}

void MainWindow::on_updateButton_clicked() {
    // update all the variables from UI to memory
    ui->updateButton->setDisabled(true);

}

void MainWindow::on_tIntervalEdit_textChanged(const QString &arg1) {
    ui->updateButton->setDisabled(false);

}

void MainWindow::on_startButton_clicked() {
    // start going through the list of actions
    ui->stopButton->setDisabled(false);
    ui->startButton->setDisabled(true);
    ui->recordButton->setDisabled(true);
    ui->repeatTimesEdit->setDisabled(true);
    currRep = 0;
    repeatTotal = ui->repeatTimesEdit->text().toInt();
    actionIndex = 0;

    updateActionCounterLabel();
    timer->start(1000);

}

void MainWindow::on_stopButton_clicked() {
    // stop the timer
    ui->stopButton->setDisabled(true);
    ui->startButton->setDisabled(false);
    ui->recordButton->setEnabled(true);
    ui->repeatTimesEdit->setEnabled(true);

    timer->stop();

}

void MainWindow::on_recordButton_clicked() {
    // start or stop recording mouse events
    if (recording) {
        // stop recording
        recording = false;
//        UnhookWindowsHookEx(MouseHook);
        ui->recordButton->setText("Record");
        ui->startButton->setEnabled(true);

        // load actions from tempfile
        loadActions(tempFileDir.toStdString());
//        showAllActions();


    } else {
        // start recording
        recording = true;
        ui->startButton->setEnabled(false);

        // rewrite the temp file
        ofstream myfile;
        //        myfile.open(MainWindow::tempFileDir.c_str());
        myfile.open(tempFileDir.toStdString().c_str());

        if (myfile.is_open()) {
            qDebug() << "Re-opened temp file";

            timeZero = QDateTime::currentMSecsSinceEpoch();
//            MouseHook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)Mouse::MouseHookProc, GetModuleHandle(NULL), 0);
            ui->recordButton->setText("Done");

            qDebug() << "timeZero: " << timeZero;

            // delete all actions previously recorded
            actions.clear();

        } else {
            qDebug() << "Could not open temp file";
            return;
        }
    }

}

void MainWindow::loadActions(string load) {
    // load actions from tempfile, overwrite last set of actions
    actions.clear();

    ifstream read(load.c_str());

    if (read.is_open()) {
        string line;

        while(getline(read, line)) {
            int pos1 = line.find('\t', 0);
            int pos2 = line.find('\t', pos1+1);
            int pos3 = line.find('\t', pos2+1);
            mouseAction temp;
            //            qDebug() << pos1 << "\t" << pos2 << "\t" << pos3;

            temp.time = (uint64_t)strtoull(line.substr(0, pos1).c_str(), NULL, 10);
            temp.action = (uint64_t)strtoull(line.substr(pos1+1, pos2-pos1-1).c_str(), NULL, 10);
            temp.pos = QPoint((int)strtoul(line.substr(pos2+1, pos3-pos2-1).c_str(), NULL, 10),
                              (int)strtoul(line.substr(pos3+1, line.length()-pos3-1).c_str(), NULL, 10));


            // calculate delay
            if (actions.size() > 0) {
                temp.delay = temp.time - actions.at(actions.size()-1).time;
            } else {
                temp.delay = 0;
            }

            actions.append(temp);

        }


        // file is loaded here
        ui->currActEdit->setText("0");
        ui->totalActEdit->setText(QString::number(actions.size()));
        ui->console->setText("File loaded: ");

        if (tempFileDir.toStdString() == load) {
            // loading the temp file
            ui->console->append(QString("tmpfile"));
        } else {
            ui->console->append(QString(load.c_str()));
        }

        fileLoaded = QString(load.c_str());


    } else {
        qDebug() << "Could not open tempFile to read actions";
    }

}

void MainWindow::saveActions(string save) {
    // save actions to file
    qDebug() << "Saving actions to file";

    ofstream write(save.c_str());

    if (write.is_open()) {

        for (int i = 0; i < actions.size(); i++) {
            write << actions.at(i).time << "\t" << actions.at(i).action
                  << "\t" << actions.at(i).pos.x() << "\t" << actions.at(i).pos.y();
            write << "\n";

        }

        write.close();

        // load actions from saved file
        loadActions(save);


    } else {
        qDebug() << "Could not open save file";
    }

}

void MainWindow::doActionList() {
    // go through the action list and execute all of them
    // the last two actions are used to stop recording, avoid them
    // this function is called by the timer


//	while (repeatTotal > currRep) {
//		// prepare to repeat the action list
//		qDebug() << "repeatTotal: " << repeatTotal << "\tcurrRep" << currRep;
//		actionIndex = 0;

//		// do action list
//		for (int i = 0; i < actions.size(); i++) {
//			qDebug() << "i: " << i << "\tactions.size()" << actions.size();

////			qApp->processEvents();

//			if (actionIndex < actions.size()-2) {
//				// keep going through the list
//				// wait a second to do the first action

////				Sleep(actions.at(actionIndex).delay);

//				timer->start((int)actions.at(actionIndex).delay);

//				ui->currActEdit->setText(QString::number(actionIndex));
////				doAction(actionIndex);
//				actionIndex++;

//			} else {
//				break;
//			}
//		}

//		currRep++;
//		updateActionCounterLabel();		// update counter label

//	}



    if (actionIndex < actions.size()-2) {
        // keep going through the list
        qDebug() << "actionIndex: " << actionIndex
                 << "\tactions.size()" << actions.size()
                 << "\tdelay: " << (int)actions.at(actionIndex).delay;
        timer->start((int)actions.at(actionIndex+1).delay);

        ui->currActEdit->setText(QString::number(actionIndex));
        doAction(actionIndex);
        actionIndex++;

    } else {
        // finished list
        currRep++;
        updateActionCounterLabel();		// update counter label

        if (repeatTotal > currRep) {
            // repeat the list
            actionIndex = 0;
            timer->start(1000);
        } else {
            on_stopButton_clicked();
        }

    }


}

void MainWindow::doAction(int index) {
    // perform the action at actionindex
//    qDebug() << "Doing action " << index;

//    SetCursorPos(actions.at(index).pos.x(), actions.at(index).pos.y());

    switch (actions.at(index).action) {
        case (513) : {
            // Left mouse DOWN
            qDebug() << "\tLeft mouse DOWN ... index: " << index << "\t" << actions.at(index).pos;
//            mouse_event(MOUSEEVENTF_LEFTDOWN, actions.at(index).pos.x(), actions.at(index).pos.y(), 0, 0);
            break;
        }
        case (514) : {
            // Left mouse UP
            qDebug() << "\tLeft mouse UP ... index: " << index << "\t" << actions.at(index).pos;
//            mouse_event(MOUSEEVENTF_LEFTUP, actions.at(index).pos.x(), actions.at(index).pos.y(), 0, 0);
            break;
        }
        case (516) : {
            // Right mouse DOWN
            qDebug() << "\tRight mouse DOWN ... index: " << index << "\t" << actions.at(index).pos;
//            mouse_event(MOUSEEVENTF_RIGHTDOWN, actions.at(index).pos.x(), actions.at(index).pos.y(), 0, 0);
            break;
        }
        case (517) : {
            // Right mouse UP
            qDebug() << "\tRight mouse UP ... index: " << index << "\t" << actions.at(index).pos;
//            mouse_event(MOUSEEVENTF_RIGHTUP, actions.at(index).pos.x(), actions.at(index).pos.y(), 0, 0);
            break;
        }

    }


}

void MainWindow::updateActionCounterLabel() {
    // build the string in this format <rep> / <totalRep>
    ui->actionCounterLabel->setText(QString(QString::number(currRep) + " / " + QString::number(repeatTotal)));

}

void MainWindow::updateConnectionStatus(QAbstractSocket::SocketState s) {
    // status of clientSocket has changed, update the UI
    switch (s) {
        case QAbstractSocket::UnconnectedState:
            ui->connStatusLabel->setText("Not Connected");
        break;

        case QAbstractSocket::HostLookupState:
            ui->connStatusLabel->setText("Looking up hostname...");
        break;

        case QAbstractSocket::ConnectingState:
            ui->connStatusLabel->setText("Connecting...");
        break;

        case QAbstractSocket::ConnectedState:
            ui->connStatusLabel->setText("Connected to " + clientSocket->peerAddress().toString());
        break;

        case QAbstractSocket::BoundState:
            ui->connStatusLabel->setText("Socket bound");
        break;

        case QAbstractSocket::ClosingState:
            ui->connStatusLabel->setText("Closing connection...");
        break;


        default:
            ui->connStatusLabel->setText("Unknown connection state");
    }

}

void MainWindow::getMyMonitors() {
    // get list of monitors and store it in myMonitors
    // using a QHash here implicitly discards any duplicates
    for (int i = 0; i < QApplication::screens().size(); i++) {
        monitor tmp;
        tmp.name = QApplication::screens().at(i)->name();
        tmp.geometry = QApplication::screens().at(i)->geometry();

        myMonitors[tmp.name] = tmp;
    }
}

QJsonObject MainWindow::monitorDataToJSON() {
    // function uses QApplication::screens() to get the monitor configuration of the device
    // and packs it in a QByteArray
    QJsonObject data;

    getMyMonitors();	// run this just in case

    data["monitorNum"] = QString::number(myMonitors.size());	// number of monitors

    // put data for each monitor
    QJsonArray jArray;
    for (int i = 0; i < myMonitors.keys().size(); i++) {
        QJsonObject jMonitorObject;
        QString currKey = myMonitors.keys().at(i);

//		jMonitorObject.insert("x", QString::number(myMonitors.at(i).geometry.x()).toStdString().c_str());
//		jMonitorObject.insert("y", QString::number(myMonitors.at(i).geometry.y()).toStdString().c_str());
//		jMonitorObject.insert("width", QString::number(myMonitors.at(i).geometry.width()).toStdString().c_str());
//		jMonitorObject.insert("height", QString::number(myMonitors.at(i).geometry.height()).toStdString().c_str());

        jMonitorObject.insert("x", QString::number(myMonitors[currKey].geometry.x()).toStdString().c_str());
        jMonitorObject.insert("y", QString::number(myMonitors[currKey].geometry.y()).toStdString().c_str());
        jMonitorObject.insert("width", QString::number(myMonitors[currKey].geometry.width()).toStdString().c_str());
        jMonitorObject.insert("height", QString::number(myMonitors[currKey].geometry.height()).toStdString().c_str());



        QJsonValue tmpValue = jMonitorObject;
        jArray.insert(i, tmpValue);
    }

    data.insert("monitorData", jArray);
    qDebug() << QJsonDocument(data);
    return data;

}

void MainWindow::jsonToMonitorData(QJsonDocument jDoc) {
    // called by the server instance to decode the JSON object received by the client with its monitor configuration
    qDebug() << "jsonToMonitorData()";
    qDebug() << jDoc;

    // for whatever reason this toString().toInt() is necessary, instead of doing toInt() only
    int monitors = jDoc.object()["monitorNum"].toString().toInt();
    QJsonArray monitorData = jDoc.object()["monitorData"].toArray();

    for (int i = 0; i < monitors; i++) {
        monitor tmp;
        tmp.geometry.setX(monitorData.at(i)["x"].toString().toInt());
        tmp.geometry.setY(monitorData.at(i)["y"].toString().toInt());
        tmp.geometry.setWidth(monitorData.at(i)["width"].toString().toInt());
        tmp.geometry.setHeight(monitorData.at(i)["height"].toString().toInt());
        tmp.name = monitorData.at(i)["name"].toString();
        clientMonitors.append(tmp);
    }


    for (int i = 0; i < clientMonitors.size(); i++) { qDebug() << clientMonitors.at(i).geometry;	}	// debug

}

void MainWindow::listMyMonitorsInUI() {
    // put the configurations of the monitors on this device
    // initialise and create the model for the UI
    qDebug() << "listMyMonitorsInUI()";

    monitorModel = new QStandardItemModel(this);
    monitorModel->setHorizontalHeaderItem(0, new QStandardItem(QString("Monitor")));
    monitorModel->setHorizontalHeaderItem(1, new QStandardItem(QString("Owner")));
    monitorModel->setHorizontalHeaderItem(2, new QStandardItem(QString("Pos X")));
    monitorModel->setHorizontalHeaderItem(3, new QStandardItem(QString("Pos Y")));
    monitorModel->setHorizontalHeaderItem(4, new QStandardItem(QString("Resolution")));
    ui->monitorTableView->setModel(monitorModel);


    // show items in model
    for (int i = 0; i < myMonitors.keys().size(); i++) {
        QString currKey = myMonitors.keys().at(i);

        monitorModel->setItem(i, 0, new QStandardItem(myMonitors[currKey].name));
        monitorModel->setItem(i, 1, new QStandardItem(QString("ME")));
        monitorModel->setItem(i, 2, new QStandardItem(QString::number(myMonitors[currKey].geometry.x())));
        monitorModel->setItem(i, 3, new QStandardItem(QString::number(myMonitors[currKey].geometry.y())));

        QString resolution = QString::number(myMonitors[currKey].geometry.size().width());
        resolution += "x";
        resolution += QString::number(myMonitors[currKey].geometry.size().height());
        monitorModel->setItem(i, 4, new QStandardItem(resolution));
    }

    ui->monitorTableView->resizeColumnsToContents();
    ui->monitorTableView->resizeRowsToContents();

}

void MainWindow::drawMyMonitorsInUI() {

}

void MainWindow::listClientMonitorsInUI() {
    // displays the list of monitors received by the client in the UI
    qDebug() << "listClientMonitorsInUI()";

    // show items in model
    int o = myMonitors.size();
    for (int i = 0; i < clientMonitors.size(); i++) {
        monitorModel->setItem(i+o, 0, new QStandardItem(clientMonitors.at(i).name));
        monitorModel->setItem(i+o, 1, new QStandardItem(serverSocket->peerAddress().toString()));
        monitorModel->setItem(i+o, 2, new QStandardItem(QString::number(clientMonitors.at(i).geometry.x())));
        monitorModel->setItem(i+o, 3, new QStandardItem(QString::number(clientMonitors.at(i).geometry.y())));

        QString resolution = QString::number(clientMonitors.at(i).geometry.size().width());
        resolution += "x";
        resolution += QString::number(clientMonitors.at(i).geometry.size().height());
        monitorModel->setItem(i+o, 4, new QStandardItem(resolution));
    }

    ui->monitorTableView->resizeColumnsToContents();
    ui->monitorTableView->resizeRowsToContents();

}

void MainWindow::drawClientMonitorsInUI() {

}



void MainWindow::showNetworkInterfaces() {
     // display list of network interfaces in the UI
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    QStringList strList;
    strList.push_back("All Interfaces");

    for (int i = 0; i < list.size(); i++) {
        strList.push_back(list.at(i).humanReadableName());
//		strList.push_back(list.at(i).name());	// these names on windows are not very descriptive

        for (int i1 = 0; i1 < list.at(i).addressEntries().size(); i1++) {
            qDebug() << "\t" << list.at(i).addressEntries().at(i1).ip().toString();
        }
    }

    ui->networkIfaceComboBox->addItems(strList);

}

void MainWindow::newConnection() {
    // called when server instance received an incoming connection
    // the server will be the one where the hardware mouse/keyboard is being moved on
    qDebug() << "newConnection()";
    serverSocket = listenSocket->nextPendingConnection();
    connect(serverSocket, SIGNAL(disconnected()), this, SLOT(clearClientData()));

    QByteArray data;

    // receive monitor details of the other device
    qDebug() << "Waiting to receive monitor information from client...";
    if (serverSocket->waitForReadyRead()) {
        data = serverSocket->readAll();
        qDebug() << "Read bytes " << data.size();
        qDebug() << data;
    } else {
        qDebug() << "Did not receive monitor information in time";
    }

    // decode monitor data and save it
    jsonToMonitorData(QJsonDocument::fromBinaryData(data));

    // display monitors received by clients in the UI
    listClientMonitorsInUI();
    drawClientMonitorsInUI();	// TODO: implement this function


    // start the loop to send mouse movements to the client



}

void MainWindow::connectedToServer() {
    // called when client socket connects to server instance
    qDebug() << "connectedToServer()";

    // send information about this machine's monitor to server
    // send it in JSON format, so it is easy to encode/decode
    QThread::sleep(1);	// this is to wait for the receiver to start the waitForReadyRead()
    qDebug () << "Sending monitor info...";

    QJsonObject jMonitorData = monitorDataToJSON();
    QByteArray monitorData = QJsonDocument(jMonitorData).toJson();

    int dataSent = (int)clientSocket->write(monitorData);
    if (dataSent == -1) { qDebug() << "Failed to send monitor data"; }





}

void MainWindow::clearClientData() {
    // this is called if the connection between the server and client drops
    clientMonitors.clear();

}

void MainWindow::processMouseMoved(QPoint p) {
    // display mouse location on UI
    QString s = QString::number(p.x()) + " x " + QString::number(p.y());
    ui->mouseLocationLabel->setText(s);   // TODO: this can actually use a fair amount of CPU

    // check whether mouse location needs to be sent to server/client


}

void MainWindow::showAllActions() {
    // shows all the actions recorded
    qDebug() << "\n\nShowing all the actions recorded ... " << actions.size();
    qDebug() << "index\ttime\tdelay\taction\tpos";
    for (int i = 0; i < actions.size(); i++) {
        qDebug() << i << "\t" << actions.at(i).time << "\t" << actions.at(i).delay << "\t" << actions.at(i).action << "\t" << actions.at(i).pos;

    }

}

void MainWindow::on_loadButton_clicked() {
    loadActions(ui->loadPathEdit->text().toStdString());

}

void MainWindow::on_loadPathEdit_textChanged(const QString &arg1) {
//    qDebug() << "on_loadPathEdit_textChanged";
    if (!arg1.isEmpty()) {
        ui->loadButton->setEnabled(true);
    } else {
        ui->loadButton->setEnabled(false);
    }
}

void MainWindow::on_savePathEdit_textChanged(const QString &arg1) {
//    qDebug() << "on_savePathEdit_textChanged";
    if (!arg1.isEmpty()) {
        ui->saveButton->setEnabled(true);
    } else {
        ui->saveButton->setEnabled(false);
    }
}

void MainWindow::on_saveButton_clicked() {
    saveActions(ui->savePathEdit->text().toStdString());

}

void MainWindow::on_repeatBox_toggled(bool checked) {
    if (checked) {
        // set up to repeat actions multiple times
        ui->repeatTimesEdit->setEnabled(true);
    } else {
        repeatTotal = 1;
        ui->repeatTimesEdit->setText(QString::number(repeatTotal));
        ui->repeatTimesEdit->setEnabled(false);
    }

}

void MainWindow::on_repeatTimesEdit_textChanged(const QString &arg1) {
    repeatTotal = arg1.toInt();

}

void MainWindow::on_listenButton_clicked() {
    // start/stop listening on the interface specified in the combobox
    if (listenSocket->isListening()) {
        // stop listening
        ui->listenButton->setText("Listen");
        ui->ifacePortLabel->setText("");
        listenSocket->close();
        return;
    }

    ui->listenButton->setText("Close Sock");    // update text on button

    // update text in the label to show where the program is listening on
    int intIndex = ui->networkIfaceComboBox->currentIndex();
    QNetworkInterface netInt;
    QString intIP;
    QString labelMessage = "Listening on ";

    if (intIndex == 0) {
        intIP = "0.0.0.0";
    } else {
        qDebug () << "intIndex: " << QString::number(intIndex);
        netInt = QNetworkInterface::allInterfaces().at(intIndex-1);
        if (!netInt.isValid()) {
            qDebug() << "netInt is invalid";
            return;
        }

        int max = netInt.addressEntries().size();
        intIP = netInt.addressEntries().at(max-1).ip().toString();
        labelMessage.append(intIP);
    }

    labelMessage.append(intIP + ":" + QString::number(port));
    ui->ifacePortLabel->setText(labelMessage);
    qDebug() << labelMessage;


    // start listening on QTcpServer socket
    if (intIndex == 0) {
        if (!listenSocket->listen(QHostAddress::Any, port)) {
            qDebug() << "Server could not start";
            return;
        }

    } else {
        if (!listenSocket->listen(QHostAddress(intIP), port)) {
            qDebug() << "Server could not start";
            return;
        }
    }

}

void MainWindow::on_connectButton_clicked() {
    // connect to the host running the server instance
    clientSocket = new QTcpSocket(this);
    connect(clientSocket, SIGNAL(connected()), this, SLOT(connectedToServer()));
    connect(clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this
            , SLOT(updateConnectionStatus(QAbstractSocket::SocketState)));

    clientSocket->connectToHost(ui->serverIPEdit->text(), port);
    if (!clientSocket->waitForConnected(5000)) {
        qDebug() << "Could not connect to server";
        delete clientSocket;
        return;
    }

}
