include $(CLEAR_VARS)

LOCAL_MODULE := boost_system
LOCAL_SRC_FILES := $(BOOST_PATH)/lib/armeabi-v7a/libboost_system.a
LOCAL_EXPORT_C_INCLUDES := $(BOOST_INCL)
	
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := boost_thread
LOCAL_SRC_FILES := $(BOOST_PATH)/lib/armeabi-v7a/libboost_thread.a
LOCAL_EXPORT_C_INCLUDES := $(BOOST_INCL)
	
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := boost_atomic
LOCAL_SRC_FILES := $(BOOST_PATH)/lib/armeabi-v7a/libboost_atomic.a
LOCAL_EXPORT_C_INCLUDES := $(BOOST_INCL)
	
include $(PREBUILT_STATIC_LIBRARY)