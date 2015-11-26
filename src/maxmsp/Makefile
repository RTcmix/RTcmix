include ../../makefile.conf

OBJECT = rtcmix~.mxo

all:
	xcodebuild -project rtcmix~.xcodeproj -target rtcmix~ -configuration Default -arch ${BUILD_ARCH} MAX_SDK_PATH=${MAX_SDK_PATH}

.PHONY: install clean cleanall

install:	all
	ditto build/Default/${OBJECT} ${MAX_INSTALL_DIR}/${OBJECT}
	
uninstall:

clean:
	xcodebuild clean

cleanall:	clean

