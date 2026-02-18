// Basic InstrumentBus test: WAVETABLE -> aux0 -> MIX -> output
// Tests the pull model with a simple instrument chain

rtsetparams(44100, 2, 512)
load("WAVETABLE")
// MIX is built into the binary - no load() needed

// WAVETABLE writes to aux bus 0 (mono)
bus_config("WAVETABLE", "aux 0 out")

// MIX reads from aux bus 0 and writes to output
bus_config("MIX", "aux 0 in", "out 0-1")

// Create a simple sine wave envelope
makegen(1, 24, 1000, 0,0, 0.1,1, 0.9,1, 1,0)  // attack-sustain-release
makegen(2, 10, 1000, 1)  // sine wave

dur = 2.0

// WAVETABLE generates audio to aux0
WAVETABLE(0, dur, 10000, 440)

// MIX passes aux0 to stereo output (center pan)
MIX(0, 0, dur, 1.0, 0, 1)

print("InstrumentBus basic test: WAVETABLE -> aux0 -> MIX -> output")