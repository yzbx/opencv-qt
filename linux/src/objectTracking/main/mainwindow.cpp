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

    trackingFactory_yzbx trackingFac;
    QStringList tlist=trackingFac.getTrackingTypeList();
    ui->comboBox_trackingType->addItems(tlist);
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
        QString configFile=fileName+"/yzbx.ini";

        QFileInfo finfo(configFile);
        if(finfo.exists()){
            loadIni(configFile);
        }
        else{
            qDebug()<<"cannot find file "<<configFile;
        }
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
        QString configFile=fileName+"/yzbx.ini";

        QFileInfo finfo(configFile);
        if(finfo.exists()){
            QSettings sett(configFile);
            sett.setValue("defaultOutputPath",fileName);
        }
        else{
            qDebug()<<"cannot find file "<<configFile;
        }
    }
}

void MainWindow::on_pushButton_bgs_clicked()
{
    cv::destroyAllWindows();
    QString inputPath=ui->lineEdit_inputPath->text();
    QString configFile=inputPath+"/yzbx.ini";

    QFileInfo finfo(configFile);
    if(finfo.exists()){
        loadIni(configFile);
    }
    else{
        qDebug()<<"cannot find file "<<configFile;
        exit(-1);
//        configFile=inputPath+"/yzbx.json";
//        loadJson(configFile);
    }


    //    QString bgsType=ui->comboBox_bgsType->currentText();
    //    bgsFactory_yzbx bgsFac;
    //    bgsFac.process(bgsType,inputPath);
}

void MainWindow::loadJson(QString filepath)
{
    QString filedata;
    QFile file;
    file.setFileName(filepath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug()<<"cannot open file "<<filepath;
        exit(-1);
    } else {
        QTextStream in(&file);
        filedata=in.readAll();
        file.close();
    }

    bool ok;
    QtJson::JsonObject result = QtJson::parse(filedata, ok).toMap();
    if (!ok) {
        qFatal("An error occurred during parsing");
    }
//    qDebug()<<"1"<<result["UrbanTracker"];
//    QtJson::JsonObject configFilePath=result["UrbanTracker"].toMap();
//    qDebug()<<"configFilePath="<<configFilePath["configFilePath"].toString();

//    qDebug()<<"input videos path"<<result["input videos path"].toString();
//    foreach(QVariant plugin, result["input videos"].toList()) {
//        qDebug() << "  -" << plugin.toString();
//    }
//    qDebug()<<"default="<<result["defaultOutputPath"].toString();

    baseVideoPath=result["input videos path"].toString();
    baseVideoPath=absolutePath(filepath,baseVideoPath);

//    QJsonDocument doc=QJsonDocument::fromJson(filedata.toUtf8());
//    qDebug()<<"doc="<<doc;
//    QJsonObject obj=doc.object();
//    qDebug()<<"obj="<<obj;
//    QJsonValue value=obj.value("UrbanTracker");
//    qDebug()<<"value="<<value.toString();
//    QJsonObject urbanTracker=value.toObject();
//    QJsonValue configFilePath=urbanTracker.value("configFilePath");
//    qDebug()<<"configFilePath="<<configFilePath.toString();

//    qDebug()<<obj.value("UrbanTracker").toObject().value("configFilePath").toString();
}

void MainWindow::loadIni(QString filepath)
{
    QSettings settings(filepath,QSettings::IniFormat);
    QString ut_configFilePath=settings.value("UrbanTracker/configFilePath").toString();
    qDebug()<<"abs test 1"<<absoluteFilePath(filepath,ut_configFilePath);

    QString defaultOutputPath=settings.value("defaultOutputPath").toString();
    QString inputVideosTxt=settings.value("inputVideosListTxt").toString();
    QString jsonFile=settings.value("ChineseConfigFile").toString();
    qDebug()<<"abs test 2"<<absoluteFilePath(filepath,defaultOutputPath);
    defaultOutputPath=absolutePath(filepath,defaultOutputPath);
    qDebug()<<"abs test 3"<<defaultOutputPath;

    inputVideosTxt=absoluteFilePath(filepath,inputVideosTxt);
    jsonFile=absoluteFilePath(filepath,jsonFile);
    loadJson(jsonFile);

    QString filedata;
    QFile file;
    file.setFileName(inputVideosTxt);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug()<<"cannot open file "<<inputVideosTxt;
        exit(-1);
    } else {
        QTextStream in(&file);
        filedata=in.readAll();
        file.close();
    }
    QStringList inputVideoList=filedata.split("\n");
    videosList=inputVideoList;

    QString text=ui->lineEdit_outputPath->text();
    if(text.isEmpty()){
        ui->lineEdit_outputPath->setText(defaultOutputPath);
    }
    else{
        settings.setValue("defaultOutputPath",ui->lineEdit_outputPath->text());
        settings.sync();
    }
}

QString MainWindow::absoluteFilePath(QString currentPathOrFile, QString fileName)
{
    QFileInfo pinfo(currentPathOrFile);
    QString currentPath=pinfo.absolutePath();


    QFileInfo info(fileName);
    if(info.isAbsolute()){
        return fileName;
    }
    else{
        if(info.isDir()){
            QDir dir(currentPath);
            dir.cd(fileName);
            return dir.absolutePath();
        }
        else{
            QDir dir(currentPath);
            return dir.absoluteFilePath(fileName);
        }
    }
}

QString MainWindow::absolutePath(QString currentPathOrFile, QString path)
{
    QFileInfo pinfo(currentPathOrFile);
    QString currentPath=pinfo.absolutePath();

    QFileInfo info(path);
    if(info.isAbsolute()){
        return path;
    }
    else{
        QDir dir(currentPath);
        dir.cd(path);
        return dir.absolutePath();
    }
}

void MainWindow::on_pushButton_tracking_clicked()
{
    QString bgsType=ui->comboBox_bgsType->currentText();
    QString trackType=ui->comboBox_trackingType->currentText();
    QString configFile=ui->lineEdit_inputPath->text()+"/yzbx.ini";

    trackingFactory_yzbx trackerFac;
    Tracking_yzbx *tracker=trackerFac.getTrackingAlgorithm(trackType);

    int n=videosList.length();
    QSettings sett(configFile,QSettings::IniFormat);
    QString outputPath=sett.value("defaultOutputPath").toString();
    QFileInfo info;
    for(int i=0;i<n;i++){
        QString videoFile=videosList.at(i);
        info.setFile(videoFile);
        if(!info.isAbsolute()){
            QDir dir(baseVideoPath);
            videoFile=dir.absoluteFilePath(videoFile);
            info.setFile(videoFile);
        }

        QString sqlFile=info.baseName()+".sqlite";
        sqlFile=outputPath+"/"+sqlFile;

        //video not exist;
        if(!info.exists()){
            qDebug()<<videoFile<<" donot exist!";
            continue;
        }

        info.setFile(sqlFile);
        if(info.exists()){
            qDebug()<<sqlFile<<" already exist!";
            continue;
        }

        tracker->process(configFile,videoFile,bgsType);
    }
}
