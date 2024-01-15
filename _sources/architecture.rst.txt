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

The analog inputs can be set to either periodic or I/O Intr scan.
They use the asyn aiFloat64Average device support.
In periodic scanning mode this averages all of the samples read by the polling thread 
each time the record processes.  For example, if the poll cycle time is 10 ms and the SCAN
time is 0.5 seconds (500 ms) then 50 samples are averaged for each reading.
In I/O Intr scan mode the SVAL record field controls the number of readings to average
before the record is processed.  If SVAL is 0 (default) then the record processes
on each polling cycle.  This allows more frequent updates at the expense of CPU load
and Channel Access traffic.

When reading analog inputs in thermocouple mode the inputs are actually read in volts,
and the conversion to temperature is done in software.  This uses the cold junction
temperature read from the device, and the temperature conversion function in the LJM library.
This allows temperature inputs to be scanned with the waveform digitizer function, which is
not possible if the temperature conversions are performed on the device itself.
