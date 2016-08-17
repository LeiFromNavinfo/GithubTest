#include "Editor.h"
#include "math.h"
#include "GeometryCalculator.h"
#include "Tools.h"

const double PI = 3.1415926;

const double MaxSpeed = 35;

namespace Editor
{
    // --------------LocationManager------------

    LocationManager* LocationManager::pInstance = NULL;

    LocationManager::LocationManager()
    {
        m_pFilter = new Filter();

        m_pMatcher = new Matcher();

        m_pResult = new Result();

        m_pStatus = new Status();
    }

    LocationManager::~LocationManager() 
    {
        if(m_pFilter != NULL)
        {
            delete m_pFilter;
        }

        if(m_pMatcher != NULL)
        {
            delete m_pMatcher;
        }

        if(m_pResult != NULL)
        {
            delete m_pResult;
        }

        if(m_pStatus != NULL)
        {
            delete m_pStatus;
        }
    }

    LocationManager* LocationManager::GetInstance()
    {
        if(pInstance == NULL)
        {
            pInstance = new LocationManager();
        }

        return pInstance;
    }

    int LocationManager::ParseInput(const std::string& sInput)
    {
        if(sInput.empty())
        {
            return 0;
        }

        rapidjson::Document doc;

        doc.Parse<0>(sInput.c_str());

        if(doc.HasParseError())
        {
            return -1;
        }

        if(doc.HasMember("id"))
        {
            m_nId = doc["id"].GetInt();
        }

        if(doc.HasMember("speed"))
        {
            m_nSpeed = doc["speed"].GetDouble();
        }

        if(doc.HasMember("dir"))
        {
            m_nDir = doc["dir"].GetDouble();
        }

        if(doc.HasMember("time"))
        {
            m_sTime = doc["time"].GetString();
        }

        if(doc.HasMember("latitude"))
        {
            m_nLatitude = doc["latitude"].GetDouble();
        }

        if(doc.HasMember("longitude"))
        {
            m_nLongitude = doc["longitude"].GetDouble();
        }

        if(doc.HasMember("altitude"))
        {
            m_nAltitude = doc["altitude"].GetDouble();
        }

        return 0;
    }

    int LocationManager::Filte() 
    {
        //m_pResult->Reset();
        m_pResult->m_nId = m_nId;

        if(m_pFilter->FilterSpeedBadPoint(m_nSpeed) != 0)
        {
            Logger::LogD("it is error point which speed is more than general speed!!!");
            Logger::LogO("it is error point which speed is more than general speed!!!");

            m_pResult->Reset();
            
            return -1;
        }

        ResetFeatures();

        if(m_pFilter->FilterDistBadPoint2(m_nLatitude, m_nLongitude, m_vReatures) != 0)
        {
            Logger::LogD("it is error point which dist is more than general dist!!!");
            Logger::LogO("it is error point which dist is more than general dist!!!");

            m_pResult->Reset();
            
            return -1;
        }

        return 0;
    }

    void LocationManager::ResetFeatures()
    {
        for(int i = 0; i < m_vReatures.size(); i++)
        {
            delete m_vReatures[i].second;
        }

        m_vReatures.clear();
    }

    int LocationManager::Match() 
    {
        EditorGeometry::Point2D point;

        point._x = m_nLongitude;

        point._y = m_nLatitude;
            
        if(m_pStatus->m_isNearEnd == false)
        {
            m_pMatcher->CaclReliabilityFirst(m_pStatus, m_pResult, m_vReatures, point);

            return 0;
        }

        m_pMatcher->CaclReliabilitySecond(m_pStatus, m_pResult, m_nDir, m_vReatures, point);

        return 0;
    }

    std::string LocationManager::ToJson()
    {
        return m_pResult->ToJson();
    }

    // ----------- Match------------------------
    
    Matcher::Matcher() 
    {        
    }

    Matcher::~Matcher() 
    {
    }

    int Matcher::CaclReliabilityFirst(Status* pStatus, Result* result, std::vector<std::pair<int, EditorGeometry::WkbGeometry*> > features, EditorGeometry::Point2D point)
    {
        double dist = -1;

        std::pair<double, double> pedalPoint = Editor::GeometryCalculator::getInstance()->GetPedalPoint(pStatus->GetMatchedLine(), point, dist);

        result->m_nMatchConfidence = 1.0;

        result->m_sMatchLocation = Tools::GenerateWktPoint(pedalPoint.first, pedalPoint.second);

        double distance = CaclNearDist(pedalPoint, pStatus->GetMatchedLine(), pStatus);

        if(distance >= 35)
        {
            pStatus->m_isNearEnd = false;
        }
        else
        {
            pStatus->m_isNearEnd = true;
        }

        pStatus->SetMatchedPoint(pedalPoint.first, pedalPoint.second);

        return 0;
    }

