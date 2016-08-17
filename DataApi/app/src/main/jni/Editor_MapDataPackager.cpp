#include "Editor_ProjectManager.h"
#include "Locker.h"
#include "boost/thread/mutex.hpp"
#include "Editor_ResumeProcessor.h"
#include "md5.hh"
#include "document.h"
#include "writer.h"
#include "BussinessConstants.h"
#include "Editor_Data.h"
namespace Editor
{
    MapDataPackager::MapDataPackager() 
    {
        m_pDataUploader = DataManager::getInstance()->getUploader();
        
        m_pDataUploader->SetMaxConcurrent(5);
        
        m_MaxChunkSize = 1024 * 1024 * 5;
        
        m_MaxUploadFileTryCount = 5;

        m_nChunkNum = 0;

        m_nSuccessChunkNum = 0;
    }

    MapDataPackager::~MapDataPackager() {}

    void MapDataPackager::AddProjectId(const char* projectId)
    {
        m_vProjectId.push_back(projectId);
    }

    void MapDataPackager::ClearProjectIds()
    {
        m_vProjectId.clear();
    }

    void MapDataPackager::AddPackageType(int type)
    {
        m_vType.push_back(type);;
    }
    
    void MapDataPackager::ClearPackageTypes()
    {
        m_vType.clear();
    }
    
    void MapDataPackager::AddGridId(const char* gridId)
    {
        m_vGridId.push_back(gridId);
    }
    
    void MapDataPackager::ClearGridIds()
    {
        m_vGridId.clear();
    }
    
    void MapDataPackager::SetCurrentUserId(const char* userId)
    {
        m_sUserId = userId;
    }
    
    void MapDataPackager::Run(std::string sToken, JOBCMD cmd, ProgressEventer* progress, const std::string& deviceId)
    {
        if (progress)
        {
        	m_Progress = progress;

            m_pDataUploader->SetProgress(progress);
        }
        
        std::vector<int>::const_iterator itor = m_vType.begin();
        
        for (; itor!=m_vType.end(); itor++)
        {
            int type = *itor;
            
            switch (type)
            {
                case POI:
                {
                	Logger::LogD("start to upload poi...");
                	Logger::LogO("start to upload poi...");
                    RunPoi(sToken, cmd, progress, type, deviceId);
                }
                    break;
                case POI_INFO:
                {
                    Logger::LogD("start to upload infor...");
                    Logger::LogO("start to upload infor...");
                    RunInfor(sToken, cmd, progress, type);
                }
                    break;
                    
                case TIPS:
                {
                	Logger::LogD("start to upload tips...");
                	Logger::LogO("start to upload tips...");
                	RunTips(cmd, progress);
                }
                    break;
                    
                case ROAD_INFO:
                {
                    Logger::LogD("start to upload infor...");
                    Logger::LogO("start to upload infor...");
                    RunInfor(sToken, cmd, progress, type);
                }
                    break;
                default:
                    break;
            }
        }
    }
    
    void MapDataPackager::RunPoi(std::string sToken, JOBCMD cmd, ProgressEventer* progress, int nIdx, const std::string& deviceId)
    {
    	CheckLastUploadIsCorrect();

    	for(int j = 0; j < m_vProjectId.size(); j++)
    	{
            Model::ProjectUser* PU = new Model::ProjectUser();
            
            GetPUFromDatabase(m_vProjectId[j], nIdx, PU);

			if(PU == NULL)
			{
				Logger::LogD("this upload does not have the notes that express download");
				Logger::LogO("this upload does not have the notes that express download");

				return;
			}

            std::string sFilename;

            std::string errmsg;

            int total=0;
            int add=0;
            int upd=0;
            int del=0;
			std::string sDataPath = GetUploadFile(sFilename, PU->upload_latest, errmsg, m_vProjectId[j], total, add, upd, del, POI);

            if(sDataPath.empty())
            {
                progress->OnProgress(Tools::GenerateProgressInfo(m_vProjectId[j], errmsg), -1);

                return;
            }

            unsigned long nFileSize;

            int nChunk;

            if(UploadSnapShot(sDataPath.c_str(), sFilename.c_str(), sToken.c_str(), nFileSize, nChunk, nIdx) == false)
            {
                return;
            }

            m_nChunkNum = nChunk;
			
            for(int k = 0; k < nChunk; k++)
            {
                Editor::Job* job = new Editor::Job();

                job->m_lTotal = nFileSize;

			    job->SetJobType(nIdx);

			    job->m_sFoundFlag = m_vProjectId[j];

                std::string sUrl = Editor::DataManager::getInstance()->getServiceAddress()->GetPoiUploadDataChunk(sToken, sFilename, k, nIdx);

			    job->SetUrl(sUrl);

			    job->SetFileName(sDataPath);

                PU->upload_url = sUrl;
			    job->SetProjectUser(PU);

                job->m_nChunkNo = k;

                job->m_nMaxChunkSize = m_MaxChunkSize;

                job->m_sFileName = sFilename;

                job->m_sToken = sToken;

                job->m_sDeviceId = deviceId;
                
                m_pDataUploader->StartJob(job, progress);
            }
            PU->InsertOrUpdateSqliteTable(m_pDataFileSystem->GetProjectDataPath());
    	}

    }

    void MapDataPackager::RunInfor(std::string sToken, JOBCMD cmd, ProgressEventer* progress, int nIdx)
    {
        CheckLastUploadIsCorrect();

        for(int j = 0; j < m_vProjectId.size(); j++)
        {
            Model::ProjectUser* PU = new Model::ProjectUser();
            
            GetPUFromDatabase(m_vProjectId[j], nIdx, PU);

			if(PU == NULL)
			{
				Logger::LogD("this upload does not have the notes that express download");
				Logger::LogO("this upload does not have the notes that express download");

				return;
			}

            PU->upload_latest = Tools::GetCurrentDateTime();
            std::string sPackageSql = "select * from edit_infos where submitStatus = 0;";

            std::string sFilename;

            std::string errmsg;
            
            std::string sUploadFilePath = GetUploadInfoFile(sFilename, errmsg, sPackageSql,m_vProjectId[j],nIdx);

            if(sUploadFilePath.empty())
            {
                progress->OnProgress(Tools::GenerateProgressInfo(m_vProjectId[j], errmsg), -1);

                return;
            }

            unsigned long nFileSize;

            int nChunk;

            if(UploadSnapShot(sUploadFilePath.c_str(), sFilename.c_str(), sToken.c_str(), nFileSize, nChunk, nIdx) == false)
            {
                return;
            }

            m_nChunkNum = nChunk;
			
            for(int k = 0; k < nChunk; k++)
            {
                Editor::Job* job = new Editor::Job();

                job->m_lTotal = nFileSize;

			    job->SetJobType(nIdx);

			    job->m_sFoundFlag = m_vProjectId[j];

                std::string sUrl = Editor::DataManager::getInstance()->getServiceAddress()->GetPoiUploadDataChunk(sToken, sFilename, k, nIdx);

			    job->SetUrl(sUrl);

                PU->upload_url = sUrl;

			    job->SetProjectUser(PU);

			    job->SetFileName(sUploadFilePath);

                job->m_nChunkNo = k;

                job->m_nMaxChunkSize = m_MaxChunkSize;

                job->m_sFileName = sFilename;

                job->m_sToken = sToken;

			    m_pDataUploader->StartJob(job, progress);
            }
            PU->InsertOrUpdateSqliteTable(m_pDataFileSystem->GetProjectDataPath());
        }
    }
    void MapDataPackager::CallBack(Editor::Job* job)
    {
    	int nType = job->GetJobType();

        switch(nType)
            {
            case POI:
                {
                    PoiCallBack(job);
                    break;
                }
            case POI_CHARGING:
                {
                    PoiCallBack(job);
                    break;
                }
            case POI_INFO:
                {
                    InforCallBack(job);
					break;
				}
            case ROAD_INFO:
                {
                    InforCallBack(job);
                    break;
                }
			}
    }

    void MapDataPackager::FailedCallBack(Editor::Job* job)
    {
        Stop();

        Editor::ProgressEventer* pPregress = job->GetPregressObj();

		if(pPregress != NULL)
		{
            pPregress->OnProgress(Tools::GenerateProgressInfo(job->m_sFoundFlag, "某一块连续三次上传失败，导致上传任务失败"), -1);
		}
    }

    void MapDataPackager::PoiCallBack(Editor::Job* job)
    {
        Locker::Lock();

        m_nSuccessChunkNum++;

        if(m_nSuccessChunkNum == m_nChunkNum)
        {
            if(UploadCheck((job->m_sToken).c_str(), (job->m_sFileName).c_str(), job->GetJobType()) == false)
            {
                Locker::UnLock();

                return;
            }

            Model::ProjectUser* pProjectUser = job->GetProjectUser();

		    if(pProjectUser != NULL)
		    {
			    Editor::DataManager* pDataManager = Editor::DataManager::getInstance();

			    Editor::DataFileSystem* pFileSystem = pDataManager->getFileSystem();

			    pProjectUser->upload_url = "";

                pProjectUser->InsertOrUpdateSqliteTable(pFileSystem->GetProjectDataPath());

                //SaveToMainDBAfterUpload(job, "poi");
		    }
            
		    SaveToMainDBAfterUpload(job, "poi");
            
            m_nSuccessChunkNum = 0;
        }

        Locker::UnLock();

	}

