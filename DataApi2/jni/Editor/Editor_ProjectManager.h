#ifndef FM_SDK_Editor_Manager_h
#define FM_SDK_Editor_Manager_h

#include "Editor.h"
#include "Model.h"
#include "Locker.h"
#include "geos.h"
#include "geos/operation/union/CascadedPolygonUnion.h"

const int GDBVersion = 2;
const int TIPSVersion = 1;
const int IMAGEVersion = 3;
namespace Editor 
{
    typedef enum INSTALL_TYPE
    {
         POI = 0,

         INFO = 1,

         GDB = 2,

         TIPS = 3,

         PatternImage = 4,
         
    }INSTALL_TYPE;
    
    typedef enum PACKAGE_TYPE
    {
        PACKAGE_POI = 0,
        
        PACKAGE_INFO = 1,
        
        PACKAGE_TIPS = 2,
        
    }PACKAGE_TYPE;
    
    typedef enum GRID_DATE
    {
        TIPS_DOWN = 0,
        
        TIPS_UP = 1,
        
    }GRID_DATE;

    enum PatternImage_Mode
    {
        PatternImage_Upload = 0,

        PatternImage_Download =1
    };

    struct UploadMsg
    {
    	UploadMsg()
    	{
    		serverMsg="";
    		filename="";
    		path="";
    		time="";
    		conncode = -2;
    		total = 0;
    		add = 0;
    		upd = 0;
    		del = 0;
    	}

    	std::string serverMsg;
    	std::string filename;
    	std::string path;
    	std::string time;

    	int conncode;
    	int total;
    	int add;
    	int upd;
    	int del;
    };
    class ProjectManager;

    class MapDataInstaller;

    class MapDataPackager;

    class Project;

    class Task;
    
    class Grid;

    class EDITOR_API ProjectManager
    {
    private:
        
        ProjectManager();
        
    public:
        
        static ProjectManager* getInstance();
        
        ~ProjectManager();
        
    public:
        
       void                     SetCurrentUser(const char* userId);

       MapDataInstaller*        getMapDataInstaller();

       MapDataPackager*         getMapDataPackager();

       void                     CreateDatabase();

       sqlite3*                 GetDatabase();

    public:
        std::vector<std::string>    GetProjectInfo(const std::string& sToken, const std::string& sUser);

        std::string                 GenerateParam(const std::string& sUser);

        bool                        ParseProjectInfoResponseJson(const std::string& sJson, const std::string& sUser, std::vector<std::string>& vResult);

        std::vector<std::string>    GetProjectDetails(const std::string sToken, const std::string& sProjectId);

        std::string                 GenerateProjectDetailsParam(const std::string& sProjectId);

        bool                        ParseProjectDetailsResponse(const std::string& sJson, std::vector<std::string>& vResult);

        std::vector<std::string>    GetTaskDetails(const std::string& sToken, const std::string& sProjectId, const std::string& sUserId);

        std::string                 GenerateTaskParam(const std::string& sProjectId, const std::string& sUserId);

        bool                        ParseTaskDetailsResponse(const std::string& sJson, const std::string& sUser,std::vector<std::string>& vResult);

        std::string                 GetVersion();

        bool                        CheckVersion(const std::string& version, int type);
        
		std::string                 GetModelVerisonFromServer(int type);
        
		bool                        MatchVersion(int type);
        std::string                 GeneratePreparedJson(int code, const std::string& errmsg);

        std::string                 GeneratePreparedJson(UploadMsg msg);
    private:
       bool                     ExecuteNonQuery(const char* sql, sqlite3* db);

       int                      CreateDatabaseTable(std::string sPath);

       bool                     IsExistInDatabase(const std::string& sTablename, const std::string& sConditon);

    private:
        static ProjectManager*  m_pInstance;

        MapDataInstaller*       m_pMapDataInstaller;

        MapDataPackager*        m_pMapDataPackager;

        sqlite3*                m_db;
            
        std::string             m_sUserId;

        DataFileSystem*         m_pDataFileSystem;
    };  

    class EDITOR_API MapDataInstaller
    {
    private:
        friend class ProjectManager;

        MapDataInstaller();

    public:
        ~MapDataInstaller();

        void             AddProjectId(const char* projectId);

        void             ClearProjectIds();

        void             AddInstallType(int type);

        void             ClearInstallTypes();

