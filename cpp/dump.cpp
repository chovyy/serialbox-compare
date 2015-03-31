/*
 * compare.cpp
 *
 *  Created on: Jan 14, 2015
 *      Author: hovy
 */

#include <iostream>
#include "Serializer.h"
#include "shared.h"

void dumpInfo(const DataFieldInfo info)
{
	std::cout << "iSize: " << info.iSize() << std::endl;
	std::cout << "jSize: " << info.jSize() << std::endl;
	std::cout << "kSize: " << info.kSize() << std::endl;
	std::cout << "lSize: " << info.lSize() << std::endl;
}

void dumpData(const double* data, int iSize, int jSize, int kSize, int lSize)
{
	bool equal = true;

	for (int i = 0; i < iSize; ++i)
	{
		for (int j = 0; j < jSize; ++j)
		{
			if (kSize > 1) std::cout << "[ ";
			for (int k = 0; k < kSize; ++k)
			{
				if (lSize > 1) std::cout << "( ";
				for (int l = 0; l < lSize; ++l)
				{
					int index = i*jSize*kSize*lSize + j*kSize*lSize + k*lSize + l;
					std::cout << data[index];
					if (l < lSize - 1) std::cout << ", ";
				}
				if (lSize > 1) std::cout << " )";
			}
			if (kSize > 1) std::cout << " ]";
		}
		std::cout << std::endl;
	}
}

void dump(const DataFieldInfo info, const double* data)
{
	dumpInfo(info);
	std::cout << std::endl;
	dumpData(data, info.iSize(), info.jSize(), info.kSize(), info.lSize());
}

int main(int argc, char* argv[])
{
	const std::string directory = argv[1];
	const std::string basename = argv[2];
	const std::string field = argv[3];

	double* data;
	DataFieldInfo info;

	readFile(directory, basename, field, info, data);
	dump(info, data);

	return 0;
}
