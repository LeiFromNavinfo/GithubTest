#include <jni.h>
#include <stdio.h>
#include <assert.h>
#include "Editor/Logger.h"
#include "Editor/Geometry.h"
#include "Editor/Editor.h"
#include "Editor/Model.h"
#include "Editor/DataTransfor.h"
#include "Editor/Editor_ProjectManager.h"
#include "Editor/Editor_MetaDataManager.h"
#include "Editor/Editor_ResumeProcessor.h"
#include "Editor/Editor_APIWrapper.h"
#include "jniProgress.h"

//#include "nds_sqlite3.h"
#include "sqlite3.h"
#include "spatialite.h"
#include "geos.h"
#include "document.h"
#include "stringbuffer.h"
#include "string.h"
#include "writer.h"
#include <sstream>


//#define JNIREG_CLASS "com/fastmap/dataapi/DataNativeApi"
#define JNIREG_CLASS "com/map/editor/DataNativeApi"

static JavaVM *gs_jvm=NULL;
static jobject gs_object=NULL;

#ifdef __cplusplus
extern "C" {
#endif

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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	apiWrapper->InstallLayers(sDownloadFile, sExistFile, tableName);

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
	std::string sCurrentToken((env)->GetStringUTFChars(sToken, NULL));

	std::string sCurrentUser((env)->GetStringUTFChars(sUser, NULL));

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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	apiWrapper->DownloadFile(sCurrentToken, sCurrentUser, ids, nType, (Editor::Job::Cmd)nCmd, p);

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
	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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
	std::string sCurrentToken((env)->GetStringUTFChars(sToken, NULL));

	std::string sCurrentUser((env)->GetStringUTFChars(sUser, NULL));
	std::string sDeviceId((env)->GetStringUTFChars(deviceId, NULL));

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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	apiWrapper->UploadFile(sCurrentToken, sCurrentUser, ids, nType, (Editor::JOBCMD)nCmd, p, sDeviceId);

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
	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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
	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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
	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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
	std::string sTable((env)->GetStringUTFChars(table, NULL));

	std::string sCondition((env)->GetStringUTFChars(condition, NULL));

	Logger::LogD("SelectMetaData==>>&s, &s", sTable.c_str(), sCondition.c_str());

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	std::string result= apiWrapper->SnapLine(param1, param2);
	
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

    Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	std::string result = apiWrapper->GetTotalPoiByFid(fidStr);

	jstring resultJstring = (env)->NewStringUTF(result.c_str());

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
        
        Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();
        
        int result = apiWrapper->InsertPoi(jsonStr,sUserId, sProjectId);

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
        
        Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

        int result = apiWrapper->UpdatePoi(jsonStr, sUserId, sProjectId);

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

        Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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

		Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

		int result = apiWrapper->DeletePoi(rowId, sUserId, sProjectId);

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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	apiWrapper->SetRootPath(root);
}

