# Quick translation of longchain.sco to Perl, for testing.  -JGG

# This score makes a wavetable synth riff and feeds it through 3 effects
# in series: flange -> delay -> reverb

use RT;

print_off();
rtsetparams(44100, 2);
load("WAVETABLE");
load("FLANGE");
load("JDELAY");
load("REVERBIT");

bus_config("WAVETABLE", "aux 0-1 out");
bus_config("FLANGE", "aux 0-1 in", "aux 10-11 out");
bus_config("JDELAY", "aux 10-11 in", "aux 4-5 out");
bus_config("REVERBIT", "aux 4-5 in", "out 0-1");

$totdur = 30;
$masteramp = 1.0;
$atk = 2; $dcy = 4;

@pitchtab = (5.00, 5.001, 5.02, 5.03, 5.05, 5.07, 5.069, 5.10, 6.00);
$numnotes = $#pitchtab + 1;

$transposition = 2.00;   # try 7.00 also, for some cool aliasing...
srand(2);


# ---------------------------------------------------------------- synth ---
$notedur = 0.10;
$incr = $notedur + 0.015;

$maxampdb = 92;
$minampdb = 75;

control_rate(20000);        # need high control rate for short synth notes
$env = maketable("line", 10000, 0,0, 1,1, 20,0);
$wavet = maketable("wave", 10000, 1, .9, .7, .5, .3, .2, .1, .05, .02);

$ampdiff = $maxampdb - $minampdb;
for ($st = 0; $st < $totdur; $st += $incr) {
   $index = int(random() * $numnotes);
   $pitch = pchoct(octpch($pitchtab[$index]) + octpch($transposition));
   $amp = ampdb($minampdb + ($ampdiff * random()));
   WAVETABLE($st, $notedur, mul($amp, $env), $pitch, $pan=random(), $wavet);
}


# for the rest
control_rate(500);
$amp = $masteramp;

# --------------------------------------------------------------- flange ---
$resonance = 0.3;
$lowpitch = 5.00;
$moddepth = 90;
$modspeed = 0.08;
$wetdrymix = 0.5;
$flangetype = "IIR";

$wavetablesize = 100000;
$wavet = maketable("wave", $wavetablesize, "sine");

$maxdelay = 1.0 / cpspch($lowpitch);

FLANGE($st=0, $insk=0, $totdur, $amp, $resonance, $maxdelay, $moddepth,
	$modspeed, $wetdrymix, $flangetype, $inchan=0, $pan=1, $ringdur=0, $wavet);

$lowpitch += 0.07;
$maxdelay = 1.0 / cpspch($lowpitch);

$wavet = maketable("wave3", $wavetablesize, 1, 1, -180);

FLANGE($st=0, $insk=0, $totdur, $amp, $resonance, $maxdelay, $moddepth,
	$modspeed, $wetdrymix, $flangetype, $inchan=1, $pan=0, $ringdur=0, $wavet);


# ---------------------------------------------------------------- delay ---
$deltime = $notedur * 2.2;
$regen = 0.70;
$wetdry = 0.12;
$cutoff = 0;
$ringdur = 2.0;

$env = maketable("line", 1000, 0,0, $atk,1, $totdur-$dcy,1, $totdur,0);

JDELAY($st=0, $insk=0, $totdur, mul($amp, $env), $deltime, $regen,
		$ringdur, $cutoff, $wetdry, $inchan=0, $pan=1);
JDELAY($st=0.02, $insk=0, $totdur, mul($amp, $env), $deltime, $regen,
		$ringdur, $cutoff, $wetdry, $inchan=1, $pan=0);


# --------------------------------------------------------------- reverb ---
$revtime = 1.0;
$revpct = 0.3;
$rtchandel = 0.05;
$cf = 0;

REVERBIT($st=0, $insk=0, $totdur + $ringdur, $amp, $revtime, $revpct,
		$rtchandel, $cf);

