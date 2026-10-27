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

int CASDK2_API CASDK2_GetVersion(int *lA, int *lB, int *lC, int *lDEFG);
int CASDK2_API CASDK2_IsCompatibleDLL(int lA, int lB, int lC, int lDEFG);

// CASDKCa200
class CASDK2_API CASDK2Ca200
{
public:
    CASDK2Ca200();
    ~CASDK2Ca200();
    CASDK2Ca200(const CASDK2Ca200&) = delete;
    CASDK2Ca200& operator=(const CASDK2Ca200&) = delete;
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
    int        CalZero                                  (bool bNoShutterControl);
    int        SetCutoffFrequencyMode                   (int lMode, int lCutoffFrequency);

    int        put_SyncMode                             (double newVal);
    int        put_DisplayMode                          (int newVal);
    int        put_AveragingMode                        (int newVal);
    int        put_BrightnessUnit                       (int newVal);
    int        put_RemoteMode                           (int newVal);
    int        put_ChannelNO                            (int newVal);
    int        put_ChannelID                            (const char* newVal);
    int        SetChannelID                             (const char* strID);
    int        Measure                                  ();
    int        Measure                                  (int samplingNumber);

    int        put_SyncMode                             (int newVal1, double newVal2);
    int        put_ExtlTrgrMeasDelay                    (int newVal);
    int        EnableExtlTrgrMeas                       (bool bEnable, bool bForceDisable = false);
    int        SetJEITAFlickerParams                    (double fFrequency);
    int        SetJEITAFlickerParams                    (double fFrequency, int lSampleCountIndex);
    int	       SetJEITAFlickerParams                    (int lMeasurementMode, double fFrequency);
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
    int        SetWaveformParams                        (int lSamplingNumberMode, int lSampleCountIndex, double fFrequency, int lXYZSensor, int lSync);
    int        SetWaveformConditionSwitch               (int newVal);
    int        SetWaveformFilterType                    (int lFilter);
    int	       SetVRRFlickerParams                      (int lCircuit, double fFrequency, int lSampleNumber);
    int	       SetVRRFlickerTrendInterval               (double lInterval);
    int        SetLPFCutoffFrequency                    (int lLowPassParam);
    int        DetectEmissionFrequency                  ();
    int        DetectVSync                              (double newVal);
    int        put_IntegrationMode                      (int newVal);
    int        put_IntegrationMode                      (int newVal1, int newVal2);
    int        put_ExternalSignalVoltage                (int newVal);
    int	       SetFlickerSensitivityFactorData          (int profile, unsigned int dataCount, int dataFormat, double* data1, double* data2);

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
    int        get_RemoteMode                 (int *pVal);
    int        put_RemoteMode                 (int newVal);
    int        get_CalStandard                (int *pVal);
    int        put_CalStandard                (int newVal);
    int        CalZero                        ();
    int        Measure                        ();
    int        Measure                        (int samplingNumber);
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
    int        SetTrgrMeasReplyCallback       (const CASDK2CaCallback pCallback, void* pUserData);
    int        SetLogSaveCallback             (const CASDK2CaCallback pCallback, void* pUserData);
    int        put_ExtlTrgrMeasDelay          (int newVal);
    int        get_ExtlTrgrMeasDelay          (int* pVal);
    int        get_ExtlTrgrMeasMode           (bool* pVal);

    int        SetJEITAFlickerParams          (double fFrequency);
    int        SetJEITAFlickerParams          (double fFrequency, int lSampleCountIndex);
    int        GetJEITAFlickerParams          (double* fFrequency, int* lSampleCountIndex);

    int        IsWaveformSupported            ();
    int        IsExpandedSamplingNumberSupported();
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
    int        CalibMeasure                   (int lClr, int lTone);
    int        CalibMeasure                   (int lMeasure, int lClr, int lTone, int lAveragingTimes, int* plJudgment);

    int        InitSettings                   ();

    int        get_TypeNO                     (int *pVal);
    int        get_SerialNO                   (char* pVal, unsigned long int* plSize);

    int        GetCurrentFlickerSensitivityFactorProfile    (int* profile);
    int        SetCurrentFlickerSensitivityFactorProfile    (int profile);
    int        GetFlickerSensitivityFactorAttribs           (int profile, double* startFrequency, double* endFrequency, double* frequencyPitch, unsigned int* dataCount);
    int        SetFlickerSensitivityFactorAttribs           (int profile, double startFrequency, double endFrequency, double frequencyPitch, unsigned int dataCount);
    int        GetFlickerSensitivityFactorData              (int profile, unsigned int dataCount, double* dataGain);
    int        SetFlickerSensitivityFactorData              (int profile, unsigned int dataCount, double* dataGain);
    int        GetFlickerSensitivityFactorData              (int profile, unsigned int dataCount, int *dataFormat, double* data1, double* data2);
    int        SetFlickerSensitivityFactorData              (int profile, unsigned int dataCount, int dataFormat, double* data1, double* data2);
    int        SaveFlickerSensitivityFactorToFile           (int profile, const char* strFileName);
    int        LoadFlickerSensitivityFactorFromFile         (int profile, const char* strFileName);

