#ifndef FM_SDK_EndianHelper_h
#define FM_SDK_EndianHelper_h

#include "stddef.h"

namespace EditorGeometry
{
	enum ENDIAN_TYPE { BigEndian, LittleEndian };

	template <typename T>
	T swap_endian(T u)
	{
		union
		{
			T u;
			unsigned char u8[sizeof(T)];
		} source, dest;

		source.u = u;

		for (size_t k = 0; k < sizeof(T); k++)
			dest.u8[k] = source.u8[sizeof(T) - k - 1];

		return dest.u;
	};

	class EndianHelper
	{
	public:

		static ENDIAN_TYPE HostEdian()
		{
			int i=1;

			if(((char*)&i)[0]==1) return LittleEndian;

			else return BigEndian;		
		}

		static short ToLittleEndian(short i)
		{
			if(HostEdian()==BigEndian)
				i=swap_endian<short>(i);

			return i;
		}

		static unsigned short ToLittleEndian(unsigned short i)
		{
			if(HostEdian()==BigEndian)
				i=swap_endian<unsigned short>(i);

			return i;
		}

		static int ToLittleEndian(int i)
		{
			if(HostEdian()==BigEndian)
				i=swap_endian<int>(i);

			return i;
		}

		static unsigned int ToLittleEndian(unsigned int i)
		{
			if(HostEdian()==BigEndian)
				i=swap_endian<unsigned int>(i);

			return i;
		}

		static long long ToLittleEndian(long long i)
		{
			if(HostEdian()==BigEndian)
				i=swap_endian<long long>(i);

			return i;
		}

		static unsigned long long ToLittleEndian(unsigned long long i)
		{
			if(HostEdian()==BigEndian)
				i=swap_endian<unsigned long long>(i);

			return i;
		}

		static short ToBigEndian(short i)
		{
			if(HostEdian()==LittleEndian)
				i=swap_endian<short>(i);

			return i;
		}

		static unsigned short ToBigEndian(unsigned short i)
		{
			if(HostEdian()==LittleEndian)
				i=swap_endian<unsigned short>(i);

			return i;
		}

		static int ToBigEndian(int i)
		{
			if(HostEdian()==LittleEndian)
				i=swap_endian<int>(i);

			return i;
		}

		static unsigned int ToBigEndian(unsigned int i)
		{
			if(HostEdian()==LittleEndian)
				i=swap_endian<unsigned int>(i);

			return i;
		}

		static long long ToBigEndian(long long i)
		{
			if(HostEdian()==LittleEndian)
				i=swap_endian<long long>(i);

			return i;
		}

		static unsigned long long ToBigEndian(unsigned long long i)
		{
			if(HostEdian()==LittleEndian)
				i=swap_endian<unsigned long long>(i);

			return i;
		}

	///////////////////////////

		static short FromLittleEndian(short i)
		{
			if(HostEdian()==BigEndian)
				i=swap_endian<short>(i);

			return i;
		}

		static unsigned short FromLittleEndian(unsigned short i)
		{
			if(HostEdian()==BigEndian)
				i=swap_endian<unsigned short>(i);

			return i;
		}

		static int FromLittleEndian(int i)
		{
			if(HostEdian()==BigEndian)
				i=swap_endian<int>(i);

			return i;
		}

		static unsigned int FromLittleEndian(unsigned int i)
		{
			if(HostEdian()==BigEndian)
				i=swap_endian<unsigned int>(i);

			return i;
		}

		static long long FromLittleEndian(long long i)
		{
			if(HostEdian()==BigEndian)
				i=swap_endian<long long>(i);

			return i;
		}

		static unsigned long long FromLittleEndian(unsigned long long i)
		{
			if(HostEdian()==BigEndian)
				i=swap_endian<unsigned long long>(i);

			return i;
		}

		static short FromBigEndian(short i)
		{
			if(HostEdian()==LittleEndian)
				i=swap_endian<short>(i);

			return i;
		}

		static unsigned short FromBigEndian(unsigned short i)
		{
			if(HostEdian()==LittleEndian)
				i=swap_endian<unsigned short>(i);

			return i;
		}

		static int FromBigEndian(int i)
		{
			if(HostEdian()==LittleEndian)
				i=swap_endian<int>(i);

			return i;
		}

		static unsigned int FromBigEndian(unsigned int i)
		{
			if(HostEdian()==LittleEndian)
				i=swap_endian<unsigned int>(i);

			return i;
		}

		static long long FromBigEndian(long long i)
		{
			if(HostEdian()==LittleEndian)
				i=swap_endian<long long>(i);

			return i;
		}

		static unsigned long long FromBigEndian(unsigned long long i)
		{
			if(HostEdian()==LittleEndian)
				i=swap_endian<unsigned long long>(i);

			return i;
		}
	};
}

#endif
