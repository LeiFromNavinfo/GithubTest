#include "Editor.h"
#include "Editor_Data.h"
#include "fstream"
#include "model.h"
#include "Editor_ProjectManager.h"
#include "BussinessConstants.h"
#include "Editor_APIWrapper.h"
#define LOADABLE_EXTENSION 1
namespace Editor
{	
	DataSource::DataSource()
	{
        m_Sqlite3Db = NULL;

        m_cache = NULL;
        
        m_dataSourceType = DATASOURCE_UNKNOWN;
	}

	DataSource::~DataSource()
	{
        for(int i=0;i<m_pDataLayers.size();i++)
        {
            delete m_pDataLayers[i];
        }
        
        m_pDataLayers.clear();
	}

    std::string DataSource::GetDataSourcePath()
    {
        std::string path;
        
        Editor::DataManager* dm = Editor::DataManager::getInstance();
        
        if(NULL == dm)
        {
            return path;
        }
        
        Editor::DataFileSystem* dfs = dm->getFileSystem();
        
        if(NULL == dfs)
        {
            return path;
        }
        
        switch (m_dataSourceType)
        {
            case DATASOURCE_COREMAP:
                path= dfs->GetCoreMapDataPath();
                break;
            case DATASOURCE_GDB:
                path=dfs->GetEditorGDBDataFile();
                break;
            case DATASOURCE_PROJECT:
                path=dfs->GetProjectDataPath();
            default:
                break;
        }
        
        return path;
    }
    
    int DataSource::Open(std::string& file)
	{
		if (0 == strcmp(Tools::ToUpper(file).c_str(), m_ConnectString.c_str()))
		{
			Logger::LogD("DataSource::Open file [%s] is already open!", file.c_str());
			Logger::LogO("DataSource::Open file [%s] is already open!", file.c_str());

			return -1;
		}

		if (!m_ConnectString.empty())
		{
			Close(m_ConnectString);
		}

		int rc = sqlite3_open_v2(file.c_str(), &m_Sqlite3Db, SQLITE_OPEN_READWRITE, NULL);

		if (rc != SQLITE_OK)
		{
			Logger::LogD("DataSource::Open [%s] open failed! errmsg [%s],ret :[%d]", file.c_str(), sqlite3_errmsg (m_Sqlite3Db),rc);
			Logger::LogO("DataSource::Open [%s] open failed! errmsg [%s],ret :[%d]", file.c_str(), sqlite3_errmsg (m_Sqlite3Db),rc);
		
			sqlite3_close_v2(m_Sqlite3Db);

			m_Sqlite3Db = NULL;

			return -1;
		}

		m_ConnectString = Tools::ToUpper(file);

		if (!m_cache)
		{
			m_cache = spatialite_alloc_connection();
			spatialite_init_ex(m_Sqlite3Db, m_cache, 0);
		}

		return 0;
	}

	int DataSource::Close(std::string& file)
	{
		if (0 != strcmp(Tools::ToUpper(file).c_str(), m_ConnectString.c_str()))
		{
			Logger::LogD("DataSource::Close file [%s] is not open!", file.c_str());
			Logger::LogO("DataSource::Close file [%s] is not open!", file.c_str());

			return -1;
		}

		if (m_cache)
		{
			spatialite_cleanup_ex(m_cache);
			m_cache=NULL;
		}

		sqlite3_close_v2(m_Sqlite3Db);

		m_Sqlite3Db = NULL;

		m_ConnectString = "";

//		if (m_cache)
//		{
//			spatialite_cleanup_ex(m_cache);
//			m_cache=NULL;
//		}

		return 0;
	}

    DataLayer* DataSource::GetLayerByType(DATALAYER_TYPE type)
	{
        if(m_Sqlite3Db == NULL)
        {
            return NULL;
        }
        
        std::vector<DataLayer*>::iterator iter = m_pDataLayers.begin();
        
        for(;iter!=m_pDataLayers.end();++iter)
        {
            if ((*iter)->GetDataLayerType() == type)
            {
                (*iter)->SetDBConnection(m_Sqlite3Db);

                return *iter;
            }
        }
        return NULL;
	}

    DataLayer* DataSource::GetLayerByName(const std::string& name)
    {
        if(m_Sqlite3Db == NULL)
        {
            return NULL;
        }
        
        std::vector<DataLayer*>::iterator iter = m_pDataLayers.begin();
        
        for(;iter!=m_pDataLayers.end();++iter)
        {
            std::string layerName = (*iter)->GetTableName();
            
            if (Tools::CaseInsensitiveCompare(layerName,name))
            {
                (*iter)->SetDBConnection(m_Sqlite3Db);
                
                return *iter;
            }
        }
        
        return NULL;
    }
    
    int DataSource::BeginTranscation()
	{
		char* pszErrMsg;

		int rc = sqlite3_exec(m_Sqlite3Db, "BEGIN", NULL, NULL, &pszErrMsg);

		if (rc != SQLITE_OK)
		{
			Logger::LogD("DataSource::BeginTranscation failed [%s]", pszErrMsg);
			Logger::LogO("DataSource::BeginTranscation failed [%s]", pszErrMsg);

			sqlite3_free (pszErrMsg);

			return -1;
		}

		return 0;
	}

    int DataSource::EndTranscation()
	{
		char* pszErrMsg;

		int rc = sqlite3_exec(m_Sqlite3Db, "COMMIT", NULL, NULL, &pszErrMsg);

		if (rc != SQLITE_OK)
		{
			Logger::LogD("DataSource::EndTranscation failed [%s]", pszErrMsg);
			Logger::LogO("DataSource::EndTranscation failed [%s]", pszErrMsg);

			sqlite3_free (pszErrMsg);

			return -1;
		}

		return 0;
	}

    int DataSource::RollbackTranscation()
	{
		char* pszErrMsg;

		int rc = sqlite3_exec(m_Sqlite3Db, "ROLLBACK", NULL, NULL, &pszErrMsg);

		if (rc != SQLITE_OK)
		{
			Logger::LogD("DataSource::RollbackTranscation failed [%s]", pszErrMsg);
			Logger::LogO("DataSource::RollbackTranscation failed [%s]", pszErrMsg);

			sqlite3_free (pszErrMsg);

			return -1;
		}

		return 0;
	}

    std::string DataSource::GetConnectionString()
	{
	    return m_ConnectString;
	}
    
    int DataSource::InstallLayers(std::string sDownloadSqlite, std::string sExistsSqlite, std::string layerName, ProgressEventer* progress)
    {
        sqlite3* hDBDownload = openSqlite(sDownloadSqlite);

        sqlite3* hDBExist = openSqlite(sExistsSqlite);

        if(hDBDownload == NULL || hDBExist == NULL)
        {
            return -1;
        }

        int ret = IsHaveColumnInTable(hDBExist, Poi_displaystyle, PoiTableName);

        if(ret == 0)
        {
            Logger::LogD("DataSource::sqlite column displayInfo not exists");
            Logger::LogO("DataSource::sqlite column displayInfo not exists");

            return -1;
        }
        else if(ret == -1)
        {
            Logger::LogD("DataSource::check column failed");
            Logger::LogO("DataSource::check column failed");

            return -1;
        }

        void* cache_download = spatialite_alloc_connection();

        spatialite_init_ex (hDBDownload, cache_download, 0);

        void* cache_exists = spatialite_alloc_connection();

        spatialite_init_ex (hDBExist, cache_exists, 0);

        std::string sql_select_download = "select * from " + layerName;

        sqlite3_stmt* stmt_download = NULL;

        sqlite3_stmt* stmt_exist = NULL;

        int rc = sqlite3_prepare_v2(hDBDownload, sql_select_download.c_str(), -1, &stmt_download, NULL);

        if(rc != SQLITE_OK)
        {
            sqlite3_close(hDBDownload);

            sqlite3_close(hDBExist);

            return -1;
        }

        if(progress != NULL)
        {
            progress->OnProgress("That the Sqlite Data haved installed is", 10.0);
        }

        while(sqlite3_step(stmt_download) == SQLITE_ROW)
        {
            
            Model::Point point;

            if(point.ParseSqlite(stmt_download) == -1)
            {
                Logger::LogD("DataSource::sqlite parse failed");
                Logger::LogO("DataSource::sqlite parse failed");

                return -1;
            }

            std::string sPid = "";

            if(point.pid != 0)
            {
                std::stringstream ss;

                ss<<point.pid;

                sPid = ss.str();
            }
            
            if(!(point.fid).empty() && IsExistsInDatabase(hDBExist, layerName, "fid", point.fid) == 0)
            {

                point.SetDisplayInfo();

                point.UpdateSqliteTable(hDBExist);
            }
            else if(!sPid.empty() && IsExistsInDatabase(hDBExist, layerName, "pid", sPid) == 0)
            {

                point.SetDisplayInfo();

                point.UpdateSqliteTable(hDBExist);
            }
            else
            {

                point.SetDisplayInfo();

                point.InsertSqliteTable(hDBExist);
            }

            sqlite3_finalize(stmt_exist);
        }

        if(progress != NULL)
        {
            progress->OnProgress("That the Sqlite Data haved installed is", 100.0);
        }

        sqlite3_finalize(stmt_download);

        sqlite3_close(hDBDownload);
        spatialite_cleanup_ex(cache_download);

        sqlite3_close(hDBExist);
        spatialite_cleanup_ex(cache_exists);

        return 0;
    }

    std::string DataSource::GenerateSql(sqlite3* db, sqlite3_stmt* stmt, std::string layerName)
    {
        std::string sql_insert = "insert into Point values(";

        std::string sql_update = "update Point set ";

        std::string fid = "";

        std::string pid = "";

        bool isFidUpdate = false;

        bool isPidUpdate = false;

        int columns = sqlite3_column_count(stmt);

        for(size_t i = 0; i < columns; i++)
        {
            std::string name = sqlite3_column_name(stmt, i);

            sql_insert += "?, ";

            sql_update += name + "=?, ";

            if(name == "fid")
            {
                fid = (const char*)sqlite3_column_text(stmt, i);;

                if(fid.empty())
                {
                    continue;
                }

                if(IsExistsInDatabase(db, layerName, "fid", fid) == 0)
                {
                    isFidUpdate = true;
                }
            }
            else if(name == "pid")
            {
                pid = (const char*)sqlite3_column_text(stmt, i);

                if(pid.empty())
                {
                    continue;
                }

                if(IsExistsInDatabase(db, layerName, "pid", pid) == 0)
                {
                    isPidUpdate = true;
                }
            }
        }

        std::string sql;

        if(isFidUpdate)
        {
            sql = sql_update.substr(0, sql_update.length() - 2) + " where fid='" + fid +"'";
        }
        else if(isPidUpdate)
        {
            sql = sql_update.substr(0, sql_update.length() - 2) + " where pid='" + pid +"'";
        }
        else
        {
            if(pid.empty() && fid.empty())
            {
                return "";
            }

            sql = sql_insert.substr(0, sql_insert.length() - 2) + ")";
        }

        return sql;
    }

