#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_videotest.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    videotestui=new Ui::Form;
    form=new QWidget;
    videotestui->setupUi(form);
    form->show();
//    videotestui->show();
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString getPath(QString fullfileName){
//    int a=fullfileName.lastIndexOf('\\');
//    if(a<0){
//        int b=fullfileName.lastIndexOf('/');
//        qDebug()<<"b= "<<b;
//    }
//                                   else{
//        qDebug()<<"a= "<<a;
//        return fullfileName.lastIndexOf('/');
//                                   }
    QString path=fullfileName.section('\\',0,-2);
    if(path.isEmpty()){
        return fullfileName.section('/',0,-2);
    }

    return path;
}

void MainWindow::init()
{
    QSettings *configIniRead = new QSettings("config.ini", QSettings::IniFormat);
    //将读取到的ini文件保存在QString中，先取值，然后通过toString()函数转换成QString类型
    QString ipResult = configIniRead->value("/ip/second").toString();
    QString portResult = configIniRead->value("/port/open").toString();
    //打印得到的结果
    qDebug() << ipResult;
    qDebug() << portResult;
    //读入入完成后删除指针
    ui->lineEdit->setText(configIniRead->value("/bgs/bgs_path").toString());
    ui->lineEdit_2->setText(configIniRead->value("/rename/src_dir").toString());
    ui->lineEdit_3->setText(configIniRead->value("/tracking/track_param").toString());
    ui->lineEdit_4->setText(configIniRead->value("/tracking/track_path").toString());
//    track_param=;
    delete configIniRead;
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

void MainWindow::on_pushButton_5_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open bgs exe"), "",
                                                    tr("bgs (*.exe)"));
    if(!fileName.isEmpty()){
        ui->lineEdit->setText(fileName);
    }
}

void MainWindow::on_pushButton_4_clicked()
{
    QString fileName = QFileDialog::getExistingDirectory(this, tr("Open Input Directory"),
                                                         "/home",
                                                         QFileDialog::ShowDirsOnly
                                                         | QFileDialog::DontResolveSymlinks);
    if(!fileName.isEmpty()){
        ui->lineEdit_2->setText(fileName);
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("main.cfg"), "",
                                                    tr("main.cfg (*.cfg)"));
    if(!fileName.isEmpty()){
        ui->lineEdit_3->setText(fileName);
    }
}

void MainWindow::on_pushButton_6_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("tracking exe"), "",
                                                    tr("tracking (*.exe)"));
    if(!fileName.isEmpty()){
        ui->lineEdit_4->setText(fileName);
    }
}

void MainWindow::on_pushButton_clicked()
{
    QString exe=ui->lineEdit->text();
//    QProcess::process;
    qDebug()<<exe;

    QString dir=getPath(exe);

    qDebug()<<dir;
    qDebug()<<QDir::currentPath();
    QDir::setCurrent(dir);
    qDebug()<<QDir::currentPath();

//    process.start("firefox www.baidu.com");
    QProcess::startDetached(exe);
}

void MainWindow::on_pushButton_2_clicked()
{
    QDir dir(ui->lineEdit_2->text());
    if(!dir.exists())
    {
        QMessageBox::warning(this,tr("dir not exist"),tr("%1 not exist").arg(ui->lineEdit_2->text()));
    }

//    dir.setNameFilters(filter);
    QStringList info=dir.entryList();
    info.removeAll(".");
    info.removeAll("..");

    int len=info.length();
    qDebug()<<"len is "<<len;
    QString root=ui->lineEdit_2->text()+"/";
//    for(int i=0;i<len;i++)
//    {
//        QString oldfilename=info.at(i);
//        if(oldfilename.startsWith('0')){
//            QFile oldfile(root+oldfilename);
//            if(oldfile.exists()){
//                if(oldfile.remove())
//                {
//                    qDebug()<<"remove "<<oldfilename<<" succeed!!!";
//                }
//            }
//        }
//    }

    for(int i=0;i<len;i++){
        QString oldfilename=info.at(i);
        if(oldfilename.startsWith('0')){
            continue;
        }
        bool ok=false;
        qDebug()<<"i is "<<i;
        QString numstr=oldfilename;
        numstr.replace(".png","");
        int num=numstr.toInt(&ok,10);
        if(!ok){
            qDebug()<<numstr<<" convert to int failed!";
            break;
        }
        QFile oldfile(root+oldfilename);

        char cstr[10];
        sprintf(cstr,"%08d",num);
        QString newfilename=QString(cstr)+".png";

//        QFile newfile(newfilename);
//        newfile.remove();
        if(oldfile.exists()){
            if(oldfile.rename(root+newfilename))
            {
                qDebug()<<oldfilename<<" rename to "<<newfilename;
            }
            else    {
                qDebug()<<oldfilename<<" rename to "<<newfilename <<" failed !!!";
    //            qDebug()<<"rename file failed!";
            }
        }
    }

}

