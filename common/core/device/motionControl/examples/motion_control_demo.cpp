#include "../motion_controller.h"
#include "common/utility/file_tool/file_manager.hpp"
#include <sstream>

#define KEY_AXIS_NAME_LOADERX           "X"
#define KEY_AXIS_NAME_LOADERY           "Y"
#define KEY_AXIS_NAME_LOADERZ1          "Z"
#define KEY_AXIS_NAME_LOADERZ2          "LoaderZ2"
#define KEY_AXIS_NAME_WAFERX            "WaferX"
#define KEY_AXIS_NAME_WAFERY            "WaferY"

#define KEY_IO_NAME_INPUT1				"input1"		
#define KEY_IO_NAME_OUTPUT1				"output1"

#define KEY_IO_EXTEND_NAME_INPUT1		"inputExtend1"	
#define KEY_IO_EXTEND_NAME_OUTPUT1		"outputExtend1"	

#define KEY_AD_EXTEND_NAME_INPUT1		"inputAD1"	
#define KEY_AD_EXTEND_NAME_OUTPUT1		"outputAD1"	

int main() {

#if _WIN32
	setlocale(LC_ALL, ".utf-8");    //设置标准库调用系统API时的默认编码为UTF-8
	SetConsoleOutputCP(CP_UTF8);    //设置控制台输出编码为UTF-8
#endif

	json j;
	// 打开文件
	std::string filePath = common::utility::FileManager::getCurrentDir() + "config/device_config.json";
	std::ifstream file(filePath);
	if (!file.is_open()) {
		fprintf(stdout, "Open file[%s] failed.\n", filePath.c_str());
		return false;
	}
	std::ostringstream stringStream;
	stringStream << file.rdbuf();
	std::string jsonStrStream = stringStream.str();
	file.close();

	// 2 反序列化JSON
	try {
		j = nlohmann::json::parse(jsonStrStream);
	}
	catch (std::exception& e) {
		fprintf(stdout, "[common][device][motion]---nlohmann parse json error, errMsg=%s\n", e.what());
		return false;
	}

	fprintf(stdout, "Demo start ... \n");
	common::device::MotionController* motionHandle = new common::device::MotionController();
	if (!motionHandle) {
		fprintf(stdout, "New motionHandle failed.\n");
		getchar();
		return -1;
	}
	fprintf(stdout, "New motionHandle success.\n");

	if (!j.contains("DeviceManager") || j["DeviceManager"].is_null()) {
		fprintf(stdout, "没有找到key: DeviceManager.\n");
		getchar();
		return -1;
	}

	json jDeviceManager = j["DeviceManager"];
	if (!jDeviceManager.contains("MotionController") || jDeviceManager["MotionController"].is_null()) {
		fprintf(stdout, "没有找到key: MotionController.\n");
		getchar();
		return -1;
	}
	motionHandle->reflectFromJson(jDeviceManager["MotionController"]);

#if 1

	// 一、连接运动板卡
	common::errors::ErrorPtr initErr = motionHandle->InitDevice();
	if (initErr != nullptr) {
		fprintf(stdout, "Device connect failed.\n");
		getchar();
		return -1;
	}
	fprintf(stdout, "Device connect success.\n");

	// 三、轴能轴
	if ((motionHandle->AxisEnable(KEY_AXIS_NAME_LOADERX, false)) != nullptr) {
		fprintf(stdout, "Axis AxisEnable failed.\n");
		getchar();
		return -1;
	}
	if ((motionHandle->AxisEnable(KEY_AXIS_NAME_LOADERX, true)) != nullptr) {
		fprintf(stdout, "Axis AxisEnable failed.\n");
		getchar();
		return -1;
	}
	// 四、轴状态读取
	common::device::axis::Axis::AxisSta axisSta;
	if (motionHandle->AxisReadStatus(KEY_AXIS_NAME_LOADERX, axisSta) != nullptr) {
		fprintf(stdout, "Axis ReadStatus failed.\n");
		getchar();
		return -1;
	}
	fprintf(stdout, "伺服报警:%d\n正硬限位:%d\n负硬限位:%d\n急停:%d\n原点:%d\n正软限位:%d\n负软限位:%d\n伺服到位:%d\nEZ:%d\n就绪:%d\n减速停止:%d\n",
		axisSta.ALM, axisSta.ELP, axisSta.ELN, axisSta.EMG, axisSta.ORG, axisSta.SLP, axisSta.SLN, axisSta.INP, axisSta.EZ, axisSta.RDY, axisSta.DSTP);

	double value = 0;
	if (motionHandle->AxisGetEncoder(KEY_AXIS_NAME_LOADERX, value) != nullptr) {
		fprintf(stdout, "Axis AxisGetEncoder failed.\n");
		getchar();
		return -1;
	}
	fprintf(stdout, "%s%s:%.4f\n", KEY_AXIS_NAME_LOADERX, "AxisGetEncoder success", value);

	if (motionHandle->AxisGetPosition(KEY_AXIS_NAME_LOADERX, value) != nullptr) {
		fprintf(stdout, "Axis AxisGetPosition failed.\n");
		getchar();
		return -1;
	}
	fprintf(stdout, "%s%s:%.4f\n", KEY_AXIS_NAME_LOADERX, "AxisGetPosition success", value);

	if (motionHandle->AxisReset(KEY_AXIS_NAME_LOADERX, 30000) != nullptr) {
		fprintf(stdout, "AxisReset failed.\n");
		getchar();
		return -1;
	}
	fprintf(stdout, "%s%s", KEY_AXIS_NAME_LOADERX, "AxisReset success.\n");

	// 五、轴运动控制
	// 单轴定长运动  KEY_AXIS_NAME_LOADERX轴绝对运动到50的位置
	if (motionHandle->AxisMove(KEY_AXIS_NAME_LOADERX, {50.0, 1u}) != nullptr) {
		fprintf(stdout, "%s%s", KEY_AXIS_NAME_LOADERX, "Axis Move failed.\n");
		getchar();
		return -1;
	}
	fprintf(stdout, "%s%s", KEY_AXIS_NAME_LOADERX, "Axis Move success.\n");

	// 单轴定长运动  KEY_AXIS_NAME_LOADERY轴绝对运动到20的位置
	if (motionHandle->AxisMove(KEY_AXIS_NAME_LOADERX, {20.0, 1u}) != nullptr) {
		fprintf(stdout, "%s%s", KEY_AXIS_NAME_LOADERX, "Axis Move failed.\n");
		getchar();
		return -1;
	}
	fprintf(stdout, "%s%s", KEY_AXIS_NAME_LOADERX, "Axis Move success.\n");

	// 六、高速位置比较触发（AxisMoveWithPositionCompare）
	for (int i = 0; i < 3; i++)
	{
		common::device::motion::AxisMoveWithPositionCompareParams highCmp;
		highCmp.startPos = 20.0;
		highCmp.endPos = 100.0;
		highCmp.comparePositions = {30.0, 90.0};
		highCmp.channel = common::device::motion::PositionCompareChannel::HighSpeed1;
		highCmp.output.pulseWidthMs = 1;
		highCmp.moveToStartFirst = true;
		if (motionHandle->AxisMoveWithPositionCompare(KEY_AXIS_NAME_LOADERX, highCmp) != nullptr) {
			fprintf(stdout, "%s%s", KEY_AXIS_NAME_LOADERX, "AxisMoveWithPositionCompare failed.\n");
			getchar();
			return -1;
		}

		highCmp.startPos = 100.0;
		highCmp.endPos = 20.0;
		highCmp.comparePositions = {90.0, 30.0};
		if (motionHandle->AxisMoveWithPositionCompare(KEY_AXIS_NAME_LOADERX, highCmp) != nullptr) {
			fprintf(stdout, "%s%s", KEY_AXIS_NAME_LOADERX, "AxisMoveWithPositionCompare failed.\n");
			getchar();
			return -1;
		}
	}

	// 七、低速位置比较触发
	for (int i = 0; i < 3; i++)
	{
		common::device::motion::AxisMoveWithPositionCompareParams lowCmp;
		lowCmp.startPos = 20.0;
		lowCmp.endPos = 100.0;
		lowCmp.comparePositions = {30.0, 90.0};
		lowCmp.channel = common::device::motion::PositionCompareChannel::LowSpeed;
		lowCmp.output.ioName = KEY_IO_NAME_OUTPUT1;
		lowCmp.output.action = common::device::axis::Axis::AxisPsoAction::WriteIO_Reverse;
		lowCmp.moveToStartFirst = true;
		if (motionHandle->AxisMoveWithPositionCompare(KEY_AXIS_NAME_LOADERX, lowCmp) != nullptr) {
			fprintf(stdout, "%s%s", KEY_AXIS_NAME_LOADERX, "AxisMoveWithPositionCompare failed.\n");
			getchar();
			return -1;
		}

		lowCmp.startPos = 100.0;
		lowCmp.endPos = 20.0;
		lowCmp.comparePositions = {90.0, 30.0};
		if (motionHandle->AxisMoveWithPositionCompare(KEY_AXIS_NAME_LOADERX, lowCmp) != nullptr) {
			fprintf(stdout, "%s%s", KEY_AXIS_NAME_LOADERX, "AxisMoveWithPositionCompare failed.\n");
			getchar();
			return -1;
		}
	}


	// 八、开启高速位置锁存
	if (motionHandle->AxisEnableLtc(KEY_AXIS_NAME_LOADERX, common::device::axis::Axis::AxisLtc(true, 1, 0, 1, 1)) != nullptr) {
		fprintf(stdout, "%s%s", KEY_AXIS_NAME_LOADERX, "AxisEnableLtc failed.\n");
		getchar();
		return -1;
	}
	if (motionHandle->AxisMove(KEY_AXIS_NAME_LOADERX, {100.0, 1u}) != nullptr) {
		fprintf(stdout, "%s%s", KEY_AXIS_NAME_LOADERX, "Axis Move failed.\n");
		getchar();
		return -1;
	}
	if (motionHandle->AxisMove(KEY_AXIS_NAME_LOADERX, {20.0, 1u}) != nullptr) {
		fprintf(stdout, "%s%s", KEY_AXIS_NAME_LOADERX, "Axis Move failed.\n");
		getchar();
		return -1;
	}

	std::vector<double> vec;
	if (motionHandle->AxisGetLtcPos(KEY_AXIS_NAME_LOADERX, common::device::axis::Axis::AxisLtc(true, 1, 0, 1, 1), vec) != nullptr) {
		fprintf(stdout, "%s%s", KEY_AXIS_NAME_LOADERX, "AxisGetLtcPos failed.\n");
		getchar();
		return -1;
	}
	fprintf(stdout, "ltcPos:");
	for (int i = 0; i < vec.size(); i++)
		fprintf(stdout, "%.4f_", vec[i]);
	fprintf(stdout, "\n\n");


	// 九、开启软件锁存
	if (motionHandle->AxisEnableLtc(KEY_AXIS_NAME_LOADERX, common::device::axis::Axis::AxisLtc(true, 0, 0, 1, 1, 12, 0, 1)) != nullptr) {
		fprintf(stdout, "%s%s", KEY_AXIS_NAME_LOADERX, "AxisEnableLtc failed.\n");
		getchar();
		return -1;
	}
	if (motionHandle->AxisMove(KEY_AXIS_NAME_LOADERX, {100.0, 1u}) != nullptr) {
		fprintf(stdout, "%s%s", KEY_AXIS_NAME_LOADERX, "Axis Move failed.\n");
		getchar();
		return -1;
	}
	if (motionHandle->AxisMove(KEY_AXIS_NAME_LOADERX, {20.0, 1u}) != nullptr) {
		fprintf(stdout, "%s%s", KEY_AXIS_NAME_LOADERX, "Axis Move failed.\n");
		getchar();
		return -1;
	}

	std::vector<double> vecSoftware;
	if (motionHandle->AxisGetLtcPos(KEY_AXIS_NAME_LOADERX, common::device::axis::Axis::AxisLtc(true, 0, 0), vecSoftware) != nullptr) {
		fprintf(stdout, "%s%s", KEY_AXIS_NAME_LOADERX, "AxisGetLtcPos failed.\n");
		getchar();
		return -1;
	}
	fprintf(stdout, "SoftwareltcPos:");
	for (int i = 0; i < vecSoftware.size(); i++)
		fprintf(stdout, "%.4f_", vecSoftware[i]);
	fprintf(stdout, "\n\n");


	// 十、轴停止
	if (motionHandle->AxisStop(KEY_AXIS_NAME_LOADERX) != nullptr) {
		fprintf(stdout, "Stop Axis failed.\n");
		getchar();
		return -1;
	}
	fprintf(stdout, "Stop Axis success.\n");
#endif // 0

	bool valueIO = false;
	motionHandle->WriteIO(KEY_IO_NAME_OUTPUT1, true);
	motionHandle->ReadIO(KEY_IO_NAME_OUTPUT1, valueIO);
	motionHandle->WriteIO(KEY_IO_NAME_OUTPUT1, false);
	motionHandle->ReadIO(KEY_IO_NAME_OUTPUT1, valueIO);

	motionHandle->ReadIO(KEY_IO_NAME_INPUT1, valueIO);
	motionHandle->ReadIO(KEY_IO_NAME_INPUT1, valueIO);

	motionHandle->WriteIO(KEY_IO_EXTEND_NAME_OUTPUT1, true);
	motionHandle->ReadIO(KEY_IO_EXTEND_NAME_OUTPUT1, valueIO);
	motionHandle->WriteIO(KEY_IO_EXTEND_NAME_OUTPUT1, false);
	motionHandle->ReadIO(KEY_IO_EXTEND_NAME_OUTPUT1, valueIO);

	motionHandle->ReadIO(KEY_IO_EXTEND_NAME_INPUT1, valueIO);
	motionHandle->ReadIO(KEY_IO_EXTEND_NAME_INPUT1, valueIO);


	double adValue = 0;
	motionHandle->ReadAD(KEY_AD_EXTEND_NAME_INPUT1, adValue);
	motionHandle->ReadAD(KEY_AD_EXTEND_NAME_INPUT1, adValue);

	motionHandle->WriteAD(KEY_AD_EXTEND_NAME_OUTPUT1, 0.4);
	motionHandle->ReadAD(KEY_AD_EXTEND_NAME_OUTPUT1, adValue);
	motionHandle->WriteAD(KEY_AD_EXTEND_NAME_OUTPUT1, 1);
	motionHandle->ReadAD(KEY_AD_EXTEND_NAME_OUTPUT1, adValue);


	// 七、断开与运动板卡的连接
	motionHandle->Release();

	getchar();

	return 0;
}

