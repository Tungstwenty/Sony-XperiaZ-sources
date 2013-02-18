LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_MODULE_TAGS := optional

datrie_include_export := $(LOCAL_PATH)

LOCAL_SRC_FILES:= \
        ./datrie/alpha-map.c \
        ./datrie/tail.c \
        ./datrie/fileutils.c \
        ./datrie/darray.c \
        ./datrie/trie.c \

LOCAL_C_INCLUDES += \
        $(LOCAL_PATH)/datrie

LOCAL_MODULE:= libdatrie

include $(BUILD_STATIC_LIBRARY)