void MainWindow::on_pushButton_7_clicked()
{
    QString strfile;
    strfile = QCoreApplication::applicationDirPath();
    qDebug()<<strfile;
//    bool QDir::setCurrent ( const QString & path )

    QString exe=ui->lineEdit_4->text();
    QString param=ui->lineEdit_3->text();


//    QString dir=exe;
//    dir.replace("TrackingApp.exe","");
    QString dir=getPath(exe);

    qDebug()<<QDir::currentPath();
    QDir::setCurrent(dir);
    qDebug()<<QDir::currentPath();

//    QProcess::process;
    qDebug()<<exe<<" "<<param;
//    process.start("firefox www.baidu.com");
    QProcess::startDetached(exe,QStringList()<<param);
}

void addBook(QSqlQuery &q, const QString &title, int year, const QVariant &authorId,
             const QVariant &genreId, int rating)
{
    q.addBindValue(title);
    q.addBindValue(year);
    q.addBindValue(authorId);
    q.addBindValue(genreId);
    q.addBindValue(rating);
    q.exec();
}

QVariant addGenre(QSqlQuery &q, const QString &name)
{
    q.addBindValue(name);
    q.exec();
    return q.lastInsertId();
}

QVariant addAuthor(QSqlQuery &q, const QString &name, const QDate &birthdate)
{
    q.addBindValue(name);
    q.addBindValue(birthdate);
    q.exec();
    return q.lastInsertId();
}


void MainWindow::on_actionTestsplite_triggered()
{
    QString cfg=ui->lineEdit_3->text();
    QString dir=getPath(cfg);

    qDebug()<<QDir::currentPath();
    QDir::setCurrent(dir);
    qDebug()<<QDir::currentPath();
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("test.sqlite");

    if (!db.open())
        qDebug()<<db.lastError();

    QStringList tables = db.tables();
    if (tables.contains("bounding_boxes", Qt::CaseInsensitive)){

    QSqlQuery query("SELECT * FROM bounding_boxes");
//       int fieldNo = query.record().indexOf("country");
    QSqlRecord rec = query.record();

    qDebug() << "Number of columns: " << rec.count();

//    int nameCol = rec.indexOf("name"); // index of the field "name"

       while (query.next()) {
           QString str="";
           for(int i=0;i<rec.count();i++){
               str+= " "+query.value(i).toString();
           }
           qDebug()<<str<<" ";

       }
    }
    else{
        qDebug()<< QSqlError();
    }
//    return QSqlError();
}

QImage* opencv2qt(Mat image)
{
    //return QImage will get empty, so return QImage *
    QImage *img;
    if(image.channels()==3)
    {
        //cvt Mat BGR 2 QImage RGB
        Mat rgb;
        cvtColor(image,rgb,CV_BGR2RGB);
        img =new QImage((const unsigned char*)(rgb.data),
                    rgb.cols,rgb.rows,
                    rgb.cols*rgb.channels(),
                    QImage::Format_RGB888);
    }
    else
    {
        img =new QImage((const unsigned char*)(image.data),
                    image.cols,image.rows,
                    image.cols*image.channels(),
                    QImage::Format_RGB888);
    }
    return img;
}
void MainWindow::paintEvent(QPaintEvent *e)
{
    //显示方法一
    QPainter painter(this);
//    QImage image1 = QImage((uchar*)(srcImage.data), srcImage.cols, srcImage.rows, QImage::Format_RGB888);
//    painter.drawImage(QPoint(20,20), image1);
    //显示方法二
//    QImage image2 = QImage((uchar*)(srcImage.data), srcImage.cols, srcImage.rows, QImage::Format_RGB888);
//    imageLabel->setPixmap(QPixmap::fromImage(image2));
//    imageLabel->resize(image2.size());
//    imageLabel->show();
}

