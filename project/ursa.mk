# top level project rules for the Amazon ursa project
#
LOCAL_DIR := $(GET_LOCAL_DIR)

TARGET := ursa

MODULES += app/aboot

DEBUG := 1

#DEFINES += WITH_DEBUG_DCC=1
DEFINES += WITH_DEBUG_UART=1
#DEFINES += WITH_DEBUG_FBCON=1
DEFINES += DEVICE_TREE=1
#DEFINES += MMC_BOOT_BAM=1
