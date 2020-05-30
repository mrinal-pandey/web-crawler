#include<stdio.h>
#include<curl/curl.h>

#include "web_crawler.h"
#include "logger.h"

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
		// If href exists, get url from it and check if url is a HTTP URL
		// check if it is less than 1000 characters long
		// Display and log if conditions are met and increase url_count by one
		if(hrefAttr)
		{
			const char* url;
			url = tidyAttrValue(hrefAttr);
			if(url && strstr(url, "http") && strlen(url) < 1000)
			{
				char url_in_log[1000];
				strcpy(url_in_log, url);
				logger(INFO_TAG, url_in_log);
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
		// If any libtidy error occurs log it
		char* message = "Can't find URL(s)";
		logger(INFO_TAG, message);
		printf("%s\n", message);
		char response_code[5];
		sprintf(response_code, "%d", response);
		char error_message[] = "libtidy failure with code : ";
		logger(ERROR_TAG, strcat(error_message, response_code));
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
	// Initialize logger
	init_logger();
	// For logging and displaying messages to user
	char* message;
	// Check if a URL was passed
	if(!url)
	{
		message = "Please pass a URL to crawl!";
		logger(WARNING_TAG, message);
		printf("%s\n", message);
		cleanup_logger();
		return;
	}
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
		// If a failure has occurred within curl, prompt and log it, else check url_count and prompt the
		// user accordingly
		if(response != CURLE_OK)
		{
			message = "Can't find URL(s)";
			logger(INFO_TAG, message);
			printf("%s\n", message);
			
			// log if a libcurl error occurs
			char response_code[5];
			sprintf(response_code, "%d", response);
			char error_message[] = "libcurl failure with code : ";
			logger(ERROR_TAG, strcat(error_message, response_code));
			
			message = "Check your internet connection, ensure that URL provided is correct.";
			logger(WARNING_TAG, message);
			printf("%s\n", message);
		}
		else if(url_count == 0)
		{
			message = "No URL(s) on this site :(";
			logger(INFO_TAG, message);
			printf("\n%s\n", message);
		}
		else
		{
			char count_of_url[50];
			sprintf(count_of_url, "%d", url_count);
			char url_found_message[] = " URL(s) found! B-)";
			logger(INFO_TAG, strcat(count_of_url, url_found_message));
			printf("\n%s\n", count_of_url);
		}
		// Clean curl handle
		curl_easy_cleanup(curl_handle);
	}
	// Clean curl globally
	curl_global_cleanup();
	// Clean up the logger
	cleanup_logger();
}
