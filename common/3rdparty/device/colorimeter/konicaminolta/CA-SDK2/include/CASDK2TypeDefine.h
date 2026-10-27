//
//  CASDK2TypeDefine.h
//
//  Copyright (c) 2016 KONICA MINOLTA, INC.
//
//
#pragma once
#include <limits>
#include <cstring>
#include <string>
#define INT_INIT_VAL -1
#define DOUBLE_INIT_VAL -1.0

typedef enum tagPortNumbers
{
    PORT_USB = 0,
    PORT_USB_AUTO = 0,
    PORT_USB_MIN = 0,
    PORT_USB_MAX = 4096,

    PORT_ETH_AUTO = 0,
    PORT_ETH_MANUAL = 1,
    PORT_ETH_MIN = 0,
    PORT_ETH_MAX = 1,

    PORT_COMM_MIN = 1,
    PORT_COMM_MAX = 4096,

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
    DSP_WVFMH = 13,
    DSP_VRR = 14,
    DSP_VRRH = 15,
} CASDK2_DisplayMode;

typedef enum tagSensitivityFormat
{
    SENSITIVITY_FORMAT_GAIN_ONLY = 0,
    SENSITIVITY_FORMAT_GAIN_PHASE = 1,
    SENSITIVITY_FORMAT_COMPLEX = 2,
} CASDK2_SensitivityFormat;

typedef enum tagSamplingNumberMode
{
    SAMPLING_NUM_POW2 = 0,
    SAMPLING_NUM_OPTIONAL = 1,

} CASDK2_SamplingNumberMode;

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
    MEAS_TYPE_AFE [[deprecated]] = 0,
    MEAS_TYPE_ADC [[deprecated]] = 1,
    MEAS_TYPE_COLOR = 0,
    MEAS_TYPE_FLICKER = 1,
    MEAS_TYPE_FLICKER_XYZ = 2,
} CASDK2_MeasurementType;

typedef enum tagMeasurementRange
{
    MEAS_RANGE_AUTO = 0,
    MEAS_RANGE_FIXED = 1,
    MEAS_RANGE_PREDICTION_OFF = 2
} CASDK2_MeasurementRange;

typedef enum tagMeasurementCircuit
{
    MEAS_CIRCUIT_AFE = 0,
    MEAS_CIRCUIT_ADC = 1,
} CASDK2_MeasurementCircuit;

typedef enum tagSampleCount
{
    SAMPLE_COUNT_64 = 6,
    SAMPLE_COUNT_128 = 7,
    SAMPLE_COUNT_256 = 8,
    SAMPLE_COUNT_512 = 9,
    SAMPLE_COUNT_1024 = 10,
    SAMPLE_COUNT_2048 = 11,
    SAMPLE_COUNT_4096 = 12,
    SAMPLE_COUNT_8192 = 13,
    SAMPLE_COUNT_16384 = 14,
    SAMPLE_COUNT_32768 = 15,
    SAMPLE_COUNT_65536 = 16,
    SAMPLE_COUNT_131072 = 17,
    SAMPLE_COUNT_262144 = 18,
} CASDK2_SampleCount;

typedef enum tagWaveformFilter
{
    FILTER_NONE = 0,
    FILTER_JEITA = 1,
    FILTER_LOWPASS = 2,
    FILTER_USER = 3,
    FILTER_IEC = 4,
    FILTER_ICDM = 5,
    FILTER_CIE_TN_006_2016 = 6,
    FILTER_CIE_249_2022 = 7,
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
    AVRG_AUTO = 3,
    AVRG_CMP_AUTO = 4
} CASDK2_AveragingMode;

typedef enum tagHalfIntegrationMode
{
    HALFINT_OFF = 0,
    HALFINT_ON = 1,
} CASDK2_HalfIntegrationMode;

typedef enum tagVsyncFixMode
{
    VSYNCFIX_OFF = 0,
    VSYNCFIX_ON = 1,
} CASDK2_VsyncFixMode;

typedef enum tagExtSignalVoltageMode
{
    EXTSIGNALVOLT_3_3 = 0,
    EXTSIGNALVOLT_1_8 = 1,
} CASDK2_ExtSignalVoltageMode;

typedef enum tagLightHistory
{
    HISTORY_OFF = 0,
    HISTORY_ON = 1,
} CASDK2_LightHistory;

typedef enum tagCutoffFrequencyMode
{
    CUTOFF_AUTO	= 0,
    CUTOFF_MANUAL = 1,
} CASDK2_CutoffFrequencyMode;

typedef enum tagJeitaMeasurementMode
{
    JEITA_FREQ_RESOLUTION = 0,
    JEITA_FUNDAMENTAL_FREQ = 1,
} CASDK2_JeitaMeasurementMode;

