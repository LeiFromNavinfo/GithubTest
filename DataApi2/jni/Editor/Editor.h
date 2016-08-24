#ifndef FM_SDK_Editor_h
#define FM_SDK_Editor_h

#include <string>
#include <vector>
#include <queue>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <map>
#include "curl.h"
#include "boost/thread.hpp"
#include "boost/bind.hpp"
#ifdef WIN32
#include <stdint.h>
#endif
//#include <nds_sqlite3.h>
#include <sqlite3.h>
#include <spatialite.h>
#include <document.h>
#include "Editor_Define.h"
#include "Geometry.h"
#include "Logger.h"
#include "Tools.h"
#include "Model.h"
#include <set>

namespace geos{namespace geom{
    class Geometry;
}}

using namespace rapidjson;

#ifdef WIN32
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#ifndef R_OK
#define R_OK 04
#endif
#ifndef F_OK
#define F_OK 00
#endif
#endif

namespace Editor 
{
    class SnapResult;
    class DataSnapper;
    class CurlHttpRequest;
    class JSON;

    class ProgressEventor;

    class DataFileSystem;
    class DataUploader;
    class DataDownloader;
    class DataServiceAddress;

    class DataSource;
    class DataLayer;
    class DataFeature;

    class DataFeaturePoi;
    class DataFeatureInfor;
    class DataFeatureTip;
    class DataFeatureRdLine;
    class DataFeatureGPSLine;

    class  DataFeatureTipsGeo;
    class  DataFeatureTipsGeoComponent;

    class DataSelector;
    class PoiClassificationSelector;

    class Matcher;
    class LocationManager;
	
    class Filter;
	
    class Result;
	
    class Status;
	
    class Reliability;
    class DataFunctor;
    enum DATALAYER_TYPE
    {
        DATALAYER_POI = 0,
        DATALAYER_INFOR,
        DATALAYER_RDLINE,
        DATALAYER_BKLINE,
        DATALAYER_FACE,
        DATALAYER_RDLINE_GSC,
        DATALAYER_TIPS,
        DATALAYER_GPSLINE,
        DATALAYER_TIPSPOINT,
        DATALAYER_TIPSLINE,
        DATALAYER_TIPSMULTILINE,
        DATALAYER_TIPSPOLYGON,
        DATALAYER_TIPSGEOCOMPONENT,
        DATALAYER_RDNODE,
        DATALAYER_TRACKPOINT,
        DATALAYER_TRACKSEGMENT,
        DATALAYER_PROJECTUSER,
        DATALAYER_PROJECTINFO,
        DATALAYER_TASKINFO
    };

    enum FIELD_TYPE
    {
        FT_INTEGER,
        FT_DOUBLE,
        FT_TEXT,
        FT_BLOB,
        FT_GEOMETRY,
    };

    typedef enum LIFECYCLE
	{
		LIFECYCLE_ORIGINAL= 0,

		LIFECYCLE_DELETE = 1,

		LIFECYCLE_UPDATE = 2,

		LIFECYCLE_ADD = 3,

	}LIFECYCLE;

    typedef enum TSTATUS
	{
		TSTATUS_ORIGINAL= 0,

		TSTATUS_WORKED = 1,

		LIFECYCLE_COMMIT = 2

	}TSTATUS;

    typedef enum FEATURE_GEOTYPE
	{
		GEO_POINT= 0,

		GEO_LINE = 1,

		GEO_POLYGON = 2,

        GEO_MULTILINE = 3,
        
        //complex tips, e.g: bridge
        GEO_MIXED = 4

	}FEATURE_GEOTYPE;

    typedef enum OPERATION_TYPE
    {
        OPERATION_INSERT =0,

        OPERATION_UPDATE =1,

        OPERATION_DELETE =2
    }OPERATION_TYPE;

    typedef enum EVIROMENT
    {
    	EVIROMENT_CLOUD = 0,

    	EVIROMENT_TEST =1,

    	EVIROMENT_DEBUG = 2,

    	EVIROMENT_REL = 3,

    }EVIROMENT;
    
    typedef enum JOBCMD
    {
        Restart = 0,
        Continue,
    }JOBCMD;
    
    typedef enum DATASOURCE_TYPE
    {
        DATASOURCE_UNKNOWN = -1,
        
        DATASOURCE_COREMAP = 0,
        
        DATASOURCE_GDB = 1,
        
        DATASOURCE_PROJECT = 2
        
    }DATASOURCE_TYPE;
    
