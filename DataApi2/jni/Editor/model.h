#ifndef MODEL_H
#define MODEL_H

#include "Editor_Define.h"
#include <iostream>
#include <string>
#include "Geometry.h"
#include "Logger.h"
#include "DataTransfor.h"
//#include <nds_sqlite3.h>
#include <sqlite3.h>
#include "document.h"
#include "prettywriter.h"
#include "stringbuffer.h"
#include <document.h>
#include <set>
#include <vector>

using namespace rapidjson;

namespace Model
{
    class EDITOR_API Point
    {
    public:
        std::string globalId;
        std::string fid;
        std::string project;
        std::string display_style;
        std::string display_text;
	    std::string name;
        int pid;
	    std::string meshid;
	    std::string kindCode;
        int evaluatePlanning;
        int qtStatus;
	    std::string guide;
	    std::string address;
        std::string postCode;
        std::string adminCode;
        std::string level;
	    int open24H;
	    std::string relateParent;
	    std::string relateChildren;
	    std::string names;
	    std::string addresses;
	    std::string contacts;
        std::string foodtypes;
        std::string parkings;
        std::string hotel;
        std::string sportsVenues;
        std::string chargingStation;
        std::string chargingPole;
        std::string gasStation;
        std::string indoor;
        std::string attachments;
        std::string brands;
        std::string rawFields;
        int submitStatus;
        std::string sourceCollection;
        int lifecycle;
	    std::string edits;
        std::string simple_parentFid;
	    int auditStatus;
        int submitStatus_desktop;
	    std::string latestMergeDate;
        double longitude;
	    double latitude;
	    int task_status;
        std::string vipFlag;
        int type;
	    std::string t_operateDate;
        std::string info_globalId;

    public:
        int ParseJson(std::string sJson);
        std::string ToJson();
        std::string ToUploadJson();
        int ParseSqlite(sqlite3_stmt *statement);
	    int InsertSqliteTable(sqlite3* db);
        int InsertSqliteTableWithoutTransaction(sqlite3* db);
        int UpdateSqliteTable(sqlite3* db);
        int UpdateSqliteTableWithoutTransaction(sqlite3* db);
        int GetMaxRowId(sqlite3* db);
        int GetRowId(sqlite3* db, std::string key, std::string value);
        std::string GetCorrectString(std::string sIn);
        void SetDisplayInfo();
        std::string SetPrimaryKey();
        
        std::string GetAsString(sqlite3_stmt *statement, unsigned int index);
    private:
        /** 
        * @brief 获得工作状态
        * @return 0表示无需作业  1表示待作业  2表示已作业（更新）  3表示已作业（删除）  4表示已作业（新增）
        */
        int GetWorkStatus() const;

        /** 
        * @brief 获取poi重要性
        * @return 0表示非重要  1表示重要
        */
        int GetImportance() const;

        /** 
        * @brief 获取种别码
        * @return 获取种别码
        */
        std::string GetKindCodeToDisplayInfo() const;
    };

	//lee add
	class Tips;
	class tips_geo
	{
	public:
		enum DisplayGeoType
		{
			displayGeoUnKnown = -1,
			displayGeoPoint =0,
			displayGeoLine = 1,
			displayGeoPolygon = 2
		};

		enum operatorType
		{
			operatorUnKnown = -1,
			operatorInsert = 0,
			operatorUpdate =1
		};

		std::string strUuid;
		int geoType;
		std::string sourceType;
		std::string display_style;
		std::string geometry;

		tips_geo()
		{
			strUuid = "";
			geoType = displayGeoUnKnown;
			sourceType = "";
			display_style = "";
		}
		virtual~tips_geo()
		{

		}
		virtual void setValues(const Tips& tip)
		{

		}
		virtual int InsertSqliteTable(const Tips& tip, sqlite3* db)
		{
			return 0;
		}
		virtual int UpdateSqliteTable(const Tips& tip, sqlite3* db)
		{
			return 0;
		}
		virtual std::string getGeoTableName()const
		{
			return "";
		}

