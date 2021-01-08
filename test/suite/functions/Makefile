POSITIVE_TESTS = bexp default-args typereturn handle if-bstml ifrand iftrue 
NEGATIVE_TESTS = too-many-args function-symbol arg-symbol lineno
COMPAT_TESTS = compat compat1 compat2 compat3
BLOCK_TESTS = block block01 block01a block02 block02a block02c block03 block03a block03c
FUNC_TESTS = argtest call-stack func_call func_return funclevel1 func_call2 funcdecl \
	recursivefun03 recursivefun04 recursivefun1 recursivefun2 func_auto_locals \
	func_local_scopes scope01 scope02 scopetest
	

CMIX = CMIX -P

all:	positive-tests compat-tests block-tests negative-tests func-tests 

# Tests which should succeed
positive-tests:
	@for TEST in $(POSITIVE_TESTS); \
	do \
	  ( echo "running $$TEST..."; \
	  $(CMIX) -f $${TEST}.sco; \
	  if [ $$? -eq 0 ]; then echo "SCORE PASSED"; else echo "SCORE FAILED"; fi ); \
	done

compat-tests:
	@for TEST in $(COMPAT_TESTS); \
	do \
	  ( echo "running $$TEST..."; \
	  $(CMIX) -f $${TEST}.sco; \
	  if [ $$? -eq 0 ]; then echo "SCORE PASSED"; else echo "SCORE FAILED"; fi ); \
	done

block-tests:
	@for TEST in $(BLOCK_TESTS); \
	do \
	  ( echo "running $$TEST..."; \
	  $(CMIX) -f $${TEST}.sco; \
	  if [ $$? -eq 0 ]; then echo "SCORE PASSED"; else echo "SCORE FAILED"; fi ); \
	done

func-tests:
	@for TEST in $(FUNC_TESTS); \
	do \
	  ( echo "running $$TEST..."; \
	  $(CMIX) -f $${TEST}.sco; \
	  if [ $$? -eq 0 ]; then echo "SCORE PASSED"; else echo "SCORE FAILED"; fi ); \
	done

# Tests which should fail

negative-tests:
	-@for TEST in $(NEGATIVE_TESTS); \
	do \
	  ( echo "running $$TEST..."; \
	  $(CMIX) -f $${TEST}.sco; \
	  if [ $$? -eq 0 ]; then echo "SCORE FAILED (BY NOT FAILING)"; else echo "SCORE PASSED (BY CORRECTLY FAILING)"; fi ); \
	done



