#include "wallpaperchanger.h"
#include "./ui_wallpaperchanger.h"

//external libs
#include "json/json.hpp"

//system stuff
#include <windows.h>
#include <Lmcons.h>

//std includes
#include <filesystem>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
//qt includes
#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>
#include <chrono>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QIcon>

using njson = nlohmann::json;
njson config; //OH, NOO, A GLOBAL, WHAT SHALL WE DO?????????????????????????????



WallpaperChanger::WallpaperChanger(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::WallpaperChanger)
{
    ui->setupUi(this);

    this->setFixedSize(this->size());
    this->setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);


    checkConfig();
    getWallpapers();
    wallCountdown = new QTimer(this);
    connect(wallCountdown, &QTimer::timeout,this,&WallpaperChanger::changeWallpaper); //when to change wallpaper
    uiPoller = new QTimer(this);
    connect(uiPoller,&QTimer::timeout, this, &WallpaperChanger::updateTimeLeft);
    uiPoller->start(500);
    setupTray();
    startCountdown(wallCountdown);
}

WallpaperChanger::~WallpaperChanger()
{
    delete ui;
}


void WallpaperChanger::setupTray()
{
    trayIcon = new QSystemTrayIcon(this);

    trayIcon->setIcon(QIcon(":assets/graphicdesignismypassion.png"));
    trayIcon->setToolTip("Wallpaper changer :)");

    QMenu* trayMenu = new QMenu(this);

    QAction* showAction = trayMenu->addAction("Show");
    QAction* changeAction = trayMenu->addAction("ChangeWallpaper");
    QAction* quitAction = trayMenu->addAction("quit");

    connect(showAction, &QAction::triggered, this, &QMainWindow::show);
    connect(changeAction, &QAction::triggered, this, &WallpaperChanger::changeWallpaper);
    connect(quitAction, &QAction::triggered, this, &QApplication::quit);
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    connect(trayIcon, &QSystemTrayIcon::activated, this, [=](QSystemTrayIcon::ActivationReason reason){
    if (reason == QSystemTrayIcon::DoubleClick) {
        show();
        raise();
        activateWindow();
    }});
}

void WallpaperChanger::closeEvent(QCloseEvent *event)
{
    if(config["closeToTray"] == true && trayIcon->isVisible())
    {
        hide();
        event->ignore();
    }
    else
    {
        event->accept();
    }
}

///config related stuff
void WallpaperChanger::checkConfig() //checks current info in config.json
{
    std::filesystem::path path = std::filesystem::current_path() / "config.json";

    qDebug() << "current path: " << path.string();
    QString test2 = QString::fromStdString(path.string());
    qDebug() << test2;
    if(!std::filesystem::exists(path)) //if config does not exist, create one
    {
        createConfig();
        return;
    }

    std::ifstream file(path);
    file >> config;
    file.close();


    ui->adressLned->setText(QString::fromUtf8((config["location"].get<std::string>().c_str()))); //just fucking kill me now
    ui->hourSpn->setValue(config["hours"]);
    ui->minSpn->setValue(config["minutes"]);
    if(config["shuffle"] == true) {ui->shuffleChk->setCheckState(Qt::Checked);}
    else {ui->shuffleChk->setCheckState(Qt::Unchecked);}
    if(config["closeToTray"] == true) {ui->actionExit_to_tray->setChecked(true);}
    else { ui->actionExit_to_tray->setChecked(false); }

}

void WallpaperChanger::createConfig() //creates the default config
{
    qDebug() << "creating config!";

    config["hours"] = 4;
    config["minutes"] = 0;
    config["shuffle"] = true;
    config["closeToTray"] = true;
    char username[UNLEN + 1];
    DWORD size = UNLEN + 1;

    if (GetUserNameA(username, &size))
    {
        qDebug() << "got username!";
    }
    else
    {
        qDebug() << "Failed to get username! huh.......";
    }

    std::string user(username);

    std::filesystem::path picturesPath = std::filesystem::path("C:/Users") / user / "Pictures";



    qDebug() << "pictures path" << picturesPath.string();
    if (std::filesystem::exists(picturesPath))
    {
        config["location"] = picturesPath.string();
    }
    else
    {
        qDebug() << "Failed to get username, why????????";
    }

    config["lastLocation"] = config["location"];

    std::ofstream file("config.json");
    file << config.dump(4);
    file.close();
    checkConfig();
}



