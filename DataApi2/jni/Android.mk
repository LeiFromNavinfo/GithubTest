LOCAL_PATH := $(call my-dir)

#ROOT_PATH := /home/travis/build/panhuidan/GithubTest
#ROOT_PATH := /home/ubuntu/GithubTest
ROOT_PATH := ../..

ICONV_PATH := ../../ThirdParty/OpenSource2/libiconv-1.13.1
ICONV_INCL := $(ROOT_PATH)/ThirdParty/OpenSource2/libiconv-1.13.1

SQLITE_PATH := ../../ThirdParty/OpenSource2/sqlite-amalgamation-3081002
SQLITE_INCL := $(ROOT_PATH)/ThirdParty/OpenSource2/sqlite-amalgamation-3081002/include

SQLITE_NDS_PATH := ../../ThirdParty/OpenSource2/sqlite_nds
SQLITE_NDS_INCL := $(ROOT_PATH)/ThirdParty/OpenSource2/sqlite_nds/include

PROJ_PATH := ../../ThirdParty/OpenSource2/proj-4.9.1
PROJ_INCL := $(ROOT_PATH)/ThirdParty/OpenSource2/proj-4.9.1/src

GEOS_PATH := ../../ThirdParty/OpenSource2/geos-3.4.2
GEOS_INCL := $(ROOT_PATH)/ThirdParty/OpenSource2/geos-3.4.2

LIBXML2_PATH := ../../ThirdParty/OpenSource2/libxml2-2.9.1
LIBXML2_INCL := $(ROOT_PATH)/ThirdParty/OpenSource2/libxml2-2.9.1/include

LZMA_PATH := ../../ThirdParty/OpenSource2/xz-5.1.3alpha
LZMA_INCL := $(ROOT_PATH)/ThirdParty/OpenSource2/xz-5.1.3alpha/src/liblzma/api

SPATIALITE_PATH := ../../ThirdParty/OpenSource2/libspatialite-4.3.0
SPATIALITE_INCL := $(ROOT_PATH)/ThirdParty/OpenSource2/libspatialite-4.3.0/headers

RAPIDJSON_INCL := $(ROOT_PATH)/ThirdParty/OpenSource2/rapidjson/include

BOOST_PATH := ../../ThirdParty/OpenSource2/boost_1_55_0
BOOST_INCL := $(ROOT_PATH)/ThirdParty/OpenSource2/boost_1_55_0

LIBCURL_PATH := $(ROOT_PATH)/ThirdParty/OpenSource2/curl-7.37.0
LIBCURL_INCL := $(ROOT_PATH)/ThirdParty/OpenSource2/curl-7.37.0/include

LIBZIP_PATH := ../../ThirdParty/OpenSource2/libzip-android
LIBZIP_INCL := $(ROOT_PATH)/ThirdParty/OpenSource2/libzip-android

EDITOR_PATH := ./Editor
EDITOR_INCL := $(ROOT_PATH)/DataApi2/jni/Editor

#NDK_PATH := /Android/android-ndk-r9d

include $(LOCAL_PATH)/Spatialite.mk
include $(LOCAL_PATH)/boost.mk
include $(LOCAL_PATH)/libcurl.mk
include $(LOCAL_PATH)/libzip.mk

include $(LOCAL_PATH)/Editor.mk