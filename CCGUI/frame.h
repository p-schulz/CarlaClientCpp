#ifndef FRAME_H
#define FRAME_H

#include <QFrame>
#include <QImage>
#include <QLabel>
#include <QImage>
#include <QPixmap>

namespace Ui {
class Frame;
}

class Frame : public QFrame
{
    Q_OBJECT

public:
    explicit Frame(QWidget *parent = nullptr);
    ~Frame();

    void refreshImage(QImage* img);
    void goOffline(QPixmap img);
    void getImg(uchar* img);
    void process();

public slots:
    void updateDisplay(QImage i);
protected:
    void paintEvent(QPaintEvent *pe);
private:
    bool img_changed;
    Ui::Frame *ui;
    QPixmap img;

signals:
    void updatedDisplay();
};

#endif // FRAME_H
