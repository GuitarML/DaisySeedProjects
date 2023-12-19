# Project Name
TARGET =  guitarpedal

# Sources
CPP_SOURCES = guitar_pedal.cpp guitar_pedal_storage.cpp $(wildcard */*.cpp) #Effect-Modules/Chopper/chopper.cpp

APP_TYPE = BOOT_SRAM

# Library Locations
DAISYSP_DIR ?= DaisySP
LIBDAISY_DIR ?= libDaisy
CLOUDSEED_DIR = Effect-Modules/CloudSeed

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

C_INCLUDES += \
-IEffect-Modules/CloudSeed \

LIBS += -lcloudseed
LIBDIR += -LEffect-Modules/CloudSeed/build