/*
 * Class:     com_fastmapsdk_dataapi_DataNativeApi
 * Method:    SetUserId
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL SetUserId
  (JNIEnv *env, jobject, jstring userId){
	const char *currentUser = (env)->GetStringUTFChars(userId, NULL);

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	apiWrapper->SetUserId(currentUser);
}

    int sqlite3_callback_fillbundle(void* para, int n_column, char** column_value, char** column_name)
    {
        for (int i=0; i<n_column; i++)
        {
            Logger::LogD("column_name:%s,%s", column_name[i], column_value[i]);
        }

        return -1;	//non-zero
    }

    bool ExecuteNonQuery(const char* sql, sqlite3* db)
    {
        char* pszErrMsg;

        int rc = sqlite3_exec(db, sql, sqlite3_callback_fillbundle, NULL, &pszErrMsg);

        if (rc != SQLITE_OK)
        {
            Logger::LogD("ExecuteNoQuery failed:%s", pszErrMsg);

            sqlite3_free(pszErrMsg);

            return false;
        }

        return true;
    }

    void errorLogCallback(void *pArg, int iErrCode, const char *zMsg)
    {
        Logger::LogD("errorLogCallback, (%d) %s", iErrCode, zMsg);
    }

    void CreateSpatialite()
    {
        sqlite3_config(SQLITE_CONFIG_LOG, errorLogCallback, NULL);

        std::string file = "/storage/sdcard0/test.sqlite";

        ::remove(file.c_str());

        sqlite3* db = NULL;

        int rc = sqlite3_open_v2(file.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

        Logger::LogD("sqlite3_open_v2, rc:%d, msg:%s", rc, sqlite3_errmsg(db));

        void* cache = spatialite_alloc_connection();

        spatialite_init_ex(db, cache, 1);

        std::string createSql = "create table test(id int32 PRIMARY KEY, name text)";

        std::string addGeoSql = "select AddGeometryColumn('test', 'geometry', 4326, 'GEOMETRY', 'XY')";

        ExecuteNonQuery("BEGIN", db);

        ExecuteNonQuery("SELECT InitSpatialMetaData()", db);

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

            return;
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

        sqlite3_finalize(stmt);

        ExecuteNonQuery("SELECT RecoverGeometryColumn('test', 'geometry', 4326, 'GEOMETRY', 'XY');", db);

        ExecuteNonQuery("SELECT CreateSpatialIndex('test','geometry')", db);

        ExecuteNonQuery("SELECT RecoverSpatialIndex('test', 'geometry', 1);", db);

        ExecuteNonQuery("COMMIT", db);

        spatialite_cleanup_ex(cache);

        spatialite_shutdown();

        cache = NULL;

        rc = sqlite3_close_v2(db);

        Logger::LogD("sqlite3_close_v2, rc:%d", rc);
    }


/*
 * Class:     com_fastmapsdk_dataapi_DataNativeApi
 * Method:    GetDataPath
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL GetDataPath
  (JNIEnv *env, jobject){
	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	std::string dataPath = apiWrapper->GetDataPath();

	Editor::DataManager* dataManager = Editor::DataManager::getInstance();

	Editor::CoreMapDataSource* dataSource = dynamic_cast<Editor::CoreMapDataSource*>(dataManager->getDataSource(0));

	std::string temp = "/storage/sdcard0/fastmap3/data/collect/4396/coremap.sqlite";

	int tempret = access(temp.c_str(), W_OK);

	Logger::LogD("tempret:%d", tempret);

//	Editor::DataFileSystem* pFileSystem = dataManager->getFileSystem();
//
//	std::string sPath = pFileSystem->GetCoreMapDataPath();

	//dataSource->Close(sPath);

	Logger::LogD("install begin:");
	Logger::LogO("install begin:");

	Tools::WatchTimerStart();

	//int ret = dataSource->InstallPoi("/storage/sdcard0/json2.zip", "projectId", NULL);

    int ret = 0;
    CreateSpatialite();

	long cost = Tools::WatchTimerStop();

	Logger::LogD("cost3 is [%ld]", cost);
	Logger::LogO("cost is [%ld]", cost);

	Logger::LogD("install end, ret:%d", ret);
	Logger::LogO("install end, ret:%d", ret);

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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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

		Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

		int result = apiWrapper->InsertTips(jsonStr);

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

	   Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	   int result = apiWrapper->UpdateTips(jsonStr);
	   
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
	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	return apiWrapper->GetCountByCondition(sTable, type);
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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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
		
	    std::string rowkeyStr((env)->GetStringUTFChars(rowKey, NULL));

	    Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	    int result = apiWrapper->DeleteTips(rowkeyStr);
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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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

		Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

		//int result = apiWrapper->InsertGpsLine(jsonStr);
		int result = apiWrapper->InsertGpsLineTip(jsonStr);

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
	std::string sToken((env)->GetStringUTFChars(token, NULL));

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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
	std::string sToken((env)->GetStringUTFChars(token, NULL));

	std::string sProjectId((env)->GetStringUTFChars(projectid, NULL));

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	std::string result= apiWrapper->GivenPathSnap(param1, param2, disThres);

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
	std::string sToken((env)->GetStringUTFChars(token, NULL));

	std::string sProjectId((env)->GetStringUTFChars(projectid, NULL));

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	std::string result = apiWrapper->GetTipsRowkey(sourceTypeStr);

	jstring resultJstring = (env)->NewStringUTF(result.c_str());

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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	std::string result= apiWrapper ->SnapRdNode(param1, param2);

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
	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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
	const char *buffer = (env)->GetStringUTFChars(rowkey, NULL);

	std::string rowkeyStr(buffer);

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	std::string result = apiWrapper->GetTipsByRowkey(rowkeyStr);

	jstring resultJstring = (env)->NewStringUTF(result.c_str());

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
	std::string sPath((env)->GetStringUTFChars(path, NULL));

	std::string sFlag((env)->GetStringUTFChars(flag, NULL));

	JNIProgress* p = new JNIProgress();

	env->GetJavaVM(&gs_jvm);

	gs_object = env->NewGlobalRef(progress);

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	return (jboolean)apiWrapper->InstallGdb(sPath, sFlag, p);
}

/*
 * Class:     com_fastmap_dataapi_DataNativeApi
 * Method:    LinkMacth
 * Signature: (Ljava/lang/String;Z)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL LocationMacth
  (JNIEnv *env, jclass obj, jstring locationJson)
{
	std::string sWkt((env)->GetStringUTFChars(locationJson, NULL));
	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();
	std::string sResult = apiWrapper->LinkMatch(sWkt);
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
	std::string sWktPoint((env)->GetStringUTFChars(wkt, NULL));
	std::string sLinkPid((env)->GetStringUTFChars(pid, NULL));
	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();
	double angle = apiWrapper->CaclAngle(sWktPoint, sLinkPid, type);
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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	std::vector<std::string> selectedLines = apiWrapper->PolygonSelectLines(param1,isIntersectIncluded);

	for(int i = 0; i< selectedLines.size(); i++)
	{
		jstring resultJstring = (env)->NewStringUTF(selectedLines[i].c_str());

		env->CallBooleanMethod(list_obj,list_add,resultJstring);
	}

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
	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();
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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	std::string sResult =apiWrapper->GetDownloadOrUploadStatus(sProjectId, sUserId, type);

	return (env)->NewStringUTF(sResult.c_str());
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
            
            Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();
            
            int result = apiWrapper->UpdateInfo(jsonStr);
            
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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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
        
        Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();
        
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
        
        Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();
        
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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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

	return list_obj;
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
	 Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();
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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	apiWrapper->DownloadFile(sInput, p);
}

/*
 * Class:     com_fastmap_dataapi_DataNativeApi
 * Method:    GetUploadPrepared
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL GetUploadPrepared(JNIEnv *env, jclass obj, jstring input)
{
	const char *sInput = (env)->GetStringUTFChars(input, NULL);

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	std::string sResult = apiWrapper->GetUploadPrepared(sInput);

	return (env)->NewStringUTF(sResult.c_str());
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

	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

	apiWrapper->UploadFile(sInput, p);
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
            
            Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();
            
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
            
            Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();
            
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
            
            Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();
            
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

    	Editor::APIWrapper* apiWrapper = Editor::APIWrapper::getInstance();

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
    	 }

    	return list_obj;
    }

    
#ifdef __cplusplus
}
#endif

/**
* Table of methods associated with a single class.
*/
static JNINativeMethod gMethods[] = {
		{"InstallLayers", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z", (void*)InstallLayers},
		{"DownloadFile", "(Ljava/lang/String;Ljava/lang/String;Ljava/util/ArrayList;ILcom/map/editor/Progress;I)Z", (void*)DownloadFile},
		{"CancelDownload", "()Z", (void*)CancelDownload},
		{"UploadFile", "(Ljava/lang/String;Ljava/lang/String;Ljava/util/ArrayList;ILcom/map/editor/Progress;ILjava/lang/String;)Z", (void*)UploadFile},
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
		{"UpdatePois","(Ljava/util/ArrayList;Ljava/lang/String;Ljava/lang/String;Ljava/util/ArrayList;Lcom/map/editor/Progress;)I",(void*)UpdatePois},
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
		{"InstallGdb","(Ljava/lang/String;Ljava/lang/String;Lcom/map/editor/Progress;)Z",(void*)InstallGdb},
		{"LocationMacth","(Ljava/lang/String;)Ljava/lang/String;",(void*)LocationMacth},
		{"CaclAngle","(Ljava/lang/String;Ljava/lang/String;I)D",(void*)CaclAngle},
		{"PolygonSelectLines","(Ljava/lang/String;Z)Ljava/util/ArrayList;",(void*)PolygonSelectLines},
		{"RepalceGDB", "()Z", (void*)RepalceGDB},
		{"GetDownloadOrUploadStatus", "(Ljava/lang/String;Ljava/lang/String;I)Ljava/lang/String;", (void*)GetDownloadOrUploadStatus},
        {"UpdateInfo","(Ljava/lang/String;)I",(void*)UpdateInfo},
		{"GetEditHistroyByFid", "(Ljava/lang/String;)Ljava/lang/String;", (void*)GetEditHistroyByFid},
		{"QueryFeatures", "(Ljava/lang/String;Ljava/lang/String;)Ljava/util/ArrayList;", (void*)QueryFeatures},
        {"QueryFeaturesBySql", "(Ljava/lang/String;Ljava/lang/String;)Ljava/util/ArrayList;", (void*)QueryFeaturesBySql},
		{"RobTask", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", (void*)RobTask},
		{"QueryTableByCondition","(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZII)Ljava/util/ArrayList;",(void*)QueryTableByCondition},
		{"InsertTrackPoint","(Lcom/map/editor/data/TrackCollection;)I",(void*)InsertTrackPoint},
		{"GetDownloadPrepared","(Ljava/lang/String;)Ljava/lang/String;",(void*)GetDownloadPrepared},
		{"DownloadFile_2","(Ljava/lang/String;Lcom/map/editor/Progress;)V",(void*)DownloadFile_2},
		{"GetUploadPrepared","(Ljava/lang/String;)Ljava/lang/String;",(void*)GetUploadPrepared},
		{"UploadFile_2","(Ljava/lang/String;Lcom/map/editor/Progress;)V",(void*)UploadFile_2},
        {"InsertFeature","(Ljava/lang/String;Ljava/lang/String;)I",(void*)InsertFeature},
        {"UpdateFeature","(Ljava/lang/String;Ljava/lang/String;)I",(void*)UpdateFeature},
        {"DeleteFeature","(Ljava/lang/String;I)I",(void*)DeleteFeature},
        {"PoiStatistics","(Ljava/lang/String;)Ljava/util/ArrayList;",(void*)PoiStatistics},
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


