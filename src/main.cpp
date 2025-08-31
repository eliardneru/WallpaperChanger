#include "wallpaperchanger.h"
#include <windows.h>
#include <QApplication>
#include <QIcon>
#include <QSharedMemory>
#include <QMessageBox>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WallpaperChanger w;
    w.setWindowIcon(QIcon(":/assets/graphicdesignismypassion.png"));
    QSharedMemory shared("WallpaperChangerUniqueKey");
    if (!shared.create(1)) {
        QMessageBox::warning(nullptr, "Already running", "Another instance of wallpaper changer is already running!");
        return 0; // exit
    }

    w.show();
    return a.exec();


}
