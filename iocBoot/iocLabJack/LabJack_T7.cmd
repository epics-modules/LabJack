# LabJack_T7.cmd

# The following environment variables must be set before loading this file

#          P : The PV prefix for this IOC
# IP_ADDRESS : The IP address or name of this Labjack
#          N : The unit number (1 for first, 2 for second, etc.)

# Use the following commands for TCP/IP
#drvAsynIPPortConfigure(const char *portName,
#                       const char *hostInfo,
#                       unsigned int priority,
#                       int noAutoConnect,
#                       int noProcessEos);
#
drvAsynIPPortConfigure("LJT7_$(N)","$(IP_ADDRESS):502",0,0,0)

#modbusInterposeConfig(const char *portName,
#                      modbusLinkType linkType,
#                      int timeoutMsec,
#                      int writeDelayMsec)
# One per controller.  One controller can support up to six drivers.
modbusInterposeConfig("LJT7_$(N)",0,2000,0)

# Modbus standard functions 3 (Read Multiple), 4 (Read One), 6 (Write One), and 16 (Write Multiple).
# For writing either function code=6 (single register) or 16 (multiple registers) can be used, but 16
# is better because it is "atomic" when writing values longer than 16-bits.

# drvModbusAsynConfigure("portName", "tcpPortName", slaveAddress, modbusFunction, modbusStartAddress, modbusLength, dataType, pollMsec, "plcType")

#                           portName             TCPName      Slave Function Address Length  Datatype    Poll   PLCType
# Read digital inputs as words (FIO, CIO, EIO, MIO)
drvModbusAsynConfigure("LJT7_$(N)_DIO_In",       "LJT7_$(N)",   1,     3,     2500,    4,        UINT16,  100, "LJT7_module")

# Set digital outputs one bit at a time (FIO, CIO, EIO, MIO)
# DIO8 - DIO15 (EIO bank) 
drvModbusAsynConfigure("LJT7_$(N)_DIO_Out",      "LJT7_$(N)",   0,     6,     2000,   23,        UINT16,    1, "LJT7_module")

# Read 14 analog inputs.
drvModbusAsynConfigure("LJT7_$(N)_AI_In",        "LJT7_$(N)",   1,     3,        0,   28,    FLOAT32_BE,  100, "LJT7_module")

# Write to 14 registers to configure analog range.
drvModbusAsynConfigure("LJT7_$(N)_AIRange",      "LJT7_$(N)",   0,    16,    40000,   28,    FLOAT32_BE,    1, "LJT7_module")

# Write to register to set each even no. AI channel Single-Ended or Differential
drvModbusAsynConfigure("LJT7_$(N)_AIDiff",       "LJT7_$(N)",   0,     6,    41000,   13,        UINT16,    1, "LJT7_module")

# Write to 14 registers to configure analog resolution index.
drvModbusAsynConfigure("LJT7_$(N)_AIResolution", "LJT7_$(N)",   0,     6,    41500,   14,        UINT16,    1, "LJT7_module")

# Set 2 primary analog outputs.
drvModbusAsynConfigure("LJT7_$(N)_AO",           "LJT7_$(N)",   0,    16,     1000,    4,    FLOAT32_BE,    1, "LJT7_module")

# Addition analog outputs using LJTick-DAC occupying FIO, EIO, CIO
drvModbusAsynConfigure("LJT7_$(N)_AO_TICK",      "LJT7_$(N)",   0,    16,    30000,   40,     FLOAT32_BE,   0, "LJT7_module")

dbLoadTemplate("LabJack_T7_$(N).substitutions", "P=$(P)")