    typedef enum TIPS_GEO_COMPONENT_TYPE
    {
        ICON = 0,
        
        START_NODE = 1,
        
        END_NODE = 2,
        
        SIMPLE_LINE = 3,
        
        MULTIL_LINE =4,
        
        POLYGON =5
        
    }TIPS_GEO_COMPONENT_TYPE;
    // 1 照片；2 语音；3 文字; 4 Json；5 图片；6 草图；
    typedef enum TIPS_Attachments_TYPE
    {
        TIPS_Attachments_Photo = 1,
        TIPS_Attachments_Voice = 2,
        TIPS_Attachments_Text = 3,
        TIPS_Attachments_Json = 4,
        TIPS_Attachments_Picture = 5,
        TIPS_Attachments_Draft = 6
        
    }TIPS_Attachments_TYPE;
    
    typedef enum Editor_PoiLifecycle_Type
    {
        Editor_PoiLifecycle_Unknown=0,
        Editor_PoiLifecycle_Del=1,
        Editor_PoiLifecycle_Modify=2,
        Editor_PoiLifecycle_Add=3
        
    }Editor_PoiLifecycle_Type;
    
    class Editor_PoiStatisticsResult
    {
    public:
        int sumCount;
        int delCount;
        int modifyCount;
        int addCount;
        std:: string toperateDate;
        Editor_PoiStatisticsResult()
        {
            sumCount=delCount=modifyCount=addCount=0;
            toperateDate="";
        }
    };

    class EDITOR_API ProgressEventer
    {
    public:

        virtual int         OnStarted() = 0;

        virtual int         OnFinished() = 0;

        virtual int         OnFailed() = 0;

        virtual int         OnProgress(std::string description, double progress) = 0;
    };

    class EDITOR_API DataManager
    {
    private:

        DataManager();

        static DataManager* m_pInstance;

        DataUploader*       m_pDataUploader;

        DataDownloader*     m_pDataDownloader;

        DataServiceAddress* m_pDataServiceAddress;

        DataSource*			m_pDataSource;

        DataFileSystem*		m_pDataFileSystem;


        DataFunctor*        m_pDataFunctor;
        
        std::map<int, DataSource*> m_dataSourceMap;
        
        DataSource* createDataSource(int type);
        
    public:

        static DataManager* getInstance();

        ~DataManager();

    public:

        DataUploader*       getUploader();

        DataDownloader*     getDownloader();

        DataServiceAddress* getServiceAddress();

        DataSource*         getDataSource(int type);
        
        DataSource*         getDataSource(const std::string& layerName);
        
        DataSource*         getDataSourceByLayerType(int layerType);

        DataFileSystem*     getFileSystem();

        DataFunctor*        getDataFunctor();
    };

    class EDITOR_API DataFileSystem
    {
    private:
        friend class DataManager;

        DataFileSystem();

        std::string         m_pRootPath;

        std::string         m_pUserId;

    public:

        ~DataFileSystem();

        std::string         GetDownloadPath();

        std::string         GetUploadPath();

        std::string         GetCoreMapDataPath();

        std::string	        GetCachesPath();

        std::string         GetPoiPhotoPath();

        std::string         GetMetaPath();

        std::string         GetPoiUploadPath();
        
        std::string			GetTipsPhotoPath();

        void                SetRootPath(const char* rootPath);

        void                SetCurrentUser(const char* userId);

        std::string         GetCurrentUserFolderPath();
        
        /*
        * @brief gdb安装的中转目录
        *
        */
        std::string         GetTempGDBDataDir();
        
        /*
         * @brief gdb同时编辑和渲染的目录
         *
         */
        std::string         GetEditorGDBDataDir();
        
        /*
         * @brief gdb安装的中转数据库
         *
         */
        std::string         GetTempGDBDataFile();
        
        /*
         * @brief gdb同时编辑和渲染的数据库
         *
         */
        std::string         GetEditorGDBDataFile();

        /** 
         * @brief 获取项目管理Sqlite文件路径
         * @return 文件路径
        */
        std::string         GetProjectDataPath();

        void                InitUserDataBase();
        
        void                InitUserGDBDataBase();

        std::string         GetTemplateFolderPath();

        std::string         GetCurrentUserId();

    };

    class EDITOR_API Address
    {
    public:

            Address();

            ~Address();                      

            CURL*				CreateDownloadRequest();

            CURL*				CreateUploadRequest();

            long				GetDownloadFileSize();

            long				GetLocalFileSize();

            void				SetUrl(const std::string& sUrl);

            void				SetFileName(const std::string& sFileName);

            void				SetOnPregress(Editor::ProgressEventer* onPregress);

        public:

            std::string m_sUrl;

            std::string m_sFile;

            long m_lUploadPos;

            std::ifstream m_infile;

            std::ofstream m_outfile;

