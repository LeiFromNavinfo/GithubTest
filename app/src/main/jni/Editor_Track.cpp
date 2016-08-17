//
//  Editor_Track.cpp
//  FMDataApi
//
//  Created by lidejun on 16/5/13.
//  Copyright © 2016年 Navinfo. All rights reserved.
//

#include "Editor_Track.h"
#include "Editor.h"
#include "BussinessConstants.h"
#include <math.h>
#include "Tools.h"
#include <fstream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include<exception>

#define MIN_Distance_filter 1.0   //米
#define MAX_Distance_filter 340.0  //米

#define MIN_Speed_Filter 1.5    //米/秒
#define MAX_Speed_Filter 340.0  //米/秒

//const double g_MaxTolerance = 20.0/ (2 * M_PI * 6378137.0) * 360;

const double g_MaxTolerance = 20.0;    //20米

const int RarefyingInterval = 500;

double g_tolerance[20]=
{
    g_MaxTolerance*pow(2, 19),
    g_MaxTolerance*pow(2, 18),
    g_MaxTolerance*pow(2, 17),
    g_MaxTolerance*pow(2, 16),
    g_MaxTolerance*pow(2, 15),
    g_MaxTolerance*pow(2, 14),
    g_MaxTolerance*pow(2, 13),
    g_MaxTolerance*pow(2, 12),
    g_MaxTolerance*pow(2, 11),
    g_MaxTolerance*pow(2, 10),
    g_MaxTolerance*pow(2, 9),
    g_MaxTolerance*pow(2, 8),
    g_MaxTolerance*pow(2, 7),
    g_MaxTolerance*pow(2, 6),
    g_MaxTolerance*pow(2, 5),
    g_MaxTolerance*pow(2, 4),
    g_MaxTolerance*pow(2, 3),
    g_MaxTolerance*pow(2, 2),
    g_MaxTolerance*pow(2, 1),
    g_MaxTolerance
    
};

namespace Editor
{
    Editor_Track::Editor_Track()
    {
        m_needRarefying = false;
        m_SmoothMode = eSmoothMode_5PointHainingFiltering;
    }
    
    Editor_Track::~Editor_Track()
    {
        
    }
    