    void MapDataPackager::InforCallBack(Editor::Job* job)
    {
        Locker::Lock();
        m_nSuccessChunkNum++;

        if(m_nSuccessChunkNum == m_nChunkNum)
        {
            if(UploadCheck((job->m_sToken).c_str(), (job->m_sFileName).c_str(),job->GetJobType()) == false)
            {
                Locker::UnLock();

                return;
            }

            Model::ProjectUser* pProjectUser = job->GetProjectUser();

		    if(pProjectUser != NULL)
		    {
			    Editor::DataManager* pDataManager = Editor::DataManager::getInstance();

			    Editor::DataFileSystem* pFileSystem = pDataManager->getFileSystem();

			    pProjectUser->upload_url = "";

                pProjectUser->InsertOrUpdateSqliteTable(pFileSystem->GetProjectDataPath());

		    }

		    if(SaveToMainDBAfterUpload(job, "infor") == true)
		    {
                Editor::DataManager* pDataManager = Editor::DataManager::getInstance();
        
                Editor::DataSource* pDataSource = pDataManager->getDataSource(InfoTableName);
        
                if (NULL==pDataSource)
                {
                    Locker::UnLock();
                    return;
                }
        
                std::string dataPath= pDataSource->GetDataSourcePath();
        
                if(strcmp(dataPath.c_str(),"") == 0)
                {
                    Locker::UnLock();
                    return;
                }
        
                if(strcmp(pDataSource->GetConnectionString().c_str(),"") == 0)
                {
                    pDataSource->Open(dataPath);
                }
        
                Editor::DataLayer* layer = pDataSource->GetLayerByName(InfoTableName);
                
                int infoType=1;
                if (job->GetJobType()==POI_INFO)
                {
                    infoType=1;
                }
                else if(job->GetJobType()==ROAD_INFO)
                {
                    infoType=2;
                }
                

                std::string sql = "update edit_infos set submitStatus = 1 where c_isAdopted!=0 and h_projectId='"+job->m_sFoundFlag+"' and b_sourceCode="+Tools::NumberToString(infoType)+";";

                if(layer->ExcuteSql(sql) !=0)
                {
                    Logger::LogO("update edit_infos failed!!!");

                    if(job->GetPregressObj() != NULL)
                    {
                        job->GetPregressObj()->OnProgress(Tools::GenerateProgressInfo(job->m_sFoundFlag, "上传后更新失败"),-1);
                    }
                }              
		    }
            
            m_nSuccessChunkNum = 0;
        }

        Locker::UnLock();
    }

    int MapDataPackager::ParsePoiUploadResponse(std::string sIn, std::string& errmsg)
    {
        rapidjson::Document doc;

        doc.Parse<0>(sIn.c_str());

        if(doc.HasParseError())
        {
            return -1;
        }

        if(doc.HasMember("data"))
        {
            rapidjson::Value& dataValue = doc["data"];

            if(dataValue.HasMember("failedDetail"))
            {
                rapidjson::Value& failDetailValue = dataValue["failedDetail"];

                if(failDetailValue.IsArray())
                {
                    for(rapidjson::SizeType i = 0; i < failDetailValue.Size(); i++)
                    {
                        rapidjson::Value& poiValue = failDetailValue[i];

                        int failType = poiValue["reason"].GetInt();

                        std::string reason = GetFailedReasonByType(failType);

                        std::string name = poiValue["name"].GetString();

                        std::string fid = poiValue["fid"].GetString();

                        // 打印未能入库的poi信息, 后续可能需要将这些未能入库的信息保存在本地

                        Logger::LogD("fail to put into storage: fid [%s], name [%s], reason [%]", fid.c_str(), name.c_str(), reason.c_str());
				        Logger::LogO("fail to put into storage: fid [%s], name [%s], reason [%]", fid.c_str(), name.c_str(), reason.c_str());
                    }
                }
            }
        }
		return 0;
    }

    std::string MapDataPackager::GetFailedReasonByType(int type)
    {
        switch(type)
        {
        case 1:
            return "数据库正在被其他进程使用，已被加锁";
        case 2:
            return "库中数据已删除，不能重复删除";
        case 3:
            return "库中没有该数据，不能修改";
        case 4:
            return "lifecycle不是1，2，3";
        case 5:
            return "数据错误";
        case 6:
            return "旋转照片出错";
        default:
            return "未知错误";
        }
    }

    void MapDataPackager::Stop()
    {
        m_pDataUploader->Cancel();
    }
    
    void MapDataPackager::RunTips(JOBCMD cmd, ProgressEventer* progress)
    {
    	if (0 == m_vGridId.size())
    	{
    		std::string description = Tools::GenerateProgressInfo("tips数据", "没有选择需要上传的grid");

    		progress->OnProgress(description, 200);

    		return;
    	}

        if(ProjectManager::getInstance()->MatchVersion(TIPSVersion) == false)
        {
            Logger::LogD("TIPSVersion : ModelVersion is error, please check!!!");
            Logger::LogO("TIPSVersion : ModelVersion is error, please check!!!");
            std::string sFlag = GetGridFlag();
            std::string description = Tools::GernerateTipsProgressInfo(sFlag,"上传", "TIPSVersion : ModelVersion is error, please check!!!");
            progress->OnProgress(description, -1);
            return;
        }
		
        if(ProjectManager::getInstance()->MatchVersion(IMAGEVersion) == false)
        {
            Logger::LogD("IMAGEVersion : ModelVersion is error, please check!!!");
            Logger::LogO("IMAGEVersion : ModelVersion is error, please check!!!");
            std::string sFlag = GetGridFlag();
            std::string description = Tools::GernerateTipsProgressInfo(sFlag,"上传", "TIPSVersion : ModelVersion is error, please check!!!");
			progress->OnProgress(description, -1);
            return;
        }
        Grid grid;
        
        GridHelper* gridHelper = GridHelper::getInstance();

        std::string wkt = grid.GridsToLocation(m_vGridId);
        
        gridHelper->CreateOrOpenDatabase();

        std::string timestamp = gridHelper->GetGridsTimeStamp(m_vGridId, TIPS_UP);
        
        int dataCount = -1;
        
        std::set<std::string> failedFeedbacks;
        
        CoreMapDataSource* pSource = dynamic_cast<CoreMapDataSource*>(DataManager::getInstance()->getDataSource(DATASOURCE_COREMAP));
        if (NULL==pSource)
        {
            return;
        }
        
        std::string zipPath = pSource->PackageTips(m_sUserId, wkt, timestamp, dataCount, failedFeedbacks, progress);
        
        if (zipPath == "")
        {
        	if (0 == dataCount)
        	{
                std::string sFlag = GetGridFlag();
                std::string description = Tools::GernerateTipsProgressInfo(sFlag, "上传", "没有需要上传的数据");

        		progress->OnProgress(description, 200);
        	}
        	else
        	{
                std::string sFlag = GetGridFlag();
        		std::string description = Tools::GernerateTipsProgressInfo(sFlag, "上传", "抽取tips数据失败");

        		progress->OnProgress(description, -1);
        	}

            gridHelper->Close();

    		return;
        }
        
        gridHelper->Close();

        int len = zipPath.length();
        
        size_t pos = zipPath.find_last_of('/');
        
        std::string zipName = zipPath.substr(pos+1, len-pos-1);
        
        unsigned long fileSize;
        
        int chunkSize;
        
        int chunkCount;
        
        std::string param = GetTipsUploadStartParameter(zipPath.c_str(), zipName.c_str(), fileSize, chunkSize, chunkCount);
        
        m_pDataUploader->SetUploadFileInfo(zipName.c_str(), fileSize*1.0, chunkCount);
        
        std::string url = DataManager::getInstance()->getServiceAddress()->GetTipsUploadStartUrlRoot() + param;
        
        std::string response;
        
        Logger::LogD("url:%s", url.c_str());
        
        bool ret = Tools::HttpGet(url.c_str(), response);
        
        if (ret == false)
        {
            return;
        }
        
        Logger::LogD("response:%s", response.c_str());
        
        int jobId = -1;
        
        std::string errmsg;
        
        int errcode = ParseTipsUploadStartResponse(response, jobId, errmsg);
        
        if (jobId == -1)
        {
            return;
        }
        
        Logger::LogD("jobId:%d", jobId);
        
        for (int chunkNo=0; chunkNo<chunkCount; chunkNo++)
        {
            Editor::UploadJob* job = new Editor::UploadJob();
            
            job->SetUploadTarget(zipPath.c_str());
            
            job->SetUploadFileName(zipName.c_str());
            
            job->SetUploadJobId(jobId);
            
            job->SetUploadChunk(chunkNo, chunkSize);
            
            std::string param = GetTipsUploadChunkParameter(jobId, chunkNo);
            
            job->SetUploadParam(param.c_str());

            job->m_sWkt = wkt;
            
            m_pDataUploader->StartJob(job);
        }
    }
    
