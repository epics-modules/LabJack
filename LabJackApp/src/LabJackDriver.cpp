/* LabJackDriver.cpp
 *
 * Driver for LabJack I/O modules
 *
 * Mark Rivers
 * University of Chicago
*/

#include <iocsh.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <vector>

#include <epicsThread.h>
#include <epicsString.h>
#include <osiSock.h>
#include <epicsTime.h>

#include <asynPortDriver.h>

#include <epicsExport.h>

#include "LabJackM.h"

#define DRIVER_VERSION "3.0.0"

// Board parameters
#define modelNameString             "MODEL_NAME"
#define firmwareVersionString       "FIRMWARE_VERSION"
#define serialNumberString          "SERIAL_NUMBER"
#define deviceTemperatureString     "DEVICE_TEMPERATURE"
#define LJMVersionString            "LJM_VERSION"
#define driverVersionString         "DRIVER_VERSION"
#define pollSleepMSString           "POLL_SLEEP_MS"
#define pollTimeMSString            "POLL_TIME_MS"
#define deviceResetString           "DEVICE_RESET"
#define ainSettlingTimeAllString    "AIN_SETTLING_TIME_ALL"
#define ainResolutionAllString      "AIN_RESOLUTION_ALL"
#define lastErrorMessageString      "LAST_ERROR_MESSAGE"

// Analog input parameters
#define analogInValueString         "ANALOG_IN_VALUE"
#define analogInModeString          "ANALOG_IN_MODE"
#define analogInRangeString         "ANALOG_IN_RANGE"
#define analogInResolutionString    "ANALOG_IN_RESOLUTION"
#define analogInDifferentialString  "ANALOG_IN_DIFFERENTIAL"
#define analogInEnableString        "ANALOG_IN_ENABLE"
#define temperatureUnitsString      "TEMPERATURE_UNITS"
#
// Analog output parameters
#define analogOutValueString        "ANALOG_OUT_VALUE"
#define analogTDACValueString       "ANALOG_TDAC_VALUE"

// Digital I/O parameters
#define digitalDirectionString      "DIGITAL_DIRECTION"
#define digitalInWordString         "DIGITAL_IN_WORD"
#define digitalOutBitString         "DIGITAL_OUT_BIT"

// Waveform digitizer parameters - global
#define waveDigDwellString          "WAVEDIG_DWELL"
#define waveDigDwellActualString    "WAVEDIG_DWELL_ACTUAL"
#define waveDigTotalTimeString      "WAVEDIG_TOTAL_TIME"
#define waveDigFirstChanString      "WAVEDIG_FIRST_CHAN"
#define waveDigNumChansString       "WAVEDIG_NUM_CHANS"
#define waveDigNumPointsString      "WAVEDIG_NUM_POINTS"
#define waveDigCurrentPointString   "WAVEDIG_CURRENT_POINT"
#define waveDigExtTriggerString     "WAVEDIG_EXT_TRIGGER"
#define waveDigExtClockString       "WAVEDIG_EXT_CLOCK"
#define waveDigResolutionString     "WAVEDIG_RESOLUTION"
#define waveDigAutoRestartString    "WAVEDIG_AUTO_RESTART"
#define waveDigSettlingTimeString   "WAVEDIG_SETTLING_TIME"
#define waveDigRunString            "WAVEDIG_RUN"
#define waveDigTimeWFString         "WAVEDIG_TIME_WF"
#define waveDigAbsTimeWFString      "WAVEDIG_ABS_TIME_WF"
#define waveDigReadWFString         "WAVEDIG_READ_WF"
// Waveform digitizer parameters - per input
#define waveDigVoltWFString         "WAVEDIG_VOLT_WF"

// Waveform generator parameters - global
#define waveGenFreqString           "WAVEGEN_FREQ"
#define waveGenDwellString          "WAVEGEN_DWELL"
#define waveGenDwellActualString    "WAVEGEN_DWELL_ACTUAL"
#define waveGenTotalTimeString      "WAVEGEN_TOTAL_TIME"
#define waveGenNumPointsString      "WAVEGEN_NUM_POINTS"
#define waveGenCurrentPointString   "WAVEGEN_CURRENT_POINT"
#define waveGenIntDwellString       "WAVEGEN_INT_DWELL"
#define waveGenUserDwellString      "WAVEGEN_USER_DWELL"
#define waveGenIntNumPointsString   "WAVEGEN_INT_NUM_POINTS"
#define waveGenUserNumPointsString  "WAVEGEN_USER_NUM_POINTS"
#define waveGenExtTriggerString     "WAVEGEN_EXT_TRIGGER"
#define waveGenExtClockString       "WAVEGEN_EXT_CLOCK"
#define waveGenContinuousString     "WAVEGEN_CONTINUOUS"
#define waveGenRunString            "WAVEGEN_RUN"
#define waveGenUserTimeWFString     "WAVEGEN_USER_TIME_WF"
#define waveGenIntTimeWFString      "WAVEGEN_INT_TIME_WF"
// Waveform generator parameters - per output
#define waveGenWaveTypeString       "WAVEGEN_WAVE_TYPE"
#define waveGenEnableString         "WAVEGEN_ENABLE"
#define waveGenAmplitudeString      "WAVEGEN_AMPLITUDE"
#define waveGenOffsetString         "WAVEGEN_OFFSET"
#define waveGenPulseWidthString     "WAVEGEN_PULSE_WIDTH"
#define waveGenIntWFString          "WAVEGEN_INT_WF"
#define waveGenUserWFString         "WAVEGEN_USER_WF"

// Modbus address definitions
#define LJT_AI_FLOAT32                0000
#define LJT_DAC_FLOAT32               1000
#define LJT_DIO_BIT                   2000
#define LJT_DIO_STATE                 2800
#define LJT_STREAM_OUT_TARGET         4040
#define LJT_STREAM_OUT_BUFFER_SIZE    4050
#define LJT_STREAM_OUT_LOOP_SIZE      4060
#define LJT_STREAM_OUT_SET_LOOP       4070
#define LJT_STREAM_OUT_BUFFER_STATUS  4080
#define LJT_STREAM_OUT_ENABLE         4090
#define LJT_STREAM_OUT_BUFFER_F32     4400
#define LJT_STREAM_OUT                4800
#define LJT_AI_EF_READ_A              7000
#define LJT_AI_EF_INDEX               9000
#define LJT_AI_EF_CONFIG_A            9300
#define LJT_TDAC_FLOAT32              30000
#define LJT_AI_RANGE                  40000
#define LJT_AI_DIFFERENTIAL           41000
#define LJT_AI_RESOLUTION             41500

// MAX_ANALOG_IN and MAX_ANALOG_OUT may need to be changed if additional models are added with larger numbers
// These are used as a convenience for allocating small arrays of pointers, not large amounts of data
#define MAX_ANALOG_IN   14
#define MAX_ANALOG_OUT  2
#define MAX_DIGITAL_IO  23
#define MAX_SIGNALS     MAX_DIGITAL_IO

#define DEFAULT_POLL_SLEEP_MS 50
#define WAVE_DIG_READ_TIME 0.1

#define PI 3.14159265

enum waveGenType {
  waveGenTypeUser,
  waveGenTypeSin,
  waveGenTypeSquare,
  waveGenTypeSawTooth,
  waveGenTypePulse,
  waveGenTypeRandom
};

enum LJTModel {
  modelT4,
  modelT7,
  modelT7Pro,
  modelT8
};

enum LJTTemperatureUnits {
  temperatureUnitsK,
  temperatureUnitsC,
  temperatureUnitsF
};

struct enumStruct {
  const char *enumString;
  int  enumValue;
};

static enumStruct inputRangeT4Low[] = {
  {"+= 10V",   0}
};
static enumStruct inputRangeT4High[] = {
  {"0-2.5V",   0}
};

static enumStruct inputRangeT7[] = {
  {"+= 10V",   10000},
  {"+= 1V",     1000},
  {"+= 0.1V",    100},
  {"+= 0.01V",    10}
};
static enumStruct inputRangeT8[] = {
  {"+= 11.0V", 11000},
  {"+= 9.7V",   9700},
  {"+= 4.8V",   4800},
  {"+= 2.4V",   2400},
  {"+= 1.2V",   1200},
  {"+= 0.75V",   750},
  {"+= 0.60V",   600},
  {"+= 0.36V",   360},
  {"+= 0.30V",   300},
  {"+= 0.18V",   180},
  {"+= 0.15V",   150}
};

static enumStruct allResolution[] = {
  {"Default", 0},
  {"1", 1},
  {"2", 2},
  {"3", 3},
  {"4", 4},
  {"5", 5},
  {"6", 6},
  {"7", 7},
  {"8", 8},
  {"9", 9},
  {"10", 10},
  {"11", 11},
  {"12", 12},
  {"13", 13},
  {"14", 14},
  {"15", 15}
};

static enumStruct inputModeSingleEnded[] = {
  {"Single-Ended",   0}
};

static enumStruct inputModeDifferential[] = {
  {"Differential",   0}
};

static enumStruct inputModeBoth[] = {
  {"Single-Ended",   0},
  {"Differential",   1},
};


