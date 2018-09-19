#ifndef IMAGESTREAM_H
#define IMAGESTREAM_H

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
#include <QLabel>
#include <QThread>
#include <QString>
#include <QFile>
#include <QTextCodec>
#include <QPixmap>
#include <QPixelFormat>
#include <QImage>
#include <QSettings>

namespace Ui {
class ImageStream;
}

class ImageStream : public QWidget
{
    Q_OBJECT

public:
    explicit ImageStream(QWidget *parent = nullptr);
    ~ImageStream();
    //! UI object
    Ui::ImageStream *ui;

};

#endif // MAINWINDOW_H
