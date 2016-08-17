#include "Tools.h"
#include <sys/stat.h>
#include <fcntl.h>
#include "curl.h"
#include <fstream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "Logger.h"
#include <document.h>
#include <stringbuffer.h>
#include <writer.h>
#include "model.h"

#ifndef WIN32
    #include <sys/time.h>
	#include <dirent.h>
	#include <math.h>
    #include <cmath>
#else
    #include <windows.h>
    #include <iomanip>
	#define _USE_MATH_DEFINES
	#include <math.h>
	#ifndef M_PI
	#define M_PI       3.14159265358979323846
	#endif
#endif

#define MAX_LAT             90.0
#define MIN_LAT             -90.0

#define MAX_LONG            180.0
#define MIN_LONG            -180.0

namespace Tools
{
    typedef struct IntervalStruct {
        double high;
        double low;
    }Interval;

    static char char_map[33] =  "0123456789bcdefghjkmnpqrstuvwxyz";

    long double _SecondsOfStart;
        
    long double _SecondsOfStop;
        
    void                    MeasureTime(long double& measure)
    {
#ifdef WIN32
        FILETIME ft;
        LARGE_INTEGER intervals;
            
        GetSystemTimeAsFileTime(&ft);
        intervals.LowPart = ft.dwLowDateTime;
        intervals.HighPart = ft.dwHighDateTime;
            
        measure = intervals.QuadPart;
        measure -= 116444736000000000.0;
        measure /= 10000000.0;
#else
        struct timeval tv;
            
        gettimeofday(&tv, NULL);
            
        measure = tv.tv_usec;
        measure /= 1000000.0;
        measure += tv.tv_sec;
#endif
    }
        
    void                    WatchTimerStart()
    {
        _SecondsOfStart = 0.0f;
            
        _SecondsOfStop = 0.0f;
            
        MeasureTime(_SecondsOfStart);
    }
        
    unsigned long           WatchTimerStop()
    {
        MeasureTime(_SecondsOfStop);
            
        return (_SecondsOfStop-_SecondsOfStart)*1000;
    }
      
    std::string             ToUpper(std::string& str)
    {
        std::string ret = str;
            
        std::transform(ret.begin(), ret.end(), ret.begin(), ::toupper);
            
        return ret;
    }
        
    std::string             ToLower(std::string& str)
    {
        std::string ret = str;
            
        std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
            
        return ret;
    }
        
    std::string             TrimRight(std::string &str, std::string ws)
    {
        std::size_t found = str.find_last_not_of(ws);
            
        if (found != std::string::npos)
            str.erase(found+1);
        else
            str.clear();
            
        return str;
    }
        
    std::string             TrimLeft(std::string &str, std::string ws)
    {
        std::size_t found = str.find_first_not_of(ws);
            
        if (found != std::string::npos)
            str.erase(0, found);
        else
            str.clear();
            
        return str;
    }
        
    std::string             Trim(std::string &str, std::string ws)
    {
        std::string ret = TrimRight(str, ws);
            
        return TrimLeft(ret , ws);
    }
        
    bool                    CaseInsensitiveCompare(std::string& left, std::string right)
    {
        std::string upLeft = ToUpper(left);
            
        std::string upRight = ToUpper(right);
            
        return upLeft.compare(upRight)==0;
    }
        
    std::vector<std::string> StringSplit(std::string &source, std::string delimiter, bool keepEmpty)
    {
        std::vector<std::string> results;
            
        std::size_t prev = 0;
        std::size_t next = 0;
            
        while ((next = source.find_first_of(delimiter, prev)) != std::string::npos)
        {
            if (keepEmpty || (next - prev != 0))
            {
                results.push_back(source.substr(prev, next - prev));
            }
            prev = next + 1;
        }
            
        if (prev < source.size())
        {
            results.push_back(source.substr(prev));
        }
            
        return results;
    }

#ifdef WIN32
	bool CheckFilePath(const char* filePath)
    {
        int length = strlen(filePath);

		char* temp = (char*)malloc(sizeof(char)*(length+1));

		temp[length] = '\0';

		strncpy(temp, filePath, length);

		for (int i=0; i<length; i++)
		{
			if (temp[i] == '\\' && i > 0)
			{
				temp[i] = '\0';

				if (0 != access(temp, 00))
				{
					if (0 != mkdir(temp))
					{
						free(temp);

						return false;
					}
				}

				temp[i] = '\\';
			}
		}

		free(temp);

		return true;
    }
#elif defined(__APPLE__)
    bool CheckFilePath(const char* filePath)
    {
        int length = strlen(filePath);
        
        char temp[length+1];
        
        temp[length] = '\0';
        
        
        if (0 == access(filePath, R_OK))
        {
            return true;
        }

        strncpy(temp, filePath, length);

        bool foundWritable = false;
        
        for (int i=0; i<length; i++)
        {
            if (temp[i] == '/' && i > 0)
            {
                temp[i] = '\0';
                
                if (!foundWritable)
                {
                    if (0 == access(temp, R_OK))
                    {
                        foundWritable = true;
                    }
                }
                else
                {
                    if (0 != access(temp, R_OK))
                    {
                        if (0 != mkdir(temp, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IWOTH))
                        {
                            Logger::LogD("temp:%s", temp);
                            return false;
                        }
                    }
                }
                
                temp[i] = '/';
            }
        }
        
        return foundWritable;
    }
    
#else
    bool CheckFilePath(const char* filePath)
    {
        int length = strlen(filePath);

		char temp[length+1];

		temp[length] = '\0';
        
		strncpy(temp, filePath, length);

		for (int i=0; i<length; i++)
		{
			if (temp[i] == '/' && i > 0)
			{
				temp[i] = '\0';

				if (0 != access(temp, R_OK))
				{
					if (0 != mkdir(temp, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IWOTH))
					{
                        Logger::LogD("temp:%s", temp);
						return false;
					}
				}

				temp[i] = '/';
			}
		}

		return true;
    }
#endif

