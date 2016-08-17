#include "Editor.h"

namespace Editor
{	
	DataFileSystem::DataFileSystem()
	{
	}

	DataFileSystem::~DataFileSystem()
	{
	}

    void            DataFileSystem::SetRootPath(const char* rootPath)
	{
	    this->m_pRootPath = rootPath;

        if(!m_pRootPath.empty() && !m_pUserId.empty())
        {
            InitUserDataBase();
            InitUserGDBDataBase();
        }
	}

    void            DataFileSystem::SetCurrentUser(const char* userId)
    {
        this->m_pUserId = userId;

        if(!m_pRootPath.empty() && !m_pUserId.empty())
        {
            InitUserDataBase();
            InitUserGDBDataBase();
        }

    }

    std::string DataFileSystem::GetCurrentUserId()
    {
        return this->m_pUserId;
    }

    std::string     DataFileSystem::GetCurrentUserFolderPath()
	{
        if(strcmp(this->m_pRootPath.c_str(),"") == 0 || strcmp(this->m_pUserId.c_str(),"") == 0)
        {
            return "";
        }
        std::string folderPath;

#ifdef WIN32
        folderPath = this->m_pRootPath + "\\data\\collect\\" + this->m_pUserId;
#else
        folderPath = this->m_pRootPath + "/data/collect/" + this->m_pUserId;
#endif
        return folderPath;
	}

    std::string     DataFileSystem::GetCoreMapDataPath()
	{
        std::string userFolderPath = GetCurrentUserFolderPath();

        if(strcmp(userFolderPath.c_str(),"") == 0)
        {
            return "";
        }
        std::string folderPath;

#ifdef WIN32
        folderPath = GetCurrentUserFolderPath() + "\\coremap.sqlite";
#else
        folderPath = GetCurrentUserFolderPath() + "/coremap.sqlite";
#endif
        if (0 != access(folderPath.c_str(), F_OK))
        {
            std::string emptyFilePath;

            #ifdef WIN32
                emptyFilePath = GetTemplateFolderPath() + "\\coremap.sqlite";
            #else
                emptyFilePath = GetTemplateFolderPath() + "/coremap.sqlite";
            #endif
                if (0 != access(emptyFilePath.c_str(), F_OK))
                {
                    folderPath = "";
                }
                else
                {
                    int rst = Tools::CopyBinaryFile(emptyFilePath.c_str(), folderPath.c_str());

                    if(rst != 0)
                    {
                        folderPath = "";
                    }
                }
        }
        return folderPath;
	}
    
     std::string DataFileSystem::GetTempGDBDataDir()
    {
        std::string folderPath;
        
#ifdef WIN32
        folderPath = m_pRootPath + "\\data\\collect";
#else
        folderPath = m_pRootPath + "/data/collect";
#endif
        return folderPath;
    }
    
    std::string DataFileSystem::GetEditorGDBDataDir()
    {
        std::string folderPath;
        
#ifdef WIN32
        folderPath = m_pRootPath + "\\data\\gdb";
#else
        folderPath = m_pRootPath + "/data/gdb";
#endif
        return folderPath;
    }
    
    std::string DataFileSystem::GetTempGDBDataFile()
    {
        std::string folderPath;
#ifdef WIN32
        folderPath = GetTempGDBDataDir() + "\\gdb.sqlite";
#else
        folderPath = GetTempGDBDataDir() + "/gdb.sqlite";
#endif
        return folderPath;
    }
    
    std::string DataFileSystem::GetEditorGDBDataFile()
    {
        std::string folderPath;
#ifdef WIN32
        folderPath = GetEditorGDBDataDir() + "\\gdb.sqlite";
#else
        folderPath = GetEditorGDBDataDir() + "/gdb.sqlite";
#endif
        return folderPath;
    }

    std::string     DataFileSystem::GetProjectDataPath()
    {
        if(strcmp(this->m_pRootPath.c_str(),"") == 0)
        {
            return "";
        }
        std::string folderPath;

#ifdef WIN32
        folderPath = this->m_pRootPath + "\\data\\collect\\project.sqlite";
#else
        folderPath = this->m_pRootPath + "/data/collect/project.sqlite";
#endif
        return folderPath;
    }

