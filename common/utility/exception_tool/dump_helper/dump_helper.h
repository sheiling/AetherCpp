#ifndef __COMMON_UTILITY_DUMP_HELPER_H__
#define __COMMON_UTILITY_DUMP_HELPER_H__

#include<string>

namespace common {

namespace utility 
{

class  DumpHelper {
public:
    // dir:dmp文件生成目录, dmpMaxCnt:dmp文件最大存放数量(超出后自动移除最旧的dmp文件)
    static void StartDumpCrash(std::string dir = "dmp", int dmpMaxCnt = 100);

    // 获取内存快照转储,可以任意时刻记录(包括非崩溃情况下)
    // <param name="path">保存文件名
    // static void Snapshot(const std::string& path)
private:
    // 设置是否记录崩溃转储
    // <param name="value">是否记录崩溃转储
    static void SetIsDumpCrash(bool value = true);

    // 设置崩溃转储路径
    // <param name="directory">崩溃转储路径
    static void SetDumpDirectory(const std::string& directory);

    // 崩溃转储文件数量
    // 超过数量自动删除旧文件
    // <param name="count">最大文件数量，默认500
    static void SetDumpMaxFileCount(int count);
};

}

}

#endif
