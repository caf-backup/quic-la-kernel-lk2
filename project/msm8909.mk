# top level project rules for the msm8909 project
#
LOCAL_DIR := $(GET_LOCAL_DIR)

TARGET := msm8909

MODULES += app/aboot

ifeq ($(TARGET_BUILD_VARIANT),user)
DEBUG := 0
else
DEBUG := 1
endif

EMMC_BOOT := 1

#Enable OTA Support
ENABLE_VBOOT_MOTA_SUPPORT := 1

ifeq ($(VERIFIED_BOOT),1)
ENABLE_MDTP_SUPPORT := 1
ifeq ($(ENABLE_VBOOT_MOTA_SUPPORT),1)
DEFINES += VBOOT_MOTA=1
else
ENABLE_SECAPP_LOADER := 1
ENABLE_RPMB_SUPPORT := 1

#enable fbcon display menu
ENABLE_FBCON_DISPLAY_MSG := 1
endif
endif

ENABLE_SMD_SUPPORT := 1
ENABLE_PWM_SUPPORT := true
#ENABLE_BOOT_CONFIG_SUPPORT := 1

#DEFINES += WITH_DEBUG_DCC=1
DEFINES += WITH_DEBUG_LOG_BUF=1
DEFINES += WITH_DEBUG_UART=1
#DEFINES += WITH_DEBUG_FBCON=1
DEFINES += DEVICE_TREE=1
#DEFINES += MMC_BOOT_BAM=1
#DEFINES += CRYPTO_BAM=1
DEFINES += SPMI_CORE_V2=1
DEFINES += ABOOT_IGNORE_BOOT_HEADER_ADDRS=1

DEFINES += ABOOT_FORCE_KERNEL_ADDR=0x80008000
DEFINES += ABOOT_FORCE_RAMDISK_ADDR=0x82000000
DEFINES += ABOOT_FORCE_TAGS_ADDR=0x81E00000
DEFINES += ABOOT_FORCE_KERNEL64_ADDR=0x00080000

DEFINES += BAM_V170=1
DEFINES += ENABLE_FBCON_LOGGING=1

#Enable the feature of long press power on
DEFINES += LONG_PRESS_POWER_ON=1

ifeq ($(ENABLE_RPMB_SUPPORT),1)
DEFINES += USE_RPMB_FOR_DEVINFO=1
endif

DEFINES += FORCE_MIN_RPMB_CNT=1

#Disable thumb mode
ENABLE_THUMB := false

ENABLE_SDHCI_SUPPORT := 1

ifeq ($(ENABLE_SDHCI_SUPPORT),1)
DEFINES += MMC_SDHCI_SUPPORT=1
endif

#enable power on vibrator feature
ENABLE_PON_VIB_SUPPORT := true

ifeq ($(EMMC_BOOT),1)
DEFINES += _EMMC_BOOT=1
endif

ifeq ($(ENABLE_PON_VIB_SUPPORT),true)
DEFINES += PON_VIB_SUPPORT=1
endif

ifeq ($(ENABLE_SMD_SUPPORT),1)
DEFINES += SMD_SUPPORT=1
endif

ifeq ($(ENABLE_BOOT_CONFIG_SUPPORT),1)
DEFINES += BOOT_CONFIG_SUPPORT=1
endif

#Enable the external reboot functions
ENABLE_REBOOT_MODULE := 1
#Use PON register for reboot reason
DEFINES += USE_PON_REBOOT_REG=1

#Enable fbcon display for verified boot.
ifeq ($(ENABLE_FBCON_DISPLAY_MSG),1)
DEFINES += FBCON_DISPLAY_MSG=1
endif

#enable battery voltage check
DEFINES += CHECK_BAT_VOLTAGE=1
