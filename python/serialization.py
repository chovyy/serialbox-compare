from __future__ import print_function
import os, sys
from os.path import join as pjoin
import ctypes

try:
    import numpy as np
except:
    raise Exception('The serialization module depends on numpy')

dirs = (
    pjoin(os.path.dirname(os.path.realpath(__file__)), os.pardir, os.pardir, 'build', 'src', 'serialization_framework_wrapper'),
    '.',
    '/informatik/home/hovy/scratch/lib/new_serialization/lib'
    )

# Load wrapper
wrapper = None
for d in dirs:
    libfile = pjoin(d, 'libSerializationFrameworkWrapper.so')
    try:
        wrapper_try = ctypes.cdll.LoadLibrary(libfile)
        if wrapper_try is not None:
            wrapper = wrapper_try
            break
    except Exception as e:
        pass

if wrapper is None:
    raise ImportError("The serialization library could not be found")

def extract_string(string):
    val = string.encode('ascii') if type(string) == str else string
    return ctypes.c_char_p(val), len(val)

booltypes = (bool, np.bool, )
inttypes = (int, np.int8, np.int16, np.int32, np.int64, np.uint8, np.uint16, np.uint32, np.uint64, )
floattypes = (np.float32, )
doubletypes = (float, np.float64, )
stringtypes = (bytes, str, )

class serializer(object):
    def __init__(self, directory, prefix, openmode):
        # Extract open mode
        openmode = openmode.lower()
        if len(openmode) != 1 or not openmode in 'rwa':
            raise ValueError("openmode can be one of 'r', 'w' or 'a'")

        # Extract strings
        d, dlength = extract_string(directory)
        p, plength = extract_string(prefix)

        self.serializer = wrapper.fs_create_serializer(d, dlength, p, plength, ord(openmode))

    def __del__(self):
        wrapper.fs_destroy_serializer(self.serializer)

    def get_field_size(self, name):
        name, namelength = extract_string(name)

        isize = np.ones((1), dtype=np.int)
        jsize = np.ones((1), dtype=np.int)
        ksize = np.ones((1), dtype=np.int)
        lsize = np.ones((1), dtype=np.int)
        
        wrapper.fs_get_field_size(self.serializer, name, namelength, isize.ctypes.data, jsize.ctypes.data, ksize.ctypes.data, lsize.ctypes.data);
        
        return (isize[0], jsize[0], ksize[0], lsize[0]);

    def load_field(self, name, field, savepoint):
        name, namelength = extract_string(name)

        # Extract strides
        strides = [field.strides[i] if i < len(field.strides) else 0 for i in range(4)]

        wrapper.fs_read_field(self.serializer, savepoint.savepoint, name, namelength,
                field.ctypes.data, strides[0], strides[1], strides[2], strides[3])

    def save_field(self, name, field, savepoint):
        self.register_field(name, field)
        name, namelength = extract_string(name)

        # Extract strides
        strides = [field.strides[i] if i < len(field.strides) else 0 for i in range(4)]

        wrapper.fs_write_field(self.serializer, savepoint.savepoint, name, namelength,
                field.ctypes.data, strides[0], strides[1], strides[2], strides[3])

    def register_field(self, name, field):
        name, namelength = extract_string(name)
        dtype, dtypelength = extract_string(field.dtype.name)
        size = [field.shape[i] if i < len(field.shape) else 1 for i in range(4)]

        wrapper.fs_register_field(self.serializer, name, namelength,
                dtype, dtypelength, field.dtype.itemsize,
                size[0], size[1], size[2], size[3],
                0, 0, 0, 0, 0, 0, 0, 0)

    def add_metainfo(self, key, value):
        key, keylength = extract_string(key)

        if type(value) in booltypes:
            wrapper.fs_add_serializer_metainfo_b(self.serializer, key, keylength, ctypes.c_bool(value))
        elif type(value) in inttypes:
            wrapper.fs_add_serializer_metainfo_i(self.serializer, key, keylength, ctypes.c_int32(value))
        elif type(value) in floattypes:
            wrapper.fs_add_serializer_metainfo_f(self.serializer, key, keylength, ctypes.c_float(value))
        elif type(value) in doubletypes:
            wrapper.fs_add_serializer_metainfo_d(self.serializer, key, keylength, ctypes.c_double(value))
        elif type(value) in stringtypes:
            value, valuelength = extract_string(value)
            wrapper.fs_add_serializer_metainfo_s(self.serializer, key, keylength, value, valuelength)
        else:
            raise AttributeError("Error: type of value not supported")


    def add_field_metainfo(self, fieldname, key, value):
        fieldname, fieldnamelength = extract_string(fieldname)
        key, keylength = extract_string(key)

        if type(value) in booltypes:
            wrapper.fs_add_field_metainfo_b(self.serializer, fieldname, fieldnamelength, key, keylength, ctypes.c_bool(value))
        elif type(value) in inttypes:
            wrapper.fs_add_field_metainfo_i(self.serializer, fieldname, fieldnamelength, key, keylength, ctypes.c_int32(value))
        elif type(value) in floattypes:
            wrapper.fs_add_field_metainfo_f(self.serializer, fieldname, fieldnamelength, key, keylength, ctypes.c_float(value))
        elif type(value) in doubletypes:
            wrapper.fs_add_field_metainfo_d(self.serializer, fieldname, fieldnamelength, key, keylength, ctypes.c_double(value))
        elif type(value) in stringtypes:
            value, valuelength = extract_string(value)
            wrapper.fs_add_field_metainfo_s(self.serializer, fieldname, fieldnamelength, key, keylength, value, valuelength)
        else:
            raise AttributeError("Error: type of value not supported")


class savepoint(object):
    def __init__(self, name, metainfo):
        # Extract string
        n, nlength = extract_string(name)
        self.savepoint = wrapper.fs_create_savepoint(n, nlength)

        for key, value in metainfo.items():
            self.add_metainfo(key, value)

    def __del__(self):
        wrapper.fs_destroy_savepoint(self.savepoint)

    def add_metainfo(self, key, value):
        key, keylength = extract_string(key)

        if type(value) in booltypes:
            wrapper.fs_add_savepoint_metainfo_b(self.savepoint, key, keylength, ctypes.c_bool(value))
        elif type(value) in inttypes:
            wrapper.fs_add_savepoint_metainfo_i(self.savepoint, key, keylength, ctypes.c_int32(value))
        elif type(value) in floattypes:
            wrapper.fs_add_savepoint_metainfo_f(self.savepoint, key, keylength, ctypes.c_float(value))
        elif type(value) in doubletypes:
            wrapper.fs_add_savepoint_metainfo_d(self.savepoint, key, keylength, ctypes.c_double(value))
        elif type(value) in stringtypes:
            value, valuelength = extract_string(value)
            wrapper.fs_add_savepoint_metainfo_s(self.savepoint, key, keylength, value, valuelength)
        else:
            raise AttributeError("Error: type of value not supported")

