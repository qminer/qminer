var assert = require('assert.js');

console.say("Test", "Starting test based on IMDB sample");

// only report failours
assert.silent = process.isArg("-nopass");

assert.exists(qm.store("People"), "Retrieve 'People' store");
assert.exists(qm.store("Movies"), "Retrieve 'Movies' store");

// test empty stores
var People = qm.store("People");
assert.equals(People.name, "People", "People.name");
assert.ok(People.empty, "People.empty");
assert.equals(People.length, 0, "People.length");
assert.equals(People.recs.length, 0, "People.recs.length");
assert.equals(People.fields.length, 2, "People.fields.length");
assert.equals(People.joins.length, 2, "People.joins.length");
assert.equals(People.keys.length, 2, "People.keys.length");
var Movies = qm.store("Movies");
assert.equals(Movies.name, "Movies", "Movies.name");
assert.ok(Movies.empty, "Movies.empty");
assert.equals(Movies.length, 0, "Movies.length");
assert.equals(Movies.recs.length, 0, "Movies.recs.length");
assert.equals(Movies.fields.length, 5+2, "Movies.fields.length");
assert.equals(Movies.joins.length, 2, "Movies.joins.length");
assert.equals(Movies.keys.length, 4, "People.keys.length");

// insert triggers
var PeopleAdd = 0, PeopleUpdate = 0;
People.addTrigger({
	onAdd: function (person) { 
		assert.exists(person, "onAdd: person");
		assert.exists(person.Name, "onAdd: person.Name");
		assert.exists(person.Gender, "onAdd: person.Gender");
		PeopleAdd = PeopleAdd + 1; 
	},
	onUpdate: function (person) { 
		assert.exists(person, "onAdd: person");
		assert.exists(person.Name, "onAdd: person.Name");
		assert.exists(person.Gender, "onAdd: person.Gender");
		PeopleUpdate = PeopleUpdate + 1; 
	}
});
var MoviesAdd = 0, MoviesUpdate = 0;
Movies.addTrigger({
	onAdd: function (movie) { 
		assert.exists(movie, "onAdd: movie");
		assert.exists(movie.Title, "onAdd: movie.Title");
		assert.exists(movie.Plot, "onAdd: movie.Plot");
		assert.exists(movie.Year, "onAdd: movie.Year");
		assert.exists(movie.Rating, "onAdd: movie.Rating");
		assert.exists(movie.Genres, "onAdd: movie.Genres");
		MoviesAdd = MoviesAdd + 1; 
	},
	onUpdate: function (movie) { 
		assert.exists(movie, "onAdd: movie");
		assert.exists(movie.Title, "onAdd: movie.Title");
		assert.exists(movie.Plot, "onAdd: movie.Plot");
		assert.exists(movie.Year, "onAdd: movie.Year");
		assert.exists(movie.Rating, "onAdd: movie.Rating");
		assert.exists(movie.Genres, "onAdd: movie.Genres");
		MoviesUpdate = MoviesUpdate + 1; 
	}
});

// insert a person
assert.equals(People.add({"Name": "Carolina Fortuna", "Gender": "Female"}), 0, "Person.add");
assert.equals(People.length, 1, "People.length");
assert.exists(People[0], "People[0]");
assert.equals(People[0].Name, "Carolina Fortuna", "People[0].Name");
assert.equals(People[0].Gender, "Female", "People[0].Gender");
// insert a person
assert.equals(People.add({"Name": "Blaz Fortuna", "Gender": "Male"}), 1, "Person.add");
assert.equals(People.length, 2, "People.length");
assert.exists(People[1], "People[1]");
assert.equals(People[1].Name, "Blaz Fortuna", "People[1].Name");
assert.equals(People[1].Gender, "Male", "People[1].Gender");
// insert existing person
assert.equals(People.add({"Name": "Blaz Fortuna", "Gender": "Male"}), 1, "Person.add");
assert.equals(People.length, 2, "People.length");

