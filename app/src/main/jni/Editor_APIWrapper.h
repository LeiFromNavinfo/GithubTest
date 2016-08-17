#ifndef FM_SDK_APIWrapper_h
#define FM_SDK_APIWrapper_h

#include "Editor.h"
#include "Editor_ProjectManager.h"

namespace Editor
{
    class EDITOR_API APIWrapper
    {
        friend class CoreMapDataSource;
    protected:
        DataManager*          m_pDataManager;
        
        CoreMapDataSource*    m_pCoreDataSource;
        
        GdbDataSource*        m_pGdbDataSource;

        APIWrapper();
    private:
        static APIWrapper*    m_pInstance;
        
        int                   InsertTipsGeo(DataFeature* tips);
        
        int                   UpdateTipsGeo(DataFeature* tips);

        int                   DeleteTipsGeo(DataFeature* tips);

        int                   InsertGeoPoint(DataFeature* tips, EditorGeometry::WkbGeometry* wkb, TIPS_GEO_COMPONENT_TYPE type, const std::string& pointDisplayStyle="");

        int                   UpdateGeoPoint(DataFeature* tips, EditorGeometry::WkbGeometry* wkb, TIPS_GEO_COMPONENT_TYPE type, const std::string& pointDisplayStyle="");
        
        int                   InsertGeoLine(DataFeature* tips);
        
        int                   InsertTipsGeoOnly(DataFeature* tips, EditorGeometry::WkbGeometry* wkb, TIPS_GEO_COMPONENT_TYPE type, const std::string& displayStyle="");
        
        int                   UpdateTipsGeoOnly(DataFeature* tips, EditorGeometry::WkbGeometry* wkb, TIPS_GEO_COMPONENT_TYPE type, const std::string& displayStyle="");

        int                   UpdateGeoLine(DataFeature* tips);
        
        int                   InsertOrUpdateGeoMixed(DataFeature* tips, OPERATION_TYPE type);

        int                   InsertGeoPolygon(DataFeature* tips);
        
        int                   UpdateGeoPolygon(DataFeature* tips);

        int                   InsertGeoComponent(const std::string& rowkey, const std::string& uuid, const std::string& tablename, int geoType);

        EditorGeometry::WkbGeometry*  GetDisplayPoint(EditorGeometry::WkbGeometry* wkb, int index);

        EditorGeometry::WkbGeometry* GetTwoDisplayPoint(EditorGeometry::WkbGeometry* wkb, EditorGeometry::WkbGeometry* &endPoint);
        EditorGeometry::WkbGeometry* ToWkbGeometry(geos::geom::Point* point);

        int                   MaintainSpecialTips(DataFeature* feature, OPERATION_TYPE type);

        std::string           GenerateStatusJsonResult(Model::ProjectUser* PU);
        
        int                  MaintainAttachmentsGeo(DataFeature* feature, OPERATION_TYPE type);
    public:

        static APIWrapper* getInstance();

        ~APIWrapper();

        //Path & User
        void                         SetRootPath(const char* rootPath);

        void                         SetUserId(const char* userId);

        std::string                  GetDataPath();

        std::string                  GetPoiPhotoPath();

        std::string                  GetTipsPhotoPath();

        //Tips
        int                          InsertTips(const std::string& jsonTips);

        int                          UpdateTips(const std::string& jsonTips);

        int                          DeleteTips(const std::string& rowkey);

        std::vector<DataFeature*>    SelectTips(const std::string& box, const std::string& point);

        std::string                  GetTipsByRowkey(const std::string& rowkey);

        //Grid
        int                          UpdateGridStatus(const std::vector<std::string>& grids);

        int                          UpdateGridStatusByBox(double dminLon, double dmaxLon, double dminLat, double dmaxLat);

        //Upload & Download
        bool                         UploadFile(const std::string& sToken, const std::string& sUser, const std::vector<std::string>& ids, int type, JOBCMD cmd, ProgressEventer* progress, const std::string& deviceId);

        bool                         DownloadFile(const std::string& sToken, const std::string& sUser, const std::vector<std::string>& ids, int type, Editor::Job::Cmd cmd, ProgressEventer* progress);

        bool                         InstallLayers(const std::string& sDownloadSqlite, const std::string& sExistsSqlite, const std::string& layers);

        void                         CancelUpload();

        void                         CancelDownload();

        //Statistics
        int                          GetCountByCondition(const std::string& tableName, int type);

