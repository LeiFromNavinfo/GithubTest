#include "Editor_ProjectManager.h"
#include <stdio.h>
#include "md5.hh"
#include "BussinessConstants.h"


namespace Editor
{
    MapDataInstaller::MapDataInstaller() 
    {
        m_pDataDownloader = Editor::DataManager::getInstance()->getDownloader();

        m_pDataDownloader->SetMaxConcurrent(3);
    }

    MapDataInstaller::~MapDataInstaller() {}

    void MapDataInstaller::AddProjectId(const char* projectId)
    {
        m_vProjectId.push_back(projectId);
    }

    void MapDataInstaller::ClearProjectIds()
    {
        m_vProjectId.clear();
    }

    void MapDataInstaller::AddInstallType(int type)
    {
        m_vType.push_back(type);
    }

    void MapDataInstaller::ClearInstallTypes()
    {
        m_vType.clear();    
    }

    void MapDataInstaller::SetCurrentUserId(const char* userId)
    {
        m_sUserId = userId;
    }

    void MapDataInstaller::Run(const std::string& sToken, Job::Cmd cmd, ProgressEventer* progress)
    {
        if(m_vType.empty())
        {
            return;
        }

        if(m_vProjectId.empty())
        {
            std::vector<int>::iterator iterator = find(m_vType.begin(),m_vType.end(),PatternImage);

            if(iterator == m_vType.end())
            {
                return;
            }
        }

        Logger::LogD("Type size is [%d], projectId size is [%d]", m_vType.size(), m_vProjectId.size());

        for(int i = 0; i < m_vType.size(); i++)
        {
            switch(m_vType[i])
            {
            case 0:
                {
                    DownloadPoi(sToken, cmd, progress, i);
                    break;
                }
            case 1:
                {
                    DownloadInfo(sToken, cmd, progress, i);
                    break;
                }
            case 2:
                {
                    DownloadGdb(sToken, cmd, progress, i);
                    break;
                }
            case 3:
                {
                    DownloadTips(cmd, progress, i);
                    break;
                }
            case PatternImage:
                {
                    DownloadPatternImage(cmd, progress, i);
                    break;
                }
            }
        }
    }

    void MapDataInstaller::Stop()
    {
        m_pDataDownloader->Cancel();
    }

    int MapDataInstaller::DownloadPoi(const std::string& sToken, Job::Cmd cmd, ProgressEventer* progress, int nIdx)
    {
        for(int j = 0; j < m_vProjectId.size(); j++)
        {
            Editor::Job* job = new Editor::Job();

            job->SetJobType(m_vType[nIdx]);

            Model::ProjectUser* PU = new Model::ProjectUser();

            GetPUFromDatabase(m_vProjectId[j], m_vType[nIdx], PU);

            std::string sMd5;

            if(cmd == (Job::Cmd)1)
            {
                if(PU == NULL || PU->sProjectId.empty() || PU->sUserId.empty())

                {
                    Logger::LogD("this download request does not have notes before");
                    Logger::LogO("this download request does not have notes before");

                    return -1;
                }
            }
            else
            {

                PU->sProjectId = m_vProjectId[j];

                PU->sUserId = m_sUserId;

                PU->nType = m_vType[nIdx];

                std::string sUrl = Editor::DataManager::getInstance()->getServiceAddress()->GetPoiDownloadUrl(sToken, m_vProjectId[j], m_sUserId,PU->download_latest, false);
                
                std::string sUrlTmp;

                std::string sDateTmp;

                if(RequestServerToGenerateData("poi", sUrl, sUrlTmp, sDateTmp, sMd5) == -1)
                {
                    return -1;
                }

                PU->download_url = sUrlTmp;

                PU->download_latest = sDateTmp;
            }

            if(PU->download_url.empty())
            {
                Logger::LogD("upload_url error, please check database and so on");
                Logger::LogO("upload_url error, please check database and so on");

                return -1;
            }

            job->SetUrl(PU->download_url);

            std::string sPath = m_pDataFileSystem->GetDownloadPath() + GetFilenameFromDir(PU->download_url);;

            if(Tools::CheckFilePath(sPath.c_str()) == false)
		    {
			    Logger::LogD("the path that is used to save the downloadfile is not existing");
                Logger::LogO("the path that is used to save the downloadfile is not existing");
			    return -1;
		    }

            job->SetFileName(sPath);

            job->SetProjectUser(PU);

            job->m_sMd5 = sMd5;

            job->m_sFoundFlag = m_vProjectId[j];

            PU->InsertOrUpdateSqliteTable(m_pDataFileSystem->GetProjectDataPath());

            m_pDataDownloader->StartJob(job, NULL, progress);
        }

        return 0;
    }

