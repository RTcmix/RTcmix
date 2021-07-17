include ../makefile.conf

all:
	cp ../src/rtcmix/rt.h .
	cp ../src/rtcmix/rtdefs.h .
	cp ../src/rtcmix/rtcmix_types.h .
	cp ../src/rtcmix/ug_intro.h .
	cp ../src/rtcmix/spray.h .
	cp ../src/rtcmix/RefCounted.h .
	cp ../src/rtcmix/lock.h .
	cp ../src/rtcmix/Lockable.h .
	cp ../src/rtcmix/Locked.h .
	cp ../src/rtcmix/Instrument.h .
	cp ../src/rtcmix/MMPrint.h .
	cp ../src/rtcmix/PFBusData.h .
	cp ../src/rtcmix/PField.h .
	cp ../src/rtcmix/PFieldSet.h .
	cp ../src/rtcmix/Random.h .
	cp ../src/rtcmix/RawDataFile.h .
	cp ../src/rtcmix/Option.h .
	cp ../src/rtcmix/sockdefs.h .
	cp ../src/rtcmix/sfheader.h .
	cp ../src/rtcmix/notetags.h .
	cp ../src/rtcmix/RTcmix.h .
	cp ../src/rtcmix/mixerr.h .
	cp ../src/rtcmix/byte_routines.h .
ifneq ($(BUILDTYPE), STANDALONE)
	cp ../src/rtcmix/RTcmix_API.h .
endif

depend:

install: all

uninstall:

clean:
	@echo "Nothing to be done for 'clean'."

cleanall:
	$(RM) rt.h
	$(RM) rtcmix_types.h
	$(RM) ug_intro.h
	$(RM) rtdefs.h
	$(RM) spray.h
	$(RM) RefCounted.h
	$(RM) Instrument.h
	$(RM) lock.h
	$(RM) Lockable.h
	$(RM) Locked.h
	$(RM) MMprint.h
	$(RM) PFBusData.h
	$(RM) PField.h
	$(RM) PFieldSet.h
	$(RM) Random.h
	$(RM) Option.h
	$(RM) sockdefs.h
	$(RM) sfheader.h
	$(RM) notetags.h
	$(RM) RTcmix.h
	$(RM) mixerr.h
	$(RM) byte_routines.h
ifneq ($(BUILDTYPE), STANDALONE)
	$(RM) RTcmix_API.h
endif


