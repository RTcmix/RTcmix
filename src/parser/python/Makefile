include ../../../makefile.conf

CXXFLAGS += -I../../../src/rtcmix

all: rtcmix

rtcmix: rtcmixmodule.cpp
	python setup.py build
	cp build/lib.linux-i686-2.2/rtcmix.so .
#FIXME: this is platform-specific -- use distutils for install

clean:
	$(RM) -r build *.so