    int CopyFile(const char* srcFile, const char* destFile)
    {
		int from_fd, to_fd;

		int bytes_read, bytes_write;

		if ((from_fd=open(srcFile, O_RDONLY)) == -1)
		{
			return -1;
		}

		CheckFilePath(destFile);

#ifdef WIN32
		if ((to_fd=open(destFile, O_WRONLY|O_CREAT, S_IREAD|S_IWRITE)) == -1)
#else
		if ((to_fd=open(destFile, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR)) == -1)
#endif
		{
			return -2;
		}

		char* buffer = (char*)::malloc(sizeof(char) * 1024000);

		char* ptr;

		bool flag = true;

		while (bytes_read = read(from_fd, buffer, 1024000))
		{
			if ((bytes_read == -1) && (errno != EINTR))
			{
				flag = false;

				break;
			}
			else if (bytes_read > 0)
			{
				ptr = buffer;

				while (bytes_write = write(to_fd, ptr, bytes_read))
				{
					if ((bytes_write == -1) && (errno != EINTR))
					{
						flag = false;

						break;
					}
					else if (bytes_write == bytes_read)
					{
						break;
					}
					else if (bytes_write > 0)
					{
						ptr += bytes_write;

						bytes_read = bytes_write;
					}
				}

				if (bytes_write == -1)
					break;
			}
		}

		close(from_fd);

		close(to_fd);

		::free(buffer);

		if (flag)
		{
			return 0;
		}
		else
		{
			return -3;
		}
    }

#ifdef WIN32
	void DelDir(const char* dirPath)
	{
		WIN32_FIND_DATAA finddata;
		HANDLE hfind;

		char pdir[MAX_PATH]; 

		memset(pdir, 0, MAX_PATH);

		strcpy(pdir, dirPath);

		if(dirPath[strlen(dirPath)-1]!='\\')
			strcat(pdir, "\\*.*");
		else
			strcat(pdir, "*.*");

		hfind=FindFirstFileA(pdir, &finddata);

		if(hfind==INVALID_HANDLE_VALUE)
		{
			return;
		}
 
		do
		{
			memset(pdir, 0, MAX_PATH);

			sprintf(pdir,"%s\\%s", dirPath, finddata.cFileName);

			if(strcmp(finddata.cFileName, ".")==0 || strcmp(finddata.cFileName, "..")==0)
			{
				continue;
			}

			if((finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
				DeleteFileA(pdir);
			else
				DelDir(pdir);

		}while(FindNextFileA(hfind, &finddata));

		FindClose(hfind);

		RemoveDirectoryA(dirPath);
	}
#else
	void DelDir(const char* dirPath)
	{
		DIR* dir = opendir(dirPath);

		if (dir == NULL)
		{
			return;
		}

		struct dirent* d;

		DIR* dirc = NULL;

		char* pathname = (char*)malloc(PATH_MAX);

		while (d = readdir(dir))
		{
			if (0 == strcmp(d->d_name, ".") || 0 == strcmp(d->d_name, ".."))
				continue;

			strcpy(pathname, dirPath);
			strcat(pathname, "/");
			strcat(pathname, d->d_name);

			dirc = opendir(pathname);

			if (dirc != NULL)
			{
				DelDir(pathname);

				closedir(dirc);

				dirc = NULL;
			}
			else
			{
				remove(pathname);
			}

			strcpy(pathname, "");
		}

		rmdir(dirPath);
		closedir(dir);
		free(pathname);
		pathname = NULL;
		d = NULL;
	}
#endif

	std::string GetCurrentDateTime()
	{
#ifdef WIN32
		SYSTEMTIME sysTime;

		GetLocalTime(&sysTime);

		char time[100];

		::sprintf(time, "%04d%02d%02d%02d%02d%02d", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

		return std::string(time);
#else
		time_t now = time(0);

		struct tm* tm_now;

		tm_now = localtime(&now);

		char buf[32];

		strftime(buf, sizeof(buf), "%Y%m%d%H%M%S", tm_now);

		return std::string(buf);
#endif
	}

	std::string GetCurrentDate()
	{
#ifdef WIN32
		SYSTEMTIME sysTime;

		GetLocalTime(&sysTime);

		char time[100];

		::sprintf(time, "%04d-%02d-%02d", sysTime.wYear, sysTime.wMonth, sysTime.wDay);

		return std::string(time);
#else
		time_t now = time(0);

		struct tm* tm_now;

		tm_now = localtime(&now);

		char buf[32];

		strftime(buf, sizeof(buf), "%Y-%m-%d", tm_now);

		return std::string(buf);
#endif
	}

	double	calculate_accurate_geodetic_distance(double latitude1, double longitude1, double latitude2, double longitude2)
	{
	    // get constants
	    double a = 6378137.0;				//Ellipsoid.WGS84.SemiMajorAxis;
	    double b = 6356752.3142451793;		//Ellipsoid.WGS84.SemiMinorAxis;
	    double f = 0.0033528106647474805;	//Ellipsoid.WGS84.Flattening;
				
	    // get parameters as radians
	    double phi1 = latitude1 * (M_PI / 180.0);
	    double lambda1 = longitude1 * (M_PI / 180.0);
	    double phi2 = latitude2 * (M_PI / 180.0);
	    double lambda2 = longitude2 * (M_PI / 180.0);

	    // calculations
	    double a2 = a * a;
	    double b2 = b * b;
	    double a2b2b2 = (a2 - b2) / b2;

	    double omega = lambda2 - lambda1;

	    double tanphi1 = std::tan(phi1);
	    double tanU1 = (1.0 - f) * tanphi1;
		double U1 = std::atan(tanU1);
	    double sinU1 = std::sin(U1);
	    double cosU1 = std::cos(U1);

	    double tanphi2 = std::tan(phi2);
	    double tanU2 = (1.0 - f) * tanphi2;
	    double U2 = std::atan(tanU2);
	    double sinU2 = std::sin(U2);
	    double cosU2 = std::cos(U2);

	    double sinU1sinU2 = sinU1 * sinU2;
	    double cosU1sinU2 = cosU1 * sinU2;
	    double sinU1cosU2 = sinU1 * cosU2;
	    double cosU1cosU2 = cosU1 * cosU2;

	    // eq. 13
	    double lambda = omega;

	    // intermediates we'll need to compute 's'
	    double A = 0.0;
	    double B = 0.0;
	    double sigma = 0.0;
	    double deltasigma = 0.0;
	    double lambda0;
	    bool converged = false;

	    for (int i = 0; i < 20; i++)
	    {
	        lambda0 = lambda;

	        double sinlambda = std::sin(lambda);
	        double coslambda = std::cos(lambda);

	        // eq. 14
	        double sin2sigma = (cosU2 * sinlambda * cosU2 * sinlambda) + std::pow(cosU1sinU2 - sinU1cosU2 * coslambda, 2.0);
	        double sinsigma = std::sqrt(sin2sigma);

	        // eq. 15
	        double cossigma = sinU1sinU2 + (cosU1cosU2 * coslambda);

	        // eq. 16
	        sigma = std::atan2(sinsigma, cossigma);

	        // eq. 17    Careful!  sin2sigma might be almost 0!
	        double sinalpha = (sin2sigma == 0) ? 0.0 : cosU1cosU2 * sinlambda / sinsigma;
	        double alpha = std::asin(sinalpha);
	        double cosalpha = std::cos(alpha);
	        double cos2alpha = cosalpha * cosalpha;

	        // eq. 18    Careful!  cos2alpha might be almost 0!
	        double cos2sigmam = cos2alpha == 0.0 ? 0.0 : cossigma - 2 * sinU1sinU2 / cos2alpha;
	        double u2 = cos2alpha * a2b2b2;

	        double cos2sigmam2 = cos2sigmam * cos2sigmam;

	        // eq. 3
	        A = 1.0 + u2 / 16384 * (4096 + u2 * (-768 + u2 * (320 - 175 * u2)));

	        // eq. 4
	        B = u2 / 1024 * (256 + u2 * (-128 + u2 * (74 - 47 * u2)));

	        // eq. 6
	        deltasigma = B * sinsigma * (cos2sigmam + B / 4 * (cossigma * (-1 + 2 * cos2sigmam2) - B / 6 * cos2sigmam * (-3 + 4 * sin2sigma) * (-3 + 4 * cos2sigmam2)));

	        // eq. 10
	        double C = f / 16 * cos2alpha * (4 + f * (4 - 3 * cos2alpha));

	        // eq. 11 (modified)
	        lambda = omega + (1 - C) * f * sinalpha * (sigma + C * sinsigma * (cos2sigmam + C * cossigma * (-1 + 2 * cos2sigmam2)));

	        // see how much improvement we got
	        double change = std::abs((lambda - lambda0) / lambda);
	            //  double change = (lambda - lambda0) / lambda > 0 ? (lambda - lambda0) / lambda :(lambda - lambda0) / lambda *-1;
	        if ((i > 1) && (change < 0.0000000000001))
	        {
	            converged = true;
	            break;
	        }
	    }

	    // eq. 19
	    return b * A * (sigma - deltasigma);
	}

	std::vector< std::pair<int, int> > SpiralArray(int xmin, int xmax, int ymin, int ymax)
	{
		std::vector< std::pair<int, int> > ret;

		int xsize = xmax-xmin+1;

		int ysize = ymax-ymin+1;

		int cx = xmin + floor(xsize*0.5 - 0.5);

		int cy = ymin + floor(ysize*0.5 - 0.5);

		int nCount = ret.size();

		int direction = 1; 	// 1 right 2 down 3 left 4 up

		int step = 0;

		int x = cx, y = cy;

		ret.push_back(std::make_pair(x, y));

		while (ret.size() < xsize*ysize)
		{
			if (direction == 1)
			{
				x = x+1;

				if (x <= xmax && x >= xmin && y<= ymax && y >= ymin)
				{
					ret.push_back(std::make_pair(x, y));
				}

				if (x == cx + step + 1)
				{
					direction = 2;

					step = step + 1;
				}
			}
			else if (direction == 2)
			{
				y = y+1;

				if (x <= xmax && x >= xmin && y<= ymax && y >= ymin)
				{
					ret.push_back(std::make_pair(x, y));
				}

				if (y == cy + step)
				{
					direction = 3;
				}
			}
			else if (direction == 3)
			{
				x = x-1;

				if (x <= xmax && x >= xmin && y<= ymax && y >= ymin)
				{
					ret.push_back(std::make_pair(x, y));
				}

				if (x <= cx-step)
				{
					direction = 4;
				}
			}
			else if (direction == 4)
			{
				y = y - 1;

				if (x <= xmax && x >= xmin && y<= ymax && y >= ymin)
				{
					ret.push_back(std::make_pair(x, y));
				}

				if (y <= cy-step)
				{
					direction = 1;
				}
			}
		}

		return ret;
	}

	void StringReplace(std::string& strBase, std::string strSrc, std::string strDes)
	{
		std::string::size_type pos = 0;

		std::string::size_type srcLen = strSrc.size();

		std::string::size_type desLen = strDes.size();
	
		while((pos = strBase.find(strSrc, pos)) != std::string::npos)
		{
			strBase.replace(pos, srcLen, strDes);

			pos += desLen;
			}
	}

	std::string DoubleToString(const double t)
	{
		std::ostringstream oss;

		oss.precision(16);
		oss.setf(std::ios::fixed);

		oss<<t;

		std::string result = oss.str();

		return result;
	}

    int StringToNum(const std::string sIn)
    {
        std::stringstream ss(sIn);

        int nReturn;

        ss>>nReturn;

        return nReturn;
    }

    std::string	ZipFolderPath(const char* folderPath, const char* zipPath)
	{
		std::string strZipPath = "";

		if (0 == strcmp(zipPath, ""))
		{
			strZipPath = std::string(folderPath) + ".zip";
		}
		else
		{
			strZipPath = zipPath;
		}

		int error;

		struct zip* folderZip = zip_open(strZipPath.c_str(), ZIP_CREATE, &error);

		if (folderZip == NULL)
		{
			return "";
		}

		ZipDir(folderPath, folderZip, "");

		zip_close(folderZip);

		folderZip = NULL;

		return strZipPath;
	}

#ifdef WIN32
	bool ZipDir(const char* dirPath, struct zip* zip, const char* parentDirRelativePath)
	{
		WIN32_FIND_DATAA finddata;
		HANDLE hfind;

		char pdir[MAX_PATH]; 

		memset(pdir, 0, MAX_PATH);

		strcpy(pdir, dirPath);

		if(dirPath[strlen(dirPath)-1]!='\\')
			strcat(pdir, "\\*.*");
		else
			strcat(pdir, "*.*");

		hfind=FindFirstFileA(pdir, &finddata);

		if(hfind==INVALID_HANDLE_VALUE)
		{
			return false;
		}

		char* relativepath = (char*)malloc(MAX_PATH);

		bool flag = true;

		do
		{
			memset(pdir, 0, MAX_PATH);

			sprintf(pdir,"%s\\%s", dirPath, finddata.cFileName);

			if(strcmp(finddata.cFileName, ".")==0 || strcmp(finddata.cFileName, "..")==0)
			{
				continue;
			}

			if((finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
			{
				zip_source* source = zip_source_file(zip, pdir, 0, -1);

				strcpy(relativepath, parentDirRelativePath);
				strcat(relativepath, "/");
				strcat(relativepath, finddata.cFileName);

				int zipRes = zip_add(zip, relativepath, source);

				if (zipRes == -1)
				{
					zip_source_free(source);

					flag = false;

					break;
				}
			}
			else
			{
				strcpy(relativepath, parentDirRelativePath);
				strcat(relativepath, "/");
				strcat(relativepath, finddata.cFileName);

				int zipRes = zip_add_dir(zip, relativepath);

				if (zipRes == -1)
				{
					flag = false;

					break;
				}

				bool rst = ZipDir(pdir, zip, relativepath);

				if (rst == false)
				{
					flag = false;

					break;
				}
			}

		}while(FindNextFileA(hfind, &finddata));

		FindClose(hfind);

		return true;
	}
#else
	bool ZipDir(const char* dirPath, struct zip* zip, const char* parentDirRelativePath)
	{
		DIR* dir = opendir(dirPath);

		if (dir == NULL)
		{
			return false;
		}

		struct dirent* d;

		DIR* dirc = NULL;

		char* pathname = (char*)malloc(PATH_MAX);

		char* relativepath = (char*)malloc(PATH_MAX);

		bool flag = true;

		while (d = readdir(dir))
		{
			if (0 == strcmp(d->d_name, ".") || 0 == strcmp(d->d_name, ".."))
				continue;

			strcpy(pathname, dirPath);
			strcat(pathname, "/");
			strcat(pathname, d->d_name);

			dirc = opendir(pathname);

			if (dirc != NULL)
			{
				strcpy(relativepath, parentDirRelativePath);
				strcat(relativepath, "/");
				strcat(relativepath, d->d_name);

				int zipRes = zip_add_dir(zip, relativepath);

				if (zipRes == -1)
				{
					flag = false;

					break;
				}

				bool rst = ZipDir(pathname, zip, relativepath);

				if (rst == false)
				{
					flag = false;

					break;
				}

				closedir(dirc);

				dirc = NULL;
			}
			else
			{
				zip_source* source = zip_source_file(zip, pathname, 0, -1);

				strcpy(relativepath, parentDirRelativePath);
				strcat(relativepath, "/");
				strcat(relativepath, d->d_name);

				int zipRes = zip_add(zip, relativepath, source);

				if (zipRes == -1)
				{
					zip_source_free(source);

					flag = false;

					break;
				}
			}

			strcpy(pathname, "");
		}

		closedir(dir);
		free(pathname);
		free(relativepath);
		pathname = NULL;
		relativepath = NULL;
		d = NULL;

		return flag;
	}
#endif

    std::string UnZipDir(const char* path, const char* outFolderPath)
	{
		std::string dirPath = "";

		if (0 == strcmp(outFolderPath, ""))
		{
			std::string strPath = std::string(path);

			size_t pos = strPath.find_last_of('.');

			dirPath = strPath.substr(0, pos);
		}
		else
		{
			dirPath = outFolderPath;
		}

		if (0 != access(dirPath.c_str(), 00))
		{
#ifdef WIN32
			if (0 != mkdir(dirPath.c_str()))
#else
			if (0 != mkdir(dirPath.c_str(), S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IWOTH))
#endif
			{
				return "";
			}
		}

		int error;

		struct zip* zip = zip_open(path, ZIP_CHECKCONS, &error);

		if (zip == NULL)
		{
			return "";
		}

		struct zip_stat fstat;

		zip_stat_init(&fstat);

		int numFiles = zip_get_num_files(zip);

		char filePath[PATH_MAX];

		for (int i=0; i<numFiles; i++)
		{
			int res = zip_stat_index(zip, i, 0, &fstat);

			if (res != 0)
			{
				zip_close(zip);

				return "";
			}

			int nameLength = strlen(fstat.name);

			if (fstat.name[nameLength-1] == '/')
			{
				continue;
			}

			memset(filePath, 0, sizeof(char)*PATH_MAX);

#ifdef WIN32
			std::string strName = std::string(fstat.name);

			Tools::StringReplace(strName, "/", "\\");

			sprintf(filePath, "%s\\%s", dirPath.c_str(), strName.c_str());

#else
			sprintf(filePath, "%s/%s", dirPath.c_str(), fstat.name);
#endif

			bool rst = Tools::CheckFilePath(filePath);

			if (rst == false)
			{
				zip_close(zip);

				return "";
			}

			struct zip_file* file = zip_fopen_index(zip, i, 0);

			if (file == NULL)
			{
				zip_close(zip);

				return "";
			}

			char* buffer = (char*)malloc(fstat.size + 1);

			long numBytesRead = zip_fread(file, buffer, fstat.size);

			buffer[fstat.size] = '\0';

			FILE* fp = fopen(filePath, "wb+");

			if (fp == NULL)
			{
				free(buffer);
				zip_fclose(file);
				zip_close(zip);

				return "";
			}

			size_t return_size = fwrite(buffer, fstat.size, 1, fp);

			free(buffer);

			fclose(fp);

			zip_fclose(file);
		}

		zip_close(zip);

		return dirPath;
	}

    int CopyBinaryFile(const char* source, const char* dest)
    {
        if(CheckFilePath(source) == false)
        {
            return 1; // 源文件不存在
        }

        if(CheckFilePath(dest) == false)
        {
            return 2; // 目标路径不存在
        }

        std::ifstream in;
        std::ofstream out;

        in.open(source, std::ios::binary);
        out.open(dest, std::ios::binary);

        if(!in || !out)
        {
            return -1; 
        }

        out<<in.rdbuf();

        out.close();

        in.close();

        return 0;
    }

    std::string ReplaceString(std::string& source, const std::string& symbol, const std::string newValue)
    {
        std::string result;
        while(true)
        {
            std::string::size_type pos(0);

            if((pos = source.find(symbol)) != std::string::npos)
            {
                source.replace(pos,symbol.length(),newValue);
            }
            else
            {
                break;
            } 
        }
        return source;
    }
    
    std::string geohash_encode(double lng, double lat, int precision)
    {
        if(precision < 1 || precision > 12)
        {
            precision = 6;
        }
        char* hash = NULL;

        if(lat <= MAX_LAT && lat >= MIN_LAT && lng <= MAX_LONG && lng >= MIN_LONG)
        {
            hash = (char*)malloc(sizeof(char) * (precision + 1));
            hash[precision] = '\0';
            precision *= 5.0;
            
            Interval lat_interval = {MAX_LAT, MIN_LAT};
            Interval lng_interval = {MAX_LONG, MIN_LONG};
            
            Interval *interval;
            double coord, mid;
            int is_even = 1;
            unsigned int hashChar = 0;
            int i;
            
            for(i = 1; i <= precision; i++) 
            {
                if(is_even)
                {
                    interval = &lng_interval;
                    coord = lng;
                }
                else 
                {
                    interval = &lat_interval;
                    coord = lat;
                }
                mid = (interval->low + interval->high) / 2.0;
                hashChar = hashChar << 1;
                
                if(coord > mid)
                {
                    interval->low = mid;
                    hashChar |= 0x01;
                } 
                else
                {
                    interval->high = mid;
                }
                
                if(!(i % 5))
                {
                    hash[(i - 1) / 5] = char_map[hashChar];
                    hashChar = 0;
                }
                is_even = !is_even;
            }
        }
        if(hash != NULL)
        {
            std::string hash_str(hash);
            return hash_str;
        }
        else
        {
            return "";
        }
    }

    std::string GetTipsRowkey(const std::string& type)
    {
        std::string rowkey;

        if(strcmp(type.c_str(),"")==0)
        {
           return rowkey; 
        }

        rowkey += "02"+type;

        boost::uuids::random_generator rgen;
        boost::uuids::uuid u = boost::uuids::random_generator()();
        std::stringstream ss;
        ss<<u;
        std::string uuid= ss.str();

        rowkey += ReplaceString(uuid,"-","");

        return rowkey;
    }

    std::string	GenerateProgressInfo(std::string flag, std::string info)
    {
    	return "{\"flag\":\"" + flag + "\",\"info\":\"" + info +"\"}";
    }
    std::string GernerateTipsProgressInfo(const std::string& flag, const std::string& info, const std::string& information,bool isImport)
    {
        Document document;
        Document docTmp;
        document.SetObject();
        Document::AllocatorType& allocator = document.GetAllocator();
        document.AddMember("flag",rapidjson::StringRef(flag.c_str()),allocator);
        document.AddMember("info",rapidjson::StringRef(info.c_str()),allocator);
        rapidjson::Value object(rapidjson::kObjectType);
        if(isImport)
        {
        	object.AddMember("serverMsg",rapidjson::StringRef(information.c_str()),allocator);
        	object.AddMember("errMsg","",allocator);
        }
        else
        {
        	object.AddMember("errMsg",rapidjson::StringRef(information.c_str()),allocator);
			object.AddMember("serverMsg","",allocator);
        }
        document.AddMember("information",object,allocator);
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);  
        document.Accept(writer);  
        std::string sResult = buffer.GetString();
        return sResult;
    }
    
    int CountLines(const char* path)
    {
        std::ifstream in;
        
        in.open(path, std::ios::in|std::ios::binary);
        
        if (!in)
        {
            return 0;
        }
        else
        {
            int nCount = std::count(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>(), '\n');
            
            in.close();
            
            return nCount;
        }
    }
    
    unsigned long GetFileSize(const char* path)
    {
        struct stat fs;
        
        if(stat(path, &fs) < 0)
        {
            return 0;
        }
        
        return (unsigned long)fs.st_size;
    }
    
    size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
    {
        std::string* str = (std::string*)userdata;
        
        if (str == NULL || ptr == NULL)
        {
            return -1;
        }
        
        size_t totalSize = size * nmemb;
        
        str->append(ptr, totalSize);
        
        return totalSize;
    }

	EDITOR_API int CheckRelationTips( int sourceType, const std::string& tipsDeep, const std::string& surveyLineRowkey, std::string& reId, bool& isReTips )
	{
		rapidjson::Document doc;

		doc.Parse<0>(tipsDeep.c_str());

		if(doc.HasParseError())
		{
			isReTips = false;
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

			//种别的关联要素，及车信的关联要素
			switch(sourceType)
			{
			case Model::RoadKind:
				if (sKey == "f")
				{
					rapidjson::Document reFeatureDoc;
					reFeatureDoc.Parse<0>(sValue.c_str());
					if(reFeatureDoc.HasParseError() 
						|| !reFeatureDoc.HasMember("id") 
						|| !reFeatureDoc.HasMember("type"))
					{
						isReTips = false;
						Logger::LogD ("Parse  Tips deep id or type failed: %s\n", sValue.c_str());
						Logger::LogO ("Parse Tips deep id or type failed: %s\n", sValue.c_str());
						return -1;
					}

					//关联要素类型:1 道路LINK；2 测线.
					if (reFeatureDoc.FindMember("type")->value.GetInt()==2)
					{
						isReTips = true;
						reId = reFeatureDoc.FindMember("id")->value.GetString();
						return strcmp(surveyLineRowkey.c_str(), reId.c_str());
					}
					else
					{
						isReTips = false;
						return -1;
					}
				}
				break;
			case Model::LaneConnexity:
				if (sKey == "in")
				{
					rapidjson::Document reFeatureDoc;
					reFeatureDoc.Parse<0>(sValue.c_str());
					if(reFeatureDoc.HasParseError() 
						|| !reFeatureDoc.HasMember("id") 
						|| !reFeatureDoc.HasMember("type"))
					{
						isReTips = false;
						Logger::LogD ("Parse  Tips deep id or type failed: %s\n", sValue.c_str());
						Logger::LogO ("Parse Tips deep id or type failed: %s\n", sValue.c_str());
						return -1;
					}

					//关联要素类型:1 道路LINK；2 测线.
					if (reFeatureDoc.FindMember("type")->value.GetInt()==2)
					{
						isReTips = true;
						reId = reFeatureDoc.FindMember("id")->value.GetString();
						return strcmp(surveyLineRowkey.c_str(), reId.c_str());
					}
					else
					{
						isReTips = false;
						return -1;
					}
				}
				break;

			default:
				break;

			}

			
		}

		return 0;
	}


    bool HttpGet(const char* url, std::string& response)
    {
        bool ret = true;
        
        char errbuf[1024];
        
        CURL* easy_handle = curl_easy_init();
        
        /* set the options (I left out a few, you'll get the point anyway) */
        curl_easy_setopt(easy_handle, CURLOPT_URL, url);
        
        curl_easy_setopt(easy_handle, CURLOPT_TIMEOUT, 20);
        
        curl_easy_setopt(easy_handle, CURLOPT_NOSIGNAL, 1L);
        
        curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 0L);
        
        curl_easy_setopt(easy_handle, CURLOPT_NOPROGRESS, 1L);
        
        curl_easy_setopt(easy_handle, CURLOPT_FORBID_REUSE, 1L);
        
        curl_easy_setopt(easy_handle, CURLOPT_ERRORBUFFER, errbuf);
        
        curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, write_callback);
        
        curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &response);
        
        CURLcode curlCode = curl_easy_perform(easy_handle);
        
        if (curlCode == CURLE_OK)
        {
            long resCode;
            
            curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &resCode);
            
            if (resCode != 200)
            {
                ret = false;
                
                Logger::LogD("HttpGet failed! http response code [%ld], url[%s]", resCode,url);
                Logger::LogO("HttpGet failed! http response code [%ld], url[%s]", resCode,url);
            }
        }
        else
        {
            ret = false;
            
            size_t len = strlen(errbuf);
            
            if (len)
            {
                Logger::LogD("HttpGet failed! curl error code [%d], msg [%s], url[%s]", curlCode, errbuf,url);
                Logger::LogO("HttpGet failed! curl error code [%d], msg [%s], url[%s]", curlCode, errbuf,url);
            }
            else
            {
                Logger::LogD("HttpGet failed! curl error code [%d], msg [%s], url[%s]", curlCode, curl_easy_strerror(curlCode) ,url);
                Logger::LogO("HttpGet failed! curl error code [%d], msg [%s], url[%s]", curlCode, curl_easy_strerror(curlCode) ,url);
            }
        }
        
        curl_easy_cleanup(easy_handle);
        
        return ret;
    }
	bool HttpGet(const char* url, std::string& response, int& code)
    {
         bool ret = true;
        
        char errbuf[1024];
        
        CURL* easy_handle = curl_easy_init();
        
        /* set the options (I left out a few, you'll get the point anyway) */
        curl_easy_setopt(easy_handle, CURLOPT_URL, url);
        
        curl_easy_setopt(easy_handle, CURLOPT_TIMEOUT, 200000);
        
        curl_easy_setopt(easy_handle, CURLOPT_NOSIGNAL, 1L);
        
        curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 0L);
        
