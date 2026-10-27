//
//  CASDK2MemoryStatus.h
//
//  Copyright (c) 2016 KONICA MINOLTA, INC.
//
//
#pragma once
#include <cstdio>
#include <cstring>

#pragma pack(push, 1)

typedef struct tagCASDK2MemoryStatus
{
    int lMemoryChannelNo;
    int lCurrentProbeNo;
    int lProbeSNOC;
    int lProbeSNOR;
    char sModelName[17];
    int lCalMode;
    int lNDCalMode;
    char sIDName[11];
    char sComment[51];
    int lDisplayMode;

    double frClr_sx;           // Reference Color sx
    double frClr_sy;           // Reference Color sy
    double frClr_Lv_fl;        // Reference Color Lv (fL)
    double frClr_Lv_cdm2;      // Reference Color Lv (cdm2)

    double fdClr_W_sx;         // Declared Calibration Color (W) sx
    double fdClr_W_sy;         // Declared Calibration Color (W) sy
    double fdClr_W_Lv_fl;      // Declared Calibration Color (W) Lv (fL)
    double fdClr_W_Lv_cdm2;    // Declared Calibration Color (W) Lv (cdm2)

    double fdClr_R_sx;         // Declared Calibration Color (R) sx
    double fdClr_R_sy;         // Declared Calibration Color (R) sy
    double fdClr_R_Lv_fl;      // Declared Calibration Color (R) Lv (fL)
    double fdClr_R_Lv_cdm2;    // Declared Calibration Color (R) Lv (cdm2)

    double fdClr_G_sx;         // Declared Calibration Color (G) sx
    double fdClr_G_sy;         // Declared Calibration Color (G) sy
    double fdClr_G_Lv_fl;      // Declared Calibration Color (G) Lv (fL)
    double fdClr_G_Lv_cdm2;    // Declared Calibration Color (G) Lv (cdm2)

    double fdClr_B_sx;         // Declared Calibration Color (B) sx
    double fdClr_B_sy;         // Declared Calibration Color (B) sy
    double fdClr_B_Lv_fl;      // Declared Calibration Color (B) Lv (fL)
    double fdClr_B_Lv_cdm2;    // Declared Calibration Color (B) Lv (cdm2)

    double fmClr_W_sx;         // Measured Calibration Color (W) sx
    double fmClr_W_sy;         // Measured Calibration Color (W) sy
    double fmClr_W_Lv_fl;      // Measured Calibration Color (W) Lv (fL)
    double fmClr_W_Lv_cdm2;    // Measured Calibration Color (W) Lv (cdm2)

    double fmClr_R_sx;         // Measured Calibration Color (R) sx
    double fmClr_R_sy;         // Measured Calibration Color (R) sy
    double fmClr_R_Lv_fl;      // Measured Calibration Color (R) Lv (fL)
    double fmClr_R_Lv_cdm2;    // Measured Calibration Color (R) Lv (cdm2)

    double fmClr_G_sx;         // Measured Calibration Color (G) sx
    double fmClr_G_sy;         // Measured Calibration Color (G) sy
    double fmClr_G_Lv_fl;      // Measured Calibration Color (G) Lv (fL)
    double fmClr_G_Lv_cdm2;    // Measured Calibration Color (G) Lv (cdm2)

    double fmClr_B_sx;         // Measured Calibration Color (B) sx
    double fmClr_B_sy;         // Measured Calibration Color (B) sy
    double fmClr_B_Lv_fl;      // Measured Calibration Color (B) Lv (fL)
    double fmClr_B_Lv_cdm2;    // Measured Calibration Color (B) Lv (cdm2)

    char sYear[5];
    char sMonth[3];
    char sDay[3];
    char sHour[3];
    char sMinutes[3];
    char sSeconds[3];

    tagCASDK2MemoryStatus()
    {
        reset();
    }

    void reset()
    {
        lMemoryChannelNo = -1;
        lCurrentProbeNo = -1;
        lProbeSNOC = -1;
        lProbeSNOR = -1;
        memset(sModelName, 0, (sizeof(sModelName) + 0) * sizeof(sModelName[0]));
        lCalMode = -1;
        lNDCalMode = -1;
        memset(sIDName, 0, (sizeof(sIDName) + 0) * sizeof(sIDName[0]));
        memset(sComment, 0, (sizeof(sComment) + 0) * sizeof(sComment[0]));
        lDisplayMode = -1;

        memset(sYear, 0, (sizeof(sYear) + 0) * sizeof(sYear[0]));
        memset(sMonth, 0, (sizeof(sMonth) + 0) * sizeof(sMonth[0]));
        memset(sDay, 0, (sizeof(sDay) + 0) * sizeof(sDay[0]));
        memset(sHour, 0, (sizeof(sHour) + 0) * sizeof(sHour[0]));
        memset(sMinutes, 0, (sizeof(sMinutes) + 0) * sizeof(sMinutes[0]));
        memset(sSeconds, 0, (sizeof(sSeconds) + 0) * sizeof(sSeconds[0]));

        frClr_sx = -1.;
        frClr_sy = -1.;
        frClr_Lv_fl = -1.;
        frClr_Lv_cdm2 = -1.;

        fdClr_W_sx = -1.;
        fdClr_W_sy = -1.;
        fdClr_W_Lv_fl = -1.;
        fdClr_W_Lv_cdm2 = -1.;

        fdClr_R_sx = -1.;
        fdClr_R_sy = -1.;
        fdClr_R_Lv_fl = -1.;
        fdClr_R_Lv_cdm2 = -1.;

        fdClr_G_sx = -1.;
        fdClr_G_sy = -1.;
        fdClr_G_Lv_fl = -1.;
        fdClr_G_Lv_cdm2 = -1.;

        fdClr_B_sx = -1.;
        fdClr_B_sy = -1.;
        fdClr_B_Lv_fl = -1.;
        fdClr_B_Lv_cdm2 = -1.;

        fmClr_W_sx = -1.;
        fmClr_W_sy = -1.;
        fmClr_W_Lv_fl = -1.;
        fmClr_W_Lv_cdm2 = -1.;

        fmClr_R_sx = -1.;
        fmClr_R_sy = -1.;
        fmClr_R_Lv_fl = -1.;
        fmClr_R_Lv_cdm2 = -1.;

        fmClr_G_sx = -1.;
        fmClr_G_sy = -1.;
        fmClr_G_Lv_fl = -1.;
        fmClr_G_Lv_cdm2 = -1.;

        fmClr_B_sx = -1.;
        fmClr_B_sy = -1.;
        fmClr_B_Lv_fl = -1.;
        fmClr_B_Lv_cdm2 = -1.;
    }
} CASDK2MemoryStatus;

#pragma pack(pop)
