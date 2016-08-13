#ifndef TRACKING_YZBX_H
#define TRACKING_YZBX_H
#include <iostream>
#include <QtCore>
#include <package_bgs/IBGS.h>
#include "bgsfactory_yzbx.h"


class Tracking_yzbx
{
public:
  virtual void process(QString configFile,QString videoFile,QString bgsType="default") = 0;
  virtual ~Tracking_yzbx(){}
};
#endif // TRACKING_YZBX_H
