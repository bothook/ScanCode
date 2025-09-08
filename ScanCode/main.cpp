#include "ScanCode.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ScanCode w;
    w.show();
    return a.exec();
}
