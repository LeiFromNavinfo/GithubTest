#include "Editor.h"
#include <string>
#include "DataTransfor.h"
#include <geos.h>
#include <stringbuffer.h>
#include <writer.h>
#include <geos/operation/distance/DistanceOp.h>
#include "BussinessConstants.h"

namespace Editor
{	
	DataSelector::DataSelector()
	{
	}

	DataSelector::~DataSelector()
	{
	}

    void DataSelector::AddTargetLayer(DataLayer* layer)
	{
		this->m_pSelectedLayers.push_back(layer);
	}

    void DataSelector::ResetTargetLayers()
	{
        this->m_pSelectedLayers.clear();
	}

	std::vector<DataFeature*> DataSelector::PointSelect(EditorGeometry::Box2D box, EditorGeometry::Point2D point, DATALAYER_TYPE layer_type)
	{
        std::vector<DataFeature*> result;

		if(m_pSelectedLayers.size() == 0)
		{
			return result;
		}

        DataLayer* targetLayer = NULL;

        std::vector<DataLayer*>::iterator layerIter = m_pSelectedLayers.begin();

        for(;layerIter != m_pSelectedLayers.end(); ++layerIter)
        {
            if((*layerIter)->GetDataLayerType() == layer_type)
            {
                targetLayer = *layerIter;

                break;
            }
        }

        if(NULL == targetLayer)
        {
            return result;
        }

        FieldDefines* fd = targetLayer->GetFieldDefines();
        
        if(fd == NULL || fd->GetColumnCount() == 0)
        {
            return result;
        }
        
		std::string boxWkt = DataTransfor::Box2Wkt(box);

		std::string source_point_x_string = Tools::DoubleToString(point._x);
		std::string source_point_y_string = Tools::DoubleToString(point._y);

		std::string xmin=Tools::DoubleToString(box._minx);
		std::string ymin=Tools::DoubleToString(box._miny);
		std::string xmax=Tools::DoubleToString(box._maxx);
		std::string ymax=Tools::DoubleToString(box._maxy);

		std::string table_name_string = targetLayer->GetTableName();

        std::string sql ="SELECT * FROM "+ table_name_string + " WHERE ROWID IN (SELECT ROWID FROM SpatialIndex WHERE f_table_name = '" + table_name_string +"' AND search_frame = BuildMbr("+xmin+","+ymin+","+xmax+","+ymax+"))";

		targetLayer->ResetReadingBySQL(sql);

        double minDistance = DBL_MAX;

        unsigned int geoIndex = -1;
        
        if(layer_type == DATALAYER_POI)
        {
            geoIndex = fd->GetColumnIndex(Poi_geometry);
        }
        else if(layer_type == DATALAYER_TIPS)
        {
            geoIndex = fd->GetColumnIndex(GLocation_Str);
        }

        if(geoIndex == -1)
        {
            return result;
        }
        
		while (DataFeature* feature = targetLayer->GetNextFeature())
		{
            EditorGeometry::WkbGeometry* geoWkb=feature->GetAsWkb(geoIndex);

            geos::geom::Coordinate coord(point._x, point._y);
            
            Geometry* geoPoint = GeometryFactory::getDefaultInstance()->createPoint(coord);
            
            geos::geom::Geometry* geo = DataTransfor::Wkb2Geo(geoWkb);
            
            CoordinateSequence* coordSeq = geos::operation::distance::DistanceOp::closestPoints(geoPoint, geo);
            
			if(coordSeq)
            {
                double nx = coordSeq->getX(1);
				
    		    double ny = coordSeq->getY(1);
            
	            double px = nx - point._x;
            
	            double py = ny - point._y;
            
	            double distance = sqrt(px*px + py*py);
            
	            if (distance < minDistance)
	            {
	                if(result.size() != 0)
	                {
	                    std::vector<DataFeature*>::iterator iterator = result.begin();

	                    for(;iterator != result.end(); ++iterator)
	                    {
	                        DataFeatureFactory::getDefaultInstance()->DestroyFeature(*iterator);
	                    }
	                    result.clear();
	                }
	                minDistance = distance;
                
	                feature->SyncDataToMemory();

	                result.push_back(feature);
	            }
	            else if(distance - minDistance < DBL_MIN)
	            {
	                feature->SyncDataToMemory();

	                result.push_back(feature);
	            }
	            else
	            {
	                DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);
	            }
	        }
            if(geoPoint != NULL)
            {
                delete geoPoint;
            }
			
            if(geo != NULL)
            {
                delete geo;
            }

			if(coordSeq != NULL)
			{
			    delete coordSeq;
			}
        }
        return result;
    }

    std::vector<DataFeature*> DataSelector::PointSelect(EditorGeometry::Box2D box, EditorGeometry::Point2D point)
    {
        std::vector<DataFeature*> result;
        
        if(box.is_empty())
        {
            Logger::LogD("PointSelect：Input box is empty.");
            Logger::LogO("PointSelect：Input box is empty.");
            return result;
        }
        
        if(m_pSelectedLayers.size() == 0)
        {
            return result;
        }
        
        std::string source_point_x_string = Tools::DoubleToString(point._x);
        std::string source_point_y_string = Tools::DoubleToString(point._y);
        
        std::string xmin=Tools::DoubleToString(box._minx);
        std::string ymin=Tools::DoubleToString(box._miny);
        std::string xmax=Tools::DoubleToString(box._maxx);
        std::string ymax=Tools::DoubleToString(box._maxy);
        
        std::vector<std::string> gpsLineUuids;
        
        double minDistance = DBL_MAX;
        
        geos::geom::Coordinate coord(point._x, point._y);
        
        Geometry* geoPoint = GeometryFactory::getDefaultInstance()->createPoint(coord);
        
        std::vector<DataLayer*>::iterator layerIter = m_pSelectedLayers.begin();
        
        for(;layerIter != m_pSelectedLayers.end(); ++layerIter)
        {
            Editor::DataLayer* targetLayer = (*layerIter);
            
            std::string table_name_string = targetLayer->GetTableName();
            
            std::string sql = "SELECT * FROM "+ table_name_string + " WHERE ROWID IN (SELECT ROWID FROM SpatialIndex WHERE f_table_name = '" + table_name_string +"' AND search_frame = BuildMbr("+xmin+","+ymin+","+xmax+","+ymax+"))";
            
            targetLayer->ResetReadingBySQL(sql);
            
            Editor::FieldDefines* fieldDefines = targetLayer->GetFieldDefines();
            
            if(fieldDefines == NULL || fieldDefines->GetColumnCount() == 0)
            {
                continue;
            }
            
            unsigned int geoIndex = -1;
            
            if(targetLayer->GetDataLayerType() == DATALAYER_TIPS)
            {
                geoIndex = fieldDefines->GetColumnIndex(GLocation_Str);
            }
            else
            {
                geoIndex = fieldDefines->GetColumnIndex(Geometry_Str);
            }
            
            if(geoIndex == -1)
            {
                continue;
            }
            
            while (DataFeature* feature = targetLayer->GetNextFeature())
            {
                int index = -1;
                
                //tipsline层过滤掉非测线
                if((*layerIter)->GetDataLayerType() == DATALAYER_TIPSLINE)
                {
                    index =  fieldDefines->GetColumnIndex("sourceType");
                    
                    if(index != -1)
                    {
                        std::string type = feature->GetAsString(index);
                        
                        if(strcmp(type.c_str(),Tools::NumberToString(Model::SurveyLine).c_str()) !=0)
                        {
                            DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);
                            
                            continue;
                        }
                    }
                }
                
                bool isTargetFeature = false;
                
                EditorGeometry::WkbGeometry* geoWkb=feature->GetAsWkb(geoIndex);
                
                geos::geom::Geometry* featureGeo = DataTransfor::Wkb2Geo(geoWkb);
                
                CoordinateSequence* coordSeq = geos::operation::distance::DistanceOp::closestPoints(geoPoint, featureGeo);
                
                if(coordSeq)
                {
                    double nx = coordSeq->getX(1);
                    
                    double ny = coordSeq->getY(1);
                    
                    double px = nx - point._x;
                    
                    double py = ny - point._y;
                    
                    double distance = sqrt(px*px + py*py);
                    
                    if (distance < minDistance)
                    {
                        if(result.size() != 0)
                        {
                            std::vector<DataFeature*>::iterator iterator = result.begin();
                            
                            for(;iterator != result.end(); ++iterator)
                            {
                                DataFeatureFactory::getDefaultInstance()->DestroyFeature(*iterator);
                            }
                            result.clear();
                        }
                        
                        if(gpsLineUuids.size() != 0)
                        {
                            gpsLineUuids.clear();
                        }
                        
                        minDistance = distance;
                        
                        isTargetFeature = true;
                    }
                    else if(distance - minDistance < DBL_MIN)
                    {
                        isTargetFeature = true;
                    }
                    
                    if(isTargetFeature)
                    {
                        if(targetLayer->GetDataLayerType() == Editor::DATALAYER_TIPSLINE)
                        {
                            index = fieldDefines->GetColumnIndex("uuid");
                            
                            if(index != -1)
                            {
                                gpsLineUuids.push_back(feature->GetAsString(index));
                            }
                            
                            DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);
                        }
                        else
                        {
                            feature->SyncDataToMemory();
                            
                            result.push_back(feature);
                        }
                    }
                    else
                    {
                        DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);
                    }
                }
                
                if(featureGeo != NULL)
                {
                    delete featureGeo;
                }
                
                if(coordSeq != NULL)
                {
                    delete coordSeq;
                }
            }
        }
        
        Editor::DataManager* dataManager = Editor::DataManager::getInstance();
        
        Editor::DataFunctor* dataFunctor = dataManager->getDataFunctor();
        
        std::vector<Editor::DataFeature*> gpsLineFeatures = dataFunctor->GetGpsTipsByGeoUuids(gpsLineUuids);
        
        std::vector<Editor::DataFeature*>::iterator iter = gpsLineFeatures.begin();
        
        for(; iter != gpsLineFeatures.end(); ++iter)
        {
            result.push_back(*iter);
        }
        
        if(geoPoint != NULL)
        {
            delete geoPoint;
        }
        
        return result;
    }
    
    
    std::vector<DataFeature*> DataSelector::PolygonSelect(const std::string& region, bool isIntersectIncluded)
	{
	    std::vector<DataFeature*> result;

        EditorGeometry::Box2D box = DataTransfor::WktToBox2D(region);

        if(box.is_empty())
        {
            Logger::LogD("Input region is empty.");
    	    Logger::LogO("Input region is empty.");
    	    return result;
        }

		if(m_pSelectedLayers.size() == 0)
		{
			return result;
		}
        
        geos::geom::Geometry* regionGeo = DataTransfor::Wkt2Geo(region);

        if(regionGeo == NULL)
        {
            Logger::LogD("Input region is invalid : [%s].",region.c_str());
    	    Logger::LogO("Input region is invalid : [%s].",region.c_str());
            return result;
        }

        std::string xmin=Tools::DoubleToString(box._minx);
		std::string ymin=Tools::DoubleToString(box._miny);
		std::string xmax=Tools::DoubleToString(box._maxx);
		std::string ymax=Tools::DoubleToString(box._maxy);

        std::vector<DataLayer*>::iterator layerIter = m_pSelectedLayers.begin();

        std::vector<std::string> gpsLineUuids;

        for(;layerIter != m_pSelectedLayers.end(); ++layerIter)
        {
            Editor::DataLayer* targetLayer = (*layerIter);

            std::string table_name_string = targetLayer->GetTableName();
            
            std::string sql = "SELECT * FROM "+ table_name_string + " WHERE ROWID IN (SELECT ROWID FROM SpatialIndex WHERE f_table_name = '" + table_name_string +"' AND search_frame = BuildMbr("+xmin+","+ymin+","+xmax+","+ymax+"))";
            
            targetLayer->ResetReadingBySQL(sql);
            
            Editor::FieldDefines* fieldDefines = targetLayer->GetFieldDefines();

            while (DataFeature* feature = targetLayer->GetNextFeature())
            {
                int index = -1;

                //tipsline层过滤掉非测线
                if((*layerIter)->GetDataLayerType() == DATALAYER_TIPSLINE)
                {
                    index =  fieldDefines->GetColumnIndex("sourceType");

                    if(index != -1)
                    {
                        std::string type = feature->GetAsString(index);

                        if(strcmp(type.c_str(),Tools::NumberToString(Model::SurveyLine).c_str()) !=0)
                        {
                            DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);
                            
                            continue;
                        }
                    }
                }

                index = fieldDefines->GetColumnIndex(Geometry_Str);

                if(index == -1)
                {
                    DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);

                    continue;
                }
                
                EditorGeometry::WkbGeometry* wkbGeo = feature->GetAsWkb(index);

                geos::geom::Geometry* featureGeo = DataTransfor::Wkb2Geo(wkbGeo);

                bool isTarget = false;

                if(isIntersectIncluded)
                {
                    //判断是否相交
                    isTarget = regionGeo->intersects(featureGeo);
                }
                
                if(!isTarget)
                {
                    //判断是否包含
                    isTarget = regionGeo->contains(featureGeo);
                }

                if(!isTarget)
                {
                    DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);

                    delete featureGeo;

                    continue;
                }
                
                if(targetLayer->GetDataLayerType() == Editor::DATALAYER_TIPSLINE)
                {
                    index = fieldDefines->GetColumnIndex("uuid");

                    if(index != -1)
                    {
                         gpsLineUuids.push_back(feature->GetAsString(index));
                    }

                    DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);
                }
                else
                {
                    feature->SyncDataToMemory();

                    result.push_back(feature);
                }

                delete featureGeo;
            }
        }

        Editor::DataManager* dataManager = Editor::DataManager::getInstance();

        Editor::DataFunctor* dataFunctor = dataManager->getDataFunctor();

        std::vector<Editor::DataFeature*> gpsLineFeatures = dataFunctor->GetGpsTipsByGeoUuids(gpsLineUuids);

        std::vector<Editor::DataFeature*>::iterator iter = gpsLineFeatures.begin();

        for(; iter != gpsLineFeatures.end(); ++iter)
        {
            result.push_back(*iter);
        }

        if(regionGeo != NULL)
        {
            delete regionGeo;
        }
        
        return result;
	}
	
	std::vector<DataFeature*> DataSelector::SimlePolygonSelect(EditorGeometry::Box2D box, DATALAYER_TYPE layer_type)
    {
        std::vector<DataFeature*> result;

		if(m_pSelectedLayers.size() == 0)
		{
			return result;
		}

        DataLayer* targetLayer = NULL;

        std::vector<DataLayer*>::iterator layerIter = m_pSelectedLayers.begin();

        for(;layerIter != m_pSelectedLayers.end(); ++layerIter)
        {
            if((*layerIter)->GetDataLayerType() == layer_type)
            {
                targetLayer = *layerIter;

                break;
            }
        }

        if(NULL == targetLayer)
        {
            return result;
        }

		std::string xmin=Tools::DoubleToString(box._minx);
		std::string ymin=Tools::DoubleToString(box._miny);
		std::string xmax=Tools::DoubleToString(box._maxx);
		std::string ymax=Tools::DoubleToString(box._maxy);

		std::string table_name_string = targetLayer->GetTableName();

        std::string sql ="SELECT * FROM "+ table_name_string + " WHERE ROWID IN (SELECT ROWID FROM SpatialIndex WHERE f_table_name = '" + table_name_string +"' AND search_frame = BuildMbr("+xmin+","+ymin+","+xmax+","+ymax+"))";

		targetLayer->ResetReadingBySQL(sql);

        while (DataFeature* feature = targetLayer->GetNextFeature())
        {
            feature->SyncDataToMemory();

            result.push_back(feature);
        }

        return result;
    }
}