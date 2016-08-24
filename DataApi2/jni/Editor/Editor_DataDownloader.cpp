#include "Editor.h"
#include "Editor_ProjectManager.h"
#ifdef ANDROID
#include "jniProgress.h"
#endif

namespace Editor
{
    DataDownloader::DataDownloader() 
    {
        m_nMaxProcess = 0;

        m_onPregress = NULL;
    }

    DataDownloader::~DataDownloader() {}

    void DataDownloader::StartJob(Job* job, Editor::Address* pAddress, Editor::ProgressEventer* onPregress)
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);

        if(onPregress != NULL)
        {
        	job->SetOnPregress(onPregress);
        }

        if(m_vJobs.size() >= m_nMaxProcess)
        {
            m_qWaitJobs.push(std::make_pair(job, pAddress));
        }
        else
        {
            m_vJobs.push_back(std::make_pair(job, pAddress));

            job->StartDownload(this, job, pAddress);
        }
    }

    void DataDownloader::CallBack(Editor::Job* job)
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);

        std::vector<std::pair<Job*, Address*> >::iterator iter = m_vJobs.begin();

        for(; iter != m_vJobs.end(); iter++)
        {
            if((iter->first)->m_sFile == job->m_sFile)
            {
                m_vJobs.erase(iter);

                break;
            }
        }

        long nCurrentSize = job->GetCurrentSize();

        long nTotalSize =  job->GetTotalSize();

        if(nCurrentSize == nTotalSize)
        {
            Editor::ProjectManager::getInstance()->getMapDataInstaller()->CallBack(job);          
        }

        if(m_qWaitJobs.size() > 0)
        {
            std::pair<Job*, Address*> job = m_qWaitJobs.front();

            m_qWaitJobs.pop();

            m_vJobs.push_back(job);

            (job.first)->StartDownload(this, job.first, job.second);
        }
#if defined ANDROID
        JNIProgress::DisposeProgress();
#endif
    }

    void DataDownloader::CancelJob(Job* job)
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);

        std::vector<std::pair<Job*, Address*> >::iterator iter = m_vJobs.begin();

        for(; iter != m_vJobs.end(); iter++)
        {
            if((iter->first)->GetFileName() == job->GetFileName())
            {
                m_vJobs.erase(iter);

                break;
            }
        }

        job->Stop(job);
    }

    void DataDownloader::Cancel()
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);

        std::vector<std::pair<Job*, Address*> >::iterator iter = m_vJobs.begin();

        for(; iter != m_vJobs.end(); iter++)
        {
            (iter->first)->Stop(iter->first);
        }

        m_vJobs.clear();

        while(m_qWaitJobs.size() > 0)
        {
            m_qWaitJobs.pop();
        }
    }

    std::vector<Editor::Job*> DataDownloader::GetCurrentDownloadJobs()
    {
        std::vector<Editor::Job*> jobsResult;

        return jobsResult;
    }

    void DataDownloader::SetMaxConcurrent(int concurrent)
    {
        m_nMaxProcess = concurrent;
    }

    double DataDownloader::GetTotalPregress()
    {
        std::vector<std::pair<Job*, Address*> >::iterator iter = m_vJobs.begin();

        double nTotal = 0;
        
        double nNow = 0;

        for(; iter != m_vJobs.end(); iter++)
        {
            nTotal += (iter->first)->GetTotalSize();

            nNow += (iter->first)->GetCurrentSize();
        }

        if(nTotal == 0 || (m_vJobs.size() + m_qWaitJobs.size()) == 0)
        {
            return 0;
        }

        return (nNow/nTotal)*(double(m_vJobs.size())/(m_vJobs.size() + m_qWaitJobs.size()))*100;
    }
}
