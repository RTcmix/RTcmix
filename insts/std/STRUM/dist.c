float dist(float x)
{
/* Soft clipping: dist = x - 1/3 * x^3  */

     if(x>1.0) {  return(.66666667); }
     else  {  if(x>-1.0) { return(x - .33333333 * x*x*x); }
              else {return(-.66666667);}   }

   /* Tube-ish distortion: dist = (x +.5)^2 -.25  */
   /* this does not work with a feedback guitar */
   /* return ((x+.5)*(x+.5) - .25);*/
}
