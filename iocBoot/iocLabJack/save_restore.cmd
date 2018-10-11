# Debug-output level
save_restoreSet_Debug(0)

# Set PV prefix
save_restoreSet_status_prefix("xyz:")

# Ok to save/restore save sets with missing values (no CA connection to PV)?
save_restoreSet_IncompleteSetsOk(1)

# Save dated backup files?
save_restoreSet_DatedBackupFiles(1)

# Number of sequenced backup files to write
save_restoreSet_NumSeqFiles(3)

# Time interval between sequenced backups
save_restoreSet_SeqPeriodInSeconds(300)

# Specify directories to search for save files
set_savefile_path(".","autosave")

###
# specify what save files should be restored.  Note these files must be
# in the directory specified in set_savefile_path(), or, if that function
# has not been called, from the directory current when iocInit is invoked
#set_pass0_restoreFile("auto_positions.sav")
# Note doAfterIocInit() supplied by std module.
#doAfterIocInit("create_monitor_set('auto_positions.req',5,'P=kag:')")

set_pass0_restoreFile("auto_settings.sav")
set_pass1_restoreFile("auto_settings.sav")

# Specify directories to search for request files
set_requestfile_path(".",  "")
set_requestfile_path(".",  "autosave")
set_requestfile_path($(AUTOSAVE),"asApp/Db")
set_requestfile_path($(TOP), "LabJackApp/Db")

# Load database
dbLoadRecords("$(AUTOSAVE)/asApp/Db/save_restoreStatus.db", "P=xyz:")
