rtsetparams(44100, 2)
load("/home/me/MYINST/libMYINST.so")

rtinput("/mysounds/foo.aiff")

MYINST(st=0, inskip=0, DUR(), amp=1, inchan=0, pctleft=.2)

