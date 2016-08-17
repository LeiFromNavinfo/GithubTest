#include "Editor.h"
#include "Editor_ProjectManager.h"
#ifdef ANDROID
#include "jniProgress.h"
#endif
#define MaxRepeatNum 3

namespace Editor
{
    DataUploader::DataUploader()
    {
        m_iConcurrent = 3;
        
        m_Progress = NULL;
    }
    
    DataUploader::~DataUploader()
    {
        Cancel();
    }
    
    void DataUploader::SetMaxConcurrent(int concurrent)
    {
        m_iConcurrent = concurrent;
    }
    
    void DataUploader::StartJob(UploadJob* job)
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        
        if (m_vJobs.size() >= m_iConcurrent)
        {
            m_qWaitJobs.push(job);
        }
        else
        {
            m_vJobs.push_back(job);
            
            job->Start();
        }
    }
    
    void DataUploader::StartJob(Job* job, Editor::ProgressEventer* onPregress)
    {
    	boost::unique_lock<boost::mutex> lock(m_mutex);

		if(onPregress != NULL)
		{
			job->SetOnPregress(onPregress);
		}

		if(m_vJobsBefore.size() >= m_iConcurrent)
		{
			m_qWaitJobsBefore.push(job);
		}
		else
		{
            m_vJobsBefore.push_back(job);

			job->StartUpload(this, job);
		}
    }

    void DataUploader::CallBack(Job* job)
	{
        boost::unique_lock<boost::mutex> lock(m_mutex);

		std::vector<Job*>::iterator iter = m_vJobsBefore.begin();

		for(; iter != m_vJobsBefore.end(); iter++)
		{
            if((*iter)->m_sFile == job->m_sFile && (*iter)->m_nChunkNo == job->m_nChunkNo)
			{
				m_vJobsBefore.erase(iter);

				break;
			}
		}

        bool flagChunkResponse = ParseUploadChunkResponse(job->m_sResponse);

		long nCurrentSize = job->GetCurrentSize();

		long nTotalSize =  job->GetTotalSize();

        if(nCurrentSize != nTotalSize || flagChunkResponse == false)
        {
            job->m_nRepeatUploadNum += 1;

            lock.unlock();

            if(job->m_nRepeatUploadNum <= MaxRepeatNum)
            {
                StartJob(job, job->GetPregressObj());
            }
            else
            {
                Editor::ProjectManager::getInstance()->getMapDataPackager()->FailedCallBack(job);
            }
        }
        else
        {
            Editor::ProjectManager::getInstance()->getMapDataPackager()->CallBack(job);
            std::map<std::string, unsigned long>::iterator it = m_mUpload.find(job->m_sFoundFlag);

            if(it != m_mUpload.end())
            {
                it->second += nTotalSize;
            }
            else
            {
                m_mUpload.insert(std::make_pair(job->m_sFoundFlag, nTotalSize));
            }

            delete job;

		    if(m_qWaitJobsBefore.size() > 0)
		    {
			    Job* job = m_qWaitJobsBefore.front();

			    m_qWaitJobsBefore.pop();

			    m_vJobsBefore.push_back(job);

			    job->StartUpload(this, job);
		    }
        }

#if defined ANDROID
		JNIProgress::DisposeProgress();
#endif
	}

    void DataUploader::CancelJob(UploadJob* job)
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        
        std::vector<UploadJob*>::iterator itor = m_vJobs.begin();
        
        for (; itor!=m_vJobs.end(); itor++)
        {
            if (*itor == job)
            {
                m_vJobs.erase(itor);
                
                break;
            }
        }
        
        job->Stop();
        
        std::string key = job->GetUploadFileName().c_str();
        
        m_mJobStatus.erase(key);
        
        m_UploadFileChunkCount.erase(key);
        
        m_UploadFileSize.erase(key);
        
        m_UploadFileFinishedSize.erase(key);
    }
    
    void DataUploader::Cancel()
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        
        std::vector<UploadJob*>::iterator itor = m_vJobs.begin();
        
        for(; itor!=m_vJobs.end(); itor++)
        {
            (*itor)->Stop();
        }
        
        m_vJobs.clear();
        
        while (m_qWaitJobs.size() > 0)
        {
            m_qWaitJobs.pop();
        }
        
        std::vector<Editor::Job*>::iterator iter = m_vJobsBefore.begin();

        for(; iter!=m_vJobsBefore.end(); iter++)
		{
			(*iter)->Stop(*iter);
		}

        m_vJobsBefore.clear();

		while (m_qWaitJobsBefore.size() > 0)
		{
			m_qWaitJobsBefore.pop();
		}

        m_mJobStatus.clear();
        
        m_UploadFileChunkCount.clear();
        
        m_UploadFileSize.clear();
        
        m_UploadFileFinishedSize.clear();
    }
    
    void DataUploader::JobFinishedCallback(UploadJob* job)
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        
        std::vector<UploadJob*>::iterator iter = m_vJobs.begin();
        
        for(; iter != m_vJobs.end(); iter++)
        {
            if (*iter == job)
            {
                m_vJobs.erase(iter);
                
                break;
            }
        }
        
        int jobStatus = job->GetJobStatus();
        
        Logger::LogD("job [%d:%d] status [%d]!", job->GetUploadJobId(), job->GetUploadChunkNo(), jobStatus);
        
        std::string key = job->GetUploadFileName();
        
        int chunkNo = job->GetUploadChunkNo();
        
        double finishedChunkSize = job->GetUploadChunkLen() * 1.0;
        
        std::map<std::string, std::map<int, int> >::iterator itor = m_mJobStatus.find(key);
        
        if (itor == m_mJobStatus.end())
        {
            std::map<int, int> map;
            
            map.insert(std::make_pair(chunkNo, jobStatus));
            
            m_mJobStatus.insert(std::make_pair(key, map));
        }
        else
        {
            itor->second.insert(std::make_pair(chunkNo, jobStatus));
        }
        
        int finishedChunkCount = itor->second.size();
        
        std::map<std::string, int>::const_iterator itorr = m_UploadFileChunkCount.find(key);
        
        if (itorr != m_UploadFileChunkCount.end())
        {
            if (m_Progress && (1 == jobStatus))
            {
                RefreshJobFinishedSizeByFile(job, finishedChunkSize);
            }
            else
            {
                RefreshJobFinishedSizeByFile(job, 0);
            }
            
            //            if (m_Progress)
            //            {
            //            	std::string msg = GetProgressMsg(job);
            //
            //            	std::map<std::string, std::pair<double, double> >::iterator iterr = m_UploadFileSize.find(job->GetUploadFileName().c_str());
            //
            //            	if (iterr != m_UploadFileSize.end())
            //            	{
            //            		iterr->second.first += job->GetUploadChunkLen() * 1.0;
            //
            //            		double percent = iterr->second.first * 100 / iterr->second.second;
            //
            //            		m_Progress->OnProgress(msg, percent);
            //            	}
            //            }
            
            int totalChunkCount = itorr->second;
            
            if (finishedChunkCount >= totalChunkCount)
            {
                ProjectManager* projectManager = ProjectManager::getInstance();
                
                MapDataPackager* packager = projectManager->getMapDataPackager();
                
                lock.unlock();
                
                packager->ConfirmUploadTips(job);
                
                lock.lock();
            }
        }
        else
        {
            //single file
        }
        
        delete job;
        
        if(m_qWaitJobs.size() > 0)
        {
            UploadJob* job = m_qWaitJobs.front();
            
            m_qWaitJobs.pop();
            
            m_vJobs.push_back(job);
            
            job->Start();
        }
        
