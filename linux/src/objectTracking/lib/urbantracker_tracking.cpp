#include "urbantracker_tracking.h"

InputFrameProviderIface* LoadVideo(po::variables_map& vm, bool& overrideTotalNbFrame, unsigned int& lastFrame)
{
    InputFrameProviderIface* vfm = nullptr;
    if(IsParameterAvailable("video-filename",vm))
        vfm = new InputVideoFileModule(vm["video-filename"].as<std::string>());
    else if(IsParameterAvailable("framelist-filename",vm))
    {
        std::string frameList = vm["framelist-filename"].as<std::string>();
        int posLastChar = max((int)frameList.find_last_of('\\'), (int)frameList.find_last_of('/'));
        std::string basePath = frameList.substr(0, posLastChar);
        vfm = new InputFrameListModule(basePath, frameList);
    }


    bool success = false;

    overrideTotalNbFrame = false;
    lastFrame = -1;

    if(vfm->isOpen())
    {
        if(vm.count("total-number-frame") > 0)
        {
            vfm->setNumberFrame(vm["total-number-frame"].as<unsigned int>());
            overrideTotalNbFrame = true;
        }
        if(vm.count("seek-frame")>0)
        {
            success = vfm->seekAtFramePos(vm["seek-frame"].as<unsigned int>());
        }
        else if(vm.count("seek-ms")>0)
        {
            success = vfm->seekAtMs(vm["seek-ms"].as<unsigned int>());
            if(!success)
                LOGWARNING("Could not seek to position");
        }
        else if((vm.count("video-start-time") > 0) && (vm.count("video-seek-time") > 0))
        {
            success = vfm->seekAtMs(vm["video-start-time"].as<std::string>(), vm["video-seek-time"].as<std::string>());
            if(!success)
                LOGWARNING("Could not seek to position");
        }

        if(vm.count("video-end-frame") > 0)
        {
            lastFrame = vm["video-end-frame"].as<unsigned int>();
        }
    }
    else
    {
        LOGERROR("Can't open video: " << vm["video-filename"].as<std::string>());
    }

    return vfm;
}

DrawingFlags LoadDrawingFlags(po::variables_map& vm)
{
    DrawingFlags drawFlags;
    drawFlags.mDrawBoundingBox = vm["draw-bounding-box"].as<bool>();
    drawFlags.mDrawCentroid = vm["draw-centroid"].as<bool>();
    drawFlags.mDrawInactiveClusters = vm["draw-inactive-cluster"].as<bool>();
    drawFlags.mDrawFPS = vm["draw-fps"].as<bool>();
    drawFlags.mDrawNbClustersTraj = vm["draw-nbobject-nbtrajectories"].as<bool>();
    drawFlags.mDrawTrajectories = vm["draw-trajectories"].as<bool>();
    drawFlags.mDrawTrackerStats = vm["draw-tracker-stats"].as<bool>();
    drawFlags.mDrawPerspective = vm["draw-perspective"].as<bool>();
    drawFlags.mDrawBackgroundSubstraction = vm["draw-bgs"].as<bool>();
    drawFlags.mDrawKeypoints = vm["draw-keypoints"].as<bool>();
    drawFlags.mDrawMatches = vm["draw-matches"].as<bool>();
    return drawFlags;
}

BlobTrackerAlgorithmParams LoadTrackerParams(po::variables_map& vm)
{



    return BlobTrackerAlgorithmParams(vm["minimum-match-between-blob"].as<int>(),
                                      vm["bgs-minimum-blob-size"].as<int>(),
                                      vm["max-lost-frame"].as<int>(),
                                      vm["brisk-threshold"].as<int>(),
                                      vm["brisk-octave"].as<int>(),
                                      vm["match-ratio"].as<double>(),
                                      vm["max-seg-dist"].as<double>(),
                                      vm["max-hypothesis"].as<int>(),
                                      vm["verify-reentering-object"].as<bool>(),
                                      vm["urban-isolated-shadow-removal"].as<bool>(),
                                      vm["bgs-remove-ghost"].as<bool>(),
                                      vm["bgs-cam-noise"].as<int>(),
                                      vm["bgs-moving-pixel-ratio"].as<double>());
}


