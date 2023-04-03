#include <stdio.h>
#include "LabJackM.h"

int main (int argc, const char* argv[])
{
  int status;
  int numFound;
  int deviceTypes[LJM_LIST_ALL_SIZE];
  int connectionTypes[LJM_LIST_ALL_SIZE];
  int serialNumbers[LJM_LIST_ALL_SIZE];
  int ipAddresses[LJM_LIST_ALL_SIZE];
  int i;
  char ipV4String[LJM_IPv4_STRING_SIZE];

  status = LJM_ListAll(0, 0, &numFound, deviceTypes, connectionTypes, serialNumbers, ipAddresses);
  if (status) {
    printf("LabJackShowDevices error calling LJM_ListAll, status=%d\n", status);
    return -1;
  } else {
    printf("LJM_ListAll found %d devices.\n", numFound);
    for (i=0; i<numFound; i++) {
      printf("  Device %d:\n", i);
      printf("        Device type: %d\n", deviceTypes[i]);
      printf("    Connection type: %d\n", connectionTypes[i]);
      printf("      Serial number: %d\n", serialNumbers[i]);
      LJM_NumberToIP(ipAddresses[i], ipV4String);
      printf("         IP address: %s\n", ipV4String);
    }
  }
  return 0;
}