void MainWindow::on_actionTestvideo_triggered()
{
    string src="D:\\Program\\matlab\\bgslibrary_mfc\\dataset\\video.avi";
    VideoCapture capture;
    capture.open(src);
    int num=0;
    cv::namedWindow("video");
    while(1){
        num=num+1;
        if(!capture.grab()){
            qDebug()<<"grab failed!";
            break;
        }
//        Mat frame;
        capture>>frame;
        if(frame.empty())
        {
            qDebug()<<"frame is empty";
            break;
        }
        if((frame.size[0]<=0)|(frame.size[1]<=0)){
            qDebug()<<"frame's col or row <=0";
            break;
        }
        qDebug()<<"num is "<<num;
        QImage img;

        //cvt Mat BGR 2 QImage RGB
        Mat rgb;
        cvtColor(frame,rgb,CV_BGR2RGB);
        img =QImage((const unsigned char*)(rgb.data),
                    rgb.cols,rgb.rows,
                    rgb.cols*rgb.channels(),
                    QImage::Format_RGB888);

        videotestui->label->setPixmap(QPixmap::fromImage(img));
        videotestui->label->resize(videotestui->label->pixmap()->size());
//        imshow("read frame",frame);
        form->update();
//        videotestui->label->update();
//        form->show();
//        videotestui->label->show();
        cvtColor(frame,srcImage,CV_BGR2RGB);
        ui->label_5->setPixmap(QPixmap::fromImage(img));
//        ui->label_5->resize(ui->label_5->pixmap()->size());
//        qDebug()<<"size "<<ui->label_5->pixmap()->size();
        ui->label_5->resize(ui->label_5->pixmap()->size());
        this->update();
        waitKey(30);
        QThread::msleep(30);
//        this->thread()->sleep(3);
//        if(frame.empty())
//        {
//            qDebug()<<"frame is empty";
//            break;
//        }
    }
}

void MainWindow::on_actionSaveFrame_triggered()
{
    vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);    //png格式下，默认的参数为3.

    if(imwrite("D:\\Program\\matlab\\bgslibrary_mfc\\dataset\\keyFrame\\alpha.png", frame, compression_params)){
            qDebug()<<"save ok!";
    }
    else{
//        fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
        qDebug()<<"Exception converting image to PNG format: ";
    }
//    fprintf(stdout, "Saved PNG file with alpha data.\n");

}

void MainWindow::on_actionSaveKeyFrame_triggered()
{
//    QString cfg=ui->lineEdit_3->text();
//    QString dir=getPath(cfg);

//    qDebug()<<QDir::currentPath();
//    QDir::setCurrent(dir);
//    qDebug()<<QDir::currentPath();
//    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
//    db.setDatabaseName("test.sqlite");

//    if (!db.open())
//        qDebug()<<db.lastError();

//    QStringList tables = db.tables();
//    if (tables.contains("bounding_boxes", Qt::CaseInsensitive)){

//    QSqlQuery query("SELECT * FROM bounding_boxes");
////       int fieldNo = query.record().indexOf("country");
//    QSqlRecord rec = query.record();

//    qDebug() << "Number of columns: " << rec.count();

////    int nameCol = rec.indexOf("name"); // index of the field "name"

//       while (query.next()) {
//           QString str="";
//           for(int i=0;i<rec.count();i++){
//               str+= " "+query.value(i).toString();
//           }
//           qDebug()<<str<<" ";

//       }
//    }
//    else{
//        qDebug()<< QSqlError();
//    }
    qDebug()<<"use matlab will be better to draw trajectory";
}

void MainWindow::on_actionTestvideoinfo_triggered()
{
    string src="D:\\Program\\matlab\\bgslibrary_mfc\\dataset\\video.avi";
    VideoCapture capture;
    capture.open(src);

    int frameCount=capture.get(CV_CAP_PROP_FRAME_COUNT);
    qDebug()<<"frame count "<<frameCount;
}
