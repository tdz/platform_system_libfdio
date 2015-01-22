LOCAL_PATH:= $(call my-dir)

local_source_files_14 := ports/timerfd.c
local_source_files_15 := $(local_source_files_14)
local_source_files_16 := $(local_source_files_15)
local_source_files_17 := $(local_source_files_16)
local_source_files_18 := $(local_source_files_17)
local_source_files    := $(local_source_files_$(PLATFORM_SDK_VERSION))

include $(CLEAR_VARS)
LOCAL_SRC_FILES := fdstate.c \
                   loop.c \
                   task.c \
                   $(local_source_files)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include
LOCAL_CFLAGS := -DANDROID_VERSION=$(PLATFORM_SDK_VERSION) -Wall -Werror
LOCAL_SHARED_LIBRARIES := libcutils liblog
LOCAL_MODULE:= libfdio
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)