#if defined ANDROID
        JNIProgress::DisposeProgress();
#endif
    }
    
    void DataUploader::JobProgressCallback(UploadJob* job)
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        
        if (!m_Progress)
            return;
        
        double finishedChunkSize = job->GetCurrentSize();
        
        RefreshJobFinishedSizeByFile(job, finishedChunkSize);
    }
    
    void DataUploader::SetUploadFileInfo(const char* fileName, double fileSize, int chunkCount)
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        
        std::map<std::string, int>::iterator itor1 = m_UploadFileChunkCount.find(fileName);
        
        if (itor1 == m_UploadFileChunkCount.end())
        {
            m_UploadFileChunkCount.insert(std::make_pair(fileName, chunkCount));
        }
        else
        {
            itor1->second = chunkCount;
        }
        
        std::map<std::string, std::pair<double, double> >::iterator itor2 = m_UploadFileSize.find(fileName);
        
        if (itor2 == m_UploadFileSize.end())
        {
            m_UploadFileSize.insert(std::make_pair(fileName, std::make_pair(0, fileSize)));
        }
        else
        {
            itor2->second = std::make_pair(0, fileSize);
        }
    }
    
    int DataUploader::GetUploadFileChunkCount(const char* fileName)
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        
        std::map<std::string, int>::iterator itor = m_UploadFileChunkCount.find(fileName);
        
        if (itor != m_UploadFileChunkCount.end())
        {
            return itor->second;
        }
        else
        {
            return 0;
        }
    }
    
    void DataUploader::RefreshJobStatusByFile(const char* fileName, std::set<int>& finishedChunkNo)
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        
        std::map<std::string, std::map<int, int> >::iterator itor = m_mJobStatus.find(fileName);
        
        if (itor == m_mJobStatus.end())
        {
            Logger::LogD("jobstatus find failed!");
            return;
        }
        
        itor->second.clear();
        
        std::set<int>::const_iterator iter = finishedChunkNo.begin();
        
        for (; iter!=finishedChunkNo.end(); iter++)
        {
            itor->second.insert(std::make_pair(*iter, 1));
        }
    }
    
    void DataUploader::RefreshJobFinishedSizeByFile(UploadJob* job, double finishedChunkSize)
    {
        std::string key = job->GetUploadFileName();
        
        int chunkNo = job->GetUploadChunkNo();
        
        std::map<std::string, std::map<int, double> >::iterator itor = m_UploadFileFinishedSize.find(key);
        
        if (itor == m_UploadFileFinishedSize.end())
        {
            std::map<int, double> map;
            
            map.insert(std::make_pair(chunkNo, finishedChunkSize));
            
            m_UploadFileFinishedSize.insert(std::make_pair(key, map));
        }
        else
        {
            std::map<int, double>::iterator iter = itor->second.find(chunkNo);
            
            if (iter == itor->second.end())
            {
                itor->second.insert(std::make_pair(chunkNo, finishedChunkSize));
            }
            else
            {
                iter->second = finishedChunkSize;
            }
        }
        
        std::map<std::string, std::pair<double, double> >::iterator itorr = m_UploadFileSize.find(key);
        
        if (itorr == m_UploadFileSize.end())
            return;
        
        double totalFileSize = itorr->second.second;
        
        itor = m_UploadFileFinishedSize.find(key);
        
        if (itor == m_UploadFileFinishedSize.end())
            return;
        
        std::map<int, double>::const_iterator iter = itor->second.begin();
        
        double finishedFileSize = 0;
        
        for (; iter!=itor->second.end(); iter++)
        {
            finishedFileSize += iter->second;
        }
        
        std::string msg = GetProgressMsg(job);
        
        double percent = finishedFileSize * 100 / totalFileSize;
        
        if (percent > 100.00)
        {
            percent = 100.00;
        }
        
        m_Progress->OnProgress(msg, percent);
    }
    
    void DataUploader::SetProgress(ProgressEventer* progress)
    {
        m_Progress = progress;
    }
    
    std::string DataUploader::GetProgressMsg(UploadJob* job)
    {
        rapidjson::Document document;
        
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
        
        document.SetObject();
        
        document.AddMember("fileName", rapidjson::StringRef(job->m_FileName.c_str()), allocator);
        
        document.AddMember("jobId", job->m_JobId, allocator);
        
        document.AddMember("flag", "tips数据", allocator);
        
        document.AddMember("info", "上传进行中", allocator);
        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        std::string res = buffer.GetString();
        
        return res;
    }

    bool DataUploader::ParseUploadChunkResponse(const std::string& response)
    {
        rapidjson::Document document;

        document.Parse<0>(response.c_str());

        if(document.HasParseError())
        {
            Logger::LogD("falied to parse the response of chunk : [%s]", response.c_str());
            Logger::LogO("falied to parse the response of chunk : [%s]", response.c_str());

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

            Logger::LogD("result of chunk express that you cannot upload because of [%s]", errmsg.c_str());
            Logger::LogO("result of chunk express that you cannot upload because of [%s]", errmsg.c_str());

            return false;
        }

        return true;
    }

    void DataUploader::RefreshProgress(const std::string& flag, unsigned long totalSize, Editor::ProgressEventer* progress)
    {
        if(progress != NULL)
        {
            unsigned long nUpload = 0;

            for(int i = 0; i < m_vJobsBefore.size(); i++)
            {
                int n = m_vJobsBefore[i]->GetTotalSize();

                nUpload += m_vJobsBefore[i]->GetCurrentSize();
            }

            std::map<std::string, unsigned long>::iterator it = m_mUpload.find(flag);

            if(it != m_mUpload.end())
            {
                nUpload += it->second;
            }

            double nProgress = (double)nUpload / totalSize;

            //std::cout<<"nUpload : "<<nUpload<<"   totalSize : "<<totalSize<<std::endl;

            if(nProgress >= 1)
            {
                m_mUpload.clear();

                nProgress = 1;
            }

            progress->OnProgress(Tools::GenerateProgressInfo(flag, "上传"), 100*nProgress);

        }      
    }
    
