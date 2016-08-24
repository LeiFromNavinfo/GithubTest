#include "Editor_ProjectManager.h"
#include "BussinessConstants.h"

namespace Editor
{
    ProjectManager* ProjectManager::m_pInstance = NULL;
    
    ProjectManager::ProjectManager()
    {
        m_pMapDataInstaller = new MapDataInstaller();

        m_pMapDataPackager = new MapDataPackager();

        Editor::DataManager* pDataManager = Editor::DataManager::getInstance();

        m_pDataFileSystem = pDataManager->getFileSystem();

        m_pMapDataInstaller->SetFileSystem(m_pDataFileSystem);

        m_pMapDataPackager->SetFileSystem(m_pDataFileSystem);

        m_db = NULL;
    }

    ProjectManager::~ProjectManager(){}

    ProjectManager* ProjectManager::getInstance()
    {
        if(m_pInstance == NULL)
        {
            m_pInstance = new ProjectManager();
        }

        return m_pInstance;
    }

    void ProjectManager::SetCurrentUser(const char* userId)
    {
        m_sUserId = userId;

        m_pMapDataInstaller->SetCurrentUserId(userId);

        m_pMapDataPackager->SetCurrentUserId(userId);
    }

    MapDataInstaller* ProjectManager::getMapDataInstaller()
    {
        return m_pMapDataInstaller;
    }

    MapDataPackager* ProjectManager::getMapDataPackager()
    {
        return m_pMapDataPackager;
    }

    sqlite3* ProjectManager::GetDatabase()
    {
        if(m_db != NULL)
        {
            return m_db;
        }

        std::string sDatabasePath = m_pDataFileSystem->GetProjectDataPath();
        int ret = sqlite3_open_v2(sDatabasePath.c_str(), &m_db, SQLITE_OPEN_READWRITE, NULL);
        if(ret != SQLITE_OK)
        {
            Logger::LogD("DataSource::sqlite [%s] or [%s] open failed,ret:[%d]", sDatabasePath.c_str(), sqlite3_errmsg(m_db),ret);
            Logger::LogO("DataSource::sqlite [%s] or [%s] open failed,ret:[%d]", sDatabasePath.c_str(), sqlite3_errmsg(m_db),ret);
            sqlite3_close(m_db);
            return NULL;
        }
        return m_db;
    }

    void ProjectManager::CreateDatabase()
    {
        if(m_db != NULL)
        {
            sqlite3_close (m_db);;
        }

        std::string sDatabasePath = m_pDataFileSystem->GetProjectDataPath();

        if(Tools::CheckFilePath(sDatabasePath.c_str()) == false)
        {
            Logger::LogD("DataSource::DIRECTORY [%s] NOT EXISTS", sDatabasePath.c_str());
            Logger::LogO("DataSource::DIRECTORY [%s] NOT EXISTS", sDatabasePath.c_str());

            return ;
        }

        bool isDatabaseExists = false;

#ifdef WIN32            
        if(access(sDatabasePath.c_str(), 00) == 0)
        {
            isDatabaseExists = true;
        }
#else
        if(access(sDatabasePath.c_str(), R_OK) == 0)
        {
            isDatabaseExists = true;
        }
#endif
        if(Tools::CheckFilePath(sDatabasePath.c_str()) == false)
        {
            Logger::LogD("DataSource::sqlite [%s] error", sDatabasePath.c_str());
            Logger::LogO("DataSource::sqlite [%s] error", sDatabasePath.c_str());
        }


        if(isDatabaseExists == false)
        {
            if(CreateDatabaseTable(sDatabasePath) == -1)
            {
                return ;
            }
        }
    }

