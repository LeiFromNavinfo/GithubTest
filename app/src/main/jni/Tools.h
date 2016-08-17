#ifndef Tsinghua_Tools_h
#define Tsinghua_Tools_h

#include "Editor_Define.h"
#include "Logger.h"

#ifdef WIN32
#include <io.h>
#include <direct.h>
#define PATH_MAX 260
#endif

#include <errno.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <zip.h>

namespace Tools
{
		template <typename T>
        std::string						   NumberToString( T val )
		{
			std::ostringstream ss;
            
			ss<<std::fixed<<val;
			return ss.str();
		}

        EDITOR_API void                    WatchTimerStart();
        
        EDITOR_API unsigned long           WatchTimerStop();
        
        EDITOR_API std::string             ToUpper(std::string& str);
        
        EDITOR_API std::string             ToLower(std::string& str);
        
        EDITOR_API std::string             TrimRight(std::string &str, std::string ws="\t\n");
        
        EDITOR_API std::string             TrimLeft(std::string &str, std::string ws=" \t\n");
        
        EDITOR_API std::string             Trim(std::string &str, std::string ws=" \t\n");
        
        EDITOR_API bool                    CaseInsensitiveCompare(std::string& left, std::string right);
        
        EDITOR_API std::vector<std::string> StringSplit(std::string &source, std::string delimiter = " ", bool keepEmpty = false);

        EDITOR_API bool					 CheckFilePath(const char* filePath);

        EDITOR_API int					 CopyFile(const char* srcFile, const char* destFile);

        EDITOR_API void					 DelDir(const char* dirPath);

        EDITOR_API std::string 			 GetCurrentDateTime();

        EDITOR_API std::string		     GetCurrentDate();

        /// 大地距离的精确计算,厘米级精度//
		/// Calculate the geodetic curve between two points on WGS84 ellipsoid.
		/// This is the solution to the inverse geodetic problem.
		/// All equation numbers refer back to Vincenty's publication:
		/// See http://www.ngs.noaa.gov/PUBS_LIB/inverse.pdf
		EDITOR_API double				 calculate_accurate_geodetic_distance(double latitude1, double longitude1, double latitude2, double longitude2);

		EDITOR_API std::vector< std::pair<int, int> > SpiralArray(int xmin, int xmax, int ymin, int ymax);

		EDITOR_API void					StringReplace(std::string& strBase, std::string strSrc, std::string strDes);

		EDITOR_API std::string			DoubleToString(const double t);

        EDITOR_API int      			StringToNum(const std::string sIn);

        EDITOR_API size_t			    OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid);

        EDITOR_API std::string	        ZipFolderPath(const char* folderPath, const char* zipPath);

        EDITOR_API bool                 ZipDir(const char* dirPath, struct zip* zip, const char* parentDirRelativePath);

        EDITOR_API std::string          UnZipDir(const char* path, const char* outFolderPath);

        EDITOR_API int                  CopyBinaryFile(const char* source, const char* dest);

        EDITOR_API std::string          ReplaceString(std::string& source, const std::string& symbol, const std::string newValue);

        EDITOR_API std::string          geohash_encode(double lat, double lng, int precision);

        EDITOR_API std::string          GetTipsRowkey(const std::string& type);

        EDITOR_API std::string			GenerateProgressInfo(std::string flag, std::string info);
    
        EDITOR_API int					CountLines(const char* path);
    
        EDITOR_API unsigned long		GetFileSize(const char* path);
    
        EDITOR_API bool 				HttpGet(const char* url, std::string& response);

        EDITOR_API bool 				HttpGet(const char* url, std::string& response, int& code);
        EDITOR_API bool                 HttpMultiPost(const char* url, char** pName, char** pContents, int iParamLen, std::string& response);

        EDITOR_API bool                 HttpPost(const char* url, std::string& response, const std::string& data);
        EDITOR_API bool                 HttpPost(const char* url, std::string& response, const std::string& data, int& code);
        
		EDITOR_API std::string 	    GenerateUuid();

		EDITOR_API int                CheckRelationTips(int sourceType, const std::string& tipsDeep,  const std::string& surveyLineRowkey, std::string& reId, bool& isReTips);

        /** 
         * @brief 从列表中拼接sql中的in语句
         * @param ids id列表
         * @param type 0:int, 1:string
         * @return in语句
        */
        EDITOR_API std::string                    GetInStatement(const std::vector<std::string>& ids, int type);

        EDITOR_API std::string                    GernerateTipsProgressInfo(const std::string& flag, const std::string& info, const std::string& information, bool isImport = false);
    
		EDITOR_API std::string          GenerateWktPoint(double lon, double lat);
    
    
        /*
         *  @brief  替换文件
         *  @param srcFilePath 源文件；
         *  @param destFilePath 目标文件；
         *
         *  @return 0 替换成功，-1 替换失败
         *
         */
        EDITOR_API int ReplaceFile(const char* srcFilePath, const char* destFilePath);
    
        /*
         *  @brief  移除目录下所有和filekey相似的文件
         *  @param dir     文件目录
         *  @param filekey 相似key
         *
         *
         */
        EDITOR_API void RemoveSimilarFilesInDir(const char *dir, const char* filekey);
    
    
        /*
         *  @brief  拷贝srcdir目录下所有和filekey相似的文件，到destdir目录
         *  @param srcdir     源文件目录
         *  @param filekey    相似key
         *  @param destdir    目标文件目录
         *
         */
        EDITOR_API void CopySimilarFiles2Dir(const char *srcdir, const char* filekey, const char *destdir);
    
        //获得子序列
		template<class T>
		EDITOR_API std::vector<T> GetSubVector(const std::vector<T>& vec, int beg, int end)
		{
			std::vector<T> ret;

			if(vec.size()<end)
			{
				return ret;
			}

			for(int i=beg;i<=end;i++)
			{
				ret.push_back(vec.at(i));
			}

			return ret;
		}
}

#endif
