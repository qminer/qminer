var er = require('../../src/nodejs/scripts/eventRegistry'); 
var c = require('../../src/nodejs/scripts/cascades');
var snap = require('../../src/nodejs/scripts/snap.js');

// suggest events with prefix 'ebola'
var suggestions;
er.suggest('ebola', function(S) { 
	suggestions = S;
	console.log("\nWIKIPEDIA SUGGESTIONS (ebola):");
	for (var i=0; i < suggestions.length; i++) { console.log(suggestions[i].label+', '+suggestions[i].id); }
});

// get event by label
var event_label = 'Ebola virus disease';
var events;
er.getEventsByName(event_label, 100, function(E) { 
	events = E.events.results;
	console.log('\nEVENTS (Ebola virus disease):');
	for (var i=0; i < events.length; i++) { console.log(events[i].uri+', '+events[i].eventDate); }
});

// get articles from the first event
var eventId = 2153989; // RECAP: Doctors investigating first ebola case in Scotland after aid worker returns from West Africa
var articles;
er.getArticles(eventId, 100, function(A) { 
	articles = A[eventId].articles.results;
	console.log('\nARTICLES (RECAP: Doctors investigating first ebola case in Scotland after aid worker returns from West Africa):');
	for (var i=0; i < articles.length; i++) { console.log(articles[i].title+', '+articles[i].date+', '+articles[i].time); }
});

// get recent articles and activity Ids
var recentArticles;
var recentEventIds;
er.getRecentActivity(10, 10, 0.5, function(J) {
	recent = J;
	recentArticles = J.recentActivity.articles.activity;
	recentEventIds = J.recentActivity.events.activity
	console.log('\nRECENT ARTICLES:');
	for (var i=0; i < recentArticles.length; i++) { console.log(recentArticles[i].title+', '+recentArticles[i].date+', '+recentArticles[i].time); }
});

// search events
var search;
er.searchEvents('ljubljana',10, function(S) {
	console.log('\nSEARCH RESULTS:');
	search = S.events.results;
	for (var i=0; i < search.length; i++) { console.log(search[i].uri+', '+search[i].eventDate); }
});

//cascades
var a = new Array();
c.getArticleBatches(events,0,a);
var p = c.getEventPublishers(a);
var c1 = c.getEventCascades(a,p);
c.printCascades('cascades2.txt',c1,p, 1000000000000);
var g = new snap.DirectedGraph();
snap.cascades('cascades2.txt',g,10,1,0);
var lbl = {};
for (var i=0; i<p.length; i++) {lbl[p[i].id] = p[i].name}
g.draw('gb10.html',lbl)