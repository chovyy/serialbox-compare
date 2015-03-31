#ifndef NEW_SERIALIZER_COMPARE_SHARED
#define NEW_SERIALIZER_COMPARE_SHARED

void readFile(const std::string directory, const std::string basename, std::string field, DataFieldInfo& info, double*& data)
{
	Serializer serializer;
	serializer.Init(directory, basename, SerializerOpenModeRead);

	Savepoint savepoint;
    savepoint.Init("diffuse_hori_velocity");

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