		virtual void Update(const Tips& tip, sqlite3* db, int operateType)
		{

		}
	};

	class tips_point:public tips_geo
	{
	public:
		tips_point();
		~tips_point();
		virtual void setValues(const Tips& tip);
		virtual int InsertSqliteTable(const Tips& tip, sqlite3* db);
		virtual int UpdateSqliteTable(const Tips& tip, sqlite3* db);
		virtual std::string getGeoTableName()const;
		virtual void Update(const Tips& tip, sqlite3* db, int operateType);

		//处理特殊的类型，如桥
		enum DisplayPointType
		{
			eDisplayPointType_normalPoint = 0,
			eDisplayPointType_startPoint = 1,
			eDisplayPointType_endPoint = 2
		};
		void setDisplayPointType(DisplayPointType displayPointType);

		std::string GetPointWktGeo(const Tips& tip);

		DisplayPointType m_displayPointType;
	};
	class tips_line:public tips_geo
	{
	public:
		tips_line();
		~tips_line();
		virtual void setValues(const Tips& tip);
		virtual int InsertSqliteTable(const Tips& tip, sqlite3* db);
		virtual int UpdateSqliteTable(const Tips& tip, sqlite3* db);
		virtual std::string getGeoTableName()const;
		virtual void Update(const Tips& tip, sqlite3* db, int operateType);
	};
	class tips_polygon:public tips_geo
	{
	public:
		tips_polygon();
		~tips_polygon();
		virtual void setValues(const Tips& tip);
		virtual int InsertSqliteTable(const Tips& tip, sqlite3* db);
		virtual int UpdateSqliteTable(const Tips& tip, sqlite3* db);
		virtual std::string getGeoTableName()const;
		virtual void Update(const Tips& tip, sqlite3* db, int operateType);
	};

	class tips_geo_component
	{
	public:
		static int InsertSqliteTable(const Tips& tip, const tips_geo& tipsGeo, sqlite3* db);
		static int UpdateSqliteTable(const Tips& tip, const tips_geo& tipsGeo, sqlite3* db);
	};

    class EDITOR_API Tips
    {
        public:
		enum DataSourceType
		{
			DataSourceUnknown		= -1,   //未知
			DataSourceFromServer	= 0,	//来自后台安装
			DataSourceFromApp		= 1		//来自app
		};
        std::string rowkey;
        int s_featureKind;
        std::string s_project;
	    int s_sourceCode;
	    std::string s_sourceId;
	    std::string s_sourceType;
	    int s_sourceProvider;
        int s_reliability;
        double longitude;
	    double latitude;
	    std::string g_guide;
	    int t_lifecycle;
        int t_status;
        std::string t_operateDate;
        int t_handler;
        std::string deep;
        std::string display_style;
        std::string attachments;
		
		std::string g_location; //统一转存wkt格式
		int m_dataSource;//0,后台安装；1，app层。
        double angle;
        int t_command;
		int t_sync;
    public:
		~Tips();
        int ParseJson(std::string sJson);
	    int InsertSqliteTable(sqlite3* db);
        int UpdateSqliteTable(sqlite3* db);
        void SetDisplayStyle(const std::string& json);

		std::vector<tips_geo*> BuildGeosBySourceType();

		tips_geo* CreateGeo(tips_geo::DisplayGeoType geotype);
		void setDataSource(int dataSource);

		//注册观察者
		void Attach(tips_geo*pObs);
		//注销观察者
		void Detach(tips_geo*pObs);
		//注销所有观察者
		void DetachAll();
		//若状态变化，则遍历观察者，逐个通知更新
		void Notify(sqlite3* db, int operateType);

		void Recycle();

		std::set<tips_geo*>  m_setObs;

	private:

		char * _strlwr(char *str);
    };
	
	

