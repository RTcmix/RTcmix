import sysconfig

#from distutils.sysconfig import get_python_inc
#print(get_python_inc(plat_specific=1))

#print(sysconfig.get_path('platinclude'))	# doesn't return valid path on my machine

print(sysconfig.get_config_var("INCLUDEPY"))
