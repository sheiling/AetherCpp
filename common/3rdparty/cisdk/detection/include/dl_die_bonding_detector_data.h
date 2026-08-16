#ifndef _DL_DIE_BONDING_DETECTOR_DATA_H_
#define _DL_DIE_BONDING_DETECTOR_DATA_H_

#include "ci_data.h"
#include "die_bonding_defect_detector_data.h"

typedef enum
{
	DL_CHIP_NORMAL = 0x00000000,                  // оƬ����
	DL_CHIP_MISSING = 0x00000001,                 // оƬȱʧ
	//DL_CHIP_DIRTY = 0x00000002,                   // оƬ���ۣ��ѷ�����  
	DL_CHIP_BROKEN = 0x00000004,                  // оƬ����
	DL_FOREIGN_OBJECT = 0x00000008,               // ����ȱ��
	DL_SOLDER_ABNORMAL = 0x00000010,              // �����쳣������ȱ�ݣ�
	DL_CHIP_BONDING_ERROR = 0x00000020,           // оƬ�̴�����RGB��λ�ù̴�

	DL_CHIP_TOMBSTONE = 0x00000040,               // оƬ��������ʱδ���ţ�
	DL_CHIP_SIDE_STANDING = 0x00000080,           // оƬ��������ʱδ���ţ�
	DL_CHIP_BONDING_REVESE = 0x00000100,          // оƬ�̷�����ʱδ���ţ�
	DL_CHIP_BONDING_OVERLAP = 0x00000200,         // оƬ����
	//DL_CHIP_OFFSET = 0x00000400,                  // оƬƫ��
	//DL_SOLDER_DIRTY = 0x00000800,                 // ��������
	//DL_CHIP_IMAGE_BLUR = 0x00001000,              // оƬģ��(��ҵ���쳣��������ͼ���������)
	DL_DEFECT_UNKNOWN = 0x00002000,               // δ֪ȱ��
}DLDieBondingDefectType;


typedef enum {
	DL_POLARITY_LEFT = 0,            // ���Ա�־����
	DL_POLARITY_RIGHT = 1,           // ���Ա�־����
	DL_POLARITY_TOP = 2,             // ���Ա�־����
	DL_POLARITY_BOTTOM = 3,		     // ���Ա�־����
	DL_POLARITY_UNDETECTABLE = 4,        // ���Ա�־����ʶ��
}DLDieBondingPolarType;

typedef enum {
	CHIP_GROUP_LOCATE_MODEL = 0,      // оƬ�鶨λģ��
	CHIP_DETECT_MODEL = 1,            // оƬ���ģ��
	CHIP_DEFECT_CLASSIFY_MODEL = 2,   // оƬȱ�ݷ���ģ��
	CHIP_POLARITY_CLASSIFY_MODEL = 3, // оƬ���Է���ģ��
	CHIP_SOLDER_PASTE_MODEL = 4,      // ������ģ��
}DLDieBondingModelType;


typedef struct DLDieBondingParameterTag
{
	AlgoSize solder_size = {0, 0};						    // ���̳ߴ�
	AlgoSize chip_size = { 0, 0 };							// оƬ�ߴ�
	AlgoSize chip_group_size = {0, 0};						// оƬ��ߴ�
	AlgoSize led_gap = {0, 0};								// led����
	DieBondingChipAlignType chip_align_type = CHIP_RGB_H;   // оƬ���з�ʽ
}DLDieBondingParameter;

#endif