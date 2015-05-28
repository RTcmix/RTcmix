float variable;
variable = 10;

float function(float variable)
{
	variable = 20;
	return 0;
}

float function2()
{
	float variable;
	variable = 30;
}

print(variable);

function(40);

print(variable);

function2();

print(variable);

