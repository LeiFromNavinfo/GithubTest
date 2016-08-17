#include "Editor_ProjectManager.h"

namespace Editor
{
	EditorGeometry::Box2D Grid::GridToLocation(std::string gridId)
	{
		EditorGeometry::Box2D box;

		box.make_empty();

		if (gridId.length() < 7)
		{
			Logger::LogD("gridId [%s] is not valid!", gridId.c_str());
			Logger::LogO("gridId [%s] is not valid!", gridId.c_str());

			return box;
		}

        if(gridId.length() == 7)
        {
            gridId ="0" + gridId;
        }
        
        int m1 = std::atoi(gridId.substr(0, 1).c_str());
        int m2 = std::atoi(gridId.substr(1, 1).c_str());
        int m3 = std::atoi(gridId.substr(2, 1).c_str());
        int m4 = std::atoi(gridId.substr(3, 1).c_str());
        int m5 = std::atoi(gridId.substr(4, 1).c_str());
        int m6 = std::atoi(gridId.substr(5, 1).c_str());
        int m7 = std::atoi(gridId.substr(6, 1).c_str());
        int m8 = std::atoi(gridId.substr(7, 1).c_str());
        
        double minx = (m3 * 10 + m4) + (m6 * 450 + m8*450/4.0)/3600 + 60;
        double miny = ((m1 * 10 + m2) * 2400 + m5 * 300 + m7*300/4)/3600.0;
        double maxx = minx+0.03125;
        double maxy = miny+(1.0/(12*4));
        
        box.make(minx, maxx, miny, maxy);
		
		return box;
	}

	std::string Grid::GridsToLocation(std::vector<std::string> gridIds)
	{
		geos::io::WKTReader wktReader;

		std::vector<geos::geom::Polygon*> geoVec;

		std::vector<std::string>::const_iterator itor = gridIds.begin();

		for (; itor!=gridIds.end(); itor++)
		{
			EditorGeometry::Box2D box = GridToLocation(*itor);

			geos::geom::CoordinateArraySequence coords;

			coords.add(geos::geom::Coordinate(box._minx, box._maxy));

			coords.add(geos::geom::Coordinate(box._maxx, box._maxy));

			coords.add(geos::geom::Coordinate(box._maxx, box._miny));

			coords.add(geos::geom::Coordinate(box._minx, box._miny));

			coords.add(geos::geom::Coordinate(box._minx, box._maxy));

			geos::geom::LinearRing* ring = geos::geom::GeometryFactory::getDefaultInstance()->createLinearRing(coords);

			geos::geom::Geometry* boxGeo = geos::geom::GeometryFactory::getDefaultInstance()->createPolygon(ring, NULL);

			geoVec.push_back(dynamic_cast<geos::geom::Polygon*>(boxGeo));
		}

		geos::geom::Geometry* geo = geos::operation::geounion::CascadedPolygonUnion::Union(&geoVec);

		int geoCount = geoVec.size();

		for (int i=0; i<geoCount; i++)
		{
			delete geoVec[i];
		}

		geoVec.clear();

		if (geo == NULL)
		{
			return "";
		}

		std::string wkt = geo->toText();

		delete geo;

		return wkt;
	}

	std::string Grid::LocationToGrid(double x, double y)
	{
        //将度单位坐标转换为秒*3600，并乘1000消除小数,最后取整
        long longX = (long)(x*3600000);
        long longY = (long)(y*3600000);
        
        int M1M2;
        int M3M4;
        int M5;
        int M6;
        int M7;
        int M8;
        
        //一个四位图幅的纬度高度为2400秒
        M1M2 = (int)(longY/(2400000));
        M3M4 = ((int)x) - 60;//简便算法
        
        int yt = (int)(longY/(300000));
        M5 = yt%8;
        int xt = (int)(longX/(450000));
        M6 = xt%8;
        
        M7 = (int)((longY%(300000))*4)/(300000);
        M8 = (int)((longX%(450000))*4)/(450000);
        
        std::string gridId;
        
        gridId += Tools::NumberToString(M1M2);
        gridId += Tools::NumberToString(M3M4);
        gridId += Tools::NumberToString(M5);
        gridId += Tools::NumberToString(M6);
        gridId += Tools::NumberToString(M7);
        gridId += Tools::NumberToString(M8);
        
        if(gridId.length() == 7)
        {
            gridId = "0" + gridId;
        }
        return gridId;
	}