// insert a movie
var movie1 = {"Title":"Every Day", "Plot":"This day really isn't all that different than every other day. Except today, Ned's gay son Jonah wants to go to a college party, his wife is bringing home their elderly father to live with them, and his outrageous boss seems to have become even more crazy and demanding than would even seem possible. As his wife tries to take care of her father reconnect with him, Ned tries to reconnect with Jonah, and then without trying, he seems to have formed a connection with his co-worker. If he can get through days like these, he should be able to get through anything else life throws at him. Ned and Jeannie: married 19 years. Ned has trouble with Garrett, his boss at the cable show he writes, and he's ill-at-ease with his older son Jonah's coming out and wanting to go to a high-school gay student society prom. Jeannie puts work on hold while she attends to Ernie, her sour and mean-spirited father whose ill health forces him to move in with them. While Jeannie taxis the boys, goes to one son's recital, sees to her father's needs, and fixes meals, Garrett assigns Ned to rewrite a script with Robin, an uninhibited, unattached colleague who offers no-strings fun. Can this family hold together while a chicken hawk circles Jonah, Robin inveigles Ned, and death hunts Ernie?", "Year":2010, "Rating":5.6, "Genres":["Comedy", "Drama"], "Director":{"Name":"Levine Richard (III)", "Gender":"Unknown"}, "Actor":[{"Name":"Beetem Chris", "Gender":"Male"}, {"Name":"Carp Stan", "Gender":"Male"}, {"Name":"Chan Albert M.", "Gender":"Male"}, {"Name":"Dennehy Brian", "Gender":"Male"}, {"Name":"Durell Jesse", "Gender":"Male"}, {"Name":"Farcher Daniel", "Gender":"Male"}, {"Name":"Fortgang Skyler", "Gender":"Male"}, {"Name":"Harbour David (I)", "Gender":"Male"}, {"Name":"Ingram Michael H.", "Gender":"Male"}, {"Name":"Izzard Eddie", "Gender":"Male"}, {"Name":"James Kahan", "Gender":"Male"}, {"Name":"Jones Tilky", "Gender":"Male"}, {"Name":"Kempner Matt", "Gender":"Male"}, {"Name":"Miller Ezra", "Gender":"Male"}, {"Name":"Orchestra Black Diamond", "Gender":"Male"}, {"Name":"Riddle George", "Gender":"Male"}, {"Name":"Routman Steve", "Gender":"Male"}, {"Name":"Schreiber Liev", "Gender":"Male"}, {"Name":"Yelsky Daniel", "Gender":"Male"}, {"Name":"Gard Cassidy", "Gender":"Female"}, {"Name":"Giancoli Bianca", "Gender":"Female"}, {"Name":"Gugino Carla", "Gender":"Female"}, {"Name":"Hahn Sabrina", "Gender":"Female"}, {"Name":"Hunt Helen (I)", "Gender":"Female"}, {"Name":"Miller June (I)", "Gender":"Female"}, {"Name":"Robledo Benita", "Gender":"Female"}]};
assert.equals(Movies.add(movie1), 0, "Movies.add");
assert.equals(Movies.length, 1, "Movies.length");
assert.equals(People.length, 29, "People.length");
// check correctly asserted
assert.exists(Movies[0], "Movies[0]");
assert.equals(Movies[0].Title, movie1.Title, "Movies[0].Title");
assert.equals(Movies[0].Plot, movie1.Plot, "Movies[0].Plot");
assert.equals(Movies[0].Year, movie1.Year, "Movies[0].Year");
assert.equals(Movies[0].Genres.length, movie1.Genres.length, "Movies[0].Genres");
assert.equals(Movies[0].Director.Name, "Levine Richard (III)", "Movies[0].Director.Name");