    int Editor_Track::writeTrackPoint(const std::string& id, double latitude,
                                      double longitude, double direction, double speed,
                                      const std::string& recordTime,
                                      int userId, const std::string& segmentId)
    {
        Editor::CoreMapDataSource* pCoreDataSource = dynamic_cast<Editor::CoreMapDataSource*>(DataManager::getInstance()->getDataSource(DATASOURCE_COREMAP));
        
        std::string dataPath= pCoreDataSource->GetDataSourcePath();
        
        if(strcmp(dataPath.c_str(),"") == 0)
        {
            Logger::LogD("Editor_Track::writeTrackPoint failed, Data Path Empty");
            Logger::LogO("Editor_Track::writeTrackPoint failed, Data Path Empty");
            return -1;
        }
        
        pCoreDataSource->Open(dataPath);
        
        try{
            DataLayer* layer = pCoreDataSource->GetLayerByType(DATALAYER_TRACKPOINT);
            
            const DataFeatureFactory* factory = DataFeatureFactory::getDefaultInstance();
            
            DataFeature* featureTrackPoint = factory->CreateFeature(layer);
            std::string strSegmentId = segmentId;
            //recordTime:YYYYMMDDHHMMSS
            //按日期生成segmentid
            if(strSegmentId=="" && recordTime.size()>=8)
            {
                std::string currentDate = recordTime.substr(0,8);
                strSegmentId = Tools::NumberToString(userId) + currentDate;
            }
            
            //按小时生成segmentid
//            if(recordTime.size()>=10)
//            {
//                std::string currentDate = recordTime.substr(0,10);
//                strSegmentId = Tools::NumberToString(userId) + currentDate;
//            }
            
            FieldDefines* pFieldDefines = layer->GetFieldDefines();
            if(NULL==pFieldDefines)
            {
                DataFeatureFactory::getDefaultInstance()->DestroyFeature(featureTrackPoint);
                
                pCoreDataSource->Close(dataPath);
                
                Logger::LogD("Editor_Track::writeTrackPoint failed In APIWrapper, trackPoint pFieldDefines is NULL!\n");
                Logger::LogO("Editor_Track::writeTrackPoint failed In APIWrapper, trackPoint pFieldDefines is NULL!\n");
                
                return -1;
            }
            
            int idIndex = pFieldDefines->GetColumnIndex(track_collection_IdStr);
            int intId = layer->GetMaxIntPrimaryKey(track_collection_IdStr)+1;
            std::string idStr = Tools::NumberToString(intId);
            std::string preIdStr = Tools::NumberToString(intId-1);
            
            //判断当前点是否需要丢弃
            tagTrackPoint preTrackPoint;
            tagTrackPoint curTrackPoint;
            curTrackPoint.id=id;
            curTrackPoint.latitude=latitude;
            curTrackPoint.longitude=longitude;
            curTrackPoint.direction=direction;
            curTrackPoint.speed=speed;
            curTrackPoint.recordTime=recordTime;
            curTrackPoint.userId=userId;
            curTrackPoint.segmentId=segmentId;
            bool isFiristPoint = false;
            if(0==getTrackPoint(layer,preIdStr,preTrackPoint))
            {
                TrackSimpleFilter filter;
                filter.iniFilterParam(MIN_Distance_filter, MAX_Distance_filter, MIN_Speed_Filter, MAX_Speed_Filter);
                if (filter.judgeIsBadData(preTrackPoint, curTrackPoint))
                {
                    DataFeatureFactory::getDefaultInstance()->DestroyFeature(featureTrackPoint);
                    pCoreDataSource->Close(dataPath);
                    
                    Logger::LogD("Editor_Track::writeTrackPoint failed In APIWrapper, is bad point!");
                    Logger::LogO("Editor_Track::writeTrackPoint failed In APIWrapper, is bad point!");
                    return -1;
                }
            }
            else
            {
               isFiristPoint = true;
            }
            
            featureTrackPoint->SetAsString(idIndex, idStr);
            featureTrackPoint->SetAsDouble(pFieldDefines->GetColumnIndex(track_collection_LatitudeStr), latitude);
            featureTrackPoint->SetAsDouble(pFieldDefines->GetColumnIndex(track_collection_LongitudeStr), longitude);
            featureTrackPoint->SetAsDouble(pFieldDefines->GetColumnIndex(track_collection_DirectionStr), direction);
            featureTrackPoint->SetAsDouble(pFieldDefines->GetColumnIndex(track_collection_SpeedStr), speed);
            featureTrackPoint->SetAsString(pFieldDefines->GetColumnIndex(track_collection_RecordTimeStr), recordTime);
            featureTrackPoint->SetAsInteger(pFieldDefines->GetColumnIndex(track_collection_UserIdStr), userId);
            featureTrackPoint->SetAsString(pFieldDefines->GetColumnIndex(track_collection_SegmentIdStr), strSegmentId);
            int result = layer->InsertFeature(featureTrackPoint);
            if(result == -1)
            {
                DataFeatureFactory::getDefaultInstance()->DestroyFeature(featureTrackPoint);
                pCoreDataSource->Close(dataPath);
                
                Logger::LogD("Editor_Track::writeTrackPoint: InsertFeature(featureTrackPoint) is failed!\n");
                Logger::LogO("Editor_Track::writeTrackPoint: InsertFeature(featureTrackPoint) is failed!\n");
                return -1;
            }
            pCoreDataSource->Close(dataPath);
            //将轨迹点信息加入轨迹线
            if (!isFiristPoint) writePoint2TrackLine(featureTrackPoint);
            DataFeatureFactory::getDefaultInstance()->DestroyFeature(featureTrackPoint);
            return result;
        }
        catch(std::exception &e)
        {
            Logger::LogD("Editor_Track::writeTrackPoint failed In APIWrapper, Exception:[%s],type[%s]", e.what(),typeid(e).name());
            Logger::LogO("Editor_Track::writeTrackPoint failed In APIWrapper, Exception:[%s],type[%s]", e.what(),typeid(e).name());
            return -1;
        }
        return 0;
    }
    