        virtual std::vector<std::string>     GetSignalLayerByCondition(const std::string& sTable, int type, int pagesize, int offset);
        
        /**
         * @bref     定制分页查询，如分页查询poi ，或按距离排序查询情报
         * @param    strTable                表名
         * @param    strWhere                where的限定条件，不包括where关键词
         * @param    strOderbyFiedOrWktPoint 按字段排序的字段名，或按距离排序的wkt点
         * @param    strGeoFieldName         几何字段名
         * @param    isOderbyFied            是否是按字段排序
         * @param    isDesc                  是否降序
         * @param    pagesize                page大小
         * @param    offset                  查询起始row
         * @return   返回分页查询的features的json串信息
         */
        virtual std::vector<std::string>     QueryTableByCondition(const std::string& strTable, const std::string& strWhere,const std::string& strOderbyFiedOrWktPoint, const std::string& strGeoFieldName, bool isOderbyFied, bool isDesc , int pagesize, int offset);
        /**
         * @bref通过Where条件查询poi个数
         * @param    strTable      表名
         * @param    strWhere      where条件
         * @return count of poi
         */
        int                          GetCountByWhere(const std::string& strTable, const std::string& strWhere);


        //GDB Data
        std::string                  GetRdLineByPid(int pid);

        //Snap & Select
        std::string                  SnapLine(const std::string& box, const std::string& point);

        std::string                  GivenPathSnap(const std::string& path, const std::string& point, double disThreshold);

        std::string                  SnapRdNode(const std::string& box, const std::string& point);

        std::vector<std::string>     PolygonSelectLines(const std::string& region, bool isIntersectIncluded);

        //Version
        bool                         InitVersionInfo(int type);

        //GPS
        int                          InsertGpsLine(const std::string& gpsJson);
		int                          InsertGpsLineTip(const std::string& tipJson);
		std::string                  GetTipsRowkey(const std::string& typetype);

		int                          DeleteSurveyLine(const std::string& rowkey, const std::vector<std::string>& reTipsRowkeys);

		int                          QueryReTipsOnSurveyLine(const std::string& surveyLineRowkey, std::vector<std::string>& reTipsRowkeys);

        std::string                  GetCoremapModelVersion(int type);

        bool						 InstallGdb(std::string sPath, std::string sFlag, ProgressEventer* progress);
        
        bool                         RepalceGDB();
        
        std::string					 LinkMatch(std::string locationJson);
        
		double                       CaclAngle(const std::string& point, const std::string& linkId, int type);
        
        std::vector<std::string>     QueryFeatures(const std::string& tableName, const std::string& searchQuery);
        
        std::vector<std::string>     QueryFeaturesBySql(const std::string& tableName, const std::string& sql);
        
        std::string                  GetDownloadOrUploadStatus(const std::string& projectId, const std::string& userId, int type);
        
		std::string                  RobTask(const std::string& token, const std::string& projectId, const std::string& taskId);
        
        //TrackPoint
        /*
         * const std::string id="";
         * double latitude=41.01346;
         * double longitude=116.47962;
         * double direction=20.324325;
         * double speed=6.718;
         * const std::string recordTime="20160523131245";//YYYYMMDDHHMMSS
         * int userId=3655;
         * const std::string segmentId="3655"+recordTime;
         *
         */
        int                          InsertTrackPoint(const std::string& id, double latitude,
                                                      double longitude, double direction, double speed,
                                                      const std::string& recordTime,
                                                      int userId, const std::string& segmentId);
		std::string                  GetDownloadPrepared(const std::string& input);

        void                         DownloadFile(const std::string& input, ProgressEventer* progress);

        std::string                  GetUploadPrepared(const std::string& input);

        void                         UploadFile(const std::string& input, ProgressEventer* progress);
        
        //General Insert/Update/Delete
        int                          InsertFeature(const std::string& tableName, const std::string& json);
        
        int                          UpdateFeature(const std::string& tableName, const std::string& json);
        
        int                          DeleteFeature(const std::string& tableName, int rowId);
        
        //统计接口
        /*
         * @bref     按日期统计轨迹长度
         * @param    userId             用户id
         * @param    results[in/out]    返回的json结果：例如[{count:6,date:"20160404",trackLength:50.0}]
         * @return                      0，表示正常；-1，有异常。
         */
        int                          TrackStatistics(const std::string& userId, std::vector<std::string>& results);
        
        /*
         * @bref     按userid和日期删除轨迹点
         * @param    userId         用户id
         * @param    date           日期
         * @return                  0，表示正常；-1，有异常。
         */
        int                          DeleteTrack(const std::string& userId, const std::string& date);
        
