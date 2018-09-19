// example code for use of client
// author: P. Schulz, 2018

#include "mainwindow.h"

#include <QApplication>
#include <QPushButton>
#include <QThreadPool>

inline size_t AlignUpTo8(size_t n) {
  // Align n to next multiple of 8 (from Hacker's Delight, Chapter 3.)
  return (n + 7) & -8;
}

int main(int argc, char *argv[])
{
    // create main window
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
