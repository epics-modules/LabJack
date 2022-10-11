# LabJack_T7_Common.cmd

# The following environment variables must be set before loading this file

#     PREFIX : The PV prefix for this IOC    
# IP_ADDRESS : The IP address or name of this Labjack
#          N : The unit number (1 for first, 2 for second, etc.)

# Use the following commands for TCP/IP
#drvAsynIPPortConfigure(const char *portName,
#                       const char *hostInfo,
#                       unsigned int priority,
#                       int noAutoConnect,
#                       int noProcessEos);
#
# 3.) LabJack unit (N) IP address
drvAsynIPPortConfigure("LJT7_$(N)","$(IP_ADDRESS):502",0,0,0)

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
#drvModbusAsynConfigure("LJT7_$(N)_FIO0_In_Word", "LJT7_$(N)", 1, 3, 2000,  1, 0, 100, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_FIO1_In_Word", "LJT7_$(N)", 1, 3, 2001,  1, 0, 100, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_FIO2_In_Word", "LJT7_$(N)", 1, 3, 2002,  1, 0, 100, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_FIO3_In_Word", "LJT7_$(N)", 1, 3, 2003,  1, 0, 100, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_FIO4_In_Word", "LJT7_$(N)", 1, 3, 2004,  1, 0, 100, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_FIO5_In_Word", "LJT7_$(N)", 1, 3, 2005,  1, 0, 100, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_FIO6_In_Word", "LJT7_$(N)", 1, 3, 2006,  1, 0, 100, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_FIO7_In_Word", "LJT7_$(N)", 1, 3, 2007,  1, 0, 100, "LJT7_module")

# Read 8 binary inputs in a single register. DIO0 - DIO7 (FIO bank)
drvModbusAsynConfigure("LJT7_$(N)_FIO_In_Word", "LJT7_$(N)", 1, 3, 2500,  1, 0, 100, "LJT7_module")
# DIO16 - DIO19 (CIO bank)
drvModbusAsynConfigure("LJT7_$(N)_CIO_In_Word", "LJT7_$(N)", 1, 3, 2502,  1, 0, 100, "LJT7_module")

# Read 14 analog inputs.
drvModbusAsynConfigure("LJT7_$(N)_AI_In", "LJT7_$(N)", 1, 3, 0,  28, 0, 100, "LJT7_module")

# Write to 14 registers to configure analog range.
drvModbusAsynConfigure("LJT7_$(N)_AIRange", "LJT7_$(N)", 0, 16, 40000,  28, FLOAT32_BE, 1, "LJT7_module")

# Write to 14 registers to configure analog resolution index.
drvModbusAsynConfigure("LJT7_$(N)_AIResolution", "LJT7_$(N)", 0, 6, 41500,  14, UINT16, 1, "LJT7_module")

# Write to register to set each even no. AI channel Single-Ended or Differential
drvModbusAsynConfigure("LJT7_$(N)_AIdiff_Out_Word",  "LJT7_$(N)", 0, 6, 41000, 13, 0, 1, "LJT7_module")


# Access 1 words as outputs.  
# Either function code=6 (single register) or 16 (multiple registers) can be used, but 16
# is better because it is "atomic" when writing values longer than 16-bits.
# Default data type unsigned integer.
# drvModbusAsynConfigure("portName", "tcpPortName", slaveAddress, modbusFunction, modbusStartAddress, modbusLength, dataType, pollMsec, "plcType")
# Not sure why the outputs can't be configured for Modbus data type 4?

# Set a digital output one register at a time.  DIO8 - DIO15 (EIO bank) plus DIO20 - DIO22 (MIO bank)
drvModbusAsynConfigure("LJT7_$(N)_EIO_Out_Word", "LJT7_$(N)", 0, 6, 2008, 8, 0, 1, "LJT7_module")
# MIO
drvModbusAsynConfigure("LJT7_$(N)_MIO_Out_Word", "LJT7_$(N)", 0, 6, 2020, 3, 0, 1, "LJT7_module")
#

# Set 2 analog outputs.
drvModbusAsynConfigure("LJT7_$(N)_AO_0", "LJT7_$(N)", 0, 16, 1000,  2, 8, 1, "LJT7_module")
drvModbusAsynConfigure("LJT7_$(N)_AO_1", "LJT7_$(N)", 0, 16, 1002,  2, 8, 1, "LJT7_module")

# 2 additional analog outputs, setup to access LJTick-DAC occupying FIO0/FIO1
drvModbusAsynConfigure("LJT7_$(N)_AO_2", "LJT7_$(N)", 0, 16, 30000,  2, 8, 0, "LJT7_module")
drvModbusAsynConfigure("LJT7_$(N)_AO_3", "LJT7_$(N)", 0, 16, 30002,  2, 8, 0, "LJT7_module")
# LJTick-DAC occupying EIO4/EIO5
#drvModbusAsynConfigure("LJT7_$(N)_AO_2", "LJT7_$(N)", 0, 16, 30024,  2, 8, 0, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_AO_3", "LJT7_$(N)", 0, 16, 30026,  2, 8, 0, "LJT7_module")
# LJTick-DAC occupying EIO6/EIO7
#drvModbusAsynConfigure("LJT7_$(N)_AO_2", "LJT7_$(N)", 0, 16, 30028,  2, 8, 0, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_AO_3", "LJT7_$(N)", 0, 16, 30030,  2, 8, 0, "LJT7_module")
# LJTick-DAC occupying CIO0/CIO1
#drvModbusAsynConfigure("LJT7_$(N)_AO_4", "LJT7_$(N)", 0, 16, 30032,  2, 8, 0, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_AO_5", "LJT7_$(N)", 0, 16, 30034,  2, 8, 0, "LJT7_module")
# LJTick-DAC occupying CIO2/CIO3
#drvModbusAsynConfigure("LJT7_$(N)_AO_6", "LJT7_$(N)", 0, 16, 30036,  2, 8, 0, "LJT7_module")
#drvModbusAsynConfigure("LJT7_$(N)_AO_7", "LJT7_$(N)", 0, 16, 30038,  2, 8, 0, "LJT7_module")

dbLoadTemplate("LabJack_T7_$(N).substitutions", "P=$(P)")