        //Poi
        virtual int                          InsertPoi(const std::string& poi, const std::string& userId, const std::string& projectId);
        
        virtual int                          UpdatePoi(const std::string& poi, const std::string& userId, const std::string& projectId);
        
        virtual int                          DeletePoi(int rowId, const std::string& userId, const std::string& projectId);
        
        virtual int                          UpdatePois(const std::list<std::string> & pois, const std::string& userId, const std::string& projectId, std::list<int> & failIndexs, ProgressEventer* progress);
        
        virtual std::vector<std::string>     SnapPoint(const std::string& box, const std::string& point);
        
        virtual std::vector<DataFeature*>    SelectPois(const std::string& box);
        
        virtual std::string                  GetTotalPoiByFid(const std::string& fid);
        
        virtual std::vector<std::string>     SelectMetaData(const std::string& sTable, std::string condition, int type);
        
        virtual std::vector<std::string>     GetProjectInfo(const std::string& sToken);
        
        virtual std::vector<std::string>     GetProjectDetails(const std::string& sToken, const std::string& sProjectId);
        
        virtual std::vector<std::string>     GetTaskDetails(const std::string& sToken, const std::string& sProjectId);
        
        virtual std::string			         GetEditHistroyByFid(const std::string& fid);
        
        virtual int                          PoiStatistics(const std::string& tableName, std::vector<std::string>& results);
        
        //Info
        /**
         *  插入情报数据
         */
        virtual int                          InsertInfos(const std::string& infosJson);
        
        //更新情报数据
        virtual int                          UpdateInfo(const std::string& info);
        
        //根据gloabalid 删除情报数据
        virtual int                          DeleteInfo(const std::string& globalid);
        
        /*
         * @brief 同步轨迹数据
         *
         */
        virtual int                          SyncTrackData2Server(const std::string sToken,
                                                                  const std::string sUserID,
                                                                  const std::string sProjectID,
                                                                  const Editor::ProgressEventer* pregress);
        
        /*
         * @根据任务圈和项目号删除poi和情报
         *
         */
        virtual int                          DeletePoiAndInfoByRegion(const std::string& projectId, const std::string& region){return -1;}
    };
    
    class EDITOR_API APIWrapper815: public APIWrapper
    {
    private:
        APIWrapper815(){};
        
        static APIWrapper815*    m_pInstance;
    public:
        static APIWrapper815*                getInstance();
        //Poi
        virtual int                          InsertPoi(const std::string& poi, const std::string& userId, const std::string& projectId);
        
        virtual int                          UpdatePoi(const std::string& poi, const std::string& userId, const std::string& projectId);
        
        virtual int                          DeletePoi(int rowId, const std::string& userId, const std::string& projectId);
        
        virtual int                          UpdatePois(const std::list<std::string> & pois, const std::string& userId, const std::string& projectId, std::list<int> & failIndexs, ProgressEventer* progress);
        
        virtual std::vector<std::string>     SnapPoint(const std::string& box, const std::string& point);
        
        virtual std::vector<DataFeature*>    SelectPois(const std::string& box);
        
        virtual std::string                  GetTotalPoiByFid(const std::string& fid);
        
        virtual std::vector<std::string>     SelectMetaData(const std::string& sTable, std::string condition, int type);
        
        virtual std::vector<std::string>     GetProjectInfo(const std::string& sToken);
        
        virtual std::vector<std::string>     GetProjectDetails(const std::string& sToken, const std::string& sProjectId);
        
        virtual std::vector<std::string>     GetTaskDetails(const std::string& sToken, const std::string& sProjectId);
        
        virtual std::string			         GetEditHistroyByFid(const std::string& fid);
        
        //Info
        /**
         *  插入情报数据
         */
        virtual int                          InsertInfos(const std::string& infosJson);
        
        //更新情报数据
        virtual int                          UpdateInfo(const std::string& info);
        
        //根据gloabalid 删除情报数据
        virtual int                          DeleteInfo(const std::string& globalid);
        
        virtual int                          SyncTrackData2Server(const std::string sToken,
                                                                  const std::string sUserID,
                                                                  const std::string sProjectID,
                                                                  const Editor::ProgressEventer* pregress);
        
        /*
         * @根据任务圈和项目号删除poi和情报
         *
         */
        virtual int                          DeletePoiAndInfoByRegion(const std::string& projectId, const std::string& region);
    };
}

#endif