            Editor::ProgressEventer* m_onPregress;
        };

    class EDITOR_API DataServiceAddress
    {
    private:
        friend class DataManager;

        DataServiceAddress();

    public:

        ~DataServiceAddress();

        Address*  get__DownloadAddress();

        Address*  get__UploadAddress();

    public:
        bool            Init(int ver);

        std::string     GetGDBDwonloadUrl(std::string sProjectId);

        std::string     GetPoiDownloadUrl(const std::string& token, std::string sProjectId, std::string sUserId, std::string date, bool isIncremental);

        std::string		GetPoiUploadUrl(std::string sFileName, std::string sProject);

        std::string     GetPoiUploadExcuteUrl(const std::string& token);

        std::string     GetInforUploadExcuteUrl(const std::string sToken, const std::string sFileName);
        std::string     GetPoiUploadSnapShot(const std::string& token, const std::string& fileName, const std::string& md5, unsigned long fileSize);

        std::string     GetPoiUploadDataChunk(const std::string& token, const std::string& fileName, int chunkNo);

        std::string     GetPoiUploadCheck(const std::string& token, const std::string& fileName);

        std::string     GetInfoDownloadUrl(const std::string& token);

        std::string     GetTipsDownloadUrl(const std::vector<std::string>& grids);
        
        std::string     GetTipsUploadStartUrlRoot();
        
        std::string		GetTipsUploadChunkUrlRoot();
        
        std::string		GetTipsUploadFinishUrlRoot();
        
        std::string		GetTipsUploadCheckUrlRoot();
        
        std::string     GetTipsUploadImportUrlRoot();

        std::string		GetUpdataGridStatusUrlRoot();

        std::string     GetProjectInfomationUrl(const std::string& sToken);

        std::string     GetProjectDetailsUrl(const std::string& sToken);

        std::string     GetTaskDetailsUrl(const std::string& sToken);

        std::string     GetPatternImageDownLoadUrl();

        std::string     GetModelVersionUrl(int type);
        std::string     GetRobTaskUrl(const std::string& token, const std::string& projectId, const std::string& taskId);
        std::string     GetUploadTime(const std::string& token, const std::string& projectId, const std::string& deviceId);
    private:
        std::string     m_sRootUrlPoi;
        std::string     m_sRootUrlGDB;
        std::string     m_sRootUrlTips;
    };

    class EDITOR_API Job
    {
    public:
        enum Cmd
        {
            Restart,
            Continue,
        };

    public:

        Job();

        virtual ~Job();

        void			SetUploadTarget(Address* addr, std::string target);

        void			SetDownloadTarget(Address* addr);

        void			Stop(Job* job);

        void			SetOnPregress(Editor::ProgressEventer* onPregress);

        void			SetUrl(const std::string& sUrl);

        void			SetFileName(const std::string& sFileName);

        void			SetTotalSize(long totalsize);

        void			SetCurrentSize(long currentsize);

        void            SetJobType(int type);

        void            SetProjectUser(Model::ProjectUser* pProjectUser);

        Model::ProjectUser* GetProjectUser();

        std::string		GetTargetFile();

        std::string		GetFileName();

        std::string		GetFinishedFile();

        long			GetTotalSize();

        long			GetCurrentSize();

        long			GetProgress();

        void			SetUploadModel(Cmd cmd);

        long			GetDownloadFileSize();

        long			GetLocalFileSize();

        int             GetJobType();

        Editor::ProgressEventer* GetPregressObj();

    private:
        friend class DataUploader;

        friend class DataDownloader;

        CURL*			CreateUploadRequest(Job* job);	

        CURL*			CreateDownloadRequest(Job* job);	

        void			UploadFile(DataUploader* uploader, Job* job);

        int				DownloadFile(DataDownloader* downloader, Job* job, Editor::Address* pAddress);

        void			StartDownload(DataDownloader* downloader, Job* job, Editor::Address* pAddress);

        void            StartUpload(DataUploader* uploader, Job* job);

        char*           GetUploadDataByChuckNo(unsigned long& nLength);

    public:

        std::ofstream m_outfile;

        int m_nCount;

        int m_nRepeatUploadNum;

        std::string m_sFoundFlag;

        unsigned long m_nMaxChunkSize;

        unsigned long m_nChunkSize;

        int m_nChunkNo;

        std::string m_sFileName;

        std::string m_sToken;

        std::string m_sMd5;

        std::string m_sDeviceId;
        std::string m_sResponse;

        unsigned long m_lTotal;
        long m_connectTime;

    private:

        CURL* m_curl;

        long m_nTotal;

        long m_nNow;

        std::string m_sUrl;

        std::string m_sFile;

