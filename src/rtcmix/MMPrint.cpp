#include <MMPrint.h>
// BGGx mm
//#include <strings.h>
#include <string.h>

#define SIZEOF_MMPRINTBUF 65536 /* should move to dyn alloc at some point */

class MMPrint {
public:
    MMPrint() {};
    ~MMPrint() {};
    
    static char mm_print_buf[];
    static char *mm_print_ptr;
    static bool mm_cleared;
};

char MMPrint::mm_print_buf[SIZEOF_MMPRINTBUF];
char *MMPrint::mm_print_ptr;
bool MMPrint::mm_cleared = true;

int get_mm_print_space()
{
	return SIZEOF_MMPRINTBUF - (MMPrint::mm_print_ptr - MMPrint::mm_print_buf) - 1;
}

char *get_mm_print_ptr()
{
	return MMPrint::mm_print_ptr;
}

char *get_mm_print_buf()
{
    return MMPrint::mm_print_buf;
}

void set_mm_print_ptr(int v)
{
	// Check for overflow
	if (MMPrint::mm_print_ptr - MMPrint::mm_print_buf < SIZEOF_MMPRINTBUF - v) {
		MMPrint::mm_print_ptr += v;
		MMPrint::mm_cleared = false;
	}
}

int is_print_cleared()
{
	return MMPrint::mm_cleared;
}

void clear_print()
{
// BGGx ww -- use memset instead of bzero
//	bzero(MMPrint::mm_print_buf, SIZEOF_MMPRINTBUF);
	memset(MMPrint::mm_print_buf, 0, SIZEOF_MMPRINTBUF);

	MMPrint::mm_print_ptr = MMPrint::mm_print_buf;
	MMPrint::mm_cleared = true;
}