///wallpaper related stuff
void WallpaperChanger::changeWallpaper()
{
    try
    {
        qDebug() << "changing wallpaper";
        if (currentWall >= wallpapers.size())
        {
            currentWall = 0;
        }
        if (wallpapers.empty())
        {
            qDebug() << "dis stupid wallpaper list is empty!";
         return;
        }
    }
    catch (const std::exception& e)
    {
        qDebug() << "some weird shit happened" << e.what();
    }

    std::string wall = wallpapers.at(currentWall);
    qDebug() << "wallpaper to change is: " << wall;
    currentWall++;
    QByteArray byteArray = QByteArray::fromStdString(wall);

    const char* wallPath = byteArray.constData();
    bool changed = SystemParametersInfoA(
        SPI_SETDESKWALLPAPER,
        0,
        (PVOID)wallPath,
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE
        );

    if (!changed) { qDebug() << "did not change wallpaper :("; }
    else {qDebug() << "changed wallpaper :)"; }
}

bool WallpaperChanger::checkWallpaper(const std::filesystem::path &p) //checks if the wallpaper is actually an wallpaper, this may need a better name
{
    static const std::vector<std::string> ends = {
        ".png", ".jpg", ".jpeg", ".bmp"
    };
    std::string end = p.extension().string();
    return std::find(ends.begin(), ends.end(), end) != ends.end();
}

void WallpaperChanger::getWallpapers() //gets the wallpapers
{
    try
    {
        for(const auto& entry : std::filesystem::directory_iterator(config["location"]))
        {
            if(std::filesystem::is_regular_file(entry.status()) && checkWallpaper(entry.path()))
            {
                wallpapers.push_back(entry.path().string());
                qDebug() << "added" << QString::fromStdString(entry.path().string()) << "to wallpaper list";
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr << "error doing stuff: " << e.what() << std::endl;
    }

    if (config["shuffle"] == true)
    {
        shuffleWallpapersNormal();
    }
}

void WallpaperChanger::shuffleWallpapersNormal()
{
    auto now = std::chrono::high_resolution_clock::now();
    unsigned seed = now.time_since_epoch().count();
    std::mt19937 g(seed); //this is some egyptian alien type shit

    std::shuffle(wallpapers.begin(), wallpapers.end(), g);
}


///time related stuff
void WallpaperChanger::startCountdown(QTimer *timer)
{
    if (timer->isActive())
    {
        timer->stop();
    }
    int time = 0;
    int timeMins = config["minutes"];
    int timeHrs = config["hours"];

    time += timeMins * 60;
    time += timeHrs * 60 * 60;
    time *= 1000;
    qDebug() << "started countdown of " << timeHrs << " hours and " << timeMins << " minutes";
    timer->start(time);
}

void WallpaperChanger::updateTimeLeft()
{
    long long hours = wallCountdown->remainingTime() / 3600000;
    long long minutes = (wallCountdown->remainingTime() % 3600000) / 60000;
    long long seconds = (wallCountdown->remainingTime() % 60000) / 1000;

    QString timeLeft = QString("time until next change: %1:%2:%3")
                           .arg(hours, 2, 10, QChar('0'))
                           .arg(minutes, 2, 10, QChar('0'))
                           .arg(seconds, 2, 10, QChar('0')); //i fucking hate QT so much it's unreal
    ui->timeLeftLbl->setText(timeLeft);

}



///what happens if buttons get clicked
void WallpaperChanger::on_saveBtn_clicked() //dumps all ui stuff into the json config file thingy funny
{
    bool shouldError = false;

    std::string loc = ui->adressLned->text().toStdString();

    if (std::filesystem::is_directory(loc))
    {
        qDebug() << "it's a directory, moving on";
    }
    else if (std::filesystem::exists(loc))
    {
        QMessageBox::critical(this, "Error", "This is not a folder!");
        return;
    }
    else
    {
        QMessageBox::critical(this, "Error", "Selected folder does not exist!");
        return;
    }


    if(ui->minSpn->text().toInt() == 0 && ui->hourSpn->text().toInt() == 0)
    {
        QMessageBox::critical(this, "Error", "Time cannot be lower than 1 minute!");
        return;
    }

    if (std::filesystem::is_empty(loc))
    {
        QMessageBox::critical(this, "Error", "The folder has no wallpapers on it, so no wallpaper changes will happen");
    }
    config["hours"] = ui->hourSpn->text().toInt();
    config["minutes"] = ui->minSpn->text().toInt();
    if (ui->shuffleChk->isChecked()) { config["shuffle"] = true; }
    else { config["shuffle"] = false; }

    if(ui->actionExit_to_tray->isChecked()) {config["closeToTray"] = true; }
    else { config["closeToTray"] = false; }

    config["location"] = ui->adressLned->text().toStdString();
    std::ofstream file("config.json");
    file << config.dump(4);
    file.close();
    startCountdown(wallCountdown);
    getWallpapers();

}


void WallpaperChanger::on_changeBtn_clicked()
{
    changeWallpaper();
}


void WallpaperChanger::on_browseBtn_clicked()
{
    ui->adressLned->setText((QFileDialog::getExistingDirectory(this, "Select folder", "C:/Users", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks)));
    config["lastLocation"] = ui->adressLned->text().toStdString();
}



