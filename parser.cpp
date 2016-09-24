//---------------------------------------------------------------------------
// CPP Implementation File for all kinds of parsers .i.e URL parser, HTML parser, etc.
//---------------------------------------------------------------------------

#include "parser.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace std;

//---------------------------------------------------------------------------
// Extract the Hostname from the URL
// Assume the url is in the correct format, no extra space at the beginning
//---------------------------------------------------------------------------
string getHostnameFromUrl(string url) {
    int offset = 0;
    offset = offset==0 && url.compare(0, 8, "https://")==0 ? 8 : offset;
    offset = offset==0 && url.compare(0, 7, "http://" )==0 ? 7 : offset;

    size_t pos = url.find("/", offset);
    string domain = url.substr(offset, (pos == string::npos ? url.length() : pos) - offset);

    return domain;
}

//---------------------------------------------------------------------------
// Extract the Host Path from the URL
// Assume the url is in the correct format, no extra space at the beginning
//---------------------------------------------------------------------------
string getHostPathFromUrl(string url) {
    int offset = 0;
    offset = offset==0 && url.compare(0, 8, "https://")==0 ? 8 : offset;
    offset = offset==0 && url.compare(0, 7, "http://" )==0 ? 7 : offset;
    
    size_t pos = url.find("/", offset);
    string path = pos==string::npos ? "/" : url.substr(pos);

    // Remove extra slashes
    pos = path.find_first_not_of('/');
    if (pos == string::npos) path = "/";
    	else path.erase(0, pos - 1);
	return path;
}

//---------------------------------------------------------------------------
// Extract all the URLS in the given text (HTTP raw response). Return a vector of <hostname, path>
//---------------------------------------------------------------------------
vector< pair<string, string> > extractUrls(string httpText) {
	// Reformat, remove the special characters first.
	string httpRaw = reformatHttpResponse(httpText);

	// URL's possible starting, only consider href, http:// and https:// case.
	const string urlStart[] = {"href=\"", "href = \"", "http://", "https://"};

	// URL's possible ending, includes #, ? for not counting the hash & query.
	const string urlEndChars = "\"#?, ";

	// Variable for result.
	vector< pair<string, string> > extractedUrls;

	for (auto startText : urlStart) {
		// For each startText, try to find all URLs matches
		while (true) {
			// Find the starting point of first possible URL
			int startPos = httpRaw.find(startText);
			if (startPos == string::npos) break;
			startPos += startText.length();

			// Find the ending point of first possible URL
			int endPos = httpRaw.find_first_of(urlEndChars, startPos);

			// Extract the url
			string url = httpRaw.substr(startPos, endPos - startPos);

			// Verify and Add to the list
			string urlDomain = getHostnameFromUrl(url);
			if (verifyUrl(url) && verifyUrl(urlDomain)) {
				extractedUrls.push_back(make_pair(urlDomain, getHostPathFromUrl(url)));
			}

			// Remove the extracted from the http data
			httpRaw.erase(0, endPos);
		}
	}

	return extractedUrls;
}

//---------------------------------------------------------------------------
// To verify a url, in case the fetched url is wrongs (in JS code, texts, etc.)
// We ignore css, js, pdf files, etc; for saving time from sending meanless requests.
// We also allow only some domains, as of the location limitation.
//---------------------------------------------------------------------------
bool verifyUrl(string url) {
	string allowedDomains[] = {".com", ".sg", ".net", ".co", ".org", ".me"};		
	bool flag = true;
	for (auto type : allowedDomains) if (url.find(type) != string::npos) {
		flag = false; break;									// check domain
	}
	if (flag) return false;

	string forbiddenTypes[] = {".css", ".js", ".pdf", ".png", ".jpeg", ".jpg"};						
	for (auto type : forbiddenTypes) 
		if (url.find(type) != string::npos) return false;		// Case special file types.
	if (url.find('.') == string::npos) return false;			// Case full-stop, double check.
	if (url.find("mailto:") != string::npos) return false;		// Case email-url, ignore it.
	return true;
}

//---------------------------------------------------------------------------
// As the raw HTTP response contains a lot of special character \0 \1 \t etc.
// This function is to reformat the data, only allow a specific set of characters.
//---------------------------------------------------------------------------
string reformatHttpResponse(string text) {
	string allowedChrs = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz01233456789.,/\":#?+-_= ";
	map<char, char> mm;
	for (char ch : allowedChrs) mm[ch] = ch;

	string res = "";
	for (char ch : text) {
		if (mm[ch]) res += tolower(mm[ch]);
	}
	return res;
}