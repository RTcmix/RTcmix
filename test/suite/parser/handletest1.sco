set_option("require_sample_rate=false", "device=Aggregate Device", "record=true");
rtsetparams(44100, 2);
rtinput("AUDIO");

bus_config("MIX", "in0-1", "out0-1");

i = makeinstrument("MIX", 0, 0, 1, 1, 0, 1);

// Handle operations

print(i + 1);
print(i * 2);
print(1 + i);
print(2 * i);

print(i * i);


