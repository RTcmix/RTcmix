include ../makefile.conf

all:
	ln -sf ../src/rtcmix/rt.h .
	ln -sf ../src/rtcmix/rtdefs.h .
	ln -sf ../src/rtcmix/rtcmix_types.h .
	ln -sf ../src/rtcmix/ug_intro.h .
	ln -sf ../src/rtcmix/spray.h .
	ln -sf ../src/rtcmix/RefCounted.h .
	ln -sf ../src/rtcmix/lock.h .
	ln -sf ../src/rtcmix/Lockable.h .
	ln -sf ../src/rtcmix/Locked.h .
	ln -sf ../src/rtcmix/Instrument.h .
	ln -sf ../src/rtcmix/MMPrint.h .
	ln -sf ../src/rtcmix/PFBusData.h .
	ln -sf ../src/rtcmix/PField.h .
	ln -sf ../src/rtcmix/PFieldSet.h .
	ln -sf ../src/rtcmix/Random.h .
	ln -sf ../src/rtcmix/RawDataFile.h .
	ln -sf ../src/rtcmix/RTsockfuncs.h .
	ln -sf ../src/rtcmix/RTOption.h .
	ln -sf ../src/rtcmix/sockdefs.h .
	ln -sf ../src/rtcmix/sfheader.h .
	ln -sf ../src/rtcmix/RTcmix.h .
	ln -sf ../src/rtcmix/mixerr.h .
	ln -sf ../src/rtcmix/byte_routines.h .
ifneq ($(BUILDTYPE), STANDALONE)
	ln -sf ../src/rtcmix/RTcmix_API.h .
endif

depend: all

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
	$(RM) RawDataFile.h
	$(RM) RTsockfuncs.h
	$(RM) RTOption.h
	$(RM) sockdefs.h
	$(RM) sfheader.h
	$(RM) notetags.h
	$(RM) RTcmix.h
	$(RM) mixerr.h
	$(RM) byte_routines.h
ifneq ($(BUILDTYPE), STANDALONE)
	$(RM) RTcmix_API.h
endif


