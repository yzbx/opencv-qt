#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    bgsFactory_yzbx bgsFac;
    QStringList list=bgsFac.getBgsTypeList();
    ui->comboBox_bgsType->addItems(list);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_inputPath_clicked()
{
//    QString fileName = QFileDialog::getOpenFileName(this,
//                                                    tr("Open bgs exe"), "",
//                                                    tr("bgs (*.exe)"));

    QString fileName = QFileDialog::getExistingDirectory(this, tr("Open Input Directory"),
                                                         ".",
                                                         QFileDialog::ShowDirsOnly
                                                         | QFileDialog::DontResolveSymlinks);
    if(!fileName.isEmpty()){
        ui->lineEdit_inputPath->setText(fileName);
    }
}

void MainWindow::on_pushButton_outputPath_clicked()
{
    QString fileName = QFileDialog::getExistingDirectory(this, tr("Open Input Directory"),
                                                         ".",
                                                         QFileDialog::ShowDirsOnly
                                                         | QFileDialog::DontResolveSymlinks);
    if(!fileName.isEmpty()){
        ui->lineEdit_outputPath->setText(fileName);
    }
}

void MainWindow::on_pushButton_bgs_clicked()
{
    cv::destroyAllWindows();
    QString inputPath=ui->lineEdit_inputPath->text();
    QString bgsType=ui->comboBox_bgsType->currentText();
    bgsFactory_yzbx bgsFac;
    bgsFac.process(bgsType,inputPath);
}
