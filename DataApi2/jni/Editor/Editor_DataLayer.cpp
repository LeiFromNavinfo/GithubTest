#include "Editor.h"

namespace Editor
{	
	DataLayer::DataLayer()
	{
		m_Sqlite3Db = NULL;

		m_pFieldDefines = NULL;

		m_stmt = NULL;
	}

	DataLayer::~DataLayer()
	{
		if (m_pFieldDefines)
		{
			delete m_pFieldDefines;

			m_pFieldDefines = NULL;
		}

		if (m_stmt)
		{
			sqlite3_finalize(m_stmt);

			m_stmt = NULL;
		}
	}

	int DataLayer::GetDataLayerType()
	{
		return m_LayerType;
	}

    void DataLayer::SetDataLayerType(DATALAYER_TYPE type)
	{
	    this->m_LayerType = type;
	}

    void DataLayer::SetDBConnection(sqlite3* db)
    {
        this->m_Sqlite3Db = db;

        if(m_pFieldDefines == NULL)
        {
            m_pFieldDefines = GetTableDefines(m_Sqlite3Db, GetTableName());
        }
    }

	int DataLayer::ResetReading()
	{
		if (m_stmt)
		{
			sqlite3_finalize(m_stmt);
		}
		
		std::string sql = "SELECT ROWID,* FROM " + GetTableName();

		int rc = sqlite3_prepare_v2(m_Sqlite3Db, sql.c_str(), -1, &m_stmt, NULL);

		if (rc != SQLITE_OK)
		{
			Logger::LogD("DataLayer::ResetReading sql [%s] failed [%s]", sql.c_str(), sqlite3_errmsg(m_Sqlite3Db));
			Logger::LogO("DataLayer::ResetReading sql [%s] failed [%s]", sql.c_str(), sqlite3_errmsg(m_Sqlite3Db));
			
			return -1;
		}

		return 0;
	}

	int DataLayer::ResetReadingBySpatial(EditorGeometry::Box2D box)
	{
		if (m_stmt)
		{
			sqlite3_finalize(m_stmt);
		}

		std::string wkt;

		std::string max_x_string = Tools::DoubleToString(box._maxx);
		std::string min_x_string = Tools::DoubleToString(box._minx);
		std::string max_y_string = Tools::DoubleToString(box._maxy);
		std::string min_y_string = Tools::DoubleToString(box._miny);

		if(box._maxx - box._minx <0.00001)
		{
			wkt ="POINT(" + min_x_string + " " + min_y_string + ")";
		}
		else
		{
			wkt ="POLYGON((" + min_x_string + " " + max_y_string + "," +
					min_x_string + " " + min_y_string + "," +
					max_x_string + " " + min_y_string + "," +
					max_x_string + " " + max_y_string + "," +
					min_x_string + " " + max_y_string + "))";
		}

		std::string sql = "SELECT ROWID,* FROM " + GetTableName();

		sql += " WHERE ST_Within(GeomFromText(ST_AsText(Geometry)), GeomFromText('"+ wkt +"')) = 1 AND ROWID IN (SELECT ROWID FROM SpatialIndex s WHERE s.f_table_name='"+
				GetTableName()+"' AND s.search_frame= GeomFromText('"+ wkt +"'))";

		int rc = sqlite3_prepare_v2(m_Sqlite3Db, sql.c_str(), -1, &m_stmt, NULL);

		if (rc != SQLITE_OK)
		{
			Logger::LogD("DataLayer::ResetReadingBySpatial sql [%s] failed [%s]", sql.c_str(), sqlite3_errmsg(m_Sqlite3Db));
			Logger::LogO("DataLayer::ResetReadingBySpatial sql [%s] failed [%s]", sql.c_str(), sqlite3_errmsg(m_Sqlite3Db));

			return -1;
		}

		return 0;
	}