    int MapDataInstaller::DownloadInfo(const std::string& sToken, Job::Cmd cmd, ProgressEventer* progress, int nIdx)
    {
		for(int j = 0; j < m_vProjectId.size(); j++)
        {
            Editor::Job* job = new Editor::Job();

            job->SetJobType(m_vType[nIdx]);

            Model::ProjectUser* PU = new Model::ProjectUser();
                
            GetPUFromDatabase(m_vProjectId[j], m_vType[nIdx], PU);

            std::string sMd5;

            if(cmd == (Job::Cmd)1)
            {
                if(PU == NULL || PU->sProjectId.empty() || PU->sUserId.empty())
                {
                    Logger::LogD("this download request does not have notes before");
                    Logger::LogO("this download request does not have notes before");

                    return -1;
                }
            }
            else
            {
                PU->sProjectId = m_vProjectId[j];

                PU->sUserId = m_sUserId;

                PU->nType = m_vType[nIdx];
            std::string sUrl = Editor::DataManager::getInstance()->getServiceAddress()->GetInfoDownloadUrl(sToken);

            std::string sUrlDest;

            std::string sDateDest;


            //std::string para = GenerateDownloadInforpara(m_vProjectId[j], PU->download_latest);齐济

            std::string debug;//情报业务需求，每次全量下载数据

            debug = GenerateDownloadInforpara(m_vProjectId[j], debug);

            if(RequestServerToGenerateInfor(sUrl, debug, sUrlDest, sDateDest, sMd5) == -1)
            {
                return -1;
            }

            if(sUrlDest.empty())
            {
                Logger::LogD("parse url from response of server unsuccessfully");
                Logger::LogO("parse url from response of server unsuccessfully");

                return -1;
            }

                PU->download_url = sUrlDest;

                PU->download_latest = sDateDest;
            }

            if(PU->download_url.empty())
            {
                Logger::LogD("upload_url error, please check database and so on");
                Logger::LogO("upload_url error, please check database and so on");

                return -1;
            }
            job->SetUrl(PU->download_url);

            std::string sPath = m_pDataFileSystem->GetDownloadPath() + GetFilenameFromDir(PU->download_url);;

            if(Tools::CheckFilePath(sPath.c_str()) == false)
		    {
			    Logger::LogD("the path that is used to save the downloadfile is not existing");
                Logger::LogO("the path that is used to save the downloadfile is not existing");
			    return -1;
		    }

            if(access(sPath.c_str(), 00) != -1)
            {
            	Logger::LogD("delete file of existing that is going to be downloaded");
				Logger::LogO("delete file of existing that is going to be downloaded");

            	remove(sPath.c_str());
            }

            job->SetFileName(sPath);

            job->SetProjectUser(PU);
            job->m_sMd5 = sMd5;

            job->m_sFoundFlag = m_vProjectId[j];
            PU->InsertOrUpdateSqliteTable(m_pDataFileSystem->GetProjectDataPath());

            m_pDataDownloader->StartJob(job, NULL, progress);
        }
        return 0;
    }

    int MapDataInstaller::DownloadGdb(const std::string& sToken, Job::Cmd cmd, ProgressEventer* progress, int nIdx)
    {
        for(int j = 0; j < m_vProjectId.size(); j++)
        {
            Editor::Job* job = new Editor::Job();

            job->SetJobType(m_vType[nIdx]);

            std::string sUrl = Editor::DataManager::getInstance()->getServiceAddress()->GetGDBDwonloadUrl(m_vProjectId[j]);
                
            std::string sUrlDest;

            std::string sDateDest;

            std::string sMd5;

            if(RequestServerToGenerateData("gdb", sUrl, sUrlDest, sDateDest, sMd5) == -1)
            {
                return -1;
            }

            if(sUrlDest.empty())
            {
                Logger::LogD("upload_url error, please check database and so on");
                Logger::LogO("upload_url error, please check database and so on");

                return -1;
            }

            job->SetUrl(sUrlDest);

            std::string sPath = m_pDataFileSystem->GetDownloadPath() + GetFilenameFromDir(sUrlDest);;

            if(Tools::CheckFilePath(sPath.c_str()) == false)
		    {
			    Logger::LogD("the path that is used to save the downloadfile is not existing");
                Logger::LogO("the path that is used to save the downloadfile is not existing");
			    return -1;
		    }

            if(access(sPath.c_str(), 00) != -1)
            {
            	Logger::LogD("delete file of existing that is going to be downloaded");
				Logger::LogO("delete file of existing that is going to be downloaded");

            	remove(sPath.c_str());
            }

            job->SetFileName(sPath);

            job->m_sFoundFlag = m_vProjectId[j];

            m_pDataDownloader->StartJob(job, NULL, progress);
        }

        return 0;
    }

