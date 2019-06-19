LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE           := readmac
LOCAL_MODULE_TAGS      := optional
LOCAL_VENDOR_MODULE    := true
LOCAL_SRC_FILES        := xiaomi_readmac.cpp
LOCAL_HEADER_LIBRARIES := libcutils_headers
LOCAL_SHARED_LIBRARIES := libbase
include $(BUILD_EXECUTABLE)