        curl_easy_setopt(easy_handle, CURLOPT_NOPROGRESS, 1L);
        
        curl_easy_setopt(easy_handle, CURLOPT_FORBID_REUSE, 1L);
        
        curl_easy_setopt(easy_handle, CURLOPT_ERRORBUFFER, errbuf);
        
        curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, write_callback);
        
        curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &response);
        
        CURLcode curlCode = curl_easy_perform(easy_handle);
        
        if (curlCode == CURLE_OK)
        {
            long resCode;
            
            curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &resCode);
            
            code = resCode;

            if (resCode != 200)
            {
                ret = false;
                
                Logger::LogD("HttpGet failed! http response code [%ld],url[%s]", resCode,url);
                Logger::LogO("HttpGet failed! http response code [%ld],url[%s]", resCode,url);
            }
        }
        else
        {
            ret = false;
            
            size_t len = strlen(errbuf);
            
            if (len)
            {
                Logger::LogD("HttpGet failed! curl error code [%d], msg [%s],url[%s]", curlCode, errbuf,url);
                Logger::LogO("HttpGet failed! curl error code [%d], msg [%s],url[%s]", curlCode, errbuf,url);
            }
            else
            {
                Logger::LogD("HttpGet failed! curl error code [%d], msg [%s],url[%s]", curlCode, curl_easy_strerror(curlCode),url);
                Logger::LogO("HttpGet failed! curl error code [%d], msg [%s],url[%s]", curlCode, curl_easy_strerror(curlCode),url);
            }
        }
        
        curl_easy_cleanup(easy_handle);
        
        return ret;
    }

    bool HttpMultiPost(const char* url, char** pName, char** pContents, int iParamLen, std::string& response)
    {
    	if (iParamLen < 2)
    	{
    		Logger::LogD("HttpMultiPost must requset at least 2 param!");
    		Logger::LogO("HttpMultiPost must requset at least 2 param!");

    		return false;
    	}

    	bool ret = true;

    	CURL *curl;

    	CURLM *multi_handle;
    	int still_running;
    	int prev_still_running;
    	int still_running_unchange = 0;

    	struct curl_httppost *formpost=NULL;
    	struct curl_httppost *lastptr=NULL;
    	struct curl_slist *headerlist=NULL;
    	static const char buf[] = "Expect:";

    	for (int i=0; i<iParamLen; i++)
    	{
    		curl_formadd(&formpost,
						 &lastptr,
						 CURLFORM_PTRNAME, pName[i],
						 CURLFORM_PTRCONTENTS, pContents[i],
						 CURLFORM_END);
    	}

    	curl = curl_easy_init();
    	multi_handle = curl_multi_init();

    	headerlist = curl_slist_append(headerlist, buf);
    	if(curl && multi_handle)
    	{
    	    curl_easy_setopt(curl, CURLOPT_URL, url);
    	    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    	    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    	    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    	    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

    	    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    	    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    	    curl_multi_add_handle(multi_handle, curl);

    	    curl_multi_perform(multi_handle, &still_running);

    	    prev_still_running = still_running;

    	    do
    	    {
    	    	struct timeval timeout;
    	        int rc;
    	        CURLMcode mc;

    	        fd_set fdread;
    	        fd_set fdwrite;
    	        fd_set fdexcep;
    	        int maxfd = -1;

    	        long curl_timeo = -1;

    	        FD_ZERO(&fdread);
    	        FD_ZERO(&fdwrite);
    	        FD_ZERO(&fdexcep);

    	        timeout.tv_sec = 1;
    	        timeout.tv_usec = 0;

    	        curl_multi_timeout(multi_handle, &curl_timeo);
    	        if(curl_timeo >= 0)
    	        {
    	        	timeout.tv_sec = curl_timeo / 1000;
    	            if(timeout.tv_sec > 1)
    	            	timeout.tv_sec = 1;
    	            else
    	            	timeout.tv_usec = (curl_timeo % 1000) * 1000;
    	        }

    	        mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

    	        if(mc != CURLM_OK)
    	        {
    	        	Logger::LogD("HttpMultiPost, curl_multi_fdset() failed! code [%d],url[%s]", mc,url);
    	    	    Logger::LogO("HttpMultiPost, curl_multi_fdset() failed! code [%d],url[%s]", mc,url);

    	    	    break;
    	        }

    	        if(maxfd == -1)
    	        {
#ifdef _WIN32
    	        	Sleep(100);
    	            rc = 0;
#else
    	            struct timeval wait = { 0, 100 * 1000 }; /* 100ms */
    	            rc = select(0, NULL, NULL, NULL, &wait);
#endif
    	      }
    	      else
    	      {
    	    	  rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
    	      }

    	      switch(rc)
    	      {
    	      case -1:
				  {
				  }
    	          break;
    	      case 0:
				  {
					  curl_multi_perform(multi_handle, &still_running);
				  }
				  break;
    	      default:
				  {
					  int last_still_running = still_running;

					  CURLMcode code = curl_multi_perform(multi_handle, &still_running);

					  if (last_still_running == still_running)
					  {
						  break;
					  }
					  else
					  {
						  struct CURLMsg* m;
						  bool flag_m = false;
						  do
						  {
							  int msgq = 0;

							  m = curl_multi_info_read(multi_handle, &msgq);

							  if (m && (m->msg == CURLMSG_DONE))
							  {
								  CURLcode curlCode = m->data.result;

								  void* curlMsg = m->data.whatever;

								  if (curlCode != CURLE_OK)
								  {
									  ret = false;
									  Logger::LogD("HttpMultiPost, curlCode [%d]", curlCode);
								  }
								  else
								  {
									  CURL* easy_handle = m->easy_handle;
									  long resCode;
									  curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &resCode);
									  if (resCode != 200)
									  {
										  ret = false;
										  Logger::LogD("HttpMultiPost, curlCode [%d], resCode [%ld]", curlCode, resCode);
									  }
								  }

								  flag_m = true;
							  }
						  } while (m);

						  if (false == flag_m && NULL != m)
						  {
							  ret = false;
							  Logger::LogD("HttpMultiPost, msg [%d]", m->msg);
						  }
					  }
				  }
				  break;
    	      }

    	      if ((prev_still_running == still_running) && (0 == rc))
    	      {
    	    	  still_running_unchange++;
    	      }
    	      else
    	      {
    	    	  still_running_unchange = 0;
    	      }

    	      prev_still_running = still_running;

    	      if (still_running_unchange > 1000)
    	      {
    	    	  ret = false;

    	    	  still_running = 0;

    	    	  Logger::LogD("HttpMultiPost, timeout!");
    	    	  Logger::LogO("HttpMultiPost, timeout!");
    	      }
    	    } while(still_running);

    	    curl_multi_remove_handle(multi_handle, curl);

    	    curl_easy_cleanup(curl);

    	    curl_multi_cleanup(multi_handle);

    	    curl_formfree(formpost);

    	    curl_slist_free_all (headerlist);
    	}

    	return ret;
    }

	bool HttpPost(const char* url, std::string& response, const std::string& data)
    {
        bool ret = true;
        
        char errbuf[1024];
        
        CURL* easy_handle = curl_easy_init();
        
        curl_easy_setopt(easy_handle, CURLOPT_URL, url);

        curl_easy_setopt(easy_handle, CURLOPT_POST, 1L);

        curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDS, data.c_str());

        curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDSIZE, data.size());
        
        curl_easy_setopt(easy_handle, CURLOPT_TIMEOUT, 20);
        
        curl_easy_setopt(easy_handle, CURLOPT_NOSIGNAL, 1L);
        
        curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 0L);
        
        curl_easy_setopt(easy_handle, CURLOPT_NOPROGRESS, 1L);
        
        curl_easy_setopt(easy_handle, CURLOPT_FORBID_REUSE, 1L);
        
        curl_easy_setopt(easy_handle, CURLOPT_ERRORBUFFER, errbuf);
        
        curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, write_callback);
        
        curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &response);
        
        CURLcode curlCode = curl_easy_perform(easy_handle);
        
        if (curlCode == CURLE_OK)
        {
            long resCode;
            
            curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &resCode);
            
            if (resCode != 200)
            {
                ret = false;
                
                Logger::LogD("HttpGet failed! http response code [%ld],url[%s]", resCode,url);
                Logger::LogO("HttpGet failed! http response code [%ld],url[%s]", resCode,url);
            }
        }
        else
        {
            ret = false;
            
            size_t len = strlen(errbuf);
            
            if (len)
            {
                Logger::LogD("HttpGet failed! curl error code [%d], msg [%s],url[%s]", curlCode, errbuf,url);
                Logger::LogO("HttpGet failed! curl error code [%d], msg [%s],url[%s]", curlCode, errbuf,url);
            }
            else
            {
                Logger::LogD("HttpGet failed! curl error code [%d], msg [%s],url[%s]", curlCode, curl_easy_strerror(curlCode),url);
                Logger::LogO("HttpGet failed! curl error code [%d], msg [%s],url[%s]", curlCode, curl_easy_strerror(curlCode),url);
            }
        }
        
        curl_easy_cleanup(easy_handle);
        
        return ret;
    }
	
	bool HttpPost(const char* url, std::string& response, const std::string& data, int& code)
    {
        bool ret = true;
        
        char errbuf[1024];
        
        CURL* easy_handle = curl_easy_init();
        
        curl_easy_setopt(easy_handle, CURLOPT_URL, url);

        curl_easy_setopt(easy_handle, CURLOPT_POST, 1L);

        curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDS, data.c_str());

        curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDSIZE, data.size());
        
        curl_easy_setopt(easy_handle, CURLOPT_TIMEOUT, 20);
        
        curl_easy_setopt(easy_handle, CURLOPT_NOSIGNAL, 1L);
        
        curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 0L);
        
        curl_easy_setopt(easy_handle, CURLOPT_NOPROGRESS, 1L);
        
        curl_easy_setopt(easy_handle, CURLOPT_FORBID_REUSE, 1L);
        
        curl_easy_setopt(easy_handle, CURLOPT_ERRORBUFFER, errbuf);
        
        curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, write_callback);
        
        curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &response);
        
        CURLcode curlCode = curl_easy_perform(easy_handle);
        
        if (curlCode == CURLE_OK)
        {
            long resCode;
            
            curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &resCode);
            
            code = resCode;

            if (resCode != 200)
            {
                ret = false;
                
                Logger::LogD("HttpGet failed! http response code [%ld],url[%s]", resCode,url);
                Logger::LogO("HttpGet failed! http response code [%ld],url[%s]", resCode,url);
            }
        }
        else
        {
            ret = false;
            
            size_t len = strlen(errbuf);
            
            if (len)
            {
                Logger::LogD("HttpGet failed! curl error code [%d], msg [%s],url[%s]", curlCode, errbuf,url);
                Logger::LogO("HttpGet failed! curl error code [%d], msg [%s],url[%s]", curlCode, errbuf,url);
            }
            else
            {
                Logger::LogD("HttpGet failed! curl error code [%d], msg [%s],url[%s]", curlCode, curl_easy_strerror(curlCode),url);
                Logger::LogO("HttpGet failed! curl error code [%d], msg [%s],url[%s]", curlCode, curl_easy_strerror(curlCode),url);
            }
        }
        
        curl_easy_cleanup(easy_handle);
        
        return ret;
    }
    std::string GenerateUuid()
    {
        std::string rowkey;

        boost::uuids::random_generator rgen;
        boost::uuids::uuid u = boost::uuids::random_generator()();
        std::stringstream ss;
        ss<<u;
        std::string uuid= ss.str();

        std::string resutl = ReplaceString(uuid,"-","");

        return resutl;
    }

    std::string GetInStatement(const std::vector<std::string>& ids, int type)
    {
        std::string inStatement = "(";

        for(int i = 0; i< ids.size(); i++)
        {
            if(type == 0)
            {
                inStatement += ids[i];
            }
            else if(type == 1)
            {
                inStatement += "'"+ ids[i] + "'";
            }

            if(i != ids.size()-1)
            {
                inStatement += ",";
            }
        }
        inStatement +=")";

        return inStatement;
    }
	std::string GenerateWktPoint(double lon, double lat)
    {
        return "POINT(" + NumberToString<double>(lon) + " " + NumberToString<double>(lat) + ")";
    }
    
    int ReplaceFile(const char* srcFilePath, const char* destFilePath)
    {
#ifdef WIN32
#else
        if (srcFilePath == NULL|| destFilePath == NULL)
        {
            return -1;
        }
        
        if (access(srcFilePath, F_OK)!=0)
        {
            return -1;
        }
        
        if (access(destFilePath, F_OK)==0)
        {
            DIR* dirc = NULL;
            dirc = opendir(destFilePath);
            
            if (dirc != NULL)
            {
                closedir(dirc);
                dirc = NULL;
                return -1;
            }
            else
            {
              remove(destFilePath);
            }

        }
        
        CopyFile(srcFilePath, destFilePath);
        
        return 0;
#endif
    }
    

    void RemoveSimilarFilesInDir(const char *dir, const char* filekey)
    {
#ifdef WIN32
        
        WIN32_FIND_DATAA finddata;
        HANDLE hfind;
        
        char pdir[MAX_PATH];
        
        memset(pdir, 0, MAX_PATH);
        
        strcpy(pdir, dirPath);
        
        if(dirPath[strlen(dirPath)-1]!='\\')
            strcat(pdir, "\\*.*");
        else
            strcat(pdir, "*.*");
        
        hfind=FindFirstFileA(pdir, &finddata);
        
        if(hfind==INVALID_HANDLE_VALUE)
        {
            return;
        }
        
        do
        {
            memset(pdir, 0, MAX_PATH);
            
            sprintf(pdir,"%s\\%s", dirPath, finddata.cFileName);
            
            if(strcmp(finddata.cFileName, ".")==0 || strcmp(finddata.cFileName, "..")==0)
            {
                continue;
            }
            
            if((finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
            {
                if (strstr(finddata.cFileName, filekey)!=NULL)
                {
                    DeleteFileA(pdir);
                }
            }
            
        }while(FindNextFileA(hfind, &finddata));
        
        FindClose(hfind);
        
#else
        DIR *dp;
        struct dirent *entry;
        struct stat statbuf;
        if((dp = opendir(dir)) == NULL)
        {
            fprintf(stderr,"cannot open directory: %s\n", dir);
            return;
        }
        chdir(dir);
        
        char* absolutepath = (char*)malloc(PATH_MAX);
        
        memset(absolutepath, 0, PATH_MAX);
        
        while((entry = readdir(dp)) != NULL)
        {
            lstat(entry->d_name,&statbuf);
            if(!S_ISDIR(statbuf.st_mode))
            {
                if (strstr(entry->d_name, filekey)!=NULL)
                {
                    strcpy(absolutepath, dir);
                    strcat(absolutepath, "/");
                    strcat(absolutepath, entry->d_name);
                    remove(absolutepath);
                }
                
            }
        }
        closedir(dp);
        free(absolutepath);
#endif
    }
    
    void CopySimilarFiles2Dir(const char *srcdir, const char* filekey, const char *destdir)
    {
#ifdef WIN32
        WIN32_FIND_DATAA finddata;
        HANDLE hfind;
        
        char pdir[MAX_PATH];
        
        memset(pdir, 0, MAX_PATH);
        
        strcpy(pdir, dirPath);
        
        if(dirPath[strlen(dirPath)-1]!='\\')
            strcat(pdir, "\\*.*");
        else
            strcat(pdir, "*.*");
        
        hfind=FindFirstFileA(pdir, &finddata);
        
        if(hfind==INVALID_HANDLE_VALUE)
        {
            return;
        }
        
        do
        {
            memset(pdir, 0, MAX_PATH);
            
            sprintf(pdir,"%s\\%s", dirPath, finddata.cFileName);
            
            if(strcmp(finddata.cFileName, ".")==0 || strcmp(finddata.cFileName, "..")==0)
            {
                continue;
            }
            
            if((finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
            {
                if (strstr(finddata.cFileName, filekey)!=NULL)
                {
                    strcpy(srcFilePath, srcdir);
                    strcat(srcFilePath, "\\");
                    strcat(srcFilePath, finddata.cFileName);
                    
                    strcpy(destFilePath, destdir);
                    strcat(destFilePath, "\\");
                    strcat(destFilePath, finddata.cFileName);
                    
                    CopyFile(srcFilePath, destFilePath);
                }
            }
            
        }while(FindNextFileA(hfind, &finddata));
        
        FindClose(hfind);
#else
        DIR *dp=NULL;
        struct dirent *entry;
        struct stat statbuf;
        if((dp = opendir(srcdir)) == NULL)
        {
            fprintf(stderr,"cannot open directory: %s\n", srcdir);
            return;
        }
        
        DIR *destdp=NULL;
        if ((destdp=opendir(destdir))==NULL)
        {
            fprintf(stderr,"cannot open directory: %s\n", destdir);
            return;
        }
        
        chdir(srcdir);
        
        char* srcFilePath = (char*)malloc(PATH_MAX);
        
        memset(srcFilePath, 0, PATH_MAX);
        
        char* destFilePath = (char*)malloc(PATH_MAX);
        
        memset(destFilePath, 0, PATH_MAX);
        
        while((entry = readdir(dp)) != NULL)
        {
            lstat(entry->d_name,&statbuf);
            if(!S_ISDIR(statbuf.st_mode))
            {
                if (strstr(entry->d_name, filekey)!=NULL)
                {
                    strcpy(srcFilePath, srcdir);
                    strcat(srcFilePath, "/");
                    strcat(srcFilePath, entry->d_name);
                    
                    strcpy(destFilePath, destdir);
                    strcat(destFilePath, "/");
                    strcat(destFilePath, entry->d_name);
                    
                    CopyFile(srcFilePath, destFilePath);
                }
                
            }
        }
        closedir(dp);
        free(srcFilePath);
        free(destFilePath);
#endif

    }
}
