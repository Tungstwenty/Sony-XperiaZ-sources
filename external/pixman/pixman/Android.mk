LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SDK_VERSION := 14

LOCAL_MODULE := libpixman

LOCAL_CFLAGS := -DHAVE_CONFIG_H

# Core files
LOCAL_SRC_FILES := \
	pixman.c \
	pixman-access.c \
	pixman-access-accessors.c \
	pixman-bits-image.c \
	pixman-combine32.c \
	pixman-combine-float.c \
	pixman-conical-gradient.c \
	pixman-filter.c \
	pixman-x86.c \
	pixman-mips.c \
	pixman-arm.c \
	pixman-ppc.c \
	pixman-edge.c \
	pixman-edge-accessors.c \
	pixman-fast-path.c \
	pixman-glyph.c \
	pixman-general.c \
	pixman-gradient-walker.c \
	pixman-image.c \
	pixman-implementation.c \
	pixman-linear-gradient.c \
	pixman-matrix.c \
	pixman-noop.c \
	pixman-radial-gradient.c \
	pixman-region16.c \
	pixman-region32.c \
	pixman-solid-fill.c \
	pixman-timer.c \
	pixman-trap.c \
	pixman-utils.c

ifeq ($(strip $(TARGET_ARCH)),arm)
	# Will only be used if runtime detection reports NEON capabilities
	LOCAL_CFLAGS += -DUSE_ARM_NEON
	LOCAL_SRC_FILES += \
		pixman-arm-neon.c \
		pixman-arm-neon-asm.S \
		pixman-arm-neon-asm-bilinear.S
endif

LOCAL_STATIC_LIBRARIES := cpufeatures

include $(BUILD_STATIC_LIBRARY)

$(call import-module,android/cpufeatures)
