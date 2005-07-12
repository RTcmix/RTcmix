use RT;

rtsetparams(44100, 2);
load("WAVETABLE");

$dur = 5;

$amp = 10000;
$env = maketable("line", 100, 0,0, 1,1, 2,0);
$amp = mul($amp, $env);

$freq = 440;
$pan = 0.5;
$wavet = maketable("wave", 1000, "sine");

WAVETABLE(0, $dur, $amp, $freq, $pan, $wavet);

