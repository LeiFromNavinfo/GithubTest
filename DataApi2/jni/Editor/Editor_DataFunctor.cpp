#include "Editor.h"
#include "BussinessConstants.h"

namespace Editor
{
    DataFunctor::DataFunctor()
    {
        m_pIsDbOpen = false;
    }

    geos::geom::Geometry* DataFunctor::GetLineGeometry(const std::string& id, DATALAYER_TYPE layerType)
    {
        Editor::DataManager* pDataManager = Editor::DataManager::getInstance();
        
        Editor::DataSource* pDataSource = pDataManager->getDataSourceByLayerType(layerType);
        
        if(NULL==pDataSource)
        {
            return NULL;
        }
        
        int sourceType = pDataSource->GetDataSourceType();
        
        std::string dataPath = "";
        
        if (DATASOURCE_COREMAP==sourceType)
        {
            dataPath = pDataManager->getFileSystem()->GetCoreMapDataPath();
        }
        else if (DATASOURCE_GDB==sourceType)
        {
            dataPath = pDataManager->getFileSystem()->GetEditorGDBDataFile();
        }
        
        if(strcmp(pDataSource->GetConnectionString().c_str(),"") == 0)
        {
            pDataSource->Open(dataPath);
            
            m_pIsDbOpen = true;
        }
        
        Editor::DataLayer* targetLayer = pDataSource->GetLayerByType(layerType);
        
        if(targetLayer == NULL)
        {
            return NULL;
        }
        
        FieldDefines* fieldDefines = targetLayer->GetFieldDefines();
        
        if(fieldDefines == NULL)
        {
            return NULL;
        }
        
        int geoIndex = fieldDefines->GetColumnIndex(Geometry_Str);
        
        if(geoIndex == -1)
        {
            return NULL;
        }
        
        std::string primaryKeyName = fieldDefines->GetColumnName(0);
        
        int columnType = fieldDefines->GetColumnType(0);
        
        std::string sql;
        
        if(columnType == FT_INTEGER)
        {
            sql = "select * from "+ targetLayer->GetTableName() + " where "+primaryKeyName+ " = "+ id;
        }
        else if(columnType == FT_TEXT)
        {
            sql = "select * from "+ targetLayer->GetTableName() + " where "+primaryKeyName+ " = '"+ id +"'";
        }
        
        targetLayer->ResetReadingBySQL(sql);
        
        Editor::DataFeature* feature = targetLayer->GetNextFeature();
        
        if(NULL==feature)
            return NULL;
        
        EditorGeometry::WkbGeometry* wkb = feature->GetAsWkb(geoIndex);
            
        geos::geom::Geometry* geo = DataTransfor::Wkb2Geo(wkb);
        
        if(m_pIsDbOpen)
        {
            pDataSource->Close(dataPath);
        }
        
        return geo;
    }
    
    
    std::vector<geos::geom::Geometry*> DataFunctor::GetGeometries(const std::vector<std::string>& ids, DATALAYER_TYPE layerType)
    {
        std::vector<geos::geom::Geometry*> geometries;

        if(ids.size() == 0)
        {
            return geometries;
        }
        Editor::DataManager* pDataManager = Editor::DataManager::getInstance();
        
        Editor::DataSource* pDataSource = pDataManager->getDataSourceByLayerType(layerType);
        
        if(NULL==pDataSource)
        {
            return geometries;
        }
        
        int sourceType = pDataSource->GetDataSourceType();
        
        std::string dataPath = "";
        
        if (DATASOURCE_COREMAP==sourceType)
        {
            dataPath = pDataManager->getFileSystem()->GetCoreMapDataPath();
        }
        else if (DATASOURCE_GDB==sourceType)
        {
            dataPath = pDataManager->getFileSystem()->GetEditorGDBDataFile();
        }

        if(strcmp(pDataSource->GetConnectionString().c_str(),"") == 0)
        {
            pDataSource->Open(dataPath);

            m_pIsDbOpen = true;
        }

        Editor::DataLayer* targetLayer = pDataSource->GetLayerByType(layerType);

        if(targetLayer == NULL)
        {
            return geometries;
        }

        FieldDefines* fieldDefines = targetLayer->GetFieldDefines();

        if(fieldDefines == NULL)
        {
            return geometries;
        }

        int geoIndex = fieldDefines->GetColumnIndex(Geometry_Str);

        if(geoIndex == -1)
        {
            return geometries;
        }

        std::string primaryKeyName = fieldDefines->GetColumnName(0);

        int columnType = fieldDefines->GetColumnType(0);

        std::string inStatement = "(";

        for(int i = 0; i< ids.size(); i++)
        {
            if(columnType == FT_INTEGER)
            {
                inStatement += ids[i];
            }
            else if(columnType == FT_TEXT)
            {
                inStatement += "'"+ ids[i] + "'";
            }

            if(i != ids.size()-1)
            {
                inStatement += ",";
            }
        }

        inStatement +=")";

        std::string sql = "select * from "+ targetLayer->GetTableName() + " where "+primaryKeyName+ " in "+ inStatement;

        targetLayer->ResetReadingBySQL(sql);

        std::vector<geos::geom::Geometry*> features;

        while(Editor::DataFeature* feature = targetLayer->GetNextFeature())
        {
        	EditorGeometry::WkbGeometry* wkb = feature->GetAsWkb(geoIndex);

            geos::geom::Geometry* geo = DataTransfor::Wkb2Geo(wkb);

            geometries.push_back(geo);

            DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);
        }

