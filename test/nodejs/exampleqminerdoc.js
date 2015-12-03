require('qminer').la.Vector.prototype.print = function () { };require('qminer').la.SparseVector.prototype.print = function () { };require('qminer').la.SparseMatrix.prototype.print = function () { };require('qminer').la.Matrix.prototype.print = function () { };describe('example tests for the qminerdoc.js file', function () {
describe("Qminer module., number 1", function () {
it("should make test number 1", function () {
 
 // import module
 var qm = require('qminer');

});
});
describe("Store schema definition object, number 2", function () {
it("should make test number 2", function () {

 var qm = require('qminer');
 // create a simple movies store, where each record contains only the movie title.
 var base = new qm.Base({
     mode: 'createClean',
     schema: [{
       name: "Movies",
       fields: [{ name: "title", type: "string" }]
     }]
 });
 base.close();

});
});
describe("Store schema field definition object, number 3", function () {
it("should make test number 3", function () {

  var qm = require('qminer');
  var base = new qm.Base({
      mode: 'createClean',
      schema: [
        { name: 'NewsArticles',
          fields: [
            { name: "ID", primary: true, type: "string", shortstring: true },
            { name: "Source", type: "string", codebook: true },
            { name: "DateTime", type: "datetime" },
            { name: "Title", type: "string", store: "cache" },
            { name: "Tokens", type: "string_v", store: "cache", null: true },
            { name: "Vector", type: "num_sp_v", store: "cache", null: true }]
        }
     ]
  });
 // add a record:
 // - we set the date using the ISO string representation
 // - we set the string vector Tokens with an array of strings
 // - we set the numeric sparse vector Vector with an array of two element arrays
 //   (index, value), see the sparse vector constructor {@link module:la.SparseVector}
 base.store('NewsArticles').push({
   ID: 't12344', 
   Source: 's1234', 
   DateTime: '2015-01-01T00:05:00', 
   Title: 'the title', 
   Tokens: ['token1', 'token2'], 
   Vector: [[0,1], [1,1]]});
 base.close();

});
});
describe("Store schema join definition object, number 4", function () {
it("should make test number 4", function () {

 var qm = require('qminer');
 // Create two stores: People which stores only names of persons and Movies, which stores only titles.
 // Each person can direct zero or more movies, so we use an index join named 'directed' and
 // each movie has a single director, so we use a field join 'director'. The joins are 
 // inverses of each other. The inverse join simplifies the linking, since only one join needs
 // to be specified, and the other direction can be linked automatically (in the example 
 // below we specify only the 'director' link and the 'directed' join is updated automatically).
 //
 var base = new qm.Base({
     mode: 'createClean',
     schema: [
       { name: 'People', 
         fields: [{ name: 'name', type: 'string', primary: true }], 
         joins: [{ name: 'directed', 'type': 'index', 'store': 'Movies', 'inverse': 'director' }] },
       { name: 'Movies', 
         fields: [{ name: 'title', type: 'string', primary: true }], 
         joins: [{ name: 'director', 'type': 'field', 'store': 'People', 'inverse': 'directed' }] }
     ]
 });
 // Adds a movie, automatically adds 'Jim Jarmusch' to People, sets the 'director' join (field join)
 // and automatically updates the index join 'directed', since it's an inverse join of 'director'
 base.store('Movies').push({ title: 'Broken Flowers', director: { name: 'Jim Jarmusch' } });
 // Adds a movie, sets the 'director' join, updates the index join of 'Jim Jarmusch'
 base.store('Movies').push({ title: 'Coffee and Cigarettes', director: { name: 'Jim Jarmusch' } });
 // Adds movie, automatically adds 'Lars von Trier' to People, sets the 'director' join
 // and 'directed' inverse join (automatically)
 base.store('Movies').push({ title: 'Dogville', director: { name: 'Lars von Trier' } });

 var movie = base.store('Movies')[0]; // get the first movie (Broken Flowers)
 // Each movie has a property corresponding to the join name: 'director'. 
 // Accessing the property returns a {@link module:qm.Record} from the store People.
 var person = movie.director; // get the director
 var personName = person.name; // get person's name ('Jim Jarmusch')
 // Each person has a property corresponding to the join name: 'directed'. 
 // Accessing the property returns a {@link module:qm.RecSet} from the store People.
 var movies = person.directed; // get all the movies the person directed.
 movies.each(function (movie) { var title = movie.title; });
 // Gets the following titles:
 //   'Broken Flowers'
 //   'Coffee and Cigarettes'
 base.close();

});
});
describe("Store schema key definition object, number 5", function () {
it("should make test number 5", function () {

 var qm = require('qminer');
 // Create a store People which stores only names of persons.
 var base = new qm.Base({
     mode: 'createClean',
     schema: [
         { name: 'People',
           fields: [{ name: 'name', type: 'string', primary: true }],
           keys: [
             { field: 'name', type: 'value'}, 
             { field: 'name', name: 'nameText', type: 'text'}
          ]
        }
     ]
 });

 base.store('People').push({name : 'John Smith'});
 base.store('People').push({name : 'Mary Smith'});
 // search based on indexed values
 base.search({$from : 'People', name: 'John Smith'}); // Return the record set containing 'John Smith'
 // search based on indexed values
 base.search({$from : 'People', name: 'Smith'}); // Returns the empty record set.
 // search based on text indexing
 base.search({$from : 'People', nameText: 'Smith'}); // Returns both records.
 base.close();

});
});
describe("Stores can have a window, which is used by garbage collector to delete records once they, number 6", function () {
it("should make test number 6", function () {

 var qm = require('qminer');
 // Create a store
 // var base = new qm.Base([{
 // ...
 //  timeWindow : { 
 //    duration : 12,
 //    unit : "hour",
 //    field : "DateTime"
 //  }
 //}]);
 //base.close();

});
});
describe("Base, number 7", function () {
it("should make test number 7", function () {

 // import qm module
 var qm = require('qminer');
 // using a constructor, in open mode
 var base = new qm.Base({mode: 'open'});
 base.close();

});
});
describe("Returns the store with the specified name., number 8", function () {
it("should make test number 8", function () {

	  // import qm module
	  var qm = require('qminer');
	  // create a base with two stores
	  var base = new qm.Base({
	     mode: "createClean",
	     schema: [
	     {
	         name: "KwikEMart",
	         fields: [
	             { name: "Worker", type: "string" },
	             { name: "Groceries", type: "string_v" }
	         ]
	     },
	     {
	         name: "NuclearPowerplant",
	         fields: [
	             { name: "Owner", type: "string" },
	             { name: "NumberOfAccidents", type: "int" },
	             { name: "Workers", type: "string_v" }
	         ]
	     }]
	  });
	  // get the "KwikEMart" store 
	  var store = base.store("KwikEMart");	// returns the store with the name "KwikEMart"
	  base.close();
	 
});
});
describe("Creates a new store., number 9", function () {
it("should make test number 9", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base with one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [
	    {
	        name: "Superheroes",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "Superpowers", type: "string_v" },
	            { name: "YearsActive", type: "int" }
	        ]
	    }]
	 });
	 // create a new store called "Supervillains" in the base
	 base.createStore({
	    name: "Supervillians",
	    fields: [
	        { name: "Name", type: "string" },
	        { name: "Superpowers", type: "string_v" },
	        { name: "YearsActive", type: "int" }
	    ]
	 });
	 // create two new stores called "Cities" and "Leagues"
	 base.createStore([
	    {
	        name: "Cities",
	        fields: [
	            { name: "Name", type: "string", primary: true },
	            { name: "Population", type: "int" }
	        ]
	    },
	    {
	        name: "Leagues",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "Members", type: "string_v" }
	        ]
	    }
	 ]);
	 base.close();
	
});
});
describe("Stores are containers of records. , number 10", function () {
it("should make test number 10", function () {

 // import qm module
 var qm = require('qminer');
 // factory based construction using base.createStore
 var base = new qm.Base({ mode: 'createClean' });
 base.createStore([{
    name: "People",
    fields: [
        { name: "Name", type: "string", primary: true },
        { name: "Gender", type: "string", shortstring: true },
        { name: "Age", type: "int" }
    ],
    joins: [
        { name: "ActedIn", type: "index", store: "Movies", inverse: "Actor" },
        { name: "Directed", type: "index", store: "Movies", inverse: "Director" }
    ],
    keys: [
        { field: "Name", type: "text" },
        { field: "Gender", type: "value" }
    ]
 },
 {
    name: "Movies",
    fields: [
        { name: "Title", type: "string", primary: true },
        { name: "Plot", type: "string", store: "cache" },
        { name: "Year", type: "int" },
        { name: "Rating", type: "float" },
        { name: "Genres", type: "string_v", codebook: true }
    ],
    joins: [
        { name: "Actor", type: "index", store: "People", inverse: "ActedIn" },
        { name: "Director", type: "index", store: "People", inverse: "Directed" }
    ],
    keys: [
        { field: "Title", type: "value" },
        { field: "Plot", type: "text", vocabulary: "voc_01" },
        { field: "Genres", type: "value" }
    ]
 }]);
 base.close();

});
});
describe("Stores are containers of records. , number 11", function () {
it("should make test number 11", function () {

 // import qm module
 var qm = require('qminer');
 // using the base constructor
 var base = new qm.Base({
    mode: "createClean",
    schema: [{
        name: "Class",
        fields: [
            { name: "Name", type: "string" },
            { name: "StudyGroup", type: "string" }
        ]
    }]
 });
 base.close();

});
});
describe("Returns a record from the store., number 12", function () {
it("should make test number 12", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a base containing the store Class. Let the Name field be the primary field. 
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Class",
	        fields: [
	            { name: "Name", type: "string", primary: true },
	            { name: "StudyGroup", type: "string" }
	        ]
	    }]
	 });
	 // add some records to the store
	 base.store("Class").push({ Name: "Dean", StudyGroup: "A" });
	 base.store("Class").push({ Name: "Chang", StudyGroup: "D" });
	 base.store("Class").push({ Name: "Magnitude", StudyGroup: "C" });
	 base.store("Class").push({ Name: "Leonard", StudyGroup: "B" });
	 // get the record with the name "Magnitude"
	 var record = base.store("Class").recordByName("Magnitude");
	 base.close();
	
});
});
describe("Executes a function on each record in store., number 13", function () {
it("should make test number 13", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a base containing the store Class
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Class",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "StudyGroup", type: "string" }
	        ]
	    }]
	 });
	 // add some records to the store
	 base.store("Class").push({ Name: "Abed", StudyGroup: "A" });
	 base.store("Class").push({ Name: "Annie", StudyGroup: "B" });
	 base.store("Class").push({ Name: "Britta", StudyGroup: "C" });
	 base.store("Class").push({ Name: "Jeff", StudyGroup: "A" });
	 // change the StudyGroup of all records of store Class to A
	 base.store("Class").each(function (rec) { rec.StudyGroup = "A"; });	// all records in Class are now in study group A
	 base.close();
	
});
});
describe("Creates an array of function outputs created from the store records., number 14", function () {
it("should make test number 14", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a base containing the store Class
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Class",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "StudyGroup", type: "string" }
	        ]
	    }]
	 });
	 // add some records to the store
	 base.store("Class").push({ Name: "Shirley", StudyGroup: "A" });
	 base.store("Class").push({ Name: "Troy", StudyGroup: "B" });
	 base.store("Class").push({ Name: "Chang", StudyGroup: "C" });
	 base.store("Class").push({ Name: "Pierce", StudyGroup: "A" });
	 // make an array of record names
	 var arr = base.store("Class").map(function (rec) { return rec.Name; }); // returns an array ["Shirley", "Troy", "Chang", "Pierce"]
	 base.close();
	
});
});
describe("Adds a record to the store., number 15", function () {
it("should make test number 15", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing two stores
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [
	    {
	        name: "Superheroes",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "Superpowers", type: "string_v" }
	        ]
	    },
	    {
	        name: "Supervillians",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "Superpowers", type: "string_v" }
	        ]
	    }]
	 });
	 // add a new superhero to the Superheroes store
	 base.store("Superheroes").push({ Name: "Superman", Superpowers: ["flight", "heat vision", "bulletproof"] }); // returns 0
	 // add a new supervillian to the Supervillians store
	 base.store("Supervillians").push({ Name: "Lex Luthor", Superpowers: ["expert engineer", "genius-level intellect", "money"] }); // returns 0
	 base.close();	
	
});
});
describe("Creates a new record of given store. The record is not added to the store., number 16", function () {
it("should make test number 16", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Planets",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "Diameter", type: "int" },
	            { name: "NearestStars", type: "string_v" }
	        ]
	    }]
	 });
	 // add a new planet in the store
	 base.store("Planets").push({ Name: "Earth", Diameter: 299196522, NearestStars: ["Sun"] });
	 // create a record of a planet (not added to the Planets store)
	 var planet = base.store("Planets").newRecord({ Name: "Tatooine", Diameter: 10465, NearestStars: ["Tatoo 1", "Tatoo 2"] });
	 base.close();
	
});
});
describe("Creates a new record set out of the records in store., number 17", function () {
it("should make test number 17", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Superheroes",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "Superpowers", type: "string_v" }
	        ]
	    }]
	 });
	 // add some new records to the store
	 base.store("Superheroes").push({ Name: "Superman", Superpowers: ["Superhuman strength, speed, hearing", "Flight", "Heat vision"] });
	 base.store("Superheroes").push({ Name: "Batman", Superpowers: ["Genius-level intellect", "Peak physical and mental conditioning", "Master detective"] });
	 base.store("Superheroes").push({ Name: "Thor", Superpowers: ["Superhuman strength, endurance and longevity", "Abilities via Mjolnir"] });
	 base.store("Superheroes").push({ Name: "Wonder Woman", Superpowers: ["Superhuman strength, agility and endurance", "Flight", "Highly skilled hand-to-hand combatant"] });
	 // create a new record set containing only the DC Comic superheroes (those with the record ids 0, 1 and 3)
	 var intVec = new qm.la.IntVector([0, 1, 3]);
	 var DCHeroes = base.store("Superheroes").newRecordSet(intVec);
	 base.close();
	
});
});
describe("Creates a record set containing random records from store., number 18", function () {
it("should make test number 18", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "TVSeries",
	        fields: [
	            { name: "Title", type: "string", primary: true },
	            { name: "NumberOfEpisodes", type: "int" }
	        ]
	    }]
	 });
	 // add some records in the store
	 base.store("TVSeries").push({ Title: "Archer", NumberOfEpisodes: 75 });
	 base.store("TVSeries").push({ Title: "The Simpsons", NumberOfEpisodes: 574 });
	 base.store("TVSeries").push({ Title: "New Girl", NumberOfEpisodes: 94 });
	 base.store("TVSeries").push({ Title: "Rick and Morty", NumberOfEpisodes: 11 });
	 base.store("TVSeries").push({ Title: "Game of Thrones", NumberOfEpisodes: 47 });
	 // create a sample record set containing 3 records
	 var randomRecordSet = base.store("TVSeries").sample(3); // contains 3 random records from the TVSeries store
	 base.close();
	
});
});
describe("Gets the details of the selected field., number 19", function () {
it("should make test number 19", function () {

	 // import qm module
	 var qm = require("qminer");
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "People",
	        fields: [
	            { name: "Name", type: "string", primary: true },
	            { name: "Gender", type: "string" },
	            { name: "Age", type: "int" }
	        ]
	    }]
	 });
	 // get the details of the field "Name" of store "People"
	 // it returns a JSON object:
	 // { id: 0, name: "Name", type: "string", primary: true }
	 var details = base.store("People").field("Name");
	 base.close();
	
});
});
describe("Checks if the field is of numeric type., number 20", function () {
it("should make test number 20", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "TVSeries",
	        fields: [
	            { name: "Title", type: "string", primary: true },
	            { name: "NumberOfEpisodes", type: "int" }
	        ]
	    }]
	 });
	 // check if the field "Title" is of numeric type
	 var isTitleNumeric = base.store("TVSeries").isNumeric("Title"); // returns false
	 // check if the field "NumberOfEpisodes" is of numeric type
	 var isNumberOfEpisodesNumeric = base.store("TVSeries").isNumeric("NumberOfEpisodes"); // returns true
	 base.close();
	
});
});
describe("Checks if the field is of string type., number 21", function () {
it("should make test number 21", function () {

	 // import qm module
	 var qm = require("qminer");
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "People",
	        fields: [
	            { name: "Name", type: "string", primary: true },
	            { name: "Gender", type: "string" },
	            { name: "Age", type: "int" }
	        ]
	    }]
	 });
	 // check if the field "Name" is of string type
	 var isNameString = base.store("People").isString("Name"); // returns true
	 // check if the field "Age" is of string type
	 var isAgeString = base.store("People").isString("Age"); // returns false
	 base.close();
	
});
});
describe("Checks if the field is of type Date., number 22", function () {
it("should make test number 22", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "BasketballPlayers",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "SeasonScore", type: "int_v" },
	            { name: "DateOfBirth", type: "datetime" }
	        ]
	    }]
	 });
	 // check if the SeasonScore field is of type Date
	 var isSeasonScoreDate = base.store("BasketballPlayers").isDate("SeasonScore"); // returns false
	 // check if the FirstPlayed field is of type Date
	 var isFirstPlayedDate = base.store("BasketballPlayers").isDate("DateOfBirth"); // returns true
	 base.close();
	
});
});
describe("Returns the details of the selected key as a JSON object., number 23", function () {
it("should make test number 23", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Countries",
	        fields: [
	            { name: "Name", type: "string", primary: true },
	            { name: "Population", type: "int" },
	            { name: "Continent", type: "string" }
	        ],
	        keys: [
	            { field: "Name", type: "text" },
	            { field: "Continent", type: "value" }
	        ]
	    }]
	 });
	 // get the details of the key of the field "Continent"
	 // returns a JSON object containing the details of the key:
	 // { fq: { length: 0 }, vocabulary: { length: 0 }, name: 'Continent', store: { name: 'Countries', ... }}
	 var details = base.store("Countries").key("Continent");
	 base.close();
	
});
});
describe("Returns the store as a JSON., number 24", function () {
it("should make test number 24", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "FootballPlayers",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "FootballClubs", type: "string_v" },
	            { name: "GoalsPerSeason", type: "int_v" },
	        ]
	    }]
	 });
	 // get the store as a JSON object
	 // the returned JSON object is:
	 // { storeId: 0, storeName: 'FootballPlayers', storeRecords: 0, fields: [...], keys: [], joins: [] }
	 var json = base.store("FootballPlayers").toJSON();
	 base.close();
	
});
});
describe("Deletes the records in the store., number 25", function () {
it("should make test number 25", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "TVSeries",
	        fields: [
	            { name: "Title", type: "string", primary: true },
	            { name: "NumberOfEpisodes", type: "int" }
	        ]
	    }]
	 });
	 // add some records in the store
	 base.store("TVSeries").push({ Title: "Archer", NumberOfEpisodes: 75 });
	 base.store("TVSeries").push({ Title: "The Simpsons", NumberOfEpisodes: 574 });
	 base.store("TVSeries").push({ Title: "New Girl", NumberOfEpisodes: 94 });
	 base.store("TVSeries").push({ Title: "Rick and Morty", NumberOfEpisodes: 11 });
	 base.store("TVSeries").push({ Title: "Game of Thrones", NumberOfEpisodes: 47 });
	 // deletes the first 2 records (Archer and The Simpsons) in TVSeries
	 base.store("TVSeries").clear(2); // returns 3
	 // delete all remaining records in TVStore
	 base.store("TVSeries").clear();  // returns 0
	 base.close();
	
});
});
describe("Gives a vector containing the field value of each record., number 26", function () {
it("should make test number 26", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Companies",
	        fields: [
	            { name: "Name", type: "string", primary: true },
	            { name: "Location", type: "string" }
	        ]
	    }]
	 });
	 // add some records to the store
	 base.store("Companies").push({ Name: "DC Comics", Location: "Burbank, California" });
	 base.store("Companies").push({ Name: "DC Shoes", Location: "Huntington Beach, California" });
	 base.store("Companies").push({ Name: "21st Century Fox", Location: "New York City, New York" });
	 // get the vector of company names
	 var companyNames = base.store("Companies").getVector("Name");	// returns a vector ["DC Comics", "DC Shoes", "21st Century Fox"]
	 base.close();
	
});
});
describe("Gives a matrix containing the field values of each record., number 27", function () {
it("should make test number 27", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "ArcheryChampionship",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "ScorePerRound", type: "float_v" }
	        ]
	    }]
	 });
	 // set new records in the store
	 base.store("ArcheryChampionship").push({ Name: "Robin Hood", ScorePerRound: [50, 48, 48] });
	 base.store("ArcheryChampionship").push({ Name: "Oliver Queen", ScorePerRound: [44, 46, 44] });
	 base.store("ArcheryChampionship").push({ Name: "Legolas", ScorePerRound: [50, 50, 48] });
	 // get the matrix containing the "score per round" values
	 // The values of the i-th column are the values of the i-th record.
	 // The function will give the matrix:
	 // 50  44  50
	 // 48  46  50
	 // 48  44  48
	 var matrix = base.store("ArcheryChampionship").getMatrix("ScorePerRound");
	 base.close();
	
});
});
describe("Gives the field value of a specific record., number 28", function () {
it("should make test number 28", function () {

	 //import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Festivals",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "Type", type: "string" },
	            { name: "Location", type: "string" }
	        ]
	    }]
	 });
	 // add some records in the store
	 base.store("Festivals").push({ Name: "Metaldays", Type: "music", Location: "Tolmin, Slovenia" });
	 base.store("Festivals").push({ Name: "Festival de Cannes", Type: "movie", Location: "Cannes, France" });
	 base.store("Festivals").push({ Name: "The Festival of Chocolate", Type: "food", Location: "Hillsborough, USA" });
	 // get the field value of the second record for field "Type"
	 var fieldValue = base.store("Festivals").cell(1, "Type"); // returns "movie"
	 base.close();
	
});
});
describe("Clones the record., number 29", function () {
it("should make test number 29", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "StarWarsMovies",
	        fields: [
	            { name: "Title", type: "string" },
	            { name: "ReleseDate", type: "datetime" },
	            { name: "Length", type: "int" }
	        ]
	    }]
	 });
	 // create some records in the new store
	 base.store("StarWarsMovies").push({ Title: "Attack of the Clones", ReleseDate: "2002-05-16T00:00:00", Length: 142 });
	 base.store("StarWarsMovies").push({ Title: "The Empire Strikes Back", ReleseDate: "1980-06-20T00:00:00", Length: 124 });
	 base.store("StarWarsMovies").push({ Title: "Return of the Jedi", ReleseDate: "1983-05-25T00:00:00", Length: 134 });
	 // create a clone of the "Attack of the Clones" record
	 var clone = base.store("StarWarsMovies")[0].$clone();
	 base.close();
	
});
});
describe("Creates a JSON version of the record., number 30", function () {
it("should make test number 30", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Musicians",
	        fields: [
	            { name: "Name", type: "string", primary: true },
	            { name: "DateOfBirth", type: "datetime" },
	            { name: "GreatestHits", type: "string_v" }
	        ]
	    }]
	 });
	 // create some records
	 base.store("Musicians").push({ Name: "Jimmy Page", DateOfBirth:  "1944-01-09T00:00:00", GreatestHits: ["Stairway to Heaven", "Whole Lotta Love"] });
	 base.store("Musicians").push({ Name: "Beyonce", DateOfBirth: "1981-09-04T00:00:00", GreatestHits: ["Single Ladies (Put a Ring on It)"] });
	 // get a JSON version of the "Beyonce" record 
	 // The JSON object for this example si:
	 // { '$id': 1, Name: 'Beyonce', ActiveSince: '1981-09-04T00:00:00', GreatestHits: ['Single Ladies (Put a Ring on It)'] }
	 var json = base.store("Musicians").recordByName("Beyonce").toJSON();
	 base.close();
	
});
});
describe("Creates a new instance of the record set., number 31", function () {
it("should make test number 31", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Philosophers",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "Era", type: "string" }
	        ]
	    }]
	 });
	 // put some records in the store
	 base.store("Philosophers").push({ Name: "Plato", Era: "Ancient philosophy" });
	 base.store("Philosophers").push({ Name: "Immanuel Kant", Era: "18th-century philosophy" });
	 base.store("Philosophers").push({ Name: "Emmanuel Levinas", Era: "20th-century philosophy" });
	 base.store("Philosophers").push({ Name: "Rene Descartes", Era: "17th-century philosophy" });
	 base.store("Philosophers").push({ Name: "Confucius", Era: "Ancient philosophy" });
	 // create a record set out of the records in store
	 var recordSet = base.store("Philosophers").allRecords;
	 // clone the record set of the "Philosophers" store
	 var philosophers = recordSet.clone();
	 base.close();
	
});
});
describe("Creates a new record set out of the join attribute of records., number 32", function () {
it("should make test number 32", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing two stores, with join attributes
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [
	    {
	        name: "Musicians",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "Instruments", type: "string_v" }
	        ],
	        joins: [
	            { name: "PlaysIn", type: "index", store: "Bands", inverse: "Members" }
	        ]
	    },
	    {
	        name: "Bands",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "Genre", type: "string" }
	        ],
	        joins: [
	            { name: "Members", type: "index", store: "Musicians", inverse: "PlaysIn" }
	        ]
	    }]
	 });
	 // add some new records to both stores
	 base.store("Musicians").push({ Name: "Robert Plant", Instruments: ["Vocals"], PlaysIn: [{Name: "Led Zeppelin", "Genre": "Rock" }] });
	 base.store("Musicians").push({ Name: "Jimmy Page", Instruments: ["Guitar"], PlaysIn: [{Name: "Led Zeppelin", "Genre": "Rock" }] });
	 base.store("Bands").push({ Name: "The White Stripes", Genre: "Rock" });
	 // create a record set containing the musicians, that are members of some bend
	 // returns a record set containing the records of "Robert Plant" and "Jimmy Page"
	 var ledZeppelin = base.store("Bands").allRecords.join("Members");
	 // create a record set containing the first musician, that is a member of some band
	 // returns a record set containing only one record, which is "Robert Plant" or "Jimmy Page"
	 var ledMember = base.store("Bands").allRecords.join("Members", 1);
	 base.close();
	
});
});
describe("Truncates the first records., number 33", function () {
it("should make test number 33", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Philosophers",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "Era", type: "string" }
	        ]
	    }]
	 });
	 // put some records in the store
	 base.store("Philosophers").push({ Name: "Plato", Era: "Ancient philosophy" });
	 base.store("Philosophers").push({ Name: "Immanuel Kant", Era: "18th-century philosophy" });
	 base.store("Philosophers").push({ Name: "Emmanuel Levinas", Era: "20th-century philosophy" });
	 base.store("Philosophers").push({ Name: "Rene Descartes", Era: "17th-century philosophy" });
	 base.store("Philosophers").push({ Name: "Confucius", Era: "Ancient philosophy" });
	 // create two identical record sets of the "Philosophers" store
	 var recordSet1 = base.store("Philosophers").allRecords;
	 var recordSet2 = base.store("Philosophers").allRecords;
	 // truncate the first 3 records in recordSet1
	 recordSet1.trunc(3); // return self, containing only the first 3 records ("Plato", "Immanuel Kant", "Emmanuel Levinas")
	 // truncate the first 2 records in recordSet2, starting with "Emmanuel Levinas"
	 recordSet2.trunc(2, 2); // returns self, containing only the 2 records ("Emmanuel Levinas", "Rene Descartes")
	 base.close();
	
});
});
describe("Creates a random sample of records of the record set., number 34", function () {
it("should make test number 34", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base with one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Movies",
	        fields: [
	            { name: "Title", type: "string" },
	            { name: "Length", type: "int" },
	            { name: "Director", type: "string" }
	        ]
	    }]
	 });
	 // put some records in the store
	 base.store("Movies").push({ Title: "The Nightmare Before Christmas", Length: 76, Director: "Henry Selick" });
	 base.store("Movies").push({ Title: "Jurassic Part", Length: 127, Director: "Steven Spielberg" });
	 base.store("Movies").push({ Title: "The Avengers", Length: 143, Director: "Joss Whedon" });
	 base.store("Movies").push({ Title: "The Clockwork Orange", Length: 136, Director: "Stanley Kubrick" });
	 base.store("Movies").push({ Title: "Full Metal Jacket", Length: 116, Director: "Stanely Kubrick" });
	 // create a sample record set of containing 3 records from the "Movies" store
	 var sample = base.store("Movies").allRecords.sample(3);
	 base.close();
	
});
});
describe("Shuffles the order of records in the record set., number 35", function () {
it("should make test number 35", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "WeatherForcast",
	        fields: [
	            { name: "Weather", type: "string" },
	            { name: "Date", type: "datetime" },
	            { name: "TemperatureDegrees", type: "int" }
	        ]
	    }]
	 });
	 // put some records in the "WeatherForecast" store
	 base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-27T11:00:00", TemperatureDegrees: 19 });
	 base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-28T11:00:00", TemperatureDegrees: 22 });
	 base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-29T11:00:00", TemperatureDegrees: 25 });
	 base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-30T11:00:00", TemperatureDegrees: 25 });
	 base.store("WeatherForcast").push({ Weather: "Scattered Showers", Date: "2015-05-31T11:00:00", TemperatureDegrees: 24 });
	 base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-06-01T11:00:00", TemperatureDegrees: 27 });
	 // get the record set containing the records from the "WeatherForcast" store
	 var recordSet = base.store("WeatherForcast").allRecords;
	 // shuffle the records in the newly created record set. Use the number 100 as the seed for the shuffle
	 recordSet.shuffle(100); // returns self, the records in the record set are shuffled
	 base.close();
	
});
});
describe("It reverses the record order., number 36", function () {
it("should make test number 36", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "WeatherForcast",
	        fields: [
	            { name: "Weather", type: "string" },
	            { name: "Date", type: "datetime" },
	            { name: "TemperatureDegrees", type: "int" },
	        ]
	    }]
	 });
	 // put some records in the "WeatherForecast" store
	 base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-27T11:00:00", TemperatureDegrees: 19 });
	 base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-28T11:00:00", TemperatureDegrees: 22 });
	 base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-29T11:00:00", TemperatureDegrees: 25 });
	 base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-30T11:00:00", TemperatureDegrees: 25 });
	 base.store("WeatherForcast").push({ Weather: "Scattered Showers", Date: "2015-05-31T11:00:00", TemperatureDegrees: 24 });
	 base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-06-01T11:00:00", TemperatureDegrees: 27 });
	 // get the record set containing the records from the "WeatherForcast" store
	 var recordSet = base.store("WeatherForcast").allRecords;
	 // reverse the record order in the record set
	 recordSet.reverse(); // returns self, the records in the record set are in the reverse order
	 base.close();
	
});
});
describe("Sorts the records according to record id., number 37", function () {
it("should make test number 37", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Tea",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "Type", type: "string"},
	            { name: "Origin", type: "string", "null": true  }
	        ]
	    }]
	 });
	 // put some records in the "Tea" store
	 base.store("Tea").push({ Name: "Tanyang Gongfu", Type: "Black", Origin: "Tanyang" });
	 base.store("Tea").push({ Name: "Rou Gui", Type: "White" });
	 base.store("Tea").push({ Name: "Tieluohan Tea", Type: "Wuyi", Origin: "Northern Fujian" });
	 base.store("Tea").push({ Name: "Red Robe", Type: "Oolong", Origin: "Wuyi Mountains" });
	 // get the records of the "Tea" store as a record set
	 var recordSet = base.store("Tea").allRecords;
	 // sort the records in the record set by their id in descending order
	 recordSet.sortById(); // returns self, the records are sorted in descending order (default)
	 // sort the records in the record set by their id in ascending order
	 recordSet.sortById(1); // returns self, the records are sorted in ascending order
	 base.close();
	
});
});
describe("Sorts the records according to a specific record field., number 38", function () {
it("should make test number 38", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "TVSeries",
	        fields: [
	            { name: "Title", type: "string", primary: true },
	            { name: "NumberOfEpisodes", type: "int" }
	        ]
	    }]
	 });
	 // add some records in the store
	 base.store("TVSeries").push({ Title: "Archer", NumberOfEpisodes: 75 });
	 base.store("TVSeries").push({ Title: "The Simpsons", NumberOfEpisodes: 574 });
	 base.store("TVSeries").push({ Title: "New Girl", NumberOfEpisodes: 94 });
	 base.store("TVSeries").push({ Title: "Rick and Morty", NumberOfEpisodes: 11 });
	 base.store("TVSeries").push({ Title: "Game of Thrones", NumberOfEpisodes: 47 });
	 // get the records of the "TVSeries" store as a record set
	 var recordSet = base.store("TVSeries").allRecords;
	 // sort the records by their "Title" field in ascending order 
	 recordSet.sortByField("Title", true); // returns self, record are sorted by their "Title"
	 base.close();
	
});
});
describe("Sorts the records according to the given callback function., number 39", function () {
it("should make test number 39", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "TVSeries",
	        fields: [
	            { name: "Title", type: "string", primary: true },
	            { name: "NumberOfEpisodes", type: "int" }
	        ]
	    }]
	 });
	 // add some records in the store
	 base.store("TVSeries").push({ Title: "Archer", NumberOfEpisodes: 75 });
	 base.store("TVSeries").push({ Title: "The Simpsons", NumberOfEpisodes: 574 });
	 base.store("TVSeries").push({ Title: "New Girl", NumberOfEpisodes: 94 });
	 base.store("TVSeries").push({ Title: "Rick and Morty", NumberOfEpisodes: 11 });
	 base.store("TVSeries").push({ Title: "Game of Thrones", NumberOfEpisodes: 47 });
	 // get the records of the "TVSeries" store as a record set
	 var recordSet = base.store("TVSeries").allRecords;
	 // sort the records by their number of episodes
	 recordSet.sort(function (rec, rec2) { return rec.NumberOfEpisodes < rec2.NumberOfEpisodes; }); // returns self, records are sorted by the number of episodes
	 base.close();
	
});
});
describe("Keeps only records with ids between or equal two values., number 40", function () {
it("should make test number 40", function () {

	 // import qm require
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "FrankSinatraGreatestHits",
	        fields: [
	            { name: "Title", type: "string" },
	            { name: "Length", type: "int" }
	        ]
	    }]
	 });
	 // put some records in the "FrankSinatraGreatesHits" store
	 base.store("FrankSinatraGreatestHits").push({ Title: "Strangers in the Night", Length: 145 });
	 base.store("FrankSinatraGreatestHits").push({ Title: "Summer Wind", Length: 173 });
	 base.store("FrankSinatraGreatestHits").push({ Title: "It Was a Very Good Year", Length: 265 });
	 base.store("FrankSinatraGreatestHits").push({ Title: "Somewhere in Your Heart", Length: 146 });
	 base.store("FrankSinatraGreatestHits").push({ Title: "Forget Domani", Length: 156 });
	 base.store("FrankSinatraGreatestHits").push({ Title: "Somethin' Stupid", Length: 155 });
	 base.store("FrankSinatraGreatestHits").push({ Title: "This Town", Length: 186 });
	 // get the records of the store as a record set
	 var recordSet = base.store("FrankSinatraGreatestHits").allRecords;
	 // from the record set keep the records with indeces between or equal 2 and 5
	 recordSet.filterById(2, 5);
	 base.close();
	
});
});
describe("Keeps only the records with a specific value of some field., number 41", function () {
it("should make test number 41", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "WeatherForcast",
	        fields: [
	            { name: "Weather", type: "string" },
	            { name: "Date", type: "datetime" },
	            { name: "TemperatureDegrees", type: "int" },
	        ]
	    }]
	 });
	 // put some records in the "WeatherForecast" store
	 base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-27T11:00:00", TemperatureDegrees: 19 });
	 base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-28T11:00:00", TemperatureDegrees: 22 });
	 base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-29T11:00:00", TemperatureDegrees: 25 });
	 base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-30T11:00:00", TemperatureDegrees: 25 });
	 base.store("WeatherForcast").push({ Weather: "Scattered Showers", Date: "2015-05-31T11:00:00", TemperatureDegrees: 24 });
	 base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-06-01T11:00:00", TemperatureDegrees: 27 });
	 // get the record set containing the records from the "WeatherForcast" store
	 var recordSet = base.store("WeatherForcast").allRecords;
	 // filter only the records, where the weather is Mostly Cloudy
	 recordSet.filterByField("Weather", "Mostly Cloudy"); // returns self, containing only the records, where the weather is "Mostly Cloudy"
	 base.close();
	
});
});
describe("Keeps only the records that pass the callback function., number 42", function () {
it("should make test number 42", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "ArcheryChampionship",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "ScorePerRound", type: "float_v" }
	        ]
	    }]
	 });
	 // set new records in the store
	 base.store("ArcheryChampionship").push({ Name: "Robin Hood", ScorePerRound: [50, 48, 48] });
	 base.store("ArcheryChampionship").push({ Name: "Oliver Queen", ScorePerRound: [44, 46, 44] });
	 base.store("ArcheryChampionship").push({ Name: "Legolas", ScorePerRound: [50, 50, 48] });
	 // create a record set out of the records of the store
	 var recordSet = base.store("ArcheryChampionship").allRecords;
	 // filter the records: which archers have scored 48 points in the third round
	 recordSet.filter(function (rec) { return rec.ScorePerRound[2] == 48; }); // keeps only the records, where the score of the third round is equal 48
	 base.close();
	
});
});
describe("Splits the record set into smaller record sets., number 43", function () {
it("should make test number 43", function () {

	 // import qm module
	 var qm = require("qminer");
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "SocialGames",
	        fields: [
	            { name: "Title", type: "string" },
	            { name: "Type", type: "string" },
	            { name: "MinPlayers", type: "int" },
	            { name: "MaxPlayers", type: "int" }
	        ]
	    }]
	 });
	 // set new records in the store
	 base.store("SocialGames").push({ Title: "DungeonsAndDragons", Type: "Role-Playing", MinPlayers: 5, MaxPlayers: 5 });
	 base.store("SocialGames").push({ Title: "Dobble", Type: "Card", MinPlayers: 2, MaxPlayers: 8 });
	 base.store("SocialGames").push({ Title: "Settlers of Catan", Type: "Board", MinPlayers: 3, MaxPlayers: 4 });
	 base.store("SocialGames").push({ Title: "Munchkin", Type: "Card", MinPlayers: 3, MaxPlayers: 6 });
	 // create a record set out of the records of the store
	 var recordSet = base.store("SocialGames").allRecords;
	 // sort the records by MinPlayers in ascending order
	 recordSet.sortByField("MinPlayers", true);
	 // split the record set by the minimum number of players
	 // returns an array containing three record sets: the first containing the "DungeonsAndDragons" record,
	 // the second containing the "Settlers of Catan" and "Munchkin" records and the third containing the 
	 // "Dobble" record
	 var arr = recordSet.split(function (rec, rec2) { return rec.MinPlayers < rec2.MinPlayers; });
	 base.close();
	
});
});
describe("Deletes the records, that are also in the other record set., number 44", function () {
it("should make test number 44", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "BookWriters",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "Genre", type: "string" },
	            { name: "Books", type: "string_v" }
	        ]
	    }]
	 });
	 // set new records in the store
	 base.store("BookWriters").push({ Name: "Terry Pratchett", Genre: "Fantasy", Books: ["The Colour of Magic", "Going Postal", "Mort", "Guards! Guards!"] });
	 base.store("BookWriters").push({ Name: "Douglas Adams", Genre: "Sci-fi", Books: ["The Hitchhiker's Guide to the Galaxy", "So Long, and Thanks for All the Fish"] });
	 base.store("BookWriters").push({ Name: "Fyodor Dostoyevsky", Genre: "Drama", Books: ["Crime and Punishment", "Demons"] });
	 base.store("BookWriters").push({ Name: "J.R.R. Tolkien", Genre: "Fantasy", Books: ["The Hobbit", "The Two Towers", "The Silmarillion" ] });
	 base.store("BookWriters").push({ Name: "George R.R. Martin", Genre: "Fantasy", Books: ["A Game of Thrones", "A Feast of Crows"] });
	 base.store("BookWriters").push({ Name: "J. K. Rowling", Genre: "Fantasy", Books: ["Harry Potter and the Philosopher's Stone"] });
	 base.store("BookWriters").push({ Name: "Ivan Cankar", Genre: "Drama", Books: ["On the Hill", "The King of Betajnova", "The Serfs"] });
	 // create one record set containing all records of store
	 var recordSet = base.store("BookWriters").allRecords;
	 // create one record set containing the records with genre "Fantasy"
	 var fantasy = base.store("BookWriters").allRecords.filterByField("Genre", "Fantasy");
	 // delete the records in recordSet, that are also in fantasy
	 recordSet.deleteRecords(fantasy); // returns self, containing only three records: "Douglas Adams", "Fyodor Dostoyevsky" and "Ivan Cankar"
	 base.close();
	
});
});
describe("Returns the record set as a JSON., number 45", function () {
it("should make test number 45", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Musicians",
	        fields: [
	            { name: "Name", type: "string", primary: true },
	            { name: "DateOfBirth", type: "datetime" },
	            { name: "GreatestHits", type: "string_v" }
	        ]
	    }]
	 });
	 // create some records
	 base.store("Musicians").push({ Name: "Jimmy Page", DateOfBirth:  "1944-01-09T00:00:00", GreatestHits: ["Stairway to Heaven", "Whole Lotta Love"] });
	 base.store("Musicians").push({ Name: "Beyonce", DateOfBirth: "1981-09-04T00:00:00", GreatestHits: ["Single Ladies (Put a Ring on It)"] });
	 // create a record set out of the records in the "Musicians" store
	 var recordSet = base.store("Musicians").allRecords;
	 // create a JSON object out of the record set
	 var json = recordSet.toJSON();
	 base.close();
	
});
});
describe("Executes a function on each record in record set., number 46", function () {
it("should make test number 46", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "People",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "Gender", type: "string" }
	        ]
	    }]
	 });
	 // put some records in the store
	 base.store("People").push({ Name: "Eric Sugar", Gender: "Male" });
	 base.store("People").push({ Name: "Jane Tokyo", Gender: "Female" });
	 base.store("People").push({ Name: "Mister Tea", Gender: "Male" });
	 // create a record set out of the records of the store
	 var recordSet = base.store("People").allRecords;
	 // change the Name of all records into "Anonymous"
	 recordSet.each(function (rec) { rec.Name = "Anonymous"; }); // returns self, all record's Name are "Anonymous"
	 base.close();
	
});
});
describe("Creates an array of function outputs created from the records in record set., number 47", function () {
it("should make test number 47", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "People",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "Gender", type: "string" }
	        ]
	    }]
	 });
	 // put some records in the store
	 base.store("People").push({ Name: "Eric Sugar", Gender: "Male" });
	 base.store("People").push({ Name: "Jane Tokyo", Gender: "Female" });
	 base.store("People").push({ Name: "Mister Tea", Gender: "Male" });
	 // create a record set out of the records of the store
	 var recordSet = base.store("People").allRecords;
	 // make an array of record Names
	 var arr = recordSet.map(function (rec) { return rec.Name; }); // returns an array: ["Eric Sugar", "Jane Tokyo", "Mister Tea"]
	 base.close();
	
});
});
describe("Creates the set intersection of two record sets., number 48", function () {
it("should make test number 48", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base with one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Movies",
	        fields: [
	            { name: "Title", type: "string" },
	            { name: "Length", type: "int" },
	            { name: "Director", type: "string" }
	        ]
	    }]
	 });
	 // put some records in the store
	 base.store("Movies").push({ Title: "The Nightmare Before Christmas", Length: 76, Director: "Henry Selick" });
	 base.store("Movies").push({ Title: "Jurassic Part", Length: 127, Director: "Steven Spielberg" });
	 base.store("Movies").push({ Title: "The Avengers", Length: 143, Director: "Joss Whedon" });
	 base.store("Movies").push({ Title: "The Clockwork Orange", Length: 136, Director: "Stanley Kubrick" });
	 base.store("Movies").push({ Title: "Full Metal Jacket", Length: 116, Director: "Stanely Kubrick" });
	 // create a record set out of the records in store, where length of the movie is greater than 110
	 var greaterSet = base.store("Movies").allRecords.filterByField("Length", 110, 150);
	 // create a record set out of the records in store, where the length of the movie is lesser than 130
	 var lesserSet = base.store("Movies").allRecords.filterByField("Length", 0, 130);
	 // get the intersection of greaterSet and lesserSet
	 var intersection = greaterSet.setIntersect(lesserSet); // returns a record set, containing the movies with lengths between 110 and 130
	 base.close();
	
});
});
describe("Creates the set union of two record sets., number 49", function () {
it("should make test number 49", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "TVSeries",
	        fields: [
	            { name: "Title", type: "string", "primary": true },
	            { name: "NumberOfEpisodes", type: "int" }
	        ]
	    }]
	 });
	 // add some records in the store
	 base.store("TVSeries").push({ Title: "Archer", NumberOfEpisodes: 75 });
	 base.store("TVSeries").push({ Title: "The Simpsons", NumberOfEpisodes: 574 });
	 base.store("TVSeries").push({ Title: "New Girl", NumberOfEpisodes: 94 });
	 base.store("TVSeries").push({ Title: "Rick and Morty", NumberOfEpisodes: 11 });
	 base.store("TVSeries").push({ Title: "Game of Thrones", NumberOfEpisodes: 47 });
	 // create a record set out of the records in store, where the number of episodes is lesser than 47
	 var lesserSet = base.store("TVSeries").allRecords.filterByField("NumberOfEpisodes", 0, 47);
	 // create a record set out of the records in store, where the number of episodes is greater than 100
	 var greaterSet = base.store("TVSeries").allRecords.filterByField("NumberOfEpisodes", 100, 600);
	 // get the union of lesserSet and greaterSet
	 var union = lesserSet.setUnion(greaterSet); // returns a record set, which is the union of the two record sets
	 base.close();
	
});
});
describe("Creates the set difference between two record sets., number 50", function () {
it("should make test number 50", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "BookWriters",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "Genre", type: "string" },
	            { name: "Books", type: "string_v" }
	        ]
	    }]
	 });
	 // set new records in the store
	 base.store("BookWriters").push({ Name: "Terry Pratchett", Genre: "Fantasy", Books: ["The Colour of Magic", "Going Postal", "Mort", "Guards! Guards!"] });
	 base.store("BookWriters").push({ Name: "Douglas Adams", Genre: "Sci-fi", Books: ["The Hitchhiker's Guide to the Galaxy", "So Long, and Thanks for All the Fish"] });
	 base.store("BookWriters").push({ Name: "Fyodor Dostoyevsky", Genre: "Drama", Books: ["Crime and Punishment", "Demons"] });
	 base.store("BookWriters").push({ Name: "J.R.R. Tolkien", Genre: "Fantasy", Books: ["The Hobbit", "The Two Towers", "The Silmarillion" ] });
	 base.store("BookWriters").push({ Name: "George R.R. Martin", Genre: "Fantasy", Books: ["A Game of Thrones", "A Feast of Crows"] });
	 base.store("BookWriters").push({ Name: "J. K. Rowling", Genre: "Fantasy", Books: ["Harry Potter and the Philosopher's Stone"] });
	 base.store("BookWriters").push({ Name: "Ivan Cankar", Genre: "Drama", Books: ["On the Hill", "The King of Betajnova", "The Serfs"] });
	 // create one record set containing all records of store
	 var recordSet = base.store("BookWriters").allRecords;
	 // create one record set containing the records with genre "Fantasy"
	 var fantasy = base.store("BookWriters").allRecords.filterByField("Genre", "Fantasy");
	 // create a new record set containing the difference of recordSet and fantasy
	 var difference = recordSet.setDiff(fantasy); // returns a record set, containing the records of Douglas Adams, Fyodor Dostoyevsky and Ivan Cankar
	 base.close();
	
});
});
describe("Creates a vector containing the field values of records., number 51", function () {
it("should make test number 51", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "TVSeries",
	        fields: [
	            { name: "Title", type: "string", "primary": true },
	            { name: "NumberOfEpisodes", type: "int" }
	        ]
	    }]
	 });
	 // add some records in the store
	 base.store("TVSeries").push({ Title: "Archer", NumberOfEpisodes: 75 });
	 base.store("TVSeries").push({ Title: "The Simpsons", NumberOfEpisodes: 574 });
	 base.store("TVSeries").push({ Title: "New Girl", NumberOfEpisodes: 94 });
	 base.store("TVSeries").push({ Title: "Rick and Morty", NumberOfEpisodes: 11 });
	 base.store("TVSeries").push({ Title: "Game of Thrones", NumberOfEpisodes: 47 });
	 // create a record set of the records of store
	 var recordSet = base.store("TVSeries").allRecords;
	 // create a vector containing the number of episodes for each series
	 // the vector will look like [75, 574, 94, 11, 47]
	 var vector = recordSet.getVector("NumberOfEpisodes");
	 base.close();
	
});
});
describe("Creates a vector containing the field values of records., number 52", function () {
it("should make test number 52", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "ArcheryChampionship",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "ScorePerRound", type: "float_v" }
	        ]
	    }]
	 });
	 // set new records in the store
	 base.store("ArcheryChampionship").push({ Name: "Robin Hood", ScorePerRound: [50, 48, 48] });
	 base.store("ArcheryChampionship").push({ Name: "Oliver Queen", ScorePerRound: [44, 46, 44] });
	 base.store("ArcheryChampionship").push({ Name: "Legolas", ScorePerRound: [50, 50, 48] });
	 // create a record set of the records in store
	 var recordSet = base.store("ArcheryChampionship").allRecords;
	 // create a matrix from the "ScorePerRound" field
	 // the i-th column of the matrix is the data of the i-th record in record set
	 // the matrix will look like
	 // 50  44  50
	 // 48  46  50
	 // 48  44  48
	 var matrix = recordSet.getMatrix("ScorePerRound");
	 base.close();
	
});
});
describe("Store Iterators allows you to iterate through the records in the store., number 53", function () {
it("should make test number 53", function () {

 // import qm module
 qm = require('qminer');
 // create a new base with a simple store
 var base = new qm.Base({ mode: "createClean" });
 base.createStore({
     name: "People",
     fields: [
         { name: "Name", type: "string" },
         { name: "Gender", type: "string" }
     ]
 });
 // add new records to the store
 base.store("People").push({ Name: "Geronimo", Gender: "Male" });
 base.store("People").push({ Name: "Pochahontas", Gender: "Female" });
 base.store("People").push({ Name: "John Rolfe", Gender: "Male" });
 base.store("People").push({ Name: "John Smith", Gender: "Male"});
 // factory based construction with forwardIter
 var iter = base.store("People").forwardIter;
 base.close();

});
});
describe("Moves to the next record., number 54", function () {
it("should make test number 54", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "TheWitcherSaga",
	        fields: [
	            { name: "Title", type: "string" },
	            { name: "YearOfRelease", type: "int" },
	            { name: "EnglishEdition", type: "bool" }
	        ]
	    }]
	 });
	 // put some records in the store
	 base.store("TheWitcherSaga").push({ Title: "Blood of Elves", YearOfRelease: 1994, EnglishEdition: true });
	 base.store("TheWitcherSaga").push({ Title: "Time of Contempt", YearOfRelease: 1995, EnglishEdition: true });
	 base.store("TheWitcherSaga").push({ Title: "Baptism of Fire", YearOfRelease: 1996, EnglishEdition: true });
	 base.store("TheWitcherSaga").push({ Title: "The Swallow's Tower", YearOfRelease: 1997, EnglishEdition: false });
	 base.store("TheWitcherSaga").push({ Title: "Lady of the Lake", YearOfRelease: 1999, EnglishEdition: false });
	 base.store("TheWitcherSaga").push({ Title: "Season of Storms", YearOfRelease: 2013, EnglishEdition: false });
	 // create an iterator for the store
	 var iter = base.store("TheWitcherSaga").forwardIter;
	 // go to the first record in the store
	 iter.next(); // returns true
	 base.close();
	
});
});
describe("@typedef {Object} FeatureExtractorConstant, number 55", function () {
it("should make test number 55", function () {

 var qm = require('qminer');
 // create a simple base, where each record contains only a persons name
 var base = new qm.Base({
   mode: 'createClean',
   schema: [{
      name: "Person",
      fields: [{ name: "Name", type: "string" }]
   }]
 });
 // create a feature space containing the constant extractor, where the constant is equal 5
 var ftr = new qm.FeatureSpace(base, { type: "constant", source: "Person", const: 5 });
 base.close();

});
});
describe("@typedef {Object} FeatureExtractorRandom, number 56", function () {
it("should make test number 56", function () {

 var qm = require('qminer');
 // create a simple base, where each record contains only a persons name
 var base = new qm.Base({
   mode: 'createClean',
   schema: [{
      name: "Person",
      fields: [{ name: "Name", type: "string" }]
   }]
 });
 // create a feature space containing the random extractor
 var ftr = new qm.FeatureSpace(base, { type: "random", source: "Person" });
 base.close();

});
});
describe("@typedef {Object} FeatureExtractorNumeric, number 57", function () {
it("should make test number 57", function () {

 var qm = require('qminer');
 // create a simple base, where each record contains the student name and it's grade
 var base = new qm.Base({
    mode: 'createClean',
    schema: [{
       name: "Class",
       fields: [
          { name: "Name", type: "string" },
          { name: "Grade", type: "int" }
       ]
    }]
 });
 // create a feature space containing the numeric extractor, where the values are
 // normalized, the values are taken from the field "Grade"
 var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "Class", normalize: true, field: "Grade" });
 base.close();

});
});
describe("@typedef {Object} FeatureExtractorSparseVector, number 58", function () {
it("should make test number 58", function () {

 var qm = require('qminer');
 // create a simple base, where each record contains the student name and it's grade
 var base = new qm.Base({
    mode: 'createClean',
    schema: [{
       "name": "Class",
       "fields": [
          { "name": "Name", "type": "string" },
          { "name": "Features", "type": "num_sp_v" }
       ]
    }]
 });
 // create a feature space containing the numeric extractor, where the values are
 // normalized, the values are taken from the field "Grade"
 var ftr = new qm.FeatureSpace(base, { type: "num_sp_v", source: "Class", normalize: false, field: "Features" });
 base.close();

});
});
describe("@typedef {Object} FeatureExtractorCategorical, number 59", function () {
it("should make test number 59", function () {

 var qm = require('qminer');
 // create a simple base, where each record contains the student name and it's study group
 // here we know the student is part of only one study group
 var base = new qm.Base({
    mode: 'createClean',
    schema: [{
       name: "Class",
       fields: [
          { name: "Name", type: "string" },
          { name: "StudyGroup", type: "string" }
       ]
    }]
 });
 // create a feature space containing the categorical extractor, where the it's values
 // are taken from the field "StudyGroup": "A", "B", "C" and "D"
 var ftr = new qm.FeatureSpace(base, { type: "categorical", source: "Class", field: "StudyGroup", values: ["A", "B", "C", "D"] });
 base.close();

});
});
describe("@typedef {Object} FeatureExtractorMultinomial, number 60", function () {
it("should make test number 60", function () {

 var qm = require('qminer');
 // create a simple base, where each record contains the student name and an array of study groups
 // here we know a student can be part of multiple study groups
 var base = new qm.Base({
    mode: 'createClean',
    schema: [{
       name: "Class",
       fields: [
          { name: "Name", type: "string" },
          { name: "StudyGroups", type: "string_v" }
       ]
    }]
 });
 // create a feature space containing the multinomial extractor, where the values are normalized,
 // and taken from the field "StudyGroup": "A", "B", "C", "D", "E", "F"
 var ftr = new qm.FeatureSpace(base, {
              type: "multinomial", source: "Class", field: "StudyGroups", normalize: true, values: ["A", "B", "C", "D", "E", "F"]
           });
 base.close();

});
});
describe("@typedef {Object} FeatureExtractorText, number 61", function () {
it("should make test number 61", function () {

 var qm = require('qminer');
 // create a simple base, where each record contains the title of the article and it's content
 var base = new qm.Base({
    mode: 'createClean',
    schema: [{
       name: "Articles",
       fields: [
          { name: "Title", type: "string" },
          { name: "Text", type: "string" }
       ]
    }]
 });
 // create a feature spave containing the text (bag of words) extractor, where the values are normalized,
 // weighted with 'tfidf' and the tokenizer is of 'simple' type, it uses english stopwords.
 var ftr = new qm.FeatureSpace(base, {
              type: "text", source: "Articles", field: "Text", normalize: true, weight: "tfidf",
              tokenizer: { type: "simple", stopwords: "en"}
           });
 base.close();

});
});
describe("@typedef {Object} FeatureExtractorJoin, number 62", function () {
it("should make test number 62", function () {

 // import qm module
 var qm = require('qminer');

});
});
describe("@typedef {Object} FeatureExtractorPair, number 63", function () {
it("should make test number 63", function () {

 var qm = require('qminer');

});
});
describe("@typedef {Object} FeatureExtractorDateWindow, number 64", function () {
it("should make test number 64", function () {

 // import qm module
 var qm = require('qminer');

});
});
describe("@typedef {Object} FeatureExtractorJsfunc, number 65", function () {
it("should make test number 65", function () {

 var qm = require('qminer');
 // create a simple base, where each record contains the name of the student and his study groups
 // each student is part of multiple study groups
 var base = new qm.Base({
    mode: 'createClean',
    schema: [{
       name: "Class",
       fields: [
          { name: "Name", type: "string" },
          { name: "StudyGroups", type: "string_v" }
       ]
    }]
 });
 // create a feature space containing the jsfunc extractor, where the function counts the number
 // of study groups each student is part of. The functions name is "NumberOFGroups", it's dimension
 // is 1 (returns only one value, not an array)
 var ftr = new qm.FeatureSpace(base, {
              type: "jsfunc", source: "Class", name: "NumberOfGroups", dim: 1,
              fun: function (rec) { return rec.StudyGroups.length; }
           });
 base.close();

});
});
describe("Feature Space, number 66", function () {
it("should make test number 66", function () {

 // import qm module
 var qm = require('qminer');
 // construct a base with the store
 var base = new qm.Base({
   mode: "createClean",
   schema: {
     name: "FtrSpace",
     fields: [
       { name: "Value", type: "float" },
       { name: "Category", type: "string" },
       { name: "Categories", type: "string_v" },
     ],
     joins: [],
     keys: []
   }
 });
 // populate the store
 Store = base.store("FtrSpace");
 Store.push({ Value: 1.0, Category: "a", Categories: ["a", "q"] });
 Store.push({ Value: 1.1, Category: "b", Categories: ["b", "w"] });
 Store.push({ Value: 1.2, Category: "c", Categories: ["c", "e"] });
 Store.push({ Value: 1.3, Category: "a", Categories: ["a", "q"] });
 // create a feature space 
 var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpace", field: "Value" });
 base.close();

});
});
describe("Clears the feature space., number 67", function () {
it("should make test number 67", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Runners",
	        fields: [
	            { name: "ID", type: "int", primary: true },
	            { name: "Name", type: "string" },
	            { name: "BestTime", type: "float" }
	        ]
	    }]
	 });
	 // put some records in the "Runners" store
	 base.store("Runners").push({ ID: 110020, Name: "Eric Ericsson", BestTime: 134.33 });
	 base.store("Runners").push({ ID: 123307, Name: "Fred Friedrich", BestTime: 101.11 });
	 base.store("Runners").push({ ID: 767201, Name: "Appel Banana", BestTime: 1034.56 });
	 // create a feature space
	 var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "Runners", field: "BestTime" });
	 // update the feature space
	 ftr.updateRecords(base.store("Runners").allRecords);
	 // clear the feature space (return the feature space to it's default values)
	 ftr.clear();
	 base.close();
	
});
});
describe("Adds a new feature extractor to the feature space., number 68", function () {
it("should make test number 68", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "WeatherForcast",
	        fields: [
	            { name: "Weather", type: "string" },
	            { name: "Date", type: "datetime" },
	            { name: "TemperatureDegrees", type: "int" }
	        ]
	    }]
	 });
	 // put some records in the "WeatherForecast" store
	 base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-27T11:00:00", TemperatureDegrees: 19 });
	 base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-28T11:00:00", TemperatureDegrees: 22 });
	 base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-29T11:00:00", TemperatureDegrees: 25 });
	 base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-30T11:00:00", TemperatureDegrees: 25 });
	 base.store("WeatherForcast").push({ Weather: "Scattered Showers", Date: "2015-05-31T11:00:00", TemperatureDegrees: 24 });
	 base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-06-01T11:00:00", TemperatureDegrees: 27 });
	 // create a feature space 
	 var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "WeatherForcast", field: "TemperatureDegrees" });
	 // add a new feature extractor to the feature space
	 // it adds the new feature extractor to the pre-existing feature extractors in the feature space
	 ftr.addFeatureExtractor({ type: "text", source: "WeatherForcast", field: "Weather", normalize: true, weight: "tfidf" });      
	 base.close();
	
});
});
describe("Updates the feature space definitions and extractors by adding one record., number 69", function () {
it("should make test number 69", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base
	 var base = new qm.Base({
       mode: "createClean",
       schema: {
         name: "FtrSpace",
         fields: [
           { name: "Value", type: "float" },
           { name: "Category", type: "string" },
           { name: "Categories", type: "string_v" },
         ]
       }
     });
     // populate the store
     Store = base.store("FtrSpace");
     Store.push({ Value: 1.0, Category: "a", Categories: ["a", "q"] });
     Store.push({ Value: 1.1, Category: "b", Categories: ["b", "w"] });
     Store.push({ Value: 1.2, Category: "c", Categories: ["c", "e"] });
     Store.push({ Value: 1.3, Category: "a", Categories: ["a", "q"] });
	 // create a new feature space
	 var ftr = new qm.FeatureSpace(base, [
	   { type: "numeric", source: "FtrSpace", normalize: true, field: "Value" },
	   { type: "categorical", source: "FtrSpace", field: "Category", values: ["a", "b", "c"] },
	   { type: "multinomial", source: "FtrSpace", field: "Categories", normalize: true, values: ["a", "b", "c", "q", "w", "e"] }
	 ]);
	 // update the feature space with the first three record of the store
	 ftr.updateRecord(Store[0]);
	 ftr.updateRecord(Store[1]);
	 ftr.updateRecord(Store[2]);
	 // get the feature vectors of these records
	 ftr.extractVector(Store[0]); // returns the vector [0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2), 0, 0]
	 ftr.extractVector(Store[1]); // returns the vector [1/2, 0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2), 0]
	 ftr.extractVector(Store[2]); // returns the vector [1, 0, 0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2)]
	 base.close();
	
});
});
describe("Updates the feature space definitions and extractors by adding all the records of a record set., number 70", function () {
it("should make test number 70", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base
	 var base = new qm.Base({
       mode: "createClean",
       schema: {
         name: "FtrSpace",
         fields: [
           { name: "Value", type: "float" },
           { name: "Category", type: "string" },
           { name: "Categories", type: "string_v" },
         ]
       }
     });
     // populate the store
	 Store = base.store("FtrSpace");
	 Store.push({ Value: 1.0, Category: "a", Categories: ["a", "q"] });
	 Store.push({ Value: 1.1, Category: "b", Categories: ["b", "w"] });
	 Store.push({ Value: 1.2, Category: "c", Categories: ["c", "e"] });
	 Store.push({ Value: 1.3, Category: "a", Categories: ["a", "q"] });
	 // create a new feature space
	 var ftr = new qm.FeatureSpace(base, [
	     { type: "numeric", source: "FtrSpace", normalize: true, field: "Value" },
	     { type: "categorical", source: "FtrSpace", field: "Category", values: ["a", "b", "c"] },
	     { type: "multinomial", source: "FtrSpace", field: "Categories", normalize: true, values: ["a", "b", "c", "q", "w", "e"] }
	 ]);
	 // update the feature space with the record set 
	 ftr.updateRecords(Store.allRecords);
	 // get the feature vectors of these records
	 ftr.extractVector(Store[0]); // returns the vector [0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2), 0, 0]
	 ftr.extractVector(Store[1]); // returns the vector [1/3, 0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2), 0]
	 ftr.extractVector(Store[2]); // returns the vector [2/3, 0, 0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2)]
	 ftr.extractVector(Store[3]); // returns the vector [1, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2), 0, 0]
	 base.close();
	
});
});
describe("Creates a sparse feature vector from the given record., number 71", function () {
it("should make test number 71", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a base containing the store Class. Let the Name field be the primary field. 
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Class",
	        fields: [
	            { name: "Name", type: "string", primary: true },
	            { name: "StudyGroup", type: "string" }
	        ]
	    }]
	 });
	 // add some records to the store
	 base.store("Class").push({ Name: "Dean", StudyGroup: "A" });
	 base.store("Class").push({ Name: "Chang", StudyGroup: "D" });
	 base.store("Class").push({ Name: "Magnitude", StudyGroup: "C" });
	 base.store("Class").push({ Name: "Leonard", StudyGroup: "B" });
	 // create a new feature space
	 // the feature space is of dimensions [0, 4]; 4 is the dimension of the categorical feature extractor
	 // and 0 is the dimension of text feature extractor (the text feature extractor doesn't have any words,
	 // need to be updated for use).
	 var ftr = new qm.FeatureSpace(base, [
	    { type: "text", source: "Class", field: "Name", normalize: false },
	    { type: "categorical", source: "Class", field: "StudyGroup", values: ["A", "B", "C", "D"] } 
	 ]);
	 // get the sparse extractor vector for the first record in store
	 // the sparse vector will be [(0, 1)] - uses only the categorical feature extractor. There are no
	 // features in the text feature extractor.
	 var vec = ftr.extractSparseVector(base.store("Class")[0]);
	 base.close();
	
});
});
describe("Creates a feature vector from the given record., number 72", function () {
it("should make test number 72", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a base containing the store Class. Let the Name field be the primary field.
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Class",
	        fields: [
	            { name: "Name", type: "string", primary: true },
	            { name: "StudyGroup", type: "string" }
	        ]
	    }]
	 });
	 // add some records to the store
	 base.store("Class").push({ Name: "Jeff", StudyGroup: "A" });
	 base.store("Class").push({ Name: "Britta", StudyGroup: "D" });
	 base.store("Class").push({ Name: "Abed", StudyGroup: "C" });
	 base.store("Class").push({ Name: "Annie", StudyGroup: "B" });
	 // create a new feature space
	 // the feature space is of dimensions [0, 4]; 4 is the dimension of the categorical feature extractor
	 // and 0 is the dimension of text feature extractor (the text feature extractor doesn't have any words,
	 // need to be updated for use).
	 var ftr = new qm.FeatureSpace(base, [
	    { type: "text", source: "Class", field: "Name", normalize: false },
	    { type: "categorical", source: "Class", field: "StudyGroup", values: ["A", "B", "C", "D"] }
	 ]);
	 // get the extractor vector for the first record in store
	 // the sparse vector will be [1, 0, 0, 0] - uses only the categorical feature extractor. There are no
	 // features in the text feature extractor.
	 var vec = ftr.extractVector(base.store("Class")[0]);
	 base.close();
	
});
});
describe("Extracts the sparse feature vectors from the record set and returns them as columns of the sparse matrix., number 73", function () {
it("should make test number 73", function () {

	 // import qm module
	 var qm = require("qminer");
	 // create a base containing the store Class. Let the Name field be the primary field. 
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Class",
	        fields: [
	            { name: "Name", type: "string", primary: true },
	            { name: "StudyGroups", type: "string_v" }
	        ]
	    }]
	 });
	 // add some records to the store
	 base.store("Class").push({ Name: "Dean", StudyGroups: ["A", "D"] });
	 base.store("Class").push({ Name: "Chang", StudyGroups: ["B", "D"] });
	 base.store("Class").push({ Name: "Magnitude", StudyGroups: ["B", "C"] });
	 base.store("Class").push({ Name: "Leonard", StudyGroups: ["A", "B"] });
	 // create a feature space containing the multinomial feature extractor
	 var ftr = new qm.FeatureSpace(base, { type: "multinomial", source: "Class", field: "StudyGroups", values: ["A", "B", "C", "D"] });
	 // create a sparse feature matrix out of the records of the store by using the feature space
	 // returns a sparse matrix equal to 
	 // [[(0, 1), (3, 1)], [(1, 1), (3, 1)], [(1, 1), (2, 1)], [(0, 1), (1, 1)]]
	 var sparseMatrix = ftr.extractSparseMatrix(base.store("Class").allRecords);
	 base.close();
	
});
});
describe("Extracts the feature vectors from the recordset and returns them as columns of a dense matrix., number 74", function () {
it("should make test number 74", function () {

	 // import qm module
	 var qm = require("qminer");
	 // create a base containing the store Class. Let the Name field be the primary field.
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Class",
	        fields: [
	            { name: "Name", type: "string", primary: true },
	            { name: "StudyGroups", type: "string_v" }
	        ]
	    }]
	 });
	 // add some records to the store
	 base.store("Class").push({ Name: "Dean", StudyGroups: ["A", "D"] });
	 base.store("Class").push({ Name: "Chang", StudyGroups: ["B", "D"] });
	 base.store("Class").push({ Name: "Magnitude", StudyGroups: ["B", "C"] });
	 base.store("Class").push({ Name: "Leonard", StudyGroups: ["A", "B"] });
	 // create a feature space containing the multinomial feature extractor
	 var ftr = new qm.FeatureSpace(base, { type: "multinomial", source: "Class", field: "StudyGroups", values: ["A", "B", "C", "D"] });
	 // create a feature matrix out of the records of the store by using the feature space
	 // returns a sparse matrix equal to
	 // 1  0  0  1
	 // 0  1  0  1
	 // 0  0  1  0
	 // 1  1  0  0
	 var matrix = ftr.extractMatrix(base.store("Class").allRecords);
	 base.close();
	
});
});
describe("Gives the name of feature extractor at given position., number 75", function () {
it("should make test number 75", function () {

	 // import qm module
	 var qm = require("qminer");
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "People",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "Gendre", type: "string" },
	            { name: "Age", type: "int" }
	        ]
	    }]
	 });
	 // create a feature space containing a categorical and numeric feature extractor
	 var ftr = new qm.FeatureSpace(base, [
	    { type: "numeric", source: "People", field: "Age" },
	    { type: "categorical", source: "People", field: "Gendre", values: ["Male", "Female"] }
	 ]);
	 // get the name of the feature extractor with index 1
	 var extractorName = ftr.getFeatureExtractor(1); // returns "Categorical[Gendre]"
	 base.close();
	
});
});
describe("Gives the name of the feature at the given position., number 76", function () {
it("should make test number 76", function () {

	 // import qm module
	 var qm = require("qminer");
	 // create a base containing the store Class. Let the Name field be the primary field.
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Class",
	        fields: [
	            { name: "Name", type: "string", primary: true },
	            { name: "StudyGroups", type: "string_v" }
	        ]
	    }]
	 });
	 // add some records to the store
	 base.store("Class").push({ Name: "Dean", StudyGroups: ["A", "D"] });
	 base.store("Class").push({ Name: "Chang", StudyGroups: ["B", "D"] });
	 base.store("Class").push({ Name: "Magnitude", StudyGroups: ["B", "C"] });
	 base.store("Class").push({ Name: "Leonard", StudyGroups: ["A", "B"] });
	 // create a feature space containing the multinomial feature extractor
	 var ftr = new qm.FeatureSpace(base, [
	 { type: "text", source: "Class", field: "Name" },
	 { type: "multinomial", source: "Class", field: "StudyGroups", values: ["A", "B", "C", "D"] }
	 ]);
	 // get the feature at position 2
	 var feature = ftr.getFeature(2); // returns "C", because the text extractor has no features at the moment
	 // update the feature space with the records of the store; see the method updateRecords
	 ftr.updateRecords(base.store("Class").allRecords);
	 // get the feature at position 2
	 var feature2 = ftr.getFeature(2); // returns "magnitude"
	 base.close();
	
});
});
describe("Performs the inverse operation of ftrVec. Works only for numeric feature extractors., number 77", function () {
it("should make test number 77", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "TheWitcherSaga",
	        fields: [
	            { name: "Title", type: "string" },
	            { name: "YearOfRelease", type: "int" },
	            { name: "EnglishEdition", type: "bool" }
	        ]
	    }]
	 });
	 // put some records in the store
	 base.store("TheWitcherSaga").push({ Title: "Blood of Elves", YearOfRelease: 1994, EnglishEdition: true });
	 base.store("TheWitcherSaga").push({ Title: "Time of Contempt", YearOfRelease: 1995, EnglishEdition: true });
	 base.store("TheWitcherSaga").push({ Title: "Baptism of Fire", YearOfRelease: 1996, EnglishEdition: true });
	 base.store("TheWitcherSaga").push({ Title: "The Swallow's Tower", YearOfRelease: 1997, EnglishEdition: false });
	 base.store("TheWitcherSaga").push({ Title: "Lady of the Lake", YearOfRelease: 1999, EnglishEdition: false });
	 base.store("TheWitcherSaga").push({ Title: "Season of Storms", YearOfRelease: 2013, EnglishEdition: false });
	 // create a feature space with the numeric feature extractor and update the feature space with the records in store
	 // for update, look the method updateRecords in feature space
	 var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "TheWitcherSaga", field: "YearOfRelease", normalize: true });
	 ftr.updateRecords(base.store("TheWitcherSaga").allRecords);
	 // get a feature vector for the second record
	 // because of the numeric feature extractor having normalize: true and of the records update of feature space, the values
	 // are not equal to those of the records, i.e. the value 1995 is now 0.105263 
	 var ftrVec = ftr.extractVector(base.store("TheWitcherSaga")[1]);
	 // get the inverse of the feature vector
	 // the function returns the values to their first value, i.e. 0.105263 returns to 1995
	 var inverse = ftr.invertFeatureVector(ftrVec); // returns a vector [1995]
	 base.close();
	
});
});
describe("Calculates the inverse of a single feature using a specific feature extractor., number 78", function () {
it("should make test number 78", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base containing one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "TheWitcherSaga",
	        fields: [
	            { name: "Title", type: "string" },
	            { name: "YearOfRelease", type: "int" },
	            { name: "EnglishEdition", type: "bool" }
	        ]
	    }]
	 });
	 // put some records in the store
	 base.store("TheWitcherSaga").push({ Title: "Blood of Elves", YearOfRelease: 1994, EnglishEdition: true });
	 base.store("TheWitcherSaga").push({ Title: "Time of Contempt", YearOfRelease: 1995, EnglishEdition: true });
	 base.store("TheWitcherSaga").push({ Title: "Baptism of Fire", YearOfRelease: 1996, EnglishEdition: true });
	 base.store("TheWitcherSaga").push({ Title: "The Swallow's Tower", YearOfRelease: 1997, EnglishEdition: false });
	 base.store("TheWitcherSaga").push({ Title: "Lady of the Lake", YearOfRelease: 1999, EnglishEdition: false });
	 base.store("TheWitcherSaga").push({ Title: "Season of Storms", YearOfRelease: 2013, EnglishEdition: false });
	 // create a feature space with the numeric feature extractor and update the feature space with the records in store
	 // for update, look the method updateRecords in feature space
	 var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "TheWitcherSaga", field: "YearOfRelease", normalize: true });
	 ftr.updateRecords(base.store("TheWitcherSaga").allRecords);
	 // because of the numeric feature extractor having normalize: true and of the records update of feature space, 
	 // the values are not equal to those of the records 
	 // invert the value 0 using the numeric feature extractor
	 var inverse = ftr.invertFeature(0, 0); // returns the value 1994
	 base.close();
	
});
});
describe("Filters the vector to keep only the elements from the feature extractor., number 79", function () {
it("should make test number 79", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a new base with one store
	 var base = new qm.Base({
	    mode: "createClean",
	    schema: [{
	        name: "Academics",
	        fields: [
	            { name: "Name", type: "string" },
	            { name: "Age", type: "int" },
	            { name: "Gendre", type: "string" },
	            { name: "Skills", type: "string_v" }
	        ]
	    }]
	 });
	 // create a new feature space
	 var ftr = new qm.FeatureSpace(base, [
         { type: "numeric", source: "Academics", field: "Age" },
         { type: "categorical", source: "Academics", field: "Gendre", values: ["Male", "Female"] },
	     { type: "multinomial", source: "Academics", field: "Skills", values: ["Mathematics", "Programming", "Philosophy", "Languages", "Politics", "Cooking"] }
	     ]);
	 // create a new dense vector
	 var vec = new qm.la.Vector([40, 0, 1, 0, 1, 1, 1, 0, 0]);
	 // filter the elements from the second feature extractor
	 var vec2 = ftr.filter(vec, 1); // returns vector [0, 0, 1, 0, 0, 0, 0, 0, 0]
	 // filter the elements from the second feature extractor, without keeping the offset
	 var vec3 = ftr.filter(vec, 1, false); // returns vector [0, 1]
	 // create a new sparse vector
	 var spVec = new qm.la.SparseVector([[0, 40], [2, 1], [4, 1], [5, 1], [6, 1]]);
	 // filter the elements from the second feature extractor
	 var spVec2 = ftr.filter(spVec, 1); // returns sparse vector [[2, 1]]
	 // filter the elements from the second feature extractor, without keeping the offset
	 var spVec3 = ftr.filter(spVec, 1, false); // returns sparse vector [[1, 1]]
	 base.close();
	
});
});
describe("@classdesc Circular buffer for storing records. Size of buffer is defined at, number 80", function () {
it("should make test number 80", function () {

	 // TODO
    
});
});
describe("Add new record to the buffer., number 81", function () {
it("should make test number 81", function () {

         // TODO
    	
});
});

});