    int Matcher::CaclReliabilitySecond(Status* pStatus, Result* result, double dir, std::vector<std::pair<int, EditorGeometry::WkbGeometry*> > features, EditorGeometry::Point2D point)
    {
        double nReliability = -1;

        Reliability reli;

        int index = -1;

        double dist = -1;

        std::pair<double, double> pedalPoint;

        for(int i = 0; i < features.size(); i++)
        {
            EditorGeometry::WkbGeometry* wkb = features[i].second;
            
            std::pair<double, double> pPoint = Editor::GeometryCalculator::getInstance()->GetPedalPoint(wkb, point, dist);

            double distConfidence;
			
            if(dist == 0.0)
            {
                distConfidence = 1.0;
            }
            else
            {
                distConfidence = reli.CaclByDist(dist);
            }

            double dirConfidence = reli.CaclByDir(dir, wkb, point);

            double nConfidence = (distConfidence + dirConfidence) / 2;

            if(nConfidence > nReliability)
            {
                index = i;

                nReliability = nConfidence;

                pedalPoint = pPoint;
            }
        }
        
        UpdateStatus(pedalPoint, features[index], pStatus);

        result->m_nMatchConfidence = nReliability;

        result->m_sMatchLocation = Tools::GenerateWktPoint(pedalPoint.first, pedalPoint.second);

        result->m_nMatchObjectId = features[index].first;

        result->m_isMatchedLine = 1;

        result->m_nMatchType = 1;

        return 0;
    }

    int Matcher::UpdateStatus(std::pair<double, double> pedal, std::pair<int, EditorGeometry::WkbGeometry*> feature, Status* pStatus)
    {
        double dist = CaclNearDist(pedal, feature.second, pStatus);

        if(dist >= 35)
        {
            pStatus->m_isNearEnd = false;
        }
        else
        {
            pStatus->m_isNearEnd = true;
        }

        pStatus->SetMatchedPoint(pedal.first, pedal.second);

        pStatus->SetMatechedLine(feature.second);

        pStatus->m_nMatchedLineId = feature.first;

        return 0;
    }

    double Matcher::CaclNearDist(std::pair<double, double> pedalPoint, EditorGeometry::WkbGeometry* wkb, Status* pStatus)
    {
        double dist;

        try{
            geos::geom::Coordinate coord;

            coord.x = pedalPoint.first;

            coord.y = pedalPoint.second;

            geos::geom::Point* point_1 = geos::geom::GeometryFactory::getDefaultInstance()->createPoint(coord);

            geos::geom::LineString* line = dynamic_cast<geos::geom::LineString*>(DataTransfor::Wkb2Geo(wkb));

            int index_1 = -1;

            if(GeometryCalculator::getInstance()->LocatePointInPath(point_1, line, index_1) == false && index_1 == -1)
            {
                Logger::LogD("locate point in LocationManager is error!!! please check");
                Logger::LogO("locate point in LocationManager is error!!! please check");

                return -1;
            }

            std::pair<double, double> prePoint = pStatus->GetPreMatchedPoint();

            if(prePoint.first - 0 == 0 && prePoint.second - 0 == 0)
            {
                return line->getLength();
            }

            coord.x = prePoint.first;

            coord.y = prePoint.second;

            geos::geom::Point* point_2 = geos::geom::GeometryFactory::getDefaultInstance()->createPoint(coord);

            int index_2 = -1;

            if(GeometryCalculator::getInstance()->LocatePointInPath(point_2, line, index_2) == false && index_2 == -1)
            {
                return line->getLength();
            }

            dist = GeometryCalculator::getInstance()->CaclDistFromPointToEndOfLIne(line, point_1, index_1, point_2, index_2);

            delete line;

            delete point_1;

            delete point_2;
        }
        catch(...)
        {
            Logger::LogD("CaclNearDist is error!!!");
            Logger::LogO("CaclNearDist is error!!!");

            return -1;
        }

        return dist;
    }

    //------------ Filter-----------------------

    Filter::Filter() 
    {
        m_db = NULL;
    }

    Filter::~Filter() {}

