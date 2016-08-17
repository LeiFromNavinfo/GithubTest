#include "Editor.h"
#include "DataTransfor.h"
#include <geos.h>
#include <geos/io/WKBReader.h>
#include <geos/operation/distance/DistanceOp.h>

namespace Editor
{
	DataSnapper::DataSnapper()
	{
		
	}

	DataSnapper::~DataSnapper()
	{
		
	}

	void DataSnapper::SetTargetLayer(DataLayer* layer)
	{
		this->m_pDataLayer = layer;
	}

	SnapResult* DataSnapper::Snapping(EditorGeometry::Box2D box, EditorGeometry::Point2D point)
	{
		if(m_pDataLayer == NULL)
		{
			return NULL;
		}

		std::string xmin = Tools::DoubleToString(box._minx);
		std::string ymin = Tools::DoubleToString(box._miny);
		std::string xmax = Tools::DoubleToString(box._maxx);
		std::string ymax = Tools::DoubleToString(box._maxy);

		std::string table_name_string = m_pDataLayer->GetTableName();

		std::string sql ="SELECT * FROM "+ table_name_string + " WHERE ROWID IN (SELECT ROWID FROM SpatialIndex WHERE f_table_name = '" + table_name_string +"' AND search_frame = BuildMbr("+xmin+","+ymin+","+xmax+","+ymax+"))";

		m_pDataLayer->ResetReadingBySQL(sql);

		double minDistance = DBL_MAX, minX = DBL_MAX, minY = DBL_MAX;

		Coordinate coord(point._x, point._y);
        
        Geometry* geoPoint = GeometryFactory::getDefaultInstance()->createPoint(coord);

		DataFeature* target_feature = NULL;

		std::string result;

        unsigned int geoIndex = 0;
        
        int type = m_pDataLayer->GetDataLayerType();

        if(type == DATALAYER_RDLINE)
        {
            geoIndex = 1;
        }
        else if(type == DATALAYER_RDNODE)
        {
            geoIndex = 1;
        }
        else if(type == DATALAYER_BKLINE)
        {
            geoIndex = 1;
        }
        else if(type == DATALAYER_TIPSLINE)
        {
            geoIndex = 4;
        }
        else if(type == DATALAYER_GPSLINE)
        {
            geoIndex = 4;
        }

		while(DataFeature* feature = m_pDataLayer->GetNextFeature())
		{
            if(type == DATALAYER_TIPSLINE)
            {
                FieldDefines* fd  = feature->GetFieldDefines();
                
                if(fd != NULL)
                {
                    int typeIndex = fd->GetColumnIndex("sourceType");

                    if(typeIndex != -1)
                    {
                        std::string sourceType = feature->GetAsString(typeIndex);

                        if(strcmp(sourceType.c_str(),Tools::NumberToString(Model::SurveyLine).c_str()) != 0)
                        {
                            DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);

                            continue;
                        }
                    }
                }
            }

			EditorGeometry::WkbGeometry* geoWkb=feature->GetAsWkb(geoIndex);

            if(NULL == geoWkb)
            {
                continue;
            }

			std::string strWkb((char*)geoWkb, geoWkb->buffer_size());

    		std::istringstream istream(strWkb);

    		WKBReader reader;

    		Geometry* geo = reader.read(istream);

			CoordinateSequence* coordSeq = geos::operation::distance::DistanceOp::closestPoints(geoPoint, geo);

    		if (coordSeq)
    		{
    			double nx = coordSeq->getX(1);

    			double ny = coordSeq->getY(1);

    			double px = nx - point._x;

    			double py = ny - point._y;

    			double distance = sqrt(px*px + py*py);

    			if (distance < minDistance)
    			{
					if(target_feature != NULL)
					{
						DataFeatureFactory::getDefaultInstance()->DestroyFeature(target_feature);
					}
    				minDistance = distance;

                    minX = nx;
                    minY = ny;

                    feature->SyncDataToMemory();

                    target_feature = feature;
    			}
				else
				{
					DataFeatureFactory::getDefaultInstance()->DestroyFeature(feature);
				}
                delete coordSeq;
    		}
    		delete geo;
		}
		delete geoPoint;

