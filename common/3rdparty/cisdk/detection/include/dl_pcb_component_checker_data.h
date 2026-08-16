#ifndef _DL_PCB_COMPONENT_CHECKE_DATA_H_
#define _DL_PCB_COMPONENT_CHECKE_DATA_H_

#include "ci_data.h"
#include "dl_pcb_component_checker_data.h"




typedef enum {
	HAVE_COMPONENT = 0,                  // 有元件
	NO_COMPONENT = 1,                    // 无元件
}DLPcbComponentStatus;                   // pcb上元件状态

typedef struct {
	AlgoRect component_rect;                               // pcb上元件的位置
	DLPcbComponentStatus component_status = NO_COMPONENT;  // pcb上元件状态
}DLPcbComponentInfo;


#endif