    int Filter::FilterDistBadPoint(double lat, double lon, std::vector<std::pair<int, EditorGeometry::WkbGeometry*> >& vec) 
    {
        Editor::DataManager* dataManager = Editor::DataManager::getInstance();

		std::string dataPath= dataManager->getFileSystem()->GetEditorGDBDataFile();
        
        Editor::GdbDataSource* dataSource = dynamic_cast<Editor::GdbDataSource*>(dataManager->getDataSource(DATASOURCE_GDB));
        
        if(NULL==dataSource)
        {
            return -1;
        }


		dataSource->Open(dataPath);

		Editor::DataLayer* layer = dataSource->GetLayerByType(DATALAYER_RDLINE);

        EditorGeometry::Box2D box2D;

        double distance = 35/100000;

        box2D.make(lon - distance, lon + distance, lat - distance, lat + distance);

        EditorGeometry::Point2D sourcePoint;

        sourcePoint._x = lon;

        sourcePoint._y = lat;

        Editor::DataSelector *selector =new Editor::DataSelector();

	    selector->AddTargetLayer(layer);

        std::vector<DataFeature*> vecResult  = selector->SimlePolygonSelect(box2D,Editor::DATALAYER_RDLINE);

        FieldDefines* fDefines = layer->GetFieldDefines();

        for(int i = 0; i < vecResult.size(); i++)
        {
            //vec.push_back(std::make_pair(vecResult[i]->GetAsInteger(fDefines->GetColumnIndex("pid")),vecResult[i]->GetAsWkb(fDefines->GetColumnIndex("geometry"))));// 需要验证

            int pid = vecResult[i]->GetAsInteger(fDefines->GetColumnIndex("pid"));

            EditorGeometry::WkbGeometry* wkb = vecResult[i]->GetAsWkb(fDefines->GetColumnIndex("geometry"));

            EditorGeometry::WkbGeometry* tmpWkb = new EditorGeometry::WkbGeometry[wkb->buffer_size()];
            
            memcpy(tmpWkb, wkb, wkb->buffer_size());

            vec.push_back(std::make_pair(pid, tmpWkb));

            DataFeatureFactory::getDefaultInstance()->DestroyFeature(vecResult[i]);
        }
        
        dataSource->Close(dataPath);
        
        delete selector;

        if(vec.size() == 0)
        {
            return -1;
        }

        return 0;
    }

	int Filter::OpenSqlite()
    {
        Editor::DataManager* dataManager = Editor::DataManager::getInstance();

		std::string dataPath= dataManager->getFileSystem()->GetEditorGDBDataFile();

        int rc = sqlite3_open_v2(dataPath.c_str(), &m_db, SQLITE_OPEN_READWRITE, NULL);

        if (rc != SQLITE_OK)
        {
            Logger::LogD("DataSource::sqlite [%s] or [%s] open failed", dataPath.c_str(), sqlite3_errmsg(m_db));
            Logger::LogO("DataSource::sqlite [%s] or [%s] open failed", dataPath.c_str(), sqlite3_errmsg(m_db));

            sqlite3_close_v2(m_db);

            return -1;
        }

         m_cache = spatialite_alloc_connection();

        spatialite_init_ex (m_db, m_cache, 0);

        return 0;
    }

    int Filter::FilterDistBadPoint2(double lat, double lon, std::vector<std::pair<int, EditorGeometry::WkbGeometry*> >& vec)
    {
        if(m_db == NULL)
        {
            if(OpenSqlite() != 0)
            {
                return -1;
            }
        }

        EditorGeometry::Box2D box2D;

        double distance = 35/100000;

        box2D.make(lon - distance, lon + distance, lat - distance, lat + distance);

        std::string xmin=Tools::DoubleToString(box2D._minx);
		std::string ymin=Tools::DoubleToString(box2D._miny);
		std::string xmax=Tools::DoubleToString(box2D._maxx);
		std::string ymax=Tools::DoubleToString(box2D._maxy);

        std::string sql ="SELECT * FROM gdb_rdLine WHERE ROWID IN (SELECT ROWID FROM SpatialIndex WHERE f_table_name = 'gdb_rdLine' AND search_frame = BuildMbr("+xmin+","+ymin+","+xmax+","+ymax+"))";



        sqlite3_stmt* stmt = NULL;

        int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, NULL);

        if(rc != 0)
        {
            sqlite3_finalize(stmt);
        }

        while(sqlite3_step(stmt) == SQLITE_ROW)
        {
            int pid = sqlite3_column_int(stmt, 0);

            int geo_length = sqlite3_column_bytes(stmt, 1);

	        const void* geo_buff = sqlite3_column_blob(stmt, 1);

            EditorGeometry::WkbGeometry* wkb = (EditorGeometry::WkbGeometry*)EditorGeometry::SpatialiteGeometry::ToWKBGeometry((EditorGeometry::SpatialiteGeometry*)geo_buff);

            EditorGeometry::WkbGeometry* tmpWkb = new EditorGeometry::WkbGeometry[wkb->buffer_size()];

            memcpy(tmpWkb, wkb, wkb->buffer_size());

            vec.push_back(std::make_pair(pid, tmpWkb));

            free(wkb);
        }

        if(vec.size() == 0)
        {
            return -1;
        }

