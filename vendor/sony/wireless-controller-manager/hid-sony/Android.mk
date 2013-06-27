LOCAL_PATH:= $(call my-dir)

KBUILD_OPTIONS := MODNAME=hid-sony
KBUILD_OPTIONS := BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)

include $(CLEAR_VARS)
LOCAL_MODULE := hid-sony.ko
LOCAL_MODLUE_KBUILD_NAME := hid-sony.ko
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/modules/ps3ctrl
include $(TOP)/device/qcom/common/dlkm/AndroidKernelModule.mk
