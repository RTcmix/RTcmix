import sys
vers = sys.version[:3]
print sys.exec_prefix + "/lib/python" + vers + "/config/libpython" + vers + ".a"