//    void DataUploader::CallBack(Job* job)
//    {
//        boost::unique_lock<boost::mutex> lock(m_mutex);
//
//        long nCurrentSize = job->GetCurrentSize();
//
//        long nTotalSize =  job->GetTotalSize();
//
//        if(nCurrentSize == nTotalSize)
//        {
//            // 上传完成并成功
//            Model::ProjectUser* pProjectUser = job->GetProjectUser();
//
//            if(pProjectUser != NULL)
//            {
//                Editor::DataManager* pDataManager = Editor::DataManager::getInstance();
//
//            Editor::DataFileSystem* pFileSystem = pDataManager->getFileSystem();
//
//            pProjectUser->upload_url = "";
//
//                pProjectUser->UpdateSqliteTable(pFileSystem->GetProjectDataPath());
//            }
//        }
//
//        std::vector<std::pair<Job*, Address*> >::iterator iter = m_vJobs.begin();
//
//        for(; iter != m_vJobs.end(); iter++)
//        {
//            if((iter->first)->m_sFile == job->m_sFile)
//            {
//                m_vJobs.erase(iter);
//
//                break;
//            }
//        }
//
//        if(m_qWaitJobs.size() > 0)
//        {
//            std::pair<Job*, Address*> job = m_qWaitJobs.front();
//
//            m_qWaitJobs.pop();
//
//            m_vJobs.push_back(job);
//
//            (job.first)->StartUpload(this, job.first, job.second);
//        }
//
//#if defined ANDROID
//        JNIProgress::DisposeProgress();
//#endif
//    }
//
//    std::vector<Editor::Job*> DataUploader::GetCurrentUploadJobs()
//    {
//        std::vector<Editor::Job*> jobsResult;
//
//        return jobsResult;
//    }
//
//    void DataUploader::SetMaxConcurrent(int concurrent)
//    {
//        m_nMaxProcess = concurrent;
//    }
//
//    double DataUploader::GetTotalPregress()
//    {
//        std::vector<std::pair<Job*, Address*> >::iterator iter = m_vJobs.begin();
//
//        double nTotal = 0;
//        
//        double nNow = 0;
//
//        for(; iter != m_vJobs.end(); iter++)
//        {
//            nTotal += (iter->first)->GetTotalSize();
//
//            nNow += (iter->first)->GetCurrentSize();
//        }
//
//        if(nTotal == 0 || (m_vJobs.size() + m_qWaitJobs.size()) == 0)
//        {
//            return 0;
//        }
//
//        return (nNow/nTotal)*(double(m_vJobs.size())/(m_vJobs.size() + m_qWaitJobs.size()))*100;
//    }
}