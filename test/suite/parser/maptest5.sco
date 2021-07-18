// Storing a map into other objects

map myMap;
myKey = 5934875;	// some random number for a key

myMap[myKey] = "Hi there";		// make it a real object


myList1 = { myMap };	// static initialization of list with map

testMap = myList1[0];
printf("This should print Hi there: %s\n", testMap[myKey]);


myList2 = {}
myList2[0] = myMap;			// dynamic adding of map to list
testMap = myList2[0];		// read back into variable
printf("This should print Hi there: %s\n", testMap[myKey]);

struct MapStore { map theMap }	// create struct to contain map
struct MapStore store;
store.theMap = myMap;			// add map to struct
testMap = store.theMap;			// read back into variable
printf("This should print Hi there: %s\n", testMap[myKey]);