    void MapDataPackager::ConfirmUploadTips(UploadJob* job)
    {
        int lastTryCount;
        
        std::string param = GetTipsUploadCheckParameter(job->GetUploadJobId());
        
        //lee add
        CURL *curl = curl_easy_init();
        if(curl) {
            printf("param before Encoded: %s\n", param.c_str());
            char *output = curl_easy_escape(curl, param.c_str(), 0);
            param = output;
            if(output) {
                printf("Encoded: %s\n", output);
                curl_free(output);
            }
        }
        curl_easy_cleanup(curl);

        std::string url = DataManager::getInstance()->getServiceAddress()->GetTipsUploadCheckUrlRoot() + param;
        
        std::string response;
        bool ret = Tools::HttpGet(url.c_str(), response);
        
        if (ret == false)
        {
            AddUploadFileTryCount(job->GetUploadFileName().c_str(), lastTryCount);
            
            if (lastTryCount >= m_MaxUploadFileTryCount)
            {
                Logger::LogD("file [%s] failed!", job->GetUploadFileName().c_str());

                std::string sFlag = GetGridFlag();
        		std::string description = Tools::GernerateTipsProgressInfo(sFlag, "上传", response);

        		m_Progress->OnProgress(description, -1);
            }
            else
            {
                Logger::LogD("try [%d] failed!", lastTryCount);
                
                this->ConfirmUploadTips(job);
            }
            
            return;
        }
        
        std::set<int> finishedChunkNo;
        
        std::string errmsg;
        
        int errcode = ParseTipsUploadCheckResponse(response, finishedChunkNo, errmsg);
        
        if (0 != errcode)
        {
            AddUploadFileTryCount(job->GetUploadFileName().c_str(), lastTryCount);
            
            if (lastTryCount >= m_MaxUploadFileTryCount)
            {
                Logger::LogD("file [%s] failed!", job->GetUploadFileName().c_str());

                std::string sFlag = GetGridFlag();
        		std::string description = Tools::GernerateTipsProgressInfo(sFlag, "上传", response);

        		m_Progress->OnProgress(description, -1);
            }
            else
            {
                Logger::LogD("try [%d] failed!", lastTryCount);
                
                this->ConfirmUploadTips(job);
            }
            
            return;
        }
        
        int totalChunkCount = m_pDataUploader->GetUploadFileChunkCount(job->GetUploadFileName().c_str());
        
        int finishedChunkCount = finishedChunkNo.size();
        
        if (finishedChunkCount == totalChunkCount)
        {
            std::string url_finish = DataManager::getInstance()->getServiceAddress()->GetTipsUploadFinishUrlRoot() + param;
            
            std::string response_finish;
            bool ret_finish = Tools::HttpGet(url_finish.c_str(), response_finish);
            
            if (ret_finish == false)
            {
                AddUploadFileTryCount(job->GetUploadFileName().c_str(), lastTryCount);
                
                if (lastTryCount >= m_MaxUploadFileTryCount)
                {
                    Logger::LogD("file [%s] failed!", job->GetUploadFileName().c_str());

                    std::string sFlag = GetGridFlag();
                    std::string description = Tools::GernerateTipsProgressInfo(sFlag, "上传", response_finish);

            		m_Progress->OnProgress(description, -1);
                }
                else
                {
                    Logger::LogD("try [%d] failed!", lastTryCount);
                    
                    this->ConfirmUploadTips(job);
                }
            }
            else
            {
                std::string errmsg;
                int errcode = ParseTipsUploadFinishResponse(response_finish, errmsg);
                
                if (errcode != 0)
                {
                    AddUploadFileTryCount(job->GetUploadFileName().c_str(), lastTryCount);
                    
                    if (lastTryCount >= m_MaxUploadFileTryCount)
                    {
                        Logger::LogD("file [%s] failed!", job->GetUploadFileName().c_str());

                        std::string sFlag = GetGridFlag();
                        std::string description = Tools::GernerateTipsProgressInfo(sFlag, "上传", response_finish);

                		m_Progress->OnProgress(description, -1);
                    }
                    else
                    {
                        Logger::LogD("try [%d] failed!", lastTryCount);
                        
                        this->ConfirmUploadTips(job);
                    }
                }
                else
                {
                    Logger::LogD("file [%s] finished!", job->GetUploadFileName().c_str());

                    std::string url_import = DataManager::getInstance()->getServiceAddress()->GetTipsUploadImportUrlRoot() + param;
                    Logger::LogD("url:%s", url_import.c_str());
                    Logger::LogO("url:%s", url_import.c_str());
                    std::string response_import;
                    bool ret_import = Tools::HttpGet(url_import.c_str(), response_import);

                    if (ret_import == false)
                    {
                        AddUploadFileTryCount(job->GetUploadFileName().c_str(), lastTryCount);

                        if (lastTryCount >= m_MaxUploadFileTryCount)
                        {
                            Logger::LogD("import [%s] failed!", job->GetUploadFileName().c_str());

                            std::string importError = ParseTipsImportResponse(response_import);
                            std::string sFlag = GetGridFlag();
                            std::string description = Tools::GernerateTipsProgressInfo(sFlag, "tips入库", importError, true);

                    		m_Progress->OnProgress(description, -1);
                        }
                        else
						{
                        	Logger::LogD("import try [%d] failed!", lastTryCount);

                        	this->ConfirmUploadTips(job);
						}
                    }
                    else
                    {
                        std::string errmsg;
                        int errcode = ParseTipsUploadFinishResponse(response_import, errmsg);

                        if (errcode != 0)
                        {
                            AddUploadFileTryCount(job->GetUploadFileName().c_str(), lastTryCount);

                            if (lastTryCount >= m_MaxUploadFileTryCount)
                            {
                                Logger::LogD("import file [%s] failed!", job->GetUploadFileName().c_str());

                                std::string importError = ParseTipsImportResponse(response_import);
								std::string sFlag = GetGridFlag();
                                std::string description = Tools::GernerateTipsProgressInfo(sFlag, "tips入库", importError, true);

                        		m_Progress->OnProgress(description, -1);
                            }
                            else
                            {
                                Logger::LogD("import try [%d] failed!", lastTryCount);

                                this->ConfirmUploadTips(job);
                            }
                        }
                        else
                        {
                        	Logger::LogD("import file [%s] finished!", job->GetUploadFileName().c_str());

                        	GridHelper* gridHelper = GridHelper::getInstance();

                        	gridHelper->CreateOrOpenDatabase();

                            std::string fileName = job->GetUploadFileName();
                            
                            int fileNameLen = fileName.length();
                            
                        	remove(job->GetUploadTarget().c_str());

                            std::string importSuccess = ParseTipsImportResponse(response_import);
                            std::string sFlag = GetGridFlag();
                            std::string description = Tools::GernerateTipsProgressInfo(sFlag, "tips入库", importSuccess, true);
                            

                    		m_Progress->OnProgress(description, 100);
                            
                    		int nFlag = 200;

                    		std::string sDescription;
                            if(UpdateDatabaseAfterTipsUpload(job->m_sWkt) == false)
                            {
                                Logger::LogD("update edit_tips after upload tips unsuccessfully");
                                Logger::LogO("update edit_tips after upload tips unsuccessfully");

                                nFlag = -1;
                                std::string sFlag = GetGridFlag();
                                sDescription = Tools::GernerateTipsProgressInfo(sFlag, "本地更新", "上传之后更新本地库失败");
                                


                            }
                            if(UpdateDatabaseAfterImageUpload() == false)
                            {
								Logger::LogD("update meta_JVImage after upload tips unsuccessfully");
								Logger::LogO("update meta_JVImage after upload tips unsuccessfully");

								nFlag = -1;
								std::string sFlag = GetGridFlag();
								sDescription = Tools::GernerateTipsProgressInfo(sFlag, "本地更新", "上传之后更新本地库失败");
  
							}
                            
                            if(nFlag == -1)
                            {
                            	m_Progress->OnProgress(sDescription, -1);
                            }
                            else
                            {
                            sFlag = GetGridFlag();
							
                            description = Tools::GernerateTipsProgressInfo(sFlag, "本地更新", "success");

                               m_Progress->OnProgress(description, 200);
                            }
                            
                            gridHelper->Close();

                        }
                    }
                }
            }
            
            return;
        }
        else
        {
            AddUploadFileTryCount(job->GetUploadFileName().c_str(), lastTryCount);
            
            if (lastTryCount >= m_MaxUploadFileTryCount)
            {
                Logger::LogD("file [%s] failed!", job->GetUploadFileName().c_str());
                std::string sFlag = GetGridFlag();
                
                std::string description = Tools::GernerateTipsProgressInfo(sFlag, "上传", "tips数据上传失败, 原因：" + response);

        		m_Progress->OnProgress(description, -1);

                return;
            }
            
            Logger::LogD("try [%d] ...", lastTryCount);
            
            m_pDataUploader->RefreshJobStatusByFile(job->GetUploadFileName().c_str(), finishedChunkNo);
            
            std::set<int> unfinishedChunkNo;
            
            std::set<int> totalChunkNo;
            
            for (int i=0; i<totalChunkCount; i++)
            {
                totalChunkNo.insert(i);
            }
            
            std::set_difference(totalChunkNo.begin(), totalChunkNo.end(), finishedChunkNo.begin(), finishedChunkNo.end(), std::inserter(unfinishedChunkNo, unfinishedChunkNo.begin()));
            
            std::set<int>::const_iterator itor = unfinishedChunkNo.begin();
            
            for (; itor!=unfinishedChunkNo.end(); itor++)
            {
                int chunkNo = *itor;
                
                Editor::UploadJob* newJob = new Editor::UploadJob();
                
                newJob->SetUploadTarget(job->GetUploadTarget().c_str());
                
                newJob->SetUploadFileName(job->GetUploadFileName().c_str());
                
                newJob->SetUploadJobId(job->GetUploadJobId());
                
                newJob->SetUploadChunk(chunkNo, job->GetUploadChunkSize());
                
                std::string param = GetTipsUploadChunkParameter(job->GetUploadJobId(), chunkNo);
                
                newJob->SetUploadParam(param.c_str());
                
                m_pDataUploader->StartJob(newJob);
                
                Logger::LogD("new job [%d:%d]", job->GetUploadJobId(), chunkNo);
            }
        }
    }
    
