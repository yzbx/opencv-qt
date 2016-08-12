#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QFileDialog>
#include "../lib/bgsfactory_yzbx.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_inputPath_clicked();

    void on_pushButton_outputPath_clicked();

    void on_pushButton_bgs_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