    int ProjectManager::CreateDatabaseTable(std::string sPath)
    {
        int ret = sqlite3_open(sPath.c_str(), &m_db);

        if(ret != SQLITE_OK)
        {
            Logger::LogD("DataSource::sqlite [%s] or [%s] open failed", sPath.c_str(), sqlite3_errmsg(m_db));
            Logger::LogO("DataSource::sqlite [%s] or [%s] open failed", sPath.c_str(), sqlite3_errmsg(m_db));

            sqlite3_close (m_db);
            return -1;
        }

        std::string sProjectUser = "create table project_user ("
			"project_id integer,"
			"user_id integer,"
			"type integer,"
			"download_latest text,"
			"download_url text,"
			"upload_latest text,"
            "upload_url text,"
        	"zip_file text,"
        	"failed_reason text,"
            "details text)";

        std::string sProjectInfo = "create table project_info ("
			"project_id integer PRIMARY KEY,"
			"project_region blob,"
			"project_name text,"
            "project_type text,"
            "project_details text)";

        std::string sTaskInfo = "create table task_info ("
			"task_id integer PRIMARY KEY,"
			"task_region text,"
            "task_type text,"
            "task_priority text,"
            "task_status text,"
            "project_id integer,"
            "user_id integer,"
            "user_excute_id integer,"
            "task_details text)";

        std::string sVersion = "create table Version (version text)";

        std::string sVersionSql = "insert into Version values('1.0')";

        if(ExecuteNonQuery(sProjectUser.c_str(), m_db) == false)
        {
            return -1;
        }

        if(ExecuteNonQuery(sProjectInfo.c_str(), m_db) == false)
        {
            return -1;
        }

        if(ExecuteNonQuery(sTaskInfo.c_str(), m_db) == false)
        {
            return -1;
        }

        if(ExecuteNonQuery(sVersion.c_str(), m_db) == false)
        {
            return -1;
        }

        if(ExecuteNonQuery(sVersionSql.c_str(), m_db) == false)
        {
            return -1;
        }

        return 0;
    }

    bool ProjectManager::ExecuteNonQuery(const char* sql, sqlite3* db)
    {
        char* pszErrMsg;

        int rc = sqlite3_exec(db, sql, NULL, NULL, &pszErrMsg);

        if (rc != SQLITE_OK)
        {
		    Logger::LogD("ExecuteNoQuery [%s] failed [%s]", sql, sqlite3_errmsg(db));
		    Logger::LogO("ExecuteNoQuery [%s] failed [%s]", sql, sqlite3_errmsg(db));

            sqlite3_free (pszErrMsg);

            return false;
        }

        return true;
    }

    bool ProjectManager::IsExistInDatabase(const std::string& sTablename, const std::string& sConditon)
    {
        Editor::DataFunctor* df = Editor::DataManager::getInstance()->getDataFunctor();
        
        std::string sql = "select count(1) from " + sTablename + " where " + sConditon;

        std::vector<Editor::DataFeature*> features = df->QueryFeaturesBySql(sTablename, sql);
        
        int countIndex =0;
        
        if(features.size()>0)
        {
            int count = features.at(0)->GetAsInteger(countIndex);
            
            if(count>0){
                return true;
            }
        }
        
        return false;
    }

    std::vector<std::string> ProjectManager::GetProjectInfo(const std::string& sToken, const std::string& sUser)
    {
        std::vector<std::string> vResult;

        if(sUser.empty())
        {
            return vResult;
        }

        std::string url = Editor::DataManager::getInstance()->getServiceAddress()->GetProjectInfomationUrl(sToken) + GenerateParam(sUser);

        std::string sResponse;

        if(Tools::HttpGet(url.c_str(), sResponse) == false)
        {
            Editor::ProjectUser PU;

            vResult = PU.GetProjectFromUser(sUser);

            return vResult;
        }

        if(ParseProjectInfoResponseJson(sResponse, sUser, vResult) == false)
        {
            vResult.clear();

            return vResult;
        }

        return vResult;
    }

    bool ProjectManager::ParseProjectInfoResponseJson(const std::string& sJson, const std::string& sUser, std::vector<std::string>& vResult)
    {
        rapidjson::Document document;

        document.Parse<0>(sJson.c_str());

        if(document.HasParseError())
        {
            Logger::LogD("ProjectManager parse json : format is uncorrect");
            Logger::LogO("ProjectManager parse json : format is uncorrect");

            return false;
        }

        if(document.HasMember("data"))
        {
            rapidjson::Value& value = document["data"];

            if(value.IsNull())
            {
                Logger::LogD("ProjectManager parse json : data is null");
                Logger::LogO("ProjectManager parse json : data is null");

                return false;
            }

            if(value.HasMember("rows"))
            {
                rapidjson::Value& arrayValue = value["rows"];

                if(!arrayValue.IsArray())
                {
                    Logger::LogD("ProjectManager parse json : rows is not array");
                    Logger::LogO("ProjectManager parse json : rows is not array");

                    return false;
                }

                for(rapidjson::SizeType i = 0; i < arrayValue.Size(); i++)
                {
                    Editor::ProjectUser PU;

                    PU.ParseJson(arrayValue[i], sUser);

                    if(PU.InsertOrUpdateSqliteTable() == -1)
                    {
                        Logger::LogD("ParseProjectInfoResponseJson : UpdateSqliteTable failed");
                        Logger::LogO("ParseProjectInfoResponseJson : UpdateSqliteTable failed");
                        
                        return false;
                    }
                    
                    vResult.push_back(PU.details);
                }
            }
        }

        return true;
    }