static const char *driverName = "LabJackDriver";

class LabJackDriver : public asynPortDriver {
public:
  LabJackDriver(const char *portName, const char *uniqueID, int maxInputPoints, int maxOutputPoints);

  /* These are the methods that we override from asynPortDriver */
  asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
//  virtual asynStatus readInt32(asynUser *pasynUser, epicsInt32 *value);
  asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
  virtual asynStatus readFloat64(asynUser *pasynUser, epicsFloat64 *value);
  asynStatus writeUInt32Digital(asynUser *pasynUser, epicsUInt32 value, epicsUInt32 mask);
  asynStatus readFloat64Array(asynUser *pasynUser, epicsFloat64 *value, size_t nElements, size_t *nIn);
  virtual asynStatus writeFloat64Array(asynUser *pasynUser, epicsFloat64 *value, size_t nElements);
//  virtual asynStatus readFloat64Array(asynUser *pasynUser, epicsFloat64 *value, size_t nElements, size_t *nIn);
  virtual asynStatus readEnum(asynUser *pasynUser, char *strings[], int values[], int severities[], size_t nElements, size_t *nIn);
//  virtual void report(FILE *fp, int details);
  // These should be private but are called from C
  static void pollerThread(void *);

protected:
  // Model parameters
  int modelName_;
  int firmwareVersion_;
  int serialNumber_;
  int deviceTemperature_;
  int LJMVersion_;
  int driverVersion_;
  int pollSleepMS_;
  int pollTimeMS_;
  int deviceReset_;
  int ainSettlingTimeAll_;
  int ainResolutionAll_;
  int lastErrorMessage_;

  // Analog input parameters
  int analogInValue_;
  int analogInMode_;
  int analogInRange_;
  int analogInResolution_;
  int analogInDifferential_;
  int analogInEnable_;
  int temperatureUnits_;

  // Analog output parameters
  int analogOutValue_;
  int analogTDACValue_;

  // Digital I/O parameters
  int digitalDirection_;
  int digitalInWord_;
  int digitalOutBit_;

  // Waveform digitizer parameters - global
  int waveDigDwell_;
  int waveDigDwellActual_;
  int waveDigTotalTime_;
  int waveDigFirstChan_;
  int waveDigNumChans_;
  int waveDigNumPoints_;
  int waveDigCurrentPoint_;
  int waveDigExtTrigger_;
  int waveDigExtClock_;
  int waveDigResolution_;
  int waveDigAutoRestart_;
  int waveDigSettlingTime_;
  int waveDigRun_;
  int waveDigTimeWF_;
  int waveDigAbsTimeWF_;
  int waveDigReadWF_;
  // Waveform digitizer parameters - per input
  int waveDigVoltWF_;

  // Waveform generator parameters - global
  int waveGenFreq_;
  int waveGenDwell_;
  int waveGenDwellActual_;
  int waveGenTotalTime_;
  int waveGenNumPoints_;
  int waveGenCurrentPoint_;
  int waveGenIntDwell_;
  int waveGenUserDwell_;
  int waveGenIntNumPoints_;
  int waveGenUserNumPoints_;
  int waveGenExtTrigger_;
  int waveGenExtClock_;
  int waveGenContinuous_;
  int waveGenRun_;
  int waveGenUserTimeWF_;
  int waveGenIntTimeWF_;
  // Waveform generator parameters - per output
  int waveGenWaveType_;
  int waveGenEnable_;
  int waveGenAmplitude_;
  int waveGenOffset_;
  int waveGenPulseWidth_;
  int waveGenIntWF_;
  int waveGenUserWF_;

private:
  int reportError(int err, const char *functionName, const char *message);
  void pollerThread();
  int LJMHandle_;
  LJTModel model_;
  int forceCallback_;

  int numDigitalIO_;
  int numAnalogIn_;
  std::vector<int> activeAiChannels_;
  int numAnalogOut_;
  int numResolution_;
  double aiValues_[MAX_ANALOG_IN];
  int aiValueAddresses_[MAX_ANALOG_IN];
  int aiTypeAddresses_[MAX_ANALOG_IN];
  int aiErrorAddresses_[MAX_ANALOG_IN];
  enumStruct *inputRangeEnums_;
  int numInputRange_;
  size_t maxInputPoints_;
  size_t maxOutputPoints_;
  epicsFloat64 *waveDigBuffer_[MAX_ANALOG_IN];
  epicsFloat64 *waveDigTempBuffer_;
  epicsFloat64 *waveDigTimeBuffer_;
  epicsFloat64 *waveDigAbsTimeBuffer_;
  epicsFloat64 *pInBuffer_;
  int waveDigScansPerRead_;
  int numWaveDigChans_;
  bool waveDigRunning_;
  int setActiveAiChannels();
  int readAnalogInputs();
  int startWaveDig();
  int stopWaveDig(bool restartOK);
  int readWaveDig();
  int readoutWaveDig();
  int computeWaveDigTimes();
  int computeAiValue(int chan, double *aiValue);
  epicsFloat64 *waveGenIntBuffer_[MAX_ANALOG_OUT];
  epicsFloat64 *waveGenUserBuffer_[MAX_ANALOG_OUT];
  epicsFloat64 *waveGenUserTimeBuffer_;
  epicsFloat64 *waveGenIntTimeBuffer_;
  epicsFloat64 *waveGenOutBuffer_;
  int numWaveGenChans_;
  bool waveGenRunning_;
  int startWaveGen();
  int stopWaveGen();
  int computeWaveGenTimes();
  int defineWaveform(int channel);

};

LabJackDriver::LabJackDriver(const char *portName, const char *uniqueID, int maxInputPoints, int maxOutputPoints)
  : asynPortDriver(portName, MAX_SIGNALS,
      asynInt32Mask      | asynFloat64Mask      | asynOctetMask | asynUInt32DigitalMask |
      asynInt32ArrayMask | asynFloat64ArrayMask | asynEnumMask  | asynDrvUserMask,
      asynInt32Mask      | asynFloat64Mask      | asynOctetMask | asynUInt32DigitalMask |
      asynInt32ArrayMask | asynFloat64ArrayMask | asynEnumMask,
      ASYN_MULTIDEVICE | ASYN_CANBLOCK, 1, /* ASYN_CANBLOCK=1, ASYN_MULTIDEVICE=1, autoConnect=1 */
      0, 0),  /* Default priority and stack size */
    forceCallback_(1),
    maxInputPoints_(maxInputPoints),
    maxOutputPoints_(maxOutputPoints),
    numWaveDigChans_(1),
    waveDigRunning_(false),
    numWaveGenChans_(1),
    waveGenRunning_(false)

