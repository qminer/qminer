/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js');     //adds assert.run function
var qm = require('qminer');
var fs = qm.fs;

///////////////////////////////////////////////////////////////////////////////
// Empty Store

function EmptyStore() {
    // creates the base and store
    this.base = new qm.Base({ mode: 'createClean' });
    this.base.createStore({
        "name": "People",
        "fields": [
            { "name": "Name", "type": "string", "primary": true },
            { "name": "Gender", "type": "string", "shortstring": true }
        ]
    });

    // closes the store
    this.close = function () {
        this.base.close();
    }
}


describe('Empty Store Tests', function () {
    var table = undefined;
    beforeEach(function () {
        table = new EmptyStore();
    });
    afterEach(function () {
        table.close();
    });

    describe('Created Test', function () {
        it('should return true if store is not null', function () {
            assert(null != table.base.store("People"));
        })
    });
    describe('Name Test', function () {
        it('should return "People" as store name', function () {
            assert.equal(table.base.store("People").name, "People");
        })
    });

    describe('Length Test', function () {
        it('should return 0', function () {
            assert.equal(table.base.store("People").length, 0);
        })
    });

    describe('Recs Test', function () {
        it('should return a json with no elements', function () {
            var recs = table.base.store("People").allRecords;
            assert(recs.empty);
            assert.equal(recs.length, 0);
        })
    });

    describe('Empty Test', function () {
        it('should return true for an empty store', function () {
            assert(table.base.store("People").empty);
        })
    })

    describe('Fields Test', function () {
        it('should return the number of fields', function () {
            var arr = table.base.store("People").fields;
            assert.equal(arr.length, 2);
        })
        it('should return an array of all Fields', function () {
            var arr = table.base.store("People").fields;
            assert.equal(arr[0].name, "Name");
            assert.equal(arr[1].name, "Gender");
        })
        it('should return the details of the given field', function () {
            var detail = table.base.store("People").field("Name");
            assert.equal(detail.id, 0);
            assert.equal(detail.name, "Name");
            assert.equal(detail.type, "string");
            assert.equal(detail.primary, true);

            var detail2 = table.base.store("People").field("Gender");
            assert.equal(detail2.id, 1);
            assert.equal(detail2.name, "Gender");
            assert.equal(detail2.type, "string");
            assert.equal(detail2.primary, false);
        })
    });

    describe('Joins Test', function () {
        it('should return an empty array', function () {
            var arr = table.base.store("People").joins;
            assert.deepEqual(arr, []);
        })
    });

    describe('Keys Test', function () {
        it('should return an empty array', function () {
            var arr = table.base.store("People").joins;
            assert.deepEqual(arr, []);
        })
    });

    describe('First Test', function () {
        it('should return a null object', function () {
            var rec = table.base.store("People").first;
            assert(rec == null);
        })
    });

    describe('Last Test', function () {
        it('should return a null object', function () {
            var rec = table.base.store("People").last;
            assert(rec == null);
        })
    });

    describe('ForwardIter Test', function () {
        it('should return an empty json', function () {
            var iter = table.base.store("People").forwardIter.next();
            assert(!iter);
        })
    });

    describe('BackwardIter Test', function () {
        it('should return an empty json', function () {
            var iter = table.base.store("People").backwardIter;
            assert.equal(iter.store.length, 0);
        })
    });

    describe('Add Test', function () {
        it('should add a new person in store', function () {
            assert.equal(table.base.store("People").push({ "Name": "Carolina Fortuna", "Gender": "Female" }), 0);
            assert.equal(table.base.store("People").length, 1);
        })
    });
})

///////////////////////////////////////////////////////////////////////////////
// Small Store

function Store() {
    this.base = new qm.Base({ mode: 'createClean' });
    this.base.createStore({
        "name": "People",
        "fields": [
            { "name": "Name", "type": "string", "primary": true },
            { "name": "Gender", "type": "string", "shortstring": true }
        ]
    });
    this.base.store("People").push({ "Name": "Carolina Fortuna", "Gender": "Female" });
    this.base.store("People").push({ "Name": "Blaz Fortuna", "Gender": "Male" });

    this.close = function () {
        this.base.close();
    }
};

