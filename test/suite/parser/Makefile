include ../../../makefile.conf

CMIX = ${CMIXDIR}/bin/CMIX

PASS_SCORES = \
bison-verification.sco \
bison-verification-2.sco \
base_init.sco \
base_no_initializer01.sco \
base_no_initializer02.sco \
basetest01.sco \
basetest03.sco \
base_method_override.sco \
bool_checks.sco \
builtin_function.sco \
check_arg_order.sco \
double_include.sco \
double_recursion.sco \
for_while_score.sco \
functest1.sco \
functest2.sco \
functest3.sco \
functest4.sco \
functest5.sco \
function_calls_method.sco \
functionlocalvar.sco \
function_types.sco \
include_at_line7.sco \
if_else_scope.sco \
list_as_object.sco \
list_compare.sco \
list_increment_element.sco \
list_operate_element.sco \
list_read_write.sco \
maptest1.sco \
maptest2.sco \
maptest3.sco \
maptest4.sco \
maptest5.sco \
maptest6.sco \
method_as_method_arg.sco \
method_calls_function.sco \
mfunction01.sco \
multistruct.sco \
negative_list_element.sco \
nested.sco \
nested_if_in_for.sco \
nested_func_in_struct.sco \
nested_functions.sco \
nestednoloop.sco \
new_features.sco \
newfunction01.sco \
newfunction02.sco \
newfunction03.sco \
null_list.sco \
object_as_listelem.sco \
object_methods.sco \
object_test01.sco \
object_test03.sco \
one_method_one_function.sco \
operator_test01.sco \
recursive_method.sco \
return_list.sco \
return_list2.sco \
return_list3.sco \
sanity_check.sco \
simple_recursion.sco \
simplefunction.sco \
struct01.sco \
struct02.sco \
struct_as_func_arg.sco \
struct_as_func_return.sco \
struct_as_list_element.sco \
struct_as_map_element.sco \
struct_as_member.sco \
struct_copy.sco \
struct_increment_element.sco \
struct_increment_element_via_method.sco \
struct_initializer.sco \
struct_var_scope.sco \
struct_with_same_as_member.sco \
struct01.sco \
struct02.sco \
ternary.sco \
test_double_include.sco \
test_rhs.sco \
two_methods.sco \
two_objects_call_method.sco \
unaryop.sco \
wrappertest.sco

FAIL_SCORES = \
base_unknown.sco \
block_scope.sco \
derived_duplicate_member.sco \
double_method_decl.sco \
error.sco \
error2.sco \
for_scope.sco \
function_scope.sco \
func_errortests1.sco \
func_with_undef_struct_arg.sco \
handletest1.sco \
if_inside_fun_scope.sco \
include_error.sco \
list_large_index.sco \
nested_func_decls.sco \
null_struct.sco \
object_test02.sco \
recursive_list.sco \
struct_dupe_member_method.sco \
struct_duplicate_member.sco \
struct_duplicate_membername.sco \
struct_init_wrong_type.sco \
struct_init_insufficient_args.sco \
type_conflict1.sco \
uninit_fun_member.sco \
uninit_mfunction.sco \
var_overwrite.sco \
while_scope.sco

all:	scores_that_pass scores_that_fail

test: all

scores_that_pass:
	@echo Running positive tests...
	@for SCORE in $(PASS_SCORES); do \
	  echo "running $$SCORE..."; \
	  $(CMIX) -f $$SCORE; \
	  if [ $$? -eq 0 ]; then \
	    echo "SCORE PASSED"; \
	  else \
	    echo "SCORE '$$SCORE' FAILED"; exit 1; \
	  fi; \
	done

scores_that_fail:
	@echo Running negative tests...
	@for SCORE in $(FAIL_SCORES); do \
          echo "running $$SCORE..."; \
          $(CMIX) -f $$SCORE; \
          if [ $$? -eq 0 ]; then \
          	echo "SCORE '$$SCORE' FAILED (BY NOT FAILING)"; exit 1; \
          else \
          	echo "SCORE PASSED (BY CORRECTLY FAILING)"; \
          fi; \
	done


