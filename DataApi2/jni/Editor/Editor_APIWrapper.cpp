#include "Editor_APIWrapper.h"
#include "Editor_ProjectManager.h"
#include "Editor_ResumeProcessor.h"
#include "Editor_MetaDataManager.h"
#include "Editor_Data.h"
#include "BussinessConstants.h"
#include "GeometryCalculator.h"
#include <typeinfo>
#include <boost/lexical_cast.hpp>
#include"Editor_Track.h"
#include <document.h>
#include <stringbuffer.h>
#include <writer.h>

using namespace std;

#define DFirstPoint 1
#define DSecondPoint 2
#define DEndPoint 999

namespace Editor
{
    APIWrapper* APIWrapper::m_pInstance = NULL;

    APIWrapper::APIWrapper()
    {
        m_pDataManager = DataManager::getInstance();
        
        m_pCoreDataSource = (CoreMapDataSource*)m_pDataManager->getDataSource(DATASOURCE_COREMAP);
        
        m_pGdbDataSource = (GdbDataSource*)m_pDataManager->getDataSource(DATASOURCE_GDB);
    }


    APIWrapper* APIWrapper::getInstance()
    {
        if(NULL == m_pInstance)
        {
            m_pInstance = new  APIWrapper();
        }

        return m_pInstance;
    }

    //Path & User
    void APIWrapper::SetRootPath(const char* rootPath)
    {
        m_pDataManager->getFileSystem()->SetRootPath(rootPath);
    }

    void APIWrapper::SetUserId(const char* userId)
    {
        m_pDataManager->getFileSystem()->SetCurrentUser(userId);
    }

    std::string APIWrapper::GetDataPath()
    {
        std::string dataPath = m_pDataManager->getFileSystem()->GetCoreMapDataPath();

        return dataPath;
    }

    std::string APIWrapper::GetPoiPhotoPath()
    {
        std::string poiPhotoPath = m_pDataManager->getFileSystem()->GetPoiPhotoPath();

        return poiPhotoPath;
    }

    std::string APIWrapper::GetTipsPhotoPath()
    {
        std::string tipsPhotoPath = m_pDataManager->getFileSystem()->GetTipsPhotoPath();

        return tipsPhotoPath;
    }