    int MapDataInstaller::DownloadTips(Job::Cmd cmd, ProgressEventer* progress, int nIdx)
    {
         Editor::Job* job = new Editor::Job();
         
         job->SetJobType(m_vType[nIdx]);
         
         std::string sUrl = Editor::DataManager::getInstance()->getServiceAddress()->GetTipsDownloadUrl(m_vProjectId);
         
         std::string sUrlDest;
         
         if(RequestServerToGenerateTips(sUrl, sUrlDest) == -1)
         {
        	 if(progress != NULL)
        	 {
                 std::string sFlag = GetGridFlag();
                 std::string sDescription = Tools::GernerateTipsProgressInfo(sFlag, "下载", "准备数据出错");
                 progress->OnProgress(sDescription, -1);
        	 }
             return -1;
         }
         
         if(sUrlDest.empty())
         {
             Logger::LogD("upload_url error, please check database and so on");
             Logger::LogO("upload_url error, please check database and so on");
             
             return -1;
         }
         job->SetUrl(sUrlDest);
         
         std::string sPath = m_pDataFileSystem->GetDownloadPath() + GetFilenameFromDir(sUrlDest);;
         
         if(Tools::CheckFilePath(sPath.c_str()) == false)
         {
             Logger::LogD("the path that is used to save the downloadfile is not existing");
             Logger::LogO("the path that is used to save the downloadfile is not existing");
             return -1;
         }
         job->m_sFoundFlag =  GetGridFlag();
         job->SetFileName(sPath);
         m_pDataDownloader->StartJob(job, NULL, progress);

         return 0;
    }

    int MapDataInstaller::DownloadPatternImage(Job::Cmd cmd, ProgressEventer* progress, int nIdx)
    {
        Editor::Job* job = new Editor::Job();
        
        job->SetJobType(m_vType[nIdx]);
        
        std::string sUrl = Editor::DataManager::getInstance()->getServiceAddress()->GetPatternImageDownLoadUrl();
        
        std::string sUrlDest;
        
        if(RequestServerToGeneratePatternImage(sUrl, sUrlDest) == -1)
        {
            return -1;
        }
        
        if(sUrlDest.empty())
        {
            Logger::LogD("Parse PatternImage download url error");
            Logger::LogO("Parse PatternImage download url error");
            
            return -1;
        }
        job->SetUrl(sUrlDest);
        
        std::string sPath = m_pDataFileSystem->GetDownloadPath() + GetFilenameFromDir(sUrlDest);;
        
        if(Tools::CheckFilePath(sPath.c_str()) == false)
        {
            Logger::LogD("the path that is used to save the downloadfile is not existing");
            Logger::LogO("the path that is used to save the downloadfile is not existing");
            
            return -1;
        }
        job->m_sFoundFlag = "PatternImage";
        job->SetFileName(sPath);
        m_pDataDownloader->StartJob(job, NULL, progress);

        return 0;
    }

    void MapDataInstaller::CallBack(Editor::Job* job)
    {
        int nType = job->GetJobType();

        switch(nType)
            {
            case 0:
                {
                    POICallBack(job);
                    break;
                }
            case 1:
                {
                    INFOCallBack(job);
                    break;
                }
            case 2:
                {
                	GDBCallBack(job);
                	break;
                }
            case 3:
                {
                    TipsCallBack(job);
                    break;
                }
            case PatternImage:
               {
                   PatternImageCallBack(job);
                   break;
               }
            }
        delete job;
    }

