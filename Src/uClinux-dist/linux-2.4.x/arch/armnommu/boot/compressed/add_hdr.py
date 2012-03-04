#!/usr/bin/python

import sys
import os
import struct 
fout = open(sys.argv[2], "wb")
fout.write(struct.pack("<I", os.path.getsize(sys.argv[1])))

fin = open(sys.argv[1], "rb")
fout.write(fin.read())
fout.close()
fin.close()

