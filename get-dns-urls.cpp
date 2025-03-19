/*
===============================================================================
Copyright 2025 Ákos Nikházy

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
===============================================================================
*/
//

/******************************************************************************
 * name:		get-dns-urls.cpp
 *
 * desc:		makes a [setupid]-data.csv file with all the urls and visit 
 *				counts. Also prints all to cli.
 *
 *
 ******************************************************************************/

#include "json.hpp" // get it here https://github.com/nlohmann/json/blob/develop/single_include/nlohmann/json.hpp
#include <iostream>
#include <curl/curl.h> // build it like this: g++ get-dns-urls.cpp -o get-dns-urls -IC:/libcurl/include -LC:/libcurl/lib -lcurl
					   // also you need libcurl-x64.dll next to it from here https://curl.se/download.html
#include <fstream>


using json = nlohmann::json;

std::string url = "https://api.nextdns.io/profiles/{id}/analytics/domains?limit=100";
std::string apiKey;

/*
==================
replace a string
to a string
==================
*/
std::string replaceString(const std::string& str, const std::string& toReplace, const std::string& replaceWith) {
// thx https://stackoverflow.com/questions/1494399/how-do-i-search-find-and-replace-in-a-standard-string
	std::string result = str;
    
	size_t pos = 0;
    
    // Find all occurrences of "toReplace" and replace them with "replaceWith"
    while ((pos = result.find(toReplace, pos)) != std::string::npos) {
        
		result.replace(pos, toReplace.length(), replaceWith);
        pos += replaceWith.length();
		
    }
    
    return result;
}

/*
==================
Callback function 
to capture the 
response data
==================
*/
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
// thx https://gist.github.com/alghanmi/c5d7b761b2c9ab199157?permalink_comment_id=2909349#file-curl_example-cpp
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;

}
/*
==================
Makes request to 
NextDNS API and
parse the 
response.
==================
*/
void parseAPIRequest(const std::string requestURL,std::ofstream& csvFile) {
    
	CURL *curl;
    CURLcode res;
    std::string readBuffer;
    
	curl = curl_easy_init();
    
    if (curl) {
		
        // Set URL and headers
        curl_easy_setopt(curl, CURLOPT_URL, requestURL.c_str());

        struct curl_slist *headers = NULL;
        std::string authHeader = "X-Api-Key:" + apiKey;
		
        headers = curl_slist_append(headers, authHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // Disable SSL verification if SSH error happens

        // Capture response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Perform the request
        res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            // Parse response
            try {
                json j = json::parse(readBuffer);

				if(j.contains("error")) {
					std::cout << std::endl  << std::endl << "NextDNS says: error"  << std::endl  << std::endl;
					return;
				}

                // Extract the data array
                if (j.contains("data")) {
					
                    for (const auto& item : j["data"]) {
						
						std::string domain = item["domain"].get<std::string>() + ";" + std::to_string(item["queries"].get<int>());
						
						// print to cli
                        std::cout << "Domain: " << domain << std::endl;
						
						// put in csv file
						csvFile << domain << std::endl;
                    }
				}

                // NextDNS uses a "cursor" value to tell us where is the next page of data
                if (j.contains("meta") && j["meta"].contains("pagination")) {

					// if the cursor is null there are no more pages
					if(j["meta"]["pagination"]["cursor"].is_null()) return;
					
					
					std::string cursor = j["meta"]["pagination"]["cursor"];
					
                    std::string nextURL = url + "&cursor=" + cursor;
					
					// print the url with cursor between pages of data
					std::cout << std::endl  << std::endl << nextURL  << std::endl  << std::endl;
                    
					// recursion ʳᵉᶜᵘʳˢᶦᵒⁿ
					parseAPIRequest(nextURL, csvFile);
                    
                }
            }
            catch (const json::exception& e) {
                std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            }
        } else {
			
            std::cerr << "CURL request failed: " << curl_easy_strerror(res) << std::endl;
        
		}

        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}
int main(int argc, char* argv[]) {
 
	// gate keepers for missing arguments
	if( 1 == argc ) {
		std::cout << std::endl << "You need two arguments (get-dns-urls [setupid] [api key])";
		return 0;
	}
	
	if( 2 == argc ) {
		std::cout << std::endl << "You need two arguments: missing api key (get-dns-urls [setupid] [api key])";
		return 0;
	}
	
	// setup id is our file name prefix
	std::ofstream csvFile(
		replaceString("{id}-data.csv", "{id}", argv[1])
	);
   
	url = replaceString(url, "{id}", argv[1]);
	
	apiKey = argv[2];
	
	if (csvFile.is_open()) {
		
	
		std::cout << "Calling " + url;
		
		parseAPIRequest(url, csvFile);
		csvFile.close();
	}else {
		 std::cerr << "Unable to open file!" << std::endl;
	}

	std::cout << std::endl << "Done and dusted!";
    getchar();
    return 0;
}
