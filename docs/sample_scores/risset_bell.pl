# Illustrates using a function to play one note consisting of 
# multiple RTcmix notes.

use RT;

rtsetparams(44100, 2);
load("WAVETABLE");

# Set to 1 to write a sound file (if processor too slow for rt playback).
$writeit = 0;

if ($writeit) {
   set_option("audio_off", "clobber_on");
   rtoutput("/tmp/bell.aiff");
}

# just a sine wave (try extra harmonics for more complex bell sound)
makegen(2, 10, 5000,  1);

# exponential amplitude envelope
makegen(1, 5, 1000,  1, 1000, .0005);

# individual bell notes
#    start dur    amp   freq
bell(0,     5.0,  4000, 1000);
bell(3,     8.0,  5000, 1200);
bell(6,    12.0,  3000,  800);

# bell notes in a loop (needs fast processor for real time playback)
$incr = 0.1;
for ($st = 10; $st < 20; $st += $incr) {
   $freq = (random() * 1000) + 400;
   bell($st, 8, 2000, $freq);
   $incr += 0.2;
}


# Play one bell note.  Parameters are start time, duration, amplitude,
# and fundamental frequency.  (Based on description of Risset's bell
# instrument in the Dodge book.)

sub bell () {
   my($start, $dur, $amp, $freq) = @_;
   my $pan;
   print "fundamental frequency: $freq\n";
   WAVETABLE($start, $dur,         $amp,         $freq * .56,        $pan=1);
   WAVETABLE($start, $dur * .9,    $amp * .67,   $freq * .56 + 1,    $pan=0);
   WAVETABLE($start, $dur * .65,   $amp,         $freq * .92,        $pan=.9);
   WAVETABLE($start, $dur * .55,   $amp * 1.8,   $freq * .92 + 1.7,  $pan=.1);
   WAVETABLE($start, $dur * .325,  $amp * 2.67,  $freq * 1.19,       $pan=.8);
   WAVETABLE($start, $dur * .35,   $amp * 1.67,  $freq * 1.7,        $pan=.2);
   WAVETABLE($start, $dur * .25,   $amp * 1.46,  $freq * 2,          $pan=.7);
   WAVETABLE($start, $dur * .2,    $amp * 1.33,  $freq * 2.74,       $pan=.3);
   WAVETABLE($start, $dur * .15,   $amp * 1.33,  $freq * 3,          $pan=.6);
   WAVETABLE($start, $dur * .1,    $amp,         $freq * 3.76,       $pan=.4);
   WAVETABLE($start, $dur * .075,  $amp * 1.33,  $freq * 4.07,       $pan=.5);
}


# [scorefile by John Gibson]
