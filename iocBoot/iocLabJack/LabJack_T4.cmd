# LabJack_T4.cmd

# The following environment variables must be set before loading this file

#          P : The PV prefix for this IOC
# IP_ADDRESS : The IP address or name of this Labjack
#          N : The unit number (1 for first, 2 for second, etc.)

### * This file and related support files are all set up for the following
###   default channel configuration:
###
### Analog Inputs (8 ch) :  AIN0-3, FIO4-7
### Digital Inputs (8ch) :  EIO0-7
### Digital Outputs (4ch):  CIO0-3
### Analog Outputs (2ch) :  DAC0-1
###
### NOTE: Other configurations are possible by commenting in/out lines in
###       this file and the .substitutions file.  Database and displays will
###       also need to be modified accordingly.  See in-line comments and
###       https://labjack.com/support/datasheets/t-series for channel options.

# Use the following commands for TCP/IP
#drvAsynIPPortConfigure(const char *portName,
#                       const char *hostInfo,
#                       unsigned int priority,
#                       int noAutoConnect,
#                       int noProcessEos);
drvAsynIPPortConfigure("LJT4_$(N)","$(IP_ADDRESS):502",0,0,0)

#modbusInterposeConfig(const char *portName,
#                      modbusLinkType linkType,
#                      int timeoutMsec,
#                      int writeDelayMsec)
#
modbusInterposeConfig("LJT4_$(N)",0,2000,0)

#     Channel configuration, Modbus setup
#     First 4 channels are dedicated Analog Inputs: (AIN0 - AIN3)
#     The next 8 channels may be AI, DI, or DO:     (FIO4-FIO7, EIO0-EIO3)
#     The last 8 channels may be DI or DO:          (EIO4-EIO7, CIO0-CIO3)
#

# Modbus standard functions 3 (Read Multiple), 4 (Read One), 6 (Write One), and 16 (Write Multiple).
# For writing either function code=6 (single register) or 16 (multiple registers) can be used, but 16
# is better because it is "atomic" when writing values longer than 16-bits.

# drvModbusAsynConfigure("portName", "tcpPortName", slaveAddress, modbusFunction, modbusStartAddress, modbusLength, dataType, pollMsec, "plcType")

#                           portName             TCPName      Slave Function Address Length  Datatype    Poll   PLCType
# Read digital inputs as words (FIO, CIO, EIO, MIO)
drvModbusAsynConfigure("LJT4_$(N)_DIO_In",       "LJT4_$(N)",   1,     3,     2500,    4,        UINT16,  100, "LJT4_module")

# Set digital outputs one bit at a time (FIO, CIO, EIO, MIO)
# DIO8 - DIO15 (EIO bank) 
drvModbusAsynConfigure("LJT4_$(N)_DIO_Out",      "LJT4_$(N)",   0,     6,     2000,   23,        UINT16,    1, "LJT4_module")

# Read 14 analog inputs.
drvModbusAsynConfigure("LJT4_$(N)_AI_In",        "LJT4_$(N)",   1,     3,        0,   28,    FLOAT32_BE,  100, "LJT4_module")

# Write to 14 registers to configure analog resolution index.
drvModbusAsynConfigure("LJT4_$(N)_AIResolution", "LJT4_$(N)",   0,     6,    41500,   14,        UINT16,    1, "LJT4_module")

# Set 2 primary analog outputs.
drvModbusAsynConfigure("LJT4_$(N)_AO",           "LJT4_$(N)",   0,    16,     1000,    4,    FLOAT32_BE,    1, "LJT4_module")

# Additional analog outputs using LJTick-DAC occupying FIO, EIO, CIO
drvModbusAsynConfigure("LJT4_$(N)_AO_TICK",      "LJT4_$(N)",   0,    16,    30000,   40,     FLOAT32_BE,   0, "LJT4_module")

dbLoadTemplate("LabJack_T4_$(N).substitutions", "P=$(P)")
