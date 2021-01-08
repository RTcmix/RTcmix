// Test multiple string-based keys

map mymap;

mymap["yikes"] = "brad";
mymap["mumble"] = "doug";

printf("should print brad followed by doug, each in quotes\n");

print(mymap["yikes"])
print(mymap["mumble"])