int mainTest(int argNumber,char* argString[])
{
    //Init Logger
    Logger::getInstance()->setLoggingLevel(eINFO);

    //We create a folder for background substraction configuration file. BGSLib will crash if that folder doesn't exist.
    boost::filesystem::create_directory("config");

    //We load the tracking configuration
    po::options_description fileConfig("Configuration file options");
    po::variables_map vm = LoadConfigurationSettings(argNumber, argString, fileConfig);
    if(vm.count("logging-level") >0)
    {
        unsigned int level = vm["logging-level"].as<unsigned int>();
        if(level >= 0 && level <= eNONE)
            Logger::getInstance()->setLoggingLevel(ELoggingLevel(level));
    }


    //We load the video
    bool overrideTotalNbFrame = false;
    unsigned int lastFrame = -1;
    InputFrameProviderIface* vfm = LoadVideo(vm, overrideTotalNbFrame, lastFrame);
    try
    {
        if(vfm && vfm->isOpen())
        {
            LOGINFO("Starting tracker");

            bool success = true;
            DrawableTimer dt;


            float pixelByMeter = 1;
            if(vm.count("scaleratio-filename") > 0 && vm["scaleratio-filename"].as<std::string>() != "none" && vm["scaleratio-filename"].as<std::string>() != "None")
            {
                success = Utils::IO::LoadScalar<float>(vm["scaleratio-filename"].as<std::string>(), pixelByMeter);
                if(!success)
                {
                    pixelByMeter = 1;
                    LOGWARNING("Can't load scale ratio file. Using 1px/m ratio");
                }
            }

            cv::Mat homography = cv::Mat::eye(3,3, CV_32FC1);
            if(vm.count("homography-filename") > 0 && vm["homography-filename"].as<std::string>() != "none" && vm["homography-filename"].as<std::string>() != "None")
            {
                success = Utils::IO::LoadMatrix<double>(vm["homography-filename"].as<std::string>(), homography);
                if(!success)
                {
                    homography = cv::Mat::eye(3,3, CV_32FC1);
                    LOGWARNING("Can't load homography file. Using identity.");
                }
            }
            cv::Mat mask;

            if(vm.count("mask-filename") > 0 && vm["mask-filename"].as<std::string>() != "none" && vm["mask-filename"].as<std::string>() != "None")
            {
                mask = cv::imread(vm["mask-filename"].as<std::string>());
                cv::cvtColor(mask, mask, CV_BGR2GRAY);
            }
            else
            {
                mask = cv::Mat(vfm->getHeight(), vfm->getWidth(), CV_8UC1);
                mask.setTo(cv::Scalar(255,255,255));
            }


            /*if(vm.count("aerialview-filename") > 0)
            {
                std::string aerialfn(vm["aerialview-filename"].as<std::string>());
                const cv::Mat aerialView = cv::imread(aerialfn);
            }*/


            DrawingFlags drawFlags = LoadDrawingFlags(vm);
            BlobTrackerAlgorithmParams algoParams = LoadTrackerParams(vm);
            DisplayInstruction();

            ApplicationContext context(mask, homography,(float)vfm->getNbFPS(), pixelByMeter, drawFlags, algoParams, vm["record-bgs"].as<bool>(), vfm);
            TrackerPersistance op(&context, vm["object-sqlite-filename"].as<std::string>());
            op.init();
            std::string bgsRecord = vm["bgs-filepath"].as<std::string>();
            context.setBGSPath(bgsRecord);
            std::string bgsType = vm["bgs-type"].as<std::string>();

            Tracker t(&context, bgsType);
            if(bgsType == "PlaybackBGS" && vm.count("bgs-filepath") > 0)
            {
                IBGS* bgs = t.getBlobDetector()->getBGS();
                PlaybackBGS* pbBGS = dynamic_cast<PlaybackBGS*>(bgs);
                if(pbBGS)
                    pbBGS->setPath(bgsRecord);
//                std::cout<<"remove playBackBgs\n";
//                exit(-1);
            }

            //bgs-filepath
            cv::Mat m;
            bool stepByStep = false;
            bool quit = false;
            bool displayMemory = vm["draw-memory"].as<bool>();


            if(!vfm->isOpen())
            {
                LOGERROR("Can't open video file");
                return 0;
            }
            int nbFrameProcess = 0;
            while(vfm->getNextFrame(m) && !quit && (!overrideTotalNbFrame || vfm->getNextFramePos() <= vfm->getNbFrame()) &&  (vfm->getNextFramePos() <= lastFrame || lastFrame == -1))
            {
                context.setCurrentFrame(m);
                dt.start();
                t.processFrame(m);
                dt.stop();
                if(context.getRefDrawingFlags()->mDrawFPS)
                    dt.drawFPS(m);

                cv::Mat trajectoryPictureClone = m.clone();
                if(drawFlags.mDrawPerspective)
                {
                    t.draw(m);
                    #ifdef _WIN32
                    if(displayMemory)
                    {
                        PROCESS_MEMORY_COUNTERS memCounter;
                        bool result = GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof( memCounter ));
                        float nbMeg = memCounter.WorkingSetSize/(1048576.f);
                        cv::putText(m, Utils::String::toString(nbMeg)+ " Mo", cv::Point(0,m.rows-5), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0,255,255),2);
                    }
                    #endif
                    cv::imshow("Video", m);
                    int key;
                    if(stepByStep)
                    {
                        key = cv::waitKey();
                    }
                    else
                        key = cv::waitKey(1);

                    if(key == 's')
                    {
                        stepByStep = !stepByStep;
                    }
                    else if(key == 'm')
                    {
                        displayMemory = !displayMemory;
                    }
                    else if(key == 'q')
                    {
                        std::cout << "Quit was requested. Exiting...";
                        quit = true;
                    }
                    else
                        UpdateContextFromKeys(&context, key);
                }
                ++nbFrameProcess;
                if(nbFrameProcess %50 == 0)
                    std::cout << "Frame " << nbFrameProcess << std::endl;
            }
            delete vfm;
        }
        else
        {
            LOGWARNING("Config file not loaded");
            LOGINFO("Loaded options:\n" << getParameterDescription(fileConfig, vm));

            if (vm.count("help"))
            {
                std::cout << fileConfig << std::endl;
            }
        }
    }
    catch(std::exception& e)
    {
        LOGASSERT(false, "Unhandled exception: " << e.what());
    }



    Logger::getInstance()->cleanUp();
    return 0;
}



