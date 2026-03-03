float testMessageHandler(string path, list args)
{
	printf("testMessageHandler called with path '%s' and args %l\n", path, args);
	return 0;
}

oscRegisterMessageHandler("/test", testMessageHandler);


