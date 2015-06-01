# Copyright 2013 The Android Open Source Project
#
LOCAL_PATH := $(call my-dir)

xz_liblzma_la_SOURCES_TUKLIB := \
       xz/src/common/tuklib_physmem.c \
       xz/src/common/tuklib_cpucores.c

xz_liblzma_la_SOURCES_COMMON := \
       xz/src/liblzma/common/common.c \
       xz/src/liblzma/common/block_util.c \
       xz/src/liblzma/common/easy_preset.c \
       xz/src/liblzma/common/filter_common.c \
       xz/src/liblzma/common/hardware_physmem.c \
       xz/src/liblzma/common/index.c \
       xz/src/liblzma/common/stream_flags_common.c \
       xz/src/liblzma/common/vli_size.c

xz_liblzma_la_SOURCES_MAIN_ENCODER := \
       xz/src/liblzma/common/alone_encoder.c \
       xz/src/liblzma/common/block_buffer_encoder.c \
       xz/src/liblzma/common/block_encoder.c \
       xz/src/liblzma/common/block_header_encoder.c \
       xz/src/liblzma/common/easy_buffer_encoder.c \
       xz/src/liblzma/common/easy_encoder.c \
       xz/src/liblzma/common/easy_encoder_memusage.c \
       xz/src/liblzma/common/filter_buffer_encoder.c \
       xz/src/liblzma/common/filter_encoder.c \
       xz/src/liblzma/common/filter_flags_encoder.c \
       xz/src/liblzma/common/index_encoder.c \
       xz/src/liblzma/common/stream_buffer_encoder.c \
       xz/src/liblzma/common/stream_encoder.c \
       xz/src/liblzma/common/stream_flags_encoder.c \
       xz/src/liblzma/common/vli_encoder.c

xz_liblzma_la_SOURCES_THREAD := \
       xz/src/liblzma/common/hardware_cputhreads.c \
       xz/src/liblzma/common/outqueue.c \
       xz/src/liblzma/common/stream_encoder_mt.c

xz_liblzma_la_SOURCES_MAIN_DECODER := \
       xz/src/liblzma/common/alone_decoder.c \
       xz/src/liblzma/common/auto_decoder.c \
       xz/src/liblzma/common/block_buffer_decoder.c \
       xz/src/liblzma/common/block_decoder.c \
       xz/src/liblzma/common/block_header_decoder.c \
       xz/src/liblzma/common/easy_decoder_memusage.c \
       xz/src/liblzma/common/filter_buffer_decoder.c \
       xz/src/liblzma/common/filter_decoder.c \
       xz/src/liblzma/common/filter_flags_decoder.c \
       xz/src/liblzma/common/index_decoder.c \
       xz/src/liblzma/common/index_hash.c \
       xz/src/liblzma/common/stream_buffer_decoder.c \
       xz/src/liblzma/common/stream_decoder.c \
       xz/src/liblzma/common/stream_flags_decoder.c \
       xz/src/liblzma/common/vli_decoder.c

xz_liblzma_la_SOURCES_CHECK := \
       xz/src/liblzma/check/crc32_tablegen.c \
       xz/src/liblzma/check/crc64_tablegen.c \
       xz/src/liblzma/check/check.c \
       xz/src/liblzma/check/crc32_table.c \
       xz/src/liblzma/check/crc32_fast.c \
       xz/src/liblzma/check/crc64_table.c \
       xz/src/liblzma/check/crc64_fast.c \
       xz/src/liblzma/check/sha256.c

xz_liblzma_la_SOURCES_LZ := \
       xz/src/liblzma/lz/lz_encoder.c \
       xz/src/liblzma/lz/lz_encoder_mf.c \
       xz/src/liblzma/lz/lz_decoder.c

xz_liblzma_la_SOURCES_LZMA := \
       xz/src/liblzma/lzma/fastpos_tablegen.c \
       xz/src/liblzma/lzma/lzma_encoder.c \
       xz/src/liblzma/lzma/lzma_encoder_presets.c \
       xz/src/liblzma/lzma/lzma_encoder_optimum_fast.c \
       xz/src/liblzma/lzma/lzma_encoder_optimum_normal.c \
       xz/src/liblzma/lzma/fastpos_table.c \
       xz/src/liblzma/lzma/lzma_decoder.c \
       xz/src/liblzma/lzma/lzma2_encoder.c \
       xz/src/liblzma/lzma/lzma2_decoder.c

