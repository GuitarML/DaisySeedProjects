# Project Name
TARGET =  guitarpedal

#APP_TYPE = BOOT_SRAM

# Sources
CPP_SOURCES = guitar_pedal.cpp guitar_pedal_storage.cpp $(wildcard */*.cpp) Effect-Modules/Chopper/chopper.cpp Effect-Modules/ImpulseResponse/ImpulseResponse.cpp Effect-Modules/ImpulseResponse/dsp.cpp

# Compiler options
#OPT = -O1
OPT=-Os

# Boot options
APP_TYPE = BOOT_SRAM

OPT=-Os

# Library Locations
DAISYSP_DIR ?= DaisySP
LIBDAISY_DIR ?= libDaisy
CLOUDSEED_DIR = Effect-Modules/CloudSeed

C_INCLUDES += \
-Ieurorack \

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

C_INCLUDES += -Ieigen
C_INCLUDES += -IRTNeural
CPPFLAGS += -DRTNEURAL_DEFAULT_ALIGNMENT=8 -DRTNEURAL_NO_DEBUG=1

C_INCLUDES += \
-IEffect-Modules/CloudSeed \

LIBS += -lcloudseed
LIBDIR += -LEffect-Modules/CloudSeed/build