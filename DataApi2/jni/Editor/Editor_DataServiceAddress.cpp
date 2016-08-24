#include "Editor.h"
#include "Editor_ProjectManager.h"

namespace Editor
{	
	Editor::DataServiceAddress::DataServiceAddress() 
    {
        m_sRootUrlPoi = "";

        m_sRootUrlGDB = "192.168.4.130";

        m_sRootUrlTips = "";
    }

	Editor::DataServiceAddress::~DataServiceAddress() 
    {
        m_sRootUrlPoi = "";

        m_sRootUrlGDB = "";

        m_sRootUrlTips = "";
    }

    bool DataServiceAddress::Init(int ver)
    {
        switch(ver)
        {
        case EVIROMENT_CLOUD:
            {
            	Logger::LogD("init EVIROMENT_CLOUD...");

                m_sRootUrlPoi = "";

                m_sRootUrlGDB = "";

                m_sRootUrlTips = "";

                break;
            }
        case EVIROMENT_TEST:
            {
            	Logger::LogD("init EVIROMENT_TEST...");

                m_sRootUrlPoi = "";

                m_sRootUrlGDB = "";

                m_sRootUrlTips = "";

                break;
            }
        case EVIROMENT_DEBUG:
            {
            	Logger::LogD("init EVIROMENT_DEBUG...");

                m_sRootUrlPoi = "";

                m_sRootUrlGDB = "192.168.4.130";

                m_sRootUrlTips = "";

                break;
            }
        case EVIROMENT_REL:
            {
            	Logger::LogD("init EVIROMENT_REL...");

                m_sRootUrlPoi = "";

                m_sRootUrlGDB = "";

                m_sRootUrlTips = "";

                break;
            }
        default:
            {
            	Logger::LogD("init default...");

                m_sRootUrlPoi = "";

                m_sRootUrlGDB = "192.168.4.188";

                m_sRootUrlTips = "";
            }
        }

        return true;
    }
    
    std::string DataServiceAddress::GetGDBDwonloadUrl(std::string sProjectId)
    {
        return "http://192.168.4.188/service/dropbox/download/prjbasedata?parameter={%22prjId%22:" + sProjectId + "}";
    }

    std::string DataServiceAddress::GetPoiDownloadUrl(const std::string& token, std::string sProjectId, std::string sUserId, std::string date, bool isIncremental)
    {
       
        if(isIncremental)
        {
            return "http://192.168.4.189/fos/editsupport/v3/compile/?projectId="+ sProjectId +"&userId="+ sUserId +"&date=" + date + "&access_token=" + token;
        }
        else
        {
            return "http://192.168.4.189/fos/editsupport/v3/compile/?projectId="+ sProjectId +"&userId="+ sUserId + "&access_token=" + token;
        }

    }

    std::string DataServiceAddress::GetInfoDownloadUrl(const std::string& token)
    {
        return "http://192.168.4.189/fos/editsupport/infor/compile/?access_token=" + token;
    }
    std::string	DataServiceAddress::GetPoiUploadUrl(std::string sFileName, std::string sProject)
    {
    	return "http://192.168.4.189/fos/editsupportTest/upload/?filename=" + sFileName + "&projectId=" + sProject;
    }

    std::string DataServiceAddress::GetPoiUploadExcuteUrl(const std::string& token)
    {
        return "http://192.168.4.189/fos/editsupport/poi/incremental/?access_token=" + token;;
    }
    std::string DataServiceAddress::GetInforUploadExcuteUrl(const std::string sToken, const std::string sFileName)
    {
        return "http://192.168.4.189/fos/editsupport/infor/incremental/?access_token=" + sToken + "&filename=" + sFileName;
    }

    std::string DataServiceAddress::GetPoiUploadSnapShot(const std::string& token, const std::string& fileName, const std::string& md5, unsigned long fileSize)
    {
        return "http://192.168.4.189/fos/resources/upload/snapshot/?access_token=" + token + "&filename=" + fileName + "&md5=" + md5 + "&filelength=" + Tools::NumberToString<unsigned long>(fileSize);
    }

    std::string DataServiceAddress::GetPoiUploadDataChunk(const std::string& token, const std::string& fileName, int chunkNo)
    {
        //return "http://fs.navinfo.com/fos/resources/upload/datachunk/?access_token=" + token + "&filename=" + fileName + "&chunkno=" + Tools::NumberToString<unsigned long>(chunkNo);

        return "http://192.168.4.189/fos/resources/upload/datachunk/";
    }

    std::string DataServiceAddress::GetPoiUploadCheck(const std::string& token, const std::string& fileName)
    {
        return "http://192.168.4.189/fos/resources/upload/check/?access_token=" + token + "&filename=" + fileName;;

    }

