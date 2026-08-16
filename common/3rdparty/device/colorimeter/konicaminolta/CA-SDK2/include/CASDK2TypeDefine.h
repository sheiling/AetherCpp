//
//  CASDK2TypeDefine.h
//
//  Copyright (c) 2016 KONICA MINOLTA, INC.
//
//
#pragma once
#include <limits>
#include <cstring>

#define INT_INIT_VAL -1
#define DOUBLE_INIT_VAL -1.0

typedef enum tagPortNumbers
{
    PORT_USB = 0,
    PORT_USB_AUTO = 0,
    PORT_USB_MIN = 0,
    PORT_USB_MAX = 255,

    PORT_ETH_AUTO = 0,
    PORT_ETH_MANUAL = 1,
    PORT_ETH_MIN = 0,
    PORT_ETH_MAX = 1,

    PORT_COMM_MIN = 1,
    PORT_COMM_MAX = 255,

    PORT_RS_AUTO = 0,
    PORT_BT_AUTO = 0,
    PORT_BT_MANUAL = 1,
} CASDK2_PortNumber;

typedef enum tagConfigTypes
{
    CONFIG_UNKNOWN = -1,
    CONFIG_USB = 0,
    CONFIG_RS = 1,
    CONFIG_ETH = 2,
    CONFIG_BT = 3
} CASDK2_ConfigurationType;

typedef enum tagBaudRate
{
    BAUD_RATE_600 = 600,
    BAUD_RATE_1200 = 1200,
    BAUD_RATE_2400 = 2400,
    BAUD_RATE_4800 = 4800,
    BAUD_RATE_9600 = 9600,
    BAUD_RATE_19200 = 19200,
    BAUD_RATE_38400 = 38400,
    BAUD_RATE_57600 = 57600,
    BAUD_RATE_115200 = 115200,
    BAUD_RATE_230400 = 230400,
    BAUD_RATE_460800 = 460800,
    BAUD_RATE_921600 = 921600
} CASDK2_BaudRate;

typedef enum tagDisplayModes
{
    DSP_LXY = 0,
    DSP_DUV = 1,
    DSP_ANL = 2,
    DSP_ANLG = 3,
    DSP_ANLR = 4,
    DSP_PUV = 5,
    DSP_FMA = 6,
    DSP_XYZC = 7,
    DSP_JEITA = 8,
    DSP_DWEPLV = 9,
    DSP_WVFM = 10,
    DSP_FMAH = 11,
    DSP_JEITAH = 12,
    DSP_WVFMH = 13
} CASDK2_DisplayMode;

typedef enum tagSyncMode
{
    SYNC_NTSC = 0,
    SYNC_PAL = 1,
    SYNC_EXT = 2,
    SYNC_UNIV = 3,
    SYNC_INT = 4,
    SYNC_MANUAL = 5
} CASDK2_SyncMode;

typedef enum tagMeasurementType
{
    MEAS_TYPE_AFE = 0,
    MEAS_TYPE_ADC = 1
} CASDK2_MeasurementType;

typedef enum tagMeasurementRange
{
    MEAS_RANGE_AUTO = 0,
    MEAS_RANGE_FIXED = 1
} CASDK2_MeasurementRange;

typedef enum tagSampleCount
{
    SAMPLE_COUNT_64 = 6,
    SAMPLE_COUNT_128 = 7,
    SAMPLE_COUNT_256 = 8,
    SAMPLE_COUNT_512 = 9,
    SAMPLE_COUNT_1024 = 10,
    SAMPLE_COUNT_2048 = 11,
    SAMPLE_COUNT_4096 = 12,
    SAMPLE_COUNT_8192 = 13
} CASDK2_SampleCount;

typedef enum tagWaveformFilter
{
    FILTER_NONE = 0,
    FILTER_JEITA = 1,
    FILTER_LOWPASS = 2,
    FILTER_USER = 3
} CASDK2_WaveformFilter;

typedef enum tagDisplayDigits
{
    DIGT_3 = 0,
    DIGT_4 = 1
} CASDK2_DisplayDigits;

typedef enum tagAveragingMode
{
    AVRG_SLOW = 0,
    AVRG_FAST = 1,
    AVRG_LTD_AUTO = 2,
    AVRG_AUTO = 3
} CASDK2_AveragingMode;

typedef enum tagLuminanceUnit
{
    BUNIT_FL = 0,
    BUNIT_CD = 1
} CASDK2_LuminanceUnit;

