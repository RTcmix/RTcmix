POSITIVE_TESTS = bexp default-args typereturn handle if-bstml ifrand iftrue exit exit2 \
	manysymbols toofewargs undefined_func var-redeclare struct_ctor_wrong_arg_count 
NEGATIVE_TESTS = too-many-args function-symbol arg-symbol lineno \
struct_ctor_wrong_arg_type two-bad-functions
COMPAT_TESTS = compat compat1 compat2 compat3
BLOCK_TESTS = block block01 block01a block02 block02a block02c block03 block03a block03c block_list
FUNC_TESTS = argtest call-stack func_call func_return funclevel1 func_call2 funcdecl \
	recursivefun03 recursivefun04 recursivefun1 recursivefun2 func_auto_locals \
	nested_builtin_functions func_local_scopes scope01 scopetest struct_ctor1 \
	two_returns two_returns_b

# These fail but shouldn't, but we leave them out for clean regression:
# scope02

CMIX = CMIX -P

all:	positive-tests compat-tests block-tests negative-tests func-tests

test: all

# Tests which should succeed
positive-tests:
	@for TEST in $(POSITIVE_TESTS); do \
	  echo "running $$TEST..."; \
	  $(CMIX) -f $${TEST}.sco; \
	  if [ $$? -eq 0 ]; then \
	    echo "SCORE PASSED"; \
	  else \
	    echo "SCORE '$${TEST}.sco' FAILED"; exit 1; \
	  fi; \
	done

compat-tests:
	@for TEST in $(COMPAT_TESTS); do \
	  echo "running $$TEST..."; \
	  $(CMIX) -f $${TEST}.sco; \
	  if [ $$? -eq 0 ]; then \
	    echo "SCORE PASSED"; \
	  else \
	    echo "SCORE '$${TEST}.sco' FAILED"; exit 1; \
	  fi; \
	done

block-tests:
	@for TEST in $(BLOCK_TESTS); do \
	  echo "running $$TEST..."; \
	  $(CMIX) -f $${TEST}.sco; \
	  if [ $$? -eq 0 ]; then \
	    echo "SCORE PASSED"; \
	  else \
	    echo "SCORE '$${TEST}.sco' FAILED"; exit 1; \
	  fi; \
	done

func-tests:
	@for TEST in $(FUNC_TESTS); do \
	  echo "running $$TEST..."; \
	  $(CMIX) -f $${TEST}.sco; \
	  if [ $$? -eq 0 ]; then \
	    echo "SCORE PASSED"; \
	  else \
	    echo "SCORE '$${TEST}.sco' FAILED"; exit 1; \
	  fi; \
	done

# Tests which should fail

negative-tests:
	-@for TEST in $(NEGATIVE_TESTS); do \
	  echo "running $$TEST..."; \
	  $(CMIX) -f $${TEST}.sco; \
          if [ $$? -eq 0 ]; then \
          	echo "SCORE '$${TEST}.sco' FAILED (BY NOT FAILING)"; exit 1; \
          else \
          	echo "SCORE PASSED (BY CORRECTLY FAILING)"; \
          fi; \
	done