    int MapDataInstaller::POICallBack(Editor::Job* job)
    {
        std::string sFilePath = job->GetFileName();

        ifstream file(sFilePath.c_str(), ios::binary);
        
        MD5 md5Context(file);

        if(md5Context.hex_digest() != job->m_sMd5)
        {
            Logger::LogD("the file downloaded is damaged, please download again");
            Logger::LogO("the file downloaded is damaged, please download again");

            return -1;
        }

         Model::ProjectUser* pProjectUser = job->GetProjectUser();

        if(pProjectUser != NULL)
        {
            Editor::DataManager* pDataManager = Editor::DataManager::getInstance();

            Editor::DataFileSystem* pFileSystem = pDataManager->getFileSystem();

            pProjectUser->download_url = "";

            pProjectUser->InsertOrUpdateSqliteTable(pFileSystem->GetProjectDataPath());
        }

        Editor::DataManager* dataManager = Editor::DataManager::getInstance();

	    Editor::CoreMapDataSource* dataSource = dynamic_cast<Editor::CoreMapDataSource*>(dataManager->getDataSource(DATASOURCE_COREMAP));
        
        if (NULL==dataSource)
        {
            return -1;
        }

        if(dataSource->InstallPoi(sFilePath, job->m_sFoundFlag, job->GetPregressObj()) == -1)
        {
            return -1;
        }

        return 0;
    }

    int MapDataInstaller::INFOCallBack(Editor::Job* job)
    {
		std::string sFilePath = job->GetFileName();

        ifstream file(sFilePath.c_str(), ios::binary);
        
        MD5 md5Context(file);

        if(md5Context.hex_digest() != job->m_sMd5)
        {
            Logger::LogD("the file downloaded is damaged, please download again");
            Logger::LogO("the file downloaded is damaged, please download again");

            return -1;
        }

        Model::ProjectUser* pProjectUser = job->GetProjectUser();

        if(pProjectUser != NULL)
        {
            Editor::DataManager* pDataManager = Editor::DataManager::getInstance();

            Editor::DataFileSystem* pFileSystem = pDataManager->getFileSystem();

            pProjectUser->download_url = "";

            pProjectUser->InsertOrUpdateSqliteTable(pFileSystem->GetProjectDataPath());
        }

        Editor::DataManager* dataManager = Editor::DataManager::getInstance();

	    Editor::CoreMapDataSource* dataSource = dynamic_cast<Editor::CoreMapDataSource*>(dataManager->getDataSource(DATASOURCE_COREMAP));
        
        if (NULL==dataSource)
        {
            return -1;
        }

        if(dataSource->InstallInfo(sFilePath, job->m_sFoundFlag, job->GetPregressObj()) == -1)
        {
            return -1;
        }
        return 0;
    }

    int MapDataInstaller::GDBCallBack(Editor::Job* job)
    {
        std::string sFilePath = job->GetFileName();

//        ifstream file(sFilePath.c_str(), ios::binary);

//		MD5 md5Context(file);

//		if(md5Context.hex_digest() != job->m_sMd5)
//		{
//			Logger::LogD("the file downloaded is damaged, please download again");
//			Logger::LogO("the file downloaded is damaged, please download again");

//			return -1;
//		}

        Editor::DataManager* dataManager = Editor::DataManager::getInstance();

	    Editor::GdbDataSource* dataSource = dynamic_cast<Editor::GdbDataSource*>(dataManager->getDataSource(DATASOURCE_GDB));
        
        if(NULL==dataSource)
        {
            return -1;
        }

        if(dataSource->InstallGDB(sFilePath, job->m_sFoundFlag, job->GetPregressObj()) == -1)
        {
            return -1;
        }

        return 0;
    }

