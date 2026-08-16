#include "../winscreen/winscreen.h"
#include <stdio.h>

int main()
{

    json j;
    json connOpts;
    connOpts["devNO"] = 0;
    j["connOpts"] = connOpts;
    j["type"] = common::device::SoftwareScreenDevType::SWSDN_WIN32;


    fprintf(stdout, "Demo start ... \n");
    common::device::WinScreen* winScreen = new common::device::WinScreen();
    if (!winScreen) {
        fprintf(stdout, "New SoftScreen failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "New SoftScreen success.\n");
    winScreen->reflectFromJson(j);

    common::device::SoftScreenParam screenParams;
    screenParams.width = 500;
    screenParams.height = 500;
    screenParams.brightness = 255;

    // 初始化窗口
    winScreen->Init();

    // 纯色红色打屏
    fprintf(stdout, "Full Red start ... \n");

    screenParams.color = common::device::SoftScreenColor::COLOR_RED;
    screenParams.mode = common::device::SoftScreenMode::FULL_MODE;
    winScreen->Display(screenParams);

    getchar();
    fprintf(stdout, "Full Red end ... \n");

    // 纯色绿色打屏
    fprintf(stdout, "Full Green start ... \n");
    
    screenParams.color = common::device::SoftScreenColor::COLOR_GREEN;
    screenParams.mode = common::device::SoftScreenMode::FULL_MODE;
    winScreen->Display(screenParams);

    getchar();
    fprintf(stdout, "Full Green end ... \n");

    // 纯色绿色打屏
    fprintf(stdout, "Full Blue start ... \n");
    
    screenParams.color = common::device::SoftScreenColor::COLOR_BLUE;
    screenParams.mode = common::device::SoftScreenMode::FULL_MODE;
    winScreen->Display(screenParams);

    getchar();
    fprintf(stdout, "Full Blue end ... \n");

    // 纯色白色打屏
    fprintf(stdout, "Full White start ... \n");
    
    screenParams.color = common::device::SoftScreenColor::COLOR_WHITE;
    screenParams.mode = common::device::SoftScreenMode::FULL_MODE;
    winScreen->Display(screenParams);

    getchar();
    fprintf(stdout, "Full White end ... \n");

    // 纯色黑色打屏
    fprintf(stdout, "Full Black start ... \n");
    
    screenParams.color = common::device::SoftScreenColor::COLOR_BLACK;
    screenParams.mode = common::device::SoftScreenMode::FULL_MODE;
    winScreen->Display(screenParams);

    getchar();
    fprintf(stdout, "Full Black end ... \n");

    // 红色奇数棋盘格打屏
    fprintf(stdout, "Odd Red start ... \n");
    
    screenParams.color = common::device::SoftScreenColor::COLOR_RED;
    screenParams.mode = common::device::SoftScreenMode::ODD_MODE;
    screenParams.interval = 2;
    winScreen->Display(screenParams);

    getchar();
    fprintf(stdout, "Odd Red end ... \n");

    // 绿色奇数棋盘格打屏
    fprintf(stdout, "Odd Green start ... \n");
    
    screenParams.color = common::device::SoftScreenColor::COLOR_GREEN;
    screenParams.mode = common::device::SoftScreenMode::ODD_MODE;
    screenParams.interval = 2;
    winScreen->Display(screenParams);

    getchar();
    fprintf(stdout, "Odd Green end ... \n");

    // 蓝色奇数棋盘格打屏
    fprintf(stdout, "Odd Blue start ... \n");
    
    screenParams.color = common::device::SoftScreenColor::COLOR_BLUE;
    screenParams.mode = common::device::SoftScreenMode::ODD_MODE;
    screenParams.interval = 2;
    winScreen->Display(screenParams);

    getchar();
    fprintf(stdout, "Odd Blue end ... \n");

    // 红色偶数棋盘格打屏
    fprintf(stdout, "Even Red start ... \n");
    
    screenParams.color = common::device::SoftScreenColor::COLOR_RED;
    screenParams.mode = common::device::SoftScreenMode::EVEN_MODE;
    screenParams.interval = 2;
    winScreen->Display(screenParams);

    getchar();
    fprintf(stdout, "Even Red end ... \n");

    // 绿色偶数棋盘格打屏
    fprintf(stdout, "Even Green start ... \n");
    
    screenParams.color = common::device::SoftScreenColor::COLOR_GREEN;
    screenParams.mode = common::device::SoftScreenMode::EVEN_MODE;
    screenParams.interval = 2;
    winScreen->Display(screenParams);

    getchar();
    fprintf(stdout, "Even Green end ... \n");

    // 蓝色偶数棋盘格打屏
    fprintf(stdout, "Even Blue start ... \n");
    
    screenParams.color = common::device::SoftScreenColor::COLOR_BLUE;
    screenParams.mode = common::device::SoftScreenMode::EVEN_MODE;
    screenParams.interval = 2;
    winScreen->Display(screenParams);

    getchar();
    fprintf(stdout, "Even Blue end ... \n");

    getchar();
    winScreen->Release();
    fprintf(stdout, "Demo end ... \n");

}