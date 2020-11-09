// testing ability to assign functions as variables

// declare function and then copy to a variable, then call function from variable

string func() { return "stringy"; }

fcopy = func;

s = fcopy();

print(s);