    //Tips
    int APIWrapper::InsertTips(const std::string& jsonTips)
    {
        std::string dataPath= m_pDataManager->getFileSystem()->GetCoreMapDataPath();
        
        if(strcmp(dataPath.c_str(),"") == 0)
        {
            Logger::LogD("Editor: InsertTips failed, Data Path Empty");
            Logger::LogO("Editor: InsertTips failed, Data Path Empty");
            return -1;
        }
        
        m_pCoreDataSource->Open(dataPath);

        try{
            DataLayer* layer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPS);

            const DataFeatureFactory* factory = DataFeatureFactory::getDefaultInstance();
            
            DataFeature* featureTips = factory->CreateFeature(layer);
            
            rapidjson::Document doc;
            
            doc.Parse<0>(jsonTips.c_str());
            
            if(doc.HasParseError() || !doc.HasMember("g_location") || !doc.HasMember("s_sourceType"))
            {
                m_pCoreDataSource->Close(dataPath);
                return -1;
            }
            
            if(doc["g_location"].IsNull() || doc["s_sourceType"].IsNull())
            {
                m_pCoreDataSource->Close(dataPath);
                return -1;
            }
            
            std::string location = doc["g_location"].GetString();
            
            EditorGeometry::Point2D point2D = DataTransfor::WktToPoint2D(location);

            if(point2D._x <0 || point2D._y <0)
            {
                m_pCoreDataSource->Close(dataPath);
                return -1;
            }

            std::string type = doc["s_sourceType"].GetString();
            
            if(strlen(type.c_str()) == 1)
            {
                type = "0"+type;
            }
            
            std::string currentTime = Tools::GetCurrentDateTime();
            
            Editor::JSON jsonObject;
            
            jsonObject.SetJsonString(jsonTips);
            
            featureTips->SetTotalPart(jsonObject);
            
            std::string displaystyle = DataTransfor::GetTipsDisplayStyle(jsonTips);
            
            int userId = Tools::StringToNum(m_pDataManager->getFileSystem()->GetCurrentUserId());

            std::string rowkey;

			if (!doc.HasMember("rowkey")
				||doc["rowkey"].IsNull()
				||(rowkey=doc["rowkey"].GetString())=="")
			{
				rowkey = Tools::GetTipsRowkey(type);
			}
            else
            {
                rowkey = doc["rowkey"].GetString();
            }
			
            featureTips->SetAsString(0, rowkey);
            featureTips->SetAsInteger(10, LIFECYCLE_ADD);
            featureTips->SetAsInteger(11, TSTATUS_WORKED);
            featureTips->SetAsString(12, currentTime);
            featureTips->SetAsInteger(13, userId);
            featureTips->SetAsString(15, displaystyle);

            if(type == Tools::NumberToString(Model::Bridge) || type == Tools::NumberToString(Model::UnderConstruction))
            {
                std::string geoWkt = DataTransfor::GetSETipsGeo(jsonTips);

                if(strcmp(geoWkt.c_str(),"") != 0)
                {
                    EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(geoWkt);

                    FieldDefines* fieldDefines = featureTips->GetFieldDefines();
                    
                    int index = fieldDefines->GetColumnIndex("g_location");
                    
                    if(index != -1)
                    {
                        featureTips->SetAsWkb(index, wkb);
                    }

                    delete[] wkb;
                }
            }

			featureTips->SetAsInteger(19, 0);
            
            int result = layer->InsertFeature(featureTips);
            
            if(result == -1)
            {
                DataFeatureFactory::getDefaultInstance()->DestroyFeature(featureTips);

                m_pCoreDataSource->Close(dataPath);

                return -1;
            }

            result = InsertTipsGeo(featureTips);

            if(result != -1)
            {
                result = MaintainSpecialTips(featureTips, OPERATION_INSERT);
            }

            DataFeatureFactory::getDefaultInstance()->DestroyFeature(featureTips);

            m_pCoreDataSource->Close(dataPath);

            return result;
        }
        catch(exception &e)
        {
            Logger::LogD("Editor: Insert Tips failed In APIWrapper, Exception:[%s],type[%s]", e.what(),typeid(e).name());
            Logger::LogO("Editor: Insert Tips failed In APIWrapper, Exception:[%s],type[%s]", e.what(),typeid(e).name());
            
            return -1;
        }
    }

    int APIWrapper::UpdateTips(const std::string& jsonTips)
    {
        try{
            std::string dataPath= m_pDataManager->getFileSystem()->GetCoreMapDataPath();
            
            if(strcmp(dataPath.c_str(),"") == 0)
            {
                Logger::LogD("Editor: UpdateTips failed, Data Path Empty");
                Logger::LogO("Editor: UpdateTips failed, Data Path Empty");
                return -1;
            }
            
            m_pCoreDataSource->Open(dataPath);

            DataLayer* layer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPS);
            
            rapidjson::Document doc;
            
            doc.Parse<0>(jsonTips.c_str());
            
            if(doc.HasParseError() || !doc.HasMember("s_sourceType"))
            {
                m_pCoreDataSource->Close(dataPath);
                return -1;
            }
            std::string type = doc["s_sourceType"].GetString();

            const DataFeatureFactory* factory = DataFeatureFactory::getDefaultInstance();
            
            DataFeature* featureTips = factory->CreateFeature(layer);
            
            JSON jsonObject;
            
            jsonObject.SetJsonString(jsonTips);
            featureTips->SetTotalPart(jsonObject);
            
            Editor::DataFeature* featureTipsOld = layer->GetFeatureByRowId(featureTips->GetRowId());
            
            if(featureTipsOld==NULL)
            {
                Logger::LogD("Editor: UpdateTips failed, featureTipsOld is NULL!");
                Logger::LogO("Editor: UpdateTips failed, featureTipsOld is NULL!");
                return -1;
            }
        
            int oLifecycle = featureTipsOld->GetAsInteger(10);
            int newLifecycle = featureTips->GetAsInteger(10);

            if(newLifecycle == Editor::LIFECYCLE_DELETE)
            {
                featureTips->SetAsInteger(10, Editor::LIFECYCLE_DELETE);
            }
            else if (oLifecycle == Editor::LIFECYCLE_ADD)
            {
                featureTips->SetAsInteger(10, Editor::LIFECYCLE_ADD);
            }
            else
            {
                featureTips->SetAsInteger(10, Editor::LIFECYCLE_UPDATE);
            }

            int userId = Tools::StringToNum(m_pDataManager->getFileSystem()->GetCurrentUserId());
            featureTips->SetAsInteger(11, TSTATUS_WORKED);
            featureTips->SetAsInteger(13, userId);

            std::string currentTime = Tools::GetCurrentDateTime();
            featureTips->SetAsString(12,currentTime);
            
            std::string displaystyle = DataTransfor::GetTipsDisplayStyle(jsonTips);
            featureTips->SetAsString(15, displaystyle);
            if(!doc.HasMember("g_location") && doc["g_location"].IsNull())
            {
                m_pCoreDataSource->Close(dataPath);
                return -1;
            }
            
            if(type == Tools::NumberToString(Model::Bridge) || type == Tools::NumberToString(Model::UnderConstruction))
            {
                std::string geoWkt = DataTransfor::GetSETipsGeo(jsonTips);

                if(strcmp(geoWkt.c_str(),"") != 0)
                {
                    EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(geoWkt);

                    FieldDefines* fieldDefines = featureTips->GetFieldDefines();
                    
                    int index = fieldDefines->GetColumnIndex("g_location");
                    
                    if(index != -1)
                    {
                        featureTips->SetAsWkb(index, wkb);
                    }

                    delete[] wkb;
                }
            }

            int result = layer->UpdateFeature(featureTips);
            
            if(result != 0)
            {
                DataFeatureFactory::getDefaultInstance()->DestroyFeature(featureTips);
                
                m_pCoreDataSource->Close(dataPath);

                return -1;
            }

            result = UpdateTipsGeo(featureTips);

            if(result != -1)
            {
                result = MaintainSpecialTips(featureTips, OPERATION_UPDATE);
            }

            DataFeatureFactory::getDefaultInstance()->DestroyFeature(featureTips);

            m_pCoreDataSource->Close(dataPath);

            return result;
        }
        catch(exception &e)
        {
            Logger::LogD("Editor: Update Tips failed In APIWrapper, Exception:[%s],type[%s]", e.what(),typeid(e).name());
            Logger::LogO("Editor: Update Tips failed In APIWrapper, Exception:[%s],type[%s]", e.what(),typeid(e).name());
            
            return -1;
        }
    }

    int APIWrapper::DeleteTips(const std::string& rowkey)
    {
        try{
        	int result = 0;
            std::string dataPath= m_pDataManager->getFileSystem()->GetCoreMapDataPath();
            
            if(strcmp(dataPath.c_str(),"") == 0)
            {
                Logger::LogD("Editor: InsertTips failed, Data Path Empty");
                Logger::LogO("Editor: InsertTips failed, Data Path Empty");
                return -1;
            }
            
            m_pCoreDataSource->Open(dataPath);

            DataLayer* layer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPS);
            
		    Logger::LogD("Editor: DeleteTips openlayer finished");
	        Logger::LogO("Editor: DeleteTips openlayer finished");
		
	        std::string table_name_string = layer->GetTableName();

	        std::string sql ="SELECT * FROM "+ table_name_string + " WHERE rowkey = '"+ rowkey +"'";

	        layer->ResetReadingBySQL(sql);

		    Logger::LogD("Editor: DeleteTips sql:[%s]",sql.c_str());
	        Logger::LogO("Editor: DeleteTips sql:[%s]",sql.c_str());
		
	        DataFeature* feature = layer->GetNextFeature();

	        if(feature == NULL)
	        {
	    	    Logger::LogD("Editor: DeleteTips failed, tips not found, rowkey: [%s]", rowkey.c_str());
	    	    Logger::LogO("Editor: DeleteTips failed, tips not found, rowkey: [%s]", rowkey.c_str());
	    	    return 0;
	        }

			FieldDefines* pFieldDefines = layer->GetFieldDefines();
			if (!pFieldDefines)
			{
				m_pCoreDataSource->Close(dataPath);
				Logger::LogD("Editor: layer->GetFieldDefines failed");
				Logger::LogO("Editor: layer->GetFieldDefines failed");
				DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);
				return -1;
			}
			unsigned int lifecycleIndex = pFieldDefines->GetColumnIndex("t_lifecycle");
			int lifecycleValue = feature->GetAsInteger(lifecycleIndex);

			unsigned int statusIndex =  pFieldDefines->GetColumnIndex("t_status");
			unsigned int operateDateIndex =  pFieldDefines->GetColumnIndex("t_operateDate");
			unsigned int handlerIndex =  pFieldDefines->GetColumnIndex("t_handler");
			
            unsigned int  displayStyleIndex =  pFieldDefines->GetColumnIndex("display_style");
            
            std::string displayStyle = feature->GetAsString(displayStyleIndex);
            
            rapidjson::Document doc;
            doc.SetObject();
            rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
            
            doc.Parse<0>(displayStyle.c_str());
            
            doc["t_lifecycle"] = (int)LIFECYCLE_DELETE;
            
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            doc.Accept(writer);
            
            std::string displayStyleNew = buffer.GetString();
            
			if (lifecycleValue==3)
			{
				unsigned int syncIndex = pFieldDefines->GetColumnIndex("t_sync");
				int syncValue = feature->GetAsInteger(syncIndex);
				Logger::LogD("Editor: Delete tips syncValue[%d]",syncValue);
				Logger::LogO("Editor: Delete tips syncValue[%d]",syncValue);
				
				if (syncValue==0)
				{
					DeleteTipsGeo(feature);
					result = layer->DeleteFeature(feature);
                    
                    if(result != -1)
                    {
                        result = MaintainSpecialTips(feature, OPERATION_DELETE);
                    }
                    
					Logger::LogD("Editor: Delete tips----DeleteFeature[%d]",result);
					Logger::LogO("Editor: Delete tips----DeleteFeature[%d]",result);
				}
				else if (syncValue==1)
				{
					feature->SetAsInteger(lifecycleIndex, LIFECYCLE_DELETE);
					int userId = Tools::StringToNum(m_pDataManager->getFileSystem()->GetCurrentUserId());
					feature->SetAsInteger(statusIndex, TSTATUS_WORKED);
					feature->SetAsString(operateDateIndex, Tools::GetCurrentDateTime());
					feature->SetAsInteger(handlerIndex, userId);
                    feature->SetAsString(displayStyleIndex, displayStyleNew);
                    
					result = layer->UpdateFeature(feature);
                    
                    if(result != -1)
                    {
                        result = UpdateTipsGeo(feature);
                    }
                    
					if(result != -1)
					{
						result = MaintainSpecialTips(feature, OPERATION_DELETE);
					}
				}
				
			}
			else
			{
				feature->SetAsInteger(lifecycleIndex, LIFECYCLE_DELETE);
				int userId = Tools::StringToNum(m_pDataManager->getFileSystem()->GetCurrentUserId());
				feature->SetAsInteger(statusIndex, TSTATUS_WORKED);
				feature->SetAsString(operateDateIndex, Tools::GetCurrentDateTime());
				feature->SetAsInteger(handlerIndex, userId);
                feature->SetAsString(displayStyleIndex, displayStyleNew);
                
				result = layer->UpdateFeature(feature);

                if(result != -1)
                {
                    result = UpdateTipsGeo(feature);
                }
                
				if(result != -1)
				{
					result = MaintainSpecialTips(feature, OPERATION_DELETE);
				}
			}
			
			DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);
			
			m_pCoreDataSource->Close(dataPath);
			Logger::LogD("Editor: Delete tips result[%d]",result);
			Logger::LogO("Editor: Delete tips result[%d]",result);
		
		    return result;
		}
        catch(exception &e)
        {
            Logger::LogD("Editor: Delete Tips failed In APIWrapper, Exception:[%s],type[%s]", e.what(),typeid(e).name());
            Logger::LogO("Editor: Delete Tips failed In APIWrapper, Exception:[%s],type[%s]", e.what(),typeid(e).name());
            
            return -1;
        }
    }

    int APIWrapper::InsertTipsGeo(DataFeature* tips)
    {
        try
        {
            if(tips == NULL)
            {
                return -1;
            }

            std::string dataPath= m_pDataManager->getFileSystem()->GetCoreMapDataPath();

            if(strcmp(dataPath.c_str(),"") == 0)
            {
    	        Logger::LogD("Editor: InsertTipsGeo failed, Data Path Empty");
    	        Logger::LogO("Editor: InsertTipsGeo failed, Data Path Empty");
                return -1;
            }

            FEATURE_GEOTYPE geoType = tips->GetGeoType(Tools::StringToNum(tips->GetAsString(5)));

            if(geoType == GEO_POINT)
            {
                if(InsertGeoPoint(tips, tips->GetAsWkb(8), ICON) == -1)
                {
                    Logger::LogD("Maintain geopoint unsuccessfully");
                    Logger::LogO("Maintain geopoint unsuccessfully");

                    return -1;
                }
            }
            else if(geoType == GEO_LINE)
            {
                if(InsertGeoLine(tips) == -1)
                {
                    Logger::LogD("Maintain geoline unsuccessfully");
                    Logger::LogO("Maintain geoline unsuccessfully");

                    return -1;
                }
            }
            else if (geoType == GEO_MIXED)
            {
                if(InsertOrUpdateGeoMixed(tips, OPERATION_INSERT)== -1)
                {
                    Logger::LogD("Maintain geoMixed unsuccessfully");
                    Logger::LogO("Maintain geoMixed unsuccessfully");
                    
                    return -1;
                }
            }
            else if(geoType == GEO_POLYGON)
            {
                if(InsertGeoPolygon(tips) == -1)
                {
                    Logger::LogD("Maintain geoPolygon unsuccessfully");
                    Logger::LogO("Maintain geoPolygon unsuccessfully");
                    
                    return -1;
                }
            }
            
            int ret = MaintainAttachmentsGeo(tips, OPERATION_INSERT);

            return ret;
        }
        catch(...)
        {
            Logger::LogD("Editor: Insert Tips Geo failed In APIWrapper");
            Logger::LogO("Editor: Insert Tips Geo failed In APIWrapper");
            return -1;
        }
    }

    int APIWrapper::InsertGeoPoint(DataFeature* tips, EditorGeometry::WkbGeometry* wkb, TIPS_GEO_COMPONENT_TYPE type, const std::string& pointDisplayStyle)
    {
        DataLayer* tipsPointlayer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPSPOINT);
            
        const DataFeatureFactory* factory = DataFeatureFactory::getDefaultInstance();
            
        DataFeature* featureTipsPoint = factory->CreateFeature(tipsPointlayer);

        std::string uuid = Tools::GenerateUuid();

        if(!uuid.empty())
        {
            featureTipsPoint->SetAsString(0,uuid);
        }

        FEATURE_GEOTYPE geoType = tips->GetGeoType(Tools::StringToNum(tips->GetAsString(5)));

        featureTipsPoint->SetAsInteger(1,(int)geoType);

        std::string sourceType = tips->GetAsString(5);

        featureTipsPoint->SetAsString(2,sourceType);
        
        if (pointDisplayStyle=="") {
            std::string displayStyle = tips->GetAsString(15);
            
            featureTipsPoint->SetAsString(3,displayStyle);
        }
        else {
            featureTipsPoint->SetAsString(3,pointDisplayStyle);
        }

        featureTipsPoint->SetAsWkb(4,wkb);

        featureTipsPoint->SetAsInteger(5, TipsGeoVisible);

        int result = tipsPointlayer->InsertFeature(featureTipsPoint);

        if(result != 0)
        {
            factory->DestroyFeature(featureTipsPoint);

            return -1;
        }

        factory->DestroyFeature(featureTipsPoint);

        if(InsertGeoComponent(tips->GetAsString(0),uuid,tipsPointlayer->GetTableName(),type))
        {
            return -1;
        }

        return 0;
    }
    
     int  APIWrapper::InsertTipsGeoOnly(DataFeature* tips, EditorGeometry::WkbGeometry* wkb, TIPS_GEO_COMPONENT_TYPE type, const std::string& displayStyle)
    {
        if(wkb==NULL)
        {
            return -1;
        }
        DATALAYER_TYPE dataType;
        
        FEATURE_GEOTYPE geoType;
      
        if (wkb->_wkbType==EditorGeometry::wkbPoint)
        {
            dataType = DATALAYER_TIPSPOINT;
            geoType = GEO_POINT;
        }
        else if(wkb->_wkbType==EditorGeometry::wkbLineString)
        {
            dataType = DATALAYER_TIPSLINE;
             geoType = GEO_LINE;
        }
        else if(wkb->_wkbType == EditorGeometry::wkbMultiLineString)
        {
            dataType = DATALAYER_TIPSMULTILINE;
            geoType = GEO_MULTILINE;
        }
        else if(wkb->_wkbType==EditorGeometry::wkbPolygon)
        {
            dataType = DATALAYER_TIPSPOLYGON;
             geoType = GEO_POLYGON;
        }
        
        DataLayer* tipslayer = m_pCoreDataSource->GetLayerByType(dataType);
        
        if (tipslayer==NULL)
        {
            return -1;
        }
        
        const DataFeatureFactory* factory = DataFeatureFactory::getDefaultInstance();
        
        DataFeature* featureTips = factory->CreateFeature(tipslayer);
        
        std::string uuid = Tools::GenerateUuid();
        
        if(!uuid.empty())
        {
            featureTips->SetAsString(0,uuid);
        }
        
        featureTips->SetAsInteger(1,(int)geoType);
        
        std::string sourceType = tips->GetAsString(5);
        
        featureTips->SetAsString(2,sourceType);
        
        if (displayStyle=="")
        {
            std::string displayStyle = tips->GetAsString(15);
            
            featureTips->SetAsString(3,displayStyle);
        }
        else
        {
            featureTips->SetAsString(3,displayStyle);
        }
        
        featureTips->SetAsWkb(4,wkb);
        
        featureTips->SetAsInteger(5, TipsGeoVisible);
        
        int result = tipslayer->InsertFeature(featureTips);
        
        if(result != 0)
        {
            factory->DestroyFeature(featureTips);
            
            return -1;
        }
        
        factory->DestroyFeature(featureTips);
        
        if(InsertGeoComponent(tips->GetAsString(0),uuid,tipslayer->GetTableName(), type))
        {
            return -1;
        }
        
        return 0;
    }
    
    int APIWrapper::UpdateTipsGeoOnly(DataFeature* tips, EditorGeometry::WkbGeometry* wkb, TIPS_GEO_COMPONENT_TYPE type, const std::string& displayStyle)
    {
        if(wkb==NULL)
        {
            return -1;
        }
        DATALAYER_TYPE dataType;
        
        FEATURE_GEOTYPE geoType;
        
        if (wkb->_wkbType==EditorGeometry::wkbPoint)
        {
            dataType = DATALAYER_TIPSPOINT;
            geoType = GEO_POINT;
        }
        else if(wkb->_wkbType==EditorGeometry::wkbLineString)
        {
            dataType = DATALAYER_TIPSLINE;
            geoType = GEO_LINE;
        }
        else if(wkb->_wkbType == EditorGeometry::wkbMultiLineString)
        {
            dataType = DATALAYER_TIPSMULTILINE;
            geoType = GEO_MULTILINE;
        }
        else if(wkb->_wkbType==EditorGeometry::wkbPolygon)
        {
            dataType = DATALAYER_TIPSPOLYGON;
            geoType = GEO_POLYGON;
        }
        
        DataLayer* tipslayer = m_pCoreDataSource->GetLayerByType(dataType);
        
        if (tipslayer==NULL)
        {
            return -1;
        }
        
        std::string tipsRowkey = tips->GetAsString(0);
        
        DataLayer* tipsGeoComponentlayer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPSGEOCOMPONENT);
        
        std::string sql = "select * from "+ tipsGeoComponentlayer->GetTableName() + " where tipsRowkey = '"+ tipsRowkey + "' and geoTableName='" + tipslayer->GetTableName() + "' and geoType = " + Tools::NumberToString(type);
        
        tipsGeoComponentlayer->ResetReadingBySQL(sql);
        
        DataFeature* geoComponentFeature = tipsGeoComponentlayer->GetNextFeature();
        
        if(geoComponentFeature == NULL)
        {
            return -1;
        }
        
        std::string geoUuid = geoComponentFeature->GetAsString(1);
        
        const DataFeatureFactory* factory = DataFeatureFactory::getDefaultInstance();
        
        factory->DestroyFeature(geoComponentFeature);
        
        sql = "select * from "+ tipslayer->GetTableName() + " where uuid = '"+ geoUuid + "'";
        
        tipslayer->ResetReadingBySQL(sql);
        
        DataFeature* featureTips = tipslayer->GetNextFeature();
        
        if(featureTips == NULL)
        {
            return -1;
        }
        
        if (displayStyle!="")
        {
            featureTips->SetAsString(3,displayStyle);
        }
        else
        {
            std::string tipsDisplayStyle = tips->GetAsString(15);
            
            featureTips->SetAsString(3,tipsDisplayStyle);
        }
        
        featureTips->SetAsWkb(4,wkb);
        
        featureTips->SetAsInteger(5, TipsGeoVisible);
        
        int result = tipslayer->UpdateFeature(featureTips);
        
        factory->DestroyFeature(featureTips);
        
        return result;
    }

    int APIWrapper::UpdateGeoPoint(DataFeature* tips, EditorGeometry::WkbGeometry* wkb, TIPS_GEO_COMPONENT_TYPE type, const std::string& pointDisplayStyle)
    {
        DataLayer* tipsPointlayer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPSPOINT);

        std::string tipsRowkey = tips->GetAsString(0);

        DataLayer* tipsGeoComponentlayer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPSGEOCOMPONENT);

        std::string sql = "select * from "+ tipsGeoComponentlayer->GetTableName() + " where tipsRowkey = '"+ tipsRowkey + "' and geoTableName='" + tipsPointlayer->GetTableName() + "' and geoType = " + Tools::NumberToString(type);

        tipsGeoComponentlayer->ResetReadingBySQL(sql);

        DataFeature* geoComponentFeature = tipsGeoComponentlayer->GetNextFeature();

        if(geoComponentFeature == NULL)
        {
            return -1;
        }

        std::string geoUuid = geoComponentFeature->GetAsString(1);

        const DataFeatureFactory* factory = DataFeatureFactory::getDefaultInstance();

        factory->DestroyFeature(geoComponentFeature);
            
        sql = "select * from "+ tipsPointlayer->GetTableName() + " where uuid = '"+ geoUuid + "'";
            
        tipsPointlayer->ResetReadingBySQL(sql);

        DataFeature* featureTipsPoint = tipsPointlayer->GetNextFeature();

        if(featureTipsPoint == NULL)
        {
            return -1;
        }
        
        if (pointDisplayStyle!="")
        {
            featureTipsPoint->SetAsString(3,pointDisplayStyle);
        }
        else
        {
            std::string displayStyle = tips->GetAsString(15);
            
            featureTipsPoint->SetAsString(3,displayStyle);
        }

        featureTipsPoint->SetAsWkb(4,wkb);

        featureTipsPoint->SetAsInteger(5, TipsGeoVisible);

        int result = tipsPointlayer->UpdateFeature(featureTipsPoint);

        factory->DestroyFeature(featureTipsPoint);

        return result;
    }

    int APIWrapper::InsertGeoLine(DataFeature* tips)
    {
        
        DataLayer* tipLinelayer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPSLINE);

        const DataFeatureFactory* factory = DataFeatureFactory::getDefaultInstance();
            
        DataFeature* featureTipsLine = factory->CreateFeature(tipLinelayer);

        std::string uuid = Tools::GenerateUuid();

        if(!uuid.empty())
        {
            featureTipsLine->SetAsString(0,uuid);
        }

        FEATURE_GEOTYPE geoType = tips->GetGeoType(Tools::StringToNum(tips->GetAsString(5)));

        featureTipsLine->SetAsInteger(1,(int)geoType);

        std::string sourceType = tips->GetAsString(5);

        featureTipsLine->SetAsString(2,sourceType);
        
        std::string deep = "";
        
        FieldDefines* pFieldDefines = tips->GetFieldDefines();
        
        if (pFieldDefines)
        {
            int deepIndex = pFieldDefines->GetColumnIndex(Tips_Column_Deep);
            unsigned int length;
            unsigned char* blob = tips->GetAsBlob(deepIndex, length);
            deep = std::string(reinterpret_cast<char*>(blob),length);
        }
        
        std::string displayStyle = DataTransfor::GetTipsLineDisplayStyle(sourceType, deep);

        featureTipsLine->SetAsString(3,displayStyle);

        EditorGeometry::WkbGeometry* wkb = tips->GetAsWkb(8);

        featureTipsLine->SetAsWkb(4,wkb);

        featureTipsLine->SetAsInteger(5, TipsGeoVisible);

        int result = tipLinelayer->InsertFeature(featureTipsLine);

        if(result != 0)
        {
            factory->DestroyFeature(featureTipsLine);

            return -1;
        }

        factory->DestroyFeature(featureTipsLine);

        if(InsertGeoComponent(tips->GetAsString(0),uuid,tipLinelayer->GetTableName(), SIMPLE_LINE))
        {
            return -1;
        }

        int nType = Tools::StringToNum(tips->GetAsString(5));
        EditorGeometry::WkbGeometry* point = NULL;
        
        //startPoint displayStyle of bridge.
        std::string bridgeStartPointDisplayStyle = "";
        
        switch(nType)
        {
		case Model::RoadName:
		case Model::SurveyLine:
			{
				unsigned int length;
				unsigned char* blob = tips->GetAsBlob(14,length);
				std::string deep(reinterpret_cast<char*>(blob),length);
				point = DataTransfor::GetDisplayPointFromDeep(deep);
				break;
			}
        default:
            {
                point = GetDisplayPoint(wkb, DSecondPoint);
            }
        }
        
        if(InsertGeoPoint(tips, point, ICON, bridgeStartPointDisplayStyle) != 0)
        {
        	delete point;

            return -1;
        }

        delete point;

        return 0;
    }

    int APIWrapper::InsertGeoPolygon(DataFeature* tips)
    {
        if(tips == NULL)
        {
            return -1;
        }
        
        FieldDefines* tipsFd = tips->GetDataLayer()->GetFieldDefines();
        
        int index = -1;
        
        index = tipsFd->GetColumnIndex(Tips_Column_SsourceType);
        
        std::string sourceType = tips->GetAsString(index);
        
        DataLayer* polygonLayer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPSPOLYGON);
        
        const DataFeatureFactory* factory = DataFeatureFactory::getDefaultInstance();
        
        DataFeature* featureTipsPolygon = factory->CreateFeature(polygonLayer);
        
        FieldDefines* polygonFd = polygonLayer->GetFieldDefines();
        
        std::string uuid = Tools::GenerateUuid();
        
        if(!uuid.empty())
        {
            index = polygonFd->GetColumnIndex(UuidStr);
            
            featureTipsPolygon->SetAsString(index,uuid);
        }
        
        FEATURE_GEOTYPE geoType = tips->GetGeoType(Tools::StringToNum(sourceType));
        
        index = polygonFd->GetColumnIndex(GeoTypeStr);
        featureTipsPolygon->SetAsInteger(index,(int)geoType);
        
        index = polygonFd->GetColumnIndex(TipsGeoSourceTypeStr);
        featureTipsPolygon->SetAsString(index,sourceType);
        
        index = polygonFd->GetColumnIndex(Display_Style);
        featureTipsPolygon->SetAsString(index,RegionalRoadPolygonStyle);
        
        index = tipsFd->GetColumnIndex(GLocation_Str);
        EditorGeometry::WkbGeometry* wkb = tips->GetAsWkb(index);
        
        index = polygonFd->GetColumnIndex(Geometry_Str);
        featureTipsPolygon->SetAsWkb(index,wkb);
        
        index = polygonFd->GetColumnIndex(VisibilityStr);
        featureTipsPolygon->SetAsInteger(index, TipsGeoVisible);
        
        int result = polygonLayer->InsertFeature(featureTipsPolygon);
        
        if(result != 0)
        {
            factory->DestroyFeature(featureTipsPolygon);
            
            return -1;
        }
        factory->DestroyFeature(featureTipsPolygon);
        
        index = tipsFd->GetColumnIndex(RowkeyStr);
        
        if(InsertGeoComponent(tips->GetAsString(index),uuid,polygonLayer->GetTableName(),POLYGON) != 0)
        {
            return -1;
        }
        
        int nType = Tools::StringToNum(sourceType);
        EditorGeometry::WkbGeometry* point = NULL;
        
        switch(nType)
        {
            case Model::RegionalRoad:
            default:
            {
                point = GetDisplayPoint(wkb, DFirstPoint);
            }
        }
        
        if(InsertGeoPoint(tips, point, ICON) != 0)
        {
            delete point;
            
            return -1;
        }
        
        delete point;
        
        return 0;
    }
    
    int APIWrapper::MaintainSpecialTips(Editor::DataFeature* feature, OPERATION_TYPE operation_type)
    {
        if(NULL == feature)
        {
            return 0;
        }

        FieldDefines* fieldDefines = feature->GetFieldDefines();

        int index = fieldDefines->GetColumnIndex(Tips_Column_SsourceType);

        if(index == -1)
        {
            return 0;
        }

        std::string tips_type = feature->GetAsString(index);

        if(atoi(tips_type.c_str()) == Model::RoadKind)
        {
            index = fieldDefines->GetColumnIndex(Tips_Column_Deep);

            if(index == -1)
            {
                return 0;
            }
            unsigned char* blob;
            unsigned int length;
            
            blob = feature->GetAsBlob(index, length);
            std::string deep_string(reinterpret_cast<char*>(blob),length);

            Document document;
            document.Parse<0>(deep_string.c_str());

            if(!document.HasMember("f") || !document.HasMember("kind"))
            {
                return 0;
            }

            if(!document["kind"].IsInt())
            {
                return 0;
            }

            int tipsKind = document["kind"].GetInt();

            Value& f_value = document["f"];

            if(!f_value.HasMember("id") || !f_value.HasMember("type"))
            {
                return 0;
            }

            int f_type = f_value["type"].GetInt();

            if(f_type != 1)
            {
                return 0;
            }
            
            if (!f_value["id"].IsString())
            {
                return 0;
            }
            

            std::string id = f_value["id"].GetString();
            
            std::string gdbDataPath= m_pDataManager->getFileSystem()->GetEditorGDBDataFile();
            
            if(strcmp(gdbDataPath.c_str(),"") == 0)
            {
                Logger::LogD("APIWrapper::MaintainSpecialTips failed, gdbData Path Empty");
                Logger::LogO("APIWrapper::MaintainSpecialTips failed, gdbData Path Empty");
                return -1;
            }
            
            m_pGdbDataSource->Open(gdbDataPath);

            DataLayer* rdLinelayer = m_pGdbDataSource->GetLayerByType(DATALAYER_RDLINE);
            
            if(rdLinelayer == NULL)
            {
                m_pGdbDataSource->Close(gdbDataPath);
                return 0;
            }

            std::string sql = "select * from "+ rdLinelayer->GetTableName() + " where pid = "+ id;
            
            rdLinelayer->ResetReadingBySQL(sql);
            
            DataFeature* rdLineFeature = rdLinelayer->GetNextFeature();
            
            if(rdLineFeature == NULL)
            {
                m_pGdbDataSource->Close(gdbDataPath);
                Logger::LogD("MaintainSpecialTips failed, Get rdLine by pid failed, sql:[%s]", id.c_str());
                Logger::LogO("MaintainSpecialTips failed, Get rdLine by pid failed, sql:[%s]", id.c_str());
                return 0;
            }

            FieldDefines* rd_line_field_defines = rdLineFeature->GetFieldDefines();

            int lineDisplayStyleIndex = rd_line_field_defines->GetColumnIndex(Display_Style);

            if(lineDisplayStyleIndex == -1)
            {
                m_pGdbDataSource->Close(gdbDataPath);
                return 0;
            }

            int kindIndex = rd_line_field_defines->GetColumnIndex(Kind_Str);

            if(kindIndex == -1)
            {
                m_pGdbDataSource->Close(gdbDataPath);
                return 0;
            }

            int lineKind = rdLineFeature->GetAsInteger(kindIndex);

            std::string lineDisplayStyle = rdLineFeature->GetAsString(lineDisplayStyleIndex);

            std::vector<std::string> displayStrings = Tools::StringSplit(lineDisplayStyle,",");

            std::string newDisplayStyle;
            
            if (displayStrings.size()<2) {
                newDisplayStyle = "";
            }
            else
            {
                if(operation_type == OPERATION_DELETE)
                {
                    newDisplayStyle = boost::lexical_cast <string>(lineKind) + "," + displayStrings[1];
                }
                else
                {
                    newDisplayStyle = boost::lexical_cast <string>(tipsKind) + "," + displayStrings[1];
                }
            }

            rdLineFeature->SetAsString(lineDisplayStyleIndex, newDisplayStyle);
            
            int ret = rdLinelayer->UpdateFeature(rdLineFeature);
            
            m_pGdbDataSource->Close(gdbDataPath);

            return ret;
        }
        return 0;
    }
    
    int APIWrapper::MaintainAttachmentsGeo(DataFeature* feature, OPERATION_TYPE type)
    {
        if(NULL == feature)
        {
            return 0;
        }
        
        FieldDefines* fieldDefines = feature->GetFieldDefines();
        
        int attachmentsIndex = fieldDefines->GetColumnIndex(Tips_Column_SsourceType);
        
        if(attachmentsIndex == -1)
        {
            return 0;
        }
        
        unsigned char* blob;
        unsigned int length;
        
        blob = feature->GetAsBlob(attachmentsIndex, length);
        std::string attachments_string(reinterpret_cast<char*>(blob),length);
        
        Document document;
        document.Parse<0>(attachments_string.c_str());
        
        if(document.HasParseError() || !document.IsArray())
        {
           return 0;
        }
        
        for (int e=0; e<document.Size(); ++e)
        {
            Value& subDocValue = document[e];
            if(!subDocValue.HasMember("type") || !subDocValue.HasMember("content"))
            {
                return 0;
            }
            
            Value& typeValue = subDocValue["type"];
            if (!typeValue.IsInt() || TIPS_Attachments_Draft!=typeValue.GetInt())
            {
                return 0;
            }
            
            Value& contentValue = subDocValue["content"];
            if (contentValue.IsArray())
            {
                std::vector<geos::geom::Geometry*> multiLines;
                std::vector<std::string> multiLineStyles;
                
                
                for (int i = 0; i< contentValue.Size(); ++i)
                {
                    Value& subValue = contentValue[i];
                    if(subValue.HasMember("geo") && !subValue["geo"].IsNull())
                    {
                        Value& geoValue = subValue["geo"];
                        if(geoValue.HasMember("type")
                           && geoValue["type"].IsString()
                           &&!geoValue["type"].IsNull()
                           &&geoValue.HasMember("coordinates"))
                        {
                            rapidjson::Value& coordinates= geoValue["coordinates"];
                            std::vector<EditorGeometry::Point2D> lonlats;
                            std::string strType = geoValue["type"].GetString();
                            if(strcmp(Tools::ToUpper(const_cast<std::string&>(PointStr)).c_str(),Tools::ToUpper(strType).c_str())==0)
                            {
                                std::string style = "";
                                if(geoValue.HasMember("style")
                                   && geoValue["style"].IsString()
                                   && !geoValue["style"].IsNull())
                                {
                                    style = geoValue["style"].GetString();
                                }
                                
                                std::string strWkt = DataTransfor::coordinates2Wkt(coordinates, EditorGeometry::wkbPoint);
                                
                                
                                if(strcmp(strWkt.c_str(),"") != 0)
                                {
                                    EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(strWkt);
                                    if (OPERATION_INSERT==type)
                                    {
                                        InsertTipsGeoOnly(feature, wkb, ICON, style);
                                    }
                                    else if (OPERATION_UPDATE==type)
                                    {
                                        UpdateTipsGeoOnly(feature, wkb, ICON, style);
                                    }
                                    delete[] wkb;
                                }
                            }
                            else if (strcmp(Tools::ToUpper(const_cast<std::string&>(LineStringStr)).c_str(),Tools::ToUpper(strType).c_str())==0)
                            {
                                std::string style = "";
                                if(geoValue.HasMember("style")
                                   && geoValue["style"].IsString()
                                   && !geoValue["style"].IsNull())
                                {
                                    style = geoValue["style"].GetString();
                                }
                                
                                multiLineStyles.push_back(style);
                                std::string strWkt = DataTransfor::coordinates2Wkt(coordinates, EditorGeometry::wkbPolygon);
                                geos::geom::LineString* lineString = dynamic_cast<geos::geom::LineString*>(DataTransfor::Wkt2Geo(strWkt));
                                if(lineString)
                                {
                                    multiLines.push_back(lineString);
                                }
                            }
                            else if (strcmp(Tools::ToUpper(const_cast<std::string&>(PolygonStr)).c_str(),Tools::ToUpper(strType).c_str())==0)
                            {
                                std::string style = "";
                                if(geoValue.HasMember("style")
                                   && geoValue["style"].IsString()
                                   && !geoValue["style"].IsNull())
                                {
                                    style = geoValue["style"].GetString();
                                }
                                
                                std::string strWkt = DataTransfor::coordinates2Wkt(coordinates, EditorGeometry::wkbPolygon);
                                
                                if(strcmp(strWkt.c_str(),"") != 0)
                                {
                                    EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(strWkt);
                                    if (OPERATION_INSERT==type)
                                    {
                                        InsertTipsGeoOnly(feature, wkb, POLYGON, style);
                                    }
                                    else if (OPERATION_UPDATE==type)
                                    {
                                        UpdateTipsGeoOnly(feature, wkb, POLYGON, style);
                                    }
                                    delete[] wkb;
                                }
                                
                            }
                        }
                    }
                    
                }
                
                //编辑多线几何表
                if(multiLines.size()
                   && multiLineStyles.size()
                   && multiLines.size()==multiLineStyles.size())
                {
                    std::string strStyles = "";
                    for (int j=0; j<multiLineStyles.size(); ++j)
                    {
                        if(0==j)
                        {
                            strStyles +=multiLineStyles[j];
                        }
                        else
                        {
                            strStyles += "," + multiLineStyles[j];
                        }
                    }
                    geos::geom::MultiLineString* multiLineGeo = geos::geom::GeometryFactory::getDefaultInstance()->createMultiLineString(multiLines);
                    std::string multiLineStr;
                    if(multiLineGeo != NULL)
                    {
                        multiLineStr = multiLineGeo->toText();
                        if(strcmp(multiLineStr.c_str(),"") != 0)
                        {
                            EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(multiLineStr);
                            if (OPERATION_INSERT==type)
                            {
                                InsertTipsGeoOnly(feature, wkb, MULTIL_LINE, strStyles);
                            }
                            else if (OPERATION_UPDATE==type)
                            {
                                UpdateTipsGeoOnly(feature, wkb, MULTIL_LINE, strStyles);
                            }
                            delete[] wkb;
                        }
                        
                        geos::geom::GeometryFactory::getDefaultInstance()->destroyGeometry(multiLineGeo);
                    }
                    
                    for (int index = 0; index<multiLines.size(); ++index)
                    {
                        geos::geom::Geometry* geo = multiLines[index];
                        if (geo)
                        {
                            geos::geom::GeometryFactory::getDefaultInstance()->destroyGeometry(geo);
                        }
                    }
                    
                }
            }

            
        }
        
        
        return 0;
    }

    EditorGeometry::WkbGeometry* APIWrapper::GetDisplayPoint(EditorGeometry::WkbGeometry* wkb, int index)
    {
        std::string strWkb((char*)wkb, wkb->buffer_size());

		std::istringstream istream(strWkb);

		geos::io::WKBReader reader;

        geos::io::WKBWriter writer;

		geos::geom::Geometry* geo = NULL;
        
        try
        {
		    geo = reader.read(istream);

            geos::geom::CoordinateSequence* collection = geo->getCoordinates();

            int nSize = collection->getSize();

            if(nSize <= index)
            {
                Logger::LogD("collection->getSize() <= index, please check...");
                Logger::LogO("collection->getSize() <= index, please check...");

                return NULL;
            }

            const geos::geom::Coordinate& coord = collection->getAt(index);

            geos::geom::GeometryFactory* gf = new geos::geom::GeometryFactory();

            geos::geom::Point* point = gf->createPoint(coord);

            delete collection;
		    delete geo;
            delete gf;
		    return ToWkbGeometry(point);          
        }
        catch(...)
        {
            Logger::LogD("getDisplayPoint error!!!");
            Logger::LogO("getDisplayPoint error!!!");
            return NULL;
        }
    }
    EditorGeometry::WkbGeometry* APIWrapper::GetTwoDisplayPoint(EditorGeometry::WkbGeometry* wkb, EditorGeometry::WkbGeometry* &endPoint)
    {
        std::string strWkb((char*)wkb, wkb->buffer_size());
		std::istringstream istream(strWkb);
		geos::io::WKBReader reader;
		geos::geom::Geometry* geo = NULL;
        try
        {
		    geo = reader.read(istream);
            geos::geom::CoordinateSequence* collection = geo->getCoordinates();
            int nSize = collection->getSize();
            const geos::geom::Coordinate& coordFirst = collection->getAt(0);
            const geos::geom::Coordinate& coordSecond = collection->getAt(nSize - 1);
            geos::geom::GeometryFactory* gf = new geos::geom::GeometryFactory();
            geos::geom::Point* pointFirst = gf->createPoint(coordFirst);
            geos::geom::Point* pointSecond = gf->createPoint(coordSecond);
            endPoint = ToWkbGeometry(pointSecond);
            delete collection;
		    delete geo;
            delete gf;
		    return ToWkbGeometry(pointFirst);          
        }
        catch(...)
        {
            Logger::LogD("getDisplayPoint error!!!");
            Logger::LogO("getDisplayPoint error!!!");
            return NULL;
        }
    }
    EditorGeometry::WkbGeometry* APIWrapper::ToWkbGeometry(geos::geom::Point* point)
    {
        geos::io::WKBWriter writer;
            std::stringbuf buffer;

            std::ostream out(&buffer);
                
            writer.write(*point, out);

            std::string wkb = buffer.str();

		    int length = wkb.size();

		    char* ret = new char[length];

		    memcpy(ret, wkb.c_str(), length);



            delete point;


		    return (EditorGeometry::WkbGeometry*)ret;          

    }

    int APIWrapper::UpdateGeoLine(DataFeature* tips)
    {
        DataLayer* tipsLinelayer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPSLINE);

        std::string tipsRowkey = tips->GetAsString(0);

        DataLayer* tipsGeoComponentlayer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPSGEOCOMPONENT);

        std::string sql = "select * from "+ tipsGeoComponentlayer->GetTableName() + " where tipsRowkey = '"+ tipsRowkey + "' and geoTableName='" + tipsLinelayer->GetTableName() + "'";

        tipsGeoComponentlayer->ResetReadingBySQL(sql);

        DataFeature* geoComponentFeature = tipsGeoComponentlayer->GetNextFeature();

        if(geoComponentFeature == NULL)
        {
            return -1;
        }

        std::string geoUuid = geoComponentFeature->GetAsString(1);

        const DataFeatureFactory* factory = DataFeatureFactory::getDefaultInstance();

        factory->DestroyFeature(geoComponentFeature);
            
        sql = "select * from "+ tipsLinelayer->GetTableName() + " where uuid = '"+ geoUuid + "'";
            
        tipsLinelayer->ResetReadingBySQL(sql);

        DataFeature* featureTipsLine = tipsLinelayer->GetNextFeature();

        if(featureTipsLine == NULL)
        {
            return -1;
        }

        EditorGeometry::WkbGeometry* wkb = tips->GetAsWkb(8);

        featureTipsLine->SetAsWkb(4,wkb);

        int result = tipsLinelayer->UpdateFeature(featureTipsLine);

        factory->DestroyFeature(featureTipsLine);

        int nType = Tools::StringToNum(tips->GetAsString(5));
        EditorGeometry::WkbGeometry* point = NULL;
        
        std::string bridgeStartPointDisplayStyle = "";
        switch(nType)
        {
		case Model::RoadName:	
		case Model::SurveyLine:
			{
				unsigned int length;
				unsigned char* blob = tips->GetAsBlob(14,length);
				std::string deep(reinterpret_cast<char*>(blob),length);
				point = DataTransfor::GetDisplayPointFromDeep(deep);
				break;
			}
        default:
            {
                point = GetDisplayPoint(wkb, DSecondPoint);
            }
        }

        if(UpdateGeoPoint(tips, point, ICON,bridgeStartPointDisplayStyle) != 0)
        {
        	delete point;

            return -1;
        }

        delete point;

        return result;
    }
    
    int APIWrapper::InsertOrUpdateGeoMixed(DataFeature* tips, OPERATION_TYPE type)
    {
        FieldDefines* pFieldDefines = tips->GetFieldDefines();
        if (pFieldDefines==NULL)
        {
            return -1;
        }
        int sourceTypeIndex = pFieldDefines->GetColumnIndex(Tips_Column_SsourceType);
        std::string sourceType = tips->GetAsString(sourceTypeIndex);
        int nType = Tools::StringToNum(sourceType);
        
        int deepIndex = pFieldDefines->GetColumnIndex(Tips_Column_Deep);
        unsigned int length;
        unsigned char* blob = tips->GetAsBlob(deepIndex, length);
        std::string deep(reinterpret_cast<char*>(blob),length);
        
        switch(nType)
        {
            case Model::MultiDigitized:
            case Model::UnderConstruction:
            case Model::Bridge:
            {
                Document deepDoc;
                deepDoc.Parse<0>(deep.c_str());
                if(deepDoc.HasParseError() || !deepDoc.HasMember("gSLoc") || !deepDoc.HasMember("gELoc"))
                {
                    return -1;
                }
                
                std::string sLocStr = DataTransfor::RapidJsonObj2String(deepDoc["gSLoc"]);
                
                if(sLocStr != "")
                {
                    std::string sLocWkt = DataTransfor::GeoJson2Wkt(sLocStr, Model::Tips::DataSourceFromServer);
                    
                    EditorGeometry::WkbGeometry* sLocGeo = DataTransfor::Wkt2Wkb(sLocWkt);
                    
                    std::string sLocDisplayStyle = DataTransfor::GetTipsSEPointDisplayStyle(nType, deepDoc, true);
                    
                    if(type == OPERATION_INSERT)
                    {
                        //起点
                        InsertTipsGeoOnly(tips, sLocGeo, START_NODE, sLocDisplayStyle);
                        
                        //icon
                        InsertTipsGeoOnly(tips, sLocGeo, ICON, "");
                    }
                    else if(type == OPERATION_UPDATE)
                    {
                        //起点
                        UpdateTipsGeoOnly(tips, sLocGeo, START_NODE, sLocDisplayStyle);
                        
                        //icon
                        UpdateTipsGeoOnly(tips, sLocGeo, ICON, "");
                    }
                    
                    delete [](char*)(sLocGeo);
                    sLocGeo = NULL;
                }
                
                std::string eLocStr = DataTransfor::RapidJsonObj2String(deepDoc["gELoc"]);
                
                if(eLocStr != "")
                {
                    std::string eLocWkt = DataTransfor::GeoJson2Wkt(eLocStr, Model::Tips::DataSourceFromServer);
                    
                    EditorGeometry::WkbGeometry* eLocGeo = DataTransfor::Wkt2Wkb(eLocWkt);
                    
                    std::string eLocDisplayStyle = DataTransfor::GetTipsSEPointDisplayStyle(nType, deepDoc, false);
                    
                    if(type == OPERATION_INSERT)
                    {
                        //终点
                        InsertTipsGeoOnly(tips, eLocGeo, END_NODE, eLocDisplayStyle);
                    }
                    else if(type == OPERATION_UPDATE)
                    {
                        //终点
                        UpdateTipsGeoOnly(tips, eLocGeo, END_NODE, eLocDisplayStyle);
                    }
                    
                    delete [](char*)(eLocGeo);
                    eLocGeo = NULL;
                }
                break;
            }
            default:
                break;
        }
        
        //写入线几何
        if(nType == Model::Bridge || nType == Model::UnderConstruction)
        {
            int geoIndex = pFieldDefines->GetColumnIndex(GLocation_Str);
            
            EditorGeometry::WkbGeometry* gLocation = tips->GetAsWkb(geoIndex);
            
            std::string disStyle = DataTransfor::GetTipsLineDisplayStyle(Tools::NumberToString(nType),"");
            
            if(type == OPERATION_INSERT)
            {
                //线几何
                InsertTipsGeoOnly(tips, gLocation, MULTIL_LINE,disStyle);
            }
            else if(type == OPERATION_UPDATE)
            {
                //线几何
                UpdateTipsGeoOnly(tips, gLocation, MULTIL_LINE,disStyle);
            }

        }
        
        return 0;
    }

    int APIWrapper::UpdateGeoPolygon(DataFeature* tips)
    {
        if(tips == NULL)
        {
            return -1;
        }
        
        FieldDefines* tipsFd = tips->GetDataLayer()->GetFieldDefines();
        
        int index = -1;
        
        index = tipsFd->GetColumnIndex(RowkeyStr);
        
        std::string tipsRowkey = tips->GetAsString(index);
        
        DataLayer* tipsPolygonlayer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPSPOLYGON);
        
        DataLayer* tipsGeoComponentlayer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPSGEOCOMPONENT);
        
        std::string sql = "select * from "+ tipsGeoComponentlayer->GetTableName() + " where tipsRowkey = '"+ tipsRowkey + "' and geoTableName='" + tipsPolygonlayer->GetTableName() + "'";
        
        tipsGeoComponentlayer->ResetReadingBySQL(sql);
        
        DataFeature* geoComponentFeature = tipsGeoComponentlayer->GetNextFeature();
        
        if(geoComponentFeature == NULL)
        {
            return -1;
        }
        
        FieldDefines* polygonFd = tipsPolygonlayer->GetFieldDefines();
        
        std::string geoUuid = geoComponentFeature->GetAsString(1);
        
        const DataFeatureFactory* factory = DataFeatureFactory::getDefaultInstance();
        
        factory->DestroyFeature(geoComponentFeature);
        
        sql = "select * from "+ tipsPolygonlayer->GetTableName() + " where uuid = '"+ geoUuid + "'";
        
        tipsPolygonlayer->ResetReadingBySQL(sql);
        
        DataFeature* featureTipsPolygon = tipsPolygonlayer->GetNextFeature();
        
        if(featureTipsPolygon == NULL)
        {
            return -1;
        }
        
        index = tipsFd->GetColumnIndex(GLocation_Str);
        EditorGeometry::WkbGeometry* wkb = tips->GetAsWkb(index);
        
        index = polygonFd->GetColumnIndex(Geometry_Str);
        featureTipsPolygon->SetAsWkb(index,wkb);
        
        int result = tipsPolygonlayer->UpdateFeature(featureTipsPolygon);
        
        factory->DestroyFeature(featureTipsPolygon);
        
        index = tipsFd->GetColumnIndex(Tips_Column_SsourceType);
        std::string sourceType = tips->GetAsString(index);
        
        int nType = Tools::StringToNum(sourceType);
        EditorGeometry::WkbGeometry* point = NULL;
        
        switch(nType)
        {
            case Model::RegionalRoad:
            default:
            {
                point = GetDisplayPoint(wkb, DFirstPoint);
            }
        }
        
        if(UpdateGeoPoint(tips, point, ICON) != 0)
        {
            delete point;
            
            return -1;
        }
        
        delete point;
        
        return result;
    }
    
    int APIWrapper::InsertGeoComponent(const std::string& rowkey, const std::string& uuid, const std::string& tablename, int geoType)
    {
        DataLayer* geoComponentlayer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPSGEOCOMPONENT);

        const DataFeatureFactory* factory = DataFeatureFactory::getDefaultInstance();
            
        DataFeature* featureTipsComponent = factory->CreateFeature(geoComponentlayer);

        featureTipsComponent->SetAsString(0,rowkey);

        featureTipsComponent->SetAsString(1,uuid);

        featureTipsComponent->SetAsString(2,tablename);
        
        featureTipsComponent->SetAsInteger(3,geoType);

        int result = geoComponentlayer->InsertFeature(featureTipsComponent);

        if(result != 0)
        {
            factory->DestroyFeature(featureTipsComponent);

            return -1;
        }

        factory->DestroyFeature(featureTipsComponent);

        return 0;
    }

    int APIWrapper::UpdateTipsGeo(DataFeature* tips)
    {
        try
        {
            if(tips == NULL)
            {
                return -1;
            }

            std::string dataPath= m_pDataManager->getFileSystem()->GetCoreMapDataPath();

            FEATURE_GEOTYPE geoType = tips->GetGeoType(Tools::StringToNum(tips->GetAsString(5)));

            if(geoType == GEO_POINT)
            {
                if(UpdateGeoPoint(tips, tips->GetAsWkb(8), ICON) != 0)
                {
                    Logger::LogD("UpdateTipsGeo geopoint failed!!!");
                    Logger::LogO("UpdateTipsGeo geopoint failed!!!");

                    return -1;
                }
            }
            else if(geoType == GEO_LINE)
            {
                if(UpdateGeoLine(tips) != 0)
                {
                    Logger::LogD("UpdateTipsGeo geoline failed!!!");
                    Logger::LogO("UpdateTipsGeo geoline failed!!!");

                    return -1;
                }
            }
            else if(geoType == GEO_MIXED)
            {
                if(InsertOrUpdateGeoMixed(tips,OPERATION_UPDATE) != 0)
                {
                    Logger::LogD("UpdateTipsGeo geoMixed failed!!!");
                    Logger::LogO("UpdateTipsGeo geoMixed failed!!!");
                    
                    return -1;
                }
            }
            else if(geoType == GEO_POLYGON)
            {
                if(UpdateGeoPolygon(tips) != 0)
                {
                    Logger::LogD("UpdateTipsGeo geoPolygon failed!!!");
                    Logger::LogO("UpdateTipsGeo geoPolygon failed!!!");

                    return -1;
                }
            }
            
            int ret = MaintainAttachmentsGeo(tips, OPERATION_UPDATE);

            return ret;
        }
        catch(...)
        {
            Logger::LogD("Editor: Update Tips Geo failed In APIWrapper");
            Logger::LogO("Editor: Update Tips Geo failed In APIWrapper");
            return -1;
        }
    }

    std::vector<DataFeature*> APIWrapper::SelectTips(const std::string& box, const std::string& point)
    {
        std::vector<DataFeature*> selectedTips;

	    EditorGeometry::Box2D box2D = DataTransfor::WktToBox2D(box);

	    EditorGeometry::Point2D sourcePoint = DataTransfor::WktToPoint2D(point);

	    if(box2D.is_empty())
	    {
		    Logger::LogD("Input Box is empty.");

		    return selectedTips;
	    }

	    std::string dataPath= m_pDataManager->getFileSystem()->GetCoreMapDataPath();

        if(strcmp(dataPath.c_str(),"") == 0)
        {
    	    Logger::LogD("Editor: SelectTips failed, Data Path Empty");
    	    Logger::LogO("Editor: SelectTips failed, Data Path Empty");
            return selectedTips;
        }

        m_pCoreDataSource->Open(dataPath);

        DataLayer* layer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPS);

	    DataSelector *selector = new DataSelector();

	    selector->AddTargetLayer(layer);

	    selectedTips = selector->PointSelect(box2D, sourcePoint , Editor::DATALAYER_TIPS);

	    m_pCoreDataSource->Close(dataPath);

	    return selectedTips;
    }

    std::string APIWrapper::GetTipsByRowkey(const std::string& rowkey)
    {
        std::string dataPath = m_pDataManager->getFileSystem()->GetCoreMapDataPath();

        if(strcmp(dataPath.c_str(),"") == 0)
        {
    	    Logger::LogD("Editor: GetTipsByRowkey failed, Data Path Empty");
    	    Logger::LogO("Editor: GetTipsByRowkey failed, Data Path Empty");
            return "";
        }

	    m_pCoreDataSource->Open(dataPath);

        Editor::DataLayer* layer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPS);

	    std::string table_name_string = layer->GetTableName();

	    std::string sql ="SELECT * FROM "+ table_name_string + " WHERE rowkey = '"+ rowkey +"'";

	    layer->ResetReadingBySQL(sql);

	    Editor::DataFeature* feature = layer->GetNextFeature();

	    if(feature == NULL)
	    {
		    m_pCoreDataSource->Close(dataPath);

		    return "";
	    }
	    Editor::JSON json = feature->GetTotalPart();

	    std::string result = json.GetJsonString();

	    delete feature;
	    
	    m_pCoreDataSource->Close(dataPath);

	    return result;
    }

    //Grid
    int APIWrapper::UpdateGridStatus(const std::vector<std::string>& grids)
    {
        GridHelper::getInstance()->CreateOrOpenDatabase();

	    int result = GridHelper::getInstance()->UpdateGridStatus(grids);

	    GridHelper::getInstance()->Close();

        return result;
    }

    int APIWrapper::UpdateGridStatusByBox(double dminLon, double dmaxLon, double dminLat, double dmaxLat)
    {
        std::vector<std::string> gridIds;

	    Grid grid;

	    gridIds=grid.CalculateGridsByBox(dminLon,dmaxLon,dminLat,dmaxLat);

	    GridHelper::getInstance()->CreateOrOpenDatabase();
	
        int retn = GridHelper::getInstance()->UpdateGridStatus(gridIds);
	    
        GridHelper::getInstance()->Close();
	    
        return retn;
    }

    //GPS
    int APIWrapper::InsertGpsLine(const std::string& gpsJson)
    {
        try{
		    //add GpsLine
            std::string dataPath= m_pDataManager->getFileSystem()->GetCoreMapDataPath();

            if(strcmp(dataPath.c_str(),"") == 0)
            {
    	        Logger::LogD("Editor: InsertGpsLine failed, Data Path Empty");
    	        Logger::LogO("Editor: InsertGpsLine failed, Data Path Empty");
                return -1;
            }
            m_pCoreDataSource->Open(dataPath);

		    DataLayer* gpslinelayer = m_pCoreDataSource->GetLayerByType(DATALAYER_GPSLINE);
		    
		    const DataFeatureFactory* factory = DataFeatureFactory::getDefaultInstance();
		    DataFeature* featureGpsline = factory->CreateFeature(gpslinelayer);
		    
		    rapidjson::Document doc;
		    doc.Parse<0>(gpsJson.c_str());
		    if(doc.HasParseError() || !doc.HasMember("geometry"))
		    {
			    return -1;
		    }
		    if(doc["geometry"].IsNull())
		    {
			    return -1;
		    }
		    std::string gpsGeometry = doc["geometry"].GetString();

            EditorGeometry::Point2D gpsLocation = DataTransfor::GetGpsLineLocation(gpsGeometry);

            if(gpsLocation._x < 0 || gpsLocation._y < 0)
            {
                return -1;
            }

		    std::string type = "12";
		    
		    std::string rowkey = Tools::GetTipsRowkey(type);
		    Editor::JSON jsonObject;
		    jsonObject.SetJsonString(gpsJson);
		    featureGpsline->SetTotalPart(jsonObject);
		    featureGpsline->SetAsString(0, rowkey);
		    int result = gpslinelayer->InsertFeature(featureGpsline);
		    DataFeatureFactory::getDefaultInstance()->DestroyFeature(featureGpsline);

		    //add Tips
		    {
			    Editor::DataLayer* tipslayer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPS);
			    
			    Editor::DataFeature* featureTips = factory->CreateFeature(tipslayer);
			    std::string currentTime = Tools::GetCurrentDateTime();
			    featureTips->SetAsString(0, rowkey);
			    featureTips->SetAsInteger(3,12);

			    char locationbuffer[100] ={0};
			    sprintf(locationbuffer,"POINT (%lf %lf)",gpsLocation._x,gpsLocation._y);
			    std::string wktGeo = locationbuffer;
			    EditorGeometry::WkbGeometry * wkb= DataTransfor::Wkt2Wkb(wktGeo);
			    featureTips->SetAsWkb(8, wkb);
			    delete[] wkb;

			    featureTips->SetAsString(12, currentTime);
			    int result = tipslayer->InsertFeature(featureTips);
			    Editor::DataFeatureFactory::getDefaultInstance()->DestroyFeature(featureTips);
		    }

            m_pCoreDataSource->Close(dataPath);

		    return result;
	    }
	    catch(...){
		    Logger::LogD("Editor: Insert Gps Line failed In APIWrapper");
		    Logger::LogO("Editor: Insert Gps Line failed In APIWrapper");
		    return -1;
	    }
    }

    //Poi
    int APIWrapper::InsertPoi(const std::string& poi, const std::string& userId, const std::string& projectId)
    {
        try{
            rapidjson::Document doc;
            
            doc.Parse<0>(poi.c_str());
            
            if(doc.HasParseError())
            {
                Logger::LogD("Editor: InsertPoi failed, Input json is invalid. [%s]",poi.c_str());
                Logger::LogO("Editor: InsertPoi failed, Input json is invalid. [%s]",poi.c_str());
                
                return -1;
            }

            std::string dataPath= m_pDataManager->getFileSystem()->GetCoreMapDataPath();
        
            if(strcmp(dataPath.c_str(),"") == 0)
            {
                Logger::LogD("Editor: InsertPoi failed, Data Path Empty");
                Logger::LogO("Editor: InsertPoi failed, Data Path Empty");
                return -1;
            }

            m_pCoreDataSource->Open(dataPath);
        
            Editor::DataLayer* layer = m_pCoreDataSource->GetLayerByType(DATALAYER_POI);

            if(NULL == layer)
            {
                Logger::LogD("Editor: InsertPoi failed, layer is null, maybe datasource is not open");
                Logger::LogO("Editor: InsertPoi failed, layer is null, maybe datasource is not open");
                return -1;
            }
            
            FieldDefines* fd = layer->GetFieldDefines();
            
            if(NULL == fd || fd->GetColumnCount() == 0)
            {
                Logger::LogD("Editor: InsertPoi failed, FieldDefines is null, maybe datasource is not open");
                Logger::LogO("Editor: InsertPoi failed, FieldDefines is null, maybe datasource is not open");
                return -1;
            }
            
            const DataFeatureFactory* factory = DataFeatureFactory::getDefaultInstance();
        
            DataFeature* featurePoi = factory->CreateFeature(layer);
        
            JSON jsonObject;
        
            jsonObject.SetJsonString(poi);
        
            featurePoi->SetTotalPart(jsonObject);
        
            Editor::EditHistory eh(projectId.c_str(), std::atoi(userId.c_str()));
        
            eh.mergeDate = Tools::GetCurrentDateTime();
        
            eh.operation = Editor::LIFECYCLE_ADD;
        
            Editor::EditAttrs ea;
        
            ea.EditHistories.push_back(eh);
        
            std::string newEditAttrs = ea.ToJsonString();

            std::string displayStyle = DataTransfor::GetPoiDisplayStyle(doc);
            
            std::string globalId = DataTransfor::GetPoiGlobalId(doc);
            
            int index = fd->GetColumnIndex(Poi_globalId);
            if(index != -1)
            {
                featurePoi->SetAsString(index,globalId);
            }
            
            index = fd->GetColumnIndex(Poi_lifecycle);
            if(index != -1)
            {
                featurePoi->SetAsInteger(index,Editor::LIFECYCLE_ADD);
            }
            
            index = fd->GetColumnIndex(Poi_edits);
            if(index != -1)
            {
                const char* blob = newEditAttrs.c_str();
                        
                unsigned char* blob_str = (unsigned char*)blob;
                        
                featurePoi->SetAsBlob(index, blob_str, strlen(blob));
            }
            
            index = fd->GetColumnIndex(Poi_latestMergeDate);
            if(index != -1)
            {
                featurePoi->SetAsString(index,eh.mergeDate);
            }
            
            index = fd->GetColumnIndex(Poi_displaystyle);
            if(index != -1)
            {
                featurePoi->SetAsString(index,displayStyle);
            }
            
            index = fd->GetColumnIndex(Poi_project);
            if(index != -1)
            {
                featurePoi->SetAsString(index,projectId);
            }
        
            int result = layer->InsertFeature(featurePoi);
        
            delete featurePoi;
        
            //lee add
            m_pCoreDataSource->Close(dataPath);

            return result;
        }
        catch(...){
            Logger::LogD("Editor: Insert Poi failed In APIWrapper");
            Logger::LogO("Editor: Insert Poi failed In APIWrapper");
            return -1;
        }
    }

    int APIWrapper::UpdatePoi(const std::string& poi, const std::string& userId, const std::string& projectId)
    {
        try{
            rapidjson::Document doc;
            
            doc.Parse<0>(poi.c_str());
            
            if(doc.HasParseError())
            {
                Logger::LogD("Editor: UpdatePoi failed, Input json is invalid. [%s]",poi.c_str());
                Logger::LogO("Editor: UpdatePoi failed, Input json is invalid. [%s]",poi.c_str());
                
                return -1;
            }
            
            std::string dataPath= m_pDataManager->getFileSystem()->GetCoreMapDataPath();
        
            if(strcmp(dataPath.c_str(),"") == 0)
            {
                Logger::LogD("Editor: UpdatePoi failed, Data Path Empty");
                Logger::LogO("Editor: UpdatePoi failed, Data Path Empty");
                return -1;
            }
            m_pCoreDataSource->Open(dataPath);
        
            Editor::DataLayer* layer = m_pCoreDataSource->GetLayerByType(DATALAYER_POI);

            if(NULL == layer)
            {
                Logger::LogD("Editor: UpdatePoi failed, layer is null, maybe datasource is not open");
                Logger::LogO("Editor: UpdatePoi failed, layer is null, maybe datasource is not open");
                return -1;
            }
            
            FieldDefines* fd = layer->GetFieldDefines();
            
            if(NULL == fd || fd->GetColumnCount() == 0)
            {
                Logger::LogD("Editor: UpdatePoi failed, FieldDefines is null, maybe datasource is not open");
                Logger::LogO("Editor: UpdatePoi failed, FieldDefines is null, maybe datasource is not open");
                return -1;
            }
            
            const Editor::DataFeatureFactory* factory = Editor::DataFeatureFactory::getDefaultInstance();
        
            Editor::DataFeature* featurePoi = factory->CreateFeature(layer);
        
            Editor::JSON jsonObject;
        
            jsonObject.SetJsonString(poi);
        
            featurePoi->SetTotalPart(jsonObject);
        
            unsigned int rowId = featurePoi->GetRowId();
            
            Editor::DataFeature* featurePoiOld = layer->GetFeatureByRowId(rowId);
            
            if(NULL == featurePoiOld)
            {
                Logger::LogD("Editor: UpdatePoi poi failed, old value not found,rowid is [%d]", rowId);
                Logger::LogO("Editor: UpdatePoi poi failed, old value not found,rowid is [%d]", rowId);
                
                delete featurePoi;
                
                return -1;
            }
            
            int index = fd->GetColumnIndex(Poi_lifecycle);
            
            int oLifecycle = featurePoiOld->GetAsInteger(index);
        
            if (oLifecycle == Editor::LIFECYCLE_DELETE)
            {
                Logger::LogD("Editor: UpdatePoi failed, lifecycle is [%d]", oLifecycle);
                Logger::LogO("Editor: UpdatePoi failed, lifecycle is [%d]", oLifecycle);
            
                delete featurePoi;
            
                delete featurePoiOld;
            
                return -1;
            }
            else if (oLifecycle == Editor::LIFECYCLE_ADD)
            {
                featurePoi->SetAsInteger(index, Editor::LIFECYCLE_ADD);
            }
            else
            {
                featurePoi->SetAsInteger(index, Editor::LIFECYCLE_UPDATE);
            }
        
            index = fd->GetColumnIndex(Poi_edits);
            
            jsonObject = featurePoi->GetTotalPart();
        
            std::string newValue = jsonObject.GetJsonString();
        
            jsonObject = featurePoiOld->GetTotalPart();
        
            std::string oldValue = jsonObject.GetJsonString();
        
		    Editor::EditAttrs ea;

		    Editor::EditHistory eh(projectId.c_str(), Tools::StringToNum(userId));

		    if(ea.GenerateUpdateEditHistory(eh, oldValue, newValue) == false)
		    {
			    Logger::LogD("Editor: Update failed");
			    Logger::LogO("Editor: Update failed");
			    return -1;
		    }
        
            unsigned char* blob;
            unsigned int length;
                    
            blob = featurePoiOld->GetAsBlob(index, length);
            std::string oldHistory(reinterpret_cast<char*>(blob),length);

            //std::string oldHistory = featurePoiOld->GetAsString(index);

		    std::string newAttrs = ea.AddEditHistory(oldHistory, eh);
        
            const char* newAttrsBlob = newAttrs.c_str();
                        
            unsigned char* blob_str = (unsigned char*)newAttrsBlob;
                        
            featurePoi->SetAsBlob(index, blob_str, strlen(newAttrsBlob));

            std::string displayStyle = DataTransfor::GetPoiDisplayStyle(doc);
            
            index = fd->GetColumnIndex(Poi_latestMergeDate);
            
            featurePoi->SetAsString(index, eh.mergeDate);

            index = fd->GetColumnIndex(Poi_displaystyle);
            
            featurePoi->SetAsString(index, displayStyle);

            index = fd->GetColumnIndex(Poi_project);
            
            featurePoi->SetAsString(index, projectId);

            int result = layer->UpdateFeature(featurePoi);
        
            delete featurePoi;
        
            delete featurePoiOld;
        
            //lee add
            m_pCoreDataSource->Close(dataPath);

            return result;
        }
        catch(...){
            Logger::LogD("Editor: Update Poi failed In APIWrapper");
            Logger::LogO("Editor: Update Poi failed In APIWrapper");
            return -1;
        }
    }

    int APIWrapper::DeletePoi(int rowId, const std::string& userId, const std::string& projectId)
    {
        try{
		    std::string dataPath= m_pDataManager->getFileSystem()->GetCoreMapDataPath();

            if(strcmp(dataPath.c_str(),"") == 0)
            {
        	    Logger::LogD("Editor: DeletePoi failed, Data Path Empty");
        	    Logger::LogO("Editor: DeletePoi failed, Data Path Empty");
                return -1;
            }

	        m_pCoreDataSource->Open(dataPath);

            Editor::DataLayer* layer = m_pCoreDataSource->GetLayerByType(DATALAYER_POI);

            if(NULL == layer)
            {
                Logger::LogD("Editor: DeletePoi failed, layer is null, maybe datasource is not open");
                Logger::LogO("Editor: DeletePoi failed, layer is null, maybe datasource is not open");
                return -1;
            }
            
            FieldDefines* fd = layer->GetFieldDefines();
            
            if(NULL == fd || fd->GetColumnCount() == 0)
            {
                Logger::LogD("Editor: DeletePoi failed, FieldDefines is null, maybe datasource is not open");
                Logger::LogO("Editor: DeletePoi failed, FieldDefines is null, maybe datasource is not open");
                return -1;
            }
            
		    Editor::DataFeature* feature = layer->GetFeatureByRowId(rowId);
            
            int index = fd->GetColumnIndex(Poi_type);
            
		    int nType = feature->GetAsInteger(index);

		    if(nType != 0)
		    {
			    Logger::LogD("delete failed! Type [%d] cannot delete!", nType);
			    Logger::LogO("delete failed! Type [%d] cannot delete!", nType);

			    delete feature;
			    return -1;
		    }

            index = fd->GetColumnIndex(Poi_lifecycle);
            
		    int nLifecycle = feature->GetAsInteger(index);

		    if (nLifecycle == Editor::LIFECYCLE_DELETE)
		    {
			    Logger::LogD("Editor: delete failed, lifecycle is [%d]", nLifecycle);
			    Logger::LogO("Editor: delete failed, lifecycle is [%d]", nLifecycle);

			    delete feature;
			    return -1;
		    }

            feature->SetAsInteger(index, Editor::LIFECYCLE_DELETE);
            
		    Editor::EditContent ec(nLifecycle, Editor::LIFECYCLE_DELETE);

		    Editor::EditHistory eh(projectId.c_str(), Tools::StringToNum(userId));

		    eh.mergeDate = Tools::GetCurrentDateTime();

		    eh.operation = Editor::LIFECYCLE_DELETE;

		    eh.contents.push_back(ec);

		    Editor::EditAttrs ea;

            index = fd->GetColumnIndex(Poi_edits);
            
		    std::string oldHistory = feature->GetAsString(index);

		    std::string newAttrs = ea.AddEditHistory(oldHistory, eh);
            
		    feature->SetAsString(index, newAttrs);

            index = fd->GetColumnIndex(Poi_latestMergeDate);
            
		    feature->SetAsString(index, eh.mergeDate);

            index = fd->GetColumnIndex(Poi_project);
            
		    feature->SetAsString(index, projectId);

		    int result = layer->UpdateFeature(feature);

		    m_pCoreDataSource->Close(dataPath);

		    return result;
        }
	    catch(...){
		    Logger::LogD("Editor: Delete failed");
		    Logger::LogO("Editor: Delete failed");
		    return -1;
	    }
    }
    
    int APIWrapper::UpdatePois(const std::list<std::string> & pois, const std::string& userId, const std::string& projectId, std::list<int> & failIndexs, ProgressEventer* progress)
    {
        if(NULL==progress || 0==pois.size())
            return 0;
        
        char flagBuf[256];
        memset(flagBuf, 0, sizeof(flagBuf));
        char infoBuf[256];
        memset(infoBuf, 0, sizeof(infoBuf));
        
        int index = 0;
        for (std::list<std::string>::const_iterator itor = pois.begin(); itor!=pois.end(); ++itor,++index)
        {
            int flag = -1;
            flag = UpdatePoi(*itor, userId, projectId);
            sprintf(flagBuf, "%d",flag);
            sprintf(infoBuf, "%d",index);
            progress->OnProgress(Tools::GenerateProgressInfo(flagBuf, infoBuf), 100*index/pois.size());
            if (-1==flag)
            {
                failIndexs.push_back(index);
            }
        }
        
        return 0;
    }

    std::vector<std::string> APIWrapper::SnapPoint(const std::string& box, const std::string& point)
    {
        std::vector<std::string> result;

	    EditorGeometry::Box2D box2D = DataTransfor::WktToBox2D(box);

	    if(box2D.is_empty())
	    {
            Logger::LogD("Editor: SnapPoint failed, Input Box is empty");
            Logger::LogO("Editor: SnapPoint failed, Input Box is empty");
		    return result;
	    }

	    EditorGeometry::Point2D sourcePoint = DataTransfor::WktToPoint2D(point);
	    
        if(sourcePoint._x < 0 || sourcePoint._y < 0)
        {
            Logger::LogD("Editor: SnapPoint failed, Input Point is invalid");
            Logger::LogO("Editor: SnapPoint failed, Input Point is invalid");
		    return result;
        }

        std::string dataPath= m_pDataManager->getFileSystem()->GetCoreMapDataPath();

        if(strcmp(dataPath.c_str(),"") == 0)
        {
    	    Logger::LogD("Editor: SnapPoint failed, Data Path Empty");
    	    Logger::LogO("Editor: SnapPoint failed, Data Path Empty");
            return result;
        }

        m_pCoreDataSource->Open(dataPath);

        Editor::DataLayer* poiLayer = m_pCoreDataSource->GetLayerByType(DATALAYER_POI);
        Editor::DataLayer* infoLayer = m_pCoreDataSource->GetLayerByType(DATALAYER_INFOR);
        
	    Editor::DataSelector *selector =new Editor::DataSelector();

	    selector->AddTargetLayer(poiLayer);
        selector->AddTargetLayer(infoLayer);
        
        std::vector<Editor::DataFeature*> points  = selector->PointSelect(box2D,sourcePoint);
        
        bool hasPoi = false;
        
        std::set<int> ignoreRelatedInfoSet;
        
        for(int i=0;i<points.size();i++)
        {
            DATALAYER_TYPE layerType = (DATALAYER_TYPE)points[i]->GetDataLayer()->GetDataLayerType();
            
            if(layerType == DATALAYER_POI)
            {
                hasPoi = true;
            }
            else if(layerType == DATALAYER_INFOR)
            {
                FieldDefines* fd = points[i]->GetDataLayer()->GetFieldDefines();
                
                int proposal =-1;
                std::string rFid="";
                
                int index =fd->GetColumnIndex(Info_proposal);
                
                if(index != -1)
                {
                    proposal = points[i]->GetAsInteger(index);
                }
                
                index = fd->GetColumnIndex(Info_r_fid);
                
                if(index != -1)
                {
                    rFid = points[i]->GetAsString(index);
                }
                
                //情报关联poi
                if(proposal !=3 && strcmp(rFid.c_str(), "") != 0)
                {
                    ignoreRelatedInfoSet.insert(i);
                }
            }

        }
        
        for(int i = 0 ; i<points.size(); i++)
        {
            rapidjson::Document document;
            rapidjson::Document docTmp;
            document.SetObject();
            rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
            
            DATALAYER_TYPE layerType = (DATALAYER_TYPE)points[i]->GetDataLayer()->GetDataLayerType();
            
            //过滤掉关联POI的情报
            if(hasPoi&&layerType == DATALAYER_INFOR && ignoreRelatedInfoSet.find(i) != ignoreRelatedInfoSet.end())
            {
                continue;
            }

            std::string jsonStr;

            int type = -1;
            
            if(layerType == DATALAYER_POI)
            {
                jsonStr = points[i]->GetSnapshotPart().GetJsonString();
                type = 0;
            }
            else if(layerType == DATALAYER_INFOR)
            {
                jsonStr = points[i]->GetTotalPart().GetJsonString();

                type = 1;
            }

            docTmp.Parse<0>(jsonStr.c_str());
            
            if(docTmp.HasParseError())
            {
                continue;
            }
            
            document.AddMember("feature", docTmp, allocator);
            
            document.AddMember("type", type, allocator);
            
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.Accept(writer);
            std::string eacheResult = buffer.GetString();
            
            result.push_back(eacheResult);
            
            DataFeatureFactory::getDefaultInstance()->DestroyFeature(points[i]);
        }
        
        m_pCoreDataSource->Close(dataPath);
        
        delete selector;
        
        return result;
    }

    std::vector<DataFeature*> APIWrapper::SelectPois(const std::string& box)
    {
        std::vector<DataFeature*> pois;

	    EditorGeometry::Box2D box2D = DataTransfor::WktToBox2D(box);

	    if(box2D.is_empty())
	    {
		    Logger::LogD("Input Box is empty.");

		    return pois;
	    }

	    std::string dataPath = m_pDataManager->getFileSystem()->GetCoreMapDataPath();

        if(strcmp(dataPath.c_str(),"") == 0)
        {
    	    Logger::LogD("Editor: SelectPois failed, Data Path Empty");
    	    Logger::LogO("Editor: SelectPois failed, Data Path Empty");
            return pois;
        }

	    m_pCoreDataSource->Open(dataPath);

        Editor::DataLayer* layer = m_pCoreDataSource->GetLayerByType(DATALAYER_POI);

	    layer->ResetReadingBySpatial(box2D);

	    while(Editor::DataFeature* feature = layer->GetNextFeature())
	    {
            feature->SyncDataToMemory();
            
		    pois.push_back(feature);
	    }
        m_pCoreDataSource->Close(dataPath);
        
        return pois;
    }

    std::string APIWrapper::GetTotalPoiByFid(const std::string& fid)
    {
	    std::string dataPath= m_pDataManager->getFileSystem()->GetCoreMapDataPath();

        if(strcmp(dataPath.c_str(),"") == 0)
        {
    	    Logger::LogD("Editor: GetTotalPoiByFid failed, Data Path Empty");
    	    Logger::LogO("Editor: GetTotalPoiByFid failed, Data Path Empty");
            return "";
        }

	    m_pCoreDataSource->Open(dataPath);

        Editor::DataLayer* layer = m_pCoreDataSource->GetLayerByType(DATALAYER_POI);

	    std::string table_name_string = layer->GetTableName();

	    std::string sql ="SELECT * FROM "+ table_name_string + " WHERE FID = '"+ fid +"'";

	    layer->ResetReadingBySQL(sql);

	    Editor::DataFeature* feature = layer->GetNextFeature();

	    if(feature == NULL)
	    {
		    m_pCoreDataSource->Close(dataPath);

		    return "";
	    }
	    Editor::JSON json = feature->GetTotalPart();

	    std::string result = json.GetJsonString();

	    delete feature;
	    
	    m_pCoreDataSource->Close(dataPath);

	    return result;
    }

    std::vector<std::string> APIWrapper::SelectMetaData(const std::string& sTable, std::string condition, int type)
    {
        Editor::MetaDataManager* mdm = Editor::MetaDataManager::GetInstance();

	    std::vector<std::string> vStr = mdm->OperateMetaData(sTable, condition, type);

        return vStr;
    }

    ///Upload & Download
    bool APIWrapper::UploadFile(const std::string& sToken, const std::string& sUser, const std::vector<std::string>& ids, int type, JOBCMD cmd, ProgressEventer* progress, const std::string& deviceId)
    {
	    Editor::ProjectManager* pManager = Editor::ProjectManager::getInstance();

	    pManager->SetCurrentUser(sUser.c_str());

	    pManager->CreateDatabase();

	    Editor::MapDataPackager* pPackager = pManager->getMapDataPackager();

	    pPackager->ClearPackageTypes();

	    pPackager->ClearGridIds();

	    pPackager->ClearProjectIds();

	    for(int i=0 ; i<ids.size() ;i++)
	    {
		    if(Editor::PACKAGE_TIPS == type)
		    {
		    	pPackager->AddGridId(ids[i].c_str());
		    }
		    else
		    {
		    	pPackager->AddProjectId(ids[i].c_str());
		    }
	    }

	    pPackager->AddPackageType(type);

	    pPackager->Run(sToken, (Editor::JOBCMD)cmd, progress, deviceId);

	    return true;
    }

    bool APIWrapper::DownloadFile(const std::string& sToken, const std::string& sUser, const std::vector<std::string>& ids, int type, Editor::Job::Cmd cmd, ProgressEventer* progress)
    {
        Editor::ProjectManager* pManager = Editor::ProjectManager::getInstance();

	    pManager->SetCurrentUser(sUser.c_str());

	    pManager->CreateDatabase();

        Editor::MapDataInstaller* pInstall = pManager->getMapDataInstaller();

	    pInstall->ClearInstallTypes();

	    pInstall->ClearProjectIds();

        for(int i=0 ; i<ids.size() ;i++)
	    {
		    pInstall->AddProjectId(ids[i].c_str());
	    }

        pInstall->AddInstallType(type);

	    pInstall->Run(sToken, (Editor::Job::Cmd)cmd, progress);

        return true;
    }

    bool APIWrapper::InstallLayers(const std::string& sDownloadSqlite, const std::string& sExistsSqlite, const std::string& layers)
    {
        DataSource* pDataSource = m_pDataManager->getDataSource(layers);
        if (NULL==pDataSource)
        {
            return false;
        }
        
        pDataSource->InstallLayers(sDownloadSqlite, sExistsSqlite, layers, NULL);

        return true;
    }

    void APIWrapper::CancelUpload()
    {
        Editor::ProjectManager* pManager = Editor::ProjectManager::getInstance();

	    Editor::MapDataPackager* pPackager = pManager->getMapDataPackager();

	    pPackager->Stop();
    }

    void APIWrapper::CancelDownload()
    {
        Editor::ProjectManager* pManager = Editor::ProjectManager::getInstance();

	    Editor::MapDataInstaller* pInstall = pManager->getMapDataInstaller();

	    pInstall->Stop();
    }

    //Statistics
    int APIWrapper::GetCountByCondition(const std::string& tableName, int type)
    {
        Editor::ClassificationSelector pcfs;

	    return pcfs.GetCountByCondition(tableName, type);
    }

    std::vector<std::string> APIWrapper::GetSignalLayerByCondition(const std::string& sTable, int type, int pagesize, int offset)
    {
        Editor::ClassificationSelector pcfs;

	    std::vector<std::string> vStr = pcfs.GetSignalLayerByCondition(sTable, type, pagesize, offset);

        return vStr;
    }
    
    std::vector<std::string> APIWrapper::QueryTableByCondition(const std::string& strTable, const std::string& strWhere,const std::string& strOderbyFiedOrWktPoint, const std::string& strGeoFieldName, bool isOderbyFied, bool isDesc , int pagesize, int offset)
    {
        std::vector<std::string> vStr;
        
        std::string strSql = "";
        
        if (""==strTable)
        {
            return vStr;
        }
        
        try
        {
            Editor::DataManager* dataManager = Editor::DataManager::getInstance();
            
            std::string dataPath= dataManager->getFileSystem()->GetCoreMapDataPath();
            
            if(strcmp(dataPath.c_str(),"") == 0)
            {
                Logger::LogD("Editor: QueryTableByCondition failed, Data Path Empty");
                Logger::LogO("Editor: QueryTableByCondition failed, Data Path Empty");
                return vStr;
            }
            
            Editor::CoreMapDataSource* dataSource = dynamic_cast<Editor::CoreMapDataSource*>(dataManager->getDataSource(DATASOURCE_COREMAP));
            
            if(NULL==dataSource)
            {
                return vStr;
            }
            
            dataSource->Open(dataPath);
            
            Editor::DataLayer* layer = dataSource->GetLayerByName(strTable);
            
            std::string orderType = "ASC";
            if(isDesc)
                orderType = "DESC";
            
            char buf[256];
            memset(buf, 0, sizeof(buf));
            
            if(isOderbyFied)
            {
                strSql = "SELECT * FROM "+strTable;
                if (strWhere!="")
                {
                    strSql += " WHERE " + strWhere;
                }
                if (strOderbyFiedOrWktPoint!="")
                {
                    strSql += " ORDER BY " + strOderbyFiedOrWktPoint + " " + orderType;
                }
                sprintf(buf," limit %d offset %d;",pagesize, offset);
                strSql +=buf;
            }
            else
            {
                if(strOderbyFiedOrWktPoint!="" && strGeoFieldName!="")
                {
                    sprintf(buf, "SELECT * ,ST_Distance(GeomFromText('%s'),%s.%s) as distance FROM %s ", strOderbyFiedOrWktPoint.c_str(),strTable.c_str(),strGeoFieldName.c_str(),strTable.c_str());
                    strSql +=buf;
                    if (strWhere!="")
                    {
                        strSql += " WHERE " + strWhere;
                    }
                    strSql += " ORDER BY distance " + orderType;
                    memset(buf, 0, sizeof(buf));
                    sprintf(buf," limit %d offset %d;",pagesize, offset);
                    strSql +=buf;
                }
                else
                {
                    sprintf(buf, "SELECT * FROM %s ", strTable.c_str());
                    strSql = buf;
                    if (strWhere!="")
                    {
                        strSql += " WHERE " + strWhere;
                    }
                    memset(buf, 0, sizeof(buf));
                    sprintf(buf," limit %d offset %d;",pagesize, offset);
                    strSql +=buf;
                }
            }
            layer->ResetReadingBySQL(strSql);
            
            while(Editor::DataFeature* feature = layer->GetNextFeature())
            {
                if(feature == NULL)
                {
                    break;
                }
                
                Editor::JSON json = feature->GetTotalPart();
                
                vStr.push_back(json.GetJsonString());
            }
            
            //lee add
            dataSource->Close(dataPath);
            
            return vStr;
        }
        catch(...)
        {
            Logger::LogD("Editor: QueryTableByCondition failed");
            Logger::LogO("Editor: QueryTableByCondition failed");
            return vStr;
        }
        
        
        return vStr;
    }

    //GDB Data
    std::string APIWrapper::GetRdLineByPid(int pid)
    {
	    std::string dataPath= m_pDataManager->getFileSystem()->GetEditorGDBDataFile();

        if(strcmp(dataPath.c_str(),"") == 0)
        {
    	    Logger::LogD("Editor: GetRdLineByPid failed, Data Path Empty");
    	    Logger::LogO("Editor: GetRdLineByPid failed, Data Path Empty");
            return "";
        }

	    m_pGdbDataSource->Open(dataPath);

        Editor::DataLayer* layer = m_pGdbDataSource->GetLayerByType(DATALAYER_RDLINE);

	    std::string table_name_string = layer->GetTableName();

	    std::stringstream ss;
	    ss<<pid;

	    std::string pidStr = ss.str();

	    std::string sql ="SELECT * FROM "+ table_name_string + " WHERE PID = "+ pidStr;

	    layer->ResetReadingBySQL(sql);

	    Editor::DataFeature* feature = layer->GetNextFeature();

	    if(feature == NULL)
	    {
		    m_pGdbDataSource->Close(dataPath);

		    return "";
	    }

	    Editor::JSON json = feature->GetTotalPart();

	    std::string result = json.GetJsonString();

	    delete feature;

	    m_pGdbDataSource->Close(dataPath);

	    return result;
    }

    std::string APIWrapper::SnapLine(const std::string& box, const std::string& point)
    {
        EditorGeometry::Box2D box2D = DataTransfor::WktToBox2D(box);

        if(box2D.is_empty())
        {
    	    Logger::LogD("Input Box is empty.");
    	    return "";
        }

	    EditorGeometry::Point2D sourcePoint = DataTransfor::WktToPoint2D(point);
	    
        if(sourcePoint._x < 0 || sourcePoint._y < 0)
        {
            Logger::LogD("Input Point is invalid.");
    	    return "";
        }
        std::string gdbDataPath= m_pDataManager->getFileSystem()->GetEditorGDBDataFile();

        if(strcmp(gdbDataPath.c_str(),"") == 0)
        {
    	    Logger::LogD("Editor: SnapLine failed, gdbData Path Empty");
    	    Logger::LogO("Editor: SnapLine failed, gdbData Path Empty");
            return NULL;
        }

	    m_pGdbDataSource->Open(gdbDataPath);

        Editor::DataLayer* rdLinelayer = m_pGdbDataSource->GetLayerByType(DATALAYER_RDLINE);

	    Editor::DataSnapper *rdLineSnapper = new Editor::DataSnapper();

	    rdLineSnapper->SetTargetLayer(rdLinelayer);
        
    
        std::string coreDataPath = m_pDataManager->getFileSystem()->GetCoreMapDataPath();
        
        if(strcmp(coreDataPath.c_str(),"") == 0)
        {
            Logger::LogD("Editor: SnapLine failed, coreData Path Empty");
            Logger::LogO("Editor: SnapLine failed, coreData Path Empty");
            return NULL;
        }
        
        m_pCoreDataSource->Open(coreDataPath);

        Editor::DataLayer* gpsLinelayer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPSLINE);

	    Editor::DataSnapper *gpsLinelayerSnapper = new Editor::DataSnapper();

	    gpsLinelayerSnapper->SetTargetLayer(gpsLinelayer);

        SnapEnvironment* sanpEnvironment = new SnapEnvironment();

        sanpEnvironment->AddSnapper(rdLineSnapper);
        sanpEnvironment->AddSnapper(gpsLinelayerSnapper);
        
	    Editor::SnapResult* snapResult = sanpEnvironment->Snapping(box2D,sourcePoint);

	    if(snapResult == NULL)
	    {
		    m_pGdbDataSource->Close(gdbDataPath);
            
            m_pCoreDataSource->Close(coreDataPath);

            delete sanpEnvironment;

		    return "";
	    }

        int snapLayerType = snapResult->GetLayerType();

        if(snapLayerType == DATALAYER_TIPSLINE)
        {
            DataFeature* snapFeature = snapResult->GetSnapFeature();

            std::string uuid = snapFeature->GetAsString(0);

            Editor::DataLayer* geoComponentLayer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPSGEOCOMPONENT);

            std::string sql = "select * from "+ geoComponentLayer->GetTableName() + " where geoUuid = '"+ uuid + "'";

            geoComponentLayer->ResetReadingBySQL(sql);

            DataFeature* geoComponentFeature = geoComponentLayer->GetNextFeature();

            if(geoComponentFeature != NULL)
            {
                std::string tipsRowkey = geoComponentFeature->GetAsString(0);

                Editor::DataLayer* tipsLayer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPS);

                sql = "select * from "+ tipsLayer->GetTableName() +" where rowkey = '" + tipsRowkey +"'";

                tipsLayer->ResetReadingBySQL(sql);

                DataFeature* tipsFeature = tipsLayer->GetNextFeature();

                if(tipsFeature != NULL)
                {
                    snapResult->SetSnapFeature(tipsFeature);
                }
            }
        }

        Editor::JSON json = snapResult->GetJson();

	    std::string result= json.GetJsonString();

	    delete snapResult;

        delete sanpEnvironment;

        m_pGdbDataSource->Close(gdbDataPath);
        
        m_pCoreDataSource->Close(coreDataPath);
	
	    return result;
    }

    std::string APIWrapper::SnapRdNode(const std::string& box, const std::string& point)
    {
        EditorGeometry::Box2D box2D = DataTransfor::WktToBox2D(box);

        if(box2D.is_empty())
        {
    	    Logger::LogD("Input Box is empty.");
    	    return "";
        }

	    EditorGeometry::Point2D sourcePoint = DataTransfor::WktToPoint2D(point);
	    
        if(sourcePoint._x < 0 || sourcePoint._y < 0)
        {
            Logger::LogD("Input Point is invalid.");
    	    return "";
        }
        std::string dataPath= m_pDataManager->getFileSystem()->GetEditorGDBDataFile();

        if(strcmp(dataPath.c_str(),"") == 0)
        {
    	    Logger::LogD("Editor: SnapRdNode failed, Data Path Empty");
    	    Logger::LogO("Editor: SnapRdNode failed, Data Path Empty");
            return "";
        }

	    m_pGdbDataSource->Open(dataPath);

        Editor::DataLayer* rdNodelayer = m_pGdbDataSource->GetLayerByType(DATALAYER_RDNODE);

        if(NULL == rdNodelayer)
        {
            return "";
        }

	    Editor::DataSnapper* snapper = new Editor::DataSnapper();

	    snapper->SetTargetLayer(rdNodelayer);

        SnapEnvironment* sanpEnvironment = new SnapEnvironment();

        sanpEnvironment->AddSnapper(snapper);
        
	    Editor::SnapResult* snapResult = sanpEnvironment->Snapping(box2D,sourcePoint);

	    if(snapResult == NULL)
	    {
		    m_pGdbDataSource->Close(dataPath);

            delete sanpEnvironment;

		    return "";
	    }

        Editor::JSON json = snapResult->GetJson();

	    std::string result= json.GetJsonString();

	    delete snapResult;

        delete sanpEnvironment;

	    m_pGdbDataSource->Close(dataPath);
	
	    return result;
    }

    std::string APIWrapper::GivenPathSnap(const std::string& path, const std::string& point, double disThreshold)
    {
        geos::geom::Geometry* geoPath = NULL;
        
        geos::geom::Geometry* geoPoint = NULL;

        geos::io::WKTReader wktReader;

        try
		{
			geoPath = wktReader.read(path);
            geoPoint = wktReader.read(point);
		}
		catch (...)
		{
			Logger::LogD("GivenPathSnap, wkt is not valid! Path: %s, Point: %s", path.c_str(), point.c_str());

			Logger::LogO("GivenPathSnap, wkt is not valid! Path: %s, Point: %s", path.c_str(), point.c_str());

            return "";
		}
        DataSnapper* snapper = new DataSnapper();

        SnapResult* snapResult = snapper->GivenPathSnapping(geoPath, geoPoint, disThreshold);

        delete geoPath;

        delete geoPoint;

        delete snapper;

        std::string result= "";
        
        if(snapResult != NULL)
        {
            result = snapResult->GetJson().GetJsonString();

            delete snapResult;
        }
        return result;
    }

    std::vector<std::string> APIWrapper::PolygonSelectLines(const std::string& region, bool isIntersectIncluded)
    {
        std::vector<std::string> results;

        std::string coreDataPath = m_pDataManager->getFileSystem()->GetCoreMapDataPath();

        if(strcmp(coreDataPath.c_str(),"") == 0)
        {
    	    Logger::LogD("Editor: PolygonSelectLines failed, coremap Path Empty");
    	    Logger::LogO("Editor: PolygonSelectLines failed, coremap Path Empty");
            return results;
        }

	    m_pCoreDataSource->Open(coreDataPath);
        
        std::string gdbDataPath = m_pDataManager->getFileSystem()->GetEditorGDBDataFile();
        
        if(strcmp(gdbDataPath.c_str(),"") == 0)
        {
            Logger::LogD("Editor: PolygonSelectLines failed, gdbData Path Empty");
            Logger::LogO("Editor: PolygonSelectLines failed, gdbData Path Empty");
            return results;
        }

        m_pGdbDataSource->Open(gdbDataPath);
        
        Editor::DataLayer* rdLinelayer = m_pGdbDataSource->GetLayerByType(DATALAYER_RDLINE);
        
        Editor::DataLayer* gpsLinelayer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPSLINE);

	    Editor::DataSelector* selector = new Editor::DataSelector();

        selector->AddTargetLayer(rdLinelayer);
        selector->AddTargetLayer(gpsLinelayer);

	    std::vector<DataFeature*> features = selector->PolygonSelect(region,isIntersectIncluded);

	    for(int i = 0 ; i<features.size(); i++)
	    {
            rapidjson::Document document;
            rapidjson::Document docTmp;
            document.SetObject();
            rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

	        std::string jsonStr = features[i]->GetTotalPart().GetJsonString();

            docTmp.Parse<0>(jsonStr.c_str());

            if(docTmp.HasParseError())
            {
                continue;
            }

            document.AddMember("feature", docTmp, allocator);

            int type = -1;

            DATALAYER_TYPE layerType = (DATALAYER_TYPE)features[i]->GetDataLayer()->GetDataLayerType();

            if(layerType == DATALAYER_RDLINE)
            {
                type = 0;
            }
            else if(layerType == DATALAYER_TIPS)
            {
                type = 1;
            }
            document.AddMember("type", type, allocator);

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);  
            document.Accept(writer);  
            std::string eacheResult = buffer.GetString();

            results.push_back(eacheResult);

            DataFeatureFactory::getDefaultInstance()->DestroyFeature(features[i]);
	    }

        delete selector;

	    m_pCoreDataSource->Close(coreDataPath);
        m_pGdbDataSource->Close(gdbDataPath);
	
	    return results;
    }

    //Version
    bool APIWrapper::InitVersionInfo(int type)
    {
        Editor::DataServiceAddress * dsa = m_pDataManager->getServiceAddress();

	    dsa->Init(type);

        return true;
    }

    std::vector<std::string> APIWrapper::GetProjectInfo(const std::string& sToken)
	{
    	std::vector<std::string> vResult;

    	Editor::DataManager* pDataManager = Editor::DataManager::getInstance();

    	Editor::DataFileSystem* pFileSystem = pDataManager->getFileSystem();

    	Editor::ProjectManager* pManager = Editor::ProjectManager::getInstance();

    	std::string userId = pFileSystem->GetCurrentUserId();

    	if(userId.empty())
    	{
    		return vResult;
    	}

		return pManager->GetProjectInfo(sToken, userId);
	}

	std::vector<std::string> APIWrapper::GetProjectDetails(const std::string& sToken, const std::string& sProjectId)
	{
		Editor::DataManager* pDataManager = Editor::DataManager::getInstance();

		Editor::ProjectManager* pManager = Editor::ProjectManager::getInstance();

		return pManager->GetProjectDetails(sToken, sProjectId);
	}
    
    std::vector<std::string> APIWrapper::GetTaskDetails(const std::string& sToken, const std::string& sProjectId)
	{
		std::vector<std::string> vResult;

		Editor::DataManager* pDataManager = Editor::DataManager::getInstance();

		Editor::DataFileSystem* pFileSystem = pDataManager->getFileSystem();

		Editor::ProjectManager* pManager = Editor::ProjectManager::getInstance();

		std::string userId = pFileSystem->GetCurrentUserId();

		if(userId.empty())
		{
			return vResult;
		}

		return pManager->GetTaskDetails(sToken, sProjectId, userId);
	}

	int APIWrapper::InsertGpsLineTip( const std::string& tipJson )
	{
		Editor::DataManager* dataManager = Editor::DataManager::getInstance();
		Editor::CoreMapDataSource* dataSource = dynamic_cast<Editor::CoreMapDataSource*>(dataManager->getDataSource(DATASOURCE_COREMAP));
        if (NULL==dataSource)
        {
            return -1;
        }
		return dataSource->InsertGpsLineTip(tipJson);
	}

	std::string APIWrapper::GetTipsRowkey(const std::string& type)
	{
		std::string rowkey = Tools::GetTipsRowkey(type);
		return rowkey;
	}
	
	std::string APIWrapper::GetCoremapModelVersion(int type)
    {
        Editor::DataManager* pDataManager = Editor::DataManager::getInstance();

        Editor::CoreMapDataSource* ds = dynamic_cast<Editor::CoreMapDataSource*>(pDataManager->getDataSource(DATASOURCE_COREMAP));
        
        if (NULL==ds)
        {
            return "";
        }
		
        return ds->GetCoremapModelVersion(type);
    }

	bool APIWrapper::InstallGdb(std::string sPath, std::string sFlag, ProgressEventer* progress)
	{
        Editor::DataManager* pDataManager = Editor::DataManager::getInstance();
        
        Editor::GdbDataSource* m_pDataSource = dynamic_cast<Editor::GdbDataSource*>(pDataManager->getDataSource(DATASOURCE_GDB));
        
        if (NULL==m_pDataSource)
        {
            return false;
        }

		int ret = m_pDataSource->InstallGDB(sPath, sFlag, progress);

		if(ret != 0)
		{
			return false;
		}

		return true;
	}
    
    bool APIWrapper::RepalceGDB()
    {
        std::string srcFileDir = m_pDataManager->getFileSystem()->GetTempGDBDataDir();
        std::string destFileDir= m_pDataManager->getFileSystem()->GetEditorGDBDataDir();
        //清空渲染目录下所有gdb数据库相关文件
        Tools::RemoveSimilarFilesInDir(destFileDir.c_str(), "gdb");
        //将editor sdk 的gdb相关文件拷贝到渲染目录下
        Tools::CopySimilarFiles2Dir(srcFileDir.c_str(), "gdb", destFileDir.c_str());
        return true;
    }
	
	std::string	APIWrapper::LinkMatch(std::string locationJson)
	{
        
        Editor::LocationManager* pLm = Editor::LocationManager::GetInstance();
        Logger::LogO("enter ParseInput...[%s]", locationJson.c_str());
        pLm->ParseInput(locationJson);
        Logger::LogO("enter Filte...");
        if(pLm->Filte() ==0)
        {
			Logger::LogO("enter Match...");
            pLm->Match();
        }
        Logger::LogO("return...");
        return pLm->ToJson();
	}
	
	int APIWrapper::DeleteSurveyLine( const std::string& rowkey, const std::vector<std::string>& reTipsRowkeys )
	{
		DeleteTips(rowkey);
		for (std::vector<std::string>::const_iterator itor = reTipsRowkeys.begin(); itor!=reTipsRowkeys.end(); itor++)
		{
			DeleteTips(*itor);
		}
		return 0;
	}

	int APIWrapper::QueryReTipsOnSurveyLine( const std::string& surveyLineRowkey, std::vector<std::string>& reTipsRowkeys )
	{
		std::string dataPath = m_pDataManager->getFileSystem()->GetCoreMapDataPath();
		if(strcmp(dataPath.c_str(),"") == 0)
		{
			Logger::LogD("Editor: DeleteSurveyLine failed, Data Path Empty");
			Logger::LogO("Editor: DeleteSurveyLine failed, Data Path Empty");
			return -1;
		}
		m_pCoreDataSource->Open(dataPath);
		Editor::DataLayer* layer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPS);
		std::string table_name_string = layer->GetTableName();
		std::string sql ="SELECT * FROM "+ table_name_string + " WHERE rowkey = '"+ surveyLineRowkey +"'";
		layer->ResetReadingBySQL(sql);
		Editor::DataFeature* feature = layer->GetNextFeature();
		if(feature == NULL)
		{
			m_pCoreDataSource->Close(dataPath);
			Logger::LogD("Editor: DeleteSurveyLine failed, no find surveyLine feature");
			Logger::LogO("Editor: DeleteSurveyLine failed, no find surveyLine feature");
			return 0;
		}

		FieldDefines* pFieldDefines = layer->GetFieldDefines();
		if (!pFieldDefines)
		{
			m_pCoreDataSource->Close(dataPath);
			Logger::LogD("Editor: layer->GetFieldDefines failed");
			Logger::LogO("Editor: layer->GetFieldDefines failed");
			return -1;
		}

		//≤È≥ˆ≤‚œﬂµƒg_location
		unsigned int index = pFieldDefines->GetColumnIndex("g_location");
		EditorGeometry::WkbGeometry* glocationWkb = feature->GetAsWkb(index);
		if (!glocationWkb)
		{
			m_pCoreDataSource->Close(dataPath);
			Logger::LogD("Editor: feature->GetAsWkb failed");
			Logger::LogO("Editor: feature->GetAsWkb failed");
			return -1;
		}
		geos::geom::Geometry*  glocation = DataTransfor::Wkb2Geo(glocationWkb);
		if (!glocation)
		{
			m_pCoreDataSource->Close(dataPath);
			Logger::LogD("Editor: DataTransfor Wkb2Geo failed");
			Logger::LogO("Editor: DataTransfor Wkb2Geo failed");
			delete glocation;
			return -1;
		}

		delete feature;
		feature = NULL;

		//bufferæ‡¿Îµ√µ•Œª «æ≠Œ≥∂»,ƒ¨»œ»°200√◊
		double earthRadius = 6371001.00;
		double bufferDistance = 200.0*180/(M_PI*earthRadius);
		geos::geom::Geometry* glocationBuffer = glocation->buffer(bufferDistance);
		if (!glocationBuffer)
		{
			m_pCoreDataSource->Close(dataPath);
			Logger::LogD("Editor: glocation->buffer failed");
			Logger::LogO("Editor: glocation->buffer failed");
			return -1;
		}
		std::string strBufferWkt = glocationBuffer->toText();
		delete glocationBuffer;
		delete glocation;

		//≤È≥ˆg_guide¬‰‘⁄≤‚œﬂ200√◊bufferƒ⁄µƒtips
		std::string sqlWithinBuffer = "SELECT * FROM " + table_name_string;
		sqlWithinBuffer += " WHERE ST_Within(GeomFromText(ST_AsText(g_guide)), GeomFromText('"+ strBufferWkt +"')) = 1 AND ROWID IN (SELECT ROWID FROM SpatialIndex s WHERE s.f_table_name='"+
			table_name_string +"' AND s.search_frame= GeomFromText('"+ strBufferWkt +"'))";

		layer->ResetReadingBySQL(sqlWithinBuffer);
		Editor::DataFeature* reFeature = layer->GetNextFeature();
		if(reFeature == NULL)
		{
			m_pCoreDataSource->Close(dataPath);
			Logger::LogD("Editor: DeleteSurveyLine , no find related feature");
			Logger::LogO("Editor: DeleteSurveyLine , no find related feature");
		}

		while (reFeature)
		{
			unsigned int sourceTypeIndex = pFieldDefines->GetColumnIndex("s_sourceType");
			std::string souceType = reFeature->GetAsString(sourceTypeIndex);
			int iType = atoi(souceType.c_str());
			
			unsigned int deepFieldIndex = pFieldDefines->GetColumnIndex("deep");
			unsigned int length;
			unsigned char* blob = reFeature->GetAsBlob(deepFieldIndex,length);
			std::string deep(reinterpret_cast<char*>(blob),length);

			std::string strReId="";
			bool isReTips = false;
			int retFlag = Tools::CheckRelationTips(iType,deep,surveyLineRowkey,strReId,isReTips);
			if (retFlag==0 && isReTips && strReId!="")
			{
				reTipsRowkeys.push_back(strReId);
			}
			reFeature = layer->GetNextFeature();
		}

		m_pCoreDataSource->Close(dataPath);
		return 0;
	}

	int APIWrapper::DeleteTipsGeo( DataFeature* tips )
	{
		std::string tipsRowkey = tips->GetAsString(0);
		DataLayer* tipsGeoComponentlayer = m_pCoreDataSource->GetLayerByType(DATALAYER_TIPSGEOCOMPONENT);
		if(!tipsGeoComponentlayer)
		{
			return -1;
		}

		const DataFeatureFactory* factory = DataFeatureFactory::getDefaultInstance();
		FieldDefines* pFieldDefines = tipsGeoComponentlayer->GetFieldDefines();
		if (!pFieldDefines)
		{
			Logger::LogD("Editor: tipsGeoComponentlayer->GetFieldDefines failed");
			Logger::LogO("Editor: tipsGeoComponentlayer->GetFieldDefines failed");
			return -1;
		}

		unsigned int geoUuidIndex = pFieldDefines->GetColumnIndex("geoUuid");
		unsigned int geoTableNameIndex = pFieldDefines->GetColumnIndex("geoTableName");

		m_pCoreDataSource->BeginTranscation();
		int ret = 0;

		std::string sql = "select * from "+ tipsGeoComponentlayer->GetTableName() + " where tipsRowkey = '"+ tipsRowkey + "'";

		tipsGeoComponentlayer->ResetReadingBySQL(sql);

		DataFeature* geoComponentFeature = tipsGeoComponentlayer->GetNextFeature();

		while (geoComponentFeature)
		{
			std::string geoUuid = geoComponentFeature->GetAsString(geoUuidIndex);
			std::string geoTableName = geoComponentFeature->GetAsString(geoTableNameIndex);

			DATALAYER_TYPE tipsLayerType = DATALAYER_TIPSPOINT;
			if(strcmp("TIPS_POINT",Tools::ToUpper(geoTableName).c_str())==0)
			{
				tipsLayerType = DATALAYER_TIPSPOINT;
			}
			else if(strcmp("TIPS_LINE",Tools::ToUpper(geoTableName).c_str())==0)
			{
				tipsLayerType = DATALAYER_TIPSLINE;
			}
			else if(strcmp("TIPS_POLYGON",Tools::ToUpper(geoTableName).c_str())==0)
			{
				tipsLayerType = DATALAYER_TIPSPOLYGON;
			}
            else if(strcmp("TIPS_MULTILINE",Tools::ToUpper(geoTableName).c_str())==0)
            {
                tipsLayerType = DATALAYER_TIPSMULTILINE;
            }


			DataLayer* tipsgeolayer = m_pCoreDataSource->GetLayerByType(tipsLayerType);

			sql = "select * from "+ tipsgeolayer->GetTableName() + " where uuid = '"+ geoUuid + "'";

			tipsgeolayer->ResetReadingBySQL(sql);

			DataFeature* featureTips = tipsgeolayer->GetNextFeature();

			while (featureTips)
			{
				ret = tipsgeolayer->DeleteFeature(featureTips);
				if(ret!=0)
				{
					m_pCoreDataSource->RollbackTranscation();
				}
				factory->DestroyFeature(featureTips);
				featureTips=tipsgeolayer->GetNextFeature();
			}

			ret = tipsGeoComponentlayer->DeleteFeature(geoComponentFeature);
			if(ret!=0)
			{
				m_pCoreDataSource->RollbackTranscation();
			}
			factory->DestroyFeature(geoComponentFeature);
			geoComponentFeature=tipsGeoComponentlayer->GetNextFeature();

		}

		m_pCoreDataSource->EndTranscation();
		return 0;
	}

    double APIWrapper::CaclAngle(const std::string& point, const std::string& linkId, int type)
    {
        int dir = 1;
        EditorGeometry::WkbGeometry* wkb = NULL;
        DataFunctor* df = Editor::DataManager::getInstance()->getDataFunctor();
        if(type == 1)
        {
            wkb = df->GetGeoFromId(linkId, dir);
        }
        else
        {
            wkb = df->GetGeoByRowkey(linkId);
        }
        if(wkb == NULL)
        {
            return 0.0;
        }

        double angle = 0.0;

        EditorGeometry::Point2D point2D = DataTransfor::WktToPoint2D(point);

		angle = Editor::GeometryCalculator::getInstance()->CalcAngle(wkb, point2D, dir);

		return angle;
    }
    
    int APIWrapper::UpdateInfo(const std::string& info)
    {
        try{
            std::string dataPath= m_pDataManager->getFileSystem()->GetCoreMapDataPath();
            
            if(strcmp(dataPath.c_str(),"") == 0)
            {
                Logger::LogD("Editor: UpdatePointInfo failed, Data Path Empty");
                Logger::LogO("Editor: UpdatePointInfo failed, Data Path Empty");
                return -1;
            }
            m_pCoreDataSource->Open(dataPath);
            
            Editor::DataLayer* layer = m_pCoreDataSource->GetLayerByType(DATALAYER_INFOR);
            
            if(NULL == layer)
            {
                Logger::LogD("Editor: UpdatePointInfo failed, layer is null, maybe datasource is not open");
                Logger::LogO("Editor: UpdatePointInfo failed, layer is null, maybe datasource is not open");
                return -1;
            }
            
            const Editor::DataFeatureFactory* factory = Editor::DataFeatureFactory::getDefaultInstance();
            
            Editor::DataFeature* featureInfo = factory->CreateFeature(layer);
            
            Editor::JSON jsonObject;
            
            jsonObject.SetJsonString(info);
            
            featureInfo->SetTotalPart(jsonObject);
            
            unsigned int rowId = featureInfo->GetRowId();
            
            Editor::DataFeature* featureInfoOld = layer->GetFeatureByRowId(rowId);
            
            if(NULL == featureInfoOld)
            {
                Logger::LogD("Editor: UpdatePointInfo failed, old value not found,rowid is [%d]", rowId);
                Logger::LogO("Editor: UpdatePointInfo failed, old value not found,rowid is [%d]", rowId);
                
                delete featureInfoOld;
                
                return -1;
            }
            
            int result = layer->UpdateFeature(featureInfo);
            
            delete featureInfo;
            
            delete featureInfoOld;
            
            m_pCoreDataSource->Close(dataPath);
            
            return result;
        }
        catch(...){
            Logger::LogD("Editor: UpdatePointInfo failed In APIWrapper");
            Logger::LogO("Editor: UpdatePointInfo failed In APIWrapper");
            return -1;
        }
    }
    std::string APIWrapper::GetDownloadOrUploadStatus(const std::string& projectId, const std::string& userId, int type)
    {
        Editor::ProjectManager* pm = Editor::ProjectManager::getInstance();

        pm->SetCurrentUser(userId.c_str());

        pm->CreateDatabase();

        Editor::MapDataInstaller* pInstall = pm->getMapDataInstaller();

        Model::ProjectUser* PU = new Model::ProjectUser();

        int ret = pInstall->GetPUFromDatabase(projectId, type, PU);

        std::string sResult;

        if(ret == 0)
        {
            sResult = GenerateStatusJsonResult(PU);
        }
        else
        {
            sResult = GenerateStatusJsonResult(NULL);
        }
      
        delete PU;
        
        return sResult;
    }

    std::string APIWrapper::GenerateStatusJsonResult(Model::ProjectUser* PU)
    {
        Document document;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();

        Value each_json_value(kStringType);  

        if(PU == NULL)
        {
            document.AddMember("isHaveInfo", 0, allocator);
        }
        else
        {
            document.AddMember("isHaveInfo", 1, allocator);

            std::string each_str_value = PU->sProjectId;
            each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
            document.AddMember("projectId", each_json_value, allocator);

            each_str_value = PU->download_latest;
            each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
            document.AddMember("lastDownLoadTime", each_json_value, allocator);

            each_str_value = PU->upload_latest;
            each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
            document.AddMember("lastUpLoadTime", each_json_value, allocator);

            if(!PU->download_url.empty())
            {
                document.AddMember("isDownLoadContinue", 1, allocator);
            }
            else
            {
                document.AddMember("isDownLoadContinue", 0, allocator);
            }

            if(!PU->upload_url.empty())
            {
                document.AddMember("isUpLoadContinue", 1, allocator);
            }
            else
            {
                document.AddMember("isUpLoadContinue", 0, allocator);
            }
        }

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();

        return sResult;
    }

    std::string	APIWrapper::GetEditHistroyByFid(const std::string& fid)
    {

    	Editor::DataManager* dataManager = Editor::DataManager::getInstance();

		std::string dataPath= dataManager->getFileSystem()->GetCoreMapDataPath();

		if(strcmp(dataPath.c_str(),"") == 0)
		{
			Logger::LogD("Editor: GetEditHistroyByFid failed, Data Path Empty");
			Logger::LogO("Editor: GetEditHistroyByFid failed, Data Path Empty");
			return "";
		}

		Editor::CoreMapDataSource* dataSource = dynamic_cast<Editor::CoreMapDataSource*>(dataManager->getDataSource(DATASOURCE_COREMAP));

		if(NULL==dataSource)
		{
			return "";
		}

		dataSource->Open(dataPath);

		Editor::DataLayer* layer = dataSource->GetLayerByType(DATALAYER_POI);

		std::string sql = "select * from edit_pois where fid ='" + fid + "'";

		FieldDefines* pFieldDefines = layer->GetFieldDefines();

		if (!pFieldDefines)
		{
			dataSource->Close(dataPath);

			Logger::LogD("Editor: layer->GetFieldDefines failed");
			Logger::LogO("Editor: layer->GetFieldDefines failed");
			return "";
		}

		unsigned int editIndex = pFieldDefines->GetColumnIndex("edits");

		layer->ResetReadingBySQL(sql);

		std::string sResult;

		if(Editor::DataFeature* feature = layer->GetNextFeature())
		{
			sResult = feature->GetAsString(editIndex);

			Logger::LogO("the edithistory of poi(fid:%s) is [%s]", fid.c_str(), sResult.c_str());

			DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);
		}

		dataSource->Close(dataPath);

		return sResult;
    }
    
    std::vector<std::string>     APIWrapper::QueryFeatures(const std::string& tableName, const std::string& searchQuery)
    {
        std::vector<std::string> result;
        
        if(std::strcmp(tableName.c_str(), "") ==0 || std::strcmp(searchQuery.c_str(), "") ==0)
        {
            return result;
        }
        
        DataFunctor* dataFunctor = Editor::DataManager::getInstance()->getDataFunctor();
        
        if(NULL == dataFunctor)
        {
            return result;
        }
        
        std::vector<Editor::DataFeature*> features = dataFunctor->QueryFeatures(tableName, searchQuery);
        
        for(int i = 0 ; i<features.size(); i++)
        {
            rapidjson::Document document;
            document.SetObject();
            rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
            
            Document docTmp(&allocator);
            
            std::string jsonStr = features[i]->GetSnapshotPart().GetJsonString();
            
            docTmp.Parse<0>(jsonStr.c_str());
            
            if(docTmp.HasParseError())
            {
                continue;
            }
            
            std::string tableName = features[i]->GetDataLayer()->GetTableName();
            Value str_value(kStringType);
            
            str_value.SetString(tableName.c_str(), tableName.size(),allocator);
            document.AddMember("layer",str_value,allocator);
            
            document.AddMember("feature", docTmp, allocator);
            
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.Accept(writer);
            std::string eacheResult = buffer.GetString();
            
            result.push_back(eacheResult);
            
            DataFeatureFactory::getDefaultInstance()->DestroyFeature(features[i]);
        }
        
        return result;
    }
    
    std::vector<std::string>     APIWrapper::QueryFeaturesBySql(const std::string& tableName, const std::string& sql)
    {
        std::vector<std::string> result;
        
        if(std::strcmp(tableName.c_str(), "") ==0 || std::strcmp(sql.c_str(), "") ==0)
        {
            return result;
        }
        
        DataFunctor* dataFunctor = Editor::DataManager::getInstance()->getDataFunctor();
        
        if(NULL == dataFunctor)
        {
            return result;
        }
        
        std::vector<Editor::DataFeature*> features = dataFunctor->QueryFeaturesBySql(tableName, sql);
        
        for(int i = 0 ; i<features.size(); i++)
        {
            rapidjson::Document document;
            document.SetObject();
            rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
            
            Document docTmp(&allocator);
            
            std::string jsonStr = features[i]->GetSnapshotPart().GetJsonString();
            
            docTmp.Parse<0>(jsonStr.c_str());
            
            if(docTmp.HasParseError())
            {
                continue;
            }
            
            std::string tableName = features[i]->GetDataLayer()->GetTableName();
            Value str_value(kStringType);
            
            str_value.SetString(tableName.c_str(), tableName.size(),allocator);
            document.AddMember("layer",str_value,allocator);
            
            document.AddMember("feature", docTmp, allocator);
            
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.Accept(writer);
            std::string eacheResult = buffer.GetString();
            
            result.push_back(eacheResult);
            
            DataFeatureFactory::getDefaultInstance()->DestroyFeature(features[i]);
        }
        
        return result;
    }
    
	std::string APIWrapper::RobTask(const std::string& token, const std::string& projectId, const std::string& taskId)
    {
        std::string url = Editor::DataManager::getInstance()->getServiceAddress()->GetRobTaskUrl(token, projectId, taskId);

        std::string response;

        if(Tools::HttpGet(url.c_str(), response) == false)
        {
            Logger::LogD("rob task successfully because of [%s]", response.c_str());
            Logger::LogO("rob task successfully because of [%s]", response.c_str());
        }

        return response;
    }
    
    int APIWrapper::InsertTrackPoint(const std::string& id, double latitude,
                                                  double longitude, double direction, double speed,
                                                  const std::string& recordTime,
                                                  int userId, const std::string& segmentId)
    {
        Editor_Track track;
        return track.writeTrackPoint(id, latitude, longitude, direction, speed, recordTime, userId, segmentId);
    }
    
    int APIWrapper::PoiStatistics(const std::string& tableName, std::vector<std::string>& results)
    {
        std::vector<std::string> result;
        
        if(std::strcmp(tableName.c_str(), "") ==0)
        {
            return 0;
        }
        
        DataFunctor* dataFunctor = Editor::DataManager::getInstance()->getDataFunctor();
        
        if(NULL == dataFunctor)
        {
            return 0;
        }
        
        char buf[256];
        
        sprintf(buf,"SELECT count(*), %s, substr(%s,0,8) as mydate FROM %s WHERE %s!=0 and length(%s)>0 group by %s,mydate order by mydate asc", Poi_lifecycle.c_str(), Poi_t_operateDate.c_str(), tableName.c_str(), Poi_lifecycle.c_str(), Poi_t_operateDate.c_str(),Poi_lifecycle.c_str());
        std::string sql = buf;
        
        std::vector<Editor::DataFeature*> features = dataFunctor->QueryFeaturesBySql(tableName, sql);
        
        int countIndex = 0;
        int lifecycleIndex = 1;
        int toperateDateIndex = 2;
        std::map<std::string, Editor_PoiStatisticsResult> date2StatisticsResult;
        for(int i = 0 ; i<features.size(); i++)
        {
            Editor::DataFeature* feature = features.at(i);
            if(feature==NULL) continue;
            std::string toperateDate = feature->GetAsString(toperateDateIndex);
            std::map<std::string, Editor_PoiStatisticsResult>::iterator itor= date2StatisticsResult.find(toperateDate);
            int count = feature->GetAsInteger(countIndex);
            int lifecycle = feature->GetAsInteger(lifecycleIndex);
            if(itor!=date2StatisticsResult.end())
            {
                switch (lifecycle)
                {
                    case Editor_PoiLifecycle_Del://del
                        itor->second.delCount += count;
                        break;
                    case Editor_PoiLifecycle_Modify:
                        itor->second.modifyCount += count;
                        break;
                    case Editor_PoiLifecycle_Add:
                        itor->second.addCount += count;
                        break;
                    default:
                        break;
                }
                
                itor->second.sumCount +=count;
            }
            else
            {
                Editor_PoiStatisticsResult record;
                record.sumCount = count;
                switch (lifecycle)
                {
                    case Editor_PoiLifecycle_Del://del
                        record.delCount += count;
                        break;
                    case Editor_PoiLifecycle_Modify:
                        record.modifyCount += count;
                        break;
                    case Editor_PoiLifecycle_Add:
                        record.addCount += count;
                        break;
                        
                    default:
                        break;
                }
                record.toperateDate = toperateDate;
                date2StatisticsResult.insert(std::make_pair(toperateDate, record));
            }
            DataFeatureFactory::getDefaultInstance()->DestroyFeature(features[i]);
        }
        
        std::map<std::string, Editor_PoiStatisticsResult>::iterator itor = date2StatisticsResult.begin();
        while(itor!=date2StatisticsResult.end())
        {
            rapidjson::Document document;
            document.SetObject();
            rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
            
            rapidjson::Value each_json_value(kStringType);
            document.AddMember("sumCount",itor->second.sumCount,allocator);
            document.AddMember("delCount",itor->second.delCount,allocator);
            document.AddMember("modifyCount",itor->second.modifyCount,allocator);
            document.AddMember("addCount",itor->second.addCount,allocator);
            each_json_value.SetString(itor->first.c_str(), itor->first.size(),allocator);
            document.AddMember("toperateDate",each_json_value,allocator);
            
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<StringBuffer> writer(buffer);
            document.Accept(writer);
            std::string result = buffer.GetString();
            results.push_back(result);
            itor++;
        }
        return 0;
    }

	
    std::string APIWrapper::GetDownloadPrepared(const std::string& input)
    {
        std::string sResult;

        rapidjson::Document document;
        
        document.Parse<0>(input.c_str());
        
        if (document.HasParseError())
        {
            Logger::LogD("GetDownloadPrepared inputJson json is error!!!");
            Logger::LogO("GetDownloadPrepared inputJson json is error!!!");
            
            sResult = Editor::ProjectManager::getInstance()->GeneratePreparedJson(-2, "");

            return sResult;
        }

        std::string sToken;
        std::string sProjectId;
        std::string sUserId;
        std::string downloadLatestTime;

        int type = -1;

        if(document.HasMember("token"))
        {
            sToken = document["token"].GetString();
        }

        if(document.HasMember("project"))
        {
            sProjectId = document["project"].GetString();
        }

        if(document.HasMember("user"))
        {
            sUserId = document["user"].GetString();
        }

        if(document.HasMember("time"))
        {
            downloadLatestTime = document["time"].GetString();
        }

        if(document.HasMember("type"))
        {
            type = document["type"].GetInt();
        }

        if(sToken.empty() || sProjectId.empty() || sUserId.empty())
        {
            Logger::LogD("inputJson error!!!");
            Logger::LogO("inputJson error!!!");

            sResult = Editor::ProjectManager::getInstance()->GeneratePreparedJson(-2, "");

            return sResult;
        }


        Editor::ProjectManager* pManager = Editor::ProjectManager::getInstance();

	    pManager->SetCurrentUser(sUserId.c_str());

	    pManager->CreateDatabase();

        sResult = pManager->getMapDataInstaller()->GetDownloadPrepared(sToken, sProjectId, sUserId, downloadLatestTime, type);

        return sResult;
    }

    void APIWrapper::DownloadFile(const std::string& input, ProgressEventer* progress)
    {     
        Editor::ProjectManager* pManager = Editor::ProjectManager::getInstance();

        Editor::MapDataInstaller* pInstall = pManager->getMapDataInstaller();

        pInstall->DownloadFile(input, progress);
    }

    std::string APIWrapper::GetUploadPrepared(const std::string& input)
    {
        std::string sResult;

        rapidjson::Document document;
        
        document.Parse<0>(input.c_str());
        
        if (document.HasParseError())
        {
            Logger::LogD("GetDownloadPrepared inputJson json is error!!!");
            Logger::LogO("GetDownloadPrepared inputJson json is error!!!");
            
            UploadMsg msg;
			msg.conncode = -2;
			sResult = Editor::ProjectManager::getInstance()->GeneratePreparedJson(msg);

            return sResult;
        }

        std::string sToken;
        std::string sProjectId;
        std::string sUserId;
        std::string sDeviceId;

        int type = -1;

        if(document.HasMember("token"))
        {
            sToken = document["token"].GetString();
        }

        if(document.HasMember("project"))
        {
            sProjectId = document["project"].GetString();
        }

        if(document.HasMember("user"))
        {
            sUserId = document["user"].GetString();
        }

        if(document.HasMember("deviceid"))
        {
            sDeviceId = document["deviceid"].GetString();
        }

        if(document.HasMember("type"))
        {
            type = document["type"].GetInt();
        }

        if(sToken.empty() || sProjectId.empty() || sDeviceId.empty() || sUserId.empty() || type == -1)
        {
            Logger::LogD("upload inputJson error!!!");
            Logger::LogO("upload inputJson error!!!");

            UploadMsg msg;
			msg.conncode = -2;
			sResult = Editor::ProjectManager::getInstance()->GeneratePreparedJson(msg);

            return sResult;
        }

        Editor::ProjectManager* pManager = Editor::ProjectManager::getInstance();

	    pManager->SetCurrentUser(sUserId.c_str());

	    pManager->CreateDatabase();

        sResult = pManager->getMapDataPackager()->GetUploadPrepared(sToken, sProjectId, sDeviceId, type);

        return sResult;
    }

    void APIWrapper::UploadFile(const std::string& input, ProgressEventer* progress)
    {
        Editor::ProjectManager* pManager = Editor::ProjectManager::getInstance();

        Editor::MapDataPackager* pPackager = pManager->getMapDataPackager();

        pPackager->UploadFile(input, progress);
    }
    
    int APIWrapper::InsertFeature(const std::string& tableName, const std::string& json)
    {
        Editor::DataManager* dataManager = Editor::DataManager::getInstance();
        
        Editor::DataFunctor* df = dataManager->getDataFunctor();
        
        int result = df->InsertFeature(tableName, json);
        
        return result;
    }

    int APIWrapper::UpdateFeature(const std::string& tableName, const std::string& json)
    {
        Editor::DataManager* dataManager = Editor::DataManager::getInstance();
        
        Editor::DataFunctor* df = dataManager->getDataFunctor();
        
        int result = df->UpdateFeature(tableName, json);
        
        return result;
    }
    
    int APIWrapper::DeleteFeature(const std::string& tableName, int rowId)
    {
        Editor::DataManager* dataManager = Editor::DataManager::getInstance();
        
        Editor::DataFunctor* df = dataManager->getDataFunctor();
        
        int result = df->DeleteFeature(tableName, rowId);
        
        return result;
    }
}