typedef enum tagFlickerPredictionCircuit
{
    FLICKER_PRED_CIRCUIT_FLICKER = 1,
    FLICKER_PRED_CIRCUIT_COLOR = 2,
} CASDK2_FlickerPredictionCircuit;

typedef enum tagCutoffFrequency
{
    CUTOFF_1KHz	= 1,
    CUTOFF_5KHz	= 5,
    CUTOFF_20KHz = 20,
    CUTOFF_NONE	= 0,
} CASDK2_CutoffFrequency;

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

typedef enum tagWaveformDataParameter
{
    PARAM_X = 0,
    PARAM_Y = 1,
    PARAM_Z = 2,
    PARAM_X_LOW = 3,
    PARAM_Y_LOW = 4
} CASDK2_WaveformDataParameter;

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
    //Non-VP probe
    PROBETYPE_CA_804 = 804,
    PROBETYPE_CA_805 = 805,
    PROBETYPE_CA_806 = 806,
    PROBETYPE_CA_807 = 807,
    PROBETYPE_CA_P403B = 808,
    PROBETYPE_CA_P405B = 809,
    PROBETYPE_CA_P427 = 810,
    PROBETYPE_CA_P427H = 811,
    PROBETYPE_CA_P427C = 814,
    PROBETYPE_CA_815 = 815,
    PROBETYPE_CA_816 = 816,
    PROBETYPE_CA_817 = 817,
    PROBETYPE_CA_818 = 818,
    PROBETYPE_CA_819 = 819,
    PROBETYPE_CA_VP410H = 820,
    PROBETYPE_CA_821 = 821,
    PROBETYPE_CA_824 = 824,
    PROBETYPE_CA_825 = 825,
    PROBETYPE_CA_826 = 826,
    PROBETYPE_CA_827 = 827,
    PROBETYPE_CA_828 = 828,
    PROBETYPE_CA_829 = 829,
    PROBETYPE_CA_MP410 = 830,
    PROBETYPE_CA_MP410H = 831,
    PROBETYPE_CA_834 = 834,
    PROBETYPE_CA_835 = 835,
    PROBETYPE_CA_836 = 836,
    PROBETYPE_CA_837 = 837,
    PROBETYPE_CA_838 = 838,
    PROBETYPE_CA_839 = 839,
    PROBETYPE_CA_P410 = 890,
    PROBETYPE_CA_P410H = 891,
    PROBETYPE_CA_P410C = 894,
    PROBETYPE_CA_895 = 895,
    PROBETYPE_CA_896 = 896,
    PROBETYPE_CA_897 = 897,
    PROBETYPE_CA_898 = 898,
    PROBETYPE_CA_899 = 899,
    //VP probe
    PROBETYPE_CA_VP427A = 800,
    PROBETYPE_CA_VP410A = 801,
    PROBETYPE_CA_VP427 = 840,
    PROBETYPE_CA_VP402 = 841,
    PROBETYPE_CA_VP_844 = 844,
    PROBETYPE_CA_VP_845 = 845,
    PROBETYPE_CA_VP_846 = 846,
    PROBETYPE_CA_VP_847 = 847,
    PROBETYPE_CA_VP_848 = 848,
    PROBETYPE_CA_VP_849 = 849,
    PROBETYPE_CA_VP410 = 850,
    PROBETYPE_CA_VP410T = 851,
    PROBETYPE_CA_VP_854 = 854,
    PROBETYPE_CA_VP_855 = 855,
    PROBETYPE_CA_VP_856 = 856,
    PROBETYPE_CA_VP_857 = 857,
    PROBETYPE_CA_VP_858 = 858,
    PROBETYPE_CA_VP_859 = 859,
    PROBETYPE_CA_VP404 = 860,
    PROBETYPE_CA_VP_861 = 861,
    PROBETYPE_CA_VP_864 = 864,
    PROBETYPE_CA_VP_865 = 865,
    PROBETYPE_CA_VP_866 = 866,
    PROBETYPE_CA_VP_867 = 867,
    PROBETYPE_CA_VP_868 = 868,
    PROBETYPE_CA_VP_869 = 869,
    PROBETYPE_CA_VP_870 = 870,
    PROBETYPE_CA_VP_871 = 871,
    PROBETYPE_CA_VP_874 = 874,
    PROBETYPE_CA_VP_875 = 875,
    PROBETYPE_CA_VP_876 = 876,
    PROBETYPE_CA_VP_877 = 877,
    PROBETYPE_CA_VP_878 = 878,
    PROBETYPE_CA_VP_879 = 879,
    PROBETYPE_CA_VP_880 = 880,
    PROBETYPE_CA_VP_881 = 881,
    PROBETYPE_CA_VP_884 = 884,
    PROBETYPE_CA_VP_885 = 885,
    PROBETYPE_CA_VP_886 = 886,
    PROBETYPE_CA_VP_887 = 887,
    PROBETYPE_CA_VP_888 = 888,
    PROBETYPE_CA_VP_889 = 889,
    //Non-VP LowerPrice probe
    PROBETYPE_CA_LP_802 = 802,
    PROBETYPE_CA_LP_803 = 803,
    PROBETYPE_CA_LP_P427 = 812,
    PROBETYPE_CA_LP_P427H = 813,
    PROBETYPE_CA_LP_822 = 822,
    PROBETYPE_CA_LP_823 = 823,
    PROBETYPE_CA_LP_MP410 = 832,
    PROBETYPE_CA_LP_MP410H = 833,
    //VP LowerPrice probe
    PROBETYPE_CA_LP_VP427 = 842,
    PROBETYPE_CA_LP_VP_843 = 843,
    PROBETYPE_CA_LP_VP410 = 852,
    PROBETYPE_CA_LP_VP_853 = 853,
    PROBETYPE_CA_LP_VP_862 = 862,
    PROBETYPE_CA_LP_VP_863 = 863,
    PROBETYPE_CA_LP_VP_872 = 872,
    PROBETYPE_CA_LP_VP_873 = 873,
    PROBETYPE_CA_LP_VP_882 = 882,
    PROBETYPE_CA_LP_VP_883 = 883,
    PROBETYPE_CA_LP_P410 = 892,
    PROBETYPE_CA_LP_P410H = 893,
    PROBETYPE_CA_DP40 = 100,
	//CA510
    PROBETYPE_CA_830_527S = 1830,
	PROBETYPE_CA_840_527 = 1840,
	PROBETYPE_CA_850_510 = 1850,
} CASDK2_ProbeType;

