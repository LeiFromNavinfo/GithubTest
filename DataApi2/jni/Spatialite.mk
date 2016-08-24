include $(CLEAR_VARS)

LOCAL_MODULE := iconv
LOCAL_SRC_FILES := $(ICONV_PATH)/lib/armeabi-v7a/libiconv.a
LOCAL_EXPORT_C_INCLUDES := $(ICONV_INCL)/include \
						   $(ICONV_INCL)/libcharset/include
                           
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := sqlite
LOCAL_SRC_FILES := $(SQLITE_PATH)/lib/armeabi-v7a/libsqlite.a
LOCAL_EXPORT_C_INCLUDES := $(SQLITE_INCL)
	
include $(PREBUILT_STATIC_LIBRARY)

#include $(CLEAR_VARS)

#LOCAL_MODULE := sqlite_nds
#LOCAL_SRC_FILES := $(SQLITE_NDS_PATH)/lib/armeabi-v7a/libsqlite_nds.so
#LOCAL_EXPORT_C_INCLUDES := $(SQLITE_NDS_INCL)
	
#include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := proj
LOCAL_SRC_FILES := $(PROJ_PATH)/lib/armeabi-v7a/libproj.a
LOCAL_EXPORT_C_INCLUDES := $(PROJ_INCL)
	
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := geos
LOCAL_SRC_FILES := $(GEOS_PATH)/lib/armeabi-v7a/libgeos.a
LOCAL_EXPORT_C_INCLUDES := $(GEOS_INCL)/include
	
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := xml2
LOCAL_SRC_FILES := $(LIBXML2_PATH)/lib/armeabi-v7a/libxml2.a
LOCAL_EXPORT_C_INCLUDES := $(LIBXML2_INCL)
	
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := lzma
LOCAL_SRC_FILES := $(LZMA_PATH)/lib/armeabi-v7a/liblzma.a
LOCAL_EXPORT_C_INCLUDES := $(LZMA_INCL)
	
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := spatialite
LOCAL_SRC_FILES := $(SPATIALITE_PATH)/lib/armeabi-v7a/libspatialite.a
LOCAL_EXPORT_C_INCLUDES := $(SPATIALITE_INCL)
	
include $(PREBUILT_STATIC_LIBRARY)



