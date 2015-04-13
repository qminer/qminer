console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js');     //adds assert.run function
var qm = require('qminer');
qm.delLock();

qm.config('qm.conf', true, 8080, 1024);

// the database/store, from which we get the record set
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
    }]);
    // adding two persons
    this.base.store("People").add({ "Name": "Carolina Fortuna", "Gender": "Female", $fq: 5});
    this.base.store("People").add({ "Name": "Blaz Fortuna", "Gender": "Male", $fq: 3 });

    // adding two movies
    this.base.store("Movies").add({ "Title": "Every Day", "Plot": "This day really isn't all that different than every other day. Except today, Ned's gay son Jonah wants to go to a college party, his wife is bringing home their elderly father to live with them, and his outrageous boss seems to have become even more crazy and demanding than would even seem possible. As his wife tries to take care of her father reconnect with him, Ned tries to reconnect with Jonah, and then without trying, he seems to have formed a connection with his co-worker. If he can get through days like these, he should be able to get through anything else life throws at him. Ned and Jeannie: married 19 years. Ned has trouble with Garrett, his boss at the cable show he writes, and he's ill-at-ease with his older son Jonah's coming out and wanting to go to a high-school gay student society prom. Jeannie puts work on hold while she attends to Ernie, her sour and mean-spirited father whose ill health forces him to move in with them. While Jeannie taxis the boys, goes to one son's recital, sees to her father's needs, and fixes meals, Garrett assigns Ned to rewrite a script with Robin, an uninhibited, unattached colleague who offers no-strings fun. Can this family hold together while a chicken hawk circles Jonah, Robin inveigles Ned, and death hunts Ernie?", "Year": 2010, "Rating": 5.6, "Genres": ["Comedy", "Drama"], "Director": { "Name": "Levine Richard (III)", "Gender": "Unknown" }, "Actor": [{ "Name": "Beetem Chris", "Gender": "Male" }, { "Name": "Carp Stan", "Gender": "Male" }, { "Name": "Chan Albert M.", "Gender": "Male" }, { "Name": "Dennehy Brian", "Gender": "Male" }, { "Name": "Durell Jesse", "Gender": "Male" }, { "Name": "Farcher Daniel", "Gender": "Male" }, { "Name": "Fortgang Skyler", "Gender": "Male" }, { "Name": "Harbour David (I)", "Gender": "Male" }, { "Name": "Ingram Michael H.", "Gender": "Male" }, { "Name": "Izzard Eddie", "Gender": "Male" }, { "Name": "James Kahan", "Gender": "Male" }, { "Name": "Jones Tilky", "Gender": "Male" }, { "Name": "Kempner Matt", "Gender": "Male" }, { "Name": "Miller Ezra", "Gender": "Male" }, { "Name": "Orchestra Black Diamond", "Gender": "Male" }, { "Name": "Riddle George", "Gender": "Male" }, { "Name": "Routman Steve", "Gender": "Male" }, { "Name": "Schreiber Liev", "Gender": "Male" }, { "Name": "Yelsky Daniel", "Gender": "Male" }, { "Name": "Gard Cassidy", "Gender": "Female" }, { "Name": "Giancoli Bianca", "Gender": "Female" }, { "Name": "Gugino Carla", "Gender": "Female" }, { "Name": "Hahn Sabrina", "Gender": "Female" }, { "Name": "Hunt Helen (I)", "Gender": "Female" }, { "Name": "Miller June (I)", "Gender": "Female" }, { "Name": "Robledo Benita", "Gender": "Female" }] });
    this.base.store("Movies").add({ "Title": "Enteng Kabisote 3: Okay ka fairy ko... The legend goes on and on and on", "Plot": "no plot available", "Year": 2006, "Rating": 5.8, "Genres": ["Action", "Comedy", "Family", "Fantasy"], "Director": { "Name": "Reyes Tony Y.", "Gender": "Unknown" }, "Actor": [{ "Name": "Aquitania Antonio", "Gender": "Male" }, { "Name": "Ballesteros Paolo", "Gender": "Male" }, { "Name": "Bayola Wally", "Gender": "Male" }, { "Name": "Casimiro Jr. Bayani", "Gender": "Male" }, { "Name": "de Leon Joey", "Gender": "Male" }, { "Name": "Forbes BJ", "Gender": "Male" }, { "Name": "Ignacio Levi", "Gender": "Male" }, { "Name": "K. Allan", "Gender": "Male" }, { "Name": "Lapid Jr. Jess", "Gender": "Male" }, { "Name": "Manalo Jose", "Gender": "Male" }, { "Name": "Salas Paul", "Gender": "Male" }, { "Name": "Santos Jimmy (I)", "Gender": "Male" }, { "Name": "Sotto Gian", "Gender": "Male" }, { "Name": "Sotto Oyo Boy", "Gender": "Male" }, { "Name": "Sotto Tito", "Gender": "Male" }, { "Name": "Sotto Vic", "Gender": "Male" }, { "Name": "V. Michael (I)", "Gender": "Male" }, { "Name": "Zamora Ramon", "Gender": "Male" }, { "Name": "Alano Alyssa", "Gender": "Female" }, { "Name": "Guanio Pia", "Gender": "Female" }, { "Name": "Hermosa Kristine", "Gender": "Female" }, { "Name": "Jones Angelica", "Gender": "Female" }, { "Name": "Loyzaga Bing", "Gender": "Female" }, { "Name": "Madrigal Ehra", "Gender": "Female" }, { "Name": "Parker J.C.", "Gender": "Female" }, { "Name": "Ponti Cassandra", "Gender": "Female" }, { "Name": "Ramirez Mikylla", "Gender": "Female" }, { "Name": "Rodriguez Ruby (I)", "Gender": "Female" }, { "Name": "Seguerra Aiza", "Gender": "Female" }, { "Name": "Sotto Ciara", "Gender": "Female" }, { "Name": "Toengi Giselle", "Gender": "Female" }, { "Name": "V. Ella", "Gender": "Female" }] });

    this.close = function () {
        this.base.close();
    }
};