    int DataSource::IsExistsInDatabase(sqlite3* db, std::string tableName, std::string key, std::string keyID)
    {
        sqlite3_stmt* stmt = NULL;

        std::string sql_select_count = "select * from " + tableName + " where " + key + "='" + keyID + "'";

        int rc = sqlite3_prepare_v2(db, sql_select_count.c_str(), -1, &stmt, NULL);

        if(rc != SQLITE_OK)
        {
            return -1;
        }

        if(sqlite3_step(stmt) == SQLITE_ROW)
        {
            return 0;
        }

        sqlite3_finalize(stmt);

        return -1;
    }

    sqlite3* DataSource::openSqlite(std::string path)
    {
        sqlite3* db = NULL;

        int rc = sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READWRITE, NULL);

        if (rc != SQLITE_OK)
        {
            Logger::LogD("DataSource::sqlite [%s] or [%s] open failed", path.c_str(), sqlite3_errmsg(db));
            Logger::LogO("DataSource::sqlite [%s] or [%s] open failed", path.c_str(), sqlite3_errmsg(db));

            sqlite3_close(db);

            return NULL;
        }

        return db;
    }

    void DataSource::closeSqlite(sqlite3* db)
    {
        sqlite3_close(db);
    }

    int DataSource::BindData(sqlite3_stmt* stmt_exist, sqlite3_stmt* stmt_download)
    {
        int columns = sqlite3_column_count(stmt_download);

        for(size_t i = 0; i < columns; i++)
        {
            int type = sqlite3_column_type(stmt_download , i); 

            switch(type)
            {
            case SQLITE_INTEGER:
                sqlite3_bind_int(stmt_exist, i + 1, sqlite3_column_int(stmt_download, i));
                break;
            case SQLITE_FLOAT:
                sqlite3_bind_double(stmt_exist, i + 1, sqlite3_column_double(stmt_download, i));
                break;
            case SQLITE_TEXT:
                sqlite3_bind_text(stmt_exist, i + 1, (const char*)sqlite3_column_text(stmt_download, i), sqlite3_column_bytes(stmt_download, i), NULL);
                break;
            case SQLITE_BLOB:
                sqlite3_bind_blob(stmt_exist, i + 1, sqlite3_column_blob(stmt_download, i), sqlite3_column_bytes(stmt_download, i), NULL);
                break;
            default:
                break;
            }
        }

        return 0;
    }

    int DataSource::IsHaveColumnInTable(sqlite3* db, const std::string& sColumn, const std::string& sTable)
    {
        if(db == NULL || sColumn.empty())
        {
            return -1;
        }

        std::string sql = "select * from " + sTable;

        char *pszErrMsg;

        char **papszResult;

        int nRowCount, iRow, nColCount;

        int rc = rc = sqlite3_get_table( db, sql.c_str(), &papszResult, &nRowCount, &nColCount, &pszErrMsg );

        if(rc != SQLITE_OK)
        {
            return -1;
        }

        std::string sResult = "";

        for(int i = 0; i < nColCount; i++)
        {
            sResult += papszResult[i];
        }

        if(sResult.empty())
        {
            return -1;
        }

        if(sResult.find(sColumn) == std::string::npos)
        {
            return 0;
        }

        return 1;
    }
    
    bool DataSource::isWithinGrid(std::string pointWkt, std::string wkt)
    {
        geos::io::WKTReader wktReader;

        geos::geom::Geometry* point = wktReader.read(pointWkt);

        geos::geom::Geometry* geo = wktReader.read(wkt);

        return geo->contains(point);
    }

    int DataSource::InitDataBaseToWal(std::string sPath, bool isWal)
    {
        char* zErrMsg;

        sqlite3* db = NULL;

        int rc = sqlite3_open_v2(sPath.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, NULL);

        if (rc != SQLITE_OK)
        {
            Logger::LogD("DataSource::sqlite [%s] or [%s] open failed", sPath.c_str(), sqlite3_errmsg(db));
            Logger::LogO("DataSource::sqlite [%s] or [%s] open failed", sPath.c_str(), sqlite3_errmsg(db));

            sqlite3_close(db);

            return -1;
        }

	    if(isWal == true)
	    {
		    rc = sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, 0, &zErrMsg);

		    if (rc != SQLITE_OK)
		    {
			    sqlite3_free(zErrMsg);

			    sqlite3_close(db);

			    return -1;
		    }

		    rc = sqlite3_exec(db, "PRAGMA wal_autocheckpoint=100;", NULL, 0, &zErrMsg);

		    if (rc != SQLITE_OK)
		    {
			    sqlite3_free(zErrMsg);

			    sqlite3_close(db);

			    return -1;
		    }
		    Logger::LogD("wal");
	    }
	    else
	    {
		    rc = sqlite3_exec(db, "PRAGMA journal_mode=DELETE;", NULL, 0, &zErrMsg);

            if (rc != SQLITE_OK)
	        {
		        sqlite3_free(zErrMsg);

		        sqlite3_close(db);

		        return -1;
	        }
        }

	    sqlite3_close(db);

	    return true;
    }
    
    int DataSource::GetDataSourceType()
    {
        return m_dataSourceType;
    }
    
    //CoreMapDataSource
    CoreMapDataSource::CoreMapDataSource():DataSource()
    {
        InitializeDataLayer();
        
        this->m_dataSourceType = DATASOURCE_COREMAP;
    }
    
    CoreMapDataSource::~CoreMapDataSource()
    {
        
    }
    
    void CoreMapDataSource::InitializeDataLayer()
    {
        DataLayer* poiLayer = new DataLayer();
        
        poiLayer->SetDataLayerType(DATALAYER_POI);
        
        m_pDataLayers.push_back(poiLayer);
        
        DataLayer* inforLayer = new DataLayer();
        
        inforLayer->SetDataLayerType(DATALAYER_INFOR);
        
        m_pDataLayers.push_back(inforLayer);
        
        DataLayer* tipsLayer = new DataLayer();
        
        tipsLayer->SetDataLayerType(DATALAYER_TIPS);
        
        m_pDataLayers.push_back(tipsLayer);
        
        DataLayer* gpslineLayer = new DataLayer();
        
        gpslineLayer->SetDataLayerType(DATALAYER_GPSLINE);
        
        m_pDataLayers.push_back(gpslineLayer);
        
        DataLayer* tipsPointLayer = new DataLayer();
        
        tipsPointLayer->SetDataLayerType(DATALAYER_TIPSPOINT);
        
        m_pDataLayers.push_back(tipsPointLayer);
        
        DataLayer* tipsLineLayer = new DataLayer();
        
        tipsLineLayer->SetDataLayerType(DATALAYER_TIPSLINE);
        
        m_pDataLayers.push_back(tipsLineLayer);
        
        DataLayer* tipsMultiLineLayer = new DataLayer();
        
        tipsMultiLineLayer->SetDataLayerType(DATALAYER_TIPSMULTILINE);
        
        m_pDataLayers.push_back(tipsMultiLineLayer);
        
        DataLayer* tipsPolygonLayer = new DataLayer();
        
        tipsPolygonLayer->SetDataLayerType(DATALAYER_TIPSPOLYGON);
        
        m_pDataLayers.push_back(tipsPolygonLayer);
        
        DataLayer* tipsGeoComponentLayer = new DataLayer();
        
        tipsGeoComponentLayer->SetDataLayerType(DATALAYER_TIPSGEOCOMPONENT);
        
        m_pDataLayers.push_back(tipsGeoComponentLayer);
        
        DataLayer* infoLayer = new DataLayer();
        
        infoLayer->SetDataLayerType(DATALAYER_INFOR);
        
        m_pDataLayers.push_back(infoLayer);
        
        
        DataLayer* trackPointLayer = new DataLayer();
        
        trackPointLayer->SetDataLayerType(DATALAYER_TRACKPOINT);
        
        m_pDataLayers.push_back(trackPointLayer);
        
        DataLayer* trackSegmentLayer = new DataLayer();
        
        trackSegmentLayer->SetDataLayerType(DATALAYER_TRACKSEGMENT);
        
        m_pDataLayers.push_back(trackSegmentLayer);
        
        
    }
    
    void errorLogCallback(void *pArg, int iErrCode, const char *zMsg){
       Logger::LogD("(%d) %s", iErrCode, zMsg);
    }

    void CreateSqlite()
    {
    	std::string sqlite_test = "/storage/sdcard0/test/test.sqlite";

    	::remove(sqlite_test.c_str());

    	//spatialite_init(1);

    	Logger::LogD("std:%s", stderr);

    	sqlite3* db = NULL;

    	int rc = sqlite3_open_v2(sqlite_test.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    	//int rc = sqlite3_open(sqlite_test.c_str(), &db);

    	Logger::LogD("rc:%d", rc);

    	char* zErrMsg;

    	rc = sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, 0, &zErrMsg);

		if (rc != SQLITE_OK)
		{	Logger::LogD("xxx2 pragma");
			sqlite3_free(zErrMsg);

			sqlite3_close_v2(db);

			return;
		}


    	//rc = sqlite3_close_v2(db);

    	//Logger::LogD("rc2:%d", rc);

    	//rc = sqlite3_open(sqlite_test.c_str(), &db);

    	//Logger::LogD("rc3:%d", rc);

//    	rc = sqlite3_enable_load_extension(db, 1);
//
//    	Logger::LogD("rc4:%d", rc);
//
//    	char* pzErrMsg;
//
//    	rc = sqlite3_load_extension(db, "libspatialite.so", "spatialite_initialize", &pzErrMsg);
//
//    	Logger::LogD("rc6:%d,%s", rc, pzErrMsg);

    	int tempret2 = access(sqlite_test.c_str(), F_OK);

    	Logger::LogD("tempret2:%d", tempret2);

    	void* cache = spatialite_alloc_connection();

    	spatialite_init_ex(db, cache, 1);



    	Logger::LogD("Blobfile Error %d : %s", errno, strerror(errno));

    	Logger::LogD("VERSION:%s,%s", sqlite3_libversion(), spatialite_version());

    	std::string sql4 = "SELECT spatialite_version(), proj4_version(), geos_version(), sqlite_version()";

		sqlite3_stmt* stmt;

		rc = sqlite3_prepare(db, sql4.c_str(), -1, &stmt, NULL);

		if (rc != SQLITE_OK)
		{
			Logger::LogD("sql [%s] failed [%s]", sql4.c_str(), sqlite3_errmsg(db));
			Logger::LogO("sql [%s] failed [%s]", sql4.c_str(), sqlite3_errmsg(db));

			sqlite3_finalize(stmt);
		}

		rc = sqlite3_step(stmt);

		if (rc == SQLITE_ROW)
		{
			int version_bytes = sqlite3_column_bytes(stmt, 0);

			char* version_str = (char*)sqlite3_column_text(stmt, 0);

			std::string version = std::string(version_str, version_bytes);

			int version_bytes1 = sqlite3_column_bytes(stmt, 1);

			char* version_str1 = (char*)sqlite3_column_text(stmt, 1);

			std::string version1 = std::string(version_str1, version_bytes1);

			int version_bytes2 = sqlite3_column_bytes(stmt, 2);

			char* version_str2 = (char*)sqlite3_column_text(stmt, 2);

			std::string version2 = std::string(version_str2, version_bytes2);

			int version_bytes3 = sqlite3_column_bytes(stmt, 3);

			char* version_str3 = (char*)sqlite3_column_text(stmt, 3);

			std::string version3 = std::string(version_str3, version_bytes3);


			Logger::LogD("version:%s,%s,%s,%s", version.c_str(), version1.c_str(), version2.c_str(), version3.c_str());
		}

		//DataTransfor::ExecuteNonQuery("BEGIN", db);

		//DataTransfor::ExecuteNonQuery("SELECT load_extension('libspatialite.so')", db);

    	DataTransfor::ExecuteNonQuery("SELECT InitSpatialMetadata(1)", db);

    	DataTransfor::ExecuteNonQuery("BEGIN", db);

    	std::string sql = "create table test (id int32 primary key, name text)";

    	std::string geo_sql = "select AddGeometryColumn('test', 'geometry', 4326, 'GEOMETRY', 'XY')";

    	//std::string sql2 = "insert into test values (32, \"name\")";

    	DataTransfor::ExecuteNonQuery(sql.c_str(), db);

    	DataTransfor::ExecuteNonQuery(geo_sql.c_str(), db);

    	//DataTransfor::ExecuteNonQuery(sql2.c_str(), db);



    	std::string sql3 = "insert into test values(?, ?, ?)";

    	//sqlite3_stmt* stmt;
    	sqlite3_reset(stmt);

    	sqlite3_clear_bindings(stmt);

    	rc = sqlite3_prepare_v2(db, sql3.c_str(), -1, &stmt, NULL);

    	Logger::LogD("rc2:%d", rc);

    	sqlite3_bind_int(stmt, 1, 32);

    	std::string nameStr = "name";

    	sqlite3_bind_text(stmt, 2, nameStr.c_str(), nameStr.length(), NULL);

    	EditorGeometry::WkbPoint* wkb=new EditorGeometry::WkbPoint();
    	wkb->_byteOrder = 1;
		wkb->_wkbType = (EditorGeometry::WkbGeometryType)1;
		wkb->_point._x = 116.40;
		wkb->_point._y = 39.39;

		EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

		sqlite3_bind_blob(stmt, 3, spGeo, wkb->buffer_size()+39, NULL);

		if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			Logger::LogD("error!");
		}

		sqlite3_finalize(stmt);

		delete wkb;

		delete spGeo;

		DataTransfor::ExecuteNonQuery("COMMIT", db);





    	spatialite_cleanup_ex(cache);

    	spatialite_shutdown();

    	rc = sqlite3_close_v2(db);
    }

    int CoreMapDataSource::InstallPoi(std::string sFilePath, std::string sFlag, ProgressEventer* progress)
    {
    	Logger::LogD("InstallPoi Begin:");
    	Logger::LogO("InstallPoi Begin:");

    	Tools::WatchTimerStart();

        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "安装"), 0.0);
        }
        
        int nBeg = sFilePath.find_last_of(".zip");
        
        if(!(nBeg != std::string::npos && nBeg == sFilePath.length() - 1))
        {
            return -1;
        }
        
        std::string sPathFolderTmp = sFilePath.substr(0, sFilePath.length() - 4);
        
