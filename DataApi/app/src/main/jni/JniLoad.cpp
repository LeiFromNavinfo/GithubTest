#include <jni.h>
#include <stdio.h>
#include <assert.h>
#include "Logger.h"
#include "Geometry.h"
#include "Editor.h"
#include "Model.h"
#include "DataTransfor.h"
#include "Editor_ProjectManager.h"
#include "Editor_MetaDataManager.h"
#include "Editor_ResumeProcessor.h"
#include "Editor_APIWrapper.h"
#include "jniProgress.h"

#include "nds_sqlite3.h"
#include "spatialite.h"
#include "geos.h"
#include "document.h"
#include "stringbuffer.h"
#include "string.h"
#include "writer.h"
#include <sstream>


#define JNIREG_CLASS "com/fastmap/dataapi/DataNativeApi"

#define Platform "815"

static JavaVM *gs_jvm=NULL;
static jobject gs_object=NULL;

#ifdef __cplusplus
extern "C" {
#endif
    

	Editor::APIWrapper* GetApiWrapper()
	{
		Editor::APIWrapper* wrapper = NULL;

		if(std::strcmp(Platform,"815") ==0)
		{
			wrapper = Editor::APIWrapper815::getInstance();
		}
		else
		{
			wrapper = Editor::APIWrapper::getInstance();
		}
		return wrapper;
	}
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    InstallLayers
     * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z
     */
    JNIEXPORT jboolean JNICALL InstallLayers
    (JNIEnv *env, jclass obj, jstring sDownloadSqlite, jstring sExistsSqlite, jstring layers)
    {
        const char *sDownload = (env)->GetStringUTFChars(sDownloadSqlite, NULL);
        
        const char *sExist = (env)->GetStringUTFChars(sExistsSqlite, NULL);
        
        const char *layer = (env)->GetStringUTFChars(layers, NULL);
        
        std::string sDownloadFile(sDownload);
        
        std::string sExistFile(sExist);
        
        std::string tableName(layer);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        apiWrapper->InstallLayers(sDownloadFile, sExistFile, tableName);
        
        env->ReleaseStringUTFChars(sDownloadSqlite, sDownload);
        env->ReleaseStringUTFChars(sExistsSqlite, sExist);
        env->ReleaseStringUTFChars(layers, layer);
        
        return (jboolean)1;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    DownloadFile
     * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/util/ArrayList;ILcom/fastmap/dataapi/Progress;I)Z
     */
    JNIEXPORT jboolean JNICALL DownloadFile
    (JNIEnv *env, jclass obj, jstring sToken, jstring sUser, jobject idList, jint nType, jobject progressObj, jint nCmd)
    {
    	const char *sTokenStr = (env)->GetStringUTFChars(sToken, NULL);
    	const char *sUserStr = (env)->GetStringUTFChars(sUser, NULL);

        std::string sCurrentToken(sTokenStr);
        
        std::string sCurrentUser(sUserStr);
        
        JNIProgress* p = new JNIProgress();
        
        env->GetJavaVM(&gs_jvm);
        
        gs_object = env->NewGlobalRef(progressObj);
        
        jclass cls_arraylist = env->GetObjectClass(idList);
        
        jmethodID arraylist_get = env->GetMethodID(cls_arraylist,"get","(I)Ljava/lang/Object;");
        jmethodID arraylist_size = env->GetMethodID(cls_arraylist,"size","()I");
        jint size = env->CallIntMethod(idList,arraylist_size);
        
        std::vector<std::string> ids;
        
        for(int i=0 ; i<size ;i++)
        {
            jobject obj_user = env->CallObjectMethod(idList,arraylist_get,i);
            jclass cls_user = env->GetObjectClass(obj_user);
            
            jmethodID toStringMethod = env->GetMethodID(cls_user,"toString","()Ljava/lang/String;");
            
            jstring id = (jstring)env->CallObjectMethod(obj_user,toStringMethod);
            
            const char *idPtr = env->GetStringUTFChars(id, NULL);
            
            Logger::LogD("DownloadFile Function id :%s",idPtr);
            Logger::LogO("DownloadFile Function id :%s",idPtr);
            
            ids.push_back(idPtr);
        }
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        apiWrapper->DownloadFile(sCurrentToken, sCurrentUser, ids, nType, (Editor::Job::Cmd)nCmd, p);
        
        env->ReleaseStringUTFChars(sToken, sTokenStr);
        env->ReleaseStringUTFChars(sUser, sUserStr);

        return (jboolean)1;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    CancelDownload
     * Signature: ()Z
     */
    JNIEXPORT jboolean JNICALL CancelDownload
    (JNIEnv *env, jclass obj)
    {
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        apiWrapper->CancelDownload();
        
        return (jboolean)1;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    UploadFile
     * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/util/ArrayList;ILcom/fastmap/dataapi/Progress;I)Z
     */
    JNIEXPORT jboolean JNICALL UploadFile
    (JNIEnv *env, jclass obj, jstring sToken, jstring sUser, jobject idList, jint nType, jobject progressObj, jint nCmd, jstring deviceId)
    {
    	const char *sTokenStr = (env)->GetStringUTFChars(sToken, NULL);
    	const char *sUserStr = (env)->GetStringUTFChars(sUser, NULL);
    	const char *deviceIdStr = (env)->GetStringUTFChars(deviceId, NULL);

        std::string sCurrentToken(sTokenStr);
        std::string sCurrentUser(sUserStr);
        std::string sDeviceId(deviceIdStr);
        
        JNIProgress* p = new JNIProgress();
        
        env->GetJavaVM(&gs_jvm);
        
        gs_object = env->NewGlobalRef(progressObj);
        
        jclass cls_arraylist = env->GetObjectClass(idList);
        
        jmethodID arraylist_get = env->GetMethodID(cls_arraylist,"get","(I)Ljava/lang/Object;");
        jmethodID arraylist_size = env->GetMethodID(cls_arraylist,"size","()I");
        jint size = env->CallIntMethod(idList,arraylist_size);
        
        std::vector<std::string> ids;
        
        for(int i=0 ; i<size ;i++)
        {
            jobject obj_user = env->CallObjectMethod(idList,arraylist_get,i);
            jclass cls_user = env->GetObjectClass(obj_user);
            
            jmethodID toStringMethod = env->GetMethodID(cls_user,"toString","()Ljava/lang/String;");
            
            jstring id = (jstring)env->CallObjectMethod(obj_user,toStringMethod);
            
            const char *idPtr = env->GetStringUTFChars(id, NULL);
            
            ids.push_back(idPtr);
        }
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        apiWrapper->UploadFile(sCurrentToken, sCurrentUser, ids, nType, (Editor::JOBCMD)nCmd, p, sDeviceId);
        
        env->ReleaseStringUTFChars(sToken, sTokenStr);
        env->ReleaseStringUTFChars(sUser, sUserStr);
        env->ReleaseStringUTFChars(deviceId, deviceIdStr);

        return (jboolean)1;
    }
    
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    CancelUpload
     * Signature: ()Z
     */
    JNIEXPORT jboolean JNICALL CancelUpload
    (JNIEnv *env, jclass obj)
    {
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        apiWrapper->CancelUpload();
        
        return (jboolean)1;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    GetPoiPhotoPath
     * Signature: ()Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL GetPoiPhotoPath
    (JNIEnv *env, jclass obj)
    {
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::string sOut = apiWrapper->GetPoiPhotoPath();
        
        return (env)->NewStringUTF(sOut.c_str());
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    GetTipsPhotoPath
     * Signature: ()Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL GetTipsPhotoPath
    (JNIEnv *env, jclass obj)
    {
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::string sOut = apiWrapper->GetTipsPhotoPath();
        
        return (env)->NewStringUTF(sOut.c_str());
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    SelectMetaData
     * Signature: (Ljava/lang/String;Ljava/lang/String;I)Ljava/util/ArrayList;
     */
    JNIEXPORT jobject JNICALL SelectMetaData
    (JNIEnv *env, jclass obj, jstring table, jstring condition, jint type)
    {
    	const char *tableStr = (env)->GetStringUTFChars(table, NULL);

    	const char *conditionStr = (env)->GetStringUTFChars(condition, NULL);

        std::string sTable(tableStr);
        
        std::string sCondition(conditionStr);
        
        Logger::LogD("SelectMetaData==>>&s, &s", sTable.c_str(), sCondition.c_str());
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::vector<std::string> vStr = apiWrapper->SelectMetaData(sTable, sCondition, type);
        
        if(vStr.size() > 0)
        {
            Logger::LogD("SelectMetaData==>>%s", vStr[0].c_str());
        }
        
        jclass list_cls = env->FindClass("java/util/ArrayList");
        
        jmethodID list_costruct = env->GetMethodID(list_cls , "<init>","()V");
        
        jobject list_obj = env->NewObject(list_cls,list_costruct,"");
        
        jmethodID list_add  = env->GetMethodID(list_cls,"add","(Ljava/lang/Object;)Z");
        
        for(int i = 0; i < vStr.size(); i++)
        {
            jstring jstr = (env)->NewStringUTF(vStr[i].c_str());
            
            env->CallBooleanMethod(list_obj,list_add,jstr);
        }
        
        env->ReleaseStringUTFChars(table, tableStr);
        env->ReleaseStringUTFChars(condition, conditionStr);

        return list_obj;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    SnapPoint
     * Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/util/ArrayList
     */
    JNIEXPORT jobject JNICALL SnapPoint
    (JNIEnv *env, jobject, jstring box, jstring point)
    {
        Logger::LogD("Editor: SnapPoint Start");
        Logger::LogO("Editor: SnapPoint Start");
        const char *buffer = (env)->GetStringUTFChars(box, NULL);
        
        const char *sPoint = (env)->GetStringUTFChars(point, NULL);
        
        std::string param1(buffer);
        
        std::string param2(sPoint);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::vector<std::string>  points = apiWrapper->SnapPoint(param1, param2);
        
        if(points.size() == 0)
        {
            return NULL;
        }
        
        jclass list_cls = env->FindClass("java/util/ArrayList");
        
        if(list_cls == NULL)
        {
            Logger::LogD("list is Null ....");
            
            return NULL;
        }
        jmethodID list_costruct = env->GetMethodID(list_cls , "<init>","()V");
        
        jobject list_obj = env->NewObject(list_cls,list_costruct,"");
        
        jmethodID list_add  = env->GetMethodID(list_cls,"add","(Ljava/lang/Object;)Z");
        
        for(int i=0; i<points.size(); i++)
        {
            jstring resultJstring = (env)->NewStringUTF(points[i].c_str());
            
            env->CallBooleanMethod(list_obj,list_add,resultJstring);
        }
        
        env->ReleaseStringUTFChars(box, buffer);
        env->ReleaseStringUTFChars(point, sPoint);
        
        return list_obj;
    }
    
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    SnapLine
     * Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL SnapLine
    (JNIEnv *env, jobject obj, jstring box, jstring point)
    {
        const char *buffer = (env)->GetStringUTFChars(box, NULL);
        
        const char *sPoint = (env)->GetStringUTFChars(point, NULL);
        
        std::string param1(buffer);
        
        std::string param2(sPoint);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::string result= apiWrapper->SnapLine(param1, param2);
        
        env->ReleaseStringUTFChars(box, buffer);
        env->ReleaseStringUTFChars(point, sPoint);
        
        return (env)->NewStringUTF(result.c_str());
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    SelectPois
     * Signature: (Ljava/lang/String;)Ljava/util/ArrayList
     */
    JNIEXPORT jobject JNICALL SelectPois
    (JNIEnv *env, jobject job, jstring box)
    {
        Logger::LogD("Enter function....");
        
        jclass list_cls = env->FindClass("java/util/ArrayList");
        
        if(list_cls == NULL)
        {
            Logger::LogD("list is Null ....");
            return NULL;
        }
        
        jmethodID list_costruct = env->GetMethodID(list_cls , "<init>","()V");
        
        jobject list_obj = env->NewObject(list_cls,list_costruct,"");
        
        jmethodID list_add  = env->GetMethodID(list_cls,"add","(Ljava/lang/Object;)Z");
        
        const char *buffer = (env)->GetStringUTFChars(box, NULL);
        
        std::string param1(buffer);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::vector<Editor::DataFeature*> pois = apiWrapper->SelectPois(param1);
        
        for(int i = 0 ; i< pois.size(); i++)
        {
            Editor::JSON json = pois[i]->GetSnapshotPart();
            
            std::string result = json.GetJsonString();
            
            Logger::LogD(result.c_str());
            
            jstring resultJstring = (env)->NewStringUTF(result.c_str());
            
            env->CallBooleanMethod(list_obj,list_add,resultJstring);
            
            delete pois[i];
        }
        
        env->ReleaseStringUTFChars(box, buffer);
        
        return list_obj;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    GetTotalPoiByFid
     * Signature: (Ljava/lang/String;)Ljava/lang/String
     */
    JNIEXPORT jstring JNICALL GetTotalPoiByFid
    (JNIEnv *env, jobject job, jstring fid)
    {
        Logger::LogD("Enter function....");
        
        const char *buffer = (env)->GetStringUTFChars(fid, NULL);
        
        std::string fidStr(buffer);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::string result = apiWrapper->GetTotalPoiByFid(fidStr);
        
        jstring resultJstring = (env)->NewStringUTF(result.c_str());
        
        env->ReleaseStringUTFChars(fid, buffer);
        
        return resultJstring;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    InsertPoi
     * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I
     */
    JNIEXPORT jint JNICALL InsertPoi
    (JNIEnv *env, jclass job, jstring json, jstring userid, jstring projectid)
    {
        try{
            const char *sUserId = (env)->GetStringUTFChars(userid, NULL);
            
            const char *sProjectId = (env)->GetStringUTFChars(projectid, NULL);
            
            const char *buffer = (env)->GetStringUTFChars(json, NULL);
            
            std::string jsonStr(buffer);
            
            Editor::APIWrapper* apiWrapper = GetApiWrapper();
            
            int result = apiWrapper->InsertPoi(jsonStr,sUserId, sProjectId);
            
            env->ReleaseStringUTFChars(userid, sUserId);
            env->ReleaseStringUTFChars(projectid, sProjectId);
            env->ReleaseStringUTFChars(json, buffer);
            
            return result;
        }
        catch(...){
            Logger::LogD("Editor: Insert Poi failed In JNI");
            Logger::LogO("Editor: Insert Poi failed In JNI");
            return -1;
        }
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    UpdatePoi
     * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I
     */
    JNIEXPORT jint JNICALL UpdatePoi
    (JNIEnv *env, jclass job, jstring json, jstring userid, jstring projectid)
    {
        try{
            const char *sUserId = (env)->GetStringUTFChars(userid, NULL);
            
            const char *sProjectId = (env)->GetStringUTFChars(projectid, NULL);
            
            const char *buffer = (env)->GetStringUTFChars(json, NULL);
            
            std::string jsonStr(buffer);
            
            Editor::APIWrapper* apiWrapper = GetApiWrapper();
            
            int result = apiWrapper->UpdatePoi(jsonStr, sUserId, sProjectId);
            
            env->ReleaseStringUTFChars(userid, sUserId);
            env->ReleaseStringUTFChars(projectid, sProjectId);
            env->ReleaseStringUTFChars(json, buffer);
            
            return result;
        }
        catch(...){
            Logger::LogD("Editor: Update Poi failed In JNI");
            Logger::LogO("Editor: Update Poi failed In JNI");
            return -1;
        }
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    UpdatePois
     * Signature: (Ljava/util/ArrayList;Ljava/lang/String;Ljava/lang/String;Ljava/util/ArrayList;Lcom/fastmap/dataapi/Progress;)I
     */
    JNIEXPORT jint JNICALL UpdatePois
    (JNIEnv *env, jclass job, jobject jsonList, jstring userid, jstring projectid, jobject failIndexList, jobject progressObj)
    {
        try{
            const char *sUserId = (env)->GetStringUTFChars(userid, NULL);
            
            const char *sProjectId = (env)->GetStringUTFChars(projectid, NULL);
            
            JNIProgress* p = new JNIProgress();
            
            env->GetJavaVM(&gs_jvm);
            
            gs_object = env->NewGlobalRef(progressObj);
            
            jclass cls_arraylist = env->GetObjectClass(jsonList);
            jmethodID arraylist_get = env->GetMethodID(cls_arraylist,"get","(I)Ljava/lang/Object;");
            jmethodID arraylist_size = env->GetMethodID(cls_arraylist,"size","()I");
            jint size = env->CallIntMethod(jsonList,arraylist_size);
            
            std::list<std::string> jsons;
            
            for(int i=0 ; i<size ;i++)
            {
                jobject obj_user = env->CallObjectMethod(jsonList,arraylist_get,i);
                jclass cls_user = env->GetObjectClass(obj_user);
                jmethodID toStringMethod = env->GetMethodID(cls_user,"toString","()Ljava/lang/String;");
                jstring json = (jstring)env->CallObjectMethod(obj_user,toStringMethod);
                const char *jsonPtr = env->GetStringUTFChars(json, NULL);
                jsons.push_back(jsonPtr);
            }
            
            Editor::APIWrapper* apiWrapper = GetApiWrapper();
            
            std::list<int> failIndexs;
            int result = apiWrapper->UpdatePois(jsons, sUserId, sProjectId, failIndexs, p);
            
            if(failIndexs.size())
            {
                jclass integer_cls = env->FindClass("Ljava/lang/Integer;");
                jmethodID integer_costruct = env->GetMethodID(integer_cls , "<init>", "(I)V");
                jmethodID arrayList_add = env->GetMethodID(cls_arraylist,"add","(Ljava/lang/Object;)Z");
                
                for(std::list<int>::iterator itor = failIndexs.begin(); itor!=failIndexs.end(); ++itor)
                {
                    jobject integer_obj = env->NewObject(integer_cls, integer_costruct, *itor);
                    env->CallObjectMethod(failIndexList,arrayList_add,integer_obj);
                }
                
            }
            env->ReleaseStringUTFChars(userid, sUserId);
            env->ReleaseStringUTFChars(projectid, sProjectId);
            
            return result;
        }
        catch(...){
            Logger::LogD("Editor: Update Pois failed In JNI");
            Logger::LogO("Editor: Update Pois failed In JNI");
            return -1;
        }
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    DeletePoi
     * Signature: (ILjava/lang/String;Ljava/lang/String;)I
     */
    JNIEXPORT jint JNICALL DeletePoi
    (JNIEnv *env, jclass job, jint rowId, jstring userid, jstring projectid)
    {
        try{
            
            const char *sUserId = (env)->GetStringUTFChars(userid, NULL);
            
            const char *sProjectId = (env)->GetStringUTFChars(projectid, NULL);
            
            Editor::APIWrapper* apiWrapper = GetApiWrapper();
            
            int result = apiWrapper->DeletePoi(rowId, sUserId, sProjectId);
            
            env->ReleaseStringUTFChars(userid, sUserId);
            env->ReleaseStringUTFChars(projectid, sProjectId);
            
            return result;
        }
        catch(...){
            Logger::LogD("Editor: Delete Poi failed In JNI");
            Logger::LogO("Editor: Delete Poi failed In JNI");
            return -1;
        }
    }
    
    /*
     * Class:     com_fastmapsdk_dataapi_DataNativeApi
     * Method:    SetRootPath
     * Signature: (Ljava/lang/String;)V
     */
    JNIEXPORT void JNICALL SetRootPath
    (JNIEnv *env, jobject, jstring rootPath){
        const char *root = (env)->GetStringUTFChars(rootPath, NULL);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        apiWrapper->SetRootPath(root);

        env->ReleaseStringUTFChars(rootPath, root);
    }
    
    /*
     * Class:     com_fastmapsdk_dataapi_DataNativeApi
     * Method:    SetUserId
     * Signature: (Ljava/lang/String;)V
     */
    JNIEXPORT void JNICALL SetUserId
    (JNIEnv *env, jobject, jstring userId){
        const char *currentUser = (env)->GetStringUTFChars(userId, NULL);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        apiWrapper->SetUserId(currentUser);

        env->ReleaseStringUTFChars(userId, currentUser);
    }

    int sqlite3_callback_fillbundle(void* para, int n_column, char** column_value, char** column_name)
    {
        for (int i=0; i<n_column; i++)
        {
            Logger::LogD("column_name:%s,%s", column_name[i], column_value[i]);
        }

        return 0;	//non-zero
    }

    bool ExecuteNonQuery(const char* sql, sqlite3* db)
    {
        char* pszErrMsg;

        int rc = sqlite3_exec(db, sql, sqlite3_callback_fillbundle, NULL, &pszErrMsg);

        if (rc != SQLITE_OK)
        {
//            Logger::LogD("ExecuteNoQuery failed:%s", pszErrMsg);

            sqlite3_free(pszErrMsg);

            return false;
        }

        return true;
    }

    void errorLogCallback(void *pArg, int iErrCode, const char *zMsg)
    {
        Logger::LogD("sqlite_tmpdir:%s, tmpdir:%s", getenv("SQLITE_TMPDIR"), getenv("TMPDIR"));

        system("ls . > /storage/sdcard0/0.txt");

        Logger::LogD("errorLogCallback, (%d) %s", iErrCode, zMsg);
    }

    void CreateSpatialite()
    {
        int rc = sqlite3_config(SQLITE_CONFIG_LOG, errorLogCallback, NULL);

        Logger::LogD("rc:%d", rc);

        std::string file = "file:/storage/extSdCard/test.sqlite?zv=zlib&password=ELytUVOx2e6CIBCb";

        ::remove("/storage/extSdCard/test.sqlite");

        sqlite3* db = NULL;

        rc = sqlite3_open_v2(file.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

        Logger::LogD("sqlite3_open_v2, rc:%d, msg:%s", rc, sqlite3_errmsg(db));

        void* cache = spatialite_alloc_connection();

        spatialite_init_ex(db, cache, 1);

        std::string createSql = "create table test(id int32 PRIMARY KEY, name text)";

        std::string addGeoSql = "select AddGeometryColumn('test', 'geometry', 4326, 'GEOMETRY', 'XY')";

        //ExecuteNonQuery("PRAGMA temp_store_directory = '/storage/extSdCard'", db);

        ExecuteNonQuery("PRAGMA temp_store_directory = '/storage/extSdCard'", db);

        ExecuteNonQuery("SELECT InitSpatialMetaData(1)", db);

        ExecuteNonQuery("BEGIN", db);



        ExecuteNonQuery(createSql.c_str(), db);

        ExecuteNonQuery(addGeoSql.c_str(), db);

        ExecuteNonQuery("COMMIT", db);

        //insert
        ExecuteNonQuery("PRAGMA synchronous = OFF", db);

        ExecuteNonQuery("PRAGMA journal_mode = MEMORY", db);

        ExecuteNonQuery("PRAGMA cache_size=10000", db);

        ExecuteNonQuery("BEGIN", db);

        ExecuteNonQuery("SELECT DisableSpatialIndex('test', 'geometry');", db);

        ExecuteNonQuery("SELECT DiscardGeometryColumn('test', 'geometry');", db);

        sqlite3_stmt* stmt;

        std::string insertSql = "insert into test values("
                "?, ?, ?)";

        rc = sqlite3_prepare_v2(db, insertSql.c_str(), -1, &stmt, NULL);

        if (rc != SQLITE_OK)
        {
            Logger::LogD("insert failed [%s]", sqlite3_errmsg(db));
            Logger::LogO("insert failed [%s]", sqlite3_errmsg(db));
        }

        int id = 1;

        std::string name = "name";

        double longitude = 116.39;

        double latitude = 39.39;

        sqlite3_bind_int(stmt, 1, id);

        sqlite3_bind_text(stmt, 2, name.c_str(), name.length(), NULL);

        EditorGeometry::WkbPoint* wkb=new EditorGeometry::WkbPoint();
        wkb->_byteOrder = 1;
        wkb->_wkbType = (EditorGeometry::WkbGeometryType)1;
        wkb->_point._x = longitude;
        wkb->_point._y = latitude;

        EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

        sqlite3_bind_blob(stmt, 3, spGeo, wkb->buffer_size()+39, NULL);

        delete wkb;

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            Logger::LogD("sqlite3_step[%s][%s]", insertSql.c_str(), sqlite3_errmsg(db));
            Logger::LogO("sqlite3_step[%s][%s]", insertSql.c_str(), sqlite3_errmsg(db));
        }

        free(spGeo);

        spGeo = NULL;

        sqlite3_reset(stmt);

        int id2 = 2;

        std::string name2 = "name2";

        double longitude2 = 117.39;

        double latitude2 = 38.39;

        sqlite3_bind_int(stmt, 1, id2);

        sqlite3_bind_text(stmt, 2, name2.c_str(), name2.length(), NULL);

        EditorGeometry::WkbPoint* wkb2=new EditorGeometry::WkbPoint();
        wkb2->_byteOrder = 1;
        wkb2->_wkbType = (EditorGeometry::WkbGeometryType)1;
        wkb2->_point._x = longitude2;
        wkb2->_point._y = latitude2;

        EditorGeometry::SpatialiteGeometry* spGeo2 = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb2);

        sqlite3_bind_blob(stmt, 3, spGeo2, wkb2->buffer_size()+39, NULL);

        delete wkb2;

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            Logger::LogD("sqlite3_step2[%s][%s]", insertSql.c_str(), sqlite3_errmsg(db));
            Logger::LogO("sqlite3_step2[%s][%s]", insertSql.c_str(), sqlite3_errmsg(db));
        }

        free(spGeo2);

        spGeo2 = NULL;

        sqlite3_finalize(stmt);

        ExecuteNonQuery("SELECT RecoverGeometryColumn('test', 'geometry', 4326, 'GEOMETRY', 'XY');", db);

        ExecuteNonQuery("SELECT CreateSpatialIndex('test','geometry')", db);

        ExecuteNonQuery("SELECT RecoverSpatialIndex('test', 'geometry', 1);", db);

        ExecuteNonQuery("COMMIT", db);

        spatialite_cleanup_ex(cache);

        cache = NULL;

        spatialite_shutdown();

        rc = sqlite3_close_v2(db);

        Logger::LogD("sqlite3_close_v2, rc:%d", rc);
    }

    void SelectSpatialite()
    {
        int rc = sqlite3_config(SQLITE_CONFIG_LOG, errorLogCallback, NULL);

        Logger::LogD("rc:%d", rc);

        std::string file = "file:/storage/extSdCard/test.sqlite?zv=zlib&password=ELytUVOx2e6CIBCb";

        sqlite3* db = NULL;

        rc = sqlite3_open_v2(file.c_str(), &db, SQLITE_OPEN_READWRITE, NULL);

        Logger::LogD("sqlite3_open_v2, rc:%d, msg:%s", rc, sqlite3_errmsg(db));

        void* cache = spatialite_alloc_connection();

        spatialite_init_ex(db, cache, 1);

        std::string selectSql = "select * from test";

        //ExecuteNonQuery("PRAGMA temp_store_directory = '/storage/extSdCard'", db);

        ExecuteNonQuery("PRAGMA temp_store_directory = '/storage/extSdCard'", db);

        ExecuteNonQuery("SELECT InitSpatialMetaData(1)", db);

        ExecuteNonQuery(selectSql.c_str(), db);

        spatialite_cleanup_ex(cache);

        cache = NULL;

        spatialite_shutdown();

        rc = sqlite3_close_v2(db);

        Logger::LogD("sqlite3_close_v2, rc:%d", rc);
    }

    void Install()
    {
        Logger::LogD("install begin:");
        Logger::LogO("install begin:");

        //Tools::WatchTimerStart();

        Editor::DataManager* dataManager = Editor::DataManager::getInstance();

        Editor::CoreMapDataSource* dataSource = dynamic_cast<Editor::CoreMapDataSource*>(dataManager->getDataSource(0));

        int ret = dataSource->InstallPoi("/storage/sdcard0/json2.zip", "projectId", 0, NULL);

//        long cost = Tools::WatchTimerStop();
//
//        Logger::LogD("cost is [%ld]", cost);
//        Logger::LogO("cost is [%ld]", cost);

        Logger::LogD("install end, ret:%d", ret);
        Logger::LogO("install end, ret:%d", ret);
    }

    std::string GetAsString(sqlite3_stmt *statement, unsigned int index)
    {
        int length;

        const unsigned char* buff;

        const void* buff2;

        length = sqlite3_column_bytes(statement, index);

        buff = sqlite3_column_text(statement, index);

        buff2 = sqlite3_column_blob(statement, index);

        std::string ret = "";

        std::string ret2 = "";

        if (0 != length)
        {
            ret = std::string((char*)buff, length);

            ret2 = std::string((char*)buff2, length);

            Logger::LogD("index:%d,length:%d,%d,%d,%d,%d", index, length, strlen((char*)buff), strlen((char*)buff2), ret.length(), ret2.length());
        }

        return ret;
    }

    void Test()
    {
        std::string file = "/storage/sdcard0/coremap(8).sqlite";

        sqlite3* db = NULL;

        int rc = sqlite3_open_v2(file.c_str(), &db, SQLITE_OPEN_READWRITE, NULL);

        Logger::LogD("sqlite3_open_v2, rc:%d, msg:%s", rc, sqlite3_errmsg(db));

        void* cache = spatialite_alloc_connection();

        spatialite_init_ex(db, cache, 1);

        std::string sql = "select geometry from edit_pois where fid='0010080301WZ200016'";

        sqlite3_stmt* stmt;

        rc = sqlite3_prepare(db, sql.c_str(), -1, &stmt, NULL);

        if (rc != SQLITE_OK)
        {
            Logger::LogD("sql [%s] failed [%s]", sql.c_str(), sqlite3_errmsg(db));
            Logger::LogO("sql [%s] failed [%s]", sql.c_str(), sqlite3_errmsg(db));

            sqlite3_close_v2(db);

            return;
        }

        rc = sqlite3_step(stmt);

        while (rc == SQLITE_ROW)
        {
            //std::string names = GetAsString(stmt, 0);

            //std::string edits = GetAsString(stmt, 1);

            int geo_length = sqlite3_column_bytes(stmt, 0);
            const void* geo_buff = sqlite3_column_blob(stmt, 0);
            EditorGeometry::WkbPoint* wkb = (EditorGeometry::WkbPoint*)EditorGeometry::SpatialiteGeometry::ToWKBGeometry((EditorGeometry::SpatialiteGeometry*)geo_buff);
            double longitude = wkb->_point._x;
            double latitude = wkb->_point._y;


            Logger::LogD("ww:%f,%f", longitude, latitude);

            std::stringstream ss;
            ss.precision(8);
            ss << "Pont";
            ss << longitude;
            ss << " ";
            ss << latitude;

            Logger::LogD("ss:%s", ss.str().c_str());

            std::string wkt = DataTransfor::Wkb2Wkt(wkb);

            Logger::LogD("wkt:%s", wkt.c_str());
            free(wkb);


            //Logger::LogD("name:%s, edits:%s", names.c_str(), edits.c_str());

            rc = sqlite3_step(stmt);
        }

        sqlite3_finalize(stmt);

        spatialite_cleanup_ex(cache);

        cache = NULL;

        spatialite_shutdown();

        rc = sqlite3_close_v2(db);

        Logger::LogD("sqlite3_close_v2, rc:%d", rc);
    }

    void Test2()
    {
        std::string temp = "/storage";

        std::string temp2 = "/storage/sdcard0";

        std::string temp3 = "/storage/emulated/0";

        std::string temp4 = "/storage/emulated";

        int ret1 = access(temp.c_str(), R_OK);

        int ret2 = access(temp2.c_str(), R_OK);

        int ret3 = access(temp3.c_str(), R_OK);

        int ret4 = access(temp4.c_str(), R_OK);

        Logger::LogD("ret:%d,%d,%d,%d", ret1, ret2, ret3, ret4);
    }


 //基类
 class CBase
 {
     public:
             int a;
     public:
//     CBase()
//     {}

           CBase(int na)
             {
                   a=na;
                 Logger::LogD("CBase constructor! ");
         }

            virtual ~CBase()
            {
                Logger::LogD("CBase deconstructor! ");
            }

          void Test1()
          {
              Logger::LogD("CBase Test1,%d", a);
          }

         virtual void Test2()
         {
             Logger::LogD("CBase Test2,%d", a);
         }

        void Test3()
        {
            Logger::LogD("CBase Test3,%d", a);
        }
 };

class CBase2 : virtual public CBase
{
public:
    int b;

public:

    CBase2(int na):CBase(na)
    {
        Logger::LogD("CBase2 constructor! ");
    }

    virtual ~CBase2()
    {
        Logger::LogD("CBase2 deconstructor! ");
    }

    void Test1()
    {
        Logger::LogD("CBase2 Test1,%d,%d", a, b);
    }

    virtual void Test2()
    {
        Logger::LogD("CBase2 Test2,%d,%d", a, b);
    }

};


class CBase3 : virtual public CBase2
{
public:
    int c;

public:

    CBase3(int na):CBase2(na),CBase(na)
    {
        Logger::LogD("CBase3 constructor! ");
    }

    virtual ~CBase3()
    {
        Logger::LogD("CBase3 deconstructor! ");
    }

    void Test1()
    {
        Logger::LogD("CBase3 Test1,%d,%d,%d", a, b, c);
    }

    virtual void Test2()
    {
        Logger::LogD("CBase3 Test2,%d,%d,%d", a, b, c);
    }

};

 class UCBase
 {
     protected:
             int a;
     public:
           UCBase(int na)
             {
                   a=na;
                 Logger::LogD("UCBase constructor! ");
         }

     ~UCBase()
     {
         Logger::LogD("UCBase deconstructor! ");
     }
 };

 //派生类1(声明CBase为虚基类)
 class CDerive1:virtual public CBase
 {
     public:
         CDerive1(int na):CBase(na)
         {
             Logger::LogD("CDerive1 constructor! ");
             }

         ~CDerive1(){
             Logger::LogD("CDerive1 deconstructor! ");
         }

         int GetA(){return a;}
     };

 class UCDerive1: public UCBase
 {
     public:
         UCDerive1(int na):UCBase(na)
         {
             Logger::LogD("UCDerive1 constructor! ");
             }

         ~UCDerive1(){
             Logger::LogD("UCDerive1 deconstructor! ");
         }

         int GetA(){return a;}
     };

 //派生类2(声明CBase为虚基类)
 class CDerive2:virtual public CBase
         {
     public:
         CDerive2(int na):CBase(na)
         {
             Logger::LogD("CDerive2 constructor! ");
             }
         ~CDerive2(){ Logger::LogD("CDerive2 deconstructor! ");}
         int GetA(){return a;}
     };

 class UCDerive2:public UCBase
 {
    public:
        UCDerive2(int na):UCBase(na)
         {
             Logger::LogD("UCDerive2 constructor! ");
             }
     ~UCDerive2(){ Logger::LogD("UCDerive2 deconstructor! ");}
     int GetA(){return a;}
 };

 //子派生类
 class CDerive12:public CDerive1,public CDerive2
        {
     public:
         CDerive12(int na1,int na2,int na3):CDerive1(na1),CDerive2(na2),CBase(na3)
         {
             Logger::LogD("CDerive12 constructor! ");
             }
         ~CDerive12(){ Logger::LogD("CDerive12 deconstructor! ");}
     };

 class UCDerive12:public UCDerive1,public UCDerive2,public UCBase
         {
     public:
         UCDerive12(int na1,int na2,int na3):UCDerive1(na1),UCDerive2(na2),UCBase(na3)
         {
             Logger::LogD("UCDerive12 constructor! ");
         }
        ~UCDerive12(){ Logger::LogD("UCDerive12 deconstructor! ");}
     };

 void Test3()
 {
//     CBase b(100);

//     b.a = 300;

     CBase* b2 = new CBase2(200);

     CBase2* b3 = dynamic_cast<CBase2*>(b2);

     b3->b = 500;

     //b2->b = 500;

//     b.Test1();
//
//     b.Test2();
//
//     b.Test3();

     b2->Test1();

     b2->Test2();

     b2->Test3();

     Logger::LogD("%d,%d,%d", b2->a, b3->a, b3->b);

     delete b2;

     CBase* b4 = new CBase3(300);

     CBase3* b5 = dynamic_cast<CBase3*>(b4);

     b5->b = 600;

     b5->c = 700;

     b4->Test1();

     b4->Test2();

     b4->Test3();

     delete b4;

//     CDerive12 obj(100,200,300);
//     //得到从CDerive1继承的值
//     Logger::LogD(" from CDerive1 : a------->%d", obj.CDerive1::GetA());
//     //得到从CDerive2继承的值
//     Logger::LogD(" from CDerive2 : a-------> %d", obj.CDerive2::GetA());

//     UCDerive12 uobj(100,200,300);
//     //得到从CDerive1继承的值
//     Logger::LogD(" from UCDerive1 : a------->%d", uobj.UCDerive1::GetA());
//     //得到从CDerive2继承的值
//     Logger::LogD(" from UCDerive2 : a------->%d", uobj.UCDerive2::GetA());

     return;
 }


/*
 * Class:     com_fastmapsdk_dataapi_DataNativeApi
 * Method:    GetDataPath
 * Signature: ()Ljava/lang/String;
 */
    JNIEXPORT jstring JNICALL GetDataPath
    (JNIEnv *env, jobject){
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::string dataPath = apiWrapper->GetDataPath();

        //sqlite3_initialize();

        //CreateSpatialite();

        //SelectSpatialite();

        //sqlite3_shutdown();

        //int ret = 0;

        Test();

        //Test3();

//        Install();

//        boost::shared_ptr<boost::thread> m_thread;
//
//        m_thread.reset(new boost::thread(boost::bind(&Install)));

//        Editor::DataManager* dataManager = Editor::DataManager::getInstance();
//
//        Editor::CoreMapDataSource* dataSource = dynamic_cast<Editor::CoreMapDataSource*>(dataManager->getDataSource(0));
//
//        int ret = dataSource->InstallPoi("/storage/sdcard0/json2.zip", "projectId", 0, NULL);

        return (env)->NewStringUTF(dataPath.c_str());
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    SelectTips
     * Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/util/ArrayList
     */
    JNIEXPORT jobject JNICALL SelectTips
    (JNIEnv *env, jobject job, jstring box, jstring point)
    {
        jclass list_cls = env->FindClass("java/util/ArrayList");
        
        if(list_cls == NULL)
        {
            Logger::LogD("list is Null ....");
            return NULL;
        }
        
        jmethodID list_costruct = env->GetMethodID(list_cls , "<init>","()V");
        
        jobject list_obj = env->NewObject(list_cls,list_costruct,"");
        
        jmethodID list_add  = env->GetMethodID(list_cls,"add","(Ljava/lang/Object;)Z");
        
        const char *buffer = (env)->GetStringUTFChars(box, NULL);
        
        std::string param1(buffer);
        
        const char *sPoint = (env)->GetStringUTFChars(point, NULL);
        
        std::string param2(sPoint);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::vector<Editor::DataFeature*> selectedTips = apiWrapper->SelectTips(param1, param2);
        
        Logger::LogD("vector size", selectedTips.size());
        
        for(int i = 0; i< selectedTips.size(); i++)
        {
            Editor::JSON json = selectedTips[i]->GetTotalPart();
            
            std::string result = json.GetJsonString();
            
            Logger::LogD(result.c_str());
            
            jstring resultJstring = (env)->NewStringUTF(result.c_str());
            
            env->CallBooleanMethod(list_obj,list_add,resultJstring);
            
            delete selectedTips[i];
        }
        
        env->ReleaseStringUTFChars(box, buffer);
        env->ReleaseStringUTFChars(point, sPoint);
        
        return list_obj;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    InsertTips
     * Signature: (Ljava/lang/String;)I
     */
    JNIEXPORT jint JNICALL InsertTips
    (JNIEnv *env, jclass job, jstring json){
        try{
            const char *buffer = (env)->GetStringUTFChars(json, NULL);
            
            std::string jsonStr(buffer);
            
            Editor::APIWrapper* apiWrapper = GetApiWrapper();
            
            int result = apiWrapper->InsertTips(jsonStr);
            
            env->ReleaseStringUTFChars(json, buffer);
            
            return result;
        }
        catch(...){
            Logger::LogD("Editor: Insert Tips failed In JNI");
            Logger::LogO("Editor: Insert Tips failed In JNI");
            return -1;
        }
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    UpdateTips
     * Signature: (Ljava/lang/String;)I
     */
    JNIEXPORT jint JNICALL UpdateTips
    (JNIEnv *env, jclass job, jstring json){
        try{
            const char *buffer = (env)->GetStringUTFChars(json, NULL);
            
            std::string jsonStr(buffer);
            
            Editor::APIWrapper* apiWrapper = GetApiWrapper();
            
            int result = apiWrapper->UpdateTips(jsonStr);
            
            env->ReleaseStringUTFChars(json, buffer);
            
            return result;
        }
        catch(...){
            Logger::LogD("Editor: Update Tips failed In JNI");
            Logger::LogO("Editor: Update Tips failed In JNI");
            return -1;
        }
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    GetRdLineByPid
     * Signature: (I)Ljava/lang/String
     */
    JNIEXPORT jstring JNICALL GetRdLineByPid
    (JNIEnv *env, jobject job, jint pid)
    {
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::string result = apiWrapper->GetRdLineByPid(pid);
        
        jstring resultJstring = (env)->NewStringUTF(result.c_str());
        
        return resultJstring;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    GetCountByCondition
     * Signature: (Ljava/lang/String;I)I
     */
    JNIEXPORT jint JNICALL GetCountByCondition
    (JNIEnv *env, jclass obj, jstring table, jint type)
    {
        const char *sTable = (env)->GetStringUTFChars(table, NULL);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        int result = apiWrapper->GetCountByCondition(sTable, type);
        
        env->ReleaseStringUTFChars(table, sTable);
        
        return result;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    GetSignalLayerByCondition
     * Signature: (Ljava/lang/String;III)Ljava/util/ArrayList;
     */
    JNIEXPORT jobject JNICALL GetSignalLayerByCondition
    (JNIEnv *env, jclass obj, jstring table, jint type, jint pagesize, jint offset)
    {
        const char *sTable = (env)->GetStringUTFChars(table, NULL);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::vector<std::string> vStr = apiWrapper->GetSignalLayerByCondition(sTable, type, pagesize, offset);
        
        jclass list_cls = env->FindClass("java/util/ArrayList");
        
        jmethodID list_costruct = env->GetMethodID(list_cls , "<init>","()V");
        
        jobject list_obj = env->NewObject(list_cls,list_costruct,"");
        
        jmethodID list_add  = env->GetMethodID(list_cls,"add","(Ljava/lang/Object;)Z");
        
        for(int i = 0; i < vStr.size(); i++)
        {
            jstring jstr = (env)->NewStringUTF(vStr[i].c_str());
            
            env->CallBooleanMethod(list_obj,list_add,jstr);
        }
        
        env->ReleaseStringUTFChars(table, sTable);
        
        return list_obj;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    InitVerstionInfo
     * Signature: (I)Z
     */
    JNIEXPORT jboolean JNICALL InitVerstionInfo
    (JNIEnv *env, jclass obj, jint type)
    {
        Logger::LogD("jniLoad... -->> InitVersionInfo");
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        apiWrapper->InitVersionInfo(type);
        
        return jboolean(1);
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    DeleteTips
     * Signature: (Ljava/lang/String;)I
     */
    JNIEXPORT jint JNICALL DeleteTips
    (JNIEnv *env, jclass job, jstring rowKey){
        try{
        	const char *sRowkey = (env)->GetStringUTFChars(rowKey, NULL);

            std::string rowkeyStr(sRowkey);
            
            Editor::APIWrapper* apiWrapper = GetApiWrapper();
            
            int result = apiWrapper->DeleteTips(rowkeyStr);

            env->ReleaseStringUTFChars(rowKey, sRowkey);

            return result;
        }
        catch(...){
            Logger::LogD("Editor: Delete tips failed In JNI");
            Logger::LogO("Editor: Delete tips failed In JNI");
            return -1;
        }
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    UpdataGridStatus
     * Signature: (Ljava/util/ArrayList;)I
     */
    JNIEXPORT jint JNICALL UpdateGridStatus
    (JNIEnv *env, jclass job, jobject idList)
    {
        jclass cls_arraylist = env->GetObjectClass(idList);
        jmethodID arraylist_get = env->GetMethodID(cls_arraylist,"get","(I)Ljava/lang/Object;");
        jmethodID arraylist_size = env->GetMethodID(cls_arraylist,"size","()I");
        jint size = env->CallIntMethod(idList,arraylist_size);
        std::vector<std::string> gridIds;
        for(int i=0 ; i<size ;i++)
        {
            jobject obj_user = env->CallObjectMethod(idList,arraylist_get,i);
            jstring jstr = (jstring)obj_user;
            char*   buf = NULL;
            jclass   clsstring   =   env->FindClass("java/lang/String");
            jstring   strencode   =   env->NewStringUTF("GB2312");
            jmethodID   mid   =   env->GetMethodID(clsstring,"getBytes","(Ljava/lang/String;)[B");
            jbyteArray barr= (jbyteArray)env->CallObjectMethod(jstr,mid,strencode);
            jsize alen = env->GetArrayLength(barr);
            jbyte*   ba   =   env->GetByteArrayElements(barr,JNI_FALSE);
            if(alen>0)
            {
                buf = (char*)malloc(alen+1);//new   char[alen+1];
                memcpy(buf,ba,alen);
                buf[alen]=0;
            }
            env->ReleaseByteArrayElements(barr,ba,0);
            std::string id(buf);
            free(buf);
            gridIds.push_back(id);
        }
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        jint retn = (jint)apiWrapper->UpdateGridStatus(gridIds);
        
        return retn;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    UpdataGridStatus
     * Signature: (D;D;D;D)I
     */
    JNIEXPORT jint JNICALL UpdateGridStatusByBox
    (JNIEnv *env, jclass job, jdouble minLon,jdouble maxLon,jdouble minLat, jdouble maxLat)
    {
        double dminLon = minLon;
        double dmaxLon = maxLon;
        double dminLat = minLat;
        double dmaxLat = maxLat;
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        jint retn = (jint)apiWrapper->UpdateGridStatusByBox(dminLon,dmaxLon,dminLat,dmaxLat);
        
        
        return retn;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    InsertGpsLine
     * Signature: (Ljava/lang/String;)I
     */
    JNIEXPORT jint JNICALL InsertGpsLine
    (JNIEnv *env, jclass job, jstring json){
        try{
            
            const char *buffer = (env)->GetStringUTFChars(json, NULL);
            std::string jsonStr(buffer);
            
            Editor::APIWrapper* apiWrapper = GetApiWrapper();
            
            //int result = apiWrapper->InsertGpsLine(jsonStr);
            int result = apiWrapper->InsertGpsLineTip(jsonStr);
            
            env->ReleaseStringUTFChars(json, buffer);
            
            return result;
        }
        catch(...){
            Logger::LogD("Editor: Insert Gps Line failed In JNI");
            Logger::LogO("Editor: Insert Gps Line failed In JNI");
            return -1;
        }
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    GetProjectInfo
     * Signature: (Ljava/lang/String;)Ljava/util/ArrayList;
     */
    JNIEXPORT jobject JNICALL GetProjectInfo
    (JNIEnv *env, jclass obj, jstring token)
    {
    	const char *tokenStr = (env)->GetStringUTFChars(token, NULL);

        std::string sToken(tokenStr);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::vector<std::string> vStr = apiWrapper->GetProjectInfo(sToken);
        
        if(vStr.size() > 0)
        {
            Logger::LogD("GetProjectInfo==>>%s", vStr[0].c_str());
        }
        
        Logger::LogD("GetProjectInfo==>>%s", vStr[0].c_str());
        
        jclass list_cls = env->FindClass("java/util/ArrayList");
        
        jmethodID list_costruct = env->GetMethodID(list_cls , "<init>","()V");
        
        jobject list_obj = env->NewObject(list_cls,list_costruct,"");
        
        jmethodID list_add  = env->GetMethodID(list_cls,"add","(Ljava/lang/Object;)Z");
        
        for(int i = 0; i < vStr.size(); i++)
        {
            jstring jstr = (env)->NewStringUTF(vStr[i].c_str());
            
            env->CallBooleanMethod(list_obj,list_add,jstr);
        }
        
        env->ReleaseStringUTFChars(token, tokenStr);

        return list_obj;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    GetProjectDetail
     * Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/util/ArrayList;
     */
    JNIEXPORT jobject JNICALL GetProjectDetail
    (JNIEnv *env, jclass obj, jstring token, jstring projectid)
    {
    	const char *tokenStr = (env)->GetStringUTFChars(token, NULL);

    	const char *projectidStr = (env)->GetStringUTFChars(projectid, NULL);

        std::string sToken(tokenStr);
        
        std::string sProjectId(projectidStr);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::vector<std::string> vStr = apiWrapper->GetProjectDetails(sToken, sProjectId);
        
        if(vStr.size() > 0)
        {
            Logger::LogD("GetProjectInfo==>>%s", vStr[0].c_str());
        }
        
        jclass list_cls = env->FindClass("java/util/ArrayList");
        
        jmethodID list_costruct = env->GetMethodID(list_cls , "<init>","()V");
        
        jobject list_obj = env->NewObject(list_cls,list_costruct,"");
        
        jmethodID list_add  = env->GetMethodID(list_cls,"add","(Ljava/lang/Object;)Z");
        
        for(int i = 0; i < vStr.size(); i++)
        {
            jstring jstr = (env)->NewStringUTF(vStr[i].c_str());
            
            env->CallBooleanMethod(list_obj,list_add,jstr);
        }
        
        env->ReleaseStringUTFChars(token, tokenStr);
        env->ReleaseStringUTFChars(projectid, projectidStr);

        return list_obj;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    GivenPathSnap
     * Signature: (Ljava/lang/String;Ljava/lang/String;D)Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL GivenPathSnap
    (JNIEnv *env, jobject obj, jstring path, jstring point, jdouble disThreshold)
    {
        const char *sPath = (env)->GetStringUTFChars(path, NULL);
        
        const char *sPoint = (env)->GetStringUTFChars(point, NULL);
        
        std::string param1(sPath);
        
        std::string param2(sPoint);
        
        double disThres = disThreshold;
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::string result= apiWrapper->GivenPathSnap(param1, param2, disThres);
        
        env->ReleaseStringUTFChars(path, sPath);
        env->ReleaseStringUTFChars(point, sPoint);
        
        return (env)->NewStringUTF(result.c_str());
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    GetTaskDetails
     * Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/util/ArrayList;
     */
    JNIEXPORT jobject JNICALL GetTaskDetail
    (JNIEnv *env, jclass obj, jstring token, jstring projectid)
    {
    	const char *tokenStr = (env)->GetStringUTFChars(token, NULL);

    	const char *projectidStr = (env)->GetStringUTFChars(projectid, NULL);

        std::string sToken(tokenStr);
        
        std::string sProjectId(projectidStr);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::vector<std::string> vStr = apiWrapper->GetTaskDetails(sToken, sProjectId);
        
        if(vStr.size() > 0)
        {
            Logger::LogD("GetProjectInfo==>>%s", vStr[0].c_str());
        }
        
        jclass list_cls = env->FindClass("java/util/ArrayList");
        
        jmethodID list_costruct = env->GetMethodID(list_cls , "<init>","()V");
        
        jobject list_obj = env->NewObject(list_cls,list_costruct,"");
        
        jmethodID list_add  = env->GetMethodID(list_cls,"add","(Ljava/lang/Object;)Z");
        
        for(int i = 0; i < vStr.size(); i++)
        {
            jstring jstr = (env)->NewStringUTF(vStr[i].c_str());
            
            env->CallBooleanMethod(list_obj,list_add,jstr);
        }
        
        env->ReleaseStringUTFChars(token, tokenStr);
        env->ReleaseStringUTFChars(projectid, projectidStr);

        return list_obj;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    GetTipsRowkey
     * Signature: (Ljava/lang/String;)Ljava/lang/String
     */
    JNIEXPORT jstring JNICALL GetTipsRowkey
    (JNIEnv *env, jobject job, jstring sourceType)
    {
        Logger::LogD("Enter function....");
        
        const char *buffer = (env)->GetStringUTFChars(sourceType, NULL);
        
        std::string sourceTypeStr(buffer);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::string result = apiWrapper->GetTipsRowkey(sourceTypeStr);
        
        jstring resultJstring = (env)->NewStringUTF(result.c_str());
        
        env->ReleaseStringUTFChars(sourceType, buffer);
        
        return resultJstring;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    SnapRdNode
     * Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL SnapRdNode
    (JNIEnv *env, jobject obj, jstring box, jstring point)
    {
        const char *buffer = (env)->GetStringUTFChars(box, NULL);
        
        const char *sPoint = (env)->GetStringUTFChars(point, NULL);
        
        std::string param1(buffer);
        
        std::string param2(sPoint);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::string result= apiWrapper ->SnapRdNode(param1, param2);
        
        env->ReleaseStringUTFChars(box, buffer);
        env->ReleaseStringUTFChars(point, sPoint);
        
        return (env)->NewStringUTF(result.c_str());
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    GetCoremapModelVersion
     * Signature: ()Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL GetCoremapModelVersion
    (JNIEnv *env, jclass obj, jint type)
    {
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::string result= apiWrapper->GetCoremapModelVersion(type);
        
        return (env)->NewStringUTF(result.c_str());
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    GetTipsByRowkey
     * Signature: (Ljava/lang/String;)Ljava/lang/String
     */
    JNIEXPORT jstring JNICALL GetTipsByRowkey
    (JNIEnv *env, jobject job, jstring rowkey)
    {
        const char *rk = (env)->GetStringUTFChars(rowkey, NULL);
        
        std::string rowkeyStr(rk);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::string result = apiWrapper->GetTipsByRowkey(rowkeyStr);
        
        jstring resultJstring = (env)->NewStringUTF(result.c_str());
        
        env->ReleaseStringUTFChars(rowkey, rk);
        
        return resultJstring;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    InstallGdb
     * Signature: (Ljava/lang/String;Ljava/lang/String;Lcom/fastmap/dataapi/Progress;)Z
     */
    JNIEXPORT jboolean InstallGdb
    (JNIEnv *env, jclass obj, jstring path, jstring flag, jobject progress)
    {
    	const char *pathStr = (env)->GetStringUTFChars(path, NULL);

    	const char *flagStr = (env)->GetStringUTFChars(flag, NULL);

        std::string sPath(pathStr);
        
        std::string sFlag(flagStr);
        
        JNIProgress* p = new JNIProgress();
        
        env->GetJavaVM(&gs_jvm);
        
        gs_object = env->NewGlobalRef(progress);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();

        bool ret = apiWrapper->InstallGdb(sPath, sFlag, p);
        
        env->ReleaseStringUTFChars(path, pathStr);
        env->ReleaseStringUTFChars(flag, flagStr);

        return (jboolean)ret;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    LinkMacth
     * Signature: (Ljava/lang/String;Z)Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL LocationMacth
    (JNIEnv *env, jclass obj, jstring locationJson)
    {
    	const char *json = (env)->GetStringUTFChars(locationJson, NULL);

        std::string sWkt(json);
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        std::string sResult = apiWrapper->LinkMatch(sWkt);

        env->ReleaseStringUTFChars(locationJson, json);

        return (env)->NewStringUTF(sResult.c_str());
    }
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    CaclAbgle
     * Signature: (Ljava/lang/String;Ljava/lang/String;I)D
     */
    JNIEXPORT jdouble JNICALL CaclAngle
    (JNIEnv *env, jclass obj, jstring wkt, jstring pid, jint type)
    {
    	const char *wktStr = (env)->GetStringUTFChars(wkt, NULL);
    	const char *pidStr = (env)->GetStringUTFChars(pid, NULL);

        std::string sWktPoint(wktStr);
        std::string sLinkPid(pidStr);

        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        double angle = apiWrapper->CaclAngle(sWktPoint, sLinkPid, type);

        env->ReleaseStringUTFChars(wkt, wktStr);
        env->ReleaseStringUTFChars(pid, pidStr);
        return angle;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    PolygonSelectLines
     * Signature: (Ljava/lang/String)Ljava/util/ArrayList
     */
    JNIEXPORT jobject JNICALL PolygonSelectLines
    (JNIEnv *env, jobject job, jstring region, jboolean isIntersectIncluded)
    {
        jclass list_cls = env->FindClass("java/util/ArrayList");
        
        if(list_cls == NULL)
        {
            Logger::LogD("list is Null ....");
            return NULL;
        }
        
        jmethodID list_costruct = env->GetMethodID(list_cls , "<init>","()V");
        
        jobject list_obj = env->NewObject(list_cls,list_costruct,"");
        
        jmethodID list_add  = env->GetMethodID(list_cls,"add","(Ljava/lang/Object;)Z");
        
        const char *buffer = (env)->GetStringUTFChars(region, NULL);
        
        std::string param1(buffer);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::vector<std::string> selectedLines = apiWrapper->PolygonSelectLines(param1,isIntersectIncluded);
        
        for(int i = 0; i< selectedLines.size(); i++)
        {
            jstring resultJstring = (env)->NewStringUTF(selectedLines[i].c_str());
            
            env->CallBooleanMethod(list_obj,list_add,resultJstring);
        }
        
        env->ReleaseStringUTFChars(region, buffer);
        
        return list_obj;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    RepalceGDB
     * Signature: ()Z
     */
    JNIEXPORT jboolean JNICALL RepalceGDB
    (JNIEnv *env, jclass obj)
    {
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        return apiWrapper->RepalceGDB();
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    GetDownloadOrUploadStatus
     * Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL GetDownloadOrUploadStatus
    (JNIEnv *env, jclass obj, jstring project, jstring user, jint type)
    {
        std::string sProjectId((env)->GetStringUTFChars(project, NULL));
        std::string sUserId((env)->GetStringUTFChars(user, NULL));
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::string sResult =apiWrapper->GetDownloadOrUploadStatus(sProjectId, sUserId, type);
        
        return (env)->NewStringUTF(sResult.c_str());
    }
    
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    InsertInfos
     * Signature: (Ljava/lang/String;)I
     */
    JNIEXPORT jint JNICALL InsertInfos
    (JNIEnv *env, jclass job, jstring json){
        try{
            const char *buffer = (env)->GetStringUTFChars(json, NULL);
            
            std::string jsonStr(buffer);
            
            Editor::APIWrapper* apiWrapper = GetApiWrapper();
            
            int result = apiWrapper->InsertInfos(jsonStr);
            
            env->ReleaseStringUTFChars(json, buffer);
            
            return result;
        }
        catch(...){
            Logger::LogD("Editor: Insert Infos failed In JNI");
            Logger::LogO("Editor: Insert Infos failed In JNI");
            return -1;
        }
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    UpdateInfo
     * Signature: (Ljava/lang/String;)I
     */
    JNIEXPORT jint JNICALL UpdateInfo
    (JNIEnv *env, jclass job, jstring json){
        try{
            const char *buffer = (env)->GetStringUTFChars(json, NULL);
            
            std::string jsonStr(buffer);
            
            Editor::APIWrapper* apiWrapper = GetApiWrapper();
            
            int result = apiWrapper->UpdateInfo(jsonStr);
            
            env->ReleaseStringUTFChars(json, buffer);
            
            return result;
        }
        catch(...){
            Logger::LogD("Editor: Update Info failed In JNI");
            Logger::LogO("Editor: Update Info failed In JNI");
            return -1;
        }
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    GetEditHistroyByFid
     * Signature: (Ljava/lang/String;)Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL GetEditHistroyByFid
    (JNIEnv *env, jclass obj, jstring fid)
    {
        std::string sFid((env)->GetStringUTFChars(fid, NULL));
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::string sResult =apiWrapper->GetEditHistroyByFid(sFid);
        
        return (env)->NewStringUTF(sResult.c_str());
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    QueryFeatures
     * Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/util/ArrayList
     */
    JNIEXPORT jobject JNICALL QueryFeatures
    (JNIEnv *env, jobject, jstring tableName, jstring searchQuery)
    {
        const char *table = (env)->GetStringUTFChars(tableName, NULL);
        
        const char *query = (env)->GetStringUTFChars(searchQuery, NULL);
        
        std::string tableNameStr(table);
        
        std::string searchQueryStr(query);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::vector<std::string>  features = apiWrapper->QueryFeatures(tableNameStr, searchQueryStr);
        
        if(features.size() == 0)
        {
            return NULL;
        }
        
        jclass list_cls = env->FindClass("java/util/ArrayList");
        
        if(list_cls == NULL)
        {
            Logger::LogD("list is Null ....");
            
            return NULL;
        }
        jmethodID list_costruct = env->GetMethodID(list_cls , "<init>","()V");
        
        jobject list_obj = env->NewObject(list_cls,list_costruct,"");
        
        jmethodID list_add  = env->GetMethodID(list_cls,"add","(Ljava/lang/Object;)Z");
        
        for(int i=0; i<features.size(); i++)
        {
            jstring resultJstring = (env)->NewStringUTF(features[i].c_str());
            
            env->CallBooleanMethod(list_obj,list_add,resultJstring);
        }
        
        env->ReleaseStringUTFChars(tableName, table);
        env->ReleaseStringUTFChars(searchQuery, query);
        
        return list_obj;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    QueryFeaturesBySql
     * Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/util/ArrayList
     */
    JNIEXPORT jobject JNICALL QueryFeaturesBySql
    (JNIEnv *env, jobject, jstring tableName, jstring searchQuery)
    {
        const char *table = (env)->GetStringUTFChars(tableName, NULL);
        
        const char *query = (env)->GetStringUTFChars(searchQuery, NULL);
        
        std::string tableNameStr(table);
        
        std::string searchQueryStr(query);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::vector<std::string>  features = apiWrapper->QueryFeaturesBySql(tableNameStr, searchQueryStr);
        
        if(features.size() == 0)
        {
            return NULL;
        }
        
        jclass list_cls = env->FindClass("java/util/ArrayList");
        
        if(list_cls == NULL)
        {
            Logger::LogD("list is Null ....");
            
            return NULL;
        }
        jmethodID list_costruct = env->GetMethodID(list_cls , "<init>","()V");
        
        jobject list_obj = env->NewObject(list_cls,list_costruct,"");
        
        jmethodID list_add  = env->GetMethodID(list_cls,"add","(Ljava/lang/Object;)Z");
        
        for(int i=0; i<features.size(); i++)
        {
            jstring resultJstring = (env)->NewStringUTF(features[i].c_str());
            
            env->CallBooleanMethod(list_obj,list_add,resultJstring);
        }
        
        env->ReleaseStringUTFChars(tableName, table);
        env->ReleaseStringUTFChars(searchQuery, query);
        
        
        return list_obj;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    RobTask
     * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL RobTask
    (JNIEnv *env, jclass obj, jstring token, jstring projectId, jstring taskId)
    {
        std::string sToken((env)->GetStringUTFChars(token, NULL));
        
        std::string sProjectId((env)->GetStringUTFChars(projectId, NULL));
        
        std::string sTaskId((env)->GetStringUTFChars(taskId, NULL));
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::string sResult =apiWrapper->RobTask(sToken, sProjectId, sTaskId);
        
        return (env)->NewStringUTF(sResult.c_str());
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    QueryTableByCondition
     * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZII)Ljava/util/ArrayList;
     */
    JNIEXPORT jobject JNICALL QueryTableByCondition
    (JNIEnv *env, jclass obj, jstring table, jstring where, jstring oderbyFiedOrWktPoint,jstring geoFieldName,jboolean isOderbyFied,jboolean isDesc,jint pagesize, jint offset)
    {
        const char *sTable = (env)->GetStringUTFChars(table, NULL);
        const char *sWhere = (env)->GetStringUTFChars(where, NULL);
        const char *sOderbyFiedOrWktPoint = (env)->GetStringUTFChars(oderbyFiedOrWktPoint, NULL);
        const char *sGeoFieldName = (env)->GetStringUTFChars(geoFieldName, NULL);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::vector<std::string> vStr = apiWrapper->QueryTableByCondition(sTable, sWhere, sOderbyFiedOrWktPoint, sGeoFieldName, isOderbyFied, isDesc, pagesize, offset);
        
        jclass list_cls = env->FindClass("java/util/ArrayList");
        
        jmethodID list_costruct = env->GetMethodID(list_cls , "<init>","()V");
        
        jobject list_obj = env->NewObject(list_cls,list_costruct,"");
        
        jmethodID list_add  = env->GetMethodID(list_cls,"add","(Ljava/lang/Object;)Z");
        
        for(int i = 0; i < vStr.size(); i++)
        {
            jstring jstr = (env)->NewStringUTF(vStr[i].c_str());
            
            env->CallBooleanMethod(list_obj,list_add,jstr);
        }
        
        env->ReleaseStringUTFChars(table, sTable);
        env->ReleaseStringUTFChars(where, sWhere);
        env->ReleaseStringUTFChars(oderbyFiedOrWktPoint, sOderbyFiedOrWktPoint);
        env->ReleaseStringUTFChars(geoFieldName, sGeoFieldName);
        
        
        return list_obj;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    GetCountByWhere
     * Signature: (Ljava/lang/String;Ljava/lang/String;)I
     */
    JNIEXPORT jint JNICALL GetCountByWhere
    (JNIEnv *env, jclass obj, jstring table, jstring where)
    {
        const char *sTable = (env)->GetStringUTFChars(table, NULL);
        const char *sWhere = (env)->GetStringUTFChars(where, NULL);
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        int ret = apiWrapper->GetCountByWhere(sTable, sWhere);
        
        env->ReleaseStringUTFChars(table, sTable);
        env->DeleteLocalRef(table);
        env->ReleaseStringUTFChars(where, sWhere);
        env->DeleteLocalRef(where);
        return (jint)ret;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    InsertTrackPoint
     * Signature: (Lcom/fastmap/dataapi/data/TrackCollection;)I
     */
    JNIEXPORT jint JNICALL InsertTrackPoint
    (JNIEnv *env, jclass obj, jobject trackPointObj)
    {
        jfieldID fid;
        jclass trackCollectionCls = env->GetObjectClass(trackPointObj);
        //类TrackCollection中有个String类型的属性id
        //获取要访问的属性的id
        fid = env->GetFieldID(trackCollectionCls,"id","Ljava/lang/String;");
        jstring jstr = (jstring)env->GetObjectField(trackPointObj,fid);
        const char* str  = env->GetStringUTFChars(jstr,NULL);
        std::string idStr = str;
        env->ReleaseStringUTFChars(jstr,str);
        double dLat(0.0),dLon(0.0),dDirection(0.0),dSpeed(0.0);
        //类TrackCollection中有个double类型的属性latitude
        fid = env->GetFieldID(trackCollectionCls,"latitude","D");
        jdouble jd = env->GetDoubleField(trackPointObj,fid);
        dLat = jd;
        //类TrackCollection中有个double类型的属性longitude
        fid = env->GetFieldID(trackCollectionCls,"longitude","D");
        jd = env->GetDoubleField(trackPointObj,fid);
        dLon = jd;
        //类TrackCollection中有个double类型的属性direction
        fid = env->GetFieldID(trackCollectionCls,"direction","D");
        jd = env->GetDoubleField(trackPointObj,fid);
        dDirection = jd;
        //类TrackCollection中有个double类型的属性speed
        fid = env->GetFieldID(trackCollectionCls,"speed","D");
        jd = env->GetDoubleField(trackPointObj,fid);
        dSpeed = jd;
        //类TrackCollection中有个String类型的属性recordTime
        fid = env->GetFieldID(trackCollectionCls,"recordTime","Ljava/lang/String;");
        jstr = (jstring)env->GetObjectField(trackPointObj,fid);
        str  = env->GetStringUTFChars(jstr,NULL);
        std::string recordTimeStr = str;
        //释放资源
        env->ReleaseStringUTFChars(jstr,str);
        //类TrackCollection中有个int类型的属性userId
        fid = env->GetFieldID(trackCollectionCls,"userId","I");
        jint juserId =  env->GetIntField(trackPointObj,fid);
        int userId = juserId;
        //类TrackCollection中有个String类型的属性segmentId
        fid = env->GetFieldID(trackCollectionCls,"segmentId","Ljava/lang/String;");
        jstr = (jstring)env->GetObjectField(trackPointObj,fid);
        str  = env->GetStringUTFChars(jstr,NULL);
        std::string segmentIdStr = str;
        //释放资源
        env->ReleaseStringUTFChars(jstr,str);
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        int ret = apiWrapper->InsertTrackPoint(idStr,dLat,dLon,dDirection,dSpeed,recordTimeStr,userId,segmentIdStr);
        return ret;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    GetDownloadPrepared
     * Signature: (Ljava/lang/String;)Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL GetDownloadPrepared
    (JNIEnv *env, jclass obj, jstring input)
    {
        const char *sInput = (env)->GetStringUTFChars(input, NULL);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::string sResult = apiWrapper->GetDownloadPrepared(sInput);
        
        return (env)->NewStringUTF(sResult.c_str());
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    DownloadFile
     * Signature: (Ljava/lang/String;Lcom/fastmap/dataapi/Progress;)V
     */
    JNIEXPORT void JNICALL DownloadFile_2(JNIEnv *env, jclass obj, jstring input, jobject progress)
    {
        const char *sInput = (env)->GetStringUTFChars(input, NULL);
        
        JNIProgress* p = new JNIProgress();
        
        env->GetJavaVM(&gs_jvm);
        
        gs_object = env->NewGlobalRef(progress);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        apiWrapper->DownloadFile(sInput, p);
        
        env->ReleaseStringUTFChars(input, sInput);
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    GetUploadPrepared
     * Signature: (Ljava/lang/String;)Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL GetUploadPrepared(JNIEnv *env, jclass obj, jstring input)
    {
        const char *sInput = (env)->GetStringUTFChars(input, NULL);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::string sResult = apiWrapper->GetUploadPrepared(sInput);
        
        jstring result = (env)->NewStringUTF(sResult.c_str());
        
        env->ReleaseStringUTFChars(input, sInput);
        
        return result;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    UploadFile
     * Signature: (Ljava/lang/String;Lcom/fastmap/dataapi/Progress;)V
     */
    JNIEXPORT void JNICALL UploadFile_2(JNIEnv *env, jclass obj, jstring input, jobject progress)
    {
        const char *sInput = (env)->GetStringUTFChars(input, NULL);
        
        JNIProgress* p = new JNIProgress();
        
        env->GetJavaVM(&gs_jvm);
        
        gs_object = env->NewGlobalRef(progress);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        apiWrapper->UploadFile(sInput, p);
        
        env->ReleaseStringUTFChars(input, sInput);
    }
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    InsertFeature
     * Signature: (Ljava/lang/String;Ljava/lang/String;)I
     */
    JNIEXPORT jint JNICALL InsertFeature
    (JNIEnv *env, jclass obj, jstring tableName, jstring json)
    {
        try
        {
            const char *table = (env)->GetStringUTFChars(tableName, NULL);
            
            std::string tableNameStr(table);
            
            const char *jsonStr = (env)->GetStringUTFChars(json, NULL);
            
            std::string jsonString(jsonStr);
            
            env->ReleaseStringUTFChars(tableName,table);
            env->ReleaseStringUTFChars(json,jsonStr);
            
            Editor::APIWrapper* apiWrapper = GetApiWrapper();
            
            int result = apiWrapper->InsertFeature(tableNameStr,jsonString);
            
            return result;
        }
        catch(...)
        {
            Logger::LogD("Editor: Insert Feature failed In JNI");
            Logger::LogO("Editor: Insert Feature failed In JNI");
            return -1;
        }
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    UpdateFeature
     * Signature: (Ljava/lang/String;Ljava/lang/String;)I
     */
    JNIEXPORT jint JNICALL UpdateFeature
    (JNIEnv *env, jclass obj, jstring tableName, jstring json)
    {
        try
        {
            const char *table = (env)->GetStringUTFChars(tableName, NULL);
            
            std::string tableNameStr(table);
            
            const char *jsonStr = (env)->GetStringUTFChars(json, NULL);
            
            std::string jsonString(jsonStr);
            
            env->ReleaseStringUTFChars(tableName,table);
            env->ReleaseStringUTFChars(json,jsonStr);
            
            Editor::APIWrapper* apiWrapper = GetApiWrapper();
            
            int result = apiWrapper->UpdateFeature(tableNameStr,jsonString);
            
            return result;
        }
        catch(...)
        {
            Logger::LogD("Editor: Update Feature failed In JNI");
            Logger::LogO("Editor: Update Feature failed In JNI");
            return -1;
        }
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    DeleteFeature
     * Signature: (Ljava/lang/String;I)I
     */
    JNIEXPORT jint JNICALL DeleteFeature
    (JNIEnv *env, jclass obj, jstring tableName, jint rowId)
    {
        try
        {
            const char *table = (env)->GetStringUTFChars(tableName, NULL);
            
            std::string tableNameStr(table);
            
            env->ReleaseStringUTFChars(tableName,table);
            
            Editor::APIWrapper* apiWrapper = GetApiWrapper();
            
            int result = apiWrapper->DeleteFeature(tableNameStr,rowId);
            
            return result;
        }
        catch(...)
        {
            Logger::LogD("Editor: Delete Feature failed In JNI");
            Logger::LogO("Editor: Delete Feature failed In JNI");
            return -1;
        }
        
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    PoiStatistics
     * Signature: (Ljava/lang/String;)Ljava/util/ArrayList;
     */
    JNIEXPORT jobject JNICALL PoiStatistics
    (JNIEnv *env, jclass job, jstring tableName)
    {
        const char *sTableName = (env)->GetStringUTFChars(tableName, NULL);
        
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        
        std::vector<std::string> results;
        
        apiWrapper->PoiStatistics(sTableName, results);
        
        jclass list_cls = env->FindClass("java/util/ArrayList");
        
        if(list_cls == NULL)
        {
            Logger::LogD("list is Null ....");
            return NULL;
        }
        
        jmethodID list_costruct = env->GetMethodID(list_cls , "<init>","()V");
        
        jobject list_obj = env->NewObject(list_cls,list_costruct,"");
        jmethodID list_add  = env->GetMethodID(list_cls,"add","(Ljava/lang/Object;)Z");
        for(int i=0; i<results.size(); i++)
        {
            jstring resultJstring = (env)->NewStringUTF(results[i].c_str());
            env->CallBooleanMethod(list_obj,list_add,resultJstring);
            env->ReleaseStringUTFChars(resultJstring, results[i].c_str());
            env->DeleteLocalRef(resultJstring);
        }
        
        return list_obj;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    TrackStatistics
     * Signature: (Ljava/lang/String;)Ljava/util/ArrayList;
     */
    JNIEXPORT jobject JNICALL TrackStatistics
    (JNIEnv *env, jclass job, jstring userId)
    {
        const char *sUserId = (env)->GetStringUTFChars(userId, NULL);
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        std::vector<std::string> results;
        apiWrapper->TrackStatistics(sUserId, results);
        jclass list_cls = env->FindClass("java/util/ArrayList");
        if(list_cls == NULL)
        {
            Logger::LogD("list is Null ....");
            return NULL;
        }
        jmethodID list_costruct = env->GetMethodID(list_cls , "<init>","()V");
        jobject list_obj = env->NewObject(list_cls,list_costruct,"");
        jmethodID list_add  = env->GetMethodID(list_cls,"add","(Ljava/lang/Object;)Z");
        for(int i=0; i<results.size(); i++)
        {
            jstring resultJstring = (env)->NewStringUTF(results[i].c_str());
            env->CallBooleanMethod(list_obj,list_add,resultJstring);
            env->ReleaseStringUTFChars(resultJstring, results[i].c_str());
            env->DeleteLocalRef(resultJstring);
        }
        return list_obj;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    DeleteTrack
     * Signature: (Ljava/lang/String;Ljava/lang/String;)I
     */
    JNIEXPORT jint JNICALL DeleteTrack
    (JNIEnv *env, jclass obj, jstring userId, jstring date)
    {
        const char *sUserId = (env)->GetStringUTFChars(userId, NULL);
        const char *sDate = (env)->GetStringUTFChars(date, NULL);
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        int ret = apiWrapper->DeleteTrack(sUserId, sDate);
        
        env->ReleaseStringUTFChars(userId, sUserId);
        env->DeleteLocalRef(userId);
        env->ReleaseStringUTFChars(date, sDate);
        env->DeleteLocalRef(date);
        return (jint)ret;
    }
    
    /*
     * Class:     com_fastmap_dataapi_DataNativeApi
     * Method:    DeletePoiAndInfoByRegion
     * Signature: (Ljava/lang/String;Ljava/lang/String;)I
     */
    JNIEXPORT jint JNICALL DeletePoiAndInfoByRegion
    (JNIEnv *env, jclass obj, jstring projectId, jstring region)
    {
        const char *sProjectId = (env)->GetStringUTFChars(projectId, NULL);
        const char *sRegion = (env)->GetStringUTFChars(region, NULL);
        Editor::APIWrapper* apiWrapper = GetApiWrapper();
        int ret = apiWrapper->DeletePoiAndInfoByRegion(sProjectId, sRegion);
        
        env->ReleaseStringUTFChars(projectId, sProjectId);
        env->DeleteLocalRef(projectId);
        env->ReleaseStringUTFChars(region, sRegion);
        env->DeleteLocalRef(region);
        return (jint)ret;
    }
    
#ifdef __cplusplus
}
#endif

/**
 * Table of methods associated with a single class.
 */
static JNINativeMethod gMethods[] = {
    {"InstallLayers", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z", (void*)InstallLayers},
    {"DownloadFile", "(Ljava/lang/String;Ljava/lang/String;Ljava/util/ArrayList;ILcom/fastmap/dataapi/Progress;I)Z", (void*)DownloadFile},
    {"CancelDownload", "()Z", (void*)CancelDownload},
    {"UploadFile", "(Ljava/lang/String;Ljava/lang/String;Ljava/util/ArrayList;ILcom/fastmap/dataapi/Progress;ILjava/lang/String;)Z", (void*)UploadFile},
    {"CancelUpload", "()Z", (void*)CancelUpload},
    {"GetPoiPhotoPath", "()Ljava/lang/String;", (void*)GetPoiPhotoPath},
    {"GetTipsPhotoPath", "()Ljava/lang/String;", (void*)GetTipsPhotoPath},
    {"SelectMetaData", "(Ljava/lang/String;Ljava/lang/String;I)Ljava/util/ArrayList;", (void*)SelectMetaData},
    {"SnapPoint", "(Ljava/lang/String;Ljava/lang/String;)Ljava/util/ArrayList;", (void*)SnapPoint},
    {"SnapLine", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", (void*)SnapLine},
    {"SnapRdNode", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", (void*)SnapRdNode},
    {"SelectPois","(Ljava/lang/String;)Ljava/util/ArrayList;",(void*)SelectPois},
    {"GetTotalPoiByFid","(Ljava/lang/String;)Ljava/lang/String;",(void*)GetTotalPoiByFid},
    {"InsertPoi","(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I",(void*)InsertPoi},
    {"UpdatePoi","(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I",(void*)UpdatePoi},
    {"UpdatePois","(Ljava/util/ArrayList;Ljava/lang/String;Ljava/lang/String;Ljava/util/ArrayList;Lcom/fastmap/dataapi/Progress;)I",(void*)UpdatePois},
    {"DeletePoi","(ILjava/lang/String;Ljava/lang/String;)I",(void*)DeletePoi},
    {"SetRootPath","(Ljava/lang/String;)V",(void*)SetRootPath},
    {"SetUserId","(Ljava/lang/String;)V",(void*)SetUserId},
    {"GetDataPath","()Ljava/lang/String;",(void*)GetDataPath},
    {"SelectTips","(Ljava/lang/String;Ljava/lang/String;)Ljava/util/ArrayList;",(void*)SelectTips},
    {"InsertTips","(Ljava/lang/String;)I",(void*)InsertTips},
    {"UpdateTips","(Ljava/lang/String;)I",(void*)UpdateTips},
    {"GetCountByCondition","(Ljava/lang/String;I)I",(void*)GetCountByCondition},
    {"GetSignalLayerByCondition","(Ljava/lang/String;III)Ljava/util/ArrayList;",(void*)GetSignalLayerByCondition},
    {"InitVerstionInfo","(I)Z",(void*)InitVerstionInfo},
    {"GetRdLineByPid","(I)Ljava/lang/String;",(void*)GetRdLineByPid},
    {"DeleteTips","(Ljava/lang/String;)I",(void*)DeleteTips},
    {"UpdateGridStatus","(Ljava/util/ArrayList;)I",(void*)UpdateGridStatus},
    {"UpdateGridStatusByBox","(DDDD)I",(void*)UpdateGridStatusByBox},
    {"InsertGpsLine","(Ljava/lang/String;)I",(void*)InsertGpsLine},
    {"GetProjectInfo","(Ljava/lang/String;)Ljava/util/ArrayList;",(void*)GetProjectInfo},
    {"GetProjectDetail","(Ljava/lang/String;Ljava/lang/String;)Ljava/util/ArrayList;",(void*)GetProjectDetail},
    {"GivenPathSnap", "(Ljava/lang/String;Ljava/lang/String;D)Ljava/lang/String;", (void*)GivenPathSnap},
    {"GetTaskDetail","(Ljava/lang/String;Ljava/lang/String;)Ljava/util/ArrayList;",(void*)GetTaskDetail},
    {"GetTipsRowkey","(Ljava/lang/String;)Ljava/lang/String;",(void*)GetTipsRowkey},
    {"GetCoremapModelVersion","(I)Ljava/lang/String;",(void*)GetCoremapModelVersion},
    {"GetTipsByRowkey","(Ljava/lang/String;)Ljava/lang/String;",(void*)GetTipsByRowkey},
    {"InstallGdb","(Ljava/lang/String;Ljava/lang/String;Lcom/fastmap/dataapi/Progress;)Z",(void*)InstallGdb},
    {"LocationMacth","(Ljava/lang/String;)Ljava/lang/String;",(void*)LocationMacth},
    {"CaclAngle","(Ljava/lang/String;Ljava/lang/String;I)D",(void*)CaclAngle},
    {"PolygonSelectLines","(Ljava/lang/String;Z)Ljava/util/ArrayList;",(void*)PolygonSelectLines},
    {"RepalceGDB", "()Z", (void*)RepalceGDB},
    {"GetDownloadOrUploadStatus", "(Ljava/lang/String;Ljava/lang/String;I)Ljava/lang/String;", (void*)GetDownloadOrUploadStatus},
    {"InsertInfos","(Ljava/lang/String;)I",(void*)InsertInfos},
    {"UpdateInfo","(Ljava/lang/String;)I",(void*)UpdateInfo},
    {"GetEditHistroyByFid", "(Ljava/lang/String;)Ljava/lang/String;", (void*)GetEditHistroyByFid},
    {"QueryFeatures", "(Ljava/lang/String;Ljava/lang/String;)Ljava/util/ArrayList;", (void*)QueryFeatures},
    {"QueryFeaturesBySql", "(Ljava/lang/String;Ljava/lang/String;)Ljava/util/ArrayList;", (void*)QueryFeaturesBySql},
    {"RobTask", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", (void*)RobTask},
    {"QueryTableByCondition","(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZII)Ljava/util/ArrayList;",(void*)QueryTableByCondition},
    {"InsertTrackPoint","(Lcom/fastmap/dataapi/data/TrackCollection;)I",(void*)InsertTrackPoint},
    {"GetDownloadPrepared","(Ljava/lang/String;)Ljava/lang/String;",(void*)GetDownloadPrepared},
    {"DownloadFile_2","(Ljava/lang/String;Lcom/fastmap/dataapi/Progress;)V",(void*)DownloadFile_2},
    {"GetUploadPrepared","(Ljava/lang/String;)Ljava/lang/String;",(void*)GetUploadPrepared},
    {"UploadFile_2","(Ljava/lang/String;Lcom/fastmap/dataapi/Progress;)V",(void*)UploadFile_2},
    {"InsertFeature","(Ljava/lang/String;Ljava/lang/String;)I",(void*)InsertFeature},
    {"UpdateFeature","(Ljava/lang/String;Ljava/lang/String;)I",(void*)UpdateFeature},
    {"DeleteFeature","(Ljava/lang/String;I)I",(void*)DeleteFeature},
    {"PoiStatistics","(Ljava/lang/String;)Ljava/util/ArrayList;",(void*)PoiStatistics},
    {"GetCountByWhere","(Ljava/lang/String;Ljava/lang/String;)I",(void*)GetCountByWhere},
    {"TrackStatistics","(Ljava/lang/String;)Ljava/util/ArrayList;",(void*)TrackStatistics},
    {"DeleteTrack","(Ljava/lang/String;Ljava/lang/String;)I",(void*)DeleteTrack},
    {"DeletePoiAndInfoByRegion","(Ljava/lang/String;Ljava/lang/String;)I",(void*)DeletePoiAndInfoByRegion},
};

/*
 * Register several native methods for one class.
 */
static int registerNativeMethods(JNIEnv* env, const char* className,
                                 JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;
    clazz = env->FindClass(className);
    if (clazz == NULL) {
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        return JNI_FALSE;
    }
    
    return JNI_TRUE;
}

/*
 * Register native methods for all classes we know about.
 */
static int registerNatives(JNIEnv* env)
{
    if (!registerNativeMethods(env, JNIREG_CLASS, gMethods,
                               sizeof(gMethods) / sizeof(gMethods[0])))
        return JNI_FALSE;
    
    return JNI_TRUE;
}

/*
 * Set some test stuff up.
 *
 * Returns the JNI version on success, -1 on failure.
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    
    jint result = -1;
    
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    assert(env != NULL);
    
    if (!registerNatives(env)) {
        return -1;
    }
    /* success -- return valid version number */
    result = JNI_VERSION_1_6;
    
    //g_jvm = vm;
    
    //jniErrorFID = NULL;
    
    return result;
}

JNIProgress::JNIProgress()
{
}

JNIProgress::~JNIProgress(){}

int JNIProgress::OnProgress(std::string description, double progress)
{
    if(gs_jvm != NULL && gs_object != NULL)
    {
        JNIEnv* env = NULL;
        
        gs_jvm->AttachCurrentThread(&env, NULL);
        
        jclass ProgressClass = (env)->GetObjectClass(gs_object);
        
        
        jmethodID ProgressMethodId = (env)->GetMethodID(ProgressClass,"OnProgress", "(Ljava/lang/String;D)D");
        
        
        jstring sTmp = (env)->NewStringUTF(description.c_str());
        
        
        jdouble ddd =  (jdouble)(env)->CallDoubleMethod(gs_object,ProgressMethodId,sTmp,progress);
        
        (env)->DeleteLocalRef(sTmp);
    }
    
    return 0;
}

void JNIProgress::DisposeProgress()
{
    gs_jvm->DetachCurrentThread();
}


