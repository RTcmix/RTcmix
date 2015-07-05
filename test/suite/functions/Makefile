POSITIVE_TESTS = default-args typereturn
NEGATIVE_TESTS = too-many-args function-symbol arg-symbol
COMPAT_TESTS = compat compat1 compat2 compat3
BLOCK_TESTS = block block01 block01a block02 block02a block03 block03a
FUNC_TESTS = func_call func_return funclevel1 func_call2 funcdecl recursivefun03 recursivefun04 recursivefun1 recursivefun2 func_auto_locals

CMIX = CMIX -P

all:	positive-tests compat-tests block-tests negative-tests func-tests 

# Tests which should succeed
positive-tests:
	@for TEST in $(POSITIVE_TESTS); \
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

func-tests:
	@for TEST in $(FUNC_TESTS); \
	do \
	  ( echo "running $$TEST..."; \
	  $(CMIX) -f $${TEST}.sco ); \
	done

# Tests which should fail

negative-tests:
	-@for TEST in $(NEGATIVE_TESTS); \
	do \
	  ( echo "running $$TEST..."; \
	  $(CMIX) -f $${TEST}.sco ); \
	done



