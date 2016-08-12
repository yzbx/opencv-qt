#include "trackingtest.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TrackingTest w;
    w.show();

    return a.exec();
}
