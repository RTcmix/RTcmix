set_option("require_sample_rate=false");
rtsetparams(44100, 2);
load("WAVETABLE");

bus_config("WAVETABLE", "out0-1");

i = makeinstrument("WAVETABLE", 0, 1, 1, 7.0, 0.5);

// Handle operations

print(i + 1);
print(i * 2);
print(1 + i);
print(2 * i);

print(i * i);


