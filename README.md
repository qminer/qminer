QMiner
======

QMiner is an analytics platform for large-scale real-time streams containing structured and
unstructured data. It is designed to for scaling to millions of instances on high-end commodity 
hardware, providing efficient storage, retrieval and analytics mechanisms with real-time response.

## Documentation

+ [Quick start](https://github.com/qminer/qminer/wiki/Introduction)
+ [Code documentation](http://agava.ijs.si/~blazf/qminer/)
+ [Wiki](https://github.com/qminer/qminer/wiki)


## Introduction

### Analytics over Structured and Unstructured Data

QMiner supports reporting over longer time periods. The user can query the data along any dimension from structured and unstructured data (e.g. text, social networks). Results can be aggregate along any dimension and visualized using techniques for structured data (e.g. charts, histograms, maps). Aggregation of unstructured data is done using text mining techniques such as clustering, feature extraction and selection, and text visualization. Query results can also be used to define and build machine learning models.

### Real Time Processing

QMiner is optimized for the real-time index updates with response time in the order of milliseconds. It is integrated with a message queue for asynchronous input of new events to QMiner, and load balancing and buffering capabilities to handle spikes and failovers.

### Event Prediction

QMiner integrates several algorithms for event prediction, such as next failure in a network, next article read, or product bought by a user. They can be used to discover patterns in sequences, such as stream of complex events, and use these patterns for tasks such as event prediction and correlation. The resulting prediction models can be applied to the data stream in real time.

### Architecture

QMiner runs as an independent server and has no dependencies to external tools such as SQL databases or web servers. The functionality is exposed over web service API. Additional functionality can be added to the server via stored procedures written in JavaScript. Abstract data layer interface allows for easy integration with new data sources and connection with the data mining algorithms. This can significantly lower the cost of including additional databases, sensor networks, and other complex event streams into QMiner.

