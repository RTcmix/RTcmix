# Quick translation of sample_scos_3.0/LONGCHAIN_1.sco to Perl, for testing.

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

$numnotes = makegen(3, 2, 99,
                   5.00, 5.001, 5.02, 5.03, 5.05, 5.07, 5.069, 5.10, 6.00);
$transposition = 2.00;   # try 7.00 also, for some cool aliasing...
srand(2);


# ---------------------------------------------------------------- synth ---
$notedur = .10;
$incr = $notedur + .015;

$maxampdb = 92;
$minampdb = 75;
$ampdiff = $maxampdb - $minampdb;

control_rate(20000);        # need high control rate for short synth notes
setline(0,0, 1,1, 20,0);
makegen(2, 10, 10000, 1, .9, .7, .5, .3, .2, .1, .05, .02);

for ($st = 0; $st < $totdur; $st += $incr) {
   $slot = random() * $numnotes;
   $pitch = pchoct(octpch(sampfunc(3, $slot)) + octpch($transposition));
   $amp = ampdb($minampdb + ($ampdiff * random()));
   WAVETABLE($st, $notedur, $amp, $pitch, $pctleft=random());
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
$flangetype = 0;

$gensize = 100000;
makegen(2,10,$gensize, 1);

$maxdelay = 1.0 / cpspch($lowpitch);

setline(0,1,1,1);

FLANGE($st=0, $insk=0, $totdur, $amp, $resonance, $maxdelay, $moddepth,
         $modspeed, $wetdrymix, $flangetype, $inchan=0, $pctleft=1);

$lowpitch += .07;
$maxdelay = 1.0 / cpspch($lowpitch);

makegen(2,9,$gensize, 1,1,-180);

FLANGE($st=0, $insk=0, $totdur, $amp, $resonance, $maxdelay, $moddepth,
         $modspeed, $wetdrymix, $flangetype, $inchan=1, $pctleft=0);


# ---------------------------------------------------------------- delay ---
$deltime = $notedur * 2.2;
$regen = 0.70;
$wetdry = 0.12;
$cutoff = 0;
$ringdur = 2.0;

setline(0,0, $atk,1, $totdur-$dcy,1, $totdur,0);

JDELAY($st=0, $insk=0, $totdur, $amp, $deltime, $regen, $ringdur, $cutoff,
         $wetdry, $inchan=0, $pctleft=1);
JDELAY($st=0.02, $insk=0, $totdur, $amp, $deltime, $regen, $ringdur, $cutoff,
         $wetdry, $inchan=1, $pctleft=0);


# --------------------------------------------------------------- reverb ---
$revtime = 1.0;
$revpct = .3;
$rtchandel = .05;
$cf = 0;

setline(0,1, 1,1);

REVERBIT($st=0, $insk=0, $totdur+$ringdur, $amp, $revtime, $revpct,
            $rtchandel, $cf);



# john gibson, 17-june-00
