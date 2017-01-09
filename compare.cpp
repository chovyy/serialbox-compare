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


bool checkSavepoints(const Serializer& serializer1, const Serializer& serializer2, std::string& savepointName1, std::string& savepointName2)
{
	std::vector<std::string> savepointNames1 = savepointNames(serializer1);
	if (savepointNames1.size() == 0)
	{
		std::cerr << "Serializer 1 contains no Savepoints." << std::endl;
		return false;
	}

	std::vector<std::string> savepointNames2 = savepointNames(serializer2);
	if (savepointNames2.size() == 0)
	{
		std::cerr << "Serializer 2 contains no Savepoints." << std::endl;
		return false;
	}

	bool print = false;

	if (savepointName1 == "")
	{
		print = true;

		if (savepointNames1.size() == 1)
		{
			savepointName1 = savepointNames1.front();
		}
		else
		{
			std::vector<std::string> commonNames(std::min(savepointNames1.size(), savepointNames2.size()));
			std::vector<std::string>::iterator it;

			std::sort(savepointNames1.begin(), savepointNames1.end());
			std::sort(savepointNames2.begin(), savepointNames2.end());

			it = std::set_intersection(savepointNames1.begin(), savepointNames1.end(), savepointNames2.begin(), savepointNames2.end(), commonNames.begin());
			commonNames.resize(it - commonNames.begin());

			if (commonNames.size() == 1)
			{
				savepointName1 = commonNames.front();
				savepointName2 = commonNames.front();
			}
			else if (commonNames.empty())
			{
				std::cerr << "No common Savepoints in Serializer 1 and 2." << std::endl;
				return false;
			}
			else
			{
				std::cerr << "Cannot decide on Savepoint; candidates: ";
				std::string separator = "";
				for(std::vector<std::string>::iterator it = commonNames.begin(); it != commonNames.end(); ++it) {
					std::cerr << separator << *it;
					separator = ", ";
				}
				std::cerr << std::endl;
				return false;
			}
		}
	}
	else
	{
		if (std::find(savepointNames1.begin(), savepointNames1.end(), savepointName1) == savepointNames1.end())
		{
			std::cerr << "Savepoint \"" << savepointName1 << "\" not found in Serializer 1." << std::endl;
			return false;
		}
	}

	if (savepointName2 == "")
	{
		print = true;

		if (savepointNames2.size() == 1)
		{
			savepointName2 = savepointNames2.front();
		}
		else
		{
			savepointName2 = savepointName1;
		}
	}

	if (std::find(savepointNames2.begin(), savepointNames2.end(), savepointName2) == savepointNames2.end())
	{
		std::cerr << "Savepoint \"" << savepointName2 << "\" not found in Serializer 2." << std::endl;
		return false;
	}

	if (print)
	{
		if (savepointName1 == savepointName2)
		{
			std::cout << "Savepoint: " << savepointName1 << std::endl;
		}
		else
		{
			std::cout << "Savepoint 1: " << savepointName1 << std::endl;
			std::cout << "Savepoint 2: " << savepointName2 << std::endl;
		}
	}

	return true;
}

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
						((data1[index] != data1[index] ^ data2[index] != data2[index]) || //Nan
						  std::abs(data1[index] - data2[index]) > tolerance)
					)
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

int compare(const std::string& directory1, const std::string& basename1, std::string& savepointName1,
		    const std::string& directory2, const std::string& basename2, std::string& savepointName2,
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

	if (!checkSavepoints(serializer1, serializer2, savepointName1, savepointName2))
	{
		return 1;
	}

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

int compareAll(const std::string& directory1, const std::string& basename1, std::string& savepointName1,
		       const std::string& directory2, const std::string& basename2, std::string& savepointName2,
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

	if (!checkSavepoints(serializer1, serializer2, savepointName1, savepointName2))
	{
		return 1;
	}

	Savepoint savepoint;
	savepoint.Init(savepointName1);

	int total = 0;
	for (std::string field1 : serializer1.FieldsAtSavepoint(savepoint))
	{
		if (!serializer2.HasField(field1))
		{
			std::cout << "*** Field " << field1 << " is missing in Serializer 2" << std::endl;
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
    bool infoOnly = false;
    bool quiet = false;
    while ( (opt = getopt(argc, argv, "i:j:k:l:t:oq")) != -1) {
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
        }
    }

	Bounds iBounds = string2bounds(i);
	Bounds jBounds = string2bounds(j);
	Bounds kBounds = string2bounds(k);
	Bounds lBounds = string2bounds(l);

	if (argc - optind < 2)
	{
		std::cerr <<
				"Usage:" << std::endl <<
				argv[0] << " [-i interval] [-j interval] [-k interval] [-l interval] [-t tolerance] [-o] [-q]" <<
						   " FILE1 FILE2 [SAVEPOINT1 [SAVEPOINT2]]" << std::endl;
		return 3;
	}

	std::string filepath1 = argv[optind++];
	std::string filepath2 = argv[optind++];
    std::string savepointName1 = "";
    std::string savepointName2 = "";
	if (optind < argc)
	{
		savepointName1 = argv[optind++];

		if (optind < argc)
		{
			savepointName2 = argv[optind++];
		}
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
