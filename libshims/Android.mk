LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libpowermanager.vendor
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PREBUILT_MODULE_FILE := $(call intermediates-dir-for,SHARED_LIBRARIES,libpowermanager)/libpowermanager.so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_TAGS := optional
LOCAL_INSTALLED_MODULE_STEM := libpowermanager.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_VENDOR_MODULE := true
LOCAL_CHECK_ELF_FILES := false
include $(BUILD_PREBUILT)
