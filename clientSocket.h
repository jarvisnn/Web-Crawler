//---------------------------------------------------------------------------
// Header File for socket, to discover one specific website.
//---------------------------------------------------------------------------

#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <string>
#include <queue>
#include <vector>
#include <map>

using namespace std;

typedef struct {
    string hostname;                                    // hostname, base url
    double averageResponseTime = -1;                    /////
    double minResponseTime = -1;                        // response time stats
    double maxResponseTime = -1;                        /////
    int numberOfPagesFailed = 0;                        // number of pages that are failed to discover
    vector<string> linkedSites;                         // linked sites
    vector< pair<string, double> > discoveredPages;     // list of pages that are discovered, with response time
} SiteStats;

class ClientSocket {
    public:
        ClientSocket(string hostname, int port=80, int pagesLimit=-1, int crawlDelay=1000);
        SiteStats startDiscovering();
    private:
        string hostname;
        int sock, pagesLimit, port, crawlDelay;
        queue<string> pendingPages;
        map<string, bool> discoveredPages;
        map<string, bool> discoveredLinkedSites;
        string startConnection();
        string closeConnection();
        string createHttpRequest(string host, string path);
};

#endif
