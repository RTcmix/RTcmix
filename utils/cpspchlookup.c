#include <math.h>
#include <stdlib.h>
float cpspch(float);

main(argc,argv)
int argc;
char *argv[];
{

  float pch;
  float cps;
  int i;
  if(argc == 1) {
    printf("you must specify pitch in 8ve.pc form\n");
    exit(0);
  }
  for(i=1; i< argc; i++)
    printf("%f = %f\n",atof(argv[i]),cpspch(atof(argv[i])));
}