    std::string	MapDataPackager::ParseTipsImportResponse(const std::string& sResponse)
    {
    	if (sResponse.empty())
		{
			return "";
		}
		rapidjson::Document document;
		document.Parse(sResponse.c_str());
		if (!document.HasParseError())
		{
			if(document.HasMember("data"))
			{
				rapidjson::Value& value = document["data"];
				rapidjson::StringBuffer buffer;
				rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
				value.Accept(writer);
				return buffer.GetString();
			}
		}
		return "";
    }
    std::string MapDataPackager::GetTipsUploadStartParameter(const char* target, const char* fileName, unsigned long& fileSize, int& chunkSize, int& chunkCount)
    {
        ifstream file(target);
        
        MD5 md5Context(file);
        
        fileSize = Tools::GetFileSize(target);
        
        chunkSize = fileSize / 10;
        
        if (chunkSize > m_MaxChunkSize)
        {
            chunkSize = m_MaxChunkSize;
        }
        

        ldiv_t div_result = ::ldiv(fileSize, chunkSize);

        chunkCount = div_result.quot;

        if (div_result.rem != 0)
        {
        	chunkCount++;
        }

        rapidjson::Document document;
        
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
        
        document.SetObject();
        
        document.AddMember("fileName", rapidjson::StringRef(fileName), allocator);
        
        document.AddMember("md5", rapidjson::StringRef(md5Context.hex_digest()), allocator);
        
        document.AddMember("fileSize", (uint64_t)fileSize, allocator);
        
        document.AddMember("chunkSize", (uint64_t)chunkSize, allocator);
        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        std::string res = buffer.GetString();
        
        return res;
    }
    
    int MapDataPackager::ParseTipsUploadStartResponse(std::string response, int& jobId, std::string& errmsg)
    {
        int errcode = -1;
        
        if (response.empty())
        {
            return errcode;
        }
        
        rapidjson::Document document;
        
        document.Parse(response.c_str());
        
        if (document.HasParseError())
        {
            int code = document.GetParseError();
            
            size_t offset = document.GetErrorOffset();
            
            Logger::LogD("ParseTipsUploadStartResponse failed! response [%s], error [%d][%d]", response.c_str(), code, offset);
            Logger::LogO("ParseTipsUploadStartResponse failed! response [%s], error [%d][%d]", response.c_str(), code, offset);
            
            return errcode;
        }
        
        errcode = document["errcode"].GetInt();
        
        if (errcode == 0
            && document.HasMember("data")
            && document["data"].IsInt())
        {
            jobId = document["data"].GetInt();
        }
        else
        {
            errmsg = document["errmsg"].GetString();
        }
        
        return errcode;
    }
    
    std::string MapDataPackager::GetTipsUploadChunkParameter(int jobId, int chunkNo)
    {
        Document document;
        
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
        
        document.SetObject();
        
        document.AddMember("jobId", jobId, allocator);
        
        document.AddMember("chunkNo", chunkNo, allocator);
        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        std::string res = buffer.GetString();
        
        return res;
    }
    
    std::string MapDataPackager::GetTipsUploadCheckParameter(int jobId)
    {
       	Document document;
        
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
        
        document.SetObject();
        
        document.AddMember("jobId", jobId, allocator);
        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        std::string res = buffer.GetString();
        
        return res;
    }
    
    int MapDataPackager::ParseTipsUploadCheckResponse(std::string response, std::set<int>& finishedChunkNo, std::string& errmsg)
    {
        int errcode = -1;
        
        if (response.empty())
        {
            return errcode;
        }
        
        rapidjson::Document document;
        
        document.Parse(response.c_str());
        
        if (document.HasParseError())
        {
            int code = document.GetParseError();
            
            size_t offset = document.GetErrorOffset();
            
            Logger::LogD("ParseTipsUploadCheckResponse failed! response [%s], error [%d][%d]", response.c_str(), code, offset);
            Logger::LogO("ParseTipsUploadCheckResponse failed! response [%s], error [%d][%d]", response.c_str(), code, offset);
            
            return errcode;
        }
        
        errcode = document["errcode"].GetInt();
        
        if (errcode == 0)
        {
            const rapidjson::Value& data = document["data"];
            
            if (data.IsArray() == false)
            {
                errcode = -1;
            }
            else
            {
                for (rapidjson::SizeType i=0; i<data.Size(); i++)
                {
                    finishedChunkNo.insert(data[i].GetInt());
                }
            }
        }
        else
        {
            errmsg = document["errmsg"].GetString();
        }
        
        return errcode;
    }
    
    int MapDataPackager::ParseTipsUploadFinishResponse(std::string response, std::string& errmsg)
    {
        int errcode = -1;
        
        if (response.empty())
        {
            return errcode;
        }
        
        rapidjson::Document document;
        
        document.Parse(response.c_str());
        
        if (document.HasParseError())
        {
            int code = document.GetParseError();
            
            size_t offset = document.GetErrorOffset();
            
            Logger::LogD("ParseTipsUploadFinishResponse failed! response [%s], error [%d][%d]", response.c_str(), code, offset);
            Logger::LogO("ParseTipsUploadFinishResponse failed! response [%s], error [%d][%d]", response.c_str(), code, offset);
            
            return errcode;
        }
        
        errcode = document["errcode"].GetInt();
        
        if (errcode != 0)
        {
            if(document.HasMember("errmsg") && document["errmsg"].IsString())
            {
                errmsg = document["errmsg"].GetString();
            }
            else
            {
                errmsg = "";
            }
            
        }
        
        return errcode;
    }
    
    void MapDataPackager::AddUploadFileTryCount(const char* fileName, int& lastTryCount)
    {
        m_Mutex.lock();
        
        std::map<std::string, int>::iterator itor = m_UploadFileTryCount.find(fileName);
        
        if (itor != m_UploadFileTryCount.end())
        {
            lastTryCount = itor->second;
            
            itor->second = lastTryCount + 1;
        }
        else
        {
            lastTryCount = 0;
            
            m_UploadFileTryCount.insert(std::make_pair(fileName, 1));
        }
        
        m_Mutex.unlock();
    }

    void MapDataPackager::SetFileSystem(DataFileSystem* pFileSystem)
    {
        m_pDataFileSystem = pFileSystem;
    }

    std::string MapDataPackager::GetUrl(Model::ProjectUser* modelPU, std::string filename)
    {
        if(modelPU->nType == 1)
        {
            return "http://fs.navinfo.com/fos/editsupportTest/upload/?filename=" + filename + "&projectId=" + modelPU->sProjectId;
        }
        else
        {
            return "http://fs.navinfo.com/fos/editsupportTest/upload/?filename=" + filename + "&projectId=" + modelPU->sProjectId;
        }
    }

    std::string MapDataPackager::GetUploadFile(std::string& sFilename, std::string sLastTime, std::string& errmsg, const std::string& sProjectId, int& total, int& add, int& upd, int& del, int type)
    {
        std::string sFilePath = GenerateUploadFile(sFilename, sLastTime, errmsg, sProjectId, total, add, upd, del, type);

        if(sFilePath.empty() || sFilename.empty())
        {

            return "";
        }

        int nBeg = sFilePath.find_last_of(sFilename);

        if(nBeg == std::string::npos)
        {
            return "";
        }

        std::string sFolderPath = m_pDataFileSystem->GetPoiUploadPath();

        if(type == POI)
            sFilename = "IncrementalData_" + m_sUserId + "_" + sProjectId + "_" + Tools::GetCurrentDateTime() + ".zip";
        if(type == POI_CHARGING)
            sFilename = "ChargingData_" + m_sUserId + "_" + Tools::GetCurrentDateTime() + ".zip";

        std::string sZipName = sFolderPath + sFilename;

        std::string zipFolder = Tools::ZipFolderPath(sFolderPath.c_str(), sZipName.c_str());

        return zipFolder;
    }
    
