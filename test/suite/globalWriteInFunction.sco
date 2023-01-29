set_option("parser_warnings=2");

float functionWhichWritesToGlobal()
{
	globalVariable = 7;
	return 0;
}

float functionWhichReadsGlobal()
{
	x = 7 + globalVariable;
	return x;
}

globalVariable = 999;

functionWhichWritesToGlobal();

print(globalVariable);

functionWhichReadsGlobal();

