#include "Interpolator.h"
#include <stdio.h>

#define TEST_LIST
//#define TEST_INTERPOLATOR

#ifdef TEST_LIST
#include "List.h"
class Foo {
public:
	Foo(int val) : _val(val), _next(NULL) {}
	void next(Foo * elem) { _next = elem; }
	Foo * next(void) const { return _next; }
	void val(int val) { _val = val; }
	int val(void) const { return _val; }
private:
	int _val;
	Foo *_next;
};
#endif // TEST_LIST

int main(int argc, char *argv[])
{
#ifdef TEST_LIST
	List <Foo *> list1;		// on stack
	List <Foo *> *list2 = new List<Foo *>();	// on heap
	Foo *foo1 = new Foo(1);
	Foo *foo2 = new Foo(2);
	Foo *foo3 = new Foo(3);
	Foo *bar1 = new Foo(11);
	Foo *bar2 = new Foo(22);
	Foo *bar3 = new Foo(33);

	list1.pushFront(foo1);
	list1.pushFront(foo2);
	list1.pushFront(foo3);
	list2->pushFront(bar1);
	list2->pushFront(bar2);
	list2->pushFront(bar3);
	//list2->removeElement(bar2);
	//list2->removeElement(bar2, bar3);	// NB: bar3 is head, bar1 is tail

	//printf("l1 head=%p (val=%d)\n", list1.head(), list1.head()->val());
#if 0
	for (int i = 0; i < 3; i++) {
		Foo *f = list1.popFront();
		Foo *b = list2->popFront();
		printf("%d, %d\n", f->val(), b->val());
	}
#else
	list1.appendList(list2);
	Foo *p = list1.head();
	while (p) {
		printf("%d (%p)\n", p->val(), p);
		p = p->next();
	}
	list1.deleteElements();
	long length = list1.length();
	printf("length: %ld\n", length);
	list1.pushFront(bar2);
	list1.pushFront(bar3);
	printf("length: %ld\n", list1.length());
/*
	long length = list1.length();
	printf("length: %ld\n", length);
	p = list1.head();
	while (p) {
		printf("%d\n", p->val());
		p = p->next();
	}
*/
#endif
#endif // TEST_LIST

#ifdef TEST_INTERPOLATOR
	Interpolator *interp = new Interpolator();

	const int period = 20;
	const int totsamps = 60;

	interp->samps(period);

	interp->init(1.0);
	interp->target(2.0);
	for (int i = 0; i < totsamps; i++) {
		float val = interp->next();
		printf("[%d] %f\n", i, val);
		if (i == 10)
			interp->target(10.0);
	}
/*
	int counter = 1;
	for (int i = 0; i < totsamps; i++) {
		if (--counter == 0) {
			interp->target(i);
			counter = period;
		}
		float val = interp->next();
		printf("%f\n", val);
	}
*/
#endif // TEST_INTERPOLATOR

	return 0;
}