#ifdef WIN32
        std::string sPathFolderFileTmp = sPathFolderTmp + "\\tmp.txt";
#else
        std::string sPathFolderFileTmp = sPathFolderTmp + "/tmp.txt";
#endif
        
        if(Tools::CheckFilePath(sPathFolderFileTmp.c_str()) == false)
        {
            return -1;
        }
        
        sPathFolderTmp = Tools::UnZipDir(sFilePath.c_str(), sPathFolderTmp.c_str());
        
        if(sPathFolderTmp.empty())
        {
            Logger::LogD("InstallPoi error :Zip File Failed, Json File Path: %s",sPathFolderTmp.c_str());
            Logger::LogO("InstallPoi error :Zip File Failed, Json File Path: %s",sPathFolderTmp.c_str());
            
            return -1;
        }
        
#ifdef WIN32
        sPathFolderFileTmp = sPathFolderTmp + "\\poi.json";
#else
        sPathFolderFileTmp = sPathFolderTmp + "/poi.json";
#endif
        
        Editor::DataManager* pDataManager = Editor::DataManager::getInstance();
        
        Editor::DataFileSystem* pFileSystem = pDataManager->getFileSystem();
        
        std::string sPath = pFileSystem->GetCoreMapDataPath();

        sqlite3* hDB = openSqlite(sPath);

        void* cache = spatialite_alloc_connection();
        spatialite_init_ex(hDB, cache, 0);

        std::ifstream in;

        in.open(sPathFolderFileTmp.c_str());
        
        if(!in)
        {
            std::cout<<"error : 打开文件失败！"<<std::endl;
            Logger::LogD("error :Open File Failed, Json File Path: %s",sPathFolderFileTmp.c_str());
            Logger::LogO("error :Open File Failed, Json File Path: %s",sPathFolderFileTmp.c_str());
            
            return -1;
        }
        
        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "安装"), 5.0);
        }
        
        std::map<std::string, unsigned int> mPoiFids;

        std::map<int, unsigned int> mPoiPids;

        std::string sql = "select rowid, fid, pid from edit_pois";

        sqlite3_stmt* stmt;

		int rc = sqlite3_prepare(hDB, sql.c_str(), -1, &stmt, NULL);

		if (rc != SQLITE_OK)
		{
			Logger::LogD("sql [%s] failed [%s]", sql.c_str(), sqlite3_errmsg(hDB));
			Logger::LogO("sql [%s] failed [%s]", sql.c_str(), sqlite3_errmsg(hDB));

			sqlite3_finalize(stmt);

			return -1;
		}

		rc = sqlite3_step(stmt);

		while (rc == SQLITE_ROW)
		{
			unsigned int rowid = sqlite3_column_int(stmt, 0);

			int fid_bytes = sqlite3_column_bytes(stmt, 1);

			char* fid_str = (char*)sqlite3_column_text(stmt, 1);

			if (fid_bytes != 0)
			{
				mPoiFids.insert(std::make_pair(std::string(fid_str, fid_bytes), rowid));
			}

			int pid = sqlite3_column_int(stmt, 2);

			if (pid != 0)
			{
				mPoiPids.insert(std::make_pair(pid, rowid));
			}

			rc = sqlite3_step(stmt);
		}

		Logger::LogD("before install, poi fid count [%d], pid count [%d]", mPoiFids.size(), mPoiPids.size());
		Logger::LogO("before install, poi fid count [%d], pid count [%d]", mPoiFids.size(), mPoiPids.size());

		sqlite3_finalize(stmt);

        std::string line;
        
        char *err_msg = NULL;
        
        int nCount = 0;
        
        while(getline(in, line))
        {
            if(nCount == 0)
            {
                int ret = sqlite3_exec (hDB, "BEGIN", NULL, NULL, &err_msg);

                if (ret != SQLITE_OK)
                {
                    Logger::LogD("InsertSqliteTable BEGIN error: %s\n", err_msg);
                    Logger::LogO("InsertSqliteTable BEGIN error: %s\n", err_msg);

                    sqlite3_free(err_msg);
                    return -1;
                }
            }
            
            nCount++;
            
            Model::Point point;
            
            point.ParseJson(line);
            
            point.type = 0;
            
            
            
            point.auditStatus = 0;
            
            point.submitStatus_desktop = 0;
            
            point.project = sFlag;
            
            point.t_operateDate = Tools::GetCurrentDateTime();
            point.globalId = point.SetPrimaryKey();
            
            if(!(point.fid).empty() && (mPoiFids.find(point.fid) != mPoiFids.end()))
            {

                point.SetDisplayInfo();

                point.UpdateSqliteTableWithoutTransaction(hDB);
            }
            else if((0 != point.pid) && (mPoiPids.find(point.pid) != mPoiPids.end()))
            {

                point.SetDisplayInfo();

                point.UpdateSqliteTableWithoutTransaction(hDB);
            }
            else
            {

                point.SetDisplayInfo();
                
                point.InsertSqliteTableWithoutTransaction(hDB);
            }
            
            if(nCount == 10000)
            {
                int ret = sqlite3_exec (hDB, "COMMIT", NULL, NULL, &err_msg);

                if (ret != SQLITE_OK)
                {
                    Logger::LogD ("COMMIT error: %s\n", err_msg);
                    Logger::LogO ("COMMIT error: %s\n", err_msg);

                    sqlite3_free (err_msg);

                    return -1;
                }

                if(progress != NULL)
                {
                    progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "安装"), 50.0);
                }

                nCount = 0;
            }
        }
        
        if(nCount < 10000)
        {
            int ret = sqlite3_exec (hDB, "COMMIT", NULL, NULL, &err_msg);

            if (ret != SQLITE_OK)
            {
                Logger::LogD ("COMMIT error: %s\n", err_msg);
                Logger::LogO ("COMMIT error: %s\n", err_msg);

                sqlite3_free (err_msg);

                return -1;
            }
        }
        
        spatialite_cleanup_ex(cache);
        cache=NULL;
        
        sqlite3_close(hDB);
        
        Tools::DelDir(sPathFolderTmp.c_str());
        
        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "安装"), 100.0);
        }
        
        long cost = Tools::WatchTimerStop();

        Logger::LogD("InstallPoi End! cost is [%ld]", cost);
        Logger::LogO("InstallPoi End! cost is [%ld]", cost);

        return 0;
    }
	int CoreMapDataSource::InstallInfo(std::string sFilePath, std::string sFlag, ProgressEventer* progress)
    {
        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "安装"), 0.0);
        }
        
        int nBeg = sFilePath.find_last_of(".zip");
        
        if(!(nBeg != std::string::npos && nBeg == sFilePath.length() - 1))
        {
            return -1;
        }
        
        std::string sPathFolderTmp = sFilePath.substr(0, sFilePath.length() - 4);
        
#ifdef WIN32
        std::string sPathFolderFileTmp = sPathFolderTmp + "\\tmp.txt";
#else
        std::string sPathFolderFileTmp = sPathFolderTmp + "/tmp.txt";
#endif
        
        if(Tools::CheckFilePath(sPathFolderFileTmp.c_str()) == false)
        {
            return -1;
        }
        
        sPathFolderTmp = Tools::UnZipDir(sFilePath.c_str(), sPathFolderTmp.c_str());
        
        if(sPathFolderTmp.empty())
        {
            Logger::LogD("InstallPoi error :Zip File Failed, Json File Path: %s",sPathFolderTmp.c_str());
            Logger::LogO("InstallPoi error :Zip File Failed, Json File Path: %s",sPathFolderTmp.c_str());
            
            return -1;
        }
        
#ifdef WIN32
        sPathFolderFileTmp = sPathFolderTmp + "\\infor.json";
#else
        sPathFolderFileTmp = sPathFolderTmp + "/infor.json";