{
  int status;
  std::string identifier = uniqueID;
  static const char *functionName = "LabJackDriver";

  // UniqueID can be an IP address or a fully qualified domain name (i.e. containing periods)
  // If it contains a period then it is definitely an IP address or IP name,
  // since device names and serial numbers cannot contain periods
  if (identifier.find(".") != std::string::npos) {
    struct sockaddr_in ipAddr;
    char ipAddrAsString[25];
    status = aToIPAddr(uniqueID, 0, &ipAddr);
    if (status != 0) {
      printf("LabJackDriver::LabJackDriver uniqueID=%s contains period but is not valid IP name or address\n", uniqueID);
      return;
    }
    ipAddrToDottedIP(&ipAddr, ipAddrAsString, sizeof(ipAddrAsString));
    // The string from ipAddrToDottedIP will always have a :port at the end
    // Parse the port and remove from the host string.
    std::string host = ipAddrAsString;
    size_t colon = host.find(":");
    if (colon != std::string::npos) {
      identifier = host.substr(0, colon);
    }
  }

  status = LJM_Open(LJM_dtANY, LJM_ctANY, identifier.c_str(), &LJMHandle_);
  reportError(status, functionName, "Calling LJM_Open");

    // Model parametersLJT_DAC_FLOAT32
  createParam(modelNameString,                  asynParamInt32, &modelName_);
  createParam(firmwareVersionString,            asynParamOctet, &firmwareVersion_);
  createParam(serialNumberString,               asynParamOctet, &serialNumber_);
  createParam(deviceTemperatureString,        asynParamFloat64, &deviceTemperature_);
  createParam(LJMVersionString,                 asynParamOctet, &LJMVersion_);
  createParam(driverVersionString,              asynParamOctet, &driverVersion_);
  createParam(pollSleepMSString,              asynParamFloat64, &pollSleepMS_);
  createParam(pollTimeMSString,               asynParamFloat64, &pollTimeMS_);
  createParam(deviceResetString,                asynParamInt32, &deviceReset_);
  createParam(ainSettlingTimeAllString,       asynParamFloat64, &ainSettlingTimeAll_);
  createParam(ainResolutionAllString,           asynParamInt32, &ainResolutionAll_);
  createParam(lastErrorMessageString,           asynParamOctet, &lastErrorMessage_);

  // Analog input parameters
  createParam(analogInValueString,             asynParamFloat64, &analogInValue_);
  createParam(analogInModeString,                asynParamInt32, &analogInMode_);
  createParam(analogInRangeString,               asynParamInt32, &analogInRange_);
  createParam(analogInResolutionString,          asynParamInt32, &analogInResolution_);
  createParam(analogInDifferentialString,        asynParamInt32, &analogInDifferential_);
  createParam(analogInEnableString,              asynParamInt32, &analogInEnable_);
  createParam(temperatureUnitsString,            asynParamInt32, &temperatureUnits_);

  // Analog input parameters
  createParam(analogOutValueString,            asynParamFloat64, &analogOutValue_);
  createParam(analogTDACValueString,           asynParamFloat64, &analogTDACValue_);

  // Digital I/O parameters
  createParam(digitalDirectionString,   asynParamUInt32Digital, &digitalDirection_);
  createParam(digitalInWordString,      asynParamUInt32Digital, &digitalInWord_);
  createParam(digitalOutBitString,      asynParamUInt32Digital, &digitalOutBit_);

  // Waveform digitizer parameters - global
  createParam(waveDigDwellString,            asynParamFloat64, &waveDigDwell_);
  createParam(waveDigDwellActualString,      asynParamFloat64, &waveDigDwellActual_);
  createParam(waveDigTotalTimeString,        asynParamFloat64, &waveDigTotalTime_);
  createParam(waveDigFirstChanString,          asynParamInt32, &waveDigFirstChan_);
  createParam(waveDigNumChansString,           asynParamInt32, &waveDigNumChans_);
  createParam(waveDigNumPointsString,          asynParamInt32, &waveDigNumPoints_);
  createParam(waveDigCurrentPointString,       asynParamInt32, &waveDigCurrentPoint_);
  createParam(waveDigExtTriggerString,         asynParamInt32, &waveDigExtTrigger_);
  createParam(waveDigExtClockString,           asynParamInt32, &waveDigExtClock_);
  createParam(waveDigResolutionString,         asynParamInt32, &waveDigResolution_);
  createParam(waveDigAutoRestartString,        asynParamInt32, &waveDigAutoRestart_);
  createParam(waveDigSettlingTimeString,     asynParamFloat64, &waveDigSettlingTime_);
  createParam(waveDigRunString,                asynParamInt32, &waveDigRun_);
  createParam(waveDigTimeWFString,      asynParamFloat64Array, &waveDigTimeWF_);
  createParam(waveDigAbsTimeWFString,   asynParamFloat64Array, &waveDigAbsTimeWF_);
  createParam(waveDigReadWFString,             asynParamInt32, &waveDigReadWF_);
  // Waveform digitizer parameters - per input
  createParam(waveDigVoltWFString,      asynParamFloat64Array, &waveDigVoltWF_);

  // Waveform generator parameters - global
  createParam(waveGenFreqString,             asynParamFloat64, &waveGenFreq_);
  createParam(waveGenDwellString,            asynParamFloat64, &waveGenDwell_);
  createParam(waveGenDwellActualString,      asynParamFloat64, &waveGenDwellActual_);
  createParam(waveGenTotalTimeString,        asynParamFloat64, &waveGenTotalTime_);
  createParam(waveGenNumPointsString,          asynParamInt32, &waveGenNumPoints_);
  createParam(waveGenCurrentPointString,       asynParamInt32, &waveGenCurrentPoint_);
  createParam(waveGenIntDwellString,         asynParamFloat64, &waveGenIntDwell_);
  createParam(waveGenUserDwellString,        asynParamFloat64, &waveGenUserDwell_);
  createParam(waveGenIntNumPointsString,       asynParamInt32, &waveGenIntNumPoints_);
  createParam(waveGenUserNumPointsString,      asynParamInt32, &waveGenUserNumPoints_);
  createParam(waveGenExtTriggerString,         asynParamInt32, &waveGenExtTrigger_);
  createParam(waveGenExtClockString,           asynParamInt32, &waveGenExtClock_);
  createParam(waveGenContinuousString,         asynParamInt32, &waveGenContinuous_);
  createParam(waveGenRunString,                asynParamInt32, &waveGenRun_);
  createParam(waveGenUserTimeWFString,  asynParamFloat64Array, &waveGenUserTimeWF_);
  createParam(waveGenIntTimeWFString,   asynParamFloat64Array, &waveGenIntTimeWF_);
  // Waveform generator parameters - per output
  createParam(waveGenWaveTypeString,           asynParamInt32, &waveGenWaveType_);
  createParam(waveGenEnableString,             asynParamInt32, &waveGenEnable_);
  createParam(waveGenAmplitudeString,        asynParamFloat64, &waveGenAmplitude_);
  createParam(waveGenOffsetString,           asynParamFloat64, &waveGenOffset_);
  createParam(waveGenPulseWidthString,       asynParamFloat64, &waveGenPulseWidth_);
  createParam(waveGenIntWFString,       asynParamFloat64Array, &waveGenIntWF_);
  createParam(waveGenUserWFString,      asynParamFloat64Array, &waveGenUserWF_);

  // Set the error message to no error
  setStringParam(lastErrorMessage_, "No error");

  // Disable watchdog timer
  status = LJM_eWriteName(LJMHandle_, "WATCHDOG_ENABLE_DEFAULT", 0);
  reportError(status, functionName, "Calling LJM_eWriteName for WATCHDOG_ENABLE_DEFAULT");

  // Get the device information
  double dTemp;
  status = LJM_eReadName(LJMHandle_, "PRODUCT_ID", &dTemp);
  reportError(status, functionName, "Calling LJM_eReadName for PRODUCT_ID");
  int productId = (int)dTemp;
  status = LJM_eReadName(LJMHandle_, "HARDWARE_INSTALLED", &dTemp);
  reportError(status, functionName, "Calling LJM_eReadName for HARDWARE_INSTALLED");
  int hardwareBits = (int)dTemp;
  switch (productId) {
    case LJM_dtT4:
      model_ = modelT4;
      numDigitalIO_ = 20;
      numAnalogIn_ = 12;
      inputRangeEnums_ = inputRangeT4Low;
      numInputRange_ = sizeof(inputRangeT4Low)/sizeof(enumStruct);
      numAnalogOut_ = 2;
      numResolution_ = 6;
      break;
    case LJM_dtT7:
      numDigitalIO_ = 23;
      numAnalogIn_ = 14;
      inputRangeEnums_ = inputRangeT7;
      numInputRange_ = sizeof(inputRangeT7)/sizeof(enumStruct);
      numAnalogOut_ = 2;
      // Determine if this is a T7-Pro by looking at the hardware bits. 3 low order bits are set for T7-Pro
      if ((hardwareBits & 7) == 7) {
        model_ = modelT7Pro;
        numResolution_ = 13;
      } else {
        model_ = modelT7;
        numResolution_ = 9;
      }
      break;
    case LJM_dtT8:
      model_ = modelT8;
      numDigitalIO_ = 23;
      numAnalogIn_ = 8;
      inputRangeEnums_ = inputRangeT8;
      numInputRange_ = sizeof(inputRangeT8)/sizeof(enumStruct);
      numAnalogOut_ = 2;
      numResolution_ = 17;
      break;
    default:
      asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "%s::%s unknown product ID %f\n", driverName, functionName, dTemp);
      setStringParam(modelName_, "Unknown");
      break;
  }
  char strTemp[20];
  setIntegerParam(modelName_, model_);
  status = LJM_eReadName(LJMHandle_, "FIRMWARE_VERSION", &dTemp);
  reportError(status, functionName, "Calling LJM_eReadName for FIRMWARE_VERSION");
  sprintf(strTemp, "%f", dTemp);
  setStringParam(firmwareVersion_, strTemp);
  status = LJM_eReadName(LJMHandle_, "SERIAL_NUMBER", &dTemp);
  reportError(status, functionName, "Calling LJM_eReadName for SERIAL_NUMBER");
  sprintf(strTemp, "%lld", epicsInt64(dTemp));
  setStringParam(serialNumber_, strTemp);
  sprintf(strTemp, "%f", LJM_VERSION);
  setStringParam(LJMVersion_, strTemp);
  setStringParam(driverVersion_, DRIVER_VERSION);

  // Allocate memory for the input and output buffers
  for (int i=0; i<numAnalogIn_; i++) {
    waveDigBuffer_[i]  = (epicsFloat64 *) calloc(maxInputPoints_,  sizeof(epicsFloat64));
  }
  waveDigTempBuffer_     = (epicsFloat64 *) calloc(maxInputPoints_* numAnalogIn_,  sizeof(epicsFloat64));
  waveDigTimeBuffer_     = (epicsFloat64 *) calloc(maxInputPoints_,  sizeof(epicsFloat64));
  waveDigAbsTimeBuffer_  = (epicsFloat64 *) calloc(maxInputPoints_,  sizeof(epicsFloat64));

  for (int i=0; i<numAnalogOut_; i++) {
    waveGenIntBuffer_[i]  = (epicsFloat64 *) calloc(maxOutputPoints_, sizeof(epicsFloat64));
    waveGenUserBuffer_[i] = (epicsFloat64 *) calloc(maxOutputPoints_, sizeof(epicsFloat64));
  }
  waveGenUserTimeBuffer_ = (epicsFloat64 *) calloc(maxOutputPoints_, sizeof(epicsFloat64));
  waveGenIntTimeBuffer_  = (epicsFloat64 *) calloc(maxOutputPoints_, sizeof(epicsFloat64));
  waveGenOutBuffer_      = (epicsFloat64 *) calloc(maxOutputPoints * numAnalogOut_, sizeof(epicsFloat64));

  // Initialize some parameters
  setIntegerParam(waveDigNumPoints_, 1);
  setIntegerParam(waveDigRun_, 0);
  setIntegerParam(waveGenUserNumPoints_, 1);
  setIntegerParam(waveGenIntNumPoints_, 1);
  setIntegerParam(waveGenRun_, 0);
  setDoubleParam(pollSleepMS_, DEFAULT_POLL_SLEEP_MS);

  // Set all analog input values to -9999 so unread values are obvious and there are no errors on startup
  for (int i=0; i<MAX_ANALOG_IN; i++) {
    setDoubleParam(i, analogInValue_, -9999.0);
  }

  // Initialize the vector of active analog inputs to disable all channels
  // This prevents error messages on T4, and autosave will overwrite this
  for (int i=0; i<numAnalogIn_; i++) {
    setIntegerParam(i, analogInEnable_, 0);
  }
  setActiveAiChannels();

  // Read the current values of the DACs so ao records initialize to current value (bumpless reboot)
  double dacValue;
  for (int i=0; i<numAnalogOut_; i++) {
    status = LJM_eReadAddress(LJMHandle_, LJT_DAC_FLOAT32 + 2*i, LJM_FLOAT32, &dacValue);
    reportError(status, functionName, "Calling LJM_eReadAddress for LJT_DAC_FLOAT32");
    setDoubleParam(i, analogOutValue_, dacValue);
  }

  // Read the current binary direction register initialize to current value (bumpless reboot)
  status = LJM_eReadName(LJMHandle_, "DIO_DIRECTION", &dTemp);
  reportError(status, functionName, "Calling LJM_eReadName for LJT_DIO_DIRECTION");
  setUIntDigitalParam(digitalDirection_, (epicsUInt32)dTemp, 0xFFFFFFFF);

  // Read the status of all output bits and initialize them
  for (int i=0; i<numDigitalIO_; i++) {
    status = LJM_eReadAddress(LJMHandle_, LJT_DIO_BIT + i, LJM_UINT16, &dTemp);
    reportError(status, functionName, "Calling LJM_eReadAddress for LJT_DIO_BIT");
    setUIntDigitalParam(i, digitalOutBit_, dTemp?1:0, 0x1);
  }

  /* Start the thread to poll counters and digital inputs and do callbacks to
   * device support */
  epicsThreadCreate("LabJackPoller",
                    epicsThreadPriorityLow,
                    epicsThreadGetStackSize(epicsThreadStackMedium),
                    (EPICSTHREADFUNC)pollerThread,
                    this);

}

