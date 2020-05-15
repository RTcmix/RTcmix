list nullList;

if (nullList) { printf("nullList not false for IF!\n"); exit("RTcmix", -1); }
while (nullList) { printf("nullList not false for while()\n"); exit("RTcmix", -1); }
if (nullList || 0) { printf("nullList not false for OR\n"); exit("RTcmix", -1); }
if (nullList) { printf("nullList not false for IF/ELSE!\n"); exit("RTcmix", -1); }
else {}
if (nullList && 1) { printf("nullList not false for AND\n"); exit("RTcmix", -1); }

goodList = { 1 };

if (goodList) { }
else { printf("goodList not true for IF/ELSE!\n"); exit("RTcmix", -1); }
if (!goodList) { printf("goodList not false for NOT\n"); exit("RTcmix", -1); }
if (!(goodList || 0)) { printf("goodList not true for OR\n"); exit("RTcmix", -1); }

handle nullHandle;

if (nullHandle) { printf("nullHandle not false for IF!\n"); exit("RTcmix", -1); }
while (nullHandle) { printf("nullHandle not false for while()\n"); exit("RTcmix", -1); }
if (nullHandle || 0) { printf("nullHandle not false for OR\n"); exit("RTcmix", -1); }
if (nullHandle) { printf("nullHandle not false for IF/ELSE!\n"); exit("RTcmix", -1); }
else {}
if (nullHandle && 1) { printf("nullHandle not false for AND\n"); exit("RTcmix", -1); }

goodHandle = maketable("literal", "nonorm", 2, 7, 8);

if (goodHandle) { }
else { printf("goodHandle not true for IF/ELSE!\n"); exit("RTcmix", -1); }
if (!goodHandle) { printf("goodHandle not false for NOT\n"); exit("RTcmix", -1); }
if (!(goodHandle || 0)) { printf("goodHandle not true for OR\n"); exit("RTcmix", -1); }

string nullString;

if (nullString) { printf("nullString not false for IF!\n"); exit("RTcmix", -1); }
while (nullString) { printf("nullString not false for while()\n"); exit("RTcmix", -1); }
if (nullString || 0) { printf("nullString not false for OR\n"); exit("RTcmix", -1); }
if (nullString) { printf("nullString not false for IF/ELSE!\n"); exit("RTcmix", -1); }
else {}
if (nullString && 1) { printf("nullString not false for AND\n"); exit("RTcmix", -1); }

goodString = "A string";

if (goodString) { }
else { printf("goodString not true for IF/ELSE!\n"); exit("RTcmix", -1); }
if (!goodString) { printf("goodString not false for NOT\n"); exit("RTcmix", -1); }
if (!(goodString || 0)) { printf("goodString not true for OR\n"); exit("RTcmix", -1); }

