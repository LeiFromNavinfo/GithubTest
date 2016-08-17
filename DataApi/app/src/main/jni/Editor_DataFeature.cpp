#include "Editor.h"
#include "Editor_Data.h"

namespace Editor
{	
    int DataFeature::m_refCount = 0;

	DataFeature::DataFeature()
	{
        m_refCount++;

		m_stmt = NULL;

		m_ChangeVals = NULL;

		m_ChangeValsLength = 0;

		m_bIsStatementOwner = false;

        m_pFieldDefines = NULL;

        m_layer = NULL;
	}

	DataFeature::~DataFeature()
	{
        m_refCount--;

        Logger::LogD("DataFeature m_refCount:%d", m_refCount);

		if (m_ChangeVals)
		{
			free(m_ChangeVals);
		}

		if (m_bIsStatementOwner)
		{
			sqlite3_finalize(m_stmt);
		}

		for (int i=0; i<m_UnfreedMemory.size(); i++)
		{
			free(m_UnfreedMemory[i]);
		}
	}

	void DataFeature::Initialize(sqlite3_stmt* stmt, bool isStatementOwner, DataLayer* layer)
	{
		this->m_stmt = stmt;

		this->m_bIsStatementOwner = isStatementOwner;

        this->m_layer = layer;

        if(layer != NULL)
        {
            this->m_pFieldDefines = layer->GetFieldDefines();
        }
	}

    FieldDefines* DataFeature::GetFieldDefines()
    {
        return this->m_pFieldDefines;
    }

    DataLayer* DataFeature::GetDataLayer()
	{
	    return this->m_layer;
	}

    int DataFeature::SyncDataToMemory()
    {
        this->SetRowId(sqlite3_column_int(m_stmt, 0));

        int nColCount = sqlite3_column_count(m_stmt);

        for (int i=0; i<nColCount-1; i++)
        {
            int type = sqlite3_column_type(m_stmt, i+1);

            switch (type)
            {
            case SQLITE_INTEGER:
                this->SetAsInteger(i, this->GetAsInteger(i));
                break;
            case SQLITE_FLOAT:
                this->SetAsDouble(i, this->GetAsDouble(i));
                break;
            case SQLITE_BLOB:
                {
                    const char* col_name = sqlite3_column_name(m_stmt, i+1);
                    int col_index = m_pFieldDefines->GetColumnIndex(col_name);
                    int col_type = m_pFieldDefines->GetColumnType(col_index);
                    if (col_type == FT_GEOMETRY)
                    {
                        EditorGeometry::WkbGeometry* wkb = this->GetAsWkb(i);
                        this->SetAsWkb(i, wkb);
                    }
                    else
                    {
                        unsigned char* blob;
                        unsigned int length;
                        blob = this->GetAsBlob(i, length);
                        this->SetAsBlob(i, blob, length);
                    }
                }
                break;
            case SQLITE_TEXT:
                this->SetAsString(i, this->GetAsString(i)); 
                break;
            default:
                Logger::LogD("DataFeature::SyncDataToMemory sqlite3_column_type col:%d type:%d", i, type);
                Logger::LogO("DataFeature::SyncDataToMemory sqlite3_column_type col:%d type:%d", i, type);
                break;
            }
        }

        return 0;
    }

	unsigned int DataFeature::GetRowId()
	{
		if (m_ChangeCols.find(-1) != m_ChangeCols.end())
		{
			unsigned int ret = *(unsigned int*)(m_ChangeVals + m_ChangeCols[-1]);

			return ret;
		}

		return sqlite3_column_int(m_stmt, 0);
	}

	int DataFeature::SetRowId(unsigned int rowId)
	{
		if (m_ChangeCols.find(-1) == m_ChangeCols.end())
		{
			m_ChangeCols.insert(std::make_pair(-1, m_ChangeValsLength));

			m_ChangeValsLength += sizeof(unsigned int);

			m_ChangeVals = (unsigned char*)realloc(m_ChangeVals, m_ChangeValsLength);
		}

		memcpy(m_ChangeVals+m_ChangeCols[-1], &rowId, sizeof(unsigned int));

		return 0;
	}