	int DataLayer::ResetReadingBySQL(std::string sql)
	{
        if (m_stmt)
		{
			sqlite3_finalize(m_stmt);
		}

        std::string select_statement_string = sql.substr(0,6);
        
        if(strcmp("SELECT",Tools::ToUpper(select_statement_string).c_str()) != 0)
        {
            Logger::LogD("DataLayer::ResetReadingBySQL sql [%s] doesn't contain SELECT", sql.c_str());
            Logger::LogO("DataLayer::ResetReadingBySQL sql [%s] doesn't contain SELECT", sql.c_str());
            
            return -1;
        }
        
        std::string::size_type pos = sql.find(select_statement_string);

		int sql_length = sql.length();

		sql = sql.substr(0, pos) + "SELECT ROWID," + sql.substr(pos+6, sql_length-pos-6);

		int rc = sqlite3_prepare_v2(m_Sqlite3Db, sql.c_str(), -1, &m_stmt, NULL);

		if (rc != SQLITE_OK)
		{
			Logger::LogD("DataLayer::ResetReadingBySQL sql [%s] failed [%s]", sql.c_str(), sqlite3_errmsg(m_Sqlite3Db));
			Logger::LogO("DataLayer::ResetReadingBySQL sql [%s] failed [%s]", sql.c_str(), sqlite3_errmsg(m_Sqlite3Db));
			
			return -1;
		}

		return 0;
	}

	DataFeature* DataLayer::GetNextFeature()
	{
		int rc = sqlite3_step(m_stmt);

		if (rc != SQLITE_ROW)
		{
			return NULL;
		}

		DataFeature* feature = DataFeatureFactory::getDefaultInstance()->CreateFeature(this);

		feature->Initialize(m_stmt, false, this);

		return feature;
	}

	DataFeature* DataLayer::GetFeatureByRowId(unsigned int rowId)
	{
		sqlite3_stmt *statement;

		std::string sql = "SELECT ROWID,* FROM " + GetTableName();

		sql += " WHERE ROWID=" + Tools::NumberToString(rowId);

		int result = sqlite3_prepare_v2(m_Sqlite3Db, sql.c_str(), -1, &statement, NULL);  

		if (result != SQLITE_OK)
		{
			Logger::LogD("DataLayer::GetFeatureByRowId sql [%s] failed [%s]", sql.c_str(), sqlite3_errmsg(m_Sqlite3Db));
			Logger::LogO("DataLayer::GetFeatureByRowId sql [%s] failed [%s]", sql.c_str(), sqlite3_errmsg(m_Sqlite3Db));
			
			return NULL;
		}

		result = sqlite3_step(statement);

		if(result != SQLITE_ROW)
		{
			sqlite3_finalize(statement);

			return NULL;
		}

		DataFeature* feature = DataFeatureFactory::getDefaultInstance()->CreateFeature(this);

		feature->Initialize(statement, true, this);

		return feature;
	}

	int DataLayer::BeginTranscation()
	{
		char* pszErrMsg;

		int rc = sqlite3_exec(m_Sqlite3Db, "BEGIN", NULL, NULL, &pszErrMsg);

		if (rc != SQLITE_OK)
		{
			Logger::LogD("DataLayer::BeginTranscation failed [%s]", pszErrMsg);
			Logger::LogO("DataLayer::BeginTranscation failed [%s]", pszErrMsg);

			sqlite3_free (pszErrMsg);

			return -1;
		}

		return 0;
	}