        int m_nType;

        Cmd m_uploadModel;

        boost::shared_ptr<boost::thread> m_thread; 

        Model::ProjectUser* m_pProjectUser;

        Editor::Address* m_pAddress;

        Editor::ProgressEventer* m_onPregress;
    };

    class EDITOR_API UploadJob
    {
        friend class DataUploader;
        
    public:
        UploadJob();
        
        ~UploadJob();
        
    public:
        void SetUploadTarget(const char* target);
        
        std::string GetUploadTarget();
        
        void SetUploadFileName(const char* fileName);
        
        std::string GetUploadFileName();
        
        void SetUploadJobId(int jobId);
        
        int GetUploadJobId();
        
        void SetUploadChunk(int chunkNo, int chunkSize);
        
        int GetUploadChunkNo();
        
        int GetUploadChunkSize();
        
        int GetUploadChunkLen();
        
        void SetUploadParam(const char* param);
        
        int  GetJobStatus();
        
        void SetCurrentSize(double size);
        
        double GetCurrentSize();
        
        void SetTotalSize(double size);
        
        double GetTotalSize();
        
        DataUploader* GetUploader();

    public:
        std::string m_sWkt;
        
    private:
        void Start();
        
        void Stop();
        
        void Upload();
        
        void MultiPost();
        
        int ParseMultiPostResponse(std::string response, std::string& errmsg);
        
        char* ReadUploadChunk(int& length);
        
    private:
        std::string m_Target;
        
        std::string m_FileName;
        
        int m_JobId;
        
        int m_ChunkNo;
        
        int m_ChunkSize;
        
        std::string m_Param;
        
        CURL* m_Curl;
        
        CURLM* m_MultiHandle;
        
        struct curl_httppost* m_FormPost;
        
        struct curl_slist* m_HeaderList;
        
        char* m_Chunk;
        
        int m_ChunkLen;
        
        int m_JobStatus;	//0:init 1:success -1:failure 2:stop
        
        double m_CurrentSize;
        
        double m_TotalSize;
        
    private:
        boost::shared_ptr<boost::thread> m_thread;
    };
    
    class EDITOR_API DataUploader
    {
    private:
        friend class DataManager;
        
        DataUploader();
        
    public:
        
        ~DataUploader();
        
        void                SetMaxConcurrent(int concurrent);
        
        void                StartJob(UploadJob* job);
        
        void				StartJob(Job* job, Editor::ProgressEventer* onPregress);

        void                CancelJob(UploadJob* job);
        
        void                Cancel();
        
        void				CallBack(Editor::Job* job);

        void				JobFinishedCallback(UploadJob* job);
        
        void				JobProgressCallback(UploadJob* job);
        
        void				SetUploadFileInfo(const char* fileName, double fileSize, int chunkCount);
        
        int 				GetUploadFileChunkCount(const char* fileName);
        
        void				RefreshJobStatusByFile(const char* fileName, std::set<int>& finishedChunkNo);
        
        void				RefreshJobFinishedSizeByFile(UploadJob* job, double finishedChunkSize);
        
        void				SetProgress(ProgressEventer* progress);
        
        std::string			GetProgressMsg(UploadJob* job);

        void                RefreshProgress(const std::string& flag, unsigned long totalSize, Editor::ProgressEventer* progress);

    private:
        bool                ParseUploadChunkResponse(const std::string& response);
        
    private:
        
        int m_iConcurrent;
        
        boost::mutex m_mutex;
        
        ProgressEventer* m_Progress;
        
        std::vector<UploadJob*> m_vJobs;
        
        std::queue<UploadJob*> m_qWaitJobs;
        
        std::vector<Editor::Job*> m_vJobsBefore;

        std::queue<Editor::Job*> m_qWaitJobsBefore;

        std::map<std::string, unsigned long> m_mUpload;

        std::map<std::string, std::map<int, int> > m_mJobStatus;
        
        std::map<std::string, int> m_UploadFileChunkCount;
        
        std::map<std::string, std::pair<double, double> > m_UploadFileSize;
        
        std::map<std::string, std::map<int, double> > m_UploadFileFinishedSize;
    };
    
    class EDITOR_API DataDownloader
    {
    private:
        friend class DataManager;
        friend class MapDataInstaller;
        DataDownloader();

    public:

        ~DataDownloader();

        void                StartJob(Job* job, Editor::Address* pAddress, Editor::ProgressEventer* onPregress);

        void                CancelJob(Job* job);

        void                Cancel();

        void				CallBack(Editor::Job* job);

        std::vector<Job*>   GetCurrentDownloadJobs();

        void                SetMaxConcurrent(int concurrent);

