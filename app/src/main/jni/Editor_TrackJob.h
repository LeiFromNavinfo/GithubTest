//
//  Editor_TrackJob.h
//  FMDataApi
//
//  Created by lidejun on 16/6/8.
//  Copyright © 2016年 Kevin Chou. All rights reserved.
//

#ifndef Editor_TrackJob_h
#define Editor_TrackJob_h
#include <string>
#include <vector>
#include "Editor.h"
#include "Geometry.h"

namespace Editor
{
    class TrackJob
    {
    public:
        TrackJob();
        virtual ~TrackJob();
//        
//        “id”:1,
//        "user_id":3655,
//        "track_date ":"2016-01-19 00:00:00 ",
//        "user_track":"MULTILINESTRING ((116.4788929999999993 40.0131280000000018, 116.4788609999999949 40.0131250000000023), (116.4790320000000037 40.0132630000000020, 116.4788810000000012 40.0130729999999986, 116.4790210000000030 40.0132040000000018), (116.4788259999999980 40.0131179999999986, 116.4790679999999981 40.0132359999999991))"
        
        /*
         * @brief 同步轨迹数据，注：这个对外使用
         *
         */
        int SyncTrackData2Server(const tagSyncTrackPara& syncPara);
        
        void CleanUnnecessary();
        
        /*
         * @brief 抽取本地当前用户下的轨迹数据
         *
         */
        int prepareUploadTrackData();
        
        /*
         * @brief 上传轨迹数据
         *
         */
        int uploadTrackData();
        
        /*
         * @brief 生成轨迹数据上传的文件全路径
         * filename form:track_3655_20160607174852.zip
         */
        std::string generateTrackUploadFilename();
        
        /*
         * @brief 轨迹数据导入mysql
         *
         */
        int importTrackData();
        
        void setUserID(const std::string& sUserID);
        void setProjectID(const std::string& sProjectID);
        void setUploadUrl(const std::string& sUploadUrl);
        void setImportUrl(const std::string& sImportUrl);
        
        std::string getUserID();
        std::string getProjectID();
        std::string getUploadUrl();
        std::string getImportUrl();
        Editor::ProgressEventer* GetPregressObj(){ return m_onPregress;}
        CURL* CreateUploadRequest();
        std::string GetUploadParameter(int projectId,const std::string& zipfileName);
        
        
    private:
        std::string m_userID;
        std::string m_projectID;
        std::string m_uploadUrl;
        std::string m_importUrl;
        
        std::string m_uploadZippath;
        std::string m_uploadZipfilename;
        std::string m_sToken;
        std::string m_sResponse;
        CURL* m_curl;
        Editor::ProgressEventer* m_onPregress;
        
//        char* m_buffer;
//        long m_zipfileSize;
    };
}


#endif /* Editor_TrackJob_h */
