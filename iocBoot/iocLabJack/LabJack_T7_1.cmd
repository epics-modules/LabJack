#
# LabJack_T7_1.cmd
#

### Configure this file for the LabJack unit.  Set the following at a minimum.
###    1.) LabJack unit number
###    2.) EPICS prefix
###    3.) LabJack unit IP number

# 1.) LabJack unit number n.  Will need a corresponding LabJack_T7_n.substitutions file.
# epicsEnvSet N n
epicsEnvSet N 1

# 2.) Prefix
# epicsEnvSet P prefix:
epicsEnvSet P xyz:

# Use the following commands for TCP/IP
#drvAsynIPPortConfigure(const char *portName,
#                       const char *hostInfo,
#                       unsigned int priority,
#                       int noAutoConnect,
#                       int noProcessEos);
#
# 3.) LabJack unit (N) IP address
drvAsynIPPortConfigure("LJT7_$(N)","192.168.100.30:502",0,0,1)

#modbusInterposeConfig(const char *portName,
#                      modbusLinkType linkType,
#                      int timeoutMsec, 
#                      int writeDelayMsec)
# One per controller.  One controller can support up to six drivers.
modbusInterposeConfig("LJT7_$(N)",0,2000,0)

# Word access at Modbus address 0
# Access 1 words as inputs.  
# Modbus standard functions 3 (Read Multiple), 4 (Read One), 6 (Write One), and 16 (Write Multiple). 
# default data type unsigned integer.
# drvModbusAsynConfigure("portName", "tcpPortName", slaveAddress, modbusFunction, modbusStartAddress, modbusLength, dataType, pollMsec, "plcType")
# 
# Read a digital input one register at a time.  DIO0 - DIO7 (FIO)
#drvModbusAsynConfigure("LJT7_$(N)_FIO0_In_Word", "LJT7_$(N)", 1, 3, 2000,  01, 0, 100, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_FIO1_In_Word", "LJT7_$(N)", 1, 3, 2001,  01, 0, 100, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_FIO2_In_Word", "LJT7_$(N)", 1, 3, 2002,  01, 0, 100, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_FIO3_In_Word", "LJT7_$(N)", 1, 3, 2003,  01, 0, 100, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_FIO4_In_Word", "LJT7_$(N)", 1, 3, 2004,  01, 0, 100, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_FIO5_In_Word", "LJT7_$(N)", 1, 3, 2005,  01, 0, 100, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_FIO6_In_Word", "LJT7_$(N)", 1, 3, 2006,  01, 0, 100, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_FIO7_In_Word", "LJT7_$(N)", 1, 3, 2007,  01, 0, 100, "LJT7_module")

# Read 8 binary inputs in a single register. DIO0 - DIO7 (FIO bank)
drvModbusAsynConfigure("LJT7_$(N)_FIO_In_Word", "LJT7_$(N)", 1, 3, 2500,  01, 0, 100, "LJT7_module")
# DIO16 - DIO19 (CIO bank)
drvModbusAsynConfigure("LJT7_$(N)_CIO_In_Word", "LJT7_$(N)", 1, 3, 2502,  01, 0, 100, "LJT7_module")

# Read 14 analog inputs.
drvModbusAsynConfigure("LJT7_$(N)_AI_In", "LJT7_$(N)", 1, 3, 0,  28, 0, 100, "LJT7_module")


# Access 1 words as outputs.  
# Either function code=6 (single register) or 16 (multiple registers) can be used, but 16
# is better because it is "atomic" when writing values longer than 16-bits.
# Default data type unsigned integer.
# drvModbusAsynConfigure("portName", "tcpPortName", slaveAddress, modbusFunction, modbusStartAddress, modbusLength, dataType, pollMsec, "plcType")
# Not sure why the outputs can't be configured for Modbus data type 4?

