TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#=============================
# Build the IOC support library

DBD += LabJackApp.dbd
DBD += LabJackSupport.dbd

LIBRARY_IOC_WIN32 += LabJack
LIBRARY_IOC_Linux += LabJack

LabJack_LIBS += asyn
LabJack_LIBS += LabJackM
LabJack_LIBS += $(EPICS_BASE_IOC_LIBS)

LabJack_SRCS += LabJackDriver.cpp

#=============================
# Build the IOC application

PROD_IOC_WIN32 += LabJackApp
PROD_IOC_Linux += LabJackApp
LabJackApp_DBD += base.dbd asyn.dbd busySupport.dbd asSupport.dbd LabJackSupport.dbd

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
LabJackApp_SRCS += LabJackApp_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
LabJackApp_SRCS += LabJackAppMain.cpp

# Link in the code from the support library
LabJackApp_LIBS += LabJack
LabJackApp_LIBS += busy
LabJackApp_LIBS += autosave
LabJackApp_LIBS += asyn
LabJackApp_LIBS += LabJackM

# Finally link to the EPICS Base libraries
LabJackApp_LIBS += $(EPICS_BASE_IOC_LIBS)

PROD_IOC_Linux += test_temp_t7pro
test_temp_t7pro_SRCS += test_temp_t7pro.c
test_temp_t7pro_LIBS += LabJackM

PROD_IOC_Linux += LabJackShowDevices
PROD_IOC_WIN32 += LabJackShowDevices
LabJackShowDevices_SRCS += LabJackShowDevices.c
LabJackShowDevices_LIBS += LabJackM

#
#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