int  LabJackDriver::reportError(int err, const char *functionName, const char *message)
{
  char libraryMessage[LJM_MAX_NAME_SIZE];
  char driverMessage[2*LJM_MAX_NAME_SIZE];
  char timeString[40];
  epicsTime now=epicsTime::getCurrent();
  now.strftime(timeString, sizeof(timeString)-1, "%Y/%m/%d %H:%M:%S.%03f");
  switch (err) {
    case 0:
      asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
        "%s::%s Info: %s\n", driverName, functionName, message);
      break;
    case -1:
      asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
        "%s::%s Error: %s\n", driverName, functionName, message);
      snprintf(driverMessage, sizeof(driverMessage)-1, "%s Error: %s", timeString, message);
      setStringParam(lastErrorMessage_, driverMessage);
      break;
    default:
      LJM_ErrorToString(err, libraryMessage);
      asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
        "%s::%s Error: %s, err=%d %s\n", driverName, functionName, message, err, libraryMessage);
      snprintf(driverMessage, sizeof(driverMessage)-1, "%s Error: %s, err=%d %s", timeString, message, err, libraryMessage);
      setStringParam(lastErrorMessage_, driverMessage);
  }
  return err;
}

asynStatus LabJackDriver::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
  int addr;
  int function = pasynUser->reason;
  int status=0;
  static const char *functionName = "writeInt32";

  this->getAddress(pasynUser, &addr);
  setIntegerParam(addr, function, value);

  // Analog input functions
  if (function == analogInEnable_) {
    if (model_ == modelT4) {
      int analogEnable;
      double dblTemp;
      status = LJM_eReadName(LJMHandle_, "DIO_ANALOG_ENABLE", &dblTemp);
      reportError(status, functionName, "Calling LJM_eReadName for DIO_ANALOG_ENABLE");
      analogEnable = (int)dblTemp;
      if (value) {
        analogEnable |= 1<<addr;
      }
      else {
        analogEnable &= ~(1<<addr);
      }
      status = LJM_eWriteName(LJMHandle_, "DIO_ANALOG_ENABLE", analogEnable);
      reportError(status, functionName, "Calling LJM_eWriteName for DIO_ANALOG_ENABLE");
    }
    setActiveAiChannels();
  }
  else if (function == analogInResolution_) {
    status = LJM_eWriteAddress(LJMHandle_, LJT_AI_RESOLUTION + addr, LJM_UINT16, value);
    reportError(status, functionName, "Calling LJM_eWriteAddress for LJT_AI_RESOLUTION");
  }
  else if (function == analogInRange_) {
    if (value != 0) {
      // Convert from integer mV in enum to volts
      double volts = value/1000.;
      status = LJM_eWriteAddress(LJMHandle_, LJT_AI_RANGE + (2*addr), LJM_FLOAT32, volts);
      reportError(status, functionName, "Calling LJM_eWriteAddress for LJT_AI_RANGE");
    }
  }
  else if (function == analogInDifferential_) {
    if (model_ == modelT7 || model_ == modelT7Pro) {
      // This maps value=0/1 to 199:addr+1 which are the required values for the device
      int ival = (value == 0) ? 199 : addr+1;
      status = LJM_eWriteAddress(LJMHandle_, LJT_AI_DIFFERENTIAL + addr, LJM_UINT16, ival);
      reportError(status, functionName, "Calling LJM_eWriteAddress for LJT_AI_DIFFERENTIAL");
      setActiveAiChannels();
    }
  }
  else if (function == ainResolutionAll_) {
    status = LJM_eWriteName(LJMHandle_, "AIN_ALL_RESOLUTION_INDEX", value);
    reportError(status, functionName, "Calling LJM_eWriteName for AIN_ALL_RESOLUTION_INDEX");
  }

  // Waveform digitizer functions
  else if (function == waveDigResolution_) {
    status = LJM_eWriteName(LJMHandle_, "STREAM_RESOLUTION_INDEX", value);
    reportError(status, functionName, "Calling LJM_eWriteName for STREAM_RESOLUTION_INDEX");
  }
  else if (function == waveDigRun_) {
    if (value && !waveDigRunning_) {
      status = startWaveDig();
    }
    else if (!value && waveDigRunning_) {
      // Don't auto-restart when stopping manually
      status = stopWaveDig(false);
    }
  }
  else if (function == waveDigReadWF_) {
    readWaveDig();
  }
  else if (function == waveDigNumPoints_) {
    if (value > (int)maxInputPoints_) {
      asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
                "%s::%s error WaveDigNumPoints=%d must be less than MaxInputPoints=%d\n",
                driverName, functionName, value, (int)maxInputPoints_);
      setIntegerParam(waveDigNumPoints_, maxInputPoints_);
    }
    computeWaveDigTimes();
  }

  // Waveform generator functions
  else if (function == waveGenRun_) {
    if (value && !waveGenRunning_)
      status = startWaveGen();
    else if (!value && waveGenRunning_)
      status = stopWaveGen();
  }
  else if ((function == waveGenWaveType_) ||
      (function == waveGenUserNumPoints_) ||
      (function == waveGenIntNumPoints_)  ||
      (function == waveGenEnable_)        ||
      (function == waveGenExtTrigger_)    ||
      (function == waveGenExtClock_)      ||
      (function == waveGenContinuous_)) {
    defineWaveform(addr);
    if (waveGenRunning_) {
      status = stopWaveGen();
      status |= startWaveGen();
    }
  }

  // Device reset
  else if (function == deviceReset_) {
    // We set the watchdog timer to expire in 10 seconds of no communication and to reset the device when it expires.
    // Thus when the IOC is stopped for at least 10 seconds the device will reset.
    // This is then disabled in the constructor when the IOC restarts.
    status = LJM_eWriteName(LJMHandle_, "WATCHDOG_ENABLE_DEFAULT", 0);
    reportError(status, functionName, "Calling LJM_eWriteName for WATCHDOG_ENABLE_DEFAULT");
    status = LJM_eWriteName(LJMHandle_, "WATCHDOG_TIMEOUT_S_DEFAULT", 10);
    reportError(status, functionName, "Calling LJM_eWriteName for WATCHDOG_TIMEOUT_S_DEFAULT");
    status = LJM_eWriteName(LJMHandle_, "WATCHDOG_RESET_ENABLE_DEFAULT", 1);
    reportError(status, functionName, "Calling LJM_eWriteName for WATCHDOG_RESET_ENABLE_DEFAULT");
    status = LJM_eWriteName(LJMHandle_, "WATCHDOG_ENABLE_DEFAULT", 1);
    reportError(status, functionName, "Calling LJM_eWriteName for WATCHDOG_ENABLE_DEFAULT");
  }

  // This is a separate if statement because these cases are also treated above
  if ((function == waveGenUserNumPoints_) ||
      (function == waveGenIntNumPoints_)) {
    computeWaveGenTimes();
  }

  callParamCallbacks(addr);
  if (status == 0) {
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
             "%s:%s, port %s, wrote %d to address %d\n",
             driverName, functionName, this->portName, value, addr);
  } else {
    asynPrint(pasynUser, ASYN_TRACE_ERROR,
             "%s:%s, port %s, ERROR writing %d to address %d, status=%d\n",
             driverName, functionName, this->portName, value, addr, status);
  }
  return (status==0) ? asynSuccess : asynError;
}

