#include "wallpaperchanger.h"
#include <windows.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WallpaperChanger w;
    w.show();
    return a.exec();


}
