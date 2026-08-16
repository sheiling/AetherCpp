#include "konica_colorimeter.h"
#include <Logger.h>
#include <common/utility/string_tool/string_format.hpp>

namespace common
{
namespace device
{

bool KonicaColorimeter::init()
{    
    if (m_objCa200 != nullptr)
    {
        if (isConnected_)
        {
            m_objCa200->AutoDisconnect();
        }
        delete m_objCa200;
        m_objCa200 = nullptr;
    }
    m_objCa200 = new CASDK2Ca200();
    isConnected_ = false;

    return true;
}
bool KonicaColorimeter::connect()
{
    if (nullptr == m_objCa200)
    { 
        LOG_ERROR("[common][device][Colorimeter][KonicaColorimeter]--pointer is null!");
        return false;
    }

    if (!isConnected_)
    {
        int ret = 0;
        do
        {
              //自动连接到色度计(连接到所有)，处理使用自动连接外，还可以使用SetConfiguration接口连接（好像是串口才这么用）
            ret = m_objCa200->AutoConnect();
            if (ret != 0)
                break;
            //获取一个CA410连接对象
            ret = m_objCa200->get_SingleCa(&m_objca);
            if (ret != 0)
                break;
            //获取当前色度计内存
            ret = m_objca->get_Memory(&m_objMemory);
            if (ret != 0)
                break;
            //获取探针
            ret = m_objca->get_SingleProbe(&m_objProbe);
            if (ret != 0)
                break;

            ret = defaultSetting();
            if (ret != 0)
                break;

            isConnected_ = true;
        } while (0);
        if(ret != 0)
        {
            //输出错误信息
            GetErrorMessage(ret);
            return false;
        }
    }

    return isConnected_;
}
bool KonicaColorimeter::disconnect() 
{
    if (nullptr == m_objCa200)
    {
        return true;
    }
    if (isConnected_)
    {
        int ret = m_objCa200->AutoDisconnect();
        if (ret != 0)
        {
            //输出错误信息
            GetErrorMessage(ret);
        }
        isConnected_ = false;
    }
    return true;
}
bool KonicaColorimeter::measureData(double& cx, double& cy, double& lum) 
{
    if (!isConnected_)
    {
        LOG_ERROR("[common][device][Colorimeter][KonicaColorimeter]--isConnected_ false!");
        return false;
    }

    double Lv = 0.0;
    double sx = 0.0;
    double sy = 0.0;

    //测量
    if (0 != GetErrorMessage(m_objca->Measure()))
        return false;
    //获取测量结果
    if (0 != GetErrorMessage(m_objProbe->get_Lv(&Lv)))
        return false;
    if (0 != GetErrorMessage(m_objProbe->get_sx(&sx)))
        return false;
    if (0 != GetErrorMessage(m_objProbe->get_sy(&sy)))
        return false;

    lum = Lv;
    cx = sx;
    cy = sy;

    return true;
}
bool KonicaColorimeter::isConnected() 
{
    return isConnected_;
}

int KonicaColorimeter::defaultSetting()
{
    int ret = 0;

    //清理比较耗时，2-3s
    ret = m_objca->CalZero();
    if (ret != 0)
        return ret;

    //设置显示探头
    ret = m_objca->put_DisplayProbe("P1");
    if (ret != 0)
        return ret;

    //设置同步模式
    ret = m_objca->put_SyncMode(m_freqmode, m_freq);
    if (ret != 0)
        return ret;

    //设置快速测量模式
    ret = m_objca->put_AveragingMode(m_speed);
    if (ret != 0)
        return ret; 
       
    //设置亮度单位
    ret = m_objca->put_BrightnessUnit(m_Lvmode);
    if (ret != 0)
        return ret;   

    //设置显示模式
    ret = m_objca->put_DisplayMode(m_displayMode);
    if (ret != 0)
        return ret;  
                 
    //设置采集通道;
    ret = m_objMemory->put_ChannelNO(m_channelNO);
    if (ret != 0)
        return ret;                  
                            
    //设置清空数据区域回调函数;
    ret = m_objca->SetExeCalZeroCallback([](void* eventData, void* userData)
    {
        ((KonicaColorimeter*)userData)->m_objca->CalZero();
    }, this);
    if (ret != 0)
        return ret;  
        
    return ret;
}

int KonicaColorimeter::GetErrorMessage(int errornum)
{
    if (errornum)
    {
        wchar_t errormessage[512] = { 0 };
        unsigned long int size = 512;
        CASDK2_GetLocalizedErrorMsgFromErrorCode(0, errornum, errormessage, &size);

        std::wstring wstr = errormessage;
        std::string str = common::utility::StringFormat::WString2String(wstr);

        //软件开发套件 CA-SDK2
        //显示器色彩分析仪 CA-410 探头
        //适用于 Windows 和 macOS 的软件包可用
        //官网 https://www.konicaminolta.com.cn/instruments/download/software/index.html#ca-s40
        LOG_ERROR("[common][device][Colorimeter][KonicaColorimeter]--{} failed, sdk_ret={}", str, errornum);
    }
    return errornum;
}

}

}
