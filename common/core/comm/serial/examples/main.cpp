#include "../serial_master.h"

int main() {
	common::serial::SerialMaster mSerialMaster;
	common::serial::SerialInfo mOpts;
	mOpts.com_ = "COM1";
	mOpts.baud_ = 115200;
	mOpts.dataBit_ = 8;
	mOpts.parity_ = common::serial::ParityMode::None;
	mOpts.stopBit_ = 1;
	mSerialMaster.init(mOpts);

	std::string str;
	mSerialMaster.read(str, 10, 2000000);
	fprintf(stdout, "%s\n", str.c_str());

	mSerialMaster.read(str, "\r\n", 2000000);
	fprintf(stdout, "%s\n", str.c_str());

	system("pause");
	return 0;
}