class Instrument;

struct rt_item {
	struct rt_item *rt_next;
	Instrument* (*rt_ptr)();
	char *rt_name;
	};

extern rt_item *rt_list;

extern void heapify(Instrument *Iptr);
extern int addrtInst(rt_item*);

extern "C" {
    void merror(char*);
}

extern "C" void rtprofile();

#define RT_INTRO(flabel, func) \
	{ extern Instrument* func(); \
		static rt_item this_rt = { NULL, func, flabel }; \
		if (addrtInst(&this_rt) == -1) \
		  merror(flabel); \
	}