    int MapDataInstaller::TipsCallBack(Editor::Job* job)
    {
        std::string sFilePath = job->GetFileName();

        int nBeg = sFilePath.find_last_of(".zip");

        if(!(nBeg != std::string::npos && nBeg == sFilePath.length() - 1))
        {
        	if (job->GetPregressObj() != NULL)
        	{
        		std::string description = Tools::GenerateProgressInfo("tips数据", "tips数据下载失败");

        		job->GetPregressObj()->OnProgress(description, -1);
        	}

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
        	if (job->GetPregressObj() != NULL)
        	{
        		std::string description = Tools::GenerateProgressInfo("tips数据", "tips数据解压失败");

        		job->GetPregressObj()->OnProgress(description, -1);
        	}

            return -1;
        }

        sPathFolderTmp = Tools::UnZipDir(sFilePath.c_str(), sPathFolderTmp.c_str());

        int beg = 0;
#ifdef WIN32
        beg = sFilePath.find_last_of("\\");
#else
        beg = sFilePath.find_last_of("/");
#endif
        std::string fileName = sFilePath.substr(beg+1, sFilePath.length() - beg - 5);

#ifdef WIN32
        sPathFolderFileTmp = sPathFolderTmp + "\\tips.txt";
#else
        sPathFolderFileTmp = sPathFolderTmp + "/tips.txt";
#endif

        Editor::DataManager* dataManager = Editor::DataManager::getInstance();

	    Editor::CoreMapDataSource* dataSource = dynamic_cast<Editor::CoreMapDataSource*>(dataManager->getDataSource(DATASOURCE_COREMAP));
        
        if (NULL==dataSource)
        {
            if (job->GetPregressObj() != NULL)
            {
                std::string description = Tools::GenerateProgressInfo("tips数据", "tips数据安装失败[dataSource指针为空]");
                
                job->GetPregressObj()->OnProgress(description, -1);
            }

            return -1;
        }

        int result = 0;

        if(dataSource->InstallTipsEx(sPathFolderFileTmp,m_vProjectId, job->GetPregressObj()) == -1)
        {
        	if (job->GetPregressObj() != NULL)
        	{
        		std::string description = Tools::GenerateProgressInfo("tips数据", "tips数据安装失败");

        		job->GetPregressObj()->OnProgress(description, -1);
        	}

            result = -1;
        }
        else
        {
        	if (job->GetPregressObj() != NULL)
        	{
        		std::string description = Tools::GenerateProgressInfo("tips数据", "tips数据安装成功");

        		job->GetPregressObj()->OnProgress(description, 200);
        	}
        }

        Tools::DelDir(sPathFolderTmp.c_str());

        return result;
    }

    int MapDataInstaller::PatternImageCallBack(Editor::Job* job)
    {
        std::string sFilePath = job->GetFileName();

        Editor::DataManager* dataManager = Editor::DataManager::getInstance();

	    Editor::CoreMapDataSource* dataSource = dynamic_cast<Editor::CoreMapDataSource*>(dataManager->getDataSource(DATASOURCE_COREMAP));

        int nBeg = sFilePath.find_last_of(".zip");

        if(!(nBeg != std::string::npos && nBeg == sFilePath.length() - 1))
        {
            return -1;
        }

        std::string sPathFolderTmp = sFilePath.substr(0, sFilePath.length() - 4);

        sPathFolderTmp = Tools::UnZipDir(sFilePath.c_str(), sPathFolderTmp.c_str());

        if(sPathFolderTmp.empty())
        {
			Logger::LogD("Install PatternImage error :UnZip File Failed, Source File Path: %s",sFilePath.c_str());
			Logger::LogO("Install PatternImage error :UnZip File Failed, Source File Path: %s",sFilePath.c_str());

			return -1;
        }

        std::string sPathFolderFile;

#ifdef WIN32
        sPathFolderFile = sPathFolderTmp + "\\" + PatternImageDownloadFileName;
#else
        sPathFolderFile = sPathFolderTmp + "/" + PatternImageDownloadFileName;
#endif
        if(0 == access(sPathFolderFile.c_str(), F_OK))
        {
            return dataSource->InstallPatternImage(sPathFolderFile, job->m_sFoundFlag, job->GetPregressObj());
        }
        return 0;
    }


    void MapDataInstaller::SetFileSystem(DataFileSystem* pFileSystem)
    {
        m_pDataFileSystem = pFileSystem;
    }
	std::string MapDataInstaller::GenerateDownloadInforpara(const std::string& projectId, const std::string& date)
    {
        Document document;
        
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
        
        document.SetObject();
        
        document.AddMember("startdate", StringRef(date.c_str()), allocator);
        
        document.AddMember("projectId", StringRef(projectId.c_str()), allocator);
        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        std::string res = buffer.GetString();
        
        return res;
    }

