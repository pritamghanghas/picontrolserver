#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

class CameraParameters
{
public:
    CameraParameters():
    targetip("192.168.1.1"),
    targetport("5001"),
    width("1280"),
      height("720"),
      bitrate("25000000"),
      timeout("0"),
      framerate("49"),
      mode("auto"),
      sharpness("80"),
      brightness("50"),
      saturation("50"),
//      ISO("")
      vstab("off"),
      ev("off"),
      exposure("auto"),
      awb("auto"),
      metering("average"),
      hflip("off"),
      vflip("off"),
      drc("med")
      {}

    QString targetip,
            targetport,
            width,
            height,
            bitrate,
            timeout,
            framerate,
            mode,
            sharpness,
            contrast,
            brightness,
            saturation,
//            ISO,
            vstab,
            ev,
            exposure,
            awb,
            metering,
            hflip,
            vflip,
//            shutter,
//            awbgains,
            drc;
};


#endif // CONSTANTS_H
