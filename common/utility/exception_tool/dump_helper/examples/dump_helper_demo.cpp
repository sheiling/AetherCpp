#include <utility/exception_tool/dump_helper/dump_helper.h>
#include <thread>
#include <vector>
#include <functional>
#include "dumpClass/dump_class.h"

void DumpFun() {
	// 触发一个"除零"异常
#if 0
	int a = 0;
	int b = 100;
	int c = b / a;
	fprintf(stdout, "c=%d\n", c);
#endif

#if 1
	// 触发一个"访问空指针"异常
	int *p = nullptr;
	*p = 100;
#endif

#if 0
	// 触发一个容器越界异常
	std::vector<int> tmpVec;
	for (int i = 0; i < 100; i ++) {
		tmpVec.push_back(i);
	}
	fprintf(stdout, "tmpVec.size=%u, tmpVec.capacity=%u, tmpVec[1000]=%d\n", tmpVec.size(), tmpVec.capacity(), tmpVec[1000000]);
	#endif
}

int main(int argc, char** argv) {
	common::utility::DumpHelper::StartDumpCrash("dump", 10);

#if 0
	// 当前线程异常测试
	DumpFun();
#endif

#if 0
	// 跨线程异常测试
	for (int i = 0; i < 9; ++i) {
		std::thread th(DumpFun);
		th.detach();
	}
#endif

#if 0
	// 当前线程异常测试
	DumpClass dump;
	dump.DumpFun();
#endif

#if 1
	// 跨线程异常测试
	DumpClass dump;
	for (int i = 0; i < 1; ++i) {
		std::thread th(std::bind(&DumpClass::DumpFun, &dump));
		th.detach();
	}
#endif

	getchar();

	return 0;
}
