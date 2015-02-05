var http = require('http');

function replaceAll(find, replace, str) {
	var find = find.replace(/[-\/\\^$*+?.()|[\]{}]/g, '\\$&');
	return str.replace(new RegExp(find, 'g'), replace);
}

module.exports = {
	
	suggest: function (prefix, resultCallback) {
	
		http.get('http://eventregistry.org/json/suggestConcepts?prefix='+prefix+'&lang=eng', function(res) {								
		
			var value='';
		
			res.on('data', function onData(d) {
					value += d;
			});
				
			res.on('end',  function() { 
				resultCallback(JSON.parse(value))
			});
			
		});
		
	},
	
	getEventsByUri: function (wikipediaUri, count, resultCallback) {

		wikipediaUri = encodeURIComponent(wikipediaUri);
		
		http.get('http://eventregistry.org/json/event?action=getEvents&conceptUri='+wikipediaUri+'&eventsConceptLang=eng&eventsCount='+count+'&eventsPage=0&eventsSortBy=rel&resultType=events', function(res) {								
		
			var value='';
		
			res.on('data', function onData(d) {
					value += d;
			});
				
			res.on('end',  function() { 
				resultCallback(JSON.parse(value))
			});
			
		});
		
	},
	
	getEventsByName: function (wikipediaName, count, resultCallback) {
		
		wikipediaName = replaceAll(' ', '_', wikipediaName);
		
		http.get('http://eventregistry.org/json/event?action=getEvents&conceptUri=http:%2F%2Fen.wikipedia.org%2Fwiki%2F'+wikipediaName+'&eventsConceptLang=eng&eventsCount='+count+'&eventsPage=0&eventsSortBy=rel&resultType=events', function(res) {								
		
			var value='';
		
			res.on('data', function onData(d) {
					value += d;
			});
				
			res.on('end',  function() { 
				resultCallback(JSON.parse(value))
			});
			
		});
		
	},
	
	getArticles: function (eventId, count, resultCallback) {
	
		http.get('http://eventregistry.org/json/event?action=getEvent&articlesCount='+count+'&articlesLang=eng&articlesPage=0&articlesSortBy=cosSim&eventUri='+eventId+'&resultType=articles',function(res) {				
			
			var value='';
		
			res.on('data', function onData(d) {
				value += d;
			});
		
			res.on('end', function() {
					resultCallback(JSON.parse(value));
			});
			
		});
	},
	
	getRecentActivity: function (articleCount, eventCount, sim, resultCallback) {
	
		http.get('http://eventregistry.org/json/overview?action=getRecentActivity&recentActivityArticlesLastArticleActivityId=0&recentActivityArticlesMaxArticleCount='+articleCount+'&recentActivityEventsLastEventActivityId=0&recentActivityEventsMaxEventCount='+eventCount+'&recentActivityEventsMinAvgCosSim='+sim,function(res) {				
			
			var value='';
		
			res.on('data', function onData(d) {
				value += d;
			});
		
			res.on('end', function() {
					resultCallback(JSON.parse(value));
			});
			
		});
	}
}