# This allows us to test before doing the install
CMD = ../../insts.base/MIX/MIX

all:	announce all_tests

test: all

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
test_simple_mix \
$(NULL)

test_unreadable_input:
	@echo
	@echo Testing unreadable input file:
	@rm -rf unreadable.snd
	@i1 unreadable.snd
	@chmod a-r unreadable.snd
	-$(CMD) < test-unreadable-input.sco
	@rm -rf unreadable.snd

test_unwriteable_output:
	@echo
	@echo Testing unwriteable output file:
	@rm -rf unwriteable.snd
	@i1 unwriteable.snd
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

clean:
	@echo Nothing to clean here.

install:
	@echo Nothing to install here.

