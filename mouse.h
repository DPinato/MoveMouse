#ifndef MOUSE_H
#define MOUSE_H

#include "mainwindow.h"

#include <ApplicationServices/ApplicationServices.h>

class Mouse : public QObject {
    Q_OBJECT


public:
    Mouse();
    ~Mouse();

    static CGEventRef myCGEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon);

public slots:
    static bool startMouseTracking(Mouse *mObj);

signals:
    void mouseMoved(QPoint pos);

private:
    QPoint mousePosition;

};

#endif // MOUSE_H