	int DataLayer::InsertFeature(DataFeature* feature)
	{
	    sqlite3_stmt *statement;

		std::map<int,int> blobIndices;

		int blobIndex=0;

		std::string sql="insert into " + GetTableName() + "(";
        
        if(NULL==m_pFieldDefines)
            return -1;

		for(int i=0;i<m_pFieldDefines->GetColumnCount();i++)
		{
			int type = m_pFieldDefines->GetColumnType(i);

			sql += m_pFieldDefines->GetColumnName(i) + ",";
		}

		sql = Tools::TrimRight(sql, ",");

		sql+=") values (";

		for(int i=0; i<m_pFieldDefines->GetColumnCount() ;i++)
		{
			int type = m_pFieldDefines->GetColumnType(i);

			switch(type)
			{
			case FT_BLOB:
			case FT_GEOMETRY:
				{
					sql += "?,";

					blobIndices.insert(std::make_pair<int,int>(i, ++blobIndex));
				}
				break;
			case FT_TEXT:
				sql += "'" + feature->GetAsString(i) + "',";
				break;
			case FT_INTEGER:
				sql += Tools::NumberToString(feature->GetAsInteger(i)) + ",";            
				break;
			case FT_DOUBLE:
				sql += Tools::NumberToString(feature->GetAsDouble(i)) + ",";
				break;
			default:
				break;
			}
		}

		sql = Tools::TrimRight(sql, ",");

		sql += " )";

		int result = sqlite3_prepare_v2(m_Sqlite3Db, sql.c_str(), -1, &statement, NULL);

		if( result != SQLITE_OK )
		{
            Logger::LogD("DataLayer::InsertFeature failed! [%s]", sql.c_str());
			Logger::LogO("DataLayer::InsertFeature failed! [%s]", sql.c_str());
			return -1;
		}

		// bind blob
		std::vector<void*> unfrees;

		for(int i=0;i<m_pFieldDefines->GetColumnCount();i++)
		{
			int type = m_pFieldDefines->GetColumnType(i);

			switch(type)
			{
			case FT_BLOB:
				{
					unsigned int length;

					unsigned char* buff = feature->GetAsBlob(i,length);

					sqlite3_bind_blob(statement, blobIndices[i], buff, length, NULL);
				}
				break;
			case FT_GEOMETRY:
				{
					EditorGeometry::WkbGeometry* geo = feature->GetAsWkb(i);
                
					if(geo == NULL)
						result = sqlite3_bind_blob(statement, blobIndices[i], NULL, 0, NULL);
					else
					{
						EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(geo);

						unfrees.push_back(spGeo);

						result = sqlite3_bind_blob(statement, blobIndices[i], spGeo, geo->buffer_size()+39, NULL);
					}
				}
				break;
			default:
				break;
			}
		}

		result = sqlite3_step(statement);

		for(int i=0;i<unfrees.size();i++)
		{
			free(unfrees[i]);
		}

		if(result != SQLITE_DONE)
		{    
			Logger::LogD("DataLayer::InsertFeature failed! [%s]", sqlite3_errmsg(m_Sqlite3Db));
			Logger::LogO("DataLayer::InsertFeature failed! [%s]", sqlite3_errmsg(m_Sqlite3Db));

			sqlite3_finalize(statement);

			return -1;
		}

		sqlite3_finalize(statement);

		unsigned int rowid = sqlite3_last_insert_rowid(m_Sqlite3Db);

		feature->SetRowId(rowid);

		return 0;
	}

	int DataLayer::UpdateFeature(DataFeature* feature)
	{
		sqlite3_stmt *statement;

		std::map<int,int> blobIndices;

		int blobIndex=0;

		int rid = feature->GetRowId();

		std::string sql="UPDATE " + GetTableName() + " SET ";

		for(int i=0;i<m_pFieldDefines->GetColumnCount();i++)
		{
			int type = m_pFieldDefines->GetColumnType(i);

			switch(type)
			{
			case FT_BLOB:
			case FT_GEOMETRY:
				{
					sql += m_pFieldDefines->GetColumnName(i) + " = ?,";

					blobIndices.insert(std::make_pair<int,int>(i, ++blobIndex));
				}
				break;
			case FT_TEXT:
				sql += m_pFieldDefines->GetColumnName(i) + " = '" + feature->GetAsString(i) + "',";
				break;
			case FT_INTEGER:
				sql += m_pFieldDefines->GetColumnName(i) + " = " + Tools::NumberToString(feature->GetAsInteger(i)) +",";
				break;
			case FT_DOUBLE:
				sql += m_pFieldDefines->GetColumnName(i) + " = " + Tools::NumberToString(feature->GetAsDouble(i)) +",";
				break;
			default:
				break;
			}
		}

		sql = Tools::TrimRight(sql, ",");

		sql += " WHERE ROWID=" + Tools::NumberToString(rid);

		int result = sqlite3_prepare_v2(m_Sqlite3Db, sql.c_str(), -1, &statement, NULL);

		if( result != SQLITE_OK )
		{
			return -1;
		}

		// bind blob
		std::vector<void*> unfrees;

		for(int i=0;i<m_pFieldDefines->GetColumnCount();i++)
		{
			int type = m_pFieldDefines->GetColumnType(i);

			switch(type)
			{
			case FT_BLOB:
				{
					unsigned int length;

					unsigned char* buff = feature->GetAsBlob(i,length);

					sqlite3_bind_blob(statement, blobIndices[i], buff, length, NULL);
				}
				break;
			case FT_GEOMETRY:
				{
					EditorGeometry::WkbGeometry* geo = feature->GetAsWkb(i);
                
					if(geo == NULL)
						result = sqlite3_bind_blob(statement, blobIndices[i], NULL, 0, NULL);
					else
					{
						EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(geo);

						unfrees.push_back(spGeo);

						result = sqlite3_bind_blob(statement, blobIndices[i], spGeo, geo->buffer_size()+39, SQLITE_TRANSIENT);
					}
				}
				break;
			default:
				break;
			}
		}

		result = sqlite3_step(statement);

		for(int i=0;i<unfrees.size();i++)
		{
			free(unfrees[i]);
		}

		if(result != SQLITE_DONE)
		{    
			Logger::LogD("DataLayer::UpdateFeature failed! [%s]", sqlite3_errmsg(m_Sqlite3Db));
			Logger::LogO("DataLayer::UpdateFeature failed! [%s]", sqlite3_errmsg(m_Sqlite3Db));

			sqlite3_finalize(statement);

			return -1;
		}

		sqlite3_finalize(statement);

		return 0;
	}

