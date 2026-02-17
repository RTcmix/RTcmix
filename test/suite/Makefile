include ../../makefile.conf

# This allows us to test before doing the install
#CMD = ${CMIXDIR}/insts.base/MIX/MIX
CMD = ${CMIXDIR}/bin/CMIX

CXXFLAGS += -DCMIXDIR=${CMIXDIR}
LDFLAGS = $(ARCH_BITFLAGS)

TESTS = \
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
run_sockettest

INSTBUS_TESTS = \
test_instbus_basic \
test_instbus_chain \
test_instbus_multi_writer \
test_instbus_deep_chain \
test_instbus_trans \
test_instbus_trans_down \
test_instbus_staggered_entry \
test_instbus_multi_writer_cascade \
test_instbus_asymmetric_tree \
test_instbus_diamond \
test_instbus_parallel_merge \
test_instbus_complex_quad \
test_instbus_dual_mono_separate \
test_instbus_dual_mono_to_stereo \
test_instbus_multilevel_merge \
test_instbus_stereo_merge \
test_instbus_quad \
test_instbus_trans_reuse

all:	announce all_tests

instbus_tests: announce $(INSTBUS_TESTS)
	@echo
	@echo "======================================="
	@echo "InstrumentBus test suite complete."
	@echo "======================================="

test: unit_tests

announce:
	@echo "***************************************************************"
	@echo Running RTcmix test suite
	@echo "***************************************************************"

# Extract peak amplitude from channel 0 and compare to expected value.
# Usage: $(call check_peak,<score>,<expected_peak>,<tolerance_pct>)
# Runs CMIX, extracts the first "channel 0:" peak value, compares within tolerance.
define check_peak
	@PEAK=$$($(CMD) -f $(1) 2>&1 | awk '/channel 0:/{print $$3; exit}'); \
	EXPECTED=$(2); \
	TOL=$(3); \
	if [ -z "$$PEAK" ]; then \
		echo "  FAIL: no peak amplitude found in output"; \
	else \
		LOWER=$$(echo "$$EXPECTED * (100 - $$TOL) / 100" | bc); \
		UPPER=$$(echo "$$EXPECTED * (100 + $$TOL) / 100" | bc); \
		IPEAK=$$(echo "$$PEAK" | awk '{printf "%d", $$1}'); \
		if [ $$IPEAK -ge $$LOWER ] && [ $$IPEAK -le $$UPPER ]; then \
			echo "  PASS: peak=$$IPEAK expected=$$EXPECTED (±$$TOL%)"; \
		else \
			echo "  FAIL: peak=$$IPEAK expected=$$EXPECTED (±$$TOL%), range=[$$LOWER,$$UPPER]"; \
		fi; \
	fi
endef

ifeq ($(OSC_SUPPORT), TRUE)
TESTS += run_osctests
endif
ifeq ($(EMBEDDED), TRUE)
TESTS += test_embedded
endif

all_tests: $(TESTS)

STRESSOBJS = stresstest.o
SOCKOBJS = sockettest.o
SOCKSENDOBJS = socksend.o
IMBCMIXOBJS += $(PROFILE_O)
PROGS = stresstest sockettest osc_send

TESTLEN = 30

stresstest: $(STRESSOBJS) $(IMBCMIXOBJS)
#	$(CXX) -o $@ $(DYN) $(STRESSOBJS) $(IMBCMIXOBJS) $(LDFLAGS) -L/home/dscott -ldmalloc
ifeq ($(ARCH), MACOSX)
	$(CXX) -o $@ $(DYN) $(STRESSOBJS) $(IMBCMIXOBJS) $(LDFLAGS) -L${CMIXDIR}/lib -Wl,-rpath,${CMIXDIR}/lib -lrtcmix
else
	$(CXX) -o $@ $(DYN) $(STRESSOBJS) $(IMBCMIXOBJS) $(LDFLAGS) -L${CMIXDIR}/lib -Wl,-rpath=${CMIXDIR}/lib -lrtcmix
endif

sockettest: $(SOCKOBJS)
	$(CXX) -o $@ $(SOCKOBJS) ${CMIXDIR}/lib/RTsockfuncs.o

socksend: $(SOCKSENDOBJS)
	$(CXX) -o $@ $(SOCKSENDOBJS) ${CMIXDIR}/lib/RTsockfuncs.o

embeddedtest: embeddedtest.o
	$(CXX) $(LDFLAGS) -g -o $@ embeddedtest.o -L${CMIXDIR}/lib -lrtcmix_embedded

osc_send_test.o:	osc_send_test.cpp
	$(CXX) ${LIBLO_CFLAGS} -g -c osc_send_test.cpp