describe('Store Tests', function () {
    var table = undefined;
    beforeEach(function () {
        table = new Store();
    });
    afterEach(function () {
        table.close();
    });

    describe('Length Test', function () {
        it('should return the number of persons in store', function () {
            assert.equal(table.base.store("People").length, 2);
        })
    });

    describe('Name Test', function () {
        it('should return the name of both persons', function () {
            assert.equal(table.base.store("People")[0].Name, "Carolina Fortuna");
            assert.equal(table.base.store("People")[1].Name, "Blaz Fortuna");
        })
    });

    describe('Gender Test', function () {
        it('should return the genders of both persons', function () {
            assert.equal(table.base.store("People")[0].Gender, "Female");
            assert.equal(table.base.store("People")[1].Gender, "Male");
        })
    });

    describe('Update Test', function () {
        it('should update the existing person', function () {
            table.base.store("People").push({ "Name": "Blaz Fortuna", "Gender": "Male" });
            assert.equal(table.base.store("People").length, 2);

        })
    })

    describe('ForwardIter Test', function () {
        it('should go through the persons in store', function () {
            var PeopleIter = table.base.store("People").forwardIter;
            assert.equal(PeopleIter.next(), true);
            assert.equal(PeopleIter.record.$id, 0);
            assert.equal(PeopleIter.next(), true);
            assert.equal(PeopleIter.record.$id, 1);
            assert.equal(PeopleIter.next(), false);
        })
    });

    describe('Base Test', function () {
        it('should return the base, in which the store is contained', function () {
            var base = table.base.store("People").base;
            assert.equal(base.store("People").name, "People");
        })
    });

    describe('IndexId Test', function () {
        it('should return the record at index 0: Carolina Fortuna', function () {
            assert.equal(table.base.store("People")[0].Name, "Carolina Fortuna");
        })
        it('should return null if index is out of bound, lesser than 0', function () {
            assert.ok(table.base.store("People")[-1] == null);
        })
        it('should return null if index is out of bound, greater than number of records', function () {
            assert.ok(table.base.store("People")[3] == null);
        })
    })

    describe('Rec Test', function () {
        it('should return the record of Carolina Fortuna', function () {
            var record = table.base.store("People").recordByName("Carolina Fortuna");
            assert.equal(record.Name, "Carolina Fortuna");
            assert.equal(record.Gender, "Female");
        })
        it('should return the record of Blaz Fortuna', function () {
            var record = table.base.store("People").recordByName("Blaz Fortuna");
            assert.equal(record.Name, "Blaz Fortuna");
            assert.equal(record.Gender, "Male");
        })
        it('should return null if record not found', function () {
            var record = table.base.store("People").recordByName("Bender Bending Rodriguez");
            assert(record == null);
        })
    });

    describe('Each Test', function () {
        it('should change the gender of all records to "Extraterrestrial"', function () {
            table.base.store("People").each(function (rec) { rec.Gender = "Extraterrestrial" });
            assert.equal(table.base.store("People")[0].Gender, "Extraterrestrial");
            assert.equal(table.base.store("People")[1].Gender, "Extraterrestrial");
        })
        it('should change the gender only of Blaz Fortuna to "Extraterrestrial"', function () {
            table.base.store("People").each(function (rec, idx) { if (idx == 1) { rec.Gender = "Extraterrestrial" } });
            assert.equal(table.base.store("People")[0].Gender, "Female");
            assert.equal(table.base.store("People")[1].Gender, "Extraterrestrial");
        })
        it('shouldn\'t make a new field', function () {
            table.base.store("People").each(function (rec) { rec.DateOfBirth = "1.1.2015" });
            assert(table.base.store("People")[0].DateOfBirth == null);
            assert(table.base.store("People")[1].DateOfBirth == null);
        })
    });

    describe('Map Test', function () {
        it('should return an array of names', function () {
            var arr = table.base.store("People").map(function (rec) { return rec.Name });
            assert.equal(arr[0], "Carolina Fortuna");
            assert.equal(arr[1], "Blaz Fortuna");
        })
        it('should return a nested array ["Carolina Fortuna", ["Blaz Fortuna", "Male"]]', function () {
            var arr = table.base.store("People").map(function (rec, idx) {
                if (idx == 0) {
                    return rec.Name;
                }
                else if (idx == 1) {
                    return [rec.Name, rec.Gender];
                }
            });
            assert.equal(arr[0], "Carolina Fortuna");
            assert.equal(arr[1][0], "Blaz Fortuna");
            assert.equal(arr[1][1], "Male");
        })
    });

    describe('Clear Test', function () {
        it('should clear all records in "People" store', function () {
            assert.equal(table.base.store("People").clear(), 0);
        })
        it('should clear only the first record in "People" store', function () {
            assert.equal(table.base.store("People").clear(1), 1);
            assert(table.base.store("People")[0] == null);
            assert.equal(table.base.store("People")[1].Name, "Blaz Fortuna");
        })
        it('should clear all the record in "People store"', function () {
            assert.equal(table.base.store("People").clear(10), 0);
        })
    });

    describe('GetVector Test', function () {
        it('should return the vector of record names in "People" store', function () {
            var vec = table.base.store("People").getVector("Name");
            assert.equal(vec[0], "Carolina Fortuna");
            assert.equal(vec[1], "Blaz Fortuna");
        })
        it('should return the vector of record genders in "People" store', function () {
            var vec = table.base.store("People").getVector("Gender");
            assert.equal(vec[0], "Female");
            assert.equal(vec[1], "Male");
        })
        it('should throw an exception if parameter isn\'t given', function () {
            assert.throws(function () {
                table.base.store("People").getVector();
            })
        })
        it('should throw an exception if parameter isn\'t a field', function () {
            assert.throws(function () {
                table.base.store("People").getVector("Payday");
            })
        })
    });

    describe('getStats Test', function () {
        it('should return stats', function () {
            var stats = table.base.getStats();
            assert.equal(stats.stores.length, 1);
        })
    });

})

///////////////////////////////////////////////////////////////////////////////
// Two Stores

