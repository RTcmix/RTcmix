#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>

int
main(int argc, char *argv[])
{
  int i;

  if(argc == 1) {
    printf("you must specify pitch in 8ve.pc form\n");
    exit(0);
  }
  for(i=1; i< argc; i++)
    printf("%f = %f\n",atof(argv[i]),cpspch(atof(argv[i])));

  return 0;
}

