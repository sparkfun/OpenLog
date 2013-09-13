#!/usr/bin/env python

import sys
import time
import struct

def usage():
	print "Usage: %s file" % (sys.argv[0])
	sys.exit(1)

def read_time(f):
	try:
		buf = f.read(4)
		if len(buf) == 0:
			sys.exit(1)
		return struct.unpack("<L", buf)[0]
	except struct.error:
		print "Error in timestamp"
		sys.exit(1)

def read_byte(f):
	try:
		buf = f.read(1)
		if len(buf) == 0:
			sys.exit(1)
		return buf
	except IOError:
		print "Read error"
		sys.exit(1)

def put_byte(b):
	sys.stdout.write( '%s' % b )
	sys.stdout.flush()

if len(sys.argv) < 2:
	usage()

f = open(sys.argv[1], "rb")
prev_time = read_time(f)
print "Recording starts at %i ms" % (prev_time)
put_byte(read_byte(f))
while True:
	t = read_time(f)
	time.sleep((t - prev_time) / 1000.0)
	prev_time = t
	put_byte(read_byte(f))

