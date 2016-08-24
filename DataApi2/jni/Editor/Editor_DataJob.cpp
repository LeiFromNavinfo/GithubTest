#include "Editor.h"

namespace Editor
{
    size_t OnWriteBuffer2File(char* buffer, size_t size, size_t nmemb, void* arg);

    size_t OnProgress(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);

    size_t OnReadFile2Buffer(char* buffer, size_t size, size_t nmemb, void* arg);

    size_t OnProgressForUpload(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);

    size_t WriteData(char* ptr, size_t size, size_t nmemb, void* userdata);

    Job::Job()
    {
        m_nCount = 0;

        m_nRepeatUploadNum = 1;

        m_nChunkSize = 0;

        m_nMaxChunkSize = -1;

        m_nChunkNo = -1;

        m_nTotal = 1;

        m_nNow = 0;

        m_sUrl = "";

        m_sFile = "";

        m_pProjectUser = NULL;

        m_sFoundFlag = "";

        m_sFileName = "";

        m_sToken = "";

        m_sResponse = "";

        m_lTotal = 0;

        m_onPregress = NULL;
    }

    Job::~Job() {}

    void Job::SetOnPregress(Editor::ProgressEventer* onPregress)
    {
        m_onPregress = onPregress;
    }

    void Job::SetUploadTarget(Address* addr, std::string target)
    {
        m_pAddress = addr;
    }

    void Job::SetUrl(const std::string& sUrl)
    {
        m_sUrl = sUrl;
    }

    void Job::SetFileName(const std::string& sFileName)
    {
        m_sFile = sFileName;
    }

    void Job::SetTotalSize(long totalsize)
    {
        m_nTotal = totalsize;
    }

    void Job::SetCurrentSize(long currentsize)
    {
        m_nNow = currentsize;
    }

    void Job::SetProjectUser(Model::ProjectUser* pProjectUser)
    {
        m_pProjectUser = pProjectUser;
    }

    void Job::SetJobType(int type)
    {
        m_nType = type;
    }

    Model::ProjectUser* Job::GetProjectUser()
    {
        return m_pProjectUser;
    }

    std::string Job::GetFileName()
    {
        if(m_sFile.empty())
        {
            return "";
        }

        return m_sFile;
    }

    std::string Job::GetFinishedFile()
    {
        return "";
    }

    void Job::StartDownload(DataDownloader* downloader, Job* job, Editor::Address* pAddress)
    {
        m_thread.reset(new boost::thread(boost::bind(&Editor::Job::DownloadFile, this, downloader, job, pAddress)));
    }

    void Job::StartUpload(DataUploader* uploader, Job* job)
    {
        m_thread.reset(new boost::thread(boost::bind(&Editor::Job::UploadFile, this, uploader, job)));
    }

    void Job::Stop(Job* job)
    {
        m_thread->interrupt();

        // 目前打断线程的实现方法是：中断连接并重新设置超时时间，超过超时时间，自动退出并正常结束线程
        curl_easy_pause( job->m_curl, CURLPAUSE_ALL);

        curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 1); 

        curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 1);
    }

    void Job::UploadFile(DataUploader* uploader, Job* job)
    {
        try
        {
            boost::this_thread::interruption_point();

            job->CreateUploadRequest(job);
        }
        catch(boost::thread_interrupted&)
        {
            Logger::LogD("停止上传");
            Logger::LogO("停止上传");
        }

        uploader->CallBack(job);
    }

    int Job::DownloadFile(DataDownloader* downloader, Job* job, Editor::Address* pAddress)
    {
        try
        {
            boost::this_thread::interruption_point();

            job->CreateDownloadRequest(job);

            //boost::this_thread::sleep(boost::posix_time::seconds(1));
        }
        catch(boost::thread_interrupted&)
        {
            if(job->m_outfile.is_open())
            {
                job->m_outfile.close();
            }
        }

        // 回调等待线程
        downloader->CallBack(job);

        return 0;
    }

    CURL* Job::CreateUploadRequest(Job* job)
    {
        char* data = GetUploadDataByChuckNo(m_nChunkSize);

        std::string sChunkNo = Tools::NumberToString<int>(m_nChunkNo);

        if(m_nChunkSize == 0 || data == NULL)
        {
            Logger::LogD("failed to read upload data stream!!!");
            Logger::LogO("failed to read upload data stream!!!");

            free(data);

            return NULL;
        }

        CURLcode res;

        struct curl_httppost* formpost = NULL;
        struct curl_httppost* lastptr = NULL;

        curl_formadd(&formpost, 
                     &lastptr, 
                     CURLFORM_PTRNAME, 
                     "access_token", 
                     CURLFORM_PTRCONTENTS, 
                     m_sToken.c_str(), 
                     CURLFORM_END);

        curl_formadd(&formpost, 
                     &lastptr, 
                     CURLFORM_PTRNAME, 
                     "filename", 
                     CURLFORM_PTRCONTENTS, 
                     m_sFileName.c_str(), 
                     CURLFORM_END);

        curl_formadd(&formpost, 
                     &lastptr, 
                     CURLFORM_PTRNAME, 
                     "chunkno", 
                     CURLFORM_PTRCONTENTS, 
                     sChunkNo.c_str(), 
                     CURLFORM_END);

    	curl_formadd(&formpost,
					 &lastptr,
					 CURLFORM_PTRNAME,
					 "file",
					 CURLFORM_BUFFER,
					 m_sFileName.c_str(),
					 CURLFORM_BUFFERPTR,
					 data,
					 CURLFORM_BUFFERLENGTH,
					 m_nChunkSize,
					 CURLFORM_END);

        m_curl = curl_easy_init();
        if(m_curl) 
        {
            curl_easy_setopt(m_curl, CURLOPT_URL, m_sUrl.c_str());

            curl_easy_setopt(m_curl, CURLOPT_HTTPPOST, formpost);

            curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0);

            curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, OnProgressForUpload);

            curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, job);

            curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1);

            curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 50); //wait for 50 seconds to connect to server

            curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 5000); 

		    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, WriteData);
		    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_sResponse);

            res = curl_easy_perform(m_curl);

            if(res != CURLE_OK)
            {
                Logger::LogD("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                Logger::LogO("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
        }

        free(data);

        curl_easy_cleanup(m_curl);

        curl_formfree(formpost);

        return m_curl;
    }

    CURL* Job::CreateDownloadRequest(Job* job)
    {
        m_curl = curl_easy_init();

        if(m_curl == NULL)
        {
            return NULL;
        }

        //支持断点续传
        long currentSize = GetLocalFileSize();

        long totalSize = GetDownloadFileSize();

        if(totalSize <= 0)
        {
            return NULL;
        }

        std::stringstream ss;

        ss<<currentSize;

        ss<<"-";

        ss<<totalSize;

        std::string sRange = ss.str();

        curl_easy_setopt(m_curl, CURLOPT_RANGE, sRange.c_str());

        struct curl_slist *headinfo = NULL;

        headinfo = curl_slist_append(headinfo, "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko");

        curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, headinfo);// 改协议头

        curl_easy_setopt(m_curl, CURLOPT_URL, m_sUrl.c_str());

        curl_easy_setopt(m_curl, CURLOPT_READFUNCTION, NULL);

        curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, OnWriteBuffer2File);

        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this);

        curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0);

        curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, OnProgress);

        curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, job);

        /**
        * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
        * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
        */
        curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1);

        curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 50); 

        curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 500);       //0 means block always

        CURLcode res = curl_easy_perform(m_curl);

        curl_easy_cleanup(m_curl);

        if(m_outfile.is_open())
        {
            m_outfile.close();
        }

        return m_curl;
    }

    long Job::GetDownloadFileSize()
    {
        if(m_sUrl.empty())
        {
            return -1;
        }

        boost::this_thread::sleep(boost::posix_time::seconds(1));

        CURL* curl = curl_easy_init();

        if(NULL == curl)
        {
            return -1;
        }

        curl_easy_setopt(curl, CURLOPT_URL, m_sUrl.c_str());

        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);

        curl_easy_setopt(curl, CURLOPT_NOBODY, 1);

        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 50); 

        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 500);       //0 means block always

        CURLcode res = curl_easy_perform(curl);

        if(res == CURLE_OK)
        {
            double sz = 0;

            curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &sz);

            curl_easy_cleanup(curl);

            return (long)sz;
        }

        return -1;
    }

    long Job::GetLocalFileSize()
    {
        std::ifstream infile(m_sFile.c_str(), std::ios::binary | std::ios::in);

        long length;

        if(infile.good())
        {
            infile.seekg(0, infile.end);

            length = infile.tellg();
        }
        else
        {
            return 0;
        }

        infile.close();

        return length;
    }

    long Job::GetTotalSize()
    {
        return m_nTotal;
    }

    long Job::GetCurrentSize()
    {
        return m_nNow;
    }

    long Job::GetProgress()
    {
        return (long)(m_nNow / m_nTotal);
    }

    int Job::GetJobType()
    {
        return m_nType;
    }

    Editor::ProgressEventer* Job::GetPregressObj()
    {
        if(m_onPregress != NULL)
        {
            return m_onPregress;
        }

        return NULL;
    }

    char* Job::GetUploadDataByChuckNo(unsigned long& nLength)
    {
        FILE* fp;

    	fp = fopen(m_sFile.c_str(), "rb");
        
    	if (fp == NULL)
    	{
    		return NULL;
    	}

    	char* buf = (char*)malloc(sizeof(char*) * m_nMaxChunkSize);

    	long offset = m_nMaxChunkSize * m_nChunkNo;

    	fseek(fp, offset, SEEK_SET);

    	nLength = fread(buf, 1, m_nMaxChunkSize, fp);

    	fclose(fp);

    	return buf;
    }

    size_t OnWriteBuffer2File(char* buffer, size_t size, size_t nmemb, void* arg)
    {
        Editor::Job* pThis = (Editor::Job*)arg;

        if(pThis == NULL)
        {
            return 0;
        }

        /*std::ofstream out(pThis->m_sFile, std::ofstream::binary);*/
        if(!pThis->m_outfile.is_open())
        {
            std::string sFileName = pThis->GetFileName();

            pThis->m_outfile.open(sFileName.c_str(), std::ofstream::binary | std::ofstream::app);
        }

        if(pThis->m_outfile.good())
        {
            const char* pData = (const char*)buffer;

            pThis->m_outfile.write(pData, size*nmemb);

            return nmemb;
        }

        boost::this_thread::interruption_point();

        return 0;
    }

    size_t OnProgress(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
    {
        Editor::Job* pJob = (Editor::Job*)clientp;

        if(pJob == NULL)
        {
            return 0;
        }
       
        Editor::ProgressEventer* pPregress = pJob->GetPregressObj();

        if(pPregress == NULL)
        {
            return 0;
        }

        double nProgress = 100*dlnow/dltotal;

        if(nProgress - pJob->m_nCount >= 0)
        {
        	if(dltotal != 0)
            {
            	pPregress->OnProgress(Tools::GenerateProgressInfo(pJob->m_sFoundFlag, "下载"), 100*dlnow/dltotal);
            }

            pJob->m_nCount += 1;
        }

        pJob->SetTotalSize(dltotal);

        pJob->SetCurrentSize(dlnow);

        return 0;
    }

    size_t OnReadFile2Buffer(char* buffer, size_t size, size_t nmemb, void* arg)
    {
        Editor::Address* pThis = (Editor::Address*)arg;

        if(pThis == NULL)
        {
            return 0;
        }

        std::ifstream infile(pThis->m_sFile.c_str(), std::ifstream::binary);

        if(infile.good())
        {
            infile.seekg(pThis->m_lUploadPos, infile.beg);

            if(infile.eof() == false)
            {
                infile.read(buffer, size * nmemb);

                pThis->m_lUploadPos += infile.gcount();

                return infile.gcount();
            }
        }

        return 0;
    }

    size_t OnProgressForUpload(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
    {
        Editor::Job* pJob = (Editor::Job*)clientp;

        if(pJob == NULL)
        {
            return 0;
        }

        pJob->SetTotalSize(ultotal);

        pJob->SetCurrentSize(ulnow);

        Editor::ProgressEventer* pPregress = pJob->GetPregressObj();

		if(pPregress == NULL)
		{
			return 0;
		}

		double nProgress = 100*ulnow/ultotal;

		if(nProgress - pJob->m_nCount >= 0)
		{
			if(ultotal != 0)
			{
                DataManager::getInstance()->getUploader()->RefreshProgress(pJob->m_sFoundFlag, pJob->m_lTotal, pPregress);

				//pPregress->OnProgress(Tools::GenerateProgressInfo(pJob->m_sFoundFlag, "上传"), 100*ulnow/ultotal);
			}

			pJob->m_nCount += 1;
		}

        return 0;
    }

    size_t WriteData(char* ptr, size_t size, size_t nmemb, void* userdata)
	{
		std::string* str = (std::string*)userdata;

		if (str == NULL || ptr == NULL)
		{
			return -1;
		}

		size_t totalSize = size * nmemb;

		str->append(ptr, totalSize);

		return totalSize;
	}

} // namespace