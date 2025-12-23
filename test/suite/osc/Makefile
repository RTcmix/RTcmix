# OSC Test Suite

COMMAND = ../osc_send -c
SCORE = ../osc_send -s
CMIX_ARGS =

# Common log file for the server
CMIX_LOG = cmix.log

.PHONY: test_quit test_restart

all:	command_tests cleanup

define RUN_SERVER
	@echo "STARTING SERVER"
	@rm -f $(CMIX_LOG)
	@CMIX -o $(CMIX_ARGS) 2>&1 | tee $(CMIX_LOG) &
	@sleep 2
endef

define STOP_SERVER
	@echo "STOPPING SERVER"
	@echo "quit" | $(COMMAND)
	@sleep 1
endef

define START_AUDIO
	@echo "STARTING AUDIO"
	@echo "rtsetparams(48000, 2)" | $(COMMAND)
	@sleep 1
endef

define STOP_AUDIO
	@echo "STOPPING AUDIO"
	@echo "stop" | $(COMMAND)
	@sleep 1
endef

command_tests: test_restart test_var_across_scores test_var_across_audio_stop \
test_instrument_command test_bad_command test_empty_command test_mixed_arg_type_command

test_quit:
	$(RUN_SERVER)
	$(START_AUDIO)
	$(STOP_SERVER)

test_restart:
	$(RUN_SERVER)
	$(START_AUDIO)
	$(STOP_AUDIO)
	$(START_AUDIO)
	$(STOP_SERVER)
	
test_var_across_scores:
	$(RUN_SERVER)
	$(START_AUDIO)
	@cat define_var.sco | $(SCORE)
	@cat use_var.sco | $(SCORE)
	$(STOP_SERVER)

test_var_across_audio_stop:
	$(RUN_SERVER)
	$(START_AUDIO)
	@cat define_var.sco | $(SCORE)
	$(STOP_AUDIO)
	$(START_AUDIO)
	@cat use_var.sco | $(SCORE)
	$(STOP_SERVER)

test_instrument_command:
	@echo "Sending a command with load and instrument call"
	$(RUN_SERVER)
	$(START_AUDIO)
	@echo 'load("WAVETABLE");' | $(COMMAND)
	@echo 'WAVETABLE(0, 5, 10000, 440);' | $(COMMAND)
	@sleep 6
	$(STOP_SERVER)

test_bad_command:
	@echo "Sending a malformed command (will fail)"
	$(RUN_SERVER)
	$(START_AUDIO)
	@echo "NOTACOMMAND" | $(COMMAND)
	$(STOP_SERVER)

test_empty_command:
	@echo "Sending an empty command"
	$(RUN_SERVER)
	$(START_AUDIO)
	@echo "" | $(COMMAND)
	$(STOP_SERVER)

test_mixed_arg_type_command:
	@echo "Sending a command with mixed arg types (will fail)"
	$(RUN_SERVER)
	$(START_AUDIO)
	@echo 'maketable("line", 1000, 0, 1, 1, 0);' | $(COMMAND)
	$(STOP_SERVER)

cleanup:
	@rm -f $(CMIX_LOG)
	@echo "killing stray CMIX processes"
	@-killall -q CMIX

.EXIT: .END

.END: cleanup
