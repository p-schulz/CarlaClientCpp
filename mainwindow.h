#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>

#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>

#include "carlaclient.h"
#include "carla_server.pb.h"

#include <QMainWindow>
#include <QTableWidget>
#include <QLabel>
#include <QThread>
#include <QString>
#include <QFile>
#include <QTextCodec>
#include <QPixmap>
#include <QPixelFormat>
#include <QImage>
#include <QSettings>

#define TRAFFIC_SIGN = 0;
#define TRAFFIC_LIGHT = 1;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

typedef struct {
    float x;
    float y;
    float z;
} VEC_3D;

typedef struct {
    VEC_3D position;
    VEC_3D rotation;
    float speed;
    float throttle;
    float brake;
    float steering;
} EGO;

typedef struct {
    VEC_3D position;
    VEC_3D rotation;
    VEC_3D box_pos;
    VEC_3D box_ext;
} VEHICLE;

typedef struct {
    VEC_3D position;
    carla_server::TrafficLight_State state;
    float value;
    int type;
} GENERIC_OBJECT;


    explicit MainWindow(QWidget *parent = nullptr);
    void updateRGB(QPixmap img);
    ~MainWindow();

public slots:
    void streamImages();

private slots:
    void on_Button_connect_clicked();
    void on_Button_start_clicked();
    void on_Button_init_clicked();
    void on_Button_disconnect_clicked();
    void on_actionExit_triggered();
    void on_actionAbout_triggered();

    void on_actionSave_configuration_triggered();

private:
    //! parameter getter
    std::string getSceneDescription(std::string settings_dir);

    //! parameter setter
    void setSceneDescription();

    //! retrieve QString from paramater list
    QString getSimString(int row);

    //! retrieve string from paramater list
    std::string getSimText(int row);

    //! retrieve float from paramater list
    float getSimFloat(int row);

    //! retrieve int from paramater list
    int getSimInt(int row);

    //! initialize parameters
    bool setup();

    //! connect to sim server
    bool connectUE();

    //! disconnect from sim server
    void goOffline();

private:
    //! UI object
    Ui::MainWindow *ui;
    QThread* streamer_thread;

    //! debug flag
    bool debug;

    //! initialization flag
    bool init;

    //! online/offline flag
    bool connection;

    //! flag for dumping images to disk
    bool save_to_disk;

    //! current frame
    int frame;

    //! current settings
    QSettings* m_settings;

    //! settings directory
    std::string m_settings_dir;

    //! ip address of carla server
    std::string m_server;

    //! carla server sim port
    uint16_t m_world_port;

    //! client object
    CarlaClient* c_client;

    //! io_service for client
    boost::asio::io_service io_service;

    //! scene configuration
    std::string scene_description;

    //! scene settings object
    QSettings carla_settings;

signals:
    void start_streaming();
};

#endif // MAINWINDOW_H
