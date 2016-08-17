#include "Editor.h"
#include "BussinessConstants.h"

namespace Editor
{	
	DataManager* DataManager::m_pInstance = NULL;

	DataManager::DataManager()
	{
		m_pDataUploader = new DataUploader();

		m_pDataDownloader = new DataDownloader();

		m_pDataServiceAddress = new DataServiceAddress();

		m_pDataFileSystem = new DataFileSystem();

        m_pDataFunctor = new DataFunctor();
	}

	DataManager::~DataManager()
	{
		if (m_pDataUploader)
		{
			delete m_pDataUploader;

			m_pDataUploader = NULL;
		}

		if (m_pDataDownloader)
		{
			delete m_pDataDownloader;

			m_pDataDownloader = NULL;
		}

		if (m_pDataServiceAddress)
		{
			delete m_pDataServiceAddress;

			m_pDataServiceAddress = NULL;
		}
        
		if (m_pDataFileSystem)
		{
			delete m_pDataFileSystem;

			m_pDataFileSystem = NULL;
		}
        
        for (std::map<int, DataSource*>::iterator itor = m_dataSourceMap.begin(); itor!=m_dataSourceMap.end(); ++itor)
        {
            if (itor->second)
            {
                delete itor->second;
                itor->second = NULL;
            }
        }
        m_dataSourceMap.clear();
	}

	DataManager* DataManager::getInstance()
	{
		if (m_pInstance == NULL)
		{
			m_pInstance = new DataManager();
		}

		return m_pInstance;
	}

	DataUploader* DataManager::getUploader()
	{
		return m_pDataUploader;
	}

	DataDownloader* DataManager::getDownloader()
	{
		return m_pDataDownloader;
	}

	DataServiceAddress* DataManager::getServiceAddress()
	{
		return m_pDataServiceAddress;
	}

	DataSource* DataManager::getDataSource(int type)
	{
        DataSource* pDataSource = NULL;
        std::map<int, DataSource*>::iterator itor = m_dataSourceMap.find(type);
        if (itor!=m_dataSourceMap.end())
        {
            pDataSource = itor->second;
        }
        else
        {
            pDataSource = createDataSource(type);
            if(pDataSource)
            {
                m_dataSourceMap.insert(std::make_pair(type, pDataSource));
            }
        }
		return pDataSource;
	}
    
    DataSource* DataManager::getDataSource(const std::string& layerName)
    {
        DataSource* pDataSource = NULL;
        if (strcmp(layerName.c_str(), PoiTableName.c_str())==0
            ||strcmp(layerName.c_str(), "edit_tips")==0
            ||strcmp(layerName.c_str(), "edit_infos")==0
            ||strcmp(layerName.c_str(), "tips_point")==0
            ||strcmp(layerName.c_str(), "tips_line")==0
            ||strcmp(layerName.c_str(), "tips_polygon")==0
            ||strcmp(layerName.c_str(), "tips_geo_component")==0
            ||strcmp(layerName.c_str(), track_collectionTableName.c_str())==0
            ||strcmp(layerName.c_str(), track_segmentTableName.c_str())==0)
        {
            pDataSource = getDataSource(DATASOURCE_COREMAP);
        }
        else if (strcmp(layerName.c_str(), "gdb_rdLine")==0
                 ||strcmp(layerName.c_str(), "gdb_rdNode")==0
                 ||strcmp(layerName.c_str(), "gdb_bkFace")==0
                 ||strcmp(layerName.c_str(), "gdb_bkLine")==0)
        {
             pDataSource = getDataSource(DATASOURCE_GDB);
        }
        
        else if (strcmp(layerName.c_str(), "project_user")==0
                 ||strcmp(layerName.c_str(), "project_info")==0
                 ||strcmp(layerName.c_str(), "task_info")==0)
        {
            pDataSource = getDataSource(DATASOURCE_PROJECT);
        }
        
        return pDataSource;
    }
    
     DataSource* DataManager::getDataSourceByLayerType(int layerType)
    {
        DataSource* pDataSource = NULL;
        switch (layerType) {
            case DATALAYER_POI:
            case DATALAYER_INFOR:
            case DATALAYER_TIPS:
            case DATALAYER_GPSLINE:
            case DATALAYER_TIPSPOINT:
            case DATALAYER_TIPSLINE:
            case DATALAYER_TIPSMULTILINE:
            case DATALAYER_TIPSPOLYGON:
            case DATALAYER_TIPSGEOCOMPONENT:
            case DATALAYER_TRACKPOINT:
            case DATALAYER_TRACKSEGMENT:
            {
                pDataSource = getDataSource(DATASOURCE_COREMAP);
                break;
            }
            case DATALAYER_RDLINE:
            case DATALAYER_BKLINE:
            case DATALAYER_FACE:
            case DATALAYER_RDNODE:
            case DATALAYER_RDLINE_GSC:
            {
                pDataSource = getDataSource(DATASOURCE_GDB);
                break;
            }
            case DATALAYER_PROJECTUSER:
            case DATALAYER_PROJECTINFO:
            case DATALAYER_TASKINFO:
            {
                pDataSource = getDataSource(DATASOURCE_PROJECT);
                break;
            }
            default:
                break;
        }
        return pDataSource;
    }
    
    DataSource* DataManager::createDataSource(int type)
    {
        DataSource* pDataSource = NULL;
        switch (type)
        {
            case DATASOURCE_COREMAP:
                pDataSource = new CoreMapDataSource();
                break;
            case DATASOURCE_GDB:
                pDataSource = new GdbDataSource();
                break;
            case DATASOURCE_PROJECT:
                pDataSource = new ProjectDataSource();
                break;
            default:
                break;
        }
        return pDataSource;
    }

	DataFileSystem* DataManager::getFileSystem()
	{
		return m_pDataFileSystem;
	}

    DataFunctor* DataManager::getDataFunctor()
    {
        return m_pDataFunctor;
    }

}