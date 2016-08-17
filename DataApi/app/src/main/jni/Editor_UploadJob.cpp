#include "Editor.h"

namespace Editor
{
    UploadJob::UploadJob()
    {
    	m_Curl = NULL;

    	m_MultiHandle = NULL;

    	m_FormPost = NULL;

    	m_HeaderList = NULL;

    	m_Chunk = NULL;

    	m_ChunkLen = 0;

    	m_JobStatus = 0;
    }

    UploadJob::~UploadJob()
    {
    	Stop();
    }

    void UploadJob::Start()
    {
    	m_thread.reset(new boost::thread(boost::bind(&UploadJob::Upload, this)));

    	m_JobStatus = 0;
    }

    void UploadJob::Stop()
    {
    	if (0 == m_JobStatus)
    	{
    		m_JobStatus = 2;
    	}

    	m_thread->interrupt();

    	if (m_Curl)
    	{
    		if (m_MultiHandle)
    		{
    			curl_multi_remove_handle(m_MultiHandle, m_Curl);
    		}

    		curl_easy_cleanup(m_Curl);

    		m_Curl = NULL;
    	}

    	if (m_MultiHandle)
    	{
    		curl_easy_cleanup(m_MultiHandle);

    		m_MultiHandle = NULL;
    	}

    	if (m_FormPost)
    	{
    		curl_formfree(m_FormPost);

    		m_FormPost = NULL;
    	}

    	if (m_HeaderList)
    	{
    		curl_slist_free_all(m_HeaderList);

    		m_HeaderList = NULL;
    	}

    	if (m_Chunk)
    	{
    		free(m_Chunk);

    		m_Chunk = NULL;
    	}
    }

    void UploadJob::Upload()
    {
    	try
		{
    		boost::this_thread::interruption_point();

			MultiPost();
		}
		catch (const boost::thread_interrupted& e)
		{
			Logger::LogD("job stop!");
		}
		catch (...)
		{
			Logger::LogD("catch Upload exception!");
			Logger::LogO("catch Upload exception!");
		}

		DataUploader* uploader = GetUploader();

		uploader->JobFinishedCallback(this);
    }

    size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
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

    int progress_callback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
    {
    	UploadJob* job = (UploadJob*)clientp;

    	if (job == NULL)
    		return 0;

    	double percent = (ulnow - job->GetCurrentSize()) / ultotal;

    	if (percent >= 0.02)
    	{
    		job->SetTotalSize(ultotal);

			job->SetCurrentSize(ulnow);

			job->GetUploader()->JobProgressCallback(job);
    	}

    	if (2 == job->GetJobStatus())
    	{
    		return -1;	//CURLE_ABORTED_BY_CALLBACK
    	}
    	else
    	{
    		return 0;
    	}
    }

