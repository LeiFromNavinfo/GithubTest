#ifndef EDITOR_RESUME_PROCESSOR_H
#define EDITOR_RESUME_PROCESSOR_H

#include "Editor.h"
#include "model.h"

namespace Editor
{
    class EditCotent;
    class EditOperator;
    class EditHistory;
    class EditAttrs;

    class EDITOR_API EditContent
	{
	public:
		EditContent();

		EditContent(int oldLifecycle, int newLifecycle);

		EditContent(std::string fieldName, std::string oldVal, std::string newVal);

	public:

        bool Parsejson(rapidjson::Value& json);

        void Serialize(rapidjson::Writer<StringBuffer>& writer) const;

	public:
		int validationMethod;

		std::string newValue;

		std::string oldValue;
	};

	class EDITOR_API EditOperator
	{
	public:
		EditOperator();

	public:

        bool ParseJson(rapidjson::Value& json);

        void Serialize(rapidjson::Writer<StringBuffer>& writer) const;

	public:
		int user;

		int role;
	};

    class EDITOR_API EditHistory
    {
    public:
		EditHistory();

		EditHistory(const char* project, int user);

	public:

        bool ParseJson(rapidjson::Value& json);

        void Serialize(rapidjson::Writer<StringBuffer>& writer) const;

        std::vector<std::string> ParseContent(std::string sIn);

		std::string ToJsonString();

	public:
		std::string sourceName;

		std::string sourceProject;

		std::string sourceTask;

		int operation;

		std::string mergeDate;

		EditOperator editOperator;

		std::vector<EditContent> contents;
    };

    class EDITOR_API EditAttrs
	{
    public:
        EditAttrs();

	public:
        void Serialize(rapidjson::Writer<StringBuffer>& writer) const;

		std::string ToJsonString();

		bool ParseJsonForExtractionPoi(std::string json, std::string& lastMergeDate);

        bool ParseJson(std::string json);

		std::string AddEditHistory(const std::string json, EditHistory history);

		std::string UpdateEditHistory(const std::string json, int& lifecycle, std::string& mergeDate, std::string& project, std::string user);	//for undelete

		bool GenerateUpdateEditHistory(Editor::EditHistory& eh, std::string& oldJson, const std::string& newJson);

		std::string GetValueString(rapidjson::Value& value);

	public:
		std::vector<EditHistory> EditHistories;
	};

} // namespace

#endif
