#ifndef NEW_SERIALIZER_COMPARE_SHARED
#define NEW_SERIALIZER_COMPARE_SHARED

#include <cstdlib>
#include <algorithm>
#include <limits>
#include <sys/stat.h>
#include "Serializer.h"

using namespace ser;

struct Bounds {
	int lower, upper;
};

std::ostream& operator << (std::ostream &o, const Bounds &bounds)
{
	o << "(" << bounds.lower << ", " << bounds.upper << ")";
	return o;
}

Bounds string2bounds(std::string boundString)
{
	Bounds bounds = { 0, std::numeric_limits<int>::max() };

	if (boundString[0] != ':')
	{
		const char *s = boundString.c_str();
		char *t;
		bounds.lower = strtol(s, &t, 10);
		boundString = std::string(t);
		if (boundString.length() == 0)
		{
			bounds.upper = bounds.lower;
		}
		else
		{
			boundString = boundString.substr(1);
		}
	}
	else
	{
		boundString = boundString.substr(1);
	}

	if (boundString.length() > 0)
	{
		const char *s = boundString.c_str();
		char *t;
		bounds.upper = strtol(s, &t, 10);
	}

	if (bounds.upper < bounds.lower)
	{
		std::swap(bounds.upper, bounds.lower);
	}

	return bounds;
}

bool fileExists(const std::string& path)
{
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0);
}

bool splitFilePathDat(const std::string& path, std::string& directory, std::string& basename, std::string& field)
{
	if (!fileExists(path))
	{
		return false;
	}

	int size = path.length();
	if (size < 7)
	{
		return false;
	}

	if (path.substr(size - 4, 4) != ".dat")
	{
		return false;
	}

	int last_ = path.find_last_of("_");
	if (last_ == std::string::npos || last_ > size - 6)
	{
		return false;
	}

	std::string jsonPath;
	do
	{
		jsonPath = path.substr(0, last_) + ".json";
	}
	while (!fileExists(jsonPath) && (last_ = path.find_last_of("_", last_ - 1)) != std::string::npos);

	if (last_ == std::string::npos)
	{
		return false;
	}

	int lastSlash = path.find_last_of("/\\");
	std::string file;

	if (lastSlash == std::string::npos)
	{
		directory = ".";
		file = path;
	}
	else
	{
		directory = path.substr(0, lastSlash);
		file = path.substr(lastSlash + 1);
	}

	basename = path.substr(lastSlash + 1, (last_ - lastSlash) - 1);
	field = path.substr(last_ + 1, size - last_ - 5);

	return true;
}

bool splitFilePathJson(const std::string& path, std::string& directory, std::string& basename)
{
	if (!fileExists(path))
	{
		return false;
	}

	int size = path.length();
	if (size < 6)
	{
		return false;
	}

	if (path.substr(size - 5, 5) != ".json")
	{
		return false;
	}

	int lastSlash = path.find_last_of("/\\");
	std::string file;

	if (lastSlash == std::string::npos)
	{
		directory = ".";
		file = path;
	}
	else
	{
		directory = path.substr(0, lastSlash);
		file = path.substr(lastSlash + 1);
	}

	basename = path.substr(lastSlash + 1, (size - lastSlash) - 6);

	return true;
}

void readInfo(const std::string& directory, const std::string& basename, const std::string& field, Serializer& serializer, DataFieldInfo& info)
{
	serializer.Init(directory, basename, SerializerOpenModeRead);
	info = serializer.FindField(field);
}

template <typename T>
void readData(const Serializer& serializer, const DataFieldInfo& info, const std::string& savepointName, T*& data)
{
	Savepoint savepoint;
    savepoint.Init(savepointName);

    int iSize = info.iSize();
    int jSize = info.jSize();
    int kSize = info.kSize();
    int lSize = info.lSize();
    int fieldLength = info.bytesPerElement();
    std::string fieldName = info.name();

    int lStride = fieldLength;
    int kStride = lSize * lStride;
    int jStride = kSize * kStride;
    int iStride = jSize * jStride;

    data = new T[iSize * jSize * kSize * lSize];
    serializer.ReadField(fieldName, savepoint, data, iStride, jStride, kStride, lStride);
}

#endif
