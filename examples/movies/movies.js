// import libraries
var qm = require('qminer.js');
var la = qm.la;
var analytics = qm.analytics;
var fs = qm.fs;

var base = new qm.Base({
	mode: "createClean",
	schema: [
	    {
	        "name": "People",
	        "fields": [
	            { "name": "Name", "type": "string", "primary": true },
	            { "name": "Gender", "type": "string", "shortstring": true }
	        ],
	        "joins": [
	            { "name": "ActedIn", "type": "index", "store": "Movies", "inverse" : "Actor" },
	            { "name": "Directed", "type": "index", "store": "Movies", "inverse" : "Director" }
	        ],
	        "keys": [
	            { "field": "Gender", "type": "value" }
	        ]
	    },
	    {
	        "name": "Movies",
	        "fields": [
	            { "name": "Title", "type": "string" },
	            { "name": "Plot", "type": "string", "store" : "cache" },
	            { "name": "Year", "type": "int" },
	            { "name": "Rating", "type": "float" },
	            { "name": "Genres", "type": "string_v", "codebook" : true }
	        ],
	        "joins": [
	            { "name": "Actor", "type": "index", "store": "People", "inverse" : "ActedIn" },
	            { "name": "Director", "type": "field", "store": "People", "inverse" : "Directed" }
	        ],
	        "keys": [
	            { "field": "Title", "type": "value" },
	            { "field": "Title", "name": "TitleTxt", "type": "text", "vocabulary" : "voc_01" },
	            { "field": "Plot", "type": "text", "vocabulary" : "voc_01" },
	            { "field": "Genres", "type": "value" }
	        ]
	    }
	]
});

// We are, we start by loading in the dataset.
console.log("Movies", "Loading and indexing input data")
base.store("Movies").loadJson("./movies.json");
// Prepare shortcuts to set of all people and all movies
var people = base.store("People").allRecords;
var movies = base.store("Movies").allRecords;
console.log("Loaded " + movies.length + " movies and " + people.length + " people.");

// Declare the features we will use to detect movie genres
var genreFeatures = [
    { type: "constant", source: "Movies" },
    { type: "text", source: "Movies", field: "Title" },
    { type: "text", source: "Movies", field: "Plot" },
    { type: "multinomial", source: { store: "Movies", join: "Actor" }, field: "Name" },
    { type: "multinomial", source: { store: "Movies", join: "Director" }, field: "Name" }
];
// Create and initialize feature matrix
var genreFeatureSpace = new qm.FeatureSpace(base, genreFeatures);
genreFeatureSpace.updateRecords(movies);
var genreFeatureMatrix = genreFeatureSpace.extractSparseMatrix(movies);
console.log("Dimensionality of feature space: " + genreFeatureSpace.dim);
// Create and initialize label matrix for training
var labelFeatures = [{ type: "multinomial", source: "Movies", field: "Genres" }]
var genreLabelSpace = new qm.FeatureSpace(base, labelFeatures);
genreLabelSpace.updateRecords(movies);
var genreLabelMatrix = genreLabelSpace.extractMatrix(movies);
console.log("Dimensionality of label space: " + genreLabelSpace.dims);
// We will use one-vs-all model for gener classification
var genreModel = new analytics.OneVsAll({
	model: analytics.SVC,
	modelParam: { c: 10, algorithm: "LIBSVM" },
	cats: genreLabelMatrix.rows,
	verbose: true
});
// Create a model for the Genres field, using all the movies as training set.
console.log("Training genre models");
genreModel.fit(genreFeatureMatrix, genreLabelMatrix);

// Declare the features we will use to predict movie rating
var ratingFeatures = [
    { type: "constant", source: "Movies" },
    { type: "text", source: "Movies", field: "Title" },
    { type: "text", source: "Movies", field: "Plot" },
    { type: "multinomial", source: "Movies", field: "Genres" },
    { type: "multinomial", source: { store: "Movies", join: "Actor" }, field: "Name" },
    { type: "multinomial", source: { store: "Movies", join: "Director" }, field: "Name" }
];
// Create and initialize feature matrix
var ratingFeatureSpace = new qm.FeatureSpace(base, ratingFeatures);
// Create and initialize vector with target ratings
ratingFeatureSpace.updateRecords(movies);
var ratingFeatureMatrix = ratingFeatureSpace.extractSparseMatrix(movies);
// Create and initialize vector with ratings for trainings
var ratingVector = movies.getVector("Rating");
// We will use regression model for predicting ratings
var ratingModel = new analytics.SVR();
// Train regression
ratingModel.fit(ratingFeatureMatrix, ratingVector);