    int MapDataInstaller::RequestServerToGenerateInfor(const std::string& sUrl, const std::string& para, std::string& sUrlDest, std::string& sDate, std::string& sMd5)
    {
        std::string sResult;

        Tools::HttpPost(sUrl.c_str(), sResult, para);

        Logger::LogD("Request Info [%s]", sResult.c_str());

        rapidjson::Document document;

        document.Parse<0>(sResult.c_str());

        if(document.HasParseError())
        {
            return -1;
        }

        rapidjson::Value& value = document["data"];

        if(value.IsNull())
        {
            Logger::LogD("Request Server To Generate Data failed");
            Logger::LogO("Request Server To Generate Data failed");

            return -1;
        }

        if(value.HasMember("downloadurl"))
        {
            sUrlDest = value["downloadurl"].GetString();
        }

        if(value.HasMember("date"))
        {
            sDate = value["date"].GetString();
        }

        if(value.HasMember("md5"))
        {
            sMd5 = value["md5"].GetString();
        }

        return 0;
    }
    
    int MapDataInstaller::RequestServerToGenerateData(const std::string& type, const std::string& sUrl, std::string& sUrlTmp, std::string& sDateTmp, std::string& sMd5)
    {
        std::string sResult;

        Tools::HttpGet(sUrl.c_str(), sResult);

        Logger::LogD("Request Info [%s]", sResult.c_str());

        rapidjson::Document document;

        document.Parse<0>(sResult.c_str());

        if(document.HasParseError())
        {
            return -1;
        }

        rapidjson::Value& value = document["data"];

        if(value.IsNull())
        {
            Logger::LogD("Request Server To Generate Data failed");
            Logger::LogO("Request Server To Generate Data failed");

            return -1;
        }

        if(value.HasMember("url"))
        {
            sUrlTmp = value["url"].GetString();
        }

        if(value.HasMember("date"))
        {
            sDateTmp = value["date"].GetString();
        }

        if(value.HasMember("md5"))
        {
            sMd5 = value["md5"].GetString();
        }

        if(value.HasMember("specVersion") && type == "gdb")
        {
            if(ProjectManager::getInstance()->CheckVersion(value["specVersion"].GetString(), GDBVersion) == false)
            {
                return -1;
            }
        }

        return 0;
    }

    int MapDataInstaller::RequestServerToGenerateTips(const std::string& sUrl, std::string& actualUrl)
    {
        std::string sResult;

        Tools::HttpGet(sUrl.c_str(), sResult);

        rapidjson::Document document;

        document.Parse<0>(sResult.c_str());

        if(document.HasParseError())
        {
            return -1;
        }

        if(!document.HasMember("errcode") || document["errcode"].GetInt() != 0 || !document.HasMember("data"))
        {
#ifdef __APPLE__
#if DEBUG
            printf("\n\nRequest URL:%s\nErro:%s\n\n",sUrl.c_str(),sResult.c_str());
#endif
#endif
            return -1;
        }

        actualUrl = document["data"].GetString();

        if(ProjectManager::getInstance()->MatchVersion(TIPSVersion) == false)
        {
            Logger::LogD("TIPSVersion : ModelVersion is error, please check!!!");
            Logger::LogO("TIPSVersion : ModelVersion is error, please check!!!");
            return -1;
        }

        return 0;
    }

    int MapDataInstaller::RequestServerToGeneratePatternImage(const std::string& sUrl, std::string& actualUrl)
    {
        std::string sResult;

        Tools::HttpGet(sUrl.c_str(), sResult);

        rapidjson::Document document;

        document.Parse<0>(sResult.c_str());

        if(document.HasParseError())
        {
            return -1;
        }

        if(!document.HasMember("errcode") || document["errcode"].GetInt() != 0 || !document.HasMember("data"))
        {
            return -1;
        }

        rapidjson::Value& dataValue = document["data"];

        if(!dataValue.HasMember("url"))
        {
            return -1;
        }

        actualUrl = dataValue["url"].GetString();
		
        if(ProjectManager::getInstance()->MatchVersion(IMAGEVersion) == false)
        {
            Logger::LogD("IMAGEVersion : ModelVersion is error, please check!!!");
            Logger::LogO("IMAGEVersion : ModelVersion is error, please check!!!");
            return -1;
        }

        return 0;
    }