    std::string     DataFileSystem::GetPoiPhotoPath()
    {
        std::string userFolderPath = GetCurrentUserFolderPath();

        if(userFolderPath.empty())
        {
            return "";
        }
#ifdef WIN32
        return userFolderPath + "\\photo\\poi\\";
#else
        return userFolderPath + "/photo/poi/";
#endif
    }
    
    std::string DataFileSystem::GetTipsPhotoPath()
    {
        std::string userFolderPath = GetCurrentUserFolderPath();
        
        if(userFolderPath.empty())
        {
            return "";
        }
#ifdef WIN32
        return userFolderPath + "\\photo\\tips\\";
#else
        return userFolderPath + "/photo/tips/";
#endif
    }

    std::string DataFileSystem::GetMetaPath()
    {
        std::string userFolderPath = GetCurrentUserFolderPath();

        if(strcmp(userFolderPath.c_str(),"") == 0)
        {
            return "";
        }

#ifdef WIN32
        std::string folderPath = this->m_pRootPath + "\\data\\collect\\metadata.sqlite";
#else
        std::string folderPath = this->m_pRootPath + "/data/collect/metadata.sqlite";
#endif
        
        return folderPath;
    }

    std::string DataFileSystem::GetPoiUploadPath()
    {
        std::string userFolderPath = GetCurrentUserFolderPath();

        if(strcmp(userFolderPath.c_str(),"") == 0)
        {
            return "";
        }

#ifdef WIN32
        std::string folderPath = GetCurrentUserFolderPath() + "\\upload\\poi\\";
#else
        std::string folderPath = GetCurrentUserFolderPath() + "/upload/poi/";
#endif
        
        return folderPath;
    }
    
    std::string DataFileSystem::GetTrackUploadPath()
    {
        std::string userFolderPath = GetCurrentUserFolderPath();
        if(strcmp(userFolderPath.c_str(),"") == 0)
        {
            return "";
        }
        
#ifdef WIN32
        std::string folderPath = GetCurrentUserFolderPath() + "\\upload\\track\\";
#else
        std::string folderPath = GetCurrentUserFolderPath() + "/upload/track/";
#endif
        
        return folderPath;
    }

    std::string DataFileSystem::GetUploadPath()
    {
    	std::string userFolderPath = GetCurrentUserFolderPath();

		if(strcmp(userFolderPath.c_str(),"") == 0)
		{
			return "";
		}

		std::string folderPath;

	#ifdef WIN32
		folderPath = GetCurrentUserFolderPath() + "\\Upload";
	#else
		folderPath = GetCurrentUserFolderPath() + "/Upload";
	#endif

		return folderPath;
    }
	
	std::string DataFileSystem::GetDownloadPath()
    {
        std::string userFolderPath = GetCurrentUserFolderPath();

        if(strcmp(userFolderPath.c_str(),"") == 0)
        {
            return "";
        }

#ifdef WIN32
        std::string folderPath = GetCurrentUserFolderPath() + "\\download\\";
#else
        std::string folderPath = GetCurrentUserFolderPath() + "/download/";
#endif
        
        return folderPath;
    }

    void DataFileSystem::InitUserDataBase()
    {
        Editor::DataManager* pDataManager = Editor::DataManager::getInstance();

	    Editor::DataSource* dataSource = pDataManager->getDataSource(DATASOURCE_COREMAP);

        std::string spath = GetCoreMapDataPath();

        dataSource->InitDataBaseToWal(spath);
    }
    
    void DataFileSystem::InitUserGDBDataBase()
    {
        Editor::DataManager* pDataManager = Editor::DataManager::getInstance();
        
        Editor::DataSource* dataSource = pDataManager->getDataSource(DATASOURCE_GDB);
        
        std::string spath = GetEditorGDBDataFile();
        
        dataSource->InitDataBaseToWal(spath);
    }

    std::string DataFileSystem::GetTemplateFolderPath()
    {
        std::string folderPath;
#ifdef WIN32
        folderPath = this->m_pRootPath + "\\data\\collect\\template";
#else
        folderPath = this->m_pRootPath + "/data/collect/template";
#endif
        return folderPath;
    }

}