///////////////////////////////////////////////////////////////////////////////
// Record set

describe('Record Set Tests', function () {

    var table;
    var recSet, recSet2;
    beforeEach(function () {
        table = new TStore();
        recSet = table.base.store("Movies").recs;
        recSet2 = table.base.store("People").recs;
    });
    afterEach(function () {
        table.close();
    });

    describe('Length Test', function () {
        it('should return the number of records', function () {
            assert.equal(recSet.length, 2);
        })
    });

    describe('Store Test', function () {
        it('should return the store of recSet', function () {
            assert.equal(recSet.store.name, "Movies");
        })
    });

    describe('Empty Test', function () {
        it('should return false for a non-empty record set', function () {
            assert(!recSet.empty);
        })
    });

    describe('Weighted Test', function () {
        it('should return false for non-weighted record sets', function () {
            assert(!recSet.weighted);
        })
    });

    describe('IndexId Test', function () {
        it('should return the records of index 1 and 2', function () {
            assert.equal(recSet[0].Title, "Every Day");
            assert.equal(recSet[1].Title, "Enteng Kabisote 3: Okay ka fairy ko... The legend goes on and on and on");
        })
    });

    describe('Clone Test', function () {
        it('should create a new instance of the record set', function () {
            var recSet2 = recSet.clone();
            assert.equal(recSet2.length, recSet.length);
            assert.equal(recSet2[0].Title, recSet[0].Title);
            assert.equal(recSet2[1].Title, recSet[1].Title);
            assert.equal(recSet2.store.name, recSet.store.name)
        })
    });

    describe('Join Tests', function () {
        it('should return all directors of some movie', function () {
            var recSet2 = recSet.join("Director");
            assert.equal(recSet2.length, 2);
            assert.equal(recSet2[0].Name, "Levine Richard (III)");
            assert.equal(recSet2[0].Gender, "Unknown");
            assert.equal(recSet2[1].Name, "Reyes Tony Y.");
            assert.equal(recSet2[1].Gender, "Unknown");
        })
        it('should return all actors of some movie', function () {
            var recSet2 = recSet.join("Actor");
            assert.equal(recSet2.length, 58);
            assert.equal(recSet2[0].Name, "Beetem Chris");
            assert.equal(recSet2[0].Gender, "Male");
            assert.equal(recSet2[1].Name, "Carp Stan");
            assert.equal(recSet2[1].Gender, "Male");
        })
        it('should return a record set of 1 sample record', function () {
            var recSet2 = recSet.join("Director", 1);
            assert.equal(recSet2.length, 1);
        })
    });

    describe('Truncate Tests', function () {
        it('should truncate the first record of the record set', function () {
            recSet.trunc(1);
            assert.equal(recSet.length, 1);
            assert.equal(recSet[0].Title, "Every Day");
            assert.equal(recSet[1], undefined);
        })
        it('should truncate the second record of the record set', function () {
            recSet.trunc(1, 1);
            assert.equal(recSet.length, 1);
            assert.equal(recSet[0].Title, "Enteng Kabisote 3: Okay ka fairy ko... The legend goes on and on and on");
            assert.equal(recSet[1], undefined);
        })
        it('should truncate the first record of the record set', function () {
            recSet.trunc(1, 0);
            assert.equal(recSet.length, 1);
            assert.equal(recSet[0].Title, "Every Day");
            assert.equal(recSet[1], undefined);
        })
        it('should truncate all the records from the record set', function () {
            recSet.trunc(2);
            assert.equal(recSet.length, 2);
            assert.equal(recSet[0].Title, "Every Day");
            assert.equal(recSet[1].Title, "Enteng Kabisote 3: Okay ka fairy ko... The legend goes on and on and on");
        })
        it('shouldn\'t truncate any record of the record set', function () {
            recSet.trunc(0);
            assert.equal(recSet.length, 0);
        })
        it('should throw an exception if no parameter is given', function () {
            assert.throws(function () {
                recSet.trunc();
            })
        })
    });

    describe('Sample Tests', function () {
        it('should return a new record set with 10 random records', function () {
            var rs = recSet2.sample(10);
            assert.equal(rs.length, 10);
            assert.equal(rs.store.name, "People");
        })
        it('should return a new record set with 1 random record', function () {
            var rs = recSet.sample(1);
            assert.equal(rs.length, 1);
            assert.equal(rs.store.name, "Movies");
        })
        it('should throw an exception, if no parameter is given', function () {
            assert.throws(function () {
                var rs = recSet2.sample();
            })
        })
        it('should return a new record set, only with shuffled records', function () {
            var rs = recSet.sample(2);
            assert.equal(rs.length, 2);
        })
    });

    describe('Shuffle Tests', function () {
        it('should shuffle the record set', function () {
            recSet.shuffle(102);
            assert.equal(recSet.length, 2);
        })
        it('should shuffle the record set recSet2', function () {
            recSet2.shuffle(250);
            assert.equal(recSet2.length, 62);
        })
        it('should shuffle the record set, no parameter given', function () {
            recSet2.shuffle();
            assert.equal(recSet2.length, 62);
        })
    });

    describe('Reverse Tests', function () {
        it('should reverse the record set recSet', function () {
            recSet.reverse();
            assert.equal(recSet.length, 2);
            assert.equal(recSet[0].Title, "Enteng Kabisote 3: Okay ka fairy ko... The legend goes on and on and on");
            assert.equal(recSet[1].Title, "Every Day");
        })
    });

    describe('SortById Tests', function () {
        it('should sort the records in ascending order', function () {
            recSet2.sortById(1);
            assert.equal(recSet2.length, 62);
            assert.equal(recSet2[0].Name, "Carolina Fortuna");
            assert.equal(recSet2[1].Name, "Blaz Fortuna");
        })
        it('should sort the records in descending order', function () {
            recSet2.sortById(-1);
            assert.equal(recSet2.length, 62);
            assert.equal(recSet2[60].Name, "Blaz Fortuna");
            assert.equal(recSet2[61].Name, "Carolina Fortuna");
        })
        it('should sort the records in descending order if parameter is zero', function () {
            recSet2.sortById(0);
            assert.equal(recSet2.length, 62);
            assert.equal(recSet2[60].Name, "Blaz Fortuna");
            assert.equal(recSet2[61].Name, "Carolina Fortuna");
        })
        it('should sort the records in ascending order if no parameter is given', function () {
            assert.throws(function () {
                recSet2.sortById();
                assert.equal(recSet2.length, 62);
                assert.equal(recSet2[0].Name, "Carolina Fortuna");
                assert.equal(recSet2[1].Name, "Blaz Fortuna");
            })
        })
    });

    describe.skip('SortByFq Tests', function () {
        it('should sort records by weight in ascending order', function () {
            recSet2.sortByFq(1);
            assert.equal(recSet2[0].Name, "Blaz Fortuna");
            assert.equal(recSet2[1].Name, "Carolina Fortuna");
        })
        it('should sort records by weight in descending order', function () {
            recSet2.sortByFq(-1);
            assert.equal(recSet2[0].Name, "Carolina Fortuna");
            assert.equal(recSet2[1].Name, "Blaz Fortuna");
        })
    })

    describe('SortByField Tests', function () {
        it('should sort the records by field: Title in ascending order', function () {
            recSet.sortByField("Title", 1);
            assert.equal(recSet[0].Title, "Enteng Kabisote 3: Okay ka fairy ko... The legend goes on and on and on");
            assert.equal(recSet[1].Title, "Every Day");
        })
        it('should sort the records by field: Title in descending order', function () {
            recSet.sortByField("Title", -1);
            assert.equal(recSet[0].Title, "Every Day");
            assert.equal(recSet[1].Title, "Enteng Kabisote 3: Okay ka fairy ko... The legend goes on and on and on");
        })
        it('should sort the records by field: Title in descending order, if second parameter is not given', function () {
            recSet.sortByField("Title");
            assert.equal(recSet[0].Title, "Every Day");
            assert.equal(recSet[1].Title, "Enteng Kabisote 3: Okay ka fairy ko... The legend goes on and on and on");
        })
        it('should throw an exception if no parameters are given', function () {
            assert.throws(function () {
                recSet.sortByField();
            })
        })
    });

    describe('Sort Tests', function () {
        it('should sort the records according by their Ratings', function () {
            recSet.sort(function (rec, rec2) { return rec.Rating < rec2.Rating });
            assert.equal(recSet[0].Rating, 5.6);
            assert.equal(recSet[1].Rating, 5.8);
        })
        it('should sort the records according by their Rating + Year', function () {
            recSet.sort(function (rec, rec2) { return rec.Rating + rec.Year < rec2.Rating + rec2.Year });
            assert.equal(recSet[0].Title, "Enteng Kabisote 3: Okay ka fairy ko... The legend goes on and on and on");
            assert.equal(recSet[1].Title, "Every Day");
        })
        it.skip('should throw an exception, if the function is incomplete', function () {
            assert.throws(function () {
                recSet.sort(function (rec, rec2) { });
            })
        })
        it.skip('should throw an exception, if no parameter is given', function () {
            assert.throws(function () {
                recSet.sort();
            })
        })
    })

    describe('FilterById Tests', function () {
        it('should return the first two records of recSet2', function () {
            recSet2.filterById(0, 1);
            assert.equal(recSet2.length, 2);
            assert.equal(recSet2[0].Name, "Carolina Fortuna");
            assert.equal(recSet2[1].Name, "Blaz Fortuna");
        })
        it('should return only the first record of recSet', function () {
            recSet.filterById(0, 0);
            assert.equal(recSet.length, 1);
            assert.equal(recSet[0].Title, "Every Day");
        })
        it('shouldn\'t change the record set recSet', function () {
            recSet.filterById();
            assert.equal(recSet.length, 2);
            assert.equal(recSet[0].Title, "Every Day");
            assert.equal(recSet[1].Title, "Enteng Kabisote 3: Okay ka fairy ko... The legend goes on and on and on");
        })
        it('shouldn\'t change the record set recSet2', function () {
            recSet2.filterById();
            assert.equal(recSet2.length, 62);
            assert.equal(recSet2[0].Name, "Carolina Fortuna");
            assert.equal(recSet2[1].Name, "Blaz Fortuna");
        })
        it('should throw an exception', function () {
            assert.throws(function () {
                recSet2.filterById(1);
            })
        })
    });

    describe('FilterByField Test', function () {
        it('should keep only the "Blaz Fortuna" record', function () {
            recSet2.filterByField("Name", "Blaz Fortuna");
            assert.equal(recSet2.length, 1);
            assert.equal(recSet2[0].Name, "Blaz Fortuna");
        })
        it('should keep only the "Every Day" record', function () {
            recSet.filterByField("Rating", 5.5, 5.7);
            assert.equal(recSet.length, 1);
            assert.equal(recSet[0].Title, "Every Day");
        })
        it('should throw an exception, if the parameters are not consistent with field type', function () {
            assert.throws(function () {
                recSet2.filterByField("Name", 100, 120);
            })
        })
        it('should throw an exception, if a parameter is missing', function () {
            assert.throws(function () {
                recSet.filterByField("Rating", 5.5);
            })
        })
        it('should throw an exception, if the field is not given', function () {
            assert.throws(function () {
                recSet.filterByField(5.5, 5.7);
            })
        })
    });

    describe('Filter Tests', function () {
        it('should filter only the "Every Day" movie', function () {
            recSet.filter(function (rec) { return rec.Rating == 5.6; });
            assert.equal(recSet.length, 1);
            assert.equal(recSet[0].Title, "Every Day");
        })
        it('should filter the all with known gender people', function () {
            recSet2.filter(function (rec) { return rec.Gender == 'Male' || rec.Gender == 'Female' });
            assert.equal(recSet2.length, 60);
            assert.equal(recSet2[0].Name, "Carolina Fortuna");
        })
        it('should throw an exception, if no parameter is given', function () {
            assert.throws(function () {
                recSet2.filter();
            })
        })
        it('should throw an exception, if the callback function is incomplete', function () {
            assert.throws(function () {
                recSet2.filter(function (rec) { });
            })
        })
        it.skip('should throw an exception, if the callback function has no parameter', function () {
            assert.throws(function () {
                recSet2.filter(function () { return 1 == 1; });
            })
        })
    });

    describe('Split Tests', function () {
        it('should split the recSet into two record sets according by their Year', function () {
            var arr = recSet.split(function (rec, rec2) { return rec.Year - rec2.Year > 2; });
            assert.equal(arr.length, 2);

            assert.equal(arr[0].length, 1);
            assert.equal(arr[0][0].Title, "Every Day");

            assert.equal(arr[1].length, 1);
            assert.equal(arr[1][0].Title, "Enteng Kabisote 3: Okay ka fairy ko... The legend goes on and on and on");
        })
        it('should throw an exception if the callback function is not complete', function () {
            assert.throws(function () {
                var arr = recSet.split(function (rec, rec2) { });
            })
        })
    });

    describe('DeleteRecs Tests', function () {
        it('should delete the records in recSet2, that are also in rs', function () {
            var rs = recSet2.clone();
            rs.filterById(2, 61);

            recSet2.deleteRecs(rs);
            assert.equal(recSet2.length, 2);
            assert.equal(recSet2[0].Name, "Carolina Fortuna");
            assert.equal(recSet2[1].Name, "Blaz Fortuna");
        })
        it('should delete all the records in recSet2', function () {
            var rs = recSet2.clone();
            recSet2.deleteRecs(rs);
            assert.equal(recSet2.length, 0);
        })
        it('should throw an exception, if parameter is not given', function () {
            assert.throws(function () {
                recSet2.deleteRecs();
            })
        })
    });

    describe('Each Tests', function () {
        it('should change the gender of all records in "People" to "Extraterrestrial"', function () {
            recSet2.each(function (rec) {
                rec.Gender = "Extraterrestrial";
            });
            for (var i = 0; i < 62; i++) {
                assert.equal(recSet2[i].Gender, "Extraterrestrial");
            }
        })
        it('should change the gender only to "Carolina Fortuna"', function () {
            recSet2.each(function (rec, idx) {
                if (idx == 0) {
                    rec.Gender = "Extraterrestrial";
                }
            })
            assert.equal(recSet2[0].Gender, "Extraterrestrial");
            assert.equal(recSet2[1].Gender, "Male");
        })
        it('shouldn\'t create a new field', function () {
            recSet2.each(function (rec) { rec.DateOfBirth = '1.1.1950'; });
            for (var i = 0; i < 62; i++) {
                assert.equal(recSet2[i].DateOfBirth, null);
            }
        })
    });

    describe('Map Tests', function () {
        it('should return an array of movie titles', function () {
            var arr = recSet.map(function (rec) { return rec.Title; });
            assert.equal(arr.length, 2);
            assert.equal(arr[0], recSet[0].Title);
            assert.equal(arr[1], recSet[1].Title);
        })
        it('should return a nested array of names and indeces of people', function () {
            var arr = recSet2.map(function (rec, idx) {
                return [idx, rec.Name];
            })
            for (var i = 0; i < 62; i++) {
                assert.equal(arr[i][0], i);
                assert.equal(arr[i][1], recSet2[i].Name);
            }
        })
    });

    describe('Intersect Tests', function () {
        it('should return the intersection of two record sets', function () {
            var rs = recSet2.clone();
            rs.filterById(0, 1);
            var rs2 = recSet2.setintersect(rs);

            assert.equal(rs2.length, 2);
            assert.equal(rs2[0].Name, "Carolina Fortuna");
            assert.equal(rs2[1].Name, "Blaz Fortuna");
        })
        it('should return an empty record set if parameter is empty', function () {
            var rs = recSet2.clone();
            rs.trunc(0);    // make the record set empty 
            var rs2 = recSet2.setintersect(rs);

            assert.equal(rs2.length, 0);
        })
        it('should return an empty record set if the first record set is empty', function () {
            var rs = recSet2.clone();
            rs.trunc(0);    // make the record set empty
            var rs2 = rs.setintersect(recSet2);

            assert.equal(rs2.length, 0);
        })
        it('should throw an exception if no parameter is given', function () {
            assert.throws(function () {
                var rs = recSet2.setintersect();
            })
        })
    })
})