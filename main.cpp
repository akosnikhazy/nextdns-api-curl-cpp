// just a very basic api call with curl, no error handling or any other functionality
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <conio.h>

int main()
{

    // you need the setup id. The API doc refers to it as :profile
    std::string url = "https://api.nextdns.io/profiles/[setup ID]/analytics/domains";
    
    // you find this at the bottom of you account page
    std::string apiKey = "[your API key]";

	CURL *curl;

	curl = curl_easy_init();
	  
	if (curl) {
       
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		struct curl_slist *headers = NULL;
		
		std::string authHeader = "X-Api-Key:" + apiKey; 
		
		headers = curl_slist_append(headers, authHeader.c_str());
		
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // Disable SSL cert verification because at writing this code nextdns fails this

		CURLcode res = curl_easy_perform(curl);
		
		// Cleanup
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
		
		
	
	}
	getch();
    return 0;

}
