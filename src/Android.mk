LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= loop.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include
LOCAL_SHARED_LIBRARIES := libcutils liblog
LOCAL_MODULE:= libfdio
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

