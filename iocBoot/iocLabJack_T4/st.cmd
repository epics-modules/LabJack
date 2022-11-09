#------------------------------------------------------------------------------
# Define environment variables
< envPaths

epicsEnvSet PORT LJT4_1
epicsEnvSet PREFIX LabJackT4_1:
epicsEnvSet WDIG_POINTS 2048
epicsEnvSet WGEN_POINTS 2048

#------------------------------------------------------------------------------
# Register all support components
dbLoadDatabase("../../dbd/LabJackApp.dbd")
LabJackApp_registerRecordDeviceDriver(pdbbase)

#------------------------------------------------------------------------------
# Create driver
LabJackShowDevices
# If identifying the device by IP name it must be fully qualified, i.e. include periods.
LabJackConfig("$(PORT)", "gse-labjack4.cars.aps.anl.gov", $(WDIG_POINTS), $(WGEN_POINTS))
#LabJackConfig("$(PORT)", "10.54.160.75",                  $(WDIG_POINTS), $(WGEN_POINTS))
#LabJackConfig("$(PORT)", "440010541",                     $(WDIG_POINTS), $(WGEN_POINTS))

#------------------------------------------------------------------------------
###  LabJack support  ###
# If running multiple LabJack devices in the same IOC must set P and PORT differently for each one.
dbLoadTemplate("$(LABJACK)/db/LabJack_T7.substitutions", "P=$(PREFIX), PORT=$(PORT), WDIG_POINTS=$(WDIG_POINTS), WGEN_POINTS=$(WGEN_POINTS)")

#------------------------------------------------------------------------------
# Configure auto save/restore
#
iocshLoad("../save_restore.cmd", PREFIX=$(PREFIX))

#------------------------------------------------------------------------------
# Start IOC
iocInit()

#------------------------------------------------------------------------------
# Start autosave

# Monitor every ten seconds
create_monitor_set("auto_settings.req", 10, "P=$(PREFIX)")
