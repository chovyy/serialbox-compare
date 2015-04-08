/*
 * compare.cpp
 *
 *  Created on: Jan 14, 2015
 *      Author: hovy
 */

#include <unistd.h>    /* for getopt */
#include <algorithm>   /* for max, min */
#include <iostream>    /* for cout */
#include "Serializer.h"
#include "shared.h"


void dumpInfo(const DataFieldInfo info, Bounds iBounds, Bounds jBounds, Bounds kBounds, Bounds lBounds)
{
	iBounds.upper = std::min(iBounds.upper, info.iSize() - 1);
	jBounds.upper = std::min(jBounds.upper, info.jSize() - 1);
	kBounds.upper = std::min(kBounds.upper, info.kSize() - 1);
	lBounds.upper = std::min(lBounds.upper, info.lSize() - 1);

	std::cout << "iSize: " << info.iSize() << " " << iBounds << std::endl;
	std::cout << "jSize: " << info.jSize() << " " << jBounds << std::endl;
	std::cout << "kSize: " << info.kSize() << " " << kBounds << std::endl;
	std::cout << "lSize: " << info.lSize() << " " << lBounds << std::endl;
}

void dumpData(const double* data, int iSize, const Bounds iBounds, int jSize, const Bounds jBounds,
								  int kSize, const Bounds kBounds, int lSize, const Bounds lBounds)
{
	bool equal = true;

	for (int i = std::max(0, iBounds.lower); i <= std::min(iSize - 1, iBounds.upper); ++i)
	{
		for (int j = std::max(0, jBounds.lower); j <= std::min(jSize - 1, jBounds.upper); ++j)
		{
			if (kSize > 1) std::cout << "[ ";
			for (int k = std::max(0, kBounds.lower); k <= std::min(kSize - 1, kBounds.upper); ++k)
			{
				if (lSize > 1) std::cout << "( ";
				for (int l = std::max(0, lBounds.lower); l <= std::min(lSize - 1, lBounds.upper); ++l)
				{
					int index = i*jSize*kSize*lSize + j*kSize*lSize + k*lSize + l;
					std::cout << data[index];
					if (l < lSize - 1) std::cout << ", ";
				}
				if (lSize > 1) std::cout << " )";
				if (k < kSize - 1) std::cout << ", ";
			}
			if (kSize > 1) std::cout << " ]";
		}
		std::cout << std::endl;
	}
}

void dump(const std::string directory, const std::string basename,
	      const std::string field, const std::string savepoint,
		  Bounds iBounds, Bounds jBounds, Bounds kBounds, Bounds lBounds)
{
	double* data;
	DataFieldInfo info;

	readFile(directory, basename, field, savepoint, info, data);

	dumpInfo(info, iBounds, jBounds, kBounds, lBounds);
	std::cout << std::endl;
	dumpData(data, info.iSize(), iBounds, info.jSize(), jBounds, info.kSize(), kBounds, info.lSize(), lBounds);
}

int main (int argc, char **argv) {
    int opt;
    std::string i = ":";
    std::string j = ":";
    std::string k = ":";
    std::string l = ":";
    while ( (opt = getopt(argc, argv, "i:j:k:l:")) != -1) {
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
        }
    }

    //TODO Nur die Datei als Parameter übergeben und directory, basename und field selbst herausfinden
    const std::string directory = argv[optind++];
	const std::string basename = argv[optind++];
	const std::string field = argv[optind++];
	const std::string savepoint = argv[optind++];

	Bounds iBounds = string2bounds(i);
	Bounds jBounds = string2bounds(j);
	Bounds kBounds = string2bounds(k);
	Bounds lBounds = string2bounds(l);

	dump(directory, basename, field, savepoint, iBounds, jBounds, kBounds, lBounds);

    exit (0);
}
