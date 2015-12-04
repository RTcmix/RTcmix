include ../../makefile.conf

OBJECT = rtcmix~.mxo
TARGET = Deployment
OSX_VERSION=`xcrun --show-sdk-version`
SDKROOT = macosx${OSX_VERSION}
MACOSX_DEPLOYMENT_TARGET = ${OSX_VERSION}

all:
	xcodebuild -project rtcmix~.xcodeproj -target rtcmix~ -configuration ${TARGET} -arch i386 MAX_SDK_PATH=${MAX_SDK_PATH} SDKROOT=${SDKROOT} MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET}

debug-all:
	xcodebuild -project rtcmix~.xcodeproj -target rtcmix~ -configuration Development -arch i386 MAX_SDK_PATH=${MAX_SDK_PATH} SDKROOT=${SDKROOT} MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET}

.PHONY: install clean cleanall

install:	all
	ditto build/${TARGET}/${OBJECT} ${MAX_INSTALL_DIR}/${OBJECT}

debug-install:	debug-all
	ditto build/Development/${OBJECT} ${MAX_INSTALL_DIR}/${OBJECT}
	
uninstall:

clean:
	xcodebuild clean

cleanall:	clean

