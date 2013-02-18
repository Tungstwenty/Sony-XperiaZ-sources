LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= \
        src/libthai.c \
        src/thrend/thrend.c \
        src/thwbrk/thwbrk.c \
        src/thcell/thcell.c \
        src/thstr/thstr.c \
        src/thctype/wtt.c \
        src/thbrk/thbrk.c \
        src/thbrk/brk-ctype.c \
        src/thctype/thctype.c \
        src/thwstr/thwstr.c \
        src/thcoll/cweight.c \
        src/thcoll/thcoll.c \
        src/thbrk/brk-maximal.c \
        src/thwctype/thwctype.c \
        src/thinp/thinp.c \
        src/thwchar/thwchar.c

LOCAL_STATIC_LIBRARIES := \
        libdatrie

LOCAL_C_INCLUDES += \
        $(LOCAL_PATH)/include \
        $(LOCAL_PATH)/libdatrie

LOCAL_MODULE:= libthai

include $(BUILD_SHARED_LIBRARY)
include $(call all-makefiles-under, $(LOCAL_PATH))