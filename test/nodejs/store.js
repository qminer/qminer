/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
 
console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js');     //adds assert.run function
var qm = require('qminer');
qm.delLock();
//qm.rmDir('db') // run from qminer/test/nodejs 

qm.config('qm.conf', true, 8080, 1024);

///////////////////////////////////////////////////////////////////////////////
// Empty Store

function EmptyStore() {
    // creates the base and store
    this.base = qm.create('qm.conf', "", true);
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
            var recs = table.base.store("People").recs;
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
            assert.equal(table.base.store("People").add({ "Name": "Carolina Fortuna", "Gender": "Female" }), 0);
            assert.equal(table.base.store("People").length, 1);
        })
    });
})

///////////////////////////////////////////////////////////////////////////////
// Small Store

function Store() {
    this.base = qm.create('qm.conf', "", true);
    this.base.createStore({
        "name": "People",
        "fields": [
            { "name": "Name", "type": "string", "primary": true },
            { "name": "Gender", "type": "string", "shortstring": true }
        ]
    });
    this.base.store("People").add({ "Name": "Carolina Fortuna", "Gender": "Female" });
    this.base.store("People").add({ "Name": "Blaz Fortuna", "Gender": "Male" });

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
            table.base.store("People").add({ "Name": "Blaz Fortuna", "Gender": "Male" });
            assert.equal(table.base.store("People").length, 2);

        })
    })

    describe('ForwardIter Test', function () {
        it('should go through the persons in store', function () {
            var PeopleIter = table.base.store("People").forwardIter;
            assert.equal(PeopleIter.next(), true);
            assert.equal(PeopleIter.rec.$id, 0);
            assert.equal(PeopleIter.next(), true);
            assert.equal(PeopleIter.rec.$id, 1);
            assert.equal(PeopleIter.next(), false);
        })
    });

    describe('Rec Test', function () {
        it('should return the record of Carolina Fortuna', function () {
            var record = table.base.store("People").rec("Carolina Fortuna");
            assert.equal(record.Name, "Carolina Fortuna");
            assert.equal(record.Gender, "Female");
        })
        it('should return the record of Blaz Fortuna', function () {
            var record = table.base.store("People").rec("Blaz Fortuna");
            assert.equal(record.Name, "Blaz Fortuna");
            assert.equal(record.Gender, "Male");
        })
        it('should return null if record not found', function () {
            var record = table.base.store("People").rec("Bender Bending Rodriguez");
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

    describe('GetVec Test', function () {
        it('should return the vector of record names in "People" store', function () {
            var vec = table.base.store("People").getVec("Name");
            assert.equal(vec[0], "Carolina Fortuna");
            assert.equal(vec[1], "Blaz Fortuna");
        })
        it('should return the vector of record genders in "People" store', function () {
            var vec = table.base.store("People").getVec("Gender");
            assert.equal(vec[0], "Female");
            assert.equal(vec[1], "Male");
        })
        it('should throw an exception if parameter isn\'t given', function () {
            assert.throws(function () {
                table.base.store("People").getVec();
            })
        })
        it('should throw an exception if parameter isn\'t a field', function () {
            assert.throws(function () {
                table.base.store("People").getVec("Payday");
            })
        })
    });
})

///////////////////////////////////////////////////////////////////////////////
// Two Stores

function TStore() {
    this.base = qm.create('qm.conf', "", true);
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
            { "name": "Score", "type": "float_v" }
        ]
    }]);
    // adding two persons
    this.base.store("People").add({ "Name": "Carolina Fortuna", "Gender": "Female" });
    this.base.store("People").add({ "Name": "Blaz Fortuna", "Gender": "Male" });

    // adding two movies
    this.movie = { "Title": "Every Day", "Plot": "This day really isn't all that different than every other day. Except today, Ned's gay son Jonah wants to go to a college party, his wife is bringing home their elderly father to live with them, and his outrageous boss seems to have become even more crazy and demanding than would even seem possible. As his wife tries to take care of her father reconnect with him, Ned tries to reconnect with Jonah, and then without trying, he seems to have formed a connection with his co-worker. If he can get through days like these, he should be able to get through anything else life throws at him. Ned and Jeannie: married 19 years. Ned has trouble with Garrett, his boss at the cable show he writes, and he's ill-at-ease with his older son Jonah's coming out and wanting to go to a high-school gay student society prom. Jeannie puts work on hold while she attends to Ernie, her sour and mean-spirited father whose ill health forces him to move in with them. While Jeannie taxis the boys, goes to one son's recital, sees to her father's needs, and fixes meals, Garrett assigns Ned to rewrite a script with Robin, an uninhibited, unattached colleague who offers no-strings fun. Can this family hold together while a chicken hawk circles Jonah, Robin inveigles Ned, and death hunts Ernie?", "Year": 2010, "Rating": 5.6, "Genres": ["Comedy", "Drama"], "Director": { "Name": "Levine Richard (III)", "Gender": "Unknown" }, "Actor": [{ "Name": "Beetem Chris", "Gender": "Male" }, { "Name": "Carp Stan", "Gender": "Male" }, { "Name": "Chan Albert M.", "Gender": "Male" }, { "Name": "Dennehy Brian", "Gender": "Male" }, { "Name": "Durell Jesse", "Gender": "Male" }, { "Name": "Farcher Daniel", "Gender": "Male" }, { "Name": "Fortgang Skyler", "Gender": "Male" }, { "Name": "Harbour David (I)", "Gender": "Male" }, { "Name": "Ingram Michael H.", "Gender": "Male" }, { "Name": "Izzard Eddie", "Gender": "Male" }, { "Name": "James Kahan", "Gender": "Male" }, { "Name": "Jones Tilky", "Gender": "Male" }, { "Name": "Kempner Matt", "Gender": "Male" }, { "Name": "Miller Ezra", "Gender": "Male" }, { "Name": "Orchestra Black Diamond", "Gender": "Male" }, { "Name": "Riddle George", "Gender": "Male" }, { "Name": "Routman Steve", "Gender": "Male" }, { "Name": "Schreiber Liev", "Gender": "Male" }, { "Name": "Yelsky Daniel", "Gender": "Male" }, { "Name": "Gard Cassidy", "Gender": "Female" }, { "Name": "Giancoli Bianca", "Gender": "Female" }, { "Name": "Gugino Carla", "Gender": "Female" }, { "Name": "Hahn Sabrina", "Gender": "Female" }, { "Name": "Hunt Helen (I)", "Gender": "Female" }, { "Name": "Miller June (I)", "Gender": "Female" }, { "Name": "Robledo Benita", "Gender": "Female" }] };
    this.movie2 = { "Title": "Enteng Kabisote 3: Okay ka fairy ko... The legend goes on and on and on", "Plot": "no plot available", "Year": 2006, "Rating": 5.8, "Genres": ["Action", "Comedy", "Family", "Fantasy"], "Director": { "Name": "Reyes Tony Y.", "Gender": "Unknown" }, "Actor": [{ "Name": "Aquitania Antonio", "Gender": "Male" }, { "Name": "Ballesteros Paolo", "Gender": "Male" }, { "Name": "Bayola Wally", "Gender": "Male" }, { "Name": "Casimiro Jr. Bayani", "Gender": "Male" }, { "Name": "de Leon Joey", "Gender": "Male" }, { "Name": "Forbes BJ", "Gender": "Male" }, { "Name": "Ignacio Levi", "Gender": "Male" }, { "Name": "K. Allan", "Gender": "Male" }, { "Name": "Lapid Jr. Jess", "Gender": "Male" }, { "Name": "Manalo Jose", "Gender": "Male" }, { "Name": "Salas Paul", "Gender": "Male" }, { "Name": "Santos Jimmy (I)", "Gender": "Male" }, { "Name": "Sotto Gian", "Gender": "Male" }, { "Name": "Sotto Oyo Boy", "Gender": "Male" }, { "Name": "Sotto Tito", "Gender": "Male" }, { "Name": "Sotto Vic", "Gender": "Male" }, { "Name": "V. Michael (I)", "Gender": "Male" }, { "Name": "Zamora Ramon", "Gender": "Male" }, { "Name": "Alano Alyssa", "Gender": "Female" }, { "Name": "Guanio Pia", "Gender": "Female" }, { "Name": "Hermosa Kristine", "Gender": "Female" }, { "Name": "Jones Angelica", "Gender": "Female" }, { "Name": "Loyzaga Bing", "Gender": "Female" }, { "Name": "Madrigal Ehra", "Gender": "Female" }, { "Name": "Parker J.C.", "Gender": "Female" }, { "Name": "Ponti Cassandra", "Gender": "Female" }, { "Name": "Ramirez Mikylla", "Gender": "Female" }, { "Name": "Rodriguez Ruby (I)", "Gender": "Female" }, { "Name": "Seguerra Aiza", "Gender": "Female" }, { "Name": "Sotto Ciara", "Gender": "Female" }, { "Name": "Toengi Giselle", "Gender": "Female" }, { "Name": "V. Ella", "Gender": "Female" }] };

    this.addMovie = function (movie) {
        this.base.store("Movies").add(movie);
    }

    this.player1 = { "Player": "Goran Dragiæ", "Score": [35, 12, 23] };
    this.player2 = { "Player": "Michael Jordan", "Score": [90, 100, 95] };
    this.player3 = { "Player": "Marko Miliæ", "Score": [50, 10, 10, 12] };

    this.addPlayer = function (player) {
        this.base.store("Basketball").add(player);
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
        it('should return the details of the key "Title"', function () {
            var detail = table.base.store("Movies").key("Title");
            assert.equal(detail.name, "Title");
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
            assert.equal(table.base.store("Movies").add(table.movie), 0);
            assert.equal(table.base.store("Movies").length, 1);
            assert.equal(table.base.store("People").length, 29);
        })
        it('should return the correct values when adding a new movie', function () {
            assert.equal(table.base.store("Movies").add(table.movie), 0);

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
            assert.equal(MoviesIter.rec.$id, 0);
            assert.equal(MoviesIter.rec.Title, "Every Day");
            assert.equal(MoviesIter.next(), true);
            assert.equal(MoviesIter.rec.$id, 1);
            assert.equal(MoviesIter.rec.Title, "Enteng Kabisote 3: Okay ka fairy ko... The legend goes on and on and on");
            assert.equal(MoviesIter.next(), false);
        })
    });

    describe('BackwardIter Test', function () {
        it('should go through the Movie iteration from end to start', function () {
            table.addMovie(table.movie);
            table.addMovie(table.movie2);

            var MoviesIter = table.base.store("Movies").backwardIter;
            assert.equal(MoviesIter.next(), true);
            assert.equal(MoviesIter.rec.$id, 1);
            assert.equal(MoviesIter.rec.Title, "Enteng Kabisote 3: Okay ka fairy ko... The legend goes on and on and on");
            assert.equal(MoviesIter.next(), true);
            assert.equal(MoviesIter.rec.$id, 0);
            assert.equal(MoviesIter.rec.Title, "Every Day");
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
            assert.equal(qm.load.jsonFile(table.base.store("Movies"), filename), 167);
        })
    })

    describe('GetMat Test', function () {
        it('should get a matrix with the players score', function () {
            table.addPlayer(table.player1);
            table.addPlayer(table.player2);

            var mat = table.base.store("Basketball").getMat("Score");
            assert.equal(mat.rows, 3);
            assert.equal(mat.cols, 2);
            assert.equal(mat.at(1, 1), 100);

        })
        it('should throw an exception, if the field values are of different lenghts', function () {
            table.addPlayer(table.player1);
            table.addPlayer(table.player2);
            table.addPlayer(table.player3);

            assert.throws(function () {
                var mat = table.base.store("Basketball").getMat("Score");
            })
        })
        it('should throw an exception, if the field type is non-numeric', function () {
            assert.throws(function () {
                var mat = table.base.store("People").getMat("Name");
            })
        })
        it('should throw an exception, if the parameter is a non-existing field', function () {
            assert.throws(function () {
                var mat = table.base.store("People").getMat("DateOfBirth");
            })
        })
        it('should return a 1-by-2 matrix containing the movie relese years', function () {
            table.addMovie(table.movie);
            table.addMovie(table.movie2);

            var mat = table.base.store("Movies").getMat("Year");
            assert.equal(mat.rows, 1);
            assert.equal(mat.cols, 2);
            assert.equal(mat.at(0, 0), 2010);
            assert.equal(mat.at(0, 1), 2006);
        })
    });

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

    describe('NewRec Tests', function () {
        it('should create a new record out of a JSON', function () {
            var rec = table.base.store("People").newRec({ "Name": "Peter Bailish", "Gender": "Male" });

            assert.equal(rec.Name, "Peter Bailish");
            assert.equal(rec.Gender, "Male");
        })
        it('should create a new record, if fields are not same', function () {
            var rec = table.base.store("People").newRec({ "Name": "Peter Bailish", "Gender": "Male", "DateOfBirth": "5.3.245" });
            assert.equal(rec.Name, "Peter Bailish");
            assert.equal(rec.Gender, "Male");
            assert.equal(rec.DateOfBirth, null);
        })
        it('should create a null record, if no field\'s in the store', function () {
            var rec = table.base.store("Movies").newRec({ "Name": "Peter Bailish", "Gender": "Male" });
            assert.equal(rec.Name, null);
            assert.equal(rec.Gender, null);
        })
    });

    describe('NewRecSet Tests', function () {
        it('should create a new record set out of the integer vector', function () {
            var recSet = table.base.store("People").newRecSet(new qm.la.IntVector([0, 1]));
            assert.equal(recSet.length, 2);
            assert.equal(recSet[0].Name, "Carolina Fortuna");
            assert.equal(recSet[1].Name, "Blaz Fortuna");
        })
        it('should throw an exception, if the parameter is an integer array', function () {
            assert.throws(function () {
                var recSet = table.base.store("People").newRecSet([0, 1]);
            })
        })
        it('should create a new record set, even if the vector values are out of bounds', function () {
            var recSet = table.base.store("People").newRecSet(new qm.la.IntVector([0, 1, 2]));
            assert.equal(recSet.length, 3);
            assert.equal(recSet[0].Name, "Carolina Fortuna");
            assert.equal(recSet[1].Name, "Blaz Fortuna");
            assert.equal(recSet[2], null);
        })
    })
});

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
