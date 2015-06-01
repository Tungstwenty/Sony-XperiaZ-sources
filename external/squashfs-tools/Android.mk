# Copyright 2005 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)

Common_Include_PATH := $(LOCAL_PATH)/squashfs/squashfs-tools

MKSQUASHFS_SRC_FILES := \
	squashfs/squashfs-tools/mksquashfs.c \
	squashfs/squashfs-tools/read_fs.c \
	squashfs/squashfs-tools/action.c \
	squashfs/squashfs-tools/swap.c \
	squashfs/squashfs-tools/pseudo.c \
	squashfs/squashfs-tools/compressor.c \
	squashfs/squashfs-tools/sort.c \
	squashfs/squashfs-tools/progressbar.c \
	squashfs/squashfs-tools/read_file.c \
	squashfs/squashfs-tools/info.c \
	squashfs/squashfs-tools/restore.c \
	squashfs/squashfs-tools/process_fragments.c \
	squashfs/squashfs-tools/caches-queues-lists.c \
	squashfs/squashfs-tools/gzip_wrapper.c \
	squashfs/squashfs-tools/lzo_wrapper.c \
	squashfs/squashfs-tools/lz4_wrapper.c \
	squashfs/squashfs-tools/xattr.c \
	squashfs/squashfs-tools/read_xattrs.c


XZ_SRC_FILES := squashfs/squashfs-tools/xz_wrapper.c

UNSQUASHFS_SRC_FILES := \
	squashfs/squashfs-tools/unsquashfs.c \
	squashfs/squashfs-tools/unsquash-1.c \
	squashfs/squashfs-tools/unsquash-2.c \
	squashfs/squashfs-tools/unsquash-3.c \
	squashfs/squashfs-tools/unsquash-4.c \
	squashfs/squashfs-tools/swap.c \
	squashfs/squashfs-tools/compressor.c \
	squashfs/squashfs-tools/unsquashfs_info.c \
	squashfs/squashfs-tools/gzip_wrapper.c \
	squashfs/squashfs-tools/xz_wrapper.c \
	squashfs/squashfs-tools/lzo_wrapper.c \
	squashfs/squashfs-tools/lz4_wrapper.c \
	squashfs/squashfs-tools/read_xattrs.c \
	squashfs/squashfs-tools/unsquashfs_xattr.c



include $(CLEAR_VARS)
LOCAL_MODULE := mksquashfs_semc
LOCAL_C_INCLUDES += $(Common_Include_PATH) 
LOCAL_SRC_FILES := $(MKSQUASHFS_SRC_FILES) 
LOCAL_SRC_FILES += $(XZ_SRC_FILES) 

LOCAL_CFLAGS =   -O2 -Wall -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE -DCOMP_DEFAULT=\"gzip\"
LOCAL_CFLAGS+=   -DGZIP_SUPPORT -DLZO_SUPPORT -DLZ4_SUPPORT -DXATTR_SUPPORT -DXATTR_DEFAULT
LOCAL_CFLAGS+=   -DXZ_SUPPORT
#LOCAL_CFLAGS+=   -Wshadow -Wpointer-arith -Wwrite-strings -Wstrict-prototypes -Wmissing-declarations
#LOCAL_CFLAGS+=   -Wmissing-prototypes -Wredundant-decls -Wnested-externs -Winline

COMPRESSORS = gzip xz lzo lz4

LOCAL_LDLIBS += -lpthread -lm 
LOCAL_STATIC_LIBRARIES := libz liblzo_host liblzo_host liblz4_host
LOCAL_STATIC_LIBRARIES += libxzsemc_host


include $(BUILD_HOST_EXECUTABLE)

$(call dist-for-goals, dist_files, $(LOCAL_BUILT_MODULE))
