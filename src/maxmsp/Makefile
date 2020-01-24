include ../../makefile.conf

OBJECT = rtcmix~.mxo
TARGET = Deployment
OSX_VERSION=`xcrun --show-sdk-version --sdk macosx`
SDKROOT = macosx${OSX_VERSION}
MACOSX_DEPLOYMENT_TARGET = ${OSX_VERSION}
MAX_LDFLAGS = "-Xlinker -U -Xlinker _object_method_imp -Xlinker -U -Xlinker _object_new_imp"

all:
	xcodebuild -project rtcmix~.xcodeproj -target rtcmix~ -configuration ${TARGET} -arch ${BUILD_ARCH} MAX_SDK_VERSION=${MAX_SDK_VERSION} MAX_SDK_PATH=${MAX_SDK_PATH} SDKROOT=${SDKROOT} MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET} OTHER_LDFLAGS=${MAX_LDFLAGS}

debug-all:
	xcodebuild -project rtcmix~.xcodeproj -target rtcmix~ -configuration Development -arch ${BUILD_ARCH} MAX_SDK_VERSION=${MAX_SDK_VERSION} MAX_SDK_PATH=${MAX_SDK_PATH} SDKROOT=${SDKROOT} MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET} OTHER_LDFLAGS=${MAX_LDFLAGS}

.PHONY: install clean cleanall

install:	all
	ditto build/${TARGET}/${OBJECT} ${MAX_INSTALL_DIR}/${OBJECT}

debug-install:	debug-all
	ditto build/Development/${OBJECT} ${MAX_INSTALL_DIR}/${OBJECT}
	
uninstall:

clean:
	xcodebuild clean

cleanall:	clean

