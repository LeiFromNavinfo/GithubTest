//
//  Editor_Track.hpp
//  FMDataApi
//
//  Created by lidejun on 16/5/13.
//  Copyright © 2016年 Navinfo. All rights reserved.
//

#ifndef Editor_Track_h
#define Editor_Track_h

#include <stdio.h>
#include <string>
#include <vector>
#include <list>
#include <geos/geom/Point.h>
#include "Geometry.h"
#include <math.h>

#define MAX_LOD 20



namespace Editor
{
    class DataFeature;
    class DataLayer;
    
    
    struct tagTrackPoint
    {
        std::string id;
        double latitude;
        double longitude;
        double direction;
        double speed;
        std::string recordTime;
        int userId;
        std::string segmentId;
    };
    
    class TrackSimpleFilter
    {
    public:
        TrackSimpleFilter();
        ~TrackSimpleFilter();
        
        void iniFilterParam(double minDistanceFilter,
                            double maxDistanceFilter,
                            double minSpeedFilter,
                            double maxSpeedFilter);
        
        bool virtual judgeIsBadData(const tagTrackPoint& preTrackPoint, const tagTrackPoint& curTrackPoint);
        
    private:
        double m_MinDistanceFilter;
        double m_MaxDistanceFilter;
        double m_MinSpeedFilter;
        double m_MaxSpeedFilter;
    };
    
    class Editor_Track
    {
    public:
        
        enum SmoothMode
        {
            eSmoothMode_NO = -1,
            eSmoothMode_3PointLinearity=0,
            eSmoothMode_5PointSecondaryFiltering=1,
            eSmoothMode_3PointClockCoil=2,
            eSmoothMode_3PointHainingFiltering=3,
            eSmoothMode_5PointHainingFiltering=4,
        };
        
        Editor_Track();
        ~Editor_Track();
        
        /**
         * @brief 将轨迹点写入轨迹点的业务表
         *
         * @para id
         * @para latitude
         * @para longitude
         * @para direction
         * @para speed
         * @para recordTime
         * @para userId
         * @para segmentId
         *
         * @return 0,成功; -1,失败。
         */
        int writeTrackPoint(const std::string& id, double latitude,
                            double longitude, double direction, double speed,
                            const std::string& recordTime,
                            int userId, const std::string& segmentId);
        
        void setNeedRarefying(bool bNeedRarefying)
        {
            m_needRarefying = bNeedRarefying;
        }
        
        void setSmoothMode(int mode)
        {
            m_SmoothMode = mode;
        }
        
    protected:
        /**
         * @brief 将轨迹点featureTrackPoint写入轨迹线的显示表
         *
         * @para featureTrackPoint
         *
         * @return 0,成功; -1,失败。
         */
        int writePoint2TrackLine(DataFeature* featureTrackPoint);
        
    private:
        
        /**
         * @brief 获取轨迹的前两个轨迹点
         *
         * @para writingTrackPoint 要写入的轨迹点
         * @para trackPointLayer   轨迹图层
         * @para firstTrackPoint   第一个轨迹点
         * @para secondTrackPoint  第二个轨迹点
         *
         * @para 获取轨迹表某个segmentId的前两个轨迹点：0，成功；-1，失败。
         */
        int getFirstTwoTrackPoints(DataFeature* writingTrackPoint, DataLayer* trackPointLayer, DataFeature** firstTrackPoint, DataFeature** secondTrackPoint);
        
        int getMaxLodTrackSegment(DataFeature* writingTrackPoint, DataLayer* trackSegLayer, DataFeature** pMaxLodDataFeature);
        
        int firstTimeWriteTarckSegment(DataFeature* writingTrackPoint, DataFeature* pFirstTrackPoint, DataLayer* trackSegLayer,const std::string& startTime, const std::string& endTime, int useid, const std::string& strSegmentId);
        
        int updateAllLodTrackSeg(DataFeature* writingTrackPoint, DataLayer* trackSegLayer,
                                 DataFeature* pMaxLodDataFeature, int geoIndex,int latIndex,int lonIndex,int endTimeIndex,const std::string& endTime);
        
        int updateSomeLodTrackSeg(DataFeature* writingTrackPoint, DataLayer* trackSegLayer,
                                  DataFeature* pSomeLodDataFeature, int geoIndex, int latIndex, int lonIndex, int endTimeIndex, int iLODIndex,const std::string& endTime,bool needRarefying);
        
        void smoothLine(std::vector<EditorGeometry::Point2D>& data, SmoothMode mode);
        
        bool isEqual(const EditorGeometry::Point2D& p1, const EditorGeometry::Point2D& p2);
        
        double perpendicularDistance(EditorGeometry::Point2D point1, EditorGeometry::Point2D point2, EditorGeometry::Point2D point);
        
        void douglasPeuckerReduction(std::vector<EditorGeometry::Point2D>& points, int firstPoint, int lastPoint, double tolerance, std::list<int> &pointIndexsToKeep);
        
        std::vector<EditorGeometry::Point2D> douglasPeucker(std::vector<EditorGeometry::Point2D> &points, double Tolerance);
        
        int getTrackPoint(DataLayer* trackPointLayer, const std::string& strid, tagTrackPoint& result);
        
        bool m_needRarefying;
        int m_SmoothMode;
        
    };
}

#endif /* Editor_Track_hpp */
