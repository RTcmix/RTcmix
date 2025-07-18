include ../../makefile.conf

# This allows us to test before doing the install
#CMD = ${CMIXDIR}/insts.base/MIX/MIX
CMD = ${CMIXDIR}/bin/CMIX

CXXFLAGS += -DCMIXDIR=${CMIXDIR}
LDFLAGS = $(ARCH_BITFLAGS)

all:	announce all_tests

test: unit_tests

announce:
	@echo "***************************************************************"
	@echo Running RTcmix test suite
	@echo "***************************************************************"

all_tests:	\
test_unreadable_input \
test_bogus_input \
test_null_input \
test_nonexistant_input \
test_unwriteable_output \
test_null_output \
test_bad_chans \
test_8_channel \
test_n_channel_blip \
test_aux_bus \
test_bus_loop \
test_simple_mix \
test_scheduler \
test_stereo \
test_load \
test_minc \
run_stresstest \
run_sockettest \
$(NULL)

STRESSOBJS = stresstest.o
SOCKOBJS = sockettest.o
SOCKSENDOBJS = socksend.o
IMBCMIXOBJS += $(PROFILE_O)
PROGS = stresstest sockettest osc_send
TESTLEN = 30

stresstest: $(STRESSOBJS) $(IMBCMIXOBJS)
#	$(CXX) -o $@ $(DYN) $(STRESSOBJS) $(IMBCMIXOBJS) $(LDFLAGS) -L/home/dscott -ldmalloc
	$(CXX) -o $@ $(DYN) $(STRESSOBJS) $(IMBCMIXOBJS) $(LDFLAGS) -L${CMIXDIR}/lib -lrtcmix

sockettest: $(SOCKOBJS)
	$(CXX) -o $@ $(SOCKOBJS) ${CMIXDIR}/lib/RTsockfuncs.o

socksend: $(SOCKSENDOBJS)
	$(CXX) -o $@ $(SOCKSENDOBJS) ${CMIXDIR}/lib/RTsockfuncs.o

embeddedtest: embeddedtest.o
	$(CXX) $(LDFLAGS) -g -o $@ embeddedtest.o -L${CMIXDIR}/lib -lrtcmix_embedded

osc_send_test.o:	osc_send_test.cpp
	$(CXX) ${LIBLO_CFLAGS} -c osc_send_test.cpp

osc_send: osc_send_test.o
	$(CXX) -g -o $@ osc_send_test.o ${LIBLO_LIBS}


run_stresstest:	stresstest
	@echo
	@echo Running interactive stress test for $(TESTLEN) seconds...
	./stresstest -d $(TESTLEN)

run_sockettest:	sockettest
	@echo
	@echo Running socket stress test for $(TESTLEN) seconds...
	./sockettest -d $(TESTLEN)
	
test_unreadable_input:
	@echo
	@echo Testing unreadable input file:
	@rm -rf unreadable.snd
	@sfcreate -r 44100 -c 1 -t sun -i unreadable.snd
	@chmod a-r unreadable.snd
	-$(CMD) < test-unreadable-input.sco
	@rm -rf unreadable.snd

test_unwriteable_output:
	@echo
	@echo Testing unwriteable output file:
	@rm -rf unwriteable.snd
	@sfcreate -r 44100 -c 1 -t sun -i unwriteable.snd
	@chmod a-w unwriteable.snd
	-$(CMD) < test-unwriteable-output.sco
	@rm -rf unwriteable.snd

test_bad_chans:
	@echo
	@echo Testing bad channel count:
	-$(CMD) < test-bad-chans.sco

test_8_channel:
	@echo
	@echo Testing 8-channel output:
	-$(CMD) < test-8-channel.sco

test_n_channel_blip:
	@echo
	@echo Testing 2-channel output:
	-$(CMD) 2 < test-n-channel-blip.sco
	@echo Testing 8-channel output:
	-$(CMD) 8 < test-n-channel-blip.sco

test_nonexistant_input:
	@echo
	@echo Testing nonexistant input soundfile:
	-$(CMD) < test-nonexistant-input.sco

test_bogus_input:
	@echo
	@echo Testing bogus input soundfile:
	-$(CMD) < test-bogus-input.sco

test_null_input:
	@echo
	@echo Testing NULL input filename:
	-$(CMD) < test-null-input.sco

test_null_output:
	@echo
	@echo Testing NULL output filename:
	-$(CMD) < test-null-output.sco

test_simple_mix:
	@echo
	@echo Testing simple MIX command:
	-$(CMD) < simple-mix.sco
	
test_aux_bus:
	@echo
	@echo Testing aux busses:
	-$(CMD) < test-aux-bus-01.sco

test_bus_loop:
	@echo
	@echo Testing looped bus config:
	-$(CMD) < test-bus-loop.sco

test_scheduler:
	@echo
	@echo Testing non-interactive scheduler:
	-$(CMD) < test-scheduler.sco

test_stereo:
	@echo
	@echo Testing stereo placement \(left-center-right\):
	-$(CMD) < test-stereo.sco

test_load:
	@echo
	@echo Testing load of all shlib dsos
	@find ${CMIXDIR}/shlib/ -name lib\* | grep -v conn | sed 's/\(.*\)/if (load("&") != 1) { print("& LOAD FAILED"); }/' > load_test.sco
	-$(CMD) < load_test.sco
#	rm load_test.sco

test_minc:
	@echo
	@echo Testing MinC functionality
	-$(CMD) < test-minc-nestedlists.sco		
	-$(CMD) < test-minc-nestedpfieldlists.sco		
	-$(CMD) < test-minc-pfieldchain.sco		
	-$(CMD) < test-minc-samplepfield.sco

unit_announce:
	@echo "***************************************************************"
	@echo Running RTcmix Unit Tests
	@echo "***************************************************************"

unit_tests: unit_announce test_minc
	@echo Running parser tests
	@cd parser; $(MAKE);
	@cd functions; $(MAKE);
	@echo RTcmix Unit Tests Done

test_embedded:
	@echo
	@echo Testing embedded use of all non-instrument functions
	@mkdir -p function_scores
	@for line in `cat every_function.sco`; do echo $$line\(\) > function_scores/$$line.sco; done
	-./embeddedtest function_scores/*.sco
	@echo Testing embedded use of all instruments
	@echo "rtinput(\"input.wav\")" > embedded_test.sco
	@find ${CMIXDIR}/shlib/ -name lib\* | grep -v conn | sed 's/^.*lib//' | sed 's/\.so/(0,0,1,1,1,1,1,1,1,1)/' >> embedded_test.sco
	-./embeddedtest embedded_test.sco
	@rm -rf function_scores

clean:
	@echo Cleaning test suite.
	$(RM) *.o $(PROGS)

install:
	@echo Nothing to install here.

