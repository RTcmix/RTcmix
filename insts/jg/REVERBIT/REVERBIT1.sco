/* REVERBIT: Lansky's fast reverberator
 *
 * Parameters:
 *    p0 = output start time
 *    p1 = input start time
 *    p2 = input duration
 *    p3 = amplitude multiplier (of input signal)
 *    p4 = reverb time (must be greater than 0)
 *    p5 = reverb percent (between 0 and 1 inclusive)
 *    p6 = right channel delay time (must be greater than 0)
 *    p7 = cutoff freq for low-pass filter (in cps)  (0 to disable filter)
 *    p8 = apply DC blocking filter (0: No, 1: Yes) [optional, default is Yes]
 *         (set to zero for a bit more speed)
 *
 *    Assumes function slot 1 is the amplitude envelope.
 *    Or you can just call setline. If no setline or function table 1, uses
 *    flat amplitude curve (all 1's). This curve, and p3, affect the
 *    input signal, not the output signal.
 *
 *    Input file can be mono or stereo. Output file must be stereo.
 *
 *    To quote Lansky, this is meant to "put a gloss on a signal."
 *    Here's how it works:
 *      (1) Runs the input signal into a simple Schroeder reverberator,
 *          scaling the output of that by the reverb percent and flipping
 *          its phase.
 *      (2) Puts output of (1) into a delay line, length determined by p6.
 *      (3) Adds output of (1) to dry signal, and places in left channel.
 *      (4) Adds output of delay to dry signal, and places in right channel.
 *
 *    I added the optional low-pass filter to the output of (1) and the
 *    optional DC blocking filter.
 *
 *    RT'd and spruced up a bit by John Gibson (jgg9c@virginia.edu), 6/24/99
 */
rtsetparams(44100, 2)
load("REVERBIT")

rtinput("../../../snd/input.wav")

start = 0
inskip = 0
dur = DUR()
amp = 1.0
rvbtime = .4
rvbpct = .5
rtchandelay = .03
cutoff = 0

REVERBIT(start, inskip, dur, amp, rvbtime, rvbpct, rtchandelay, cutoff)

start = start + dur + 2.0
rvbtime = 3.5
rvbpct = .2
rtchandelay = .08
cutoff = 1000

REVERBIT(start, inskip, dur, amp, rvbtime, rvbpct, rtchandelay, cutoff)
