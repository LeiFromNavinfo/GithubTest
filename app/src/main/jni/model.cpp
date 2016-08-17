#include "Model.h"
#include "Tools.h"
#include <sstream>
#include <map>
#include <algorithm>
#include "BussinessConstants.h"
#include "Editor.h"

namespace Model
{
    int Point::ParseJson(std::string sJson)
    {   
        rapidjson::Document doc;

        doc.Parse<0>(sJson.c_str());

        if(doc.HasParseError())
        {
            Logger::LogD("Point::ParseJson sJson[%s]:HasParseError",sJson.c_str());
            Logger::LogO("Point::ParseJson sJson[%s]:HasParseError",sJson.c_str());
            
            return -1;
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

            if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_attachments)).c_str()))
            {
                attachments = sValue;
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_indoor)).c_str()))
            {
                indoor = sValue;
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_pid)).c_str()))
            {
                pid = (ptr->value).GetInt();
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_brands)).c_str()))
            {
                brands = sValue;
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_names)).c_str()))
            {
                names = sValue;
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_sportsVenues)).c_str()))
            {
                sportsVenues = sValue;
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_addresses)).c_str()))
            {
                addresses = sValue;
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_contacts)).c_str()))
            {
                contacts = sValue;
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_kindCode)).c_str()))
            {
                kindCode = GetCorrectString(sValue);
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_chargingStation)).c_str()))
            {
                chargingStation = sValue;
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_edits)).c_str())
                    ||0 == strcmp(Tools::ToUpper(sKey).c_str(),"EDITHISTORY"))
            {
                edits = sValue;
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_fid)).c_str()))
            {
                fid = GetCorrectString(sValue);
            }
            /*else if(sKey == "rowkey")
            {
                rowkey = GetCorrectString(sValue);
            }*/
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_sourceCollection)).c_str()))
            {
                sourceCollection = GetCorrectString(sValue);
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_open24H)).c_str()))
            {
                open24H = (ptr->value).GetInt();
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_parkings)).c_str()))
            {
                parkings = sValue;
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_adminCode)).c_str()))
            {
                adminCode = GetCorrectString(sValue);
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_evaluatePlanning)).c_str()))
            {
                evaluatePlanning = (ptr->value).GetInt();
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_foodtypes)).c_str()))
            {
                foodtypes = sValue;
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_address)).c_str()))
            {
                address = GetCorrectString(sValue);;
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_rawFields)).c_str()))
            {
                rawFields = sValue;
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_lifecycle)).c_str()))
            {
                lifecycle = (ptr->value).GetInt();
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_submitStatus)).c_str()))
            {
                submitStatus = (ptr->value).GetInt();
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_gasStation)).c_str()))
            {
                gasStation = sValue;
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_name)).c_str()))
            {
                name = GetCorrectString(sValue);
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_meshid)).c_str()))
            {
                meshid = GetCorrectString(sValue);
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_level)).c_str()))
            {
                level = GetCorrectString(sValue);
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_geometry)).c_str()))
            {
                /*std::string geometry = GetCorrectString(sValue);

                sscanf(geometry.c_str(), "POINT (%lf %lf)", &longitude, &latitude);*/

                if(!valueKey.IsNull())
                {
                    if(valueKey.HasMember("latitude"))
                    {
                        latitude = valueKey["latitude"].GetDouble();
                    }

                    if(valueKey.HasMember("longitude"))
                    {
                        longitude = valueKey["longitude"].GetDouble();
                    }
                }
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),"LOCATION"))
            //else if(sKey == "location") // 有的数据中有geometry  有的数据有location
            {
                if(valueKey.HasMember("latitude"))
                {
                    latitude = valueKey["latitude"].GetDouble();
                }
                if(valueKey.HasMember("longitude"))
                {
                    longitude = valueKey["longitude"].GetDouble();
                }
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_hotel)).c_str()))
            {
                hotel = sValue;
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_relateChildren)).c_str()))
            {
                relateChildren = sValue;
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_relateParent)).c_str()))
            {
                relateParent = sValue;

                if(sValue != "null")
                {
                    simple_parentFid = valueKey["parentFid"].GetString();
                }
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_postCode)).c_str()))
            {
                postCode = GetCorrectString(sValue);
            }
            /*else if(sKey == "_id")
            {
                _id = GetCorrectString(sValue);
            }*/
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_guide)).c_str()))
            {
                guide = sValue;
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_chargingPole)).c_str()))
            {
                chargingPole = sValue;
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_qtStatus)).c_str()))
            {
            	qtStatus = (ptr->value).GetInt();
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_type)).c_str()))
            {
                type = (ptr->value).GetInt();
            }
            else if(0 == strcmp(Tools::ToUpper(sKey).c_str(),Tools::ToUpper(const_cast<std::string&>(Poi_srcInformation)).c_str()))
            {
                srcInformation = sValue;
            }
            else if (sKey == "project")
            {
                project = GetCorrectString(sValue);
            }
        }

        return 0;
    }

    std::string Point::GetCorrectString(std::string sIn)
    {
        if(sIn.empty())
        {
            return "";
        }

        if(sIn.size() >= 2 && (sIn[0] == '\"' && sIn[sIn.length() - 1] == '\"'))
        {
            return sIn.substr(1, sIn.size() - 2);
        }

        return "";
    }

    std::string Point::ToJson()
    {
        Document document;
        Document docTmp;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();


        Value each_json_value(kStringType);  
        std::string each_str_value = fid;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);  
        document.AddMember("fid",each_json_value,allocator);

        each_str_value = project;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);  
        document.AddMember("project",each_json_value,allocator);


        each_str_value = display_style;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("display_style",each_json_value,allocator);

        each_str_value = meshid;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("meshid",each_json_value,allocator);

        document.AddMember("pid",pid,allocator);
        each_str_value = name;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("name",each_json_value,allocator);

        each_str_value = kindCode;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("kindCode",each_json_value,allocator);

        document.AddMember("evaluatePlanning",evaluatePlanning,allocator);
        document.AddMember("qtStatus",qtStatus,allocator);

        each_str_value = guide;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("guide",docTmp,allocator);

        each_str_value = address;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("address",each_json_value,allocator);

        each_str_value = postCode;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("postCode",each_json_value,allocator);

        each_str_value = adminCode;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("adminCode",each_json_value,allocator);

        each_str_value = level;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("level",each_json_value,allocator);

        document.AddMember("open24H",open24H,allocator);

        each_str_value = relateParent;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("relateParent", docTmp, allocator);

        each_str_value = relateChildren;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("relateChildren", docTmp, allocator);

        each_str_value = names;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("names", docTmp, allocator);

        each_str_value = addresses;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("addresses", docTmp, allocator);

        each_str_value = contacts;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("contacts", docTmp, allocator);

        each_str_value = foodtypes;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("foodtypes", docTmp, allocator);

        each_str_value = parkings;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("parkings", docTmp, allocator);

        each_str_value = hotel;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("hotel", docTmp, allocator);

        each_str_value = sportsVenues;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("sportsVenues", docTmp, allocator);

        each_str_value = chargingStation;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("chargingStation", docTmp, allocator);

        each_str_value = chargingPole;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("chargingPole", docTmp, allocator);

        each_str_value = gasStation;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("gasStation", docTmp, allocator);

        each_str_value = indoor;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("indoor", docTmp, allocator);

        each_str_value = attachments;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("attachments", docTmp, allocator);

        each_str_value = brands;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("brands", docTmp, allocator);

        each_str_value = rawFields;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("rawFields", docTmp, allocator);

        document.AddMember("submitStatus",submitStatus,allocator);

        each_str_value = sourceCollection;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("sourceCollection", docTmp, allocator);

        document.AddMember("lifecycle",lifecycle,allocator);

        each_str_value = edits;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("edits", docTmp, allocator);

        each_str_value = simple_parentFid;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("simple_parentFid", docTmp, allocator);

        document.AddMember("auditStatus",auditStatus,allocator);

        document.AddMember("submitStatus_desktop",submitStatus_desktop,allocator);

        each_str_value = latestMergeDate;
        docTmp.Parse<0>(each_str_value.c_str());
        document.AddMember("latestMergeDate", docTmp, allocator);

        std::stringstream ss;

        ss<<"Point(";
        ss<<longitude;
        ss<<" ";
        ss<<latitude;
        ss<<")";
        each_str_value = ss.str();
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("geometry",each_json_value,allocator);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();

        return sResult;
    }

    std::string Point::ToUploadJson()
    {
        Document document;
        Document docTmp;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();

        Value each_json_value(kStringType);  
        std::string each_str_value = fid;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);  
        document.AddMember("fid",each_json_value,allocator);

        each_str_value = latestMergeDate;
		each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("mergeDate", each_json_value, allocator);

        each_str_value = "Android";
		each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("sourceName", each_json_value, allocator);

        each_str_value = "";
		each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("_id", each_json_value, allocator);

        each_str_value = "null";
		each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("rowkey", each_json_value, allocator);

        each_str_value = Tools::NumberToString(pid);
		each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("pid", each_json_value, allocator);

        each_str_value = meshid;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("meshid",each_json_value,allocator);

        each_str_value = name;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("name",each_json_value,allocator);

        each_str_value = kindCode;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("kindCode",each_json_value,allocator);

        each_str_value = Tools::NumberToString(evaluatePlanning);
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("evaluatePlanning",each_json_value,allocator);

        each_str_value = Tools::NumberToString(qtStatus);
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("qtStatus",each_json_value,allocator);

        each_str_value = vipFlag;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("vipFlag",each_json_value,allocator);

        each_str_value = guide;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("guide",each_json_value,allocator);

        each_str_value = address;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("address",each_json_value,allocator);

        each_str_value = postCode;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("postCode",each_json_value,allocator);

        each_str_value = adminCode;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("adminCode",each_json_value,allocator);

        each_str_value = level;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("level",each_json_value,allocator);

        each_str_value = Tools::NumberToString(open24H);
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("open24H",each_json_value,allocator);

        each_str_value = relateParent;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("relateParent",each_json_value,allocator);

        each_str_value = relateChildren;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("relateChildren",each_json_value,allocator);

        each_str_value = names;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("names",each_json_value,allocator);

        each_str_value = addresses;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("addresses",each_json_value,allocator);

        each_str_value = contacts;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("contacts",each_json_value,allocator);

        each_str_value = foodtypes;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("foodtypes",each_json_value,allocator);

        each_str_value = parkings;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("parkings",each_json_value,allocator);

        each_str_value = hotel;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("hotel",each_json_value,allocator);

        each_str_value = sportsVenues;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("sportsVenues",each_json_value,allocator);

        each_str_value = chargingStation;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("chargingStation",each_json_value,allocator);

        each_str_value = chargingPole;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("chargingPole",each_json_value,allocator);

        each_str_value = gasStation;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("gasStation",each_json_value,allocator);

        each_str_value = indoor;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("indoor",each_json_value,allocator);

        each_str_value = attachments;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("attachments",each_json_value,allocator);

        each_str_value = brands;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("brands",each_json_value,allocator);

        each_str_value = rawFields;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("rawFields",each_json_value,allocator);

        each_str_value = Tools::NumberToString(submitStatus);
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("submitStatus",each_json_value,allocator);

        each_str_value = sourceCollection;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("sourceCollection",each_json_value,allocator);

        each_str_value = Tools::NumberToString(lifecycle);
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("lifecycle",each_json_value,allocator);

        std::stringstream ss;

		ss<<"Point(";
		ss<<longitude;
		ss<<" ";
		ss<<latitude;
		ss<<")";
		each_str_value = ss.str();
		each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
		document.AddMember("geometry",each_json_value,allocator);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();

        return sResult;
    }

    int Point::ParseSqlite(sqlite3_stmt *statement)
    {
        globalId = GetAsString(statement, 0);
	    fid = GetAsString(statement, 1);
        project = GetAsString(statement, 2);
        display_style = GetAsString(statement, 3);
        display_text = GetAsString(statement, 4);
	    name = GetAsString(statement, 5);
        pid = sqlite3_column_int(statement, 6);
	    meshid = GetAsString(statement, 7);
	    kindCode = GetAsString(statement, 8);
	    evaluatePlanning = sqlite3_column_int(statement, 9);
        qtStatus = sqlite3_column_int(statement, 10);
	    guide = GetAsString(statement, 11);
	    address = GetAsString(statement, 12);
	    postCode = GetAsString(statement, 13);
	    adminCode = GetAsString(statement, 14);
	    level = GetAsString(statement, 15);
	    open24H = sqlite3_column_int(statement, 16);
	    relateParent = GetAsString(statement, 17);
	    relateChildren = GetAsString(statement, 18);
        names = GetAsString(statement, 19);
        addresses = GetAsString(statement, 20);
	    contacts = GetAsString(statement, 21);
	    foodtypes = GetAsString(statement, 22);
	    parkings = GetAsString(statement, 23);
	    hotel = GetAsString(statement, 24);
	    sportsVenues = GetAsString(statement, 25);
	    chargingStation = GetAsString(statement, 26);
	    chargingPole = GetAsString(statement, 27);
	    gasStation = GetAsString(statement, 28);
	    indoor = GetAsString(statement, 29);
	    attachments = GetAsString(statement, 30);
	    brands = GetAsString(statement, 31);
	    rawFields = GetAsString(statement, 32);
	    submitStatus = sqlite3_column_int(statement, 33);
	    sourceCollection = GetAsString(statement, 34);
	    lifecycle = sqlite3_column_int(statement, 35);
        
	    //edits = GetAsString(statement, 36);
        unsigned int length = sqlite3_column_bytes(statement, 36);
        unsigned char* blob = (unsigned char*)sqlite3_column_blob(statement, 36);
        edits =std::string(reinterpret_cast<char*>(blob),length);
        
	    simple_parentFid = GetAsString(statement, 37);
	    auditStatus = sqlite3_column_int(statement, 38);
	    submitStatus_desktop = sqlite3_column_int(statement, 39);
	    latestMergeDate = GetAsString(statement, 40);
				
	    int geo_length = sqlite3_column_bytes(statement, 41);
	    const void* geo_buff = sqlite3_column_blob(statement, 41);
	    EditorGeometry::WkbPoint* wkb = (EditorGeometry::WkbPoint*)EditorGeometry::SpatialiteGeometry::ToWKBGeometry((EditorGeometry::SpatialiteGeometry*)geo_buff);
	    longitude = wkb->_point._x;
	    latitude = wkb->_point._y;
	    free(wkb);

        task_status = sqlite3_column_int(statement, 42);
        vipFlag = GetAsString(statement, 43);
        type = sqlite3_column_int(statement, 44);
        t_operateDate = GetAsString(statement, 45);
        info_globalId = GetAsString(statement, 46);
	    return 0;
    }
    
    std::string Point::GetAsString(sqlite3_stmt *statement, unsigned int index)
    {
        int length;
        
        const unsigned char* buff;
        
        length = sqlite3_column_bytes(statement, index+1);
        
        buff = sqlite3_column_text(statement, index+1);
        
        std::string ret((char*)buff, length);
        
        return ret;
    }

    int Point::InsertSqliteTable(sqlite3* db)
    {
        char *err_msg = NULL;

	    int ret = sqlite3_exec (db, "BEGIN", NULL, NULL, &err_msg);

        if (ret != SQLITE_OK)
        {
		    Logger::LogD("InsertSqliteTable BEGIN error: %s\n", err_msg);
            Logger::LogO("InsertSqliteTable BEGIN error: %s\n", err_msg);

            sqlite3_free(err_msg);
            return false;
        }

	    sqlite3_stmt* stmt;

	    std::string sql = "insert into edit_pois values("
		    "?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
		    "?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
		    "?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
		    "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

	    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("insert %s failed [%s]", "Point", sqlite3_errmsg(db));
            Logger::LogO("insert %s failed [%s]", "Point", sqlite3_errmsg(db));

		    return false;
	    }

	    sqlite3_bind_text(stmt, 1, globalId.c_str(), globalId.length(), NULL);

	    sqlite3_bind_text(stmt, 2, fid.c_str(), fid.length(), NULL);

	    sqlite3_bind_text(stmt, 3, project.c_str(), project.length(), NULL);

	    sqlite3_bind_text(stmt, 4, display_style.c_str(), display_style.length(), NULL);

	    sqlite3_bind_text(stmt, 5, display_text.c_str(), display_text.length(), NULL);
        sqlite3_bind_text(stmt, 6, name.c_str(), name.length(), NULL);

        sqlite3_bind_int(stmt, 7, pid);


	    sqlite3_bind_text(stmt, 8, meshid.c_str(), meshid.length(), NULL);


	    sqlite3_bind_text(stmt, 9, kindCode.c_str(), kindCode.length(), NULL);


	    sqlite3_bind_int(stmt, 10, evaluatePlanning);

        sqlite3_bind_int(stmt, 11, qtStatus);

	    sqlite3_bind_blob(stmt, 12, guide.c_str(), guide.length(), NULL);

	    sqlite3_bind_text(stmt, 13, address.c_str(), address.length(), NULL);

	    sqlite3_bind_text(stmt, 14, postCode.c_str(), postCode.length(), NULL);

	    sqlite3_bind_text(stmt, 15, adminCode.c_str(), adminCode.length(), NULL);

	    sqlite3_bind_text(stmt, 16, level.c_str(), level.length(), NULL);

	    sqlite3_bind_int(stmt, 17, open24H);

	    sqlite3_bind_blob(stmt, 18, relateParent.c_str(), relateParent.length(), NULL);

	    sqlite3_bind_blob(stmt, 19, relateChildren.c_str(), relateChildren.length(), NULL);

	    sqlite3_bind_blob(stmt, 20, names.c_str(), names.length(), NULL);

	    sqlite3_bind_blob(stmt, 21, addresses.c_str(), addresses.length(), NULL);

	    sqlite3_bind_blob(stmt, 22, contacts.c_str(), contacts.length(), NULL);

	    sqlite3_bind_blob(stmt, 23, foodtypes.c_str(), foodtypes.length(), NULL);

	    sqlite3_bind_blob(stmt, 24, parkings.c_str(), parkings.length(), NULL);

	    sqlite3_bind_blob(stmt, 25, hotel.c_str(), hotel.length(), NULL);

	    sqlite3_bind_blob(stmt, 26, sportsVenues.c_str(), sportsVenues.length(), NULL);

	    sqlite3_bind_blob(stmt, 27, chargingStation.c_str(), chargingStation.length(), NULL);

	    sqlite3_bind_blob(stmt, 28, chargingPole.c_str(), chargingPole.length(), NULL);

	    sqlite3_bind_blob(stmt, 29, gasStation.c_str(), gasStation.length(), NULL);

	    sqlite3_bind_blob(stmt, 30, indoor.c_str(), indoor.length(), NULL);

	    sqlite3_bind_blob(stmt, 31, attachments.c_str(), attachments.length(), NULL);

	    sqlite3_bind_blob(stmt, 32, brands.c_str(), brands.length(), NULL);

	    sqlite3_bind_text(stmt, 33, rawFields.c_str(), rawFields.length(), NULL);

	    sqlite3_bind_int(stmt, 34, submitStatus);

	    sqlite3_bind_text(stmt, 35, sourceCollection.c_str(), sourceCollection.length(), NULL);

	    sqlite3_bind_int(stmt, 36, lifecycle);

	    sqlite3_bind_blob(stmt, 37, edits.c_str(), edits.length(), NULL);

	    sqlite3_bind_text(stmt, 38, simple_parentFid.c_str(), simple_parentFid.length(), NULL);

	    sqlite3_bind_int(stmt, 39, auditStatus);

	    sqlite3_bind_int(stmt, 40, submitStatus_desktop);

	    sqlite3_bind_text(stmt, 41, latestMergeDate.c_str(), latestMergeDate.length(), NULL);

        EditorGeometry::WkbPoint* wkb=new EditorGeometry::WkbPoint();
	    wkb->_byteOrder = 1;
		wkb->_wkbType = (EditorGeometry::WkbGeometryType)1;
        wkb->_point._x = longitude;
	    wkb->_point._y = latitude;

	    EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

	    sqlite3_bind_blob(stmt, 42, spGeo, wkb->buffer_size()+39, NULL);

        sqlite3_bind_int(stmt, 43, task_status);
        sqlite3_bind_text(stmt, 44, vipFlag.c_str(), vipFlag.length(), NULL);

        sqlite3_bind_int(stmt, 45, type);
        sqlite3_bind_text(stmt, 46, t_operateDate.c_str(), t_operateDate.length(), NULL);
        sqlite3_bind_text(stmt, 47, info_globalId.c_str(), info_globalId.length(), NULL);

	    delete wkb;

	    if (sqlite3_step(stmt) != SQLITE_DONE)
	    {
			free(spGeo);
			
		    sqlite3_finalize(stmt);

		    Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
            Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));

            return false;
	    }

	    free(spGeo);

	    spGeo = NULL;

	    sqlite3_finalize(stmt);

        ret = sqlite3_exec (db, "COMMIT", NULL, NULL, &err_msg);

        if (ret != SQLITE_OK)
        {
            Logger::LogD ("COMMIT error: %s\n", err_msg);
            Logger::LogO ("COMMIT error: %s\n", err_msg);

            sqlite3_free (err_msg);
		
		    return false;
        }

        return 0;
    }

    int Point::InsertSqliteTableWithoutTransaction(sqlite3* db)
    {
        sqlite3_stmt* stmt;

	    std::string sql = "insert into edit_pois values("
		    "?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
		    "?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
		    "?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
		    "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

	    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("insert %s failed [%s]", "Point", sqlite3_errmsg(db));
            Logger::LogO("insert %s failed [%s]", "Point", sqlite3_errmsg(db));

		    return false;
	    }

	    sqlite3_bind_text(stmt, 1, globalId.c_str(), globalId.length(), NULL);

	    sqlite3_bind_text(stmt, 2, fid.c_str(), fid.length(), NULL);

	    sqlite3_bind_text(stmt, 3, project.c_str(), project.length(), NULL);

	    sqlite3_bind_text(stmt, 4, display_style.c_str(), display_style.length(), NULL);

	    sqlite3_bind_text(stmt, 5, display_text.c_str(), display_text.length(), NULL);
        sqlite3_bind_text(stmt, 6, name.c_str(), name.length(), NULL);

        sqlite3_bind_int(stmt, 7, pid);


	    sqlite3_bind_text(stmt, 8, meshid.c_str(), meshid.length(), NULL);


	    sqlite3_bind_text(stmt, 9, kindCode.c_str(), kindCode.length(), NULL);


	    sqlite3_bind_int(stmt, 10, evaluatePlanning);

        sqlite3_bind_int(stmt, 11, qtStatus);

	    sqlite3_bind_blob(stmt, 12, guide.c_str(), guide.length(), NULL);

	    sqlite3_bind_text(stmt, 13, address.c_str(), address.length(), NULL);

	    sqlite3_bind_text(stmt, 14, postCode.c_str(), postCode.length(), NULL);

	    sqlite3_bind_text(stmt, 15, adminCode.c_str(), adminCode.length(), NULL);

	    sqlite3_bind_text(stmt, 16, level.c_str(), level.length(), NULL);

	    sqlite3_bind_int(stmt, 17, open24H);

	    sqlite3_bind_blob(stmt, 18, relateParent.c_str(), relateParent.length(), NULL);

	    sqlite3_bind_blob(stmt, 19, relateChildren.c_str(), relateChildren.length(), NULL);

	    sqlite3_bind_blob(stmt, 20, names.c_str(), names.length(), NULL);

	    sqlite3_bind_blob(stmt, 21, addresses.c_str(), addresses.length(), NULL);

	    sqlite3_bind_blob(stmt, 22, contacts.c_str(), contacts.length(), NULL);

	    sqlite3_bind_blob(stmt, 23, foodtypes.c_str(), foodtypes.length(), NULL);

	    sqlite3_bind_blob(stmt, 24, parkings.c_str(), parkings.length(), NULL);

	    sqlite3_bind_blob(stmt, 25, hotel.c_str(), hotel.length(), NULL);

	    sqlite3_bind_blob(stmt, 26, sportsVenues.c_str(), sportsVenues.length(), NULL);

	    sqlite3_bind_blob(stmt, 27, chargingStation.c_str(), chargingStation.length(), NULL);

	    sqlite3_bind_blob(stmt, 28, chargingPole.c_str(), chargingPole.length(), NULL);

	    sqlite3_bind_blob(stmt, 29, gasStation.c_str(), gasStation.length(), NULL);

	    sqlite3_bind_blob(stmt, 30, indoor.c_str(), indoor.length(), NULL);

	    sqlite3_bind_blob(stmt, 31, attachments.c_str(), attachments.length(), NULL);

	    sqlite3_bind_blob(stmt, 32, brands.c_str(), brands.length(), NULL);

	    sqlite3_bind_text(stmt, 33, rawFields.c_str(), rawFields.length(), NULL);

	    sqlite3_bind_int(stmt, 34, submitStatus);

	    sqlite3_bind_text(stmt, 35, sourceCollection.c_str(), sourceCollection.length(), NULL);

	    sqlite3_bind_int(stmt, 36, lifecycle);

	    sqlite3_bind_blob(stmt, 37, edits.c_str(), edits.length(), NULL);

	    sqlite3_bind_text(stmt, 38, simple_parentFid.c_str(), simple_parentFid.length(), NULL);

	    sqlite3_bind_int(stmt, 39, auditStatus);

	    sqlite3_bind_int(stmt, 40, submitStatus_desktop);

	    sqlite3_bind_text(stmt, 41, latestMergeDate.c_str(), latestMergeDate.length(), NULL);

        EditorGeometry::WkbPoint* wkb=new EditorGeometry::WkbPoint();
	    wkb->_byteOrder = 1;
		wkb->_wkbType = (EditorGeometry::WkbGeometryType)1;
        wkb->_point._x = longitude;
	    wkb->_point._y = latitude;

	    EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

	    sqlite3_bind_blob(stmt, 42, spGeo, wkb->buffer_size()+39, NULL);

        sqlite3_bind_int(stmt, 43, task_status);
        sqlite3_bind_text(stmt, 44, vipFlag.c_str(), vipFlag.length(), NULL);

        sqlite3_bind_int(stmt, 45, type);
        sqlite3_bind_text(stmt, 46, t_operateDate.c_str(), t_operateDate.length(), NULL);
        sqlite3_bind_text(stmt, 47, info_globalId.c_str(), info_globalId.length(), NULL);
        //sqlite3_bind_text(stmt, 48, srcInformation.c_str(), srcInformation.length(), NULL);

	    delete wkb;

	    if (sqlite3_step(stmt) != SQLITE_DONE)
	    {
			free(spGeo);
			
		    sqlite3_finalize(stmt);

		    Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
            Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));

            return false;
	    }

	    free(spGeo);

	    spGeo = NULL;

	    sqlite3_finalize(stmt);

        return 0;
    }

    int Point::UpdateSqliteTable(sqlite3* db)
    {
        char *err_msg = NULL;

	    int ret = sqlite3_exec (db, "BEGIN", NULL, NULL, &err_msg);

        if (ret != SQLITE_OK)
        {
		    Logger::LogD("UpdateSqliteTable BEGIN error: %s\n", err_msg);
            Logger::LogO("UpdateSqliteTable BEGIN error: %s\n", err_msg);

            sqlite3_free (err_msg);
            return false;
        }

	    sqlite3_stmt* stmt;

	    std::string sql = "update edit_pois set "
		    "globalId=?, fid=?, project=?, display_style=?, display_text=?, name=?, pid=?, meshid=?, kindCode=?,"
		    "evaluatePlanning=?, qtStatus=?, guide=?, address=?, postCode=?, adminCode=?, level=?, open24H=?, relateParent=?, relateChildren=?,"
		    "names=?, addresses=?, contacts=?, foodtypes=?, parkings=?, hotel=?, sportsVenues=?, chargingStation=?, chargingPole=?, gasStation=?, indoor=?,"
		    "attachments=?, brands=?, rawFields=?, submitStatus=?, sourceCollection=?, lifecycle=?, edits=?, simple_parentFid=?, auditStatus=?,"
		    "submitStatus_desktop=?, latestMergeDate=?, Geometry=?, task_status=?, vipFlag=?, type=?, t_operateDate=?, info_globalId=? where globalId=?";

	    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("UpdateSqliteTable: update %s failed [%s]", "Point", sqlite3_errmsg(db));
            Logger::LogO("UpdateSqliteTable: update %s failed [%s]", "Point", sqlite3_errmsg(db));

		    return false;
	    }

	    sqlite3_bind_text(stmt, 1, globalId.c_str(), globalId.length(), NULL);

	    sqlite3_bind_text(stmt, 2, fid.c_str(), fid.length(), NULL);

	    sqlite3_bind_text(stmt, 3, project.c_str(), project.length(), NULL);

	    sqlite3_bind_text(stmt, 4, display_style.c_str(), display_style.length(), NULL);

	    sqlite3_bind_text(stmt, 5, display_text.c_str(), display_text.length(), NULL);
        sqlite3_bind_text(stmt, 6, name.c_str(), name.length(), NULL);

        sqlite3_bind_int(stmt, 7, pid);


	    sqlite3_bind_text(stmt, 8, meshid.c_str(), meshid.length(), NULL);


	    sqlite3_bind_text(stmt, 9, kindCode.c_str(), kindCode.length(), NULL);


	    sqlite3_bind_int(stmt, 10, evaluatePlanning);

        sqlite3_bind_int(stmt, 11, qtStatus);

	    sqlite3_bind_blob(stmt, 12, guide.c_str(), guide.length(), NULL);

	    sqlite3_bind_text(stmt, 13, address.c_str(), address.length(), NULL);

	    sqlite3_bind_text(stmt, 14, postCode.c_str(), postCode.length(), NULL);

	    sqlite3_bind_text(stmt, 15, adminCode.c_str(), adminCode.length(), NULL);

	    sqlite3_bind_text(stmt, 16, level.c_str(), level.length(), NULL);

	    sqlite3_bind_int(stmt, 17, open24H);

	    sqlite3_bind_blob(stmt, 18, relateParent.c_str(), relateParent.length(), NULL);

	    sqlite3_bind_blob(stmt, 19, relateChildren.c_str(), relateChildren.length(), NULL);

	    sqlite3_bind_blob(stmt, 20, names.c_str(), names.length(), NULL);

	    sqlite3_bind_blob(stmt, 21, addresses.c_str(), addresses.length(), NULL);

	    sqlite3_bind_blob(stmt, 22, contacts.c_str(), contacts.length(), NULL);

	    sqlite3_bind_blob(stmt, 23, foodtypes.c_str(), foodtypes.length(), NULL);

	    sqlite3_bind_blob(stmt, 24, parkings.c_str(), parkings.length(), NULL);

	    sqlite3_bind_blob(stmt, 25, hotel.c_str(), hotel.length(), NULL);

	    sqlite3_bind_blob(stmt, 26, sportsVenues.c_str(), sportsVenues.length(), NULL);

	    sqlite3_bind_blob(stmt, 27, chargingStation.c_str(), chargingStation.length(), NULL);

	    sqlite3_bind_blob(stmt, 28, chargingPole.c_str(), chargingPole.length(), NULL);

	    sqlite3_bind_blob(stmt, 29, gasStation.c_str(), gasStation.length(), NULL);

	    sqlite3_bind_blob(stmt, 30, indoor.c_str(), indoor.length(), NULL);

	    sqlite3_bind_blob(stmt, 31, attachments.c_str(), attachments.length(), NULL);

	    sqlite3_bind_blob(stmt, 32, brands.c_str(), brands.length(), NULL);

	    sqlite3_bind_text(stmt, 33, rawFields.c_str(), rawFields.length(), NULL);

	    sqlite3_bind_int(stmt, 34, submitStatus);

	    sqlite3_bind_text(stmt, 35, sourceCollection.c_str(), sourceCollection.length(), NULL);

	    sqlite3_bind_int(stmt, 36, lifecycle);

	    sqlite3_bind_blob(stmt, 37, edits.c_str(), edits.length(), NULL);

	    sqlite3_bind_text(stmt, 38, simple_parentFid.c_str(), simple_parentFid.length(), NULL);

	    sqlite3_bind_int(stmt, 39, auditStatus);

	    sqlite3_bind_int(stmt, 40, submitStatus_desktop);

	    sqlite3_bind_text(stmt, 41, latestMergeDate.c_str(), latestMergeDate.length(), NULL);

	    EditorGeometry::WkbPoint* wkb=new EditorGeometry::WkbPoint();
	    wkb->_byteOrder = 1;
		wkb->_wkbType = (EditorGeometry::WkbGeometryType)1;
        wkb->_point._x = longitude;
	    wkb->_point._y = latitude;

	    EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

	    sqlite3_bind_blob(stmt, 42, spGeo, wkb->buffer_size()+39, NULL);

        sqlite3_bind_int(stmt, 43, task_status);
        sqlite3_bind_text(stmt, 44, vipFlag.c_str(), vipFlag.length(), NULL);
        sqlite3_bind_int(stmt, 45, type);
        sqlite3_bind_text(stmt, 46, t_operateDate.c_str(), t_operateDate.length(), NULL);
        sqlite3_bind_text(stmt, 47, info_globalId.c_str(), info_globalId.length(), NULL);
	    delete wkb;

        sqlite3_bind_text(stmt, 48, globalId.c_str(), globalId.length(), NULL);


	    if (sqlite3_step(stmt) != SQLITE_DONE)
	    {
		    Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
		    Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
			
			free(spGeo);

		    sqlite3_finalize(stmt);

            return false;
	    }

	    free(spGeo);

	    spGeo = NULL;

	    sqlite3_finalize(stmt);

	    ret = sqlite3_exec (db, "COMMIT", NULL, NULL, &err_msg);
        if (ret != SQLITE_OK)
        {
            Logger::LogD ("COMMIT error: %s\n", err_msg);
            Logger::LogO ("COMMIT error: %s\n", err_msg);

            sqlite3_free (err_msg);
		
		    return false;
        }
        return 0;
    }

    int Point::UpdateSqliteTableWithoutTransaction(sqlite3* db)
    {
        sqlite3_stmt* stmt;

	    std::string sql = "update edit_pois set "
		    "globalId=?, fid=?, project=?, display_style=?, display_text=?, name=?, pid=?, meshid=?, kindCode=?,"
		    "evaluatePlanning=?, qtStatus=?, guide=?, address=?, postCode=?, adminCode=?, level=?, open24H=?, relateParent=?, relateChildren=?,"
		    "names=?, addresses=?, contacts=?, foodtypes=?, parkings=?, hotel=?, sportsVenues=?, chargingStation=?, chargingPole=?, gasStation=?, indoor=?,"
		    "attachments=?, brands=?, rawFields=?, submitStatus=?, sourceCollection=?, lifecycle=?, edits=?, simple_parentFid=?, auditStatus=?,"
		    "submitStatus_desktop=?, latestMergeDate=?, Geometry=?, task_status=?, vipFlag=?, type=?, t_operateDate=?, info_globalId=? where globalId=?";

	    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("UpdateSqliteTableWithoutTransaction: update %s failed [%s]", "Point", sqlite3_errmsg(db));
            Logger::LogO("UpdateSqliteTableWithoutTransaction: update %s failed [%s]", "Point", sqlite3_errmsg(db));

		    return false;
	    }

	    sqlite3_bind_text(stmt, 1, globalId.c_str(), globalId.length(), NULL);

	    sqlite3_bind_text(stmt, 2, fid.c_str(), fid.length(), NULL);

	    sqlite3_bind_text(stmt, 3, project.c_str(), project.length(), NULL);

	    sqlite3_bind_text(stmt, 4, display_style.c_str(), display_style.length(), NULL);

	    sqlite3_bind_text(stmt, 5, display_text.c_str(), display_text.length(), NULL);
        sqlite3_bind_text(stmt, 6, name.c_str(), name.length(), NULL);

        sqlite3_bind_int(stmt, 7, pid);


	    sqlite3_bind_text(stmt, 8, meshid.c_str(), meshid.length(), NULL);


	    sqlite3_bind_text(stmt, 9, kindCode.c_str(), kindCode.length(), NULL);


	    sqlite3_bind_int(stmt, 10, evaluatePlanning);

        sqlite3_bind_int(stmt, 11, qtStatus);

	    sqlite3_bind_blob(stmt, 12, guide.c_str(), guide.length(), NULL);

	    sqlite3_bind_text(stmt, 13, address.c_str(), address.length(), NULL);

	    sqlite3_bind_text(stmt, 14, postCode.c_str(), postCode.length(), NULL);

	    sqlite3_bind_text(stmt, 15, adminCode.c_str(), adminCode.length(), NULL);

	    sqlite3_bind_text(stmt, 16, level.c_str(), level.length(), NULL);

	    sqlite3_bind_int(stmt, 17, open24H);

	    sqlite3_bind_blob(stmt, 18, relateParent.c_str(), relateParent.length(), NULL);

	    sqlite3_bind_blob(stmt, 19, relateChildren.c_str(), relateChildren.length(), NULL);

	    sqlite3_bind_blob(stmt, 20, names.c_str(), names.length(), NULL);

	    sqlite3_bind_blob(stmt, 21, addresses.c_str(), addresses.length(), NULL);

	    sqlite3_bind_blob(stmt, 22, contacts.c_str(), contacts.length(), NULL);

	    sqlite3_bind_blob(stmt, 23, foodtypes.c_str(), foodtypes.length(), NULL);

	    sqlite3_bind_blob(stmt, 24, parkings.c_str(), parkings.length(), NULL);

	    sqlite3_bind_blob(stmt, 25, hotel.c_str(), hotel.length(), NULL);

	    sqlite3_bind_blob(stmt, 26, sportsVenues.c_str(), sportsVenues.length(), NULL);

	    sqlite3_bind_blob(stmt, 27, chargingStation.c_str(), chargingStation.length(), NULL);

	    sqlite3_bind_blob(stmt, 28, chargingPole.c_str(), chargingPole.length(), NULL);

	    sqlite3_bind_blob(stmt, 29, gasStation.c_str(), gasStation.length(), NULL);

	    sqlite3_bind_blob(stmt, 30, indoor.c_str(), indoor.length(), NULL);

	    sqlite3_bind_blob(stmt, 31, attachments.c_str(), attachments.length(), NULL);

	    sqlite3_bind_blob(stmt, 32, brands.c_str(), brands.length(), NULL);

	    sqlite3_bind_text(stmt, 33, rawFields.c_str(), rawFields.length(), NULL);

	    sqlite3_bind_int(stmt, 34, submitStatus);

	    sqlite3_bind_text(stmt, 35, sourceCollection.c_str(), sourceCollection.length(), NULL);

	    sqlite3_bind_int(stmt, 36, lifecycle);

	    sqlite3_bind_blob(stmt, 37, edits.c_str(), edits.length(), NULL);

	    sqlite3_bind_text(stmt, 38, simple_parentFid.c_str(), simple_parentFid.length(), NULL);

	    sqlite3_bind_int(stmt, 39, auditStatus);

	    sqlite3_bind_int(stmt, 40, submitStatus_desktop);

	    sqlite3_bind_text(stmt, 41, latestMergeDate.c_str(), latestMergeDate.length(), NULL);

	    EditorGeometry::WkbPoint* wkb=new EditorGeometry::WkbPoint();
	    wkb->_byteOrder = 1;
		wkb->_wkbType = (EditorGeometry::WkbGeometryType)1;
        wkb->_point._x = longitude;
	    wkb->_point._y = latitude;

	    EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

	    sqlite3_bind_blob(stmt, 42, spGeo, wkb->buffer_size()+39, NULL);

        sqlite3_bind_int(stmt, 43, task_status);
        sqlite3_bind_text(stmt, 44, vipFlag.c_str(), vipFlag.length(), NULL);
        sqlite3_bind_int(stmt, 45, type);
        sqlite3_bind_text(stmt, 46, t_operateDate.c_str(), t_operateDate.length(), NULL);
        sqlite3_bind_text(stmt, 47, info_globalId.c_str(), info_globalId.length(), NULL);
	    delete wkb;

        sqlite3_bind_text(stmt, 48, globalId.c_str(), globalId.length(), NULL);
        //sqlite3_bind_text(stmt, 49, srcInformation.c_str(), srcInformation.length(), NULL);

	    if (sqlite3_step(stmt) != SQLITE_DONE)
	    {
		    Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
		    Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
			
			free(spGeo);

		    sqlite3_finalize(stmt);

            return false;
	    }

	    free(spGeo);

	    spGeo = NULL;

	    sqlite3_finalize(stmt);

        return 0;
    }

    int Point::GetMaxRowId(sqlite3* db)
    {
        sqlite3_stmt *statement; 

	    std::string sql = "SELECT max(ROWSETROWID) FROM Point";

	    int max = -1;

	    if ( sqlite3_prepare(db, sql.c_str(),strlen(sql.c_str()), &statement, NULL ) == SQLITE_OK )
        {
            int res = sqlite3_step(statement);

		    if(res == SQLITE_ROW)
		    {
			    max = sqlite3_column_int(statement, 0);
		    }
       
		    sqlite3_finalize (statement);
        }
	    else
	    {
		    max=-1;
	    }

	    if(max == -1)
	    {
		    Logger::LogD("Point max RowSetRowId is -1");
            Logger::LogO("Point max RowSetRowId is -1");
	    }

	    return max;
    }
    
    int Point::GetRowId(sqlite3* db, std::string key, std::string value)
    {
        sqlite3_stmt *statement; 

	    std::string sql = "SELECT RowsetRowId FROM Point where " + key + "='" + value + "'";

	    int rowId = -1;

	    if ( sqlite3_prepare(db, sql.c_str(),strlen(sql.c_str()), &statement, NULL ) == SQLITE_OK )
        {
            int res = sqlite3_step(statement);

		    if(res == SQLITE_ROW)
		    {
			    rowId = sqlite3_column_int(statement, 0);
		    }
       
		    sqlite3_finalize (statement);
        }
	    else
	    {
		    rowId=-1;
	    }

	    if(rowId == -1)
	    {
		    Logger::LogD("Point max RowSetRowId is -1");
            Logger::LogO("Point max RowSetRowId is -1");
	    }

	    return rowId;
    }

    void Point::SetDisplayInfo(int basicType)
    {
        if(basicType == 0)//poi displaystyle
        {
            int nStatus = GetWorkStatus();
            
            if(nStatus == -1)
            {
                Logger::LogD("SetDisplayInfo failed");
                Logger::LogO("SetDisplayInfo failed");
                
                return;
            }
            
            if(nStatus == 0)
            {
                display_style = "0,0";
            }
            
            if(type == 1 || type == 21) //删除标记和大门
                display_style = Tools::NumberToString(type);
            else
            {
                std::stringstream sstream;
                
                sstream<<nStatus;
                
                sstream<<",";
                
                int nImportance = GetImportance();
                
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
                
                display_style = sstream.str();
            }
        }
        if(basicType == 6)   //充电桩/充电桩displaystyle
        {
            if(lifecycle == 0 )
                display_style = kindCode + "_"+ "0";
            else
                display_style = kindCode + "_"+ "1";
        }
    }



    int Point::GetWorkStatus() const
    {
        if((lifecycle == 0 && qtStatus == 0) || qtStatus == 1) //待作业
        {
            return 1;    
        }
        else if((lifecycle != 0 && qtStatus == 0) || qtStatus == 2) // 已作业
        {
            if(lifecycle == 1)
            {
                return 3;
            }
            else if(lifecycle == 2)
            {
                return 2;
            }
            else if(lifecycle == 3)
            {
                return 4;
            }            
        }
        else
        {
            return 0;
        }

        return -1;
    }

    int Point::GetImportance() const
    {
        if(evaluatePlanning == 1)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    std::string Point::SetPrimaryKey()
    {
        if(type == 0)
        {
            return fid + project;
        }
        else
        {
            return Tools::NumberToString(pid) + Tools::NumberToString(type);
        }
    }
    
    std::string Point::GetKindCodeToDisplayInfo() const
    {
        int namelength = name.length();

		std::string endNameStr = "";

		if (namelength > 9)
		{
			endNameStr = name.substr(namelength-9, 9);
		}

		if(kindCode == "150101")
		{
#ifdef WIN32
			if (endNameStr == "\uEFBCA1\uEFBCB4\uEFBCAD")
#else
			if (endNameStr == "ＡＴＭ")
#endif
			{
				return "150101+ATM";
			}
			else
			{
				return "150101";
			}
		}

        return kindCode;
    }

    ProjectUser::ProjectUser()
    {
        sProjectId = "";
        sUserId="";
        nType = -1;
        download_latest="";
        download_url="";
        upload_latest="";
        upload_url="";
        zip_file="";
        failed_reason="";
        details="";
        download_type = -1;
        upload_type = -1;
    }

    int ProjectUser::InsertSqliteTable(std::string sDBPath)
    {
        sqlite3* db = NULL;

        int ret = sqlite3_open_v2(sDBPath.c_str(), &db, SQLITE_OPEN_READWRITE, NULL);

        if(ret != SQLITE_OK)
        {
            Logger::LogD("DataSource::sqlite [%s] or [%s] open failed", sDBPath.c_str(), sqlite3_errmsg(db));
            Logger::LogO("DataSource::sqlite [%s] or [%s] open failed", sDBPath.c_str(), sqlite3_errmsg(db));

            sqlite3_close(db);
            return -1;
        }

        char *err_msg = NULL;

	    ret = sqlite3_exec (db, "BEGIN", NULL, NULL, &err_msg);

        if (ret != SQLITE_OK)
        {
		    Logger::LogD("InsertSqliteTable BEGIN error: %s\n", err_msg);
            Logger::LogO("InsertSqliteTable BEGIN error: %s\n", err_msg);

            sqlite3_free(err_msg);
            return false;
        }

	    sqlite3_stmt* stmt;

	    std::string sql = "insert into project_user values("
		    "?, ?, ?, ?, ?, ?, ?, ?)";

	    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("insert %s failed [%s]", "Point", sqlite3_errmsg(db));
            Logger::LogO("insert %s failed [%s]", "Point", sqlite3_errmsg(db));

		    return false;
	    }

        sqlite3_bind_int(stmt, 1, Tools::StringToNum(sProjectId));

	    sqlite3_bind_int(stmt, 2, Tools::StringToNum(sUserId));

	    sqlite3_bind_int(stmt, 3, nType);

	    sqlite3_bind_text(stmt, 4, download_latest.c_str(), download_latest.length(), NULL);

	    sqlite3_bind_text(stmt, 5, download_url.c_str(), download_url.length(), NULL);

        sqlite3_bind_text(stmt, 6, upload_latest.c_str(), upload_latest.length(), NULL);

	    sqlite3_bind_text(stmt, 7, upload_url.c_str(), upload_url.length(), NULL);

        sqlite3_bind_text(stmt, 8, details.c_str(), details.length(), NULL);

	    if (sqlite3_step(stmt) != SQLITE_DONE)
	    {			
		    sqlite3_finalize(stmt);

		    Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
            Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));

            return false;
	    }

	    sqlite3_finalize(stmt);

        ret = sqlite3_exec(db, "COMMIT", NULL, NULL, &err_msg);

        if (ret != SQLITE_OK)
        {
            Logger::LogD ("COMMIT error: %s\n", err_msg);
            Logger::LogO ("COMMIT error: %s\n", err_msg);

            sqlite3_free (err_msg);
		
		    return false;
        }

        sqlite3_close(db);

        return 0;
    }

    int ProjectUser::InsertOrUpdateSqliteTable(std::string sDBPath)
    {
        sqlite3* db = NULL;

        int ret = sqlite3_open_v2(sDBPath.c_str(), &db, SQLITE_OPEN_READWRITE, NULL);

        if(ret != SQLITE_OK)
        {
            Logger::LogD("DataSource::sqlite [%s] or [%s] open failed", sDBPath.c_str(), sqlite3_errmsg(db));
            Logger::LogO("DataSource::sqlite [%s] or [%s] open failed", sDBPath.c_str(), sqlite3_errmsg(db));

            sqlite3_close(db);
            return -1;
        }

        char *err_msg = NULL;

	    ret = sqlite3_exec (db, "BEGIN", NULL, NULL, &err_msg);

        if (ret != SQLITE_OK)
        {
		    Logger::LogD("InsertSqliteTable BEGIN error: %s\n", err_msg);
            Logger::LogO("InsertSqliteTable BEGIN error: %s\n", err_msg);

            sqlite3_free(err_msg);
            sqlite3_close(db);
            return false;
        }

	    sqlite3_stmt* stmt;

	    std::string sql = "REPLACE INTO project_user("
            "project_id, user_id, type, download_latest,download_url, upload_latest, upload_url, zip_file, failed_reason, details) "
		    "values(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

	    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("insert %s failed [%s]", "Point", sqlite3_errmsg(db));
            Logger::LogO("insert %s failed [%s]", "Point", sqlite3_errmsg(db));
            sqlite3_close(db);
		    return false;
	    }

        sqlite3_bind_int(stmt, 1, Tools::StringToNum(sProjectId));

	    sqlite3_bind_int(stmt, 2, Tools::StringToNum(sUserId));

	    sqlite3_bind_int(stmt, 3, nType);

	    sqlite3_bind_text(stmt, 4, download_latest.c_str(), download_latest.length(), NULL);

	    sqlite3_bind_text(stmt, 5, download_url.c_str(), download_url.length(), NULL);

        sqlite3_bind_text(stmt, 6, upload_latest.c_str(), upload_latest.length(), NULL);

	    sqlite3_bind_text(stmt, 7, upload_url.c_str(), upload_url.length(), NULL);

	    sqlite3_bind_text(stmt, 8, zip_file.c_str(), zip_file.length(), NULL);

	    sqlite3_bind_text(stmt, 9, failed_reason.c_str(), failed_reason.length(), NULL);

        sqlite3_bind_text(stmt, 10, details.c_str(), details.length(), NULL);

	    if (sqlite3_step(stmt) != SQLITE_DONE)
	    {			
		    sqlite3_finalize(stmt);

		    Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
            Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
            sqlite3_close(db);
            return false;
	    }

	    sqlite3_finalize(stmt);

        ret = sqlite3_exec(db, "COMMIT", NULL, NULL, &err_msg);

        if (ret != SQLITE_OK)
        {
            Logger::LogD ("COMMIT error: %s\n", err_msg);
            Logger::LogO ("COMMIT error: %s\n", err_msg);

            sqlite3_free (err_msg);
		    sqlite3_close(db);
		    return false;
        }

        sqlite3_close(db);

        return 0;
    }

    int ProjectUser::ParseSqlite(sqlite3_stmt *statement)
    {
        sProjectId = (const char*)sqlite3_column_text(statement, 0);
	    sUserId = (const char*)sqlite3_column_text(statement, 1);
        ProjectUser::nType = sqlite3_column_int(statement, 2);
	    download_latest = (const char*)sqlite3_column_text(statement, 3);
	    download_url = (const char*)sqlite3_column_text(statement, 4);
	    upload_latest = (const char*)sqlite3_column_text(statement, 5);
	    upload_url = (const char*)sqlite3_column_text(statement, 6);
        details = (const char*)sqlite3_column_text(statement, 7);

        return 0;
    }

    int Tips::ParseJson(std::string sJson)
    {
        rapidjson::Document doc;

        doc.Parse<0>(sJson.c_str());

        if(doc.HasParseError())
        {
            return -1;
        }
       
        for(rapidjson::Document::MemberIterator ptr = doc.MemberBegin(); ptr != doc.MemberEnd(); ++ptr)
        {
            std::string  sKey = (ptr->name).GetString();
            
            rapidjson::Value &valueKey = (ptr->value);

            std::string sValue = "";

            if((ptr->value).IsNull())
            {
                continue;
            }

            if(!(ptr->value).IsString() || !(ptr->value).IsNumber())
            {
                rapidjson::StringBuffer buffer;

                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

                valueKey.Accept(writer); 

                sValue = buffer.GetString();
            }

	        if(sKey == "rowkey")
            {
                rowkey = (ptr->value).GetString();;
            }
            else if(sKey == "s_featureKind")
            {
                s_featureKind = (ptr->value).GetInt();
            }
            else if(sKey == "s_project")
            {
                s_project = (ptr->value).GetString();;
            }
            else if(sKey == "s_sourceCode")
            {
                s_sourceCode = (ptr->value).GetInt();
            }
            else if(sKey == "s_sourceId")
            {
                s_sourceId = (ptr->value).GetString();
            }
            else if(sKey == "s_sourceType")
            {
                s_sourceType = (ptr->value).GetString();
            }
            else if(sKey == "s_sourceProvider")
            {
                s_sourceProvider = (ptr->value).GetInt();
            }
            else if(sKey == "s_reliability")
            {
                s_reliability = (ptr->value).GetInt();
            }
            else if(sKey == "g_guide")
            {
				//后台安装格式不是wkt，是geoJson，统一转为wkt
				if(m_dataSource==Tips::DataSourceFromServer)
				{
					sValue= DataTransfor::GeoJson2Wkt(sValue, Tips::DataSourceFromServer);
				}

				//app 层是wkt
				g_guide = sValue;
				if(sValue.size() >= 2 && (sValue[0] == '\"' && sValue[sValue.length() - 1] == '\"'))
				{
					g_guide = sValue.substr(1, sValue.size() - 2);
				}
				
            }
            else if(sKey == "t_lifecycle")
            {
                t_lifecycle = (ptr->value).GetInt();
            }
            else if(sKey == "t_status")
            {
                t_status = (ptr->value).GetInt();
            }
            else if(sKey == "t_operateDate")
            {
                t_operateDate = (ptr->value).GetString();
            }
            else if(sKey == "t_handler")
            {
                t_handler = (ptr->value).GetInt();
            }
            else if(sKey == "deep")
            {
                deep = sValue;
            }
            else if(sKey == "attachments")
            {
                attachments = sValue;
            }
            else if(sKey == "g_location")
            {
				//后台安装格式不是wkt，是geoJson，统一转为wkt
				if(m_dataSource==Tips::DataSourceFromServer)
				{
					sValue= DataTransfor::GeoJson2Wkt(sValue, Tips::DataSourceFromServer);
				}

				//app 层是wkt
				g_location = sValue;
				if(sValue.size() >= 2 && (sValue[0] == '\"' && sValue[sValue.length() - 1] == '\"'))
				{
					g_location = sValue.substr(1, sValue.size() - 2);
				}
            }
            else if (sKey == "angle")
            {
                angle = (ptr->value).GetDouble();
            }
            else if (sKey == "t_command")
            {
                t_command = (ptr->value).GetInt();
            }
			else if (sKey == "t_sync")
			{
				t_sync = (ptr->value).GetInt();
			}
        }

        if(strcmp(attachments.c_str(),"") == 0)
        {
            attachments = "[]";
        }

        return 0;
    }

    int Tips::UpdateSqliteTable(sqlite3* db)
    {
	    sqlite3_stmt* stmt;

	    std::string sql = "update edit_tips set "
		    "rowkey=?, s_featureKind=?, s_project=?, s_sourceCode=?, s_sourceId=?, s_sourceType=?, s_sourceProvider=?, s_reliability=?, g_location=?, "
		    "g_guide=?, t_lifecycle=?, t_status=?, t_operateDate=?, t_handler=?, deep=?, display_style=?, attachments=?, angle=?, t_command=?, t_sync=? where rowkey=?";

	    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("update %s failed [%s]", "Tips", sqlite3_errmsg(db));
            Logger::LogO("update %s failed [%s]", "Tips", sqlite3_errmsg(db));

		    return false;
	    }

	    sqlite3_bind_text(stmt, 1, rowkey.c_str(), rowkey.length(), NULL);

	    sqlite3_bind_int(stmt, 2, s_featureKind);

	    sqlite3_bind_text(stmt, 3, s_project.c_str(), s_project.length(), NULL);

	    sqlite3_bind_int(stmt, 4, s_sourceCode);

        sqlite3_bind_text(stmt, 5, s_sourceId.c_str(), s_sourceId.length(), NULL);

        sqlite3_bind_text(stmt, 6, s_sourceType.c_str(), s_sourceType.length(), NULL);

        sqlite3_bind_int(stmt, 7, s_sourceProvider);

        sqlite3_bind_int(stmt, 8, s_reliability);

       /* EditorGeometry::WkbPoint* wkb = new EditorGeometry::WkbPoint();
	    wkb->_byteOrder = 1;
		wkb->_wkbType = (EditorGeometry::WkbGeometryType)1;
        wkb->_point._x = longitude;
	    wkb->_point._y = latitude;

	    EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

	    sqlite3_bind_blob(stmt, 9, spGeo, wkb->buffer_size()+39, NULL);

	    delete wkb;*/

		EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(g_location);

		EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

		sqlite3_bind_blob(stmt, 9, spGeo, wkb->buffer_size()+39, NULL);

		delete[] wkb;

	    sqlite3_bind_text(stmt, 10, g_guide.c_str(), g_guide.length(), NULL);

        sqlite3_bind_int(stmt, 11, t_lifecycle);

        sqlite3_bind_int(stmt, 12, t_status);

        sqlite3_bind_text(stmt, 13, t_operateDate.c_str(), t_operateDate.length(), NULL);

        sqlite3_bind_int(stmt, 14, t_handler);

        sqlite3_bind_blob(stmt, 15, deep.c_str(), deep.length(), NULL);

        sqlite3_bind_text(stmt, 16, display_style.c_str(), display_style.length(), NULL);

        sqlite3_bind_blob(stmt, 17, attachments.c_str(), attachments.length(), NULL);

        sqlite3_bind_double(stmt, 18, angle);

        sqlite3_bind_int(stmt, 19, t_command);

		sqlite3_bind_int(stmt, 20, t_sync);

        sqlite3_bind_text(stmt, 21, rowkey.c_str(), rowkey.length(), NULL);

	    if (sqlite3_step(stmt) != SQLITE_DONE)
	    {
		    Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
		    Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
			
			free(spGeo);

		    sqlite3_finalize(stmt);

            return false;
	    }

	    free(spGeo);

	    spGeo = NULL;

	    sqlite3_finalize(stmt);

        return true;
    }

    int Tips::InsertSqliteTable(sqlite3* db)
    {
	    sqlite3_stmt* stmt;

        std::string sql = "insert into edit_tips values("
		    "?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
		    "?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
		
		int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("insert %s failed [%s]", "Tips", sqlite3_errmsg(db));
            Logger::LogO("insert %s failed [%s]", "Tips", sqlite3_errmsg(db));

		    return false;
	    }

	    sqlite3_bind_text(stmt, 1, rowkey.c_str(), rowkey.length(), NULL);

	    sqlite3_bind_int(stmt, 2, s_featureKind);

	    sqlite3_bind_text(stmt, 3, s_project.c_str(), s_project.length(), NULL);

	    sqlite3_bind_int(stmt, 4, s_sourceCode);

        sqlite3_bind_text(stmt, 5, s_sourceId.c_str(), s_sourceId.length(), NULL);

        sqlite3_bind_text(stmt, 6, s_sourceType.c_str(), s_sourceType.length(), NULL);

        sqlite3_bind_int(stmt, 7, s_sourceProvider);

        sqlite3_bind_int(stmt, 8, s_reliability);

		EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(g_location);

		EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

		sqlite3_bind_blob(stmt, 9, spGeo, wkb->buffer_size()+39, NULL);

		delete[] wkb;
		
		sqlite3_bind_text(stmt, 10, g_guide.c_str(), g_guide.length(), NULL);

        sqlite3_bind_int(stmt, 11, t_lifecycle);

        sqlite3_bind_int(stmt, 12, t_status);

        sqlite3_bind_text(stmt, 13, t_operateDate.c_str(), t_operateDate.length(), NULL);

        sqlite3_bind_int(stmt, 14, t_handler);

        sqlite3_bind_blob(stmt, 15, deep.c_str(), deep.length(), NULL);

        sqlite3_bind_text(stmt, 16, display_style.c_str(), display_style.length(), NULL);

        sqlite3_bind_blob(stmt, 17, attachments.c_str(), attachments.length(), NULL);

        sqlite3_bind_double(stmt, 18, angle);

        sqlite3_bind_int(stmt, 19, t_command);

		sqlite3_bind_int(stmt, 20, t_sync);

	    if (sqlite3_step(stmt) != SQLITE_DONE)
	    {
		    Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
		    Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
			
			free(spGeo);

		    sqlite3_finalize(stmt);

            return false;
	    }

	    free(spGeo);

	    spGeo = NULL;

	    sqlite3_finalize(stmt);

        return true;
    }

    void Tips::SetDisplayStyle(const std::string& json)
    {
        std::string display_string = DataTransfor::GetTipsDisplayStyle(json);

        display_style = display_string;
    }

	tips_geo* Tips::CreateGeo( tips_geo::DisplayGeoType geotype )
	{
		tips_geo* newGeo=NULL;
		switch (geotype)
		{
		case tips_geo::displayGeoPoint:
			newGeo = new tips_point();
			break;
		case tips_geo::displayGeoLine:
			newGeo = new tips_line();
			break;
		case tips_geo::displayGeoPolygon:
			newGeo = new tips_polygon();
			break;
		case tips_geo::displayGeoUnKnown:
			break;
		}
		return newGeo;
	}

	void Tips::Attach(tips_geo*pObs)
	{
		if(!pObs)return;
		m_setObs.insert(pObs);
	}
	void Tips::Detach(tips_geo*pObs)
	{
		if(!pObs)return;
		m_setObs.erase(pObs);
	}
	void Tips::DetachAll()
	{
		m_setObs.clear();
	}

	void Tips::Notify( sqlite3* db, int operateType )
	{
		std::set<tips_geo*>::iterator itr=m_setObs.begin();
		for(;itr!=m_setObs.end();itr++)
		{
			(*itr)->Update(*this, db, operateType);
		}
	}

	void Tips::Recycle()
	{
		std::set<tips_geo*>::iterator itor = m_setObs.begin();
		while (itor!=m_setObs.end())
		{
			tips_geo* pTipsGeo = *itor;
			if (pTipsGeo)
			{
				delete pTipsGeo;
				pTipsGeo = NULL;
			}
			itor++;
		}
		m_setObs.clear();
	}

	Tips::~Tips()
	{
		Recycle();
	}

	

	std::vector<tips_geo*> Tips::BuildGeosBySourceType()
	{
		std::vector<tips_geo*> result;
		int tipsType = atoi(s_sourceType.c_str());

		switch (tipsType)
		{
		case Model::PointSpeedLimit:
		case Model::RoadKind:
		case Model::OrdinaryTurnRestriction:
		case Model::CrossName:
		case Model::LaneConnexity:
		case Model::RoadDirect:
        case Model::HighwayBranch:
			{
				tips_point* norlmaPoint = dynamic_cast<tips_point*>(CreateGeo(tips_geo::displayGeoPoint));
				if (norlmaPoint)
				{
					norlmaPoint->setDisplayPointType(tips_point::eDisplayPointType_normalPoint);
					result.push_back(norlmaPoint);
				}
				break;
			}
		case Model::RoadName:
		case Model::SurveyLine:
			{
				tips_point* norlmaPoint = dynamic_cast<tips_point*>(CreateGeo(tips_geo::displayGeoPoint));
				if (norlmaPoint)
				{
					norlmaPoint->setDisplayPointType(tips_point::eDisplayPointType_normalPoint);
					result.push_back(norlmaPoint);
				}
				tips_geo* line = CreateGeo(tips_geo::displayGeoLine);
				if (line)
				{
					result.push_back(line);
				}
				break;
			}
		case Model::Bridge:
			{
				tips_point* startPoint = dynamic_cast<tips_point*>(CreateGeo(tips_geo::displayGeoPoint));
				if (startPoint)
				{
					startPoint->setDisplayPointType(tips_point::eDisplayPointType_startPoint);
					result.push_back(startPoint);
				}

				tips_point* endPoint = dynamic_cast<tips_point*>(CreateGeo(tips_geo::displayGeoPoint));
				if (endPoint)
				{
					endPoint->setDisplayPointType(tips_point::eDisplayPointType_endPoint);
					result.push_back(endPoint);
				}

				tips_geo* line = CreateGeo(tips_geo::displayGeoLine);
				if (line)
				{
					result.push_back(line);
				}
				break;
			}
        case Model::Hitching:
            {
                if (deep!="")
                {
                    Document deepDoc;
                    deepDoc.Parse<0>(deep.c_str());
                    if(deepDoc.HasParseError() || !deepDoc.HasMember(Tips_Column_Geo.c_str()) || deepDoc[Tips_Column_Geo.c_str()].IsNull())
                    {
                        return result;
                    }
                    
                    if (!deepDoc[Tips_Column_Geo.c_str()].IsArray())
                    {
                        return result;
                    }
                    
                    rapidjson::Value& arrayValue = deepDoc[Tips_Column_Geo.c_str()];
                    if (!arrayValue.IsArray())
                    {
                        return result;
                    }
                    
                    for(int index = 0; index<arrayValue.Capacity(); index++)
                    {
                        rapidjson::Value& geovalue = arrayValue[index];
                        
                        std::string sValue = "";
                        
                        if(geovalue.IsObject())
                        {
                            rapidjson::StringBuffer buffer;
                            
                            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                            
                            geovalue.Accept(writer);
                            
                            sValue = buffer.GetString();
                        }
                        else if(geovalue.IsString())
                        {
                            sValue = geovalue.GetString();
                        }
                        
                        if (sValue=="")
                        {
                            continue;
                        }
                        
                        std::string valueWkt = DataTransfor::GeoJson2Wkt(sValue, Model::Tips::DataSourceFromServer);
                        
                        EditorGeometry::WkbGeometry *pGeo = DataTransfor::Wkt2Wkb(valueWkt);
                        
                        if (pGeo!=NULL)
                        {
                            if(pGeo->_wkbType==EditorGeometry::wkbPoint)
                            {
                                tips_point* norlmaPoint = dynamic_cast<tips_point*>(CreateGeo(tips_geo::displayGeoPoint));
                                if (norlmaPoint)
                                {
                                    norlmaPoint->setDisplayPointType(tips_point::eDisplayPointType_normalPoint);
                                    norlmaPoint->geometry = valueWkt;
                                    result.push_back(norlmaPoint);
                                }
                                
                            }
                            else if(pGeo->_wkbType==EditorGeometry::wkbLineString)
                            {
                                tips_geo* line = CreateGeo(tips_geo::displayGeoLine);
                                if (line)
                                {
                                    line->geometry = valueWkt;
                                    result.push_back(line);
                                }
                            }
                            else if(pGeo->_wkbType==EditorGeometry::wkbPolygon)
                            {
                                tips_polygon* polygon = dynamic_cast<tips_polygon*>(CreateGeo(tips_geo::displayGeoPolygon));
                                if (polygon)
                                {
                                    polygon->geometry = valueWkt;
                                    result.push_back(polygon);
                                }
                            }
                            
                            delete [] (char*)pGeo;
                            pGeo = NULL;
                        }
                        
                    }
                }

               break;
            }
            case Model::RegionalRoad:
            {
                
                tips_point* iconPoint = dynamic_cast<tips_point*>(CreateGeo(tips_geo::displayGeoPoint));
                if (iconPoint)
                {
                    iconPoint->setDisplayPointType(tips_point::eDisplayPointType_normalPoint);
                    result.push_back(iconPoint);
                }
                tips_geo* polygon = CreateGeo(tips_geo::displayGeoPolygon);
                if (polygon)
                {
                    result.push_back(polygon);
                }
                break;
            }
		default:
			break;
		}
		return result;
	}

	char * Tips::_strlwr( char *str )
	{
		char *p = str;
		while (*p != '\0')
		{
			if(*p >= 'A' && *p <= 'Z')
				*p = (*p) + 0x20;
			p++;
		}
		return str;
	}

	void Tips::setDataSource(int dataSource)
	{
		m_dataSource = dataSource;
	}


    
    int ParaKindTop::ParseSqlite(sqlite3_stmt *statement)
    {
        id = sqlite3_column_int(statement, 0);
        name = (const char*)sqlite3_column_text(statement, 1);
        code = sqlite3_column_int(statement, 2);

        return 0;
    }

    std::string ParaKindTop::ToJson()
    {
        Document document;
        Document docTmp;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();

        document.AddMember("id",id,allocator);

        Value each_json_value(kStringType);
        each_json_value.SetString(name.c_str(), name.size(),allocator);  
        document.AddMember("name",each_json_value,allocator);

        document.AddMember("code",code,allocator);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();

        return sResult;
    }

    int ParaKindMedium::ParseSqlite(sqlite3_stmt *statement)
    {
        id = sqlite3_column_int(statement, 0);
        top_id = sqlite3_column_int(statement, 1);
        name = (const char*)sqlite3_column_text(statement, 2);
        code = sqlite3_column_int(statement, 3);
        const unsigned char* tmp = sqlite3_column_text(statement, 4);

        if(tmp != NULL)
        {
            description = (const char*)sqlite3_column_text(statement, 4);
        }

        return 0;
    }

    std::string ParaKindMedium::ToJson()
    {
        Document document;
        Document docTmp;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();

        document.AddMember("id",id,allocator);

        document.AddMember("top_id",top_id,allocator);

        Value each_json_value(kStringType);
        each_json_value.SetString(name.c_str(), name.size(),allocator);  
        document.AddMember("name",each_json_value,allocator);

        document.AddMember("code",code,allocator);

        each_json_value.SetString(description.c_str(), description.size(),allocator);  
        document.AddMember("description",each_json_value,allocator);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();

        return sResult;
    }

    int ParaKind::ParseSqlite(sqlite3_stmt *statement)
    {
        id = sqlite3_column_int(statement, 0);
        mediun_id = sqlite3_column_int(statement, 1);
        name = (const char*)sqlite3_column_text(statement, 2);
        code = sqlite3_column_int(statement, 3);
        kind_code = (const char*)sqlite3_column_text(statement, 4);
        region = sqlite3_column_int(statement, 5);
        type = sqlite3_column_int(statement, 6);
        const unsigned char* tmp = sqlite3_column_text(statement, 7);

        if(tmp != NULL)
        {
            description = (const char*)sqlite3_column_text(statement, 7);
        }
        full_name = (const char*)sqlite3_column_text(statement, 8);

        return 0;
    }

    std::string ParaKind::ToJson()
    {
        Document document;
        Document docTmp;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();

        document.AddMember("id",id,allocator);

        document.AddMember("medium_id",mediun_id,allocator);

        Value each_json_value(kStringType);
        each_json_value.SetString(name.c_str(), name.size(),allocator);  
        document.AddMember("name",each_json_value,allocator);

        document.AddMember("code",code,allocator);

        each_json_value.SetString(kind_code.c_str(), kind_code.size(),allocator);  
        document.AddMember("kind_code",each_json_value,allocator);

        document.AddMember("region",region,allocator);

        document.AddMember("type",type,allocator);

        each_json_value.SetString(description.c_str(), description.size(),allocator);  
        document.AddMember("description",each_json_value,allocator);

        each_json_value.SetString(full_name.c_str(), full_name.size(),allocator);  
        document.AddMember("full_name",each_json_value,allocator);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();

        return sResult;
    }

    int ChargingCarid::ParseSqlite(sqlite3_stmt *statement)
    {
        id = sqlite3_column_int(statement, 0);
        chain_name = (const char*)sqlite3_column_text(statement, 1);
        chain_code = (const char*)sqlite3_column_text(statement, 2);

        return 0;
    }

    std::string ChargingCarid::ToJson()
    {
        Document document;
        Document docTmp;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();

        document.AddMember("id",id,allocator);

        Value each_json_value(kStringType);
        each_json_value.SetString(chain_name.c_str(), chain_name.size(),allocator);  
        document.AddMember("chain_name",each_json_value,allocator);

        each_json_value.SetString(chain_code.c_str(), chain_code.size(),allocator);  
        document.AddMember("chain_code",each_json_value,allocator);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();

        return sResult;
    }

    int ParaChain::ParseSqlite(sqlite3_stmt *statement)
    {
        id = sqlite3_column_int(statement, 0);
        name = (const char*)sqlite3_column_text(statement, 1);
        code = (const char*)sqlite3_column_text(statement, 2);
        region = (const char*)sqlite3_column_text(statement, 3);
        type = sqlite3_column_int(statement, 4);
        category = sqlite3_column_int(statement, 5);
        weight = sqlite3_column_int(statement, 6);
        return 0;
    }

    std::string ParaChain::ToJson()
    {
        Document document;
        Document docTmp;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();

        document.AddMember("id",id,allocator);

        Value each_json_value(kStringType);
        each_json_value.SetString(name.c_str(), name.size(),allocator);  
        document.AddMember("name",each_json_value,allocator);

        each_json_value.SetString(code.c_str(), code.size(),allocator);  
        document.AddMember("code",each_json_value,allocator);

        each_json_value.SetString(region.c_str(), region.size(),allocator);  
        document.AddMember("region",each_json_value,allocator);

        document.AddMember("type",type,allocator);

        document.AddMember("category",category,allocator);

        document.AddMember("weight",weight,allocator);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();

        return sResult;
    }

    int ParaControl::ParseSqlite(sqlite3_stmt *statement)
    {
        id = sqlite3_column_int(statement, 0);
	    kind_id = sqlite3_column_int(statement, 1);
	    kind_code = (const char*)sqlite3_column_text(statement, 2);
	    kind_change = sqlite3_column_int(statement, 3);
	    parent = sqlite3_column_int(statement, 4);
	    important = sqlite3_column_int(statement, 5);
	    const unsigned char* tmp = sqlite3_column_text(statement, 6);
		if(tmp != NULL)
		{
			name_keyword = (const char*)sqlite3_column_text(statement, 6);
		}
		tmp = sqlite3_column_text(statement, 7);
		if(tmp != NULL)
		{
			level = (const char*)sqlite3_column_text(statement, 7);
		}
		tmp = sqlite3_column_text(statement, 8);
		if(tmp != NULL)
		{
			eng_permit = (const char*)sqlite3_column_text(statement, 8);
		}
	    agent = sqlite3_column_int(statement, 9);
	    region = sqlite3_column_int(statement, 10);
	    tenant = sqlite3_column_int(statement, 11);
        extend = sqlite3_column_int(statement, 12);
	    extend_photo = sqlite3_column_int(statement, 13);
	    photo = sqlite3_column_int(statement, 14);
	    interna = sqlite3_column_int(statement, 15);
	    chain = sqlite3_column_int(statement, 16);
	    rel_cs = sqlite3_column_int(statement, 17);
	    add_cs = sqlite3_column_int(statement, 18);
	    tmp = sqlite3_column_text(statement, 19);
		if(tmp != NULL)
		{
			mhm_des = (const char*)sqlite3_column_text(statement, 19);
		}
        disp_onlink = sqlite3_column_int(statement, 20);

        return 0;
    }

    std::string ParaControl::ToJson()
    {
        Document document;
        Document docTmp;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();

        document.AddMember("id",id,allocator);

        document.AddMember("kind_id",kind_id,allocator);

        Value each_json_value(kStringType);
        each_json_value.SetString(kind_code.c_str(), kind_code.size(),allocator);  
        document.AddMember("kind_code",each_json_value,allocator);

        document.AddMember("kind_change",kind_change,allocator);

        document.AddMember("parent",parent,allocator);

        document.AddMember("important",important,allocator);

        each_json_value.SetString(name_keyword.c_str(), name_keyword.size(),allocator);  
        document.AddMember("name_keyword",each_json_value,allocator);

        each_json_value.SetString(level.c_str(), level.size(),allocator);  
        document.AddMember("level",each_json_value,allocator);

        each_json_value.SetString(eng_permit.c_str(), eng_permit.size(),allocator);  
        document.AddMember("eng_permit",each_json_value,allocator);

        document.AddMember("agent",agent,allocator);

        document.AddMember("region",region,allocator);

        document.AddMember("tenant",tenant,allocator);

        document.AddMember("extend",extend,allocator);

        document.AddMember("extend_photo",extend_photo,allocator);

        document.AddMember("photo",photo,allocator);

        document.AddMember("internal",interna,allocator);

        document.AddMember("chain",chain,allocator);

        document.AddMember("rel_cs",rel_cs,allocator);

        document.AddMember("add_cs",add_cs,allocator);

        each_json_value.SetString(mhm_des.c_str(), mhm_des.size(),allocator);  
        document.AddMember("mhm_des",each_json_value,allocator);

        document.AddMember("disp_onlink",disp_onlink,allocator);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();

        return sResult;
    }

    int ParaFood::ParseSqlite(sqlite3_stmt *statement)
    {
        id = sqlite3_column_int(statement, 0);
        kind_id = sqlite3_column_int(statement, 1);
        food_name = (const char*)sqlite3_column_text(statement, 2);
        food_code = sqlite3_column_int(statement, 3);
        food_type = sqlite3_column_int(statement, 4);

        return 0;
    }

    std::string ParaFood::ToJson()
    {
        Document document;
        Document docTmp;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();

        document.AddMember("id",id,allocator);

        document.AddMember("kind_id",kind_id,allocator);

        Value each_json_value(kStringType);
        each_json_value.SetString(food_name.c_str(), food_name.size(),allocator);  
        document.AddMember("food_name",each_json_value,allocator);

        document.AddMember("food_code",food_code,allocator);

        document.AddMember("food_type",food_type,allocator);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();

        return sResult;
    }

    int ParaIcon::ParseSqlite(sqlite3_stmt *statement)
    {
        id = sqlite3_column_int(statement, 0);
        idcode = (const char*)sqlite3_column_text(statement, 1);
        name_in_nav = (const char*)sqlite3_column_text(statement, 2);
        type = sqlite3_column_int(statement, 3);

        return 0;
    }

    std::string ParaIcon::ToJson()
    {
        Document document;
        Document docTmp;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();

        document.AddMember("id",id,allocator);

        Value each_json_value(kStringType);
        each_json_value.SetString(idcode.c_str(), idcode.size(),allocator);  
        document.AddMember("idcode",each_json_value,allocator);

        each_json_value.SetString(name_in_nav.c_str(), name_in_nav.size(),allocator);  
        document.AddMember("name_in_nav",each_json_value,allocator);

        document.AddMember("type",type,allocator);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();

        return sResult;
    }

    int ParaKindChain::ParseSqlite(sqlite3_stmt *statement)
    {
        id = sqlite3_column_int(statement, 0);
        kind_id = sqlite3_column_int(statement, 1);
        chain_id = sqlite3_column_int(statement, 2);
        chain_name = (const char*)sqlite3_column_text(statement, 3);
        chain = (const char*)sqlite3_column_text(statement, 4);
        food_type = (const char*)sqlite3_column_text(statement, 5);
        level = (const char*)sqlite3_column_text(statement, 6);
        chain_type = (const char*)sqlite3_column_text(statement, 7);
        kind_code = (const char*)sqlite3_column_text(statement, 8);

        return 0;
    }

    std::string ParaKindChain::ToJson()
    {
        Document document;
        Document docTmp;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();

        document.AddMember("id",id,allocator);

        document.AddMember("kind_id",kind_id,allocator);

        document.AddMember("chain_id",chain_id,allocator);

        Value each_json_value(kStringType);
        each_json_value.SetString(chain_name.c_str(), chain_name.size(),allocator);  
        document.AddMember("chain_name",each_json_value,allocator);

        each_json_value.SetString(chain.c_str(), chain.size(),allocator);  
        document.AddMember("chain",each_json_value,allocator);

        each_json_value.SetString(food_type.c_str(), food_type.size(),allocator);  
        document.AddMember("food_type",each_json_value,allocator);

        each_json_value.SetString(level.c_str(), level.size(),allocator);  
        document.AddMember("level",each_json_value,allocator);

        each_json_value.SetString(chain_type.c_str(), chain_type.size(),allocator);  
        document.AddMember("chain_type",each_json_value,allocator);

        each_json_value.SetString(kind_code.c_str(), kind_code.size(),allocator);  
        document.AddMember("kind_code",each_json_value,allocator);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();

        return sResult;
    }

    int ParaSensitiveWords::ParseSqlite(sqlite3_stmt *statement)
    {
        id = sqlite3_column_int(statement, 0);
        sensitive_word = (const char*)sqlite3_column_text(statement, 1);
        type = (const char*)sqlite3_column_text(statement, 2);

        return 0;
    }

    std::string ParaSensitiveWords::ToJson()
    {
        Document document;
        Document docTmp;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();

        document.AddMember("id",id,allocator);

        Value each_json_value(kStringType);
        each_json_value.SetString(sensitive_word.c_str(), sensitive_word.size(),allocator);  
        document.AddMember("sensitive_word",each_json_value,allocator);

        each_json_value.SetString(type.c_str(), type.size(),allocator);  
        document.AddMember("type",each_json_value,allocator);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();

        return sResult;
    }

    int ParaTel::ParseSqlite(sqlite3_stmt *statement)
    {
        id = sqlite3_column_int(statement, 0);
        province = (const char*)sqlite3_column_text(statement, 1);
        city = (const char*)sqlite3_column_text(statement, 2);
        code = (const char*)sqlite3_column_text(statement, 3);
        tel_len = (const char*)sqlite3_column_text(statement, 4);
        city_code = (const char*)sqlite3_column_text(statement, 5);

        return 0;
    }

    std::string ParaTel::ToJson()
    {
        Document document;
        Document docTmp;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();

        document.AddMember("id",id,allocator);

        Value each_json_value(kStringType);
        each_json_value.SetString(province.c_str(), province.size(),allocator);  
        document.AddMember("province",each_json_value,allocator);

        each_json_value.SetString(city.c_str(), city.size(),allocator);  
        document.AddMember("city",each_json_value,allocator);

        each_json_value.SetString(code.c_str(), code.size(),allocator);  
        document.AddMember("code",each_json_value,allocator);

        each_json_value.SetString(tel_len.c_str(), tel_len.size(),allocator);  
        document.AddMember("tel_len",each_json_value,allocator);

        each_json_value.SetString(city_code.c_str(), city_code.size(),allocator);  
        document.AddMember("city_code",each_json_value,allocator);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();

        return sResult;
    }

    int GDBBkFace::ParseJson(std::string sJon)
    {
        return 0;
    }

    int GDBBkFace::InsertSqliteTable(sqlite3* db)
    {
        char *err_msg = NULL;

	    int ret = sqlite3_exec (db, "BEGIN", NULL, NULL, &err_msg);

        if (ret != SQLITE_OK)
        {
		    Logger::LogD("InsertSqliteTable BEGIN error: %s\n", err_msg);
            Logger::LogO("InsertSqliteTable BEGIN error: %s\n", err_msg);

            sqlite3_free(err_msg);
            return false;
        }

	    sqlite3_stmt* stmt;

	    std::string sql = "insert into gdb_bkFace values("
		    "?, ?, ?, ?, ?, ?, ?, ?)";

	    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("insert %s failed [%s]", "gdb_bkFace", sqlite3_errmsg(db));
            Logger::LogO("insert %s failed [%s]", "gdb_bkFace", sqlite3_errmsg(db));

		    return false;
	    }

	    sqlite3_bind_int(stmt, 1, pid);

        EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(geometry);

        EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

	    sqlite3_bind_blob(stmt, 2, spGeo, wkb->buffer_size()+39, NULL);

		delete[] wkb;

	    sqlite3_bind_text(stmt, 3, display_style.c_str(), display_style.length(), NULL);

	    sqlite3_bind_text(stmt, 4, display_text.c_str(), display_text.length(), NULL);

	    sqlite3_bind_text(stmt, 5, meshid.c_str(), meshid.length(), NULL);

	    sqlite3_bind_int(stmt, 6, kind);

	    sqlite3_bind_text(stmt, 7, op_date.c_str(), op_date.length(), NULL);

	    sqlite3_bind_int(stmt, 8, op_lifecycle);

	    if (sqlite3_step(stmt) != SQLITE_DONE)
	    {
            free(spGeo);

			sqlite3_finalize(stmt);

		    Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
            Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));

            return false;
	    }

        free(spGeo);

	    sqlite3_finalize(stmt);

        ret = sqlite3_exec (db, "COMMIT", NULL, NULL, &err_msg);

        if (ret != SQLITE_OK)
        {
            Logger::LogD ("COMMIT error: %s\n", err_msg);
            Logger::LogO ("COMMIT error: %s\n", err_msg);

            sqlite3_free (err_msg);
		
		    return false;
        }

        return 0;
    }

    int GDBBkFace::UpdateSqliteTable(sqlite3* db)
    {
        char *err_msg = NULL;

	    int ret = sqlite3_exec (db, "BEGIN", NULL, NULL, &err_msg);

        if (ret != SQLITE_OK)
        {
		    Logger::LogD("UpdateSqliteTable BEGIN error: %s\n", err_msg);
            Logger::LogO("UpdateSqliteTable BEGIN error: %s\n", err_msg);

            sqlite3_free (err_msg);
            return false;
        }
        
	    sqlite3_stmt* stmt;

	    std::string sql = "update gdb_bkFace set "
		    "pid=?, geometry=?, display_style=?, display_text=?, meshid=?, kind=?, op_date=?, op_lifecycle=?"
		    " where pid=?";

	    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("insert %s failed [%s]", "gdb_bkFace", sqlite3_errmsg(db));
            Logger::LogO("insert %s failed [%s]", "gdb_bkFace", sqlite3_errmsg(db));

		    return false;
	    }

	    sqlite3_bind_int(stmt, 1, pid);

	    EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(geometry);

        EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

	    sqlite3_bind_blob(stmt, 2, spGeo, wkb->buffer_size()+39, NULL);

		delete[] wkb;

        sqlite3_bind_text(stmt, 3, display_style.c_str(), display_style.length(), NULL);

	    sqlite3_bind_text(stmt, 4, display_text.c_str(), display_text.length(), NULL);

	    sqlite3_bind_text(stmt, 5, meshid.c_str(), meshid.length(), NULL);

	    sqlite3_bind_int(stmt, 6, kind);

	    sqlite3_bind_text(stmt, 7, op_date.c_str(), op_date.length(), NULL);

	    sqlite3_bind_int(stmt, 8, op_lifecycle);

        sqlite3_bind_int(stmt, 9, pid);

	    if (sqlite3_step(stmt) != SQLITE_DONE)
	    {
		    Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
		    Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
			
			free(spGeo);

		    sqlite3_finalize(stmt);

            return false;
	    }

        free(spGeo);

	    sqlite3_finalize(stmt);

	    ret = sqlite3_exec (db, "COMMIT", NULL, NULL, &err_msg);
        if (ret != SQLITE_OK)
        {
            Logger::LogD ("COMMIT error: %s\n", err_msg);
            Logger::LogO ("COMMIT error: %s\n", err_msg);

            sqlite3_free (err_msg);
		
		    return false;
        }
        return 0;
    }


    int GDBBkLine::ParseJson(std::string sJon)
    {
        return 0;
    }

    int GDBBkLine::InsertSqliteTable(sqlite3* db)
    {
        char *err_msg = NULL;

	    int ret = sqlite3_exec (db, "BEGIN", NULL, NULL, &err_msg);

        if (ret != SQLITE_OK)
        {
		    Logger::LogD("InsertSqliteTable BEGIN error: %s\n", err_msg);
            Logger::LogO("InsertSqliteTable BEGIN error: %s\n", err_msg);

            sqlite3_free(err_msg);
            return false;
        }

	    sqlite3_stmt* stmt;

	    std::string sql = "insert into gdb_bkLine values("
		    "?, ?, ?, ?, ?, ?, ?, ?)";

	    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("insert %s failed [%s]", "gdb_bkLine", sqlite3_errmsg(db));
            Logger::LogO("insert %s failed [%s]", "gdb_bkLine", sqlite3_errmsg(db));

		    return false;
	    }

	    sqlite3_bind_int(stmt, 1, pid);

        EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(geometry);

        EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

	    sqlite3_bind_blob(stmt, 2, spGeo, wkb->buffer_size()+39, NULL);

		delete[] wkb;

	    sqlite3_bind_text(stmt, 3, display_style.c_str(), display_style.length(), NULL);

	    sqlite3_bind_text(stmt, 4, display_text.c_str(), display_text.length(), NULL);

	    sqlite3_bind_text(stmt, 5, meshid.c_str(), meshid.length(), NULL);

	    sqlite3_bind_int(stmt, 6, kind);

	    sqlite3_bind_text(stmt, 7, op_date.c_str(), op_date.length(), NULL);

	    sqlite3_bind_int(stmt, 8, op_lifecycle);

	    if (sqlite3_step(stmt) != SQLITE_DONE)
	    {
            free(spGeo);

			sqlite3_finalize(stmt);

		    Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
            Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));

            return false;
	    }

        free(spGeo);

	    sqlite3_finalize(stmt);

        ret = sqlite3_exec (db, "COMMIT", NULL, NULL, &err_msg);

        if (ret != SQLITE_OK)
        {
            Logger::LogD ("COMMIT error: %s\n", err_msg);
            Logger::LogO ("COMMIT error: %s\n", err_msg);

            sqlite3_free (err_msg);
		
		    return false;
        }
        return 0;
    }

    int GDBBkLine::UpdateSqliteTable(sqlite3* db)
    {
        char *err_msg = NULL;

	    int ret = sqlite3_exec (db, "BEGIN", NULL, NULL, &err_msg);

        if (ret != SQLITE_OK)
        {
		    Logger::LogD("UpdateSqliteTable BEGIN error: %s\n", err_msg);
            Logger::LogO("UpdateSqliteTable BEGIN error: %s\n", err_msg);

            sqlite3_free (err_msg);
            return false;
        }
        
	    sqlite3_stmt* stmt;

	    std::string sql = "update gdb_bkLine set "
		    "pid=?, geometry=?, display_style=?, display_text=?, meshid=?, kind=?, op_date=?, op_lifecycle=?"
		    " where pid=?";

	    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("insert %s failed [%s]", "gdb_bkLine", sqlite3_errmsg(db));
            Logger::LogO("insert %s failed [%s]", "gdb_bkLine", sqlite3_errmsg(db));

		    return false;
	    }

	    sqlite3_bind_int(stmt, 1, pid);

	    EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(geometry);

        EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

	    sqlite3_bind_blob(stmt, 2, spGeo, wkb->buffer_size()+39, NULL);

		delete[] wkb;

        sqlite3_bind_text(stmt, 3, display_style.c_str(), display_style.length(), NULL);

	    sqlite3_bind_text(stmt, 4, display_text.c_str(), display_text.length(), NULL);

	    sqlite3_bind_text(stmt, 5, meshid.c_str(), meshid.length(), NULL);

	    sqlite3_bind_int(stmt, 6, kind);

	    sqlite3_bind_text(stmt, 7, op_date.c_str(), op_date.length(), NULL);

	    sqlite3_bind_int(stmt, 8, op_lifecycle);

        sqlite3_bind_int(stmt, 9, pid);

	    if (sqlite3_step(stmt) != SQLITE_DONE)
	    {
		    Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
		    Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
			
			free(spGeo);

		    sqlite3_finalize(stmt);

            return false;
	    }

        free(spGeo);

	    sqlite3_finalize(stmt);

	    ret = sqlite3_exec (db, "COMMIT", NULL, NULL, &err_msg);
        if (ret != SQLITE_OK)
        {
            Logger::LogD ("COMMIT error: %s\n", err_msg);
            Logger::LogO ("COMMIT error: %s\n", err_msg);

            sqlite3_free (err_msg);
		
		    return false;
        }
        return 0;
    }


    int GDBRdLine::ParseJson(std::string sJson)
    {
         rapidjson::Document doc;

        doc.Parse<0>(sJson.c_str());

        if(doc.HasParseError())
        {
            return -1;
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

	        if(sKey == "pid")
            {
                pid = (ptr->value).GetInt();
            }
            else if(sKey == "geometry")
            {
                geometry = GetCorrectString(sValue);         
            }
            else if(sKey == "display_style")
            {
                display_style = sValue;
            }
            else if(sKey == "display_text")
            {
                display_text = GetCorrectString(sValue);
            }
            else if(sKey == "meshid")
            {
                meshid = GetCorrectString(sValue);
            }
            else if(sKey == "kind")
            {
                kind = (ptr->value).GetInt();
            }
            else if(sKey == "direct")
            {
                direct = (ptr->value).GetInt();
            }
            else if(sKey == "appInfo")
            {
                appInfo = (ptr->value).GetInt();
            }
            else if(sKey == "tollInfo")
            {
                tollInfo = (ptr->value).GetInt();
            }
            else if(sKey == "multiDigitized")
            {
                multiDigitized = (ptr->value).GetInt();
            }
            else if(sKey == "specialTraffic")
            {
                specialTraffic = (ptr->value).GetInt();
            }
            else if(sKey == "fc")
            {
                fc = (ptr->value).GetInt();
            }
            else if(sKey == "laneNum")
            {
                laneNum = (ptr->value).GetInt();
            }
            else if(sKey == "laneLeft")
            {
                laneLeft = (ptr->value).GetInt();
            }
            else if(sKey == "laneRight")
            {
                laneRight = (ptr->value).GetInt();
            }
            else if(sKey == "isViaduct")
            {
                isViaduct = (ptr->value).GetInt();
            }
            else if(sKey == "paveStatus")
            {
                paveStatus = (ptr->value).GetInt();
            }
            else if(sKey == "forms")
            {
                forms = sValue;
            }
            else if(sKey == "styleFactors")
            {
                styleFactors = sValue;
            }
            else if(sKey == "speedLimit")
            {
                speedLimit = sValue;
            }
            else if(sKey == "op_date")
            {
                op_date = GetCorrectString(sValue);
            }
            else if(sKey == "op_lifecycle")
            {
                op_lifecycle = (ptr->value).GetInt();
            }
        }

        return 0;
    }

    std::string GDBRdLine::GetCorrectString(std::string sIn)
    {
        if(sIn.empty())
        {
            return "";
        }

        if(sIn.size() >= 2 && (sIn[0] == '\"' && sIn[sIn.length() - 1] == '\"'))
        {
            return sIn.substr(1, sIn.size() - 2);
        }

        return "";
    }

    int GDBRdLine::InsertSqliteTable(sqlite3* db)
    {
         char *err_msg = NULL;

	    int ret = sqlite3_exec (db, "BEGIN", NULL, NULL, &err_msg);

        if (ret != SQLITE_OK)
        {
		    Logger::LogD("InsertSqliteTable BEGIN error: %s\n", err_msg);
            Logger::LogO("InsertSqliteTable BEGIN error: %s\n", err_msg);

            sqlite3_free(err_msg);
            return false;
        }

	    sqlite3_stmt* stmt;

	    std::string sql = "insert into gdb_rdLine values("
		    "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

	    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("insert %s failed [%s]", "gdb_rdLine", sqlite3_errmsg(db));
            Logger::LogO("insert %s failed [%s]", "gdb_rdLine", sqlite3_errmsg(db));

		    return false;
	    }

	    sqlite3_bind_int(stmt, 1, pid);

        EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(geometry);

        EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

	    sqlite3_bind_blob(stmt, 2, spGeo, wkb->buffer_size()+39, NULL);

		delete[] wkb;

	    sqlite3_bind_text(stmt, 3, display_style.c_str(), display_style.length(), NULL);

	    sqlite3_bind_text(stmt, 4, display_text.c_str(), display_text.length(), NULL);

	    sqlite3_bind_text(stmt, 5, meshid.c_str(), meshid.length(), NULL);

	    sqlite3_bind_int(stmt, 6, kind);

	    sqlite3_bind_int(stmt, 7, direct);

        sqlite3_bind_int(stmt, 8, appInfo);

	    sqlite3_bind_int(stmt, 9, tollInfo);

        sqlite3_bind_int(stmt, 10, multiDigitized);

        sqlite3_bind_int(stmt, 11, specialTraffic);

        sqlite3_bind_int(stmt, 12, fc);

        sqlite3_bind_int(stmt, 13, laneNum);

        sqlite3_bind_int(stmt, 14, laneLeft);

        sqlite3_bind_int(stmt, 15, laneRight);

        sqlite3_bind_int(stmt, 16, isViaduct);

        sqlite3_bind_int(stmt, 17, paveStatus);

        sqlite3_bind_text(stmt, 18, forms.c_str(), forms.length(), NULL);

        sqlite3_bind_text(stmt, 19, styleFactors.c_str(), styleFactors.length(), NULL);

        sqlite3_bind_text(stmt, 20, speedLimit.c_str(), speedLimit.length(), NULL);

        sqlite3_bind_text(stmt, 21, op_date.c_str(), op_date.length(), NULL);

        sqlite3_bind_int(stmt, 22, op_lifecycle);

	    if (sqlite3_step(stmt) != SQLITE_DONE)
	    {
            free(spGeo);

			sqlite3_finalize(stmt);

		    Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
            Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));

            return false;
	    }

        free(spGeo);

	    sqlite3_finalize(stmt);

        ret = sqlite3_exec (db, "COMMIT", NULL, NULL, &err_msg);

        if (ret != SQLITE_OK)
        {
            Logger::LogD ("COMMIT error: %s\n", err_msg);
            Logger::LogO ("COMMIT error: %s\n", err_msg);

            sqlite3_free (err_msg);
		
		    return false;
        }
        return 0;
    }

    int GDBRdLine::InsertSqliteTableWithoutTransaction(sqlite3* db)
    {
        sqlite3_stmt* stmt;

	    std::string sql = "insert into gdb_rdLine values("
		    "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

	    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("insert %s failed [%s]", "gdb_rdLine", sqlite3_errmsg(db));
            Logger::LogO("insert %s failed [%s]", "gdb_rdLine", sqlite3_errmsg(db));

		    return false;
	    }

	    sqlite3_bind_int(stmt, 1, pid);

        EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(geometry);

        EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

	    sqlite3_bind_blob(stmt, 2, spGeo, wkb->buffer_size()+39, NULL);

		delete[] wkb;

	    sqlite3_bind_text(stmt, 3, display_style.c_str(), display_style.length(), NULL);

	    sqlite3_bind_text(stmt, 4, display_text.c_str(), display_text.length(), NULL);

	    sqlite3_bind_text(stmt, 5, meshid.c_str(), meshid.length(), NULL);

	    sqlite3_bind_int(stmt, 6, kind);

	    sqlite3_bind_int(stmt, 7, direct);

        sqlite3_bind_int(stmt, 8, appInfo);

	    sqlite3_bind_int(stmt, 9, tollInfo);

        sqlite3_bind_int(stmt, 10, multiDigitized);

        sqlite3_bind_int(stmt, 11, specialTraffic);

        sqlite3_bind_int(stmt, 12, fc);

        sqlite3_bind_int(stmt, 13, laneNum);

        sqlite3_bind_int(stmt, 14, laneLeft);

        sqlite3_bind_int(stmt, 15, laneRight);

        sqlite3_bind_int(stmt, 16, isViaduct);

        sqlite3_bind_int(stmt, 17, paveStatus);

        sqlite3_bind_text(stmt, 18, forms.c_str(), forms.length(), NULL);

        sqlite3_bind_text(stmt, 19, styleFactors.c_str(), styleFactors.length(), NULL);

        sqlite3_bind_text(stmt, 20, speedLimit.c_str(), speedLimit.length(), NULL);

        sqlite3_bind_text(stmt, 21, op_date.c_str(), op_date.length(), NULL);

        sqlite3_bind_int(stmt, 22, op_lifecycle);

	    if (sqlite3_step(stmt) != SQLITE_DONE)
	    {
            free(spGeo);

			sqlite3_finalize(stmt);

		    Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
            Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));

            return false;
	    }

        free(spGeo);

	    sqlite3_finalize(stmt);

        return 0;
    }

    int GDBRdLine::UpdateSqliteTable(sqlite3* db)
    {
        char *err_msg = NULL;

	    int ret = sqlite3_exec (db, "BEGIN", NULL, NULL, &err_msg);

        if (ret != SQLITE_OK)
        {
		    Logger::LogD("UpdateSqliteTable BEGIN error: %s\n", err_msg);
            Logger::LogO("UpdateSqliteTable BEGIN error: %s\n", err_msg);

            sqlite3_free (err_msg);
            return false;
        }
        
	    sqlite3_stmt* stmt;

	    std::string sql = "update gdb_rdLine set "
		    "pid=?, geometry=?, display_style=?, display_text=?, meshid=?, kind=?, direct=?, appInfo=?, tollInfo=?,"
            "multiDigitized=?, specialTraffic=?, fc=?, laneNum=?, laneLeft=?, laneRight=?, isViaduct=?, paveStatus=?, "
            "forms=?, styleFactors=?, speedLimit=?, op_date=?, op_lifecycle=?"
		    " where pid=?";

	    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("insert %s failed [%s]", "gdb_rdLine", sqlite3_errmsg(db));
            Logger::LogO("insert %s failed [%s]", "gdb_rdLine", sqlite3_errmsg(db));

		    return false;
	    }

	    sqlite3_bind_int(stmt, 1, pid);

		EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(geometry);

		EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

		sqlite3_bind_blob(stmt, 2, spGeo, wkb->buffer_size()+39, NULL);

		delete[] wkb;

	    sqlite3_bind_text(stmt, 3, display_style.c_str(), display_style.length(), NULL);

	    sqlite3_bind_text(stmt, 4, display_text.c_str(), display_text.length(), NULL);

	    sqlite3_bind_text(stmt, 5, meshid.c_str(), meshid.length(), NULL);

	    sqlite3_bind_int(stmt, 6, kind);

	    sqlite3_bind_int(stmt, 7, direct);

        sqlite3_bind_int(stmt, 8, appInfo);

	    sqlite3_bind_int(stmt, 9, tollInfo);

        sqlite3_bind_int(stmt, 10, multiDigitized);

        sqlite3_bind_int(stmt, 11, specialTraffic);

        sqlite3_bind_int(stmt, 12, fc);

        sqlite3_bind_int(stmt, 13, laneNum);

        sqlite3_bind_int(stmt, 14, laneLeft);

        sqlite3_bind_int(stmt, 15, laneRight);

        sqlite3_bind_int(stmt, 16, isViaduct);

        sqlite3_bind_int(stmt, 17, paveStatus);

        sqlite3_bind_text(stmt, 18, forms.c_str(), forms.length(), NULL);

        sqlite3_bind_text(stmt, 19, styleFactors.c_str(), styleFactors.length(), NULL);

        sqlite3_bind_text(stmt, 20, speedLimit.c_str(), speedLimit.length(), NULL);

        sqlite3_bind_text(stmt, 21, op_date.c_str(), op_date.length(), NULL);

        sqlite3_bind_int(stmt, 22, op_lifecycle);

        sqlite3_bind_int(stmt, 23, pid);

	    if (sqlite3_step(stmt) != SQLITE_DONE)
	    {
		    Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
		    Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
			
			free(spGeo);

		    sqlite3_finalize(stmt);

            return false;
	    }

        free(spGeo);

	    sqlite3_finalize(stmt);

	    ret = sqlite3_exec (db, "COMMIT", NULL, NULL, &err_msg);
        if (ret != SQLITE_OK)
        {
            Logger::LogD ("COMMIT error: %s\n", err_msg);
            Logger::LogO ("COMMIT error: %s\n", err_msg);

            sqlite3_free (err_msg);
		
		    return false;
        }
        return 0;
    }

    int GDBRdLine::UpdateSqliteTableWithoutTransaction(sqlite3* db)
    {
        sqlite3_stmt* stmt;

	    std::string sql = "update gdb_rdLine set "
		    "pid=?, geometry=?, display_style=?, display_text=?, meshid=?, kind=?, direct=?, appInfo=?, tollInfo=?,"
            "multiDigitized=?, specialTraffic=?, fc=?, laneNum=?, laneLeft=?, laneRight=?, isViaduct=?, paveStatus=?, "
            "forms=?, styleFactors=?, speedLimit=?, op_date=?, op_lifecycle=?"
		    " where pid=?";

	    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("insert %s failed [%s]", "gdb_rdLine", sqlite3_errmsg(db));
            Logger::LogO("insert %s failed [%s]", "gdb_rdLine", sqlite3_errmsg(db));

		    return false;
	    }

	    sqlite3_bind_int(stmt, 1, pid);

        EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(geometry);

        EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

	    sqlite3_bind_blob(stmt, 2, spGeo, wkb->buffer_size()+39, NULL);

		delete[] wkb;

	    sqlite3_bind_text(stmt, 3, display_style.c_str(), display_style.length(), NULL);

	    sqlite3_bind_text(stmt, 4, display_text.c_str(), display_text.length(), NULL);

	    sqlite3_bind_text(stmt, 5, meshid.c_str(), meshid.length(), NULL);

	    sqlite3_bind_int(stmt, 6, kind);

	    sqlite3_bind_int(stmt, 7, direct);

        sqlite3_bind_int(stmt, 8, appInfo);

	    sqlite3_bind_int(stmt, 9, tollInfo);

        sqlite3_bind_int(stmt, 10, multiDigitized);

        sqlite3_bind_int(stmt, 11, specialTraffic);

        sqlite3_bind_int(stmt, 12, fc);

        sqlite3_bind_int(stmt, 13, laneNum);

        sqlite3_bind_int(stmt, 14, laneLeft);

        sqlite3_bind_int(stmt, 15, laneRight);

        sqlite3_bind_int(stmt, 16, isViaduct);

        sqlite3_bind_int(stmt, 17, paveStatus);

        sqlite3_bind_text(stmt, 18, forms.c_str(), forms.length(), NULL);

        sqlite3_bind_text(stmt, 19, styleFactors.c_str(), styleFactors.length(), NULL);

        sqlite3_bind_text(stmt, 20, speedLimit.c_str(), speedLimit.length(), NULL);

        sqlite3_bind_text(stmt, 21, op_date.c_str(), op_date.length(), NULL);

        sqlite3_bind_int(stmt, 22, op_lifecycle);

        sqlite3_bind_int(stmt, 23, pid);

	    if (sqlite3_step(stmt) != SQLITE_DONE)
	    {
		    Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
		    Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
			
			free(spGeo);

		    sqlite3_finalize(stmt);

            return false;
	    }

        free(spGeo);

	    sqlite3_finalize(stmt);

        return 0;
    }

    void GDBRdLine::SetDisplayStyle()
    {
        if(paveStatus == 1)
        {
            display_style = Tools::NumberToString<int>(kind) + ",0";

            return;
        }

        rapidjson::Document doc;

        doc.Parse<0>(forms.c_str());

        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        if(doc.HasParseError() && !doc.IsArray())
        {
            return;
        }

        if(doc.Size() == 1)
        {
            rapidjson::Value& p = doc[0];

            if(p.HasMember("form"))
            {
                int nTmp = p["form"].GetInt();

                if(nTmp == 24 && isViaduct == 1)
                {
                    display_style = Tools::NumberToString<int>(kind) + ",1";
                    return;
                }
                else if(nTmp == 30 && isViaduct == 1)
                {
                    display_style = Tools::NumberToString<int>(kind) + ",2";
                    return;
                }
                else if(isViaduct == 1)
                {
                    display_style = Tools::NumberToString<int>(kind) + ",3";
                    return;
                }
                else if(nTmp == 15)
                {
                    display_style = Tools::NumberToString<int>(kind) + ",4";
                    return;
                }
                else if(nTmp == 16)
                {
                    display_style = Tools::NumberToString<int>(kind) + ",5";
                    return;
                }
                else if(nTmp == 17)
                {
                    display_style = Tools::NumberToString<int>(kind) + ",6";
                    return;
                }
                else if(nTmp == 22)
                {
                    display_style = Tools::NumberToString<int>(kind) + ",7";
                    return;
                }
                else if(nTmp == 24)
                {
                    display_style = Tools::NumberToString<int>(kind) + ",9";
                    return;
                }
                else if(nTmp == 30)
                {
                    display_style = Tools::NumberToString<int>(kind) + ",10";
                    return;
                }
                else if(nTmp == 31)
                {
                    display_style = Tools::NumberToString<int>(kind) + ",11";
                    return;
                }
                else if(nTmp == 34)
                {
                    display_style = Tools::NumberToString<int>(kind) + ",12";
                    return;
                }
                else if(nTmp == 36)
                {
                    display_style = Tools::NumberToString<int>(kind) + ",14";
                    return;
                }
                else if(nTmp == 52)
                {
                     display_style = Tools::NumberToString<int>(kind) + ",15";
                     return;
                }
                else if(nTmp == 53)
                {
                    display_style = Tools::NumberToString<int>(kind) + ",16";
                    return;
                }
                else if(nTmp == 60)
                {
                    display_style = Tools::NumberToString<int>(kind) + ",17";
                    return;
                }
            }

        }
        else if(doc.Size() >= 2)
        {
            
            std::vector<int> vTmp;
        
            for(rapidjson::SizeType i = 0; i < doc.Size(); i++)
            {
                rapidjson::Value& p = doc[i];

                if(p.HasMember("form"))
                {
                    vTmp.push_back(p["form"].GetInt());

                    if(find(vTmp.begin(), vTmp.end(), 24) != vTmp.end() && find(vTmp.begin(), vTmp.end(), 24) != vTmp.end())
                    {
                        display_style = Tools::NumberToString<int>(kind) + ",8";
                        return;
                    }
                    else if(find(vTmp.begin(), vTmp.end(), 36) != vTmp.end() && find(vTmp.begin(), vTmp.end(), 52) != vTmp.end())
                    {
                        display_style = Tools::NumberToString<int>(kind) + ",13";
                        return;
                    }
                }
            }
        }

        if(SetDisplayStyleFromStyleFactors() ==0)
        {
            return;
        }
        
        display_style = Tools::NumberToString<int>(kind) + ",-1";

        return;
    }

    int GDBRdLine::SetDisplayStyleFromStyleFactors()
    {
        rapidjson::Document doc;

        doc.Parse<0>(styleFactors.c_str());

        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        if(doc.HasParseError() && !doc.IsArray())
        {
            return -1;
        }

        if(doc.Size() == 1)
        {
            rapidjson::Value& p = doc[0];

            if(p.HasMember("factor"))
            {
                int nTmp = p["factor"].GetInt();

                if(nTmp == 0)
                {
                    display_style = Tools::NumberToString<int>(kind) + ",18";
                    return 0;
                }
                else if(nTmp == 6)
                {
                    display_style = Tools::NumberToString<int>(kind) + ",19";
                    return 0;
                }
                else if(nTmp == 99)
                {
                    display_style = Tools::NumberToString<int>(kind) + ",20";
                    return 0;
                }
            }
        }

        return -1;
    }

	tips_point::tips_point()
	{
		strUuid = "";
		geoType = displayGeoPoint;
		sourceType = "";
		display_style = "";
	}

	tips_point::~tips_point()
	{

	}

	int tips_point::InsertSqliteTable( const Tips& tip, sqlite3* db )
	{
        EditorGeometry::WkbGeometry* wkb=NULL;
        EditorGeometry::SpatialiteGeometry* spGeo=NULL;
        
        wkb = DataTransfor::Wkt2Wkb(geometry);
        
        if(NULL == wkb)
        {
            return 0;
        }
        
        spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

		sqlite3_stmt* stmt;

		std::string sql = "insert into tips_point values(?, ?, ?, ?, ?, ?)";

		int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

		if (rc != SQLITE_OK)
		{
			Logger::LogD("insert %s failed [%s]", "Tips", sqlite3_errmsg(db));
			Logger::LogO("insert %s failed [%s]", "Tips", sqlite3_errmsg(db));

			return false;
		}

		sqlite3_bind_text(stmt, 1, strUuid.c_str(), strUuid.length(), NULL);

		sqlite3_bind_int(stmt, 2, geoType);

		sqlite3_bind_text(stmt, 3, sourceType.c_str(), sourceType.length(), NULL);

		sqlite3_bind_text(stmt, 4, display_style.c_str(), display_style.length(), NULL);

		sqlite3_bind_blob(stmt, 5, spGeo, wkb->buffer_size()+39, NULL);

		//visibility
		int tipsType = atoi(tip.s_sourceType.c_str());
		if (tipsType==Bridge)
		{
			sqlite3_bind_int(stmt, 6, 0);
		}
		else
		{
			sqlite3_bind_int(stmt, 6, 1);
		}
		

		delete[] wkb;
		
		if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
			Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));

			free(spGeo);

			sqlite3_finalize(stmt);

			return false;
		}

		free(spGeo);

		spGeo = NULL;

		sqlite3_finalize(stmt);

		return true;
	}

	int tips_point::UpdateSqliteTable( const Tips& tip, sqlite3* db )
	{
        EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(geometry);
        
        if(wkb == NULL)
        {
            return 0;
        }
        
		sqlite3_stmt* stmt;

		std::string sql = "update tips_point set "
			"uuid=?, geoType=?, sourceType=?, display_style=?, geometry=?, visibility=?  "
			"where uuid in (select geoUuid from tips_geo_component where rowkey=?)";

		int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

		if (rc != SQLITE_OK)
		{
			Logger::LogD("update %s failed [%s]", "Tips", sqlite3_errmsg(db));
			Logger::LogO("update %s failed [%s]", "Tips", sqlite3_errmsg(db));

			return false;
		}

		sqlite3_bind_text(stmt, 1, strUuid.c_str(), strUuid.length(), NULL);

		sqlite3_bind_int(stmt, 2, geoType);

		sqlite3_bind_text(stmt, 3, sourceType.c_str(), sourceType.length(), NULL);

		sqlite3_bind_text(stmt, 4, display_style.c_str(), display_style.length(), NULL);

		EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

		sqlite3_bind_blob(stmt, 5, spGeo, wkb->buffer_size()+39, NULL);

		delete[] wkb;

		//visibility
		int tipsType = atoi(tip.s_sourceType.c_str());
		if (tipsType==Bridge)
		{
			sqlite3_bind_int(stmt, 6, 0);
		}
		else
		{
			sqlite3_bind_int(stmt, 6, 1);
		}

		sqlite3_bind_text(stmt, 7, tip.rowkey.c_str(), tip.rowkey.length(), NULL);

		if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
			Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));

			free(spGeo);

			sqlite3_finalize(stmt);

			return false;
		}

		free(spGeo);

		spGeo = NULL;

		sqlite3_finalize(stmt);

		return true;
	}

	std::string tips_point::getGeoTableName() const
	{
		return "tips_point";
	}

	void tips_point::setValues( const Tips& tip )
	{
		strUuid = Tools::GenerateUuid();
		geoType = displayGeoPoint;
		sourceType = tip.s_sourceType;
		display_style = tip.display_style;
        
        int tipsType = atoi(sourceType.c_str());
        switch (tipsType)
        {
            case Model::Bridge:
                if (tip.deep!="")
                {
                    Document deepDoc;
                    deepDoc.Parse<0>(tip.deep.c_str());
                    display_style = DataTransfor::GetTipsSEPointDisplayStyle(tipsType, deepDoc, m_displayPointType==eDisplayPointType_startPoint);
                }
                geometry = GetPointWktGeo(tip);
                break;
            case Model::Hitching:
                break;
            default:
                geometry = GetPointWktGeo(tip);
                break;
        }
        
	}

	void tips_point::Update( const Tips& tip, sqlite3* db, int operateType )
	{
		switch(operateType)
		{
		case  operatorInsert:
			{
				setValues(tip);
				InsertSqliteTable(tip,db);
				tips_geo_component::InsertSqliteTable(tip,*this,db);
				break;
			}
		case  operatorUpdate:
			{
				setValues(tip);
				UpdateSqliteTable(tip,db);
				//tips_geo_component::UpdateSqliteTable(tip,*this,db);
				break;
			}
		case  operatorUnKnown:
			break;
		}
	}

	std::string tips_point::GetPointWktGeo( const Tips& tip )
	{
		std::string strWkt="";
		int tipsType = atoi(tip.s_sourceType.c_str());
		switch(tipsType)
		{
		case PointSpeedLimit:
		case RoadKind:
		case OrdinaryTurnRestriction:
		case CrossName:
		case LaneConnexity:
		case RoadDirect:
        case HighwayBranch:
			{
				//char buf[200];
				//sprintf(buf, "Point(%f %f)",tip.longitude,tip.latitude);
				strWkt = tip.g_location;
				break;
			}
		case RoadName:
		case SurveyLine:
			{

				rapidjson::Document doc;

				doc.Parse<0>(tip.deep.c_str());

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

					if(sKey == "geo")
					{
						strWkt = DataTransfor::GeoJson2Wkt(sValue,tip.m_dataSource);

						break;
					}
				}

			}
			break;
		case Bridge:
        case RegionalRoad:
			{
				geos::geom::Geometry * geo = DataTransfor::Wkt2Geo(tip.g_location);
				if (geo)
				{
					geos::geom::CoordinateSequence*  pCoordinateSequence= geo->getCoordinates();
					geos::geom::GeometryFactory* gf = new geos::geom::GeometryFactory();
					if (pCoordinateSequence && pCoordinateSequence->getSize())
					{
						if (m_displayPointType==eDisplayPointType_startPoint)
						{
							const geos::geom::Coordinate& coord = pCoordinateSequence->getAt(0);
							geos::geom::Point* point = gf->createPoint(coord);
							if (point)
							{
								strWkt = point->toText();
								delete point;
							}
						}
						else if (m_displayPointType==eDisplayPointType_endPoint)
						{
							const geos::geom::Coordinate& coord = pCoordinateSequence->getAt(pCoordinateSequence->getSize()-1);
							geos::geom::Point* point = gf->createPoint(coord);
							if (point)
							{
								strWkt = point->toText();
								delete point;
							}
						}
					}

					//释放
					if (pCoordinateSequence)
					{
						delete pCoordinateSequence;
					}
					if (geo)
					{
						delete geo;
					}
					if (gf)
					{
						delete gf;
					}

				}
				break;
			}
		default:
			break;
		}
		return strWkt;
	}

	void tips_point::setDisplayPointType( DisplayPointType displayPointType )
	{
		m_displayPointType = displayPointType;
	}

	int tips_geo_component::InsertSqliteTable( const Tips& tip, const tips_geo& tipsGeo, sqlite3* db )
	{
		sqlite3_stmt* stmt;

		std::string sql = "insert into tips_geo_component values(?, ?, ?)";

		int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

		if (rc != SQLITE_OK)
		{
			Logger::LogD("insert %s failed [%s]", "tips_geo_component", sqlite3_errmsg(db));
			Logger::LogO("insert %s failed [%s]", "tips_geo_component", sqlite3_errmsg(db));

			return false;
		}

		sqlite3_bind_text(stmt, 1, tip.rowkey.c_str(), tip.rowkey.length(), NULL);

		sqlite3_bind_text(stmt, 2, tipsGeo.strUuid.c_str(), tipsGeo.strUuid.length(), NULL);

		std::string tableName = tipsGeo.getGeoTableName();

		sqlite3_bind_text(stmt, 3, tableName.c_str(), tableName.length(), NULL);

		if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
			Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));

			sqlite3_finalize(stmt);

			return false;
		}

		sqlite3_finalize(stmt);

		return true;
	}

	int tips_geo_component::UpdateSqliteTable( const Tips& tip, const tips_geo& tipsGeo, sqlite3* db )
	{
		return 0;
	}

	tips_line::tips_line()
	{

	}

	tips_line::~tips_line()
	{

	}

	void tips_line::setValues( const Tips& tip )
	{
		strUuid = Tools::GenerateUuid();
		geoType = displayGeoPoint;
		sourceType = tip.s_sourceType;
		display_style = tip.display_style;
		int tipsType = atoi(sourceType.c_str());
		switch(tipsType)
		{
		case Bridge:
		case RoadName:
            {
                display_style=DataTransfor::GetTipsLineDisplayStyle(sourceType.c_str(),"");
                geometry = tip.g_location;
                break;
            }
		case SurveyLine:
			geometry = tip.g_location;
			break;
        case Hitching:
                break;
		default:
			break;
		}
	}

	int tips_line::InsertSqliteTable( const Tips& tip, sqlite3* db )
	{
		sqlite3_stmt* stmt;

		std::string sql = "insert into tips_line values(?, ?, ?, ?, ?, ?)";

		int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

		if (rc != SQLITE_OK)
		{
			Logger::LogD("insert %s failed [%s]", "Tips", sqlite3_errmsg(db));
			Logger::LogO("insert %s failed [%s]", "Tips", sqlite3_errmsg(db));

			return false;
		}

		sqlite3_bind_text(stmt, 1, strUuid.c_str(), strUuid.length(), NULL);

		sqlite3_bind_int(stmt, 2, geoType);

		sqlite3_bind_text(stmt, 3, sourceType.c_str(), sourceType.length(), NULL);

		sqlite3_bind_text(stmt, 4, display_style.c_str(), display_style.length(), NULL);

		EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(geometry);

		EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

		sqlite3_bind_blob(stmt, 5, spGeo, wkb->buffer_size()+39, NULL);

		delete[] wkb;

		//visibility
		int tipsType = atoi(tip.s_sourceType.c_str());
		if (tipsType==Bridge)
		{
			sqlite3_bind_int(stmt, 6, 0);
		}
		else
		{
			sqlite3_bind_int(stmt, 6, 1);
		}

		if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
			Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));

			free(spGeo);

			sqlite3_finalize(stmt);

			return false;
		}

		free(spGeo);

		spGeo = NULL;

		sqlite3_finalize(stmt);

		return true;
	}

	int tips_line::UpdateSqliteTable( const Tips& tip, sqlite3* db )
	{
		sqlite3_stmt* stmt;

		std::string sql = "update tips_line set "
			"uuid=?, geoType=?, sourceType=?, display_style=?, geometry=?, visibility=?  "
			"where uuid in (select geoUuid from tips_geo_component where rowkey=?)";

		int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

		if (rc != SQLITE_OK)
		{
			Logger::LogD("update %s failed [%s]", "tips_line", sqlite3_errmsg(db));
			Logger::LogO("update %s failed [%s]", "tips_line", sqlite3_errmsg(db));

			return false;
		}

		sqlite3_bind_text(stmt, 1, strUuid.c_str(), strUuid.length(), NULL);

		sqlite3_bind_int(stmt, 2, geoType);

		sqlite3_bind_text(stmt, 3, sourceType.c_str(), sourceType.length(), NULL);

		sqlite3_bind_text(stmt, 4, display_style.c_str(), display_style.length(), NULL);

		EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(geometry);

		EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

		sqlite3_bind_blob(stmt, 5, spGeo, wkb->buffer_size()+39, NULL);

		delete[] wkb;

		//visibility
		int tipsType = atoi(tip.s_sourceType.c_str());
		if (tipsType==Bridge)
		{
			sqlite3_bind_int(stmt, 6, 0);
		}
		else
		{
			sqlite3_bind_int(stmt, 6, 1);
		}

		sqlite3_bind_text(stmt, 7, tip.rowkey.c_str(), tip.rowkey.length(), NULL);

		if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
			Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));

			free(spGeo);

			sqlite3_finalize(stmt);

			return false;
		}

		free(spGeo);

		spGeo = NULL;

		sqlite3_finalize(stmt);

		return true;
	}

	std::string tips_line::getGeoTableName() const
	{
		return "tips_line";
	}

	void tips_line::Update( const Tips& tip, sqlite3* db, int operateType )
	{
		switch(operateType)
		{
		case  operatorInsert:
			{
				setValues(tip);
				InsertSqliteTable(tip,db);
				tips_geo_component::InsertSqliteTable(tip,*this,db);
				break;
			}
		case  operatorUpdate:
			{
				setValues(tip);
				UpdateSqliteTable(tip,db);
				//tips_geo_component::UpdateSqliteTable(tip,*this,db);
				break;
			}
		case  operatorUnKnown:
			break;
		}
	}

	tips_polygon::tips_polygon()
	{

	}

	tips_polygon::~tips_polygon()
	{

	}

	void tips_polygon::setValues( const Tips& tip )
	{
        strUuid = Tools::GenerateUuid();
        geoType = displayGeoPoint;
        sourceType = tip.s_sourceType;
        display_style = tip.display_style;
        int tipsType = atoi(sourceType.c_str());
        switch(tipsType)
        {
            case RegionalRoad:
            {
                display_style= RegionalRoadPolygonStyle;
                geometry = tip.g_location;
                break;
            }
            default:
                geometry="";
                break;
        }

	}

	int tips_polygon::InsertSqliteTable( const Tips& tip, sqlite3* db )
	{
		sqlite3_stmt* stmt;
        
        std::string sql = "insert into tips_polygon values(?, ?, ?, ?, ?, ?)";
        
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
        
        if (rc != SQLITE_OK)
        {
            Logger::LogD("insert %s failed [%s]", "tips_polygon", sqlite3_errmsg(db));
            Logger::LogO("insert %s failed [%s]", "tips_polygon", sqlite3_errmsg(db));
            
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, strUuid.c_str(), strUuid.length(), NULL);
        
        sqlite3_bind_int(stmt, 2, geoType);
        
        sqlite3_bind_text(stmt, 3, sourceType.c_str(), sourceType.length(), NULL);
        
        sqlite3_bind_text(stmt, 4, display_style.c_str(), display_style.length(), NULL);
        
        EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(geometry);
        
        EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);
        
        sqlite3_bind_blob(stmt, 5, spGeo, wkb->buffer_size()+39, NULL);
        
        delete[] wkb;
        
        sqlite3_bind_int(stmt, 6, 1);
        
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
            Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
            
            free(spGeo);
            
            sqlite3_finalize(stmt);
            
            return false;
        }
        
        free(spGeo);
        
        spGeo = NULL;
        
        sqlite3_finalize(stmt);
        
        return true;
	}

	int tips_polygon::UpdateSqliteTable( const Tips& tip, sqlite3* db )
	{
		sqlite3_stmt* stmt;

		std::string sql = "update tips_polygon set "
			"uuid=?, geoType=?, sourceType=?, display_style=?, geometry=?  "
			"where uuid in (select geoUuid from tips_geo_component where rowkey=?)";

		int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

		if (rc != SQLITE_OK)
		{
			Logger::LogD("update %s failed [%s]", "tips_polygon", sqlite3_errmsg(db));
			Logger::LogO("update %s failed [%s]", "tips_polygon", sqlite3_errmsg(db));

			return false;
		}

		sqlite3_bind_text(stmt, 1, strUuid.c_str(), strUuid.length(), NULL);

		sqlite3_bind_int(stmt, 2, geoType);

		sqlite3_bind_text(stmt, 3, sourceType.c_str(), sourceType.length(), NULL);

		sqlite3_bind_text(stmt, 4, display_style.c_str(), display_style.length(), NULL);

		EditorGeometry::WkbGeometry* wkb = DataTransfor::Wkt2Wkb(geometry);

		EditorGeometry::SpatialiteGeometry* spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

		sqlite3_bind_blob(stmt, 5, spGeo, wkb->buffer_size()+39, NULL);

		delete[] wkb;

		sqlite3_bind_text(stmt, 6, tip.rowkey.c_str(), tip.rowkey.length(), NULL);

		if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
			Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));

			free(spGeo);

			sqlite3_finalize(stmt);

			return false;
		}

		free(spGeo);

		spGeo = NULL;

		sqlite3_finalize(stmt);

		return true;
	}

	std::string tips_polygon::getGeoTableName() const
	{
		return "tips_polygon";
	}

	void tips_polygon::Update( const Tips& tip, sqlite3* db, int operateType )
	{
        switch(operateType)
        {
            case  operatorInsert:
            {
                setValues(tip);
                InsertSqliteTable(tip,db);
                tips_geo_component::InsertSqliteTable(tip,*this,db);
                break;
            }
            case  operatorUpdate:
            {
                setValues(tip);
                UpdateSqliteTable(tip,db);
                //tips_geo_component::UpdateSqliteTable(tip,*this,db);
                break;
            }
            case  operatorUnKnown:
                break;
        }
	}

 	Infor::Infor()
    {
        b_featureKind = 1;
        b_sourceCode = 1;
        b_reliability = 1;
        t_status = 0;
        t_payPoints = 0;
        c_isAdopted = 0;
        c_featureKind = 1;
        c_userId = 0;
        c_pid = 0;
        r_featureKind = 1;
        r_pid = 0;
        task_status = 1;
        submitStatus = 0;
        
        globalId =_id=b_sourceId=g_location="";
        g_guide=t_expectDate=t_publishDate=t_operateDate="";
        h_projectId=i_kindCode=i_name=i_address="";
        i_telephone=i_brandCode=i_memo="";
        i_proposal=i_level=i_infoType=0;
        c_denyReason=c_fid=c_project="";
        r_fid=r_similarity=f_array=geometry=displayStyle="";
    }
    int Infor::ParseJson(std::string sJson)
    {
        rapidjson::Document doc;

        doc.Parse<0>(sJson.c_str());

        if(doc.HasParseError())
        {
            Logger::LogO("HasParseError sJson:%s",sJson.c_str());
            Logger::LogD("HasParseError sJson:%s",sJson.c_str());
            return -1;
        }
       
        for(rapidjson::Document::MemberIterator ptr = doc.MemberBegin(); ptr != doc.MemberEnd(); ptr++)
        {
            std::string  sKey = (ptr->name).GetString();
            
            rapidjson::Value &valueKey = (ptr->value);

            std::string sValue = "";

            if(!((ptr->value).IsNumber() || (ptr->value).IsString()))
            {
                rapidjson::StringBuffer buffer;

                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

                valueKey.Accept(writer); 

                sValue = buffer.GetString();
            }

	        if(sKey == "i_name" && (ptr->value).IsString())
            {
                i_name = (ptr->value).GetString();
            }
            else if(sKey == "i_telephone" && (ptr->value).IsString())
            {
                i_telephone = (ptr->value).GetString();
            }
            else if(sKey == "r_features")
            {
                if(valueKey.IsArray())
                {
                    if(valueKey.Size() > 0)
                    {
                        for(int i = 0; i < valueKey.Size(); i++)
                        {
                            rapidjson::Value& childValueFeatures = valueKey[i];

                            if(childValueFeatures.IsObject())
                            {
                                if(childValueFeatures.HasMember("featureKind") && childValueFeatures["featureKind"].IsInt())
                                    r_featureKind = childValueFeatures["featureKind"].GetInt();

                                if (childValueFeatures.HasMember("pid") && childValueFeatures["pid"].IsInt())
                                    r_pid = childValueFeatures["pid"].GetInt();

                                if (childValueFeatures.HasMember("fid") && childValueFeatures["fid"].IsString())
                                    r_fid = childValueFeatures["fid"].GetString();

                                if (childValueFeatures.HasMember("similarity") && childValueFeatures["similarity"].IsString())
                                    r_similarity = childValueFeatures["similarity"].GetString();
                            }
                        }
                    }
                    else
                    {
                        r_featureKind = 1;

                        r_pid = 0;

                        r_fid = "";

                        r_similarity = "";
                    }
                }
            }
            else if(sKey == "b_sourceId" && (ptr->value).IsString())
            {
                b_sourceId = (ptr->value).GetString();
            }
            else if(sKey == "g_location" && (ptr->value).IsString())
            {
                g_location = (ptr->value).GetString();
                geometry = g_location;
            }
            else if(sKey == "i_level" && (ptr->value).IsInt())
            {
                i_level = (ptr->value).GetInt();
            }
            else if(sKey == "i_proposal" && (ptr->value).IsInt())
            {
                i_proposal = (ptr->value).GetInt();
                MaintainRelation();
            }
            else if(sKey == "h_projectId" && (ptr->value).IsString())
            {
                h_projectId = (ptr->value).GetString();
            }
            else if(sKey == "i_kindCode" && (ptr->value).IsString())
            {
                i_kindCode = (ptr->value).GetString();
            }
            else if(sKey == "i_address" && (ptr->value).IsString())
            {
                i_address = (ptr->value).GetString();
            }
            else if(sKey == "t_publishDate" && (ptr->value).IsString())
            {
                t_publishDate = (ptr->value).GetString();
            }
            else if(sKey == "i_infoType" && (ptr->value).IsInt())
            {
                i_infoType = (ptr->value).GetInt();
            }
            else if(sKey == "b_featureKind" && (ptr->value).IsInt())
            {
                b_featureKind = (ptr->value).GetInt();
            }
            else if(sKey == "t_payPoints" && (ptr->value).IsInt())
            {
                t_payPoints = (ptr->value).GetInt();
            }
            else if(sKey == "b_reliability" && (ptr->value).IsInt())
            {
                b_reliability = (ptr->value).GetInt();
            }
            else if(sKey == "f_array")
            {
                f_array = sValue;
            }
            else if(sKey == "t_expectDate" && (ptr->value).IsString())
            {
                t_expectDate = (ptr->value).GetString();
            }
            else if(sKey == "i_memo" && (ptr->value).IsString())
            {
                i_memo = (ptr->value).GetString();
            }
            else if(sKey == "g_guide")
            {
                g_guide = sValue;
            }
            else if(sKey == "_id" && (ptr->value).IsString())
            {
                _id = (ptr->value).GetString();
            }
            else if(sKey == "b_sourceCode" && (ptr->value).IsInt())
            {
                b_sourceCode = (ptr->value).GetInt();
            }
        }

        if(SetPrimaryKey(globalId) == -1)
        {
            Logger::LogO("sJson:%s",sJson.c_str());
            Logger::LogD("sJson:%s",sJson.c_str());
            Logger::LogO("SetPrimaryKey:-1, b_featureKind[%d],b_sourceCode[%d]",b_featureKind,b_sourceCode);
            Logger::LogD("SetPrimaryKey:-1, b_featureKind[%d],b_sourceCode[%d]",b_featureKind,b_sourceCode);
            
            return -1;
        }

        if(geometry.empty())
        {
            geometry = g_location;
        }
        
        displayStyle = DataTransfor::GetInfoDisplayStyle(doc);
        Logger::LogO("displayStyle:%s",displayStyle.c_str());
        Logger::LogD("displayStyle:%s",displayStyle.c_str());

        return 0;
    }

    int Infor::MaintainRelation()
    {
        if(i_proposal == 3 || r_fid.empty() || c_pid != 0)
        {
            Logger::LogO("not to maintain relation because of adding");
            Logger::LogD("not to maintain relation because of adding");
            return 0;
        }

        Editor::DataManager* dataManager = Editor::DataManager::getInstance();

        std::string dataPath= dataManager->getFileSystem()->GetCoreMapDataPath();

		if(strcmp(dataPath.c_str(),"") == 0)
		{
			Logger::LogD("Editor: GetRdLineByPid failed, Data Path Empty");
			Logger::LogO("Editor: GetRdLineByPid failed, Data Path Empty");
			return -1;
		}

        sqlite3* db = NULL;

        int rc = sqlite3_open_v2(dataPath.c_str(), &db, SQLITE_OPEN_READWRITE, NULL);

        if (rc != SQLITE_OK)
        {
            Logger::LogD("DataSource::sqlite [%s] or [%s] open failed", dataPath.c_str(), sqlite3_errmsg(db));
            Logger::LogO("DataSource::sqlite [%s] or [%s] open failed", dataPath.c_str(), sqlite3_errmsg(db));

            sqlite3_close(db);

            return -1;
        }

        std::string sql;
        
        if(!r_fid.empty())
        {
            sql= "select * from edit_pois where fid='" + r_fid + "'";
        }
        else
        {
            sql = "select * from edit_pois where pid=" + Tools::NumberToString<int>(r_pid);
        }

        sqlite3_stmt* stmt = NULL;

        rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("select %s failed [%s]", "edit_pois", sqlite3_errmsg(db));
            Logger::LogO("select %s failed [%s]", "edit_pois", sqlite3_errmsg(db));

		    return -1;
	    }
        
        Point poi;

        bool flag = false;

        if(sqlite3_step(stmt) == SQLITE_ROW)
        {
            poi.ParseSqlite(stmt);

            std::stringstream ss;
		    ss<<"Point(";
		    ss<<poi.longitude;
		    ss<<" ";
		    ss<<poi.latitude;
		    ss<<")";

            geometry = ss.str();

            poi.info_globalId = globalId;

            flag = true;
        }

        sqlite3_finalize(stmt);

        if(flag)
        {
            poi.UpdateSqliteTableWithoutTransaction(db);
        }

        sqlite3_close_v2(db);

        return 0;
    }
    std::string Infor::ToUploadJson()
    {
        Document document;
        Document docTmp;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();

        Value each_json_value(kStringType);  
        std::string each_str_value = _id;  
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);  
        document.AddMember("_id",each_json_value,allocator);

        document.AddMember("b_featureKind",b_featureKind,allocator);

        document.AddMember("b_sourceCode",b_sourceCode,allocator);

        document.AddMember("b_reliability",b_reliability,allocator);

        each_str_value = g_location;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("g_location",each_json_value,allocator);

        each_str_value = h_projectId;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("h_projectId",each_json_value,allocator);

        docTmp.Parse<0>(f_array.c_str());

        if(docTmp.HasParseError())
        {
            Logger::LogO("generate json error!!!");
        }

        document.AddMember("f_array", docTmp,allocator);

        document.AddMember("c_isAdopted",c_isAdopted,allocator);

        each_str_value = c_denyReason;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("c_denyReason",each_json_value,allocator);

        document.AddMember("c_featureKind",c_featureKind,allocator);

        document.AddMember("c_userId",c_userId,allocator);

        document.AddMember("c_pid",c_pid,allocator);

        each_str_value = c_fid;
        each_json_value.SetString(each_str_value.c_str(), each_str_value.size(),allocator);
        document.AddMember("c_fid",each_json_value,allocator);
       
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();

        return sResult;
    }

    int Infor::ParseSqlite(sqlite3_stmt *statement)
    {
        globalId = (const char*)sqlite3_column_text(statement, 0);
        _id = (const char*)sqlite3_column_text(statement, 1);
	    b_featureKind = sqlite3_column_int(statement, 2);
	    b_sourceCode = sqlite3_column_int(statement, 3);
	    b_sourceId = (const char*)sqlite3_column_text(statement, 4);
	    b_reliability = sqlite3_column_int(statement, 5);
	    g_location = (const char*)sqlite3_column_text(statement, 6);
	    g_guide = (const char*)sqlite3_column_text(statement, 7);
	    t_expectDate = (const char*)sqlite3_column_text(statement, 8);
	    t_publishDate = (const char*)sqlite3_column_text(statement, 9);
	    t_operateDate = (const char*)sqlite3_column_text(statement, 10);
	    t_status = sqlite3_column_int(statement, 11);
	    t_payPoints = sqlite3_column_int(statement, 12);
        h_projectId = (const char*)sqlite3_column_text(statement, 13);
	    i_kindCode = (const char*)sqlite3_column_text(statement, 14);
	    i_name = (const char*)sqlite3_column_text(statement, 15);
	    i_address = (const char*)sqlite3_column_text(statement, 16);
	    i_telephone = (const char*)sqlite3_column_text(statement, 17);
	    i_brandCode = (const char*)sqlite3_column_text(statement, 18);
	    i_memo = (const char*)sqlite3_column_text(statement, 19);
	    i_proposal = sqlite3_column_int(statement, 20);
	    i_level = sqlite3_column_int(statement, 21);
        i_infoType = sqlite3_column_int(statement, 22);
        c_isAdopted = sqlite3_column_int(statement, 23);
	    c_denyReason = (const char*)sqlite3_column_text(statement, 24);
	    c_featureKind = sqlite3_column_int(statement, 25);
	    c_userId = sqlite3_column_int(statement, 26);
	    c_pid = sqlite3_column_int(statement, 27);
	    c_fid = (const char*)sqlite3_column_text(statement, 28);
	    c_project = (const char*)sqlite3_column_text(statement, 29);
	    r_featureKind = sqlite3_column_int(statement, 30);
	    r_pid = sqlite3_column_int(statement, 31);
	    r_fid = (const char*)sqlite3_column_text(statement, 32);				
	    r_similarity = (const char*)sqlite3_column_text(statement, 33);
	    f_array = (const char*)sqlite3_column_text(statement, 34);

        int geo_length = sqlite3_column_bytes(statement, 35);

        if(geo_length != 0)
        {
	        const void* geo_buff = sqlite3_column_blob(statement, 35);
            EditorGeometry::WkbGeometry* wkb = (EditorGeometry::WkbGeometry*)EditorGeometry::SpatialiteGeometry::ToWKBGeometry((EditorGeometry::SpatialiteGeometry*)geo_buff);
            geometry=DataTransfor::Wkb2Wkt(wkb);
            delete wkb;
        }
        else
        {
            geometry = "";
        }

	    task_status = sqlite3_column_int(statement, 36);
	    submitStatus = sqlite3_column_int(statement, 37);

	    return 0;
    }

    int Infor::InsertSqliteTableWithoutTransaction(sqlite3* db)
    {
        sqlite3_stmt* stmt;

	    std::string sql = "insert into edit_infos values("
		    "?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
		    "?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
		    "?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
		    "?, ?, ?, ?, ?, ?, ?, ?, ?)";

	    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("insert %s failed [%s]", "Infor", sqlite3_errmsg(db));
            Logger::LogO("insert %s failed [%s]", "Infor", sqlite3_errmsg(db));

		    return false;
	    }

        sqlite3_bind_text(stmt, 1, globalId.c_str(), globalId.length(), NULL);

        sqlite3_bind_text(stmt, 2, _id.c_str(), _id.length(), NULL);

	    sqlite3_bind_int(stmt, 3, b_featureKind);

	    sqlite3_bind_int(stmt, 4, b_sourceCode);

	    sqlite3_bind_text(stmt, 5, b_sourceId.c_str(), b_sourceId.length(), NULL);

	    sqlite3_bind_int(stmt, 6, b_reliability);

        sqlite3_bind_text(stmt, 7, g_location.c_str(), g_location.length(), NULL);

	    sqlite3_bind_text(stmt, 8, g_guide.c_str(), g_guide.length(), NULL);

	    sqlite3_bind_text(stmt, 9, t_expectDate.c_str(), t_expectDate.length(), NULL);

	    sqlite3_bind_text(stmt, 10, t_publishDate.c_str(), t_publishDate.length(), NULL);

	    sqlite3_bind_text(stmt, 11, t_operateDate.c_str(), t_operateDate.length(), NULL);

	    sqlite3_bind_int(stmt, 12, t_status);

	    sqlite3_bind_int(stmt, 13, t_payPoints);

        sqlite3_bind_text(stmt, 14, h_projectId.c_str(), h_projectId.length(), NULL);

	    sqlite3_bind_text(stmt, 15, i_kindCode.c_str(), i_kindCode.length(), NULL);

	    sqlite3_bind_text(stmt, 16, i_name.c_str(), i_name.length(), NULL);

	    sqlite3_bind_text(stmt, 17, i_address.c_str(), i_address.length(), NULL);

	    sqlite3_bind_text(stmt, 18, i_telephone.c_str(), i_telephone.length(), NULL);

	    sqlite3_bind_text(stmt, 19, i_brandCode.c_str(), i_brandCode.length(), NULL);

        sqlite3_bind_text(stmt, 20, i_memo.c_str(), i_memo.length(), NULL);

	    sqlite3_bind_int(stmt, 21, i_proposal);

	    sqlite3_bind_int(stmt, 22, i_level);

	    sqlite3_bind_int(stmt, 23, i_infoType);

	    sqlite3_bind_int(stmt, 24, c_isAdopted);

	    sqlite3_bind_text(stmt, 25, c_denyReason.c_str(), c_denyReason.length(), NULL);

	    sqlite3_bind_int(stmt, 26, c_featureKind);

	    sqlite3_bind_int(stmt, 27, c_userId);

	    sqlite3_bind_int(stmt, 28, c_pid);

	    sqlite3_bind_text(stmt, 29, c_fid.c_str(), c_fid.length(), NULL);

	    sqlite3_bind_text(stmt, 30, c_project.c_str(), c_project.length(), NULL);

	    sqlite3_bind_int(stmt, 31, r_featureKind);

	    sqlite3_bind_int(stmt, 32, r_pid);

	    sqlite3_bind_text(stmt, 33, r_fid.c_str(), r_fid.length(), NULL);

	    sqlite3_bind_text(stmt, 34, r_similarity.c_str(), r_similarity.length(), NULL);

	    sqlite3_bind_blob(stmt, 35, f_array.c_str(), f_array.length(), NULL);

        EditorGeometry::WkbGeometry* wkb = NULL;

        EditorGeometry::SpatialiteGeometry* spGeo = NULL;

        if(geometry.empty())
        {
            sqlite3_bind_blob(stmt, 36, geometry.c_str(), geometry.length(), NULL);
        }
        else
        {
            wkb = DataTransfor::Wkt2Wkb(geometry);

		    spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

		    sqlite3_bind_blob(stmt, 36, spGeo, wkb->buffer_size()+39, NULL);
        }


	    sqlite3_bind_int(stmt, 37, task_status);

	    sqlite3_bind_int(stmt, 38, submitStatus);

        sqlite3_bind_text(stmt, 39, displayStyle.c_str(), displayStyle.length(), NULL);
        
        if(wkb != NULL)
        {
            delete wkb;
        }

	    if (sqlite3_step(stmt) != SQLITE_DONE)
	    {
            if(spGeo != NULL)
            {
                free(spGeo);
            }

			sqlite3_finalize(stmt);

		    Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
            Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));

            return false;
	    }

        if(spGeo != NULL)
        {
            free(spGeo);
        }

	    sqlite3_finalize(stmt);

        return 0;
    }

    int Infor::UpdateSqliteTableWithoutTransaction(sqlite3* db)
    {
        sqlite3_stmt* stmt;

	    std::string sql = "update edit_infos set "
		    "globalId=?, _id=?, b_featureKind=?, b_sourceCode=?, b_sourceId=?, b_reliability=?, g_location=?, g_guide=?, t_expectDate=?, t_publishDate=?, t_operateDate=?, t_status=?,"
		    "t_payPoints=?, h_projectId=?, i_kindCode=?, i_name=?, i_address=?, i_telephone=?, i_brandCode=?, i_memo=?, i_proposal=?, i_level=?,"
		    "i_infoType=?, c_isAdopted=?, c_denyReason=?, c_featureKind=?, c_userId=?, c_pid=?, c_fid=?, c_project=?, r_featureKind=?, r_pid=?, r_fid=?,"
		    "r_similarity=?, f_array=?, geometry=?, task_status=?, submitStatus=?, display_style=? where globalId=?";

	    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

	    if (rc != SQLITE_OK)
	    {
		    Logger::LogD("update %s failed [%s]", "Infor", sqlite3_errmsg(db));
            Logger::LogO("update %s failed [%s]", "Infor", sqlite3_errmsg(db));

		    return false;
	    }

        sqlite3_bind_text(stmt, 1, globalId.c_str(), globalId.length(), NULL);
        sqlite3_bind_text(stmt, 2, _id.c_str(), _id.length(), NULL);

	    sqlite3_bind_int(stmt, 3, b_featureKind);

	    sqlite3_bind_int(stmt, 4, b_sourceCode);

	    sqlite3_bind_text(stmt, 5, b_sourceId.c_str(), b_sourceId.length(), NULL);

	    sqlite3_bind_int(stmt, 6, b_reliability);

        sqlite3_bind_text(stmt, 7, g_location.c_str(), g_location.length(), NULL);

	    sqlite3_bind_text(stmt, 8, g_guide.c_str(), g_guide.length(), NULL);

	    sqlite3_bind_text(stmt, 9, t_expectDate.c_str(), t_expectDate.length(), NULL);

	    sqlite3_bind_text(stmt, 10, t_publishDate.c_str(), t_publishDate.length(), NULL);

	    sqlite3_bind_text(stmt, 11, t_operateDate.c_str(), t_operateDate.length(), NULL);

	    sqlite3_bind_int(stmt, 12, t_status);

	    sqlite3_bind_int(stmt, 13, t_payPoints);

        sqlite3_bind_text(stmt, 14, h_projectId.c_str(), h_projectId.length(), NULL);

	    sqlite3_bind_text(stmt, 15, i_kindCode.c_str(), i_kindCode.length(), NULL);

	    sqlite3_bind_text(stmt, 16, i_name.c_str(), i_name.length(), NULL);

	    sqlite3_bind_text(stmt, 17, i_address.c_str(), i_address.length(), NULL);

	    sqlite3_bind_text(stmt, 18, i_telephone.c_str(), i_telephone.length(), NULL);

	    sqlite3_bind_text(stmt, 19, i_brandCode.c_str(), i_brandCode.length(), NULL);

        sqlite3_bind_text(stmt, 20, i_memo.c_str(), i_memo.length(), NULL);

	    sqlite3_bind_int(stmt, 21, i_proposal);

	    sqlite3_bind_int(stmt, 22, i_level);

	    sqlite3_bind_int(stmt, 23, i_infoType);

	    sqlite3_bind_int(stmt, 24, c_isAdopted);

	    sqlite3_bind_text(stmt, 25, c_denyReason.c_str(), c_denyReason.length(), NULL);

	    sqlite3_bind_int(stmt, 26, c_featureKind);

	    sqlite3_bind_int(stmt, 27, c_userId);

	    sqlite3_bind_int(stmt, 28, c_pid);

	    sqlite3_bind_text(stmt, 29, c_fid.c_str(), c_fid.length(), NULL);

	    sqlite3_bind_text(stmt, 30, c_project.c_str(), c_project.length(), NULL);

	    sqlite3_bind_int(stmt, 31, r_featureKind);

	    sqlite3_bind_int(stmt, 32, r_pid);

	    sqlite3_bind_text(stmt, 33, r_fid.c_str(), r_fid.length(), NULL);

	    sqlite3_bind_text(stmt, 34, r_similarity.c_str(), r_similarity.length(), NULL);

	    sqlite3_bind_blob(stmt, 35, f_array.c_str(), f_array.length(), NULL);

        EditorGeometry::WkbGeometry* wkb = NULL;

        EditorGeometry::SpatialiteGeometry* spGeo = NULL;

        if(geometry.empty())
        {
            sqlite3_bind_blob(stmt, 36, geometry.c_str(), geometry.length(), NULL);
        }
        else
        {
            wkb = DataTransfor::Wkt2Wkb(geometry);

		    spGeo = EditorGeometry::SpatialiteGeometry::FromWKBGeometry(wkb);

		    sqlite3_bind_blob(stmt, 36, spGeo, wkb->buffer_size()+39, NULL);
        }


	    sqlite3_bind_int(stmt, 37, task_status);

	    sqlite3_bind_int(stmt, 38, submitStatus);

        sqlite3_bind_text(stmt, 39, displayStyle.c_str(), displayStyle.length(), NULL);
        
        sqlite3_bind_text(stmt, 40, globalId.c_str(), globalId.length(), NULL);

        if(wkb != NULL)
        {
            delete wkb;
        }

	    if (sqlite3_step(stmt) != SQLITE_DONE)
	    {
            if(spGeo != NULL)
            {
                free(spGeo);
            }

			sqlite3_finalize(stmt);

		    Logger::LogD("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));
            Logger::LogO("sqlite3_step[%s][%s]", sql.c_str(), sqlite3_errmsg(db));

            return false;
	    }

        if(spGeo != NULL)
        {
            free(spGeo);
        }

	    sqlite3_finalize(stmt);

        return 0;
    }

    int Infor::SetPrimaryKey(std::string& id)
    {
        std::string sTmp_b_featureKind, sTmp_b_sourceCode;

        if(b_featureKind < 0)
        {
            return -1;
        }
        if(b_featureKind >=0 && b_featureKind <= 9)
        {
            sTmp_b_featureKind = "0" + Tools::NumberToString(b_featureKind);
        }
        else 
        {
            sTmp_b_featureKind = Tools::NumberToString(b_featureKind);
        }

        if(b_sourceCode < 0)
        {
            return -1;
        }
        if(b_sourceCode >=0 && b_sourceCode <= 9)
        {
            sTmp_b_sourceCode = "0" + Tools::NumberToString(b_featureKind);
        }
        else 
        {
            sTmp_b_sourceCode = Tools::NumberToString(b_featureKind);
        }

        id = sTmp_b_featureKind + sTmp_b_sourceCode + b_sourceId;

        return 0;
    }
}