        void             Run(const std::string& sToken, Job::Cmd cmd, ProgressEventer* progress);

        void             Stop();

        void             CallBack(Editor::Job* job);

        void             RunTips(const std::vector<int>& grids, const std::string& timeString, ProgressEventer* progress);
    public:
        std::string      GetGridFlag();

        int             GetPUFromDatabase(std::string sProjectId, int nType, Model::ProjectUser* PU);
        std::string     GetDownloadPrepared(const std::string sToken, const std::string sProjectId, const std::string sUserId, const std::string time, int type);
        void            DownloadFile(const std::string& inputJson, ProgressEventer* progress);
        
    private:
        int                     POICallBack(Editor::Job* job);

        int                     GDBCallBack(Editor::Job* job);

        int                     INFOCallBack(Editor::Job* job);

        int                     TipsCallBack(Editor::Job* job);

        int                     PatternImageCallBack(Editor::Job* job);

        int                     DownloadPoi(const std::string& sToken, Job::Cmd cmd, ProgressEventer* progress, int nIdx);

        int                     DownloadInfo(const std::string& sToken, Job::Cmd cmd, ProgressEventer* progress, int nIdx);

        int                     DownloadGdb(const std::string& sToken, Job::Cmd cmd, ProgressEventer* progress, int nIdx);

        int                     DownloadTips(Job::Cmd cmd, ProgressEventer* progress, int nIdx);

        int                     DownloadPatternImage(Job::Cmd cmd, ProgressEventer* progress, int nIdx);

        int                     RequestServerToGenerateData(const std::string& type, const std::string& sUrl, std::string& sUrlTmp, std::string& sDateTmp, std::string& sMd5);

        int                     RequestServerToGenerateInfor(const std::string& sUrl, const std::string& para, std::string& sUrlDest, std::string& sDate, std::string& sMd5);

        std::string             GenerateDownloadInforpara(const std::string& projectId, const std::string& date);

        int                     RequestServerToGenerateTips(const std::string& sUrl, std::string& actualUrl);

        int                     RequestServerToGeneratePatternImage(const std::string& sUrl, std::string& actualUrl);

        std::string             GetUrl(Model::ProjectUser* modelPU, bool isIncremental = false);

        void                    SetCurrentUserId(const char* userId);

        std::string             GetFilenameFromDir(std::string sIn);

        void                    SetFileSystem(DataFileSystem* pFileSystem);


        std::string             GetTipsDownloadUrl(const std::vector<int>& grids, const std::string& timeString);
        

    private:
        DataDownloader*                     m_pDataDownloader;

        std::vector<std::string>            m_vProjectId;

        std::vector<int>                    m_vType;

        std::string                         m_sUserId;
        
        std::map<std::string, Model::ProjectUser>  m_mPdPu;

        std::string                         m_sDirectory;

        DataFileSystem*                     m_pDataFileSystem;
    };

    class EDITOR_API MapDataPackager
    {
    private:
        friend class ProjectManager;

        MapDataPackager();

    public:
        ~MapDataPackager();

        void             AddProjectId(const char* projectId);

        void             ClearProjectIds();

        void             AddPackageType(int type);
        
        void             ClearPackageTypes();
        
        void			 AddGridId(const char* gridId);
        
        void             ClearGridIds();

        void             Run(std::string sToken, JOBCMD cmd, ProgressEventer* progress, const std::string& deviceId);

        void             Stop();
        
        void			 ConfirmUploadTips(UploadJob* job);

        void             CallBack(Editor::Job* job);

        void             FailedCallBack(Editor::Job* job);

    public:

        bool            UploadPoiPhotoToLocal(const std::string& sProject, const std::string& sFid, std::string& source);

        void            UploadPoiPhotoToServer(ProgressEventer* progress);
		
        std::string     GetGridFlag();

        std::string     GetUploadPrepared(const std::string& sToken, const std::string sProjectId, const std::string& sDeviceId, int type);
        void            UploadFile(const std::string& input, ProgressEventer* progress);
    private:

        void					PoiCallBack(Editor::Job* job);

        void                    InforCallBack(Editor::Job* job);

        std::string             GetUrl(Model::ProjectUser* modelPU, std::string filename);

        void                    SetCurrentUserId(const char* userId);

        void                    SetFileSystem(DataFileSystem* pFileSystem);

        std::string             GetUploadFile(std::string& sFilename, const std::string sLastTime, std::string& errmsg, const std::string& sProjectId, int& total, int& add, int& upd, int& del);