    int        put_NegativeValue                (int newVal);
    int        get_NegativeValue                (int *pVal);

    int        SetMesSwitch                     (int newVal);
    int        GetMesSwitch                     (int *pVal);

    int        DetectEmissionFrequency          ();
    int        DetectVSync                      (double newVal);
    int        put_IntegrationMode              (int newVal);
    int        put_IntegrationMode              (int newVal1, int newVal2);
    int        get_IntegrationMode              (int *pVal);
    int        get_IntegrationMode              (int *pVal1, int *pVal2);
    int        put_ExternalSignalVoltage        (int newVal);
    int        get_ExternalSignalVoltage        (int *pVal);
    int        SetLHMode                        (int lHMod);
    int        GetLHMode                        (int *pHMod);
    int        SetVRRFlickerParams              (int lCircuit, double fFrequency, int lSampleNumber);
    int        GetVRRFlickerParams              (int lCircuit, double *pFrequency, int *pSampleNumber);
    int        SetVRRFlickerTrendInterval       (double lInterval);
    int        GetVRRFlickerTrendInterval       (double* pInterval);
    int        GetVRRFlickerValue               (double* pVRRFlickerValue);
    int        get_VRRFTrendDataNumber          (unsigned long int* pVal);
    int        GetVRRFlickerTrend               (double* pfVRRFlickerTrend, unsigned long int* plSize);
    int        SetJEITAFlickerParams            (int lMeasurementMode, double fFrequency);
    int        GetJEITAFlickerParams            (int *plMeasurementMode, double* pfFrequency);
    int        SetWaveformParams                (int lSamplingNumberMode, int lSampleCountIndex, double fFrequency, int lXYZSensor, int lSync);
    int        GetWaveformParams                (int *lSamplingNumberMode, int *lSampleCountIndex, double *fFrequency, int *lXYZSensor, int *lSync);
    int        SetCutoffFrequencyMode           (int lMode, int lCutoffFrequency);
    int        GetCutoffFrequencyMode           (int *lMode, int *lCutoffFrequency);
    int        CalZero                          (bool bNoShutterControl);
    int        GetExtSyncFrequency              (double *extFrequency);
    int        GetNDCalibKind                   (int lChNo, int *pKind, int *pKindND);
    int        GetNDCalibStatus                 (int *pR, int *pG, int *pB, int *pW);
    int        CalibMeasure                     (int lMeasure, int lClr, int lAveragingTimes, int *pJudgment, double *pY, double *pYt);
    int        SetLvxyCalMode                   (bool bDisableND);

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
    int        InitMemChannelDataND      (int lNumber);
    int        CopyMemChannelDataND      (int lSrcNumber, int lDstNumber);
    int        SetMultiChannel           (const char* strComment);
    int        SetMultiChannel           (int lStartCH, int lEndCH);
    int        GetMultiStatus            (int* plMultiCH, int* plClr, int* plSensor, char* strComment, unsigned long int* plCommentSize);

    int        GetCalibMode(int* plCalibMode);
    int        GetChannelTone            (int lCHNo, int lClr, int* plTone);
    int        SetMultiCalibColor        (int lClr, int lTargetSensor);
    int        GetLastChannel(int* plCHNo);
    int        SetChannelInterpolation   (int lInterpolation);
    int        GetChannelInterpolation   (int* plInterpolation);
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
    int        GetSpectrum              (double Frequency, double *pNormalizedFFT, double *pSVMWeighted);
    int        GetSpectrumParameters    (double *pMinFreq, double *pMaxFreq, double *pPitch);
    int        get_TypeName             (char* pVal, unsigned long int* plSize);
    int        get_TypeNO               (int *pVal);

    int        get_ld                   (double *pVal);
    int        get_Pe                   (double *pVal);
    int        get_ld_Pe                (double *pVal1, double *pVal2, double wp_sx, double wp_sy);
    int        SetMeasurementRange      (int lMeasurementType, int lRangeMode, int lRangeNum);
    int        GetMeasurementRange      (int lMeasurementType, int* lRangeMode, int* lRangeNum);
    int        GetJEITAFlickerData      (double* pfFlickerData, unsigned long int* plSize);
    int        GetJEITAFlickerData510AFE(double* pfFlickerDatasX, double* pfFlickerDatasY, double* pfFlickerDataY, unsigned long int* plSize);
    int        GetJEITAMeasureTime      (double* pfTime);
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
    int	       SetJEITAFlickerParams    (int lMeasurementMode, double fFrequency);
    int        GetJEITAFlickerParams    (int *plMeasurementMode, double* pfFrequency);
    int        SetWaveformParams        (int lSamplingNumberMode, int lSampleCountIndex, double fFrequency, int lXYZSensor, int lSync);
    int        GetWaveformParams        (int *lSamplingNumberMode, int *lSampleCountIndex, double *fFrequency, int *lXYZSensor, int *lSync);
    int        GetJEITAFlickerParams    (double *fFrequency, int* lSampleCountIndex);

