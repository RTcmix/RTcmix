include ../../makefile.conf

OBJECT = rtcmix~.mxo
TARGET = Deployment
OSX_VERSION=`xcrun --show-sdk-version --sdk macosx`
SDKROOT = macosx${OSX_VERSION}
MACOSX_DEPLOYMENT_TARGET = ${OSX_VERSION}
MAX_LDFLAGS = "-Xlinker -U -Xlinker _object_method_imp -Xlinker -U -Xlinker _object_new_imp"
LOCAL_FLAGS = -Wno-shorten-64-to-32
PROJECT = rtcmix~.xcodeproj

# This is the version of the Xcode project for the C++ version of the source.
ifeq ($(MAX_MAJOR_VERSION), 8)
	ifeq ($(MAX_MINOR_VERSION), 6)
		PROJECT = min-rtcmix~.xcodeproj
	endif
endif

ifeq ($(MAX_SDK_MAJOR_VERSION), 6)
	MAX_FLAGS = "$(LOCAL_FLAGS) -DMAX_SDK_MAJOR_VERSION=6"
endif
ifeq ($(MAX_SDK_MAJOR_VERSION), 7)
	MAX_FLAGS = "$(LOCAL_FLAGS) -DMAX_SDK_MAJOR_VERSION=7"
endif

# We need to know we're doing a backwards-compatible build for SDK 8.2 and later
ifeq ($(MAX_SDK_MAJOR_VERSION), 8)
	ifeq ($(MAX_SDK_MINOR_VERSION), 2)
		MAX_FLAGS = "$(LOCAL_FLAGS) -DMAX_SDK_MAJOR_VERSION=8 -DC74_DEFINE_DEPRECATED_TYPES -DMAX_USE_NEW_HEADERS"
	endif
endif

all: check-version
	xcodebuild -project ${PROJECT} -target rtcmix~ -configuration ${TARGET} -arch ${BUILD_ARCH} MIN_SDK_PATH=${MIN_SDK_PATH} MAX_SDK_PATH=${MAX_SDK_PATH} SDKROOT=${SDKROOT} MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET} OTHER_LDFLAGS=${MAX_LDFLAGS} OTHER_CFLAGS=${MAX_FLAGS}

debug-all: check-version
	xcodebuild -project ${PROJECT} -target rtcmix~ -configuration Development -arch ${BUILD_ARCH} MIN_SDK_PATH=${MIN_SDK_PATH} MAX_SDK_PATH=${MAX_SDK_PATH} SDKROOT=${SDKROOT} MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET} OTHER_LDFLAGS=${MAX_LDFLAGS} OTHER_CFLAGS=${MAX_CFLAGS}

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
# NOTE: THIS NEEDS TO BE DISABLED TO ALLOW AN 8.6 OR LATER VERSION TO COMPILE - DAS
ifeq ($(MAX_MAJOR_VERSION), 8)
ifeq ($(MAX_MINOR_VERSION), 6)
	@echo MaxMSP version ${MAX_MAJOR_VERSION}.${MAX_MINOR_VERSION} and later are not yet supported by RTcmix
	@false;
endif
endif

