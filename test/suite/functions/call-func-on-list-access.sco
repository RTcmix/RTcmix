float foo() { printf("foo called\n"); return 0; }

flist = { foo };

flist[0]();