    std::string ProjectManager::GenerateParam(const std::string& sUser)
    {
        Document document;
        
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

        Value each_json_key(kStringType);        
        Value each_json_value(kStringType);
        
        document.SetObject();
        
        each_json_key.SetString("pageno");
        each_json_value.SetString("1");
        document.AddMember(each_json_key, each_json_value, allocator);
        
        each_json_key.SetString("pagesize");
        each_json_value.SetString("1000");
        document.AddMember(each_json_key, each_json_value, allocator);

        each_json_key.SetString("snapshot");
        each_json_value.SetString("snapshot");
        document.AddMember(each_json_key, each_json_value, allocator);

        each_json_key.SetString("from");
        each_json_value.SetString("app");
        document.AddMember(each_json_key, each_json_value, allocator);
        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        std::string res = buffer.GetString();
        
        return res;
    }

    std::vector<std::string> ProjectManager::GetProjectDetails(const std::string sToken, const std::string& sProjectId)
    {
        std::vector<std::string> vResult;

        if(sToken.empty() || sProjectId.empty())
        {
            return vResult;
        }

        std::string url = Editor::DataManager::getInstance()->getServiceAddress()->GetProjectDetailsUrl(sToken) + GenerateProjectDetailsParam(sProjectId);

        std::string sResponse;

        if(Tools::HttpGet(url.c_str(), sResponse) == false)
        {
            Editor::ProjectInfo p;

            vResult = p.GetProjectInfo(sProjectId);

            return vResult;
        }

        if(ParseProjectDetailsResponse(sResponse, vResult) == false)
        {
            vResult.clear();

            return vResult;
        }

        return vResult;
    }

    std::string ProjectManager::GenerateProjectDetailsParam(const std::string& sProjectId)
    {
        Document document;
        
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

        Value each_json_key(kStringType);        
        Value each_json_value(kStringType);
        
        document.SetObject();
        
        each_json_key.SetString("projectId");
        each_json_value.SetString(sProjectId.c_str(), sProjectId.size(),allocator);
        document.AddMember(each_json_key, each_json_value, allocator);
        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        std::string res = buffer.GetString();
        
        return res;
    }

    bool ProjectManager::ParseProjectDetailsResponse(const std::string& sJson, std::vector<std::string>& vResult)
    {
        rapidjson::Document document;

        document.Parse<0>(sJson.c_str());

        if(document.HasParseError())
        {
            Logger::LogD("ProjectManager parse json : format is uncorrect");
            Logger::LogO("ProjectManager parse json : format is uncorrect");

            return false;
        }

        if(document.HasMember("data"))
        {
            rapidjson::Value& value = document["data"];

            if(value.IsNull())
            {
                Logger::LogD("ProjectManager parse json : data is null");
                Logger::LogO("ProjectManager parse json : data is null");

                return false;
            }

            Editor::ProjectInfo p;

            p.ParseJson(value);

            std::string sCondition = "project_id=" + p.ProjectId;;

            if(IsExistInDatabase("project_info", sCondition) == true)
            {
                if(p.UpdateSqliteTable() == -1)
                {
                    Logger::LogD("ParseProjectDetailsResponse : UpdateSqliteTable failed");
                    Logger::LogO("ParseProjectDetailsResponse : UpdateSqliteTable failed");

                    return false;
                }
            }
            else
            {
                if(p.InsertSqliteTable() == -1)
                {
                    Logger::LogD("ParseProjectDetailsResponse : InsertSqliteTable failed");
                    Logger::LogO("ParseProjectDetailsResponse : InsertSqliteTable failed");

                    return false;
                }
            }

            vResult.push_back(p.ProjectDetails);
        }
        
        return true;
    }

