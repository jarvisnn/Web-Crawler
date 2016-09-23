//---------------------------------------------------------------------------
// C++ Implementation file for socket, to discover one specific website.
//---------------------------------------------------------------------------

#include "clientSocket.h"
#include "parser.h"
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

//---------------------------------------------------------------------------
// ClientSocket constructor
//---------------------------------------------------------------------------
ClientSocket::ClientSocket(string hostname, int port, int pagesLimit, int crawlDelay) {
    this->pagesLimit = pagesLimit;
    this->hostname = hostname;
    this->port = port;
    this->pendingPages.push("/");
    this->crawlDelay = crawlDelay;
    this->discoveredLinkedSites.clear();
}

//---------------------------------------------------------------------------
// Create a socket & connect to the hostname with well-known port 80.
// Return Error Description if failed or "" if successed.
//---------------------------------------------------------------------------
string ClientSocket::startConnection() {
    // Variables
    struct hostent *host;
    struct sockaddr_in server_addr;     

    // Get host address by hostname
    host = gethostbyname(hostname.c_str());
    if (host == NULL || host->h_addr == NULL) {
        return "Error getting DNS info!";
    }

    // Create Socket structure
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return "Cannot create socket!";
    }
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;     
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(server_addr.sin_zero),8); 

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        return "Cannot connect to server!";
    }

    return "";
}

//---------------------------------------------------------------------------
// Disconnect, slose the socket.
//---------------------------------------------------------------------------
string ClientSocket::closeConnection() {
    try {
        close(sock);
    } catch ( exception &error ) {
        return error.what();
    }
    return "";
}

//---------------------------------------------------------------------------
// Create HTTP GET request
//---------------------------------------------------------------------------
string ClientSocket::createHttpRequest(string host, string path) {
    string request = "";
    request += "GET " + path + " HTTP/1.1\r\n";
    request += "HOST:" + host + "\r\n";
    request += "Connection: close\r\n\r\n";
    return request;
}

//---------------------------------------------------------------------------
// Discover the website. Ignore failed requests(sends)
//---------------------------------------------------------------------------
SiteStats ClientSocket::startDiscovering() {
    SiteStats stats;
    stats.hostname = hostname;

    // While still have pages to discover & not exceed the max-page limit for a site.
    while (!pendingPages.empty() && (pagesLimit == -1 || stats.pagesDiscovered < pagesLimit)) {
        // Next page to fetch
        string path = pendingPages.front();
        pendingPages.pop();

        // Sleep for crawlDelay if this is not the first request
        if (path != "/") usleep(crawlDelay);

        // First mark this page as discovered
        discoveredPages[path] = true;

        // Clock Start
        high_resolution_clock::time_point startTime = high_resolution_clock::now();

        // Cannot create connection, simply ignore.
        if (this->startConnection() != "") {
            stats.pagesFailed++;
            continue;
        }

        // send GET resquest
        string send_data = createHttpRequest(hostname, path);
        if (send(sock, send_data.c_str(), strlen(send_data.c_str()), 0) < 0) {
            // Send failed. Note it and continue.
            stats.pagesFailed++;
            continue;
        }
        
        // get HTTP response from server  
        char recv_data[1024];
        int totalBytesRead = 0;
        string httpResponse = "";
        while (true) {
            bzero(recv_data, sizeof(recv_data));
            int bytesRead = recv(sock, recv_data, sizeof(recv_data), 0);
            if (bytesRead > 0) {
                string ss(recv_data);
                httpResponse += ss;
                totalBytesRead += bytesRead;
            } else {
                break;
            }
        }

        // Close connection. Ingore the error as data is received anyway.
        this->closeConnection();

        // Clock End
        high_resolution_clock::time_point endTime = high_resolution_clock::now();

        // Save Stats abt Response Time
        double responseTime = duration<double, milli>(endTime - startTime).count();
        stats.totalResponseTime += responseTime;
        stats.minResponseTime = stats.minResponseTime < 0 ? responseTime : min(stats.minResponseTime, responseTime);
        stats.maxResponseTime = stats.maxResponseTime < 0 ? responseTime : max(stats.maxResponseTime, responseTime);

        // Extract URLs from the received data.
        vector< pair<string, string> > extractedUrls = extractUrls(httpResponse);
        for (auto url : extractedUrls) {
            if (url.first == "" || url.first == hostname) {
                // Case 1: In the same host. Check if the path is discovered
                if (!discoveredPages[url.second]) {
                    pendingPages.push(url.second);
                }
            } else {
                // Case 2: In a different host, add to linkedSites
                if (!discoveredLinkedSites[url.first]) { 
                    discoveredLinkedSites[url.first] = true;
                    stats.linkedSites.push_back(url.first);
                }
            }
        }

        // Increase the number of pages discovered.
        stats.pagesDiscovered++;
    }
        
    if (stats.pagesDiscovered) 
        stats.averageResponseTime = stats.totalResponseTime / stats.pagesDiscovered;
    return stats;
}
