#!/usr/bin/python

from serialization import serializer, savepoint
import numpy as np
from hgext.mq import series

if __name__ == '__main__':
    print "test1"
    
    savepoint = savepoint("diffuse_hori_velocity", dict());
    serializer = serializer("/informatik/home/hovy/scratch/svn/icon-test-lesturb/experiments/nh_cbl", "serializer_diffuse_hori_velocity_0", "r");
    sizes = serializer.get_field_size("ddt_u");
#     sizes = (32, 64, 1156, 1)
    a = np.empty((sizes[0] * sizes[1] * sizes[2] * sizes[3]), dtype=np.double);
    serializer.load_field("ddt_u", a, savepoint);
