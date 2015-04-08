#include <cstdlib>
#include <algorithm>
#include <limits>

#ifndef NEW_SERIALIZER_COMPARE_SHARED
#define NEW_SERIALIZER_COMPARE_SHARED

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

void readFile(const std::string directory, const std::string basename, const std::string savepointName, std::string field, DataFieldInfo& info, double*& data)
{
	Serializer serializer;
	serializer.Init(directory, basename, SerializerOpenModeRead);

	Savepoint savepoint;
    savepoint.Init(savepointName);

    info = serializer.FindField(field);

    int iSize = info.iSize();
    int jSize = info.jSize();
    int kSize = info.kSize();
    int lSize = info.lSize();
    int fieldLength = info.bytesPerElement();

    int lStride = fieldLength;
    int kStride = lSize * lStride;
    int jStride = kSize * kStride;
    int iStride = jSize * jStride;

    data = new double[iSize * jSize * kSize * lSize];
    serializer.ReadField(field, savepoint, data, iStride, jStride, kStride, lStride);
}

#endif
