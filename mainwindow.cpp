#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionRun_triggered()
{
    qDebug()<<"a";
//    cv::Mat a;
//    Mat a;
    Mat img=imread("D://a.png",CV_LOAD_IMAGE_COLOR);
    if(!img.data)
    {
        qDebug()<<"read file D:/a.png failed!";
    }
    else{
        namedWindow("test",WINDOW_AUTOSIZE);
        imshow("test",img);
    }
}