	EditorGeometry::Box2D Grid::CalcMeshBound(std::string meshId, double& centerLon, double& centerLat)
	{
		EditorGeometry::Box2D box;

		box.make_empty();

		if (meshId.empty() || meshId.length() != 6)
		{
			Logger::LogD("meshId [%s] is not valid!", meshId.c_str());
			Logger::LogO("meshId [%s] is not valid!", meshId.c_str());

			return box;
		}

		char strMesh[7];

		strcpy(strMesh, meshId.c_str());

		char str0 = '0';

		int M1 = strMesh[0] - str0;

		int M2 = strMesh[1] - str0;

		int M3 = strMesh[2] - str0;

		int M4 = strMesh[3] - str0;

		int M5 = strMesh[4] - str0;

		int M6 = strMesh[5] - str0;

		//该图幅的左下角点
		double x = (M3 * 10 + M4) * 3600 + M6 * 450 + 60 * 3600;

		double y = (M1 * 10 + M2) * 2400 + M5 * 300;

		//该图幅的中间点
		centerLon = SecondToDegree(x + 450/2);

		centerLat = SecondToDegree(y + 300/2);

		box.make(SecondToDegree(x), SecondToDegree(x+450), SecondToDegree(y), SecondToDegree(y+300));

		return box;
	}

	std::string Grid::MeshLocator_25T(double dLongitude, double dLatitude)
	{
		if (0x01 == (IsAt25TMeshBorder(dLongitude, dLatitude) & 0x0F))	//为了保证总返回右上的图幅
				dLatitude += 0.00001;

		int remainder = 0;

		int rowInx = CalculateRealRowIndex(dLatitude, remainder);

		int colInx = CalculateRealColumnIndex(dLongitude, remainder);

		//第1、2位：纬度取整拉伸1.5倍//
		int M1M2 = (int)(dLatitude * 1.5);

		//第3、4位：经度减去日本角点 60度//
		int M3M4 = (int)(dLongitude) - 60;

		//第5位：//
		int M5 = rowInx % 8;

		//第6位：每列450秒，每度包含8列//
		int M6 = colInx % 8;

		//连接以上数字，组成图幅号//
		char mesh[10];

		memset(mesh, 0, 10);

		if ((M1M2 > 99) || (M3M4 > 99))
		{
			Logger::LogD("MeshLocator_25T mesh is calculated wrong! dLongitude:%f, dLatitude:%f", dLongitude, dLatitude);

			Logger::LogO("MeshLocator_25T mesh is calculated wrong! dLongitude:%f, dLatitude:%f", dLongitude, dLatitude);

			return mesh;
		}

		if (M3M4 < 0)
		{
			Logger::LogD("MeshLocator_25T mesh is calculated wrong! dLongitude:%f, dLatitude:%f", dLongitude, dLatitude);

			Logger::LogO("MeshLocator_25T mesh is calculated wrong! dLongitude:%f, dLatitude:%f", dLongitude, dLatitude);

			return mesh;
		}

		if (M1M2 < 10)
		{
			::sprintf(mesh, "0%d%d%d%d", M1M2, M3M4, M5, M6);
		}
		else
		{
			::sprintf(mesh, "%d%d%d%d", M1M2, M3M4, M5, M6);
		}

		return mesh;
	}

	double Grid::SecondToDegree(double second)
	{
		return second / 3600.0;
	}

	/*
	* 点是否在图框上
	* 0x01——上下图框 0x10——左右图框
	*/
	int Grid::IsAt25TMeshBorder(double dLongitude, double dLatitude)
	{
	int model = 0;

	int remainder = 0;

	int rowIndex = CalculateIdealRowIndex(dLatitude, remainder);

	switch (rowIndex % 3)
	{
	case 0:	//第一行//
		{
			if (300000 - remainder == 12)	//余数距离上框等于0.012秒//
				model |= 0x01;
			else if (remainder == 0)
				model |= 0x01;
		}
		break;
	case 1: //第二行由于上下边框均不在其内，因此不在图框上//
		break;
	case 2: //第三行//
		{
			if (remainder == 12)	//余数距离下框等于0.012秒//
				model |= 0x01;
		}
		break;
	}

	CalculateRealColumnIndex(dLongitude, remainder);

	if (0 == remainder)
		model |= 0x10;

	return model;
	}

