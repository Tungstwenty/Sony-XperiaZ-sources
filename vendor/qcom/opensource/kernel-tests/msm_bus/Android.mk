ifeq ($(TARGET_BOARD_PLATFORM),msm8660)

DLKM_DIR   := device/qcom/common/dlkm
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO   := kernel-tests/msm_bus
LOCAL_COPY_HEADERS      := msm_bus_test.h
include $(BUILD_COPY_HEADERS)

LOCAL_MODULE      := msm_bus_ioctl.ko
LOCAL_MODULE_TAGS := eng
include $(DLKM_DIR)/AndroidKernelModule.mk

endif
