#ifndef Editor_BussinessConstant_h
#define Editor_BussinessConstant_h

#include <string>

const std::string ResourceSplitSymbol = "_";

const std::string GdbDownloadFileName = "gdbdata.sqlite";

const std::string PatternImageDownloadFileName = "image.sqlite";

//icon code
const std::string IconNameStr = "iconName";
const std::string RowStr = "row";
const std::string ColumnStr = "column";

const std::string BridgeDispaly_Line = "10";
const std::string RoadNameDisplay_Line = "21";
const std::string HitchingDisplay_Line = "-2";
const std::string UnderConstructionDisplay_Line = "22";
const int SurveyLineDisplayLineCode = 25;

const std::string ActualRestrictionFlag = "0";
const std::string TheoreticalRestrictionFlag = "1";
const std::string ActualTimeRestrictionFlag = "2";
const std::string TheoreticalTimeRestrictionFlag = "3";

#define SpeedLimitStart "0"
#define SpeedLimitEnd "1"
#define TheoreticSpeedLimitStart "2"
#define TheoreticSpeedLimitEnd "3"

//地球半径 WGS-84 椭球
#define EARTH_RADIUS 6378137

#define PI 3.1415926

#define PrecisionMap 0.00001

//Tips几何子表
#define TipsGeoVisible 1
#define TipsGeoInVisible 0
const std::string UuidStr = "uuid";
const std::string GeoTypeStr = "geoType";
const std::string TipsGeoSourceTypeStr = "sourceType";
const std::string VisibilityStr = "visibility";

//Geometry
const std::string PointStr = "Point";
const std::string LineStringStr = "LineString";
const std::string PolygonStr = "Polygon";
const std::string MultiLineStringStr = "MultiLineString";
const std::string CoordinatesStr = "coordinates";

//Tips column name
const std::string Tips_Column_SsourceType = "s_sourceType";
const std::string Tips_Column_Deep = "deep";
const std::string Tips_Column_Geo = "geo";

const std::string RowkeyStr = "rowkey";
const std::string Display_Style = "display_style";
const std::string Kind_Str = "kind";
const std::string Geometry_Str = "geometry";
const std::string GLocation_Str = "g_location";
const std::string TLifecycle_Str = "t_lifecycle";
const std::string THandler_Str = "t_handler";
const std::string TStatus_Str = "t_status";
const std::string TSync_Str = "t_sync";
const std::string Attachments_Str = "attachments";

//track_collection
const std::string track_collectionTableName = "track_collection";
const std::string track_collection_IdStr="id";
const std::string track_collection_LatitudeStr="latitude";
const std::string track_collection_LongitudeStr="longitude";
const std::string track_collection_DirectionStr="direction";
const std::string track_collection_SpeedStr="speed";
const std::string track_collection_RecordTimeStr="recordTime";
const std::string track_collection_UserIdStr="userId";
const std::string track_collection_SegmentIdStr="segmentId";
//track_segment
const std::string track_segmentTableName = "track_segment";
const std::string track_segment_IdStr="id";
const std::string track_segment_GeometryStr="GEOMETRY";
const std::string track_segment_LODStr="LOD";
const std::string track_segment_StartTimeStr="StartTime";
const std::string track_segment_EndTimeStr="EndTime";
const std::string track_segment_UserIdStr="UserId";
const std::string track_segment_SegmentIdStr="segmentId";

//Tips渲染样式
const std::string RegionalRoadPolygonStyle = "301924352";

