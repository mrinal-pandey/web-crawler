#include<stdio.h>
#include<curl/curl.h>

#include "web_crawler.h"

int url_count = 0;

void extract_urls(TidyNode root)
{
	// Loop through all the childs of given node
	for(TidyNode child = tidyGetChild(root); child != NULL; child = tidyGetNext(child))
	{
		// Recursively call the method to visit each node
		extract_urls(child);
		// Get href attribute of a tag
		TidyAttr hrefAttr = tidyAttrGetById(child, TidyAttr_HREF);
		// If href exists, get its value and check if it is a HTTP URL or a file URL
		// Display if it is a HTTP URL and increase url_count by one
		if(hrefAttr)
		{
			const char* url;
			url = tidyAttrValue(hrefAttr);
			if(strstr(url, "http"))
			{
				printf("%s\n", url);
				++url_count;
			}
		}
	}
}

void tidy_the_html(char* input_html)
{
	// To store parsed HTML
	TidyBuffer parsed_html = {0};
	// To store any errors
	TidyBuffer error_buffer = {0};
	// To store response code by libtidy
	int response = -1;
	Bool status;
    
	// Create TidyDoc instance
	TidyDoc tdoc = tidyCreate();                     
    
	// Set all the options and check if an error occurs at any step
	status = tidyOptSetBool(tdoc, TidyXhtmlOut, yes);  
	if(status)
	{
        	response = tidySetErrorBuffer(tdoc, &error_buffer);
	}
	if(response >= 0)
	{
        	response = tidyParseString(tdoc, input_html);
	}
	if(response >= 0)
	{
		response = tidyCleanAndRepair(tdoc);  
	}
	if(response >= 0)
	{
		response = tidyRunDiagnostics(tdoc);
	}
	if(response > 1)
	{
        	response = (tidyOptSetBool(tdoc, TidyForceOutput, yes) ? response : -1);
	}
	if(response >= 0)
	{
        	response = tidySaveBuffer(tdoc, &parsed_html);
	}
    	if(response >= 0)
	{
		response = tidyParseBuffer(tdoc, &parsed_html);
	}
	if(response >= 0)
      	{
		// Get body of the HTML
		TidyNode root_node = tidyGetBody(tdoc);
		// Send it to extract_urls to extract URLs out of HTML
		extract_urls(root_node);
      	}
      	else
	{
		// If any libtidy error occurs
        	printf("Can't find URL(s)\n[ERROR] TidyHTML failure with (%d) code :(\n", response);
	}
	// Free the libtidy resources allocated
	tidyBufFree(&parsed_html);
      	tidyBufFree(&error_buffer);
      	tidyRelease(tdoc);
}

// callback function triggered by curl
size_t callback(char *data, size_t size, size_t nmemb){
	size_t realsize = size * nmemb;
	// send the data to libtidy to clean it and extract urls out of the HTML
	tidy_the_html(data);
	return realsize;
}

// function to get URLs on the site provided as a command line argument
void getURLs(char* url)
{
	// libcurl handle
	CURL *curl_handle;
	CURLcode response;
	// globally initialize curl
	curl_global_init(CURL_GLOBAL_DEFAULT);
	// initialize curl handle
	curl_handle = curl_easy_init();
	if(curl_handle) {
    		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
		// callback is the method called when curl is done downloading HTML from given url
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, callback);
		// perform the download
		response = curl_easy_perform(curl_handle);
		// If a failure has occurred within curl, prompt it, else check url_count and prompt the
		// user accordingly
		if(response != CURLE_OK)
		{
			printf("Can't find URL(s)\n[ERROR] libcurl failure with (%d) code :(\n", response);
		}
		else if(url_count == 0)
		{
			printf("\nNo URL(s) on this site :(\n");
		}
		else
		{
			printf("\n%d URL(s) found! B-)\n", url_count);
		}
		// Clean curl handle
		curl_easy_cleanup(curl_handle);
	}
	// Clean curl globally
	curl_global_cleanup();
}