xz_liblzma_la_SOURCES_RANGECODE := \
       xz/src/liblzma/rangecoder/price_tablegen.c \
       xz/src/liblzma/rangecoder/price_table.c

xz_liblzma_la_SOURCES_DELTA := \
       xz/src/liblzma/delta/delta_common.c \
       xz/src/liblzma/delta/delta_encoder.c \
       xz/src/liblzma/delta/delta_decoder.c

xz_liblzma_la_SOURCES_SIMPLE := \
       xz/src/liblzma/simple/simple_coder.c \
       xz/src/liblzma/simple/simple_encoder.c \
       xz/src/liblzma/simple/simple_decoder.c \
       xz/src/liblzma/simple/x86.c \
       xz/src/liblzma/simple/powerpc.c \
       xz/src/liblzma/simple/ia64.c \
       xz/src/liblzma/simple/armthumb.c \
       xz/src/liblzma/simple/sparc.c \
       xz/src/liblzma/simple/arm.c

common_SRC_FILES :=  \
       xz_liblzma_la_SOURCES_TUKLIB \
       xz_liblzma_la_SOURCES_COMMON \
       xz_liblzma_la_SOURCES_MAIN_ENCODER \
       xz_liblzma_la_SOURCES_THREAD \
       xz_liblzma_la_SOURCES_MAIN_DECODER \
       xz_liblzma_la_SOURCES_CHECK \
       xz_liblzma_la_SOURCES_LZ \
       xz_liblzma_la_SOURCES_LZMA \
       xz_liblzma_la_SOURCES_RANGECODE \
       xz_liblzma_la_SOURCES_DELTA \
       xz_liblzma_la_SOURCES_SIMPLE
 
common_xz_export_INCLUDES := $(LOCAL_PATH)/xz/src/liblzma/api \
       $(LOCAL_PATH)/xz/src/liblzma/api/lzma

common_C_INCLUDES := $(LOCAL_PATH) \
       $(LOCAL_PATH)/xz/src/common \
       $(LOCAL_PATH)/xz/src/liblzma/api \
       $(LOCAL_PATH)/xz/src/liblzma/common \
       $(LOCAL_PATH)/xz/src/liblzma/check \
       $(LOCAL_PATH)/xz/src/liblzma/lz \
       $(LOCAL_PATH)/xz/src/liblzma/rangecoder \
       $(LOCAL_PATH)/xz/src/liblzma/lzma \
       $(LOCAL_PATH)/xz/src/liblzma/delta \
       $(LOCAL_PATH)/xz/src/liblzma/simple
	
include $(CLEAR_VARS)
LOCAL_MODULE := libxzsemc_host
LOCAL_CFLAGS = -O3 -std=c99 -Wall -D_GNU_SOURCE -DHAVE_CONFIG_H
#LOCAL_CFLAGS = -O3 -Wall -DHAVE_CONFIG_H
LOCAL_CFLAGS+=   -Wshadow -Wpointer-arith -Wwrite-strings -Wstrict-prototypes -Wmissing-declarations
LOCAL_CFLAGS+=   -Wmissing-prototypes -Wredundant-decls -Wnested-externs -Winline
LOCAL_C_INCLUDES+= $(common_C_INCLUDES)
LOCAL_SRC_FILES := $(xz_liblzma_la_SOURCES_TUKLIB)
LOCAL_SRC_FILES += $(xz_liblzma_la_SOURCES_COMMON)
LOCAL_SRC_FILES += $(xz_liblzma_la_SOURCES_MAIN_ENCODER)
LOCAL_SRC_FILES += $(xz_liblzma_la_SOURCES_THREAD)
LOCAL_SRC_FILES += $(xz_liblzma_la_SOURCES_MAIN_DECODER)
LOCAL_SRC_FILES += $(xz_liblzma_la_SOURCES_CHECK)
LOCAL_SRC_FILES += $(xz_liblzma_la_SOURCES_LZ)
LOCAL_SRC_FILES += $(xz_liblzma_la_SOURCES_LZMA)
LOCAL_SRC_FILES += $(xz_liblzma_la_SOURCES_RANGECODE)
LOCAL_SRC_FILES += $(xz_liblzma_la_SOURCES_DELTA)
LOCAL_SRC_FILES += $(xz_liblzma_la_SOURCES_SIMPLE)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(common_xz_export_INCLUDES)
LOCAL_LDLIBS += -lpthread -lm
include $(BUILD_HOST_STATIC_LIBRARY)