// insert a movie
var movie2 = {"Title":"Enteng Kabisote 3: Okay ka fairy ko... The legend goes on and on and on", "Plot":"no plot available", "Year":2006, "Rating":5.8, "Genres":["Action", "Comedy", "Family", "Fantasy"], "Director":{"Name":"Reyes Tony Y.", "Gender":"Unknown"}, "Actor":[{"Name":"Aquitania Antonio", "Gender":"Male"}, {"Name":"Ballesteros Paolo", "Gender":"Male"}, {"Name":"Bayola Wally", "Gender":"Male"}, {"Name":"Casimiro Jr. Bayani", "Gender":"Male"}, {"Name":"de Leon Joey", "Gender":"Male"}, {"Name":"Forbes BJ", "Gender":"Male"}, {"Name":"Ignacio Levi", "Gender":"Male"}, {"Name":"K. Allan", "Gender":"Male"}, {"Name":"Lapid Jr. Jess", "Gender":"Male"}, {"Name":"Manalo Jose", "Gender":"Male"}, {"Name":"Salas Paul", "Gender":"Male"}, {"Name":"Santos Jimmy (I)", "Gender":"Male"}, {"Name":"Sotto Gian", "Gender":"Male"}, {"Name":"Sotto Oyo Boy", "Gender":"Male"}, {"Name":"Sotto Tito", "Gender":"Male"}, {"Name":"Sotto Vic", "Gender":"Male"}, {"Name":"V. Michael (I)", "Gender":"Male"}, {"Name":"Zamora Ramon", "Gender":"Male"}, {"Name":"Alano Alyssa", "Gender":"Female"}, {"Name":"Guanio Pia", "Gender":"Female"}, {"Name":"Hermosa Kristine", "Gender":"Female"}, {"Name":"Jones Angelica", "Gender":"Female"}, {"Name":"Loyzaga Bing", "Gender":"Female"}, {"Name":"Madrigal Ehra", "Gender":"Female"}, {"Name":"Parker J.C.", "Gender":"Female"}, {"Name":"Ponti Cassandra", "Gender":"Female"}, {"Name":"Ramirez Mikylla", "Gender":"Female"}, {"Name":"Rodriguez Ruby (I)", "Gender":"Female"}, {"Name":"Seguerra Aiza", "Gender":"Female"}, {"Name":"Sotto Ciara", "Gender":"Female"}, {"Name":"Toengi Giselle", "Gender":"Female"}, {"Name":"V. Ella", "Gender":"Female"}]};
assert.equals(Movies.add(movie2), 1, "Movies.add");
assert.equals(Movies.length, 2, "Movies.length");
assert.equals(People.length, 62, "People.length");
// check correctly inserted
assert.exists(Movies[1], "Movies[1]");
assert.equals(Movies[1].Title, movie2.Title, "Movies[1].Title");
assert.equals(Movies[1].Plot, movie2.Plot, "Movies[1].Plot");
assert.equals(Movies[1].Year, movie2.Year, "Movies[1].Year");
assert.equals(Movies[1].Genres.length, movie2.Genres.length, "Movies[1].Genres");
assert.equals(Movies[1].Director.Name, "Reyes Tony Y.", "Movies[1].Director.Name");

// stringify of test for record and record set
function testStringify(obj, name) {
	assert.exists(obj.toJSON(), name + ".toJSON()");
	//console.say(JSON.stringify(obj.toJSON()));
	assert.exists(obj.toJSON(true), name + ".toJSON(true)");
	//console.say(JSON.stringify(obj.toJSON(true)));
	assert.exists(obj.toJSON(true, true), name + ".toJSON(true, true)");
	//console.say(JSON.stringify(obj.toJSON(true, true)));
	assert.exists(JSON.stringify(obj), "JSON.stringify(" + name + ")");
	//console.say(JSON.stringify(obj));
}

testStringify(People[0], "People[0]");
testStringify(Movies[0], "Movies[0]");
testStringify(People.recs, "People.recs");
testStringify(Movies.recs, "Movies.recs");

// sample
assert.equals(People.sample(0).length, 0, "People.sample(0).length");
assert.equals(People.sample(10).length, 10, "People.sample(10).length");
assert.equals(People.sample(20).length, 20, "People.sample(20).length");
assert.equals(People.sample(62).length, 62, "People.sample(62).length");
assert.equals(People.sample(63).length, 62, "People.sample(63).length");
assert.equals(People.sample(100000).length, 62, "People.sample(100000).length");

// loading in larger dataset
var filename = "./sandbox/movies/movies_data.txt"
assert.equals(qm.load.jsonFile(Movies, filename), 167, "qm.load.jsonFile(Movies, filename)");

// check update and add counts
assert.equals(PeopleAdd, 3138, "PeopleAdd");
assert.equals(PeopleUpdate, 87, "PeopleUpdate");
assert.equals(MoviesAdd, 167, "MoviesAdd");
assert.equals(MoviesUpdate, 2, "MoviesUpdate");

