
LOCAL_PATH:= $(call my-dir)

common_SRC_FILES:=  \
	lzo/src/lzo_crc.c \
	lzo/src/lzo_init.c \
	lzo/src/lzo_ptr.c \
	lzo/src/lzo_str.c \
	lzo/src/lzo_util.c \
	lzo/src/lzo1.c \
	lzo/src/lzo1_99.c \
	lzo/src/lzo1a.c \
	lzo/src/lzo1a_99.c \
	lzo/src/lzo1b_1.c \
	lzo/src/lzo1b_2.c \
	lzo/src/lzo1b_3.c \
	lzo/src/lzo1b_4.c \
	lzo/src/lzo1b_5.c \
	lzo/src/lzo1b_6.c \
	lzo/src/lzo1b_7.c \
	lzo/src/lzo1b_8.c \
	lzo/src/lzo1b_9.c \
	lzo/src/lzo1b_99.c \
	lzo/src/lzo1b_9x.c \
	lzo/src/lzo1b_cc.c \
	lzo/src/lzo1b_d1.c \
	lzo/src/lzo1b_d2.c \
	lzo/src/lzo1b_rr.c \
	lzo/src/lzo1b_xx.c \
	lzo/src/lzo1c_1.c \
	lzo/src/lzo1c_2.c \
	lzo/src/lzo1c_3.c \
	lzo/src/lzo1c_4.c \
	lzo/src/lzo1c_5.c \
	lzo/src/lzo1c_6.c \
	lzo/src/lzo1c_7.c \
	lzo/src/lzo1c_8.c \
	lzo/src/lzo1c_9.c \
	lzo/src/lzo1c_99.c \
	lzo/src/lzo1c_9x.c \
	lzo/src/lzo1c_cc.c \
	lzo/src/lzo1c_d1.c \
	lzo/src/lzo1c_d2.c \
	lzo/src/lzo1c_rr.c \
	lzo/src/lzo1c_xx.c \
	lzo/src/lzo1f_1.c \
	lzo/src/lzo1f_9x.c \
	lzo/src/lzo1f_d1.c \
	lzo/src/lzo1f_d2.c \
	lzo/src/lzo1x_1.c \
	lzo/src/lzo1x_9x.c \
	lzo/src/lzo1x_d1.c \
	lzo/src/lzo1x_d2.c \
	lzo/src/lzo1x_d3.c \
	lzo/src/lzo1x_o.c \
	lzo/src/lzo1x_1k.c \
	lzo/src/lzo1x_1l.c \
	lzo/src/lzo1x_1o.c \
	lzo/src/lzo1y_1.c \
	lzo/src/lzo1y_9x.c \
	lzo/src/lzo1y_d1.c \
	lzo/src/lzo1y_d2.c \
	lzo/src/lzo1y_d3.c \
	lzo/src/lzo1y_o.c \
	lzo/src/lzo1z_9x.c \
	lzo/src/lzo1z_d1.c \
	lzo/src/lzo1z_d2.c \
	lzo/src/lzo1z_d3.c \
	lzo/src/lzo2a_9x.c \
	lzo/src/lzo2a_d1.c \
	lzo/src/lzo2a_d2.c


common_C_INCLUDES += $(LOCAL_PATH)/lzo/include

# static library
# =====================================================

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= $(common_SRC_FILES)
LOCAL_C_INCLUDES:= $(common_C_INCLUDES)
LOCAL_MODULE := liblzo-static
LOCAL_PRELINK_MODULE:= false
LOCAL_MODULE_TAGS := optional
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= $(common_SRC_FILES)
LOCAL_C_INCLUDES:= $(common_C_INCLUDES)
LOCAL_MODULE := liblzo_host
LOCAL_PRELINK_MODULE:= false
LOCAL_MODULE_TAGS := optional
LOCAL_EXPORT_C_INCLUDE_DIRS := $(common_C_INCLUDES)
include $(BUILD_HOST_STATIC_LIBRARY)

# dynamic library
# =====================================================

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= $(common_SRC_FILES)
LOCAL_C_INCLUDES:= $(common_C_INCLUDES)
LOCAL_MODULE := liblzo
LOCAL_PRELINK_MODULE:= false
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)
