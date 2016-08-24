#include "Editor.h"

namespace Editor
{
    Address::Address() 
    {
        m_onPregress = NULL;
    }

    Address::~Address() 
    {
        if(m_onPregress == NULL)
        {
            delete m_onPregress;

            m_onPregress = NULL;
        }
    }

    void Address::SetUrl(const std::string& sUrl)
    {
        m_sUrl = sUrl;
    }

    void Address::SetFileName(const std::string& sFileName)
    {
        m_sFile = sFileName;
    }

    void Address::SetOnPregress(Editor::ProgressEventer* onPregress)
    {
        m_onPregress = onPregress;
    }

    CURL* Address::CreateDownloadRequest()
    {
        CURL* m_curl = curl_easy_init();

        if(m_curl == NULL)
        {
            return NULL;
        }

        // 支持断点续传
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
        //curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, OnWriteBuffer2File);
        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this);

        curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0);
        //curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, OnProgress);
        curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, NULL);

        /**
        * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
        * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
        */
        curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 50); //wait for 4 seconds to connect to server
        curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 500);       //0 means block always

        CURLcode res = curl_easy_perform(m_curl);

        curl_easy_cleanup(m_curl);

        if(m_outfile.is_open())
        {
            m_outfile.close();
        }

        return m_curl;
    }

    CURL* Address::CreateUploadRequest()
    {
        CURL* curl;
        CURLcode res;

        struct curl_httppost* formpost = NULL;
        struct curl_httppost* lastptr = NULL;

        curl_formadd(&formpost, &lastptr, CURLFORM_PTRNAME, "filetype", CURLFORM_PTRCONTENTS, "video", CURLFORM_END);
        curl_formadd(&formpost, &lastptr, CURLFORM_PTRNAME, "file", CURLFORM_FILE, m_sFile.c_str(), CURLFORM_END);

        curl = curl_easy_init();
        if(curl) 
        {
            curl_easy_setopt(curl, CURLOPT_URL, m_sUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);

            curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 4); //wait for 4 seconds to connect to server
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 500); 
            res = curl_easy_perform(curl);
            if(res != CURLE_OK)
            {
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
            }

            curl_easy_cleanup(curl);

            curl_formfree(formpost);
        }

        return curl;
    }

    long Address::GetDownloadFileSize()
    {
        if(m_sUrl.empty())
        {
            return -1;
        }

        CURL* curl = curl_easy_init();

        if(NULL == curl)
        {
            return -1;
        }

        curl_easy_setopt(curl, CURLOPT_URL, m_sUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1);

        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 50); //wait for 4 seconds to connect to server
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

    long Address::GetLocalFileSize()
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

} // namespace