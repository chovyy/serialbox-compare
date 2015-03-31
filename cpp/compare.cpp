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

bool compareData(const double* data1, const double* data2, int iSize, int jSize, int kSize, int lSize)
{
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
					if (data1[index] != data2[index])
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

bool compare(const DataFieldInfo info1, const double* data1, const DataFieldInfo info2, const double* data2)
{
	return compareInfo(info1, info2) && compareData(data1, data2, info1.iSize(), info1.jSize(), info1.kSize(), info1.lSize());
}

int main(int argc, char* argv[])
{
	const std::string directory1 = argv[1];
	const std::string basename1 = argv[2];
	const std::string directory2 = argv[3];
	const std::string basename2 = argv[4];
	const std::string field = argv[5];

	double* data1;
	double* data2;
	DataFieldInfo info1;
	DataFieldInfo info2;

	readFile(directory1, basename1, field, info1, data1);
	readFile(directory2, basename2, field, info2, data2);
	compare(info1, data1, info2, data2);

	return 0;
}