        double              GetTotalPregress();

    private:

        int m_nMaxProcess;

        boost::mutex m_mutex;

        Editor::ProgressEventer* m_onPregress;

        std::vector<std::pair<Job*, Address*> > m_vJobs;

        std::queue<std::pair<Job*, Address*> > m_qWaitJobs;
    };

    class EDITOR_API SnapEnvironment
    {
    private:
        std::vector<DataSnapper*>   m_pSnappers;
    public:
		
		SnapEnvironment();

		virtual ~SnapEnvironment();

		void					    AddSnapper(DataSnapper* snapper);

        void                        RemoveSnapper(DataSnapper* snapper);

        /*
			All layers snap
		*/
        SnapResult*	                Snapping(EditorGeometry::Box2D, EditorGeometry::Point2D);
    };

    class EDITOR_API DataSnapper
    {
    private:

        DataLayer*                  m_pDataLayer;

    public:

        DataSnapper();

        ~DataSnapper();

        void			            ResetTargetLayers();

        void			            SetTargetLayer(DataLayer* layer);

        void			            StartSnap(EditorGeometry::Box2D, EditorGeometry::Point2D);

        SnapResult*	                Snapping(EditorGeometry::Box2D, EditorGeometry::Point2D);

        SnapResult*	                GivenPathSnapping(geos::geom::Geometry* path, geos::geom::Geometry* point, double disThreshold);

        void			            StopSnap();
    };

    class EDITOR_API SnapResult
    {
    private:
        DataFeature*               m_pSnapFeature;

        int                        m_pLayerType;

        EditorGeometry::Point2D    m_pCloestPoint;

        double                     m_pDistance;
    public:
        SnapResult();

        ~SnapResult();

        void                       SetSnapFeature(DataFeature* feature);

        void                       SetCloestPoint(double x, double y);

        void                       SetDistance(double distance);

        void                       SetDataLayerType(int type);

        JSON                       GetJson();

        double                     GetDistance();

        int                        GetLayerType();

        DataFeature*               GetSnapFeature();
    };

    class EDITOR_API DataSelector
    {
    private:
        friend class DataLayer;

        std::vector<DataLayer*>     m_pSelectedLayers;

    public:

        DataSelector();

        ~DataSelector();

        void						AddTargetLayer(DataLayer* layer);

        void						ResetTargetLayers();

        std::vector<DataFeature*>   PointSelect(EditorGeometry::Box2D box, EditorGeometry::Point2D point, DATALAYER_TYPE layer_type);
        
        std::vector<DataFeature*>   PointSelect(EditorGeometry::Box2D box, EditorGeometry::Point2D point);

        DataFeature*				SelectNearestRoad(EditorGeometry::Box2D box, EditorGeometry::Point2D point);

        std::vector<DataFeature*>   PolygonSelect(const std::string& region, bool isIntersectIncluded);
		
		std::vector<DataFeature*>   SimlePolygonSelect(EditorGeometry::Box2D box, DATALAYER_TYPE layer_type);
    };

    class EDITOR_API DataSource
    {
     protected:
        friend class DataManager;

        DataSource();

        std::vector<DataLayer*>     m_pDataLayers;

        std::string                 m_ConnectString;

        sqlite3*                    m_Sqlite3Db;

        void*                       m_cache;
        
        DATASOURCE_TYPE             m_dataSourceType;
        
        virtual void                InitializeDataLayer() = 0;

    public:

        virtual ~DataSource();

        virtual DataLayer*                  GetLayerByType(DATALAYER_TYPE type);

        virtual DataLayer*                  GetLayerByName(const std::string& name);

        virtual int                         Open(std::string& file);

        virtual int                         Close(std::string& file);

        virtual int                         BeginTranscation();

        virtual int                         EndTranscation();

        virtual int                         RollbackTranscation();

        virtual std::string                 GetConnectionString();
        
        virtual int                         InitDataBaseToWal(std::string sPath, bool isWal = true);
        
        virtual int                         InstallLayers(std::string sDownloadSqlite, std::string sExistsSqlite, std::string layerName, ProgressEventer* progress);
        
        virtual int GetDataSourceType();
        
        std::string                         GetDataSourcePath();

    protected:

        sqlite3*	    openSqlite(std::string path);

        void			closeSqlite(sqlite3* db);

        std::string		GenerateSql(sqlite3* db, sqlite3_stmt* stmt, std::string layerName);

        int				IsExistsInDatabase(sqlite3* db, std::string tableName, std::string key, std::string keyID);

        int				BindData(sqlite3_stmt* stmt_exist, sqlite3_stmt* stmt_download);

