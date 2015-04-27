/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */
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
	}
}