    class EDITOR_API ProjectUser
    {
    public:
        std::string sProjectId;
        std::string sUserId;
        int         nType;
        std::string download_latest;
        std::string download_url;
        std::string upload_latest;
        std::string upload_url;
        std::string	zip_file;
        std::string failed_reason;
        std::string details;

    public:
        int download_type;
        int upload_type;
    public:
        ProjectUser();
        int ParseSqlite(sqlite3_stmt *statement);
        int InsertSqliteTable(std::string sDBPath);
        int InsertOrUpdateSqliteTable(std::string sDBPath);
    };
	
	enum TipsType
	{
        PointSpeedLimit = 1101,
		RoadKind =1201,
		OrdinaryTurnRestriction = 1302,
		CrossName = 1704,
		LaneConnexity = 1301,
		RoadDirect = 1203,
        Bridge = 1510,
		RoadName = 1901,
		SurveyLine = 2001,
        Hitching = 1803,
        HighwayBranch = 1407,
        RegionalRoad = 1604,
        MultiDigitized = 1501,
        UnderConstruction = 1514,
        Gsc = 1801
	};

    enum TypeFlag
    {
        IntFlag = 0,
        StringFlag = 1
    };

    class EDITOR_API ParaKindTop
    {
    public:
        int id;
        std::string name;
        int code;

    public:
        int ParseSqlite(sqlite3_stmt *statement);

        std::string ToJson();
    };

    class EDITOR_API ParaKindMedium
    {
    public:
        int id;
        int top_id;
        std::string name;
        int code;
        std::string description;

    public:
        int ParseSqlite(sqlite3_stmt *statement);

        std::string ToJson();
    };

    class EDITOR_API ParaKind
    {
    public:
        int id;
        int mediun_id;
        std::string name;
        int code;
        std::string kind_code;
        int region;
        int type;
        std::string description;
        std::string full_name;

    public:
        int ParseSqlite(sqlite3_stmt *statement);

        std::string ToJson();
    };

    class EDITOR_API ChargingCarid
    {
    public:
        int id;
        std::string chain_name;
        std::string chain_code;

    public:
        int ParseSqlite(sqlite3_stmt *statement);

        std::string ToJson();
    };

    class EDITOR_API ParaChain
    {
    public:
        int id;
        std::string name;
        std::string code;
        std::string region;
        int type;
        int category;
        int weight;

    public:
        int ParseSqlite(sqlite3_stmt *statement);

        std::string ToJson();
    };

    class EDITOR_API ParaControl
    {
    public:
        int id;
        int kind_id;
        std::string kind_code;
        int kind_change;
        int parent;
        int important;
        std::string name_keyword;
        std::string level;
        std::string eng_permit;
        int agent;
        int region;
        int tenant;
        int extend;
        int extend_photo;
        int photo;
        int interna;
        int chain;
        int rel_cs;
        int add_cs;
        std::string mhm_des;
        int disp_onlink;

    public:
        int ParseSqlite(sqlite3_stmt *statement);

        std::string ToJson();
    };

    class EDITOR_API ParaFood
    {
        int id;
        int kind_id;
        std::string food_name;
        int food_code;
        int food_type;

    public:
        int ParseSqlite(sqlite3_stmt *statement);

        std::string ToJson();
    };

    class EDITOR_API ParaIcon
    {
    public:
        int id;
        std::string idcode;
        std::string name_in_nav;
        int type;

    public:
        int ParseSqlite(sqlite3_stmt *statement);

        std::string ToJson();
    };

    class EDITOR_API ParaKindChain
    {
    public:
        int id;
        int kind_id;
        int chain_id;
        std::string chain_name;
        std::string chain;
        std::string food_type;
        std::string level;
        std::string chain_type;
        std::string kind_code;

    public:
        int ParseSqlite(sqlite3_stmt *statement);

        std::string ToJson();
    };

    class EDITOR_API ParaSensitiveWords
    {
    public:
        int id;
        std::string sensitive_word;
        std::string type;