	int DataFeature::GetAsInteger(unsigned int index)
	{
		if (m_ChangeCols.find(index) != m_ChangeCols.end())
		{
			int ret = *(int*)(m_ChangeVals + m_ChangeCols[index]);

			return ret;
		}

		return sqlite3_column_int(m_stmt, index+1);
	}

	double DataFeature::GetAsDouble(unsigned int index)
	{
		if (m_ChangeCols.find(index) != m_ChangeCols.end())
		{
			char buffer[1024];

			memcpy(buffer, m_ChangeVals + m_ChangeCols[index], sizeof(double));

			double ret = *(double*)buffer;

			return ret;
		}

		return sqlite3_column_double(m_stmt, index+1);
	}

	std::string DataFeature::GetAsString(unsigned int index)
	{
		int length;

		const unsigned char* buff;

		if (m_ChangeCols.find(index) != m_ChangeCols.end())
		{
			length = *(int*)(m_ChangeVals + m_ChangeCols[index]);

			buff = m_ChangeVals + m_ChangeCols[index] + sizeof(int);
		}
		else
		{
			length = sqlite3_column_bytes(m_stmt, index+1);

			buff = sqlite3_column_text(m_stmt, index+1);
		}

		std::string ret((char*)buff, length);

		return ret;
	}

	unsigned char* DataFeature::GetAsBlob(unsigned int index, unsigned int& length)
	{
		const void* buff;

		if (m_ChangeCols.find(index) != m_ChangeCols.end())
		{
			length = *(unsigned int*)(m_ChangeVals + m_ChangeCols[index]);

			buff = m_ChangeVals + m_ChangeCols[index] + sizeof(unsigned int);
		}
		else
		{
			length = sqlite3_column_bytes(m_stmt, index+1);

			buff = sqlite3_column_blob(m_stmt, index+1);
		}

		if (0 == length)
			return NULL;

		unsigned char* ret = (unsigned char*)malloc(length);

		memset(ret,0,length);
		memcpy(ret, buff, length);

		m_UnfreedMemory.push_back(ret);

		return ret;
	}

	EditorGeometry::WkbGeometry* DataFeature::GetAsWkb(unsigned int index)
	{
		unsigned int length = 0;

		unsigned char* buff = NULL;

		EditorGeometry::WkbGeometry* ret = NULL;

		if (m_ChangeCols.find(index) != m_ChangeCols.end())
		{
			length = *(int*)(m_ChangeVals + m_ChangeCols[index]);

			if (0 == length)
			{
				return ret;
			}

			buff = m_ChangeVals + m_ChangeCols[index] + sizeof(int);

			ret = (EditorGeometry::WkbGeometry*)malloc(((EditorGeometry::WkbGeometry*)buff)->buffer_size());

			memcpy(ret, buff, ((EditorGeometry::WkbGeometry*)buff)->buffer_size());
		}
		else
		{
			buff = GetAsBlob(index, length);

			if (0 == length)
			{
				return ret;
			}

			ret = EditorGeometry::SpatialiteGeometry::ToWKBGeometry((EditorGeometry::SpatialiteGeometry*)buff);
		}

		m_UnfreedMemory.push_back(ret);

		return ret;
	}

	int DataFeature::SetAsInteger(unsigned int index, int value)
	{
		if (m_ChangeCols.find(index) == m_ChangeCols.end())
		{
			m_ChangeCols.insert(std::make_pair(index, m_ChangeValsLength));

			m_ChangeValsLength += sizeof(int);

			m_ChangeVals = (unsigned char*)realloc(m_ChangeVals, m_ChangeValsLength);
		}

		memcpy(m_ChangeVals+m_ChangeCols[index], &value, sizeof(int));

		return 0;
	}

