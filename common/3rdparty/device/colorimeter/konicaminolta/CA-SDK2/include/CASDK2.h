//
//  CASDK2.h
//
//  Copyright (c) 2016 KONICA MINOLTA, INC.
//
//

#ifndef CASDK2_H
#define CASDK2_H

#include "CASDK2Common.h"
#include "CASDK2TypeDefine.h"
#include "CASDK2ErrorDefine.h"
#include "CASDK2MemoryStatus.h"
#pragma pack(push, 1)

#define CASDK2_VERSION_A 1
#define CASDK2_VERSION_B 3
#define CASDK2_VERSION_C 1
#define CASDK2_VERSION_DEFG 2

int CASDK2_API CASDK2_GetVersion(int *lA, int *lB, int *lC, int *lDEFG);
int CASDK2_API CASDK2_IsCompatibleDLL(int lA, int lB, int lC, int lDEFG);

// CASDKCa200
class CASDK2_API CASDK2Ca200
{
public:
    CASDK2Ca200();
    ~CASDK2Ca200();
    int        AutoConnect          ();
    int        SetConfiguration     (int lNumber, const char* strConnecString, long long int lPort, int lBaudRate, int lType);
    int        get_Cas              (CASDK2Cas** pVal);
    int        get_SingleCa         (CASDK2Ca** pVal);

    int        AutoDisconnect       ();
    int        Disconnect           (int lNumber);
    int        DisconnectAll        ();
    int        SetEthernetSetting   (int lNumber, const char* strIpAddress);
    int        SetBluetoothSetting  (int lNumber, const char* strBtMacAddress);
    int        SetBluetoothSetting  (int lNumber, const char* strBtMacAddress, const char* strBtName);

private:
    CASDK2Ca200Impl* impl;
    CASDK2Ca200Data* data;
};


// CASDK2Cas
class CASDK2_API CASDK2Cas
{
    friend class CASDK2Ca200Data;
public:
    int        get_Item                                 (const char* strVal, CASDK2Ca** pVal);
    int        get_Item                                 (int lNumber, CASDK2Ca** pVal);
    int        get_Count                                (int *pVal);
    int        get_ItemOfNumber                         (int lNumber, CASDK2Ca** pVal);
    int        SendMsr                                  ();
    int        ReceiveMsr                               ();
    int        SetCaID                                  (int lNumber, const char* strID);

    int        CalZero                                  ();
    int        put_SyncMode                             (double newVal);
    int        put_DisplayMode                          (int newVal);
    int        put_AveragingMode                        (int newVal);
    int        put_BrightnessUnit                       (int newVal);
    int        put_RemoteMode                           (int newVal);
    int        put_ChannelNO                            (int newVal);
    int        put_ChannelID                            (const char* newVal);
    int        SetChannelID                             (const char* strID);
    int        Measure                                  ();

    int        put_SyncMode                             (int newVal1, double newVal2);
    int        put_ExtlTrgrMeasDelay                    (int newVal);
    int        EnableExtlTrgrMeas                       (bool bEnable, bool bForceDisable = false);
    int        SetJEITAFlickerParams                    (double fFrequency);
    int        SetJEITAFlickerParams                    (double fFrequency, int lSampleCountIndex);
    int        OpenShutterForProbes                     (int lProbeNo);
    int        CloseShutterForProbes                    (int lProbeNo);
    int        SetBaudRate                              (int newVal);
    int        SetCurrentFlickerSensitivityFactorProfile(int profile);
    int        SetFlickerSensitivityFactorAttribs       (int profile, double startFrequency, double endFrequency, double frequencyPitch, unsigned int dataCount);
    int        SetFlickerSensitivityFactorData          (int profile, unsigned int dataCount, double* data);
    int        LoadFlickerSensitivityFactorFromFile     (int profile, const char* strFileName);
    int        SetLEDDspMode                            (int lMode);
    int        SetColorFlickerMeas                      (int lNewVal);
    int        InitSettings                             ();
    int        put_NegativeValue                        (int newVal);
    int        SetMesSwitch                             (int newVal);
    int        SetWaveformParams                        (int lSampleCountIndex, int lSamplingInterval, int lXYZSensor);
    int        SetWaveformConditionSwitch               (int newVal);
    int        SetWaveformFilterType                    (int lFilter);
    int        SetLPFCutoffFrequency                    (int lLowPassParam);
private:
    CASDK2Cas(CASDK2CasImpl* impl);
    ~CASDK2Cas();

