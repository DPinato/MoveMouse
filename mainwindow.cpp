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


	// put text in the various edits, labels and buttons
	ui->console->setText("No file loaded");
	ui->repeatTimesEdit->setText(QString::number(repeatTotal));
	ui->loadPathEdit->setText(defFileToLoad);


	// initialise data
	Mouse::tmpActFile = tempFileDir;
	loadActions(tempFileDir.toStdString());





	// debug    ===============================================================
	//    qDebug() << "argc: " << QCoreApplication::arguments().size();
	//    for (int i = 0; i < QCoreApplication::arguments().size(); i++) {
	//        qDebug() << i << "\targv: " << QCoreApplication::arguments().at(i);
	//    }

	qDebug() << "exeDir: " << exeDir;
	qDebug() << "tempFileDir; " << tempFileDir;


}

MainWindow::~MainWindow() {
	timer->stop();
	//    UnhookWindowsHookEx(MouseHook);
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
		UnhookWindowsHookEx(MouseHook);
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
			MouseHook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)Mouse::MouseHookProc, GetModuleHandle(NULL), 0);
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

	SetCursorPos(actions.at(index).pos.x(), actions.at(index).pos.y());

	switch (actions.at(index).action) {
		case (513) : {
			// Left mouse DOWN
			qDebug() << "\tLeft mouse DOWN ... index: " << index << "\t" << actions.at(index).pos;
			mouse_event(MOUSEEVENTF_LEFTDOWN, actions.at(index).pos.x(), actions.at(index).pos.y(), 0, 0);
			break;
		}
		case (514) : {
			// Left mouse UP
			qDebug() << "\tLeft mouse UP ... index: " << index << "\t" << actions.at(index).pos;
			mouse_event(MOUSEEVENTF_LEFTUP, actions.at(index).pos.x(), actions.at(index).pos.y(), 0, 0);
			break;
		}
		case (516) : {
			// Right mouse DOWN
			qDebug() << "\tRight mouse DOWN ... index: " << index << "\t" << actions.at(index).pos;
			mouse_event(MOUSEEVENTF_RIGHTDOWN, actions.at(index).pos.x(), actions.at(index).pos.y(), 0, 0);
			break;
		}
		case (517) : {
			// Right mouse UP
			qDebug() << "\tRight mouse UP ... index: " << index << "\t" << actions.at(index).pos;
			mouse_event(MOUSEEVENTF_RIGHTUP, actions.at(index).pos.x(), actions.at(index).pos.y(), 0, 0);
			break;
		}

	}


}

void MainWindow::updateActionCounterLabel() {
	// build the string in this format <rep> / <totalRep>
	ui->actionCounterLabel->setText(QString(QString::number(currRep) + " / " + QString::number(repeatTotal)));

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