	int DataLayer::DeleteFeature(DataFeature* feature)
	{
		char* zErrMsg;

		int rid = feature->GetRowId();

		std::string sql = "DELETE FROM " + GetTableName() + " where ROWID=" + Tools::NumberToString(rid);

		int result = sqlite3_exec(m_Sqlite3Db, sql.c_str(), NULL, NULL, &zErrMsg);

		if (result != SQLITE_OK)
		{
			Logger::LogD("DataLayer::DeleteFeature failed! [%s]", sqlite3_errmsg(m_Sqlite3Db));
			Logger::LogO("DataLayer::DeleteFeature failed! [%s]", sqlite3_errmsg(m_Sqlite3Db));

			sqlite3_free (zErrMsg);

			return -1;
		}    
    
		return 0;
	}

    int DataLayer::ExcuteSql(const std::string& sql)
    {
        char* zErrMsg;

        int result = sqlite3_exec(m_Sqlite3Db, sql.c_str(), NULL, NULL, &zErrMsg);

		if (result != SQLITE_OK)
		{
			Logger::LogD("DataLayer:: Excute sql failed! [%s]", sqlite3_errmsg(m_Sqlite3Db));
			Logger::LogO("DataLayer:: Excute sql failed! [%s]", sqlite3_errmsg(m_Sqlite3Db));

			sqlite3_free (zErrMsg);

			return -1;
		}    
    
		return 0;
    }
	int DataLayer::EndTranscation()
	{
		char* pszErrMsg;

		int rc = sqlite3_exec(m_Sqlite3Db, "COMMIT", NULL, NULL, &pszErrMsg);

		if (rc != SQLITE_OK)
		{
			Logger::LogD("DataLayer::EndTranscation failed [%s]", pszErrMsg);
			Logger::LogO("DataLayer::EndTranscation failed [%s]", pszErrMsg);

			sqlite3_free (pszErrMsg);

			return -1;
		}

		return 0;
	}

	int DataLayer::RollbackTranscation()
	{
		char* pszErrMsg;

		int rc = sqlite3_exec(m_Sqlite3Db, "ROLLBACK", NULL, NULL, &pszErrMsg);

		if (rc != SQLITE_OK)
		{
			Logger::LogD("DataLayer::RollbackTranscation failed [%s]", pszErrMsg);
			Logger::LogO("DataLayer::RollbackTranscation failed [%s]", pszErrMsg);

			sqlite3_free (pszErrMsg);

			return -1;
		}

		return 0;
	}

