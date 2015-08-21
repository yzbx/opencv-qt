#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
//#include <QtPlugin>
#include <QtSql>
#include <QPainter>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <vector>

using namespace std;
using namespace cv;

namespace Ui {
class MainWindow;
class Form;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void init();

private slots:
    void on_actionRun_triggered();

    void on_pushButton_5_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_7_clicked();

    void on_actionTestsplite_triggered();
    
    void on_actionTestvideo_triggered();

    void on_actionSaveFrame_triggered();

    void on_actionSaveKeyFrame_triggered();

    void on_actionTestvideoinfo_triggered();

private:
    Ui::MainWindow *ui;
    QWidget *form;
    Ui::Form *videotestui;
    Mat srcImage;
    Mat frame;
protected:
    void paintEvent(QPaintEvent *e);

};

#endif // MAINWINDOW_H