// Test de-serialized models on two new movies
var newHorrorMovie = base.store("Movies").newRecord({
    "Title":"Unnatural Selection",
    "Plot":"When corpses are found with internal organs missing, Liz Shaw and P.R.O.B.E. " +
           "investigate a defunct government project from the 1970s that aimed to predict " +
           "the course of human evolution. But was the creature it produced really destroyed," +
           "or has it resurfaced twenty years on?",
    "Year":1996.000000,
    "Rating":6.200000,
    "Genres":["Horror", "Sci-Fi"],
    "Director":{"Name":"Baggs Bill", "Gender":"Unknown"},
    "Actor":[
        {"Name":"Beevers Geoffrey", "Gender":"Male"},
        {"Name":"Bradshaw Stephen (I)", "Gender":"Male"},
        {"Name":"Brooks Keith (III)", "Gender":"Male"},
        {"Name":"Gatiss Mark", "Gender":"Male"},
        {"Name":"Kay Charles", "Gender":"Male"},
        {"Name":"Kirk Alexander (I)", "Gender":"Male"},
        {"Name":"Moore Mark (II)", "Gender":"Male"},
        {"Name":"Murphy George A.", "Gender":"Male"},
        {"Name":"Mykaj Gabriel", "Gender":"Male"},
        {"Name":"Rigby Jonathan", "Gender":"Male"},
        {"Name":"Wolfe Simon (I)", "Gender":"Male"},
        {"Name":"Jameson Louise", "Gender":"Female"},
        {"Name":"John Caroline", "Gender":"Female"},
        {"Name":"Merrick Patricia", "Gender":"Female"},
        {"Name":"Randall Zoe", "Gender":"Female"},
        {"Name":"Rayner Kathryn", "Gender":"Female"}
    ]
});
// apply genre model
var newHorrorMovieGenreVector = genreFeatureSpace.extractSparseVector(newHorrorMovie);
console.log("Top genre: " + genreLabelSpace.getFeature(genreModel.predict(newHorrorMovieGenreVector)));
// apply rating model
var newHorrorMovieRatingVector = ratingFeatureSpace.extractSparseVector(newHorrorMovie);
console.log("Predicted rating: " + ratingModel.predict(newHorrorMovieRatingVector).toFixed(1));
console.log("True rating:      " + newHorrorMovie.Rating);

var newComedyMovie = base.store("Movies").newRecord({
    "Title":"Die Feuerzangenbowle",
    "Plot":"Hans Pfeiffer and some of his friends are drinking \"Feuerzangenbowle\". Talking " +
           "about their school-time they discover that Hans never was at a regular school and " +
           "so, as they think, missed an important part of his youth. They decide to send him " +
           "back to school to do all the things he never could do before.",
    "Year":1944.000000,
    "Rating":7.800000,
    "Genres":["Comedy"],
    "Director":{"Name":"Weiss Helmut", "Gender":"Unknown"},
    "Actor":[
        {"Name":"Biegel Erwin", "Gender":"Male"},
        {"Name":"Etlinger Karl", "Gender":"Male"},
        {"Name":"Florath Albert", "Gender":"Male"},
        {"Name":"Gutz Lutz", "Gender":"Male"},
        {"Name":"Gulstorff Max", "Gender":"Male"},
        {"Name":"Hasse Clemens", "Gender":"Male"},
        {"Name":"Henckels Paul", "Gender":"Male"},
        {"Name":"Leibelt Hans", "Gender":"Male"},
        {"Name":"Platen Karl", "Gender":"Male"},
        {"Name":"Ponto Erich", "Gender":"Male"},
        {"Name":"Richter Hans (I)", "Gender":"Male"},
        {"Name":"Ruhmann Heinz", "Gender":"Male"},
        {"Name":"Schippel Rudi", "Gender":"Male"},
        {"Name":"Schnell Georg H.", "Gender":"Male"},
        {"Name":"Vogel Egon (I)", "Gender":"Male"},
        {"Name":"Vogelsang Georg", "Gender":"Male"},
        {"Name":"Wenck Ewald", "Gender":"Male"},
        {"Name":"Werner Walter", "Gender":"Male"},
        {"Name":"Himboldt Karin", "Gender":"Female"},
        {"Name":"Litto Maria", "Gender":"Female"},
        {"Name":"Schun Margarete", "Gender":"Female"},
        {"Name":"Sessak Hilde", "Gender":"Female"},
        {"Name":"Wangel Hedwig", "Gender":"Female"},
        {"Name":"Wurtz Anneliese", "Gender":"Female"}
    ]
});
// apply genre model
var newComedyMovieGenreVector = genreFeatureSpace.extractSparseVector(newComedyMovie);
console.log("Top genre: " + genreLabelSpace.getFeature(genreModel.predict(newComedyMovieGenreVector)));
// apply rating model
var newComedyMovieRatingVector = ratingFeatureSpace.extractSparseVector(newComedyMovie);
console.log("Predicted rating: " + ratingModel.predict(newComedyMovieRatingVector).toFixed(1));
console.log("True rating:      " + newComedyMovie.Rating);