        sqlite3_finalize(stmt);

        return 0;
    }

    int Filter::FilterSpeedBadPoint(double speed) 
    {
        if(speed > MaxSpeed)
        {
            return -1;
        }

        return 0;
    }

    // ----------- Matcher ---------------------

    // ----------- Reliability------------------

    Reliability::Reliability() {}

    Reliability::~Reliability() {}

    double Reliability::CaclByDir(double dir, EditorGeometry::WkbGeometry* line, EditorGeometry::Point2D point) 
    {
        double angle = GeometryCalculator::getInstance()->CalcAngle(line, point, true);

        double angleShift = abs(dir - angle);

        if(angleShift - 90 < 0)
        {
            return 1 - sin(angleShift * PI / 180);
        }

        return 0.0;
    }

    double Reliability::CaclByDist(double dist) 
    {
        double nBroadReliability = 20 / dist;

        return 2*atan(nBroadReliability) / PI;
    }

    //----------- Result --------------

    Result::Result() 
    {
        m_isMatchedLine = 0;

        m_nId = -1;

        m_nMatchConfidence = 0.0;

        m_nMatchType = -1;

        m_nMatchObjectId = -1;

        m_sMatchLocation = "";
    }

    Result::~Result() 
    {       
    }

    std::string Result::ToJson() 
    {
        Document document;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();

        document.AddMember("id",m_nId,allocator);
        document.AddMember("isMatchedLine",m_isMatchedLine,allocator);
        document.AddMember("reliability",m_nMatchConfidence,allocator);
        document.AddMember("matchedType",m_nMatchType,allocator);
        document.AddMember("matchedObjectId",m_nMatchObjectId,allocator);
        Value each_json_value(kStringType);
        each_json_value.SetString(m_sMatchLocation.c_str(), m_sMatchLocation.size(), allocator);
        document.AddMember("location",each_json_value,allocator);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();

        return sResult;
    }

    void Result::Reset()
    {
        m_isMatchedLine = 0;

        m_nMatchConfidence = 0.0;

        m_nMatchType = -1;

        m_nMatchObjectId = -1;

        m_sMatchLocation = "";
    }

    //---------- Status ---------------

    Status::Status() 
    {
        nIterator = 0;

        m_isNearEnd = true;

        m_nMatchedLineId = -1;

        m_pMatchedPoint = new std::vector<std::pair<double, double> >(5);

        m_pGeo = NULL;
    }

    Status::~Status() 
    {
        if(m_pMatchedPoint != NULL)
        {
            delete m_pMatchedPoint;

            m_pMatchedPoint = NULL;
        }

        if(m_pGeo != NULL)
        {
            delete m_pGeo;

            m_pGeo = NULL;
        }
    }

    int Status::CaclDistToEnd()
    {
        std::pair<double, double> curPoint = GetCurMatchedPoint();

        geos::geom::Coordinate coord;

        coord.x = curPoint.first;

        coord.x = curPoint.second;

        geos::geom::Point* point = geos::geom::GeometryFactory::getDefaultInstance()->createPoint(coord);
        
        geos::geom::LineString* line = dynamic_cast<geos::geom::LineString*>(DataTransfor::Wkb2Geo(m_pGeo));

        int index = -1;

        bool flag = Editor::GeometryCalculator::getInstance()->LocatePointInPath(point, line, index);

        if(flag == -1)
        {
            Logger::LogD("location point in path unsuccessfully!!!");
            Logger::LogO("location point in path unsuccessfully!!!");

            return -1;
        }



        return 0;
    }
    
    void Status::SetMatchedPoint(double lon, double lat)
    {
        (*m_pMatchedPoint)[nIterator] = std::make_pair<double, double>(lon, lat);

        if(nIterator + 1 == 5)
        {
            nIterator = 0;
        }
        else
        {
            nIterator++;
        }
    }

    std::pair<double, double> Status::GetCurMatchedPoint()
    {
        return m_pMatchedPoint->at(nIterator);
    }

    std::pair<double, double> Status::GetPreMatchedPoint()
    {
        int nIndex = -1;

        if(nIterator - 1 == -1)
        {
            nIndex = 4;
        }
        else
        {
            nIndex = nIterator - 1;
        }

        return m_pMatchedPoint->at(nIndex);
    }

    void Status::SetMatechedLine(EditorGeometry::WkbGeometry* geo)
    {
        if(m_pGeo != NULL)
        {
            delete m_pGeo;
        }

        m_pGeo = new EditorGeometry::WkbGeometry[geo->buffer_size()];

        memcpy(m_pGeo, geo, geo->buffer_size());
    }

    EditorGeometry::WkbGeometry* Status::GetMatchedLine()
    {
        return m_pGeo;
    }

} // namespace Editor