    std::vector<std::string> ProjectManager::GetTaskDetails(const std::string& sToken, const std::string& sProjectId, const std::string& sUserId)
    {
        std::vector<std::string> vResult;

        if(sToken.empty() || sProjectId.empty() || sUserId.empty())
        {
            return vResult;
        }

        std::string url = Editor::DataManager::getInstance()->getServiceAddress()->GetTaskDetailsUrl(sToken) + GenerateTaskParam(sProjectId, sUserId);

        std::string sResponse;

        if(Tools::HttpGet(url.c_str(), sResponse) == false)
        {
            Editor::Task t;

            vResult = t.GetTaskDetails(sProjectId, sUserId);

            return vResult;
        }

        if(ParseTaskDetailsResponse(sResponse, sUserId, vResult) == false)
        {
            vResult.clear();

            return vResult;
        }

        return vResult;
    }

    std::string ProjectManager::GenerateTaskParam(const std::string& sProjectId, const std::string& sUserId)
    {
        Document document;
        
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

        Value each_json_key(kStringType);        
        Value each_json_value(kStringType);
        
        document.SetObject();
        
        each_json_key.SetString("type");
        each_json_value.SetString("app");
        document.AddMember(each_json_key, each_json_value, allocator);

        each_json_key.SetString("userId");
        each_json_value.SetString(sUserId.c_str(), sUserId.size(),allocator);
        document.AddMember(each_json_key, each_json_value, allocator);

        each_json_key.SetString("projectId");
        each_json_value.SetString(sProjectId.c_str(), sProjectId.size(),allocator);
        document.AddMember(each_json_key, each_json_value, allocator);
        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        std::string res = buffer.GetString();
        
        return res;
    }

    bool ProjectManager::ParseTaskDetailsResponse(const std::string& sJson, const std::string& sUser, std::vector<std::string>& vResult)
    {
        rapidjson::Document document;

        document.Parse<0>(sJson.c_str());

        if(document.HasParseError())
        {
            Logger::LogD("ProjectManager parse json : format is uncorrect");
            Logger::LogO("ProjectManager parse json : format is uncorrect");

            return false;
        }

        if(document.HasMember("data"))
        {
            rapidjson::Value& value = document["data"];

            if(value.IsNull())
            {
                Logger::LogD("ProjectManager parse json : data is null");
                Logger::LogO("ProjectManager parse json : data is null");

                return false;
            }

            if(value.HasMember("rows"))
            {
                rapidjson::Value& arrayValue = value["rows"];

                if(!arrayValue.IsArray())
                {
                    Logger::LogD("ProjectManager parse json : rows is not array");
                    Logger::LogO("ProjectManager parse json : rows is not array");

                    return false;
                }

                for(rapidjson::SizeType i = 0; i < arrayValue.Size(); i++)
                {
                    Editor::Task t;

                    t.ParseJson(arrayValue[i], sUser);

                    std::string sCondition = "task_id=" + t.TaskId;

                    if(IsExistInDatabase("task_info",sCondition) == true)
                    {
                        if(t.UpdateSqliteTable() == -1)
                        {
                            Logger::LogD("ParseProjectInfoResponseJson : UpdateSqliteTable failed");
                            Logger::LogO("ParseProjectInfoResponseJson : UpdateSqliteTable failed");

                            return false;
                        }
                    }
                    else
                    {
                        if(t.InsertSqliteTable() == -1)
                        {
                            Logger::LogD("ParseProjectInfoResponseJson : InsertSqliteTable failed");
                            Logger::LogO("ParseProjectInfoResponseJson : InsertSqliteTable failed");

                            return false;
                        }
                    }

                    vResult.push_back(t.TaskDetails);
                }
            }
        }

        return true;
    }

    std::string ProjectManager::GeneratePreparedJson(int code, const std::string& errmsg)
    {
        Document document;
        
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

        Value each_json_key(kStringType);        
        Value each_json_value(kStringType);
        
        document.SetObject();
        
        each_json_key.SetString("serverMsg");
        each_json_value.SetString(errmsg.c_str(), errmsg.size(),allocator);
        document.AddMember(each_json_key, each_json_value, allocator);

        document.AddMember("conncode", code, allocator);

        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        std::string res = buffer.GetString();
        
        return res;
    }

