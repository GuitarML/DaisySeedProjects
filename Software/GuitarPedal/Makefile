# Project Name
TARGET =  guitarpedal

APP_TYPE = BOOT_SRAM
#APP_TYPE = BOOT_NONE

# Sources
CPP_SOURCES = guitar_pedal.cpp guitar_pedal_storage.cpp $(wildcard */*.cpp) Effect-Modules/Chopper/chopper.cpp

OPT=-Os

# Library Locations
DAISYSP_DIR ?= DaisySP
LIBDAISY_DIR ?= libDaisy

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

C_INCLUDES += -I../../../Seed/RTNeural
CPPFLAGS += -DRTNEURAL_DEFAULT_ALIGNMENT=8 -DRTNEURAL_NO_DEBUG=1