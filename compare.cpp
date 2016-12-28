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
#include "shared.h"

bool compareInfo(const DataFieldInfo& info1, const DataFieldInfo& info2, std::ostream& out = std::cout)
{
	bool equal = true;
	if (info1.type() != info2.type())
	{
		equal = false;
		out << "Type: " << info1.type() << " != " << info2.type() << std::endl;
	}
	if (info1.rank() != info2.rank())
	{
		equal = false;
		out << "Rank: " << info1.rank() << " != " << info2.rank() << std::endl;
	}
	if (info1.bytesPerElement() != info2.bytesPerElement())
	{
		equal = false;
		out << "Bytes per Element: " << info1.bytesPerElement() << " != " << info2.bytesPerElement() << std::endl;
	}
	if (info1.iSize() != info2.iSize())
	{
		equal = false;
		out << "iSize: " << info1.iSize() << " != " << info2.iSize() << std::endl;
	}
	if (info1.jSize() != info2.jSize())
	{
		equal = false;
		out << "jSize: " << info1.jSize() << " != " << info2.jSize() << std::endl;
	}
	if (info1.kSize() != info2.kSize())
	{
		equal = false;
		out << "kSize: " << info1.kSize() << " != " << info2.kSize() << std::endl;
	}
	if (info1.lSize() != info2.lSize())
	{
		equal = false;
		out << "lSize: " << info1.lSize() << " != " << info2.lSize() << std::endl;
	}

	return equal;
}

template <typename T>
bool compareData(const Serializer& serializer1, const Serializer& serializer2,
				 const std::string& savepointName1, const std::string& savepointName2,
				 const DataFieldInfo& info,
		         const Bounds& iBounds, const Bounds& jBounds, const Bounds& kBounds, const Bounds& lBounds, double tolerance,
				 std::ostream& out = std::cout)
{
	T* data1;
	readData(serializer1, info, savepointName1, data1);

	T* data2;
	readData(serializer2, info, savepointName2, data2);

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
						out << "(" << i << ", " << j << ", " << k << ", " << l << ") : ";
						out << data1[index] << " != " << data2[index] << std::endl;
					}
				}
			}
		}
	}

	return equal;
}

int compare(const std::string& directory1, const std::string& basename1, const std::string& savepointName1,
		    const std::string& directory2, const std::string& basename2, const std::string& savepointName2,
			const std::string& field,
			const Bounds& iBounds, const Bounds& jBounds, const Bounds& kBounds, const Bounds& lBounds, double tolerance, bool infoOnly,
			std::ostream& out = std::cout)
{
	Serializer serializer1;
	DataFieldInfo info1;
	readInfo(directory1, basename1, field, serializer1, info1);

	Serializer serializer2;
	DataFieldInfo info2;
	readInfo(directory2, basename2, field, serializer2, info2);

	bool equal = compareInfo(info1, info2, out);
	if (!equal)
	{
		return 1;
	}
	else if (infoOnly)
	{
		return 0;
	}

	if (info1.type() == "int")
	{
		equal = compareData<int>(serializer1, serializer2, savepointName1, savepointName2, info1, iBounds, jBounds, kBounds, lBounds, tolerance, out);
	}
	else if (info1.type() == "double")
	{
		equal = compareData<double>(serializer1, serializer2, savepointName1, savepointName2, info1, iBounds, jBounds, kBounds, lBounds, tolerance, out);
	}
	else if (info1.type() == "float")
	{
		equal = compareData<float>(serializer1, serializer2, savepointName1, savepointName2, info1, iBounds, jBounds, kBounds, lBounds, tolerance, out);
	}
	else
	{
		std::cerr << "Unsupported type: " << info1.type() << std::endl;
		return 2;
	}

	return (int) (!equal);
}

int compareAll(const std::string& directory1, const std::string& basename1, const std::string& savepointName1,
		       const std::string& directory2, const std::string& basename2, const std::string& savepointName2,
			   double tolerance, bool infoOnly, bool quiet)
{
	Bounds iBounds = string2bounds(":");
	Bounds jBounds = string2bounds(":");
	Bounds kBounds = string2bounds(":");
	Bounds lBounds = string2bounds(":");

	Serializer serializer1;
	serializer1.Init(directory1, basename1, SerializerOpenModeRead);

	Serializer serializer2;
	serializer2.Init(directory2, basename2, SerializerOpenModeRead);

	Savepoint savepoint;
	savepoint.Init(savepointName1);

	int total = 0;
	for (std::string field1 : serializer1.FieldsAtSavepoint(savepoint))
	{
		if (!serializer2.HasField(field1))
		{
			std::cout << "*** Field " << field1 << " is missing in serializer2" << std::endl;
		}
		else
		{
			std::ostringstream buffer;
			int result = compare(directory1, basename1, savepointName1, directory2, basename2, savepointName2, field1,
								 iBounds, jBounds, kBounds, lBounds, tolerance, infoOnly, buffer);
			if (result > 0)
			{
				std::cout << "*** Field: " << field1 << " ***" << std::endl;
				if (! quiet)
				{
					std::cout << buffer.str();
				}
			}

			total = std::max(total, result);
		}

	}

	return total;
}

int main (int argc, char **argv)
{
	int opt;
    std::string i = ":";
    std::string j = ":";
    std::string k = ":";
    std::string l = ":";
    double tolerance = 0.0;
    std::string savepointName2 = "";
    bool infoOnly = false;
    bool quiet = false;
    while ( (opt = getopt(argc, argv, "i:j:k:l:t:s:oq")) != -1) {
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
        case 'o':
			infoOnly = true;
            break;
        case 'q':
        	quiet = true;
            break;
        case 's':
			savepointName2 = optarg;
            break;
        }
    }

	Bounds iBounds = string2bounds(i);
	Bounds jBounds = string2bounds(j);
	Bounds kBounds = string2bounds(k);
	Bounds lBounds = string2bounds(l);

	if (argc - optind < 3)
	{
		std::cerr <<
				"Usage:\n" <<
				argv[0] << " [-i interval] [-j interval] [-k interval] [-l interval] [-t tolerance] [-o] [-q] [-s savepoint2]" <<
						   " FILE1 FILE2 SAVEPOINT1\n";
		return 3;
	}

	std::string filepath1 = argv[optind++];
	std::string filepath2 = argv[optind++];
	std::string savepointName1 = argv[optind++];

	if (savepointName2 == "")
	{
		savepointName2 = savepointName1;
	}

	std::string directory1;
	std::string basename1;
	std::string field1 = "";
	bool json = false;
	if (!splitFilePathDat(filepath1, directory1, basename1, field1))
	{
		json = true;
		if (!splitFilePathJson(filepath1, directory1, basename1))
		{
			std::cerr << "Invalid file 1: " << filepath1 << std::endl;
			return 2;
		}
	}

	std::string directory2;
	std::string basename2;
	std::string field2 = "";
	if (   !json && !splitFilePathDat(filepath2, directory2, basename2, field2)
         || json && !splitFilePathJson(filepath2, directory2, basename2))
	{
		std::cerr << "Invalid file 2: " << filepath2 << std::endl;
		return 2;
	}

	if (!json && field1 != field2)
	{
		std::cout << "Field: " << field1 << " != " << field2 << std::endl;
		return 1;
	}

	if (json)
	{
		return compareAll(directory1, basename1, savepointName1, directory2, basename2, savepointName2,
				          tolerance, infoOnly, quiet);
	}
	else
	{
		return compare(directory1, basename1, savepointName1, directory2, basename2, savepointName2, field1,
					   iBounds, jBounds, kBounds, lBounds, tolerance, infoOnly);
	}
}
