float randf(float *oldval, float factor)
{
      float ccrandom(),range, xlowb, upb, temp;
      range = 2.0 - (2.0 * factor);
      xlowb = *oldval - range;
      if(xlowb < -1.) xlowb = -1; 
      upb = *oldval+range;
      if(upb > 1) upb = 1;  
      temp = xlowb+(upb-xlowb)*((ccrandom(*oldval)+1)*.5);
      *oldval=temp;
      return (temp); 
}


float ccrandom(x)
float x;
{
	int n;
	n=x*1048576.;
	return((float)((1061*n+221589) % 1048576)/1048576.);
}
