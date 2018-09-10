#include "frame.h"
#include "ui_frame.h"

Frame::Frame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::Frame)
{
    ui->setupUi(this);
    resize(800, 600);
    connect(this, SIGNAL(updatedDisplay()), this, SLOT(updateDisplay()), Qt::QueuedConnection);
}

Frame::~Frame()
{
    delete ui;
}


void Frame::refreshImage(QImage* img)
{
    ui->IMAGE_RGB->setPixmap(QPixmap::fromImage(*img).scaled(800,600, Qt::KeepAspectRatio));
    repaint();
}

void Frame::goOffline(QPixmap img)
{
    ui->IMAGE_RGB->setPixmap(img);
    repaint();
}

void Frame::getImg(uchar* i)
{
    QImage* output = new QImage(i, 800, 600, QImage::Format_RGB888, nullptr, nullptr);
    img = QPixmap::fromImage(*output);
    emit updatedDisplay();
}

void Frame::process()
{
    //QImage* output_img1 = new QImage(img, 800, 600, QImage::Format_RGB888, nullptr, nullptr);
    //ui->IMAGE_RGB->setPixmap(QPixmap::fromImage(*output_img1).scaled(800,600, Qt::KeepAspectRatio));
    //repaint();
}

void Frame::updateDisplay(QImage i)
{
    img_changed = true;
    img = QPixmap::fromImage(i);
    ui->IMAGE_RGB->setPixmap(img);
    update();
}

void Frame::paintEvent(QPaintEvent*)
{
    if(img_changed)
        ui->IMAGE_RGB->setPixmap(img);
}
