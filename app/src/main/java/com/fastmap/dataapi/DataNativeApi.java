package com.fastmap.dataapi;

import java.util.ArrayList;

import com.fastmap.dataapi.data.TrackCollection;

public class DataNativeApi {
    /**
     * 捕捉POI
     * @param box wkt形式
     * @param point wkt形式
     * @return 所选所有POI的 Json字符串
     */
    public static native ArrayList<String> SnapPoint(String box, String point);
    
    
    /**
     * 捕捉道路
     * @param box wkt形式
     * @param point wkt形式
     * @return 所选道路的 Json字符串
     */
    public static native String SnapLine(String box, String point);
    
    
    /**
     * 选择POI
     * @am box Wkt格式的矩形
	 * @return box中的POI列表par
     */
    public static native ArrayList<String> SelectPois(String box);
    
    /**
     * 根据fid获取POI
     * @param fid
     * @return
     */
    public static native String GetTotalPoiByFid(String fid);
    
    /**
     * 安装sqlite数据
     * @param sDownloadSqlite
     * @param sExistsSqlite
     * @param layers
     * @return true means success, false means failed
     */
    public static native boolean InstallLayers(String sDownloadSqlite, String sExistsSqlite, String layers);
    
    /**
     * 安装json数据
     * @param sExistDB
     * @param sJsonFile
     * @param layers
     * @return true means success, false means failed
     */
    public static native boolean InstallJsonData(String sExistDB, String sJsonFile, String layers);
    
    /**
     * 下载
     * @param sUser
     * @param ids  projectids for poi, grids for tips
     * @param nType
     * @param progress
     * @param cmd
     * @return true means success, false means failed
     */
 	public static native boolean DownloadFile(String sToken, String sUser, ArrayList<String> ids, int nType, Progress progress, int cmd);
 	
 	public static native boolean CancelDownload();
	
 	
 	/**
     * 上传
     * @param sUser
     * @param ids  projectids for poi, grids for tips
     * @param nType
     * @param progress
     * @param cmd
     * @return true means success, false means failed
     */
	public static native boolean UploadFile(String sToken, String sUser, ArrayList<String> ids, int nType, Progress progress, int cmd, String deviceId);
	
	public static native boolean CancelUpload();

 	
 	/**
     *插入POI
     * @param POI Json
     * @return
     */
	public static native int InsertPoi(String json, String sUserId, String sProjectId);
	
    /**
     * 更新POI
     * @param POI Json
     * @return
     */
	public static native int UpdatePoi(String json, String sUserId, String sProjectId);
	
	/**
     * 删除POI
     * @param rowId
     * @return
     */
	public static native int DeletePoi(int rowId, String sUserId, String sProjectId);
	
	/**
     * 设置根目录
     * @param 根目录
     * @return
     */
	public static native void SetRootPath(String rootPath);
	
	/**
     * 设置UserId
     * @param UserId
     * @return
     */
	public static native void SetUserId(String userId);
	
	/**
     * 获取数据路径
     * @return
     */
	public static native String GetDataPath();
	
	/**
     * 点选Tips
     * @param box Wkt格式的矩形
     * @param point wkt形式
     * @return box中的Tips列表
     */
    public static native ArrayList<String> SelectTips(String box, String point);
	
	/**
     *插入Tips
     * @param Tips Json
     * @return -1:error 0:success
     */
    public static native int InsertTips(String json);
	
    /**
     * 更新Tips
     * @param Tips Json
     * @return -1:error 0:success
     */
	public static native int UpdateTips(String json);
	
	/**
     * 查询sql
     * @param sTable
     * @param sCondition
     * @param type
     * @return object of list
     */
	public static native ArrayList<String> SelectMetaData(String sTable, String sConfition, int type);
	
	/**
     * 获得poi照片路径
     * @return path of poi
     */
	public static native String GetPoiPhotoPath();
	
	/**
     * 获得tips照片路径
     * @return path of tips
     */
	public static native String GetTipsPhotoPath();
	
	/**
     * 通过Pid获取Rdline
     * @param Pid
     * @return Rdline Json
     */
	public static native String GetRdLineByPid(int pid);
	
	/**
     * 通过条件查询poi个数，条件包括：新增、更新、删除
     * @return count of poi
     */
	public static native int GetCountByCondition(String sTableName, int nType);
	
	/**
     * 通过条件查询项目的poi个数，条件包括：新增、更新、删除
     * @return count of poi
     */
	public static native int GetProjectCountByCondition(String sTableName, String sProject, int nType);
	
	/**
     * 分页查询
     * @return object of poi
     */
	public static native ArrayList<String> GetSignalLayerByCondition(String sTableName, int nType, int nPageSize, int nOffset);
	
	
	/**
     * 初始化版本信息
     */
	public static native boolean InitVerstionInfo(int version);
	
	/**
     * 删除tips
     * @param rowKey
     * @return
     */
	public static native int DeleteTips(String rowKey);
	
