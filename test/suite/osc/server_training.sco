float testMessageHandler(list args)
{
	printf("testMessageHandler called with args %l\n", args);
	return 0;
}

registerOSCMessageHandler("/test", testMessageHandler);


