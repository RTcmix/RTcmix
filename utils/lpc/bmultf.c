bmultf(array,mult,number)
float *array,mult;
{
	int i;
	for(i=0; i<number; i++) *array++ *= mult;
}