	int DataFeature::SetAsDouble(unsigned int index, double value)
	{
		if (m_ChangeCols.find(index) == m_ChangeCols.end())
		{
			m_ChangeCols.insert(std::make_pair(index, m_ChangeValsLength));

			m_ChangeValsLength += sizeof(double);

			m_ChangeVals = (unsigned char*)realloc(m_ChangeVals, m_ChangeValsLength);
		}

		memcpy(m_ChangeVals+m_ChangeCols[index], &value, sizeof(double));

		return 0;
	}

	int DataFeature::SetAsString(unsigned int index, const std::string& value)
	{
		if (m_ChangeCols.find(index) == m_ChangeCols.end())
		{
			m_ChangeCols.insert(std::make_pair(index, m_ChangeValsLength));
		}
		else
		{
			m_ChangeCols[index] = m_ChangeValsLength;
		}

		int size = value.size();

		m_ChangeValsLength += sizeof(int);

		m_ChangeValsLength += size;

		m_ChangeVals = (unsigned char*)realloc(m_ChangeVals, m_ChangeValsLength);

		memcpy(m_ChangeVals+m_ChangeCols[index], &size, sizeof(int));

		memcpy(m_ChangeVals+m_ChangeCols[index]+sizeof(int), value.c_str(), size);

		return 0;
	}

	int DataFeature::SetAsBlob(unsigned int index, unsigned char* blob, unsigned int length)
	{
		if (m_ChangeCols.find(index) == m_ChangeCols.end())
		{
			m_ChangeCols.insert(std::make_pair(index, m_ChangeValsLength));
		}
		else
		{
			m_ChangeCols[index] = m_ChangeValsLength;
		}

		m_ChangeValsLength += sizeof(unsigned int);

		m_ChangeValsLength += length;

		m_ChangeVals = (unsigned char*)realloc(m_ChangeVals, m_ChangeValsLength);

		memcpy(m_ChangeVals+m_ChangeCols[index], &length, sizeof(unsigned int));

		memcpy(m_ChangeVals+m_ChangeCols[index]+sizeof(unsigned int), blob, length);

		return 0;
	}

	int DataFeature::SetAsWkb(unsigned int index, EditorGeometry::WkbGeometry* wkb)
	{
		if (m_ChangeCols.find(index) == m_ChangeCols.end())
		{
			m_ChangeCols.insert(std::make_pair(index, m_ChangeValsLength));
		}
		else
		{
			m_ChangeCols[index] = m_ChangeValsLength;
		}

		int size = 0;

		if (wkb != NULL)
			size = wkb->buffer_size();

		m_ChangeValsLength += sizeof(int);

		m_ChangeValsLength += size;

		m_ChangeVals = (unsigned char*)realloc(m_ChangeVals, m_ChangeValsLength);

		memcpy(m_ChangeVals+m_ChangeCols[index], &size, sizeof(int));

		memcpy(m_ChangeVals+m_ChangeCols[index]+sizeof(int), wkb, size);

		return 0;
	}
    
