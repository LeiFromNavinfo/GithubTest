include $(CLEAR_VARS)

LOCAL_MODULE := libcurl
LOCAL_SRC_FILES := $(LIBCURL_PATH)/lib/armeabi-v7a/libcurl.so
LOCAL_EXPORT_C_INCLUDES := $(LIBCURL_INCL)
                           
include $(PREBUILT_SHARED_LIBRARY)