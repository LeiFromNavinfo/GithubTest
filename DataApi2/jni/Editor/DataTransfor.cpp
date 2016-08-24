#include "DataTransfor.h"
#include "Logger.h"
#include <geos/io/WKBReader.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKBWriter.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include "Tools.h"
#include <stringbuffer.h>
#include <writer.h>
#include <map>
#include "Model.h"
#include "BussinessConstants.h"
#include <math.h>
#include "GeometryCalculator.h"

namespace DataTransfor
{
	std::string Wkb2Wkt(EditorGeometry::WkbGeometry* wkb)
	{
		if (wkb == NULL)
		{
			Logger::LogD("Wkb2Wkt, wkb is NULL!");

			Logger::LogO("Wkb2Wkt, wkb is NULL!");

			return "";
		}

		std::string strWkb((char*)wkb, wkb->buffer_size());

		std::istringstream istream(strWkb);

		geos::io::WKBReader reader;

		geos::geom::Geometry* geo = NULL;

		try
		{
			geo = reader.read(istream);
		}
		catch (...)
		{
			Logger::LogD("Wkb2Wkt, wkb is not valid!");

			Logger::LogO("Wkb2Wkt, wkb is not valid!");
		}

		if (geo == NULL)
		{
			return "";
		}

		std::string ret = geo->toText();

		delete geo;

		return ret;
	}

	std::string Box2Wkt(EditorGeometry::Box2D box)
	{
		std::string wkt;

		std::string max_x_string = Tools::DoubleToString(box._maxx);
		std::string min_x_string = Tools::DoubleToString(box._minx);
		std::string max_y_string = Tools::DoubleToString(box._maxy);
		std::string min_y_string = Tools::DoubleToString(box._miny);

		if(box._maxx - box._minx <0.00001)
		{
			wkt ="POINT(" + min_x_string + " " + min_y_string + ")";
		}
		else
		{
			wkt ="POLYGON((" + min_x_string + " " + max_y_string + "," +
					min_x_string + " " + min_y_string + "," +
					max_x_string + " " + min_y_string + "," +
					max_x_string + " " + max_y_string + "," +
					min_x_string + " " + max_y_string + "))";
		}
		return wkt;
	}