    CASDK2CasImpl* impl;
    CASDK2CasData* data;
};

// CASDK2Ca
class CASDK2_API CASDK2Ca
{
    friend class CASDK2CasData;
public:
    int        get_Probes                     (CASDK2Probes** pVal);
    int        get_OutputProbes               (CASDK2OutputProbes** pVal);
    int        get_Memory                     (CASDK2Memory** pVal);
    int        get_DisplayProbe               (char* pVal, unsigned long int* plSize);
    int        put_DisplayProbe               (const char* newVal);
    int        get_SingleProbe                (CASDK2Probe** pVal);
    int        get_SyncMode                   (double *pVal);
    int        put_SyncMode                   (double newVal);
    int        get_DisplayMode                (int *pVal);
    int        put_DisplayMode                (int newVal);
    int        get_DisplayDigits              (int *pVal);
    int        put_DisplayDigits              (int newVal);
    int        get_AveragingMode              (int *pVal);
    int        put_AveragingMode              (int newVal);
    int        get_BrightnessUnit             (int *pVal);
    int        put_BrightnessUnit             (int newVal);
    int        get_CAType                     (char* pVal, unsigned long int* plSize);
    int        get_CAVersion                  (char* pVal, unsigned long int* plSize);
    int        get_Number                     (int *pVal);
    int        get_PortID                     (char* pVal, unsigned long int* plSize);
    int        get_ID                         (char* pVal, unsigned long int* plSize);
    int        put_ID                         (const char* newVal);
    int        put_RemoteMode                 (int newVal);
    int        get_CalStandard                (int *pVal);
    int        put_CalStandard                (int newVal);
    int        CalZero                        ();
    int        Measure                        ();
    int        SetAnalogRange                 (double Range1, double Range2);
    int        GetAnalogRange                 (double *pfRange1, double *pfRange2);
    int        SetFMAAnalogRange              (double Range);
    int        GetFMAAnalogRange              (double *pfRange);
    int        SetPWROnStatus                 ();
    int        SetDisplayProbe                (int lDisplayProbe);
    int        SetAnalyzerCalMode             ();
    int        ResetAnalyzerCalMode           ();
    int        SetAnalyzerCalData             (int lClr);
    int        Enter                          ();
    int        SetLvxyCalMode                 ();
    int        ResetLvxyCalMode               ();
    int        SetLvxyCalData                 (int lClr, double fVal1, double fVal2, double fVal3);
    int        SetExeCalZeroCallback          (const CASDK2CaCallback pCallback, void* pUserData);
    int        Command                        (const char* strCommand, char* strResponse, unsigned long int* plSize);

    int        put_SyncMode                   (int newVal1, double newVal2);
    int        get_SyncMode                   (int* pVal1, double *pVal2);

    int        EnableExtlTrgrMeas             (bool bEnable);
    int        SetTrgrMeasReceivedCallback    (const CASDK2CaCallback pCallback, void* pUserData);
    int        put_ExtlTrgrMeasDelay          (int newVal);
    int        get_ExtlTrgrMeasDelay          (int* pVal);
    int        get_ExtlTrgrMeasMode           (bool* pVal);

    int        SetJEITAFlickerParams          (double fFrequency);
    int        SetJEITAFlickerParams          (double fFrequency, int lSampleCountIndex);
    int        GetJEITAFlickerParams          (double* fFrequency, int* lSampleCountIndex);