// Minolta calibration standards
typedef enum tagMinoltaCalibrationStandard
{
    CAL_D65 = 1,
    CAL_9300 = 2,
    CAL_CA100 = 3
} CASDK2_MinoltaCalibrationStandard;

typedef enum tagCAType
{
    CATYPE_210 = 0,
    CATYPE_210S = 1,
    CATYPE_100PLUS = 2,
    CATYPE_200 = 3,
    CATYPE_210U = 4,
    CATYPE_310 = 5
} CASDK2_CAType;

typedef enum tagProbeType
{
    PROBETYPE_CA_MP410 = 830,
    PROBETYPE_CA_MP410H = 831,
    PROBETYPE_CA_P427 = 810,
    PROBETYPE_CA_P427H = 811,
    PROBETYPE_CA_P427C = 814,
    PROBETYPE_CA_P410 = 890,
    PROBETYPE_CA_P410H = 891,
    PROBETYPE_CA_P410C = 894,
    PROBETYPE_CA_VP427 = 840,
    PROBETYPE_CA_VP410 = 850,
    PROBETYPE_CA_DP40 = 100,
    PROBETYPE_CA_LP_MP410 = 832,
    PROBETYPE_CA_LP_MP410H = 833,
    PROBETYPE_CA_LP_P427 = 812,
    PROBETYPE_CA_LP_P427H = 813,
    PROBETYPE_CA_LP_P410 = 892,
    PROBETYPE_CA_LP_P410H = 893,
    PROBETYPE_CA_LP_VP427 = 842,
    PROBETYPE_CA_LP_VP410 = 852
} CASDK2_ProbeType;

typedef enum tagRemoteMode
{
    REMOTE_OFF = 0,
    REMOTE_ON = 1,
    REMOTE_LOCK = 2
} CASDK2_RemoteMode;

// Color Calibration
typedef enum tagCalibrationColor
{
    CAL_RED = 0,
    CAL_GREEN = 1,
    CAL_BLUE = 2,
    CAL_WHITE = 3
} CASDK2_CalibrationColor;

typedef enum tagZeroCalState
{
    ZEROCAL_STATE_INVALID = -1,
    ZEROCAL_STATE_NOT_EXECUTED = 0,
    ZEROCAL_STATE_RECOMMENDED = 1,
    ZEROCAL_STATE_COMPLETED = 2
} CASDK2_ZeroCalState;

typedef enum tagTrgrMeasResult
{
    EXT_TRGR_RESULT_SUCCEEDED = 0,
    EXT_TRGR_RESULT_TIMEDOUT = -1,
    EXT_TRGR_RESULT_CANCELLED = -2
} CASKD2_TrgrMeasResult;

typedef enum tagOnOff
{
    SWITCH_OFF = 0,
    SWITCH_ON = 1
} CASDK2_OnOff;

typedef enum tagLanguage
{
    LANG_EN = 0,
    LANG_JP = 1,
    LANG_CH_SIMP = 2,
    LANG_CH_TRAD = 3,
    LANG_KR = 4
} CASDK2_Language;

typedef enum tagNumType
{
    TYPE_CHANNEL_NO = 1,
    TYPE_PROBE = 2,
} CASDK2_NumType;

typedef enum tagBatteryStatus
{
    BATT_LOCK = 0,
    BATT_ALARM = 1,
    BATT_ENOUGH = 2,
    BATT_FULL = 3,
    BATT_NONE = 4
} CASDK2_BatteryStatus;

typedef enum tagFlickerSensitivityFactorProfile
{
    FSFP_ORIG = 0,
    FSFP_USER
} CASDK2_FlickerSensitivityFactorProfile;

typedef enum tagLCDBrightness
{
    BR_LV_1_DARK = 1,
    BR_LV_2 = 2,
    BR_LV_3_STANDARD = 3,
    BR_LV_4 = 4,
    BR_LV_5_BRIGHT = 5
} CASDK2_LCDBrightness;

typedef enum tagPowerSaveTime
{
    PS_5_MIN = 0,
    PS_10_MIN = 1,
    PS_30_MIN = 2
} CASDK2_PowerSaveTime;

typedef enum tagColorFlickerMeasMode
{
    MEAS_MODE_COLOR_FLICKER = 0,
    MEAS_MODE_COLOR_ONLY = 1,
    MEAS_MODE_FLICKER_ONLY = 2
} CASDK2_ColorFlickerMeasMode;

