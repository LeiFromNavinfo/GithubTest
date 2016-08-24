#include <string>
#include <sstream>
#include "Geometry.h"
#include <document.h>
//#include <nds_sqlite3.h>
#include <sqlite3.h>
#include <vector>
#include <geos/io/WKBReader.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKBWriter.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>

namespace DataTransfor
{
	EDITOR_API std::string                    Wkb2Wkt(EditorGeometry::WkbGeometry* wkb);

	EDITOR_API std::string                    Box2Wkt(EditorGeometry::Box2D box);

    EDITOR_API std::string                    JSONToString(rapidjson::Document& Json);

    EDITOR_API EditorGeometry::Box2D          WktToBox2D(std::string wkt);

    EDITOR_API EditorGeometry::Point2D        WktToPoint2D(const std::string& wkt);

    EDITOR_API EditorGeometry::WkbGeometry*   Wkt2Wkb(const std::string& strWkt);

    EDITOR_API bool                           ExecuteNonQuery(const char* sql, sqlite3* db);

    EDITOR_API std::string                    GetPoiGlobalId(rapidjson::Document& doc);
    
    EDITOR_API std::string                    GetPoiDisplayStyle(rapidjson::Document& doc);
    
    EDITOR_API std::string                    GetTipsDisplayStyle(const std::string& tipsJson);
    
	EDITOR_API std::string                    GetTipsLineDisplayStyle(const std::string& type, const std::string& tipsDeep);
    
    EDITOR_API std::string                    GetSurveyLineDisplayStyle(const std::string& tipsDeep);

    EDITOR_API std::string                    GetConstantsDisplayStyle(const std::string& iconName);

    EDITOR_API std::string                    GetOrdinaryTurnRestrictionDisplayStyle(rapidjson::Document& doc);

    EDITOR_API std::string                    GetRoadKindDisplayStyle(rapidjson::Document& doc);

    EDITOR_API std::string                    GetCrossNameDisplayStyle(rapidjson::Document& doc);

    EDITOR_API std::string                    GetLaneConnexityDisplayStyle(rapidjson::Document& doc);

    EDITOR_API std::string                    GetRoadDirectDisplayStyle(rapidjson::Document& doc);

    EDITOR_API std::string                    GetRoadNameDisplayStyle(rapidjson::Document& doc);
    
    EDITOR_API std::string                    GetTipsSEPointDisplayStyle(int type, rapidjson::Document& doc, bool isStartPoint);

    EDITOR_API std::string                    GetBridgeDisplayStyle(rapidjson::Document& doc);

    EDITOR_API std::string                    GetPointSpeedLimitDisplayStyle(rapidjson::Document& doc);
    
    EDITOR_API std::string                    GetHitchingPointDisplayStyle(rapidjson::Document& doc);
    
    EDITOR_API std::string                    GetHighwayBranchDisplayStyle(rapidjson::Document& doc);

    EDITOR_API EditorGeometry::Point2D        GetGpsLineLocation(const std::string& wkt);

    EDITOR_API std::string                    GeoJson2Wkt(const std::string& geoJson,int sourceDataType);
	
	EDITOR_API std::string                    Points2Wkt(std::vector<EditorGeometry::Point2D>& points, int geoType);
    
    EDITOR_API std::string                    coordinates2Wkt(rapidjson::Value& coordinatesValue, int geoType);

    EDITOR_API double                         CalculateSphereDistance(double x1, double y1, double x2, double y2);
	
	EDITOR_API geos::geom::Geometry*          Wkb2Geo(EditorGeometry::WkbGeometry* wkb);

	EDITOR_API geos::geom::Geometry*          Wkt2Geo(std::string wkt);

	EDITOR_API EditorGeometry::WkbGeometry*   GetDisplayPointFromDeep(std::string& deep);

    EDITOR_API std::string                    Wkb2GeoJson(EditorGeometry::WkbGeometry* wkb);

    EDITOR_API std::string                    GeosPoint2GeoJson(geos::geom::Geometry* geometry);

    EDITOR_API std::string                    GeosLineString2GeoJson(geos::geom::Geometry* geometry);
    
    EDITOR_API std::string                    GeosPolygon2GeoJson(geos::geom::Geometry* geometry);
    
    EDITOR_API std::string                    GeosMultiLineString2GeoJson(geos::geom::Geometry* geometry);
    
    EDITOR_API std::string                    GetRdLink(const std::string& tipsJson);

    EDITOR_API std::string                    GetRoadDirRelateRdLink(rapidjson::Value& deepValue);
    EDITOR_API std::string                    GetBridgeGeo(const std::string& tipsJson);
    
    EDITOR_API std::string                    GetSETipsGeo(const std::string& tipsJson);

    EDITOR_API bool                           CheckBridgeDeep(rapidjson::Value& fArray, std::string& id, int& type, std::string& flag);

    EDITOR_API geos::geom::LineString*        GetBridgeRelatedLineGeo(rapidjson::Value& fArray,geos::geom::LineString** startLink, geos::geom::LineString** endLink);
    
    EDITOR_API std::vector<geos::geom::Geometry*>        GetSETipsRelatedLineGeo(rapidjson::Value& fArray);
    
    EDITOR_API std::string                    RapidJsonObj2String(rapidjson::Value& rValue);
    
    //经纬度转墨卡托
    EDITOR_API EditorGeometry::Point2D LonLat2Mercator(EditorGeometry::Point2D lonLat);
    //墨卡托转经纬度
    EDITOR_API EditorGeometry::Point2D Mercator2LonLat(EditorGeometry::Point2D mercator);
}
