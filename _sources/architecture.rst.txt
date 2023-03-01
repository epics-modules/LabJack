Driver architecture
-------------------
The driver has a polling thread that periodically reads the state of the
digital I/O bits and the values of the analog inputs.  If the waveform
digitizer or waveform generator functions are active it polls the status
of those as well.  
The delay time at the end of the polling cycle can be controlled via an EPICS PV.
The actual poll cycle time, including the delay, is reported in an EPICS PV.

The digital I/O are normally set to SCAN=I/O Intr so that they change state quickly
when the poller reads them.

The analog inputs can be set to either periodic or I/O Intr scan.  I/O Intr scan
allows more frequent updates at the expense of CPU load and Channel Access traffic.

When reading analog inputs in thermocouple mode the inputs are actually read in volts,
and the conversion to temperature is done in software.  This uses the cold junction
temperature read from the device, and the temperature conversion function in the LJM library.
This allows temperature inputs to be scanned with the waveform digitizer function, which is
not possible if the temperature conversions are performed on the device itself.