asynStatus LabJackDriver::writeUInt32Digital(asynUser *pasynUser, epicsUInt32 value, epicsUInt32 mask)
{
  int function = pasynUser->reason;
  int status=0;
  int addr;
  epicsUInt32 direction=0;
  static const char *functionName = "writeUInt32Digital";

  this->getAddress(pasynUser, &addr);
  if (function == digitalDirection_) {
    getUIntDigitalParam(digitalDirection_, &direction, 0xFFFFFFFF);
    if (value) direction |= mask;
    else direction &= ~mask;
    status = LJM_eWriteName(LJMHandle_, "DIO_DIRECTION", direction);
    reportError(status, functionName, "Calling LJM_eWriteName for LJT_DIO_DIRECTION");
    setUIntDigitalParam(addr, function, direction, 0xFFFFFFFF);
  }
  else if (function == digitalOutBit_) {
    // Don't try to access DIO beyond available number
    if (addr < numDigitalIO_) {
      status = LJM_eWriteAddress(LJMHandle_, LJT_DIO_BIT + addr, LJM_UINT16, value?1:0);
      reportError(status, functionName, "Calling LJM_eWriteAddress for LJT_DIO_BIT");
    }
  }

  callParamCallbacks();
  if (status == 0) {
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
             "%s:%s, port %s, function=%d, wrote value=0x%x, mask=0x%x, direction=0x%x\n",
             driverName, functionName, this->portName, function, value, mask, direction);
  } else {
    asynPrint(pasynUser, ASYN_TRACE_ERROR,
             "%s:%s, port %s, ERROR writing %d to address %d, status=%d\n",
             driverName, functionName, this->portName, value, addr, status);
  }
  return (status==0) ? asynSuccess : asynError;
}

asynStatus LabJackDriver::readFloat64(asynUser *pasynUser, epicsFloat64 *value)
{
  int addr;
  int function = pasynUser->reason;
  int status=0;
  static const char *functionName = "readFloat64";

  this->getAddress(pasynUser, &addr);

  // Device temperature input function
  if (function == deviceTemperature_) {
    if (waveDigRunning_) return asynSuccess;
    status = LJM_eReadName(LJMHandle_, "TEMPERATURE_DEVICE_K", value);
    setDoubleParam(deviceTemperature_, *value);
    reportError(status, functionName, "Calling LJM_eReadName for TEMPERATURE_DEVICE_K");
    // On the T7-PRO it seems necessary to read the analog inputs continuously for 150 ms 
    // after reading the device temperature to avoid glitches
    if (model_ == modelT7Pro) {
      epicsTime tStart = epicsTime::getCurrent();
      while (1) {
        readAnalogInputs();
        if ((epicsTime::getCurrent() - tStart) > 0.15) break;
      }
    }
  }
  // Other functions we call the base class method
  else {
     status = asynPortDriver::readFloat64(pasynUser, value);
  }

  callParamCallbacks(addr);
  return (status==0) ? asynSuccess : asynError;
}

asynStatus LabJackDriver::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
  int addr;
  int function = pasynUser->reason;
  int status=0;
  static const char *functionName = "writeFloat64";

  this->getAddress(pasynUser, &addr);
  setDoubleParam(addr, function, value);

  // Analog output functions
  if (function == analogOutValue_) {
    if (waveGenRunning_) {
      reportError(-1, functionName, "cannot write analog outputs while waveform generator is running.");
      return asynError;
    }
    status = LJM_eWriteAddress(LJMHandle_, LJT_DAC_FLOAT32 + (2*addr), LJM_FLOAT32, value);
    reportError(status, functionName, "Calling LJM_eWriteAddress for LJT_DAC_FLOAT32");
  }
  else if (function == analogTDACValue_) {
    status = LJM_eWriteAddress(LJMHandle_, LJT_TDAC_FLOAT32 + (2*addr), LJM_FLOAT32, value);
    reportError(status, functionName, "Calling LJM_eWriteAddress for LJT_TDAC_FLOAT32");
  }
  else if (function == ainSettlingTimeAll_) {
    status = LJM_eWriteName(LJMHandle_, "AIN_ALL_SETTLING_US", value);
    reportError(status, functionName, "Calling LJM_eWriteName for AIN_ALL_SETTLING_US");
  }
  // Waveform digitizer functions
  else if (function == waveDigSettlingTime_) {
    status = LJM_eWriteName(LJMHandle_, "STREAM_SETTLING_US", value);
    reportError(status, functionName, "Calling LJM_eWriteName for LJT_WAVEDIG_SETTLING");
  }
  else if (function == waveDigDwell_) {
    computeWaveDigTimes();
  }

  // Waveform generator functions
  else if ((function == waveGenUserDwell_)  ||
           (function == waveGenIntDwell_)   ||
           (function == waveGenPulseWidth_) ||
           (function == waveGenAmplitude_)  ||
           (function == waveGenOffset_)) {
    defineWaveform(addr);
    if (waveGenRunning_) {
      status = stopWaveGen();
      status |= startWaveGen();
    }
  }

  // This is a separate if statement because these cases are also treated above
  if ((function == waveGenUserDwell_)  ||
      (function == waveGenIntDwell_)) {
    computeWaveGenTimes();
  }

  callParamCallbacks(addr);
  if (status == 0) {
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
             "%s:%s, port %s, wrote %f to address %d\n",
             driverName, functionName, this->portName, value, addr);
  } else {
    asynPrint(pasynUser, ASYN_TRACE_ERROR,
             "%s:%s, port %s, ERROR writing %f to address %d, status=%d\n",
             driverName, functionName, this->portName, value, addr, status);
  }
  return (status==0) ? asynSuccess : asynError;
}

asynStatus LabJackDriver::readFloat64Array(asynUser *pasynUser, epicsFloat64 *value, size_t nElements, size_t *nIn)
{
  int function = pasynUser->reason;
  int addr;
  int numPoints;
  epicsFloat64 *inPtr;
  static const char *functionName = "readFloat64Array";

  this->getAddress(pasynUser, &addr);

  if (function == waveDigTimeWF_) {
    inPtr = waveDigTimeBuffer_;
    getIntegerParam(waveDigNumPoints_, &numPoints);
  }
  getIntegerParam(waveGenNumPoints_, &numPoints);
  if (function == waveGenUserWF_)
    inPtr = waveGenUserBuffer_[addr];
  else if (function == waveGenIntWF_)
    inPtr = waveGenIntBuffer_[addr];
  else if (function == waveGenUserTimeWF_)
    inPtr = waveGenUserTimeBuffer_;
  else if (function == waveGenIntTimeWF_)
    inPtr = waveGenIntTimeBuffer_;
  else {
    asynPrint(pasynUser, ASYN_TRACE_ERROR,
      "%s:%s: ERROR: unknown function=%d\n",
      driverName, functionName, function);
    return asynError;
  }
  *nIn = nElements;
  if (*nIn > (size_t) numPoints) *nIn = (size_t) numPoints;
  memcpy(value, inPtr, *nIn*sizeof(epicsFloat64));

  return asynSuccess;
}

asynStatus LabJackDriver::writeFloat64Array(asynUser *pasynUser, epicsFloat64 *value, size_t nElements)
{
  int function = pasynUser->reason;
  int addr;
  static const char *functionName = "writeFloat64Array";

  this->getAddress(pasynUser, &addr);

  if (function == waveGenUserWF_) {
    if ((addr >= numAnalogOut_) || (nElements > maxOutputPoints_)) {
      asynPrint(pasynUser, ASYN_TRACE_ERROR,
        "%s:%s: ERROR: addr=%d max=%d, nElements=%d max=%d\n",
        driverName, functionName, addr, numAnalogOut_-1, (int)nElements, (int)maxOutputPoints_);
      return asynError;
    }
    memcpy(waveGenUserBuffer_[addr], value, nElements*sizeof(epicsFloat64));
  }
  else {
    asynPrint(pasynUser, ASYN_TRACE_ERROR,
      "%s:%s: ERROR: unknown function=%d\n",
      driverName, functionName, function);
    return asynError;
  }

  return asynSuccess;
}