    int        IsWaveformSupported            ();
    int        SetWaveformParams              (int lSampleCountIndex, int lSamplingInterval, int lXYZSensor);
    int        GetWaveformParams              (int* lSampleCountIndex, int* lSamplingInterval, int* lXYZSensor);
    int        SetWaveformConditionSwitch     (int newVal);
    int        GetWaveformConditionSwitch     (int* pVal);
    int        SetWaveformFilterType          (int lFilter);
    int        GetWaveformFilterType          (int *lFilter);
    int        SetLPFCutoffFrequency          (int lLowPassParam);
    int        GetLPFCutoffFrequency          (int *lLowPassParam);

    int        GetDeviceStatus                ();

    int        GetIntervalLogNum              (int* pVal);
    int        GetIntervalMeasMode            (int* pVal);
    int        SetIntervalMeasMode            (int newVal);
    int        GetIntervalMeasParams          (int* lMeasInterval, int* lNumMeas, int* lChangeWarning, double* lThreshold);
    int        SetIntervalMeasParams          (int lMeasInterval, int lNumMeas, int lChangeWarning, double lThreshold);
    int        GetIntervalMetadata            (int lLogNo, CASDK2IntervalMetadata* intervalMetadata);
    int        GetIntervalData                (int lLogNo, int lProbeNo, int lLogDataCnt, CASDK2IntervalData* intervalData, unsigned long int* plSize);
    int        DeleteIntervalData             ();
    int        SaveIntervalDataToCsv          (int lLogNo, const char* strFileName);
    int        SaveIntervalDataToCsv          (int lLogNo, const char* strFileName, int dateFormat);
    int        CancelSaveIntervalDataToCsv    ();

    int        PackageMeasure                 (int lNumber);

    int        GetAveragingMeasMode           (int* lColorNum, int* lFlickerNum);
    int        SetAveragingMeasMode           (int lColorNum, int lFlickerNum);

    int        GetAutoStoreMeasData           (int* pVal);
    int        SetAutoStoreMeasData           (int newVal);
    int        GetMeasurementData             (int lChannelNo, CASDK2MeasData* measData, unsigned long int* plSize);
    int        DeleteMeasurementData          (int lChannelNo);
    int        SaveMeasurementDataToCsv       (int lNumber, int lNumType, const char * strFileName);
    int        SaveMeasurementDataToCsv       (int lNumber, int lNumType, const char * strFileName, int dateFormat);

    int        GetAutoZeroCal                 (int* pVal);
    int        SetAutoZeroCal                 (int newVal);
    int        OpenShutterForProbes           (int lProbeNo);
    int        CloseShutterForProbes          (int lProbeNo);

    int        GetBodyDate                    (int* year, int* month, int* day, int* hour, int* minute, int* second, int* lDateFormat);
    int        SetBodyDate                    (int newVal);
    int        GetLCDBrightness               (int* pOnOff, int* pBrightness);
    int        SetLCDBrightness               (int lOnOff, int lBrightness);
    int        GetLanguage                    (int* pVal);
    int        SetLanguage                    (int newVal);
    int        GetBuzzerMode                  (int* pVal);
    int        SetBuzzerMode                  (int newVal);

    int        GetAutoPowerSavingMode         (int* lMode, int* lTime);
    int        SetAutoPowerSavingMode         (int lMode, int lTime);
    int        GetBatteryStatus               (int* pVal);
    int        GetPowerSupplyStatus           (int* pVal);

    int        GetBaudRate                    (int* pVal);
    int        SetBaudRate                    (int newVal);
    int        GetConnectionAddress           (char* pVal, unsigned long int* plSize);
    int        GetEthConnParams               (int* lDhcpVal, char* strIpAddress, char* strSubnetMask, char* strGateway);
    int        SetEthConnParams               (int lDhcpVal, const char* strIpAddress, const char* strSubnetMask, const char* strGateway);
    int        GetBluetoothConnMode           (int* pVal);
    int        SetBluetoothConnMode           (int newVal);

    int        SetLEDDspMode                  (int lMode);
    int        GetLEDDspMode                  (int* lMode);
    int        SetColorFlickerMeas            (int lNewVal);
    int        GetColorFlickerMeas            (int* pVal);

    int        CalibMeasure                   (int lClr);

