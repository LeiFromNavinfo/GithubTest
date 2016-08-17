#include "Editor_ResumeProcessor.h"

namespace Editor
{
    EditHistory::EditHistory()
    {
	    
    }

    EditHistory::EditHistory(const char* project, int user)
    {
	    sourceName = "Android";

		sourceProject = project;

		sourceTask = "0";

		editOperator.user = user;
    }

    bool EditHistory::ParseJson(rapidjson::Value& json)
    {
        if(json.IsObject())
        {
            if(json.HasMember("mergeDate"))
            {
                mergeDate = json["mergeDate"].GetString();
            }

            if(json.HasMember("sourceName"))
            {
                sourceName = json["sourceName"].GetString();
            }

            if(json.HasMember("sourceProject"))
            {
                sourceProject = json["sourceProject"].GetString();
            }

            if(json.HasMember("sourceTask"))
            {
                sourceTask = json["sourceTask"].GetString();
            }

            if(json.HasMember("operation"))
            {
                operation = json["operation"].GetInt();
            }

            rapidjson::Value& operatorValue = json["operator"];

            if(operatorValue.IsObject())
            {
                EditOperator eo;

                eo.ParseJson(operatorValue);

                editOperator = eo;
            }

            rapidjson::Value& mergeContentsValue = json["mergeContents"];

            if(mergeContentsValue.IsArray())
            {
                for(int i = 0; i < mergeContentsValue.Size(); i++)
                {
                    EditContent ec;

                    ec.Parsejson(mergeContentsValue[i]);

                    contents.push_back(ec);
                }
            }
        }

        return true;
    }

    void EditHistory::Serialize(rapidjson::Writer<StringBuffer>& writer) const
    {
    	writer.StartObject();

    	writer.Key("sourceName");

    	writer.String(sourceName.c_str());

    	writer.Key("sourceProject");

    	writer.String(sourceProject.c_str());

    	writer.Key("sourceTask");

    	writer.String(sourceTask.c_str());

    	writer.Key("operator");

    	editOperator.Serialize(writer);

    	writer.Key("operation");

    	writer.Int(operation);

    	writer.Key("mergeDate");

    	writer.String(mergeDate.c_str());

    	writer.Key("mergeContents");

    	writer.StartArray();

    	for(int i = 0; i < contents.size(); i++)
    	{
    		contents[i].Serialize(writer);
		}

    	writer.EndArray();

    	writer.EndObject();
    }

    std::vector<std::string> EditHistory::ParseContent(std::string sIn)
    {
        std::vector<std::string> vResult;

        rapidjson::Document doc;

        doc.Parse<0>(sIn.c_str());

        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        if(doc.HasParseError())
        {
            return vResult;
        }

        if(!doc.IsArray())
        {
            return vResult;
        }

        for(rapidjson::SizeType i = 0; i < doc.Size(); i++)
        {
            rapidjson::Value& p = doc[i];

            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);  
            p.Accept(writer);  
            std::string sResult = buffer.GetString();

            // todo  parse EditContent

            vResult.push_back(sResult);
        }

        return vResult;
    }

	std::string EditHistory::ToJsonString()
    {
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        Serialize(writer);
        std::string sResult = buffer.GetString();

        return sResult;
    }
}
