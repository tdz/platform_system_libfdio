LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= fdstate.c \
                  loop.c \
                  task.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include
LOCAL_CFLAGS := -DANDROID_VERSION=$(PLATFORM_SDK_VERSION) -Wall -Werror
LOCAL_SHARED_LIBRARIES := libcutils liblog
LOCAL_MODULE:= libfdio
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)