//Info column name
const std::string InfoTableName = "edit_infos";
const std::string Info_globalId = "globalId";
const std::string Info_id = "_id";
const std::string Info_bfeatureKind = "b_featureKind";
const std::string Info_bsourceCode = "b_sourceCode";
const std::string Info_bsourceId = "b_sourceId";
const std::string Info_breliability = "b_reliability";
const std::string Info_location = "g_location";
const std::string Info_guide = "g_guide";
const std::string Info_expectDate = "t_expectDate";
const std::string Info_publishDate = "t_publishDate";
const std::string Info_operateDate = "t_operateDate";
const std::string Info_status = "t_status";
const std::string Info_payPoints = "t_payPoints";
const std::string Info_hprojectId = "h_projectId";
const std::string Info_ikindCode = "i_kindCode";
const std::string Info_name = "i_name";
const std::string Info_address = "i_address";
const std::string Info_telephone = "i_telephone";
const std::string Info_brandCode = "i_brandCode";
const std::string Info_memo = "i_memo";
const std::string Info_proposal = "i_proposal";
const std::string Info_level = "i_level";
const std::string Info_isAdopted = "c_isAdopted";
const std::string Info_denyReason = "c_denyReason";
const std::string Info_cfeatureKind = "c_featureKind";
const std::string Info_userId = "c_userId";
const std::string Info_cpid = "c_pid";
const std::string Info_cfid = "c_fid";
const std::string Info_cProject = "c_project";
const std::string Info_r_featureKind = "r_featureKind";
const std::string Info_r_pid = "r_pid";
const std::string Info_r_fid = "r_fid";
const std::string Info_r_similarity = "r_similarity";
const std::string Info_f_array = "f_array";
const std::string Info_Geometry = "geometry";
const std::string Info_task_status = "task_status";
const std::string Info_submitStatus = "submitStatus";

//Poi column name
const std::string PoiTableName = "edit_pois";
const std::string Poi_globalId = "globalId";
const std::string Poi_fid = "fid";
const std::string Poi_project = "project";
const std::string Poi_displaystyle = "display_style";
const std::string Poi_displaytext= "display_text";
const std::string Poi_name = "name";
const std::string Poi_type = "type";

const std::string Poi_id = "_id";
const std::string Poi_rowkey = "rowkey";

const std::string Poi_pid = "pid";
const std::string Poi_meshid = "meshid";
const std::string Poi_kindCode = "kindCode";
const std::string Poi_evaluatePlanning = "evaluatePlanning";
const std::string Poi_qtStatus = "qtStatus";
const std::string Poi_guide = "guide";
const std::string Poi_address = "address";
const std::string Poi_postCode = "postCode";
const std::string Poi_adminCode = "adminCode";
const std::string Poi_level = "level";
const std::string Poi_open24H = "open24H";
const std::string Poi_relateParent = "relateParent";
const std::string Poi_relateChildren = "relateChildren";
const std::string Poi_names = "names";
const std::string Poi_addresses = "addresses";
const std::string Poi_contacts = "contacts";
const std::string Poi_foodtypes = "foodtypes";
const std::string Poi_parkings = "parkings";
const std::string Poi_hotel = "hotel";
const std::string Poi_sportsVenues = "sportsVenues";
const std::string Poi_chargingStation = "chargingStation";
const std::string Poi_chargingPole = "chargingPole";
const std::string Poi_gasStation = "gasStation";
const std::string Poi_indoor = "indoor";
const std::string Poi_attachments = "attachments";
const std::string Poi_brands = "brands";
const std::string Poi_rawFields = "rawFields";
const std::string Poi_submitStatus = "submitStatus";
const std::string Poi_sourceCollection = "sourceCollection";
const std::string Poi_lifecycle = "lifecycle";
const std::string Poi_edits = "edits";
const std::string Poi_relateParent_parentFid = "relateParent_parentFid";
const std::string Poi_auditStatus = "auditStatus";
const std::string Poi_submitStatus_desktop = "submitStatus_desktop";
const std::string Poi_latestMergeDate = "latestMergeDate";
const std::string Poi_geometry = "geometry";
const std::string Poi_task_status = "task_status";
const std::string Poi_vipFlag = "vipFlag";
const std::string Poi_t_operateDate = "t_operateDate";

//table name
const std::string TableName_ProjectUser="project_user";
const std::string TableName_ProjectInfo="project_info";
const std::string TableName_TaskInfo="task_info";

#define PrecisionMap 0.00001

#define PrecisionGeo 10000

#endif