osc_send: osc_send_test.o
	$(CXX) -g -o $@ osc_send_test.o ${LIBLO_LIBS}

run_osctests: osc_send
	@echo
	@echo Running OSC tests
	cd ./osc; make all;

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
	@find ${CMIXDIR}/shlib/ -name lib\*.so | grep -v conn | grep -v MAX | grep -v libPV\* | sed 's/\(.*\)/if (load("&") != 1) { exit("& LOAD FAILED"); }/' > load_test.sco
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

# ===== InstrumentBus Pull Model Tests =====
# Each test runs a score and checks peak amplitude on channel 0
# against expected value within tolerance (default 10%).

test_instbus_basic:
	@echo
	@echo "Testing instbus basic (WAVETABLE -> aux -> MIX -> out):"
	$(call check_peak,instbus_basic_test.sco,10000,10)

test_instbus_chain:
	@echo
	@echo "Testing instbus chain (WAVETABLE -> TRANS -> TRANS -> STEREO):"
	$(call check_peak,instbus_chain_test.sco,5000,10)

test_instbus_multi_writer:
	@echo
	@echo "Testing instbus multi-writer (2x WAVETABLE -> aux -> MIX -> out):"
	$(call check_peak,instbus_multi_writer_test.sco,10000,10)

test_instbus_deep_chain:
	@echo
	@echo "Testing instbus deep chain (5-stage MIX chain):"
	$(call check_peak,instbus_deep_chain_test.sco,11810,10)

test_instbus_trans:
	@echo
	@echo "Testing instbus TRANS up (WAVETABLE -> TRANS +1oct -> STEREO):"
	$(call check_peak,instbus_trans_test.sco,5000,10)

test_instbus_trans_down:
	@echo
	@echo "Testing instbus TRANS down (WAVETABLE -> TRANS -1oct -> STEREO):"
	$(call check_peak,instbus_trans_down_test.sco,5000,10)

test_instbus_staggered_entry:
	@echo
	@echo "Testing instbus staggered entry (3 writers at t=0,1,2):"
	$(call check_peak,instbus_staggered_entry_test.sco,12000,10)

test_instbus_multi_writer_cascade:
	@echo
	@echo "Testing instbus multi-writer cascade (2 writers/bus, 2 stages):"
	$(call check_peak,instbus_multi_writer_cascade_test.sco,12000,10)

test_instbus_asymmetric_tree:
	@echo
	@echo "Testing instbus asymmetric tree (3 branches depth 3/2/1):"
	$(call check_peak,instbus_asymmetric_tree_test.sco,12000,10)

test_instbus_diamond:
	@echo
	@echo "Testing instbus diamond (source splits to 2 TRANS, merges):"
	$(call check_peak,instbus_diamond_test.sco,2100,15)

test_instbus_parallel_merge:
	@echo
	@echo "Testing instbus parallel merge (2 TRANS chains -> stereo):"
	$(call check_peak,instbus_parallel_merge_test.sco,8000,10)

test_instbus_complex_quad:
	@echo
	@echo "Testing instbus complex quad (multi-stage to 4ch):"
	$(call check_peak,instbus_complex_quad_test.sco,6000,15)

test_instbus_dual_mono_separate:
	@echo
	@echo "Testing instbus dual mono separate (aux -> L, aux -> R):"
	$(call check_peak,instbus_dual_mono_separate_test.sco,8000,10)

test_instbus_dual_mono_to_stereo:
	@echo
	@echo "Testing instbus dual mono to stereo (2 aux -> stereo):"
	$(call check_peak,instbus_dual_mono_to_stereo_test.sco,8000,10)

test_instbus_multilevel_merge:
	@echo
	@echo "Testing instbus multilevel merge (3 sources, 3 levels):"
	$(call check_peak,instbus_multilevel_merge_test.sco,12000,10)

test_instbus_stereo_merge:
	@echo
	@echo "Testing instbus stereo merge (2 mono -> stereo):"
	$(call check_peak,instbus_stereo_merge_test.sco,8000,10)

test_instbus_quad:
	@echo
	@echo "Testing instbus quad (4 sources -> 4 channels):"
	$(call check_peak,instbus_quad_test.sco,6000,10)

test_instbus_trans_reuse:
	@echo
	@echo "Testing instbus TRANS reuse (long TRANS, staggered writers):"
	$(call check_peak,instbus_trans_reuse_test.sco,5000,10)

clean:
	@echo Cleaning test suite.
	$(RM) *.o $(PROGS)

install: osc_send
	$(INSTALL) osc_send ${CMIXDIR}/bin

