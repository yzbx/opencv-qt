#include "bgstest.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BgsTest w;
    w.show();

    return a.exec();
}
