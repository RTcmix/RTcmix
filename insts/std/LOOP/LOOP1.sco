rtsetparams(44100, 2, 256)
load("LOOP")
rtinput("../../../snd/nucular.wav", "mem");

insk=0
outsk=0
amp = 1
transp = 0.0;
if (?transposition) {
	transp = $transposition;
}

nframes = trunc(44100 * DUR());
inchan = 0

lstart = makeconnection("mouse", "x", min=0, max=nframes-1, dflt=0, lag=90, "position", "samples", 2);

llen = makeconnection("mouse", "y", min=16, max=nframes/8, dflt=4410, lag=90, "loop length", "samples", 2);

LOOP(outsk,insk,120,amp,transp,lstart,llen,inchan,pan=0.5);
