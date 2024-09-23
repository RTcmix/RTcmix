include ../../makefile.conf

OBJECT = rtcmix~.mxo
TARGET = Deployment
OSX_VERSION=`xcrun --show-sdk-version --sdk macosx`
SDKROOT = macosx${OSX_VERSION}
MACOSX_DEPLOYMENT_TARGET = ${OSX_VERSION}
MAX_LDFLAGS = "-Xlinker -U -Xlinker _object_method_imp -Xlinker -U -Xlinker _object_new_imp"

ifeq ($(MAX_SDK_MAJOR_VERSION), 8)
	MAX_FLAGS = -DMAX_SDK_MAJOR_VERSION=6
endif
# We need to know we're doing a backwards-compatible build for SDK 8.2 and later
ifeq ($(MAX_SDK_MAJOR_VERSION), 8)
	ifeq ($(MAX_SDK_MINOR_VERSION), 2)
		MAX_FLAGS = "-DMAX_SDK_MAJOR_VERSION=8 -DC74_DEFINE_DEPRECATED_TYPES -DMAX_USE_NEW_HEADERS"
	endif
endif

all: check-version
	xcodebuild -project rtcmix~.xcodeproj -target rtcmix~ -configuration ${TARGET} -arch ${BUILD_ARCH} MAX_SDK_VERSION=${MAX_SDK_VERSION} MAX_SDK_PATH=${MAX_SDK_PATH} SDKROOT=${SDKROOT} MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET} OTHER_LDFLAGS=${MAX_LDFLAGS} OTHER_CFLAGS=${MAX_FLAGS}

debug-all: check-version
	xcodebuild -project rtcmix~.xcodeproj -target rtcmix~ -configuration Development -arch ${BUILD_ARCH} MAX_SDK_VERSION=${MAX_SDK_VERSION} MAX_SDK_PATH=${MAX_SDK_PATH} SDKROOT=${SDKROOT} MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET} OTHER_LDFLAGS=${MAX_LDFLAGS} OTHER_CFLAGS=${MAX_CFLAGS}

.PHONY: install clean cleanall

install:	all
	ditto build/${TARGET}/${OBJECT} ${MAX_INSTALL_DIR}/${OBJECT}

debug-install:	debug-all
	ditto build/Development/${OBJECT} ${MAX_INSTALL_DIR}/${OBJECT}
	
uninstall:

clean:
	rm -rf build

cleanall:	clean

check-version::
# Check for currently-unsupported Max versions
ifeq ($(MAX_MAJOR_VERSION), 8)
ifeq ($(MAX_MINOR_VERSION), 6)
	@echo MaxMSP version ${MAX_MAJOR_VERSION}.${MAX_MINOR_VERSION} and later are not yet supported by RTcmix
	@false;
endif
endif

