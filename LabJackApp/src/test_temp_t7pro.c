/**
 * Name: test_device_temp_glitch.c
 * Desc: Tests whether there is a glitch on AIN0 immediately after reading the DEVICE_TEMPERATURE_K register.
 *       This problem occurs on the T7-PRO.
 * Wiring for test: Thermocouple connected to AIN0/AIN1, DAC0 connected to AIN2, DAC1 connected to AIN3, all others open
 *        
**/

// For printf
#include <stdio.h>

// For the LabJackM Library
#include <LabJackM.h>

// For LabJackM helper functions, such as OpenOrDie, PrintDeviceInfoFromHandle, ErrorCheck, etc.
#include "LJM_Utilities.h"
#include "LJM_StreamUtilities.h"

int main(int argc, char *argv[])
{
  const int NUM_DAC_LOOPS = 4;
  const int SETTLING_TIME_US = 2000;
  const int MAX_AIN_CHANNELS = 14;
  int err;
  int handle;
  int i, j, k;
  double voltsIn[MAX_AIN_CHANNELS];
  int errorsIn[MAX_AIN_CHANNELS];
  double coldJunctionTemperature;
  double tempK;
  double dac0Values[] = {0,0,5,5};
  double dac1Values[] = {0,5,0,5};
  int channelsToRead;
  int ain0Resolution;
  int readingsPerLoop;

  // Modbus addresses
  const int LJT_AIN0            = 0; 
  const int LJT_AI_RANGE        = 40000;
  const int LJT_AI_DIFFERENTIAL = 41000;
  const int LJT_AI_RESOLUTION   = 41500;

  if (argc != 5) {
    printf("Usage: test_temp_t7pro IPAddress ain0Resolution channelsToRead readingsPerLoop\n");
    return 0;
  }
  
  // Open the T7-PRO LabJack at IP address 10.54.160.73
  err = LJM_Open(LJM_dtANY, LJM_ctANY, argv[1], &handle);
  ErrorCheck(err, "LJM_Open");

  PrintDeviceInfoFromHandle(handle);

  printf("\nConfiguration:\n");
  ain0Resolution = atoi(argv[2]);
  printf("    ain0Resolution: %d\n", ain0Resolution);
  channelsToRead = atoi(argv[3]);
  printf("    channelsToRead: %d\n", channelsToRead);
  readingsPerLoop = atoi(argv[4]);
  printf("    readingsPerLoop: %d\n", readingsPerLoop);

  DisableStreamIfEnabled(handle);

  // AIN0 is differential, negative channel is 1
  WriteNameOrDie(handle, "AIN0_NEGATIVE_CH", 1);
  printf("    AIN0_NEGATIVE_CH : %d\n", 1);

  // All other channels are single-ended
  for (i=2; i<channelsToRead; i++) {
    err = LJM_eWriteAddress(handle, LJT_AI_DIFFERENTIAL+i, LJM_UINT16, 199);
    ErrorCheck(err, "LJM_eWriteAddress for LJT_AI_DIFFERENTIAL");
    printf("    AIN%d_NEGATIVE_CH : %d\n", i, 199);
  }

  // Set range for each channel. 0.01V for AIN0
  WriteNameOrDie(handle, "AIN0_RANGE", 0.01);
  printf("    AIN0_RANGE : %f\n", 0.01);
  // All other channels are 10V
  for (i=2; i<channelsToRead; i++) {
    err = LJM_eWriteAddress(handle, LJT_AI_RANGE+2*i, LJM_FLOAT32, 10.);
    ErrorCheck(err, "LJM_eWriteAddress for LJT_AI_RANGE");
    printf("    AIN%d_NEGATIVE_CH : %f\n", i, 10.);
  }

  WriteNameOrDie(handle, "AIN0_RESOLUTION_INDEX", ain0Resolution);
  printf("    AIN0_RESOLUTION_INDEX : %d\n", ain0Resolution);
  // All other channels are 0 (default)
  for (i=2; i<channelsToRead; i++) {
    err = LJM_eWriteAddress(handle, LJT_AI_RESOLUTION+i, LJM_UINT16, 0);
    ErrorCheck(err, "LJM_eWriteAddress for LJT_AI_RESOLUTION");
    printf("    AIN%d_RESOLUTION_INDEX: %d\n", i, 0);
  }

  // Settling time for all channels
  WriteNameOrDie(handle, "AIN_ALL_SETTLING_US", SETTLING_TIME_US);
  printf("    AIN_ALL_SETTLING_US : %d\n", SETTLING_TIME_US);

  for (i=0; i<NUM_DAC_LOOPS; i++) {
    err = LJM_eWriteName(handle, "DAC0", dac0Values[i]);
    ErrorCheck(err, "LJM_eWriteName writing DAC0");
    err = LJM_eWriteName(handle, "DAC1", dac1Values[i]);
    ErrorCheck(err, "LJM_eWriteName writing DAC1");
    err = LJM_eReadName(handle, "TEMPERATURE_DEVICE_K", &coldJunctionTemperature);
    ErrorCheck(err, "LJM_eReadName reading TEMPERATURE_DEVICE_K");
    printf("\nCold junction temperature (C): %f, DAC0: %f, DAC1: %f\n", coldJunctionTemperature-273.15, dac0Values[i], dac1Values[i]);
    long long startTime = GetCurrentTimeMS();
    for (j=0; j<readingsPerLoop; j++) {
      err = LJM_eReadAddressArray(handle, LJT_AIN0, LJM_FLOAT32, channelsToRead, voltsIn, errorsIn);
      ErrorCheck(err, "LJM_eReadAddressArray reading analog inputs");
      err = LJM_TCVoltsToTemp(LJM_ttK, voltsIn[0], coldJunctionTemperature, &tempK);
      long long elapsedTime = GetCurrentTimeMS() - startTime;
      printf("Point: %d, Elapsed ms: %lld, Temp (C): %f", j, elapsedTime, tempK - 273.15); 
      for (k=2; k<channelsToRead; k++) printf(", %f", voltsIn[k]);
      printf("\n");
    }
  }

  // Stream data from AIN0
  const int numChannels = 1;
  const int numScans = 256;
  const unsigned int numSamples = numChannels * numScans;
  int aScanList[numChannels];
  double scanRate = 500;
  double aBurstSamples[numSamples];
  const char * POS_NAMES[] = {"AIN0"};
  const int STREAM_TRIGGER_INDEX = 0;
  const int STREAM_CLOCK_SOURCE = 0;
  const int STREAM_RESOLUTION_INDEX = 8;
  const double STREAM_SETTLING_US = 0;
  unsigned int timeStart, timeEnd;

  printf("\nConfiguring stream:\n");

  printf("    Setting STREAM_TRIGGER_INDEX to %d\n", STREAM_TRIGGER_INDEX);
  WriteNameOrDie(handle, "STREAM_TRIGGER_INDEX", STREAM_TRIGGER_INDEX);

  printf("    Setting STREAM_CLOCK_SOURCE to %d\n", STREAM_CLOCK_SOURCE);
  WriteNameOrDie(handle, "STREAM_CLOCK_SOURCE", STREAM_CLOCK_SOURCE);

  printf("    Setting STREAM_RESOLUTION_INDEX to %d\n", STREAM_RESOLUTION_INDEX);
  WriteNameOrDie(handle, "STREAM_RESOLUTION_INDEX", STREAM_RESOLUTION_INDEX);

  printf("    Setting STREAM_SETTLING_US to %f\n", STREAM_SETTLING_US);
  WriteNameOrDie(handle, "STREAM_SETTLING_US", STREAM_SETTLING_US);

  err = LJM_NamesToAddresses(numChannels, POS_NAMES, aScanList, NULL);
  ErrorCheck(err, "Getting channel addresses");

  memset(aBurstSamples, 0, numSamples * sizeof(double));

  printf("\nStarting stream:\n");
  printf("    scan rate: %.02f Hz (%.02f sample rate)\n",
    scanRate, scanRate * numChannels);
  printf("    number of scans  : %u\n", numScans);
  printf("    number of samples: %u\n", numScans * numChannels);

  timeStart = GetCurrentTimeMS();
  err = LJM_StreamBurst(handle, numChannels, aScanList, &scanRate, numScans, aBurstSamples);
  timeEnd = GetCurrentTimeMS();
  ErrorCheck(err, "LJM_eStreamBurst");

  printf("\nStream burst complete:\n");
  printf("    Actual scanRate was: %f\n", scanRate);
  printf("    %d scans over approximately %d milliseconds\n", numScans, timeEnd - timeStart);

  // Print the scan data in temperature
  printf("\nStream data:\n");
  for (i=0; i<numScans; i++) {
    err = LJM_TCVoltsToTemp(LJM_ttK, aBurstSamples[i], coldJunctionTemperature, &tempK);
    printf("Point %d: Temp (C): %f\n", i, tempK - 273.15); 
  }

  err = LJM_Close(handle);
  ErrorCheck(err, "LJM_Close");

  return LJME_NOERROR;
}