asynStatus LabJackDriver::readEnum(asynUser *pasynUser, char *strings[], int values[], int severities[], size_t nElements, size_t *nIn)
{
  int function = pasynUser->reason;
  enumStruct *pEnum=0;
  int numEnums=1;
  int i;
  bool validAddr = false;
  int addr;

  this->getAddress(pasynUser, &addr);

  //static const char *functionName = "readEnum";

  if (function == analogInRange_) {
    pEnum    = inputRangeEnums_;
    numEnums = numInputRange_;
    if (model_ == modelT4 && (addr > 3)) {
      pEnum = inputRangeT4High;
    }
    if (addr < numAnalogIn_) validAddr = true;
  }
  else if ((function == analogInResolution_) ||
           (function == waveDigResolution_)  ||
           (function == ainResolutionAll_)) {
    pEnum    = allResolution;
    numEnums = numResolution_;
    if (addr < numAnalogIn_) validAddr = true;
  }
  else if (function == analogInDifferential_) {
    switch (model_) {
      case modelT4:
        pEnum = inputModeSingleEnded;
        numEnums = 1;
        break;
      case modelT7:
      case modelT7Pro:
        pEnum = inputModeBoth;
        numEnums = 2;
        break;
      case modelT8:
        pEnum = inputModeDifferential;
        numEnums = 1;
        break;
    }
    if (addr < numAnalogIn_) validAddr = true;
  }
  else {
    *nIn = 0;
    return asynError;
  }
  if (!validAddr) {
    *nIn = 0;
    return asynError;
  }
  for (i=0; ((i<numEnums) && (i<(int)nElements)); i++) {
    if (strings[i]) free(strings[i]);
    strings[i] = epicsStrDup(pEnum[i].enumString);
    values[i] = pEnum[i].enumValue;
    severities[i] = 0;
  }
  *nIn = i;
  return asynSuccess;
}

int LabJackDriver::setActiveAiChannels() {
  // NOTE: This needs work for the T4 where registers need to be set to enable/disable analog inputs 5-12
  int i;
  int enable;
  int differential;

  activeAiChannels_.clear();
  for (i=0; i<numAnalogIn_; i++) {
    getIntegerParam(i, analogInEnable_, &enable);
    getIntegerParam(i, analogInDifferential_, &differential);
    if (enable) activeAiChannels_.push_back(i);
    if (differential) i++;
  }
  return asynSuccess;
}

int LabJackDriver::readAnalogInputs() {
  static const char *functionName = "readAnalogInputs";
  int status = asynSuccess;
  int numChannels = (int)activeAiChannels_.size();
  if (numChannels == 0) return asynSuccess;
  if (waveDigRunning_) return asynSuccess;
  for (int i=0; i<numChannels; i++) {
    aiValueAddresses_[i] = LJT_AI_FLOAT32 + 2*activeAiChannels_[i];
    aiTypeAddresses_[i] = LJM_FLOAT32;
  }
  status = LJM_eReadAddresses(LJMHandle_, numChannels, aiValueAddresses_, aiTypeAddresses_, aiValues_, aiErrorAddresses_);
  reportError(status, functionName, "Error calling LJM_eReadAddresses");
  return status;
}

int LabJackDriver::startWaveDig()
{
  int firstChan, lastChan, numChans, numPoints;
  int scanList[MAX_ANALOG_IN*2];
  int i;
  int extTrigger, extClock;
  int status;
  double dwell;
  static const char *functionName = "startWaveDig";

  getIntegerParam(waveDigNumPoints_,  &numPoints);
  getIntegerParam(waveDigFirstChan_,  &firstChan);
  getIntegerParam(waveDigNumChans_,   &numChans);
  numWaveDigChans_ = numChans;
  getIntegerParam(waveDigExtTrigger_, &extTrigger);
  getIntegerParam(waveDigExtClock_,   &extClock);
  getDoubleParam(waveDigDwell_, &dwell);

  lastChan = firstChan + numChans - 1;
  setIntegerParam(waveDigCurrentPoint_, 0);

  // Construct the scan list
  for (i=0; i<numChans; i++) {
    scanList[i] = LJT_AI_FLOAT32 + (firstChan+i)*2;
  }
  waveDigScansPerRead_ = (int) (WAVE_DIG_READ_TIME / dwell);
  if (waveDigScansPerRead_ < 1) waveDigScansPerRead_ = 1;
  double scanRate = 1./dwell;

  status = LJM_WriteLibraryConfigS(LJM_STREAM_SCANS_RETURN, LJM_STREAM_SCANS_RETURN_ALL_OR_NONE);
  status = LJM_eStreamStart(LJMHandle_, waveDigScansPerRead_, numChans, scanList, &scanRate);
  reportError(status, functionName, "Calling LJM_eStreamStart");
  if (status) return status;

  // Convert back from scanRate to dwell, since value might have changed
  dwell = (1. / scanRate);
  setDoubleParam(waveDigDwellActual_, dwell);
  waveDigRunning_ = 1;
  setIntegerParam(waveDigRun_, 1);
  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s: called LJM_eStreamStart, firstChan=%d, lastChan=%d, waveDigScansPerRead=%d, dwell=%f\n",
    driverName, functionName, firstChan, lastChan, waveDigScansPerRead_, dwell);

  setDoubleParam(waveDigTotalTime_, dwell*numPoints);
  return 0;
}

int LabJackDriver::stopWaveDig(bool restartOK)
{
  int autoRestart;
  int status;
  static const char *functionName = "stopWaveDig";

  waveDigRunning_ = 0;
  setIntegerParam(waveDigRun_, 0);
  readWaveDig();
  status = LJM_eStreamStop(LJMHandle_);
  reportError(status, functionName, "Calling LJM_eStreamStop");
  getIntegerParam(waveDigAutoRestart_, &autoRestart);
  if (autoRestart && restartOK) {
    epicsThreadSleep(0.1);
    status = startWaveDig();
  }

  return status;
}

int LabJackDriver::readWaveDig()
{
  int firstChan, lastChan;
  int currentPoint;
  int i;
  static const char *functionName = "readWaveDig";

  getIntegerParam(waveDigFirstChan_,    &firstChan);
  lastChan = firstChan + numWaveDigChans_ - 1;
  getIntegerParam(waveDigCurrentPoint_, &currentPoint);

  for (i=firstChan; i<=lastChan; i++) {
    asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
      "%s:%s:, doing callbacks on input %d, first value=%f\n",
      driverName, functionName, i, waveDigBuffer_[i][0]);
    doCallbacksFloat64Array(waveDigBuffer_[i], currentPoint, waveDigVoltWF_, i);
  }
  doCallbacksFloat64Array(waveDigAbsTimeBuffer_, currentPoint, waveDigAbsTimeWF_, 0);
  return 0;
}

int LabJackDriver::readoutWaveDig()
{
  int firstChan;
  int numPoints;
  int currentPoint;
  int deviceScanBacklog;
  int LJMScanBacklog;
  int status;
  epicsTimeStamp now;
  static const char *functionName = "readoutWaveDig";

  if (!waveDigRunning_) return asynSuccess;
  getIntegerParam(waveDigNumPoints_,    &numPoints);
  getIntegerParam(waveDigFirstChan_,    &firstChan);
  getIntegerParam(waveDigCurrentPoint_, &currentPoint);

  LJMScanBacklog = 1;
  while (LJMScanBacklog != 0) {
    status = LJM_eStreamRead(LJMHandle_, waveDigTempBuffer_, &deviceScanBacklog, &LJMScanBacklog);
    if (status == LJME_NO_SCANS_RETURNED) return 0;
    reportError(status, functionName, "LJM_eStreamRead");
    if (status == 2942) {  // There are no symbolic constants in LabJackM.h for the stream error codes
      stopWaveDig(false);
      return status;
    }
    // The buffer we just read contains waveDigScansPerRead time points
    epicsTimeGetCurrent(&now);
    epicsFloat64 *pInput = waveDigTempBuffer_;
    double aiValue;
    for (int scan=0; scan<waveDigScansPerRead_; scan++) {
      for (int chan=firstChan; chan<firstChan+numWaveDigChans_; chan++) {
        aiValue = *pInput++;
        computeAiValue(chan, &aiValue);
        waveDigBuffer_[chan][currentPoint] = aiValue;
      }
      waveDigAbsTimeBuffer_[currentPoint] = now.secPastEpoch + now.nsec/1.e9;
      currentPoint++;
      setIntegerParam(waveDigCurrentPoint_, currentPoint);
      if (currentPoint == numPoints) {
        stopWaveDig(true);
        return 0;
      }
    }
    callParamCallbacks();
  }
  return 0;
}

int LabJackDriver::computeWaveDigTimes()
{
  int numPoints, i;
  double dwell;

  getIntegerParam(waveDigNumPoints_, &numPoints);
  getDoubleParam(waveDigDwell_, &dwell);
  for (i=0; i<numPoints; i++) {
    waveDigTimeBuffer_[i] = (epicsFloat64) (i * dwell);
  }
  doCallbacksFloat64Array(waveDigTimeBuffer_, numPoints, waveDigTimeWF_, 0);
  return 0;
}