#endif
        
        Editor::DataManager* pDataManager = Editor::DataManager::getInstance();
        
        Editor::DataFileSystem* pFileSystem = pDataManager->getFileSystem();
        
        std::string sPath = pFileSystem->GetCoreMapDataPath();
        
        sqlite3* hDB = openSqlite(sPath);
        
        void* cache = spatialite_alloc_connection();
        spatialite_init_ex(hDB, cache, 0);
        
        std::ifstream in;
        
        in.open(sPathFolderFileTmp.c_str());
        
        if(!in)
        {
            std::cout<<"error : 打开文件失败！"<<std::endl;
            Logger::LogD("error :Open File Failed, Json File Path: %s",sPathFolderFileTmp.c_str());
            Logger::LogO("error :Open File Failed, Json File Path: %s",sPathFolderFileTmp.c_str());
            
            return -1;
        }
        
        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "安装"), 5.0);
        }
        
        std::string line;
        
        char *err_msg = NULL;
        
        int nCount = 0;
        
        while(getline(in, line))
        {
            if(nCount == 0)
            {
                int ret = sqlite3_exec (hDB, "BEGIN", NULL, NULL, &err_msg);
                
                if (ret != SQLITE_OK)
                {
                    Logger::LogD("InsertSqliteTable BEGIN error: %s\n", err_msg);
                    Logger::LogO("InsertSqliteTable BEGIN error: %s\n", err_msg);
                    
                    sqlite3_free(err_msg);
                    return -1;
                }
            }
            
            nCount++;
            
            Model::Infor info;

            info.ParseJson(line);
            
            if(info.h_projectId=="")
                info.h_projectId = sFlag;

            if(IsExistsInDatabase(hDB, "edit_infos", "_id", info._id) == 0)
            {
                info.UpdateSqliteTableWithoutTransaction(hDB);
            }
            else
            {
                info.InsertSqliteTableWithoutTransaction(hDB);
            }

            
            if(nCount == 10000)
            {
                int ret = sqlite3_exec (hDB, "COMMIT", NULL, NULL, &err_msg);
                
                if (ret != SQLITE_OK)
                {
                    Logger::LogD ("COMMIT error: %s\n", err_msg);
                    Logger::LogO ("COMMIT error: %s\n", err_msg);
                    
                    sqlite3_free (err_msg);
                    
                    return -1;
                }
                
                if(progress != NULL)
                {
                    progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "安装"), 50.0);
                }
                
                nCount = 0;
            }
        }
        
        if(nCount < 10000)
        {
            int ret = sqlite3_exec (hDB, "COMMIT", NULL, NULL, &err_msg);
            
            if (ret != SQLITE_OK)
            {
                Logger::LogD ("COMMIT error: %s\n", err_msg);
                Logger::LogO ("COMMIT error: %s\n", err_msg);
                
                sqlite3_free (err_msg);
                
                return -1;
            }
        }
        
        spatialite_cleanup_ex(cache);
        cache=NULL;
        
        sqlite3_close(hDB);
        
        Tools::DelDir(sPathFolderTmp.c_str());
        
        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "安装"), 100.0);
        }
        
        return 0;
    }
    
    int CoreMapDataSource::InstallTips(std::string sJsonPath, const std::vector<std::string>& grids, ProgressEventer* progress)
    {
        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo("tips", "安装"), 0.0);
        }
        
        std::string sExistSqlite = Editor::DataManager::getInstance()->getFileSystem()->GetCoreMapDataPath();
        
        sqlite3* hDB = openSqlite(sExistSqlite);
        
        if(hDB == NULL)
        {
            return -1;
        }
        
        void* cache_download = spatialite_alloc_connection();
        
        spatialite_init_ex (hDB, cache_download, 0);
        
        std::ifstream in;
        
        in.open(sJsonPath.c_str());
        
        if(!in)
        {
            Logger::LogD("error :Open File Failed, Json File Path: %s",sJsonPath.c_str());
            Logger::LogO("error :Open File Failed, Json File Path: %s",sJsonPath.c_str());
        }
        
        std::string line;
        
        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo("tips", "安装"), 10.0);
        }
        
        char *err_msg = NULL;
        
        int installCount = 0;
        
        bool isInstalled = false;
        
        while(getline(in, line))
        {
            if(installCount == 0)
            {
                int ret = sqlite3_exec (hDB, "BEGIN", NULL, NULL, &err_msg);
                
                if (ret != SQLITE_OK)
                {
                    Logger::LogD("InsertSqliteTable BEGIN error: %s\n", err_msg);
                    Logger::LogO("InsertSqliteTable BEGIN error: %s\n", err_msg);
                    
                    sqlite3_free(err_msg);
                    return -1;
                }
            }
            
            Model::Tips serverTips;
            serverTips.setDataSource(Model::Tips::DataSourceFromServer);
            serverTips.ParseJson(line);
            
            //lee add
            std::vector<Model::tips_geo*> geosVector;
            geosVector = serverTips.BuildGeosBySourceType();
            std::vector<Model::tips_geo*>::iterator itor = geosVector.begin();
            while (itor!=geosVector.end())
            {
                if (*itor)
                {
                    serverTips.Attach(*itor);
                }
                ++itor;
            }
            
            
            if(!(serverTips.rowkey).empty())
            {
                installCount++;
                
                isInstalled = true;
                
                Model::Tips localTips = IsTipsExistInDatabase(hDB, "edit_tips", "rowkey", serverTips.rowkey);
                
                if(strcmp(localTips.rowkey.c_str(), "") != 0)
                {
                    if(localTips.t_lifecycle == LIFECYCLE_ORIGINAL)
                    {
                        if(serverTips.t_lifecycle == LIFECYCLE_ORIGINAL || serverTips.t_lifecycle == LIFECYCLE_UPDATE ||
                           serverTips.t_lifecycle == LIFECYCLE_ADD)
                        {
                            serverTips.t_lifecycle = 0;
                            serverTips.t_status = 0;
                            serverTips.t_handler = 0;
                        }
                        else if(serverTips.t_lifecycle == LIFECYCLE_DELETE)
                        {
                            serverTips.t_lifecycle = 1;
                            serverTips.t_status = 0;
                            serverTips.t_handler = 0;
                        }
                    }
                    else
                    {
                        serverTips.t_status = localTips.t_status;
                        serverTips.t_handler = localTips.t_handler;
                        
                    }
                    serverTips.t_sync =1;
                    serverTips.SetDisplayStyle(line);
                    
                    serverTips.UpdateSqliteTable(hDB);
                    
                    //lee add
                    serverTips.Notify(hDB,Model::tips_geo::operatorUpdate);
                    
                }
                else
                {
                    if(serverTips.t_lifecycle == LIFECYCLE_ORIGINAL || serverTips.t_lifecycle == LIFECYCLE_UPDATE ||
                       serverTips.t_lifecycle == LIFECYCLE_ADD)
                    {
                        serverTips.t_lifecycle = 0;
                        serverTips.t_status = 0;
                        serverTips.t_handler = 0;
                    }
                    else if(serverTips.t_lifecycle == LIFECYCLE_DELETE)
                    {
                        serverTips.t_lifecycle = 1;
                        serverTips.t_status = 0;
                        serverTips.t_handler = 0;
                    }
                    serverTips.t_sync =1;
                    serverTips.SetDisplayStyle(line);
                    
                    serverTips.InsertSqliteTable(hDB);
                    
                    //lee add
                    serverTips.Notify(hDB,Model::tips_geo::operatorInsert);
                }
            }
            
            if(installCount == 10000)
            {
                int ret = sqlite3_exec (hDB, "COMMIT", NULL, NULL, &err_msg);
                
                if (ret != SQLITE_OK)
                {
                    Logger::LogD ("COMMIT error: %s\n", err_msg);
                    Logger::LogO ("COMMIT error: %s\n", err_msg);
                    
                    sqlite3_free (err_msg);
                    
                    return -1;
                }
                
                if(progress != NULL)
                {
                    progress->OnProgress(Tools::GenerateProgressInfo("tips", "安装"), 50.0);
                }
                
                installCount = 0;
            }
        }
        
        if(installCount < 10000 && installCount >0)
        {
            int ret = sqlite3_exec (hDB, "COMMIT", NULL, NULL, &err_msg);
            
            if (ret != SQLITE_OK)
            {
                Logger::LogD ("COMMIT error: %s\n", err_msg);
                Logger::LogO ("COMMIT error: %s\n", err_msg);
                
                sqlite3_free (err_msg);
                
                return -1;
            }
        }
        
        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo("tips", "安装"), 100.0);
        }
        
        if(isInstalled)
        {
            Editor::GridHelper* gh = Editor::GridHelper::getInstance();
            gh->SetDbConnection(hDB);
            
            gh->RefreshGridsTimeStamp(grids, Tools::GetCurrentDateTime());
        }
        
        sqlite3_close_v2(hDB);
        
        spatialite_cleanup_ex(cache_download);
        
        return 0;
    }
    
 ////////////////
    int CoreMapDataSource::InstallTipsEx(std::string sJsonPath, const std::vector<std::string>& grids, ProgressEventer* progress)
    {
        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo("tips", "安装"), 0.0);
        }
        
        std::string sExistSqlite = Editor::DataManager::getInstance()->getFileSystem()->GetCoreMapDataPath();
        
        if(strcmp(sExistSqlite.c_str(),"") == 0)
        {
            Logger::LogD("Editor: InstallTipsEx failed, GetCoreMapData Path Empty");
            Logger::LogO("Editor: InstallTipsEx failed, GetCoreMapData Path Empty");
            return -1;
        }
        
        Open(sExistSqlite);
        
        Editor::DataLayer* layer = GetLayerByType(DATALAYER_TIPS);
        
        if(NULL == layer)
        {
            Logger::LogD("Editor: InstallTipsEx failed, layer is null, maybe datasource is not open");
            Logger::LogO("Editor: InstallTipsEx failed, layer is null, maybe datasource is not open");
            return -1;
        }
        
        FieldDefines* fd = layer->GetFieldDefines();
        
        if(NULL == fd || fd->GetColumnCount() == 0)
        {
            Logger::LogD("Editor: InstallTipsEx failed, FieldDefines is null, maybe datasource is not open");
            Logger::LogO("Editor: InstallTipsEx failed, FieldDefines is null, maybe datasource is not open");
            return -1;
        }
        
        int rowKeyIndex = fd->GetColumnIndex(RowkeyStr);
        
        if (-1==rowKeyIndex)
        {
            Logger::LogD("InstallTipsEx rowKeyIndex[-1]\n");
            Logger::LogO("InstallTipsEx rowKeyIndex[-1]\n");
            return -1;
        }
        
        int lifecycleIndex = fd->GetColumnIndex(TLifecycle_Str);
        
        if (-1==lifecycleIndex)
        {
            Logger::LogD("InstallTipsEx lifecycleIndex[-1]\n");
            Logger::LogO("InstallTipsEx lifecycleIndex[-1]\n");
            return -1;
        }
        
        int useidIndex = fd->GetColumnIndex(THandler_Str);
        
        if (-1==useidIndex)
        {
            Logger::LogD("InstallTipsEx useidIndex[-1]\n");
            Logger::LogO("InstallTipsEx useidIndex[-1]\n");
            return -1;
        }
        
        int statusIndex = fd->GetColumnIndex(TStatus_Str);
        
        if (-1==statusIndex)
        {
            Logger::LogD("InstallTipsEx statusIndex[-1]\n");
            Logger::LogO("InstallTipsEx statusIndex[-1]\n");
            return -1;
        }
        
        int syncIndex = fd->GetColumnIndex(TSync_Str);
        
        if (-1==syncIndex)
        {
            Logger::LogD("InstallTipsEx syncIndex[-1]\n");
            Logger::LogO("InstallTipsEx syncIndex[-1]\n");
            return -1;
        }
        
        int displayStyleIndex = fd->GetColumnIndex(Display_Style);
        if (-1==displayStyleIndex)
        {
            Logger::LogD("InstallTipsEx displayStyleIndex[-1]\n");
            Logger::LogO("InstallTipsEx displayStyleIndex[-1]\n");
            return -1;
        }
        
        int sourceTypeIndex = fd->GetColumnIndex(Tips_Column_SsourceType);
        if (-1==sourceTypeIndex)
        {
            Logger::LogD("InstallTipsEx sourceTypeIndex[-1]\n");
            Logger::LogO("InstallTipsEx sourceTypeIndex[-1]\n");
            return -1;
        }
        
        const DataFeatureFactory* factory = DataFeatureFactory::getDefaultInstance();
        
        std::ifstream in;
        
        in.open(sJsonPath.c_str());
        
        if(!in)
        {
            Logger::LogD("error :Open File Failed, Json File Path: %s",sJsonPath.c_str());
            Logger::LogO("error :Open File Failed, Json File Path: %s",sJsonPath.c_str());
        }
        
        std::string line;
        
        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo("tips", "安装"), 10.0);
        }
        
        char *err_msg = NULL;
        int installCount = 0;
        bool isInstalled = false;
        while(getline(in, line))
        {
            if(installCount == 0)
            {
                int ret =  BeginTranscation();
                if (ret != 0)
                {
                    Logger::LogD("InstallTipsEx BEGIN error: %s\n", sqlite3_errmsg (m_Sqlite3Db));
                    Logger::LogO("InstallTipsEx BEGIN error: %s\n", sqlite3_errmsg (m_Sqlite3Db));
                    Close(sExistSqlite);
                    return -1;
                }
            }
            
            DataFeature* featureTips = factory->CreateFeature(layer);
            Editor::JSON jsonObject;
            jsonObject.SetJsonString(line);
            featureTips->SetTotalPart(jsonObject);
            
            std::string rowkey = featureTips->GetAsString(rowKeyIndex);
            if(!rowkey.empty())
            {
                installCount++;
                
                std::string displaystyle = DataTransfor::GetTipsDisplayStyle(line);
                featureTips->SetAsString(displayStyleIndex, displaystyle);
                
                std::string type = featureTips->GetAsString(sourceTypeIndex);
                if(type == Tools::NumberToString(Model::Bridge) || type == Tools::NumberToString(Model::UnderConstruction))
                {
                    std::string geoWkt = DataTransfor::GetSETipsGeo(line);
                    
                    if(strcmp(geoWkt.c_str(),"") != 0)
                    {
                        EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(geoWkt);
                        
                        FieldDefines* fieldDefines = featureTips->GetFieldDefines();
                        
                        int index = fieldDefines->GetColumnIndex(GLocation_Str);
                        
                        if(index != -1)
                        {
                            featureTips->SetAsWkb(index, wkb);
                        }
                        
                        delete[] wkb;
                    }
                }
                
                int newlifecycle = featureTips->GetAsInteger(lifecycleIndex);
                if(newlifecycle == LIFECYCLE_ORIGINAL || newlifecycle == LIFECYCLE_UPDATE ||
                   newlifecycle == LIFECYCLE_ADD)
                {
                    featureTips->SetAsInteger(lifecycleIndex, 0);
                    featureTips->SetAsInteger(statusIndex, 0);
                    featureTips->SetAsInteger(useidIndex, 0);
                }
                else if(newlifecycle == LIFECYCLE_DELETE)
                {
                    featureTips->SetAsInteger(lifecycleIndex, 1);
                    featureTips->SetAsInteger(statusIndex, 0);
                    featureTips->SetAsInteger(useidIndex, 0);
                }
                featureTips->SetAsInteger(syncIndex, 1);
                
                Model::Tips localTips = IsTipsExistInDatabase(m_Sqlite3Db, "edit_tips", "rowkey", rowkey);
                if(strcmp(localTips.rowkey.c_str(), "") != 0)
                {
                    if(localTips.t_lifecycle != LIFECYCLE_ORIGINAL)
                    {
                        featureTips->SetAsInteger(statusIndex, localTips.t_status);
                        featureTips->SetAsInteger(useidIndex, localTips.t_handler);
                    }
                    
                    int result = layer->UpdateFeature(featureTips);
                    if(result != 0)
                    {
                        DataFeatureFactory::getDefaultInstance()->DestroyFeature(featureTips);
                        continue;
                    }
                    
                    result = APIWrapper::getInstance()->UpdateTipsGeo(featureTips);
                    DataFeatureFactory::getDefaultInstance()->DestroyFeature(featureTips);
                    
                }
                else
                {
                    int result = layer->InsertFeature(featureTips);
                    if(result == -1)
                    {
                        DataFeatureFactory::getDefaultInstance()->DestroyFeature(featureTips);
                        continue;
                    }
                    
                    result = APIWrapper::getInstance()->InsertTipsGeo(featureTips);
                    DataFeatureFactory::getDefaultInstance()->DestroyFeature(featureTips);
                }
                isInstalled = true;
            }
            
            if(installCount == 10000)
            {
                int ret = EndTranscation();
                
                if (ret != SQLITE_OK)
                {
                    Logger::LogD ("COMMIT error: %s\n", err_msg);
                    Logger::LogO ("COMMIT error: %s\n", err_msg);
                    
                    sqlite3_free (err_msg);
                    
                    return -1;
                }
                
                if(progress != NULL)
                {
                    progress->OnProgress(Tools::GenerateProgressInfo("tips", "安装"), 50.0);
                }
                
                installCount = 0;
            }
        }
        
        if(installCount < 10000 && installCount >0)
        {
            int ret = EndTranscation();
            
            if (ret != SQLITE_OK)
            {
                Logger::LogD ("COMMIT error: %s\n", err_msg);
                Logger::LogO ("COMMIT error: %s\n", err_msg);
                
                sqlite3_free (err_msg);
                
                return -1;
            }
        }
        
        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo("tips", "安装"), 100.0);
        }
        
        if(isInstalled)
        {
            Editor::GridHelper* gh = Editor::GridHelper::getInstance();
            gh->SetDbConnection(m_Sqlite3Db);
            
            gh->RefreshGridsTimeStamp(grids, Tools::GetCurrentDateTime());
        }
        
        Close(sExistSqlite);
        return 0;
    }
