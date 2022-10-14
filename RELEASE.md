# labjack Release Notes

## R2-0 (October XXX, 2022)
Many changes to simplify and add new features.
- Added support for the analog input Range feature to select the voltage range.
- Added support for the analog input Resolution feature to control resolution, allowing decreased noise at the expense of speed.
- Added control for Range and Resolution on LabJack_T7_AiSetup.adl.
- Added LabJack_T4_AiSetup.adl to control Resolution.
- Decreased the number drvModbusAsyn port drivers created for the T7 from 25 to 8.
- Replaced multiple startup scripts (LabJack_T7_1.cmd, LabJack_T7_2.cmd, etc.) with a single LabJack_T7.cmd using environment variables.
- Added generate_T7_2_and_3.subsititions.sh script that generates the subsititions files for modules 2 and 3 from LabJack_T7_1.substitutions.
- Added longin records for the FIO, CIO, EIO, and MIO binary inputs to the main T7 and T4 screens. 

## R1-0 (October 14, 2022)
- Initial release of labjack module.
