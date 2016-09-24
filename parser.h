//---------------------------------------------------------------------------
// Header File for all kinds of parsers .i.e URL parser, HTML parser, etc.
//---------------------------------------------------------------------------

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

using namespace std;

string getHostnameFromUrl(string url);
string getHostPathFromUrl(string url);

vector< pair<string, string> > extractUrls(string httpRaw);
bool verifyUrl(string url);
string reformatHttpResponse(string text);

#endif
