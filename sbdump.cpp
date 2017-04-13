/*
 * compare.cpp
 *
 *  Created on: Jan 14, 2015
 *      Author: hovy
 */

#include <unistd.h>    /* for getopt */
#include <algorithm>   /* for max, min */
#include <iostream>    /* for cout */

#include "sbshared.h"


bool checkSavepoint(const Serializer& serializer, std::string& savepointName)
{
	std::vector<std::string> savepntNames = savepointNames(serializer);
	if (savepntNames.size() == 0)
	{
		std::cerr << "Serializer contains no Savepoints." << std::endl;
		return false;
	}

	if (savepointName == "")
	{
		if (savepntNames.size() == 1)
		{
			savepointName = savepntNames.front();
		}
		else
		{
			std::cerr << "Cannot decide on Savepoint; candidates: ";
			std::string separator = "";
			for(std::vector<std::string>::iterator it = savepntNames.begin(); it != savepntNames.end(); ++it) {
				std::cerr << separator << *it;
				separator = ", ";
			}
			std::cerr << std::endl;
			return false;
		}
	}
	else
	{
		if (std::find(savepntNames.begin(), savepntNames.end(), savepointName) == savepntNames.end())
		{
			std::cerr << "Savepoint \"" << savepointName << "\" not found in Serializer." << std::endl;
			return false;
		}
	}

	std::cout << "Savepoint: " << savepointName << std::endl;

	return true;
}

void dumpInfo(const DataFieldInfo& info, Bounds iBounds, Bounds jBounds, Bounds kBounds, Bounds lBounds)
{
	iBounds.upper = std::min(iBounds.upper, info.iSize() - 1);
	jBounds.upper = std::min(jBounds.upper, info.jSize() - 1);
	kBounds.upper = std::min(kBounds.upper, info.kSize() - 1);
	lBounds.upper = std::min(lBounds.upper, info.lSize() - 1);

	std::cout << "Field: " << info.name() << std::endl;
	std::cout << "Type: " << info.type() << std::endl;
	std::cout << "Rank: " << info.rank() << std::endl;
	std::cout << "Bytes per Element: " << info.bytesPerElement() << std::endl;
	std::cout << "iSize: " << info.iSize() << " " << iBounds << std::endl;
	std::cout << "jSize: " << info.jSize() << " " << jBounds << std::endl;
	std::cout << "kSize: " << info.kSize() << " " << kBounds << std::endl;
	std::cout << "lSize: " << info.lSize() << " " << lBounds << std::endl;
}

template <typename T>
void dumpData(const Serializer& serializer, const DataFieldInfo& info, const std::string& savepointName,
			  const Bounds& iBounds, const Bounds& jBounds, const Bounds& kBounds, const Bounds& lBounds)
{
	T* data;
	readData(serializer, info, savepointName, data);

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

	for (int i = iLower; i <= iUpper; ++i)
	{
		for (int j = jLower; j <= jUpper; ++j)
		{
			if (kSize > 1) std::cout << "[ ";
			for (int k = kLower; k <= kUpper; ++k)
			{
				if (lSize > 1) std::cout << "( ";
				for (int l = lLower; l <= lUpper; ++l)
				{
					int index = i*jSize*kSize*lSize + j*kSize*lSize + k*lSize + l;
					std::cout << data[index];
					if (l < lUpper) std::cout << ", ";
				}
				if (lSize > 1) std::cout << " )";
				if (k < kUpper) std::cout << ", ";
			}
			if (kSize > 1) std::cout << " ]";
			if (j < jUpper) std::cout << ", ";
		}
		std::cout << std::endl;
	}
}

int dump(const std::string& directory, const std::string& basename,
	     const std::string& field, std::string& savepointName,
		 const Bounds& iBounds, const Bounds& jBounds, const Bounds& kBounds, const Bounds& lBounds, bool infoOnly)
{
	Serializer serializer;
	DataFieldInfo info;

	readInfo(directory, basename, field, serializer, info);

	dumpInfo(info, iBounds, jBounds, kBounds, lBounds);

	if (infoOnly)
	{
		return 0;
	}

	if (!checkSavepoint(serializer, savepointName))
	{
		return 1;
	}

	std::cout << std::endl;

	if (info.type() == "int")
	{
		dumpData<int>(serializer, info, savepointName, iBounds, jBounds, kBounds, lBounds);
	}
	else if (info.type() == "double")
	{
		dumpData<double>(serializer, info, savepointName, iBounds, jBounds, kBounds, lBounds);
	}
	else if (info.type() == "float")
	{
		dumpData<float>(serializer, info, savepointName, iBounds, jBounds, kBounds, lBounds);
	}
	else
	{
		std::cerr << "Unsupported type: " << info.type();
		return 2;
	}

	return 0;
}

void printHelp(std::ostream& out, const std::string& mainCommand)
{
	out << "Usage:" << std::endl <<
	mainCommand << " [-h] [-i interval] [-j interval] [-k interval] [-l interval] [-o] [-q] FILE [SAVEPOINT]\n" << std::endl <<
	std::endl <<
	"Dumps Serialbox field in FILE at SAVEPOINT." << std::endl <<
	"FILE has to be a .dat file and the field name has to be the last part of the file name, separated by _." << std::endl <<
	"If no savepoint is given, the right savepoint is guessed." << std::endl <<
	std::endl <<
	"Options: " << std::endl <<
	"-h : Print this help" << std::endl <<
	"-i/j/k/l INTERVAL : Limit the output to the given interval in the i/j/k/l dimension. The INTERVAL is given by START:END, START:, or :END, e.g. -i 23:42" << std::endl <<
	"-o : Dump only field meta data" << std::endl <<
	"-q : Alias for -o" << std::endl;
}

int main (int argc, char **argv) {
    int opt;
    std::string i = ":";
    std::string j = ":";
    std::string k = ":";
    std::string l = ":";
    bool infoOnly = false;
    while ( (opt = getopt(argc, argv, "i:j:k:l:oqh")) != -1) {
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
        case 'o':
			infoOnly = true;
			break;
        case 'q':
			infoOnly = true;
            break;
        case 'h':
			printHelp(std::cout, argv[0]);
            return 0;
        }
    }

	Bounds iBounds = string2bounds(i);
	Bounds jBounds = string2bounds(j);
	Bounds kBounds = string2bounds(k);
	Bounds lBounds = string2bounds(l);

	if (argc - optind < 1)
	{
		printHelp(std::cerr, argv[0]);
		return 3;
	}

	std::string filepath = argv[optind++];
    std::string savepointName = "";
	if (optind < argc)
	{
		savepointName = argv[optind++];
	}

    std::string directory;
	std::string basename;
	std::string field;
	if (!splitFilePathDat(filepath, directory, basename, field))
	{
		std::cerr << "Invalid file: " << filepath << std::endl;
		return 2;
	}

	std::cout << "Directory: " << directory << std::endl;
	std::cout << "Basename: " << basename << std::endl;

	return dump(directory, basename, field, savepointName, iBounds, jBounds, kBounds, lBounds, infoOnly);
}
