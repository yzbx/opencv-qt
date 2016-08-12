#ifndef BGSTEST_H
#define BGSTEST_H

#include <QMainWindow>
#include "../lib/bgsfactory_yzbx.h"
#include <QtCore>
#include <QFileDialog>

namespace Ui {
class BgsTest;
}

class BgsTest : public QMainWindow
{
    Q_OBJECT

public:
    explicit BgsTest(QWidget *parent = 0);
    ~BgsTest();

private slots:
    void on_pushButton_bgs_clicked();

    void on_pushButton_inputPath_clicked();

private:
    Ui::BgsTest *ui;
    IBGS *ibgs;
    QString inputPath;
    void initComboBox_bgsType();
};

#endif // BGSTEST_H