    std::string DataServiceAddress::GetTipsDownloadUrl(const std::vector<std::string>& grids)
    {
        if(grids.size() == 0)
        {
            return "";
        }

        Editor::GridHelper* gridHelper = GridHelper::getInstance();

        gridHelper->CreateOrOpenDatabase();

        std::string timeStamp = gridHelper->GetGridsTimeStamp(grids, TIPS_DOWN);
        
        gridHelper->Close();

        std::string url = "http://192.168.4.188/service/fcc/tip/export?parameter=";

        rapidjson::Document urlDoc;
        urlDoc.SetObject();
        rapidjson::Document::AllocatorType& allocator = urlDoc.GetAllocator();

        rapidjson::Value gridsArray(rapidjson::kArrayType);
        for(int i = 0; i<grids.size(); i++)
        {
            rapidjson::Value grid_value(kStringType);  
            grid_value.SetString(grids[i].c_str(), grids[i].size(),allocator);
            gridsArray.PushBack(grid_value,allocator);
        }
        urlDoc.AddMember("grids",gridsArray, allocator);

        rapidjson::Value json_value(kStringType);  

        json_value.SetString(timeStamp.c_str(), timeStamp.size(),allocator);
        urlDoc.AddMember("date",json_value,allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);  
        urlDoc.Accept(writer);  
        std::string parameter = buffer.GetString();

        url += parameter;

        return url;
    }
    
    std::string DataServiceAddress::GetTipsUploadStartUrlRoot()
    {
        return "http://192.168.4.188/service/dropbox/upload/start?parameter=";
    }
    
    std::string DataServiceAddress::GetTipsUploadChunkUrlRoot()
    {
        return "http://192.168.4.188/service/dropbox/upload/chunk";
    }
    
    std::string DataServiceAddress::GetTipsUploadFinishUrlRoot()
    {
        return "http://192.168.4.188/service/dropbox/upload/finish?parameter=";
    }
    
    std::string DataServiceAddress::GetTipsUploadCheckUrlRoot()
    {
        return "http://192.168.4.188/service/dropbox/upload/check?parameter=";
    }
    
    std::string DataServiceAddress::GetTipsUploadImportUrlRoot()
    {
        return "http://192.168.4.188/service/fcc/tip/import?parameter=";
    }

    std::string DataServiceAddress::GetUpdataGridStatusUrlRoot()
    {
    	return "http://192.168.4.188/service/fcc/tip/checkUpdate?parameter=";
    }

    std::string DataServiceAddress::GetProjectInfomationUrl(const std::string& sToken)
    {
        return "http://192.168.4.189/fos/project/list/?access_token=" + sToken +"&parameter=";
    }

    std::string DataServiceAddress::GetProjectDetailsUrl(const std::string& sToken)
    {
        return "http://192.168.4.189/fos/project/query/?access_token=" + sToken + "&parameter=";
    }

    std::string DataServiceAddress::GetTaskDetailsUrl(const std::string& sToken)
    {
        return "http://192.168.4.189/fos/project/task/list/?access_token=" + sToken +"&parameter=";
    }

    std::string DataServiceAddress::GetPatternImageDownLoadUrl()
	{
        std::string url;

        Editor::GridHelper* gridHelper = GridHelper::getInstance();

        gridHelper->CreateOrOpenDatabase();

        std::string timeStamp = gridHelper->GetPatternImageTimeStamp(PatternImage_Download);
        
        gridHelper->Close();

        if(strcmp(timeStamp.c_str(),"") == 0)
        {
            url = "http://192.168.4.188/service/dropbox/download/patternimage";
        }
        else
        {
            url = "http://192.168.4.188/service/metadata/patternImage/download?parameter={%22date%22:%22"+ timeStamp +"%22}";
        }

	    return url;
	}
    std::string DataServiceAddress::GetModelVersionUrl(int type)
    {
        return "http://192.168.4.188/service/project/version/get?parameter={%22type%22:" + Tools::NumberToString<int>(type) + "}";
    }
    std::string DataServiceAddress::GetRobTaskUrl(const std::string& token, const std::string& projectId, const std::string& taskId)
    {
        return "http://192.168.4.189/fos/project/task/rob/?access_token=" + token + "&projectId=" + projectId + "&taskId="+ taskId;
    }
    std::string DataServiceAddress::GetUploadTime(const std::string& token, const std::string& projectId, const std::string& deviceId)
    {
        return "http://192.168.4.189/fos/project/user/uploadTime/?access_token=" + token + "&projectId=" + projectId + "&deviceId=" + deviceId;
    }
}