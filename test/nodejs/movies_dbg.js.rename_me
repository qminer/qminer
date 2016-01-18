/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js'); //adds assert.run function
var qm = require('qminer');


        qm.delLock();
        // add store.addTrigger method
        var backward = require('../../src/nodejs/scripts/backward.js');
        backward.addToProcess(process); // adds process.isArg function

        var base = new qm.Base({ mode: 'createClean' });


        //console.log("Movies", "Starting test based on IMDB sample");

        // only report failours
        //assert.silent = !process.isArg("-verbose");
        // name of the debug process
        //assert.consoleTitle = "Movies";

        // create stores
        base.createStore([{
            "name": "People",
            "fields": [
              { "name": "Name", "type": "string", "primary": true },
              { "name": "Gender", "type": "string", "shortstring": true }
            ],
            "joins": [
              { "name": "ActedIn", "type": "index", "store": "Movies", "inverse": "Actor" },
              { "name": "Directed", "type": "index", "store": "Movies", "inverse": "Director" }
            ],
            "keys": [
              { "field": "Name", "type": "text" },
              { "field": "Gender", "type": "value" }
            ]
          },
          {
              "name": "Movies",
              "fields": [
                { "name": "Title", "type": "string", "primary": true },
                { "name": "Plot", "type": "string", "store": "cache" },
                { "name": "Year", "type": "int" },
                { "name": "Rating", "type": "float" },
                { "name": "Genres", "type": "string_v", "codebook": true }
              ],
              "joins": [
                { "name": "Actor", "type": "index", "store": "People", "inverse": "ActedIn" },
                { "name": "Director", "type": "field", "store": "People", "inverse": "Directed" }
              ],
              "keys": [
                { "field": "Title", "type": "value" },
                { "field": "Title", "name": "TitleTxt", "type": "text", "vocabulary": "voc_01" },
                { "field": "Plot", "type": "text", "vocabulary": "voc_01" },
                { "field": "Genres", "type": "value" }
              ]
          }]);

        assert(null != base.store("People"), "Retrieve 'People' store");
        assert(null != base.store("Movies"), "Retrieve 'Movies' store");

        // test empty stores
        var People = base.store("People");
        assert.equal(People.name, "People", "People.name");
        assert(People.empty, "People.empty");
        assert.equal(People.length, 0, "People.length");
        assert.equal(People.allRecords.length, 0, "People.allRecords.length");
        assert.equal(People.fields.length, 2, "People.fields.length");
        assert.equal(People.joins.length, 2, "People.joins.length");
        assert.equal(People.keys.length, 2, "People.keys.length");
        assert.equal(People.forwardIter.next(), false, "People.forwardIter.next()");
        var Movies = base.store("Movies");
        assert.equal(Movies.name, "Movies", "Movies.name");
        assert(Movies.empty, "Movies.empty");
        assert.equal(Movies.length, 0, "Movies.length");
        assert.equal(Movies.allRecords.length, 0, "Movies.allRecords.length");
        assert.equal(Movies.fields.length, 5 + 2, "Movies.fields.length");
        assert.equal(Movies.joins.length, 2, "Movies.joins.length");
        assert.equal(Movies.keys.length, 4, "Movies.keys.length");
        assert.equal(Movies.forwardIter.next(), false, "Movies.forwardIter.next()");

        // insert triggers
        var PeopleAdd = 0, PeopleUpdate = 0;
        People.addTrigger({
            onAdd: function (person) {
                assert(null != person, "onAdd: person");
                assert(null != person.Name, "onAdd: person.Name");
                assert(null != person.Gender, "onAdd: person.Gender");
                PeopleAdd = PeopleAdd + 1;
            },
            onUpdate: function (person) {
                assert(null != person, "onAdd: person");
                assert(null != person.Name, "onAdd: person.Name");
                assert(null != person.Gender, "onAdd: person.Gender");
                PeopleUpdate = PeopleUpdate + 1;
            }
        });
        var MoviesAdd = 0, MoviesUpdate = 0;
        Movies.addTrigger({
            onAdd: function (movie) {
                assert(null != movie, "onAdd: movie");
                assert(null != movie.Title, "onAdd: movie.Title");
                assert(null != movie.Plot, "onAdd: movie.Plot");
                assert(null != movie.Year, "onAdd: movie.Year");
                assert(null != movie.Rating, "onAdd: movie.Rating");
                assert(null != movie.Genres, "onAdd: movie.Genres");
                MoviesAdd = MoviesAdd + 1;
            },
            onUpdate: function (movie) {
                assert(null != movie, "onAdd: movie");
                assert(null != movie.Title, "onAdd: movie.Title");
                assert(null != movie.Plot, "onAdd: movie.Plot");
                assert(null != movie.Year, "onAdd: movie.Year");
                assert(null != movie.Rating, "onAdd: movie.Rating");
                assert(null != movie.Genres, "onAdd: movie.Genres");
                MoviesUpdate = MoviesUpdate + 1;
            }
        });

        // insert a person
        assert.equal(People.push({ "Name": "Carolina Fortuna", "Gender": "Female" }), 0, "Person.add");
        assert.equal(People.length, 1, "People.length");
        assert(null != People[0], "People[0]");
        assert.equal(People[0].Name, "Carolina Fortuna", "People[0].Name");
        assert.equal(People[0].Gender, "Female", "People[0].Gender");
        // insert a person
        assert.equal(People.push({ "Name": "Blaz Fortuna", "Gender": "Male" }), 1, "Person.add");
        assert.equal(People.length, 2, "People.length");
        assert(null != People[1], "People[1]");
        assert.equal(People[1].Name, "Blaz Fortuna", "People[1].Name");
        assert.equal(People[1].Gender, "Male", "People[1].Gender");
        // insert existing person
        assert.equal(People.push({ "Name": "Blaz Fortuna", "Gender": "Male" }), 1, "Person.add");
        assert.equal(People.length, 2, "People.length");

        var PeopleIter = People.forwardIter;
        assert.equal(PeopleIter.next(), true, "People.forwardIter.next()");
        assert.equal(PeopleIter.record.$id, 0, "People.forwardIter.record.$id");
        assert.equal(PeopleIter.next(), true, "People.forwardIter.next()");
        assert.equal(PeopleIter.record.$id, 1, "People.forwardIter.record.$id");
        assert.equal(PeopleIter.next(), false, "People.forwardIter.next()");

        // insert a movie
        var movie1 = { "Title": "Every Day", "Plot": "This day really isn't all that different than every other day. Except today, Ned's gay son Jonah wants to go to a college party, his wife is bringing home their elderly father to live with them, and his outrageous boss seems to have become even more crazy and demanding than would even seem possible. As his wife tries to take care of her father reconnect with him, Ned tries to reconnect with Jonah, and then without trying, he seems to have formed a connection with his co-worker. If he can get through days like these, he should be able to get through anything else life throws at him. Ned and Jeannie: married 19 years. Ned has trouble with Garrett, his boss at the cable show he writes, and he's ill-at-ease with his older son Jonah's coming out and wanting to go to a high-school gay student society prom. Jeannie puts work on hold while she attends to Ernie, her sour and mean-spirited father whose ill health forces him to move in with them. While Jeannie taxis the boys, goes to one son's recital, sees to her father's needs, and fixes meals, Garrett assigns Ned to rewrite a script with Robin, an uninhibited, unattached colleague who offers no-strings fun. Can this family hold together while a chicken hawk circles Jonah, Robin inveigles Ned, and death hunts Ernie?", "Year": 2010, "Rating": 5.6, "Genres": ["Comedy", "Drama"], "Director": { "Name": "Levine Richard (III)", "Gender": "Unknown" }, "Actor": [{ "Name": "Beetem Chris", "Gender": "Male" }, { "Name": "Carp Stan", "Gender": "Male" }, { "Name": "Chan Albert M.", "Gender": "Male" }, { "Name": "Dennehy Brian", "Gender": "Male" }, { "Name": "Durell Jesse", "Gender": "Male" }, { "Name": "Farcher Daniel", "Gender": "Male" }, { "Name": "Fortgang Skyler", "Gender": "Male" }, { "Name": "Harbour David (I)", "Gender": "Male" }, { "Name": "Ingram Michael H.", "Gender": "Male" }, { "Name": "Izzard Eddie", "Gender": "Male" }, { "Name": "James Kahan", "Gender": "Male" }, { "Name": "Jones Tilky", "Gender": "Male" }, { "Name": "Kempner Matt", "Gender": "Male" }, { "Name": "Miller Ezra", "Gender": "Male" }, { "Name": "Orchestra Black Diamond", "Gender": "Male" }, { "Name": "Riddle George", "Gender": "Male" }, { "Name": "Routman Steve", "Gender": "Male" }, { "Name": "Schreiber Liev", "Gender": "Male" }, { "Name": "Yelsky Daniel", "Gender": "Male" }, { "Name": "Gard Cassidy", "Gender": "Female" }, { "Name": "Giancoli Bianca", "Gender": "Female" }, { "Name": "Gugino Carla", "Gender": "Female" }, { "Name": "Hahn Sabrina", "Gender": "Female" }, { "Name": "Hunt Helen (I)", "Gender": "Female" }, { "Name": "Miller June (I)", "Gender": "Female" }, { "Name": "Robledo Benita", "Gender": "Female" }] };
        assert.equal(Movies.push(movie1), 0, "Movies.add");
        assert.equal(Movies.length, 1, "Movies.length");
        assert.equal(People.length, 29, "People.length");
        // check correctly asserted
        assert(null != Movies[0], "Movies[0]");
        assert.equal(Movies[0].Title, movie1.Title, "Movies[0].Title");
        assert.equal(Movies[0].Plot, movie1.Plot, "Movies[0].Plot");
        assert.equal(Movies[0].Year, movie1.Year, "Movies[0].Year");
        assert.equal(Movies[0].Genres.length, movie1.Genres.length, "Movies[0].Genres");
        assert.equal(Movies[0].Director.Name, "Levine Richard (III)", "Movies[0].Director.Name");

        var MoviesIter = Movies.forwardIter;
        assert.equal(MoviesIter.next(), true, "Movies.forwardIter.next()");
        assert.equal(MoviesIter.record.$id, 0, "Movies.forwardIter.record.$id");
        assert.equal(MoviesIter.next(), false, "Movies.forwardIter.next()");

        // insert a movie
        var movie2 = { "Title": "Enteng Kabisote 3: Okay ka fairy ko... The legend goes on and on and on", "Plot": "no plot available", "Year": 2006, "Rating": 5.8, "Genres": ["Action", "Comedy", "Family", "Fantasy"], "Director": { "Name": "Reyes Tony Y.", "Gender": "Unknown" }, "Actor": [{ "Name": "Aquitania Antonio", "Gender": "Male" }, { "Name": "Ballesteros Paolo", "Gender": "Male" }, { "Name": "Bayola Wally", "Gender": "Male" }, { "Name": "Casimiro Jr. Bayani", "Gender": "Male" }, { "Name": "de Leon Joey", "Gender": "Male" }, { "Name": "Forbes BJ", "Gender": "Male" }, { "Name": "Ignacio Levi", "Gender": "Male" }, { "Name": "K. Allan", "Gender": "Male" }, { "Name": "Lapid Jr. Jess", "Gender": "Male" }, { "Name": "Manalo Jose", "Gender": "Male" }, { "Name": "Salas Paul", "Gender": "Male" }, { "Name": "Santos Jimmy (I)", "Gender": "Male" }, { "Name": "Sotto Gian", "Gender": "Male" }, { "Name": "Sotto Oyo Boy", "Gender": "Male" }, { "Name": "Sotto Tito", "Gender": "Male" }, { "Name": "Sotto Vic", "Gender": "Male" }, { "Name": "V. Michael (I)", "Gender": "Male" }, { "Name": "Zamora Ramon", "Gender": "Male" }, { "Name": "Alano Alyssa", "Gender": "Female" }, { "Name": "Guanio Pia", "Gender": "Female" }, { "Name": "Hermosa Kristine", "Gender": "Female" }, { "Name": "Jones Angelica", "Gender": "Female" }, { "Name": "Loyzaga Bing", "Gender": "Female" }, { "Name": "Madrigal Ehra", "Gender": "Female" }, { "Name": "Parker J.C.", "Gender": "Female" }, { "Name": "Ponti Cassandra", "Gender": "Female" }, { "Name": "Ramirez Mikylla", "Gender": "Female" }, { "Name": "Rodriguez Ruby (I)", "Gender": "Female" }, { "Name": "Seguerra Aiza", "Gender": "Female" }, { "Name": "Sotto Ciara", "Gender": "Female" }, { "Name": "Toengi Giselle", "Gender": "Female" }, { "Name": "V. Ella", "Gender": "Female" }] };
        assert.equal(Movies.push(movie2), 1, "Movies.add");
        assert.equal(Movies.length, 2, "Movies.length");
        assert.equal(People.length, 62, "People.length");
        // check correctly inserted
        assert(null != Movies[1], "Movies[1]");
        assert.equal(Movies[1].Title, movie2.Title, "Movies[1].Title");
        assert.equal(Movies[1].Plot, movie2.Plot, "Movies[1].Plot");
        assert.equal(Movies[1].Year, movie2.Year, "Movies[1].Year");
        assert.equal(Movies[1].Genres.length, movie2.Genres.length, "Movies[1].Genres");
        assert.equal(Movies[1].Director.Name, "Reyes Tony Y.", "Movies[1].Director.Name");

        // stringify of test for record and record set
        function testStringify(obj, name) {
            assert(null != obj.toJSON(), name + ".toJSON()");
            //console.log(JSON.stringify(obj.toJSON()));
            assert(null != obj.toJSON(true), name + ".toJSON(true)");
            //console.log(JSON.stringify(obj.toJSON(true)));
            assert(null != obj.toJSON(true, true), name + ".toJSON(true, true)");
            //console.log(JSON.stringify(obj.toJSON(true, true)));
            assert(null != JSON.stringify(obj), "JSON.stringify(" + name + ")");
            //console.log(JSON.stringify(obj));
        }

        testStringify(People[0], "People[0]");
        testStringify(Movies[0], "Movies[0]");
        testStringify(People.allRecords, "People.allRecords");
        testStringify(Movies.allRecords, "Movies.allRecords");

        // sample
        assert.equal(People.sample(0).length, 0, "People.sample(0).length");
        assert.equal(People.sample(10).length, 10, "People.sample(10).length");
        assert.equal(People.sample(20).length, 20, "People.sample(20).length");
        assert.equal(People.sample(62).length, 62, "People.sample(62).length");
        assert.equal(People.sample(63).length, 62, "People.sample(63).length");
        assert.equal(People.sample(100000).length, 62, "People.sample(100000).length");

        // loading in larger dataset
        var filename = "./sandbox/movies/movies_data.txt"
        assert.equal(Movies.loadJson(filename), 167, "Movies.loadJson(Movies, filename)");

        // check update and add counts
        assert.equal(PeopleAdd, 3138, "PeopleAdd");
        assert.equal(PeopleUpdate, 87, "PeopleUpdate");
        assert.equal(MoviesAdd, 167, "MoviesAdd");
        assert.equal(MoviesUpdate, 2, "MoviesUpdate");

        // search
        var queries = [
            { query: { $from: "People", Name: "john" }, records: 49 },
            { query: { $from: "Movies", $or: [{ Title: "lost" }, { Plot: "lost" }] }, records: 2 },
            { query: { $from: "Movies", Genres: "Horror", $or: [{ Title: "lost" }, { Plot: "lost" }] }, records: 1 },
            { query: { $join: { $name: "Actor", $query: { $from: "Movies", Genres: "Horror", $or: [{ Title: "lost" }, { Plot: "lost" }] } } }, records: 38 },
            { query: { $join: { $name: "Actor", $query: { $from: "Movies", Genres: "Horror", $or: [{ Title: "lost" }, { Plot: "lost" }] } }, Name: "Massey" }, records: 2 }
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
            var res = base.search(queries[i].query);
            assert(null != res, "base.search(queries[" + i + "].query)");
            assert.equal(res.length, queries[i].records, JSON.stringify(queries[i].query));
            // check all records exist
            for (var j = 0; j < res.length; j++) {
                assert(null != res[j], "base.search(queries[" + i + "].query)[" + j + "]");
            }
            // joins
            if (res.store.name == "People") {
                assert(null != res.join("ActedIn"), "res.join('ActedIn')");
                assert(null != res.join("Directed"), "res.join('Directed')");
            } else if (res.store.name == "Movies") {
                assert(null != res.join("Actor"), "res.join('Actor')");
                assert(null != res.join("Director"), "res.join('Director')");
            }
            // sample
            assert.equal(res.sample(100).length, queries[i].records, "res.sample(100)");
            assert.equal(res.sample(1).length, 1, "res.sample(1)");
            assert.equal(res.sample(0).length, 0, "res.sample(0)");
            // trunc
            var truncres = res.clone();
            truncres.trunc(100); assert.equal(truncres.length, queries[i].records, "truncres.trunc(100)");
            truncres.trunc(1); assert.equal(truncres.length, 1, "truncres.trunc(1)");
            truncres.trunc(0); assert.equal(truncres.length, 0, "truncres.trunc(0)");
            // reverse and shuffle
            assert.run(res.shuffle(), "res.shuffle()");
            assert.run(res.reverse(), "res.reverse()");
            // aggr
            if (res.store.name == "People") {
                //console.log("People store");
                for (var j = 0; j < people_aggr.length; j++) {
                    //console.log("TEST: res.aggr(" + JSON.stringify(people_aggr[j]) + ")");
                    assert(null != res.aggr(people_aggr[j]), "res.aggr(" + JSON.stringify(people_aggr[j]) + ")");
                    //console.log(JSON.stringify(res.aggr(people_aggr[j])));
                }
            } else if (res.store.name == "Movies") {
                //console.log("Movies store");
                for (var j = 0; j < movies_aggr.length; j++) {
                    //console.log("TEST: res.aggr(" + JSON.stringify(movies_aggr[j]) + ")");
                    assert(null != res.aggr(movies_aggr[j]), "res.aggr(" + JSON.stringify(movies_aggr[j]) + ")");
                    //console.log(JSON.stringify(res.aggr(movies_aggr[j])));
                }
            }
            // sort by fq
            assert.run(res.sortByFq(1), "res.sortByFq(1)");
            for (var j = 1; j < res.length; j++) {
                assert(res[j - 1].$fq <= res[j].$fq, "res[j-1].$fq <= res[j].$fq");
            }
            assert.run(res.sortByFq(-1), "res.sortByFq(-1)");
            for (var j = 1; j < res.length; j++) {
                assert(res[j - 1].$fq >= res[j].$fq, "res[j-1].$fq >= res[j].$fq");
            }
            // sort by id
            assert.run(res.sortById(1), "res.sortById(1)");
            for (var j = 1; j < res.length; j++) {
                assert(res[j - 1].$id <= res[j].$id, "res[j-1].$id <= res[j].$id");
            }
            assert.run(res.sortById(-1), "res.sortById(-1)");
            for (var j = 1; j < res.length; j++) {
                assert(res[j - 1].$id >= res[j].$id, "res[j-1].$id >= res[j].$id");
            }
            // field sort
            if (res.store.name == "People") {
                // no sortable fields
            } else if (res.store.name == "Movies") {
                // rating (float)
                assert.run(res.sortByField("Rating", 1), 'res.sortByField("Rating")');
                for (var j = 1; j < res.length; j++) {
                    assert(res[j - 1].Rating <= res[j].Rating, "res[j-1].Rating <= res[j].Rating");
                }
                assert.run(res.sortByField("Rating", -1), 'res.sortByField("Rating")');
                for (var j = 1; j < res.length; j++) {
                    assert(res[j - 1].Rating >= res[j].Rating, "res[j-1].Rating >= res[j].Rating");
                }
                // rating (javascript comparator)
                assert.run(res.sort(function (rec1, rec2) { return rec1.Rating < rec2.Rating; }), 'res.sort(function (rec1, rec2) { return rec1.Rating < rec2.Rating })');
                for (var j = 1; j < res.length; j++) {
                    assert(res[j - 1].Rating <= res[j].Rating, "res[j-1].Rating <= res[j].Rating");
                }
                assert.run(res.sort(function (rec1, rec2) { return rec1.Rating > rec2.Rating; }), 'res.sort(function (rec1, rec2) { return rec1.Rating > rec2.Rating })');
                for (var j = 1; j < res.length; j++) {
                    assert(res[j - 1].Rating >= res[j].Rating, "res[j-1].Rating >= res[j].Rating");
                }
                // year (int)
                assert.run(res.sortByField("Year", 1), 'res.sortByField("Year")');
                for (var j = 1; j < res.length; j++) {
                    assert(res[j - 1].Year <= res[j].Year, "res[j-1].Year <= res[j].Year");
                }
                assert.run(res.sortByField("Year", -1), 'res.sortByField("Year")');
                for (var j = 1; j < res.length; j++) {
                    assert(res[j - 1].Year >= res[j].Year, "res[j-1].Year >= res[j].Year");
                }
                // year (javascript comparator)
                assert.run(res.sort(function (rec1, rec2) { return rec1.Year < rec2.Year; }), 'res.sort(function (rec1, rec2) { return rec1.Year < rec2.Year })');
                for (var j = 1; j < res.length; j++) {
                    assert(res[j - 1].Year <= res[j].Year, "res[j-1].Year <= res[j].Year");
                }
                assert.run(res.sort(function (rec1, rec2) { return rec1.Year > rec2.Year; }), 'res.sort(function (rec1, rec2) { return rec1.Year > rec2.Year })');
                for (var j = 1; j < res.length; j++) {
                    assert(res[j - 1].Year >= res[j].Year, "res[j-1].Year >= res[j].Year");
                }
                // rating * year (javascript comparator)
                assert.run(res.sort(function (rec1, rec2) { return rec1.Year * rec1.Rating < rec2.Year * rec2.Rating; }), 'res.sort(function (rec1, rec2) { return rec1.Year*rec1.Rating < rec2.Year*rec2.Rating })');
                for (var j = 1; j < res.length; j++) {
                    assert(res[j - 1].Year * res[j - 1].Rating <= res[j].Year * res[j].Rating, "res[j-1].Year*res[j-1].Rating <= res[j].Year*res[j].Rating");
                }
                assert.run(res.sort(function (rec1, rec2) { return rec1.Year * rec1.Rating > rec2.Year * rec2.Rating; }), 'res.sort(function (rec1, rec2) { return rec1.Year*rec1.Rating > rec2.Year*rec2.Rating })');
                for (var j = 1; j < res.length; j++) {
                    assert(res[j - 1].Year * res[j - 1].Rating >= res[j].Year * res[j].Rating, "res[j-1].Year*res[j-1].Rating >= res[j].Year*res[j].Rating");
                }
            }
            // filter
            if (res.store.name == "People") {
                var filter = res.clone();
                assert.run(filter.filterByField("Gender", "Male"), 'filter.filterByField("Gender", "Male")');
                for (var j = 0; j < filter.length; j++) {
                    assert(filter[j].Gender === "Male", 'filter[j].Gender === "Male"');
                }
                filter = res.clone();
                assert.run(filter.filter(function (rec) { return rec.Gender === "Male"; }), 'filter.filter(function(rec) { return rec.Gender === "Male";})');
                for (var j = 0; j < filter.length; j++) {
                    assert(filter[j].Gender === "Male", 'filter[j].Gender === "Male"');
                }
            } else if (res.store.name == "Movies") {
                var filter = res.clone();
                assert.run(filter.filterByField("Year", 2000, 2003), 'filter.filterByField("Year", 2000, 2003)');
                for (var j = 0; j < filter.length; j++) {
                    assert(filter[j].Year >= 2000, "filter[j].Year >= 2000");
                    assert(filter[j].Year <= 2003, "filter[j].Year <= 2003");
                }
                filter = res.clone();
                assert.run(filter.filterByField("Rating", 7.0, 9.0), 'filter.filterByField("Rating", 7.0, 9.0)');
                for (var j = 0; j < filter.length; j++) {
                    assert(filter[j].Rating >= 7.0, "filter[j].Year >= 7.0");
                    assert(filter[j].Rating <= 9.0, "filter[j].Year <= 9.0");
                }
            }
        }

        // test forward iterator
        var moviesIter = Movies.forwardIter;
        var moviesCount = 0;
        while (moviesIter.next()) {
            assert.equal(moviesCount, moviesIter.record.$id, "moviesCount == moviesIter.record.$id");
            moviesCount++;
        }
        assert.equal(moviesCount, Movies.length, "moviesCount = Movies.length");
        // test backward iterator
        var moviesIter = Movies.backwardIter;
        while (moviesIter.next()) {
            moviesCount--;
            assert.equal(moviesCount, moviesIter.record.$id, "moviesCount == moviesIter.record.$id");
        }
        assert.equal(moviesCount, 0, "moviesCount = 0");
        // test first record
        assert.equal(Movies.first.$id, 0, "Movies.first.$id");
        // test last record
        assert.equal(Movies.last.$id, Movies.length - 1, "Movies.last.$id");

        base.close();

        //})});
