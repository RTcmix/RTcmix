system("F1 /audio/doug/lptest.snd");
output("/audio/doug/lptest.snd");

float frames;

frames = dataset("/audio/doug/lpcfiles/em.lpc");

lpcstuff(.001, .3, 0);

set_thresh(0.0, 1.0);

lpcplay(0, 1.0, 0.001, 1, frames, 1, -0.4657, 0);
