POSITIVE_TESTS = default-args typereturn
NEGATIVE_TESTS = too-many-args function-symbol arg-symbol
COMPAT_TESTS = compat compat1 compat2 compat3
BLOCK_TESTS = block block01 block01a block02 block02a block03 block03a
CMIX = CMIX -P

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

compat-tests:
	@for TEST in $(COMPAT_TESTS); \
	do \
	  ( echo "running $$TEST..."; \
	  $(CMIX) -f $${TEST}.sco ); \
	done

block-tests:
	@for TEST in $(BLOCK_TESTS); \
	do \
	  ( echo "running $$TEST..."; \
	  $(CMIX) -f $${TEST}.sco ); \
	done

all:	positive-tests compat-tests block-tests negative-tests