	std::string DataLayer::GetTableName()
	{
		std::string tableName = "";

		switch (m_LayerType)
		{
		case DATALAYER_POI:
			{
				tableName = "edit_pois";
			}
			break;
		case DATALAYER_RDLINE:
			{
				tableName = "gdb_rdLine";
			}
			break;
        case DATALAYER_TIPS:
            {
                tableName = "edit_tips";
            }
            break;
        case DATALAYER_TIPSPOINT:
            {
        		tableName = "tips_point";
        	}
            break;
        case DATALAYER_GPSLINE:
        case DATALAYER_TIPSLINE:
        	{
        		tableName = "tips_line";
        	}
            break;
        case DATALAYER_TIPSMULTILINE:
            {
                tableName = "tips_multiLine";
                
                break;
            }
        case DATALAYER_TIPSPOLYGON:
        	{
        		tableName = "tips_polygon";
        	}
            break;
        case DATALAYER_TIPSGEOCOMPONENT:
        	{
        		tableName = "tips_geo_component";
        	}
            break;
        case DATALAYER_RDNODE:
            {
                tableName = "gdb_rdNode";
            }
            break;
        case DATALAYER_INFOR:
            {
                tableName = "edit_infos";
            }
            break;
        case DATALAYER_RDLINE_GSC:
            {
                tableName = "gdb_rdLink_gsc";
            }
            break;
        case DATALAYER_BKLINE:
            {
                tableName = "gdb_bkLine";
            }
            break;
        case DATALAYER_FACE:
            {
                tableName = "gdb_bkFace";
            }
            break;
        case DATALAYER_PROJECTUSER:
            {
                tableName = "project_user";
            }
                break;
        case DATALAYER_PROJECTINFO:
            {
                tableName = "project_info";
            }
                break;
        case DATALAYER_TASKINFO:
            {
                tableName = "task_info";
            }
                break;
        case DATALAYER_TRACKPOINT:
            {
               tableName = "track_collection";
            }
                break;
        case DATALAYER_TRACKSEGMENT:
            {
                tableName = "track_segment";
            }
                break;
		default:
			break;
		}

		return tableName;
	}

	FieldDefines* DataLayer::GetTableDefines(sqlite3* db, const std::string& tablename)
	{
		char** papszResult;

		int nRowCount, iRow, nColCount;

		char* pszErrMsg;

		FieldDefines* fieldDefines = NULL;

		std::string sql = "SELECT sql FROM sqlite_master WHERE type='table' and name='" + tablename + "'";

		int rc = sqlite3_get_table(db, sql.c_str(), &papszResult, &nRowCount, &nColCount, &pszErrMsg);

		if (rc != SQLITE_OK)
		{
			Logger::LogD("DataLayer::GetTableDefines unable to fetch list of table [%s]", pszErrMsg);
			Logger::LogO("DataLayer::GetTableDefines unable to fetch list of table [%s]", pszErrMsg);

			sqlite3_free(pszErrMsg);

			return fieldDefines;
		}

		if (nRowCount > 0)
		{
			sql = papszResult[1];

			fieldDefines = ParseSql(sql);
		}

		sqlite3_free_table(papszResult);

		return fieldDefines;
	}

	FieldDefines* DataLayer::ParseSql(const std::string& sql)
	{
		FieldDefines* fieldDefines = new FieldDefines();

		size_t pos1 = sql.find("(");

		size_t pos2 = sql.find(")");

		std::string str = sql.substr(pos1+1, pos2-pos1-1);

		std::vector<std::string> fields = Tools::StringSplit(str,",");

		fieldDefines->SetColumnCount(fields.size());

		for(int i=0;i<fields.size();i++)
		{
			std::vector<std::string> cols = Tools::StringSplit(fields[i]," ");

			if(cols.size()<2)
			{
				delete fieldDefines;

				fieldDefines = NULL;

				return NULL;
			}

			fieldDefines->SetColumnName(i, Tools::Trim(cols[0],"\""));

			if(Tools::CaseInsensitiveCompare(cols[1],"BYTE"))
			{
				fieldDefines->SetColumnType(i, FT_INTEGER);
			}
			else if(Tools::CaseInsensitiveCompare(cols[1],"INT16"))
			{
				fieldDefines->SetColumnType(i, FT_INTEGER);
			}
			else if(Tools::CaseInsensitiveCompare(cols[1],"INT32"))
			{
				fieldDefines->SetColumnType(i, FT_INTEGER);
			}
			else if(Tools::CaseInsensitiveCompare(cols[1],"INTEGER"))
			{
				fieldDefines->SetColumnType(i, FT_INTEGER);
			}
			else if(Tools::CaseInsensitiveCompare(cols[1],"REAL32"))
			{
				fieldDefines->SetColumnType(i, FT_DOUBLE);
			}
			else if(Tools::CaseInsensitiveCompare(cols[1],"DOUBLE"))
			{
				fieldDefines->SetColumnType(i, FT_DOUBLE);
			}
			else if(Tools::CaseInsensitiveCompare(cols[1],"BLOB"))
			{
				fieldDefines->SetColumnType(i, FT_BLOB);
			}
			else if(Tools::CaseInsensitiveCompare(cols[1],"TEXT"))
			{
				fieldDefines->SetColumnType(i, FT_TEXT);
			}
			else if(Tools::CaseInsensitiveCompare(cols[1],"GEOMETRY")
				|| Tools::CaseInsensitiveCompare(cols[1],"POINT")
				|| Tools::CaseInsensitiveCompare(cols[1],"LINESTRING")
				|| Tools::CaseInsensitiveCompare(cols[1],"POLYGON")
				|| Tools::CaseInsensitiveCompare(cols[1],"MULTIPOINT")
				|| Tools::CaseInsensitiveCompare(cols[1],"GEOMETRYCOLLECTION")
                || Tools::CaseInsensitiveCompare(cols[1],"MULTILINESTRING")
				)
			{
				fieldDefines->SetColumnType(i, FT_GEOMETRY);
			}
			else 
			{
            
			}
		}

		return fieldDefines;
	}

