// Basic sanity check for struct constructor

struct MyStruct { float f };

ms = MyStruct(7);

if (ms.f != 7) { error("Constructor failed to initialize struct's float member"); }

struct MyStruct2 { string s };

ms2 = MyStruct2("hello");

if (ms2.s != "hello") { error("Constructor failed to initialize struct's string member"); }
