/* This is to work around an OS X linker bug.  The bug seems to have been
   fixed in OS X 10.1.
*/

void _carbon_init(int argc, char **argv)
{
}

void _objcInit(void)
{
}