    void UploadJob::MultiPost()
    {
    	int still_running;
    	int prev_still_running;
    	int still_running_unchange = 0;

    	struct curl_httppost *lastptr=NULL;
    	static const char buf[] = "Expect:";

    	m_Chunk = ReadUploadChunk(m_ChunkLen);

    	if ((0 == m_ChunkLen) || (NULL == m_Chunk))
    	{
    		m_JobStatus = -1;

    		return;
    	}

    	curl_formadd(&m_FormPost,
					 &lastptr,
					 CURLFORM_PTRNAME, "parameter",
					 CURLFORM_PTRCONTENTS, m_Param.c_str(),
					 CURLFORM_END);

    	curl_formadd(&m_FormPost,
					 &lastptr,
					 CURLFORM_PTRNAME,
					 "file",
					 CURLFORM_BUFFER,
					 m_FileName.c_str(),
					 CURLFORM_BUFFERPTR,
					 m_Chunk,
					 CURLFORM_BUFFERLENGTH,
					 m_ChunkLen,
					 CURLFORM_END);

    	m_Curl = curl_easy_init();

    	if (NULL == m_Curl)
    	{
    		free(m_Chunk);
    		m_Chunk = NULL;

    		m_JobStatus = -1;

    		return;
    	}

    	m_MultiHandle = curl_multi_init();

    	if (NULL == m_MultiHandle)
    	{
    		free(m_Chunk);
    		m_Chunk = NULL;

    		curl_easy_cleanup(m_Curl);
    		m_Curl = NULL;

    		m_JobStatus = -1;

    		return;
    	}

    	m_HeaderList = curl_slist_append(m_HeaderList, buf);

    	std::string url = DataManager::getInstance()->getServiceAddress()->GetTipsUploadChunkUrlRoot();

    	curl_easy_setopt(m_Curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(m_Curl, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(m_Curl, CURLOPT_VERBOSE, 0L);
		curl_easy_setopt(m_Curl, CURLOPT_TIMEOUT, 300);

		curl_easy_setopt(m_Curl, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(m_Curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(m_Curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
		curl_easy_setopt(m_Curl, CURLOPT_PROGRESSDATA, this);

		curl_easy_setopt(m_Curl, CURLOPT_HTTPHEADER, m_HeaderList);
		curl_easy_setopt(m_Curl, CURLOPT_HTTPPOST, m_FormPost);

		std::string response;
		curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, &response);

		curl_multi_add_handle(m_MultiHandle, m_Curl);

		boost::this_thread::interruption_point();

		curl_multi_perform(m_MultiHandle, &still_running);

		prev_still_running = still_running;

		do
		{
			struct timeval timeout;
			int rc; /* select() return code */
			CURLMcode mc; /* curl_multi_fdset() return code */

			fd_set fdread;
			fd_set fdwrite;
			fd_set fdexcep;
			int maxfd = -1;

			long curl_timeo = -1;

			FD_ZERO(&fdread);
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdexcep);

			/* set a suitable timeout to play around with */
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;

			curl_multi_timeout(m_MultiHandle, &curl_timeo);
			if(curl_timeo >= 0)
			{
				timeout.tv_sec = curl_timeo / 1000;
				if(timeout.tv_sec > 1)
					timeout.tv_sec = 1;
				else
					timeout.tv_usec = (curl_timeo % 1000) * 1000;
			}

			/* get file descriptors from the transfers */
			mc = curl_multi_fdset(m_MultiHandle, &fdread, &fdwrite, &fdexcep, &maxfd);

			if(mc != CURLM_OK)
			{
				Logger::LogD("curl_multi_fdset failed! code [%d]", mc);
				Logger::LogO("curl_multi_fdset failed! code [%d]", mc);
				break;
			}

			if(maxfd == -1)
			{
#ifdef WIN32
				Sleep(100);
				rc = 0;
#else
				struct timeval wait = { 0, 100 * 1000 }; /* 100ms */
				rc = select(0, NULL, NULL, NULL, &wait);
#endif
			}
			else
			{
			/* Note that on some platforms 'timeout' may be modified by select().
			   If you need access to the original value save a copy beforehand. */
				rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
			}

			boost::this_thread::interruption_point();

			switch(rc)
			{
			case -1: /* select error */
				{
				}
				break;
			case 0:	/* timeout */
				{
					curl_multi_perform(m_MultiHandle, &still_running);
				}
				break;
			default: /* readable/writable sockets */
				{
					int last_still_running = still_running;

					CURLMcode code = curl_multi_perform(m_MultiHandle, &still_running);

					if (last_still_running == still_running)
					{
						break;
					}
					else
					{
						struct CURLMsg *m;
						bool flag_m = false;
						do
						{
							int msgq = 0;

							m = curl_multi_info_read(m_MultiHandle, &msgq);

							if(m && (m->msg == CURLMSG_DONE))
							{
								CURLcode curlCode = m->data.result;
								void* curlMsg = m->data.whatever;

								if (curlCode != CURLE_OK)
								{
									m_JobStatus = -1;

									Logger::LogD("MultiPost, curlCode [%d]", curlCode);
								}
								else
								{
									CURL* easy_handle = m->easy_handle;
									long resCode;
									curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &resCode);
									if (resCode != 200)
									{
										m_JobStatus = -1;

										Logger::LogD("MultiPost, curlCode [%d], resCode [%ld]", curlCode, resCode);
									}
									else
									{
										if (m_JobStatus != -1)
										{
											m_JobStatus = 1;
										}

										std::string errmsg;
										int errcode = ParseMultiPostResponse(response, errmsg);
										if (errcode != 0)
										{
											m_JobStatus = -1;
										}
									}
								}

								flag_m = true;
							}
						} while(m);

						if (false == flag_m && NULL != m)
						{
							m_JobStatus = -1;

							Logger::LogD("MultiPost, msg [%d]", m->msg);
						}
					}
				}
				break;
			}

			if ((prev_still_running == still_running) && (0 == rc))
			{
				still_running_unchange++;
			}
			else
			{
				still_running_unchange = 0;
			}

			prev_still_running = still_running;

			if (still_running_unchange > 1000)
			{
				m_JobStatus = -1;

				still_running = 0;	//failed!

				Logger::LogD("MultiPost, timeout!");
			}
		}
		while (still_running);

		if (m_JobStatus != 1 && m_JobStatus != -1)
		{
			m_JobStatus = -1;
		}

		curl_multi_remove_handle(m_MultiHandle, m_Curl);

		curl_easy_cleanup(m_Curl);
		m_Curl = NULL;

		curl_multi_cleanup(m_MultiHandle);
		m_MultiHandle = NULL;

		curl_formfree(m_FormPost);
		m_FormPost = NULL;

		curl_slist_free_all(m_HeaderList);
		m_HeaderList = NULL;

    	free(m_Chunk);
    	m_Chunk = NULL;
    }

    int UploadJob::ParseMultiPostResponse(std::string response, std::string& errmsg)
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

    		Logger::LogD("ParseMultiPostResponse failed! response [%s], error [%d][%d]", response.c_str(), code, offset);
    		Logger::LogO("ParseMultiPostResponse failed! response [%s], error [%d][%d]", response.c_str(), code, offset);

    		return errcode;
    	}

    	errcode = document["errcode"].GetInt();

    	if (errcode != 0)
    	{
    		errmsg = document["errmsg"].GetString();
    	}

    	return errcode;
    }

