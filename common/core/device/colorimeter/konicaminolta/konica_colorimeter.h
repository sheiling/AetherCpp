#ifndef _KONICA_COLORIMETER_HH
#define _KONICA_COLORIMETER_HH

#include "../colorimeter_base.h"
#include <atomic>
#include "CASDK2.h"

namespace common
{
namespace device
{
class COLORIMETER_EXPORT KonicaColorimeter : public ColorimeterBase {
public:
    KonicaColorimeter() {
	}
    ~KonicaColorimeter() = default;
public:
    //初始化设备
    virtual bool init() override;

    //连接设备
    virtual bool connect() override;

    //断开设备
    virtual bool disconnect() override;

    //采集数据
    virtual bool measureData(double& cx, double& cy, double& lum) override;

    // 连接状态
    virtual bool isConnected() override;

private:
    //默认设置
    int defaultSetting();

    //获取错误信息
    static int GetErrorMessage(int errornum);
private:
    bool isConnected_ = false;
    CASDK2Ca200* m_objCa200 = nullptr;
    CASDK2Ca* m_objca = nullptr;
    CASDK2Probe* m_objProbe = nullptr;
    CASDK2Memory* m_objMemory = nullptr;

    //测量参数
    int                             m_freqmode = 4;             //测量模式 : INT   0: NTSC  1: PAL  3 : UNIV  4 : INT  5 : MANUAL
    double                          m_freq = 60.0;              //频率Hz
    int                             m_speed = 1;                //测量速度 0-SLOW  1- FAST  2- LTD.AUTO 3- AUTO
    int                             m_Lvmode = 1;               //亮度段位 1-cd/m2   0-fL
    int                             m_displayMode = 0;          //显示模式（返回的数据类型（lxy？xyz？等））
    int                             m_channelNO = 1;            //设置采集通道

};

}

}

#endif