function TStore() {
    this.base = new qm.Base({ mode: 'createClean' });
	this.base.createStore([{
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
    },
    {
        "name": "Basketball",
        "fields": [
            { "name": "Player", "type": "string", "primary": true },
            { "name": "Score", "type": "float_v" },
            { "name": "FirstPlayed", "type": "datetime" }
        ]
    }]);
    // adding two persons
    this.base.store("People").push({ "Name": "Carolina Fortuna", "Gender": "Female" });
    this.base.store("People").push({ "Name": "Blaz Fortuna", "Gender": "Male" });

    // adding two movies
    this.movie = { "Title": "Every Day", "Plot": "This day really isn't all that different than every other day. Except today, Ned's gay son Jonah wants to go to a college party, his wife is bringing home their elderly father to live with them, and his outrageous boss seems to have become even more crazy and demanding than would even seem possible. As his wife tries to take care of her father reconnect with him, Ned tries to reconnect with Jonah, and then without trying, he seems to have formed a connection with his co-worker. If he can get through days like these, he should be able to get through anything else life throws at him. Ned and Jeannie: married 19 years. Ned has trouble with Garrett, his boss at the cable show he writes, and he's ill-at-ease with his older son Jonah's coming out and wanting to go to a high-school gay student society prom. Jeannie puts work on hold while she attends to Ernie, her sour and mean-spirited father whose ill health forces him to move in with them. While Jeannie taxis the boys, goes to one son's recital, sees to her father's needs, and fixes meals, Garrett assigns Ned to rewrite a script with Robin, an uninhibited, unattached colleague who offers no-strings fun. Can this family hold together while a chicken hawk circles Jonah, Robin inveigles Ned, and death hunts Ernie?", "Year": 2010, "Rating": 5.6, "Genres": ["Comedy", "Drama"], "Director": { "Name": "Levine Richard (III)", "Gender": "Unknown" }, "Actor": [{ "Name": "Beetem Chris", "Gender": "Male" }, { "Name": "Carp Stan", "Gender": "Male" }, { "Name": "Chan Albert M.", "Gender": "Male" }, { "Name": "Dennehy Brian", "Gender": "Male" }, { "Name": "Durell Jesse", "Gender": "Male" }, { "Name": "Farcher Daniel", "Gender": "Male" }, { "Name": "Fortgang Skyler", "Gender": "Male" }, { "Name": "Harbour David (I)", "Gender": "Male" }, { "Name": "Ingram Michael H.", "Gender": "Male" }, { "Name": "Izzard Eddie", "Gender": "Male" }, { "Name": "James Kahan", "Gender": "Male" }, { "Name": "Jones Tilky", "Gender": "Male" }, { "Name": "Kempner Matt", "Gender": "Male" }, { "Name": "Miller Ezra", "Gender": "Male" }, { "Name": "Orchestra Black Diamond", "Gender": "Male" }, { "Name": "Riddle George", "Gender": "Male" }, { "Name": "Routman Steve", "Gender": "Male" }, { "Name": "Schreiber Liev", "Gender": "Male" }, { "Name": "Yelsky Daniel", "Gender": "Male" }, { "Name": "Gard Cassidy", "Gender": "Female" }, { "Name": "Giancoli Bianca", "Gender": "Female" }, { "Name": "Gugino Carla", "Gender": "Female" }, { "Name": "Hahn Sabrina", "Gender": "Female" }, { "Name": "Hunt Helen (I)", "Gender": "Female" }, { "Name": "Miller June (I)", "Gender": "Female" }, { "Name": "Robledo Benita", "Gender": "Female" }] };
    this.movie2 = { "Title": "Enteng Kabisote 3: Okay ka fairy ko... The legend goes on and on and on", "Plot": "no plot available", "Year": 2006, "Rating": 5.8, "Genres": ["Action", "Comedy", "Family", "Fantasy"], "Director": { "Name": "Reyes Tony Y.", "Gender": "Unknown" }, "Actor": [{ "Name": "Aquitania Antonio", "Gender": "Male" }, { "Name": "Ballesteros Paolo", "Gender": "Male" }, { "Name": "Bayola Wally", "Gender": "Male" }, { "Name": "Casimiro Jr. Bayani", "Gender": "Male" }, { "Name": "de Leon Joey", "Gender": "Male" }, { "Name": "Forbes BJ", "Gender": "Male" }, { "Name": "Ignacio Levi", "Gender": "Male" }, { "Name": "K. Allan", "Gender": "Male" }, { "Name": "Lapid Jr. Jess", "Gender": "Male" }, { "Name": "Manalo Jose", "Gender": "Male" }, { "Name": "Salas Paul", "Gender": "Male" }, { "Name": "Santos Jimmy (I)", "Gender": "Male" }, { "Name": "Sotto Gian", "Gender": "Male" }, { "Name": "Sotto Oyo Boy", "Gender": "Male" }, { "Name": "Sotto Tito", "Gender": "Male" }, { "Name": "Sotto Vic", "Gender": "Male" }, { "Name": "V. Michael (I)", "Gender": "Male" }, { "Name": "Zamora Ramon", "Gender": "Male" }, { "Name": "Alano Alyssa", "Gender": "Female" }, { "Name": "Guanio Pia", "Gender": "Female" }, { "Name": "Hermosa Kristine", "Gender": "Female" }, { "Name": "Jones Angelica", "Gender": "Female" }, { "Name": "Loyzaga Bing", "Gender": "Female" }, { "Name": "Madrigal Ehra", "Gender": "Female" }, { "Name": "Parker J.C.", "Gender": "Female" }, { "Name": "Ponti Cassandra", "Gender": "Female" }, { "Name": "Ramirez Mikylla", "Gender": "Female" }, { "Name": "Rodriguez Ruby (I)", "Gender": "Female" }, { "Name": "Seguerra Aiza", "Gender": "Female" }, { "Name": "Sotto Ciara", "Gender": "Female" }, { "Name": "Toengi Giselle", "Gender": "Female" }, { "Name": "V. Ella", "Gender": "Female" }] };

    this.addMovie = function (movie) {
        this.base.store("Movies").push(movie);
    }

    this.player1 = { "Player": "Goran Dragic", "Score": [35, 12, 23], "FirstPlayed": "1984-01-01T00:00:00" };
    this.player2 = { "Player": "Michael Jordan", "Score": [90, 100, 95], "FirstPlayed": "2003-01-01T00:00:00" };
    this.player3 = { "Player": "Marko Milic", "Score": [50, 10, 10, 12], "FirstPlayed": "1991-01-01T00:00:00" };

    this.addPlayer = function (player) {
        this.base.store("Basketball").push(player);
    }

    this.close = function () {
        this.base.close();
    }
};

