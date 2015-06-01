include $(call all-subdir-makefiles)

LOCAL_PATH := external/e2fsprogs

e2fstool_src_files := $(addprefix e2fsck/,$(e2fsck_src_files))
e2fstool_src_files += $(addprefix misc/,$(mke2fs_src_files))
e2fstool_src_files += misc/tune2fs.c e2fstool.c

e2fstool_c_includes := \
	external/e2fsprogs/lib \
	external/e2fsprogs/e2fsck

e2fstool_cflags := -O2 -g -W -Wall -DMULTICALL\
	-DHAVE_STRCASECMP \
	-DHAVE_GETOPT_H \
	-DHAVE_DIRENT_H \
	-DHAVE_ERRNO_H \
	-DHAVE_INTTYPES_H \
	-DHAVE_LINUX_FD_H \
	-DHAVE_NETINET_IN_H \
	-DHAVE_SETJMP_H \
	-DHAVE_SYS_IOCTL_H \
	-DHAVE_SYS_MMAN_H \
	-DHAVE_SYS_MOUNT_H \
	-DHAVE_SYS_PRCTL_H \
	-DHAVE_SYS_RESOURCE_H \
	-DHAVE_SYS_SELECT_H \
	-DHAVE_SYS_STAT_H \
	-DHAVE_SYS_TYPES_H \
	-DHAVE_STDLIB_H \
	-DHAVE_UNISTD_H \
	-DHAVE_UTIME_H \
	-DHAVE_STRDUP \
	-DHAVE_MMAP \
	-DHAVE_GETPAGESIZE \
	-DHAVE_LSEEK64 \
	-DHAVE_LSEEK64_PROTOTYPE \
	-DHAVE_EXT2_IOCTLS \
	-DHAVE_TYPE_SSIZE_T \
	-DHAVE_INTPTR_T \
	-DENABLE_HTREE=1 \
	-DHAVE_SYS_TIME_H \
	-DHAVE_SYS_PARAM_H \
	-DHAVE_SYSCONF \
	-DNO_CHECK_BB

e2fstool_static_libraries := \
	libext2fs_static \
	libext2_blkid_static \
	libext2_uuid_static \
	libext2_profile_static \
	libext2_com_err_static \
	libext2_e2p_static \
	libext2_quota_static \
	libext2fs_static

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(e2fstool_src_files)
LOCAL_C_INCLUDES := $(e2fstool_c_includes)
LOCAL_CFLAGS := $(e2fstool_cflags)
LOCAL_STATIC_LIBRARIES := $(e2fstool_static_libraries) libc
LOCAL_MODULE := e2fstool
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT_SBIN)
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_UNSTRIPPED)
LOCAL_FORCE_STATIC_EXECUTABLE := true

include $(BUILD_EXECUTABLE)

LINKS := $(addprefix $(TARGET_ROOT_OUT_SBIN)/, e2fsck_static tune2fs_static fota-mke2fs)

e2fstool_links: e2fstool_bin := $(LOCAL_MODULE)
e2fstool_links:
	@mkdir -p $(TARGET_ROOT_OUT_SBIN)
	@$(foreach var,$(LINKS), \
      echo "Symlink: $(e2fstool_bin) -> $(var)"; \
      ln -sf $(e2fstool_bin) $(var); \
    )

ALL_MODULES.$(LOCAL_MODULE).INSTALLED := \
    $(ALL_MODULES.$(LOCAL_MODULE).INSTALLED) e2fstool_links

