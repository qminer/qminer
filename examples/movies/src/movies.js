// Import analytics module
var analytics = require("analytics.js");

// Prepare shortcuts to people and movies store
var People = qm.store("People");
var Movies = qm.store("Movies");

// Check if we are starting for the first time
if (Movies.empty) {
    // We are, we start by loading in the dataset.
    console.log("Movies", "Loading and indexing input data")
    var filename = "./sandbox/movies/movies.json"
    qm.load.jsonFile(Movies, filename);
    console.log("Loaded " + Movies.length + " movies and " + People.length + " people.");
    
    // Declare the features we will use to build genre classification models
    var genreFeatures = [
        { type: "text", source: "Movies", field: "Title" },
        { type: "text", source: "Movies", field: "Plot" },        
        { type: "multinomial", source: { store: "Movies", join: "Actor" }, field: "Name" },
        { type: "multinomial", source: { store: "Movies", join: "Director"}, field: "Name" }
    ];
    // Create a model for the Genres field, using all the movies as training set.
    // Since the target field is discrete the underlaying model will be based on classification.
    var genreModel = analytics.newBatchModel(Movies.recs, genreFeatures, Movies.field("Genres"));
    // Serialize the model to disk so we can use it later
    var genreOut = fs.openWrite("./sandbox/movies/genre.dat");
    genreModel.save(genreOut); genreOut.close();   

    // Declare the features we will use to build the rating regression model
    var ratingFeatures = [
        { type: "text", source: "Movies", field: "Title" },
        { type: "text", source: "Movies", field: "Plot" },        
        { type: "multinomial", source: "Movies", field: "Genres" },
        { type: "multinomial", source: { store: "Movies", join: "Actor" }, field: "Name" },
        { type: "multinomial", source: { store: "Movies", join: "Director"}, field: "Name" }
    ];
    // Create a model for the Rating field, using all the movies as training set.
    // Since the target field is discrete the underlaying model will be based on regression.
    var ratingModel = analytics.newBatchModel(Movies.recs, ratingFeatures, Movies.field("Rating"));
    // Serialize the model to disk so we can use it later
    var ratingOut = fs.openWrite("./sandbox/movies/rating.dat");
    ratingModel.save(ratingOut); ratingOut.close();
}

// Load the models for genres and rating from disk
var genreModel = analytics.loadBatchModel(fs.openRead("./sandbox/movies/genre.dat"));
var ratingModel = analytics.loadBatchModel(fs.openRead("./sandbox/movies/rating.dat"));

// Test de-serialized models on two new movies
var newHorrorMovie = Movies.newRec({
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
console.log(JSON.stringify(genreModel.predict(newHorrorMovie)));
console.log(JSON.stringify(ratingModel.predict(newHorrorMovie)) + " vs. " + newHorrorMovie.Rating);
var newComedyMovie = Movies.newRec({
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
console.log(JSON.stringify(genreModel.predict(newComedyMovie)));
console.log(JSON.stringify(ratingModel.predict(newComedyMovie)) + " vs. " + newHorrorMovie.Rating);        









