describe("Two Store Tests", function () {
    var table = undefined;
    beforeEach(function () {
        table = new TStore();
    });
    afterEach(function () {
        table.close();
    });

    describe('PartialFlush Test', function () {
        it('should return 2', function () {
            var res = table.base.partialFlush(1000);
            assert.equal(res, 5);
            res = table.base.partialFlush(1000);
            assert.equal(res, 0);
        })
    });

    describe('Length Test', function () {
        it('should return the length of both stores', function () {
            assert.equal(table.base.store("People").length, 2);
            assert.equal(table.base.store("Movies").length, 0);
        })
    });

    describe('IsNumeric Test', function () {
        it('should return true for the fields ""Year" and "Ratings"', function () {
            assert(table.base.store("Movies").isNumeric("Year"));
            assert(table.base.store("Movies").isNumeric("Rating"));
        })
        it('should return false for the field "Title"', function () {
            assert(!table.base.store("Movies").isNumeric("Title"));
        })
        it('should throw an exception if the field doesn\'t exists', function () {
            assert.throws(function () {
                table.base.store("Movies").isNumeric("Actors");
            })
        })
    });

    describe('IsString Test', function () {
        it('should return true for the fields "Title" and "Plot"', function () {
            assert(table.base.store("Movies").isString("Title"));
            assert(table.base.store("Movies").isString("Plot"));
        })
        it('should return false for the field "Year"', function () {
            assert(!table.base.store("Movies").isString("Year"));
        })
        it('should throw an exception if the field doesn\'t exist', function () {
            assert.throws(function () {
                table.base.store("Movies").isString("Actors");
            })
        })
    });

    describe('Joins Test', function () {
        it('should return the number of joins for each store', function () {
            assert.equal(table.base.store("People").joins.length, 2);
            assert.equal(table.base.store("Movies").joins.length, 2);
        })
        it('should return the name of joins for each store', function () {
            var PeopleJoins = table.base.store("People").joins;
            var MoviesJoins = table.base.store("Movies").joins;

            assert.equal(PeopleJoins[0].name, "ActedIn");
            assert.equal(PeopleJoins[1].name, "Directed");

            assert.equal(MoviesJoins[0].name, "Actor");
            assert.equal(MoviesJoins[1].name, "Director");
        })
    });

    describe('Key Test', function () {
        it('should return the number of keys for each store', function () {
            assert.equal(table.base.store("People").keys.length, 2);
            assert.equal(table.base.store("Movies").keys.length, 4);
        })
        it('should return the name of the keys for each store', function () {
            var PeopleKeys = table.base.store("People").keys;
            var MoviesKeys = table.base.store("Movies").keys;

            assert.equal(PeopleKeys[0].name, "Name");
            assert.equal(PeopleKeys[1].name, "Gender");

            assert.equal(MoviesKeys[0].name, "Title");
            assert.equal(MoviesKeys[1].name, "TitleTxt");
            assert.equal(MoviesKeys[2].name, "Plot");
            assert.equal(MoviesKeys[3].name, "Genres");
        })
        it('should return the details of the key "Plot"', function () {
            var detail = table.base.store("Movies").key("Plot");
            assert.equal(detail.name, "Plot");
        })
        it('should return null if the key doesn\'t exist', function () {
            assert(table.base.store("Movies").key("Watched") == null);
        })
    });

    describe('Movies Fields Test', function () {
        it('should return the number of fields', function () {
            var arr = table.base.store("Movies").fields;
            // it also returns internal fields that are created for index joins (2 additional fields)
            assert.equal(arr.length, 7);
        })
        it('should return an array of Movies store fields', function () {
            var arr = table.base.store("Movies").fields;
            assert.equal(arr[0].name, "Title");
            assert.equal(arr[1].name, "Plot");
            assert.equal(arr[2].name, "Year");
            assert.equal(arr[3].name, "Rating");
            assert.equal(arr[4].name, "Genres");
        })
    });

    describe('First Test', function () {
        it('should return the first record of store "Person"', function () {
            var record = table.base.store("People").first;
            assert.equal(record.Name, "Carolina Fortuna");
            assert.equal(record.Gender, "Female");
        })
    });

    describe('Last Test', function () {
        it('should return the last record of the store "People"', function () {
            var record = table.base.store("People").last;
            assert.equal(record.Name, "Blaz Fortuna");
            assert.equal(record.Gender, "Male");
        })
    })

    describe('Add Movie Test', function () {

        it('should add a movie and all people that acted or directed', function () {
            assert.equal(table.base.store("Movies").push(table.movie), 0);
            assert.equal(table.base.store("Movies").length, 1);
            assert.equal(table.base.store("People").length, 29);
        })
        it('should return the correct values when adding a new movie', function () {
            assert.equal(table.base.store("Movies").push(table.movie), 0);

            assert(table.base.store("Movies")[0] != null);
            assert.equal(table.base.store("Movies")[0].Title, table.movie.Title);
            assert.equal(table.base.store("Movies")[0].Plot, table.movie.Plot);
            assert.equal(table.base.store("Movies")[0].Year, table.movie.Year);
            assert.equal(table.base.store("Movies")[0].Genres.length, table.movie.Genres.length);
            assert.equal(table.base.store("Movies")[0].Director.Name, "Levine Richard (III)");
        })
        it('should add 2 movies and all the people that acted or directed', function () {
            table.addMovie(table.movie);
            table.addMovie(table.movie2);

            assert.equal(table.base.store("Movies").length, 2);
            assert.equal(table.base.store("People").length, 62);
        })
        it('should return the correct values when adding a new movie', function () {
            table.addMovie(table.movie);
            table.addMovie(table.movie2);

            assert(table.base.store("Movies")[1] != null);
            assert.equal(table.base.store("Movies")[1].Title, table.movie2.Title);
            assert.equal(table.base.store("Movies")[1].Plot, table.movie2.Plot);
            assert.equal(table.base.store("Movies")[1].Year, table.movie2.Year);
            assert.equal(table.base.store("Movies")[1].Genres.length, table.movie2.Genres.length);
            assert.equal(table.base.store("Movies")[1].Director.Name, "Reyes Tony Y.");
        })
    });

    describe('ForwardIter Test', function () {
        it('should go through the Movies iteration from start to end', function () {
            table.addMovie(table.movie);
            table.addMovie(table.movie2);

            var MoviesIter = table.base.store("Movies").forwardIter;
            assert.equal(MoviesIter.next(), true);
            assert.equal(MoviesIter.record.$id, 0);
            assert.equal(MoviesIter.record.Title, "Every Day");
            assert.equal(MoviesIter.next(), true);
            assert.equal(MoviesIter.record.$id, 1);
            assert.equal(MoviesIter.record.Title, "Enteng Kabisote 3: Okay ka fairy ko... The legend goes on and on and on");
            assert.equal(MoviesIter.next(), false);
        })
    });

    describe('BackwardIter Test', function () {
        it('should go through the Movie iteration from end to start', function () {
            table.addMovie(table.movie);
            table.addMovie(table.movie2);

            var MoviesIter = table.base.store("Movies").backwardIter;
            assert.equal(MoviesIter.next(), true);
            assert.equal(MoviesIter.record.$id, 1);
            assert.equal(MoviesIter.record.Title, "Enteng Kabisote 3: Okay ka fairy ko... The legend goes on and on and on");
            assert.equal(MoviesIter.next(), true);
            assert.equal(MoviesIter.record.$id, 0);
            assert.equal(MoviesIter.record.Title, "Every Day");
            assert.equal(MoviesIter.next(), false);
        })
    })

    describe('Sample Test', function () {
        it('should return a sample of persons out of People', function () {
            table.addMovie(table.movie);
            table.addMovie(table.movie2);

            assert.equal(table.base.store("People").sample(0).length, 0);
            assert.equal(table.base.store("People").sample(1).length, 1);
            assert.equal(table.base.store("People").sample(10).length, 10);
            assert.equal(table.base.store("People").sample(62).length, 62);
            assert.equal(table.base.store("People").sample(63).length, 62);
            assert.equal(table.base.store("People").sample(100000).length, 62);
        })
        it('should throw an exception if sample parameter is negative', function () {
            table.addMovie(table.movie);
            table.addMovie(table.movie2);

            assert.throws(function () {
                table.base.store("People").sample(-10);
            })
        })
    });

    describe('Importing Test', function () {
        it('should import the movies in the movies_data.txt', function () {
            var filename = "./sandbox/movies/movies_data.txt"
            assert.equal(table.base.store("Movies").loadJson(filename), 167);
        })
    })

    describe('GetMatrix Test', function () {
        it('should get a matrix with the players score', function () {
            table.addPlayer(table.player1);
            table.addPlayer(table.player2);

            var mat = table.base.store("Basketball").getMatrix("Score");
            assert.equal(mat.rows, 3);
            assert.equal(mat.cols, 2);
            assert.equal(mat.at(1, 1), 100);

        })
        it('should throw an exception, if the field values are of different lenghts', function () {
            table.addPlayer(table.player1);
            table.addPlayer(table.player2);
            table.addPlayer(table.player3);

            assert.throws(function () {
                var mat = table.base.store("Basketball").getMatrix("Score");
            })
        })
        it('should throw an exception, if the field type is non-numeric', function () {
            assert.throws(function () {
                var mat = table.base.store("People").getMatrix("Name");
            })
        })
        it('should throw an exception, if the parameter is a non-existing field', function () {
            assert.throws(function () {
                var mat = table.base.store("People").getMatrix("DateOfBirth");
            })
        })
        it('should return a 1-by-2 matrix containing the movie relese years', function () {
            table.addMovie(table.movie);
            table.addMovie(table.movie2);

            var mat = table.base.store("Movies").getMatrix("Year");
            assert.equal(mat.rows, 1);
            assert.equal(mat.cols, 2);
            assert.equal(mat.at(0, 0), 2010);
            assert.equal(mat.at(0, 1), 2006);
        })
    });

    describe('isDate Test', function () {
        it('should return true for FirstPlayed field', function () {
            assert.ok(table.base.store("Basketball").isDate("FirstPlayed"));
        })
        it('should return false for Score field', function () {
            assert.ok(!table.base.store("Basketball").isDate("Score"));
        })
    })

    describe('ToJSON Test', function () {
        it('should return the store "People" as a JSON', function () {
            var json = table.base.store("People").toJSON();
            assert.equal(json.storeName, "People");
            assert.equal(json.storeRecords, 2);
            assert.equal(json.fields[0].fieldName, "Name");
            assert.equal(json.fields[1].fieldName, "Gender");
        })
        it('should return the store "Movies" as a JSON', function () {
            var json = table.base.store("Movies").toJSON();
            assert.equal(json.storeName, "Movies");
            assert.equal(json.storeRecords, 0);
            assert.equal(json.fields[0].fieldName, "Title");
            assert.equal(json.fields[1].fieldName, "Plot");
            assert.equal(json.fields[2].fieldName, "Year");
            assert.equal(json.fields[3].fieldName, "Rating");
            assert.equal(json.fields[4].fieldName, "Genres");
        })
    });

    describe('Cell Tests', function () {
        it('should return the name of the first record in "People"', function () {
            var name = table.base.store("People").cell(0, "Name");
            assert.equal(name, "Carolina Fortuna");
        })
        it('should return null, if the id is out of bound', function () {
            var name = table.base.store("People").cell(3, "Name");
            assert.equal(name, null);
        })
        it('should throw an exception, if the fieldName doesn\'t exist', function () {
            assert.throws(function () {
                var date = table.base.store("People").cell(0, "Date");
            })
        })
    });

    describe('NewRecord Tests', function () {
        it('should create a new record out of a JSON', function () {
            var rec = table.base.store("People").newRecord({ "Name": "Peter Bailish", "Gender": "Male" });

            assert.equal(rec.Name, "Peter Bailish");
            assert.equal(rec.Gender, "Male");
        })
        it('should create a new record, if fields are not same', function () {
            var rec = table.base.store("People").newRecord({ "Name": "Peter Bailish", "Gender": "Male", "DateOfBirth": "5.3.245" });
            assert.equal(rec.Name, "Peter Bailish");
            assert.equal(rec.Gender, "Male");
            assert.equal(rec.DateOfBirth, null);
        })
        it('should create a null record, if no field\'s in the store', function () {
            var rec = table.base.store("Movies").newRecord({ "Name": "Peter Bailish", "Gender": "Male" });
            assert.equal(rec.Name, null);
            assert.equal(rec.Gender, null);
        })
    });

    describe('NewRecordSet Tests', function () {
        it('should create a new record set out of the integer vector', function () {
            var recSet = table.base.store("People").newRecordSet(new qm.la.IntVector([0, 1]));
            assert.equal(recSet.length, 2);
            assert.equal(recSet[0].Name, "Carolina Fortuna");
            assert.equal(recSet[1].Name, "Blaz Fortuna");
        })
        it('should throw an exception, if the parameter is an integer array', function () {
            assert.throws(function () {
                var recSet = table.base.store("People").newRecordSet([0, 1]);
            })
        })
        it('should create a new record set, even if the vector values are out of bounds', function () {
            var recSet = table.base.store("People").newRecordSet(new qm.la.IntVector([0, 1, 2]));
            assert.equal(recSet.length, 3);
            assert.equal(recSet[0].Name, "Carolina Fortuna");
            assert.equal(recSet[1].Name, "Blaz Fortuna");
            assert.equal(recSet[2], null);
        })
    })
});

