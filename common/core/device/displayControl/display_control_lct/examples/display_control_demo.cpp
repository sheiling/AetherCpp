#include <common/core/device/displayControl/display_control_base.h>
#include <common/core/device/displayControl/display_control_lct/display_control_lct.h>
#include <common/utility/string_tool/string_format/string_format.hpp>


const std::map<int, const std::string> TestItemInfos
{
	{0,"初始化"},
	{1,"打屏控制"},
	{2,"系数擦除"},
	{3,"校正开关切换(0:关闭, 1:开启亮度, 2:开启色度)"},
	{4,"获取屏体信息"},
	{5,"切换系数上传模式"},
	{6,"系数上传"},
	{7,"系数固化"},
	{8,"flash检测"},
	{9,"固化配置文件"},
	{10,"回读画质引擎"},
	{11,"固化画质引擎"},
	{12,"写入模组ID"},
	{13,"读取模组ID"},
	{14,"写入箱体ID"},
	{15,"读取箱体ID"},
	{16,"判断是否支持硬件打屏"},
	{17,"判断是否支持写入模组ID"},
	{18,"获取屏体模式"},
	{19,"多灰阶系数上传"},
	{20,"获取伽马列表"},
};

int main() {
#if _WIN32
	setlocale(LC_ALL, ".utf-8");    //设置标准库调用系统API时的默认编码为UTF-8
	SetConsoleOutputCP(CP_UTF8);    //设置控制台输出编码为UTF-8
#endif
	hlog_set_level(0);

	json j;
	json connOpts;
	connOpts["ip"] = "192.168.250.17";
	connOpts["port"] = 8090;
	//connOpts["ip"] = "10.246.10.110";
	//connOpts["port"] = 8080;

    j["type"] = common::device::DisplayControlDevType::DCU_MARS;
	j["connOpts"] = connOpts;

	fprintf(stdout, "Demo start ... \n");
    std::unique_ptr<common::device::DisplayControlLct> m_displayControlLct = std::make_unique<common::device::DisplayControlLct>();

	m_displayControlLct->reflectFromJson(j);

	while (1)
	{
		for (auto tii : TestItemInfos)
		{
			std::cout << tii.first << ":" << tii.second << std::endl;
		}
		printf("输入序号:   \n");
		std::string inputs;
		std::getline(std::cin, inputs);
		std::vector<std::string> words;
        words = common::utility::StringFormat::splitString(inputs, " ");
		if (words.size() <= 0)
			continue;

		int cmd = stoi(words[0]);
		switch (cmd)
		{
			case 0:
			if (m_displayControlLct->init()) {
				fprintf(stdout, "初始化成功\n");
			}
			else {
				fprintf(stdout, "初始化失败\n");
			}
			break;
			case 1:
			break;
			case 2:
			break;
			case 3:
			{
				std::string mode = "0";
				if (words.size() >= 2)
					mode = words[1];

                common::device::ST_ToggleCorrectionSwitch ToggleCorrectionSwitch;
				ToggleCorrectionSwitch.isEnableCorrect = true;

				if (mode == "0") {
					ToggleCorrectionSwitch.isEnableCorrect = false;
				}
				else if (mode == "1") {
                    ToggleCorrectionSwitch.correctMode = common::device::enum_CorrectMode::CorrectModeLum;
				}
				else if (mode == "2") {
                    ToggleCorrectionSwitch.correctMode = common::device::enum_CorrectMode::CorrectModeChroma;
				}

				if (m_displayControlLct->toggleCorrectionSwitch(ToggleCorrectionSwitch).resultCode_ == 0) {
					fprintf(stdout, "校正开关切换成功\n");
				}
				else {
					fprintf(stdout, "校正开关切换失败\n");
				}
			}
			break;
			case 4:
			break;
			case 5:
			break;
			case 6:
			break;
			case 7:
			break;
			case 8:
			break;
			case 9:
			break;
			case 10:
			break;
			case 11:
			break;
			case 12:
			break;
			case 13:
			break;
			case 14:
			break;
			case 15:
			break;
			case 16:
			break;
			case 17:
			break;
			case 18:
			break;
			case 19:
			break;
			case 20:
			break;
			case 21:
			break;
			case 22:
			break;
			case 23:
			break;
			case 24:
			break;
			case 25:
			break;
			case 26:
			break;
			case 27:
			break;
			case 28:
			break;
			case 29:
			break;
			case 30:
			break;
		}

	}









	while (1)
	{
		Sleep(50);
	}

	m_displayControlLct->release();

	getchar();
	return 0;
}