	std::string MapDataPackager::GetUploadInfoFile(std::string& sFilename, std::string& errmsg, const std::string& sPackageSql,const std::string& sProjectId,int type)
    {
        std::string sFilePath = GenerateUploadFile(sFilename, errmsg, sPackageSql, type);

        if(sFilePath.empty() || sFilename.empty())
        {

            return "";
        }

        int nBeg = sFilePath.find_last_of(sFilename);

        if(nBeg == std::string::npos)
        {
            return "";
        }

        std::string sFolderPath = m_pDataFileSystem->GetPoiUploadPath();
        if(type == POI_INFO)
            sFilename = "InforData_" + m_sUserId + "_" + sProjectId + "_" + Tools::GetCurrentDateTime() + ".zip";
        else
            sFilename = "InforReportData_" + m_sUserId + "_" + Tools::GetCurrentDateTime() + ".zip";

        std::string sZipName = sFolderPath + sFilename;

        std::string zipFolder = Tools::ZipFolderPath(sFolderPath.c_str(), sZipName.c_str());

        return zipFolder;
    }
    std::string MapDataPackager::GenerateUploadFile(std::string& sFilename, std::string sLastTime, std::string& errmsg, const std::string& sProjectId, int& total, int& add, int& upd, int& del, int type)
    {
        std::string sData = m_pDataFileSystem->GetCoreMapDataPath();

        std::string sTimeTmp = Tools::GetCurrentDateTime();

        if(type == POI)
            sFilename = "Datum_Point.json";
        if(type == POI_CHARGING)
            sFilename = "Datum_Charging.json";

        std::string sPath = m_pDataFileSystem->GetPoiUploadPath() + sFilename;

        if(Tools::CheckFilePath(sPath.c_str()) == false)
        {
            return "";
        }

        std::ofstream out;

        out.open(sPath.c_str());

        if(!out)
        {
            Logger::LogD("open writefile error");
            Logger::LogO("open writefile error");
            errmsg = "open writefile error";

            return "";
        }

        Editor::DataManager* dataManager = Editor::DataManager::getInstance();
        
        Editor::DataFunctor* dataFunctor = dataManager->getDataFunctor();
        
        std::string sql;
        if(type == POI)
            sql = "select * from edit_pois where lifecycle !=0 and project = '" + sProjectId + "' and type = 0";
        if(type == POI_CHARGING)
            sql = "select * from edit_pois where lifecycle !=0 and project = '" + sProjectId + "' and type = 2";
        
        std::vector<Editor::DataFeature*> pois = dataFunctor->QueryFeaturesBySql("edit_pois", sql);
        
        int nLineCount = 0;
        
        for(int i=0; i< pois.size(); i++)
        {
            FieldDefines* fd = pois.at(i)->GetFieldDefines();
            
            if(NULL== fd)
            {
                continue;
            }
            
            int index = fd->GetColumnIndex(Poi_edits);
            
            if(index == -1)
            {
                continue;
            }
            
            unsigned char* blob = NULL;
            unsigned int length;
            
            blob = pois.at(i)->GetAsBlob(index, length);
            
            if (NULL==blob)
            {
                Logger::LogD("MapDataPackager::GenerateUploadFile ，edits is null");
                Logger::LogO("MapDataPackager::GenerateUploadFile ，edits is null");
                continue;
            }
            
            std::string edits_string(reinterpret_cast<char*>(blob),length);
            
            if(edits_string.empty())
            {
                continue;
            }
            
            Editor::EditAttrs ea;
            
            std::string lastMergeDate;
            
            ea.ParseJsonForExtractionPoi(edits_string, lastMergeDate);
            
            if(lastMergeDate > sLastTime)
            {
                std::string sTmp;
                if(type == POI)
                    sTmp = ((DataFeaturePoi*)(pois.at(i)))->ToUploadJsonStr();
                if(type == POI_CHARGING)
                    sTmp = ((DataFeaturePoi*)(pois.at(i)))->ToUploadChargingJsonStr();
                
                out<<sTmp<<std::endl;
                
                nLineCount++;
                
                total++;

                index = fd->GetColumnIndex(Poi_lifecycle);

                if(index != -1)
                {
                	int nLifecycle = pois.at(i)->GetAsInteger(index);

                	if(nLifecycle == 1)
                	{
                		del++;
                	}
                	else if(nLifecycle == 2)
                	{
                		upd++;
                	}
                	else if(nLifecycle == 3)
                	{
                		add++;
                	}
                }

                index = fd->GetColumnIndex(Poi_attachments);
                
                if(index != -1)
                {
                    blob = pois.at(i)->GetAsBlob(index, length);
                    
                    std::string attachments_string(reinterpret_cast<char*>(blob),length);
                    
                    if(!attachments_string.empty() && attachments_string != "[]")
                    {
                        GnerateUploadPhoto(attachments_string);
                    }
                }
            }

        }
        
        out.close();
        
        if(nLineCount == 0)
        {
            errmsg = "no package data";
            return "";
        }

        return sPath;
    }

    std::string MapDataPackager::GenerateUploadFile(std::string& sFilename, std::string& errmsg, const std::string& packageSql, int type)
    {
        /*std::string sData = m_pDataFileSystem->GetCoreMapDataPath();

        sqlite3* db = NULL;

        int ret = sqlite3_open_v2(sData.c_str(), &db, SQLITE_OPEN_READWRITE, NULL);

        if(ret != SQLITE_OK)
        {
            Logger::LogD("DataSource::sqlite [%s] or [%s] open failed", sData.c_str(), sqlite3_errmsg(db));
            Logger::LogO("DataSource::sqlite [%s] or [%s] open failed", sData.c_str(), sqlite3_errmsg(db));
            errmsg = "open database failed";

            sqlite3_close(db);
            return "";
        }

        sqlite3_stmt* stmt = NULL;

        ret = sqlite3_prepare_v2(db, packageSql.c_str(), -1, &stmt, NULL);

	    if (ret != SQLITE_OK)
	    {
		    Logger::LogD("select %s failed [%s]", "Point", sqlite3_errmsg(db));
            Logger::LogO("select %s failed [%s]", "Point", sqlite3_errmsg(db));
            errmsg = "excute sqlite failed";

		    return "";
	    }

        std::string sTimeTmp = Tools::GetCurrentDateTime();*/

        sFilename = "Datum_Infor.json";

        std::string sPath = m_pDataFileSystem->GetPoiUploadPath() + sFilename;

        if(Tools::CheckFilePath(sPath.c_str()) == false)
        {
            return "";
        }

        std::ofstream out;

        out.open(sPath.c_str());

        if(!out)
        {
            Logger::LogD("open writefile error");
            Logger::LogO("open writefile error");

            errmsg = "open writefile error";
            return "";
        }

        Editor::DataManager* dataManager = Editor::DataManager::getInstance();
        
        Editor::DataFunctor* dataFunctor = dataManager->getDataFunctor();
        
        std::vector<Editor::DataFeature*> infos = dataFunctor->QueryFeaturesBySql("edit_infos", packageSql);

        int nLineCount = 0;

        for(int i = 0; i <infos.size(); i++ )
        {
            std::string sTmp;
            if(type == ROAD_INFO)
                sTmp =  ((DataFeatureInfor*)(infos[i]))->ToUploadRoadJsonStr();  //道路情报
            else
                sTmp =  ((DataFeatureInfor*)(infos[i]))->ToUploadJsonStr();     //poi情报

            out<<sTmp<<std::endl;

            nLineCount++;
        }

        out.close();

        if(nLineCount == 0)
        {
            errmsg = "no package data";
            return "";
        }

        return sPath;
    }

    void MapDataPackager::GnerateUploadPhoto(const std::string& sJosn)
    {
        rapidjson::Document doc;

        doc.Parse<0>(sJosn.c_str());

        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        if(doc.HasParseError())
        {
            return;
        }

        if(!doc.IsArray())
        {
            return;
        }

        std::vector<std::string> vResult;

        for(rapidjson::SizeType i = 0; i < doc.Size(); i++)
        {
            rapidjson::Value& p = doc[i];

            if(p.HasMember("url"))
            {
                rapidjson::Value& v = p["url"];
                
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                v.Accept(writer); 

                std::string sResult = buffer.GetString();
                
                if(sResult.size() > 4)
                {
                    sResult = sResult.substr(1, sResult.size() - 2);
                }

                vResult.push_back(sResult);
            }
        }

        for(int i = 0; i < vResult.size(); i++)
        {
            std::string sSourcePath = m_pDataFileSystem->GetPoiPhotoPath() + vResult[i];

#ifdef WIN32
            std::string sDest = m_pDataFileSystem->GetPoiUploadPath() + "photo\\" + GeneratePhotoName(sSourcePath);
#else
            std::string sDest = m_pDataFileSystem->GetPoiUploadPath() + "photo/" + GeneratePhotoName(sSourcePath);
#endif

            if(Tools::CheckFilePath(sDest.c_str()) == false)
            {
                Logger::LogD("filepath [%s] create falied", sSourcePath.c_str());
                Logger::LogO("filepath [%s] create falied", sSourcePath.c_str());

                return;
            }

            int ret = Tools::CopyBinaryFile(sSourcePath.c_str(), sDest.c_str());

            if(ret == 1)
            {
                Logger::LogD("filepath [%s] not exists", sSourcePath.c_str());
                Logger::LogO("filepath [%s] not exists", sSourcePath.c_str());

                return;
            }
            else if(ret == 2)
            {
                Logger::LogD("filepath [%s] not exists", sDest.c_str());
                Logger::LogO("filepath [%s] not exists", sDest.c_str());

                return;
            }
        }
    }

    int MapDataPackager::GetPUFromDatabase(std::string sProjectId, int nType, Model::ProjectUser* PU)
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

        std::string sql = "select * from project_user where project_id=" + sProjectId + " and user_id=" + m_sUserId + " and type="+ Tools::NumberToString(nType);

        sqlite3_stmt* stmt;

        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("select %s failed [%s]", "Point", sqlite3_errmsg(db));
            Logger::LogO("select %s failed [%s]", "Point", sqlite3_errmsg(db));