        /** 
        * @brief 判断数据表中是否存在某字段
        * @param db 数据库
        * @param sColumn 字段名
        * @param sTable 数据表名
        * @return -1表示失败，0表示不存在，1表示存在
        */
        int                             IsHaveColumnInTable(sqlite3* db, const std::string& sColumn, const std::string& sTable);
        bool                            isWithinGrid(std::string pointWkt, std::string wkt);
    };
    
    //coremap datasource
    class CoreMapDataSource:public DataSource
    {
    protected:
        
        friend class DataManager;
        
        CoreMapDataSource();
        
        virtual void InitializeDataLayer();
        
    public:
        
        virtual ~CoreMapDataSource();
        
        virtual int                         InstallPoi(std::string sFilePath, std::string sFlag, ProgressEventer* progress);
        
        virtual int                         InstallInfo(std::string sFilePath, std::string sFlag, ProgressEventer* progress);
        
        virtual int                         InstallTips(std::string sJsonPath, const std::vector<std::string>& grids, ProgressEventer* progress);
        
        virtual int                         InstallTipsEx(std::string sJsonPath, const std::vector<std::string>& grids, ProgressEventer* progress);
        
        virtual int                         InstallPatternImage(std::string sFilePath, std::string sFlag, ProgressEventer* progress);
        
        virtual std::string                 PackageTips(std::string userId, std::string wkt, std::string timestamp, int& dataCount, std::set<std::string>& failedFeedbacks, ProgressEventer* progress);
        
        virtual int                         InsertGpsLineTip(const std::string& tipJson);
        
        virtual std::string                 GetCoremapModelVersion(int type);
    
        
    private:
        
        Model::Tips                     IsTipsExistInDatabase(sqlite3* db, std::string tableName, std::string key, std::string keyID);
        
        int 			                PackageTips(std::ofstream* out, sqlite3_stmt* stmt, const char* outPath, std::set<std::string>& failedFeedbacks);
        
        int                             PackageTipsFromTip(std::ofstream* out, sqlite3* db, std::string wkt, const char* outPath, std::set<std::string>& failedFeedbacks);
        
        void			                PackageTipsPhotos(std::string content, const char* outPath, std::set<std::string>& failedFeedbacks);
        
        int                             PackageTipsFromGPSLine(std::ofstream* out, sqlite3* db, std::string timestamp, const char* outPath, std::set<std::string>& failedFeedbacks);
        
        int                             PackagePatternImage(std::ofstream* out, sqlite3* db);
        
        std::set<std::string>           SelectRowkeyFromEditGPSLine(sqlite3* db, std::string wkt);
    };
    
    //gdb datasource
    class GdbDataSource:public DataSource
    {
   protected:
        
        friend class DataManager;
        
        GdbDataSource();
        
        virtual void InitializeDataLayer();
        
    public:
        
        virtual ~GdbDataSource();
        
        virtual int                     InstallGDB(std::string sFilePath, std::string sFlag, ProgressEventer* progress);
        
        int                             InstallGDBFromSqlite(std::string sFilePath, std::string sFlag, ProgressEventer* progress);
        
        int                             InstallGDBFromJson(std::string sFilePath, std::string sFlag, ProgressEventer* progress);
    
    };

    //Project datasource
    class ProjectDataSource:public DataSource
    {
    protected:
        
        friend class DataManager;
        
        ProjectDataSource();
        
        virtual void InitializeDataLayer();
        
    public:
        
        virtual ~ProjectDataSource();
    };
    
    class FieldDefines
    {
        friend class DataLayer;

        struct Field
        {
            std::string _Name;

            FIELD_TYPE _Type;
        };

        std::vector<Field>          m_vFields;

        std::map<std::string, int>  m_mColumns;

        FieldDefines();

    public:
        ~FieldDefines();

        void                        SetColumnCount(unsigned int count);

        unsigned int                GetColumnCount();

        void                        SetColumnType(unsigned int index, int data_type);

        int                         GetColumnType(unsigned int index);

        void                        SetColumnName(unsigned int index, const std::string& col_name);

        std::string                 GetColumnName(unsigned int index);

        int                         GetColumnIndex(const std::string& col_name);
    };

    class EDITOR_API DataLayer
    {
    private:
        friend class DataSource;
        friend class CoreMapDataSource;
        friend class GdbDataSource;
        friend class ProjectDataSource;
        
        DATALAYER_TYPE              m_LayerType;

        sqlite3*                    m_Sqlite3Db;

        FieldDefines*               m_pFieldDefines;

        sqlite3_stmt*               m_stmt;

        DataLayer();

        std::vector<std::string>    CheckResultList;

		void                        SetDBConnection(sqlite3* db);

    public:

        ~DataLayer();

        void                        SetDataLayerType(DATALAYER_TYPE type);

        int			                GetDataLayerType();

        int                         ResetReading();

        int                         ResetReadingBySpatial(EditorGeometry::Box2D box);

        int                         ResetReadingBySQL(std::string sql);	

        DataFeature*                GetNextFeature();

        DataFeature*                GetFeatureByRowId(unsigned int rowId);

        int                         BeginTranscation();

        int                         InsertFeature(DataFeature* feature);

        int                         UpdateFeature(DataFeature* feature);

        int                         DeleteFeature(DataFeature* feature);

        int                         EndTranscation();

        int                         RollbackTranscation();

        std::string                 GetTableName();

        int                         GetMaxIntPrimaryKey(std::string primaryKeyName);

		FieldDefines*               GetFieldDefines();

        int                         ExcuteSql(const std::string& sql);
    private:
        FieldDefines*               GetTableDefines(sqlite3* db, const std::string& tablename);

        FieldDefines*               ParseSql(const std::string& sql);
    };

    class  DataCheckReport
    {
    public:
        DataCheckReport();

        ~ DataCheckReport();

    };

    class EDITOR_API DataFeatureFactory
    {
    private:
        DataFeatureFactory();

        virtual ~DataFeatureFactory();

    public:
        static const DataFeatureFactory*    getDefaultInstance();

        DataFeature*				        CreateFeature(DataLayer* layer) const;

        void						        DestroyFeature(DataFeature* feature) const;
    };

    class EDITOR_API DataFeature
    {
    private:

        FieldDefines*			            m_pFieldDefines;

        sqlite3_stmt*			            m_stmt;

        std::map<int, int>		            m_ChangeCols;

        unsigned char*			            m_ChangeVals;

        int						            m_ChangeValsLength;

        bool                                m_bIsStatementOwner;

        std::vector<void*>		            m_UnfreedMemory;

        static int                          m_refCount;

        DataLayer*			                m_layer;

    public:
        DataFeature();

        virtual ~DataFeature();      

    public:
        virtual int				            InnerCheck() = 0;

        virtual JSON                        GetSnapshotPart();

        virtual JSON                        GetEditHistoryPart() = 0;

        virtual JSON                        GetTotalPart();

        virtual int                         SetTotalPart(JSON json);

        int                                 GetCheckReportCount() ;

        DataCheckReport                     GetCheckReport(unsigned int index);

        void					            Initialize(sqlite3_stmt* stmt, bool isStatementOwner, DataLayer* layer);

        int                                 SyncDataToMemory();

        unsigned int			            GetRowId();

        int						            SetRowId(unsigned int rowId);

        int						            GetAsInteger(unsigned int index);

        double					            GetAsDouble(unsigned int index);

        std::string				            GetAsString(unsigned int index);

        unsigned char*                      GetAsBlob(unsigned int index, unsigned int& length);

        EditorGeometry::WkbGeometry*	    GetAsWkb(unsigned int index);

        JSON                                GetAsJson(unsigned int index);  

        int					                SetAsInteger(unsigned int index, int value);

        int					                SetAsDouble(unsigned int index, double value);

        int						            SetAsString(unsigned int index, const std::string& value);

        int						            SetAsBlob(unsigned int index, unsigned char* blob, unsigned int length);

        int						            SetAsWkb(unsigned int index, EditorGeometry::WkbGeometry* wkb);

        int						            SetAsJson(unsigned int index, JSON json);

        virtual FEATURE_GEOTYPE             GetGeoType(int type){return GEO_POINT;}

        FieldDefines*                       GetFieldDefines();

        DataLayer*                          GetDataLayer();
        
        virtual std::string                 ToUploadJsonStr(){return "";}
    };

    class EDITOR_API JSON
    {
    private:
        std::string    m_JsonString;

    public:
        JSON(const std::string& str)
        {
            this->m_JsonString = str;
        };

        JSON()        {};

        ~JSON()       {};

        std::string    GetJsonString()
        {
            return this->m_JsonString;
        }

        void           SetJsonString(const std::string& json_string)
        {
            this->m_JsonString = json_string;
        }
    };

    class EDITOR_API ClassificationSelector
    {
    public:
        int GetCountByCondition(std::string sTable, int type);

        std::vector<std::string> GetSignalLayerByCondition(std::string sTable, int type, int pagesize, int offset);

        std::string GenerateSqlFromTableName(std::string sTable);

        std::string GetTablenameAndColumn(std::string sTable);
    };
 class EDITOR_API Matcher
    {
    public:
        Matcher();