    int        IsWaveformSupported      ();
    int        IsExpandedSamplingNumberSupported();
    int        SetWaveformParams        (int lSampleCountIndex, int lSamplingInterval, int lXYZSensor);
    int        GetWaveformParams        (int *lSampleCountIndex, int *lSamplingInterval, int *lXYZSensor);
    int        SetWaveformConditionSwitch    (int newVal);
    int        GetWaveformConditionSwitch    (int* pVal);

    int        get_RWaveform            (int *pVal);
    int        get_WaveformDataNumber   (unsigned long int *pVal);
    int        GetWaveformData          (double* pfWfData, double* pfFilteredWfData, unsigned long int* plSize, double* pfTimeSec);
    int        get_PeakFrequency        (double *pVal);
    int        get_PeakFrequency        (double* pNormalizedPeakFrequency, double* pSVMPeakFrequency);
    int        GetFlickerDerivedParams  (int *lTotalSpectrumCount, double *fMinFreq, double *fMaxFreq, double *fFreqPitch);

    int        GetSensorOfFlickerType   (int lDisplayMode, int* lSensor);

    int        GetDetectedEmissionFrequency  (double *pVal);
    int        GetDetectedVSync              (double *pVal);
    int        GetPredictedColorRange        (int *pRange);
    int        GetWaveformData               (int lDatatype, double* pfWfData, double* pfFilteredWfData, unsigned long int* plSize, double* pfTimeSec);
    int        GetWaveformData               (double *pfWfData_X, double *pfFilteredWfData_X, double *pfWfData_Y, double *pfFilteredWfData_Y, double *pfWfData_Z, double *pfFilteredWfData_Z, double *pfWfData_x, double *pfFilteredWfData_x, double *pfWfData_y, double *pfFilteredWfData_y, unsigned long int *plSize, double *pfTimeSec);

    int        GetModulationDepth       (double *pModulationDepth);
    int        GetWaveformDepth         (double* pWaveformDepth);

    int        GetVRRFlickerTrendStatistics         (double *pMax, double *pMin, double *pAverage);
    int        GetVRRFlickerWeightedDataStatistics  (double *pMax, double *pMin, double *pAverage);
    int        GetVRRFTrendTimeRangeStatistics          (double startTime, double endTime, double* pMax, double* pMin, double* pAverage);
    int        GetVRRFTrendSpecifiedFrequencyStatistics (double fFrequency, double* pMax, double* pMin, double* pAverage);
    
    int        GetSVMValue              (double* pSVMValue);
    int        SetSVMFindPeakMode       (bool bEnable);
    int        GetSVMFindPeakMode       (bool* pbEnable);
    int        GetUserTmMode            (bool* pbEnable);
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
    static int    SetFlickerWeights     (int lProfile, double *plWeights, double *plPhase, unsigned int lFreqBegin100, unsigned int lFreqEnd100, unsigned int lFreqPitch100);
    static int    SetFilterType         (int lFilterType);
    static int    SetSamplingCondition  (double fFrequency, unsigned int n);
    static int    SetCutoffFrequency    (double fFrequency);
    static int    SetCircuit            (int lCircuit);
    static int    FilterWaveform        (int lRange, double fSampFreq, unsigned int lDataLen, double* pfWfData, double* pfFilteredWfData, double* pfFlicker, double* pfPeakFreq, double* pfPowerSpectrum, unsigned int* plPowerSpectrumLen, double* pfMinFreq, double* pfMaxFreq, double* pfFreqPitch);
    static int    CalculateFlicker      (int lProfile, int lRange, double* pfData, unsigned int lDataLen, double fDarkData, double fSampFreq, double* pfFlickerJEITA, double* pfPeakFreq, double* pfPowerSpectrum, unsigned int* plPowerSpectrumLen);
    static int    CalculateFlicker      (int lProfile, int lRange, double* pfData, unsigned int lDataLen, double fDarkData, double fSampFreq, double* pfFlickerJEITA, double* pfPeakFreq, double* pfPowerSpectrum, unsigned int* plPowerSpectrumLen, double fFundamentalFreq, double fMeasureFrequency);
    static int    CalculateColor        (double dX, double dY, double dZ, double* pfXW, double* pfYW, double* pfZW, double *pfsx, double* pfsy, double* pfsLv, double* pfLvfL, double* pfud, double* pfvd, double* plT, double* pfduv, double* pfLsUser, double* pfusUser, double* pfvsUser, double* pfdEUser, double* pfdwl, double* pfpe);
    static int    CalculateDwPe         (double dY, double pfsx, double pfsy, double dsxWP, double dsyWP, double* pfdwl, double* pfpe);
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