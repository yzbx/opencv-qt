# opencv-qt
opencv + qt to do moving detection and tracking

# this is the the main branch, to view simple demo of opencv and qt, check  to branch demo

# UrbanTracker
to use UrbanTracker under ubuntu, you need instasll boost, and activate c++11

## build
sudo apt-get install libboost-all-dev
```
#list(APPEND CMAKE_CXX_FLAGS "-std=c++0x -g -ftest-coverage -fprofile-arcs -msse -msse2 -msse3 -march=native")
list(APPEND CMAKE_CXX_FLAGS "-std=c++11 -g -ftest-coverage -fprofile-arcs -msse -msse2 -msse3 -march=native")
```
cmake && make
## output
- TrackingApp/bin/TrackingApp
- QtGroundTruthBuilderApp/bin/QtGroundTruthBuilderApp
- MetricsEvaluationApp/bin/MetricsEvaluationApp

## usage
TrackingApp/bin/TrackingApp xxx/main.cfg

## bug
- error in parse xxx.cfg, add one line may crash.
- may not support for Chinese path, but support Chinese file.

---

# BGS
we need use BGS library under qmake, and to choose a algorithm from QComboBox. thus we define class bgsFactory
## install BGS library
- src/objectTracking/extern/bgslibrary
cd build && cmake .. && sudo make install
- default make install have bug
sudo cp -r package_bgs /usr/local/include/
- set bgslibrary for pkgconfig
sudo cp bgslibrary.pc /usr/local/lib/pkgconfig

## use BGS library
you need mkdir config

---
# convert from cmake to qmake
## UrbanTracker
1. IBGS.h have channged in UrbanTracker, we need modify, remove the use of getNbChannel() in file BlobDetector.cpp, and this file is encode with ISO-8859, not utf-8
```
//	if(mBackgroundSubstractor->getNbChannel() == 1 && mMask.channels() > 1)
//		cv::cvtColor(mMask, mMask, CV_BGR2GRAY);
//	else if(mBackgroundSubstractor->getNbChannel() == 3 && mMask.channels() == 1)
//		cv::cvtColor(mMask, mMask, CV_GRAY2BGR);
```
2. I offer bgsFactory_yzbx for use bgslibrary in QComboBox
3. modify .pro
```
CONFIG +=c++11
QMAKE_CXXFLAGS = "-g -ftest-coverage -fprofile-arcs -msse -msse2 -msse3 -march=native"
```
4. remove PlaybackBGS
5. update for bgslibray in UrbanTracker, such as LOBSTER

# reference
- [motion tracking](www.stats.ox.ac.uk/~wauthier/tracker)