		    return -1;
	    }


        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            PU->ParseSqlite(stmt);
        }
        else if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);

            sqlite3_close(db);
            return 1;
        }
        sqlite3_finalize(stmt);

        sqlite3_close(db);

        return 0;
    }

    bool MapDataPackager::UploadPoiPhotoToLocal(const std::string& sProject, const std::string& sFid, std::string& source)
    {
        std::string sPathDest = m_pDataFileSystem->GetPoiPhotoPath();

        std::string dateStr = Tools::GetCurrentDate();

	    int nBeg = dateStr.find('-');

	    while(nBeg != std::string::npos)
	    {
		    dateStr = dateStr.substr(0, nBeg) + dateStr.substr(nBeg + 1);

		    nBeg = dateStr.find('-');
	    }

        std::string sDateValue = "";
        
#ifdef WIN32
        sDateValue =  sProject + "\\" + dateStr + "\\" + GeneratePhotoName(source);
#else
        sDateValue =  sProject + "/" + dateStr + "\\" +  GeneratePhotoName(source);
#endif

        sPathDest += sDateValue;

        int ret = Tools::CopyBinaryFile(source.c_str(), sPathDest.c_str());

        if(ret == 1)
        {
            Logger::LogD("filepath [%s] not exists", source.c_str());
            Logger::LogO("filepath [%s] not exists", source.c_str());

            return false;
        }
        else if(ret == 2)
        {
            Logger::LogD("filepath [%s] not exists", sPathDest.c_str());
            Logger::LogO("filepath [%s] not exists", sPathDest.c_str());

            return false;
        }

        Locker::Lock();


        if(UpdatePhoto(sFid, sDateValue) == false)
        {
            Logger::LogD("update poi attachments failed");
            Logger::LogO("update poi attachments failed");

            return false;
        }

        Locker::UnLock();

        return true;
    }

    void MapDataPackager::UploadPoiPhotoToServer(ProgressEventer* progress)
    {
        std::string sFilename = Tools::GetCurrentDateTime() + ".zip";

#ifdef WIN32
        std::string sPath = m_pDataFileSystem->GetPoiPhotoPath() + "\\" + sFilename;
#else
        std::string sPath = m_pDataFileSystem->GetPoiPhotoPath() + "/" + sFilename;
#endif
        std::string zipFolder = Tools::ZipFolderPath(m_pDataFileSystem->GetPoiPhotoPath().c_str(), sPath.c_str());

        Model::ProjectUser* PU = new Model::ProjectUser();

        Editor::Job* job = new Editor::Job();

        job->SetUrl("http://192.168.4.130/TestUpload/servlet/fileServlet");

        job->SetFileName(zipFolder);

        job->SetProjectUser(PU);

//        m_pDataUploader->StartJob(job, NULL, progress);
    }

    bool MapDataPackager::UpdatePhoto(const std::string& sFid, std::string sValue)
    {
        std::string sDataPath = m_pDataFileSystem->GetCoreMapDataPath();

        sqlite3* db = NULL;

        int ret = sqlite3_open_v2(sDataPath.c_str(), &db, SQLITE_OPEN_READWRITE, NULL);

        if(ret != SQLITE_OK)
        {
            Logger::LogD("DataSource::sqlite [%s] or [%s] open failed", sDataPath.c_str(), sqlite3_errmsg(db));
            Logger::LogO("DataSource::sqlite [%s] or [%s] open failed", sDataPath.c_str(), sqlite3_errmsg(db));

            sqlite3_close(db);
            return false;
        }

        std::string sql = "select * from point where fid = '" + sFid + "'";

        sqlite3_stmt* stmt;

        ret = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (ret != SQLITE_OK)
	    {
		    Logger::LogD("select %s failed [%s]", "Point", sqlite3_errmsg(db));
            Logger::LogO("select %s failed [%s]", "Point", sqlite3_errmsg(db));

		    return false;
	    }

        Model::Point* point = new Model::Point();

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            point->ParseSqlite(stmt);
        }

        rapidjson::Document doc;

        doc.Parse<0>((point->attachments).c_str());

        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        if(doc.HasParseError())
        {
            return -1;
        }

        rapidjson::Value object(rapidjson::kObjectType);

        rapidjson::Value each_json_value(kStringType); 
        each_json_value.SetString(sValue.c_str(), sValue.size(),allocator); 

        object.AddMember("url",each_json_value, allocator);
        object.AddMember("tag", 1, allocator);
        object.AddMember("type", 1, allocator);

        doc.PushBack(object, allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        point->attachments = buffer.GetString();

        point->UpdateSqliteTable(db);

        sqlite3_finalize(stmt);

        sqlite3_close(db);

        delete point;

        return true;
    }

    std::string MapDataPackager::GeneratePhotoName(const std::string& source)
    {
        std::string sName = "";

#ifdef WIN32
        int nBeg = source.find_last_of("\\");
#else
        int nBeg = source.find_last_of("/");
#endif

        if(nBeg != std::string::npos)
        {
        	sName = source.substr(nBeg + 1);
        }
        else
        {
            return "";
        }

        return sName;
    }

    int MapDataPackager::CheckLastUploadIsCorrect()
    {
        // 由于当前业务上对这里的处理还不是很明确，暂不处理
        std::string poiUploadPath = m_pDataFileSystem->GetPoiUploadPath();

        Tools::DelDir(poiUploadPath.c_str());

        return 0;
    }

    bool MapDataPackager::UploadSnapShot(const char* target, const char* fileName, const char* token, unsigned long& fileSize, int& chunkCount, int type)
    {
        ifstream file(target, ios::binary);
        
        MD5 md5Context(file);
        
        fileSize = Tools::GetFileSize(target);       

        if(fileSize > m_MaxChunkSize)
        {
            ldiv_t div_result = ::ldiv(fileSize, m_MaxChunkSize);

            chunkCount = div_result.quot;

            if (div_result.rem != 0)
            {
        	    chunkCount++;
            }
        }
        else
        {
            chunkCount = 1;
        }

        std::string sSnapshot = Editor::DataManager::getInstance()->getServiceAddress()->GetPoiUploadSnapShot(token, fileName, md5Context.hex_digest(), fileSize, type);

        std::string sResponse;

        if(Tools::HttpGet(sSnapshot.c_str(), sResponse) == false)
        {
            Logger::LogD("snapshot failed : [%s]", sSnapshot.c_str());
			Logger::LogO("snapshot failed : [%s]", sSnapshot.c_str());

            return false;
        }

        rapidjson::Document document;

        document.Parse<0>(sResponse.c_str());

        if(document.HasParseError())
        {
            Logger::LogD("falied to parse the response of snapshot : [%s]", sResponse.c_str());
            Logger::LogO("falied to parse the response of snapshot : [%s]", sResponse.c_str());

            return false;
        }

        int nErrCode = -1;

        if(document.HasMember("errcode"))
        {
            nErrCode = document["errcode"].GetInt();
        }
        
        if(nErrCode != 0)
        {
            std::string errmsg = "";

            if(document.HasMember("errmsg"))
            {
                errmsg = document["errmsg"].GetString();
            }

            Logger::LogD("result of snapshot express that you cannot upload because of [%s]", errmsg.c_str());
            Logger::LogO("result of snapshot express that you cannot upload because of [%s]", errmsg.c_str());

            return false;
        }

        return true;
    }

    bool MapDataPackager::UploadCheck(const char* token, const char* sFilename, int type)
    {
        std::string sCheck = Editor::DataManager::getInstance()->getServiceAddress()->GetPoiUploadCheck(token, sFilename, type);

        std::string sResponse;

        if(Tools::HttpGet(sCheck.c_str(), sResponse) == false)
        {
            Logger::LogD("check failed : [%s]", sCheck.c_str());
			Logger::LogO("check failed : [%s]", sCheck.c_str());

            return false;
        }

        Logger::LogD("the response of check : [%s]", sResponse.c_str());
		Logger::LogO("the response of check : [%s]", sResponse.c_str());

        rapidjson::Document document;

        document.Parse<0>(sResponse.c_str());

        if(document.HasParseError())
        {
            Logger::LogD("falied to parse the response of check : [%s]", sResponse.c_str());
            Logger::LogO("falied to parse the response of check : [%s]", sResponse.c_str());

            return false;
        }

        int nErrCode = -1;

        if(document.HasMember("errcode"))
        {
            nErrCode = document["errcode"].GetInt();
        }
        
        if(nErrCode != 0)
        {
            std::string errmsg = "";

            if(document.HasMember("errmsg"))
            {
                errmsg = document["errmsg"].GetString();
            }

            Logger::LogD("result of check express that you cannot upload because of []", errmsg.c_str());
            Logger::LogO("result of check express that you cannot upload because of []", errmsg.c_str());

            return false;
        }

        return true;
    }

    bool MapDataPackager::SaveToMainDBAfterUpload(Editor::Job* job, const std::string type)
    {
        std::string sFileName = GeneratePhotoName(job->GetFileName());

        std::string sProjectId = job->m_sFoundFlag;

        std::string sDeviceId = job->m_sDeviceId;
        std::string sExcuteUrl;

        std::string sRespose = "";
        if(type == "poi")
        {
            sExcuteUrl = Editor::DataManager::getInstance()->getServiceAddress()->GetPoiUploadExcuteUrl(job->m_sToken, job->GetJobType());

            Logger::LogO("poi import url is [%s]", sExcuteUrl.c_str());

        char** pName = new char*[3];

		char** pContent = new char*[3];

			std::string project = "projectId";
			pName[0] = (char*)project.c_str();
		pContent[0] = (char*)sProjectId.c_str();

			std::string filename = "filename";
			pName[1] = (char*)filename.c_str();
		pContent[1] = (char*)sFileName.c_str();

			std::string deviceId = "deviceId";
			pName[2] = (char*)deviceId.c_str();
		pContent[2] = (char*)sDeviceId.c_str();
            
        if(Tools::HttpMultiPost(sExcuteUrl.c_str(),pName, pContent, 3, sRespose) == false)
        {
            Logger::LogD("insert into server database failed after upload");
			Logger::LogO("insert into server database failed after upload");

            Logger::LogO("poi import response is [%s]", sRespose.c_str());
            
            return false;
        }
            
            if (pName)
            {
                delete [] pName;
                pName = NULL;
            }
            
            if(pContent)
            {
                delete [] pContent;
                pContent = NULL;
            }
        }
        else if(type == "infor")
        {
            
            sExcuteUrl = Editor::DataManager::getInstance()->getServiceAddress()->GetInforUploadExcuteUrl(job->m_sToken, sFileName);

            Logger::LogD("info import url is [%s]", sExcuteUrl.c_str());
            Logger::LogO("info import url is [%s]", sExcuteUrl.c_str());
           

            if(Tools::HttpGet(sExcuteUrl.c_str(), sRespose) == false)
			{
				Logger::LogD("insert into database failed after upload");
				Logger::LogO("insert into database failed after upload");

				Logger::LogO("info import response is [%s]", sRespose.c_str());

				return false;
			}

            Logger::LogD("info import response is [%s]", sRespose.c_str());
            Logger::LogO("info import response is [%s]", sRespose.c_str());
        }
        else
        {
            Logger::LogD("type error!!!");
            Logger::LogO("type error!!!");
            return false;
        }

        Editor::ProgressEventer* pPregress = job->GetPregressObj();

		if(pPregress == NULL)
		{
			return false;
		}
        pPregress->OnProgress(Tools::GenerateProgressInfo(job->m_sFoundFlag, sRespose), 999);

        return true;
    }

    bool MapDataPackager::UpdateDatabaseAfterTipsUpload(std::string wkt)
    {
        std::set<std::string> setUploadTips;

        std::string sql = "select * from edit_tips where t_status=1 and (Within(g_location, GeomFromText('" + wkt + "')) or Crosses(g_location, GeomFromText('" + wkt + "'))) and rowid in (select rowid from SpatialIndex s where s.f_table_name='edit_tips' and s.search_frame=g_location)";

        std::string sData = m_pDataFileSystem->GetCoreMapDataPath();

        sqlite3* db = NULL;

        int ret = sqlite3_open_v2(sData.c_str(), &db, SQLITE_OPEN_READWRITE, NULL);
        

        if(ret != SQLITE_OK)
        {
            Logger::LogD("sqlite [%s] open unsuccessfully bacause of [%s]", sData.c_str(), sqlite3_errmsg(db));
            Logger::LogO("sqlite [%s] open unsuccessfully bacause of [%s]", sData.c_str(), sqlite3_errmsg(db));

            sqlite3_close(db);
            return false;
        }
        
        //lee add
        void* cache = spatialite_alloc_connection();
        spatialite_init_ex(db, cache, 0);
        

        sqlite3_stmt* stmt = NULL;

        ret = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (ret != SQLITE_OK)
	    {
		    Logger::LogD("select %s unsuccessfully bacause of [%s]", "Tips", sqlite3_errmsg(db));
            Logger::LogO("select %s unsuccessfully bacause of [%s]", "Tips", sqlite3_errmsg(db));

            //lee add
            if (cache)
            {
                spatialite_cleanup_ex(cache);
                cache=NULL;
            }
            
            sqlite3_close(db);
		    return false;
	    }

        while(sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::string rowkey = (const char*)sqlite3_column_text(stmt, 0);

            setUploadTips.insert(rowkey);
           
        }

        sqlite3_finalize(stmt);

        if(setUploadTips.size() > 0)
        {
        if(UpdateTstatus(db, setUploadTips, "update edit_tips set t_status=2,t_sync=1") == false)
        {
            //lee add
            if (cache)
            {
                spatialite_cleanup_ex(cache);
                cache=NULL;
            }
            
            sqlite3_close(db);

            return false;
			}
        }
        
        //lee add
        if (cache)
        {
            spatialite_cleanup_ex(cache);
            cache=NULL;
        }

        sqlite3_close(db);

        return true;
    }

    bool MapDataPackager::UpdateTstatus(sqlite3* db, std::set<std::string> setUploadTips, const std::string& sql)
    {
        char *err_msg = NULL;

        int ret = sqlite3_exec(db, "BEGIN", NULL, NULL, &err_msg);
        
        if (ret != SQLITE_OK)
        {
		    Logger::LogD("Update edit_tips BEGIN error: %s\n", err_msg);
            Logger::LogO("Update edit_tips BEGIN error: %s\n", err_msg);

            sqlite3_free(err_msg);
            return false;
        }

        std::set<std::string>::const_iterator iter = setUploadTips.begin();

        for(; iter != setUploadTips.end(); iter++)
        {
            std::string updateSql = sql + " where rowkey = '" + *iter + "'";

            sqlite3_stmt* stmt = NULL;

            int rc = sqlite3_prepare_v2(db, updateSql.c_str(), -1, &stmt, NULL);

	        if (rc != SQLITE_OK)
	        {
		        Logger::LogD("update %s unsuccessfully bacause of [%s]", "edit_tips", sqlite3_errmsg(db));
                Logger::LogO("update %s unsuccessfully bacause of [%s]", "edit_tips", sqlite3_errmsg(db));

                sqlite3_finalize(stmt);

		        return false;
	        }

            if (sqlite3_step(stmt) != SQLITE_DONE)
	        {
		        Logger::LogD("sqlite3_step[%s][%s]", updateSql.c_str(), sqlite3_errmsg(db));
		        Logger::LogO("sqlite3_step[%s][%s]", updateSql.c_str(), sqlite3_errmsg(db));

                sqlite3_finalize(stmt);

                return false;
	        }

            sqlite3_finalize(stmt);
        }

        ret = sqlite3_exec (db, "COMMIT", NULL, NULL, &err_msg);

        if (ret != SQLITE_OK)
        {
            Logger::LogD("Update edit_tips COMMIT error: %s\n", err_msg);
            Logger::LogO("Update edit_tips COMMIT error: %s\n", err_msg);

            sqlite3_free(err_msg);
		
		    return false;
        }

        delete err_msg;

        return true;
    }
    bool MapDataPackager::UpdateDatabaseAfterImageUpload()
    {
        std::set<std::string> setUploadTips;
		
        std::string sql = "select * from meta_JVImage where status = 2";
		
        std::string sData = m_pDataFileSystem->GetCoreMapDataPath();
		
        sqlite3* db = NULL;
		
        int ret = sqlite3_open_v2(sData.c_str(), &db, SQLITE_OPEN_READWRITE, NULL);
		
        if(ret != SQLITE_OK)
        {
            Logger::LogD("sqlite [%s] open unsuccessfully bacause of [%s]", sData.c_str(), sqlite3_errmsg(db));
            Logger::LogO("sqlite [%s] open unsuccessfully bacause of [%s]", sData.c_str(), sqlite3_errmsg(db));
            sqlite3_close(db);
			
            return false;
        }
        
        //lee add
        void* cache = spatialite_alloc_connection();
        spatialite_init_ex(db, cache, 0);
		
        sqlite3_stmt* stmt = NULL;
		
        ret = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
		
	    if (ret != SQLITE_OK)
	    {
		    Logger::LogD("select %s unsuccessfully bacause of [%s]", "Tips", sqlite3_errmsg(db));
            Logger::LogO("select %s unsuccessfully bacause of [%s]", "Tips", sqlite3_errmsg(db));
            
            //lee add
            if (cache)
            {
                spatialite_cleanup_ex(cache);
                cache=NULL;
            }
            
            sqlite3_close(db);
			
		    return false;
	    }
		
        while(sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::string rowkey = (const char*)sqlite3_column_text(stmt, 0);
			
            setUploadTips.insert(rowkey);
        }
		
        sqlite3_finalize(stmt);
		
        if(setUploadTips.size() > 0)
        {
        if(UpdateTstatus(db, setUploadTips, "update meta_JVImage set status=1") == false)
        {
            //lee add
            if (cache)
            {
                spatialite_cleanup_ex(cache);
                cache=NULL;
            }
            
            sqlite3_close(db);
            return false;
			}
        }
        
        //lee add
        if (cache)
        {
            spatialite_cleanup_ex(cache);
            cache=NULL;
        }
		
        sqlite3_close(db);
		
        return true;
    }
    std::string MapDataPackager::GetGridFlag()
    {
        std::string sResult;
        for(int i = 0; i < m_vGridId.size(); i++)
        {
            sResult += m_vGridId[i];
        }
        return sResult;
    }

    std::string MapDataPackager::GernerateGetUploadTimePara(const std::string& sToken, const std::string& sProjectId, const std::string deviceId)
    {
        Document document;
        
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
        
        document.SetObject();
        
        document.AddMember("access_token", StringRef(sToken.c_str()), allocator);
        
        document.AddMember("projectId", StringRef(sProjectId.c_str()), allocator);

        document.AddMember("deviceId", StringRef(deviceId.c_str()), allocator);
        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        std::string res = buffer.GetString();
        
        return res;
    }

    std::string MapDataPackager::GetUploadPrepared(const std::string& sToken, const std::string sProjectId, const std::string& sDeviceId, int type)
    {
        std::string sResult;

        std::string sGetUploadTimeUrl = Editor::DataManager::getInstance()->getServiceAddress()->GetUploadTime(sToken, sProjectId, sDeviceId);

        //std::string para = GernerateGetUploadTimePara(sToken, sProjectId, sDeviceId);

        std::string response;

        std::string time;

        int errcode;

        if(Tools::HttpGet(sGetUploadTimeUrl.c_str(), response, errcode) == false)
        {
            Logger::LogD("GetUploadPrepared : httppost getUploadTime error!!!");
            Logger::LogO("GetUploadPrepared : httppost getUploadTime error!!!");

            UploadMsg msg;

            msg.conncode = errcode;

            msg.serverMsg = response;

            sResult = Editor::ProjectManager::getInstance()->GeneratePreparedJson(msg);

            return sResult;
        }
        else
        {
            rapidjson::Document document;
        
            document.Parse<0>(response.c_str());
        
            if (document.HasParseError())
            {
                Logger::LogD("GetUploadPrepared http json is error!!!");
                Logger::LogO("GetUploadPrepared http json is error!!!");

                UploadMsg msg;

                msg.conncode = -2;
            
                sResult = Editor::ProjectManager::getInstance()->GeneratePreparedJson(msg);

                return sResult;
            }

            if(document.HasMember("errcode"))
            {
                int error = document["errcode"].GetInt();

                if(error != 0)
                {
                    Logger::LogD("GetDownloadPrepared inputJson json is error!!!");
                    Logger::LogO("GetDownloadPrepared inputJson json is error!!!");

                    UploadMsg msg;

					msg.conncode = -2;

                    sResult = Editor::ProjectManager::getInstance()->GeneratePreparedJson(msg);

                    return sResult;
                }
            }

            if(!document.HasMember("data") || document["data"].IsNull())
            {
                Logger::LogD("GetUploadPrepared: server return value is null. Server response:[%s]",response.c_str());
                Logger::LogO("GetUploadPrepared: server return value is null. Server response:[%s]",response.c_str());
                
                time = "";
            }
            else
            {
                time = document["data"].GetString();
            }
        }

        if(type == POI)
        {
            std::string sFilename;

            std::string errmsg;

            int total=0, add=0, upd=0, del=0;

		    std::string sDataPath = GetUploadFile(sFilename, time, errmsg, sProjectId, total, add, upd, del, POI);

            if(sDataPath.empty() || sFilename.empty())
            {
                Logger::LogD("GetUploadPrepared : package poi error!!!");
                Logger::LogO("GetUploadPrepared : package poi error!!!");

                UploadMsg msg;

				msg.conncode = -2;

                sResult = Editor::ProjectManager::getInstance()->GeneratePreparedJson(msg);

                return sResult;
            }

            UploadMsg msg;

            msg.conncode = 0;
            msg.serverMsg="";
            msg.filename = sFilename;
            msg.path = sDataPath;
            msg.time = time;
            msg.total = total;
            msg.add = add;
            msg.upd = upd;
            msg.del = del;

            sResult = Editor::ProjectManager::getInstance()->GeneratePreparedJson(msg);

            return sResult;
        }
        
        if(type == POI_CHARGING)
        {
            std::string sFilename;
            
            std::string errmsg;
            
            int total=0, add=0, upd=0, del=0;
            
            std::string sDataPath = GetUploadFile(sFilename, time, errmsg, sProjectId, total, add, upd, del,POI_CHARGING);
            
            if(sDataPath.empty() || sFilename.empty())
            {
                Logger::LogD("GetUploadPrepared : package poi charging* error!!!");
                Logger::LogO("GetUploadPrepared : package poi charging* error!!!");
                
                UploadMsg msg;
                
                msg.conncode = -2;
                
                sResult = Editor::ProjectManager::getInstance()->GeneratePreparedJson(msg);
                
                return sResult;
            }
            
            UploadMsg msg;
            
            msg.conncode = 0;
            msg.serverMsg="";
            msg.filename = sFilename;
            msg.path = sDataPath;
            msg.time = time;
            msg.total = total;
            msg.add = add;
            msg.upd = upd;
            msg.del = del;
            
            sResult = Editor::ProjectManager::getInstance()->GeneratePreparedJson(msg);
            
            return sResult;
        }

        if(type == POI_INFO)
        {
            std::string sPackageSql = "select * from edit_infos where b_sourceCode=1 and c_isAdopted!=0 and submitStatus=0 and h_projectId='"+sProjectId+"';";
            
            std::string sFilename;

            std::string errmsg;
            
            std::string sUploadFilePath = GetUploadInfoFile(sFilename, errmsg, sPackageSql,sProjectId,POI_INFO);
            
            if(sUploadFilePath.empty() || sFilename.empty())
            {
                Logger::LogD("GetUploadPrepared : package poi_info error!!!");
                Logger::LogO("GetUploadPrepared : package poi_info error!!!");

                UploadMsg msg;

				msg.conncode = -2;

				sResult = Editor::ProjectManager::getInstance()->GeneratePreparedJson(msg);

				return sResult;
            }

            UploadMsg msg;

			msg.conncode = 0;
			msg.serverMsg="";
			msg.filename = sFilename;
			msg.path = sUploadFilePath;

            sResult = Editor::ProjectManager::getInstance()->GeneratePreparedJson(msg);

            return sResult;
        }
        
        if(type == ROAD_INFO)
        {
            std::string sPackageSql = "select * from edit_infos where b_sourceCode=2 and c_isAdopted!=0 and submitStatus=0 and h_projectId='"+sProjectId+"';";
            
            std::string sFilename;
            
            std::string errmsg;
            
            std::string sUploadFilePath = GetUploadInfoFile(sFilename, errmsg, sPackageSql,sProjectId,ROAD_INFO);
            
            if(sUploadFilePath.empty() || sFilename.empty())
            {
                Logger::LogD("GetUploadPrepared : package road_info error!!!");
                Logger::LogO("GetUploadPrepared : package road_info error!!!");
                
                UploadMsg msg;
                
                msg.conncode = -2;
                
                sResult = Editor::ProjectManager::getInstance()->GeneratePreparedJson(msg);
                
                return sResult;
            }
            
            UploadMsg msg;
            
            msg.conncode = 0;
            msg.serverMsg="";
            msg.filename = sFilename;
            msg.path = sUploadFilePath;
            
            sResult = Editor::ProjectManager::getInstance()->GeneratePreparedJson(msg);
            
            return sResult;
        }

        return "";
    }

    void MapDataPackager::UploadFile(const std::string& input, ProgressEventer* progress)
    {
        Logger::LogO("start to download  from [%s]", input.c_str());

        rapidjson::Document document;
        
        document.Parse<0>(input.c_str());
        
        if (document.HasParseError())
        {
            Logger::LogD("UploadFile inputJson json is error!!!");
            Logger::LogO("UploadFile inputJson json is error!!!");

            progress->OnProgress("inputJson error", -1);
            
            return ;
        }

        std::string sToken;
        std::string sProjectId;
        std::string sPath;
        std::string sFilename;
        std::string sDeviceId;

        int type = -1;
        int connectTime = -1;

        if(document.HasMember("token"))
        {
            sToken = document["token"].GetString();
        }

        if(document.HasMember("filename"))
        {
            sFilename = document["filename"].GetString();
        }

        if(document.HasMember("project"))
        {
            sProjectId = document["project"].GetString();
        }

        if(document.HasMember("deviceid"))
        {
            sDeviceId = document["deviceid"].GetString();
        }

        if(document.HasMember("url"))
        {
            sPath = document["url"].GetString();
        }

        if(document.HasMember("type"))
        {
            type = document["type"].GetInt();
        }

        if(document.HasMember("connecttime"))
        {
            connectTime = document["connecttime"].GetInt();
        }

        if(sToken.empty() || sPath.empty() || sFilename.empty() || sDeviceId.empty())
        {
            Logger::LogD("inputJson parameter error!!!");
            Logger::LogO("inputJson parameter error!!!");

            progress->OnProgress("inputJson parameter error!!!", -1);

            return;
        }

        if(Tools::CheckFilePath(sPath.c_str()) == false)
		{
			Logger::LogD("the path that is used to save the downloadfile is not existing");
            Logger::LogO("the path that is used to save the downloadfile is not existing");

            progress->OnProgress("save path is errror", -1);
			return;
		}

        unsigned long nFileSize;

        int nChunk;

        if(UploadSnapShot(sPath.c_str(), sFilename.c_str(), sToken.c_str(), nFileSize, nChunk, type) == false)
        {
            return;
        }

        m_nChunkNum = nChunk;
			
        for(int k = 0; k < nChunk; k++)
        {
            Editor::Job* job = new Editor::Job();

            job->m_lTotal = nFileSize;

			job->SetJobType(type);

			job->m_sFoundFlag = sProjectId;

            std::string sUrl = Editor::DataManager::getInstance()->getServiceAddress()->GetPoiUploadDataChunk(sToken, sFilename, k, type);

			job->SetUrl(sUrl);

			job->SetFileName(sPath);

            job->m_nChunkNo = k;

            job->m_nMaxChunkSize = m_MaxChunkSize;

            job->m_sFileName = sFilename;

            job->m_sToken = sToken;

            job->m_sDeviceId = sDeviceId;
            
			m_pDataUploader->StartJob(job, progress);
        }
    }
}
