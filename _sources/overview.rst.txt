.. _LabJackCorp:           https://www.labjack.com
.. _EPICS:                 https://epics-controls.org/
.. _asyn:                  https://github.com/epics-modules/asyn
.. _asynPortDriver:        https://epics-modules.github.io/master/asyn/R4-42/asynPortDriver.html

Overview
--------

This is an EPICS_ driver for the
T-series devices from LabJackCorp_.
These multi-function devices support support analog input, 
thermocouple input, analog output, and binary I/O.

The driver is written in C++, and consists of a class that inherits from
asynPortDriver_, which is part of the EPICS asyn_ module.

The driver is written to be general, so that it can be used with any
LabJack T-series module. The T-series devices all use Modbus for the low-level
communications, and use a consistent set of Modbus register addresses, so the
code is largely model-independent.  The driver does require small modifications
to be be used with a new model.