int LabJackDriver::defineWaveform(int channel)
{
  int waveGenType;
  int numPoints;
  int nPulse;
  int i;
  epicsFloat64 *outPtr = waveGenIntBuffer_[channel];
  double dwell, offset, base, amplitude, pulseWidth, scale;
  static const char *functionName = "defineWaveform";

  getIntegerParam(channel, waveGenWaveType_,  &waveGenType);
  if (waveGenType == waveGenTypeUser) {
    getIntegerParam(waveGenUserNumPoints_, &numPoints);
    if ((size_t)numPoints > maxOutputPoints_) {
      asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
        "%s:%s: ERROR numPoints=%d must be less than maxOutputPoints=%d\n",
        driverName, functionName, numPoints, (int)maxOutputPoints_);
      return -1;
    }
    getDoubleParam(waveGenUserDwell_, &dwell);
    setIntegerParam(waveGenNumPoints_, numPoints);
    setDoubleParam(waveGenDwell_, dwell);
    setDoubleParam(waveGenFreq_, 1./dwell/numPoints);
    return 0;
  }

  getIntegerParam(waveGenIntNumPoints_,  &numPoints);
  if ((size_t)numPoints > maxOutputPoints_) {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: ERROR numPoints=%d must be less than maxOutputPoints=%d\n",
      driverName, functionName, numPoints, (int)maxOutputPoints_);
    return -1;
  }

  getDoubleParam(waveGenIntDwell_,             &dwell);
  getDoubleParam(channel, waveGenOffset_,      &offset);
  getDoubleParam(channel, waveGenAmplitude_,   &amplitude);
  getDoubleParam(channel, waveGenPulseWidth_,  &pulseWidth);
  setIntegerParam(waveGenNumPoints_, numPoints);
  setDoubleParam(waveGenDwell_, dwell);
  setDoubleParam(waveGenFreq_, 1./dwell/numPoints);
  base = offset - amplitude/2.;
  switch (waveGenType) {
    case waveGenTypeSin:
      scale = 2.*PI/(numPoints-1);
      for (i=0; i<numPoints; i++)           *outPtr++ = (epicsFloat64) (offset + amplitude/2. * sin(i*scale));
      break;
    case waveGenTypeSquare:
      for (i=0; i<numPoints/2; i++)         *outPtr++ = (epicsFloat64) (base + amplitude);
      for (i=numPoints/2; i<numPoints; i++) *outPtr++ = (epicsFloat64) (base);
      break;
    case waveGenTypeSawTooth:
      scale = 1./(numPoints-1);
      for (i=0; i<numPoints; i++)           *outPtr++ = (epicsFloat64) (base + amplitude*i*scale);
      break;
    case waveGenTypePulse:
      nPulse = (int) ((pulseWidth / dwell) + 0.5);
      if (nPulse < 1) nPulse = 1;
      if (nPulse >= numPoints-1) nPulse = numPoints-1;
      for (i=0; i<nPulse; i++)              *outPtr++ = (epicsFloat64) (base + amplitude);
      for (i=nPulse; i<numPoints; i++)      *outPtr++ = (epicsFloat64) (base);
      break;
    case waveGenTypeRandom:
      scale = amplitude / RAND_MAX;
      srand(1);
      for (i=0; i<numPoints; i++)           *outPtr++ = (epicsFloat64) (base + rand() * scale);
      break;
  }
  doCallbacksFloat64Array(waveGenIntBuffer_[channel], numPoints, waveGenIntWF_, channel);
  return 0;
}

int LabJackDriver::startWaveGen()
{
  int status=0;
  int numPoints;
  int enable;
  int firstChan=-1, lastChan=-1, firstType=-1;
  int waveGenType;
  int extTrigger, extClock, continuous;
  int i;
  epicsFloat64* inPtr[MAX_ANALOG_OUT];
  static const char *functionName = "startWaveGen";

  getIntegerParam(waveGenExtTrigger_, &extTrigger);
  getIntegerParam(waveGenExtClock_,   &extClock);
  getIntegerParam(waveGenContinuous_, &continuous);

  // Assume failure, so we can exit on error.
  // It will be set back to 1 if everything succeeds
  setIntegerParam(waveGenRun_, 0);

  for (i=0; i<numAnalogOut_; i++) {
    getIntegerParam(i, waveGenEnable_, &enable);
    if (!enable) continue;
    getIntegerParam(i, waveGenWaveType_,  &waveGenType);
    if (waveGenType == waveGenTypeUser)
      inPtr[i] = waveGenUserBuffer_[i];
    else
      inPtr[i] = waveGenIntBuffer_[i];
    if (firstChan < 0) {
      firstChan = i;
      firstType = waveGenType;
    }
    // Cannot mix user-defined and internal waveform types, because internal modifies dwell time
    // based on frequency
    if (((firstType == waveGenTypeUser) && (waveGenType != waveGenTypeUser)) ||
        ((firstType != waveGenTypeUser) && (waveGenType == waveGenTypeUser))) {
      asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
        "%s:%s: ERROR if any enabled waveform type is user-defined then all must be.\n",
        driverName, functionName);
      return -1;
    }
    lastChan = i;
    status = defineWaveform(i);
    if (status) return -1;
  }

  if (firstChan < 0) {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: ERROR no enabled channels\n",
      driverName, functionName);
     return -1;
  }

  numWaveGenChans_ = lastChan - firstChan + 1;

  // dwell and numPoints were computed by defineWaveform above
  getIntegerParam(waveGenNumPoints_, &numPoints);
  int aScanList[MAX_ANALOG_OUT];

  for (i=0; i<numWaveGenChans_; i++) {
    // Allocate memory for the stream-out buffer
    status = LJM_eWriteAddress(LJMHandle_, LJT_STREAM_OUT_TARGET + 2*i, LJM_UINT32, LJT_DAC_FLOAT32 + 2*(firstChan+i));
    reportError(status, functionName, "LJM_eWriteAddress for STREAM_OUT_TARGET");
    // Buffer size is in bytes.  Each buffer element on the device is 2 bytes
    status = LJM_eWriteAddress(LJMHandle_, LJT_STREAM_OUT_BUFFER_SIZE + 2*i, LJM_UINT32, maxOutputPoints_*2);
    reportError(status, functionName, "LJM_eWriteAddress for STREAM_OUT_BUFFER_SIZE");
    status = LJM_eWriteAddress(LJMHandle_, LJT_STREAM_OUT_ENABLE + 2*i, LJM_UINT32, 1);
    reportError(status, functionName, "LJM_eWritAddress for STREAM_OUT_ENABLE");

    // Write values to the stream-out buffer
    status = LJM_eWriteAddress(LJMHandle_, LJT_STREAM_OUT_LOOP_SIZE + 2*i, LJM_UINT32, numPoints-1);
    reportError(status, functionName, "LJM_eWriteAddress for STREAM_OUT_LOOP_SIZE");
    int errorAddress;
    status = LJM_eWriteAddressArray(LJMHandle_, LJT_STREAM_OUT_BUFFER_F32 + 2*i, LJM_FLOAT32, numPoints, inPtr[firstChan+i], &errorAddress);
    reportError(status, functionName, "LJM_eWriteAddressArray for STREAM_OUT_BUFFER_F32");
    // SET_LOOP is called with 0 for one-shot and 1 for repeating
    status = LJM_eWriteAddress(LJMHandle_, LJT_STREAM_OUT_SET_LOOP +2*i, LJM_UINT32, continuous);
    reportError(status, functionName, "LJM_eWriteAddress for STREAM_OUT_SET_LOOP");

    status = LJM_eWriteName(LJMHandle_, "STREAM_TRIGGER_INDEX", 0);
    reportError(status, functionName, "LJM_eWriteName for STREAM_TRIGGER_INDEX");
    if (model_ != modelT4) {
      status = LJM_eWriteName(LJMHandle_, "STREAM_CLOCK_SOURCE", 0);
      reportError(status, functionName, "LJM_eWriteName for STREAM_CLOCK_SOURCE");
    }

    aScanList[i] = LJT_STREAM_OUT + i;
  }

  // On the T7-Pro we need to set the ResolutionAll to a maximum of 8 in order to read inputs while scanning
  if (model_ == modelT7Pro) {
    int resolution;
    getIntegerParam(ainResolutionAll_, &resolution);
    if ((resolution == 0) || (resolution > 8)) {
      status = LJM_eWriteName(LJMHandle_, "AIN_ALL_RESOLUTION_INDEX", 8);
      reportError(status, functionName, "Calling LJM_eWriteName for AIN_ALL_RESOLUTION_INDEX");
    }
  }

  int scansPerRead = 1;
  double dwell;
  getDoubleParam(waveGenDwell_, &dwell);
  double rate = 1./dwell;
  status = LJM_eStreamStart(LJMHandle_, scansPerRead, numWaveGenChans_, aScanList, &rate);
  reportError(status, functionName, "LJM_eStreamStart");
  if (status) return status;

  // The dwell may have changed, set the actual value
  dwell = 1./rate;
  waveGenRunning_ = 1;
  setIntegerParam(waveGenRun_, 1);
  setDoubleParam(waveGenDwellActual_, dwell);
  setDoubleParam(waveGenTotalTime_, dwell*numPoints);
  return 0;
}

