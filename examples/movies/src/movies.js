var util = require("utilities.js");
var analytics = require("analytics.js");

console.say("Movies", "Starting IMDB example");

// prepare shortcuts
var People = qm.store("People");
var Movies = qm.store("Movies");

// check if starting first time
if (Movies.empty) {
    // loading in larger dataset
    console.log("Movies", "First start: loading, parsing and indexing movie data")
    var filename = "./sandbox/movies/movies.json"
    qm.load.jsonFileLimit(Movies, filename, 10000);   
    // report on what was loaded
    console.log("Loaded " + Movies.length + " movies and " + People.length + " people.");
    
    // prepare genre classification models
    var genreFeatures = [
        { type: "text", source: "Movies", field: "Title" },
        { type: "text", source: "Movies", field: "Plot" },        
        { type: "multinomial", source: { store: "Movies", join: "Actor" }, field: "Name" },
        { type: "multinomial", source: { store: "Movies", join: "Director"}, field: "Name" }
    ];
    var genreModel = analytics.newBatchModel(Movies.recs, genreFeatures, Movies.field("Genres"));
    // serialize
    var genreOut = fs.openWrite("./sandbox/movies/genre.dat");
    genreModel.save(genreOut); genreOut.close();   

    // prepare rating regression model
    var ratingFeatures = [
        { type: "text", source: "Movies", field: "Title" },
        { type: "text", source: "Movies", field: "Plot" },        
        { type: "multinomial", source: "Movies", field: "Genres" },
        { type: "multinomial", source: { store: "Movies", join: "Actor" }, field: "Name" },
        { type: "multinomial", source: { store: "Movies", join: "Director"}, field: "Name" }
    ];
    var ratingModel = analytics.newBatchModel(Movies.recs, ratingFeatures, Movies.field("Rating"));
    // serialize
    var ratingOut = fs.openWrite("./sandbox/movies/rating.dat");
    ratingModel.save(ratingOut); ratingOut.close();
    
    // test genre classification
    var newHorrorMovie = Movies.newRec({"Title":"Unnatural Selection", "Plot":"When corpses are found with internal organs missing, Liz Shaw and P.R.O.B.E. investigate a defunct government project from the 1970s that aimed to predict the course of human evolution. But was the creature it produced really destroyed, or has it resurfaced twenty years on?", "Year":1996.000000, "Rating":6.200000, "Genres":["Horror", "Sci-Fi"], "Director":{"Name":"Baggs Bill", "Gender":"Unknown"}, "Actor":[{"Name":"Beevers Geoffrey", "Gender":"Male"}, {"Name":"Bradshaw Stephen (I)", "Gender":"Male"}, {"Name":"Brooks Keith (III)", "Gender":"Male"}, {"Name":"Gatiss Mark", "Gender":"Male"}, {"Name":"Kay Charles", "Gender":"Male"}, {"Name":"Kirk Alexander (I)", "Gender":"Male"}, {"Name":"Moore Mark (II)", "Gender":"Male"}, {"Name":"Murphy George A.", "Gender":"Male"}, {"Name":"Mykaj Gabriel", "Gender":"Male"}, {"Name":"Rigby Jonathan", "Gender":"Male"}, {"Name":"Wolfe Simon (I)", "Gender":"Male"}, {"Name":"Jameson Louise", "Gender":"Female"}, {"Name":"John Caroline", "Gender":"Female"}, {"Name":"Merrick Patricia", "Gender":"Female"}, {"Name":"Randall Zoe", "Gender":"Female"}, {"Name":"Rayner Kathryn", "Gender":"Female"}]});
    console.log(JSON.stringify(genreModel.predict(newHorrorMovie)));
    console.log(JSON.stringify(ratingModel.predict(newHorrorMovie)) + " vs. " + newHorrorMovie.Rating);

    var newComedyMovie = Movies.newRec({"Title":"Die Feuerzangenbowle", "Plot":"Hans Pfeiffer and some of his friends are drinking \"Feuerzangenbowle\". Talking about their school-time they discover that Hans never was at a regular school and so, as they think, missed an important part of his youth. They decide to send him back to school to do all the things he never could do before.", "Year":1944.000000, "Rating":7.800000, "Genres":["Comedy"], "Director":{"Name":"Weiss Helmut", "Gender":"Unknown"}, "Actor":[{"Name":"Biegel Erwin", "Gender":"Male"}, {"Name":"Etlinger Karl", "Gender":"Male"}, {"Name":"Florath Albert", "Gender":"Male"}, {"Name":"G\ufffffff6tz Lutz", "Gender":"Male"}, {"Name":"G\ufffffffclstorff Max", "Gender":"Male"}, {"Name":"Hasse Clemens", "Gender":"Male"}, {"Name":"Henckels Paul", "Gender":"Male"}, {"Name":"Leibelt Hans", "Gender":"Male"}, {"Name":"Platen Karl", "Gender":"Male"}, {"Name":"Ponto Erich", "Gender":"Male"}, {"Name":"Richter Hans (I)", "Gender":"Male"}, {"Name":"R\ufffffffchmann Heinz", "Gender":"Male"}, {"Name":"Schippel Rudi", "Gender":"Male"}, {"Name":"Schnell Georg H.", "Gender":"Male"}, {"Name":"Vogel Egon (I)", "Gender":"Male"}, {"Name":"Vogelsang Georg", "Gender":"Male"}, {"Name":"Wenck Ewald", "Gender":"Male"}, {"Name":"Werner Walter", "Gender":"Male"}, {"Name":"Himboldt Karin", "Gender":"Female"}, {"Name":"Litto Maria", "Gender":"Female"}, {"Name":"Sch\ufffffff6n Margarete", "Gender":"Female"}, {"Name":"Sessak Hilde", "Gender":"Female"}, {"Name":"Wangel Hedwig", "Gender":"Female"}, {"Name":"W\ufffffffcrtz Anneliese", "Gender":"Female"}]});
    console.log(JSON.stringify(genreModel.predict(newComedyMovie)));
    console.log(JSON.stringify(ratingModel.predict(newComedyMovie)) + " vs. " + newHorrorMovie.Rating);        
}

