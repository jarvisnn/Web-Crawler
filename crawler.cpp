//
//  main.cpp
//  Web Crawler
//
//  Created by Jarvis Nguyen on 16/9/16.
//  Copyright © 2016 Jarvis Nguyen. All rights reserved.
//

#include "clientSocket.h"
#include "parser.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <map>
#include <condition_variable>

using namespace std;

// Config Struct with default settings.
typedef struct {
	int crawlDelay = 1000;
	int maxThreads = 10;
	int depthLimit = 10;
	int pagesLimit = 10;
	int linkedSitesLimit = 10;
	vector<string> startUrls;
} Config;

// CrawlerState for storing necessary info of the Crawler
struct CrawlerState {
	int threadsCount;
	queue< pair<string, int> > pendingSites;
	map<string, bool> discoveredSites;
};

// Variables
Config config;
CrawlerState crawlerState;
mutex m_mutex;
condition_variable m_condVar;
bool threadFinished;

Config readConfigFile();
void initialize();
void scheduleCrawlers();
void startCrawler(string baseUrl, int currentDepth, CrawlerState &crawlerState);

int main(int argc, const char * argv[]) {		
	config = readConfigFile();
	initialize();
	scheduleCrawlers();
    return 0;
}

//***************************************************************************
// Functions' Implementation 
//***************************************************************************

//---------------------------------------------------------------------------
// Read and Process the config file. 
//---------------------------------------------------------------------------
Config readConfigFile() {
	try {
		ifstream cfFile ("config.txt");
		string var, val, url;
		Config cf;
		while (cfFile >> var >> val) {
			if (var == "crawlDelay") cf.crawlDelay = stoi(val);
			else if (var == "maxThreads") cf.maxThreads = stoi(val);
			else if (var == "depthLimit") cf.depthLimit = stoi(val);
			else if (var == "pagesLimit") cf.pagesLimit = stoi(val);
			else if (var == "linkedSitesLimit") cf.linkedSitesLimit = stoi(val);
			else if (var == "startUrls") {
				for (int i = 0; i < stoi(val); i++) {
					cfFile >> url;
					cf.startUrls.push_back(url);
				}
			}
		}
		cfFile.close();
		return cf;
	} catch ( exception &error ) {
		cerr << "Exception (@readConfigFile): " << error.what() << endl;
		exit(1);
	}
}

//---------------------------------------------------------------------------
// Initialize the Crawler. 
//---------------------------------------------------------------------------
void initialize() {
	// Set threads count to 0
	crawlerState.threadsCount = 0; 
	// Add starting urls
	for (auto url : config.startUrls) {
		crawlerState.pendingSites.push(make_pair(getHostnameFromUrl(url), 0));
		crawlerState.discoveredSites[getHostnameFromUrl(url)] = true;
	}
}

//---------------------------------------------------------------------------
// Schedule crawlers in multithreads. Each thread discovers a specific host.
//---------------------------------------------------------------------------
void scheduleCrawlers() {
	while (crawlerState.threadsCount != 0 || !crawlerState.pendingSites.empty()) {
		m_mutex.lock();
		threadFinished = false;
		while (!crawlerState.pendingSites.empty() && crawlerState.threadsCount < config.maxThreads) {
			// getting the next url to fetch
			auto nextSite = crawlerState.pendingSites.front();
			crawlerState.pendingSites.pop();
			crawlerState.threadsCount++;

			// start a new thread for crawling
			thread t = thread(startCrawler, nextSite.first, nextSite.second, ref(crawlerState));
			if (t.joinable()) t.detach();
			// if (t.joinable()) t.join();
		}
		m_mutex.unlock();

		// wait for some thread is done, then request a lock & try to schedule again
		unique_lock<mutex> m_lock(m_mutex);
		while (!threadFinished) m_condVar.wait(m_lock);
	}
}

//---------------------------------------------------------------------------
// Start a crawler to discover a specific website.
//---------------------------------------------------------------------------
void startCrawler(string hostname, int currentDepth, CrawlerState &crawlerState) {
	// Create socket, discover this website
	ClientSocket clientSocket = ClientSocket(hostname, 80, config.pagesLimit, config.crawlDelay);
	SiteStats stats = clientSocket.startDiscovering();

	// Finish discovering, Exiting a crawler thread, output all statistics of the website.
	m_mutex.lock();
	cout << "----------------------------------------------------------------------------" << endl; 
	cout << "Website: " << stats.hostname << endl;
	cout << "Depth (distance from the starting pages): " << currentDepth << endl;
	cout << "Number of Pages Discovered: " << stats.pagesDiscovered << endl;
	cout << "Number of Pages Failed to Discover: " << stats.pagesFailed << endl;
	cout << "Number of Linked Sites: " << stats.linkedSites.size() << endl;
	if (stats.minResponseTime < 0) cout << "Min. Response Time: N.A" << endl;
		else cout << "Min. Response Time: " << stats.minResponseTime << "ms" << endl;
	if (stats.maxResponseTime < 0) cout << "Max. Response Time: N.A" << endl;
		else cout << "Max. Response Time: " << stats.maxResponseTime << "ms" << endl;
	if (stats.averageResponseTime < 0) cout << "Average Response Time: N.A" << endl;
		else cout << "Average Response Time: " << stats.averageResponseTime << "ms" << endl;

	// Only discover more if haven't reached the depthLimit
	if (currentDepth < config.depthLimit) {
		// Only discover more maximum of "linkedSitesLimit" websites.
		for (int i = 0; i < min(int(stats.linkedSites.size()), config.linkedSitesLimit); i++) {
			string site = stats.linkedSites[i];
			if (!crawlerState.discoveredSites[site]) {
				crawlerState.pendingSites.push(make_pair(site, currentDepth+1));
				crawlerState.discoveredSites[site] = true;
			}
		}
	}
	crawlerState.threadsCount --;
	threadFinished = true;
	m_mutex.unlock();
	
	// Notify the master (original thread) about this ThreadFinished event.
	m_condVar.notify_one();
}


