#include "mouse.h"

Mouse::Mouse() {

}

Mouse::~Mouse() {

}

bool Mouse::startMouseTracking(Mouse *mObj) {
    // I have found this here:
    // http://osxbook.com/book/bonus/chapter2/altermouse/
    qDebug() << "Mouse::startMouseTracking()";
    CFMachPortRef eventTap;
    CGEventMask eventMask;
    CFRunLoopSourceRef runLoopSource;

    // Create an event tap. We are interested in mouse movements.
    eventMask = (1 << kCGEventMouseMoved);
    eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault, eventMask, myCGEventCallback, mObj);
    if (!eventTap) {
        qDebug() << "failed to create event tap";
        return false;
    }

    runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);        // Create a run loop source.
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);        // Add to the current run loop.
    CGEventTapEnable(eventTap, true);       // Enable the event tap.

//    CFRunLoopRun();   // this is not needed, as it blocks

    return true;

}

CGEventRef Mouse::myCGEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
    // Do some sanity check.
    if (type != kCGEventMouseMoved) { return event; }

    // The incoming mouse position.
    CGPoint location = CGEventGetLocation(event);

    // We can change aspects of the mouse event.
    // For example, we can use CGEventSetLoction(event, newLocation).
    // Here, we just print the location.
//    qDebug() << (int)location.x << "\t" << (int)location.y;

    // emit mouseMoved signal to tell MainWindow
    emit ((Mouse*)refcon)->mouseMoved(QPoint((int)location.x, (int)location.y));

    return event;    // We must return the event for it to be useful.

}