void LoadConfigFileinStore(po::options_description& fileConfig, po::variables_map& vm, const std::string& configFileName)
{
    if(!configFileName.empty())
    {
        try
        {
            std::ifstream configurationFile(configFileName.c_str());
            if(configurationFile.is_open())
            {
                store(po::parse_config_file(configurationFile, fileConfig), vm);
                notify(vm);
                configurationFile.close();
                LOGINFO("Config file loaded");
            }
            else
            {
                LOGERROR("Impossible to load file " << configFileName);
            }
        }
        catch(std::exception e)
        {
            LOGERROR(e.what());
        }
    }
}


po::variables_map LoadConfigurationSettings(int argNumber, char* argsString[], po::options_description& fileConfig)
{
    po::positional_options_description p;
    p.add("config-file", 1);
    po::options_description general = getGeneralConfigFileOptions();
    po::options_description cmdline= getCMDLineOptions();
    fileConfig.add(general).add(BlobTrackerAlgorithmParams::getTrackerOptions());

    po::variables_map vm;

    store(po::command_line_parser(argNumber, argsString).options(cmdline).positional(p).allow_unregistered().run(), vm);
    notify(vm);
    std::string configFileName = vm["config-file"].as<std::string>();
    std::string path = getPath(configFileName);
    LoadConfigFileinStore(fileConfig, vm, configFileName);
    LoadConfigFileinStore(fileConfig, vm, path + vm["filepath-filename"].as<std::string>());
    LoadConfigFileinStore(fileConfig, vm, path + vm["tracker-filename"].as<std::string>());

    return vm;
}

std::string getPath(const std::string& fullPath)
{
    std::string path = "";
    unsigned int idx = fullPath.find_last_of("\\/");
    if(idx != std::string::npos)
    {
        path = fullPath.substr(0, idx+1);
    }
    return path;
}

