#include "bgstest.h"
#include "ui_bgstest.h"

BgsTest::BgsTest(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BgsTest)
{
    ui->setupUi(this);
    initComboBox_bgsType();
}

BgsTest::~BgsTest()
{
    delete ui;
}

void BgsTest::initComboBox_bgsType()
{
    bgsFactory_yzbx bgsFac;
    QStringList list=bgsFac.getBgsTypeList();
    ui->comboBox_bgsType->addItems(list);
}

void BgsTest::on_pushButton_bgs_clicked()
{
    cv::destroyAllWindows();
    QString inputPath=ui->lineEdit_inputPath->text();
    QString bgsType=ui->comboBox_bgsType->currentText();
    bgsFactory_yzbx bgsFac;
    bgsFac.process(bgsType,inputPath);
}

void BgsTest::on_pushButton_inputPath_clicked()
{
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Open video file"), "",
                                                        tr("bgs (*.avi *.mp4 *.mov)"));

//        QString fileName = QFileDialog::getExistingDirectory(this, tr("Open Input Directory"),
//                                                             ".",
//                                                             QFileDialog::ShowDirsOnly
//                                                             | QFileDialog::DontResolveSymlinks);
        if(!fileName.isEmpty()){
            ui->lineEdit_inputPath->setText(fileName);
        }
}