        ~Matcher();
    public:
        int CaclReliabilityFirst(Status* pStatus, Result* result, std::vector<std::pair<int, EditorGeometry::WkbGeometry*> > features, EditorGeometry::Point2D point);

        int CaclReliabilitySecond(Status* pStatus, Result* result, double dir, std::vector<std::pair<int, EditorGeometry::WkbGeometry*> > features, EditorGeometry::Point2D point);

        int UpdateStatus(std::pair<double, double> pedal, std::pair<int, EditorGeometry::WkbGeometry*> feature, Status* pStatus);

        double CaclNearDist(std::pair<double, double> pedalPoint, EditorGeometry::WkbGeometry* wkb, Status* pStatus);
    private:
        
    };

    class EDITOR_API LocationManager
    {
    private:
        LocationManager();

    public:
        static LocationManager* GetInstance();

        ~LocationManager();

        int ParseInput(const std::string& sInput);

        int Filte();
        
        int Match();

        std::string ToJson();

        void ResetFeatures();

    private:
        static LocationManager* pInstance;

        double m_nLatitude;

        double m_nLongitude;

        double m_nAltitude;

        double m_nSpeed;

        double m_nDir;

        std::string m_sTime;

        int m_nId;

        Filter* m_pFilter;

        Matcher* m_pMatcher;

        Result* m_pResult;

        Status* m_pStatus;

        std::vector<std::pair<int, EditorGeometry::WkbGeometry*> > m_vReatures;
    };

    class EDITOR_API Filter
    {
    public:
        Filter();

        ~Filter();

        int FilterSpeedBadPoint(double speed);

        int FilterDistBadPoint(double lat, double lon, std::vector<std::pair<int, EditorGeometry::WkbGeometry*> >& vec);
        
		int FilterDistBadPoint2(double lat, double lon, std::vector<std::pair<int, EditorGeometry::WkbGeometry*> >& vec);
    private:
        int OpenSqlite();
		
    private:
        sqlite3* m_db;
        void* m_cache;
    };

    class EDITOR_API Reliability
    {
    public:
        Reliability();

        ~Reliability();

        double CaclByDist(double dist);

        double CaclByDir(double dir, EditorGeometry::WkbGeometry* line, EditorGeometry::Point2D point);
    };

    class EDITOR_API Result
    {
    public:
        Result();
        
        ~Result();

        std::string ToJson();

        void    Reset();

    public:
        int m_isMatchedLine;

        int m_nId;

        double m_nMatchConfidence;

        int m_nMatchType;

        int m_nMatchObjectId;

        std::string m_sMatchLocation;
    };

    class EDITOR_API Status
    {
    public:
        Status();

        ~Status();

        void SetMatchedPoint(double lon, double lat);

        std::pair<double, double> GetCurMatchedPoint();

        std::pair<double, double> GetPreMatchedPoint();

        void SetMatechedLine(EditorGeometry::WkbGeometry* geo);

        EditorGeometry::WkbGeometry* GetMatchedLine();

        int CaclDistToEnd();

    public:
        bool m_isNearEnd;

        int m_nMatchedLineId;
    private:

        std::vector<std::pair<double, double> >* m_pMatchedPoint;

        EditorGeometry::WkbGeometry* m_pGeo;

        int nIterator;
    };

    class EDITOR_API DataFunctor
    {
    private:
        friend class DataManager;
        DataFunctor();

        bool                                      m_pIsDbOpen;
    public:
        ~DataFunctor();
    public:
        std::vector<geos::geom::Geometry*>        GetGeometries(const std::vector<std::string>& ids, DATALAYER_TYPE layerType);
        
        /**
         * @brief 获得link或测线几何
         * @param id
         * @param layerType 终点
         * @return 线几何
         */
        geos::geom::Geometry*                     GetLineGeometry(const std::string& id, DATALAYER_TYPE layerType);
        
        EditorGeometry::WkbGeometry* 			  GetGeoFromId(std::string id, int& dir);
        
		EditorGeometry::WkbGeometry*              GetGeoByRowkey(std::string rowkey);

        std::vector<DataFeature*>                 GetGpsTipsByGeoUuids(const std::vector<std::string>& uuids);
        
        std::vector<Editor::DataFeature*>         QueryFeatures(const std::string& tableName, const std::string& searchQuery);
        
        std::vector<Editor::DataFeature*>         QueryFeaturesBySql(const std::string& tableName, const std::string& sql);
        
        //General Insert/Update/Delete
        int                                       InsertFeature(const std::string& tableName, const std::string& json);
        
        int                                       UpdateFeature(const std::string& tableName, const std::string& json);
        
        int                                       DeleteFeature(const std::string& tableName, int rowId);
    };
}

#endif
