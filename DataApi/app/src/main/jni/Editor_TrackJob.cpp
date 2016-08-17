//
//  Editor_TrackJob.cpp
//  FMDataApi
//
//  Created by lidejun on 16/6/2.
//  Copyright © 2016年 Kevin Chou. All rights reserved.
//
#include "Editor_TrackJob.h"
#include "Editor.h"
#include "Tools.h"
#include "BussinessConstants.h"

namespace Editor
{
    size_t OnProgressForTrackUpload(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
    size_t WriteServerResponse(char* ptr, size_t size, size_t nmemb, void* userdata);
    
    TrackJob::TrackJob()
    {
        m_userID="";
        m_projectID="";
        m_uploadUrl="";
        m_importUrl="";
        m_uploadZippath="";
        m_uploadZipfilename="";
        m_sToken="";
        m_sResponse="";
        m_curl=NULL;
        m_onPregress=NULL;
//        m_buffer = NULL;
//        m_zipfileSize = 0;
    }
    
    TrackJob::~TrackJob()
    {
//        if (m_buffer)
//        {
//            free(m_buffer);
//        }
    }
    
    int TrackJob::SyncTrackData2Server(const tagSyncTrackPara& syncPara)
    {
        CleanUnnecessary();
        m_userID = syncPara.sUserID;
        m_projectID = syncPara.sProjectID;
        m_sToken = syncPara.sToken;
        m_onPregress  = syncPara.pregress;
        //准备数据
        int ret = prepareUploadTrackData();
        if (-1==ret)
        {
            if (m_onPregress)
            {
                m_onPregress->OnProgress(Tools::GenerateProgressInfo(m_projectID, "准备轨迹数据失败！"), -1);
                return -1;
            }
        }
        //开启上传请求
        ret = uploadTrackData();
        return ret;
    }
    
    void TrackJob::CleanUnnecessary()
    {
        Editor::DataManager* dataManager = Editor::DataManager::getInstance();
        std::string trackUploadPath = dataManager->getFileSystem()->GetTrackUploadPath();
        Tools::DelDir(trackUploadPath.c_str());
    }
    
    int TrackJob::prepareUploadTrackData()
    {
        generateTrackUploadFilename();
        std::string sFilePath = generateTrackUploadFilename();
        if(sFilePath.empty()) return -1;
        DataManager* pDataManager = DataManager::getInstance();
        DataFileSystem* pDataFileSystem = pDataManager->getFileSystem();
        std::string sFolderPath = pDataFileSystem->GetTrackUploadPath();
        m_uploadZipfilename = trackzipfilename_head + "_" + m_userID + "_" + m_projectID + "_" + Tools::GetCurrentDateTime() + ".zip";
        std::string sZipName = sFolderPath + m_uploadZipfilename;
        m_uploadZippath = Tools::ZipFolderPath(sFolderPath.c_str(), sZipName.c_str());
        
//        FILE * pFile = NULL;
//        size_t result;
//        pFile = fopen (sZipName.c_str(), "rb" );
//        if (pFile==NULL)
//        {
//            fputs("File error",stderr);
//            Logger::LogD("File error");
//            Logger::LogO("File error");
//            return -1;
//        }
//        fseek (pFile , 0 , SEEK_END);
//        m_zipfileSize = ftell (pFile);
//        rewind (pFile);
//        if (m_buffer)
//        {
//            free(m_buffer);
//            m_buffer = NULL;
//        }
//        m_buffer = (char*) malloc (sizeof(char)*m_zipfileSize);
//        if (m_buffer == NULL)
//        {
//            fputs ("Memory error",stderr);
//            Logger::LogD("Memory error");
//            Logger::LogO("Memory error");
//            return -1;
//        }
//        result = fread (m_buffer,1,m_zipfileSize,pFile);
//        if (result != m_zipfileSize)
//        {
//            fputs ("Reading error",stderr);
//            Logger::LogD("Reading error");
//            Logger::LogO("Reading error");
//            return -1;
//        }
//        fclose(pFile);
        
        return 0;
    }
    
    int TrackJob::uploadTrackData()
    {
        CURL* curl = NULL;
        curl = CreateUploadRequest();
        if (!curl) return -1;
        return 0;
    }
    
    std::string TrackJob::generateTrackUploadFilename()
    {
        DataManager* pDataManager = DataManager::getInstance();
        DataFileSystem* pDataFileSystem = pDataManager->getFileSystem();
        pDataFileSystem->GetCoreMapDataPath();
        Editor::CoreMapDataSource* dataSource = dynamic_cast<Editor::CoreMapDataSource*>(pDataManager->getDataSource(DATASOURCE_COREMAP));
        
        std::string sFilename = "Datum_Track.json";
        
        std::string sPath = pDataFileSystem->GetTrackUploadPath() + sFilename;
        
        if(Tools::CheckFilePath(sPath.c_str()) == false)
        {
            return "";
        }
        Editor::DataManager* dataManager = Editor::DataManager::getInstance();
        Editor::DataFunctor* dataFunctor = dataManager->getDataFunctor();
        
        char buf[256];
        sprintf(buf, "select * , substr(%s,0,%d) as mydate from track_segment where UserId=%s and LOD=20 order by mydate",track_segment_StartTimeStr.c_str(), int(DATE_FORM0.size()+1), m_userID.c_str());
        std::string sql = buf;
        std::vector<Editor::DataFeature*> trackSegment = dataFunctor->QueryFeaturesBySql("track_segment", sql);
        std::string mydate = "";
        
        Logger::LogD("TrackJob::generateTrackUploadFilename sql[%s],trackSegment.size[%d]", sql.c_str(),trackSegment.size());
        Logger::LogO("TrackJob::generateTrackUploadFilename sql[%s],trackSegment.size[%d]", sql.c_str(),trackSegment.size());
        
        int mydateIndex=-1, geoIndex=-1, segmentIdIndex=-1,UserIdIndex=-1;
        std::map<std::string, tagTrackforServer> segid2track;
        for(int i=0; i< trackSegment.size(); ++i)
        {
            FieldDefines* fd = trackSegment.at(i)->GetFieldDefines();
            if(NULL== fd)
            {
                continue;
            }
            if (i==0)
            {
                mydateIndex = fd->GetColumnIndex("mydate");
                geoIndex = fd->GetColumnIndex(track_segment_GeometryStr);
                segmentIdIndex = fd->GetColumnIndex(track_segment_SegmentIdStr);
                UserIdIndex = fd->GetColumnIndex(track_segment_UserIdStr);
            }
            EditorGeometry::WkbGeometry* pGeometry = trackSegment.at(i)->GetAsWkb(geoIndex);
            geos::geom::Geometry* geo = DataTransfor::Wkb2Geo(pGeometry);
            if (geo) continue;
            std::string segid =  trackSegment.at(i)->GetAsString(segmentIdIndex);
            std::map<std::string, tagTrackforServer>::iterator itor = segid2track.find(segid);
            if (itor!=segid2track.end())
            {
                itor->second.trackGeos.push_back(geo);
            }
            else
            {
                tagTrackforServer temTrack;
                temTrack.trackGeos.push_back(geo);
                temTrack.strID = segid;
                temTrack.userID = m_userID;
                std::string date = trackSegment.at(i)->GetAsString(mydateIndex);
                temTrack.trackdate = date.substr(0, strlen(DATE_FORM0.c_str()));
                segid2track.insert(std::make_pair(segid, temTrack));
            }
        }
        std::string errmsg;
        std::ofstream out;
        out.open(sPath.c_str());
        if(!out)
        {
            Logger::LogD("open writefile error");
            Logger::LogO("open writefile error");
            errmsg = "open writefile error";
            return "";
        }
        
        Document document;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();
        
        std::map<std::string, tagTrackforServer>::iterator trackItor = segid2track.begin();
        rapidjson::Value stringValue(kStringType);
        while (trackItor!=segid2track.end())
        {
            stringValue.SetString(trackItor->second.strID.c_str(),trackItor->second.strID.size(),allocator);
            document.AddMember(StringRef(trackdata2server_idStr.c_str()), stringValue, allocator);
            document.AddMember(StringRef(trackdata2server_user_idStr.c_str()), Tools::StringToNum(trackItor->second.userID), allocator);
            stringValue.SetString(trackItor->second.trackdate.c_str(),trackItor->second.trackdate.size(),allocator);
            document.AddMember(StringRef(trackdata2server_track_dateStr.c_str()), stringValue, allocator);
            /// Construct a MultiLineString with a deep-copy of given arguments
            geos::geom::MultiLineString* multiLineString = geos::geom::GeometryFactory::getDefaultInstance()->createMultiLineString(trackItor->second.trackGeos);
            if(multiLineString)
            {
                ++trackItor;
                continue;
            }
            std::string wktstr = multiLineString->toString();
            stringValue.SetString(wktstr.c_str(),wktstr.size(),allocator);
            document.AddMember(StringRef(trackdata2server_user_trackStr.c_str()), stringValue, allocator);
            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            document.Accept(writer);
            std::string sResult = buffer.GetString();
            out<<sResult<<std::endl;
            geos::geom::GeometryFactory::getDefaultInstance()->destroyGeometry(multiLineString);
            for (int j=0; j<trackItor->second.trackGeos.size(); ++j)
            {
                if (trackItor->second.trackGeos[j])
                    geos::geom::GeometryFactory::getDefaultInstance()->destroyGeometry(trackItor->second.trackGeos[j]);
            }
            ++trackItor;
        }
        out.close();
        return sPath;
    }
    
    int TrackJob::importTrackData()
    {
//        if (m_buffer)
//        {
//            free(m_buffer);
//            m_buffer = NULL;
//            m_zipfileSize = 0;
//        }
        std::string param = GetUploadParameter(Tools::StringToNum(m_projectID), m_uploadZipfilename);
        std::string sExcuteUrl = Editor::DataManager::getInstance()->getServiceAddress()->GetTrackImportExcuteUrl(m_sToken, param);
        Logger::LogD("track import url is [%s]", sExcuteUrl.c_str());
        Logger::LogO("track import url is [%s]", sExcuteUrl.c_str());
        std::string sRespose = "";
        if(Tools::HttpGet(sExcuteUrl.c_str(), sRespose) == false)
        {
            Logger::LogD("insert into database failed after upload");
            Logger::LogO("insert into database failed after upload");
            
            Logger::LogO("track import response is [%s]", sRespose.c_str());
            
            return -1;
        }
        Logger::LogD("track import response is [%s]", sRespose.c_str());
        Logger::LogO("track import response is [%s]", sRespose.c_str());
        return 0;
    }
    
    void TrackJob::setUserID(const std::string& sUserID)
    {
        m_userID = sUserID;
    }
    
    void TrackJob::setProjectID(const std::string& sProjectID)
    {
        m_projectID = sProjectID;
    }
    
    void TrackJob::setUploadUrl(const std::string& sUrl)
    {
        m_uploadUrl = sUrl;
    }
    
    void TrackJob::setImportUrl(const std::string& sUrl)
    {
        m_importUrl = sUrl;
    }
    
    std::string TrackJob::getUserID()
    {
        return  m_userID;
    }
    std::string TrackJob::getProjectID()
    {
        return m_projectID;
    }
    std::string TrackJob::getUploadUrl()
    {
        return  m_uploadUrl;
    }
    
    std::string TrackJob::getImportUrl()
    {
        return  m_importUrl;
    }
    
    CURL* TrackJob::CreateUploadRequest()
    {
        m_uploadUrl = Editor::DataManager::getInstance()->getServiceAddress()->GetUploadTrackUrl();
        std::string param = GetUploadParameter(Tools::StringToNum(m_projectID), m_uploadZipfilename);
        CURLcode res;
        struct curl_httppost* formpost = NULL;
        struct curl_httppost* lastptr = NULL;
        
        Logger::LogD("track CreateUploadRequest url is [%s],m_uploadZippath[%s]", m_uploadUrl.c_str(),m_uploadZippath.c_str());
        Logger::LogO("track CreateUploadRequest url is [%s],m_uploadZippath[%s]", m_uploadUrl.c_str(),m_uploadZippath.c_str());
        
        curl_formadd(&formpost,
                     &lastptr,
                     CURLFORM_PTRNAME,
                     "access_token",
                     CURLFORM_PTRCONTENTS,
                     m_sToken.c_str(),
                     CURLFORM_END);
        
        curl_formadd(&formpost,
                     &lastptr,
                     CURLFORM_PTRNAME, "parameter",
                     CURLFORM_PTRCONTENTS, param.c_str(),
                     CURLFORM_END);
        curl_formadd(&formpost, &lastptr, CURLFORM_PTRNAME, "file", CURLFORM_FILE, m_uploadZippath.c_str(), CURLFORM_END);
        
//        curl_formadd(&formpost,
//                     &lastptr,
//                     CURLFORM_PTRNAME,
//                     "file",
//                     CURLFORM_BUFFER,
//                     m_uploadZipfilename.c_str(),
//                     CURLFORM_BUFFERPTR,
//                     m_buffer,
//                     CURLFORM_BUFFERLENGTH,
//                     m_zipfileSize,
//                     CURLFORM_END);
        
        m_curl = curl_easy_init();
        if(m_curl)
        {
            curl_easy_setopt(m_curl, CURLOPT_URL, m_uploadUrl.c_str());
            
            curl_easy_setopt(m_curl, CURLOPT_HTTPPOST, formpost);
            
            curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0);
            
            curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, OnProgressForTrackUpload);
            
            curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, this);
            
            curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1);
            
            curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 50); //wait for 50 seconds to connect to server
            
            curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 5000);
            
            curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, WriteServerResponse);
            curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_sResponse);
            
            res = curl_easy_perform(m_curl);
            
            if(res != CURLE_OK)
            {
                Logger::LogD("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                Logger::LogO("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
        }
        
        curl_easy_cleanup(m_curl);
        
        curl_formfree(formpost);
        
        return m_curl;
    }
    
    std::string TrackJob::GetUploadParameter(int projectId,const std::string& zipfileName)
    {
        Document document;
        
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
        
        document.SetObject();
        
        document.AddMember("projectId", projectId, allocator);
        
        rapidjson::Value stringValue(kStringType);
        stringValue.SetString(zipfileName.c_str(),zipfileName.size(),allocator);
        document.AddMember("fileName", stringValue, allocator);
        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        std::string res = buffer.GetString();
        
        return res;
    }
    
    size_t OnProgressForTrackUpload(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
    {
        Editor::TrackJob* pJob = (Editor::TrackJob*)clientp;
        
        if(pJob == NULL)
        {
            return 0;
        }
        
        Editor::ProgressEventer* pPregress = pJob->GetPregressObj();
        
//        if(pPregress == NULL)
//        {
//            return 0;
//        }
        
        double nProgress = 0.0;
        if (ultotal>0.0)
        {
            nProgress = 100*ulnow/ultotal;
        }
        else
        {
            return 0;
        }
        
        if (nProgress<=0.0)
        {
            return 0;
        }
        
        if(pPregress) pPregress->OnProgress(Tools::GenerateProgressInfo(pJob->getProjectID(), "轨迹上传"), nProgress);
        //轨迹数据开始入库
        if (ulnow==ultotal)
        {
            int ret = pJob->importTrackData();
            if(0==ret)
            {
                if(pPregress) pPregress->OnProgress(Tools::GenerateProgressInfo(pJob->getProjectID(), "轨迹入库"), 999);
            }
            else
            {
                if(pPregress) pPregress->OnProgress(Tools::GenerateProgressInfo(pJob->getProjectID(), "轨迹入库"), ret);
            }
            
        }
        return 0;
    }
    
    size_t WriteServerResponse(char* ptr, size_t size, size_t nmemb, void* userdata)
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


}
