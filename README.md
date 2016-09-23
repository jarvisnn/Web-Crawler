Web Crawler
======
A simple web crawler in C++, using basic C++ socket programming.

Structure
------
+ **crawler.cpp**: main file, to manage threads, base URLs and to do the scheduling.
+ **parser.h/cpp**: includes URL parser, URL extractor from HTTP Raw Response, etc.
+ **clientSocket.h/cpp**: to discover pages of a website; create the socket, connect to server, send and receive HTTP messages, etc.

Setting
------
Custom setting is defined inside **config.txt**
+ **crawlDelay** time delay for fetching pages of same host.
+ **maxThreads** maximum threads, not includes the main thread.
+ **depthLimit** maximum depth to crawl; depth refers to the shortest distance/connection of a website to one of the starting sites.
+ **pagesLimit** maximum number of pages to discover in each site.
+ **linkedSitesLimit** maximum number of linked sites to discover; a website may discover a lot of more sites, the cost to discover all of them is too much.
+ **startUrls** list of starting URLs, refer to the file for the syntax.

Run
------
All is in **Makefile**
+ Run with terminal output:
```
make
```
+ Run with file output:
```
make file-output
```
