// testing storing/retrieving function objects on lists

string sfunc() { return "returned_string"; }

list_with_function = { sfunc };

//print(list_with_function);

// call function via list element

s = list_with_function[0]();

print(s);


