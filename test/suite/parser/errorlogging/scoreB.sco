

if (testlevel == 2) { printf("error in scoreB.sco, line 3\n"); ++b; }


include scoreC.sco

if (testlevel == 3) { printf("error in scoreB.sco, line 8, between include of C and D\n"); ++b; }

include scoreD.sco

if (testlevel == 4) { printf("error in scoreB.sco, line 12, after both includes\n"); ++b }
