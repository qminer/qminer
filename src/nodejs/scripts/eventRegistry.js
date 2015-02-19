var http = require('http');

function replaceAll(find, replace, str) {
	var find = find.replace(/[-\/\\^$*+?.()|[\]{}]/g, '\\$&');
	return str.replace(new RegExp(find, 'g'), replace);
}

module.exports = {
		
	login: function (resultCallback) {
		
		var user = {
			email: 'mario.karlovcec@ijs.si',
			pass: 'jerneja4521tioS'
		};

		var userString = JSON.stringify(user);

		var headers = {
		  'Content-Type': 'application/json',
		  'Content-Length': userString.length
		};

		var options = {
		  host: 'eventregistry.org',
		  path: '/login',
		  method: 'POST',
		  headers: headers
		};

		var req = http.request(options, function(res) {
			res.setEncoding('utf-8');
			var responseString = '';

			res.on('data', function(data) {
				responseString += data;
			});

			res.on('end', function() {
				resultCallback(JSON.parse(responseString));
			});
		});

		req.on('error', function(e) {
			console.log('error happened: '+e);
		});
		
		req.write(userString);
		req.end();
		
	},
	
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
	
		http.get('http://eventregistry.org/json/event?action=getEvent&articlesCount='+count+'&articlesPage=0&articlesSortBy=date&eventUri='+eventId+'&resultType=articles',function(res) {				
			
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
	},
	
	searchEvents: function (term, count, resultCallback) {
	
		http.get('http://www.eventregistry.org/json/event?action=getEvents&eventsConceptLang=eng&eventsCount='+count+'&eventsPage=0&eventsSortBy=rel&ignoreKeywords=&keywords='+term+'&resultType=events',function(res) {				
			
			var value='';
		
			res.on('data', function onData(d) {
				value += d;
			});
		
			res.on('end', function() {
					resultCallback(JSON.parse(value));
			});
			
		});
	},
	
	getArticleBatchesArr: function (er, arr, outArray, resultCallback) {
		var i=0;
		getArticleBatchesArr_rec(er, arr, i, outArray, resultCallback);
	}
}

function getArticleBatchesArr_rec(er, arr, i, outArray, resultCallback) {

	var eventId = arr[i];
	
	er.getArticles(eventId, 100, function(A) { 
		articles = A[eventId].articles.results;
		outArray.push(articles);
		if (outArray.length<arr.length) {
			i++;
			getArticleBatchesArr_rec(er, arr, i, outArray, resultCallback);
		}
		if (outArray.length==arr.length) {
			console.log('now calling back '+i);
			resultCallback(outArray);
		}
	});
}