    std::string MapDataInstaller::GetUrl(Model::ProjectUser* modelPU, bool isIncremental)
    {
        if(modelPU->nType == 1)
        {
            if(isIncremental == true)
            {
                return "http://fs.navinfo.com/fos/editsupportTest/info/compile/?projectId="+ modelPU->sProjectId +"&userId="+ modelPU->sUserId +"&date=" + Tools::GetCurrentDateTime();
            }
            else
            {
                return "http://fs.navinfo.com/fos/editsupportTest/info/compile/?projectId="+ modelPU->sProjectId +"&userId="+ modelPU->sUserId;
            }
        }
        else
        {
            if(isIncremental)
            {
                return "http://fs.navinfo.com/fos/editsupportTest/compile/?projectId="+ modelPU->sProjectId +"&userId="+ modelPU->sUserId +"&date=" + Tools::GetCurrentDateTime();
            }
            else
            {
                return "http://fs.navinfo.com/fos/editsupportTest/compile/?projectId="+ modelPU->sProjectId +"&userId="+ modelPU->sUserId;
            }
        }
    }

    std::string MapDataInstaller::GetFilenameFromDir(std::string sIn)
    {
        int nBeg = 0;

        nBeg = sIn.find_last_of("/");

        if(nBeg == std::string::npos)
        {
            return "";
        }

        return sIn.substr(nBeg + 1);
    }