	/*
	* 根据纬度计算该点位于理想图幅分割的行序号
	* 纬度：单位 度
	* 余数：单位 千秒
	*/
	int Grid::CalculateIdealRowIndex(double dLatitude, int& remainder)
	{
	//相对区域纬度 = 绝对纬度 - 0.0//
	double regionLatitude = dLatitude - 0.0;

	//相对的以秒为单位的纬度//
	double secondLatitude = regionLatitude * 3600;

	//为避免浮点数的内存影响，将秒＊10的三次方（由于0.00001度为0.036秒）//
	long longsecond = (int)(secondLatitude * 1000);

	remainder = (int)longsecond % 300000;

	return (int)(longsecond / 300000);
	}

	/*
	* 根据经度计算该点位于理想图幅分割的列序号
	* 经度：单位 度
	* 余数：单位 千秒
	*/
	int Grid::CalculateIdealColumnIndex(double dLongitude, int& remainder)
	{
	//相对区域经度 = 绝对经度 - 60.0//
	double regionLongitude = dLongitude - 60.0;

	//相对的以秒为单位的经度//
	double secondLongitude = regionLongitude * 3600;

	//为避免浮点数的内存影响，将秒＊10三次方（由于0.00001度为0.036秒）//
	long longsecond = (int)(secondLongitude * 1000);

	remainder = (int)longsecond % 450000;

	return (int)(longsecond / 450000);
	}

	/*
	* 根据纬度计算该点位于实际图幅分割的行序号
	* 纬度：单位 度
	* 余数：单位 千秒
	*/
	int Grid::CalculateRealRowIndex(double dLatitude, int& remainder)
	{
	//理想行号//
	int idealRow = CalculateIdealRowIndex(dLatitude, remainder);

	int realRow = idealRow;

	switch (idealRow % 3)	//三个一组的余数//
	{
	case 0:	//第一行//
		{
			if (300000 - remainder <= 12)	//余数距离上框小于0.012秒//
				realRow++;
		}
		break;
	case 1: //第二行//
		break;
	case 2: //第三行//
		{
			if (remainder < 12)	//余数距离下框小于等于0.012秒//
				realRow--;
		}
		break;
	}

	return realRow;
	}

	/*
	* 根据经度计算该点位于实际图幅分割的列序号
	* 经度：单位 度
	* 余数：单位 千秒
	*/
	int Grid::CalculateRealColumnIndex(double dLongitude, int& remainder)
	{
	return CalculateIdealColumnIndex(dLongitude, remainder);
	}

	std::vector<std::string> Grid::CalculateGridsByBox( double dminLon, double dmaxLon, double dminLat, double dmaxLat )
	{
		std::vector<std::string> reVector;
		std::set<std::string> reSet;
		if(dminLon>dmaxLon)
		{
			double temlon = dminLon;
			dminLon = dmaxLon;
			dmaxLon = temlon;
		}

		if (dminLat>dmaxLat)
		{
			double temlat = dminLat;
			dminLat = dmaxLat;
			dmaxLat = temlat;
		}

		long iMinLon = dminLon*3600;
		long iMaxLon = dmaxLon*3600;
		long iMinLat = dminLat*3600;
		long iMaxLat = dmaxLat*3600;

		long addLon = 112;
		long addLat = 75;

		long nLonExpand = addLon;
		long nLatExpand = addLat;

		for (long i=iMinLon-nLonExpand;i<=iMaxLon+nLonExpand;i+=addLon)
		{
			for (long j=iMinLat-nLatExpand;j<=iMaxLat+nLatExpand;j+=addLat)
			{
				reSet.insert(LocationToGrid(SecondToDegree(i), SecondToDegree(j)));
			}
		}
		std::set<std::string>::iterator itor=reSet.begin();
		while(itor!=reSet.end())
		{
			reVector.push_back(*itor);
			itor++;
		}
		return reVector;
		
	}

} // namespace Editor