po::options_description getGeneralConfigFileOptions()
{
    po::options_description general("General options");
    general.add_options()
        ("video-filename,v", po::value<std::string>(), "Path to the video file to process")
        ("framelist-filename", po::value<std::string>(), "Path to a text file with a framelist to process")
        ("homography-filename,h", po::value<std::string>()->default_value("\"\""), "Path to the homography matrix")
        ("scaleratio-filename", po::value<std::string>(), "Path to the pixel/meter ratio file")
        ("aerialview-filename,a", po::value<std::string>()->default_value("\"\""), "Path to the aerial view screenshot")
        ("mask-filename,m", po::value<std::string>()->default_value("\"\""), "Path to the mask image (where features are detected)")
        ("display-trajectories,d", po::value<bool>()->default_value(true), "Display trajectories on the video")
        ("draw-aerial", po::value<bool>()->default_value(true), "show the aerial view")
        ("draw-bgs", po::value<bool>()->default_value(true), "show the background substraction")
        ("draw-perspective", po::value<bool>()->default_value(true), "show the perspective view")
        ("draw-bounding-box", po::value<bool>()->default_value(false), "Draw the bounding box of all the points at the current time of the objects")
        ("draw-trajectories", po::value<bool>()->default_value(true), "Draw the trajectories")
        ("draw-centroid", po::value<bool>()->default_value(false), "Draw cluster centroid")
        ("draw-inactive-cluster", po::value<bool>()->default_value(false), "Draw inactive cluster")
        ("draw-tracker-stats", po::value<bool>()->default_value(false), "Draw tracker stats")
        ("draw-fps", po::value<bool>()->default_value(true), "Display FPS")
        ("draw-keypoints", po::value<bool>()->default_value(false), "Display Keypoints")
        ("draw-matches", po::value<bool>()->default_value(false), "Display matches")
        ("draw-memory", po::value<bool>()->default_value(false), "Draw memory consumption")
        ("draw-nbobject-nbtrajectories", po::value<bool>()->default_value(false), "Display number of object and trajectories")
        ("seek-ms", po::value<unsigned int>(), "The number of ms to seek to...")
        ("seek-frame", po::value<unsigned int>(), "The number of frame to seek to...")
        ("video-start-time", po::value<std::string>(), "The start time displayed by the video in the format HH:MM::SS")
        ("video-end-frame", po::value<unsigned int>(), "The last frame we want to process")
        ("video-seek-time", po::value<std::string>(), "The position you want to seek to in the video in the format HH:MM::SS")
        ("bgs-type", po::value<std::string>()->default_value("LBMixtureOfGaussians"), "Name of the background substraction technic to use.")
        ("record-bgs", po::value<bool>()->default_value(false), "If we should record the BGS for further playback or not")
        ("tracker-filename",  po::value<std::string>(), "Name of the file that contains the tracker specific information")
        ("filepath-filename",  po::value<std::string>(), "Name of the file that contains the filepath specific information")
        ("bgs-filepath",  po::value<std::string>()->default_value(""), "Path to the directory containing the background substraction. To be used with PlaybackBGS")
        ("logging-level", po::value<unsigned int>(), "The current logging level (0 debug, 1 info, 2 warning, 3 error, 4 none)")


        ("total-number-frame", po::value<unsigned int>(), "Total number of frame in file")
        ("object-sqlite-filename",  po::value<std::string>(), "Name of the file that contains the object database")
        ("run-time-verification", po::value<bool>()->default_value(false), "The system does runtime verification to verify of the integrity of the structure. This is for debugging only as it takes a lot of CPU");

    return general;
}

po::options_description getCMDLineOptions()
{
    po::options_description onlyCmdLine("Program options");
    onlyCmdLine.add_options()
        ("help,h", "Shows available options")
        ("config-file,c", po::value<std::string>()->default_value("M:\\code\\PolyMaitrise\\trunk\\recherche\\urbantracker\\code\\config\\tracking.cfg"), "configuration file")
        ;
    return onlyCmdLine;
}

bool IsParameterAvailable(const std::string& parameterAvailable, po::variables_map& vm)
{
    bool success = false;
    if(vm.count(parameterAvailable) >0)
        success = true;
    else
        LOGWARNING(parameterAvailable << " is not available");
    return success;
}

void UpdateContextFromKeys(ApplicationContext* appContext, int key)
{
    switch (key)
    {
    case '1':
        appContext->getRefDrawingFlags()->mDrawBoundingBox = !appContext->getRefDrawingFlags()->mDrawBoundingBox;
        break;
    case '2':
        appContext->getRefDrawingFlags()->mDrawTrackerStats = !appContext->getRefDrawingFlags()->mDrawTrackerStats;
        break;
    case '3':
        appContext->getRefDrawingFlags()->mDrawCentroid = !appContext->getRefDrawingFlags()->mDrawCentroid;
        break;
    case '4':
        appContext->getRefDrawingFlags()->mDrawInactiveClusters = !appContext->getRefDrawingFlags()->mDrawInactiveClusters;
        break;
    case '5':
        appContext->getRefDrawingFlags()->mDrawFPS = !appContext->getRefDrawingFlags()->mDrawFPS;
        break;
    case '6':
        appContext->getRefDrawingFlags()->mDrawBackgroundSubstraction = !appContext->getRefDrawingFlags()->mDrawBackgroundSubstraction;
        break;
    case '7':
        appContext->getRefDrawingFlags()->mDrawTrajectories = !appContext->getRefDrawingFlags()->mDrawTrajectories;
        break;
    case '8':
        appContext->getRefDrawingFlags()->mDrawKeypoints = !appContext->getRefDrawingFlags()->mDrawKeypoints;
        break;
    case '9':
        appContext->getRefDrawingFlags()->mDrawMatches = !appContext->getRefDrawingFlags()->mDrawMatches;
        break;
    }
}

