rtsetparams(44100, 1, 1024);
load("./libLOOP.so");

rtinput("../../../snd/input.wav")

loopstart = 1999;

looplen = makeconnection("mouse", "x", 16, 60000, 5000, 50);
transp = makeconnection("mouse", "y", -3.0, 3.0, 0.0, 50);

LOOP(st=0, inskip=0, 50.0, amp=1, makeconverter(transp,"pchoct"), loopstart, looplen);