# Set a digital output one register at a time.  DIO8 - DIO15 (EIO bank) plus DIO20 - DIO22 (MIO bank)
drvModbusAsynConfigure("LJT7_$(N)_EIO0_Out_Word", "LJT7_$(N)", 0, 6, 2008, 1, 0, 1, "LJT7_module")
drvModbusAsynConfigure("LJT7_$(N)_EIO1_Out_Word", "LJT7_$(N)", 0, 6, 2009, 1, 0, 1, "LJT7_module")
drvModbusAsynConfigure("LJT7_$(N)_EIO2_Out_Word", "LJT7_$(N)", 0, 6, 2010, 1, 0, 1, "LJT7_module")
drvModbusAsynConfigure("LJT7_$(N)_EIO3_Out_Word", "LJT7_$(N)", 0, 6, 2011, 1, 0, 1, "LJT7_module")
drvModbusAsynConfigure("LJT7_$(N)_EIO4_Out_Word", "LJT7_$(N)", 0, 6, 2012, 1, 0, 1, "LJT7_module")
drvModbusAsynConfigure("LJT7_$(N)_EIO5_Out_Word", "LJT7_$(N)", 0, 6, 2013, 1, 0, 1, "LJT7_module")
drvModbusAsynConfigure("LJT7_$(N)_EIO6_Out_Word", "LJT7_$(N)", 0, 6, 2014, 1, 0, 1, "LJT7_module")
drvModbusAsynConfigure("LJT7_$(N)_EIO7_Out_Word", "LJT7_$(N)", 0, 6, 2015, 1, 0, 1, "LJT7_module")
# MIO
drvModbusAsynConfigure("LJT7_$(N)_MIO0_Out_Word", "LJT7_$(N)", 0, 6, 2020, 1, 0, 1, "LJT7_module")
drvModbusAsynConfigure("LJT7_$(N)_MIO1_Out_Word", "LJT7_$(N)", 0, 6, 2021, 1, 0, 1, "LJT7_module")
drvModbusAsynConfigure("LJT7_$(N)_MIO2_Out_Word", "LJT7_$(N)", 0, 6, 2022, 1, 0, 1, "LJT7_module")
#
# Write to register to set each even no. AI channel Single-Ended or Differential
drvModbusAsynConfigure("LJT7_$(N)_AI0diff_Out_Word", "LJT7_$(N)", 0, 6, 41000, 1, 0, 1, "LJT7_module")
drvModbusAsynConfigure("LJT7_$(N)_AI2diff_Out_Word", "LJT7_$(N)", 0, 6, 41002, 1, 0, 1, "LJT7_module")
drvModbusAsynConfigure("LJT7_$(N)_AI4diff_Out_Word", "LJT7_$(N)", 0, 6, 41004, 1, 0, 1, "LJT7_module")
drvModbusAsynConfigure("LJT7_$(N)_AI6diff_Out_Word", "LJT7_$(N)", 0, 6, 41006, 1, 0, 1, "LJT7_module")
drvModbusAsynConfigure("LJT7_$(N)_AI8diff_Out_Word", "LJT7_$(N)", 0, 6, 41008, 1, 0, 1, "LJT7_module")
drvModbusAsynConfigure("LJT7_$(N)_AI10diff_Out_Word", "LJT7_$(N)", 0, 6, 41010, 1, 0, 1, "LJT7_module")
drvModbusAsynConfigure("LJT7_$(N)_AI12diff_Out_Word", "LJT7_$(N)", 0, 6, 41012, 1, 0, 1, "LJT7_module")

# Set 2 analog outputs.
drvModbusAsynConfigure("LJT7_$(N)_AO_0", "LJT7_$(N)", 0, 16, 1000,  2, 8, 1, "LJT7_module")
drvModbusAsynConfigure("LJT7_$(N)_AO_1", "LJT7_$(N)", 0, 16, 1002,  2, 8, 1, "LJT7_module")

# 2 additional analog outputs, setup to access LJTick-DAC occupying FIO0/FIO1
#drvModbusAsynConfigure("LJT7_$(N)_AO_2", "LJT7_$(N)", 0, 16, 30000,  2, 8, 1, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_AO_3", "LJT7_$(N)", 0, 16, 30002,  2, 8, 1, "LJT7_module")
# LJTick-DAC occupying EIO4/EIO5
drvModbusAsynConfigure("LJT7_$(N)_AO_2", "LJT7_$(N)", 0, 16, 30024,  2, 8, 1, "LJT7_module")
drvModbusAsynConfigure("LJT7_$(N)_AO_3", "LJT7_$(N)", 0, 16, 30026,  2, 8, 1, "LJT7_module")
# LJTick-DAC occupying EIO6/EIO7
#drvModbusAsynConfigure("LJT7_$(N)_AO_2", "LJT7_$(N)", 0, 16, 30028,  2, 8, 1, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_AO_3", "LJT7_$(N)", 0, 16, 30030,  2, 8, 1, "LJT7_module")
# LJTick-DAC occupying CIO0/CIO1
#drvModbusAsynConfigure("LJT7_$(N)_AO_4", "LJT7_$(N)", 0, 16, 30032,  2, 8, 1, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_AO_5", "LJT7_$(N)", 0, 16, 30034,  2, 8, 1, "LJT7_module")
# LJTick-DAC occupying CIO2/CIO3
#drvModbusAsynConfigure("LJT7_$(N)_AO_6", "LJT7_$(N)", 0, 16, 30036,  2, 8, 1, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_AO_7", "LJT7_$(N)", 0, 16, 30038,  2, 8, 1, "LJT7_module")

# Prefixes in this file must match $(P) set above
dbLoadTemplate("LabJack_T7_$(N).substitutions")

# Digital Inputs
dbLoadRecords("$(LABJACK)/LabJackApp/Db/LabJack_T7_di.db", "P=$(P)LJT7:$(N):")

# Analog Inputs SE/Diff select
dbLoadRecords("$(LABJACK)/LabJackApp/Db/LabJack_T7_seDiff.db", "P=$(P)LJT7:$(N):")

