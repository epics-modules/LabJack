[![Labjack](https://github.com/epics-modules/LabJack/actions/workflows/ci-scripts-build.yml/badge.svg)](https://github.com/epics-modules/LabJack/actions/workflows/ci-scripts-build.yml)

This repository contains EPICS support for the LabJack T-series I/O modules.

It is based on asynPortDriver and the vendor LJM library.

It supports the following 4 LabJack modules.  All modules have both Ethernet and USB communications.

[LabJack T4](https://labjack.com/products/labjack-t4)
- 4 single-ended analog inputs
  - +- 10V range
  - 12-bit
  - Up to 50 kHz total streaming input rate, i.e. 1 channel at 50 kHz, 2 channels at 25 kHz, etc.
- Up to 8 additional analog inputs (0-2.5 V range, 12-bit).  These can also be configured as digital I/O bits.
- 2 analog outputs
  - 0-5V range
  - 10-bit
  - Up to 50 kHz streaming output rate
- 20 digital I/O bits
  - Rach configurable as input or output. 
  - 8 of these can configured as 0-2.5V analog inputs.
- $245

[LabJack T7](https://labjack.com/products/t7)
- 14 single-ended / 7 differential analog inputs
  - Programmable range (+-10V, +-1V, +-0.1V, +-0.01V)
  - 16-bit
  - Up to 100 kHz total streaming input rate, i.e. 1 channel at 100 kHz, 2 channels at 50 kHz, etc.
- 2 analog outputs
  - 0-5V range
  - 12-bit
  - Up to 100 kHz streaming output rate
- 23 digital I/O bits
  - Each configurable as input or output.
- $520

[LabJack T7-Pro](https://labjack.com/products/labjack-t7-pro)
- Same as the T7 except that each analog input can be software switched between 16-bit and 24-bit ADC, trading off resolution for speed.
- In 24-bit mode the ADCs support 9 types of thermocouple inputs.
- $750

[LabJack T8](https://labjack.com/products/t8)
- 8 differential analog inputs
  - 11 input ranges from +-11V to +-0.15 V
  - 24-bit sigma/delta ADC
  - Supports 9 types of thermocouple inputs
  - Up to 40 kHz streaming input per channel, i.e. up to 320 kHz total scanning rate.
- 2 analog outputs
  - 0-10V range
  - 16-bit
  - Up to 40 kHz streaming output rate
- 23 digital I/O bits, each configurable as input or output.
- $1,400

## Additional information:
* [Documentation](https://epics-modules.github.io/LabJack)
* [Release notes](RELEASE.md)
