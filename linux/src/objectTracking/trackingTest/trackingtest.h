#ifndef TRACKINGTEST_H
#define TRACKINGTEST_H

#include <QMainWindow>
#include "ApplicationContext.h"
#include "BlobTrackerAlgorithmParams.h"
#include "DrawableTimer.h"
#include "InputVideoFileModule.h"
#include "Logger.h"
#include "MatrixIO.h"
#include "OpenCVHelpers.h"
#include "StringHelpers.h"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <deque>
#include <iostream>
#include <list>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "TrackerPersistance.h"
#include "InputFrameListModule.h"
#include "Tracker.h"
#include "BlobDetector.h"
#include "IBGS.h"
//#include "PlaybackBGS.h"
#include <boost/exception/all.hpp>

using namespace cv;
namespace po = boost::program_options;

bool IsParameterAvailable(const std::string& parameterAvailable, po::variables_map& vm);
po::options_description getCMDLineOptions();
po::options_description getGeneralConfigFileOptions();
std::string getParameterDescription(po::options_description& options, const po::variables_map& vm);
std::string getPath(const std::string& fullPath);
void DisplayInstruction();
void LoadConfigFileinStore(po::options_description& fileConfig, po::variables_map& vm, const std::string& configFileName);
void UpdateContextFromKeys(ApplicationContext* appContext, int key);

po::variables_map LoadConfigurationSettings(int argNumber, char* argsString[], po::options_description& fileConfig);

BlobTrackerAlgorithmParams LoadTrackerParams(po::variables_map& vm);


namespace Ui {
class TrackingTest;
}

class TrackingTest : public QMainWindow
{
    Q_OBJECT

public:
    explicit TrackingTest(QWidget *parent = 0);
    ~TrackingTest();

private slots:
    void on_pushButton_tracking_clicked();

private:
    Ui::TrackingTest *ui;
};

#endif // TRACKINGTEST_H