    int        InitSettings                   ();

    int        get_TypeNO                     (int *pVal);
    int        get_SerialNO                   (char* pVal, unsigned long int* plSize);

    int        GetCurrentFlickerSensitivityFactorProfile    (int* profile);
    int        SetCurrentFlickerSensitivityFactorProfile    (int profile);
    int        GetFlickerSensitivityFactorAttribs           (int profile, double* startFrequency, double* endFrequency, double* frequencyPitch, unsigned int* dataCount);
    int        SetFlickerSensitivityFactorAttribs           (int profile, double startFrequency, double endFrequency, double frequencyPitch, unsigned int dataCount);
    int        GetFlickerSensitivityFactorData              (int profile, unsigned int dataCount, double* data);
    int        SetFlickerSensitivityFactorData              (int profile, unsigned int dataCount, double* data);
    int        SaveFlickerSensitivityFactorToFile           (int profile, const char* strFileName);
    int        LoadFlickerSensitivityFactorFromFile         (int profile, const char* strFileName);

    int        put_NegativeValue               (int newVal);
    int        get_NegativeValue               (int *pVal);

    int        SetMesSwitch                    (int newVal);
    int        GetMesSwitch                    (int *pVal);

private:
    CASDK2Ca(CASDK2CaImpl* impl);
    ~CASDK2Ca();

    CASDK2CaImpl* impl;
    CASDK2CaData* data;
};

// CASDK2Memory
class CASDK2_API CASDK2Memory
{
    friend class CASDK2CaData;
public:
    int        get_ChannelNO             (int *pVal);
    int        put_ChannelNO             (int newVal);
    int        get_ChannelID             (char* pVal, unsigned long int* plSize);
    int        put_ChannelID             (const char* newVal);
    int        get_ChannelID             (int lProbeNo, char* pVal, unsigned long int* plSize);
    int        put_ChannelID             (int lProbeNo, const char* newVal);

    int        GetReferenceColor         (const char* strID, double* frclrx, double* frclry, double* frclrLv);
    int        SetChannelID              (const char* strID);
    int        SetChannelID              (int lProbeNo, int lCHNo, const char* strID);
    int        GetChannelID              (int lProbeNo, int lCHNo, char* pVal, unsigned long int* plSize);
    int        GetMemoryStatus           (int lNumber, int* lCProbeSNO, int* lRProbeSNO, int* lCalMode);
    int        CheckCalData              (int lNumber, const char* strFileName, int* plResult);
    int        CopyToFile                (int lNumber, const char* strFileName);
    int        CopyFromFile              (int lNumber, const char* strFileName);

    int        GetChannelCmt             (int lProbeNo, int lCHNo, char * pVal, unsigned long int * plSize);
    int        SetChannelCmt             (int lProbeNo, int lCHNo, const char * newVal);
    int        get_ChannelDateTime       (int lNumber, int *year, int *month, int *day, int *hour, int *minute, int *second);

    int        get_ChannelNO             (int lProbeNo, int* pVal);
    int        put_ChannelNO             (int lProbeNo, int newVal);

    int        InitMemChannelData        (int lNumber, int lProbe);
    int        InitColorRefData          (int lNumber, int lProbe);
    int        GetMemoryCHData           (int lProbeNo, int lCH, CASDK2MemoryStatus* pMemCHData);
    int        GetMemoryStatusOfAllCH    (int lProbeNo);
    int        GetMemoryStatusOfAllCH    (int lProbeNo, int lEndCHNo);
private:
    CASDK2Memory(CASDK2MemoryImpl* impl);
    ~CASDK2Memory();

    CASDK2MemoryImpl* impl;
    CASDK2MemoryData* data;
};

// CASDK2Probes
class CASDK2_API CASDK2Probes
{
    friend class CASDK2CaData;
public:
    int        get_Item            (const char* bstrVal, CASDK2Probe** pVal);
    int        get_Item            (int lNumber, CASDK2Probe** pVal);
    int        get_ItemOfNumber    (int lNumber, CASDK2Probe** pVal);
    int        get_Count           (int *pVal);
    int        SetProbeID          (int lNumber, const char* strID);
private:
    CASDK2Probes(CASDK2ProbesImpl* impl);
    ~CASDK2Probes();

