POSITIVE_TESTS = default-args typereturn
NEGATIVE_TESTS = too-many-args function-symbol arg-symbol
CMIX = CMIX

# Tests which should succeed
positive-tests:
	@for TEST in $(POSITIVE_TESTS); \
	do \
	  ( echo "running $$TEST..."; \
	  $(CMIX) -f $${TEST}.sco ); \
	done

# Tests which should fail

negative-tests:
	@for TEST in $(NEGATIVE_TESTS); \
	do \
	  ( echo "running $$TEST..."; \
	  $(CMIX) -f $${TEST}.sco ); \
	done

all:	positive-tests negative-tests


