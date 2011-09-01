#include <ugens.h>
#include <spray.h>

void
sprayinit(struct slist *slist, int size, unsigned int seed)
{
	int i;
	if (size > MAX_SPRAY_SIZE)
		size = MAX_SPRAY_SIZE;
	slist->size = size;
	slist->current = size;
	srrand(seed);
	for(i=0; i<size; i++) {
		slist->array[i] = i;
		}
}

int
spray(struct slist *slist)
{
	int n,i,j;
	float rrand();
	n = ((rrand() + 1.)/2.) * (slist->current -1) +.5 ;
	j = slist->array[n];
	for(i=n; i<(slist->current-1); i++)
		slist->array[i] = slist->array[i+1];
	slist->current--;
	if(!slist->current) {
		slist->current = slist->size;
		for(i=0; i<slist->size; i++) {
			slist->array[i] = i;
		}
	}
	return(j);
}

