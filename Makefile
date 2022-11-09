#Makefile at top of application tree
TOP = .
include $(TOP)/configure/CONFIG
DIRS := $(DIRS) $(filter-out $(DIRS), configure)
DIRS += LabJackSupport
DIRS += LabJackApp 
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard iocBoot))

LabJackApp_DEPEND_DIRS += LabJackSupport

define DIR_template
 $(1)_DEPEND_DIRS = configure
endef
$(foreach dir, $(filter-out configure,$(DIRS)),$(eval $(call DIR_template,$(dir))))

iocBoot_DEPEND_DIRS += $(filter %App,$(DIRS))

include $(TOP)/configure/RULES_TOP
