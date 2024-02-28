#!/bin/sh

if [ $# -eq 0 ]
then
	echo Usage: `basename $0` name-of-Python-executable
	exit
fi
PYTHON="$1"

prefix=`${PYTHON} -c "import sysconfig; print(sysconfig.get_config_var('LIBPL'))"`
pythonlib=`${PYTHON} -c "import sysconfig; print(sysconfig.get_config_var('LIBRARY'))"`
goodpath=${prefix}/${pythonlib}

badpath=`/usr/bin/otool -L PYMIX | awk '/Python/ { print $1 }'`

cmd="/usr/bin/install_name_tool -change ${badpath} ${goodpath} PYMIX"
echo ${cmd}
$cmd