typedef enum tagRemoteMode
{
    REMOTE_OFF = 0,
    REMOTE_ON = 1,
    REMOTE_LOCK = 2
} CASDK2_RemoteMode;

// The interpolation mode is managed as an int instead of a bool,
// considering the possibility of adding more interpolation methods in the future.
typedef enum tagInterpolationMode
{
    INTERPOLATION_OFF = 0,
    INTERPOLATION_ON = 1,
} CASDK2_InterpolationMode;

// Color Calibration
typedef enum tagCalibrationColor
{
    CAL_RED = 0,
    CAL_GREEN = 1,
    CAL_BLUE = 2,
    CAL_WHITE = 3
} CASDK2_CalibrationColor;

typedef enum tagStimulusSensor
{
    SENSOR_X = 0,
    SENSOR_Y = 1,
    SENSOR_Z = 2,
} CASDK2_StimulusSensor;

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

typedef enum tagCalibrationType
{
    CALIBRATION_FACTORY = 0,
    CALIBRATION_SINGLE = 1,
    CALIBRATION_RGBW = 2,
    CALIBRATION_CORRECTION = 3,
} CASDK2_CalibrationType;
typedef enum tagWaveformSensor
{
    WVFM_SENSOR_X = 0,
    WVFM_SENSOR_Y = 1,
    WVFM_SENSOR_Z = 2,
    WVFM_SENSOR_XYZ = 3, // for WCR value
    WVFM_SENSOR_SX = 3,
    WVFM_SENSOR_SY = 4
} CASDK2_WaveformSensor;

typedef enum tagCalibMeasureNDModes
{
    CALIBND_PREMEASURE = 0,
    CALIBND_MEASURE = 1,
} CASDK2_CalibMeasureNDMode;

typedef enum tagCalibMeasureModes
{
    CALIB_ND_PREMEASURE = 0,
    CALIB_ND_MEASURE = 1,
    CALIB_ACCURATE_MEASURE = 3,
} CASDK2_CalibMeasureModes;

typedef enum tagNegativeWarningOutputMode
{
    NEGATIVEWARNING_OUTPUTMODE_OFF = 0,
    NEGATIVEWARNING_OUTPUTMODE_ON = 1,
}CASDK2_NegativeWarningOutputMode;

typedef enum tagSVMerror
{
    SVM_OK = 0,
    SVM_WARN = 1,
    SVM_PARAM_ERROR = 2,
    SVM_CALC_ERROR = 3,
    SVM_ERROR = 4,
} CASDK2_SVM_ERROR;

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

typedef struct tagCASDK2ExtlTrgrMeasReplyCBResult
{
    CASDK2Ca* pCa;
    char *reply;
    int size;
} CASDK2ExtlTrgrMeasReplyCBResult;

typedef struct tagCASDK2LogSaveCBResult
{
    CASDK2Ca* pCa;
    int dataNum;
} CASDK2LogSaveCBResult;

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
    const double FlickerVRR = DOUBLE_MAX;
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