    std::string ProjectManager::GeneratePreparedJson(UploadMsg msg)
    {
        Document document;
        document.SetObject();
        
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

        Value each_json_value(kStringType);
        
        each_json_value.SetString(msg.serverMsg.c_str(), msg.serverMsg.size(),allocator);
        document.AddMember("serverMsg", each_json_value, allocator);

        document.AddMember("conncode", msg.conncode, allocator);

        each_json_value.SetString(msg.filename.c_str(), msg.filename.size(),allocator);
        document.AddMember("filename", each_json_value, allocator);

        each_json_value.SetString(msg.path.c_str(), msg.path.size(),allocator);
        document.AddMember("url", each_json_value, allocator);

        each_json_value.SetString(msg.time.c_str(), msg.time.size(),allocator);
        document.AddMember("time", each_json_value, allocator);

        document.AddMember("total", msg.total, allocator);

        document.AddMember("add", msg.add, allocator);

        document.AddMember("update", msg.upd, allocator);

        document.AddMember("delete", msg.del, allocator);
        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        std::string res = buffer.GetString();
        
        return res;
    }
    std::string ProjectManager::GetVersion()
    {
        sqlite3* db = GetDatabase();

        if(db == NULL)
        {
            Logger::LogD("ProjectManager::open failed");
            Logger::LogO("ProjectManager::open failed");
            sqlite3_close(db);
            return "";
        }

        std::string sql = "select version from Version";

        sqlite3_stmt* stmt = NULL;

        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

        if(rc != SQLITE_OK)
        {
            Logger::LogD("ProjectManager in GetCoremapModelVersion : failed");
            Logger::LogO("ProjectManager in GetCoremapModelVersion : failed");
            sqlite3_close(db);
            return "";
        }

        std::string detailsTmp = "";

        while(sqlite3_step(stmt) == SQLITE_ROW)
        {
            detailsTmp = (const char*)sqlite3_column_text(stmt, 0);
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return detailsTmp;
    }

    bool ProjectManager::CheckVersion(const std::string& version, int type)
    {
        Editor::DataManager* pDataManager = Editor::DataManager::getInstance();
		
		Editor::CoreMapDataSource* ds = dynamic_cast<Editor::CoreMapDataSource*>(pDataManager->getDataSource(DATASOURCE_COREMAP));
        
        if (NULL==ds)
        {
            return false;
        }
		
        std::string sLocalVersion = ds->GetCoremapModelVersion(type);
		
        if(sLocalVersion != version)
        {
            Logger::LogD("the versio of local is not equel to version of server, please update!!!");
            Logger::LogO("the versio of local is not equel to version of server, please update!!!");
            return false;
        }
        return true;
    }
	
    std::string ProjectManager::GetModelVerisonFromServer(int type)
    {
        std::string sUrl = Editor::DataManager::getInstance()->getServiceAddress()->GetModelVersionUrl(type);
        
		std::string sResult; 
		
        Tools::HttpGet(sUrl.c_str(), sResult);
		
        rapidjson::Document document;
		
        document.Parse<0>(sResult.c_str());
		
        if(document.HasParseError())
        {
            return "";
        }
		
        if(document.HasMember("data"))
        {
            rapidjson::Value& value = document["data"];
			
            if(value.HasMember("specVersion"))
            {
                return value["specVersion"].GetString();
            }
        }
        return "";
    }
    bool ProjectManager::MatchVersion(int type)
    {
        std::string sUrl = Editor::DataManager::getInstance()->getServiceAddress()->GetModelVersionUrl(type);
        
		std::string sVersion = GetModelVerisonFromServer(type);
		
        if(!sVersion.empty())
        {
            if(CheckVersion(sVersion, type) == false)
            {
                return false;
            }
        }
        else
        {
            return false;
        }
        return true;
    }
	
    ProjectUser::ProjectUser()
    {
        nType = 0;
    }

    std::vector<std::string> ProjectUser::GetProjectFromUser(const std::string& sUser)
    {
        std::vector<std::string> vResult;

        Editor::DataFunctor* df = Editor::DataManager::getInstance()->getDataFunctor();
        
        std::string sql = "select details from project_user where user_id=" + sUserId;

        std::vector<Editor::DataFeature*> features = df->QueryFeaturesBySql(TableName_ProjectUser, sql);
        
        int detailsIndex = 0;
        
        for(int i =0; i<features.size();i++)
        {
            std::string detail = features.at(i)->GetAsString(detailsIndex);
            
            vResult.push_back(detail);
            
            delete features.at(i);
        }
        
        return vResult;
    }

    int ProjectUser::InsertSqliteTable()
    {
        JSON json = ToJson();
        
        Editor::DataFunctor* df = Editor::DataManager::getInstance()->getDataFunctor();
        
        int ret = df->InsertFeature(TableName_ProjectUser, json.GetJsonString());
        
        return ret;
    }

    int ProjectUser::InsertOrUpdateSqliteTable()
    {
        Editor::DataFunctor* df = Editor::DataManager::getInstance()->getDataFunctor();

        std::string sql = "select * from "+ TableName_ProjectUser + " where project_id=" + sProjectId+" and user_id="+ sUserId+ " and type="+Tools::NumberToString(nType);
        
        std::vector<DataFeature*> features = df->QueryFeaturesBySql(TableName_ProjectUser, sql);
        
        int ret =0;
        
        if(features.size()>0)
        {
            Editor::DataFeature* targetFeature = features.at(0);
            
            ret = df->UpdateFeature(TableName_ProjectUser, targetFeature->GetTotalPart().GetJsonString());
        }
        else
        {
            ret = df->InsertFeature(TableName_ProjectUser, ToJson().GetJsonString());
        }
        
        for(int i=0;i<features.size();i++)
        {
            delete features.at(i);
        }
        
        return ret;
    }

    int ProjectUser::ParseSqlite(sqlite3_stmt *statement)
    {
        sProjectId = (const char*)sqlite3_column_text(statement, 0);
	    sUserId = (const char*)sqlite3_column_text(statement, 1);
        ProjectUser::nType = sqlite3_column_int(statement, 2);
	    download_latest = (const char*)sqlite3_column_text(statement, 3);
	    download_url = (const char*)sqlite3_column_text(statement, 4);
	    upload_latest = (const char*)sqlite3_column_text(statement, 5);
	    upload_url = (const char*)sqlite3_column_text(statement, 6);
        details = (const char*)sqlite3_column_text(statement, 7);

        return 0;
    }

    int ProjectUser::ParseJson(rapidjson::Value& json, const std::string sUser)
    {
        if(json.HasMember("projectId"))
        {
            sProjectId = Tools::NumberToString<int>(json["projectId"].GetInt());
        }

        sUserId = sUser;

        if(json.HasMember("projectType"))
        {
            int nServerType = json["projectType"].GetInt();
            
            if(nServerType == 1) // 常规poi项目
            {
                nType = 0;
            }
            else if(nServerType == 4) // 情报
            {
                nType = 1;
            }           
        }

        rapidjson::StringBuffer buffer;

        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        json.Accept(writer); 

        details = buffer.GetString();

        return 0;
    }

    JSON ProjectUser::ToJson()
    {
        Document document;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();
        
        Value each_json_value(kStringType);
        std::string each_str_value ;
        
        document.AddMember("project_id",atoi(sProjectId.c_str()),allocator);
        document.AddMember("user_id",atoi(sUserId.c_str()),allocator);
        
        document.AddMember("type",nType,allocator);
        
        each_str_value = download_latest;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("download_latest",each_json_value,allocator);
        
        each_str_value = download_url;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("download_url",each_json_value,allocator);
        
        each_str_value = upload_latest;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("upload_latest",each_json_value,allocator);
        
        each_str_value = upload_url;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("upload_url",each_json_value,allocator);
        
        each_str_value = zip_file;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("zip_file",each_json_value,allocator);
        
        each_str_value = failed_reason;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("failed_reason",each_json_value,allocator);
        
        each_str_value = details;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("details",each_json_value,allocator);
        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<StringBuffer> writer(buffer);
        document.Accept(writer);
        std::string result = buffer.GetString();
        
        JSON json;
        
        json.SetJsonString(result);
        
        return json;
    }
    
    ProjectInfo::ProjectInfo()
    {
    }

    std::vector<std::string> ProjectInfo::GetProjectInfo(const std::string& sProjectId)
    {
        std::vector<std::string> vResult;
        
        Editor::DataFunctor* df = Editor::DataManager::getInstance()->getDataFunctor();
        
        std::string sql = "select project_details from "+TableName_ProjectInfo+" where project_id=" + sProjectId;
        
        std::vector<Editor::DataFeature*> features = df->QueryFeaturesBySql(TableName_ProjectInfo, sql);
        
        int detailsIndex = 0;
        
        for(int i =0; i<features.size();i++)
        {
            std::string detail = features.at(i)->GetAsString(detailsIndex);
            
            vResult.push_back(detail);
            
            delete features.at(i);
        }
        
        return vResult;
    }

    bool ProjectInfo::ParseJson(rapidjson::Value& json)
    {
        if(json.HasMember("projectId"))
        {
            ProjectId = Tools::NumberToString<int>(json["projectId"].GetInt());
        }

        if(json.HasMember("projectName"))
        {
            ProjectName = json["projectName"].GetString();
        }

        if(json.HasMember("projectRegion"))
        {
            ProjectRegion = json["projectRegion"].GetString();
        }

        if(json.HasMember("projectType"))
        {
            ProjectType = Tools::NumberToString<int>(json["projectType"].GetInt());
        }

        rapidjson::StringBuffer buffer;

        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        json.Accept(writer); 

        ProjectDetails = buffer.GetString();

        return 0; 
    }

    int ProjectInfo::InsertSqliteTable()
    {
        Editor::DataFunctor* df = Editor::DataManager::getInstance()->getDataFunctor();
        
        int ret = df->InsertFeature(TableName_ProjectInfo, ToJson().GetJsonString());
        
        return ret;
    }

    int ProjectInfo::UpdateSqliteTable()
    {
        Editor::DataFunctor* df = Editor::DataManager::getInstance()->getDataFunctor();
        
        std::string sql = "select * from "+ TableName_ProjectInfo + " where project_id=" +ProjectId;
        
        std::vector<DataFeature*> features = df->QueryFeaturesBySql(TableName_ProjectInfo, sql);
        
        if(features.size()==0)
        {
            Logger::LogD("ProjectInfo::UpdateSqliteTable()::can not find record, projectId:[%s]",ProjectId.c_str());
            Logger::LogO("ProjectInfo::UpdateSqliteTable()::can not find record, projectId:[%s]",ProjectId.c_str());

            return -1;
        }
        Editor::DataFeature* targetFeature = features.at(0);
            
        int ret = df->UpdateFeature(TableName_ProjectUser, targetFeature->GetTotalPart().GetJsonString());
        
        for(int i=0;i<features.size();i++)
        {
            delete features.at(i);
        }
        
        return ret;
    }

    int ProjectInfo::ParseSqlite(sqlite3_stmt *statement)
    {
        ProjectId = (const char*)sqlite3_column_text(statement, 0);

        int length = sqlite3_column_bytes(statement, 1);

        const unsigned char* tmp = sqlite3_column_text(statement, 1);

        if(tmp != NULL)
        {
	        ProjectRegion = (const char*)sqlite3_column_text(statement, 1);
        }
	    ProjectName = (const char*)sqlite3_column_text(statement, 2);
	    ProjectType = (const char*)sqlite3_column_text(statement, 3);
	    ProjectDetails = (const char*)sqlite3_column_text(statement, 4);

        return 0;
    }

    JSON ProjectInfo::ToJson()
    {
        Document document;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();
        
        Value each_json_value(kStringType);
        std::string each_str_value ;
        
        document.AddMember("project_id",atoi(ProjectId.c_str()),allocator);
        
        each_str_value = ProjectRegion;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("project_region",each_json_value,allocator);
        
        each_str_value = ProjectName;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("project_name",each_json_value,allocator);
        
        each_str_value = ProjectType;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("project_type",each_json_value,allocator);
        
        each_str_value = ProjectDetails;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("project_details",each_json_value,allocator);
        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<StringBuffer> writer(buffer);
        document.Accept(writer);
        std::string result = buffer.GetString();
        
        JSON json;
        
        json.SetJsonString(result);
        
        return json;
    }
    
    Task::Task()
    {
        
    }

    std::vector<std::string> Task::GetTaskDetails(const std::string& sProjectId, const std::string& sUser)
    {
        std::vector<std::string> vResult;
        
        Editor::DataFunctor* df = Editor::DataManager::getInstance()->getDataFunctor();
        
        int projectId = std::atoi(sProjectId.c_str());
        
        if(projectId == 0)
        {
            return vResult;
        }
        
        std::string sql = "select task_details from "+ TableName_TaskInfo +" where project_id=" + sProjectId + " and user_id=" + sUser;
        
        std::vector<Editor::DataFeature*> features = df->QueryFeaturesBySql(TableName_TaskInfo, sql);
        
        int detailsIndex = 0;
        
        for(int i =0; i<features.size();i++)
        {
            std::string detail = features.at(i)->GetAsString(detailsIndex);
            
            vResult.push_back(detail);
            
            delete features.at(i);
        }
        
        return vResult;
    }

    int Task::ParseJson(rapidjson::Value& json, const std::string sUser)
    {
        UserId = sUser;
    
        if(json.HasMember("taskId"))
        {
            TaskId = Tools::NumberToString<int>(json["taskId"].GetInt());
        }

        if(json.HasMember("taskRegion"))
        {
            TaskRegion = json["taskRegion"].GetString();
        }

        if(json.HasMember("taskType"))
        {
            TaskType = json["taskType"].GetString();
        }

        if(json.HasMember("taskPriority") && json["taskPriority"].IsString())
        {
            TaskPriority = json["taskPriority"].GetString();
        }

        if(json.HasMember("taskStatus"))
        {
            TaskStatus = json["taskStatus"].GetString();
        }

        if(json.HasMember("projectId"))
        {
            ProjectId = Tools::NumberToString<int>(json["projectId"].GetInt());
        }

        if(json.HasMember("excuteId"))
        {
            UserExcuteId = Tools::NumberToString<int>(json["excuteId"].GetInt());
        }

        rapidjson::StringBuffer buffer;

        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        json.Accept(writer); 

        TaskDetails = buffer.GetString();

        return 0;
    }

    int Task::ParseSqlite(sqlite3_stmt *statement)
    {
        TaskId = Tools::NumberToString<int>(sqlite3_column_int(statement, 0));
        TaskRegion = (const char*)sqlite3_column_text(statement, 1);
	    TaskType = (const char*)sqlite3_column_text(statement, 2);
	    TaskPriority = (const char*)sqlite3_column_text(statement, 3);
	    TaskStatus = (const char*)sqlite3_column_text(statement, 4);
        ProjectId = (const char*)sqlite3_column_text(statement, 5);
        ProjectId = (const char*)sqlite3_column_text(statement, 6);
        UserExcuteId = (const char*)sqlite3_column_text(statement, 7);
        TaskDetails = (const char*)sqlite3_column_text(statement, 8);

        return 0;
    }

    int Task::InsertSqliteTable()
    {
        Editor::DataFunctor* df = Editor::DataManager::getInstance()->getDataFunctor();
        
        int ret = df->InsertFeature(TableName_TaskInfo, ToJson().GetJsonString());
        
        return ret;
    }

    int Task::UpdateSqliteTable()
    {
        Editor::DataFunctor* df = Editor::DataManager::getInstance()->getDataFunctor();
        
        std::string sql = "select * from "+ TableName_TaskInfo + " where task_id=" + TaskId;
        
        std::vector<DataFeature*> features = df->QueryFeaturesBySql(TableName_TaskInfo, sql);
        
        if(features.size()==0)
        {
            Logger::LogD("Task::UpdateSqliteTable()::can not find record, task_id:[%s]",TaskId.c_str());
            Logger::LogO("Task::UpdateSqliteTable()::can not find record, task_id:[%s]",TaskId.c_str());
            
            return -1;
        }
        Editor::DataFeature* targetFeature = features.at(0);
        
        int ret = df->UpdateFeature(TableName_TaskInfo, targetFeature->GetTotalPart().GetJsonString());
        
        for(int i=0;i<features.size();i++)
        {
            delete features.at(i);
        }
        
        return ret;
    }
    
    JSON Task::ToJson()
    {
        Document document;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();
        
        Value each_json_value(kStringType);
        std::string each_str_value ;
        
        document.AddMember("task_id",atoi(TaskId.c_str()),allocator);
        
        each_str_value = TaskRegion;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("task_region",each_json_value,allocator);
        
        each_str_value = TaskType;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("task_type",each_json_value,allocator);
        
        each_str_value = TaskPriority;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("task_priority",each_json_value,allocator);
        
        each_str_value = TaskStatus;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("task_status",each_json_value,allocator);
        
        document.AddMember("project_id",atoi(ProjectId.c_str()),allocator);
        document.AddMember("user_id",atoi(UserId.c_str()),allocator);
        document.AddMember("user_excute_id",atoi(UserExcuteId.c_str()),allocator);
        
        each_str_value = TaskDetails;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("task_details",each_json_value,allocator);
        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<StringBuffer> writer(buffer);
        document.Accept(writer);
        std::string result = buffer.GetString();
        
        JSON json;
        
        json.SetJsonString(result);
        
        return json;
    }

} // namespace Editor