    public:
        int ParseSqlite(sqlite3_stmt *statement);

        std::string ToJson();
    };

    class EDITOR_API ParaTel
    {
    public:
        int id;
        std::string province;
        std::string city;
        std::string code;
        std::string tel_len;
        std::string city_code;

    public:
        int ParseSqlite(sqlite3_stmt *statement);

        std::string ToJson();
    };

    class EDITOR_API GDBBkFace
    {
    public:
        int                     pid;
        std::string             geometry;;
        std::string             display_style;
        std::string             display_text;
        std::string             meshid;
        int                     kind;
        std::string             op_date;
        int op_lifecycle;
    public:
        int                     ParseJson(std::string sJon);
        int                     InsertSqliteTable(sqlite3* db);
        int                     UpdateSqliteTable(sqlite3* db);
    };

    class EDITOR_API GDBBkLine
    {
    public:
        int                     pid;
        std::string             geometry;;
        std::string             display_style;
        std::string             display_text;
        std::string             meshid;
        int                     kind;
        std::string             op_date;
        int                     op_lifecycle;
    public:
        int                     ParseJson(std::string sJon);
        int                     InsertSqliteTable(sqlite3* db);
        int                     UpdateSqliteTable(sqlite3* db);
    };

    class EDITOR_API GDBRdLine
    {
    public:
        int                     pid;
        std::string             geometry;
        std::string             display_style;
        std::string             display_text;
        std::string             meshid;
        int                     kind;
        int                     direct;
        int                     appInfo;
        int                     tollInfo;
        int                     multiDigitized;
        int                     specialTraffic;
        int                     fc;
        int                     laneNum;
        int                     laneLeft;
        int                     laneRight;
        int                     isViaduct;
        int                     paveStatus;
        std::string             forms;
        std::string             styleFactors;
        std::string             speedLimit;
        std::string             op_date;
        int                     op_lifecycle;

    public:
        int                     ParseJson(std::string sJson);
        int                     InsertSqliteTable(sqlite3* db);
        int                     InsertSqliteTableWithoutTransaction(sqlite3* db);
        int                     UpdateSqliteTable(sqlite3* db);
        int                     UpdateSqliteTableWithoutTransaction(sqlite3* db);
        std::string             GetCorrectString(std::string sIn);
        void                    SetDisplayStyle();
        int                     SetDisplayStyleFromStyleFactors();
    };
	class EDITOR_API Infor
    {
    public:
        std::string             globalId;
        std::string             _id;
        int                     b_featureKind;
        int                     b_sourceCode;
        std::string             b_sourceId;
        int                     b_reliability;
        std::string             g_location;
        std::string             g_guide;
        std::string             t_expectDate;
        std::string             t_publishDate;
        std::string             t_operateDate;
        int                     t_status;
        int                     t_payPoints;
        std::string             h_projectId;
        std::string             i_kindCode;
        std::string             i_name;
        std::string             i_address;
        std::string             i_telephone;
        std::string             i_brandCode;
        std::string             i_memo;
        int                     i_proposal;
        int                     i_level;
        int                     i_infoType;
        int                     c_isAdopted;
        std::string             c_denyReason;
        int                     c_featureKind;
        int                     c_userId;
        int                     c_pid;
        std::string             c_fid;
        std::string             c_project;
        int                     r_featureKind;
        int                     r_pid;
        std::string             r_fid;
        std::string             r_similarity;
        std::string             f_array;
        std::string             geometry;
        int                     task_status;
        int                     submitStatus;
    public:
        Infor();
    public:
        int ParseJson(std::string sJson);
        std::string ToUploadJson();
        int ParseSqlite(sqlite3_stmt *statement);

        int InsertSqliteTableWithoutTransaction(sqlite3* db);

        int UpdateSqliteTableWithoutTransaction(sqlite3* db);
        int SetPrimaryKey(std::string& id);
        int MaintainRelation();
    };
}

#endif
