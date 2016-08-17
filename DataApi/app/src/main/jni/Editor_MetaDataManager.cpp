#include "Editor_MetaDataManager.h"

namespace Editor
{
    MetaDataManager* MetaDataManager::m_pInstance = NULL;

    MetaDataManager::MetaDataManager() 
    {
        m_pDb = NULL;

        Init();
    }

    MetaDataManager::~MetaDataManager() 
    {
        if(m_pDb != NULL)
        {
            sqlite3_close(m_pDb);

            m_pDb = NULL;
        }
    }

    MetaDataManager* MetaDataManager::GetInstance()
    {
        if(m_pInstance == NULL)
        {
            m_pInstance = new MetaDataManager();
        }

        return m_pInstance;
    }

    bool MetaDataManager::Init()
    {
        Editor::DataManager* pDataManager = Editor::DataManager::getInstance();

        Editor::DataFileSystem* pFileSystem = pDataManager->getFileSystem();

        std::string sMetadataPath = pFileSystem->GetMetaPath();

        if(openSqlite(sMetadataPath) == false)
        {
            return false;
        }

        return true;
    }

    bool MetaDataManager::openSqlite(std::string path)
    {
        if(path.empty())
        {
            return false;
        }

        int rc = sqlite3_open_v2(path.c_str(), &m_pDb, SQLITE_OPEN_READWRITE, NULL);

        if (rc != SQLITE_OK)
        {
            Logger::LogD("DataSource::sqlite [%s] or [%s] open failed", path.c_str(), sqlite3_errmsg(m_pDb));
            Logger::LogO("DataSource::sqlite [%s] or [%s] open failed", path.c_str(), sqlite3_errmsg(m_pDb));

            sqlite3_close(m_pDb);

            return false;
        }

        return true;
    }

    std::vector<std::string> MetaDataManager::OperateMetaData(std::string sTable, std::string condition, int type)
    {
        std::vector<std::string> vResult;

        sqlite3_stmt* stmt = NULL;

        std::string sql = GenerateSql(sTable, condition, type);

        int rc = sqlite3_prepare_v2(m_pDb, sql.c_str(), -1, &stmt, NULL);

        if(rc != SQLITE_OK)
        {
            Logger::LogD("EXCUTE SQL [%s] failed: \n", sql.c_str());
            Logger::LogO("EXCUTE SQL [%s] failed: \n", sql.c_str());

            return vResult;
        }

        while(sqlite3_step(stmt) == SQLITE_ROW)
        {
            if(sTable == "charging_carid")
            {
                Model::ChargingCarid cc;

                cc.ParseSqlite(stmt);

                vResult.push_back(cc.ToJson());
            }
            else if(sTable == "ci_para_chain")
            {
                Model::ParaChain pc;

                pc.ParseSqlite(stmt);

                vResult.push_back(pc.ToJson());
            }
            else if(sTable == "ci_para_control")
            {
                Model::ParaControl pctl;

                pctl.ParseSqlite(stmt);

                vResult.push_back(pctl.ToJson());
            }
            else if(sTable == "ci_para_food")
            {
                Model::ParaFood pf;

                pf.ParseSqlite(stmt);

                vResult.push_back(pf.ToJson());
            }
            else if(sTable == "ci_para_icon")
            {
                Model::ParaIcon pi;

                pi.ParseSqlite(stmt);

                vResult.push_back(pi.ToJson());
            }
            else if(sTable == "ci_para_kind")
            {
                Model::ParaKind pk;

                pk.ParseSqlite(stmt);

                vResult.push_back(pk.ToJson());
            }
            else if(sTable == "ci_para_kind_chain")
            {
                Model::ParaKindChain pkc;

                pkc.ParseSqlite(stmt);

                vResult.push_back(pkc.ToJson());
            }
            else if(sTable == "ci_para_kind_medium")
            {
                Model::ParaKindMedium pkm;

                pkm.ParseSqlite(stmt);

                vResult.push_back(pkm.ToJson());
            }
            else if(sTable == "ci_para_kind_top")
            {
                Model::ParaKindTop pkt;

                pkt.ParseSqlite(stmt);

                vResult.push_back(pkt.ToJson());
            }
            else if(sTable == "ci_para_sensitive_words")
            {
                Model::ParaSensitiveWords psw;

                psw.ParseSqlite(stmt);

                vResult.push_back(psw.ToJson());
            }
            else if(sTable == "ci_para_tel")
            {
                Model::ParaTel pt;

                pt.ParseSqlite(stmt);

                vResult.push_back(pt.ToJson());
            }
        }

        sqlite3_finalize(stmt);	

        return vResult;
    }

    std::string MetaDataManager::GenerateSql(std::string sTable, std::string condition, int type)
    {
        std::string sOut;

        sOut = "select * from " + sTable + " where " + condition;

        return sOut;
    }
    
} // namesapce
