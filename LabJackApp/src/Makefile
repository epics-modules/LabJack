TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#=============================
# Build the IOC support library

DBD += LabJackApp.dbd

LabJack_LIBS += asyn
LabJack_LIBS += modbus
LabJack_LIBS += $(EPICS_BASE_IOC_LIBS)

LabJack_LIBS_vxWorks += asyn
LabJack_LIBS_vxWorks += modbus
LabJack_LIBS_vxWorks += $(EPICS_BASE_IOC_LIBS)

#=============================
# Build the IOC application

PROD_IOC = LabJackApp

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
LabJackApp_SRCS += LabJackApp_registerRecordDeviceDriver.cpp
LabJackApp_SRCS_vxWorks += LabJackAppVxWorks_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
LabJackApp_SRCS += LabJackAppMain.cpp

# Link in the code from the support library
LabJackApp_LIBS += asyn modbus autosave

# Finally link to the EPICS Base libraries
LabJackApp_LIBS += $(EPICS_BASE_IOC_LIBS)

#
#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