typedef enum tagCSVDateFormat
{
    CSV_DATE_YYYYMMDD = 0,
    CSV_DATE_DDMMYYYY = 1,
    CSV_DATE_MMDDYYYY = 2
} CASDK2_CSVDateFormat;

typedef enum tagWaveformConditionSwitch
{
    WVFM_COND_SWITCH_ASYNC = 0,
    WVFM_COND_SWITCH_SYNC = 1
} CASDK2_WaveformConditionSwitch;

typedef enum tagWaveformSensor
{
    WVFM_SENSOR_X = 0,
    WVFM_SENSOR_Y = 1,
    WVFM_SENSOR_Z = 2
} CASDK2_WaveformSensor;

// Class declarations
class CASDK2Ca200Impl;
class CASDK2Ca200Data;
class CASDK2Ca200;
class CASDK2CasImpl;
class CASDK2CasData;
class CASDK2Cas;
class CASDK2CaImpl;
class CASDK2CaData;
class CASDK2Ca;
class CASDK2MemoryImpl;
class CASDK2MemoryData;
class CASDK2Memory;
class CASDK2ProbesImpl;
class CASDK2ProbesData;
class CASDK2Probes;
class CASDK2OutputProbesImpl;
class CASDK2OutputProbesData;
class CASDK2OutputProbes;
class CASDK2ProbeImpl;
class CASDK2ProbeData;
class CASDK2Probe;

// Struct Type Definitions
#pragma pack(push, 1)
typedef struct tagCASDK2DeviceData
{
    // (required)
    // if device is Body type: configiration type
    // if device is Probe type connected to a Body type: parent's configuration type
    CASDK2_ConfigurationType iConfigurationType;

    // (required)
    // USB/RS/BT (Windows) : "COM" + <lPortNo> + '\0'
    // USB/RS/BT (Mac) : IOCalloutDevice value + '\0'
    // ETH : <IP Address> + '\0' (ex. "192.168.1.10")
    char strTarget[64];

    // (required)
    // USB/RS/BT (Windows) : actual COM port number detected by the SearchUSBDevice
    // USB/RS (Mac) : location ID (ex. 0xFFFFFFFFFA130000 = 0xFFFFFFFFFA130000 = -99418112)
    // BT (Mac) : fixed at PORT_BT_MANUAL
    // ETH : fixed at CEthernetPort::CASDK2_ETH_TCP_PORT
    long long int lPortNo;

    // (required)
    // USB/RS/BT (Windows) : empty string
    // USB/RS (Mac) : empty string
    // BT (Mac) : MAC address + '\0' (format: "XX-XX-XX-XX-XX-XX")
    // ETH : empty string
    char strBluetoothMACAddress[32];

    // (optional)
    // 8-digit CA device serial number + '\0'
    char strSerialNo[9];

    // (optional)
    // if device is Probe type connected to a Body type: parent's serial number
    // if device is Body type: empty string
    char strParentSerialNo[9];

    // (required)
    // Probe type
    CASDK2_ProbeType iProbeType;

    // (optional)
    // True if the device is not a Body Type device
    // and device is the current display probe
    bool bIsDisplayProbe;

    // (optional)
    // user-specified CA number
    int lCaNumber;

    // (optional)
    // if device is Probe type connected to a Body type: slot number of probe [1 - 10]
    // if device is Body type: 0
    int iSlotNo;

    // (required)
    // 9 characters of firmware version number + '\0'
    // "x.xx.xxxx"
    char strFwVerNo[10];

    // (optional)
    // USB (Windows): Location Path of USB device
    // USB (Mac): Location ID
    // RS/BT/ETH: empty string
    char locationPath[1024];
} CASDK2DeviceData;

typedef void(*CASDK2CaCallback)(void* eventData, void* userData);

typedef struct tagCASDK2ExeCalZeroCBResult
{
    CASDK2Ca* pCa;
} CASDK2ExeCalZeroCBResult;

typedef struct tagCASDK2ExtlTrgrMeasCBResult
{
    CASDK2Ca* pCa;
    int result;
} CASDK2ExtlTrgrMeasCBResult;

typedef struct tagCASDK2Date
{
    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;
    tagCASDK2Date()
    {
        year = INT_INIT_VAL;
        month = INT_INIT_VAL;
        day = INT_INIT_VAL;
        hour = INT_INIT_VAL;
        min = INT_INIT_VAL;
        sec = INT_INIT_VAL;
    }
} CASDK2Date;

