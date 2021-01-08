// testing storing/retrieving function objects on structs

string sfunc() { return "returned_string"; }

struct Test { mfunction fun };

struct Test t;

t.fun = sfunc;

//print(list_with_function);

// call function via struct member

s = t.fun();

print(s);