	/**
	 * @ArrayList<String> gridsList grids的id list
	 *
	 * @return 0 success; -1 false
	 */
	public static native int UpdateGridStatus(ArrayList<String> gridsList);
	
	/**
	 * @bref 更新box范围内的grid的状态
	 * @para dminLon box最小经度
	 * @para dmaxLon box最大经度
	 * @para dminLat box最小纬度
	 * @para dmaxLat box最大纬度 
	 *
	 * @return 0 success; -1 false
	 */
	public static native int UpdateGridStatusByBox(double dminLon, double dmaxLon, double dminLat, double dmaxLat);
	
	/**
     *插入GpsLine
     * @param GpsLine Json
     * @return -1:error 0:success
     */
	public static native int InsertGpsLine(String json);
	
	/**
	 * @bref 获取单个用户的项目信息
	 * @para sToken 
	 * @para sProjectId 
	 *
	 * @return ArrayList of Result
	 */
	public static native ArrayList<String> GetProjectInfo(String sToken);
	
	/**
	 * @bref 获取单个项目的详细信息
	 * @para sToken 
	 * @para sProjectId 
	 * @return ArrayList of Result
	 */
	public static native ArrayList<String> GetProjectDetail(String sToken, String sProjectId);
	
	/**
	 * @bref 获取单个项目的任务信息
	 * @para sToken 
	 * @para sProjectId 
	 * @return ArrayList of Result
	 */
	public static native ArrayList<String> GetTaskDetail(String sToken, String sProjectId);
	
	/**
     * 指定线几何捕捉
     * @param path wkt形式
     * @param point wkt形式
	 * @param disThreshold 距离阈值，若点与线距离超过该值则捕捉结果为空
     * @return 捕捉结果 Json字符串
     */
	 public static native String GivenPathSnap(String path, String point, double disThreshold);

	/**
	 * 生成新增tips的rowkey
	 *
	 * @param sourceType 字符串
	 * @return rowkey    字符串
	 */
	public static native String GetTipsRowkey(String sourceType);
	
	/**
	 * @bref 获取版本信息
	 * @return version of Result
	 */
	public static native String GetCoremapModelVersion(int type);
	
	/**
     * 捕捉Rd Node
     * @param box wkt形式
     * @param point wkt形式
     * @return 捕捉Node的 Json字符串
     */
    public static native String SnapRdNode(String box, String point);
	
	/**
     * 根据rowkey获取tips
     * @param rowkey
     * @return
     */
    public static native String GetTipsByRowkey(String rowkey);
    
    /**
	 * @bref 安装GDB
	 * @return true is success
	 */
	public static native boolean InstallGdb(String sPath, String flag, Progress progress);
	
	/**
	 * @bref 计算角度
	 * @param wkt
	 * @param linkId
	 * @param type
	 * @return true is success
	 */
	public static native double CaclAngle(String wktPoint, String linkId, int type);
	
	/**
	 * @bref LocationMacth
	 * @param locationJson
	 * @return true is success
	 */
	public static native String LocationMacth(String locationJson);
	
	//public static native String LinkMacth(String wkt, boolean isPoint);
	/**
     * 框选RdLine及测线
     * @param region Wkt格式的多边形
     * @param isIntersectIncluded false:不包含与region相交元素，true：包含与region相交元素
     * @return 线列表
     */
    public static native ArrayList<String> PolygonSelectLines(String region,boolean isIntersectIncluded);
    //public static native ArrayList<String> PolygonSelectLines(String region);
    /*
     * 替换gdb
     * 
     */
    public static native boolean RepalceGDB();
    
    /**
     * 批量更新poi数据
     * @param jsonList        多个poi数据的json列表格式
     * @param userid          用户id 
     * @param projectid       项目id
     * @param failIndexList   更新失败的poi数据的索引值
     * @param progressObj     更新的进度回调对象
     * @return                更新结果：-1表示失败，0表示成功。
     */
    public static native int UpdatePois(ArrayList<String> jsonList, String userid, String projectid, ArrayList<Integer> failIndexList, Progress progressObj);
    
    public static native String GetDownloadOrUploadStatus(String projectId, String userId, int type);
    
    
    /**
     * 插入情报
     * @param json
     * @return
     */
    public static native int InsertInfos(String json);
    
    /**
     * 更新情报
     * @param Info Json
     * @return -1:error 0:success
     */
    public static native int UpdateInfo(String json);
    
    /**
     * 获取履历
     * @param fid of poi
     * @return edithistroy
     */
    public static native String GetEditHistroyByFid(String fid);
	
	/**
     * 属性查询coremap
     * @param tableName coremap表名
     * @param searchQuery json格式的查询条件，如：{"globalId":"foobar"}
     * @return 根据查询条件获取的查询结果，json格式，如 {"layer":"edit_pois","feature":{}}
     */
    public static native ArrayList<String> QueryFeatures(String tableName, String searchQueryJson);
    
    
    /**
     * GetDownloadPrepared
     * @param input json
     * @return json
     */
    public static native String GetDownloadPrepared(String input);
	
