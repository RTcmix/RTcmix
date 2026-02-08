// Tier multi-writer test: Multiple WAVETABLE -> aux0 -> MIX -> output
// Tests accumulation of multiple writers to same tier

rtsetparams(44100, 2, 512)
load("WAVETABLE")
// MIX is built into the binary - no load() needed

// Both WAVETABLEs write to aux bus 0
bus_config("WAVETABLE", "aux 0 out")

// MIX reads from aux bus 0 and writes to output
bus_config("MIX", "aux 0 in", "out 0-1")

// Create envelopes
makegen(1, 24, 1000, 0,0, 0.1,1, 0.9,1, 1,0)
makegen(2, 10, 1000, 1)  // sine wave

dur = 2.0

// Two WAVETABLEs with different frequencies - should mix together
WAVETABLE(0, dur, 5000, 440)   // A4
WAVETABLE(0, dur, 5000, 554.37)  // C#5 (major third above A4)

// MIX passes the combined signal to stereo output
MIX(0, 0, dur, 1.0, 0, 1)

print("Tier multi-writer test: 2x WAVETABLE -> aux0 -> MIX -> output")