#include "Editor_ProjectManager.h"
#include "DataTransfor.h"

#define SetBit(resultValue,BitPlace,BitValue) ( resultValue=( resultValue&~(1<<BitPlace) )|(BitValue<<BitPlace) )

namespace Editor
{
    GridHelper* GridHelper::m_pInstance = NULL;

    GridHelper::GridHelper()
    {
        DataManager* pDataManager = DataManager::getInstance();

        m_pDataFileSystem = pDataManager->getFileSystem();

        m_db = NULL;

        m_pConnectString = "";

		m_cache=NULL;
    }


    GridHelper* GridHelper::getInstance()
    {
        if(m_pInstance == NULL)
        {
           m_pInstance = new GridHelper();
        }

        return m_pInstance;
    }

    void GridHelper::SetDbConnection(sqlite3* db)
    {
        this->m_db = db;
    }

    int GridHelper::CreateTable(std::string sPath)
    {
        if(this->m_db == NULL)
        {
            return -1;
        }

        int ret = sqlite3_open(sPath.c_str(), &m_db);

        if(ret != SQLITE_OK)
        {
            Logger::LogD("DataSource::sqlite [%s] or [%s] open failed", sPath.c_str(), sqlite3_errmsg(m_db));
            Logger::LogO("DataSource::sqlite [%s] or [%s] open failed", sPath.c_str(), sqlite3_errmsg(m_db));

            sqlite3_close (m_db);
            return -1;
        }

        std::string gridManagerCreateSql = "CREATE TABLE grid_Manager("
			"gridId integer PRIMARY KEY,"
			"coreMapDownloadDate text,"
			"coreMapUploadDate text,"
			"tipsDownloadDate text,"
			"tipsUploadDate text,"
			"tipsGridStatus integer)";

        if(DataTransfor::ExecuteNonQuery(gridManagerCreateSql.c_str(), m_db) == false)
        {
            return -1;
        }

        return 0;
    }

    int GridHelper::CreateOrOpenDatabase()
    {
        std::string coreMapPath = m_pDataFileSystem->GetCoreMapDataPath();

        if(Tools::CheckFilePath(coreMapPath.c_str()) == false)
        {
            Logger::LogD("DataSource::DIRECTORY [%s] NOT EXISTS", coreMapPath.c_str());
            Logger::LogO("DataSource::DIRECTORY [%s] NOT EXISTS", coreMapPath.c_str());

            return -1;
        }

        if (0 == strcmp(Tools::ToUpper(coreMapPath).c_str(), m_pConnectString.c_str()) && m_db != NULL)
		{
			return 1;
		}

        bool isDatabaseExists = false;

#ifdef WIN32            
        if(access(coreMapPath.c_str(), 00) == 0)
        {
            isDatabaseExists = true;
        }
#else
        if(access(coreMapPath.c_str(), R_OK) == 0)
        {
            isDatabaseExists = true;
        }
#endif
        if(isDatabaseExists == false)
        {
            if(CreateTable(coreMapPath) == -1)
            {
                return -1;
            }
        }

        int ret = sqlite3_open_v2(coreMapPath.c_str(), &m_db, SQLITE_OPEN_READWRITE, NULL);

        if(ret != SQLITE_OK)
        {
            Logger::LogD("DataSource::sqlite [%s] or [%s] open failed", coreMapPath.c_str(), sqlite3_errmsg(m_db));
            Logger::LogO("DataSource::sqlite [%s] or [%s] open failed", coreMapPath.c_str(), sqlite3_errmsg(m_db));

            sqlite3_close(m_db);
            return -1;
        }

        m_pConnectString = Tools::ToUpper(coreMapPath);

		//lee add
		if (!m_cache)
		{
			m_cache = spatialite_alloc_connection();
			spatialite_init_ex(this->m_db, m_cache, 0);
		}

        return 1;
    }

    void GridHelper::Close()
    {
        sqlite3_close_v2(m_db);

        m_db = NULL;

        m_pConnectString = "";

		//lee add
		if (m_cache)
		{
			spatialite_cleanup_ex(m_cache);
			m_cache=NULL;
		}
		
    }