///////////////////////////////////////////////////////////////////////////////
// Many Stores

describe('Many Stores Test', function () {
    var base = undefined;
    beforeEach(function () {
        base = new qm.Base({ mode: 'createClean' });
    });
    afterEach(function () {
        base.close();
    });

    describe('Creating 254 Stores Test', function () {
        it('should create 254 stores', function () {
            var numOfStores = 254

            for (i = 0; i < numOfStores; i++) {
                var storeDef = {
                    "name": "TestStore_" + i,
                    "fields": [{ "name": "Test", "type": "float" }]
                }
                base.createStore(storeDef);
            }

            assert.equal(254, base.getStoreList().length);
        })
    });

    // takes to long
    //describe('Creating 1000 Stores Test', function () {
    //    it('should create 1000 stores', function () {
    //        var numOfStores = 1000

    //        for (i = 0; i < numOfStores; i++) {
    //            var storeDef = {
    //                "name": "TestStore_" + i,
    //                "fields": [{ "name": "Test", "type": "float" }]
    //            }
    //            base.createStore(storeDef);
    //        }

    //        assert.equal(1000, base.getStoreList().length);
    //    })
    //});
})

///////////////////////////////////////////////////////////////////////////////
// AddTrigger

describe('AddTrigger Tests', function () {
    var table = undefined;
    beforeEach(function () {
        table = new TStore();
    });
    afterEach(function () {
        table.close();
    });

    describe('Adding the AddTrigger Test', function () {
        it('should add the addTrigger for the people', function () {
            var PeopleAdd = 0; var PeopleUpdate = 0;
            table.base.store("People").addTrigger({
                onAdd: function (person) {
                    assert(null != person);
                    assert(null != person.Name);
                    assert(null != person.Gender);
                    PeopleAdd = PeopleAdd + 1;
                },
                onUpdate: function (person) {
                    assert(null != person);
                    assert(null != person.Name);
                    assert(null != person.Gender);
                    PeopleUpdate = PeopleUpdate + 1;
                }
            });

            // adding the people from movies
            table.addMovie(table.movie);
            table.addMovie(table.movie2);

            assert.equal(PeopleAdd, 60);
            assert.equal(PeopleUpdate, 0);
        })

        it('should add the addTrigger for movies', function () {
            var MoviesAdd = 0; var MoviesUpdate = 0;
            table.base.store("Movies").addTrigger({
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

            // adding the people from movies
            table.addMovie(table.movie);
            table.addMovie(table.movie2);

            assert.equal(MoviesAdd, 2);
            assert.equal(MoviesUpdate, 0);
        })
    });
})

///////////////////////////////////////////////////////////////////////////////
// Query

describe('Query Tests', function () {
    var table = undefined;
    beforeEach(function () {
        table = new TStore();
        table.addMovie(table.movie);
        table.addMovie(table.movie2);
    });
    afterEach(function () {
        table.close();
    });

    describe('Creating Query Test', function () {
        it("should create a query", function () {
            query = { $from: "People", Name: "john" };
            assert.equal(query.$from, "People");
            assert.equal(query.Name, "john");
        })
    })
})

//////////////////////////////////////////////////////////////////////////////
// CSV
//describe('Load CSV tests', function () {
//	var fout = null;
//	var base = null;
//
//	beforeEach(function () {
//		base = new qm.Base({ mode: 'createClean' });
//    });
//    afterEach(function () {
//    	base.close();
//    });
//
//	describe('Creating load CSV test ...', function () {
//		it('should create a full store with correct types', function () {
//			// generate a CSV file
//			var headers = ['A', 'B', 'C', 'D'];
//			var types = ['string', 'float', 'float', 'string'];
//
//			var table = [];
//
//			var n = 100;
//
//			fout = new fs.FOut('test.csv', false);
//			fout.writeLine(headers.join(','));
//			for (var i = 0; i < n; i++) {
//				var lineStr = '';
//				var row = [];
//				for (var j = 0; j < headers.length; j++) {
//					if (types[j] == 'string') {
//						row.push(Math.random().toString(36).replace(/[^a-z]+/g, '').substr(0, 5));	// random string
//					} else {
//						row.push(i);
//					}
//				}
//
//				table.push(row);
//				fout.write(row.join(','));
//
//				if (i < n - 1)
//					fout.write('\n');
//			}
//
//			fout.flush();
//			fout.close();
//
//			// read the CSV file into a store
//
//			base.loadCSV({file: 'test.csv', store: 'test'}, function (err, store) {
//				if (err != null) {
//					assert.fail(0, 0, 'Exception while loading CSV file! ' + err, ',');
//				} else {
//					assert(store != null, 'Store is not defined!');
//
//					// check if the fields are correct
//					var fields = store.fields;
//
//					assert.equal(fields.length, headers.length, 'Invalid number of fields generated!');
//
//					for (var i = 0; i < fields.length; i++) {
//						var field = fields[i];
//
//						assert.equal(field.name, headers[i], 'Invalid name of field!');
//						assert.equal(field.type, types[i], 'Invalid type of field!');
//					}
//
//					// check if the values are correct
//					for (var i = 0; i < store.length; i++) {
//						var rec = store[i];
//						for (var j = 0; j < headers.length; j++) {
//							var val = rec[headers[j]];
//
//							assert.equal(val, table[i][j], 'Value mismatch!');
//						}
//					}
//				}
//
//				fs.del('test.csv');
//				assert(!fs.exists('test.csv'), 'The test CSV file could not be deleted!');
//			});
//		});
//	})
//});


///////////////////////////////////////////////////////////////////////////////
// Schema Time WIndow Test#

describe('Schema Time Window Test', function () {

    describe('Testing window (size: 3)', function () {
        // generate store with window 3
        base = new qm.Base({ mode: 'createClean' });
        base.createStore({
            "name": "TestStore",
            "fields": [
                { "name": "DateTime", "type": "datetime" },
                { "name": "Measurement", "type": "float" }
            ],
            window: 3,
        });

        // push 5 records into created store
        for (var i = 0; i < 5; i++) {
            var rec = {
                "DateTime": new Date().toISOString(),
                "Measurement": i
            };
            base.store("TestStore").push(rec);
        }

        // clean base with garbage collector
        var before = base.store("TestStore").allRecords.length;
        base.garbageCollect();
        var after =  base.store("TestStore").allRecords.length;

        // test number of records in store before garbage cleanup
        it('store should contain 5 records before .garbageCollect()', function () {
            assert.equal(5, before);
        });

        // test number of records in store after garbage cleanup
        it('store should contain 3 records after .garbageCollect()', function () {
            assert.equal(3, after);
        });

        base.close();
    });

    describe('Testing timeWindow (size: 2h)', function () {
                // generate store with window 3
        base = new qm.Base({ mode: 'createClean' });
        base.createStore({
            "name": "TestStore",
            "fields": [
                { "name": "DateTime", "type": "datetime" },
                { "name": "Measurement", "type": "float" }
            ],
            timeWindow: {
                duration: 2,
                unit: "hour",
                field: "DateTime"
            }
        });

        // push 5 records into created store
        for (var i = 0; i < 5; i++) {
            var rec = {
                "DateTime": new Date(new Date().getTime() + i * 60 * 60 * 1001).toISOString(),
                "Measurement": i
            };
            base.store("TestStore").push(rec);
        }

        // clean base with garbage collector
        var before = base.store("TestStore").allRecords.length;
        base.garbageCollect();
        var after =  base.store("TestStore").allRecords.length;

        // test number of records in store before garbage cleanup
        it('store should contain 5 records before .garbageCollect()', function () {
            assert.equal(5, before);
        });

        // test number of records in store after garbage cleanup
        it('store should contain 2 records after .garbageCollect()', function () {
            assert.equal(2, after);
        });

        base.close();
    });

    describe('Testing window garbage collection timeout', function () {
        // generate store with window 3
        base = new qm.Base({ mode: 'createClean' });
        base.createStore({
            "name": "TestStore",
            "fields": [
                { "name": "Measurement", "type": "float" }
            ],
            window: 3,
        });

        // push 100k records into created store
        for (var i = 0; i < 100000; i++) {
            base.store("TestStore").push({
                "Measurement": i
            });
        }
        // clean base with garbage collector giving it 10ms
        var recs1 = base.store("TestStore").length;
        var time1 = new Date().getTime();
        base.garbageCollect(10);
        var recs2 = base.store("TestStore").length;
        var time2 = new Date().getTime();
        base.garbageCollect(20);
        var recs3 = base.store("TestStore").length;
        var time3 = new Date().getTime();

        // tests assume 30ms is to little time to delete 100000 records
        it('should delete some records', function () {
            assert(recs1 > recs2);
            assert(recs2 > recs3);
        });
        it('should delete twice as many records when given double time', function () {
            var ratio = (recs3-recs2) / (recs2-recs1);
            assert(ratio > 1.5);
            assert(ratio < 2.5);
        });
        it.skip('should take approximatey as much time as give', function () {
            // should not take less then give, since we assume there is to little time to delete what is needed
            assert((time2-time1) >= 10);
            assert((time3-time2) >= 20);
            // we allow for max 20ms overhead
            assert((time2-time1) < (10 + 20));
            assert((time3-time2) < (20 + 20));
        });

        base.garbageCollect();
        var recs4 = base.store("TestStore").length;
        it('should delete all but 3 records when no timeout parameter given', function () {
            assert.equal(3, recs4);
        });

        base.close();
    });

    describe('Primary key test', function () {
        var base;
        beforeEach(function () {
            base = new qm.Base({ mode: 'createClean' });
        });
        afterEach(function () {
            base.close();
        });

        describe('String', function () {
            it('should pass', function () {
                base.createStore({
                    "name": "People",
                    "fields": [
                        { "name": "Name", "type": "string", "primary": true },
                        { "name": "Age", "type": "int" }
                    ]
                });
                var store = base.store("People");

                // Add recores and make sure name does not repeate
                store.push({ Name: "John", Age: 12 });
                assert.equal(store.length, 1);
                store.push({ Name: "Mary", Age: 13 });
                assert.equal(store.length, 2);
                store.push({ Name: "John", Age: 14 });
                assert.equal(store.length, 2);
                store.push({ Name: "Steve", Age: 15 });
                assert.equal(store.length, 3);
                // check they have correct IDs
                assert.equal(store.recordByName("John").$id, 0);
                assert.equal(store.recordByName("Mary").$id, 1);
                assert.equal(store.recordByName("Steve").$id, 2);
                // if we rename the ID should still hold
                store[2].Name = "Martin";
                assert.equal(store.recordByName("Steve"), null);
                assert.equal(store.recordByName("Martin").$id, 2);
                // should work
                store[1].Name = "Jane"
                // should crash
                assert.throws(function () { store[2].Name = "John"; })
                // should work again
                store[0] = "Johnny";
                store[2] = "John";
            })
        });

        describe('Int', function () {
            it('should pass', function () {
                base.createStore({
                    "name": "People",
                    "fields": [
                        { "name": "Name", "type": "string" },
                        { "name": "Age", "type": "int", "primary": true }
                    ]
                });
                var store = base.store("People");

                // Add recores and make sure name does not repeate
                store.push({ Name: "John", Age: 12 });
                assert.equal(store.length, 1);
                store.push({ Name: "Mary", Age: 13 });
                assert.equal(store.length, 2);
                store.push({ Name: "Martin", Age: 12 });
                assert.equal(store.length, 2);
                store.push({ Name: "Steve", Age: 15 });
                assert.equal(store.length, 3);
                // should work
                store[1] = 16;
                // should crash
                assert.throws(function () { store[2].Age = 12; })
                // should work again
                store[0] = 11;
                store[2] = 12;
            })
        });

        describe('Float', function () {
            it('should pass', function () {
                base.createStore({
                    "name": "People",
                    "fields": [
                        { "name": "Name", "type": "string" },
                        { "name": "Age", "type": "float", "primary": true }
                    ]
                });
                var store = base.store("People");

                // Add recores and make sure name does not repeate
                store.push({ Name: "John", Age: 12.1 });
                assert.equal(store.length, 1);
                store.push({ Name: "Mary", Age: 13.1 });
                assert.equal(store.length, 2);
                store.push({ Name: "Martin", Age: 12.1 });
                assert.equal(store.length, 2);
                store.push({ Name: "Steve", Age: 15.1 });
                assert.equal(store.length, 3);
                // should work
                store[1] = 16.1;
                // should crash
                assert.throws(function () { store[2].Age = 12.1; })
                // should work again
                store[0] = 11.1;
                store[2] = 12.1;
            })
        });

        describe('UInt64', function () {
            it('should pass', function () {
                base.createStore({
                    "name": "People",
                    "fields": [
                        { "name": "Name", "type": "string" },
                        { "name": "Age", "type": "uint64", "primary": true }
                    ]
                });
                var store = base.store("People");

                // Add recores and make sure name does not repeate
                store.push({ Name: "John", Age: 12 });
                assert.equal(store.length, 1);
                store.push({ Name: "Mary", Age: 13 });
                assert.equal(store.length, 2);
                store.push({ Name: "Martin", Age: 12 });
                assert.equal(store.length, 2);
                store.push({ Name: "Steve", Age: 15 });
                assert.equal(store.length, 3);
                // should work
                store[1] = 16;
                // should crash
                assert.throws(function () { store[2].Age = 12; })
                // should work again
                store[0] = 11;
                store[2] = 12;
            })
        });

    });

})

describe('Int-ish field-type tests ', function () {
    it('should allow query with eact value over linear index', function () {
        var base = new qm.Base({ mode: 'createClean' });
        base.createStore({
            "name": "Alerts",
            "fields": [
                { "name": "title", "type": "string", "store" : "cache" },
                { "name": "ts", "type": "datetime", "store": "cache" },
                { "name": "is_child", "type": "byte", "store" : "cache", "default": 0 }
            ],
            "joins": [
                { "name": "children", "type": "index", "store": "Alerts", "storage_location" : "cache" },
                { "name": "parent", "type": "field", "store": "Alerts", "inverse": "children", "storage_location" : "cache" }
            ],
            "keys": [
                { "field": "title", "type": "text" },
                { "field": "ts", "type": "linear" },
                { "field": "is_child", "type": "linear" }
            ]
        });
        var store = base.store("Alerts");

        var id1 = store.push({ title : "test title 1", ts : (new Date()).getTime(), is_child: 0 });
        var id2 = store.push({ title : "test title 2", ts : (new Date()).getTime(), is_child: 1 });

        var query = {
            $from : "Alerts",
            $limit : 50,
            $sort : { ts : 0 },
            is_child : 0
        };

        var rs = base.search(query);
        assert.equal(rs.length, 1);
        assert.equal(rs[0].$id, id1);

        query.is_child = 1;
        rs = base.search(query);
        assert.equal(rs.length, 1);
        assert.equal(rs[0].$id, id2);

        base.close();
    })
})
