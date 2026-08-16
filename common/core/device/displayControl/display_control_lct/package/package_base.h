#ifndef _LCT_PACKAGE_BASE_HH
#define _LCT_PACKAGE_BASE_HH

#include "../../display_control_data.h"
#include <stdlib.h>
#include <string>
#include <vector>

namespace common
{
namespace device
{
class PackageBase
{
public:
    PackageBase();
    PackageBase(int timeout, std::string endTag, CommResults result, const std::string& data, PackTypes packageType, PackCMDTypes  packageCMDtype);
    PackageBase(const PackageBase& packageBase);
    virtual ~PackageBase();
    PackageBase& operator=(const PackageBase&);

    std::string getEndTag();
    void setEndTag(const std::string& tag);

    uint32_t getSerialNum();
    void setSerialNum(uint32_t serialNum);

    void setData(const std::string& data);
    std::string getData();
    std::string getDataProto();
    uint32_t getDataLength() const;

    void setCommResults(const CommResults& result);
    CommResults getCommResults();

    void setPackTypes(const PackTypes& type);
    PackTypes getPackTypes();

    void setPackCMDTypes(const PackCMDTypes& type);
    PackCMDTypes getPackCMDTypes();

    int getTimeout();
    void setTimeout(int timeout);

    void setPackageResults(const PackageResults& result);
    PackageResults getPackageResults();

    // 添加对命令码的获取，接收判断使用
    void setOutCmd(uint32_t cmdCode);
    uint32_t getOutCmd();

    bool isPackageSuccess();

private:
    void fromOtherObj(const PackageBase& other);

public:
    static uint64_t     g_serialNum;
    uint32_t            m_cmdCode = 0;

protected:
    CommResults m_commResult = CommResults::FileErr;                //此处表示由对方返回的协议携带的错误类型
    PackageResults m_packgeResults = PackageResults::ok;            //此处表示本地处理逻辑等一些列的错误类型,诸如发送文件时该文件已经打开失败了
    PackTypes m_packageType = PackTypes::Pack_OutCMD;               //包类型 
    PackCMDTypes m_packCMDTypes = PackCMDTypes::Pack_Common;        //包命令类型

    std::string m_strEndTag;                                        //结束符
    std::string m_data;                                             //原始数据
    std::string m_dataProto;                                        //协议数据

    uint32_t m_dataLength = 0;                                      //数据长度
    uint64_t m_serialNum = 0;                                       //序列号
    int m_Timeout;


};

}
}

#endif