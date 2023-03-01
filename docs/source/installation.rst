Installation
------------
The EPICS LabJack module uses the 
`LJM library <https://labjack.com/pages/support?doc=/software-driver/ljm-users-guide/>`__ 
from LabJack.  It runs on Linux and Windows.  

Most Linux versions should be supported.  It has been tested on Centos 7.  
The EPICS module includes the LJM header and library files, so ideally
LJM would not need to be installed locally on Linux.  
However, LJM uses configuration files which it installs in /usr/local/share/LabJack/LJM,
so the LJM package does need to be installed, and this requires root privilege.
I have asked LabJack support if it is possible for those files to be located 
in a directory that does not require root permission to write to.

On Windows the LJM library package needs to be installed to run the IOC.