    std::string GridHelper::GetGridsTimeStamp(const std::vector<std::string>& grids, int type)
    {
        std::string timeStmap = "";
        
        if(grids.size() == 0 || this->m_db == NULL)
        {
            return timeStmap;
        }
        
        std::stringstream ss;
        ss<<"(";
        for(int i =0 ; i<grids.size(); ++i)
        {
            ss<<grids[i];
            if(i != grids.size()-1)
            {
                ss<<",";
            }
        }
        ss<<")";
        
        std::string gridSql = ss.str();
        
        std::string sql = "";
        
        if(type == TIPS_UP)
        {
            sql = "select min(tipsUploadDate) from grid_Manager where gridid in " + gridSql;
        }
        else
        {
            sql = "select min(tipsDownloadDate) from grid_Manager where gridid in " + gridSql;
        }
        
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, NULL);
        
        if (rc != SQLITE_OK)
        {
            Logger::LogD("select %s failed [%s]", "Grid", sqlite3_errmsg(m_db));
            Logger::LogO("select %s failed [%s]", "Grid", sqlite3_errmsg(m_db));
            
            //Close();
            
            return timeStmap;
        }
        
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int type = sqlite3_column_type(stmt,0);
            
            //[SQLITE_TEXT]
            if(type == 3)
            {
                timeStmap = (const char *)sqlite3_column_text(stmt, 0);
            }
        }
        
        sqlite3_finalize(stmt);
        
        //Close();
        
        return timeStmap;
    }
    
    int GridHelper::RefreshGridsTimeStamp(const std::vector<std::string>& grids, const std::string& timeStamp, int type)
    {
        if(grids.size() == 0 || this->m_db == NULL)
        {
            return -1;
        }
        
        const std::string tableName = "grid_Manager";
        
        std::string columnName = "";
        
        if(type == TIPS_UP)
        {
            columnName = "tipsUploadDate";
        }
        else
        {
            columnName = "tipsDownloadDate";
        }
        
        sqlite3_stmt* stmt;
        
        for(int i =0 ; i<grids.size(); ++i)
        {
            std::string sql = "select "+  columnName + " from " + tableName + " where gridid =" + grids[i];
            
            int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, NULL);
            
            if (rc != SQLITE_OK)
            {
                Logger::LogD("select %s failed [%s]", "Grid", sqlite3_errmsg(m_db));
                Logger::LogO("select %s failed [%s]", "Grid", sqlite3_errmsg(m_db));
                
                Close();
                
                return -1;
            }
            
            if (sqlite3_step(stmt) != SQLITE_ROW)
            {
                if(type == TIPS_UP)
                {
                    sql = "insert into "+  tableName + " values(" + grids[i] + ",'','','','"+  timeStamp + "', 0)";
                }
                else
                {
                    sql = "insert into "+  tableName + " values(" + grids[i] + ",'','','"+  timeStamp + "','', 0)";
                }
            }
            else
            {
                sql = "update "+ tableName +" set " +columnName+ " = '"+timeStamp + "' where gridid = " + grids[i];
            }
            
            sqlite3_reset(stmt);
            
            rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, NULL);
            
            if (rc != SQLITE_OK)
            {
                Logger::LogD("Insert or Update %s failed [%s]", "Grid", sqlite3_errmsg(m_db));
                Logger::LogO("Insert or Update %s failed [%s]", "Grid", sqlite3_errmsg(m_db));
                
                Close();
                
                return -1;
            }
            
            int sqlResult = sqlite3_step(stmt);
            
            if (sqlResult != SQLITE_DONE)
            {
                sqlite3_finalize(stmt);
                
                Close();
                
                return -1;
            }
        }
        
        sqlite3_finalize(stmt);
        
        Close();
        
        return 1;
    }
    
    ///lee add
    /*
     * @brief
     *
     * @para grids
     * @return int :0 succeed, -1 failure
     */
     int GridHelper::UpdateGridStatus(const std::vector<std::string>& grids)
     {
		 if(grids.size() == 0 || this->m_db == NULL)
		 {
			 return 0;
		 }
		 
		 rapidjson::Document doc;
    	 std::string strUpdataGridStatusUrlRoot = DataManager::getInstance()->getServiceAddress()->GetUpdataGridStatusUrlRoot();
    	 int nDbOperationCount = 0;
		 char *err_msg = NULL;
		 int nCommitStepNode = 1000;
		 for(int i=0;i<grids.size();++i)
    	 {
			 if (grids.at(i)=="")continue;
			 ///开启事务
			 if(nDbOperationCount == 0)
			 {
				 int ret = sqlite3_exec (this->m_db, "BEGIN", NULL, NULL, &err_msg);

				 if (ret != SQLITE_OK)
				 {
					 Logger::LogD("InsertSqliteTable BEGIN error: %s\n", err_msg);
					 Logger::LogO("InsertSqliteTable BEGIN error: %s\n", err_msg);

					 sqlite3_free(err_msg);
					 return -1;
				 }
			 }
			 nDbOperationCount++;
			 
			 ///查grid数据本地是否有
			 bool hasFlag = HasGrid(grids.at(i));
			 //firstBitValue是否待上传
			 bool firstBitValue = 0;
			 //secondBitValue是否待下载
			 bool secondBitValue = 0;
			 //当前状态值
			 int iTipsGridStatus = 0;

    		 std::vector<std::string> temv;
    		 temv.push_back(grids.at(i));
    		 std::string strDownTimeStamp = GetGridsTimeStamp(temv, TIPS_DOWN);

			 Logger::LogD("UpdateGridStatus grdid:%s\n",grids.at(i).c_str());

			 ///数据没有下载时间戳，或本地没这条记录，secondBitValue直接为true
			 if (strDownTimeStamp==""||!hasFlag)
			 {
				 secondBitValue = true;
			 }
			 else
			 {
				 char buf[200]={0};
				 sprintf(buf,"%s{\"grid\":\"%s\",\"date\":\"%s\"}",strUpdataGridStatusUrlRoot.c_str(),grids.at(i).c_str(),strDownTimeStamp.c_str());
				 std::string httpStr = buf;
				 std::string strResult;
				 bool flag = Tools::HttpGet(httpStr.c_str(), strResult);

				 Logger::LogD("UpdateGridStatus HttpGet result:%s\n",strResult.c_str());
				 Logger::LogD("UpdateGridStatus HttpGet strDownTimeStamp:%s ,grdid:%s\n",strDownTimeStamp.c_str(),grids.at(i).c_str());
				 if(flag==false)
				 {
					 return -1;
				 }

				 ///通过后台获取下载位标示值secondBitValue
				 std::string errmsg="";
				 std::string strData="";
				 int errcode = 0;
				 doc.Parse<0>(strResult.c_str());
				 if(doc.HasParseError())
				 {
					 return -1;
				 }
				 for(rapidjson::Document::MemberIterator ptr = doc.MemberBegin(); ptr != doc.MemberEnd(); ++ptr)
				 {
					 std::string  sKey = (ptr->name).GetString();
					 rapidjson::Value &valueKey = (ptr->value);
					 std::string sValue = "";
					 if((ptr->value).IsNull())
					 {
						 continue;
					 }
					 if(!(ptr->value).IsString() || !(ptr->value).IsNumber())
					 {
						 rapidjson::StringBuffer buffer;
						 rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
						 valueKey.Accept(writer);
						 sValue = buffer.GetString();
					 }
					 if(sKey == "data")
					 {
						 secondBitValue=(ptr->value).GetBool();
					 }
					 if(sKey == "errmsg")
					 {
						 errmsg=(ptr->value).GetString();
					 }
					 if(sKey == "errcode")
					 {
						 errcode=(ptr->value).GetInt();
					 }
				 }

				 if(errcode!=0||errmsg!="success")
				 {
					 return -1;
				 }
			 }
    		 
			 ///通过判断是否需要上传，计算标识值firstBitValue
			 if (!GetUploadGridFlag(grids.at(i), hasFlag, firstBitValue))
			 {
				 return -1;
			 }

			 ///如果有记录，需要获取原来的tipsGridStatus值
			if (hasFlag)
			{
				if (!GetGridStatus(grids.at(i),iTipsGridStatus))
				{
					return -1;
				}
			}

			 Logger::LogD("before UpdateGridStatus  firstBitValue:%d,secondBitValue:%d,iTipsGridStatus:%d\n",firstBitValue,secondBitValue,iTipsGridStatus);
			

			 SetBit(iTipsGridStatus,0,firstBitValue);
			 SetBit(iTipsGridStatus,1,secondBitValue);

			 Logger::LogD("after UpdateGridStatus  firstBitValue:%d,secondBitValue:%d,iTipsGridStatus:%d\n",firstBitValue,secondBitValue,iTipsGridStatus);
			 std::stringstream ss;
			 if (hasFlag)
			 {
				 ss<<"update grid_Manager set tipsGridStatus=";
				 ss<<iTipsGridStatus;
				 ss<<" where gridid=";
				 ss<<grids.at(i);
			 }
			 else
			 {
				ss<<"insert into grid_Manager values(";
				ss<<grids[i];
				ss<<",'','','','',";
				ss<<iTipsGridStatus;
				ss<<")";
			 }
    		 
    		 std::string sql = ss.str();
    		 sqlite3_stmt* stmt;
    		 int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, NULL);
    		 if (rc != SQLITE_OK)
    		 {
    			 Logger::LogD("select %s failed [%s]", "Grid", sqlite3_errmsg(m_db));
    		     Logger::LogO("select %s failed [%s]", "Grid", sqlite3_errmsg(m_db));
				 sqlite3_finalize(stmt);
				 
				 return -1;
    		 }

    		 if (sqlite3_step(stmt) != SQLITE_DONE)
    		 {
    			/* sqlite3_finalize(stmt);
    			 return -1;*/
				 ///回滚
				 int ret = sqlite3_exec (this->m_db, "ROLLBACK", NULL, NULL, &err_msg);
				 if (ret != SQLITE_OK)
				 {
					 Logger::LogD ("ROLLBACK error: %s\n", err_msg);
					 Logger::LogO ("ROLLBACK error: %s\n", err_msg);

					 sqlite3_free (err_msg);
					 return -1;
				 }

				 if (sqlite3_step(stmt) != SQLITE_DONE)
				 {
					 sqlite3_finalize(stmt);
					 
					 return -1;
				 }

    		 }
    		 sqlite3_finalize(stmt);

			 //更新提交
			 if(nDbOperationCount == nCommitStepNode)
			 {
				 int ret = sqlite3_exec (this->m_db, "COMMIT", NULL, NULL, &err_msg);
				 if (ret != SQLITE_OK)
				 {
					 Logger::LogD ("COMMIT error: %s\n", err_msg);
					 Logger::LogO ("COMMIT error: %s\n", err_msg);

					 sqlite3_free (err_msg);
					 return -1;
				 }
				 nDbOperationCount = 0;
			 }

    	 }
		 
		 //最后提交
		 if(nDbOperationCount < nCommitStepNode && nDbOperationCount >0)
		 {
			 int ret = sqlite3_exec (this->m_db, "COMMIT", NULL, NULL, &err_msg);

			 if (ret != SQLITE_OK)
			 {
				 Logger::LogD ("COMMIT error: %s\n", err_msg);
				 Logger::LogO ("COMMIT error: %s\n", err_msg);

				 sqlite3_free (err_msg);
				 return -1;
			 }
		 }

    	 return 0;
     }

	 bool GridHelper::GetGridStatus(const std::string& grid, int& reStatus)
	 {
		 if(grid==""||this->m_db == NULL)
		 {
			 return false;
		 }

		 std::string sql = "";
		 sql = "select tipsGridStatus from grid_Manager where gridid=" + grid;

		 sqlite3_stmt* stmt;

		 int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, NULL);

		 if (rc != SQLITE_OK)
		 {
			 Logger::LogD("select %s failed [%s]", "Grid", sqlite3_errmsg(m_db));
			 Logger::LogO("select %s failed [%s]", "Grid", sqlite3_errmsg(m_db));

			 //Close();
			 sqlite3_finalize(stmt);
			 return false;
		 }

		 if (sqlite3_step(stmt) == SQLITE_ROW)
		 {
			 int type = sqlite3_column_type(stmt,0);

			 //[SQLITE_INTEGER]
			 if(type == 1)
			 {
				 reStatus = sqlite3_column_int(stmt, 0);
			 }
		 }

		 sqlite3_finalize(stmt);

		 //Close();

		 return true;
	 }

	 bool GridHelper::GetUploadGridFlag(const std::string& grid, bool bHas, bool& flag)
	 {
		 if(grid==""||this->m_db == NULL)
		 {
			 return false;
		 }
		 
		 int iCount = 0;
		 Grid oogrid;
		 std::vector<std::string> vgrid;
		 vgrid.push_back(grid);

		 std::string strWkt=oogrid.GridsToLocation(vgrid);
         
         std::string sql = "select count(1) from edit_tips where t_status=1 and (Within(g_location, GeomFromText('" + strWkt + "')) or Crosses(g_location, GeomFromText('" + strWkt + "'))) and rowid in (select rowid from SpatialIndex s where s.f_table_name='edit_tips' and s.search_frame=g_location)";
		 
		 Logger::LogD("GetUploadGridFlag  sql string :%s",sql.c_str());
		 
		 sqlite3_stmt* stmt;

		 int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, NULL);

		 if (rc != SQLITE_OK)
		 {
			 Logger::LogD("select %s failed [%s] errcode[%d]", "Grid", sqlite3_errmsg(m_db),rc);
			 Logger::LogO("select %s failed [%s] errcode[%d]", "Grid", sqlite3_errmsg(m_db),rc);

			 //Close();
			 sqlite3_finalize(stmt);
			 return false;
		 }

		 if (sqlite3_step(stmt) == SQLITE_ROW)
		 {
			 int type = sqlite3_column_type(stmt,0);

			 //[SQLITE_INTEGER]
			 if(type == 1)
			 {
				 iCount = sqlite3_column_int(stmt, 0);

				  Logger::LogD("GetUploadGridFlag iCount:%d",iCount);
			 }
		 }

		 sqlite3_finalize(stmt);
		 
		 flag=(iCount>0);
         
         return true;
	 }

     bool GridHelper::IsHasGPSLineCrossGrid(const std::string& wkt)
     {
         std::string sql = "select count(1) from tips_line where sourceType = 2001 and Intersects(Geometry, GeomFromText('" + wkt + "')) and rowid in (select rowid from SpatialIndex s where s.f_table_name='tips_line' and s.search_frame=Geometry)";
		 
         sqlite3_stmt* stmt;

		 int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, NULL);

		 if (rc != SQLITE_OK)
		 {
			 Logger::LogD("select %s failed [%s] errcode[%d]", "tips_line", sqlite3_errmsg(m_db),rc);
			 Logger::LogO("select %s failed [%s] errcode[%d]", "tips_line", sqlite3_errmsg(m_db),rc);

			 sqlite3_finalize(stmt);
			 return false;
		 }

		 if (sqlite3_step(stmt) == SQLITE_ROW)
		 {
			 sqlite3_finalize(stmt);

             return true;
		 }

		 sqlite3_finalize(stmt);

         return false;
     }

	 bool GridHelper::HasGrid( const std::string& grid )
	 {
		 if(grid==""||this->m_db == NULL)
		 {
			 return false;
		 }
		 
		 sqlite3_stmt* stmt;
		 const std::string tableName = "grid_Manager";
		 std::string sql = "select * from " + tableName + " where gridid =" + grid;

		 int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, NULL);

		 if (rc != SQLITE_OK)
		 {
			 Logger::LogD("select %s failed [%s]", "Grid", sqlite3_errmsg(m_db));
			 Logger::LogO("select %s failed [%s]", "Grid", sqlite3_errmsg(m_db));

			 sqlite3_finalize(stmt);
			 return false;
		 }

		 if (sqlite3_step(stmt) != SQLITE_ROW)
		 {
			 sqlite3_finalize(stmt);
			 return false;
		 }
		 sqlite3_finalize(stmt);
		 return true;
	 }
     
     std::string GridHelper::GetPatternImageTimeStamp(PatternImage_Mode mode)
     {
        std::string timeStmap = "";
        
        if(this->m_db == NULL)
        {
            return timeStmap;
        }
        
        std::string sql = "";
        
        if(mode == PatternImage_Download)
        {
            sql = "select max(downloadDate) from meta_JVImage";
        }
        
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, NULL);
        
        if (rc != SQLITE_OK)
        {
            Logger::LogD("select %s failed [%s]", "downloadDate", sqlite3_errmsg(m_db));
            Logger::LogO("select %s failed [%s]", "downloadDate", sqlite3_errmsg(m_db));
            
            return timeStmap;
        }
        
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int type = sqlite3_column_type(stmt,0);
            
            //[SQLITE_TEXT]
            if(type == 3)
            {
                timeStmap = (const char *)sqlite3_column_text(stmt, 0);
            }
        }
        
        sqlite3_finalize(stmt);
        
        return timeStmap;
     }
}
