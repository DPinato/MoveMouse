#ifndef MOUSE_H
#define MOUSE_H

#include "mainwindow.h"

class Mouse : public QObject {
		Q_OBJECT

	public:
		Mouse();
		~Mouse();

		static LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);
		static QString tmpActFile;


		// get
		QPoint getMousePosition();

		// set
		void setMousePosition(QPoint pos);

	public slots:
		void trackMousePosition(int interval);

	private:
		QPoint mousePosition;

};

#endif // MOUSE_H
