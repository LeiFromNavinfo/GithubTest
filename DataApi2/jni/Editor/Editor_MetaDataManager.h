#ifndef EDITOR_PARAKINDINFO_H
#define EDITOR_PARAKINDINFO_H

#include "Editor.h"

namespace Editor
{
    class EDITOR_API MetaDataManager
    {
    private:
        MetaDataManager();

    public:
        static MetaDataManager* GetInstance();
        ~MetaDataManager();

    private:
        static MetaDataManager*     m_pInstance;

        sqlite3*                    m_pDb;

    private:
        bool                        Init();

        bool                        openSqlite(std::string path);

    public:

        std::vector<std::string>    OperateMetaData(std::string sTable, std::string condition, int type);

        std::string                 GenerateSql(std::string sTable, std::string condition, int type);
    };
}

#endif