    /**
     * DownloadFile_2
     * @param input json{{"token":"000003E400O7GPV64C43579179ACE8D17C9FC8D057F40BAF","path":"","project":"2119112190","url":"http://192.168.4.189/resources/editsupport/download/test/20160519/2119112190/4396_2119112190_20160519175245.zip","md5":"2beaf30d39ca51c3b72c9c182a32cec6","type":0,"connectTime":5000}}
     * @param progress
     */
	public static native void   DownloadFile_2(String input, Progress progress);
	
	/**
     * GetUploadPrepared
     * @param input json {"token":"000003E400O7GPV64C43579179ACE8D17C9FC8D057F40BAF","project":"2119112190","user":"4396","deviceid":"00000000-54b3-e7c7-0000-000046bffd97","time":"","type":0,"connectTime":5000}
     * @return json {"serverMsg":"","conncode":-2,"filename":"","path":"","time":"","total":0,"add":0,"update":0,"delete":0}
     */
	public static native String GetUploadPrepared(String input);
	
	/**
     * UploadFile_2
     * @param input json{{"token":"000003E400O7GPV64C43579179ACE8D17C9FC8D057F40BAF","project":"2119112190","path":"E:\\c++\\data\\collect\\4396\\upload\\poi\\IncrementalData_4396_2119112190_20160519182848.zip","deviceid":"00000000-54b3-e7c7-0000-000046bffd97","filename":"IncrementalData_4396_2119112190_20160519182848.zip","type":0,"connectTime":5000}}
     * @param progress
     */
	public static native void   UploadFile_2(String input, Progress progress);
    
    /**
     * sql查询coremap
     * @param tableName coremap表名
     * @param sql 查询语句
     * @return 根据查询条件获取的查询结果，json格式，如 {"layer":"edit_pois","feature":{}}
     */
    public static native ArrayList<String> QueryFeaturesBySql(String tableName, String sql);
    
    /**
     * rob task
     * @param token
     * @param projectId
     * @param taskId
     * @return edithistroy
     */
    public static native String RobTask(String token, String projectId, String taskId);
    
    /**
     * @bref     定制分页查询，如分页查询poi ，或按距离排序查询情报
     * @param    strTable                表名
     * @param    strWhere                where的限定条件，不包括where关键词
     * @param    strOderbyFiedOrWktPoint 按字段排序的字段名，或按距离排序的wkt点
     * @param    strGeoFieldName         几何字段名
     * @param    isOderbyFied            是否是按字段排序
     * @param    isDesc                  是否降序
     * @param    pagesize                page大小
     * @param    offset                  查询起始row
     * @return   返回分页查询的features的json串信息
     */
    public static native ArrayList<String> QueryTableByCondition(String strTable, String strWhere,String strOderbyFiedOrWktPoint, String strGeoFieldName, boolean isOderbyFied, boolean isDesc , int pagesize, int offset);
    /**
     * @bref 插入轨迹点
     * 
     * @param trackPoint 轨迹点信息
     * @return -1，失败；0，成功
     */
    public static native int InsertTrackPoint(TrackCollection trackPoint);
    
    /**
     * @bref 统计poi不同日期的增删改的数量
     * @param poitTablename poi表名
     * @return 返回统计查询的jsons信息
     */
    public static native ArrayList<String> PoiStatistics(String poitTablename);
     /** 插入记录（该方法不涉及业务逻辑的维护）
     * @param tableName 数据表名（不限于coremap）
     * @param json 属性json字符串
     * @return 0:成功 －1:失败
     */
    public static native int InsertFeature(String tableName, String json);
    
    /**
     * 更新记录（该方法不涉及业务逻辑的维护）
     * @param tableName 数据表名（不限于coremap）
     * @param json 属性json字符串（需要包含rowid）
     * @return 0:成功 －1:失败
     */
    public static native int UpdateFeature(String tableName, String json);
    
    /**
     * 删除记录（该方法不涉及业务逻辑的维护）
     * @param tableName 数据表名（不限于coremap）
     * @param rowId 需要删除记录的rowid
     * @return 0:成功 －1:失败
     */
    public static native int DeleteFeature(String tableName, int rowId);
    
    /**
     * 通过Where条件查询poi个数
     * @return count of poi
     */
	public static native int GetCountByWhere(String sTableName, String sWhere);
	
	/*
     * @bref     按日期统计轨迹长度
     * @param    userId             用户id
     * @return   results            返回的json结果：例如[{count:6,date:"20160404",trackLength:50.0}]
     */
	public static native ArrayList<String> TrackStatistics(String userId);
	
	/*
     * @bref     按userid和日期删除轨迹点
     * @param    userId         用户id
     * @param    date           日期
     * @return                  0，表示正常；-1，有异常。
     */
	public static native int DeleteTrack(String sUserId, String sDate);

    /*
     * @bref     按范围删除poi和情报
     * @param    projectId         项目id
     * @param    region            范围
     * @return                  0，表示正常；-1，有异常。
     */
    public static native int DeletePoiAndInfoByRegion(String projectId, String region);
}