	FieldDefines::FieldDefines()
	{
	}

	FieldDefines::~FieldDefines()
	{
	}

	void FieldDefines::SetColumnCount(unsigned int count)
	{
		m_vFields.resize(count);
	}

	unsigned int FieldDefines::GetColumnCount()
	{
		return m_vFields.size();
	}

	void FieldDefines::SetColumnType(unsigned int index, int data_type)
	{
		if (index >= m_vFields.size())
			return;

		std::vector<Field>::iterator itor = m_vFields.begin() + index;

		itor->_Type = (FIELD_TYPE)data_type;
	}

	int FieldDefines::GetColumnType(unsigned int index)
	{
		if (index >= m_vFields.size())
			return -1;

		std::vector<Field>::const_iterator itor = m_vFields.begin() + index;

		return itor->_Type;
	}

	void FieldDefines::SetColumnName(unsigned int index, const std::string& col_name)
	{
		if (index >= m_vFields.size())
			return;

		std::string name = col_name;

		std::vector<Field>::iterator itor = m_vFields.begin() + index;

		itor->_Name = Tools::Trim(name, "\x0a\"");

		m_mColumns.insert(std::make_pair<std::string, int>(itor->_Name, index));
	}

	std::string FieldDefines::GetColumnName(unsigned int index)
	{
		if (index >= m_vFields.size())
			return "";

		std::vector<Field>::const_iterator itor = m_vFields.begin() + index;

		return itor->_Name;
	}

	int FieldDefines::GetColumnIndex(const std::string& col_name)
	{
		if (m_mColumns.find(col_name) == m_mColumns.end())
			return -1;

		return m_mColumns[col_name];
	}

    int  DataLayer::GetMaxIntPrimaryKey(std::string primaryKey)
	{
	    sqlite3_stmt *statement; 

	    std::string sql = "SELECT max(CAST(" + primaryKey + " AS int)) FROM " + this->GetTableName();

	    int max = -1;

	    if ( sqlite3_prepare(m_Sqlite3Db, sql.c_str(),strlen(sql.c_str()), &statement, NULL ) == SQLITE_OK )
        {
            int res = sqlite3_step(statement);

		    if(res == SQLITE_ROW)
		    {
			    max = sqlite3_column_int(statement, 0);
		    }
       
		    sqlite3_finalize (statement);
        }
	    else
	    {
		    max=-1;
	    }

	    if(max == -1)
	    {
            Logger::LogD("Get [%s] max [%s] Failed",this->GetTableName().c_str(),primaryKey.c_str());
            Logger::LogO("Get [%s] max [%s] Failed",this->GetTableName().c_str(),primaryKey.c_str());
	    }

	    return max;
	}

	FieldDefines* DataLayer::GetFieldDefines()
	{
		return m_pFieldDefines;
	}
}