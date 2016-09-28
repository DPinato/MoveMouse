#include "mouse.h"

QString Mouse::tmpActFile = "";

Mouse::Mouse() {


}

LRESULT Mouse::MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    // detect mouse action and record it
    PKBDLLHOOKSTRUCT k = (PKBDLLHOOKSTRUCT)(lParam);
    POINT p;
    mouseAction temp;
    bool write = false;

//    qDebug() << "lParam ... " << lParam;

    switch (wParam) {
//    case (WM_MOUSEMOVE) : {
//        GetCursorPos(&p);
////        qDebug() << "Mouse moved ... " << p.x << "\t" << p.y;
//        break;
//    }

    case (WM_RBUTTONDOWN) : {
        GetCursorPos(&p);
        qDebug() << wParam << "\tRight button DOWN at " << p.x << "\t" << p.y;
        temp.action = (uint16_t)WM_RBUTTONDOWN;
        temp.time = QDateTime::currentMSecsSinceEpoch();
        write = true;
        break;
    }
    case (WM_RBUTTONUP) : {
        GetCursorPos(&p);
        qDebug() <<  wParam << "\tRight button UP at " << p.x << "\t" << p.y;
        temp.action = (uint16_t)WM_RBUTTONUP;
        temp.time = QDateTime::currentMSecsSinceEpoch();
        write = true;
        break;
    }

    case (WM_LBUTTONDOWN) : {
        GetCursorPos(&p);
        qDebug() <<  wParam << "\tLeft button DOWN at " << p.x << "\t" << p.y;
        temp.action = (uint16_t)WM_LBUTTONDOWN;
        temp.time = QDateTime::currentMSecsSinceEpoch();
        write = true;
        break;
    }
    case (WM_LBUTTONUP) : {
        GetCursorPos(&p);
        qDebug() <<  wParam << "\tLeft button UP at " << p.x << "\t" << p.y;
        temp.action = (uint16_t)WM_LBUTTONUP;
        temp.time = QDateTime::currentMSecsSinceEpoch();
        write = true;
        break;
    }

    }


    temp.pos = QPoint(p.x, p.y);

    // only write if something useful was taken
    if (write) {
        if (tmpActFile.isEmpty()) {
            qDebug() << "NO TEMP FILE FOR ACTIONS WAS DEFINED";

        } else {
            ofstream toTemp(Mouse::tmpActFile.toStdString().c_str(), ios::app);
            if (toTemp.is_open()) {
                toTemp << temp.time << "\t" << temp.action << "\t" << temp.pos.x() << "\t" << temp.pos.y();
                toTemp << "\n";
                toTemp.close();

            }
        }

    }


    return CallNextHookEx(NULL, nCode, wParam, lParam);

}