// initialize models
var genreModel = analytics.loadBatchModel(fs.openRead("./sandbox/movies/genre.dat"));
var ratingModel = analytics.loadBatchModel(fs.openRead("./sandbox/movies/rating.dat"));

// test classification
var newHorrorMovie = Movies.newRec({"Title":"Unnatural Selection", "Plot":"When corpses are found with internal organs missing, Liz Shaw and P.R.O.B.E. investigate a defunct government project from the 1970s that aimed to predict the course of human evolution. But was the creature it produced really destroyed, or has it resurfaced twenty years on?", "Year":1996.000000, "Rating":6.200000, "Genres":["Horror", "Sci-Fi"], "Director":{"Name":"Baggs Bill", "Gender":"Unknown"}, "Actor":[{"Name":"Beevers Geoffrey", "Gender":"Male"}, {"Name":"Bradshaw Stephen (I)", "Gender":"Male"}, {"Name":"Brooks Keith (III)", "Gender":"Male"}, {"Name":"Gatiss Mark", "Gender":"Male"}, {"Name":"Kay Charles", "Gender":"Male"}, {"Name":"Kirk Alexander (I)", "Gender":"Male"}, {"Name":"Moore Mark (II)", "Gender":"Male"}, {"Name":"Murphy George A.", "Gender":"Male"}, {"Name":"Mykaj Gabriel", "Gender":"Male"}, {"Name":"Rigby Jonathan", "Gender":"Male"}, {"Name":"Wolfe Simon (I)", "Gender":"Male"}, {"Name":"Jameson Louise", "Gender":"Female"}, {"Name":"John Caroline", "Gender":"Female"}, {"Name":"Merrick Patricia", "Gender":"Female"}, {"Name":"Randall Zoe", "Gender":"Female"}, {"Name":"Rayner Kathryn", "Gender":"Female"}]});
console.log(JSON.stringify(genreModel.predict(newHorrorMovie)));
console.log(JSON.stringify(ratingModel.predict(newHorrorMovie)) + " vs. " + newHorrorMovie.Rating);

var newComedyMovie = Movies.newRec({"Title":"Die Feuerzangenbowle", "Plot":"Hans Pfeiffer and some of his friends are drinking \"Feuerzangenbowle\". Talking about their school-time they discover that Hans never was at a regular school and so, as they think, missed an important part of his youth. They decide to send him back to school to do all the things he never could do before.", "Year":1944.000000, "Rating":7.800000, "Genres":["Comedy"], "Director":{"Name":"Weiss Helmut", "Gender":"Unknown"}, "Actor":[{"Name":"Biegel Erwin", "Gender":"Male"}, {"Name":"Etlinger Karl", "Gender":"Male"}, {"Name":"Florath Albert", "Gender":"Male"}, {"Name":"G\ufffffff6tz Lutz", "Gender":"Male"}, {"Name":"G\ufffffffclstorff Max", "Gender":"Male"}, {"Name":"Hasse Clemens", "Gender":"Male"}, {"Name":"Henckels Paul", "Gender":"Male"}, {"Name":"Leibelt Hans", "Gender":"Male"}, {"Name":"Platen Karl", "Gender":"Male"}, {"Name":"Ponto Erich", "Gender":"Male"}, {"Name":"Richter Hans (I)", "Gender":"Male"}, {"Name":"R\ufffffffchmann Heinz", "Gender":"Male"}, {"Name":"Schippel Rudi", "Gender":"Male"}, {"Name":"Schnell Georg H.", "Gender":"Male"}, {"Name":"Vogel Egon (I)", "Gender":"Male"}, {"Name":"Vogelsang Georg", "Gender":"Male"}, {"Name":"Wenck Ewald", "Gender":"Male"}, {"Name":"Werner Walter", "Gender":"Male"}, {"Name":"Himboldt Karin", "Gender":"Female"}, {"Name":"Litto Maria", "Gender":"Female"}, {"Name":"Sch\ufffffff6n Margarete", "Gender":"Female"}, {"Name":"Sessak Hilde", "Gender":"Female"}, {"Name":"Wangel Hedwig", "Gender":"Female"}, {"Name":"W\ufffffffcrtz Anneliese", "Gender":"Female"}]});
console.log(JSON.stringify(genreModel.predict(newComedyMovie)));     
console.log(JSON.stringify(ratingModel.predict(newComedyMovie)) + " vs. " + newHorrorMovie.Rating);        
