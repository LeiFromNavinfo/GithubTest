#ifndef POI_CLASSIFICATION_SELECTOR_H
#define POI_CLASSIFICATION_SELECTOR_H

#include "Editor.h"
#include "BussinessConstants.h"

namespace Editor
{
    int ClassificationSelector::GetCountByCondition(std::string sTable, int type)
    {
        try
	    {
        	Editor::DataManager* dataManager = Editor::DataManager::getInstance();

            std::string dataPath= dataManager->getFileSystem()->GetCoreMapDataPath();

			if(strcmp(dataPath.c_str(),"") == 0)
			{
				Logger::LogD("Editor: GetRdLineByPid failed, Data Path Empty");
				Logger::LogO("Editor: GetRdLineByPid failed, Data Path Empty");
				return -1;
			}

			Editor::CoreMapDataSource* dataSource = dynamic_cast<Editor::CoreMapDataSource*>(dataManager->getDataSource(DATASOURCE_COREMAP));
            
            if(NULL==dataSource)
            {
                return -1;
            }

            dataSource->Open(dataPath);

			Editor::DataLayer* layer = NULL;
			if(sTable == PoiTableName)
			{
				layer = dataSource->GetLayerByType(DATALAYER_POI);
			}
			else if(sTable == "edit_tips")
			{
				layer = dataSource->GetLayerByType(DATALAYER_TIPS);
			}

		    std::string sql = "";

		    switch(type)
		    {
			    case LIFECYCLE_ORIGINAL:
			    {
				    sql = "select count(*) from " + GetTablenameAndColumn(sTable) + " != 0";
				    break;
			    }
			    case LIFECYCLE_DELETE:
			    {
				    sql = "select count(*) from " + GetTablenameAndColumn(sTable) + " = 1";
				    break;
			    }
			    case LIFECYCLE_UPDATE:
			    {
				    sql = "select count(*) from " + GetTablenameAndColumn(sTable) + " = 2";
				    break;
			    }
			    case LIFECYCLE_ADD:
			    {
				    sql = "select count(*) from " + GetTablenameAndColumn(sTable) + " = 3";
				    break;
			    }
		    }

		    layer->ResetReadingBySQL(sql);

		    Editor::DataFeature* feature = layer->GetNextFeature();

		    int nReturn = 0;
            if(feature != NULL)
            {
                nReturn = feature->GetAsInteger(0);
            }

			//lee add
			dataSource->Close(dataPath);

            return nReturn;
	    }
	    catch(...)
	    {
		    Logger::LogD("Editor: Update Tips failed");
		    Logger::LogO("Editor: Update Tips failed");
		    return -1;
	    }

	    return 0;
    }

    std::vector<std::string> ClassificationSelector::GetSignalLayerByCondition(std::string sTable, int type, int pagesize, int offset)
    {
        std::vector<std::string> vStr;

	    try
	    {
	    	Editor::DataManager* dataManager = Editor::DataManager::getInstance();

            std::string dataPath= dataManager->getFileSystem()->GetCoreMapDataPath();

			if(strcmp(dataPath.c_str(),"") == 0)
			{
				Logger::LogD("Editor: GetRdLineByPid failed, Data Path Empty");
				Logger::LogO("Editor: GetRdLineByPid failed, Data Path Empty");
				return vStr;
			}

            Editor::CoreMapDataSource* dataSource = dynamic_cast<Editor::CoreMapDataSource*>(dataManager->getDataSource(DATASOURCE_COREMAP));
            
            if(NULL==dataSource)
            {
                return vStr;
            }
            
            dataSource->Open(dataPath);

			Editor::DataLayer* layer = NULL;

			if(sTable == PoiTableName)
			{
				layer = dataSource->GetLayerByType(DATALAYER_POI);
			}
			else if(sTable == "edit_tips")
			{
				layer = dataSource->GetLayerByType(DATALAYER_TIPS);
			}

            std::string sql = "";

            std::string sPageSize = Tools::NumberToString<int>(pagesize);

            std::string sOffset = Tools::NumberToString<int>(offset);

		    switch(type)
		    {
		        case LIFECYCLE_ORIGINAL:
		        {
			        sql = "select * from " + GetTablenameAndColumn(sTable) + " != 0 " + GenerateSqlFromTableName(sTable) + " limit " + sPageSize + " offset " + sOffset;
			        break;
		        }
		        case LIFECYCLE_DELETE:
		        {
			        sql = "select * from " + GetTablenameAndColumn(sTable) + " = 1 " + GenerateSqlFromTableName(sTable) + " limit " + sPageSize + " offset " + sOffset;
			        break;
		        }
		        case LIFECYCLE_UPDATE:
		        {
			        sql = "select * from " + GetTablenameAndColumn(sTable) + " = 2 " + GenerateSqlFromTableName(sTable) + " limit " + sPageSize + " offset " + sOffset;
			        break;
		        }
		        case LIFECYCLE_ADD:
		        {
			        sql = "select * from " + GetTablenameAndColumn(sTable) + " = 3 " + GenerateSqlFromTableName(sTable) + " limit " + sPageSize + " offset " + sOffset;
			        break;
		        }
		    }

		    layer->ResetReadingBySQL(sql);

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
		    Logger::LogD("Editor: Update Tips failed");
		    Logger::LogO("Editor: Update Tips failed");
		    return vStr;
	    }

	    return vStr;
    }

    std::string ClassificationSelector::GenerateSqlFromTableName(std::string sTable)
    {
		if(sTable == PoiTableName)
		{
			return "order by latestMergeDate DESC";
		}
		else if(sTable == "edit_tips")
		{
			return "order by t_operateDate DESC";
		}
		else
		{
			return "order by latestMergeDate DESC";
		}
    }

    std::string ClassificationSelector::GetTablenameAndColumn(std::string sTable)
    {
    	if(sTable == PoiTableName)
		{
			return  PoiTableName + " where lifecycle";
		}
		else if(sTable == "edit_tips")
		{
			return "edit_tips where t_lifecycle";
		}
		else
		{
			return PoiTableName + " where lifecycle";
		}

    }
} // namespace

#endif