    CASDK2ProbesImpl* impl;
    CASDK2ProbesData* data;
};

// CASDK2OutputProbes
class CASDK2_API CASDK2OutputProbes
{
    friend class CASDK2CaData;
public:
    int        get_Item            (const char* bstrVal, CASDK2Probe** pVal);
    int        get_Item            (int lNumber, CASDK2Probe** pVal);
    int        get_Count           (int *pVal);
    int        get_ItemOfNumber    (int lNumber, CASDK2Probe** pVal);
    int        Add                 (const char* strID);
    int        AddAll              ();
    int        RemoveAll           ();
    int        Clone               (CASDK2OutputProbes** pVal);

    static int    DestroyClone     (CASDK2OutputProbes** pVal);
private:
    CASDK2OutputProbes(CASDK2OutputProbesImpl* impl);
    ~CASDK2OutputProbes();

    CASDK2OutputProbesImpl* impl;
    CASDK2OutputProbesData* data;
};

// CASDK2Probe
class CASDK2_API CASDK2Probe
{
    friend class CASDK2ProbesData;
    friend class CASDK2OutputProbesData;
public:

    int        get_X                    (double *pVal);
    int        get_Y                    (double *pVal);
    int        get_Z                    (double *pVal);
    int        get_Lv                   (double *pVal);
    int        get_LvfL                 (double *pVal);
    int        get_sx                   (double *pVal);
    int        get_sy                   (double *pVal);
    int        get_ud                   (double *pVal);
    int        get_vd                   (double *pVal);
    int        get_T                    (double *pVal);
    int        get_duv                  (double *pVal);
    int        get_R                    (double *pVal);
    int        get_G                    (double *pVal);
    int        get_B                    (double *pVal);
    int        get_LsUser               (double *pVal);
    int        get_usUser               (double *pVal);
    int        get_vsUser               (double *pVal);
    int        get_dEUser               (double *pVal);
    int        get_FlckrJEITA           (double *pVal);
    int        get_FlckrFMA             (double *pVal);
    int        get_Number               (int *pVal);
    int        get_ID                   (char* pVal, unsigned long int* plSize);
    int        put_ID                   (const char* newVal);
    int        get_SerialNO             (char* pVal, unsigned long int* plSize);
    int        get_RD                   (int *pVal);
    int        get_RJEITA               (int *pVal);
    int        get_RFMA                 (int *pVal);
    int        get_RAD                  (int *pVal);
    int        GetSpectrum              (double Frequency, double *pSpectrumValue);
    int        get_TypeName             (char* pVal, unsigned long int* plSize);
    int        get_TypeNO               (int *pVal);

    int        get_ld                   (double *pVal);
    int        get_Pe                   (double *pVal);
    int        SetMeasurementRange      (int lMeasurementType, int lRangeMode, int lRangeNum);
    int        GetMeasurementRange      (int lMeasurementType, int* lRangeMode, int* lRangeNum);
    int        GetJEITAFlickerData      (double* pfFlickerData, unsigned long int* plSize);
    int        get_CurrDeltaTemp        (double* fCurrentDeltaTemp);
    int        get_LastMeasDeltaTemp    (double* fLastMeasDeltaTemp);
    int        get_ConnectionStatus     (bool* bIsConnected);
    int        get_ZeroCalStatus        (int* lZeroCalStatus);

    int        get_DisplayMode          (int *pVal);
    int        put_DisplayMode          (int newVal);
    int        put_SyncMode             (int newVal1, double newVal2);
    int        get_SyncMode             (int* pVal1, double *pVal2);
    int        get_AveragingMode        (int *pVal);
    int        put_AveragingMode        (int newVal);
    int        get_CAVersion            (char* pVal, unsigned long int* plSize);

    int        InitSettings();

    int        GetPackageData           (CASDK2PackageData* pData, unsigned long int* plSize);

