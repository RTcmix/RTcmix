// Storing a map as a MinC function argument

map myMap;
myKey = 5934875;	// some random number for a key

myMap[myKey] = "Hi there";		// make it a real object

// Declaring a function which takes a map as an argument

float mapAsArgument(map mapArgument)
{
	printf("This should print Hi there: %s\n", mapArgument[myKey]);
	return 0;
}

// Calling that function with a map

mapAsArgument(myMap);
