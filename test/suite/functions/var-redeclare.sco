float variable;
variable = 10;

float function()
{
	variable = 20;
	return 0;
}

float function2(float variable)
{
	variable = 20;
	return 0;
}

float function3()
{
	float variable;
	variable = 30;
	return variable;
}

print(variable);
if (variable != 10) { error("score failed"); }

function();
print(variable);
if (variable != 20) { error("setting of global within function() failed"); }

function2(40);
print(variable);
if (variable != 20) { error("global should not have been set within function2()"); }


function3();
print(variable);
if (variable != 20) { error("global should not have been set within function3()"); }