void DisplayInstruction()
{
    std::cout <<"Instructions:\n"
        <<"1  Draw bounding box\n"
        <<"2  Draw tracker stats\n"
        <<"3  Draw current trajectory centroid\n"
        <<"4  Draw inactive clusters\n"
        <<"5  Draw FPS\n"
        <<"6  Draw background substraction\n"
        <<"7  Draw trajectories\n"
        <<"8  Draw keypoints\n"
        <<"9  Draw matches\n"
        <<"m  Display memory consumption\n"
        <<"s  Step-by-step\n"
        <<"q  Quit application\n";
}


string getParameterDescription(po::options_description& options, const po::variables_map& vm)  {
    std::stringstream stream;
    std::vector<boost::shared_ptr<po::option_description> > optionsVec = options.options();
    for (unsigned int i=0; i<optionsVec.size(); ++i) {
        boost::any value = vm[optionsVec[i]->long_name()].value();
        stream << optionsVec[i]->long_name() << " = ";
        if (value.type() == typeid(bool))
            stream << boost::any_cast<bool>(value) << " (bool)" <<std::endl;
        else if (value.type() == typeid(int))
            stream << boost::any_cast<int>(value) << " (int)" << std::endl;
        else if (value.type() == typeid(unsigned int))
            stream << boost::any_cast<unsigned int>(value) << " (unsigned int)" << std::endl;
        else if (value.type() == typeid(float))
            stream << boost::any_cast<float>(value) << " (float)" << std::endl;
        else if (value.type() == typeid(std::string))
            stream << boost::any_cast<std::string>(value) << " (std::string)" << std::endl;
        else
            std::cerr << "the type of the option " << optionsVec[i]->long_name() << " (" << i << ") is not int, float or string." << std::endl;
    }

    return stream.str();
}

urbanTracker_tracking::urbanTracker_tracking()
{

}

void urbanTracker_tracking::process(QString configFile,QString videoFile, QString bgsType)
{
    qDebug()<<"configFile="<<configFile;
    qDebug()<<videoFile;
    qDebug()<<bgsType;

    QFileInfo info(videoFile);
    if(!info.exists()||!info.isFile()){
        qDebug()<<"empty file or not file "<<videoFile;
        exit(-1);
    }
    QString sqlFile=info.baseName()+".sqlite";

    info.setFile(configFile);
    if(!info.exists()||!info.isFile()){
        qDebug()<<"empty file or not file "<<configFile;
        exit(-1);
    }


    QSettings *sett=new QSettings(configFile,QSettings::IniFormat);
    QString maincfg=sett->value("UrbanTracker/configFilePath").toString();
    maincfg=getAbsFilePath(configFile,maincfg);
    QString outputPath=sett->value("defaultOutputPath").toString();
    outputPath=getAbsFilePath(configFile,outputPath);
    sqlFile=getAbsFilePath(outputPath,sqlFile);


    info.setFile(maincfg);
    if(!info.exists()||!info.isFile()){
        qDebug()<<"empty file or not file "<<maincfg;
        exit(-1);
    }
    QSettings mainsett(maincfg,QSettings::IniFormat);
    qDebug()<<"maincfg keys: "<<mainsett.allKeys();
    QString filecfg=mainsett.value("filepath-filename").toString();
    QString algocfg=mainsett.value("tracker-filename").toString();
    filecfg=getAbsFilePath(maincfg,filecfg);
    algocfg=getAbsFilePath(maincfg,algocfg);

    QSettings filesett(filecfg,QSettings::IniFormat);
    QSettings algosett(algocfg,QSettings::IniFormat);

    filesett.setValue("video-filename",videoFile);
    filesett.setValue("mask-filename",QString("none"));
    filesett.setValue("object-sqlite-filename",sqlFile);
    algosett.setValue("bgs-type",bgsType);

    filesett.sync();
    algosett.sync();

    char *argString[]={"./TrackingTest","/home/yzbx/git/opencv-qt/linux/config/UrbanTracker/main.cfg"};
    char **p;
    p=argString;
    p[1]=(char *)maincfg.toStdString().c_str();
    mainTest(2,argString);
}

QString urbanTracker_tracking::getAbsFilePath(QString base, QString fileName)
{
    QFileInfo pinfo(base);
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