// search
var queries = [
	{query: { $from: "People", Name: "john" }, records: 51},
	{query: { $from: "Movies", $or: [{ Title: "lost" }, { Plot: "lost" }]}, records: 2},
	{query: { $from: "Movies", Genres: "Horror", $or: [ { Title: "lost" }, { Plot: "lost" }]}, records: 1},
	{query: { $join: { $name: "Actor", $query: { $from: "Movies", Genres: "Horror", $or: [{ Title: "lost" }, { Plot: "lost" }]}}}, records: 38},
	{query: { $join: { $name: "Actor", $query: { $from: "Movies", Genres: "Horror", $or: [{ Title: "lost" }, { Plot: "lost" }]}}, Name: "Massey"}, records: 2}
];
var people_aggr = [
	{ name: "Gender", type: "count", field: "Gender" },
	{ name: "Name", type: "keywords", field: "Name" }
];
var movies_aggr = [
	{ name: "Year", type: "count", field: "Year" },
	{ name: "Title", type: "keywords", field: "Title" },
	{ name: "Plot", type: "keywords", field: "Plot" },
	{ name: "Rating", type: "histogram", field: "Rating" },
	{ name: "Genres", type: "count", field: "Genres" }
];
for (var i = 0; i < queries.length; i++) {
	var res = qm.search(queries[i].query);
	assert.exists(res, "qm.search(queries[" + i + "].query)");
	assert.equals(res.length, queries[i].records, JSON.stringify(queries[i].query));
	// check all records exist
	for (var j = 0; j < res.length; j++) {
		assert.exists(res[j], "qm.search(queries[" + i + "].query)[" + j + "]");
	}
	// joins
	if (res.store.name == "People") {
		assert.exists(res.join("ActedIn"), "res.join('ActedIn')");
		assert.exists(res.join("Directed"), "res.join('Directed')");
	} else if (res.store.name == "Movies") {
		assert.exists(res.join("Actor"), "res.join('Actor')");
		assert.exists(res.join("Director"), "res.join('Director')");	
	}
	// sample
	assert.equals(res.sample(100).length, queries[i].records, "res.sample(100)");
	assert.equals(res.sample(1).length, 1, "res.sample(1)");
	assert.equals(res.sample(0).length, 0, "res.sample(0)");
	// trunc
	var truncres = res.clone();
	truncres.trunc(100); assert.equals(truncres.length, queries[i].records, "truncres.trunc(100)");
	truncres.trunc(1); assert.equals(truncres.length, 1, "truncres.trunc(1)");
	truncres.trunc(0); assert.equals(truncres.length, 0, "truncres.trunc(0)");
	// reverse and shuffle
	assert.run(res.shuffle(), "res.shuffle()");
	assert.run(res.reverse(), "res.reverse()");
	// aggr
	if (res.store.name == "People") {
        console.say("People store");
		for (var j = 0; j < people_aggr.length; j++) {           
            console.say("TEST: res.aggr(" + JSON.stringify(people_aggr[j]) + ")");
			assert.exists(res.aggr(people_aggr[j]), "res.aggr(" + JSON.stringify(people_aggr[j]) + ")");
            console.say(JSON.stringify(res.aggr(people_aggr[j])));
		}
	} else if (res.store.name == "Movies") {
        console.say("Movies store");
		for (var j = 0; j < movies_aggr.length; j++) {
            console.say("TEST: res.aggr(" + JSON.stringify(movies_aggr[j]) + ")");
			assert.exists(res.aggr(movies_aggr[j]), "res.aggr(" + JSON.stringify(movies_aggr[j]) + ")");
			console.say(JSON.stringify(res.aggr(movies_aggr[j])));
		}
	}
	// sort by fq
	assert.run(res.sortByFq(1), "res.sortByFq(1)");
	for (var j = 1; j < res.length; j++) { 
		assert.ok(res[j-1].$fq <= res[j].$fq, "res[j-1].$fq <= res[j].$fq"); }
	assert.run(res.sortByFq(-1), "res.sortByFq(-1)");
	for (var j = 1; j < res.length; j++) { 
		assert.ok(res[j-1].$fq >= res[j].$fq, "res[j-1].$fq >= res[j].$fq"); }
	// sort by id
	assert.run(res.sortById(1), "res.sortById(1)");
	for (var j = 1; j < res.length; j++) { 
		assert.ok(res[j-1].$id <= res[j].$id, "res[j-1].$id <= res[j].$id"); }
	assert.run(res.sortById(-1), "res.sortById(-1)");
	for (var j = 1; j < res.length; j++) { 
		assert.ok(res[j-1].$id >= res[j].$id, "res[j-1].$id >= res[j].$id"); }
	// field sort
	if (res.store.name == "People") {
		// no sortable fields
	} else if (res.store.name == "Movies") {
		// rating (float)
		assert.run(res.sortByField("Rating", 1), 'res.sortByField("Rating")');
		for (var j = 1; j < res.length; j++) { 
			assert.ok(res[j-1].Rating <= res[j].Rating, "res[j-1].Rating <= res[j].Rating"); }
		assert.run(res.sortByField("Rating", -1), 'res.sortByField("Rating")');
		for (var j = 1; j < res.length; j++) { 
			assert.ok(res[j-1].Rating >= res[j].Rating, "res[j-1].Rating >= res[j].Rating"); }
   		// rating (javascript comparator)
		assert.run(res.sort(function (rec1, rec2) { return rec1.Rating < rec2.Rating; }), 'res.sort(function (rec1, rec2) { return rec1.Rating < rec2.Rating })');
		for (var j = 1; j < res.length; j++) { 
			assert.ok(res[j-1].Rating <= res[j].Rating, "res[j-1].Rating <= res[j].Rating"); }
		assert.run(res.sort(function (rec1, rec2) { return rec1.Rating > rec2.Rating; }), 'res.sort(function (rec1, rec2) { return rec1.Rating > rec2.Rating })');
		for (var j = 1; j < res.length; j++) { 
			assert.ok(res[j-1].Rating >= res[j].Rating, "res[j-1].Rating >= res[j].Rating"); }
		// year (int)
		assert.run(res.sortByField("Year", 1), 'res.sortByField("Year")');
		for (var j = 1; j < res.length; j++) { 
			assert.ok(res[j-1].Year <= res[j].Year, "res[j-1].Year <= res[j].Year"); }
		assert.run(res.sortByField("Year", -1), 'res.sortByField("Year")');
		for (var j = 1; j < res.length; j++) { 
			assert.ok(res[j-1].Year >= res[j].Year, "res[j-1].Year >= res[j].Year"); }
   		// year (javascript comparator)
		assert.run(res.sort(function (rec1, rec2) { return rec1.Year < rec2.Year; }), 'res.sort(function (rec1, rec2) { return rec1.Year < rec2.Year })');
		for (var j = 1; j < res.length; j++) { 
			assert.ok(res[j-1].Year <= res[j].Year, "res[j-1].Year <= res[j].Year"); }
		assert.run(res.sort(function (rec1, rec2) { return rec1.Year > rec2.Year; }), 'res.sort(function (rec1, rec2) { return rec1.Year > rec2.Year })');
		for (var j = 1; j < res.length; j++) { 
			assert.ok(res[j-1].Year >= res[j].Year, "res[j-1].Year >= res[j].Year"); }
   		// rating * year (javascript comparator)
		assert.run(res.sort(function (rec1, rec2) { return rec1.Year*rec1.Rating < rec2.Year*rec2.Rating; }), 'res.sort(function (rec1, rec2) { return rec1.Year*rec1.Rating < rec2.Year*rec2.Rating })');
		for (var j = 1; j < res.length; j++) { 
			assert.ok(res[j-1].Year*res[j-1].Rating <= res[j].Year*res[j].Rating, "res[j-1].Year*res[j-1].Rating <= res[j].Year*res[j].Rating"); }
		assert.run(res.sort(function (rec1, rec2) { return rec1.Year*rec1.Rating > rec2.Year*rec2.Rating; }), 'res.sort(function (rec1, rec2) { return rec1.Year*rec1.Rating > rec2.Year*rec2.Rating })');
		for (var j = 1; j < res.length; j++) { 
			assert.ok(res[j-1].Year*res[j-1].Rating >= res[j].Year*res[j].Rating, "res[j-1].Year*res[j-1].Rating >= res[j].Year*res[j].Rating"); }
    }
	// filter
	if (res.store.name == "People") {
        var filter = res.clone();
		assert.run(filter.filterByField("Gender", "Male"), 'filter.filterByField("Gender", "Male")');
		for (var j = 0; j < filter.length; j++) { 
			assert.ok(filter[j].Gender === "Male", 'filter[j].Gender === "Male"');
        }                
        filter = res.clone();
		assert.run(filter.filter(function(rec) { return rec.Gender === "Male";}), 'filter.filter(function(rec) { return rec.Gender === "Male";})');
		for (var j = 0; j < filter.length; j++) { 
			assert.ok(filter[j].Gender === "Male", 'filter[j].Gender === "Male"');
        }           
	} else if (res.store.name == "Movies") {
        var filter = res.clone();
		assert.run(filter.filterByField("Year", 2000, 2003), 'filter.filterByField("Year", 2000, 2003)');
		for (var j = 0; j < filter.length; j++) { 
			assert.ok(filter[j].Year >= 2000, "filter[j].Year >= 2000"); 
			assert.ok(filter[j].Year <= 2003, "filter[j].Year <= 2003"); 
        }       
        filter = res.clone();
		assert.run(filter.filterByField("Rating", 7.0, 9.0), 'filter.filterByField("Rating", 7.0, 9.0)');
		for (var j = 0; j < filter.length; j++) { 
			assert.ok(filter[j].Rating >= 7.0, "filter[j].Year >= 7.0"); 
			assert.ok(filter[j].Rating <= 9.0, "filter[j].Year <= 9.0"); 
        }     
	}
}

http.onGet("test1", function(req,res) { return "OK"; });
http.onGet("test2", function(req,res) { return "OK"; });
http.onPost("test3", function(req,res) { return "OK"; });