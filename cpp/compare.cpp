/*
 * compare.cpp
 *
 *  Created on: Jan 14, 2015
 *      Author: hovy
 */

#include <unistd.h>    /* for getopt */
#include <algorithm>   /* for max, min */
#include <cmath>       /* for abs */
#include <iostream>    /* for cout */
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
bool compareData(const Serializer& serializer1, const Serializer& serializer2, const DataFieldInfo& info, const std::string savepointName,
		         Bounds iBounds, Bounds jBounds, Bounds kBounds, Bounds lBounds, double tolerance)
{
	T* data1;
	readData(serializer1, info, savepointName, data1);

	T* data2;
	readData(serializer2, info, savepointName, data2);

	int iSize = info.iSize();
	int iLower = std::max(0, iBounds.lower);
	int iUpper = std::min(iSize - 1, iBounds.upper);
	int jSize = info.jSize();
	int jLower = std::max(0, jBounds.lower);
	int jUpper = std::min(jSize - 1, jBounds.upper);
	int kSize = info.kSize();
	int kLower = std::max(0, kBounds.lower);
	int kUpper = std::min(kSize - 1, kBounds.upper);
	int lSize = info.lSize();
	int lLower = std::max(0, lBounds.lower);
	int lUpper = std::min(lSize - 1, lBounds.upper);

	bool equal = true;

	for (int i = iLower; i <= iUpper; ++i)
	{
		for (int j = jLower; j <= jUpper; ++j)
		{
			for (int k = kLower; k <= kUpper; ++k)
			{
				for (int l = lLower; l <= lUpper; ++l)
				{
					int index = i*jSize*kSize*lSize + j*kSize*lSize + k*lSize + l;
					if (data1[index] != data2[index] &&
					    !(data1[index] != data1[index] && data2[index] != data2[index]) && //NaN
						std::abs(data1[index] - data2[index]) > tolerance)
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

int compare(const std::string directory1, const std::string basename1,
		    const std::string directory2, const std::string basename2,
			const std::string savepointName, const std::string field,
			Bounds iBounds, Bounds jBounds, Bounds kBounds, Bounds lBounds, double tolerance, bool infoOnly)
{
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
	else if (infoOnly)
	{
		return 0;
	}

	//TODO Bounds + Tolerance
	if (info1.type() == "integer")
	{
		equal = compareData<int>(serializer1, serializer2, info1, savepointName, iBounds, jBounds, kBounds, lBounds, tolerance);
	}
	else if (info1.type() == "double")
	{
		equal = compareData<double>(serializer1, serializer2, info1, savepointName, iBounds, jBounds, kBounds, lBounds, tolerance);
	}
	else
	{
		std::cerr << "Unsupported type: " << info1.type() << std::endl;
		return 2;
	}

	return (int) (!equal);
}

int main (int argc, char **argv) {
    int opt;
    std::string i = ":";
    std::string j = ":";
    std::string k = ":";
    std::string l = ":";
    bool infoOnly = false;
    double tolerance = 0.0;
    while ( (opt = getopt(argc, argv, "i:j:k:l:t:q")) != -1) {
        switch (opt)
        {
        case 'i':
			i = optarg;
            break;
        case 'j':
			j = optarg;
            break;
        case 'k':
			k = optarg;
            break;
        case 'l':
			l = optarg;
            break;
        case 't':
        	tolerance = strtod(optarg, NULL);
            break;
        case 'q':
			infoOnly = true;
            break;
        }
    }

	Bounds iBounds = string2bounds(i);
	Bounds jBounds = string2bounds(j);
	Bounds kBounds = string2bounds(k);
	Bounds lBounds = string2bounds(l);

    //TODO Nur die Dateien als Parameter übergeben und directory, basename und field selbst herausfinden
	const std::string directory1 = argv[optind++];
	const std::string basename1 = argv[optind++];
	const std::string directory2 = argv[optind++];
	const std::string basename2 = argv[optind++];
	const std::string savepointName = argv[optind++];
	const std::string field = argv[optind++];

	return compare(directory1, basename1, directory2, basename2, savepointName, field,
				   iBounds, jBounds, kBounds, lBounds, tolerance, infoOnly);
}