    int DataFeature::SetTotalPart(JSON json)
    {
        rapidjson::Document doc;
        
        doc.Parse<0>(json.GetJsonString().c_str());
        
        if(doc.HasParseError())
        {
            return -1;
        }
        
        FieldDefines* fd = this->GetFieldDefines();
        
        if(fd == NULL)
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
            
            if(!(ptr->value).IsString() && !(ptr->value).IsNumber())
            {
                rapidjson::StringBuffer buffer;
                
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                
                valueKey.Accept(writer);
                
                sValue = buffer.GetString();
            }
            
            if(Tools::CaseInsensitiveCompare(sKey, "rowid"))
            {
                this->SetRowId((ptr->value).GetInt());
            }
            else
            {
                int col_index = fd->GetColumnIndex(sKey);
                
                if(col_index == -1)
                {
                    continue;
                }
                
                int col_type = fd->GetColumnType(col_index);
                
                switch(col_type)
                {
                    case FT_INTEGER:
                        this->SetAsInteger(col_index, (ptr->value).GetInt());
                        break;
                    case FT_DOUBLE:
                        this->SetAsDouble(col_index, (ptr->value).GetDouble());
                        break;
                    case FT_TEXT:
                        this->SetAsString(col_index, (ptr->value).GetString());
                        break;
                    case FT_BLOB:
                    {
                        const char* blob = sValue.c_str();
                        
                        unsigned char* blob_str = (unsigned char*)blob;
                        
                        this->SetAsBlob(col_index, blob_str, strlen(blob));
                        
                        break;
                    }
                    case FT_GEOMETRY:
                    {
                        std::string wktGeo = (ptr->value).GetString();
                        
                        EditorGeometry::WkbGeometry * wkb= DataTransfor::Wkt2Wkb(wktGeo);
                        
                        SetAsWkb(col_index, wkb);
                        
                        delete[] wkb;
                        
                        break;
                    }
                }
                
            }
        }
        return 0;
    }
    
    JSON DataFeature::GetTotalPart()
    {
        JSON json;
        
        FieldDefines* fd = this->GetFieldDefines();
        
        if(NULL == fd || fd->GetColumnCount() == 0)
        {
            return json;
        }
        Document document;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();
        
        Value each_json_value(kStringType);
        std::string each_str_value ;
        
        document.AddMember("rowid",GetRowId(),allocator);
        
        for(int i=0;i<fd->GetColumnCount();i++)
        {
            std::string columnName = fd->GetColumnName(i);
            int columnType = fd->GetColumnType(i);
            
            Value key_name(kStringType);
            
            key_name.SetString(columnName.c_str(), columnName.size(), allocator);
            
            switch(columnType)
            {
                case FT_INTEGER:
                    document.AddMember(key_name,GetAsInteger(i),allocator);
                    break;
                case FT_DOUBLE:
                    document.AddMember(key_name,GetAsDouble(i),allocator);
                    break;
                case FT_TEXT:
                {
                    each_str_value = GetAsString(i);
                    each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
                    document.AddMember(key_name,each_json_value,allocator);
                    break;
                }
                case FT_BLOB:
                {
                    Document docTmp(&allocator);
                    
                    unsigned char* blob = NULL;
                    unsigned int length;
                    blob = this->GetAsBlob(i, length);
                    
                    if (NULL==blob)
                    {
                        docTmp.SetNull();
                    }
                    else
                    {
                        std::string temp_string(reinterpret_cast<char*>(blob),length);
                        
                        docTmp.Parse<0>(temp_string.c_str());
                        
                        if (docTmp.HasParseError())
                        {
                            Logger::LogD("DataFeature::GetTotalPart fieldError[value is null or form error],fieldName[%s],fieldValve[%s]",columnName.c_str(), temp_string.c_str());
                            Logger::LogO("DataFeature::GetTotalPart fieldError[value is null or form error],fieldName[%s],fieldValve[%s]",columnName.c_str(), temp_string.c_str());
                            docTmp.SetNull();
                        }
                    }
                    document.AddMember(key_name,docTmp,allocator);
                    break;
                }
                case FT_GEOMETRY:
                {
                    EditorGeometry::WkbGeometry* geoWkb = GetAsWkb(i);
                    each_str_value = DataTransfor::Wkb2Wkt(geoWkb);
                    each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
                    document.AddMember(key_name,each_json_value,allocator);
                    break;
                }
            }
        }
        
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        document.Accept(writer);
        
        std::string result = buffer.GetString();
        
        json.SetJsonString(result);
        
        return json;
    }
    
    JSON DataFeature::GetSnapshotPart()
    {
        return this->GetTotalPart();
    }
}
