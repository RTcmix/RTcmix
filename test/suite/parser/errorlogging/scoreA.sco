testlevel = 0;
if (?level) { testlevel = $level; }

include scoreB.sco

if (testlevel == 0) { printf("error in scoreA.sco, line 6, after first include\n"); ++a; }

include scoreE.sco

if (testlevel == 1) { printf("error in scoreA.sco, line 10, after second include\n"); ++a }