        if(m_pIsDbOpen)
        {
            pDataSource->Close(dataPath);
        }

        return geometries;
    }

    EditorGeometry::WkbGeometry* DataFunctor::GetGeoFromId(std::string id, int& dir)
	{
    	Editor::DataManager* dataManager = Editor::DataManager::getInstance();
		std::string dataPath= dataManager->getFileSystem()->GetEditorGDBDataFile();
		Editor::GdbDataSource* dataSource = dynamic_cast<Editor::GdbDataSource*>(dataManager->getDataSource(DATASOURCE_GDB));
        if (NULL==dataSource)
        {
            return NULL;
        }
		dataSource->Open(dataPath);
		Editor::DataLayer* layer = dataSource->GetLayerByType(DATALAYER_RDLINE);
		if(!layer)
		{
			return NULL;
		}
		std::string sql = "select * from gdb_rdLine where pid ='" + id +"'";
		layer->ResetReadingBySQL(sql);
		Editor::DataFeature* feature = layer->GetNextFeature();
        EditorGeometry::WkbGeometry* wkb = NULL;
        EditorGeometry::WkbGeometry* ret = NULL;
        if(feature != NULL)
        {
		    wkb = feature->GetAsWkb(1);
		    dir = feature->GetAsInteger(6);
		    ret = (EditorGeometry::WkbGeometry*)malloc(wkb->buffer_size());

		    memcpy(ret, wkb, wkb->buffer_size());
		    DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);
        }
		return ret;
	}
    
	EditorGeometry::WkbGeometry* DataFunctor::GetGeoByRowkey(std::string rowkey)
    {
        Editor::DataManager* dataManager = Editor::DataManager::getInstance();
		
		std::string coredataPath= dataManager->getFileSystem()->GetCoreMapDataPath();
		
		Editor::CoreMapDataSource* dataSource = dynamic_cast<Editor::CoreMapDataSource*>(dataManager->getDataSource(DATASOURCE_COREMAP));
        
        if (NULL==dataSource)
        {
            return NULL;
        }
		
		dataSource->Open(coredataPath);
		
		Editor::DataLayer* layer = dataSource->GetLayerByType(DATALAYER_TIPS);
		
        std::string sql = "select * from edit_tips where rowkey ='" + rowkey +"'";
		
		layer->ResetReadingBySQL(sql);
		
		Editor::DataFeature* feature = layer->GetNextFeature();
		
        EditorGeometry::WkbGeometry* wkb = NULL;
        EditorGeometry::WkbGeometry* ret = NULL;
		
        if(feature != NULL)
        {
        	FieldDefines* defines = layer->GetFieldDefines();
        	if(defines != NULL)
        	{
        		wkb = feature->GetAsWkb(defines->GetColumnIndex("g_location"));
        	}

        	ret = (EditorGeometry::WkbGeometry*)malloc(wkb->buffer_size());

        	memcpy(ret, wkb, wkb->buffer_size());
        DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);
        }
		
		return ret;
    }

    std::vector<DataFeature*> DataFunctor::GetGpsTipsByGeoUuids(const std::vector<std::string>& uuids)
    {
        std::vector<DataFeature*> features;

        if(uuids.size() == 0)
        {
            return features;
        }
        Editor::DataManager* pDataManager = Editor::DataManager::getInstance();

        Editor::CoreMapDataSource* pDataSource = dynamic_cast<Editor::CoreMapDataSource*>(pDataManager->getDataSource(DATASOURCE_COREMAP));
        if (NULL==pDataSource)
        {
            return features;
        }

        std::string coredataPath= pDataManager->getFileSystem()->GetCoreMapDataPath();

        if(strcmp(pDataSource->GetConnectionString().c_str(),"") == 0)
        {
            pDataSource->Open(coredataPath);

            m_pIsDbOpen = true;
        }

        std::string uuidInStatement = Tools::GetInStatement(uuids,1);

        Editor::DataLayer* geoComponentLayer = pDataSource->GetLayerByType(DATALAYER_TIPSGEOCOMPONENT);

        FieldDefines* geoComponFieldDefines = geoComponentLayer->GetFieldDefines();

        if(geoComponFieldDefines == NULL)
        {
            Logger::LogD("GetGpsTipsByGeoUuids, get geoComponFieldDefines failed");
			Logger::LogO("GetGpsTipsByGeoUuids, get geoComponFieldDefines failed");

            return features;
        }

        int tipsRowkeyIndex = geoComponFieldDefines->GetColumnIndex("tipsRowkey");

        if(tipsRowkeyIndex == -1)
        {
            Logger::LogD("GetGpsTipsByGeoUuids, get column tipsRowkey failed");
			Logger::LogO("GetGpsTipsByGeoUuids, get column tipsRowkey failed");

            return features;
        }

        std::string sql = "select * from "+ geoComponentLayer->GetTableName() + " where geoUuid in "+ uuidInStatement;

        geoComponentLayer->ResetReadingBySQL(sql);

        std::vector<std::string> tipsRowkeys;

        while(DataFeature* feature = geoComponentLayer->GetNextFeature())
        {
            tipsRowkeys.push_back(feature->GetAsString(tipsRowkeyIndex));

            DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);
        }

        if(tipsRowkeys.size() == 0)
        {
            return features;
        }

        std::string rowkeyInStatement = Tools::GetInStatement(tipsRowkeys,1);

        DataLayer* tipsLayer = pDataSource->GetLayerByType(DATALAYER_TIPS);

        sql = "select * from "+ tipsLayer->GetTableName() + " where rowkey in "+ rowkeyInStatement + " and s_sourceType ='2001'";

        tipsLayer->ResetReadingBySQL(sql);

        while(DataFeature* tipsFeature = tipsLayer->GetNextFeature())
        {
            tipsFeature->SyncDataToMemory();

            features.push_back(tipsFeature);
        }

        if(m_pIsDbOpen)
        {
            pDataSource->Close(coredataPath);
        }

        return features;
    }
    
    std::vector<Editor::DataFeature*> DataFunctor::QueryFeatures(const std::string& tableName, const std::string& searchQuery)
    {
        std::vector<Editor::DataFeature*> result;
        
        if(std::strcmp(tableName.c_str(), "") ==0 || std::strcmp(searchQuery.c_str(), "") ==0)
        {
            return result;
        }
        
        Editor::DataManager* pDataManager = Editor::DataManager::getInstance();
        
        Editor::DataSource* pDataSource = pDataManager->getDataSource(tableName);
        
        if (NULL==pDataSource)
        {
            return result;
        }
        
        std::string dataPath= pDataSource->GetDataSourcePath();
        
        if(strcmp(dataPath.c_str(),"") == 0)
        {
            Logger::LogD("DataFunctor: GetFeature failed, Data Path Empty");
            Logger::LogO("DataFunctor: GetFeature failed, Data Path Empty");
            return result;
        }
        
        if(strcmp(pDataSource->GetConnectionString().c_str(),"") == 0)
        {
            pDataSource->Open(dataPath);
            
            m_pIsDbOpen = true;
        }
        
        Editor::DataLayer* layer = pDataSource->GetLayerByName(tableName);
        
        if(NULL == layer)
        {
            Logger::LogD("DataFunctor: GetFeature failed, can not get target layer , layerName:[%s]",tableName.c_str());
            Logger::LogO("DataFunctor: GetFeature failed, can not get target layer , layerName:[%s]",tableName.c_str());
            return result;
        }
        
        rapidjson::Document doc;
        
        doc.Parse<0>(searchQuery.c_str());
        
        if(doc.HasParseError())
        {
            Logger::LogD("DataFunctor: GetFeature failed, searchQuery is not a valid json");
            Logger::LogO("DataFunctor: GetFeature failed, searchQuery is not a valid json");
            return result;
        }
        
        std::string querySql;
        
        for(rapidjson::Document::MemberIterator ptr = doc.MemberBegin(); ptr != doc.MemberEnd(); ++ptr)
        {
            std::string  key = (ptr->name).GetString();
            
            rapidjson::Value &valueKey = (ptr->value);
            
            if(!valueKey.IsString() && !valueKey.IsNumber())
            {
                continue;
            }
            
            if(valueKey.IsNumber())
            {
                querySql += " " + key +" = " + Tools::NumberToString(valueKey.GetInt());
            }
            else if(valueKey.IsString())
            {
                querySql += " " + key +" = '" + valueKey.GetString() + "'";
            }
            
            querySql += " AND";
            
        }
        
        if(strcmp(querySql.c_str(), "") ==0)
        {
            return result;
        }
        
        std::string newSql = querySql = querySql.substr(0, querySql.length()-3);
        
        std::string sql ="SELECT * FROM "+ tableName + " WHERE "+ newSql;
        
        layer->ResetReadingBySQL(sql);
        
        while(Editor::DataFeature* feature = layer->GetNextFeature())
        {
            feature->SyncDataToMemory();
            
            result.push_back(feature);
        }
        
        if(m_pIsDbOpen)
        {
            pDataSource->Close(dataPath);
        }
        
        return result;
    }
    
    std::vector<Editor::DataFeature*> DataFunctor::QueryFeaturesBySql(const std::string& tableName, const std::string& sql)
    {
        std::vector<Editor::DataFeature*> result;
        
        if(std::strcmp(tableName.c_str(), "") ==0 || std::strcmp(sql.c_str(), "") ==0)
        {
            return result;
        }
        
        Editor::DataManager* pDataManager = Editor::DataManager::getInstance();
        
        Editor::DataSource* pDataSource = pDataManager->getDataSource(tableName);
        
        if (NULL==pDataSource)
        {
            return result;
        }
        
        std::string dataPath= pDataSource->GetDataSourcePath();
        
        if(strcmp(dataPath.c_str(),"") == 0)
        {
            Logger::LogD("DataFunctor: QueryFeaturesBySql failed, Data Path Empty");
            Logger::LogO("DataFunctor: QueryFeaturesBySql failed, Data Path Empty");
            return result;
        }
        
        if(strcmp(pDataSource->GetConnectionString().c_str(),"") == 0)
        {
            pDataSource->Open(dataPath);
            
            m_pIsDbOpen = true;
        }
        
        Editor::DataLayer* layer = pDataSource->GetLayerByName(tableName);
        
        if(NULL == layer)
        {
            Logger::LogD("DataFunctor: QueryFeaturesBySql failed, can not get target layer , layerName:[%s]",tableName.c_str());
            Logger::LogO("DataFunctor: QueryFeaturesBySql failed, can not get target layer , layerName:[%s]",tableName.c_str());
            return result;
        }
        
        if(layer->ResetReadingBySQL(sql) != 0)
        {
            Logger::LogD("DataFunctor: QueryFeaturesBySql failed, invalid sql, sql:[%s]",sql.c_str());
            Logger::LogO("DataFunctor: QueryFeaturesBySql failed, invalid sql, sql:[%s]",sql.c_str());
            
            return result;
        }
        
        while(Editor::DataFeature* feature = layer->GetNextFeature())
        {
            feature->SyncDataToMemory();
            
            result.push_back(feature);
        }
        
        if(m_pIsDbOpen)
        {
            pDataSource->Close(dataPath);
        }
        
        return result;
    }
    
    int DataFunctor::InsertFeature(const std::string& tableName, const std::string& json)
    {
        if(strcmp(tableName.c_str(), "")==0 || strcmp(json.c_str(), "")==0)
        {
            Logger::LogD("InsertFeature failed: tablename or json is invalid");
            Logger::LogO("InsertFeature failed: tablename or json is invalid");
            
            return -1;
        }
        
        rapidjson::Document document;
        
        document.Parse(json.c_str());
        
        if (document.HasParseError())
        {
            Logger::LogD("InsertFeature failed: inputJson json is invalid");
            Logger::LogO("InsertFeature failed: inputJson json is invalid");
            
            return -1;
        }
        
        Editor::DataManager* dataManager = Editor::DataManager::getInstance();
        
        Editor::DataSource* dataSource = dataManager->getDataSource(tableName);
        
        if(NULL == dataSource)
        {
            Logger::LogD("InsertFeature failed: no such table. Table name: [%s].",tableName.c_str());
            Logger::LogO("InsertFeature failed: no such table. Table name: [%s].",tableName.c_str());
            
            return -1;
        }
        
        try
        {
            std::string dataPath = dataSource->GetDataSourcePath();
            
            dataSource->Open(dataPath);
            
            Editor::DataLayer* layer = dataSource->GetLayerByName(tableName);
            
            if(NULL == layer)
            {
                Logger::LogD("InsertFeature failed: no such layer. Table name: [%s].",tableName.c_str());
                Logger::LogO("InsertFeature failed: no such layer. Table name: [%s].",tableName.c_str());
                
                return -1;
            }
            
            const DataFeatureFactory* factory = DataFeatureFactory::getDefaultInstance();
            
            DataFeature* feature = factory->CreateFeature(layer);
            
            Editor::JSON jsonObject;
            
            jsonObject.SetJsonString(json);
            
            feature->SetTotalPart(jsonObject);
            
            int result = layer->InsertFeature(feature);
            
            if(result == -1)
            {
                DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);
                
                dataSource->Close(dataPath);
                
                return -1;
            }
            
            DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);
            
            dataSource->Close(dataPath);
            
            return result;
        }
        
        catch(std::exception &e)
        {
            Logger::LogD("Editor: Insert Feature failed , Exception:[%s],type[%s]", e.what(),typeid(e).name());
            Logger::LogO("Editor: Insert Feature failed , Exception:[%s],type[%s]", e.what(),typeid(e).name());
            
            return -1;
        }
    }
    
    int DataFunctor::UpdateFeature(const std::string& tableName, const std::string& json)
    {
        if(strcmp(tableName.c_str(), "")==0 || strcmp(json.c_str(), "")==0)
        {
            Logger::LogD("UpdateFeature failed: tablename or json is invalid");
            Logger::LogO("UpdateFeature failed: tablename or json is invalid");
            
            return -1;
        }
        
        rapidjson::Document document;
        
        document.Parse(json.c_str());
        
        if (document.HasParseError())
        {
            Logger::LogD("UpdateFeature failed: inputJson json is invalid");
            Logger::LogO("UpdateFeature failed: inputJson json is invalid");
            
            return -1;
        }
        
        Editor::DataManager* dataManager = Editor::DataManager::getInstance();
        
        Editor::DataSource* dataSource = dataManager->getDataSource(tableName);
        
        if(NULL == dataSource)
        {
            Logger::LogD("UpdateFeature failed: no such table. Table name: [%s].",tableName.c_str());
            Logger::LogO("UpdateFeature failed: no such table. Table name: [%s].",tableName.c_str());
            
            return -1;
        }
        
        try
        {
            std::string dataPath = dataSource->GetDataSourcePath();
            
            dataSource->Open(dataPath);
            
            Editor::DataLayer* layer = dataSource->GetLayerByName(tableName);
            
            if(NULL == layer)
            {
                Logger::LogD("UpdateFeature failed: no such layer. Table name: [%s].",tableName.c_str());
                Logger::LogO("UpdateFeature failed: no such layer. Table name: [%s].",tableName.c_str());
                
                return -1;
            }
            
            const DataFeatureFactory* factory = DataFeatureFactory::getDefaultInstance();
            
            DataFeature* feature = factory->CreateFeature(layer);
            
            Editor::JSON jsonObject;
            
            jsonObject.SetJsonString(json);
            
            feature->SetTotalPart(jsonObject);
            
            Editor::DataFeature* featureOld = layer->GetFeatureByRowId(feature->GetRowId());
            
            if(featureOld==NULL)
            {
                Logger::LogD("Editor: UpdateFeature failed, featureOld is NULL!");
                Logger::LogO("Editor: UpdateFeature failed, featureOld is NULL!");
                return -1;
            }
            
            DataFeatureFactory::getDefaultInstance()->DestroyFeature(featureOld);
            
            int result = layer->UpdateFeature(feature);
            
            if(result == -1)
            {
                DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);
                
                dataSource->Close(dataPath);
                
                return -1;
            }
            
            DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);
            
            dataSource->Close(dataPath);
            
            return result;
        }
        
        catch(std::exception &e)
        {
            Logger::LogD("Editor: Update Feature failed, Exception:[%s],type[%s]", e.what(),typeid(e).name());
            Logger::LogO("Editor: Update Feature failed, Exception:[%s],type[%s]", e.what(),typeid(e).name());
            
            return -1;
        }
    }
    
    int DataFunctor::DeleteFeature(const std::string& tableName, int rowId)
    {
        if(strcmp(tableName.c_str(), "")==0)
        {
            Logger::LogD("DeleteFeature failed: tablename is invalid");
            Logger::LogO("DeleteFeature failed: tablename is invalid");
            
            return -1;
        }
        
        Editor::DataManager* dataManager = Editor::DataManager::getInstance();
        
        Editor::DataSource* dataSource = dataManager->getDataSource(tableName);
        
        if(NULL == dataSource)
        {
            Logger::LogD("DeleteFeature failed: no such table. Table name: [%s].",tableName.c_str());
            Logger::LogO("DeleteFeature failed: no such table. Table name: [%s].",tableName.c_str());
            
            return -1;
        }
        
        try
        {
            std::string dataPath = dataSource->GetDataSourcePath();
            
            dataSource->Open(dataPath);
            
            Editor::DataLayer* layer = dataSource->GetLayerByName(tableName);
            
            if(NULL == layer)
            {
                Logger::LogD("DeleteFeature failed: no such layer. Table name: [%s].",tableName.c_str());
                Logger::LogO("DeleteFeature failed: no such layer. Table name: [%s].",tableName.c_str());
                
                return -1;
            }
            
            const DataFeatureFactory* factory = DataFeatureFactory::getDefaultInstance();
            
            Editor::DataFeature* featureOld = layer->GetFeatureByRowId(rowId);
            
            if(featureOld==NULL)
            {
                Logger::LogD("Editor: DeleteFeature failed, featureOld is NULL!");
                Logger::LogO("Editor: DeleteFeature failed, featureOld is NULL!");
                return -1;
            }
            
            int result = layer->DeleteFeature(featureOld);
            
            if(result == -1)
            {
                DataFeatureFactory::getDefaultInstance()->DestroyFeature(featureOld);
                
                dataSource->Close(dataPath);
                
                return -1;
            }
            
            DataFeatureFactory::getDefaultInstance()->DestroyFeature(featureOld);
            
            dataSource->Close(dataPath);
            
            return result;
        }
        
        catch(std::exception &e)
        {
            Logger::LogD("Editor: Delete Feature failed, Exception:[%s],type[%s]", e.what(),typeid(e).name());
            Logger::LogO("Editor: Delete Feature failed, Exception:[%s],type[%s]", e.what(),typeid(e).name());
            
            return -1;
        }
    }
}