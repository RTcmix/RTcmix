float crandom(float x)
{
	int n;
	n=x*1048576.;
	return((float)((1061*n+221589) % 1048576)/1048576.);
}