///////////////////////
    
    int CoreMapDataSource::InstallPatternImage(std::string sFilePath, std::string sFlag, ProgressEventer* progress)
    {
        std::string sPathFolderFileTmp = sFilePath;
        
        Editor::DataManager* pDataManager = Editor::DataManager::getInstance();
        
        Editor::DataFileSystem* pFileSystem = pDataManager->getFileSystem();
        
        std::string sPath = pFileSystem->GetCoreMapDataPath();
        
        sqlite3* hDB = openSqlite(sPath);
        
        void* cache = spatialite_alloc_connection();
        spatialite_init_ex(hDB, cache, 0);
        char* err_msg = NULL;
        
        std::string sql = "attach '" + sFilePath + "' as newDb";
        
        int ret = sqlite3_exec(hDB, sql.c_str(), NULL, NULL, &err_msg);
        
        if (ret != SQLITE_OK)
        {
            Logger::LogD("attach  error: %s\n", err_msg);
            Logger::LogO("attach  error: %s\n", err_msg);
            
            sqlite3_free(err_msg);
            
            spatialite_cleanup_ex(cache);
            closeSqlite(hDB);
            return -1;
        }
        
        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "安装"), 5.0);
        }
        
        ret = sqlite3_exec (hDB, "BEGIN", NULL, NULL, &err_msg);
        
        if (ret != SQLITE_OK)
        {
            Logger::LogD("InsertSqliteTable BEGIN error: %s\n", err_msg);
            Logger::LogO("InsertSqliteTable BEGIN error: %s\n", err_msg);
            
            sqlite3_free(err_msg);
            
            spatialite_cleanup_ex(cache);
            closeSqlite(hDB);
            return -1;
        }
        
        sql = "insert or replace into meta_JVImage (name,format,content,bType,mType,userId,operateDate,uploadDate,downloadDate,status) select "
        "name,format,content,bType,mType,userId,operateDate,uploadDate,downloadDate,status from newDb.meta_JVImage;";
        
        ret = sqlite3_exec(hDB, sql.c_str(), NULL, NULL, &err_msg);
        
        if (ret != SQLITE_OK)
        {
            Logger::LogD("insert or replace  error: %s\n", err_msg);
            Logger::LogO("insert or replace  error: %s\n", err_msg);
            
            sqlite3_free(err_msg);
            
            spatialite_cleanup_ex(cache);
            closeSqlite(hDB);
            return -1;
        }
        
        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "安装"), 50.0);
        }
        
        ret = sqlite3_exec(hDB, "COMMIT", NULL, NULL, &err_msg);
        
        if (ret != SQLITE_OK)
        {
            Logger::LogD("COMMIT  error: %s\n", err_msg);
            Logger::LogO("COMMIT  error: %s\n", err_msg);
            
            sqlite3_free(err_msg);
            closeSqlite(hDB);
            return -1;
        }
        
        spatialite_cleanup_ex(cache);
        cache=NULL;
        closeSqlite(hDB);
        
        std::string sDelPath = pFileSystem->GetDownloadPath();
        
        Tools::DelDir(sDelPath.c_str());
        
        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "安装"), 100.0);
        }
        
        return 0;
    }
    
    std::string CoreMapDataSource::PackageTips(std::string userId, std::string wkt, std::string timestamp, int& dataCount, std::set<std::string>& failedFeedbacks, ProgressEventer* progress)
    {
        DataFileSystem* fileSystem = DataManager::getInstance()->getFileSystem();
        
        fileSystem->SetCurrentUser(userId.c_str());
        
        std::string dataPath = fileSystem->GetCoreMapDataPath();
        
        if (0 != access(dataPath.c_str(), R_OK))
        {
            Logger::LogD("dataPath [%s] access failed!", dataPath.c_str());
            Logger::LogO("dataPath [%s] access failed!", dataPath.c_str());
            
            return "";
        }
        
        sqlite3* db = NULL;
        
        int rc = sqlite3_open_v2(dataPath.c_str(), &db, SQLITE_OPEN_READONLY, NULL);
        
        if (rc != SQLITE_OK)
        {
            sqlite3_close_v2(db);
            
            db = NULL;
            
            Logger::LogD("sqlite [%s] open failed! errmsg [%s]", dataPath.c_str(), sqlite3_errmsg(db));
            Logger::LogO("sqlite [%s] open failed! errmsg [%s]", dataPath.c_str(), sqlite3_errmsg(db));
            
            return "";
        }
        
        char outPath[PATH_MAX];
        
        char outPathFolder[PATH_MAX];
        
        ::sprintf(outPath, "%s/tips_%s_%s", fileSystem->GetUploadPath().c_str(), userId.c_str(), Tools::GetCurrentDateTime().c_str());
        
        ::sprintf(outPathFolder, "%s/", outPath);
        
        bool rst = Tools::CheckFilePath(outPathFolder);
        
        if (!rst)
        {
            sqlite3_close_v2(db);
            
            db = NULL;
            
            Logger::LogD("CheckFilePath failed [%s]! msg [%s]", outPath, strerror(errno));
            Logger::LogO("CheckFilePath failed [%s]! msg [%s]", outPath, strerror(errno));
            
            return "";
        }
        
        Logger::LogD("PackageTips begin, userId [%s], wkt [%s], timestamp [%s]", userId.c_str(), wkt.c_str(), timestamp.c_str());
        Logger::LogO("PackageTips begin, userId [%s], wkt [%s], timestamp [%s]", userId.c_str(), wkt.c_str(), timestamp.c_str());
        
        //lee add
        void* cache = spatialite_alloc_connection();
        spatialite_init_ex(db, cache, 0);
        
        char jsonFile[PATH_MAX];
        
        ::sprintf(jsonFile, "%s/tips.txt", outPath);
        
        std::ofstream out;
        
        out.open(jsonFile, std::ios::out|std::ios::binary|std::ios::trunc);
        
        int ret = PackageTipsFromTip(&out, db, wkt, outPath, failedFeedbacks);
        
        //ret = PackageTipsFromGPSLine(&out, db, wkt, outPath, failedFeedbacks);
        
        out.close();
        
        char JsonImageFile[PATH_MAX];
        
        ::sprintf(JsonImageFile, "%s/JVImage.txt", outPath);
        
        out.open(JsonImageFile, std::ios::out|std::ios::binary|std::ios::trunc);
        
        ret = PackagePatternImage(&out, db);
        
        dataCount = Tools::CountLines(JsonImageFile);
        
        if(dataCount == 0)
        {
            remove(JsonImageFile);
        }
        
        dataCount = Tools::CountLines(jsonFile);
        
        std::string zipPath = "";
        
        if (dataCount > 0)
        {
            zipPath = Tools::ZipFolderPath(outPath, "");
        }
        
        Tools::DelDir(outPath);
        
        sqlite3_close_v2(db);
        
        db = NULL;
        
        //lee add
        spatialite_cleanup_ex(cache);
        cache=NULL;
        
        Logger::LogD("PackageTips end, userId [%s], wkt [%s], timestamp [%s], zip [%s], dataCount [%d], failedAttachments [%d]", userId.c_str(), wkt.c_str(), timestamp.c_str(), zipPath.c_str(), dataCount, failedFeedbacks.size());
        Logger::LogO("PackageTips end, userId [%s], wkt [%s], timestamp [%s], zip [%s], dataCount [%d], failedAttachments [%d]", userId.c_str(), wkt.c_str(), timestamp.c_str(), zipPath.c_str(), dataCount, failedFeedbacks.size());
        
        return zipPath;
    }
    
    int CoreMapDataSource::InsertGpsLineTip( const std::string& tipJson )
    {
        std::string sExistSqlite = Editor::DataManager::getInstance()->getFileSystem()->GetCoreMapDataPath();
        
        sqlite3* hDB = openSqlite(sExistSqlite);
        
        if(hDB == NULL)
        {
            return -1;
        }
        
        void* cache = spatialite_alloc_connection();
        
        spatialite_init_ex (hDB, cache, 0);
        
        char* err_msg = NULL;
        int ret = sqlite3_exec (hDB, "BEGIN", NULL, NULL, &err_msg);
        
        if (ret != SQLITE_OK)
        {
            Logger::LogD("InsertGpsLineTip BEGIN error: %s\n", err_msg);
            Logger::LogO("InsertGpsLineTip BEGIN error: %s\n", err_msg);
            
            sqlite3_free(err_msg);
            return -1;
        }
        
        Model::Tips appTips;
        appTips.setDataSource(Model::Tips::DataSourceFromApp);
        appTips.ParseJson(tipJson);
        
        //lee add
        std::vector<Model::tips_geo*> geosVector;
        geosVector = appTips.BuildGeosBySourceType();
        std::vector<Model::tips_geo*>::iterator itor = geosVector.begin();
        while (itor!=geosVector.end())
        {
            if (*itor)
            {
                appTips.Attach(*itor);
            }
            ++itor;
        }
        
        appTips.SetDisplayStyle(tipJson);
        
        std::string currentTime = Tools::GetCurrentDateTime();
        appTips.t_operateDate = currentTime;
        
        appTips.InsertSqliteTable(hDB);
        
        //lee add
        appTips.Notify(hDB,Model::tips_geo::operatorInsert);
        
        ret = sqlite3_exec (hDB, "COMMIT", NULL, NULL, &err_msg);
        
        if (ret != SQLITE_OK)
        {
            Logger::LogD ("COMMIT error: %s\n", err_msg);
            Logger::LogO ("COMMIT error: %s\n", err_msg);
            
            sqlite3_free (err_msg);
            
            return -1;
        }
        
        sqlite3_close_v2(hDB);
        
        spatialite_cleanup_ex(cache);
        return 0;
    }
    
    std::string CoreMapDataSource::GetCoremapModelVersion(int type)
    {
        Editor::DataManager* pDataManager = Editor::DataManager::getInstance();
        
        Editor::DataFileSystem* pFileSystem = pDataManager->getFileSystem();
        
        std::string sPath = pFileSystem->GetCoreMapDataPath();
        
        sqlite3* hDB = openSqlite(sPath);
        
        void* cache = spatialite_alloc_connection();
        
        spatialite_init_ex(hDB, cache, 0);
        
        std::string sql = "select version from meta_Parameter where type=" + Tools::NumberToString<int>(type);
        
        sqlite3_stmt* stmt = NULL;
        
        int rc = sqlite3_prepare_v2(hDB, sql.c_str(), -1, &stmt, NULL);
        
        if(rc != SQLITE_OK)
        {
            Logger::LogD("DataSource : GetCoremapModelVersion : failed");
            Logger::LogO("DataSource : GetCoremapModelVersion : failed");
            return "";
        }
        
        std::string version = "";
        
        if(sqlite3_step(stmt) == SQLITE_ROW)
        {
            version = (const char*)sqlite3_column_text(stmt, 0);
        }
        
        sqlite3_finalize(stmt);
        
        spatialite_cleanup_ex(cache);
        
        cache=NULL;
        
        sqlite3_close(hDB);
        
        return version;
    }
    
    Model::Tips CoreMapDataSource::IsTipsExistInDatabase(sqlite3* db, std::string tableName, std::string key, std::string keyID)
    {
        Model::Tips tips;
        
        sqlite3_stmt* stmt = NULL;
        
        std::string sql_select_count = "select t_lifecycle, t_status, t_handler from " + tableName + " where " + key + "='" + keyID + "'";
        
        int rc = sqlite3_prepare_v2(db, sql_select_count.c_str(), -1, &stmt, NULL);
        
        if(rc != SQLITE_OK)
        {
            return tips;
        }
        
        if(sqlite3_step(stmt) != SQLITE_ROW)
        {
            return tips;
        }
        
        tips.rowkey = "rowkey";
        tips.t_lifecycle = sqlite3_column_int(stmt, 0);
        tips.t_status = sqlite3_column_int(stmt, 1);
        tips.t_handler = sqlite3_column_int(stmt, 2);
        
        sqlite3_finalize(stmt);
        
        return tips;
    }
    
    int CoreMapDataSource::PackageTips(std::ofstream* out, sqlite3_stmt* stmt, const char* outPath, std::set<std::string>& failedFeedbacks)
    {
        Document document;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();
        
        Value each_json_key(kStringType);
        
        Value each_json_value(kStringType);
        
        std::string each_str_value;
        
        Document each_document_value;
        
        Value each_json_null(kNullType);
        
        Value each_array_value(kArrayType);
        
        int nColCount = sqlite3_column_count(stmt);
        
        for (int i=0; i<nColCount; i++)
        {
            int type = sqlite3_column_type(stmt, i);
            
            const char* name = sqlite3_column_name(stmt, i);
            
            each_json_key.SetString(name, strlen(name), allocator);
            
            switch (type)
            {
                case SQLITE_INTEGER:
                {
                	if(strcmp(name, "t_sync") == 0)
					{
						continue;
					}
                    int value = sqlite3_column_int(stmt, i);
                    document.AddMember(each_json_key, value, allocator);
                }
                    break;
                case SQLITE_FLOAT:
                {
                    double value = sqlite3_column_double(stmt, i);
                    document.AddMember(each_json_key, value, allocator);
                }
                    break;
                case SQLITE_BLOB:
                {
                    int length = sqlite3_column_bytes(stmt, i);
                    
                    const void* buff = sqlite3_column_blob(stmt, i);
                    
                    if (0 == strcmp(name, "g_location"))
                    {
                        EditorGeometry::WkbGeometry* wkb = EditorGeometry::SpatialiteGeometry::ToWKBGeometry((EditorGeometry::SpatialiteGeometry*)buff);
                        
                        each_str_value = DataTransfor::Wkb2GeoJson(wkb);
                        
                        each_document_value.Parse(each_str_value.c_str());
                        
                        document.AddMember(each_json_key, each_document_value, allocator);
                        
                        free(wkb);
                    }
                    else if (0 == strcmp(name, "attachments"))
                    {
                        if (0 == length)
                        {
                            document.AddMember(each_json_key, each_array_value, allocator);
                        }
                        else
                        {
                            std::string strBlob = std::string((char*)buff, length);
                            
                            if (0 == strcmp(Tools::ToLower(strBlob).c_str(), "null"))
                            {
                                document.AddMember(each_json_key, each_array_value, allocator);
                            }
                            else
                            {
                                each_document_value.Parse((char*)buff);
                                
                                if (each_document_value.IsArray())
                                {
                                    for (int k=0; k<each_document_value.Size(); k++)
                                    {
                                        const rapidjson::Value& object = each_document_value[k];
                                        
                                        int type = object["type"].GetInt();
                                        
                                        if (type == 3)
                                            continue;
                                        
                                        std::string content = object["content"].GetString();
                                        
                                        PackageTipsPhotos(content, outPath, failedFeedbacks);
                                    }
                                }
                                
                                document.AddMember(each_json_key, each_document_value, allocator);
                            }
                        }
                    }
                    else
                    {
                        if (0 == length)
                        {
                            document.AddMember(each_json_key, each_json_null, allocator);
                        }
                        else
                        {
                            std::string strBlob = std::string((char*)buff, length);
                            
                            if (0 == strcmp(Tools::ToLower(strBlob).c_str(), "null"))
                            {
                                document.AddMember(each_json_key, each_json_null, allocator);
                            }
                            else
                            {
                                each_document_value.Parse((char*)buff);
                                
                                document.AddMember(each_json_key, each_document_value, allocator);
                            }
                        }
                    }
                }
                    break;
                case SQLITE_TEXT:
                {
                    int length = sqlite3_column_bytes(stmt, i);
                    
                    const unsigned char* value = sqlite3_column_text(stmt, i);
                    
                    if (0 == strcmp(name, "g_guide"))
                    {
                        EditorGeometry::WkbPoint* point = (EditorGeometry::WkbPoint*)DataTransfor::Wkt2Wkb(std::string((char*)value, length));
                        
                        if (point == NULL)
                        {
                            continue;
                        }
                        
                        each_str_value = "{\"type\":\"Point\",\"coordinates\":[" + Tools::NumberToString(point->_point._x) + "," + Tools::NumberToString(point->_point._y) + "]}";
                        
                        delete[] point;
                        
                        each_document_value.Parse(each_str_value.c_str());
                        
                        document.AddMember(each_json_key, each_document_value, allocator);
                    }
                    else if (0 == strcmp(name, "deep"))
                    {
                        each_document_value.Parse((char*)value);
                        
                        document.AddMember(each_json_key, each_document_value, allocator);
                    }
                    else if (0 == strcmp(name, "display_style"))
                    {
                        continue;
                    }
                    else
                    {
                        if (0 == length)
                        {
                            document.AddMember(each_json_key, each_json_null, allocator);
                        }
                        else
                        {
                            std::string strText = std::string((char*)value, length);
                            
                            if (0 == strcmp(Tools::ToLower(strText).c_str(), "null"))
                            {
                                document.AddMember(each_json_key, each_json_null, allocator);
                            }
                            else
                            {
                                each_json_value.SetString((char*)value, length, allocator);
                                
                                document.AddMember(each_json_key, each_json_value, allocator);
                            }
                        }
                    }
                }
                    break;
                case SQLITE_NULL:
                    document.AddMember(each_json_key, each_json_null, allocator);
                    break;
                default:
                    break;
            }
        }
        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<StringBuffer> writer(buffer);
        document.Accept(writer);
        std::string jsonString = buffer.GetString();
        
        *out << jsonString << std::endl;
        
        return 0;
    }
    
    int CoreMapDataSource::PackageTipsFromTip(std::ofstream* out, sqlite3* db, std::string wkt, const char* outPath, std::set<std::string>& failedFeedbacks)
    {
        std::string sql = "select * from edit_tips where t_status=1 and (Within(g_location, GeomFromText('" + wkt + "')) or Crosses(g_location, GeomFromText('" + wkt + "'))) and rowid in (select rowid from SpatialIndex s where s.f_table_name='edit_tips' and s.search_frame=g_location)";
        
        //        std::string sql = "select * from edit_tips where t_status=1;";
        
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
        
        if (rc != SQLITE_OK)
        {
            Logger::LogD("PackageTips sql [%s] failed! msg [%s]", sql.c_str(), sqlite3_errmsg(db));
            Logger::LogO("PackageTips sql [%s] failed! msg [%s]", sql.c_str(), sqlite3_errmsg(db));
            
            return -1;
        }
        
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            PackageTips(out, stmt, outPath, failedFeedbacks);
        }
        
        sqlite3_finalize(stmt);
        
        return 0;
    }
    
    void CoreMapDataSource::PackageTipsPhotos(std::string content, const char* outPath, std::set<std::string>& failedFeedbacks)
    {
        std::string srcPhotoFolderPath = DataManager::getInstance()->getFileSystem()->GetTipsPhotoPath();
        
        std::string destFolder = std::string(outPath);
        
        std::string srcFile = srcPhotoFolderPath + content;
        
        std::string destFile = destFolder + "/photo/" + content;
        
        bool flag = false;
        
        for (int i=0; i<3; i++)
        {
            int rst = Tools::CopyFile(srcFile.c_str(), destFile.c_str());
            
            if ((rst != 0) && (i == 0))
            {
                Logger::LogD("PackageTipsPhotos CopyFile from [%s] to [%s] Failed! rst [%d]", srcFile.c_str(), destFile.c_str(), rst);
                Logger::LogO("PackageTipsPhotos CopyFile from [%s] to [%s] Failed! rst [%d]", srcFile.c_str(), destFile.c_str(), rst);
            }
            
            if ((rst == 0) || (rst == -1))
            {
                flag = true;
                
                break;
            }
        }
        
        if (!flag)
        {
            failedFeedbacks.insert(content);
        }
    }
    
    int CoreMapDataSource::PackageTipsFromGPSLine(std::ofstream* out, sqlite3* db, std::string wkt, const char* outPath, std::set<std::string>& failedFeedbacks)
    {
        std::set<std::string> setTips = SelectRowkeyFromEditGPSLine(db, wkt);
        
        std::set<std::string>::const_iterator iter = setTips.begin();
        
        for(; iter != setTips.end(); iter++)
        {
            std::string updateSql = "select * from edit_tips where rowkey = '" + *iter + "'";
            
            sqlite3_stmt* stmt = NULL;
            
            int rc = sqlite3_prepare_v2(db, updateSql.c_str(), -1, &stmt, NULL);
            
            if (rc != SQLITE_OK)
            {
                Logger::LogD("select %s unsuccessfully bacause of [%s]", "edit_tips", sqlite3_errmsg(db));
                Logger::LogO("select %s unsuccessfully bacause of [%s]", "edit_tips", sqlite3_errmsg(db));
                
                sqlite3_finalize(stmt);
                
                return -1;
            }
            
            if (sqlite3_step(stmt) != SQLITE_DONE)
            {
                PackageTips(out, stmt, outPath, failedFeedbacks);
            }
            
            sqlite3_finalize(stmt);
        }
        
        return 0;
    }
    
    int CoreMapDataSource::PackagePatternImage(std::ofstream* out, sqlite3* db)
    {
        std::string sql = "select * from meta_JVImage where status = 2";
        
        sqlite3_stmt* stmt = NULL;
        
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
        
        if (rc != SQLITE_OK)
        {
            Logger::LogD("select %s unsuccessfully bacause of [%s]", "meta_JVImage", sqlite3_errmsg(db));
            Logger::LogO("select %s unsuccessfully bacause of [%s]", "meta_JVImage", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            
            return -1;
        }
        
        std::set<std::string> setTmp;
        
        while(sqlite3_step(stmt) != SQLITE_DONE)
        {
            PackageTips(out, stmt, "", setTmp);
        }
        
        sqlite3_finalize(stmt);
        
        return 0;
    }
    
    std::set<std::string> CoreMapDataSource::SelectRowkeyFromEditGPSLine(sqlite3* db, std::string wkt)
    {
        std::set<std::string> setTips;
        
        std::string sql = "select * from tips_line where sourceType = 2001 and Intersects(geometry, GeomFromText('" + wkt + "')) and rowid in (select rowid from SpatialIndex s where s.f_table_name='tips_line' and s.search_frame=geometry)";
        
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
        
        if (rc != SQLITE_OK)
        {
            Logger::LogD("PackageTips sql [%s] failed! msg [%s]", sql.c_str(), sqlite3_errmsg(db));
            Logger::LogO("PackageTips sql [%s] failed! msg [%s]", sql.c_str(), sqlite3_errmsg(db));
            
            return setTips;
        }
        
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::string rowkey = (const char*)sqlite3_column_text(stmt, 0);
            
            int geo_length = sqlite3_column_bytes(stmt, 8);
            const void* geo_buff = sqlite3_column_blob(stmt, 8);
            EditorGeometry::WkbPoint* wkb = (EditorGeometry::WkbPoint*)EditorGeometry::SpatialiteGeometry::ToWKBGeometry((EditorGeometry::SpatialiteGeometry*)geo_buff);
            
            std::string point = DataTransfor::Wkb2Wkt(wkb);
            
            if(isWithinGrid(point, wkt) == true)
            {
                setTips.insert(rowkey);
            }
        }
        
        sqlite3_finalize(stmt);
        
        return setTips;
    }
    
    //Gdb dataSource
    GdbDataSource::GdbDataSource():DataSource()
    {
        InitializeDataLayer();
        
        this->m_dataSourceType = DATASOURCE_GDB;
    }
    
    GdbDataSource::~GdbDataSource()
    {
        
    }
    
    void GdbDataSource::InitializeDataLayer()
    {
        DataLayer* rdlineLayer = new DataLayer();
        
        rdlineLayer->SetDataLayerType(DATALAYER_RDLINE);
        
        m_pDataLayers.push_back(rdlineLayer);
        
        DataLayer* faceLayer = new DataLayer();
        
        faceLayer->SetDataLayerType(DATALAYER_FACE);
        
        m_pDataLayers.push_back(faceLayer);
        
        DataLayer* rdNodeLayer = new DataLayer();
        
        rdNodeLayer->SetDataLayerType(DATALAYER_RDNODE);
        
        m_pDataLayers.push_back(rdNodeLayer);
        
        //立交桥
        DataLayer* rdLinegscLayer = new DataLayer();
        
        rdLinegscLayer->SetDataLayerType(DATALAYER_RDLINE_GSC);
        
        m_pDataLayers.push_back(rdLinegscLayer);
        
        //gdb_bkLine
        DataLayer* bkLineLayer = new DataLayer();
        
        bkLineLayer->SetDataLayerType(DATALAYER_BKLINE);
        
        m_pDataLayers.push_back(bkLineLayer);
        
        
    }
    
    int GdbDataSource::InstallGDB(std::string sFilePath, std::string sFlag, ProgressEventer* progress)
    {
        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "安装"), 0.0);
        }
        
        int nBeg = sFilePath.find_last_of(".zip");
        
        if(!(nBeg != std::string::npos && nBeg == sFilePath.length() - 1))
        {
            return -1;
        }
        
        std::string sPathFolderTmp = sFilePath.substr(0, sFilePath.length() - 4);
        
