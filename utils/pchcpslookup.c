#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>

int
main(int argc, char *argv[])
{
  int i;

  if(argc == 1) {
    printf("you must specify pitch in hz\n");
    exit(0);
  }
  for(i=1; i< argc; i++)
    printf("%f = %f\n",atof(argv[i]),pchcps(atof(argv[i])));

  return 0;
}