    int MapDataInstaller::GetPUFromDatabase(std::string sProjectId, int nType, Model::ProjectUser* PU)
    {
        std::string sDBPath = m_pDataFileSystem->GetProjectDataPath();

        sqlite3* db = NULL;

        int ret = sqlite3_open_v2(sDBPath.c_str(), &db, SQLITE_OPEN_READWRITE, NULL);

        if(ret != SQLITE_OK)
        {
            Logger::LogD("DataSource::sqlite [%s] or [%s] open failed", sDBPath.c_str(), sqlite3_errmsg(db));
            Logger::LogO("DataSource::sqlite [%s] or [%s] open failed", sDBPath.c_str(), sqlite3_errmsg(db));

            sqlite3_close(db);
            return -1;
        }

        std::string sql = "select * from project_user where project_id='" + sProjectId + "' and user_id='" + m_sUserId + "' and type="+ Tools::NumberToString(nType);

        sqlite3_stmt* stmt;

        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("select %s failed [%s]", "Point", sqlite3_errmsg(db));
            Logger::LogO("select %s failed [%s]", "Point", sqlite3_errmsg(db));
            sqlite3_close(db);
		    return -1;
	    }


        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW)
        {
            PU->ParseSqlite(stmt);
        }
        else if(rc == SQLITE_DONE)
        {
            sqlite3_finalize(stmt);

            sqlite3_close(db);
            return 1; //表示无数据
        }
        sqlite3_finalize(stmt);

        sqlite3_close(db);

        return 0;
    }

    std::string MapDataInstaller::GetTipsDownloadUrl(const std::vector<int>& grids, const std::string& timeString)
    {
        std::string downLoadUrl = "";
        std::string url = "http://192.168.4.130/FosEngineWeb/fcc/tip/export?parameter=";

        Document urlDoc;
        urlDoc.SetObject();
        rapidjson::Document::AllocatorType& allocator = urlDoc.GetAllocator();

        rapidjson::Value gridsArray(rapidjson::kArrayType);
        for(int i = 0; i<grids.size(); i++)
        {
            gridsArray.PushBack(grids[i],allocator);
        }
        urlDoc.AddMember("grids",gridsArray, allocator);

        Value json_value(kStringType);  

        json_value.SetString(timeString.c_str(), timeString.size(),allocator);
        urlDoc.AddMember("date",json_value,allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);  
        urlDoc.Accept(writer);  
        std::string parameter = buffer.GetString();

        url += parameter;

        std::string sResult;

        Tools::HttpGet(url.c_str(), sResult);

        rapidjson::Document document;

        document.Parse<0>(sResult.c_str());

        if(document.HasParseError())
        {
            return downLoadUrl;
        }

        if(!document.HasMember("errcode") || document["errcode"].GetInt() != 0 || !document.HasMember("data"))
        {
            return downLoadUrl;
        }

        return document["data"].GetString();
    }

    void MapDataInstaller::RunTips(const std::vector<int>& grids, const std::string& timeString, ProgressEventer* progress)
    {
        std::string sPath = m_pDataFileSystem->GetCurrentUserFolderPath();

        std::string downLoadUri = GetTipsDownloadUrl(grids, timeString);

#ifdef WIN32            
        sPath += "\\" + GetFilenameFromDir(downLoadUri);
#else
        sPath += "/" + GetFilenameFromDir(downLoadUri);
#endif
        Editor::Job* job = new Editor::Job();

        job->SetProjectUser(NULL);
        job->SetFileName(sPath);
        job->SetUrl(downLoadUri);

        m_pDataDownloader->StartJob(job, NULL, progress);
    }
	
    std::string MapDataInstaller::GetGridFlag()
    {
        std::string sResult;
        for(int i = 0; i < m_vProjectId.size(); i++)
        {
            sResult += m_vProjectId[i];
        }
        return sResult;
    }
		
	std::string MapDataInstaller::GetDownloadPrepared(const std::string sToken, const std::string sProjectId, const std::string sUserId, const std::string time, int type)
    {
        std::string sUrl;

        std::string response;

        std::string sResult;

        int errcode = -2;

        if(type == 0)
        {
            sUrl = Editor::DataManager::getInstance()->getServiceAddress()->GetPoiDownloadUrl(sToken, sProjectId, sUserId,time, time!="");
            
            if(Tools::HttpGet(sUrl.c_str(),response, errcode) == false)
            {
                Logger::LogO("GetDownloadPrepared : HttpGet [0:poi] error!!!");
                Logger::LogD("GetDownloadPrepared : HttpGet [0:poi] error!!!");
            }
        }
        else if(type == 1)
        {
            sUrl = Editor::DataManager::getInstance()->getServiceAddress()->GetInfoDownloadUrl(sToken);

            std::string para = GenerateDownloadInforpara(sProjectId, time);

            if(Tools::HttpPost(sUrl.c_str(), response, para, errcode) == false)
            {
                Logger::LogO("GetDownloadPrepared : HttpGet [1:info] error!!!");
                Logger::LogD("GetDownloadPrepared : HttpGet [1:info] error!!!");
            }
        }

        sResult = Editor::ProjectManager::getInstance()->GeneratePreparedJson(errcode, response);

        return sResult;

    }

    void MapDataInstaller::DownloadFile(const std::string& inputJson, ProgressEventer* progress)
    {
        Logger::LogO("start to download  from [%s]", inputJson.c_str());

        rapidjson::Document document;
        
        document.Parse<0>(inputJson.c_str());
        
        if (document.HasParseError())
        {
            Logger::LogD("GetDownloadPrepared inputJson json is error!!!");
            Logger::LogO("GetDownloadPrepared inputJson json is error!!!");

            progress->OnProgress("inputJson error", -1);
            
            return ;
        }

        std::string sToken;
        std::string sProjectId;
        std::string sUrl;
        std::string sMD5;
        std::string sPath;

        int type = -1;
        int connectTime = -1;

        if(document.HasMember("token"))
        {
            sToken = document["token"].GetString();
        }

        if(document.HasMember("project"))
        {
            sProjectId = document["project"].GetString();
        }

        if(document.HasMember("url"))
        {
            sUrl = document["url"].GetString();
        }

        if(document.HasMember("md5"))
        {
            sMD5 = document["md5"].GetString();
        }

        if(document.HasMember("path"))
        {
            sPath = document["path"].GetString();
        }

        if(document.HasMember("type"))
        {
            type = document["type"].GetInt();
        }

        if(document.HasMember("connecttime"))
        {
            connectTime = document["connecttime"].GetInt();
        }

        if(sToken.empty() || sProjectId.empty() || sUrl.empty() || sMD5.empty())
        {
            Logger::LogD("inputJson parameter error!!!");
            Logger::LogO("inputJson parameter error!!!");

            progress->OnProgress("inputJson parameter error!!!", -1);

            return;
        }

        if(sPath.empty())
        {
            sPath = m_pDataFileSystem->GetDownloadPath() + GetFilenameFromDir(sUrl);;
        }

        if(Tools::CheckFilePath(sPath.c_str()) == false)
		{
			Logger::LogD("the path that is used to save the downloadfile is not existing");
            Logger::LogO("the path that is used to save the downloadfile is not existing");

            progress->OnProgress("save path is errror", -1);
			return;
		}

        Logger::LogD("start to download : type [%d]", type);

        Editor::Job* job = new Editor::Job();

        job->SetUrl(sUrl);

        job->SetJobType(type);

        job->SetFileName(sPath);

        job->m_sMd5 = sMD5;

        if(connectTime != -1)
        {
            job->m_connectTime = connectTime;
        }

        job->m_sFoundFlag = sProjectId;

        m_pDataDownloader->StartJob(job, NULL, progress);
    }
		
		
} // namespace Editor
