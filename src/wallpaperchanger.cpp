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
#include "QtConcurrent/QtConcurrent" // wtf

using njson = nlohmann::json;
struct programState //OH, NOO, A GLOBAL, WHAT SHALL WE DO?????????????????????????????
{
    njson config;
    int wallCount = 0;
    int sinceChanged = 0;
};
programState state;



WallpaperChanger::WallpaperChanger(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::WallpaperChanger)
{
    ui->setupUi(this);

    this->setFixedSize(this->size());
    this->setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);

    checkConfig();
    getWallpapers(state.config["changeOnOpen"]);
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
    trayIcon->setToolTip("Wallpaper changer ");

    QMenu* trayMenu = new QMenu(this);

    QAction* showAction = trayMenu->addAction("Show");
    QAction* changeAction = trayMenu->addAction("Change wallpaper");
    QAction* quitAction = trayMenu->addAction("Quit");

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
    if(state.config["closeToTray"] == true && trayIcon->isVisible())
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
    if(!std::filesystem::exists(path)) //if state.config does not exist, create one
    {
        createConfig();
        return;
    }

    std::ifstream file(path);
    file >> state.config;
    file.close();


    ui->adressLned->setText(QString::fromUtf8((state.config["location"].get<std::string>().c_str()))); //just fucking kill me now
    ui->hourSpn->setValue(state.config["hours"]);
    ui->minSpn->setValue(state.config["minutes"]);
    if(state.config["shuffle"] == true) {ui->shuffleChk->setCheckState(Qt::Checked);}
    else {ui->shuffleChk->setCheckState(Qt::Unchecked);}
    if(state.config["closeToTray"] == true) {ui->actionExit_to_tray->setChecked(true);}
    else { ui->actionExit_to_tray->setChecked(false); }
    if(state.config["changeOnOpen"] == true){ ui->acionChange_on_open->setChecked(true); }
    else { ui->acionChange_on_open->setChecked(false); }
    if(state.config["changeOnSave"] == true){ ui->actionChange_on_save->setChecked(true); }
    else { ui->actionChange_on_save->setChecked(false); }
    //TODO: i could add a thing that just does this as a function, but idk if that would be very worthwhile for such a small program
}

void WallpaperChanger::createConfig() //creates the default config
{
    qDebug() << "creating state.config!";

    state.config["hours"] = 4;
    state.config["minutes"] = 0;
    state.config["shuffle"] = true;
    state.config["closeToTray"] = true;
    state.config["changeOnOpen"] = true;
    state.config["changeOnSave"] = false;
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

    std::filesystem::path picturesPath = std::filesystem::path("C:\\Users") / user / "Pictures";



    qDebug() << "pictures path" << picturesPath.string();
    if (std::filesystem::exists(picturesPath))
    {
        state.config["location"] = picturesPath.string();
    }
    else
    {
        qDebug() << "Failed to get username, why????????";
    }

    state.config["lastLocation"] = state.config["location"];

    std::ofstream file("config.json");
    file << state.config.dump(4);
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
    ++state.sinceChanged;
    if(state.sinceChanged>=5)
    {
        shuffleWallpapersNormal();
        state.sinceChanged = 0;
    }
}

bool WallpaperChanger::checkWallpaper(const std::filesystem::path &p) //checks if the wallpaper is actually an wallpaper, this may need a better name
{
    static const std::vector<std::string> ends = {
        ".png", ".jpg", ".jpeg", ".bmp"
    };
    std::string end = p.extension().string();
    return std::find(ends.begin(), ends.end(), end) != ends.end();
}

void WallpaperChanger::getWallpapers(bool shouldChange)
{
    QtConcurrent::run([=]() {
        state.wallCount = 0;
        wallpapers.clear();
        try {
            ui->saveBtn->setEnabled(false);
            ui->changeBtn->setEnabled(false);
            for (const auto& entry : std::filesystem::directory_iterator(state.config["location"])) {
                if (std::filesystem::is_regular_file(entry.status()) && checkWallpaper(entry.path())) {
                    wallpapers.push_back(entry.path().string());

                    // update wallpaper count safely on main thread
                    QMetaObject::invokeMethod(this, [=]() {
                        state.wallCount++;
                        updateWallCount();
                    }, Qt::QueuedConnection);
                }
            }
            ui->saveBtn->setEnabled(true);
            ui->changeBtn->setEnabled(true);
        } catch (const std::filesystem::filesystem_error& e) {
            qDebug() << "Error: " << e.what();
        }

        if (state.config["shuffle"] == true) {
            shuffleWallpapersNormal();
        }

        // final UI updates and wallpaper change
        QMetaObject::invokeMethod(this, [=]() {
            updateWallCount();
            ui->saveBtn->setEnabled(true);
            if (shouldChange) changeWallpaper();
        }, Qt::QueuedConnection);
    });

}




void WallpaperChanger::shuffleWallpapersNormal()
{
    auto now = std::chrono::high_resolution_clock::now();
    unsigned seed = now.time_since_epoch().count();
    std::mt19937 g(seed); //this is some egyptian alien type shit
    qDebug() << "shuffled wallpapers!";
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
    int timeMins = state.config["minutes"];
    int timeHrs = state.config["hours"];

    time += timeMins * 60;
    time += timeHrs * 60 * 60;
    time *= 1000;
    qDebug() << "started countdown of " << timeHrs << " hours and " << timeMins << " minutes";
    timer->start(time);
}

void WallpaperChanger::updateWallCount()
{
    //qDebug() << "updating wallpaper count";
    QString count = QString("Wallpapers loaded: %1").arg(state.wallCount);
    ui->wallCountLbl->setText(count);
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
void WallpaperChanger::on_saveBtn_clicked() //dumps all ui stuff into the json state.config file thingy funny
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
    state.config["hours"] = ui->hourSpn->text().toInt();
    state.config["minutes"] = ui->minSpn->text().toInt();
    if (ui->shuffleChk->isChecked()) { state.config["shuffle"] = true; }
    else { state.config["shuffle"] = false; }

    if(ui->actionExit_to_tray->isChecked()) {state.config["closeToTray"] = true; }
    else { state.config["closeToTray"] = false; }

    if(ui->actionChange_on_save->isChecked()){state.config["changeOnSave"] = true;}
    else {state.config["changeOnSave"] = false;}
    if(ui->acionChange_on_open->isChecked()){state.config["changeOnOpen"] = true;}
    else{state.config["changeOnOpen"] = false;}

    state.config["location"] = ui->adressLned->text().toStdString();
    std::ofstream file("config.json");
    file << state.config.dump(4);
    file.close();
    startCountdown(wallCountdown);

    getWallpapers(state.config["changeOnSave"]);
}


void WallpaperChanger::on_changeBtn_clicked()
{
    changeWallpaper();
}


void WallpaperChanger::on_browseBtn_clicked()
{
    ui->adressLned->setText((QFileDialog::getExistingDirectory(this, "Select folder", "C:/Users", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks)));
    state.config["lastLocation"] = ui->adressLned->text().toStdString();
}

