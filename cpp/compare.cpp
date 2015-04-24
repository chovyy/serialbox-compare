/*
 * compare.cpp
 *
 *  Created on: Jan 14, 2015
 *      Author: hovy
 */

#include <iostream>
#include "Serializer.h"
#include "shared.h"

bool compareInfo(const DataFieldInfo info1, const DataFieldInfo info2)
{
	bool equal = true;
	if (info1.type() != info2.type())
	{
		equal = false;
		std::cout << "Type: " << info1.type() << " != " << info2.type() << std::endl;
	}
	if (info1.rank() != info2.rank())
	{
		equal = false;
		std::cout << "Rank: " << info1.rank() << " != " << info2.rank() << std::endl;
	}
	if (info1.bytesPerElement() != info2.bytesPerElement())
	{
		equal = false;
		std::cout << "Bytes per Element: " << info1.bytesPerElement() << " != " << info2.bytesPerElement() << std::endl;
	}
	if (info1.iSize() != info2.iSize())
	{
		equal = false;
		std::cout << "iSize: " << info1.iSize() << " != " << info2.iSize() << std::endl;
	}
	if (info1.jSize() != info2.jSize())
	{
		equal = false;
		std::cout << "jSize: " << info1.jSize() << " != " << info2.jSize() << std::endl;
	}
	if (info1.kSize() != info2.kSize())
	{
		equal = false;
		std::cout << "kSize: " << info1.kSize() << " != " << info2.kSize() << std::endl;
	}
	if (info1.lSize() != info2.lSize())
	{
		equal = false;
		std::cout << "lSize: " << info1.lSize() << " != " << info2.lSize() << std::endl;
	}

	return equal;
}

template <typename T>
bool compareData(const Serializer& serializer1, const Serializer& serializer2, const DataFieldInfo& info, const std::string savepointName)
{
	T* data1;
	readData(serializer1, info, savepointName, data1);

	T* data2;
	readData(serializer2, info, savepointName, data2);

	int iSize = info.iSize();
	int jSize = info.jSize();
	int kSize = info.kSize();
	int lSize = info.lSize();

	bool equal = true;

	for (int i = 0; i < iSize; ++i)
	{
		for (int j = 0; j < jSize; ++j)
		{
			for (int k = 0; k < kSize; ++k)
			{
				for (int l = 0; l < lSize; ++l)
				{
					int index = i*jSize*kSize*lSize + j*kSize*lSize + k*lSize + l;
					if (data1[index] != data2[index] &&
					    !(data1[index] != data1[index] && data2[index] != data2[index])) //NaN
					{
						equal = false;
						std::cout << "(" << i << ", " << j << ", " << k << ", " << l << ") : ";
						std::cout << data1[index] << " != " << data2[index] << std::endl;
					}
				}
			}
		}
	}

	return equal;
}

int main(int argc, char* argv[])
{
	const std::string directory1 = argv[1];
	const std::string basename1 = argv[2];
	const std::string directory2 = argv[3];
	const std::string basename2 = argv[4];
	const std::string savepoint = argv[5];
	const std::string field = argv[6];

	Serializer serializer1;
	DataFieldInfo info1;
	readInfo(directory1, basename1, field, serializer1, info1);

	Serializer serializer2;
	DataFieldInfo info2;
	readInfo(directory2, basename2, field, serializer2, info2);

	bool equal = compareInfo(info1, info2);
	if (!equal)
	{
		return 1;
	}

	if (info1.type() == "integer")
	{
		equal = compareData<int>(serializer1, serializer2, info1, savepoint);
	}
	else if (info1.type() == "double")
	{
		equal = compareData<double>(serializer1, serializer2, info1, savepoint);
	}
	else
	{
		std::cerr << "Unsupported type: " << info1.type() << std::endl;
		return 2;
	}

	return (int) (!equal);
}
