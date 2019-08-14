rtsetparams(44100, 2, 256)
load("LOOP")
rtinput("../../../snd/nucular.wav", "mem");

insk=0
outsk=0
amp = 1

nframes = trunc(44100 * DUR());
inchan = 0

lstart = nframes/5
llen = makeconnection("mouse", "x", min=16, max=nframes/8, dflt=4410, lag=50, "loop length", "samples", 2);

otrans = makeconnection("mouse", "y", min=-2.00, max=2.00, dflt=0, lag=50, "pitch", "lin 8ve", 2);

trans = makeconverter(otrans, "pchoct");

LOOP(outsk,insk,120,amp,trans,lstart,llen,inchan,pan=0.5);
