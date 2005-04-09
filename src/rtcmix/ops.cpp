/* RTcmix  - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
// Functions to use when operator overloading is not available in the
// scripting language.  Currently, we can't do op overloading in Perl,
// so we use mul, add, div and sub functions as substitutes.   -JGG

#include <rtcmix_types.h>
#include <PField.h>
#include <utils.h>
#include <ugens.h>		// for warn, die

extern "C" {
	Handle mul(const Arg args[], const int nargs);
	Handle add(const Arg args[], const int nargs);
	Handle div(const Arg args[], const int nargs);
	Handle sub(const Arg args[], const int nargs);
};


// --------------------------------------------------------------------- mul ---
Handle mul(const Arg args[], const int nargs)
{
	if (nargs == 2) {
		PField *pf0 = (PField *) args[0];
		PField *pf1 = (PField *) args[1];
		if (!pf0 && args[0].isType(DoubleType))
			pf0 = new ConstPField((double) args[0]);
		if (!pf1 && args[1].isType(DoubleType))
			pf1 = new ConstPField((double) args[1]);
		if (pf0 && pf1)
			return createPFieldHandle(new MultPField(pf0, pf1));
	}
	die("mul", "Usage: pfield = mul(arg1, arg2)\n"
					"(Args can be pfields or constants.)");
	return NULL;
}


// --------------------------------------------------------------------- add ---
Handle add(const Arg args[], const int nargs)
{
	if (nargs == 2) {
		PField *pf0 = (PField *) args[0];
		PField *pf1 = (PField *) args[1];
		if (!pf0 && args[0].isType(DoubleType))
			pf0 = new ConstPField((double) args[0]);
		if (!pf1 && args[1].isType(DoubleType))
			pf1 = new ConstPField((double) args[1]);
		if (pf0 && pf1)
			return createPFieldHandle(new AddPField(pf0, pf1));
	}
	die("add", "Usage: pfield = add(arg1, arg2)\n"
					"(Args can be pfields or constants.)");
	return NULL;
}


// --------------------------------------------------------------------- div ---
static double divop(double x, double y)
{
	return (y != 0) ? x / y : 999999999999999999.9;
}

Handle div(const Arg args[], const int nargs)
{
	if (nargs == 2) {
		PField *pf0 = (PField *) args[0];
		PField *pf1 = (PField *) args[1];
		if (!pf0 && args[0].isType(DoubleType))
			pf0 = new ConstPField((double) args[0]);
		if (!pf1 && args[1].isType(DoubleType))
			pf1 = new ConstPField((double) args[1]);
		if (pf0 && pf1)
			return createPFieldHandle(new PFieldBinaryOperator(pf0, pf1, divop));
	}
	die("div", "Usage: pfield = div(arg1, arg2)\n"
					"(Args can be pfields or constants.)");
	return NULL;
}


// --------------------------------------------------------------------- sub ---
static double subop(double x, double y)
{
	return x - y;
}

Handle sub(const Arg args[], const int nargs)
{
	if (nargs == 2) {
		PField *pf0 = (PField *) args[0];
		PField *pf1 = (PField *) args[1];
		if (!pf0 && args[0].isType(DoubleType))
			pf0 = new ConstPField((double) args[0]);
		if (!pf1 && args[1].isType(DoubleType))
			pf1 = new ConstPField((double) args[1]);
		if (pf0 && pf1)
			return createPFieldHandle(new PFieldBinaryOperator(pf0, pf1, subop));
	}
	die("sub", "Usage: pfield = sub(arg1, arg2)\n"
					"(Args can be pfields or constants.)");
	return NULL;
}


