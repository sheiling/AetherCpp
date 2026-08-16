#include "dump_class.h"

void DumpClass::DumpFun() {
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
