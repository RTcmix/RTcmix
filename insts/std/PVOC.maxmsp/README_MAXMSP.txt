This is out-of-date with the main 4.0 PVOC, some kind of dynlib
monkey-busiess is going on with loading differet filters, and
max/msp dynloading isn't there yet --

BGG mm 6/2007

I put this "PVOC.maxmsp" folder into the main RTcmix trunk so that
I can compile PVOC for rtcmix~ and iRTcmix.  The Makefile will
choose one or the other; unfortunately this version will remain
out-of-date.  I don't see dynloading working easily for either
rtcmix~ or iOS (in fact I think it's still officially prohibited
by Apple for iOS) in the near future.

BGG 1/2013
