CMIX = CMIX

PASS_SCORES = \
bool_checks.sco \
builtin_function.sco \
check_arg_order.sco \
double_include.sco \
functest1.sco \
functest2.sco \
functest3.sco \
functest4.sco \
functest5.sco \
func_with_undef_struct_arg.sco \
if_else_scope.sco \
include_at_line7.sco \
list_compare.sco \
list_increment_element.sco \
list_read_write.sco \
maptest2.sco \
maptest3.sco \
maptest4.sco \
maptest5.sco \
maptest6.sco \
function_calls_method.sco \
method_calls_function.sco \
two_methods.sco \
multistruct.sco \
negative_list_element.sco \
nested.sco \
nested_functions.sco \
nestednoloop.sco \
new_features.sco \
newfunction01.sco \
newfunction02.sco \
newfunction03.sco \
sanity_check.sco \
struct01.sco \
struct02.sco \
struct_as_func_arg.sco \
struct_as_func_return.sco \
struct_as_member.sco \
struct_copy.sco \
struct_increment_element.sco \
struct_initializer.sco \
struct_with_same_as_member.sco \
test_double_include.sco \
unaryop.sco

FAIL_SCORES = \
func_errortests1.sco \
handletest1.sco \
include_error.sco \
list_large_index.sco \
nested_func_decls.sco \
struct_duplicate_member.sco \
struct_duplicate_membername.sco \
struct_init_wrong_type.sco \
struct_init_insufficient_args.sco \
struct_var_scope.sco \
uninit_fun_member.sco


scores_that_pass:
	@for SCORE in $(PASS_SCORES); \
        do \
          ( echo "running $$SCORE..."; \
          $(CMIX) -f $$SCORE; \
		  if [ $$? -eq 0 ]; then echo "SCORE PASSED"; else echo "SCORE '$$SCORE' FAILED" && exit -1; fi ); \
        done

scores_that_fail:
	@for SCORE in $(FAIL_SCORES); \
        do \
          ( echo "running $$SCORE..."; \
          $(CMIX) -f $$SCORE; \
		  if [ $$? -eq 0 ]; then echo "SCORE '$$SCORE' FAILED (BY NOT FAILING)" && exit -1; else echo "SCORE PASSED (BY CORRECTLY FAILING)"; fi ); \
        done

all:	scores_that_pass scores_that_fail


