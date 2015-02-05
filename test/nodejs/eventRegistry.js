var er = require('../../src/nodejs/scripts/eventRegistry'); 

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
er.getEventsByName(event_label, 10, function(E) { 
	events = E.events.results;
	console.log('\nEVENTS (Ebola virus disease):');
	for (var i=0; i < events.length; i++) { console.log(events[i].uri+', '+events[i].eventDate); }
});

// get articles from the first event
var eventId = 2205305; // RECAP: Doctors investigating first ebola case in Scotland after aid worker returns from West Africa
var articles;
er.getArticles(eventId, 10, function(A) { 
	articles = A[eventId].articles.results;
	console.log('\nARTICLES (RECAP: Doctors investigating first ebola case in Scotland after aid worker returns from West Africa):');
	for (var i=0; i < articles.length; i++) { console.log(articles[i].title+', '+articles[i].date+', '+articles[i].time); }
});

// get recent articles and activity Ids
var recentArticles;
var recentEventIds;
er.getRecentActivity(10, 10, 0.3, function(J) {
	recent = J;
	recentArticles = J.recentActivity.articles.activity;
	recentEventIds = J.recentActivity.events.activity
	console.log('\nRECENT ARTICLES:');
	for (var i=0; i < recentArticles.length; i++) { console.log(recentArticles[i].title+', '+recentArticles[i].date+', '+recentArticles[i].time); }
});

