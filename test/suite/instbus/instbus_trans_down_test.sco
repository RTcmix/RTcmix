// InstrumentBus TRANS test: WAVETABLE -> aux0 -> TRANS -> aux1 -> STEREO -> output
// Tests pull model with frame-ratio-changing instrument (TRANS) - octave DOWN
//
// Transposing DOWN (interval -1.0 = octave down): consumes 0.5x frames (1:2 ratio)
// TRANS produces 2 output frames for every 1 input frame.
//
// The pull model enables TRANS to request the correct number of frames from
// upstream, which was previously impossible with aux bus input.

rtsetparams(44100, 2, 512)
load("WAVETABLE")
load("TRANS")
load("STEREO")

// WAVETABLE writes to aux bus 0 (mono)
bus_config("WAVETABLE", "aux 0 out")

// TRANS reads from aux0, writes to aux1 (mono)
bus_config("TRANS", "aux 0 in", "aux 1 out")

// STEREO reads from aux1, writes to stereo output
bus_config("STEREO", "aux 1 in", "out 0-1")

// Create envelopes
makegen(1, 24, 1000, 0,0, 0.1,1, 0.9,1, 1,0)  // attack-sustain-release
makegen(2, 10, 1000, 1)  // sine wave

dur = 2.0

// === Test: Transpose DOWN one octave (1:2 ratio) ===
// TRANS at -1.0 octave produces samples slower than it consumes them
// With pull model, TRANS requests 0.5x frames from WAVETABLE

WAVETABLE(0, dur, 10000, 880)  // A5 -> becomes A4 (440Hz) after TRANS
TRANS(0, 0, dur, 1.0, -1.0)    // transpose down one octave
STEREO(0, 0, dur, 1.0, 0.5)    // pan center

print("InstrumentBus TRANS test: WAVETABLE(880Hz) -> TRANS(-1oct) -> STEREO -> output")
print("Expected output: 440Hz tone (A4)")