int LabJackDriver::stopWaveGen()
{
  waveGenRunning_ = 0;
  setIntegerParam(waveGenRun_, 0);
  return LJM_eStreamStop(LJMHandle_);
}

int LabJackDriver::computeWaveGenTimes()
{
  int numPoints, i;
  double dwell;

  getIntegerParam(waveGenUserNumPoints_, &numPoints);
  getDoubleParam(waveGenUserDwell_, &dwell);
  for (i=0; i<numPoints; i++) {
    waveGenUserTimeBuffer_[i] = (epicsFloat64) (i * dwell);
  }
  doCallbacksFloat64Array(waveGenUserTimeBuffer_, numPoints, waveGenUserTimeWF_, 0);

  getIntegerParam(waveGenIntNumPoints_, &numPoints);
  getDoubleParam(waveGenIntDwell_, &dwell);
  for (i=0; i<numPoints; i++) {
    waveGenIntTimeBuffer_[i] = (epicsFloat64) (i * dwell);
  }
  doCallbacksFloat64Array(waveGenIntTimeBuffer_, numPoints, waveGenIntTimeWF_, 0);
  return 0;
}

// Converts from volts to temperature when needed
int LabJackDriver::computeAiValue(int chan, double *aiValue) {
  static const char *functionName = "computeAiValue";
  double coldJunctionTemp;
  int status = asynSuccess;
  getDoubleParam(deviceTemperature_, &coldJunctionTemp);
  int aiMode;
  getIntegerParam(chan, analogInMode_, &aiMode);
  if (aiMode != 0) {
    // Thermocouple mode
    int tempUnits;
    getIntegerParam(chan, temperatureUnits_, &tempUnits);
    double tempK;
    status = LJM_TCVoltsToTemp(aiMode, *aiValue, coldJunctionTemp, &tempK);
    reportError(status, functionName, "Error calling LJM_TCVoltsToTemp");
    switch (tempUnits) {
      case temperatureUnitsK:
        *aiValue = tempK;
        break;
      case temperatureUnitsC:
        *aiValue = tempK - 273.15;
        break;
      case temperatureUnitsF:
        *aiValue = (tempK - 273.15) * 9./5. + 32;
        break;
    }
  }
  return status;
}

void LabJackDriver::pollerThread(void * pPvt)
{
  LabJackDriver *pLabJackDriver = (LabJackDriver *)pPvt;
  pLabJackDriver->pollerThread();
}

void LabJackDriver::pollerThread()
{
  /* This function runs in a separate thread.  It waits for the poll
   * time */
  static const char *functionName = "pollerThread";
  epicsUInt32 newValue, changedBits, prevInput=0;
  int i;
  double dblTemp;
  epicsTime startTime=epicsTime::getCurrent(), endTime;
  int status=0, prevStatus=0;

  while(1) {
    lock();
    endTime = epicsTime::getCurrent();
    setDoubleParam(pollTimeMS_, (endTime-startTime)*1000.);
    startTime = epicsTime::getCurrent();

    // Read the digital inputs
    status = LJM_eReadAddress(LJMHandle_, LJT_DIO_STATE, LJM_INT32, &dblTemp);
    reportError(status, functionName, "Calling LJM_eReadAddress");
    newValue = (epicsUInt32) dblTemp;
    if (status) {
      if (!prevStatus) {
        reportError(status, functionName, "Reading DIO_STATE");
      }
      goto error;
    }
    changedBits = newValue ^ prevInput;
    if (forceCallback_ || (changedBits != 0)) {
      prevInput = newValue;
      forceCallback_ = 0;
      setUIntDigitalParam(0, digitalInWord_, newValue, 0xFFFFFFFF);
      // Extract the FIO, EIO, CIO, and MIO bits
      setUIntDigitalParam(1, digitalInWord_, newValue>>0, 0xFF);
      setUIntDigitalParam(2, digitalInWord_, newValue>>8, 0xFF);
      setUIntDigitalParam(3, digitalInWord_, newValue>>16, 0xF);
      setUIntDigitalParam(4, digitalInWord_, newValue>>20, 0x7);
    }

    if (waveGenRunning_) {
      int continuous;
      getIntegerParam(waveGenContinuous_, &continuous);
      int numPoints;
      getIntegerParam(waveGenNumPoints_, &numPoints);
      if (!continuous) {
        double bufferStatus;
        status = LJM_eReadName(LJMHandle_, "STREAM_OUT0_BUFFER_STATUS", &bufferStatus);
        reportError(status, functionName, "Calling LJM_eReadName for STREAM_OUT0_BUFFER_STATUS");
        // In single-shot mode bufferStatus is the amount of free buffer space; <= maxOutputPoints_
        int bufferUsed = maxOutputPoints_ - (int)bufferStatus;
        int currentPoint = numPoints - bufferUsed;
        //asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "%s::%s bufferStatus=%f, bufferUsed=%d, currentPoint=%d\n",
        //         driverName, functionName, bufferStatus, bufferUsed, currentPoint);
        setIntegerParam(waveGenCurrentPoint_, currentPoint);
        // If this is the last point stop the stream output
        if (currentPoint == numPoints) {
          stopWaveGen();
        }
      }
    }

    if (waveDigRunning_) {
      readoutWaveDig();
    } else {
      // Read the analog inputs
      // Set all channels to -9999.9999 so disabled channels are obvious
      for (i=0; i<MAX_ANALOG_IN; i++)
        setDoubleParam(i, analogInValue_, -9999.9999);
      status = readAnalogInputs();
      if (status) {
        if (!prevStatus) {
          reportError(status, functionName, "Calling LJM_eReadAddresses for analog inputs");
        }
        goto error;
      }
      int numChannels = (int)activeAiChannels_.size();
      for (i=0; i<numChannels; i++) {
        int chan = activeAiChannels_[i];
        double aiValue = aiValues_[i];
        computeAiValue(chan, &aiValue);
        setDoubleParam(chan, analogInValue_, aiValue);
      }
    }
    // Do callbacks
    for (i=0; i<MAX_SIGNALS; i++) {
      callParamCallbacks(i);
    }

error:
    if (prevStatus && !status) {
      reportError(-1, functionName, "Device returned to normal status");
    }
    prevStatus = status;
    double pollTime;
    getDoubleParam(pollSleepMS_, &pollTime);
    unlock();
    epicsThreadSleep(pollTime/1000.);
  }
}

/** Configuration command, called directly or from iocsh */
extern "C" int LabJackConfig(const char *portName, const char *uniqueID,
                             int maxInputPoints, int maxOutputPoints)
{
  new LabJackDriver(portName, uniqueID, maxInputPoints, maxOutputPoints);
  return asynSuccess;
}


static const iocshArg configArg0 = { "Port name",      iocshArgString};
static const iocshArg configArg1 = { "UniqueID",       iocshArgString};
static const iocshArg configArg2 = { "Max. input points", iocshArgInt};
static const iocshArg configArg3 = { "Max. output points",iocshArgInt};
static const iocshArg * const configArgs[] = {&configArg0,
                                              &configArg1,
                                              &configArg2,
                                              &configArg3};
static const iocshFuncDef configFuncDef = {"LabJackConfig",4,configArgs};
static void configCallFunc(const iocshArgBuf *args)
{
  LabJackConfig(args[0].sval, args[1].sval, args[2].ival, args[3].ival);
}


static const iocshFuncDef showDevicesFuncDef = {"LabJackShowDevices",0,0};
static void LabJackShowDevices(const iocshArgBuf *args)
{
  int status;
  int numFound;
  int deviceTypes[LJM_LIST_ALL_SIZE];
  int connectionTypes[LJM_LIST_ALL_SIZE];
  int serialNumbers[LJM_LIST_ALL_SIZE];
  int ipAddresses[LJM_LIST_ALL_SIZE];
  char ipV4String[LJM_IPv4_STRING_SIZE];
  static const char *functionName = "LabJackShowDevices";

  status = LJM_ListAll(0, 0, &numFound, deviceTypes, connectionTypes, serialNumbers, ipAddresses);
  if (status) {
    printf("%s::%s error calling LJM_ListAll, status=%d\n", driverName, functionName, status);
  } else {
    printf("LJM_ListAll found %d devices.\n", numFound);
    for (int i=0; i<numFound; i++) {
      printf("  Device %d:\n", i);
      printf("        Device type: %d\n", deviceTypes[i]);
      printf("    Connection type: %d\n", connectionTypes[i]);
      printf("      Serial number: %d\n", serialNumbers[i]);
      LJM_NumberToIP(ipAddresses[i], ipV4String);
      printf("         IP address: %s\n", ipV4String);
    }
  }
}

void LabJackRegister(void)
{
  iocshRegister(&configFuncDef,configCallFunc);
  iocshRegister(&showDevicesFuncDef,LabJackShowDevices);
}

extern "C" {
epicsExportRegistrar(LabJackRegister);
}