#ifdef WIN32
        std::string sPathFolderFileTmp = sPathFolderTmp + "\\tmp.txt";
#else
        std::string sPathFolderFileTmp = sPathFolderTmp + "/tmp.txt";
#endif
        
        if(Tools::CheckFilePath(sPathFolderFileTmp.c_str()) == false)
        {
            return -1;
        }
        
        sPathFolderTmp = Tools::UnZipDir(sFilePath.c_str(), sPathFolderTmp.c_str());
        
        if(sPathFolderTmp.empty())
        {
            Logger::LogD("InstallGDB error :Zip File Failed, Json File Path: %s",sPathFolderTmp.c_str());
            Logger::LogO("InstallGDB error :Zip File Failed, Json File Path: %s",sPathFolderTmp.c_str());
            
            return -1;
        }
        
#ifdef WIN32
        sPathFolderFileTmp = sPathFolderTmp + "\\rdline.txt";
#else
        sPathFolderFileTmp = sPathFolderTmp + "/rdline.txt";
#endif
        
        if (0 == access(sPathFolderFileTmp.c_str(), F_OK))
        {
            return InstallGDBFromJson(sPathFolderFileTmp, sFlag, progress);
        }
        
#ifdef WIN32
        sPathFolderFileTmp = sPathFolderTmp + "\\" + GdbDownloadFileName;
