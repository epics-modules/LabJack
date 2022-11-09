# Debug-output level
save_restoreSet_Debug(0)

# Set PV prefix
save_restoreSet_status_prefix("$(PREFIX)")

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


set_pass0_restoreFile("auto_settings.sav")
set_pass1_restoreFile("auto_settings.sav")

# Specify directories to search for request files
set_requestfile_path(".",  "")
set_requestfile_path(".",  "autosave")
set_requestfile_path($(AUTOSAVE),"db")
set_requestfile_path($(LABJACK), "db")

# Load database
dbLoadRecords("$(AUTOSAVE)/db/save_restoreStatus.db", "P=$(PREFIX)")
