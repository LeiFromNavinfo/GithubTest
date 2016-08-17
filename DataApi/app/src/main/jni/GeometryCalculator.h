#ifndef FM_SDK_GeometryCalculator_h
#define FM_SDK_GeometryCalculator_h

#include "Editor.h"
#include <geos/geom/Point.h>

namespace Editor
{
    class EDITOR_API GeometryCalculator
    {
    private:
        GeometryCalculator();

        GeometryCalculator(const GeometryCalculator& other);

        static GeometryCalculator*                                 m_pInstance;

    public:
        static GeometryCalculator*                                 getInstance();             

        bool                                                       IsPointEquals(double x1, double y1, double x2, double y2);

        bool                                                       IsPointEquals(const geos::geom::Coordinate& point1, const geos::geom::Coordinate& point2);

        /** 
        * @brief 判断a,b,c三点是否共线
        * @param a,b,c 三点
        * @param max_tolerance 容差
        * @return true 共线 false 不共线
        */
        bool                                                       IsCollinear(const geos::geom::Coordinate& a, const geos::geom::Coordinate& b, const geos::geom::Coordinate& c,double max_tolerance = 0.01);

        /** 
        * @brief 判断目标点是否在线段之间
        * @param start 线段起点
        * @param end 线段终点
        * @param target 目标点
        * @param max_tolerance 容差
        * @return true 目标点在线段之间 false 目标点不在线段之间
        */
        bool                                                       IsPointAtLineInter(const geos::geom::Coordinate& start, const geos::geom::Coordinate& end, const geos::geom::Coordinate& target,double max_tolerance = 0.01);

        /** 
        * @brief 计算线上一点位于线的第几个形状点之后
        * @param target 目标点
        * @param line 线
        * @param index -1：目标点不在线上； 
        * 如果返回值为true表示目标点是线的第几个形状点
        * 如果返回值为false表示目标点在线的第几个形状点后
        * @return true 目标点与线的形状点重合 false 目标点不与线的形状点重合
        */
        bool                                                       LocatePointInPath(geos::geom::Point* target, geos::geom::LineString* line, int& index);
        double                         								CalcAngle(EditorGeometry::WkbGeometry* wkb, EditorGeometry::Point2D, int type);
		std::pair<double, double>      								GetVector(EditorGeometry::WkbGeometry* wkb, EditorGeometry::Point2D, int type);
		double                         								MeasurePointToPointDistance(double x1, double y1, double x2, double y2);
		std::pair<double, std::pair<double, double> >   			MeasurePointToSegLineDistanceByHeron(double px, double py, double ax, double ay, double bx, double by);

        /** 
        * @brief 获得两点打断后的线段
        * @param sPoint 起点
        * @param ePoint 终点
        * @param line 线
        * @return true 线段
        */
        geos::geom::LineString*                                    GetLineSegment(geos::geom::Point* sPoint, geos::geom::Point* ePoint, geos::geom::LineString* line);

        /** 
        * @brief 获得目标几何与源几何相交点的类型
        * @param source 源几何
        * @param ePoint 目标几何
        * @return 
        *      0： 源点串的起点与目标几何的起点挂接
        *      1： 源点串的起点与目标几何的终点挂接
        *      2： 源点串的终点与目标几何的起点挂接
        *      3： 源点串的终点与目标几何的起点挂接
        *     -1： 不挂接
        */
        int                                                        GetCrossPointType(geos::geom::Geometry* source, geos::geom::Geometry* target);
        
        /**
         * @brief 获得目标几何与源几何相交点的类型
         * @param source 源几何点list
         * @param ePoint 目标几何
         * @return  0：源点串的起点与目标几何的起点；
         *          1：源点串的终点与目标几何的起点；
         *          2：源点串的起点与目标几何的终点；
         *          3：源点串的终点与目标几何的终点；
         *         -1：非起终点。
         */
        int                                                        GetCrossPointTypeEx(std::list<geos::geom::Coordinate>& source, geos::geom::Geometry* target);
        
		std::pair<double, double>                                  GetVector(const std::string& point, EditorGeometry::WkbGeometry* line, int dir);
		std::pair<double, double>                                  GetPedalPoint(EditorGeometry::WkbGeometry* wkb, EditorGeometry::Point2D, double& distance);
	   double                                                      CaclDistFromPointToEndOfLIne(geos::geom::Geometry* geo, geos::geom::Point* point_1, int index_1, geos::geom::Point* point_2, int index_2);
        
        /**
         * @brief 获得起终点类tips截取后的起终线几何
         * @param lines 所有组成线：起始link在前，终点link在最后
         * @param point 起终点
         * @param type 0:起点  1:终点
         * @return 线几何
         */
        geos::geom::Geometry*                                      GetSETipsStartEndLineGeo(const std::vector<geos::geom::Geometry*>& lines, geos::geom::Geometry* point, int type);
    };
}

#endif