    int        SetJEITAFlickerParams    (double fFrequency);
    int        SetJEITAFlickerParams    (double fFrequency, int lSampleCountIndex);
    int        GetJEITAFlickerParams    (double *fFrequency, int* lSampleCountIndex);

    int        IsWaveformSupported      ();
    int        SetWaveformParams        (int lSampleCountIndex, int lSamplingInterval, int lXYZSensor);
    int        GetWaveformParams        (int *lSampleCountIndex, int *lSamplingInterval, int *lXYZSensor);
    int        SetWaveformConditionSwitch    (int newVal);
    int        GetWaveformConditionSwitch    (int* pVal);

    int        get_RWaveform            (int *pVal);
    int        get_WaveformDataNumber   (unsigned long int *pVal);
    int        GetWaveformData          (double* pfWfData, double* pfFilteredWfData, unsigned long int* plSize, double* pfTimeSec);
    int        get_PeakFrequency        (double *pVal);
    int        GetFlickerDerivedParams  (int *lTotalSpectrumCount, double *fMinFreq, double *fMaxFreq, double *fFreqPitch);

    int        GetSensorOfFlickerType   (int lDisplayMode, int* lSensor);
private:
    CASDK2Probe(CASDK2ProbeImpl* impl);
    ~CASDK2Probe();

    CASDK2ProbeImpl* impl;
    CASDK2ProbeData* data;
};

// Global functions
int CASDK2_API CASDK2_GetLocalizedErrorMsgFromErrorCode               (int langID, int lErr, wchar_t* pVal, unsigned long int* plSize);
int CASDK2_API CASDK2_GetLocalizedErrorMsgFromMeasurementErrorCode    (int langID, int lErr, wchar_t* pVal, unsigned long int* plSize);

// CASDK2CalculateFunctions
class CASDK2_API CASDK2CalculateFunctions
{
public:
    static int    SetFlickerWeights     (int lProfile, double *plWeights, unsigned int lFreqBegin100, unsigned int lFreqEnd100, unsigned int lFreqPitch100);
    static int    SetFilterType         (int lFilterType);
    static int    SetSamplingCondition  (double fFrequency, unsigned int n);
    static int    SetCutoffFrequency    (double fFrequency);
    static int    SetCircuit            (int lCircuit);
    static int    FilterWaveform        (int lRange, double fSampFreq, unsigned int lDataLen, double* pfWfData, double* pfFilteredWfData, double* pfFlicker, double* pfPeakFreq, double* pfPowerSpectrum, unsigned int* plPowerSpectrumLen, double* pfMinFreq, double* pfMaxFreq, double* pfFreqPitch);
    static int    CalculateFlicker      (int lProfile, int lRange, double* pfData, unsigned int lDataLen, double fDarkData, double fSampFreq, double* pfFlickerJEITA, double* pfPeakFreq, double* pfPowerSpectrum, unsigned int* plPowerSpectrumLen);
    static int    CalculateColor        (double dX, double dY, double dZ, double* pfXW, double* pfYW, double* pfZW, double *pfsx, double* pfsy, double* pfsLv, double* pfLvfL, double* pfud, double* pfvd, double* plT, double* pfduv, double* pfLsUser, double* pfusUser, double* pfvsUser, double* pfdEUser, double* pfdwl, double* pfpe);
};

// CASDK2Discovery
class CASDK2_API CASDK2Discovery
{
public:
    static int SearchAllUSBDevices     (CASDK2DeviceData** pResultArray, unsigned long int* plSize);
    static int SearchAllBTDevices      (CASDK2DeviceData** pResultArray, unsigned long int* plSize);
    static int SearchAllRSDevices      (CASDK2DeviceData** pResultArray, unsigned long int* plSize);
    static int SearchAllIPDevices      (CASDK2DeviceData** pResultArray, unsigned long int* plSize);
    static int DestroyListOfDevices    (CASDK2DeviceData** pResultArray);
    static int IsDeviceConnected       (const CASDK2DeviceData *device, bool *result);
};

#pragma pack(pop)
#endif    //CASDK2_H