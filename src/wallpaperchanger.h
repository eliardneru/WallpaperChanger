#ifndef WALLPAPERCHANGER_H
#define WALLPAPERCHANGER_H

#include <QMainWindow>
#include <QTimer>
#include <vector>
#include <filesystem>
#include <QSystemTrayIcon>
#include <QCloseEvent>



QT_BEGIN_NAMESPACE
namespace Ui {
class WallpaperChanger;
}
QT_END_NAMESPACE

class WallpaperChanger : public QMainWindow
{
    Q_OBJECT

public:
    WallpaperChanger(QWidget *parent = nullptr);
    ~WallpaperChanger();
    void setupTray();
    void checkConfig();
    void createConfig();
    void changeWallpaper(); //changes the wallpaper
    bool checkWallpaper(const std::filesystem::path& p);
    void getWallpapers(bool shouldChange);
    void updateWallCount();
    void shuffleWallpapersNormal();
    void startCountdown(QTimer *timer);
    void updateTimeLeft();

private slots:

    void on_saveBtn_clicked();

    void on_changeBtn_clicked();

    void on_browseBtn_clicked();

protected:
    void closeEvent(QCloseEvent* event) override; // override the close event

private:
    Ui::WallpaperChanger *ui;
    QTimer *wallCountdown;
    QTimer *uiPoller;
    std::vector<std::string> wallpapers;
    int currentWall = 0; // random number that will be used to select which wallpaper we want
    QSystemTrayIcon* trayIcon;
};
#endif // WALLPAPERCHANGER_H