    std::string JSONToString(rapidjson::Document& Json)
	{
	    rapidjson::StringBuffer buffer;
	    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);  
		Json.Accept(writer);  
		std::string result = buffer.GetString();
		return result;
	}

    EditorGeometry::Box2D    WktToBox2D(std::string wkt)
	{
        EditorGeometry::Box2D box;

        box.make_empty();

	    geos::io::WKTReader wktReader;

        geos::geom::Geometry* geoBox = wktReader.read(wkt);

        if(geoBox == NULL)
        {
            return box;
        }

        geoBox->getEnvelope();

        const geos::geom::Envelope* envelope = geoBox->getEnvelopeInternal();

        if (envelope == NULL)
        {
            return box;
        }

        box.make(envelope->getMinX(), envelope->getMaxX(), envelope->getMinY(), envelope->getMaxY());

        delete geoBox;

        return box;
	}

    EditorGeometry::WkbGeometry * Wkt2Wkb(const std::string& strWkt)
    {
        geos::geom::Geometry* geo = NULL;
        
        geos::io::WKTReader wktReader;

        geos::io::WKBWriter wkbWriter;

        try
		{
			geo = wktReader.read(strWkt);
		}
		catch (...)
		{
			Logger::LogD("Wkt2Wkb, wkt:%s is not valid!", strWkt.c_str());

			Logger::LogO("Wkt2Wkb, wkt:%s is not valid!", strWkt.c_str());
		}

		if (geo == NULL)
		{
			return NULL;
		}

		std::stringbuf buffer;

		std::ostream os(&buffer);

		wkbWriter.write(*geo, os);

		std::string wkb = buffer.str();

		int length = wkb.size();

		char* ret = new char[length];

		memcpy(ret, wkb.c_str(), length);

		delete geo;

		return (EditorGeometry::WkbGeometry*)ret;
    }


    int sqlite3_callback_fillbundle(void* para, int n_column, char** column_value, char** column_name)
    {
    	for (int i=0; i<n_column; i++)
    	{
    		Logger::LogD("column_name:%s,%s", column_name[i], column_value[i]);
    	}

    	return -1;	//non-zero
    }


    bool ExecuteNonQuery(const char* sql, sqlite3* db)
	{
        char* pszErrMsg;
        

        	int rc = sqlite3_exec(db, sql, sqlite3_callback_fillbundle, NULL, &pszErrMsg);

        Logger::LogD("executenoquery:%d", rc);
        if (rc != SQLITE_OK)
        {
            printf("ExecuteNoQuery failed:%s", pszErrMsg);
            
            return false;
        }
        return true;
    }

    std::string GetPoiGlobalId(rapidjson::Document& doc)
    {
        if(doc.HasParseError() || !doc.HasMember(Poi_type.c_str()) ||!doc.HasMember(Poi_project.c_str()) ||
           !doc.HasMember(Poi_fid.c_str()) || !doc.HasMember(Poi_pid.c_str()))
        {
            Logger::LogD ("GetPoiGlobalId::Parse poi failed");
            Logger::LogO ("GetPoiGlobalId::Parse poi failed");
            return "";
        }
        
        int type,pid;
        
        std::string fid,project;
        
        if(!doc[Poi_type.c_str()].IsNull())
        {
            type = doc[Poi_type.c_str()].GetInt();
        }
        
        if(!doc[Poi_pid.c_str()].IsNull())
        {
            pid = doc[Poi_pid.c_str()].GetInt();
        }
        
        if(!doc[Poi_fid.c_str()].IsNull())
        {
            fid = doc[Poi_fid.c_str()].GetString();
        }

        if(!doc[Poi_project.c_str()].IsNull())
        {
            project = doc[Poi_project.c_str()].GetString();
        }
        
        std::string globalId;
        if(type == 0)
        {
            globalId = fid+ project;
        }
        else if (type == 1)
        {
            globalId = Tools::NumberToString(pid)+"1";
        }
        else if (type == 2)
        {
            globalId = Tools::NumberToString(pid)+"2";
        }
        
        return globalId;
    }
    
    std::string GetPoiDisplayStyle(rapidjson::Document& doc)
    {
        if(doc.HasParseError() || !doc.HasMember(Poi_lifecycle.c_str()) ||!doc.HasMember(Poi_qtStatus.c_str()) ||
           !doc.HasMember(Poi_evaluatePlanning.c_str()) ||!doc.HasMember(Poi_name.c_str()) || !doc.HasMember(Poi_kindCode.c_str()))
        {
            Logger::LogD ("GetPoiDisplayStyle::Parse poi failed");
            Logger::LogO ("GetPoiDisplayStyle::Parse poi failed");
            return "";
        }
        
        int lifecycle,qtStatus,evaluatePlanning;
        
        std::string name,kindCode;
        
        if(!doc[Poi_lifecycle.c_str()].IsNull())
        {
            lifecycle = doc[Poi_lifecycle.c_str()].GetInt();
        }
        
        if(!doc[Poi_qtStatus.c_str()].IsNull())
        {
            qtStatus = doc[Poi_qtStatus.c_str()].GetInt();
        }
        
        if(!doc[Poi_evaluatePlanning.c_str()].IsNull())
        {
            evaluatePlanning = doc[Poi_evaluatePlanning.c_str()].GetInt();
        }
        
        if(!doc[Poi_name.c_str()].IsNull())
        {
            name = doc[Poi_name.c_str()].GetString();
        }
        
        if(!doc[Poi_kindCode.c_str()].IsNull())
        {
            kindCode = doc[Poi_kindCode.c_str()].GetString();
        }
        
        int nStatus=-1;
        
        if((lifecycle == 0 && qtStatus == 0) || qtStatus == 1) //待作业
        {
            nStatus = 1;
        }
        else if((lifecycle != 0 && qtStatus == 0) || qtStatus == 2) // 已作业
        {
            if(lifecycle == 1)
            {
                nStatus = 3;
            }
            else if(lifecycle == 2)
            {
                nStatus = 2;
            }
            else if(lifecycle == 3)
            {
                nStatus = 4;
            }
        }
        else
        {
            nStatus = 0;
        }
        
        if(nStatus == -1)
        {
            Logger::LogD("GetPoiDisplayStyle failed");
            Logger::LogO("GetPoiDisplayStyle failed");
            
            return "";
        }
        
        std::string displaystyle;
        
        if(nStatus == 0)
        {
            displaystyle = "0,0";
        }
        
        std::stringstream sstream;
        
        sstream<<nStatus;
        
        sstream<<",";
        
        int nImportance = evaluatePlanning == 1?1:0;
        
        sstream<<nImportance;
        
        if (1 == nStatus && 0 == nImportance)
        {
            int namelength = name.length();
            
            std::string endNameStr = "";
            
            if (namelength > 9)
            {
                endNameStr = name.substr(namelength-9, 9);
            }
            
            std::string pointSymbols[] = {"110101", "110102", "110103", "110301", "110303", "110304",
                "120101", "120201",
                "130106", "130501",
                "150101", "150101+ATM",
                "170101", "170102",
                "180104", "180105", "180201", "180209", "180210", "180308", "180309",
                "200103", "200104", "210204", "210215", "210302",
                "230103", "230111", "230125", "230126", "230127", "230208", "230210", "230213", "230214", "230215",
            };
            
            int pointSymbolsLength = sizeof(pointSymbols)/sizeof(std::string);
            
            if(kindCode == "150101")
            {
#ifdef WIN32
                if (endNameStr == "\uEFBCA1\uEFBCB4\uEFBCAD")
#else
                    if (endNameStr == "ＡＴＭ")
#endif
                    {
                        sstream << ",150101+ATM";
                    }
                    else
                    {
                        sstream << ",150101";
                    }
            }
            else
            {
                for (int k=0; k<pointSymbolsLength; k++)
                {
                    if (kindCode == pointSymbols[k])
                    {
                        sstream << "," << kindCode;
                        
                        break;
                    }
                }
            }
        }
        displaystyle = sstream.str();
        
        return displaystyle;
    }
    
    std::string GetTipsDisplayStyle(const std::string& tipsJson)
	{
	    rapidjson::Document tipsDoc;

        tipsDoc.Parse<0>(tipsJson.c_str());

        if(tipsDoc.HasParseError() || !tipsDoc.HasMember("s_sourceType") ||!tipsDoc.HasMember("angle") ||
           !tipsDoc.HasMember("g_guide") || !tipsDoc.HasMember("t_lifecycle"))
        {
            Logger::LogD ("Parse Tips failed: %s\n", tipsJson.c_str());
            Logger::LogO ("Parse Tips failed: %s\n", tipsJson.c_str());
            return "";
        }
        Document deepDoc;
        
        if(tipsDoc.HasMember("deep") && !tipsDoc["deep"].IsNull())
        {
            rapidjson::Value& deepValue = tipsDoc["deep"];
            
            deepDoc.CopyFrom(deepValue,deepDoc.GetAllocator());
        }

        std::string iconStyleString;

        std::string sourceType = tipsDoc.FindMember("s_sourceType")->value.GetString();

        int tipsType = atoi(sourceType.c_str());

	    switch (tipsType)
	    {
        case Model::OrdinaryTurnRestriction:
            iconStyleString = GetOrdinaryTurnRestrictionDisplayStyle(deepDoc);
            break;
        case Model::RoadKind:
            iconStyleString = GetRoadKindDisplayStyle(deepDoc);
            break;
        case Model::Hitching:
            iconStyleString = GetHitchingPointDisplayStyle(deepDoc);
            break;
        case Model::CrossName:
            iconStyleString = GetCrossNameDisplayStyle(deepDoc);
            break;
		case Model::SurveyLine:
            {
                std::string iconName = Tools::NumberToString(Model::SurveyLine) + ResourceSplitSymbol + "0" + ResourceSplitSymbol +"0";

                iconStyleString = GetConstantsDisplayStyle(iconName);

                break;
            }
        case Model::LaneConnexity:
            iconStyleString = GetLaneConnexityDisplayStyle(deepDoc);
            break;
        case Model::RoadDirect:
            iconStyleString = GetRoadDirectDisplayStyle(deepDoc);
            break;
        case Model::RoadName:
            {
                iconStyleString = GetRoadNameDisplayStyle(deepDoc);

                break;
            }
		case Model::Bridge:
			iconStyleString = GetBridgeDisplayStyle(deepDoc);
            break;
        case Model::PointSpeedLimit:
            iconStyleString = GetPointSpeedLimitDisplayStyle(deepDoc);
            break;
        case Model::HighwayBranch:
            iconStyleString = GetHighwayBranchDisplayStyle(deepDoc);
            break;
        case Model::RegionalRoad:
            {
                std::string iconName = Tools::NumberToString(Model::RegionalRoad) + ResourceSplitSymbol + "0" + ResourceSplitSymbol +"0";
                
                iconStyleString = GetConstantsDisplayStyle(iconName);
                
                break;
            }
        case Model::MultiDigitized:
            {
                std::string iconName = Tools::NumberToString(Model::MultiDigitized) + ResourceSplitSymbol + "0" + ResourceSplitSymbol +"0";
                
                iconStyleString = GetConstantsDisplayStyle(iconName);
                
                break;
            }
        default:
            {
                std::string iconName = Tools::NumberToString(tipsType) + ResourceSplitSymbol + "0" + ResourceSplitSymbol +"0";
                
                iconStyleString = GetConstantsDisplayStyle(iconName);
                
                break;
            }
            break;
	    }

        rapidjson::Document disDoc, tempDoc;
        disDoc.SetObject();

        tempDoc.Parse<0>(iconStyleString.c_str());

        rapidjson::Document::AllocatorType& allocator = disDoc.GetAllocator();

        rapidjson::Value stringValue(kStringType);

        rapidjson::Value& guideValue = tipsDoc.FindMember("g_guide")->value;

        std::string guide;

        if(guideValue.IsString())
        {
            guide = guideValue.GetString();
        }
        else if(!guideValue.IsNull())
        {
            rapidjson::StringBuffer buffer;

            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            
            guideValue.Accept(writer); 
            
            guide = GeoJson2Wkt(buffer.GetString(),Model::Tips::DataSourceFromServer);
        }

        stringValue.SetString(guide.c_str(),guide.length(),allocator);

        double angle = tipsDoc.FindMember("angle")->value.GetDouble();
        int lifeCycle = tipsDoc.FindMember("t_lifecycle")->value.GetInt();

        disDoc.AddMember("g_guide", stringValue, allocator);
        disDoc.AddMember("t_lifecycle", lifeCycle, allocator);
        disDoc.AddMember("angle", angle, allocator);

        disDoc.AddMember("icon", tempDoc, allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        disDoc.Accept(writer);

        std::string displayString = buffer.GetString();

        return displayString;
	}
	std::string GetTipsLineDisplayStyle(const std::string& type, const std::string& tipsDeep)
    {
        std::string display_string;
        int tipsType = Tools::StringToNum(type);
	    switch (tipsType)
	    {
        case Model::RoadName:
            display_string = RoadNameDisplay_Line;
            break;
        case Model::Bridge:
            display_string = BridgeDispaly_Line;
            break;
        case Model::Hitching:
            display_string = HitchingDisplay_Line;
            break;
        case Model::UnderConstruction:
            display_string = UnderConstructionDisplay_Line;
            break;
        case Model::SurveyLine:
            {
              display_string = GetSurveyLineDisplayStyle(tipsDeep);
            }
            break;
        default:
            break;
	    }
        return display_string;
    }
    
    std::string GetSurveyLineDisplayStyle(const std::string& tipsDeep)
    {
        std::string display_string = "";
        rapidjson::Document deepDoc;
        deepDoc.Parse<0>(tipsDeep.c_str());
        if (!deepDoc.HasParseError()
            && deepDoc.HasMember("src")
            && !deepDoc["src"].IsNull()
            && !deepDoc["src"].IsInt())
        {
            rapidjson::Value& srcValue= deepDoc["src"];
            int src = srcValue.GetInt();
            char buf[256];
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "%d,%d", src, SurveyLineDisplayLineCode);
            //display_string = display_string;
        }
        return display_string;
    }

    std::string GetOrdinaryTurnRestrictionDisplayStyle(rapidjson::Document& doc)
	{
	    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        rapidjson::Value arrayValue(rapidjson::kArrayType);
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        rapidjson::Document::MemberIterator member_iterator = doc.FindMember("o_array");

        if(member_iterator == doc.MemberEnd())
        {
            return "";
        }

        rapidjson::Value &valueOarray = (member_iterator->value);

        if(!valueOarray.IsArray())
        {
            return "";
        }

        std::map<int, bool> timeFlagMap;

        for(int i = 0; i< valueOarray.Capacity(); ++i)
        {
            rapidjson::Value& eachValue = valueOarray[i];

            int sq=0;
            std::string time ="";

            if(eachValue.HasMember("sq"))
            {
                sq = eachValue["sq"].GetInt();
            }
            if(eachValue.HasMember("time"))
            {
                if(eachValue["time"].IsString())
                {
                    time = eachValue["time"].GetString();
                }
            }
            if(strcmp(time.c_str(),"") == 0)
            {
                timeFlagMap.insert(std::make_pair(sq, false));
            }
            else
            {
                timeFlagMap.insert(std::make_pair(sq, true));
            }
        }

        member_iterator = doc.FindMember("info");

        if(member_iterator == doc.MemberEnd())
        {
            return "";
        }

        rapidjson::Value &valueInfo = (member_iterator->value);

        if(!valueInfo.IsArray())
        {
            return "";
        }
        //key: sq , value: each displayinfo
        std::multimap<int, std::string> info_map;

        for(int i = 0; i< valueInfo.Capacity(); ++i)
        {
            rapidjson::Value& eachValue = valueInfo[i];

            int info=0, flag=0, sq=0;

            if(eachValue.HasMember("info"))
            {
                info = eachValue["info"].GetInt();
            }
            if(eachValue.HasMember("flag"))
            {
                flag = eachValue["flag"].GetInt();
            }
            if(eachValue.HasMember("sq"))
            {
                sq = eachValue["sq"].GetInt();
            }
            std::stringstream ss;
            ss<<(int)Model::OrdinaryTurnRestriction;
            ss<<ResourceSplitSymbol;
            ss<<info;
            ss<<ResourceSplitSymbol;

            std::map<int, bool>::iterator timeFlagIter = timeFlagMap.find(sq);

            if(timeFlagIter == timeFlagMap.end())
            {
                continue;
            }

            bool timeFlag = timeFlagIter->second;

            if(flag == 1)
            {
                ss<< (timeFlag ? ActualTimeRestrictionFlag:ActualRestrictionFlag);
            }
            else if(flag == 2)
            {
                ss<< (timeFlag ? TheoreticalTimeRestrictionFlag:TheoreticalRestrictionFlag);
            }
            info_map.insert(std::make_pair(sq,ss.str()));
        }
        int columnCount = 0;

        for(std::multimap<int, std::string>::iterator iterator = info_map.begin(); iterator != info_map.end(); ++iterator)
        {
            Value json_value(kStringType);
            json_value.SetString(iterator->second.c_str(), iterator->second.size(),allocator);

            rapidjson::Value object(rapidjson::kObjectType);
            object.AddMember("iconName", json_value, allocator);
            object.AddMember("row", 0, allocator);
            object.AddMember("column", columnCount++, allocator);

            arrayValue.PushBack(object, allocator);
        }
        arrayValue.Accept(writer);
        
        std::string display_string = buffer.GetString();

        return display_string;
	}

    std::string  GetRoadDirectDisplayStyle(rapidjson::Document& doc)
    {
        int nDirect = -1;

        int nTime = 0;

        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        if(doc.HasMember("dr"))
        {
            nDirect = doc["dr"].GetInt();
        }

        //单方向道路才判断时间段
        if(nDirect == 2)
        {
            if(doc.HasMember("time"))
            {
                rapidjson::Value& value = doc["time"];
                
                if(!value.IsNull())
                {
                    std::string sTime = doc["time"].GetString();
                    
                    if(!sTime.empty())
                    {
                        nTime = 1;
                    }
                }
            }
        }
        
        if(nDirect == -1)
        {
            Logger::LogD("the json of deep is error, please check...");
            Logger::LogO("the json of deep is error, please check...");

            return "";
        }

        std::string sIconName = Tools::NumberToString<int>(Model::RoadDirect) + ResourceSplitSymbol + Tools::NumberToString<int>(nDirect) + ResourceSplitSymbol + Tools::NumberToString<int>(nTime);

        rapidjson::Value arrayValue(rapidjson::kArrayType);

        rapidjson::Value json_value(kStringType);

        json_value.SetString(sIconName.c_str(), sIconName.size(),allocator);

        rapidjson::Value object(rapidjson::kObjectType);

        object.AddMember("iconName", json_value, allocator);

        object.AddMember("row", 0, allocator);       

        object.AddMember("column", 0, allocator);

        arrayValue.PushBack(object, allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        arrayValue.Accept(writer); 

        return buffer.GetString();
    }
	
	std::string GetRoadNameDisplayStyle(rapidjson::Document& doc)
    {
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        rapidjson::Value arrayValue(rapidjson::kArrayType);
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        rapidjson::Document::MemberIterator member_iterator = doc.FindMember("n_array");
        
        if(member_iterator == doc.MemberEnd())
        {
            return "";
        }
        rapidjson::Value &valueNames = (member_iterator->value);
        
        if(!valueNames.IsArray())
        {
            return "";
        }

        std::string name;

        for(int i = 0; i<valueNames.Size(); i++)
        {
            rapidjson::Value& eachName = valueNames[i];

            if(!eachName.IsString())
            {
                continue;
            }

            name += eachName.GetString();

            if(i!=valueNames.Size()-1)
            {
                name +="/";
            }
        }

        std::string iconName =  Tools::NumberToString(Model::RoadName) + ResourceSplitSymbol + "0" + ResourceSplitSymbol +"0";

        Value json_value(kStringType);
        json_value.SetString(iconName.c_str(), iconName.size(),allocator);
        
        rapidjson::Value iconValue(rapidjson::kObjectType);
        iconValue.AddMember("iconName", json_value, allocator);
        iconValue.AddMember("row", 0, allocator);
        iconValue.AddMember("column", 0, allocator);
        arrayValue.PushBack(iconValue, allocator);

        json_value.SetString(name.c_str(), name.size(),allocator);

        rapidjson::Value nameValue(rapidjson::kObjectType);
        nameValue.AddMember("name", json_value, allocator);
        nameValue.AddMember("row", 0, allocator);
        nameValue.AddMember("column", 1, allocator);
        arrayValue.PushBack(nameValue, allocator);

        arrayValue.Accept(writer);
        
        std::string display_string = buffer.GetString();

        return display_string;
    }

	std::string GetBridgeDisplayStyle(rapidjson::Document& doc)
    {
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        std::string sIconName = Tools::NumberToString<int>((int)Model::Bridge) + ResourceSplitSymbol + "0" + ResourceSplitSymbol + "0";

        rapidjson::Value arrayValue(rapidjson::kArrayType);

        rapidjson::Value json_value(kStringType);

        json_value.SetString(sIconName.c_str(), sIconName.size(),allocator);

        rapidjson::Value object(rapidjson::kObjectType);

        object.AddMember("iconName", json_value, allocator);

        object.AddMember("row", 0, allocator);

        object.AddMember("column", 0, allocator);

        arrayValue.PushBack(object, allocator);

        std::string name;

        if(doc.HasMember("name"))
        {
            if(doc["name"].IsString())
            {
                name = doc["name"].GetString();
            }
        }

        json_value.SetString(name.c_str(), name.size(),allocator);

        rapidjson::Value nameObject(rapidjson::kObjectType);

        nameObject.AddMember("name", json_value, allocator);

        nameObject.AddMember("row", 0, allocator);

        nameObject.AddMember("column", 1, allocator);

        arrayValue.PushBack(nameObject, allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        arrayValue.Accept(writer);

        return buffer.GetString();
    }
    
    std::string GetTipsSEPointDisplayStyle(int type, rapidjson::Document& doc, bool isStartPoint)
    {
        if(doc.HasParseError())
        {
            return "";
        }
        
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        
        std::string sIconName = "";
        if (isStartPoint) {
             sIconName = Tools::NumberToString<int>(type) + ResourceSplitSymbol + "1" + ResourceSplitSymbol + "0";
        }
        else {
            sIconName = Tools::NumberToString<int>(type) + ResourceSplitSymbol + "2" + ResourceSplitSymbol + "0";
        }
       
        
        rapidjson::Document resultDoc;
        
        resultDoc.SetObject();
        
        rapidjson::Value arrayValue(rapidjson::kArrayType);
        
        rapidjson::Value json_value(kStringType);
        
        json_value.SetString(sIconName.c_str(), sIconName.size(),allocator);
        
        rapidjson::Value object(rapidjson::kObjectType);
        
        object.AddMember("iconName", json_value, allocator);
        
        object.AddMember("row", 0, allocator);
        
        object.AddMember("column", 0, allocator);
        
        arrayValue.PushBack(object, allocator);
        
        if (isStartPoint)
        {
            
            std::string name="";
            
            if (doc.HasMember("name") && !doc["name"].IsNull() && doc["name"].IsString())
            {
                
                name = doc["name"].GetString();
                
            }
            
            json_value.SetString(name.c_str(), name.size(),allocator);
            
            rapidjson::Value nameObject(rapidjson::kObjectType);
            
            nameObject.AddMember("name", json_value, allocator);
            
            nameObject.AddMember("row", 0, allocator);
            
            nameObject.AddMember("column", 1, allocator);
            
            arrayValue.PushBack(nameObject, allocator);
        }
        
        resultDoc.AddMember("icon",arrayValue, resultDoc.GetAllocator());
        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        resultDoc.Accept(writer);
        
        return buffer.GetString();
    }

    std::string  GetLaneConnexityDisplayStyle(rapidjson::Document& doc)
    {
        std::map<int, std::pair<std::string, int> > mResult;

        std::map<int, std::string> transitResult;
		
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        if(!doc.HasMember("info"))
        {
            Logger::LogD("RDLaneConnexity tips of deep does not have info of column");
            Logger::LogD("RDLaneConnexity tips of deep does not have info of column");

            return "";
        }

        rapidjson::Value& infoValue = doc["info"];

        if(!infoValue.IsArray())
        {
            Logger::LogD("RDLaneConnexity tips of deep is wrong!!!");
            Logger::LogD("RDLaneConnexity tips of deep is wrong!!!");

            return "";
        }

        for(int i = 0; i < infoValue.Size(); i++)
        {
            int nSq = -1;

            int nExt = -1;

            std::string sArwG = "";

            std::string sArwB = "";
			
            rapidjson::Value& value = infoValue[i];

            if(value.HasMember("sq"))
            {
                nSq = value["sq"].GetInt();
            }

            if(value.HasMember("ext"))
            {
                nExt = value["ext"].GetInt();
            }

            if(value.HasMember("arwG"))
            {
                sArwG = value["arwG"].GetString();
            }

            if(value.HasMember("arwB"))
            {
            	rapidjson::Value& arwb = value["arwB"];
            	if(!arwb.IsNull())
            	{
                sArwB = value["arwB"].GetString();
                transitResult.insert(std::make_pair(nSq, sArwB));
				}
            }
            mResult.insert(std::make_pair(nSq, std::make_pair(sArwG, nExt)));
        }

        rapidjson::Value arrayValue(rapidjson::kArrayType);
        std::map<int, std::string>::iterator it;

        std::map<int, std::pair<std::string, int> >::iterator iter = mResult.begin();

        for(; iter != mResult.end(); iter++)
        {
            rapidjson::Value json_value(kStringType);

            std::string sIconName = Tools::NumberToString<int>(Model::LaneConnexity) + ResourceSplitSymbol + iter->second.first + ResourceSplitSymbol + Tools::NumberToString<int>(iter->second.second);

            json_value.SetString(sIconName.c_str(), sIconName.size(),allocator);

            rapidjson::Value object(rapidjson::kObjectType);

            object.AddMember("iconName", json_value, allocator);

                object.AddMember("row", 0, allocator);
				
            object.AddMember("column", iter->first-1, allocator);

            it = transitResult.find(iter->first);
			
            if(it != transitResult.end())
            {
                std::string sIconName = Tools::NumberToString<int>(Model::LaneConnexity) + ResourceSplitSymbol + it->second + ResourceSplitSymbol + Tools::NumberToString<int>(2);
                
                rapidjson::Value commonObject(rapidjson::kObjectType);
				json_value.SetString(sIconName.c_str(), sIconName.size(),allocator);
                
				commonObject.AddMember("iconName", json_value, allocator);
                
				commonObject.AddMember("row", 1, allocator);
            
				commonObject.AddMember("column", iter->first-1, allocator);
				arrayValue.PushBack(commonObject, allocator);
            }


            arrayValue.PushBack(object, allocator);
        }

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        arrayValue.Accept(writer); 

        return buffer.GetString();
    }

    std::string GetRoadKindDisplayStyle(rapidjson::Document& doc)
	{
	    rapidjson::Document::MemberIterator member_iterator = doc.FindMember("kind");
        
        if(member_iterator == doc.MemberEnd())
        {
            return "";
        }
        rapidjson::Value &valueKey = (member_iterator->value);

        if(!valueKey.IsInt())
        {
            return "";
        }

        int kind = (member_iterator->value).GetInt();

        std::string iconName = Tools::NumberToString(Model::RoadKind) + ResourceSplitSymbol + Tools::NumberToString(kind) + ResourceSplitSymbol +"0";

        std::string displayStyle = GetConstantsDisplayStyle(iconName);

        return displayStyle;
	}

    std::string GetCrossNameDisplayStyle(rapidjson::Document& doc)
	{
	    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        rapidjson::Value arrayValue(rapidjson::kArrayType);
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        rapidjson::Document::MemberIterator member_iterator = doc.FindMember("name");
        
        if(member_iterator == doc.MemberEnd())
        {
            return "";
        }
        rapidjson::Value &valueKey = (member_iterator->value);
        
        if(!valueKey.IsString())
        {
            return "";
        }

        std::string crossIconName =  Tools::NumberToString(Model::CrossName) + ResourceSplitSymbol + "0" + ResourceSplitSymbol +"0";

        Value json_value(kStringType);
        json_value.SetString(crossIconName.c_str(), crossIconName.size(),allocator);
        
        rapidjson::Value iconValue(rapidjson::kObjectType);
        iconValue.AddMember("iconName", json_value, allocator);
        iconValue.AddMember("row", 0, allocator);
        iconValue.AddMember("column", 0, allocator);
        arrayValue.PushBack(iconValue, allocator);

        std::string name = valueKey.GetString();
        json_value.SetString(name.c_str(), name.size(),allocator);

        rapidjson::Value nameValue(rapidjson::kObjectType);
        nameValue.AddMember("name", json_value, allocator);
        nameValue.AddMember("row", 0, allocator);
        nameValue.AddMember("column", 1, allocator);
        arrayValue.PushBack(nameValue, allocator);

        arrayValue.Accept(writer);
        
        std::string display_string = buffer.GetString();

        return display_string;
	}

    std::string GetConstantsDisplayStyle(const std::string& iconName)
	{
        rapidjson::Document doc;
        doc.SetObject();
	    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        rapidjson::Value arrayValue(rapidjson::kArrayType);
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        Value json_value(kStringType);
        json_value.SetString(iconName.c_str(), iconName.size(),allocator);
        
        doc.AddMember("iconName", json_value, allocator);
        doc.AddMember("row", 0, allocator);
        doc.AddMember("column", 0, allocator);
        
        arrayValue.PushBack(doc, allocator);
        arrayValue.Accept(writer);
        
        std::string display_string = buffer.GetString();

        return display_string;
	}

    std::string GetPointSpeedLimitDisplayStyle(rapidjson::Document& doc)
	{
	    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        rapidjson::Value arrayValue(rapidjson::kArrayType);
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        if(!doc.HasMember("rdDir") || !doc.HasMember("value") || !doc.HasMember("se") || !doc.HasMember("flag"))
        {
            return "";
        }

        int rdDir = -1, se = -1, flag = -1;

        double value = 0.0;

        if(doc["rdDir"].IsInt())
        {
            rdDir = doc["rdDir"].GetInt();
        }

        if(doc["value"].IsDouble())
        {
            value = doc["value"].GetDouble();
        }
        else if(doc["value"].IsInt())
        {
            value = doc["value"].GetInt();
        }

        if(doc["se"].IsInt())
        {
            se = doc["se"].GetInt();
        }

        if(doc["flag"].IsInt())
        {
            flag = doc["flag"].GetInt();
        }

        std::string valueFlag;

        //限速开始
        if(flag == 0 && se == 0)
        {
            valueFlag = SpeedLimitStart;
        }
        if(flag == 0 && se == 1)//限速结束
        {
            valueFlag = SpeedLimitEnd;
        }
        if(flag == 1 && se == 0)//理论限速开始
        {
            valueFlag = TheoreticSpeedLimitStart;
        }
        if(flag == 1 && se == 1)//理论限速结束
        {
            valueFlag = TheoreticSpeedLimitEnd;
        }

        std::string valueIconName =  Tools::NumberToString(Model::PointSpeedLimit) + ResourceSplitSymbol + Tools::NumberToString<int>((int)value) + ResourceSplitSymbol + valueFlag;

        Value json_value(kStringType);
        json_value.SetString(valueIconName.c_str(), valueIconName.size(),allocator);
        
        rapidjson::Value iconValue(rapidjson::kObjectType);
        iconValue.AddMember(StringRef(IconNameStr.c_str()), json_value, allocator);
        iconValue.AddMember(StringRef(RowStr.c_str()), 1, allocator);
        iconValue.AddMember(StringRef(ColumnStr.c_str()), 0, allocator);
        arrayValue.PushBack(iconValue, allocator);

        std::string arrowName = Tools::NumberToString(Model::PointSpeedLimit) + ResourceSplitSymbol + Tools::NumberToString(2) + ResourceSplitSymbol + valueFlag; // 正北方向图片渲染值要求
        json_value.SetString(arrowName.c_str(), arrowName.size(),allocator);

        rapidjson::Value arrowValue(rapidjson::kObjectType);
        arrowValue.AddMember(StringRef(IconNameStr.c_str()), json_value, allocator);
        arrowValue.AddMember(StringRef(RowStr.c_str()), 0, allocator);
        arrowValue.AddMember(StringRef(ColumnStr.c_str()), 0, allocator);
        arrayValue.PushBack(arrowValue, allocator);

        arrayValue.Accept(writer);
        
        std::string display_string = buffer.GetString();

        return display_string;
	}
    std::string GetHitchingPointDisplayStyle(rapidjson::Document& doc)
    {
        rapidjson::Document::MemberIterator member_iterator = doc.FindMember("pcd");
        
        if(member_iterator == doc.MemberEnd())
        {
            return "";
        }
        rapidjson::Value &valueKey = (member_iterator->value);
        
        if(!valueKey.IsString())
        {
            return "";
        }
        
        std::string iconName = (member_iterator->value).GetString();
        
        std::string displayStyle = GetConstantsDisplayStyle(iconName);
        
        return displayStyle;
    }
    
    std::string GetHighwayBranchDisplayStyle(rapidjson::Document& doc)
    {
        std::string iconName = Tools::NumberToString(Model::HighwayBranch) + ResourceSplitSymbol + "0" + ResourceSplitSymbol +"0";
        
        std::string displayStyle = GetConstantsDisplayStyle(iconName);
        
        return displayStyle;
    }
    
    EditorGeometry::Point2D  WktToPoint2D(const std::string& wkt)
	{
        EditorGeometry::Point2D point2D;
        point2D._x = -1;
        point2D._y = -1;

	    geos::io::WKTReader wktReader;

        geos::geom::Geometry* geo = wktReader.read(wkt);

        if(NULL == geo)
        {
            return point2D;
        }

        const geos::geom::Coordinate* coordinate = geo->getCoordinate();

        if(NULL == coordinate)
        {
            return point2D;
        }

        point2D._x = coordinate->x;
        point2D._y = coordinate->y;

        delete geo;

        return point2D;
	}

    EditorGeometry::Point2D    GetGpsLineLocation(const std::string& wkt)
	{
	    geos::geom::Geometry* geo = NULL;
        
        EditorGeometry::Point2D point;
        point._x= -1.0;
        point._y= -1.0;

        geos::io::WKTReader wktReader;

        try
		{
			geo = wktReader.read(wkt);
		}
		catch (...)
		{
			Logger::LogD("GetGpsLineLocation, wkt:%s is not valid!", wkt.c_str());

			Logger::LogO("GetGpsLineLocation, wkt:%s is not valid!", wkt.c_str());
		}

		if (geo == NULL)
		{
			return point;
		}

        geos::geom::CoordinateSequence* coords = geo->getCoordinates();

        if(coords)
        {
            point._x = coords->getX(0);
            point._y = coords->getY(0);
        }

		delete geo;

		return point;
	}

    std::string GeoJson2Wkt(const std::string& geoJson,int sourceDataType)
    {
    	std::string strWkt="";

		std::string strPoint		= "Point";
		std::string strLineString	= "LineString";
		std::string strPolygon		= "Polygon";

		if(sourceDataType==Model::Tips::DataSourceFromServer)
    	{
			rapidjson::Document doc;
			doc.Parse<0>(geoJson.c_str());
			if(doc.HasParseError())
			{
				return "";
			}
			
			//geoJson
			//std::vector<double> lonlats;
			std::vector<EditorGeometry::Point2D> lonlats;
			if (!doc.HasMember("coordinates"))
			{
				return "";
			}
			rapidjson::Value& coordinates= doc["coordinates"];
			if (coordinates.IsArray())
			{
				if (!doc.HasMember("type"))
				{
					return "";
				}
				
				rapidjson::Value& typeValue= doc["type"];
				std::string typeString = "";
				if (typeValue.IsString())
				{
					typeString = typeValue.GetString();
				}
                
                int geoType = 0;
				if(0 == strcmp(Tools::ToUpper(typeString).c_str(), Tools::ToUpper(strPoint).c_str()))
				{
                    geoType = EditorGeometry::wkbPoint;
				}
				else if(0 == strcmp(Tools::ToUpper(typeString).c_str(), Tools::ToUpper(strLineString).c_str()))
				{
					geoType = EditorGeometry::wkbLineString;
				}
				else if(0 == strcmp(Tools::ToUpper(typeString).c_str(), Tools::ToUpper(strPolygon).c_str()))
				{
					geoType = EditorGeometry::wkbPolygon;
				}
                strWkt = coordinates2Wkt(coordinates, geoType);
			}
    	}
    	else if(sourceDataType==Model::Tips::DataSourceFromApp)
    	{
    		rapidjson::Document doc;
    		doc.Parse<0>(geoJson.c_str());
    		if(doc.HasParseError())
    		{
    			return "";
    		}
			
    		for(rapidjson::Document::MemberIterator ptr = doc.MemberBegin(); ptr != doc.MemberEnd(); ptr++)
    		{
    			std::string  sKey = (ptr->name).GetString();
    			rapidjson::Value &valueKey = (ptr->value);
    			std::string sValue = "";
    			if(!(ptr->value).IsNumber())
    			{
    				rapidjson::StringBuffer buffer;
    				rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    				valueKey.Accept(writer);
    				sValue = buffer.GetString();
    			}

				std::string strGeometryKey = "geometry";

    			if(0 == strcmp(Tools::ToUpper(sKey).c_str(), Tools::ToUpper(strGeometryKey).c_str()))
    			{
    				//geoJson
    				std::vector<EditorGeometry::Point2D> lonlats;
    				rapidjson::Document subdoc;
    				subdoc.Parse<0>(sValue.c_str());
					if (!subdoc.HasMember("coordinates"))
					{
						return "";
					}
    				rapidjson::Value& coordinates= subdoc["coordinates"];
    				if (coordinates.IsArray())
    				{
						if (!subdoc.HasMember("type"))
						{
							return "";
						}
						rapidjson::Value& typeValue= subdoc["type"];
						std::string typeString = "";
						if (typeValue.IsString())
						{
							typeString = typeValue.GetString();
						}
                        
                        int geoType = 0;

						if(0 == strcmp(Tools::ToUpper(typeString).c_str(), Tools::ToUpper(strPoint).c_str()))
						{
							geoType = EditorGeometry::wkbPoint;
						}
						else if(0 == strcmp(Tools::ToUpper(typeString).c_str(), Tools::ToUpper(strLineString).c_str()))
						{
                            geoType = EditorGeometry::wkbLineString;
						}
						else if(0 == strcmp(Tools::ToUpper(typeString).c_str(), Tools::ToUpper(strPolygon).c_str()))
						{
							geoType = EditorGeometry::wkbPolygon;
						}
                        strWkt = coordinates2Wkt(coordinates, geoType);
    				}
    			}
    		}
			
    }
	return strWkt;
}
    
EDITOR_API std::string Points2Wkt( std::vector<EditorGeometry::Point2D>& points, int geoType )
{
	std::string wkt="";
	geos::geom::CoordinateArraySequence sequence;

	for (int i=0;i<points.size();++i)
	{
		sequence.add(geos::geom::Coordinate(points.at(i)._x, points.at(i)._y));
	}

	switch (geoType)
	{
	case EditorGeometry::wkbPoint:
		{
			geos::geom::Point* point = geos::geom::GeometryFactory::getDefaultInstance()->createPoint(sequence);

			wkt = point->toText();
		}
		break;
	case EditorGeometry::wkbLineString:
		{
			geos::geom::LineString* lineString = geos::geom::GeometryFactory::getDefaultInstance()->createLineString(sequence);
			wkt = lineString->toText();
		}
		break;
	case EditorGeometry::wkbPolygon:
		{
			geos::geom::LinearRing* linearRing = geos::geom::GeometryFactory::getDefaultInstance()->createLinearRing(sequence);
			geos::geom::Polygon* polygon = geos::geom::GeometryFactory::getDefaultInstance()->createPolygon(linearRing,NULL);
			wkt = polygon->toText();
		}
		break;
	default:
		break;
	}

	return wkt;
}
    
    EDITOR_API std::string coordinates2Wkt(rapidjson::Value& coordinates, int geoType)
    {
        std::vector<EditorGeometry::Point2D> lonlats;
        std::string strWkt="";
        
        if (EditorGeometry::wkbPoint==geoType)
        {
            for (int i = 0; i < coordinates.Size()-1; i +=2)
            {
                EditorGeometry::Point2D point;
                rapidjson::Value& valDouble = coordinates[i];
                //json第i列
                if (valDouble.IsDouble())
                {
                    point._x=valDouble.GetDouble();
                }
                valDouble = coordinates[i+1];
                //json第i+1列
                if (valDouble.IsDouble())
                {
                    point._y=valDouble.GetDouble();
                }
                lonlats.push_back(point);
            }
            
            strWkt = DataTransfor::Points2Wkt(lonlats,EditorGeometry::wkbPoint);
        }
        else if (EditorGeometry::wkbLineString==geoType)
        {
            //LINESTRING(116.37604 39.73979, 116.375 39.73965)
            for (int i = 0; i < coordinates.Size(); i++)
            {
                rapidjson::Value& valInnerArray = coordinates[i];
                //json第i⾏行
                if(valInnerArray.IsArray())//判断是否为数组
                {
                    for (int j = 0; j < valInnerArray.Size()-1; j +=2)
                    {
                        EditorGeometry::Point2D point;
                        rapidjson::Value& valDouble = valInnerArray[j];
                        //json第j列
                        if (valDouble.IsDouble())
                        {
                            point._x=valDouble.GetDouble();
                        }
                        valDouble = valInnerArray[j+1];
                        //json第j+1列
                        if (valDouble.IsDouble())
                        {
                            point._y=valDouble.GetDouble();
                        }
                        lonlats.push_back(point);
                    }
                }
            }
            
            strWkt = Points2Wkt(lonlats,EditorGeometry::wkbLineString);
        }
        else if (EditorGeometry::wkbPolygon==geoType)
        {
            //Polygon(116.37604 39.73979, 116.375 39.73965)
            for (int i = 0; i < coordinates.Size(); i++)
            {
                rapidjson::Value& valInnerArray = coordinates[i];
                //json第i⾏行
                if(valInnerArray.IsArray())//判断是否为数组
                {
                    for (int i = 0; i < valInnerArray.Capacity(); i++)
                    {
                        rapidjson::Value& childvalInnerArray = valInnerArray[i];
                        if(childvalInnerArray.IsArray())//判断是否为数组
                        {
                            for (int j = 0; j < childvalInnerArray.Capacity()-1; j +=2)
                            {
                                EditorGeometry::Point2D point;
                                rapidjson::Value& valDouble = childvalInnerArray[j];
                                //json第j列
                                if (valDouble.IsDouble())
                                {
                                    point._x=valDouble.GetDouble();
                                }
                                valDouble = childvalInnerArray[j+1];
                                //json第j+1列
                                if (valDouble.IsDouble())
                                {
                                    point._y=valDouble.GetDouble();
                                }
                                lonlats.push_back(point);
                            }
                        }
                    }
                }
            }
            
            strWkt = Points2Wkt(lonlats,EditorGeometry::wkbPolygon);
        }
        
        return strWkt;
    }

    double CalculateSphereDistance(double x1, double y1, double x2, double y2)
	{
	    if (std::abs(x1 - x2) < PrecisionMap && abs(y1 - y2) < PrecisionMap)
	    {
	        return 0.0;
	    }
        x1 = x1 * (PI / 180.0);
        y1 = y1 * (PI / 180.0);
        x2 = x2 * (PI / 180.0);
        y2 = y2 * (PI / 180.0);

        double distance = 2 * EARTH_RADIUS * asin(0.5 * sqrt(2 - 2 * sin(y1) * sin(y2) - 2 * cos(y1) * cos(y2) * cos(x1 - x2)));
        
        return distance;
	}
    geos::geom::Geometry* Wkb2Geo(EditorGeometry::WkbGeometry* wkb)
    {
        geos::io::WKBReader wkbReader;

        geos::geom::Geometry* geo = NULL;
        try
        {
            std::string strWkb((char*)wkb, wkb->buffer_size());
	        std::istringstream istream(strWkb);
            geo = wkbReader.read(istream);
        }
        catch(...)
        {
            Logger::LogD("wkb of format is error!");
            Logger::LogD("wkb of format is error!");
            return NULL;
        }
        return geo;
    }

	EditorGeometry::WkbGeometry* GetDisplayPointFromDeep( std::string& deep )
	{
		rapidjson::Document doc;

		doc.Parse<0>(deep.c_str());

		if(doc.HasParseError())
		{
			return NULL;
		}

		for(rapidjson::Document::MemberIterator ptr = doc.MemberBegin(); ptr != doc.MemberEnd(); ptr++)
		{
			std::string  sKey = (ptr->name).GetString();

			rapidjson::Value &valueKey = (ptr->value);

			std::string sValue = "";

			if(!(ptr->value).IsNumber())
			{
				rapidjson::StringBuffer buffer;

				rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

				valueKey.Accept(writer); 

				sValue = buffer.GetString();
			}

			if(sKey == "geo")
			{
				//geoJson
				std::string strWkt = GeoJson2Wkt(sValue,Model::Tips::DataSourceFromServer);
				EditorGeometry::WkbGeometry* wkb = Wkt2Wkb(strWkt);
				return wkb;
			}
		}

		return NULL;
	}

	EDITOR_API geos::geom::Geometry* Wkt2Geo( std::string wkt )
	{
		geos::geom::Geometry* geo = NULL;
        
        geos::io::WKTReader wktReader;

        try
		{
			geo = wktReader.read(wkt);
		}
		catch (...)
		{
			Logger::LogD("Wkt2Geo, wkt:%s is not valid!", wkt.c_str());

			Logger::LogO("Wkt2Geo, wkt:%s is not valid!", wkt.c_str());
		}

        return geo;
	}

    std::string Wkb2GeoJson(EditorGeometry::WkbGeometry* wkb)
	{
	    if(NULL == wkb)
	    {
	        return "";
	    }
        
        geos::geom::Geometry* geosGeo = Wkb2Geo(wkb);

        if(NULL == geosGeo)
        {
            return "";
        }

        std::string geoJson;

	    switch (geosGeo->getGeometryTypeId())
	    {
            case geos::geom::GEOS_POINT:
                geoJson = GeosPoint2GeoJson(geosGeo);
                break;
            case geos::geom::GEOS_LINESTRING:
                geoJson = GeosLineString2GeoJson(geosGeo);
                break;
            case geos::geom::GEOS_POLYGON:
                geoJson = GeosPolygon2GeoJson(geosGeo);
                break;
            case geos::geom::GEOS_MULTILINESTRING:
                geoJson = GeosMultiLineString2GeoJson(geosGeo);
                break;
            default: 
                break;
	    }

        delete geosGeo;

        return geoJson;
	}

    std::string GeosPoint2GeoJson(geos::geom::Geometry* geometry)
	{
	    if(NULL == geometry)
	    {
	        return "";
	    }

        geos::geom::Point* point = dynamic_cast<geos::geom::Point*>(geometry);

        if(NULL == point)
        {
            return "";
        }

        rapidjson::Document doc;
        doc.SetObject();

        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        rapidjson::Value stringValue(kStringType);

        stringValue.SetString(PointStr.c_str(),PointStr.size(),allocator);

        doc.AddMember("type",stringValue,allocator);

        rapidjson::Value arrayValue(rapidjson::kArrayType);

        arrayValue.PushBack(point->getCoordinate()->x, allocator);
        arrayValue.PushBack(point->getCoordinate()->y, allocator);

        doc.AddMember(StringRef(CoordinatesStr.c_str()),arrayValue,allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        doc.Accept(writer);

        std::string geoJson = buffer.GetString();

        return geoJson;
	}

    std::string GeosLineString2GeoJson(geos::geom::Geometry* geometry)
	{
	    if(NULL == geometry)
	    {
	        return "";
	    }

        geos::geom::LineString* line = dynamic_cast<geos::geom::LineString*>(geometry);

        if(NULL == line)
        {
            return "";
        }

        rapidjson::Document doc;
        doc.SetObject();

        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        rapidjson::Value stringValue(kStringType);

        stringValue.SetString(LineStringStr.c_str(),LineStringStr.size(),allocator);

        doc.AddMember("type",stringValue,allocator);

	    geos::geom::CoordinateSequence* coordinateSequence = line->getCoordinates();

        rapidjson::Value arrayValue(rapidjson::kArrayType);

        for(int i = 0; i< coordinateSequence->size(); i++)
        {
            geos::geom::Coordinate coordinate = coordinateSequence->getAt(i);

            rapidjson::Value eachAarrayValue(rapidjson::kArrayType);

            eachAarrayValue.PushBack(coordinate.x, allocator);
            eachAarrayValue.PushBack(coordinate.y, allocator);

            arrayValue.PushBack(eachAarrayValue, allocator);
        }

        doc.AddMember(StringRef(CoordinatesStr.c_str()),arrayValue,allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        doc.Accept(writer);

        std::string geoJson = buffer.GetString();

        return geoJson;
	}

    std::string GeosPolygon2GeoJson(geos::geom::Geometry* geometry)
    {
        if(NULL == geometry)
        {
            return "";
        }
        
        geos::geom::Polygon* polygon = dynamic_cast<geos::geom::Polygon*>(geometry);
        
        if(NULL == polygon)
        {
            return "";
        }
        
        if(!polygon->isSimple())
        {
            return "";
        }
        
        rapidjson::Document doc;
        doc.SetObject();
        
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        
        rapidjson::Value stringValue(kStringType);
        
        stringValue.SetString(PolygonStr.c_str(),PolygonStr.size(),allocator);
        
        doc.AddMember("type",stringValue,allocator);
        
        geos::geom::CoordinateSequence* coordinateSequence = polygon->getCoordinates();
        
        rapidjson::Value arrayValue(rapidjson::kArrayType);
        
        for(int i = 0; i< coordinateSequence->size(); i++)
        {
            geos::geom::Coordinate coordinate = coordinateSequence->getAt(i);
            
            rapidjson::Value eachAarrayValue(rapidjson::kArrayType);
            
            eachAarrayValue.PushBack(coordinate.x, allocator);
            eachAarrayValue.PushBack(coordinate.y, allocator);
            
            arrayValue.PushBack(eachAarrayValue, allocator);
        }
        
        rapidjson::Value resultArrayValue(rapidjson::kArrayType);
        
        resultArrayValue.PushBack(arrayValue, allocator);
        
        doc.AddMember(StringRef(CoordinatesStr.c_str()),resultArrayValue,allocator);
        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        
        doc.Accept(writer);
        
        std::string geoJson = buffer.GetString();
        
        return geoJson;
    }
    
    std::string GeosMultiLineString2GeoJson(geos::geom::Geometry* geometry)
    {
        if(NULL == geometry)
        {
            return "";
        }
        
        geos::geom::MultiLineString* multiline = dynamic_cast<geos::geom::MultiLineString*>(geometry);
        
        if(NULL == multiline)
        {
            return "";
        }
        
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        
        rapidjson::Value stringValue(kStringType);
        stringValue.SetString(MultiLineStringStr.c_str(),MultiLineStringStr.size(),allocator);
        doc.AddMember("type",stringValue,allocator);
        
        rapidjson::Value multilineValue(rapidjson::kArrayType);
        
        for(geos::geom::GeometryCollection::const_iterator itor = multiline->begin();itor!=multiline->end();++itor)
        {
            geos::geom::LineString* line = dynamic_cast<geos::geom::LineString*>(*itor);
            
            if(NULL == line)
            {
                return "";
            }
            
            geos::geom::CoordinateSequence* coordinateSequence = multiline->getCoordinates();
            
            rapidjson::Value lineValue(rapidjson::kArrayType);
            
            for(int i = 0; i< coordinateSequence->size(); ++i)
            {
                geos::geom::Coordinate coordinate = coordinateSequence->getAt(i);
                
                rapidjson::Value xyValue(rapidjson::kArrayType);
                
                xyValue.PushBack(coordinate.x, allocator);
                xyValue.PushBack(coordinate.y, allocator);
                
                lineValue.PushBack(xyValue, allocator);
            }
            
            multilineValue.PushBack(lineValue, allocator);
        }
        
        doc.AddMember(StringRef(CoordinatesStr.c_str()),multilineValue,allocator);
        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        
        doc.Accept(writer);
        
        std::string geoJson = buffer.GetString();
        
        return geoJson;
    }
    
    std::string GetRdLink(const std::string& tipsJson)
    {
        rapidjson::Document tipsDoc;
        tipsDoc.Parse<0>(tipsJson.c_str());
        std::string linkPid;
        if(tipsDoc.HasMember("s_sourceType") && tipsDoc.HasMember("deep"))
        {
            rapidjson::Value& deepValue = tipsDoc["deep"];
            int type = Tools::StringToNum(tipsDoc["s_sourceType"].GetString());
            switch (type)
	        {
            case Model::RoadDirect:
                return GetRoadDirRelateRdLink(deepValue);
            default:
                return "";
            }
        }
        return "";
    }
    std::string   GetRoadDirRelateRdLink(rapidjson::Value& deepValue)
    {
        std::string linkPid;
        if(deepValue.HasMember("deep"))
        {
            rapidjson::Value& value = deepValue["deep"];
            if(value.HasMember("f"))
            {
                rapidjson::Value& fValue = value["f"];
                if(fValue.HasMember("id"))
                {
                    linkPid = fValue["id"].GetString();
                }
            }
        }
        return linkPid;
    }

    bool CheckBridgeDeep(rapidjson::Value& fArray, std::string& id, int& type, std::string& flag)
	{
	    if(fArray.IsNull())
	    {
	        return false;
	    }

        if(!fArray.HasMember("id") || !fArray.HasMember("type") || !fArray.HasMember("flag"))
        {
            return false;
        }
        
        if(fArray["id"].IsNull() || fArray["flag"].IsNull())
        {
            return false;
        }

        id = fArray["id"].GetString();
        type = fArray["type"].GetInt();
        flag = fArray["flag"].GetString();

        return true;
	}
    
    std::vector<geos::geom::Geometry*> GetSETipsRelatedLineGeo(rapidjson::Value& fArray)
    {
        std::vector<geos::geom::Geometry*> result;
        
        if(!fArray.IsArray())
        {
            return result;
        }
        
        //起始link,终点link;
        std::string startLinkID="",endLinkID="";
        
        Editor::DATALAYER_TYPE startLinkLayerType, endLinkLayerType;
        
        std::vector<std::string> componentRdLineIds;
        std::vector<std::string> componentTipsLineIds;
        
        for(int i =0;i<fArray.Capacity();i++)
        {
            rapidjson::Value& each = fArray[i];
            
            std::string id, flag;
            int type;
            
            if(!CheckBridgeDeep(each, id, type ,flag))
            {
                continue;
            }
            
            //中间组成线
            if(flag == "0")
            {
                if(type == 1)
                {
                    componentRdLineIds.push_back(id);
                }
                else
                {
                    componentTipsLineIds.push_back(id);
                }
            }
            else if(flag == "1")//关联起点
            {
                startLinkID = id;
                
                startLinkLayerType = type==1?Editor::DATALAYER_RDLINE:Editor::DATALAYER_TIPSLINE;
            }
            else if(flag == "2")//关联终点
            {
                endLinkID = id;
                
                endLinkLayerType = type==1?Editor::DATALAYER_RDLINE:Editor::DATALAYER_TIPSLINE;
            }
            else//同时关联起终点
            {
                startLinkID = id;
                
                endLinkID = id;
                
                startLinkLayerType = type==1?Editor::DATALAYER_RDLINE:Editor::DATALAYER_TIPSLINE;
            }
            
        }
        
        if(startLinkID == "" || endLinkID == "")
        {
            Logger::LogD("GetSETipsRelatedLineGeo failed, start or end link is missing, startLinkID:[%s],endLinkID:[%s]", startLinkID.c_str(),endLinkID.c_str());
            Logger::LogO("GetSETipsRelatedLineGeo failed, start or end link is missing, startLinkID:[%s],endLinkID:[%s]", startLinkID.c_str(),endLinkID.c_str());
            
            return result;
        }
        
        Editor::DataManager* dataManager = Editor::DataManager::getInstance();
        
        Editor::DataFunctor* dataFunctor = dataManager->getDataFunctor();
        
        geos::geom::Geometry* startLineGeometry = dataFunctor->GetLineGeometry(startLinkID, startLinkLayerType);
        
        if(startLinkID == endLinkID)
        {
            result.push_back(startLineGeometry);
            
            return result;
        }
        
        geos::geom::Geometry* endLineGeometry = dataFunctor->GetLineGeometry(endLinkID, endLinkLayerType);
        
        std::vector<geos::geom::Geometry*> rdLineGeometries =  dataFunctor->GetGeometries(componentRdLineIds, Editor::DATALAYER_RDLINE);
        
        std::vector<geos::geom::Geometry*> tipsLineGeometries =  dataFunctor->GetGeometries(componentTipsLineIds, Editor::DATALAYER_TIPSLINE);
        
        result.push_back(startLineGeometry);
        
        for(int i =0;i<rdLineGeometries.size();i++)
        {
            result.push_back(rdLineGeometries.at(i));
        }
        
        for(int i =0;i<tipsLineGeometries.size();i++)
        {
            result.push_back(tipsLineGeometries.at(i));
        }
        
        result.push_back(endLineGeometry);
        
        return result;
    }

    geos::geom::LineString* GetBridgeRelatedLineGeo(rapidjson::Value& fArray, geos::geom::LineString** startLink, geos::geom::LineString** endLink)
	{
	    if(!fArray.IsArray())
	    {
	        return NULL;
	    }
        
        std::vector<std::string> componentRdLineIds;
        std::vector<std::string> componentTipsLineIds;

        std::map<std::string,int> idIndexMap;

        int componentLineCount=0;
        
        //起始link,终末link;
        std::string startLinkID="",endLinkID="";
        //1:rdLink, 2:测线；
        int startLinkType=0,endLinkLinkType=0;

        for(int i =0;i<fArray.Capacity();i++)
        {
            rapidjson::Value& each = fArray[i];
               
            std::string id, flag;
            int type;

            if(!CheckBridgeDeep(each, id, type ,flag))
            {
                continue;
            }

            if(type == 1)
            {
                componentRdLineIds.push_back(id);
            }
            else
            {
                componentTipsLineIds.push_back(id);
            }

            idIndexMap[id] = i;

            componentLineCount++;
            
            if(atoi(flag.c_str())==1)
            {
                startLinkID=id;
                startLinkType = type;
            }
            
            if(atoi(flag.c_str())==2)
            {
                endLinkID=id;
                endLinkLinkType = type;
            }
        }

        Editor::DataManager* dataManager = Editor::DataManager::getInstance();

        Editor::DataFunctor* dataFunctor = dataManager->getDataFunctor();

        std::vector<geos::geom::Geometry*> rdLineGeometries =  dataFunctor->GetGeometries(componentRdLineIds, Editor::DATALAYER_RDLINE);

        std::vector<geos::geom::Geometry*> tipsLineGeometries =  dataFunctor->GetGeometries(componentTipsLineIds, Editor::DATALAYER_TIPSLINE);
        
        if (startLinkID!="")
        {
            Editor::DATALAYER_TYPE stDataLayerType = Editor::DATALAYER_RDLINE;
            if (startLinkType==1)
            {
                stDataLayerType = Editor::DATALAYER_RDLINE;
            }
            else if (startLinkType==2)
            {
               stDataLayerType = Editor::DATALAYER_TIPSLINE;
            }
            
            std::vector<std::string> stIDs;
            stIDs.push_back(startLinkID);
            std::vector<geos::geom::Geometry*> stGeos =  dataFunctor->GetGeometries(stIDs, stDataLayerType);
            
            if(stGeos.size()>0)
            {
                geos::geom::CoordinateArraySequence arraySequence;
                geos::geom::Geometry* geo = stGeos.at(0);
                if (geo!=NULL)
                {
                    geos::geom::CoordinateSequence* pTargSequence = geo->getCoordinates();
                    
                    if(pTargSequence)
                    {
                        for (int i=0; i<pTargSequence->size(); ++i)
                        {
                            arraySequence.add(pTargSequence->getAt(i));
                        }
                        
                        *startLink = geos::geom::GeometryFactory::getDefaultInstance()->createLineString(arraySequence);
                    }
                }
               
            }
            
        }
        
        if (endLinkID!="")
        {
            Editor::DATALAYER_TYPE endDataLayerType = Editor::DATALAYER_RDLINE;
            std::vector<std::string> endIDs;
            endIDs.push_back(endLinkID);
            
            if (endLinkLinkType==1)
            {
                endDataLayerType = Editor::DATALAYER_RDLINE;
            }
            else if (endLinkLinkType==2)
            {
                endDataLayerType = Editor::DATALAYER_TIPSLINE;
            }
            
            std::vector<geos::geom::Geometry*> endGeos =  dataFunctor->GetGeometries(endIDs, endDataLayerType);
            if(endGeos.size()>0)
            {
                geos::geom::CoordinateArraySequence arraySequence;
                geos::geom::Geometry* geo = endGeos.at(0);
                if (geo!=NULL)
                {
                    geos::geom::CoordinateSequence* pTargSequence = geo->getCoordinates();
                    if(pTargSequence)
                    {
                        for (int i=0; i<pTargSequence->size(); ++i)
                        {
                            arraySequence.add(pTargSequence->getAt(i));
                        }
                        *endLink=geos::geom::GeometryFactory::getDefaultInstance()->createLineString(arraySequence);
                    }
                }
                
            }

        }
        

        if(rdLineGeometries.size() != componentRdLineIds.size() || tipsLineGeometries.size() != componentTipsLineIds.size())
        {
            return NULL;
        }

        std::map<int, geos::geom::Geometry*> lineGeometries;

        for(int i=0 ;i<componentRdLineIds.size();i++)
        {
            if(idIndexMap.find(componentRdLineIds[i]) != idIndexMap.end())
            {
                lineGeometries[idIndexMap[componentRdLineIds[i]]] = rdLineGeometries[i];
            }
        }

        for(int i=0 ;i<tipsLineGeometries.size();i++)
        {
            if(idIndexMap.find(componentTipsLineIds[i]) != idIndexMap.end())
            {
                lineGeometries[idIndexMap[componentTipsLineIds[i]]] = tipsLineGeometries[i];
            }
        }

        geos::geom::CoordinateArraySequence arraySequence;

        Editor::GeometryCalculator* geometryCalculator = Editor::GeometryCalculator::getInstance();

        ///拼接桥的关联link的几何
        std::list<geos::geom::Coordinate> coordinatesList;
        
        for(int i =0 ; i<componentLineCount;i++)
        {
            std::map<int, geos::geom::Geometry*>::iterator targetIter = lineGeometries.find(i);
            if (targetIter==lineGeometries.end()||targetIter->second==NULL)
            {
                continue;
            }
            
            geos::geom::CoordinateSequence * targeCoords = targetIter->second->getCoordinates();
            
            if(targeCoords==NULL)
            {
               continue;
            }
            
            if(0==i)
            {
                for(int j = 0;j<targeCoords->size(); j++)
                {
                    coordinatesList.push_back(targeCoords->getAt(j));
                }
                
                int crossType = -1;
                
                std::map<int, geos::geom::Geometry*>::iterator sourceIter = lineGeometries.find(i+1);
                if(sourceIter!=lineGeometries.end())
                {
                    crossType = geometryCalculator->GetCrossPointTypeEx(coordinatesList, sourceIter->second);
                }
                
                if (2==crossType||0==crossType)
                {
                    coordinatesList.reverse();
                }
            }
            else
            {
                int crossType = -1;
                crossType = geometryCalculator->GetCrossPointTypeEx(coordinatesList, targetIter->second);
                
                if (0==crossType)
                {
                    //源点串的起点与目标点串的起点:挂接
                    for(int j = 1;j<targeCoords->size(); j++)
                    {
                        coordinatesList.push_front(targeCoords->getAt(j));
                    }
                }
                else if (1==crossType)
                {
                    //源点串的终点与目标点串的起点:挂接
                    for(int j = 1;j<targeCoords->size(); j++)
                    {
                        coordinatesList.push_back(targeCoords->getAt(j));
                    }
                }
                else if (2==crossType)
                {
                    //源点串的起点与目标点串的终点:挂接
                    for(int j = int(targeCoords->size())-1; j>1; j--)
                    {
                        coordinatesList.push_front(targeCoords->getAt(j));
                    }
                }
                else if (3==crossType)
                {
                    //源点串的终点与目标点串的终点:挂接
                    for(int j = int(targeCoords->size())-1; j>1; j--)
                    {
                        coordinatesList.push_back(targeCoords->getAt(j));
                    }
                }
            }
            
        }
        
        std::list<geos::geom::Coordinate>::iterator itor = coordinatesList.begin();
        while(itor!=coordinatesList.end())
        {
            arraySequence.add(*itor);
            itor++;
        }

        geos::geom::LineString* resultLine = geos::geom::GeometryFactory::getDefaultInstance()->createLineString(arraySequence);
        
        std::map<int, geos::geom::Geometry*>::iterator iterator = lineGeometries.begin();

        for(; iterator != lineGeometries.end(); ++iterator)
        {
            delete iterator->second;
        }
        
        return resultLine;
	}

    std::string GetSETipsGeo(const std::string& tipsJson)
    {
        rapidjson::Document tipsDoc;
        tipsDoc.SetObject();
        
        if(tipsDoc.Parse<0>(tipsJson.c_str()).HasParseError())
        {
            return "";
        }
        
        if(!tipsDoc.HasMember("deep"))
        {
            return "";
        }
        
        rapidjson::Value& docDeep = tipsDoc["deep"];
        
        if(!docDeep.HasMember("f_array") || !docDeep.HasMember("gSLoc") || !docDeep.HasMember("gELoc"))
        {
            return "";
        }
        
        rapidjson::Value& f_value= docDeep["f_array"];
        rapidjson::Value& sLoc_value= docDeep["gSLoc"];
        rapidjson::Value& eLoc_value= docDeep["gELoc"];
        
        if(f_value.IsNull() || sLoc_value.IsNull() || eLoc_value.IsNull())
        {
            return "";
        }
        
        std::string sLoc_Str,eLoc_Str;
        
        rapidjson::StringBuffer sBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> sWriter(sBuffer);
        sLoc_value.Accept(sWriter);
        sLoc_Str= sBuffer.GetString();
        
        rapidjson::StringBuffer eBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> eWriter(eBuffer);
        eLoc_value.Accept(eWriter);
        eLoc_Str = eBuffer.GetString();
        
        std::string sLoc_wkt = GeoJson2Wkt(sLoc_Str, Model::Tips::DataSourceFromServer);
        std::string eLoc_wkt = GeoJson2Wkt(eLoc_Str, Model::Tips::DataSourceFromServer);
        
        geos::geom::Geometry* s_point = Wkt2Geo(sLoc_wkt);
        geos::geom::Geometry* e_point = Wkt2Geo(eLoc_wkt);
        
        if(s_point == NULL || e_point == NULL)
        {
            return "";
        }
        
        std::vector<geos::geom::Geometry*> lines = GetSETipsRelatedLineGeo(f_value);
        
        if(lines.size() == 0)
        {
            return "";
        }
        
        Editor::GeometryCalculator* geometryCalculator = Editor::GeometryCalculator::getInstance();
        
        geos::geom::Geometry* startLine = NULL;
        geos::geom::Geometry* endLine = NULL;
        
        if(lines.size() == 1)
        {
            startLine = geometryCalculator->GetLineSegment(dynamic_cast<geos::geom::Point*>(s_point), dynamic_cast<geos::geom::Point*>(e_point), dynamic_cast<geos::geom::LineString*>(lines.at(0)));
        }
        else
        {
            startLine = geometryCalculator->GetSETipsStartEndLineGeo(lines, s_point, 0);
            
            endLine = geometryCalculator->GetSETipsStartEndLineGeo(lines, e_point, 1);
        }
        
        std::vector<geos::geom::Geometry*> multiLines;
        
        if(startLine != NULL)
        {
            multiLines.push_back(startLine);
        }
        
        if(endLine != NULL)
        {
            multiLines.push_back(endLine);
        }
        
        if(lines.size()>1)
        {
            for(int i = 1 ; i< lines.size()-1; i++)
            {
                multiLines.push_back(lines.at(i));
            }
        }
        
        geos::geom::MultiLineString* multiLineGeo = geos::geom::GeometryFactory::getDefaultInstance()->createMultiLineString(multiLines);
        
        std::string multiLineStr;
        
        if(multiLineGeo != NULL)
        {
            multiLineStr = multiLineGeo->toText();
        }
        
        delete multiLineGeo;
        
        delete s_point;
        
        delete e_point;
        
        for(int i =0; i<multiLines.size();i++)
        {
            if(multiLines.at(i) != NULL)
            {
                delete multiLines.at(i);
                
                multiLines.at(i) = NULL;
            }
        }
        
        delete lines.at(0);
        
        if(lines.size()>1)
        {
            delete lines.at(lines.size()-1);
        }
        
        return multiLineStr;
    }
    
    std::string GetBridgeGeo(const std::string& tipsJson)
	{
        rapidjson::Document tipsDoc;
        tipsDoc.SetObject();

        if(tipsDoc.Parse<0>(tipsJson.c_str()).HasParseError())
        {
            return "";
        }

        if(!tipsDoc.HasMember("deep"))
        {
            return "";
        }

	    rapidjson::Value& docDeep = tipsDoc["deep"];

        if(!docDeep.HasMember("f_array") || !docDeep.HasMember("gSLoc") || !docDeep.HasMember("gELoc"))
        {
            return "";
        }

        rapidjson::Value& f_value= docDeep["f_array"];
        rapidjson::Value& sLoc_value= docDeep["gSLoc"];
        rapidjson::Value& eLoc_value= docDeep["gELoc"];

        if(f_value.IsNull() || sLoc_value.IsNull() || eLoc_value.IsNull())
        {
            return "";
        }

        std::string sLoc_Str,eLoc_Str;

        rapidjson::StringBuffer sBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> sWriter(sBuffer);
        sLoc_value.Accept(sWriter);
        sLoc_Str= sBuffer.GetString();

        rapidjson::StringBuffer eBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> eWriter(eBuffer);
        eLoc_value.Accept(eWriter);
        eLoc_Str = eBuffer.GetString();

        std::string sLoc_wkt = GeoJson2Wkt(sLoc_Str, Model::Tips::DataSourceFromServer);
        std::string eLoc_wkt = GeoJson2Wkt(eLoc_Str, Model::Tips::DataSourceFromServer);

        geos::geom::Geometry* s_point = Wkt2Geo(sLoc_wkt);
        geos::geom::Geometry* e_point = Wkt2Geo(eLoc_wkt);
        

        if(s_point == NULL || e_point == NULL)
        {
            return "";
        }

	    Editor::GeometryCalculator* geometryCalculator = Editor::GeometryCalculator::getInstance();

        Editor::DataManager* dataManager = Editor::DataManager::getInstance();

        if(!f_value.IsArray() || f_value.Capacity() == 0)
        {
            return "";
        }


        geos::geom::LineString* resultLinestring = NULL;

        geos::geom::LineString* componentLinestring = NULL;
        
        geos::geom::LineString* stGeo = NULL;
        
         geos::geom::LineString* endGeo =NULL;

        int componentLineCount = f_value.Capacity();

        if(componentLineCount == 0)
        {
            return "";
        }
        else
        {
            componentLinestring = GetBridgeRelatedLineGeo(f_value, &stGeo, &endGeo);
        }
    
        //求起始点和终末点的垂足；
        geos::geom::Point* stFoot = NULL;
        geos::geom::Point* endFoot = NULL;
        
        geos::geom::Point* stPoint = dynamic_cast<geos::geom::Point*>(s_point);
        if (stGeo && stPoint)
        {
            
            std::pair<double, double> result;
            geos::geom::CoordinateSequence* coords = stGeo->getCoordinates();
            double dist = DBL_MAX;
            double footx = -1;
            double footy = -1;
            for(int i = 0; i < coords->size() - 1; i++)
            {
                std::pair<double, std::pair<double, double> > result = Editor::GeometryCalculator::getInstance()->MeasurePointToSegLineDistanceByHeron(stPoint->getX(), stPoint->getY(),coords->getAt(i).x, coords->getAt(i).y,coords->getAt(i+1).x, coords->getAt(i+1).y);
                if(result.first < dist)
                {
                    dist = result.first;
                    footx = result.second.first;
                    footy = result.second.second;
                }
            }
            
            if (footx!=-1&&footy!=-1)
            {
                geos::geom::Coordinate coord;
                coord.x = footx;
                coord.y = footy;
                
                stFoot = geos::geom::GeometryFactory::getDefaultInstance()->createPoint(coord);
            }

        }
        
        geos::geom::Point* endPoint = dynamic_cast<geos::geom::Point*>(e_point);
        if (endGeo && endPoint)
        {
            std::pair<double, double> result;
            geos::geom::CoordinateSequence* coords = endGeo->getCoordinates();
            double dist = DBL_MAX;
            double footx = -1;
            double footy = -1;
            for(int i = 0; i < coords->size() - 1; i++)
            {
                std::pair<double, std::pair<double, double> > result = Editor::GeometryCalculator::getInstance()->MeasurePointToSegLineDistanceByHeron(endPoint->getX(), endPoint->getY(),coords->getAt(i).x, coords->getAt(i).y,coords->getAt(i+1).x, coords->getAt(i+1).y);
                if(result.first < dist)
                {
                    dist = result.first;
                    footx = result.second.first;
                    footy = result.second.second;
                }
            }
            
            if (footx!=-1&&footy!=-1)
            {
                geos::geom::Coordinate coord;
                coord.x = footx;
                coord.y = footy;
                
                endFoot = geos::geom::GeometryFactory::getDefaultInstance()->createPoint(coord);
            }
            
        }
        
        if(stFoot && endFoot)
        {
            resultLinestring = geometryCalculator->GetLineSegment(stFoot,
                                                                  endFoot,componentLinestring);
        }
        else
        {
            resultLinestring = geometryCalculator->GetLineSegment(dynamic_cast<geos::geom::Point*>(s_point),
                                                                  dynamic_cast<geos::geom::Point*>(e_point),componentLinestring);
        }
        
        
        std::string resultStr;

        if (s_point)
        {
            delete s_point;
        }
        
        if (e_point)
        {
            delete e_point;
        }
        
        if(stGeo)
        {
            delete stGeo;
        }
        
        if (endGeo)
        {
            delete endGeo;
        }

        if(resultLinestring !=NULL)
        {
            resultStr = resultLinestring->toText();

            delete resultLinestring;
        }
        
        if(componentLinestring!=NULL)
        {
            delete componentLinestring;
        }
        
        
        return resultStr;
	}
    
    EDITOR_API std::string RapidJsonObj2String(rapidjson::Value& rValue)
    {
        if(rValue.IsNull())
        {
            return "";
        }
        
        std::string result;
        
        if(rValue.IsString())
        {
            result = rValue.GetString();
        }
        else if(!rValue.IsNull())
        {
            rapidjson::StringBuffer buffer;
            
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            
            rValue.Accept(writer);
            
            result = buffer.GetString();
        }
        
        return result;
    }
    
    EDITOR_API EditorGeometry::Point2D LonLat2Mercator(EditorGeometry::Point2D lonLat)
    {
        EditorGeometry::Point2D  mercator;
        double x = lonLat._x *20037508.34/180;
        double y = log(tan((90+lonLat._y)*M_PI/360))/(M_PI/180);
        y = y *20037508.34/180;
        mercator._x = x;
        mercator._y = y;
        return mercator ;
    }
    
    EDITOR_API EditorGeometry::Point2D Mercator2LonLat(EditorGeometry::Point2D mercator)
    {
        EditorGeometry::Point2D lonLat;
        double x = mercator._x/20037508.34*180;
        double y = mercator._y/20037508.34*180;
        y= 180/M_PI*(2*atan(exp(y*M_PI/180))-M_PI/2);
        lonLat._x = x;
        lonLat._y = y;
        return lonLat;
    }
}