        if(target_feature != NULL)
        {
            SnapResult* snapResult = new SnapResult();
            snapResult->SetSnapFeature(target_feature);
            snapResult->SetCloestPoint(minX, minY);
            snapResult->SetDistance(minDistance);
            snapResult->SetDataLayerType(type);
            return snapResult;
        }
        return NULL;
	}

    SnapResult* DataSnapper::GivenPathSnapping(geos::geom::Geometry* path, geos::geom::Geometry* point, double disThreshold)
	{
	    if(NULL == path || NULL == point)
	    {
	        return NULL;
	    }

        if(path->getGeometryTypeId() != GEOS_LINESTRING || point->getGeometryTypeId() != GEOS_POINT)
        {
            return NULL;
        }

        SnapResult* snapResult = NULL;

        CoordinateSequence* coordSeq = geos::operation::distance::DistanceOp::closestPoints(point, path);

        if(coordSeq)
        {
            double x1 = coordSeq->getX(1);
            double y1 = coordSeq->getY(1);
            
            double x2 = point->getCoordinate()->x;
            double y2 = point->getCoordinate()->y;
            
            double distance = DataTransfor::CalculateSphereDistance(x1, y1, x2, y2);

            if(distance < disThreshold)
            {
                snapResult = new SnapResult();
                
                snapResult->SetCloestPoint(x1, y1);
                snapResult->SetDistance(distance);
            }

            delete coordSeq;
        }
        return snapResult;
	}

    void SnapResult::SetSnapFeature(DataFeature* feature)
	{
        if(this->m_pSnapFeature != NULL)
        {
            delete this->m_pSnapFeature;

            this->m_pSnapFeature = NULL;
        }
	    this->m_pSnapFeature = feature;
	}

    void SnapResult::SetCloestPoint(double x, double y)
	{
	    this->m_pCloestPoint._x = x;
        this->m_pCloestPoint._y = y;
	}

    void SnapResult::SetDistance(double distance)
	{
	    this->m_pDistance = distance;
	}

    void SnapResult::SetDataLayerType(int type)
    {
        this->m_pLayerType = type;
    }

    SnapResult::SnapResult()
	{
	    this->m_pSnapFeature = NULL;
	}

    SnapResult::~SnapResult()
	{
        delete m_pSnapFeature;

        m_pSnapFeature = NULL;

        this->m_pCloestPoint._x = 0.0;
        this->m_pCloestPoint._y = 0.0;

        m_pDistance = 0.0;
	}

    JSON SnapResult::GetJson()
	{
        JSON json;

        rapidjson::Document document;
        rapidjson::Document docTmp;
        document.SetObject();
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

        rapidjson::Value json_value(kStringType);

        if(m_pSnapFeature != NULL)
        {
            std::string featureJsonStr = m_pSnapFeature->GetTotalPart().GetJsonString();

            docTmp.Parse<0>(featureJsonStr.c_str());
            document.AddMember("feature", docTmp, allocator);
            document.AddMember("layerType", m_pLayerType, allocator);
        }
        else
        {
            std::string featureStr= "";
            json_value.SetString(featureStr.c_str(), featureStr.size(),allocator);
            document.AddMember("feature", json_value, allocator);
            document.AddMember("layerType", -1, allocator);
        }
        std::string pointXStr = Tools::NumberToString(m_pCloestPoint._x);
        std::string pointYStr = Tools::NumberToString(m_pCloestPoint._y);

        std::string pointStr= "POINT("+pointXStr+" "+pointYStr+")";

        json_value.SetString(pointStr.c_str(), pointStr.size(),allocator);
        document.AddMember("cloestPoint",json_value,allocator);

        document.AddMember("distance", m_pDistance, allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string result = buffer.GetString();

        json.SetJsonString(result);

        return json;
	}

    double SnapResult::GetDistance()
    {
        return this->m_pDistance;
    }

    int SnapResult::GetLayerType()
    {
        return this->m_pLayerType;
    }

    DataFeature* SnapResult::GetSnapFeature()
    {
        return this->m_pSnapFeature;
    }

}