        std::string             GetUploadInfoFile(std::string& sFilename, std::string& errmsg, const std::string& sPackageSql);

        std::string             GenerateUploadFile(std::string& sFilename, const std::string sLastTime, std::string& errmsg, const std::string& sProjectId, int& total, int& add, int& upd, int& del);

        std::string             GenerateUploadFile(std::string& sFilename, std::string& errmsg, const std::string& packageSql);

        void                    GnerateUploadPhoto(const std::string& sJosn);

        int                     GetPUFromDatabase(std::string sProjectId, int nType, Model::ProjectUser* PU);

        bool                    UpdatePhoto(const std::string& sFid, std::string sValue);

        std::string             GeneratePhotoName(const std::string& source);
        
        void					RunTips(JOBCMD cmd, ProgressEventer* progress);
        
        void					RunPoi(std::string sToken, JOBCMD cmd, ProgressEventer* progress, int nIdx, const std::string& deviceId);

        void                    RunInfor(std::string sToken, JOBCMD cmd, ProgressEventer* progress, int nIdx);

        std::string				GetTipsUploadStartParameter(const char* target, const char* fileName, unsigned long& fileSize, int& chunkSize, int& chunkCount);
        
        int 					ParseTipsUploadStartResponse(std::string response, int& jobId, std::string& errmsg);
        
        std::string				GetTipsUploadChunkParameter(int jobId, int chunkNo);
        
        std::string				GetTipsUploadCheckParameter(int jobId);
        
        int                     ParseTipsUploadCheckResponse(std::string response, std::set<int>& finishedChunkNo, std::string& errmsg);
        
        int 					ParseTipsUploadFinishResponse(std::string response, std::string& errmsg);
        
        void					AddUploadFileTryCount(const char* fileName, int& lastTryCount);

        int                     ParsePoiUploadResponse(std::string sIn, std::string& errmsg);

        std::string             GetFailedReasonByType(int type);
        
        int                     CheckLastUploadIsCorrect();

        bool                    UploadSnapShot(const char* target, const char* fileName, const char* token, unsigned long& fileSize, int& chunkCount);

        bool                    UploadCheck(const char* token, const char* sFilename);

        bool                    SaveToMainDBAfterUpload(Editor::Job* job, const std::string type);
        
        bool                    UpdateDatabaseAfterTipsUpload(std::string wkt);

        bool                    UpdateTstatus(sqlite3* db, std::set<std::string> setUploadTips, const std::string& sql);
        
		bool                    UpdateDatabaseAfterImageUpload();

		std::string				ParseTipsImportResponse(const std::string& sResponse);
        std::string             GernerateGetUploadTimePara(const std::string& sToken, const std::string& sProjectId, const std::string deviceId);
    private:
        DataUploader*                       m_pDataUploader;

        std::vector<std::string>            m_vProjectId;

        std::vector<int>                    m_vType;
        
        std::vector<std::string>            m_vGridId;

        std::string                         m_sUserId;

        std::map<std::string, Model::ProjectUser>  m_mPdPu;

        DataFileSystem*                     m_pDataFileSystem;
        
        int									m_MaxChunkSize;
        
        int									m_MaxUploadFileTryCount;
        
        std::map<std::string, int>			m_UploadFileTryCount;
        
        boost::mutex						m_Mutex;

        ProgressEventer* 					m_Progress;

        unsigned int                        m_nChunkNum;

        unsigned int                        m_nSuccessChunkNum;
    };

    class EDITOR_API ProjectUser
    {
    public:
        std::string sProjectId;
        std::string sUserId;
        int         nType;
        std::string download_latest;
        std::string download_url;
        std::string upload_latest;
        std::string upload_url;
        std::string zip_file;
        std::string failed_reason;
        std::string details;

    public:
        ProjectUser();
        std::vector<std::string>    GetProjectFromUser(const std::string& sUser);
        int                         ParseJson(rapidjson::Value& json, const std::string sUser);
        int                         ParseSqlite(sqlite3_stmt *statement);
        int                         InsertSqliteTable();
        int                         InsertOrUpdateSqliteTable();
        JSON                        ToJson();
    };

    class EDITOR_API ProjectInfo
    {
    public:
        std::string ProjectId;

        std::string ProjectName;

        std::string ProjectRegion;

        std::string ProjectType;