typedef struct tagCASDK2CommonMeasData
{
    int probeNo;
    double sx;
    double sy;
    double lv;
    double lvfl;
    double tcp;
    double duv;
    double u;
    double v;
    double X;
    double Y;
    double Z;
    double ld;
    double pe;
    double dT;
    double flickerVal;
    tagCASDK2CommonMeasData()
    {
        probeNo = INT_INIT_VAL;
        sx = DOUBLE_INIT_VAL;
        sy = DOUBLE_INIT_VAL;
        lv = DOUBLE_INIT_VAL;
        lvfl = DOUBLE_INIT_VAL;
        tcp = DOUBLE_INIT_VAL;
        duv = DOUBLE_INIT_VAL;
        u = DOUBLE_INIT_VAL;
        v = DOUBLE_INIT_VAL;
        X = DOUBLE_INIT_VAL;
        Y = DOUBLE_INIT_VAL;
        Z = DOUBLE_INIT_VAL;
        ld = DOUBLE_INIT_VAL;
        pe = DOUBLE_INIT_VAL;
        dT = DOUBLE_INIT_VAL;
        flickerVal = DOUBLE_INIT_VAL;
    }
} CASDK2CommonMeasData;

typedef struct tagCASDK2MeasData
{
    CASDK2Date date;
    int measSpeed;
    int measFreqMode;
    double intMeasFreq;
    double extMeasFreq;
    double manualMeasTime;
    int triggerMode;
    CASDK2CommonMeasData data;
    double jeitaFlickerVal;
    double jeitaFlickerFreq;
    char calibCH[3];
    char calibCHId[11];
    int calibType;
    char modelName[17];
    char serialNo[9];
    tagCASDK2MeasData()
    {
        memset(calibCH, '\0', sizeof(calibCH));
        memset(calibCHId, '\0', sizeof(calibCHId));
        memset(modelName, '\0', sizeof(modelName));
        memset(serialNo, '\0', sizeof(serialNo));
        measSpeed = INT_INIT_VAL;
        measFreqMode = INT_INIT_VAL;
        intMeasFreq = DOUBLE_INIT_VAL;
        extMeasFreq = DOUBLE_INIT_VAL;
        manualMeasTime = DOUBLE_INIT_VAL;
        triggerMode = INT_INIT_VAL;
        jeitaFlickerVal = DOUBLE_INIT_VAL;
        jeitaFlickerFreq = DOUBLE_INIT_VAL;
        calibType = INT_INIT_VAL;
    }
} CASDK2MeasData;

typedef struct tagCASDK2IntervalMetadata
{
    int logNo; // log number <1 to 100>
    int totalNumMeas; // total number of measurements available for the given log number <1 to 9999>
    CASDK2Date startDate;
    CASDK2Date endDate;
    tagCASDK2IntervalMetadata()
    {
        logNo = INT_INIT_VAL; // log number <1 to 100>
        totalNumMeas = INT_INIT_VAL; // total number of measurements available for the given log number <1 to 9999>
    }
} CASDK2IntervalMetadata;

typedef struct tagCASDK2IntervalData
{
    int measNo; // measurement number <1 to 9999>
    CASDK2IntervalMetadata metadata;
    CASDK2MeasData measdata;
    tagCASDK2IntervalData()
    {
        measNo = INT_INIT_VAL;
    }
} CASDK2IntervalData;

typedef struct tagCASDK2PackageData
{
    char errorCode[5];
    int measNo;
    CASDK2CommonMeasData data;
    tagCASDK2PackageData()
    {
        measNo = INT_INIT_VAL;
        memset(errorCode, '\0', sizeof(errorCode));
    }
} CASDK2PackageData;

#pragma pack(pop)

namespace CASDK2ErrorValues
{
    const double DOUBLE_MAX = std::numeric_limits<double>::infinity();
    const double X_Y_Z = DOUBLE_MAX;
    const double Lv_LvfL = DOUBLE_MAX;
    const double lsUser_usUser_vsUser_deUser = DOUBLE_MAX;
    const double FlickerWVFM = DOUBLE_MAX;
    const double FlickerJEITA = DOUBLE_MAX;
    const double FlickerFMA = DOUBLE_MAX;
    const double R_G_B = DOUBLE_MAX;
    const double sx_sy = DOUBLE_MAX;
    const double ud_vd = DOUBLE_MAX;
    const double Tcp_duv = DOUBLE_MAX;
    const double ld_Pe = DOUBLE_MAX;
    const double FlickerWVFM_spectrum = DOUBLE_MAX;
    const double FlickerJEITA_spectrum = DOUBLE_MAX;
}