     int Editor_Track::getFirstTwoTrackPoints(DataFeature* writingTrackPoint, DataLayer* trackPointLayer, DataFeature** firstTrackPoint, DataFeature** secondTrackPoint)
    {
        FieldDefines* pTrackPointFieldDefines = trackPointLayer->GetFieldDefines();
        if(NULL==pTrackPointFieldDefines) return -1;
        int segmentIdIndex = pTrackPointFieldDefines->GetColumnIndex(track_collection_SegmentIdStr.c_str());
        std::string strSegmentId = writingTrackPoint->GetAsString(segmentIdIndex);
        char buf[256];
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "SELECT * FROM %s WHERE %s='%s' ORDER BY %s ASC;", trackPointLayer->GetTableName().c_str(),track_collection_SegmentIdStr.c_str(), strSegmentId.c_str(), track_collection_RecordTimeStr.c_str());
        trackPointLayer->ResetReadingBySQL(buf);
        std::string startTime = "";
        std::string endTime = startTime;
        *firstTrackPoint = trackPointLayer->GetNextFeature();
        *secondTrackPoint = trackPointLayer->GetNextFeature();
        return 0;
    }
    
    int Editor_Track::getMaxLodTrackSegment(DataFeature* writingTrackPoint, DataLayer* trackSegLayer, DataFeature** pMaxLodDataFeature)
    {
        FieldDefines* pTrackSegFieldDefines = trackSegLayer->GetFieldDefines();
        if (NULL==pTrackSegFieldDefines) return -1;
        FieldDefines* pTrackPointFieldDefines = writingTrackPoint->GetFieldDefines();
        if (NULL==pTrackPointFieldDefines) return -1;
        //5.如果lod最大的轨迹线没查到，表明是第一次插入轨迹线，直接写入轨迹线
        int segmentIdIndex = pTrackPointFieldDefines->GetColumnIndex(track_collection_SegmentIdStr.c_str());
        std::string strSegmentId = writingTrackPoint->GetAsString(segmentIdIndex);
        char buf[256];
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "SELECT * FROM %s WHERE %s='%s' ORDER BY %s DESC;", trackSegLayer->GetTableName().c_str(),track_segment_SegmentIdStr.c_str(), strSegmentId.c_str(), track_segment_LODStr.c_str());
        trackSegLayer->ResetReadingBySQL(buf);
        *pMaxLodDataFeature = trackSegLayer->GetNextFeature();
        return 0;
    }
    
    int Editor_Track::firstTimeWriteTarckSegment(DataFeature* writingTrackPoint, DataFeature* pFirstTrackPoint, DataLayer* trackSegLayer,const std::string& startTime, const std::string& endTime, int useid, const std::string& strSegmentId)
    {
        FieldDefines*pTrackPointFieldDefines = pFirstTrackPoint->GetFieldDefines();
        if (NULL==pTrackPointFieldDefines) return -1;
        int latIndex = pTrackPointFieldDefines->GetColumnIndex(track_collection_LatitudeStr);
        int lonIndex = pTrackPointFieldDefines->GetColumnIndex(track_collection_LongitudeStr);
        double lon = pFirstTrackPoint->GetAsDouble(lonIndex);
        double lat = pFirstTrackPoint->GetAsDouble(latIndex);
        geos::geom::CoordinateArraySequence arraySequence;
        arraySequence.add(geos::geom::Coordinate(lon,lat));
        lon = writingTrackPoint->GetAsDouble(lonIndex);
        lat = writingTrackPoint->GetAsDouble(latIndex);
        arraySequence.add(geos::geom::Coordinate(lon,lat));
        geos::geom::LineString* line = geos::geom::GeometryFactory::getDefaultInstance()->createLineString(arraySequence);
        if (line)
        {
            std::string wktstr = line->toText();
            EditorGeometry::WkbGeometry * wkb= DataTransfor::Wkt2Wkb(wktstr);
            DataFeature* pTrackSegFeature = DataFeatureFactory::getDefaultInstance()->CreateFeature(trackSegLayer);
            
            FieldDefines*  pTrackSegFieldDefines = pTrackSegFeature->GetFieldDefines();
            if (NULL==pTrackSegFieldDefines) return -1;
            int idIndex  = pTrackSegFieldDefines->GetColumnIndex(track_segment_IdStr);
            int geoIndex = pTrackSegFieldDefines->GetColumnIndex(track_segment_GeometryStr);
            int lodIndex = pTrackSegFieldDefines->GetColumnIndex(track_segment_LODStr);
            int startTimeIndex = pTrackSegFieldDefines->GetColumnIndex(track_segment_StartTimeStr);
            int endTimeIndex = pTrackSegFieldDefines->GetColumnIndex(track_segment_EndTimeStr);
            int segUseidIndex = pTrackSegFieldDefines->GetColumnIndex(track_segment_UserIdStr);
            int segidIndex = pTrackSegFieldDefines->GetColumnIndex(track_segment_SegmentIdStr);
            int maxPrimaryKey = trackSegLayer->GetMaxIntPrimaryKey(track_segment_IdStr);

            pTrackSegFeature->SetAsWkb(geoIndex, wkb);
            delete[] wkb;
            pTrackSegFeature->SetAsString(startTimeIndex, startTime);
            pTrackSegFeature->SetAsString(endTimeIndex, endTime);
            pTrackSegFeature->SetAsInteger(segUseidIndex, useid);
            pTrackSegFeature->SetAsString(segidIndex, strSegmentId);
            for (int i=1; i<=MAX_LOD; ++i)
            {
                pTrackSegFeature->SetAsString(idIndex, Tools::NumberToString(++maxPrimaryKey));
                pTrackSegFeature->SetAsInteger(lodIndex, i);
                trackSegLayer->InsertFeature(pTrackSegFeature);
            }
            
            DataFeatureFactory::getDefaultInstance()->DestroyFeature(pTrackSegFeature);
        }
        return 0;

    }
    
    int Editor_Track::updateAllLodTrackSeg(DataFeature* writingTrackPoint, DataLayer* trackSegLayer,
                                           DataFeature* pMaxLodDataFeature, int geoIndex,int latIndex,int lonIndex,int endTimeIndex,const std::string& endTime)
    {
        EditorGeometry::WkbGeometry* wkb = pMaxLodDataFeature->GetAsWkb(geoIndex);
        geos::geom::Geometry* pGeo = DataTransfor::Wkb2Geo(wkb);
        geos::geom::LineString* pLineString = dynamic_cast<geos::geom::LineString*>(pGeo);
        if (pLineString)
        {
            geos::geom::CoordinateSequence* pCoordinateSequence = pLineString->getCoordinates();
            if (pCoordinateSequence)
            {
                std::vector<EditorGeometry::Point2D> iniPoints;
                EditorGeometry::Point2D pos;
                for (int j =0; j<pCoordinateSequence->size(); ++j)
                {
                    geos::geom::Coordinate c = pCoordinateSequence->getAt(j);
                    pos._x = c.x;
                    pos._y = c.y;
                    iniPoints.push_back(pos);
                }
                double curlat = writingTrackPoint->GetAsDouble(latIndex);
                double curlon = writingTrackPoint->GetAsDouble(lonIndex);
                pos._x = curlon;
                pos._y = curlat;
                iniPoints.push_back(pos);
                
                //7.将6的结果作为所有lod的数据基础，进行抽稀和平滑；
                //8.将7.的结果写入轨迹线表；
                for (int e=MAX_LOD-1; e>0; --e)
                {
                    std::vector<EditorGeometry::Point2D> resultPoints = douglasPeucker(iniPoints,g_tolerance[e]);
                    smoothLine(resultPoints, eSmoothMode_3PointLinearity);
                    std::string geoWkt = DataTransfor::Points2Wkt(resultPoints,EditorGeometry::wkbLineString);
                    EditorGeometry::WkbGeometry * wkb = DataTransfor::Wkt2Wkb(geoWkt);
                    if(e!=(MAX_LOD-1))
                    {
                        pMaxLodDataFeature = trackSegLayer->GetNextFeature();
                    }
                    
                    if (pMaxLodDataFeature)
                    {
                        pMaxLodDataFeature->SetAsString(endTimeIndex, endTime);
                        pMaxLodDataFeature->SetAsWkb(geoIndex, wkb);
                        trackSegLayer->UpdateFeature(pMaxLodDataFeature);
                    }
                    delete[] wkb;
                }
                DataFeatureFactory::getDefaultInstance()->DestroyFeature(pMaxLodDataFeature);
            }
        }
        return 0;
    }
    
    int Editor_Track::updateSomeLodTrackSeg(DataFeature* writingTrackPoint, DataLayer* trackSegLayer,
                                            DataFeature* pSomeLodDataFeature, int geoIndex, int latIndex, int lonIndex, int endTimeIndex, int iLODIndex,const std::string& endTime,bool needRarefying)
    {
        EditorGeometry::WkbGeometry* wkb = pSomeLodDataFeature->GetAsWkb(geoIndex);
        geos::geom::Geometry* pGeo = DataTransfor::Wkb2Geo(wkb);
        geos::geom::LineString* pLineString = dynamic_cast<geos::geom::LineString*>(pGeo);
        int lod = pSomeLodDataFeature->GetAsInteger(iLODIndex);
        if (pLineString)
        {
            geos::geom::CoordinateSequence* pCoordinateSequence = pLineString->getCoordinates();
            if (pCoordinateSequence)
            {
                std::vector<EditorGeometry::Point2D> iniPoints;
                EditorGeometry::Point2D pos;
                for (int j =0; j<pCoordinateSequence->size(); ++j)
                {
                    geos::geom::Coordinate c = pCoordinateSequence->getAt(j);
                    pos._x = c.x;
                    pos._y = c.y;
                    iniPoints.push_back(pos);
                }
                double curlat = writingTrackPoint->GetAsDouble(latIndex);
                double curlon = writingTrackPoint->GetAsDouble(lonIndex);
                pos._x = curlon;
                pos._y = curlat;
                iniPoints.push_back(pos);
                
                //进行抽稀和平滑；
                std::vector<EditorGeometry::Point2D> resultPoints;
                if (needRarefying)
                {
                    if (iniPoints.size()>RarefyingInterval)
                    {
                        //分段抽稀，如临近的500点抽稀
                        std::vector<EditorGeometry::Point2D> ::iterator itor = iniPoints.end()-RarefyingInterval;
                        if(itor!=iniPoints.begin())
                        {
                            std::vector<EditorGeometry::Point2D> beforPart;
                            beforPart.assign(iniPoints.begin(), itor-1);
                            std::vector<EditorGeometry::Point2D> afterPart;
                            afterPart.assign(itor, iniPoints.end());
                            std::vector<EditorGeometry::Point2D> rarefiedPart = douglasPeucker(afterPart, g_tolerance[lod-1]);
                            beforPart.insert(beforPart.end(), rarefiedPart.begin(), rarefiedPart.end());
                            resultPoints = beforPart;
                        }
                    }
                    else
                    {
                      resultPoints = douglasPeucker(iniPoints,g_tolerance[lod-1]);
                    }
                   
                }
                else
                {
                    resultPoints = iniPoints;
                }
                smoothLine(resultPoints, (SmoothMode)m_SmoothMode);
                std::string geoWkt = DataTransfor::Points2Wkt(resultPoints,EditorGeometry::wkbLineString);
                EditorGeometry::WkbGeometry * wkb = DataTransfor::Wkt2Wkb(geoWkt);
                //写入轨迹线表；
                pSomeLodDataFeature->SetAsString(endTimeIndex, endTime);
                pSomeLodDataFeature->SetAsWkb(geoIndex, wkb);
                trackSegLayer->UpdateFeature(pSomeLodDataFeature);
                delete[] wkb;
            }
        }
        return 0;
    }
    
    int Editor_Track::writePoint2TrackLine(DataFeature* featureTrackPoint)
    {
        try
        {
            int ret = 0;
            //1.判断是不是第一个点；
            std::string dataPath= DataManager::getInstance()->getFileSystem()->GetCoreMapDataPath();
            if(strcmp(dataPath.c_str(),"") == 0)
            {
                Logger::LogD("Editor_Track::writeTrackPoint failed, Data Path Empty");
                Logger::LogO("Editor_Track::writeTrackPoint failed, Data Path Empty");
                return -1;
            }
            Editor::CoreMapDataSource* pCoreDataSource = dynamic_cast<Editor::CoreMapDataSource*>(DataManager::getInstance()->getDataSource(DATASOURCE_COREMAP));
            pCoreDataSource->Open(dataPath);
            DataLayer* trackPointLayer = pCoreDataSource->GetLayerByType(DATALAYER_TRACKPOINT);
            if (trackPointLayer)
            {
                FieldDefines* pTrackPointFieldDefines = trackPointLayer->GetFieldDefines();
                if(NULL==pTrackPointFieldDefines)
                {
                    pCoreDataSource->Close(dataPath);
                    return -1;
                }
                std::string startTime = "";
                std::string endTime = startTime;
                int recordTimeIndex = pTrackPointFieldDefines->GetColumnIndex(track_collection_RecordTimeStr);
                int latIndex = pTrackPointFieldDefines->GetColumnIndex(track_collection_LatitudeStr);
                int lonIndex = pTrackPointFieldDefines->GetColumnIndex(track_collection_LongitudeStr);
                
                DataFeature* pFirstTrackPoint = NULL;
                DataFeature* pSecondTrackPoint = NULL;
                ret = getFirstTwoTrackPoints(featureTrackPoint, trackPointLayer, &pFirstTrackPoint, &pSecondTrackPoint);
                if (ret==-1)
                {
                    pCoreDataSource->Close(dataPath);
                    return -1;
                }
                //2.如果不到两个轨迹点，就不插入轨迹线，即有两个点才插入轨迹线；
                if(pFirstTrackPoint==NULL || pSecondTrackPoint==NULL)
                {
                    if (pFirstTrackPoint)
                        DataFeatureFactory::getDefaultInstance()->DestroyFeature(pFirstTrackPoint);
                    if(pSecondTrackPoint)
                        DataFeatureFactory::getDefaultInstance()->DestroyFeature(pSecondTrackPoint);
                    pCoreDataSource->Close(dataPath);
                    return 0;
                }
                startTime = pFirstTrackPoint->GetAsString(recordTimeIndex);
                endTime = featureTrackPoint->GetAsString(recordTimeIndex);
                //3.如果至少有两个轨迹点，就开始插入轨迹线；
                //4.取出lod最大的轨迹线；
                DataLayer* trackSegLayer = pCoreDataSource->GetLayerByType(DATALAYER_TRACKSEGMENT);
                if (trackSegLayer)
                {
                    FieldDefines* pTrackSegFieldDefines = trackSegLayer->GetFieldDefines();
                    if (NULL==pTrackSegFieldDefines)
                    {
                        if (pFirstTrackPoint)
                            DataFeatureFactory::getDefaultInstance()->DestroyFeature(pFirstTrackPoint);
                        if (pSecondTrackPoint)
                            DataFeatureFactory::getDefaultInstance()->DestroyFeature(pSecondTrackPoint);
                        pCoreDataSource->Close(dataPath);
                        return 0;
                    }
                    int geoIndex = pTrackSegFieldDefines->GetColumnIndex(track_segment_GeometryStr);
                    int endTimeIndex = pTrackSegFieldDefines->GetColumnIndex(track_segment_EndTimeStr);
                    int iLODIndex = pTrackSegFieldDefines->GetColumnIndex(track_segment_LODStr);
                    //5.如果lod最大的轨迹线没查到，表明是第一次插入轨迹线，直接写入轨迹线
                    int index = pTrackPointFieldDefines->GetColumnIndex(track_collection_UserIdStr);
                    int useid = featureTrackPoint->GetAsInteger(index);
                    int segmentIdIndex = pTrackPointFieldDefines->GetColumnIndex(track_collection_SegmentIdStr.c_str());
                    std::string strSegmentId = featureTrackPoint->GetAsString(segmentIdIndex);
                    DataFeature* pMaxLodDataFeature = NULL;
                    ret = getMaxLodTrackSegment(featureTrackPoint, trackSegLayer, &pMaxLodDataFeature);
                    if(ret==-1)
                    {
                       pCoreDataSource->Close(dataPath);
                        return 0;
                    }
                    if (pMaxLodDataFeature==NULL)
                    {
                        ret = firstTimeWriteTarckSegment(featureTrackPoint, pFirstTrackPoint, trackSegLayer,startTime, endTime, useid,strSegmentId);
                        pCoreDataSource->Close(dataPath);
                        return ret;
                    }
                    //6.查出每个lod下的feature，并插入最新点；
                    //7.将6的结果进行抽稀和平滑；
                    //8.将7.的结果写入轨迹线表；
                    //9.ok!
                    DataFeature* pLodDataFeature = pMaxLodDataFeature;
                    int idIndex = pTrackPointFieldDefines->GetColumnIndex(track_collection_IdStr);
                    std::string idStr = featureTrackPoint->GetAsString(idIndex);
                    int idInt = Tools::StringToNum(idStr);
                    bool needRarefying = m_needRarefying|(idInt%RarefyingInterval==0);
                    while (pLodDataFeature)
                    {
                        updateSomeLodTrackSeg(featureTrackPoint, trackSegLayer,pLodDataFeature,geoIndex, latIndex, lonIndex, endTimeIndex, iLODIndex,endTime, needRarefying);
                        DataFeatureFactory::getDefaultInstance()->DestroyFeature(pLodDataFeature);
                        pLodDataFeature = trackSegLayer->GetNextFeature();
                    }
                    pCoreDataSource->Close(dataPath);
                    return ret;
                }
            }
        }
        catch (std::exception &e)
        {
            Logger::LogD("Editor_Track::writePoint2TrackLine failed In APIWrapper, Exception:[%s],type[%s]", e.what(),typeid(e).name());
            Logger::LogO("Editor_Track::writePoint2TrackLine failed In APIWrapper, Exception:[%s],type[%s]", e.what(),typeid(e).name());
            return -1;
        }
        
        return 0;
    }
    
   void Editor_Track::smoothLine(std::vector<EditorGeometry::Point2D>& data, SmoothMode mode)
    {
        int dataNum = data.size();
        switch(mode)
        {
            case eSmoothMode_3PointLinearity: //三点线性
            {
                for(int i=1;i<dataNum-1;i++)
                {
                    data[i]._x = (data[i-1]._x + data[i]._x + data[i+1]._x)/3.0;
                    data[i]._y = (data[i-1]._y + data[i]._y + data[i+1]._y)/3.0;
                }
            }
            break;
            case eSmoothMode_5PointSecondaryFiltering://五点二次滤波
            {
                for(int i=2;i<dataNum-2;i++)
                {
                    data[i]._x = (12.0*(data[i-1]._x+data[i+1]._x)
                                        -3.0*(data[i-2]._x+data[i+2]._x)
                                        +17.0*data[i]._x)/35.0;
                    data[i]._y = (12.0*(data[i-1]._y +data[i+1]._y )
                                  -3.0*(data[i-2]._y +data[i+2]._y )
                                  +17.0*data[i]._y )/35.0;
                }
            }
            break;
            case eSmoothMode_3PointClockCoil://三点钟形滤波
            {
                for(int i=1;i<dataNum-1;i++)
                {
                    data[i]._x = 0.212*data[i-1]._x + 0.576*data[i]._x
                                               + 0.212f*data[i+1]._x;
                    data[i]._y = 0.212*data[i-1]._y + 0.576*data[i]._y
                    + 0.212*data[i+1]._y;
                }
            }
            break;
            case eSmoothMode_3PointHainingFiltering://五点钟形滤波
            {
                for(int i=2;i<dataNum-2;i++)
                {
                    data[i]._x = (0.11*(data[i-2]._x + data[i+2]._x)
                                               + 0.24*(data[i-1]._x + data[i+1]._x)
                                               + 0.3*data[i]._x);
                    data[i]._y = (0.11*(data[i-2]._y + data[i+2]._y)
                                  + 0.24*(data[i-1]._y + data[i+1]._y)
                                  + 0.3*data[i]._y);
                }
            }
            break;
            case eSmoothMode_5PointHainingFiltering://三点汉明滤波
            {
                for(int i=1;i<dataNum-1;i++)
                {
                    data[i]._x = 0.07*data[i-1]._x + 0.86*data[i]._x
                                                + 0.07*data[i+1]._x;
                    
                    data[i]._y = 0.07*data[i-1]._y + 0.86*data[i]._y
                                   + 0.07*data[i+1]._y;
                }
            }
            break;
            default:
                break;
        }
    }
    
    bool Editor_Track::isEqual(const EditorGeometry::Point2D& p1, const EditorGeometry::Point2D& p2)
    {
        return fabs(p1._x-p2._x)<0.000001 && fabs(p1._y-p2._y)<0.000001;
    }
    
    double Editor_Track::perpendicularDistance(EditorGeometry::Point2D point1, EditorGeometry::Point2D point2, EditorGeometry::Point2D point)
    {
        //Area = |(1/2)(x1y2 + x2y3 + x3y1 - x2y1 - x3y2 - x1y3)|   *Area of triangle
        //Base = v((x1-x2)2+(x1-x2)2)                               *Base of Triangle*
        //Area = .5*Base*H                                          *Solve for height
        //Height = Area/.5/Base
        
        double area = abs(0.5 * (point1._x * point2._y + point2._x * point._y + point._x * point1._y - point2._x * point1._y - point._x * point2._y - point1._x * point._y));
        double bottom = sqrt(pow(point1._x - point2._x, 2) + pow(point1._y - point2._y, 2));
        double height = area / bottom * 2;
        
        return height;
        
    }
    
    
    void Editor_Track::douglasPeuckerReduction(std::vector<EditorGeometry::Point2D>& points, int firstPoint, int lastPoint, double tolerance, std::list<int> &pointIndexsToKeep)
    {
        double maxDistance = 0;
        int indexFarthest = 0;
        
        for (int index = firstPoint; index < lastPoint; index++)
        {
            EditorGeometry::Point2D firstPos = DataTransfor::LonLat2Mercator(points[firstPoint]);
            EditorGeometry::Point2D lastPos = DataTransfor::LonLat2Mercator(points[lastPoint]);
            EditorGeometry::Point2D curPos = DataTransfor::LonLat2Mercator(points[index]);
            double distance = perpendicularDistance(firstPos, lastPos, curPos);
            if (distance > maxDistance)
            {
                maxDistance = distance;
                indexFarthest = index;
            }
        }
        
        if (maxDistance > tolerance && indexFarthest != 0)
        {
            //Add the largest point that exceeds the tolerance
            pointIndexsToKeep.push_back(indexFarthest);
            
            douglasPeuckerReduction(points, firstPoint,
                                    indexFarthest, tolerance, pointIndexsToKeep);
            
            douglasPeuckerReduction(points, indexFarthest,
                                    lastPoint, tolerance, pointIndexsToKeep);
        }
    }
    
    
    std::vector<EditorGeometry::Point2D> Editor_Track::douglasPeucker(std::vector<EditorGeometry::Point2D> &points, double tolerance)
    {
        if (points.empty() || (points.size() < 3))
            return points;
        
        int firstPoint = 0;
        int lastPoint = points.size() - 1;
        std::list<int> pointIndexsToKeep ;
        
        //Add the first and last index to the keepers
        pointIndexsToKeep.push_back(firstPoint);
        pointIndexsToKeep.push_back(lastPoint);
        
        //The first and the last point cannot be the same
        while (isEqual(points[firstPoint],(points[lastPoint])))
        {
            lastPoint--;
        }
        
        douglasPeuckerReduction(points, firstPoint, lastPoint,
                                tolerance, pointIndexsToKeep);
        
        std::vector<EditorGeometry::Point2D> returnPoints ;
        pointIndexsToKeep.sort();
        std::list<int>::iterator theIterator;
        for( theIterator = pointIndexsToKeep.begin(); theIterator != pointIndexsToKeep.end(); theIterator++ )
        {
            returnPoints.push_back(points[*theIterator]);
        }
        
        return returnPoints;
    }
    
    int Editor_Track::getTrackPoint(DataLayer* trackPointLayer, const std::string& strid,tagTrackPoint& result)
    {
        char buf[256];
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "select * from %s where %s=%s;", trackPointLayer->GetTableName().c_str(), track_collection_IdStr.c_str(), strid.c_str());
        trackPointLayer->ResetReadingBySQL(buf);
        
        DataFeature* featureTrackPoint = trackPointLayer->GetNextFeature();
        
        if (NULL==featureTrackPoint) return -1;
        
        FieldDefines*pFieldDefines = trackPointLayer->GetFieldDefines();
        
        if (NULL==pFieldDefines) return -1;
        
        result.id = featureTrackPoint->GetAsString(pFieldDefines->GetColumnIndex(track_collection_IdStr));
        result.latitude = featureTrackPoint->GetAsDouble(pFieldDefines->GetColumnIndex(track_collection_LatitudeStr));
        result.longitude = featureTrackPoint->GetAsDouble(pFieldDefines->GetColumnIndex(track_collection_LongitudeStr));
        result.direction =featureTrackPoint->GetAsDouble(pFieldDefines->GetColumnIndex(track_collection_DirectionStr));
        result.speed=featureTrackPoint->GetAsDouble(pFieldDefines->GetColumnIndex(track_collection_SpeedStr));
        result.recordTime = featureTrackPoint->GetAsString(pFieldDefines->GetColumnIndex(track_collection_RecordTimeStr));
        result.userId=featureTrackPoint->GetAsInteger(pFieldDefines->GetColumnIndex(track_collection_UserIdStr));
        result.segmentId=featureTrackPoint->GetAsString(pFieldDefines->GetColumnIndex(track_collection_SegmentIdStr));
        DataFeatureFactory::getDefaultInstance()->DestroyFeature(featureTrackPoint);
        return 0;
    }
    
    
    //gps信号简单过滤器
    TrackSimpleFilter::TrackSimpleFilter()
    {
        
    }
    TrackSimpleFilter::~TrackSimpleFilter()
    {
        
    }
    
    void TrackSimpleFilter::iniFilterParam(double minDistanceFilter,
                                           double maxDistanceFilter,
                                           double minSpeedFilter,
                                           double maxSpeedFilter)
    {
        m_MinDistanceFilter = minDistanceFilter;
        m_MaxDistanceFilter = maxDistanceFilter;
        m_MinSpeedFilter = minSpeedFilter;
        m_MaxSpeedFilter = maxSpeedFilter;
    }
    
    bool TrackSimpleFilter::judgeIsBadData(const tagTrackPoint& preTrackPoint, const tagTrackPoint& curTrackPoint)
    {
        //计算出前一点和当前点距离，单位为米
        double distance = Tools::calculate_accurate_geodetic_distance(preTrackPoint.latitude, preTrackPoint.longitude, curTrackPoint.latitude, curTrackPoint.longitude);
        
        Logger::LogD("Editor_Track::writeTrackPoint distance[%f]!", distance);
        Logger::LogO("Editor_Track::writeTrackPoint distance[%f]!", distance);
        
        if(distance<m_MinDistanceFilter || distance>m_MaxDistanceFilter) return true;
        
        int timeInterval = 1;
        
        //因为时间是14位格式:YYYYMMDDHHMMSS
        if(curTrackPoint.recordTime.size()>=14 && preTrackPoint.recordTime.size()>=14)
        {
            timeInterval = abs(Tools::StringToNum(curTrackPoint.recordTime.substr(8,strlen(curTrackPoint.recordTime.c_str())-8)) - Tools::StringToNum(preTrackPoint.recordTime.substr(8,strlen(preTrackPoint.recordTime.c_str())-8)));
        }
        
//        if(timeInterval)
//        {
//            double speed = distance/timeInterval;
//            
//            Logger::LogD("Editor_Track::writeTrackPoint speed[%f]!", speed);
//            Logger::LogO("Editor_Track::writeTrackPoint speed[%f]!", speed);
//            
//            if (speed<m_MinSpeedFilter || speed>m_MaxSpeedFilter) return true;
//        }
        return false;
    }

    
}
