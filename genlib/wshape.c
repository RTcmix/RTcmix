float wshape(float x, double *f, int len)
{
/* use function f as static transfer function from x to returned value*/

        int i1;
	float x1, diff;


	x1=(x+1)*(float)((len-1)/2.);
	i1=x1;
	diff=(float)(x1-i1);

	return(*(f+i1)+(*(f+i1+1)-*(f+i1))*diff);
}
