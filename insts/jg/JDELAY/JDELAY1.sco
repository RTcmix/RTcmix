/* JDELAY: Regenerating delay instrument, adapted from DELAY.
 *    The differences between JDELAY and DELAY are:
 *       - JDELAY uses interpolating delay line fetch and
 *         longer-than-necessary delay line, both of
 *         which make it sound less buzzy for audio-range delays
 *       - provides a simple low-pass filter
 *       - provides control over wet/dry mix
 *       - provides a DC blocking filter
 *       - by default the delay is "post-fader," as before, but there's
 *         a pfield switch to make it "pre-fader."
 *
 * Parameters:
 *    p0 = output start time
 *    p1 = input start time
 *    p2 = input duration
 *    p3 = amplitude multiplier
 *    p4 = delay time
 *    p5 = regeneration multiplier (must be less than 1!)
 *    p6 = ring-down duration
 *    p7 = cutoff freq for low-pass filter (in cps)  (0 to disable filter)
 *    p8 = wet/dry mix (0: dry -> 1: wet)
 *    p9 = input channel number [optional, default is 0]
 *    p10 = stereo spread (0-1, % to left chan) [optional, default is .5]
 *    p11 = pre-fader send (0: No, 1: Yes) [optional, default is No]
 *    p12 = apply DC blocking filter (0: No, 1: Yes) [optional, default is Yes]
 *          (DC bias can affect sounds made with high regeneration setting.)
 *
 *    Assumes function slot 1 is the amplitude envelope (see above)
 *    Or you can just call setline. If no setline or function table 1, uses
 *    flat amplitude curve.
 *
 *    If pre-fader send is set to 1, sends input signal to delay line with
 *    no attenuation. Then p3 (amp multiplier) and setline controls entire
 *    note, including delay ring-down.
 *
 *    John Gibson (jgg9c@virginia.edu), 6/23/99
 */
rtsetparams(44100, 2)
load("JDELAY")

rtinput("astereo.snd")

outskip = 0
inskip = 1
indur = DUR() - inskip
amp = 1.0
deltime = 1/cpspch(7.02)
feedback = .980
ringdowndur = 2
percent_wet = 0.5

setline(0,0, .01,1, indur/1.1,1, indur,0)

cutoff = 4000
JDELAY(outskip, inskip, indur, amp, deltime, feedback, ringdowndur,
       cutoff, percent_wet, inchan=0, spread=1)
JDELAY(outskip, inskip, indur, amp, deltime, feedback, ringdowndur,
       cutoff, percent_wet, inchan=1, spread=0)