    char* UploadJob::ReadUploadChunk(int& length)
    {
    	FILE* fp;

    	fp = fopen(m_Target.c_str(), "rb");

    	if (fp == NULL)
    	{
    		return NULL;
    	}

    	char* buf = (char*)malloc(sizeof(char*) * m_ChunkSize);

    	long offset = m_ChunkSize * m_ChunkNo;

    	fseek(fp, offset, SEEK_SET);

    	length = fread(buf, 1, m_ChunkSize, fp);

    	fclose(fp);

    	return buf;
    }

    void UploadJob::SetUploadTarget(const char* target)
    {
    	m_Target = target;
    }

    std::string UploadJob::GetUploadTarget()
    {
    	return m_Target;
    }

    void UploadJob::SetUploadFileName(const char* fileName)
    {
    	m_FileName = fileName;
    }

    std::string UploadJob::GetUploadFileName()
    {
    	return m_FileName;
    }

    void UploadJob::SetUploadJobId(int jobId)
    {
    	m_JobId = jobId;
    }

    int UploadJob::GetUploadJobId()
    {
    	return m_JobId;
    }

    void UploadJob::SetUploadChunk(int chunkNo, int chunkSize)
    {
    	m_ChunkNo = chunkNo;

    	m_ChunkSize = chunkSize;
    }

    int UploadJob::GetUploadChunkNo()
    {
    	return m_ChunkNo;
    }

    int UploadJob::GetUploadChunkSize()
    {
    	return m_ChunkSize;
    }

    int UploadJob::GetUploadChunkLen()
    {
    	return m_ChunkLen;
    }

    void UploadJob::SetUploadParam(const char* param)
    {
    	m_Param = param;
    }

    int UploadJob::GetJobStatus()
    {
    	return m_JobStatus;
    }

    void UploadJob::SetCurrentSize(double size)
    {
    	m_CurrentSize = size;
    }

    double UploadJob::GetCurrentSize()
    {
    	return m_CurrentSize;
    }

    void UploadJob::SetTotalSize(double size)
    {
    	m_TotalSize = size;
    }

    double UploadJob::GetTotalSize()
    {
    	return m_TotalSize;
    }

    DataUploader* UploadJob::GetUploader()
    {
    	DataUploader* uploader = DataManager::getInstance()->getUploader();

    	return uploader;
    }
}
