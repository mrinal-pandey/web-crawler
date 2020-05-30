#include<stdio.h>
#include "web_crawler.h"

int main(int argc, char* argv[])
{
	// Pass URL to be crawled as a command line argument
	getURLs(argv[1]);
	return 0;
}
