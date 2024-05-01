#include "megatron.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Megatron w;
    w.show();
    return a.exec();
}