#else
        sPathFolderFileTmp = sPathFolderTmp + "/" + GdbDownloadFileName;
#endif
        
        if(0 == access(sPathFolderFileTmp.c_str(), F_OK))
        {
            return InstallGDBFromSqlite(sPathFolderFileTmp, sFlag, progress);
        }
        
        return 0;
    }
    
    int GdbDataSource::InstallGDBFromSqlite(std::string sFilePath, std::string sFlag, ProgressEventer* progress)
    {
        Tools::WatchTimerStart();
        
        std::string sPathFolderFileTmp = sFilePath;
        
        Editor::DataManager* pDataManager = Editor::DataManager::getInstance();
        
        Editor::DataFileSystem* pFileSystem = pDataManager->getFileSystem();
        
        std::string sPath = pFileSystem->GetTempGDBDataFile();
        
        sqlite3* hDB = openSqlite(sPath);
        
        if (hDB==NULL)
        {
            if(progress != NULL)
            {
                progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "GDB安装失败"), -1);
            }
            return -1;
        }
        
        void* cache = spatialite_alloc_connection();
        spatialite_init_ex(hDB, cache, 0);
        char* err_msg = NULL;
        
        std::string sql = "attach '" + sFilePath + "' as newDb";
        
        int ret = sqlite3_exec(hDB, sql.c_str(), NULL, NULL, &err_msg);
        
        if (ret != SQLITE_OK)
        {
            Logger::LogD("attach  error: %s\n", err_msg);
            Logger::LogO("attach  error: %s\n", err_msg);
            
            sqlite3_free(err_msg);
            closeSqlite(hDB);
            if(progress != NULL)
            {
                progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "GDB安装失败"), -1);
            }
            return -1;
        }
        
        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "安装"), 5.0);
        }
        
        ret = sqlite3_exec (hDB, "BEGIN", NULL, NULL, &err_msg);
        
        if (ret != SQLITE_OK)
        {
            Logger::LogD("InsertSqliteTable BEGIN error: %s\n", err_msg);
            Logger::LogO("InsertSqliteTable BEGIN error: %s\n", err_msg);
            
            sqlite3_free(err_msg);
            closeSqlite(hDB);
            if(progress != NULL)
            {
                progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "GDB安装失败"), -1);
            }
            return -1;
        }
        
        std::map<std::string,std::string> tableName2Fields;
        std::vector<DATALAYER_TYPE> layerTypes;
        layerTypes.push_back(DATALAYER_RDNODE);
        layerTypes.push_back(DATALAYER_RDLINE);
        layerTypes.push_back(DATALAYER_BKLINE);
        layerTypes.push_back(DATALAYER_FACE);
        layerTypes.push_back(DATALAYER_RDLINE_GSC);
        
        Editor::DataManager* dataManager = Editor::DataManager::getInstance();
        std::string gdbDataPath= dataManager->getFileSystem()->GetEditorGDBDataFile();
        
        if(strcmp(gdbDataPath.c_str(),"") == 0)
        {
            Logger::LogD("gdbData Path of map  is Empty");
            Logger::LogO("gdbData Path of map  is Empty");
            return -1;
        }
        
        Open(gdbDataPath);
        for (int i = 0; i<layerTypes.size(); ++i)
        {
            DataLayer* layer = GetLayerByType(layerTypes.at(i));
            if(layer)
            {
                FieldDefines* pFieldDefines=layer->GetFieldDefines();
                std::string layerName=layer->GetTableName();
                if (pFieldDefines)
                {
                    int colCount = pFieldDefines->GetColumnCount();
                    std::string strFieldDefinesSql = "";
                    for (int j = 0; j<colCount; ++j)
                    {
                        if(j==0)
                        {
                            strFieldDefinesSql = pFieldDefines->GetColumnName(j);
                        }
                        else if(j>0)
                        {
                            strFieldDefinesSql += "," + pFieldDefines->GetColumnName(j);
                        }
                    }
                    tableName2Fields.insert(std::make_pair(layerName, strFieldDefinesSql));
                }
            }
        }
        Close(gdbDataPath);
        
        //先执行属性写入
        int index =0;
        for (std::map<std::string,std::string>::iterator itor = tableName2Fields.begin();itor!=tableName2Fields.end();++itor)
        {
            sql = "insert or replace into " + itor->first + "(" + itor->second + ") select " + itor->second + " from newDb." + itor->first;
            ret = sqlite3_exec(hDB, sql.c_str(), NULL, NULL, &err_msg);
            
            if (ret != SQLITE_OK)
            {
                Logger::LogD("insert or replace  error: %s\n", err_msg);
                Logger::LogO("insert or replace  error: %s\n", err_msg);
                
                sqlite3_free(err_msg);
                closeSqlite(hDB);
                if(progress != NULL)
                {
                    progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "GDB安装失败"), -1);
                }
                return -1;
            }
            
            if(progress != NULL)
            {
                progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "安装"), 10.0+40.0*double(index/tableName2Fields.size()));
            }
            ++index;
        }
        
        //后执行空间索引恢复
        for (std::map<std::string,std::string>::iterator itor1 = tableName2Fields.begin();itor1!=tableName2Fields.end();++itor1)
        {
            char buf[256];
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"SELECT RecoverSpatialIndex('%s', 'geometry');", itor1->first.c_str());
            sql = buf;
            ret = sqlite3_exec(hDB, sql.c_str(), NULL, NULL, &err_msg);
            
            if (ret != SQLITE_OK)
            {
                Logger::LogD("RecoverSpatialIndex  error: %s\n", err_msg);
                Logger::LogO("RecoverSpatialIndex  error: %s\n", err_msg);
                
                sqlite3_free(err_msg);
                closeSqlite(hDB);
                if(progress != NULL)
                {
                    progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "GDB安装失败"), -1);
                }
                return -1;
            }

        }
        
        ret = sqlite3_exec(hDB, "COMMIT", NULL, NULL, &err_msg);
        
        if (ret != SQLITE_OK)
        {
            Logger::LogD("COMMIT  error: %s\n", err_msg);
            Logger::LogO("COMMIT  error: %s\n", err_msg);
            
            sqlite3_free(err_msg);
            closeSqlite(hDB);
            if(progress != NULL)
            {
                progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "GDB安装失败"), -1);
            }
            return -1;
        }
        
        spatialite_cleanup_ex(cache);
        cache=NULL;
        closeSqlite(hDB);
        
        std::string sDelPath = pFileSystem->GetDownloadPath();
        
        Tools::DelDir(sDelPath.c_str());
        
        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "安装"), 100.0);
        }
        
        long cost = Tools::WatchTimerStop();
        
        Logger::LogD("cost is [%ld]", cost);
        Logger::LogO("cost is [%ld]", cost);
        
        return 0;
    }
    
    int GdbDataSource::InstallGDBFromJson(std::string sFilePath, std::string sFlag, ProgressEventer* progress)
    {
        Tools::WatchTimerStart();
        
        std::string sPathFolderFileTmp = sFilePath;
        
        Editor::DataManager* pDataManager = Editor::DataManager::getInstance();
        
        Editor::DataFileSystem* pFileSystem = pDataManager->getFileSystem();
        
        std::string sPath = pFileSystem->GetEditorGDBDataFile();
        
        sqlite3* hDB = openSqlite(sPath);
        void* cache = spatialite_alloc_connection();
        spatialite_init_ex(hDB, cache, 0);
        
        std::ifstream in;
        
        in.open(sPathFolderFileTmp.c_str());
        
        if(!in)
        {
            std::cout<<"error : 打开文件失败！"<<std::endl;
            Logger::LogD("error :Open File Failed, Json File Path: %s",sPathFolderFileTmp.c_str());
            Logger::LogO("error :Open File Failed, Json File Path: %s",sPathFolderFileTmp.c_str());
            
            return -1;
        }
        
        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "安装"), 5.0);
        }
        
        std::string line;
        
        char *err_msg = NULL;
        
        int nCount = 0;
        
        while(getline(in, line))
        {
            if(nCount == 0)
            {
                int ret = sqlite3_exec (hDB, "BEGIN", NULL, NULL, &err_msg);
                
                if (ret != SQLITE_OK)
                {
                    Logger::LogD("BEGIN error: %s\n", err_msg);
                    Logger::LogO("BEGIN error: %s\n", err_msg);
                    
                    sqlite3_free(err_msg);
                    sqlite3_close(hDB);
                    return -1;
                }
            }
            
            nCount++;
            
            Model::GDBRdLine rd;
            
            rd.ParseJson(line);
            
            if(rd.pid != 0 && IsExistsInDatabase(hDB, "gdb_rdLine","pid", Tools::NumberToString<int>(rd.pid)) == 0)
            {
                rd.SetDisplayStyle();
                
                rd.UpdateSqliteTableWithoutTransaction(hDB);
            }
            else
            {
                rd.SetDisplayStyle();
                
                rd.InsertSqliteTableWithoutTransaction(hDB);
            }
            
            if(nCount == 10000)
            {
                int ret = sqlite3_exec (hDB, "COMMIT", NULL, NULL, &err_msg);
                
                if (ret != SQLITE_OK)
                {
                    Logger::LogD ("COMMIT error: %s\n", err_msg);
                    Logger::LogO ("COMMIT error: %s\n", err_msg);
                    
                    sqlite3_free (err_msg);
                    sqlite3_close(hDB);
                    return -1;
                }
                
                if(progress != NULL)
                {
                    progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "安装"), 50.0);
                }
                
                nCount = 0;
            }
        }
        
        if(nCount < 10000)
        {
            int ret = sqlite3_exec (hDB, "COMMIT", NULL, NULL, &err_msg);
            
            if (ret != SQLITE_OK)
            {
                Logger::LogD ("COMMIT error: %s\n", err_msg);
                Logger::LogO ("COMMIT error: %s\n", err_msg);
                
                sqlite3_free (err_msg);
                sqlite3_close(hDB);
                return -1;
            }
        }
        
        spatialite_cleanup_ex(cache);
        cache=NULL;
        sqlite3_close(hDB);
        
        std::string sDelPath = pFileSystem->GetDownloadPath();
        
        Tools::DelDir(sDelPath.c_str());
        
        if(progress != NULL)
        {
            progress->OnProgress(Tools::GenerateProgressInfo(sFlag, "安装"), 100.0);
        }
        
        long cost = Tools::WatchTimerStop();
        
        Logger::LogD("cost is [%ld]", cost);
        Logger::LogO("cost is [%ld]", cost);
        
        return 0;
    }
    
    //Gdb dataSource
    ProjectDataSource::ProjectDataSource():DataSource()
    {
        InitializeDataLayer();
        
        this->m_dataSourceType = DATASOURCE_PROJECT;
    }
    
    ProjectDataSource::~ProjectDataSource()
    {
        
    }
    
    void ProjectDataSource::InitializeDataLayer()
    {
        DataLayer* puLayer = new DataLayer();
        
        puLayer->SetDataLayerType(DATALAYER_PROJECTUSER);
        
        m_pDataLayers.push_back(puLayer);
        
        DataLayer* projectInfoLayer = new DataLayer();
        
        projectInfoLayer->SetDataLayerType(DATALAYER_PROJECTINFO);
        
        m_pDataLayers.push_back(projectInfoLayer);
        
        DataLayer* taskInfoLayer = new DataLayer();
        
        taskInfoLayer->SetDataLayerType(DATALAYER_TASKINFO);
        
        m_pDataLayers.push_back(taskInfoLayer);
    }
}