        std::string ProjectDetails;

    public:
        ProjectInfo();
        std::vector<std::string>    GetProjectInfo(const std::string& sProjectId);
        bool                        ParseJson(rapidjson::Value& json);
        int                         ParseSqlite(sqlite3_stmt *statement);
        int                         InsertSqliteTable();
        int                         UpdateSqliteTable();
        JSON                        ToJson();
    };

    class EDITOR_API Task
    {
    public:

        std::string TaskId;

        std::string TaskRegion;

        std::string TaskType;

        std::string TaskPriority;

        std::string TaskStatus;

        std::string ProjectId;
        
        std::string UserId;

        std::string UserExcuteId;

        std::string TaskDetails;

    public:
        Task();
        std::vector<std::string>    GetTaskDetails(const std::string& sProjectId, const std::string& sUser);
        int                         ParseJson(rapidjson::Value& json, const std::string sUser);
        int                         ParseSqlite(sqlite3_stmt *statement);
        int                         InsertSqliteTable();
        int                         UpdateSqliteTable();
        JSON                        ToJson();

    };

    class EDITOR_API Grid
	{
    public:
    	EditorGeometry::Box2D GridToLocation(std::string gridId);

    	std::string GridsToLocation(std::vector<std::string> gridIds);

    	std::string LocationToGrid(double dLongitude, double dLatitude);

		/**
		* @bref 计算box范围内grid id 列表
		* @para dminLon box最小经度
		* @para dmaxLon box最大经度
		* @para dminLat box最小纬度
		* @para dmaxLat box最大纬度
		* @return box范围内grid id 列表
		*/
		std::vector<std::string> CalculateGridsByBox(double dminLon, double dmaxLon, double dminLat, double dmaxLat);


    private:
    	EditorGeometry::Box2D CalcMeshBound(std::string meshId, double& centerLon, double& centerLat);

    	std::string MeshLocator_25T(double dLongitude, double dLatitude);

    private:
    	double SecondToDegree(double second);

		int IsAt25TMeshBorder(double dLongitude, double dLatitude);

		int CalculateIdealRowIndex(double dLatitude, int& remainder);

		int CalculateIdealColumnIndex(double dLongitude, int& remainder);

		int CalculateRealRowIndex(double dLatitude, int& remainder);

		int CalculateRealColumnIndex(double dLongitude, int& remainder);
	};

    class EDITOR_API GridHelper
    {
    private:
        
        static GridHelper*     m_pInstance;

        sqlite3*               m_db;

        DataFileSystem*        m_pDataFileSystem;

        std::string            m_pConnectString;

		void* m_cache;
		
		GridHelper();

        int                    CreateTable(std::string sPath);

       // void                   Close();

    public:
        
        static GridHelper*     getInstance();
        
        ~GridHelper();

        void                   SetDbConnection(sqlite3* db);

        int                    CreateOrOpenDatabase();

		void                   Close();

        std::string            GetGridsTimeStamp(const std::vector<std::string>& grids, int type);
        
        int                    RefreshGridsTimeStamp(const std::vector<std::string>& grids, const std::string& timeStamp, int type = 0);
        
        ///lee add
        /*
        * @brief UpdateGridStatus()
        *
        * @para grids
        * @return int :0 succeed, -1 failure
        */
        int UpdateGridStatus(const std::vector<std::string>& grids);

		/*
        * @brief UpdataGridStatus()
        *
        * @para grids
		* @para reStatus 状态值
        * @return bool :true 成功,false 失败
        */
		bool GetGridStatus(const std::string& grid, int& reStatus);

		/*
        * @brief GetUploadGridFlag()
        *
        * @para grid           id
		* @para bhas           本地是否有这条grid记录
		* @para falg          判断是否需要上传的标识
        * @return bool :true 成功,false 失败
        */
		bool GetUploadGridFlag(const std::string& grid, bool bHas, bool& flag);

		/*
		* @bref 插入表中没有的grid数据
		*
		* @para grid id
		* @return true 表中没有; false 表中有
		*/
		bool HasGrid(const std::string& grid);

        /*
        * @brief IsHasGPSLineCrossGrid()
        *
        * @para wkt           几何圈
        * @return bool :true 成功,false 失败
        */
        bool IsHasGPSLineCrossGrid(const std::string& wkt);

        std::string GetPatternImageTimeStamp(PatternImage_Mode mode);
    };
}

#endif
