include $(CLEAR_VARS)

LOCAL_MODULE := Editor
LOCAL_C_INCLUDES := \
    $(RAPIDJSON_INCL) \
	$(EDITOR_INCL)
	
LOCAL_SRC_FILES := \
    $(EDITOR_PATH)/Logger.cpp \
    $(EDITOR_PATH)/Geometry.cpp \
    $(EDITOR_PATH)/Tools.cpp \
    $(EDITOR_PATH)/Editor_DataManager.cpp \
    $(EDITOR_PATH)/Editor_DataUploader.cpp \
    $(EDITOR_PATH)/Editor_DataDownloader.cpp \
    $(EDITOR_PATH)/Editor_DataServiceAddress.cpp \
    $(EDITOR_PATH)/Editor_DataSource.cpp \
    $(EDITOR_PATH)/Editor_DataFileSystem.cpp \
    $(EDITOR_PATH)/Editor_DataLayer.cpp \
    $(EDITOR_PATH)/Editor_DataFeature.cpp \
    $(EDITOR_PATH)/Editor_DataFeatureFactory.cpp \
    $(EDITOR_PATH)/Editor_DataFeaturePoi.cpp \
    $(EDITOR_PATH)/Editor_DataFeatureInfor.cpp \
    $(EDITOR_PATH)/Editor_DataFeatureRdLine.cpp \
	$(EDITOR_PATH)/Editor_DataFeatureGPSLine.cpp \
    $(EDITOR_PATH)/Editor_DataFeatureTip.cpp \
    $(EDITOR_PATH)/Editor_DataSelector.cpp \
    $(EDITOR_PATH)/DataTransfor.cpp \
    $(EDITOR_PATH)/Editor_DataSnapper.cpp \
    $(EDITOR_PATH)/Editor_DataJob.cpp \
    $(EDITOR_PATH)/Editor_DataAddress.cpp \
    $(EDITOR_PATH)/Editor_ProjectManager.cpp \
    $(EDITOR_PATH)/Editor_MapDataInstaller.cpp \
    $(EDITOR_PATH)/Editor_MapDataPackager.cpp \
    $(EDITOR_PATH)/Editor_EditAttrs.cpp \
    $(EDITOR_PATH)/Editor_EditHistory.cpp \
    $(EDITOR_PATH)/Editor_EditOperator.cpp \
    $(EDITOR_PATH)/Editor_EditContent.cpp \
    $(EDITOR_PATH)/Editor_Grid.cpp \
    $(EDITOR_PATH)/Editor_GridHelper.cpp\
    $(EDITOR_PATH)/Editor_MetaDataManager.cpp\
    $(EDITOR_PATH)/Editor_ClassificationSelector.cpp \
	$(EDITOR_PATH)/Editor_SnapEnvironment.cpp \
	$(EDITOR_PATH)/Editor_LocationManager.cpp \
    $(EDITOR_PATH)/model.cpp \
    $(EDITOR_PATH)/Locker.cpp \
    $(EDITOR_PATH)/geohash.cpp \
    $(EDITOR_PATH)/driver.cc \
    $(EDITOR_PATH)/md5.cc \
    $(EDITOR_PATH)/Editor_UploadJob.cpp \
    $(EDITOR_PATH)/Editor_APIWrapper.cpp \
	$(EDITOR_PATH)/Editor_DataFeatureRdNode.cpp \
	$(EDITOR_PATH)/Editor_DataFunctor.cpp \
	$(EDITOR_PATH)/GeometryCalculator.cpp \
	$(EDITOR_PATH)/Editor_Track.cpp\
    $(EDITOR_PATH)/Editor_DataFeatureTrackPoint.cpp\
    $(EDITOR_PATH)/Editor_DataFeatureTrackSegment.cpp\
    JniLoad.cpp \
    
LOCAL_LDFLAGS += -llog -lz 

LOCAL_STATIC_LIBRARIES := sqlite spatialite iconv proj geos libxml2 liblzma boost_system boost_thread boost_atomic libzip
LOCAL_SHARED_LIBRARIES := libcurl
#LOCAL_SHARED_LIBRARIES := spatialite sqlite_nds iconv proj geos libxml2 liblzma libcurl
#LOCAL_STATIC_LIBRARIES := boost_system boost_thread boost_atomic libzip

include $(BUILD_SHARED_LIBRARY)
