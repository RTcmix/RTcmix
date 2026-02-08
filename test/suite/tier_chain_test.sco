// Tier chain test: WAVETABLE -> aux0 -> STEREO -> aux1 -> MIX -> output
// Tests multi-tier pull model with cascading dependencies

rtsetparams(44100, 2, 512)
load("WAVETABLE")
load("STEREO")
// MIX is built into the binary - no load() needed

// WAVETABLE writes to aux bus 0 (mono)
bus_config("WAVETABLE", "aux 0 out")

// STEREO reads from aux0, writes to aux1 (stereo)
bus_config("STEREO", "aux 0 in", "aux 1-2 out")

// MIX reads from aux1-2 and writes to output
bus_config("MIX", "aux 1-2 in", "out 0-1")

// Create envelopes
makegen(1, 24, 1000, 0,0, 0.1,1, 0.9,1, 1,0)  // attack-sustain-release
makegen(2, 10, 1000, 1)  // sine wave

dur = 2.0

// WAVETABLE generates 440Hz sine to aux0
WAVETABLE(0, dur, 10000, 440)

// STEREO pans the mono signal to stereo on aux1-2
STEREO(0, 0, dur, 1.0, 0.5)

// MIX passes aux1-2 to output
MIX(0, 0, dur, 1.0, 0, 1)

print("Tier chain test: WAVETABLE -> aux0 -> STEREO -> aux1-2 -> MIX -> output")