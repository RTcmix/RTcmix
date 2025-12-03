# OSC Test Suite

COMMAND = ../osc_send -c
SCORE = ../osc_send -s

.PHONY: test_quit test_restart

all:	command_tests

define RUN_SERVER
	@echo "STARTING SERVER"
	@CMIX -o &
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

command_tests: test_restart test_var_across_scores test_var_across_audio_stop

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

test_bad_command:
	@echo "Sending a malformed command"
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

.EXIT:
	@echo "killing stray CMIX processes"
	@killall CMIX
