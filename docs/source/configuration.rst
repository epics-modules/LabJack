Configuration
-------------

The following lines are needed in the EPICS startup script for the LabJack driver.

::

   ## Configure port driver
   # LabJackConfig(portName,        # The name to give to this asyn port driver
   #               uniqueID,        # The IP name, IP address, or serial number of the LabJack module.
   #               maxInputPoints,  # Maximum number of input points for waveform digitizer
   #               maxOutputPoints) # Maximum number of output points for waveform generator
   LabJackConfig("LJT7_1", "gse-labjack1.cars.aps.anl.gov, 2048, 2048)

The uniqueID is a string that identifies the device to be controlled.  It can be any of the following:

- A fully qualified domain name with periods, e.g. gse-labjack1.cars.aps.anl.gov.
  The periods are needed to distinguish an IP name from a serial number.
- An IP address, e.g. 10.54.160.72.
- A module serial number, e.g. 470029169.

The LabJack module comes with example iocBoot/ directories that contain
example startup scripts and example substitutions files for each model.

