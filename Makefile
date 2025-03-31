
PROJECT_NAME := demo_rd04
PROJECT_PATH := $(abspath .)
PROJECT_BOARD := evb
export PROJECT_PATH PROJECT_BOARD
#CONFIG_TOOLPREFIX :=

-include ./proj_config.mk

ifeq ($(origin BL60X_SDK_PATH), undefined)
BL60X_SDK_PATH ?= Ai-Thinker-WB2
$(info ****** Please SET BL60X_SDK_PATH ******)
$(info ****** Trying SDK PATH [$(BL60X_SDK_PATH)])
endif

COMPONENTS_BLSYS   := bltime blfdt blmtd bloop loopset looprt loopadc
COMPONENTS_VFS     := romfs
COMPONENTS_NETWORK := wifi wifi_manager wpa_supplicant bl_os_adapter wifi_hosal lwip lwip_dhcpd httpc netutils blcrypto_suite dns_server

INCLUDE_COMPONENTS += freertos_riscv_ram
INCLUDE_COMPONENTS += bl602 bl602_std
INCLUDE_COMPONENTS += hosal mbedtls_lts cli vfs yloop utils blog blog_testc newlibc coredump easyflash4
INCLUDE_COMPONENTS += rfparam_adapter_tmp
INCLUDE_COMPONENTS += $(COMPONENTS_NETWORK)
INCLUDE_COMPONENTS += $(COMPONENTS_BLSYS)
INCLUDE_COMPONENTS += $(COMPONENTS_VFS)
INCLUDE_COMPONENTS += $(PROJECT_NAME)

include $(BL60X_SDK_PATH)/make_scripts_riscv/project.mk
