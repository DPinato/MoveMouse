#ifndef MOUSE_H
#define MOUSE_H

#include "mainwindow.h"

class Mouse
{
public:
    Mouse();

    static LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);
    static QString tmpActFile;

};

#endif // MOUSE_H
