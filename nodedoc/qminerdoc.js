/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
/**
* Qminer module.
* @module qm
* @example 
* // import module
* var qm = require('qminer');
*/
/**
	* Set verbosity of QMiner internals.
	* @param {number} [level=0] - verbosity level: 0 = no output, 1 = log output, 2 = log and debug output.
	*/
 exports.verbosity = function (level) { }
/**
* Base access modes.
* @readonly
* @enum {string}
*/
 var baseModes = {
    /** sets up the db folder */
    create: 'create',
    /** cleans the db folder and calls create */
    createClean: 'createClean',
    /** opens with write permissions */
    open: 'open',
    /** opens in read-only mode */
    openReadOnly: 'openReadOnly'
 }
/**
* Base constructor parameter object
* @typedef {Object} BaseConstructorParam
* @property  {module:qm~baseModes} [BaseConstructorParam.mode='openReadOnly'] - Base access mode: 
* <br> create (sets up the db folder), 
* <br> createClean (cleans db folder and then sets it up), 
* <br> open (opens the db with read/write permissions), 
* <br> openReadOnly (opens the db in read only mode).
* @property  {number} [BaseConstructorParam.indexCache=1024] - The ammount of memory reserved for indexing (in MB).
* @property  {number} [BaseConstructorParam.storeCache=1024] - The ammount of memory reserved for store cache (in MB).
* @property  {string} [BaseConstructorParam.schemaPath=''] - The path to schema definition file.
* @property  {Array<module:qm~SchemaDefinition>} [BaseConstructorParam.schema=[]] - Schema definition object array.
* @property  {string} [BaseConstructorParam.dbPath='./db/'] - The path to db directory.
*/
/**
* Store schema definition object
* @typedef {Object} SchemaDefinition
* @property {string} name - The name of the store. Store name can be composed by from English letters, numbers, _ or $ characters. It can only begin with a character.
* @property {Array<module:qm~SchemaFieldDefinition>} fields - The array of field descriptors. 
* @property {Array<module:qm~SchemaJoinDefinition>} [joins=[]] - The array of join descriptors, used for linking records from different stores.
* @property {Array<module:qm~SchemaKeyDefinition>} [keys=[]] - The array of key descriptors. Keys define how records are indexed, which is needed for search using the query language.
* @property {module:qm~SchemaTimeWindowDefinition} [timeWindow] - Time window description. Stores can have a window, which is used by garbage collector to delete records once they fall out of the time window. Window can be defined by number of records or by time.
* @example
* var qm = require('qminer');
* // create a simple movies store, where each record contains only the movie title.
* var base = new qm.Base({
*     mode: 'createClean',
*     schema: [{
*       "name": "Movies",
*       "fields": [{ name: "title", type: "string" }]
*     }]
* });
*/
/**
* Field types.
* @readonly
* @enum {string}
*/
 var fieldTypes = {
    /** signed 32-bit integer */
    int: 'int', 
    /** vector of signed 32-bit integers */
    int_v: 'int_v', 
 /** string */
 string : 'string',
 /** vector of strings */
 string_v : 'string_v',
 /** boolean */
 bool : 'bool',
 /** double precision floating point number */
 float : 'float',
 /** a pair of floats, useful for storing geo coordinates */
 float_pair : 'float_pair',
 /** vector of floats */
 float_v : 'float_v',
 /** date and time format, stored in a form of milliseconds since 1600 */
 datetime : 'datetime',
 /** sparse vector(same format as used by QMiner JavaScript linear algebra library) */
 num_sp_v : 'num_sp_v',
 }
/**
* Store schema field definition object
* @typedef {Object} SchemaFieldDefinition
* @property {string} name - The name of the field.
* @property {module:qm~fieldTypes} type - The type of the field.
* @property {boolean} [primary=false] - Field which can be used to identify record. There can be only one primary field in a store. There can be at most one record for each value of the primary field. Currently following fields can be marked as primary: int, uin64, string, float, datetime. Primary fields of type string are also used for record names.
* @property {boolean} [null=false] - When set to true, null is a possible value for a field (allow missing values).
* @property {string} [store='memory'] - Defines where to store the field, options are: <b>'cache'</b> or <b>'memory'</b>. The default option is <b>'memory'</b>, which stores the values in RAM. Option <b>'cache'</b> stores the values on disk, with a layer of FIFO cache in RAM, storing the most recently used values.
* @property {Object} [default] - Default value for field when not given for a new record.
* @property {boolean} [codebook=false] - Useful when many records have only few different values of this field. If set to true, then a separate table of all values is kept, and records only point to this table (replacing variable string field in record serialisation with fixed-length integer). Useful to decrease memory footprint, and faster to update. (STRING FIELD TYPE SPECIFIC).
* @property {boolean} [shortstring=false] - Useful for string shorter then 127 characters (STRING FIELD TYPE SPECIFIC).
* @example
*  var qm = require('qminer');
*  var base = new qm.Base({
*      mode: 'createClean',
*      schema: [
*        { name: 'NewsArticles',
*          fields: [
*            { name: "ID", primary: true, type: "string", shortstring: true },
*            { name: "Source", type: "string", codebook: true },
*            { name: "DateTime", type: "datetime" },
*            { name: "Title", type: "string", store: "cache" },
*            { name: "Tokens", type: "string_v", store: "cache", null: true },
*            { name: "Vector", type: "num_sp_v", store: "cache", null: true }]
*        }
*     ]
*  });
* // add a record:
* // - we set the date using the ISO string representation
* // - we set the string vector Tokens with an array of strings
* // - we set the numeric sparse vector Vector with an array of two element arrays
* //   (index, value), see the sparse vector constructor {@link module:la.SparseVector}
* base.store('NewsArticles').push({
*   ID: 't12344', 
*   Source: 's1234', 
*   DateTime: '2015-01-01T00:05:00', 
*   Title: 'the title', 
*   Tokens: ['token1', 'token2'], 
*   Vector: [[0,1], [1,1]]})
*/
/**
* Store schema join definition object
* @typedef {Object} SchemaJoinDefinition
* @property {string} name - The name of the join.
* @property {string} type - The supported types are: <b>'field'</b> and <b>'index'</b>. 
* <br> A join with type=<b>'field'</b> can point to zero or one record and is implemented as an additional hidden field of type uint64, which can hold the ID of the record it links to. Accessing the record's join returns a record.
* <br> A join with type=<b>'index'</b> can point to any number of records and is implemented using the inverted index, where for each record a list (vector) of linked records is kept. Accessing the record's join returns a record set.
* @property {string} store - The store name from which the linked records are.
* @example
* var qm = require('qminer');
* // Create two stores: People which stores only names of persons and Movies, which stores only titles.
* // Each person can direct zero or more movies, so we use an index join named 'directed' and
* // each movie has a single director, so we use a field join 'director'. The joins are 
* // inverses of each other. The inverse join simplifies the linking, since only one join needs
* // to be specified, and the other direction can be linked automatically (in the example 
* // below we specify only the 'director' link and the 'directed' join is updated automatically).
* //
* var base = new qm.Base({
*     mode: 'createClean',
*     schema: [
*       { name: 'People', 
*         fields: [{ name: 'name', type: 'string', primary: true }], 
*         joins: [{ name: 'directed', 'type': 'index', 'store': 'Movies', 'inverse': 'director' }] },
*       { name: 'Movies', 
*         fields: [{ name: 'title', type: 'string', primary: true }], 
*         joins: [{ name: 'director', 'type': 'field', 'store': 'People', 'inverse': 'directed' }] }
*     ]
* });
* // Adds a movie, automatically adds 'Jim Jarmusch' to People, sets the 'director' join (field join)
* // and automatically updates the index join 'directed', since it's an inverse join of 'director'
* base.store('Movies').push({ title: 'Broken Flowers', director: { name: 'Jim Jarmusch' } });
* // Adds a movie, sets the 'director' join, updates the index join of 'Jim Jarmusch'
* base.store('Movies').push({ title: 'Coffee and Cigarettes', director: { name: 'Jim Jarmusch' } });
* // Adds movie, automatically adds 'Lars von Trier' to People, sets the 'director' join
* // and 'directed' inverse join (automatically)
* base.store('Movies').push({ title: 'Dogville', director: { name: 'Lars von Trier' } });
*
* var movie = base.store('Movies')[0]; // get the first movie (Broken Flowers)
* // Each movie has a property corresponding to the join name: 'director'. 
* // Accessing the property returns a {@link module:qm.Record} from the store People.
* var person = movie.director; // get the director
* console.log(person.name); // prints 'Jim Jarmusch'
* // Each person has a property corresponding to the join name: 'directed'. 
* // Accessing the property returns a {@link module:qm.RecSet} from the store People.
* var movies = person.directed; // get all the movies the person directed.
* movies.each(function (movie) { console.log(movie.title); }); 
* // prints: 
* //   'Broken Flowers'
* //   'Coffee and Cigarettes'
*/
/**
* Store schema key definition object
* @typedef {Object} SchemaKeyDefinition
* @property {string} field - The name of the field that will be indexed.
* @property {string} type - The supported types are: <b>'value'</b>, <b>'text'</b> and <b>'location'</b>.
* <br> A key with type=<b>'value'</b> indexes records using an inverted index using full value of the field (no processing).
*  The key type supports 'string', 'string_v' and 'datetime' fields types.
* <br> A key with type=<b>'text'</b> indexes string fields by using a tokenizer and text processing. Supported by string fields.
* <br> A key with type=<b>'location'</b> indexes records as points on a sphere and enables nearest-neighbour queries. Supported by float_pair type fields.
* @property {string} [name] - Allows using a different name for the key in search queries. This allows for multiple keys to be put against the same field. Default value is the name of the field.
* @property {string} [vocabulary] - defines the name of the vocabulary used to store the tokens or values. This can be used indicate to several keys to use the same vocabulary, to save on memory. Supported by 'value' and 'text' keys.
* @property {string} [tokenize] - defines the tokenizer that is used for tokenizing the values stored in indexed fields. Tokenizer uses same parameters as in bag-of-words feature extractor. Default is english stopword list and no stemmer. Supported by 'text' keys.
* @example
* var qm = require('qminer');
* // Create a store People which stores only names of persons.
* var base = new qm.Base({
*     mode: 'createClean',
*     schema: [
*         { name: 'People',
*           fields: [{ name: 'name', type: 'string', primary: true }],
*           keys: [
*             { field: 'name', type: 'value'}, 
*             { field: 'name', name: 'nameText', type: 'text'}
*          ]
*        }
*     ]
* });
*
* base.store('People').push({name : 'John Smith'});
* base.store('People').push({name : 'Mary Smith'});
* // search based on indexed values
* base.search({$from : 'People', name: 'John Smith'}); // Return the record set containing 'John Smith'
* // search based on indexed values
* base.search({$from : 'People', name: 'Smith'}); // Returns the empty record set.
* // search based on text indexing
* base.search({$from : 'People', nameText: 'Smith'}); // Returns both records.
*/
/**
* Stores can have a window, which is used by garbage collector to delete records once they
* fall out of the time window. Window can be defined by number of records or by time.
* Window defined by parameter window, its value being the number of records to be kept.
* @typedef {Object} SchemaTimeWindowDefinition
* @property {number} duration - the size of the time window (in number of units).
* @property {string} unit - defines in which units the window size is specified. Possible values are <b>second</b>, <b>minute</b>, <b>hour</b>, <b>day</b>, <b>week</b> or <b>month</b>.
* @property {string} [field] - name of the datetime filed, which defines the time of the record. In case it is not given, the insert time is used in its place.
* @example
* var qm = require('qminer');
* // Create a store
* var base = new qm.Base([{
* // ...
*   timeWindow : { 
*     duration : 12,
*     unit : "hour",
*     field : "DateTime"
*   }
* }]);
*/
/**
* Feature extractor types.
* @typedef {Object} FeatureExtractors
* @property {module:qm~FeatureExtractorConstant} constant - The constant type.
* @property {module:qm~FeatureExtractorRandom} random - The random type.
* @property {module:qm~FeatureExtractorNumeric} numeric - The numeric type.
* @property {module:qm~FeatureExtractorCategorical} categorical - The categorical type.
* @property {module:qm~FeatureExtractorMultinomial} multinomial - The multinomial type.
* @property {module:qm~FeatureExtractorText} text - The text type.
* @property {module:qm~FeatureExtractorJoin} join - The join type.
* @property {module:qm~FeatureExtractorPair} pair - The pair type.
* @property {module:qm~FeatureExtractorJsfunc} jsfunc - The jsfunc type.
* @property {module:qm~FeatureExtractorDateWindow} dateWindow - The dateWindow type.
*
*/
/**
* Feature type: constant
* @typedef {Object} FeatureExtractorConstant
* @property {string} type - The type of the extractor. It must be equal 'constant'.
* @property {number} [const = 1.0] - A constant number. 
* @property {module:qm~FeatureSource} source - The source of the extractor.
* @example
* var qm = require('qminer');
* // create a simple base, where each record contains only a persons name
* var base = new qm.Base({
*   mode: 'createClean',
*   schema: [{
*      "name": "Person",
*      "fields": [{ "name": "Name", "type": "string" }]
*   }]
* });
* // create a feature space containing the constant extractor, where the constant is equal 5
* var ftr = qm.FeatureSpace(base, { type: "constant", source: "Person", const: 5 });
*/
/**
* Feature type: random
* @typedef {Object} FeatureExtractorRandom
* @property {string} type - The type of the extractor. It must be equal 'random'.
* @property {number} [seed = 0] - A random seed number.
* @property {module:qm~FeatureSource} source - The source of the extractor.
* @example
* var qm = require('qminer');
* // create a simple base, where each record contains only a persons name
* var base = new qm.Base({
*   mode: 'createClean',
*   schema: [{
*      "name": "Person",
*      "fields": [{ "name": "Name", "type": "string" }]
*   }]
* });
* // create a feature space containing the random extractor
* var ftr = qm.FeatureSpace(base, { type: "random", source: "Person" });
*/
/**
* Feature type: numeric
* @typedef {Object} FeatureExtractorNumeric 
* @property {string} type - The type of the extractor. It must be equal 'numeric'.
* @property {boolean} [normalize = 'false'] - Normalize values between 0.0 and 1.0.
* @property {number} [min] - The minimal value used to form the normalization.
* @property {number} [max] - The maximal value used to form the normalization.
* @property {string} field - The name of the field from which to take the value.
* @property {module:qm~FeatureSource} source - The source of the extractor.
* @example
* var qm = require('qminer');
* // create a simple base, where each record contains the student name and it's grade
* var base = new qm.Base({
*    mode: 'createClean',
*    schema: [{
*       "name": "Class",
*       "fields": [
*          { "name": "Name", "type": "string" },
*          { "name": "Grade", "type": "number" }
*       ]
*    }]
* });
* // create a feature space containing the numeric extractor, where the values are 
* // normalized, the values are taken from the field "Grade"
* var ftr = qm.FeatureSpace(base, { type: "numeric", source: "Class", normalize: true, field: "Grade" });
*/
/**
* Feature type: categorical
* @typedef {Object} FeatureExtractorCategorical
* @property {string} type - The type of the extractor. It must be equal 'categorical'.
* @property {Array.<Object>} [values] - A fixed set of values, which form a fixed feature set. No dimensionalizy changes if new values are seen in the upgrades.
* @property {number} [hashDimension] - A hashing code to set the fixed dimensionality. All values are hashed and divided modulo hasDimension to get the corresponding dimension.
* @property {string} field - The name of the field form which to take the values.
* @property {module:qm~FeatureSource} source - The source of the extractor.
* @example
* var qm = require('qminer');
* // create a simple base, where each record contains the student name and it's study group
* // here we know the student is part of only one study group
* var base = new qm.Base({
*    mode: 'createClean',
*    schema: [{
*       "name": "Class",
*       "fields": [
*          { "name": "Name", "type": "string" },
*          { "name": "StudyGroup", "type": "string" }
*       ]
*    }]
* });
* // create a feature space containing the categorical extractor, where the it's values
* // are taken from the field "StudyGroup": "A", "B", "C" and "D"
* var ftr = qm.FeatureSpace(base, { type: "categorical", source: "Class", field: "StudyGroup", values: ["A", "B", "C", "D"] });
*/
/**
* Feature type: multinomial
* @typedef {Object} FeatureExtractorMultinomial
* @property {string} type - The type of the extractor. It must be equal 'multinomial'.
* @property {boolean} [normalize = 'false'] - Normalize the resulting vector of the extractor to have L2 norm 1.0.
* @property {Array.<Object>} [values] - A fixed set of values, which form a fixed feature set, no dimensionality changes if new values are seen in the updates.
* @property {number} [hashDimension] - A hashing code to set the fixed dimensionality. All values are hashed and divided modulo hashDimension to get the corresponding dimension.
* @property {Object} [datetime = false] - Same as 'values', only with predefined values which are extracted from date and time (month, day of month, day of week, time of day, hour).
* <br> This fixes the dimensionality of feature extractor at the start, making it not dimension as new dates are seen. Cannot be used the same time as values.
* @property {string} field - The name of the field from which to take the value.
* @property {module:qm~FeatureSource} source - The source of the extractor.
* @example
* var qm = require('qminer');
* // create a simple base, where each record contains the student name and an array of study groups
* // here we know a student can be part of multiple study groups
* var base = new qm.Base({
*    mode: 'createClean',
*    schema: [{
*       "name": "Class",
*       "fields": [
*          { "name": "Name", "type": "string" },
*          { "name": "StudyGroups", "type": "string_v" }
*       ]
*    }]
* });
* // create a feature space containing the multinomial extractor, where the values are normalized,
* // and taken from the field "StudyGroup": "A", "B", "C", "D", "E", "F"
* var ftr = qm.FeatureSpace(base, { 
*              type: "multinomial", source: "CLass", field: "StudyGroups", normalize: true, values: ["A", "B", "C", "D", "E", "F"]
*           });
*/
/**
* Feature type: text
* @typedef {Object} FeatureExtractorText
* @property {string} type - The type of the extractor. It must be equal 'text'.
* @property {boolean} [normalize = 'true'] - Normalize the resulting vector of the extractor to have L2 norm 1.0.
* @property {module:qm~FeatureWeight} [weight = 'tfidf'] - Type of weighting used for scoring terms.
* @property {number} [hashDimension] - A hashing code to set the fixed dimensionality. All values are hashed and divided modulo hashDimension to get the corresponding dimension.
* @property {string} field - The name of the field from which to take the value.
* @property {module:qm~FeatureTokenizer} tokenizer - The settings for extraction of text.
* @property {module:qm~FeatureMode} mode - How are multi-record cases combined into single vector.
* @property {module:qm~FeatureStream} stream - Details on forgetting old IDFs when running on stream.
* @property {module:qm~FeatureSource} source - The source of the extractor.
* @example
* var qm = require('qminer');
* // create a simple base, where each record contains the title of the article and it's content
* var base = new qm.Base({
*    mode: 'createClean',
*    schema: [{
*       "name": "Articles",
*       "fields": [
*          { "name": "Title", "type": "string" },
*          { "name": "Text", "type": "string" }
*       ]
*    }]
* });
* // create a feature spave containing the text (bag of words) extractor, where the values are normalized,
* // weighted with 'tfidf' and the tokenizer is of 'simple' type, it uses english stopwords.
* var ftr = qm.FeatureSpace(base, { 
*              type: "text", source: "Articles", field: "Text", normalize: true, weight: "tfidf",
*              tokenizer: { type: "simple", stopwords: "en"}
*           });
*/
/**
* Feature type: join
* @typedef {Object} FeatureExtractorJoin
* @property {string} type - The type of the extractor. It must be equal 'join'.
* @property {number} [bucketSize = 1] - The size of the bucket in which we group consecutive records.
* @property {module:qm~FeatureSource} source - The source of the extractor.
*/
/**
* Feature type: pair
* @typedef {Object} FeatureExtractorPair
* @property {string} type - The type of the extractor. It must be equal 'pair'.
* @property {module:qm~FeatureExtractors} first - The first feature extractor.
* @property {module:qm~FeatureExtractors} second - The second feature extractor.
* @property {module:qm~FeatureSource} source - The source of the extractor.
*/
/** 
* Feature type: dateWindow
* @typedef {Object} FeatureExtractorDateWindow
* @property {string} type - The type of the extractor. It must be equal 'dateWindow'.
* @property {string} [unit = 'day'] - How granular is the time window. Options: day, week, month, year, 12hours, 6hours, 4hours, 2hours,
* hour, 30minutes, 15minutes, 10minutes, minute, second.
* @property {number} [window = 1] - The size of the window.
* @property {boolean} [normalize = 'false'] - Normalize the resulting vector of the extractor to have L2 norm 1.0. //TODO
* @property {number} start - //TODO
* @property {number} end - //TODO
* @property {module:qm~FeatureSource} source - The source of the extractor.
*/
/**
* Feature type: jsfunc
* @typedef {Object} FeatureExtractorJsfunc
* @property {string} type - The type of the extractor. It must be equal 'jsfunc'.
* @property {string} name - The feature's name.
* @property {function} fun - The javascript function callback. It should take a record as input and return a number or a dense vector.
* @property {number} [dim = 1] - The dimension of the feature extractor.
* @property {module:qm~FeatureSource} source - The source of the extractor.
* @example
* var qm = require('qminer');
* // create a simple base, where each record contains the name of the student and his study groups
* // each student is part of multiple study groups
* var base = new qm.Base({
*    mode: 'createClean',
*    schema: [{
*       "name": "Class",
*       "fields": [
*          { "name": "Name", "type": "string" },
*          { "name": "StudyGroups", "type": "string_v" }
*       ]
*    }]
* });
* // create a feature space containing the jsfunc extractor, where the function counts the number 
* // of study groups each student is part of. The functions name is "NumberOFGroups", it's dimension
* // is 1 (returns only one value, not an array)
* var ftr = qm.FeatureSpace(base, {
*              type: "jsfunc", source: "Class", name: "NumberOfGroups", dim: 1,
*              fun: function (rec) { returns rec.StudyGroups.length; }
*           });
*/
/**
* From where are the input records taken.
* @typedef {Object} FeatureSource
* @property {string} store - The store name.
*/
/**
* Type of weighting used for scoring terms.
* @readonly
* @enum {string}
*/
 var FeatureWeight = {
 /** Sets 1 if term occurs, 0 otherwise. */
 none: 'none',
 /** Sets the term frequency in the document. */
 tf: 'tf',
 /** Sets the inverse document frequency in the document. */
 idf: 'idf',
 /** Sets the product of the tf and idf score. */
 tfidf: 'tfidf'
 }
/**
* The settings for extraction of text.
* @typedef {Object} FeatureTokenizer
* @property {module:qm~FeatureTokenizerType} [type = 'simple'] - The type of the encoding text.
* @property {module:qm~FeatureTokenizerStopwords} [stopwords = 'en'] - The stopwords used for extraction.
* @property {module:qm~FeatureTokenizerStemmer} [stemmer = 'none'] - The stemmer used for extraction.
* @property {boolean} [uppercase = 'true'] - Changing all words to uppercase.
*/
/**
* The type of the encoding text.
* @readonly
* @enum {string}
*/
 var FeatureTokenizerType = {
	/** The simple encoding. */
	simple: 'simple',
	/** The html encoding. */
 html: 'html',
 /** The unicode encoding. */
 unicode: 'unicode'
 }
/**
* THe stopwords used for extraction.
* @readonly
* @enum {Object}
*/
 var FeatureTokenizerStopwords = {
 /** The pre-defined stopword list (none). */
 none: 'none',
 /** The pre-defined stopword list (english). */
 en: 'en',
 /** The pre-defined stopword list (slovene). */
 si: 'si',
 /** The pre-defined stopword list (spanish). */
 es: 'es',
 /** The pre-defined stopword list (german). */
 de: 'de',
 /** An array of stopwords. The array must be given as a parameter instead of 'array'! */
 array: 'array'
 }
/**
* The steemer used for extraction.
* @readonly
* @enum {Object}
*/
 var FeatureTokenizerStemmer = {
 /** For using the porter stemmer. */
 boolean: 'true',
 /** For using the porter stemmer. */
 porter: 'porter',
 /** For using no stemmer. */
 none: 'none',
 }
/**
* How are multi-record cases combined into a single vector. //TODO not implemented for join record cases (works only if the start store and the 
* feature store are the same)
* @readonly
* @enum {string}
*/
 var FeatureMode = {
		/** Multi-record cases are merged into one document. */
		concatenate: 'concatenate', 
		/** Treat each case as a separate document. */
		centroid: 'centroid', 
		/** //TODO (Use the tokenizer option) */
		tokenized : 'tokenized'
 }
/**
* Details on forgetting old IDFs when running on stream.
* @readonly
* @enum {string}
*/
 var FeatureStream = {
 /** (optional) Field name which is providing timestamp, if missing system time is used. */
 field: 'field',
 /** Forgetting factor, by which the old IDFs are multiplied after each iteration. */
 factor: 'factor',
 /** The time between iterations when the factor is applied, standard JSon time format is used to specify the interval duration. */
 interval: 'interval'
 }
/**
* Base
* @classdesc Represents the database and holds stores.
* @class
* @param {module:qm~BaseConstructorParam} paramObj - The base constructor parameter object.
* @example
* // import qm module
* var qm = require('qminer');
* // using a constructor, in open mode:
* var base = new qm.Base({mode: 'open'});
*/
 exports.Base = function (paramObj) {};
/**
	* Closes the database.
	* @returns {null}
	*/
 exports.Base.prototype.close = function () {}
/**
	 * Returns the store with the specified name.
	 *
	 * @param {string} name - Name of the store.
	 * @returns {module:qm.Store} The store.
	 * @example
	 * // import qm module
	 * var qm = require('qminer');
	 * // create a base with two stores
	 * var base = new qm.Base({
	 *    mode: "createClean",
	 *    schema: [
	 *    {
	 *        "name": "Kwik-E-Mart",
	 *        "fields": [
	 *            { "name": "Worker", "type": "string" },
	 *            { "name": "Groceries", "type": "string_v" }
	 *        ]
	 *    },
	 *    {
	 *        "name": "NuclearPowerplant",
	 *        "fields": [
	 *            { "name": "Owner", "type": "string" },
	 *            { "name": "NumberOfAccidents", "type": "int" },
	 *            { "name": "Workers", "type": "string_v" }
	 *        ]
	 *    }]
	 * })
	 * // get the "Kwik-E-Mart" store 
	 * var store = base.store("Kwik-E-Mart");	// returns the store with the name "Kwik-E-Mart"
	 */
 exports.Base.prototype.store = function (name) { return Object.create(require('qminer').Store.prototype); }
/**
	 * Returns a list of store descriptors.
	 *
	 * @returns {Object[]}
	 */
 exports.Base.prototype.getStoreList = function () { return [{storeId:'', storeName:'', storeRecords:'', fields: [], keys: [], joins: []}]; }
/**
	* Creates a new store.
	* @param {Array<module:qm~SchemaDefinition>} storeDef - The definition of the store(s).
	* @param {number} [storeSizeInMB = 1024] - The reserved size of the store(s).
	* @returns {(module:qm.Store | module:qm.Store[])} - Returns a store or an array of stores (if the schema definition was an array).
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base with one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [
	*    {
	*        "name": "Superheroes",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "Superpowers", "type": "string_v" },
	*            { "name": "YearsActive", "type": "int" }
	*        ]
	*    }]
	* })
	* // create a new store called "Supervillains" in the base
	* base.createStore({
	*    "name": "Supervillians"
	*    "fields": [
	*        { "name": "Name", "type": "string" },
	*        { "name": "Superpowers", "type": "string_v" },
	*        { "name": "YearsActive", "type": "int" }
	*    ]
	* })
	* // create two new stores called "Cities" and "Leagues"
	* base.createStore([
	*    {
	*        "name": "Cities",
	*        "fields": [
	*            { "name": "Name", "type": "string", "primary": true },
	*            { "name": "Population", "type": "int" }
	*        ]
	*    },
	*    {
	*        "name": "Leagues",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "Members", "type": "string_v" }
	*        ]
	*    }
	* ])
	*/
 exports.Base.prototype.createStore = function (storeDef, storeSizeInMB) { return storeDef instanceof Array ? [Object.create(require('qminer').Store.prototype)] : Object.create(require('qminer').Store.prototype) ;}
/**
	* Creates a new store.
	* @param {module:qm~QueryObject} query - query language JSON object	
	* @returns {module:qm.RecordSet} - Returns the record set that matches the search criterion
	*/
 exports.Base.prototype.search = function (query) { return Object.create(require('qminer').RecordSet.prototype);}
/**
	* Calls qminer garbage collector to remove records outside time windows.
	*/
 exports.Base.prototype.garbageCollect = function () { }
/**
	* Calls qminer partial flush - base saves dirty data given some time-window.
	* @param {number} window - Length of available time-window in msec. Default 500.
	*/
 exports.Base.prototype.partialFlush = function () { }
/**
	* Retrieves performance statistics for qminer.
	*/
 exports.Base.prototype.getStats = function () { }
/**
* Store (factory pattern result) 
* @namespace
* @example
* // import qm module
* var qm = require('qminer');
* // factory based construction using base.createStore
* var base = qm.create('qm.conf', "", true);
* base.createStore([{
*    "name": "People",
*    "fields": [
*        { "name": "Name", "type": "string", "primary": true },
*        { "name": "Gender", "type": "string", "shortstring": true },
*        { "name": "Age", "type": "int" }
*    ],
*    "joins": [
*        { "name": "ActedIn", "type": "index", "store": "Movies", "inverse": "Actor" },
*        { "name": "Directed", "type": "index", "store": "Movies", "inverse": "Director" }
*    ],
*    "keys": [
*        { "field": "Name", "type": "text" },
*        { "field": "Gender", "type": "value" }
*    ]
* },
* {
*    "name": "Movies",
*    "fields": [
*        { "name": "Title", "type": "string", "primary": true },
*        { "name": "Plot", "type": "string", "store": "cache" },
*        { "name": "Year", "type": "int" },
*        { "name": "Rating", "type": "float" },
*        { "name": "Genres", "type": "string_v", "codebook": true }
*    ],
*    "joins": [
*        { "name": "Actor", "type": "index", "store": "People", "inverse": "ActedIn" },
*        { "name": "Director", "type": "index", "store": "People", "inverse": "Directed" }
*    ],
*    "keys": [
*        { "field": "Title", "type": "value" },
*        { "field": "Plot", "type": "text", "vocabulary": "voc_01" },
*        { "field": "Genres", "type": "value" }
*    ]
* }]);
* // using the base constructor
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [{
*        "name": "Class",
*        "fields": [
*            { "name": "Name", "type": "string" },
*            { "name": "StudyGroup", "type": "string" }
*        ]
*    }]
* });
*/
 exports.Store = function (base, storeDef) {};
/**
	* Returns a record from the store.
	* @param {string} recName - Record name.
	* @returns {(module:qm.Record | null)} Returns the record. If the record doesn't exist, it returns null.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing the store Class. Let the Name field be the primary field. 
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "Class",
	*        "fields": [
	*            { "name": "Name", "type": "string", "primary": true },
	*            { "name": "StudyGroup", "type": "string" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Class").push({ "Name": "Dean", "StudyGroup": "A" });
	* base.store("Class").push({ "Name": "Chang", "StudyGroup": "D" });
	* base.store("Class").push({ "Name": "Magnitude", "StudyGroup": "C" });
	* base.store("Class").push({ "Name": "Leonard", "StudyGroup": "B" });
	* // get the record with the name "Magnitude"
	* var record = base.store("Class").rec("Magnitude");
	*/
 exports.Store.prototype.rec = function (recName) {};
/**
	* Executes a function on each record in store.
	* @param {function} callback - Function to be executed. It takes two parameters:
	* <br>rec - The current record.
	* <br>[idx] - The index of the current record.
	* @returns {module:qm.Store} Self.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing the store Class
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "Class",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "StudyGroup", "type": "string" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Class").push({ "Name": "Abed", "StudyGroup": "A" });
	* base.store("Class").push({ "Name": "Annie", "StudyGroup": "B" });
	* base.store("Class").push({ "Name": "Britta", "StudyGroup": "C" });
	* base.store("Class").push({ "Name": "Jeff", "StudyGroup": "A" });
	* // change the StudyGroup of all records of store Class to A
	* base.store("Class").each(function (rec) { rec.StudyGroup = "A"; });	// all records in Class are now in study group A
	*/
 exports.Store.prototype.each = function (callback) {}
/**
	* Creates an array of function outputs created from the store records.
	* @param {function} callback - Function that generates the array. It takes two parameters:
	* <br>rec - The current record.
	* <br>[idx] - The index of the current record.
	* @returns {Array<Object>} The array created by the callback function. //TODO
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing the store Class
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "Class",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "StudyGroup", "type": "string" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Class").push({ "Name": "Shirley", "StudyGroup": "A" });
	* base.store("Class").push({ "Name": "Troy", "StudyGroup": "B" });
	* base.store("Class").push({ "Name": "Chang", "StudyGroup": "C" });
	* base.store("Class").push({ "Name": "Pierce", "StudyGroup": "A" });
	* // make an array of record names
	* var arr = base.store("Class").map(function (rec) { return rec.Name; }); // returns an array ["Shirley", "Troy", "Chang", "Pierce"]
	*/
 exports.Store.prototype.map = function (callback) {}
/**
	* Adds a record to the store.
	* @param {Object} rec - The added record. The record must be a JSON object corresponding to the store schema.
	* @returns {number} The ID of the added record.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing two stores
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [
	*    {
	*        "name": "Superheroes",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "Superpowers", "type": "string_v" }
	*        ]
	*    },
	*    {
	*        "name": "Supervillians",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "Superpowers", "type": "string_v" }
	*        ]
	*    }]
	* });
	* // add a new superhero to the Superheroes store
	* base.store("Superheroes").push({ "Name": "Superman", "Superpowers": ["flight", "heat vision", "bulletproof"] }); // returns 0
	* // add a new supervillian to the Supervillians store
	* base.store("Supervillians").push({ "Name": "Lex Luthor", "Superpowers": ["expert engineer", "genius-level intellect", "money"] }); // returns 0
	*/
 exports.Store.prototype.push = function (rec) {}
/**
	* Creates a new record of given store. The record is not added to the store.
	* @param {Object} json - A JSON value of the record.
	* @returns {module:qm.Record} The record created by the JSON value and the store.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "Planets",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "Diameter", "type": "int" },
	*            { "name": "NearestStars", "type": "string_v" }
	*        ]
	*    }]
	* });
	* // add a new planet in the store
	* base.store("Planets").push({ "Name": "Earth", "Diameter": 299196522, "NearestStars": ["Sun"] });
	* // create a record of a planet (not added to the Planets store)
	* var planet = base.store("Planets").newRecord({ "Name": "Tatooine", "Diameter": 10465, "NearestStars": ["Tatoo 1", "Tatoo 2"] });
	*/
 exports.Store.prototype.newRecord = function (json) {};
/**
	* Creates a new record set out of the records in store.
	* @param {module:la.IntVector} idVec - The integer vector containing the ids of selected records.
	* @returns {module:qm.RecordSet} The record set that contains the records gained with idVec.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "Superheroes",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "Superpowers", "type": "string_v" }
	*        ]
	*    }]
	* });
	* // add some new records to the store
	* base.store("Superheroes").push({ "Name": "Superman", "Superpowers": ["Superhuman strength, speed, hearing", "Flight", "Heat vision"] });
	* base.store("Superheroes").push({ "Name": "Batman", "Superpowers": ["Genius-level intellect", "Peak physical and mental conditioning", "Master detective"] });
	* base.store("Superheroes").push({ "Name": "Thor", "Superpowers": ["Superhuman strength, endurance and longevity", "Abilities via Mjolnir"] });
	* base.store("Superheroes").push({ "Name": "Wonder Woman", "Superpowers": ["Superhuman strength, agility and endurance", "Flight", "Highly skilled hand-to-hand combatant"] });
	* // create a new record set containing only the DC Comic superheroes (those with the record ids 0, 1 and 3)
	* var intVec = new qm.la.IntVector([0, 1, 3]);
	* var DCHeroes = base.store("Superheroes").newRecordSet(intVec);
	*/
 exports.Store.prototype.newRecordSet = function (idVec) {};
/**
	* Creates a record set containing random records from store.
	* @param {number} sampleSize - The size of the record set.
	* @returns {module:qm.RecordSet} Returns a record set containing random records.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "TVSeries",
	*        "fields": [
	*            { "name": "Title", "type": "string", "primary": true },
	*            { "name": "NumberOfEpisodes", "type": "int" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("TVSeries").push({ "Title": "Archer", "NumberOfEpisodes": 75 });
	* base.store("TVSeries").push({ "Title": "The Simpsons", "NumberOfEpisodes": 574 });
	* base.store("TVSeries").push({ "Title": "New Girl", "NumberOfEpisodes": 94 });
	* base.store("TVSeries").push({ "Title": "Rick and Morty", "NumberOfEpisodes": 11 });
	* base.store("TVSeries").push({ "Title": "Game of Thrones", "NumberOfEpisodes": 47 });
	* // create a sample record set containing 3 records
	* var randomRecordSet = base.store("TVSeries").sample(3); // contains 3 random records from the TVSeries store
	*/
 exports.Store.prototype.sample = function (sampleSize) {};
/**
	* Gets the details of the selected field.
	* @param {string} fieldName - The name of the field.
	* @returns {Object} The JSON object containing the details of the field. 
	* @example
	* // import qm module
	* var qm = require("qminer");
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "People",
	*        "fields": [
	*            { "name": "Name", "type": "string", "primary": true },
	*            { "name": "Gender", "type": "string" },
	*            { "name": "Age", "type": "int" }
	*        ]
	*    }]
	* });
	* // get the details of the field "Name" of store "People"
	* // it returns a JSON object:
	* // { id: 0, name: "Name", type: "string", primary: true }
	* var details = base.store("People").field("Name");
	*/
 exports.Store.prototype.field = function (fieldName) {}; 
/**
	* Checks if the field is of numeric type.
	* @param {string} fieldName - The checked field.
	* @returns {boolean} True, if the field is of numeric type. Otherwise, false.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "TVSeries",
	*        "fields": [
	*            { "name": "Title", "type": "string", "primary": true },
	*            { "name": "NumberOfEpisodes", "type": "int" }
	*        ]
	*    }]
	* });
	* // check if the field "Title" is of numeric type
	* var isTitleNumeric = base.store("TVSeries").isNumeric("Title"); // returns false
	* // check if the field "NumberOfEpisodes" is of numeric type
	* var isNumberOfEpisodesNumeric = base.store("TVSeries").isNumeric("NumberOfEpisodes"); // returns true
	*/
 exports.Store.prototype.isNumeric = function (fieldName) {};
/**
	* Checks if the field is of string type.
	* @param {string} fieldName - The checked field.
	* @returns {boolean} True, if the field is of string type. Otherwise, false.
	* @example
	* // import qm module
	* var qm = require("qminer");
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "People",
	*        "fields": [
	*            { "name": "Name", "type": "string", "primary": true },
	*            { "name": "Gender", "type": "string" },
	*            { "name": "Age", "type": "int" }
	*        ]
	*    }]
	* });
	* // check if the field "Name" is of string type
	* var isNameString = base.store("People").isString("Name"); // returns true
	* // check if the field "Age" is of string type
	* var isAgeString = base.store("People").isString("Age"); // returns false
	*/
 exports.Store.prototype.isString = function (fieldName) {}; 
/**
	* Checks if the field is of type Date.
	* @param {string} fieldName - The checked field.
	* @returns {boolean} True, if the field is of type Date. Otherwise, false.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "BasketballPlayers",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "SeasonScore", "type": "int_v" },
	*            { "name": "DateOfBirth", "type": "datetime" }
	*        ]
	*    }]
	* });
	* // check if the SeasonScore field is of type Date
	* var isSeasonScoreDate = base.store("BasketballPlayers").isDate("SeasonScore"); // returns false
	* // check if the FirstPlayed field is of type Date
	* var isFirstPlayedDate = base.store("BasketballPlayers").isDate("DateOfBirth"); // returns true
	*/
 exports.Store.prototype.isDate = function (fieldName) {}
/**
	* Returns the details of the selected key as a JSON object.
	* @param {string} keyName - The selected key as a JSON object.
	* @returns {Object} The JSON object containing the details of the key.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "Countries",
	*        "fields": [
	*            { "name": "Name", "type": "string", "primary": true },
	*            { "name": "Population", "type": "int" },
	*            { "name": "Continent", "type": "string" }
	*        ],
	*        "keys": [
	*            { "field": "Name", "type": "text" },
	*            { "field": "Continent", "type": "value" }
	*        ]
	*    }]
	* });
	* // get the details of the key of the field "Continent"
	* // returns a JSON object containing the details of the key:
	* // { fq: { length: 0 }, vocabulary: { length: 0 }, name: 'Continent', store: { name: 'Countries', ... }}
	* var details = base.store("Countries").key("Continent");
	*/
 exports.Store.prototype.key = function (keyName) {};
/**
	* //TODO
	* @param {string} saName - The name of the stream aggregate.
	* @ignore
	*/
 exports.Store.prototype.getStreamAggr = function (saName) {}
/**
	* //TODO
	* @ignore
	*/
 exports.Store.prototype.getStreamAggrNames = function () {}
/**
	* Returns the store as a JSON.
	* @returns {Object} The store as a JSON.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "FootballPlayers",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "FootballClubs", "type": "string_v" },
	*            { "name": "GoalsPerSeason", "type": "int_v" },
	*        ]
	*    }]
	* });
	* // get the store as a JSON object
	* // the returned JSON object is:
	* // { storeId: 0, storeName: 'FootballPlayers', storeRecords: 0, fields: [...], keys: [], joins: [] }
	* var json = base.store("FootballPlayers").toJSON();
	*/
 exports.Store.prototype.toJSON = function () {};
/**
	* Deletes the records in the store.
	* @param {number} [num] - The number of deleted records. If the number is given, the first num records will be deleted.
	* @returns {number} The number of remaining records in the store.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "TVSeries",
	*        "fields": [
	*            { "name": "Title", "type": "string", "primary": true },
	*            { "name": "NumberOfEpisodes", "type": "int" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("TVSeries").push({ "Title": "Archer", "NumberOfEpisodes": 75 });
	* base.store("TVSeries").push({ "Title": "The Simpsons", "NumberOfEpisodes": 574 });
	* base.store("TVSeries").push({ "Title": "New Girl", "NumberOfEpisodes": 94 });
	* base.store("TVSeries").push({ "Title": "Rick and Morty", "NumberOfEpisodes": 11 });
	* base.store("TVSeries").push({ "Title": "Game of Thrones", "NumberOfEpisodes": 47 });
	* // deletes the first 2 records (Archer and The Simpsons) in TVSeries
	* base.store("TVSeries").clear(2); // returns 3
	* // delete all remaining records in TVStore
	* base.store("TVSeries").clear();  // returns 0
	*/
 exports.Store.prototype.clear = function (num) {};
/**
	* Gives a vector containing the field value of each record.
	* @param {string} fieldName - The field name. Field must be of one-dimensional type, e.g. int, float, string...
	* @returns {module:la.Vector} The vector containing the field values of each record.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "Companies",
	*        "fields": [
	*            { "name": "Name", "type": "string", "primary": true },
	*            { "name": "Location", "type": "string" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Companies").push({ "Name": "DC Comics", "Location": "Burbank, California" });
	* base.store("Companies").push({ "Name": "DC Shoes", "Location": "Huntington Beach, California" });
	* base.store("Companies").push({ "Name": "21st Century Fox", "Location": "New York City, New York" });
	* // get the vector of company names
	* var companyNames = base.store("Companies").getVector("Name");	// returns a vector ["DC Comics", "DC Shoes", "21st Century Fox"]
	*/
 exports.Store.prototype.getVector = function (fieldName) {};
/**
	* Gives a matrix containing the field values of each record.
	* @param {string} fieldName - The field name. Field mustn't be of type string.
	* @returns {(module:la.Matrix | module:la.SparseMatrix)} The matrix containing the field values. 
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "ArcheryChampionship",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "ScorePerRound", "type": "float_v" }
	*        ]
	*    }]
	* });
	* // set new records in the store
	* base.store("ArcheryChampionship").push({ "Name": "Robin Hood", "ScorePerRound": [50, 48, 48] });
	* base.store("ArcheryChampionship").push({ "Name": "Oliver Queen", "ScorePerRound": [44, 46, 44] });
	* base.store("ArcheryChampionship").push({ "Name": "Legolas", "ScorePerRound": [50, 50, 48] });
	* // get the matrix containing the "score per round" values
	* // The values of the i-th column are the values of the i-th record.
	* // The function will give the matrix:
	* // 50  44  50
	* // 48  46  50
	* // 48  44  48
	* var matrix = base.store("ArcheryChampionship").getMatrix("ScorePerRound");
	*/
 exports.Store.prototype.getMatrix = function (fieldName) {};
/**
	* Gives the field value of a specific record.
	* @param {number} recId - The record id.
	* @param {string} fieldName - The field's name.
	* @returns {Object} The fieldName value of the record with recId.
	* @example
	* //import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "Festivals",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "Type", "type": "string" },
	*            { "name": "Location", "type": "string" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("Festivals").push({ "Name": "Metaldays", "Type": "music", "Location": "Tolmin, Slovenia" });
	* base.store("Festivals").push({ "Name": "Festival de Cannes", "Type": "movie", "Location": "Cannes, France" });
	* base.store("Festivals").push({ "Name": "The Festival of Chocolate", "Type": "food", "Location": "Hillsborough, USA" });
	* // get the field value of the second record for field "Type"
	* var fieldValue = base.store("Festivals").cell(1, "Type"); // returns "movie"
	*/
 exports.Store.prototype.cell = function (recId, fieldName) {};
/**
	* Gives the name of the store.
	*/
 exports.Store.prototype.name = undefined;
/**
	* Checks if the store is empty.
	*/
 exports.Store.prototype.empty = undefined;
/**
	* Gives the number of records.
	*/
 exports.Store.prototype.length = 0;
/**
	* Creates a record set containing all the records from the store.
	*/
 exports.Store.prototype.recs = undefined;
/**
	* Gives an array of all field descriptor JSON objects.
	*/
 exports.Store.prototype.fields = undefinied;
/**
	* Gives an array of all join descriptor JSON objects.
	*/
 exports.Store.prototype.joins = undefined;
/**
	* Gives an array of all key descriptor JSON objects.
	*/
 exports.Store.prototype.keys = undefined;
/**
	* Returns the first record of the store.
	*/
 exports.Store.prototype.first = undefined;
/**
	* Returns the last record of the store.
	*/
 exports.Store.prototype.last = undefined;
/**
	* Returns an iterator for iterating over the store from start to end.
	*/
 exports.Store.prototype.forwardIter = undefined;
/**
	* Returns an iterator for iterating over the store form end to start.
	*/
 exports.Store.prototype.backwardIter = undefined;
/**
	* Gets the record with the given ID. //TODO
	* @param {number} recId - The id of the record.
	* @returns {module:qm.Record} The record with the ID equal to recId.
	*/
 exports.Store.prototype.store = function (recId) {};
/**
	* Returns the base, in which the store is contained.
	*/
 exports.Store.prototype.base = undefined;
/**
* Record (factory pattern).
* @namespace
*/
 exports.Record = function () {}; 
/**
	* Clones the record.
	* @returns {module:qm.Record} The clone of the record.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "StarWarsMovies",
	*        "fields": [
	*            { "name": "Title", "type": "string" },
	*            { "name": "ReleseDate", "type": "datetime" },
	*            { "name": "Length", "type": "int" }
	*        ]
	*    }]
	* });
	* // create some records in the new store
	* base.store("StarWarsMovies").push({"Title": "Attack of the Clones", "ReleseDate": "2002-05-16T00:00:00", "Length": 142 });
	* base.store("StarWarsMovies").push({"Title": "The Empire Strikes Back", "ReleseDate": "1980-06-20T00:00:00", "Length": 124 });
	* base.store("StarWarsMovies").push({"Title": "Return of the Jedi", "ReleseDate": "1983-05-25T00:00:00", "Length": 134 });
	* // create a clone of the "Attack of the Clones" record
	* var clone = base.store("StarWarsMovies")[0].$clone();
	*/
 exports.Record.prototype.$clone = function () {};
/**
	* addJoin // TODO
	* @param {string} joinName
	* @param {(module:qm.Record | number)} joinRecord
	* @param {number} [joinFrequency]
	* @returns {module:qm.Record} Record.
	*/
 exports.Record.prototype.addJoin = function (joinName, joinRecord, joinFrequency) {}
/**
	* delJoin // TODO
	* @param {string} joinName
	* @param {(module:qm.Record | number)} joinRecord
	* @param {number} [joinFrequency]
	* @returns {module:qm.Record} Record.
	*/
 exports.Record.prototype.delJoin = function (joinName, joinRecord, joinFrequency) {}
/**
	* Creates a JSON version of the record.
	* @returns {Object} The JSON version of the record.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "Musicians",
	*        "fields": [
	*            { "name": "Name", "type": "string", "primary": true },
	*            { "name": "DateOfBirth", "type": "datetime" },
	*            { "name": "GreatestHits", "type": "string_v" }
	*        ]
	*    }]
	* });
	* // create some records
	* base.store("Musicians").push({ "Name": "Jimmy Page", "DateOfBirth":  "1944-01-09T00:00:00", "GreatestHits": ["Stairway to Heaven", "Whole Lotta Love"] });
	* base.store("Musicians").push({ "Name": "Beyonce", "DateOfBirth": "1981-09-04T00:00:00", "GreatestHits": ["Single Ladies (Put a Ring on It)"] });
	* // get a JSON version of the "Beyonce" record 
	* // The JSON object for this example si:
	* // { '$id': 1, Name: 'Beyonce', ActiveSince: '1981-09-04T00:00:00', GreatestHits: ['Single Ladies (Put a Ring on It)'] }
	* var json = base.store("Musicians").rec("Beyonce").toJSON();
	*/
 exports.Record.prototype.toJSON = function () {};
/**
	* Returns the id of the record.
	*/
 exports.Record.prototype.$id = undefined;
/**
	* Returns the name of the record.
	*/
 exports.Record.prototype.$name = undefined;
/**
	* Returns the frequency of the record.
	* @ignore
	*/
 exports.Record.prototype.$fq = undefined;
/**
	* Returns the store the record belongs to.
	*/
 exports.Record.prototype.store = undefined;
/**
* Record Set (factory pattern)
* @namespace
* @example
* // import qm module
* var qm = require('qminer');
* // factory based construction using store.recs
* var rs = store.recs;
*/
 exports.RecordSet = function () {}
/**
	* Creates a new instance of the record set.
	* @returns {module:qm.RecordSet} A copy of the record set.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "Philosophers",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "Era", "type": "string" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("Philosophers").push({ "Name": "Plato", "Era": "Ancient philosophy" });
	* base.store("Philosophers").push({ "Name": "Immanuel Kant", "Era": "18th-century philosophy" });
	* base.store("Philosophers").push({ "Name": "Emmanuel Levinas", "Era": "20th-century philosophy" });
	* base.store("Philosophers").push({ "Name": "Rene Descartes", "Era": "17th-century philosophy" });
	* base.store("Philosophers").push({ "Name": "Confucius", "Era": "Ancient philosophy" });
	* // create a record set out of the records in store
	* var recordSet = base.store("Philosophers").recs;
	* // clone the record set of the "Philosophers" store
	* var philosophers = recordSet.clone();
	*/
 exports.RecordSet.prototype.clone = function () {};
/**
	* Creates a new record set out of the join attribute of records.
	* @param {string} joinName - The name of the join attribute.
	* @param {number} [sampleSize] - The number of records to be used for construction of the record set.
	* @returns {module:qm.RecordSet} The record set containing the join records.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing two stores, with join attributes
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [
	*    {
	*        "name": "Musicians",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "Instruments", "type": "string_v" }
	*        ],
	*        "joins": [
	*            { "name": "PlaysIn", "type": "index", "store": "Bands", "inverse": "Members" }
	*        ]
	*    },
	*    {
	*        "name": "Bands",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "Genre", "type": "string" }
	*        ],
	*        "joins": [
	*            { "name": "Members", "type": "index", "store": "Musicians", "inverse": "PlaysIn" }
	*        ]
	*    }]
	* });
	* // add some new records to both stores
	* base.store("Musicians").push({ "Name": "Robert Plant", "Instruments": ["Vocals"], "PlaysIn": [{"Name": "Led Zeppelin", "Genre": "Rock" }] });
	* base.store("Musicians").push({ "Name": "Jimmy Page", "Instruments": ["Guitar"], "PlaysIn": [{"Name": "Led Zeppelin", "Genre": "Rock" }] });
	* base.store("Bands").push({ "Name": "The White Stripes", "Genre": "Rock" });
	* // create a record set containing the musicians, that are members of some bend
	* // returns a record set containing the records of "Robert Plant" and "Jimmy Page"
	* var ledZeppelin = base.store("Bands").recs.join("Members");
	* // create a record set containing the first musician, that is a member of some band
	* // returns a record set containing only one record, which is "Robert Plant" or "Jimmy Page"
	* var robertPlant = base.store("Bands").recs.join("Members", 1);
	*/
 exports.RecordSet.prototype.join = function (joinName, sampleSize) {};
/**
	* Aggr // TODO
	* @param {Object} [aggrQueryJSON] 
	* @returns {Object} Aggregate
	* @ignore
	*/
 exports.RecordSet.prototype.aggr = function (aggrQueryJSON) {};
/**
	* Truncates the first records.
	* @param {number} limit_num - How many records to truncate.
	* @param {number} [offset_num] - Where to start to truncate.
	* @returns {module:qm.RecordSet} Self.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "Philosophers",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "Era", "type": "string" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("Philosophers").push({ "Name": "Plato", "Era": "Ancient philosophy" });
	* base.store("Philosophers").push({ "Name": "Immanuel Kant", "Era": "18th-century philosophy" });
	* base.store("Philosophers").push({ "Name": "Emmanuel Levinas", "Era": "20th-century philosophy" });
	* base.store("Philosophers").push({ "Name": "Rene Descartes", "Era": "17th-century philosophy" });
	* base.store("Philosophers").push({ "Name": "Confucius", "Era": "Ancient philosophy" });
	* // create two identical record sets of the "Philosophers" store
	* var recordSet1 = base.store("Philosophers").recs;
	* var recordSet2 = base.store("Philosophers").recs;
	* // truncate the first 3 records in recordSet1
	* recordSet1.trunc(3); // return self, containing only the first 3 records ("Plato", "Immanuel Kant", "Emmanuel Levinas")
	* // truncate the first 2 records in recordSet2, starting with "Emmanuel Levinas"
	* recordSet2.trunc(2, 2); // returns self, containing only the 2 records ("Emmanuel Levinas", "Rene Descartes")
	*/
 exports.RecordSet.prototype.trunc = function (limit_num, offset_num) {};
/**
	* Creates a random sample of records of the record set.
	* @param {number} num - The number of records in the sample.
	* @returns {module:qm.RecordSet} A record set containing the sample records.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base with one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "Movies",
	*        "fields": [
	*            { "name": "Title", "type": "string" },
	*            { "name": "Length", "type": "int" },
	*            { "name": "Director", "type": "string" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("Movies").push({ "Title": "The Nightmare Before Christmas", "Length": 76, "Director": "Henry Selick" });
	* base.store("Movies").push({ "Title": "Jurassic Part", "Length": 127, "Director": "Steven Spielberg" });
	* base.store("Movies").push({ "Title": "The Avengers", "Length": 143, "Director": "Joss Whedon" });
	* base.store("Movies").push({ "Title": "The Clockwork Orange", "Length": 136, "Director": "Stanley Kubrick" });
	* base.store("Movies").push({ "Title": "Full Metal Jacket", "Length": 116, "Director": "Stanely Kubrick" });
	* // create a sample record set of containing 3 records from the "Movies" store
	* var sample = base.store("Movies").recs.sample(3);
	*/
 exports.RecordSet.prototype.sample = function (num) {};
/**
	* Shuffles the order of records in the record set.
	* @param {number} [seed] - Integer.
	* @returns {module:qm.RecordSet} Self. The records in the record set are in a different order.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "WeatherForcast",
	*        "fields": [
	*            { "name": "Weather", "type": "string" },
	*            { "name": "Date", "type": "datetime" },
	*            { "name": "TemperatureDegrees", "type": "int" }
	*        ]
	*    }]
	* });
	* // put some records in the "WeatherForecast" store
	* base.store("WeatherForcast").push({ "Weather": "Partly Cloudy", "Date": "2015-05-27T11:00:00", "TemperatureDegrees": 19 });
	* base.store("WeatherForcast").push({ "Weather": "Partly Cloudy", "Date": "2015-05-28T11:00:00", "TemperatureDegrees": 22 });
	* base.store("WeatherForcast").push({ "Weather": "Mostly Cloudy", "Date": "2015-05-29T11:00:00", "TemperatureDegrees": 25 });
	* base.store("WeatherForcast").push({ "Weather": "Mostly Cloudy", "Date": "2015-05-30T11:00:00", "TemperatureDegrees": 25 });
	* base.store("WeatherForcast").push({ "Weather": "Scattered Showers", "Date": "2015-05-31T11:00:00", "TemperatureDegrees": 24 });
	* base.store("WeatherForcast").push({ "Weather": "Mostly Cloudy", "Date": "2015-06-01T11:00:00", "TemperatureDegrees": 27 });
	* // get the record set containing the records from the "WeatherForcast" store
	* var recordSet = base.store("WeatherForcast").recs;
	* // shuffle the records in the newly created record set. Use the number 100 as the seed for the shuffle
	* recordSet.shuffle(100); // returns self, the records in the record set are shuffled
	*/
 exports.RecordSet.prototype.shuffle = function (seed) {};
/**
	* It reverses the record order.
	* @returns {module:qm.RecordSet} Self. Records are in reversed order.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "WeatherForcast",
	*        "fields": [
	*            { "name": "Weather", "type": "string" },
	*            { "name": "Date", "type": "datetime" },
	*            { "name": "TemperatureDegrees", "type": "int" },
	*        ]
	*    }]
	* });
	* // put some records in the "WeatherForecast" store
	* base.store("WeatherForcast").push({ "Weather": "Partly Cloudy", "Date": "2015-05-27T11:00:00", "TemperatureDegrees": 19 });
	* base.store("WeatherForcast").push({ "Weather": "Partly Cloudy", "Date": "2015-05-28T11:00:00", "TemperatureDegrees": 22 });
	* base.store("WeatherForcast").push({ "Weather": "Mostly Cloudy", "Date": "2015-05-29T11:00:00", "TemperatureDegrees": 25 });
	* base.store("WeatherForcast").push({ "Weather": "Mostly Cloudy", "Date": "2015-05-30T11:00:00", "TemperatureDegrees": 25 });
	* base.store("WeatherForcast").push({ "Weather": "Scattered Showers", "Date": "2015-05-31T11:00:00", "TemperatureDegrees": 24 });
	* base.store("WeatherForcast").push({ "Weather": "Mostly Cloudy", "Date": "2015-06-01T11:00:00", "TemperatureDegrees": 27 });
	* // get the record set containing the records from the "WeatherForcast" store
	* var recordSet = base.store("WeatherForcast").recs;
	* // reverse the record order in the record set
	* recordSet.reverse(); // returns self, the records in the record set are in the reverse order
	*/
 exports.RecordSet.prototype.reverse = function () {};
/**
	* Sorts the records according to record id.
	* @param {number} [asc=1] - If asc > 0, it sorts in ascending order. Otherwise, it sorts in descending order.  
	* @returns {module:qm.RecordSet} Self. Records are sorted according to record id and asc.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "Tea",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "Type", "type": "string" },
	*            { "name": "Origin", "type": "string" }
	*        ]
	*    }]
	* });
	* // put some records in the "Tea" store
	* base.store("Tea").push({ "Name": "Tanyang Gongfu", "Type": "Black", "Origin": "Tanyang" });
	* base.store("Tea").push({ "Name": "Rou Gui", "Type": "White" });
	* base.store("Tea").push({ "Name": "Tieluohan Tea", "Type": "Wuyi", "Origin": "Northern Fujian" });
	* base.store("Tea").push({ "Name": "Red Robe", "Type": "Oolong", "Origin": "Wuyi Mountains" });
	* // get the records of the "Tea" store as a record set
	* var recordSet = base.store("Tea").recs;
	* // sort the records in the record set by their id in descending order
	* recordSet.sortById(-1); // returns self, the records are sorted in descending order
	* // sort the records in the record set by their id in ascending order
	* recordSet.sortById(); // returns self, the records are sorted in ascending order (default)
	*/
 exports.RecordSet.prototype.sortById = function (asc) {}; 
/**
	* Sorts the records according to their weight.
	* @param {number} [asc=1] - If asc > 0, it sorts in ascending order. Otherwise, it sorts in descending order.
	* @returns {module:qm.RecordSet} Self. Records are sorted according to record weight and asc.
	* @ignore
	*/
 exports.RecordSet.prototype.sortByFq = function (asc) {}; 
/**
	* Sorts the records according to a specific record field.
	* @param {string} fieldName - The field by which the sort will work.
	* @param {number} [arc=-1] - if asc > 0, it sorts in ascending order. Otherwise, it sorts in descending order.
	* @returns {module:qm.RecordSet} Self. Records are sorted according to fieldName and arc.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "TVSeries",
	*        "fields": [
	*            { "name": "Title", "type": "string", "primary": true },
	*            { "name": "NumberOfEpisodes", "type": "int" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("TVSeries").push({ "Title": "Archer", "NumberOfEpisodes": 75 });
	* base.store("TVSeries").push({ "Title": "The Simpsons", "NumberOfEpisodes": 574 });
	* base.store("TVSeries").push({ "Title": "New Girl", "NumberOfEpisodes": 94 });
	* base.store("TVSeries").push({ "Title": "Rick and Morty", "NumberOfEpisodes": 11 });
	* base.store("TVSeries").push({ "Title": "Game of Thrones", "NumberOfEpisodes": 47 });
	* // get the records of the "TVSeries" store as a record set
	* var recordSet = base.store("TVSeries").recs;
	* // sort the records by their "Title" field in descending order 
	* recordSet.sortByField("Title"); // returns self, record are sorted by their "Title"
	*/
 exports.RecordSet.prototype.sortByField = function (fieldName, asc) {};
/**
	* Sorts the records according to the given callback function.
	* @param {function} callback - The function used to sort the records. It takes two parameters:
	* <br>1. rec - The first record.
	* <br>2. rec2 - The second record.
	* <br>It returns a boolean object.
	* @returns {module:qm.RecordSet} Self. The records are sorted according to the callback function.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "TVSeries",
	*        "fields": [
	*            { "name": "Title", "type": "string", "primary": true },
	*            { "name": "NumberOfEpisodes", "type": "int" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("TVSeries").push({ "Title": "Archer", "NumberOfEpisodes": 75 });
	* base.store("TVSeries").push({ "Title": "The Simpsons", "NumberOfEpisodes": 574 });
	* base.store("TVSeries").push({ "Title": "New Girl", "NumberOfEpisodes": 94 });
	* base.store("TVSeries").push({ "Title": "Rick and Morty", "NumberOfEpisodes": 11 });
	* base.store("TVSeries").push({ "Title": "Game of Thrones", "NumberOfEpisodes": 47 });
	* // get the records of the "TVSeries" store as a record set
	* var recordSet = base.store("TVSeries").recs;
	* // sort the records by their number of episodes
	* recordSet.sort(function (rec, rec2) { return rec.NumberOfEpisodes < rec2.NumberOfEpisodes; }); // returns self, records are sorted by the number of episodes
	*/
 exports.RecordSet.prototype.sort = function (callback) {};
/**
	* Keeps only records with ids between or equal two values.
	* @param {number} [minId] - The minimum id.
	* @param {number} [maxId] - The maximum id.
	* @returns {module:qm.RecordSet} Self. 
	* <br>1. Contains only the records of the original with ids between minId and maxId, if parameters are given.
	* <br>2. Contains all the records of the original, if no parameter is given.
	* @example
	* // import qm require
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "FrankSinatraGreatestHits",
	*        "fields": [
	*            { "name": "Title", "type": "string" },
	*            { "name": "Length", "type": "int" }
	*        ]
	*    }]
	* });
	* // put some records in the "FrankSinatraGreatesHits" store
	* base.store("FrankSinatraGreatestHits").push({ "Title": "Strangers in the Night", "Length": 145 });
	* base.store("FrankSinatraGreatestHits").push({ "Title": "Summer Wind", "Length": 173 });
	* base.store("FrankSinatraGreatestHits").push({ "Title": "It Was a Very Good Year", "Length": 265 });
	* base.store("FrankSinatraGreatestHits").push({ "Title": "Somewhere in Your Heart", "Length": 146 });
	* base.store("FrankSinatraGreatestHits").push({ "Title": "Forget Domani", "Length": 156 });
	* base.store("FrankSinatraGreatestHits").push({ "Title": "Somethin' Stupid", "Length": 155 });
	* base.store("FrankSinatraGreatestHits").push({ "Title": "This Town", "Length": 186 });
	* // get the records of the store as a record set
	* var recordSet = base.store("FrankSinatraGreatestHits").recs;
	* // from the record set keep the records with indeces between or equal 2 and 5
	* recordSet.filterById(2, 5);
	*/
 exports.RecordSet.prototype.filterById = function (minId, maxId) {};
/**
	* Keeps only the records with weight between two values.
	* @param {number} [minFq] - The minimum value.
	* @param {number} [maxFq] - The maximum value.
	* @returns {module:qm.RecordSet} Self.
	* <br>1. Contains only the records of the original with weights between minFq and maxFq, if parameters are given.
	* <br>2. Contains all the records of the original, if no parameter is given.
	* @ignore
	*/
 exports.RecordSet.prototype.filterByFq = function (minFq, maxFq) {};
/**
	* Keeps only the records with a specific value of some field.
	* @param {string} fieldName - The field by which the records will be filtered.
	* @param {(string | number)} minVal -
	* <br>1. Is a string, if the field type is a string. The exact string to compare.
	* <br>2. Is a number, if the field type is a number. The minimal value for comparison.
	* <br>3. TODO Time field
	* @param {number} maxVal - Only in combination with minVal for non-string fields. The maximal value for comparison.
	* @returns {module:qm.RecordSet} Self. Containing only the records with the fieldName value between minVal and maxVal. If the fieldName type is string,
	* it contains only the records with fieldName equal to minVal.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "WeatherForcast",
	*        "fields": [
	*            { "name": "Weather", "type": "string" },
	*            { "name": "Date", "type": "datetime" },
	*            { "name": "TemperatureDegrees", "type": "int" },
	*        ]
	*    }]
	* });
	* // put some records in the "WeatherForecast" store
	* base.store("WeatherForcast").push({ "Weather": "Partly Cloudy", "Date": "2015-05-27T11:00:00", "TemperatureDegrees": 19 });
	* base.store("WeatherForcast").push({ "Weather": "Partly Cloudy", "Date": "2015-05-28T11:00:00", "TemperatureDegrees": 22 });
	* base.store("WeatherForcast").push({ "Weather": "Mostly Cloudy", "Date": "2015-05-29T11:00:00", "TemperatureDegrees": 25 });
	* base.store("WeatherForcast").push({ "Weather": "Mostly Cloudy", "Date": "2015-05-30T11:00:00", "TemperatureDegrees": 25 });
	* base.store("WeatherForcast").push({ "Weather": "Scattered Showers", "Date": "2015-05-31T11:00:00", "TemperatureDegrees": 24 });
	* base.store("WeatherForcast").push({ "Weather": "Mostly Cloudy", "Date": "2015-06-01T11:00:00", "TemperatureDegrees": 27 });
	* // get the record set containing the records from the "WeatherForcast" store
	* var recordSet = base.store("WeatherForcast").recs;
	* // filter only the records, where the weather is Mostly Cloudy
	* recordSet.filterByField("Weather", "Mostly Cloudy"); // returns self, containing only the records, where the weather is "Mostly Cloudy"
	*/
 exports.RecordSet.prototype.filterByField = function (fieldName, minVal, maxVal) {};
/**
	* Keeps only the records that pass the callback function.
	* @param {function} callback - The filter function. It takes one parameter and return a boolean object.
	* @returns {module:qm.RecordSet} Self. Containing only the record that pass the callback function.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "ArcheryChampionship",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "ScorePerRound", "type": "float_v" }
	*        ]
	*    }]
	* });
	* // set new records in the store
	* base.store("ArcheryChampionship").push({ "Name": "Robin Hood", "ScorePerRound": [50, 48, 48] });
	* base.store("ArcheryChampionship").push({ "Name": "Oliver Queen", "ScorePerRound": [44, 46, 44] });
	* base.store("ArcheryChampionship").push({ "Name": "Legolas", "ScorePerRound": [50, 50, 48] });
	* // create a record set out of the records of the store
	* var recordSet = base.store("ArcheryChampionship").recs;
	* // filter the records: which archers have scored 48 points in the third round
	* recordSet.filter(function (rec) { return rec.ScorePerRound[2] == 48; }); // keeps only the records, where the score of the third round is equal 48
	*/
 exports.RecordSet.prototype.filter = function (callback) {}; 
/**
	* Splits the record set into smaller record sets.
	* @param {function} callback - The splitter function. It takes two parameters (records) and returns a boolean object.
	* @returns {Array.<module:qm.RecordSet>} An array containing the smaller record sets. The records are split according the callback function.
	* @example
	* // import qm module
	* var qm = require("qminer");
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "SocialGames",
	*        "fields": [
	*            { "name": "Title", "type": "string" },
	*            { "name": "Type", "type": "string" },
	*            { "name": "MinPlayers", "type": "int" },
	*            { "name": "MaxPlayers", "type": "int" }
	*        ]
	*    }]
	* });
	* // set new records in the store
	* base.store("SocialGames").push({ "Title": "DungeonsAndDragons", "Type": "Role-Playing", "MinPlayers": 5, "MaxPlayers": 5 });
	* base.store("SocialGames").push({ "Title": "Dobble", "Type": "Card", "MinPlayers": 2, "MaxPlayers": 8 });
	* base.store("SocialGames").push({ "Title": "Settlers of Catan", "Type": "Board", "MinPlayers": 3, "MaxPlayers": 4 });
	* base.store("SocialGames").push({ "Title": "Munchkin", "Type": "Card", "MinPlayers": 3, "MaxPlayers": 6 });
	* // create a record set out of the records of the store
	* var recordSet = base.store("SocialGames").recs;
	* // sort the records by MinPlayers in ascending order
	* recordSet.sortByField("MinPlayers", true);
	* // split the record set by the minimum number of players
	* // returns an array containing three record sets: the first containing the "DungeonsAndDragons" record,
	* // the second containing the "Settlers of Catan" and "Munchkin" records and the third containing the 
	* // "Dobble" record
	* // recordSet.split(function (rec, rec2) { return rec.MinPlayers < rec2.MinPlayers; });
	*/
 exports.RecordSet.prototype.split = function (callback) {};
/**
	* Deletes the records, that are also in the other record set.
	* @param {module:qm.RecordSet} rs - The other record set.
	* @returns {module:qm.RecordSet} Self. Contains only the records, that are not in rs.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "BookWriters",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "Genre", "type": "string" },
	*            { "name": "Books", "type": "string_v" }
	*        ]
	*    }]
	* });
	* // set new records in the store
	* base.store("BookWriters").push({ "Name": "Terry Pratchett", "Genre": "Fantasy", "Books": ["The Colour of Magic", "Going Postal", "Mort", "Guards! Guards!"] });
	* base.store("BookWriters").push({ "Name": "Douglas Adams", "Genre": "Sci-fi", "Books": ["The Hitchhiker's Guide to the Galaxy", "So Long, and Thanks for All the Fish"] });
	* base.store("BookWriters").push({ "Name": "Fyodor Dostoyevsky", "Genre": "Drama", "Books": ["Crime and Punishment", "Demons"] });
	* base.store("BookWriters").push({ "Name": "J.R.R. Tolkien", "Genre": "Fantasy", "Books": ["The Hobbit", "The Two Towers", "The Silmarillion" ] });
	* base.store("BookWriters").push({ "Name": "George R.R. Martin", "Genre": "Fantasy", "Books": ["A Game of Thrones", "A Feast of Crows"] });
	* base.store("BookWriters").push({ "Name": "J. K. Rowling", "Genre": "Fantasy", "Books": ["Harry Potter and the Philosopher's Stone"] });
	* base.store("BookWriters").push({ "Name": "Ivan Cankar", "Genre": "Drama", "Books": ["On the Hill", "The King of Betajnova", "The Serfs"] });
	* // create one record set containing all records of store
	* var recordSet = base.store("BookWriters").recs;
	* // create one record set containing the records with genre "Fantasy"
	* var fantasy = base.store("BookWriters").recs.filterByField("Genre", "Fantasy");
	* // delete the records in recordSet, that are also in fantasy
	* recordSet.deleteRecords(fantasy); // returns self, containing only three records: "Douglas Adams", "Fyodor Dostoyevsky" and "Ivan Cankar"
	*/
 exports.RecordSet.prototype.deleteRecords = function (rs) {}; 
/**
	* Returns the record set as a JSON.
	* @returns {Object} The record set as a JSON.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "Musicians",
	*        "fields": [
	*            { "name": "Name", "type": "string", "primary": true },
	*            { "name": "DateOfBirth", "type": "datetime" },
	*            { "name": "GreatestHits", "type": "string_v" }
	*        ]
	*    }]
	* });
	* // create some records
	* base.store("Musicians").push({ "Name": "Jimmy Page", "DateOfBirth":  "1944-01-09T00:00:00", "GreatestHits": ["Stairway to Heaven", "Whole Lotta Love"] });
	* base.store("Musicians").push({ "Name": "Beyonce", "DateOfBirth": "1981-09-04T00:00:00", "GreatestHits": ["Single Ladies (Put a Ring on It)"] });
	* // create a record set out of the records in the "Musicians" store
	* var recordSet = base.store("Musicians").recs;
	* // create a JSON object out of the record set
	* var json = recordSet.toJSON();
	*/
 exports.RecordSet.prototype.toJSON = function () {};
/**
	* Executes a function on each record in record set.
	* @param {function} callback - Function to be executed. It takes two parameters:
	* <br>rec - The current record.
	* <br>[idx] - The index of the current record.
	* @returns {module:qm.RecordSet} Self.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "People",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "Gender", "type": "string" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("People").push({ "Name": "Eric Sugar", "Gender": "Male" });
	* base.store("People").push({ "Name": "Jane Tokyo", "Gender": "Female" });
	* base.store("People").push({ "Name": "Mister Tea", "Gender": "Male" });
	* // create a record set out of the records of the store
	* var recordSet = base.store("People").recs;
	* // change the Name of all records into "Anonymous"
	* recordSet.each(function (rec) { rec.Name = "Anonymous"; }); // returns self, all record's Name are "Anonymous"
	*/
 exports.RecordSet.prototype.each = function (callback) {}
/**
	* Creates an array of function outputs created from the records in record set.
	* @param {function} callback - Function that generates the array. It takes two parameters:
	* <br>rec - The current record.
	* <br>[idx] - The index of the current record.
	* @returns {Array<Object>} The array created by the callback function.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "People",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "Gender", "type": "string" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("People").push({ "Name": "Eric Sugar", "Gender": "Male" });
	* base.store("People").push({ "Name": "Jane Tokyo", "Gender": "Female" });
	* base.store("People").push({ "Name": "Mister Tea", "Gender": "Male" });
	* // create a record set out of the records of the store
	* var recordSet = base.store("People").recs;
	* // make an array of record Names
	* var arr = recordSet.map(function (rec) { return rec.Name; }); // returns an array: ["Eric Sugar", "Jane Tokyo", "Mister Tea"]
	*/
 exports.RecordSet.prototype.map = function (callback) {}
/**
	* Creates the set intersection of two record sets.
	* @param {module:qm.RecordSet} rs - The other record set.
	* @returns {module:qm.RecordSet} The intersection of the two record sets.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base with one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "Movies",
	*        "fields": [
	*            { "name": "Title", "type": "string" },
	*            { "name": "Length", "type": "int" },
	*            { "name": "Director", "type": "string" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("Movies").push({ "Title": "The Nightmare Before Christmas", "Length": 76, "Director": "Henry Selick" });
	* base.store("Movies").push({ "Title": "Jurassic Part", "Length": 127, "Director": "Steven Spielberg" });
	* base.store("Movies").push({ "Title": "The Avengers", "Length": 143, "Director": "Joss Whedon" });
	* base.store("Movies").push({ "Title": "The Clockwork Orange", "Length": 136, "Director": "Stanley Kubrick" });
	* base.store("Movies").push({ "Title": "Full Metal Jacket", "Length": 116, "Director": "Stanely Kubrick" });
	* // create a record set out of the records in store, where length of the movie is greater than 110
	* var greaterSet = base.store("Movies").recs.filterByField("Length", 110);
	* // create a record set out of the records in store, where the length of the movie is lesser than 130
	* var lesserSet = base.store("Movies").recs.filterByField("Length", 0, 130);
	* // get the intersection of greaterSet and lesserSet
	* var intersection = greaterSet.setIntersect(lesserSet); // returns a record set, containing the movies with lengths between 110 and 130
	*/
 exports.RecordSet.prototype.setIntersect = function (rs) {};
/**
	* Creates the set union of two record sets.
	* @param {module:qm.RecordSet} rs - The other record set.
	* @returns {module:qm.RecordSet} The union of the two record sets.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "TVSeries",
	*        "fields": [
	*            { "name": "Title", "type": "string", "primary": true },
	*            { "name": "NumberOfEpisodes", "type": "int" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("TVSeries").push({ "Title": "Archer", "NumberOfEpisodes": 75 });
	* base.store("TVSeries").push({ "Title": "The Simpsons", "NumberOfEpisodes": 574 });
	* base.store("TVSeries").push({ "Title": "New Girl", "NumberOfEpisodes": 94 });
	* base.store("TVSeries").push({ "Title": "Rick and Morty", "NumberOfEpisodes": 11 });
	* base.store("TVSeries").push({ "Title": "Game of Thrones", "NumberOfEpisodes": 47 });
	* // create a record set out of the records in store, where the number of episodes is lesser than 47
	* var lesserSet = base.store("TVSeries").recs.filterByField("NumberOfEpisodes", 0, 47);
	* // create a record set out of the records in store, where the number of episodes is greater than 100
	* var greaterSet = base.store("TVSeries").recs.filterByField("NumberOfEpisodes", 100);
	* // get the union of lesserSet and greaterSet
	* var union = lesserSet.setUnion(greaterSet); // returns a record set, which is the union of the two record sets
	*/
 exports.RecordSet.prototype.setUnion = function (rs) {};
/**
	* Creates the set difference between two record sets.
	* @param {module:qm.RecordSet} rs - The other record set.
	* @returns {module:qm.RecordSet} The difference between the two record sets.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "BookWriters",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "Genre", "type": "string" },
	*            { "name": "Books", "type": "string_v" }
	*        ]
	*    }]
	* });
	* // set new records in the store
	* base.store("BookWriters").push({ "Name": "Terry Pratchett", "Genre": "Fantasy", "Books": ["The Colour of Magic", "Going Postal", "Mort", "Guards! Guards!"] });
	* base.store("BookWriters").push({ "Name": "Douglas Adams", "Genre": "Sci-fi", "Books": ["The Hitchhiker's Guide to the Galaxy", "So Long, and Thanks for All the Fish"] });
	* base.store("BookWriters").push({ "Name": "Fyodor Dostoyevsky", "Genre": "Drama", "Books": ["Crime and Punishment", "Demons"] });
	* base.store("BookWriters").push({ "Name": "J.R.R. Tolkien", "Genre": "Fantasy", "Books": ["The Hobbit", "The Two Towers", "The Silmarillion" ] });
	* base.store("BookWriters").push({ "Name": "George R.R. Martin", "Genre": "Fantasy", "Books": ["A Game of Thrones", "A Feast of Crows"] });
	* base.store("BookWriters").push({ "Name": "J. K. Rowling", "Genre": "Fantasy", "Books": ["Harry Potter and the Philosopher's Stone"] });
	* base.store("BookWriters").push({ "Name": "Ivan Cankar", "Genre": "Drama", "Books": ["On the Hill", "The King of Betajnova", "The Serfs"] });
	* // create one record set containing all records of store
	* var recordSet = base.store("BookWriters").recs;
	* // create one record set containing the records with genre "Fantasy"
	* var fantasy = base.store("BookWriters").recs.filterByField("Genre", "Fantasy");
	* // create a new record set containing the difference of recordSet and fantasy
	* var difference = recordSet.setDiff(fantasy); // returns a record set, containing the records of Douglas Adams, Fyodor Dostoyevsky and Ivan Cankar
	*/
 exports.RecordSet.prototype.setDiff = function (rs) {}; 
/**
	* Creates a vector containing the field values of records.
	* @param {string} fieldName - The field from which to take the values. It's type must be one-dimensional, e.g. float, int, string,...
	* @returns {module:la.Vector} The vector containing the field values of records. The type it contains is dependant of the field type.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "TVSeries",
	*        "fields": [
	*            { "name": "Title", "type": "string", "primary": true },
	*            { "name": "NumberOfEpisodes", "type": "int" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("TVSeries").push({ "Title": "Archer", "NumberOfEpisodes": 75 });
	* base.store("TVSeries").push({ "Title": "The Simpsons", "NumberOfEpisodes": 574 });
	* base.store("TVSeries").push({ "Title": "New Girl", "NumberOfEpisodes": 94 });
	* base.store("TVSeries").push({ "Title": "Rick and Morty", "NumberOfEpisodes": 11 });
	* base.store("TVSeries").push({ "Title": "Game of Thrones", "NumberOfEpisodes": 47 });
	* // create a record set of the records of store
	* var recordSet = base.store("TVSeries").recs;
	* // create a vector containing the number of episodes for each series
	* // the vector will look like [75, 574, 94, 11, 47]
	* var vector = recordSet.getVector("NumberOfEpisodes");
	*/
 exports.RecordSet.prototype.getVector = function (fieldName) {}; 
/**
	* Creates a vector containing the field values of records.
	* @param {string} fieldName - The field from which to take the values. It's type must be numeric, e.g. float, int, float_v, num_sp_v,...
	* @returns {(module:la.Matrix|module:la.SparseMatrix)} The matrix containing the field values of records.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "ArcheryChampionship",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "ScorePerRound", "type": "float_v" }
	*        ]
	*    }]
	* });
	* // set new records in the store
	* base.store("ArcheryChampionship").push({ "Name": "Robin Hood", "ScorePerRound": [50, 48, 48] });
	* base.store("ArcheryChampionship").push({ "Name": "Oliver Queen", "ScorePerRound": [44, 46, 44] });
	* base.store("ArcheryChampionship").push({ "Name": "Legolas", "ScorePerRound": [50, 50, 48] });
	* // create a record set of the records in store
	* var recordSet = base.store("ArcheryChampionship").recs;
	* // create a matrix from the "ScorePerRound" field
	* // the i-th column of the matrix is the data of the i-th record in record set
	* // the matrix will look like
	* // 50  44  50
	* // 48  46  50
	* // 48  44  48
	* var matrix = recordSet.getMatrix("ScorePerRound");
	*/
 exports.RecordSet.prototype.getMatrix = function (fieldName) {};
/**
	* Returns the store, where the records in the record set are stored.
	*/
 exports.RecordSet.prototype.store = undefined;
/**
	* Returns the number of records in record set.
	*/
 exports.RecordSet.prototype.length = undefined;
/**
	* Checks if the record set is empty. If the record set is empty, then it returns true. Otherwise, it returns false.
	*/
 exports.RecordSet.prototype.empty = undefined;
/**
	* Checks if the record set is weighted. If the record set is weighted, then it returns true. Otherwise, it returns false.
	*/
 exports.RecordSet.prototype.weighted = undefined;
/**
* Store Iterator (factory pattern)
* @namespace
* @example
* // import qm module
* qm = require('qminer');
* // factory based construction with store.forwardIter
* var iter = store.forwardIter;
*/
 exports.Iterator = function () {};
/**
	* Moves to the next record.
	* @returns {boolean} 
	* <br>1. True, if the iteration successfully moves to the next record.
	* <br>2. False, if there is no record left.
	*/
 exports.Iterator.prototype.next = function () {};
/**
	* Gives the store of the iterator.
	*/
 exports.Iterator.prototype.store = undefined;
/**
	* Gives the current record.
	*/
 exports.Iterator.prototype.record = undefined;
/**
* Feature Space
* @classdesc Represents the feature space.
* @class
* @param {module:qm.Base} base - The base where the features are extracted from.
* @param {Array.<Object>} extractors - The extractors.
* @example
* // import qm module
* var qm = require('qminer');
* // construct a base with the store
* var base = new qm.Base({
*   mode: "create",
*   schema: {
*     name: "FtrSpace",
*     fields: [
*       { name: "Value", type: "float" },
*       { name: "Category", type: "string" },
*       { name: "Categories", type: "string_v" },
*     ],
*     joins: [],
*     keys: []
*   }
* });
* // populate the store
* Store = base.store("FtrSpace");
* Store.push({ Value: 1.0, Category: "a", Categories: ["a", "q"] });
* Store.push({ Value: 1.1, Category: "b", Categories: ["b", "w"] });
* Store.push({ Value: 1.2, Category: "c", Categories: ["c", "e"] });
* Store.push({ Value: 1.3, Category: "a", Categories: ["a", "q"] });
* // create a feature space 
* var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpace", field: "Value" });
*/
 exports.FeatureSpace = function (base, extractors) {};
/**
	* Returns the dimension of the feature space.
	*/
 exports.FeatureSpace.prototype.dim = undefined;
/**
	* Returns an array of the dimensions of each feature extractor in the feature space.
	*/
 exports.FeatureSpace.prototype.dims = undefined;
/**
	* Serialize the feature space to an output stream.
	* @param {module:fs.FOut} fout - The output stream.
	* @returns {module:fs.FOut} The output stream.
	*/
 exports.FeatureSpace.prototype.save = function (fout) {};
/**
	* Adds a new feature extractor to the feature space.
	* @param {Object} obj - The added feature extracture.
	* @returns {module:qm.FeatureSpace} Self.
	*/
 exports.FeatureSpace.prototype.addFeatureExtractor = function (obj) {};
/**
	* Updates the feature space definitions and extractors by adding one record.
	* <br> For text feature extractors, it can update it's vocabulary by taking into account the new text.
	* <br> For numeric feature extractors, it can update the minimal and maximal values used to form the normalization.
	* <br> For jsfunc feature extractors, it can update a parameter used in it's function.
	* <br> For dateWindow feature extractor, it can update the start and the end of the window period to form the normalization.
	* @param {module:qm.Record} rec - The record, which updates the feature space.
	* @returns {module:qm.FeatureSpace} Self.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base
	* var base = new qm.Base({
    *   mode: "create",
    *   schema: {
    *     name: "FtrSpace",
    *     fields: [
    *       { name: "Value", type: "float" },
    *       { name: "Category", type: "string" },
    *       { name: "Categories", type: "string_v" },
    *     ],
    *     joins: [],
    *     keys: []
    *   }
    * });
    * // populate the store
    * Store = base.store("FtrSpace");
    * Store.push({ Value: 1.0, Category: "a", Categories: ["a", "q"] });
    * Store.push({ Value: 1.1, Category: "b", Categories: ["b", "w"] });
    * Store.push({ Value: 1.2, Category: "c", Categories: ["c", "e"] });
    * Store.push({ Value: 1.3, Category: "a", Categories: ["a", "q"] });
	* // create a new feature space
	* var ftr = new qm.FeatureSpace(base, [
	*   { type: "numeric", source: "FtrSpace", normalize: true, field: "Values" },
	*   { type: "categorical", source: "FtrSpace", field: "Category", values: ["a", "b", "c"] },
	*   { type: "multinomial", source: "FtrSpace", field: "Categories", normalize: true, values: ["a", "b", "c", "q", "w", "e"] }
	* ]);
	* // update the feature space with the first three record of the store
	* ftr.updateRecord(Store[0]);
	* ftr.updateRecord(Store[1]);
	* ftr.updateRecord(Store[2]);
	* // get the feature vectors of these records
	* ftr.extractVector(Store[0]); // returns the vector [0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2), 0, 0]
	* ftr.extractVector(Store[1]); // returns the vector [1/2, 0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2), 0]
	* ftr.extractVector(Store[2]); // returns the vector [1, 0, 0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2)]
	*/
 exports.FeatureSpace.prototype.updateRecord = function (rec) {};
/**
	* Updates the feature space definitions and extractors by adding all the records of a record set.
	* <br> For text feature extractors, it can update it's vocabulary by taking into account the new text.
	* <br> For numeric feature extractors, it can update the minimal and maximal values used to form the normalization.
	* <br> For jsfunc feature extractors, it can update a parameter used in it's function.
	* <br> For dateWindow feature extractor, it can update the start and the end of the window period to form the normalization.
	* @param {module:qm.RecordSet} rs - The record set, which updates the feature space.
	* @returns {module:qm.FeatureSpace} Self.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base
	* var base = new qm.Base({
    *   mode: "create",
    *   schema: {
    *     name: "FtrSpace",
    *     fields: [
    *       { name: "Value", type: "float" },
    *       { name: "Category", type: "string" },
    *       { name: "Categories", type: "string_v" },
    *     ],
    *     joins: [],
    *     keys: []
    *   }
    * });
    * // populate the store
	* Store = base.store("FtrSpace");
	* Store.push({ Value: 1.0, Category: "a", Categories: ["a", "q"] });
	* Store.push({ Value: 1.1, Category: "b", Categories: ["b", "w"] });
	* Store.push({ Value: 1.2, Category: "c", Categories: ["c", "e"] });
	* Store.push({ Value: 1.3, Category: "a", Categories: ["a", "q"] });
	* // create a new feature space
	* var ftr = new qm.FeatureSpace(base, [
	*	  { type: "numeric", source: "FtrSpace", normalize: true, field: "Values" },
	*     { type: "categorical", source: "FtrSpace", field: "Category", values: ["a", "b", "c"] },
	*     { type: "multinomial", source: "FtrSpace", field: "Categories", normalize: true, values: ["a", "b", "c", "q", "w", "e"] }
	* ]);
	* // update the feature space with the record set 
	* var rs = Store.recs;
	* ftr.updateRecords(rs);
	* // get the feature vectors of these records
	* ftr.extractVector(Store[0]); // returns the vector [0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2), 0, 0]
	* ftr.extractVector(Store[1]); // returns the vector [1/3, 0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2), 0]
	* ftr.extractVector(Store[2]); // returns the vector [2/3, 0, 0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2)]
	* ftr.extractVector(Store[3]); // returns the vector [1, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2), 0, 0]
	*/
 exports.FeatureSpace.prototype.updateRecords = function (rs) {};
/**
	* Creates a sparse feature vector from the given record.
	* @param {module:qm.Record} rec - The given record.
	* @returns {module:la.SparseVector} The sparse feature vector gained from rec.
	*/
 exports.FeatureSpace.prototype.extractSparseVector = function (rec) {}
/**
	* Creates a feature vector from the given record.
	* @param {module:qm.Record} rec - The given record.
	* @returns {module:la.Vector} The feature vector gained from rec.
	*/
 exports.FeatureSpace.prototype.extractVector = function (rec) {};
/**
	* Performs the inverse operation of ftrVec. Works only for numeric feature extractors.
	* @param {(module:qm.Vector | Array.<Object>)} ftr - The feature vector or an array with feature values.
	* @returns {module:qm.Vector} The inverse of ftr as vector.
	*/
 exports.FeatureSpace.prototype.invertFeatureVector = function (ftr) {};
/**
	* Calculates the inverse of a single feature using a specific feature extractor.
	* @param {number} idx - The index of the specific feature extractor.
	* @param {Object} val - The value to be inverted.
	* @returns {Object} The inverse of val using the feature extractor with index idx.
	*/
 exports.FeatureSpace.prototype.invertFeature = function (idx, val) {};
/**
	* Extracts the sparse feature vectors from the record set and returns them as columns of the sparse matrix.
	* @param {module:qm.RecordSet} rs - The given record set.
	* @returns {module:la.SparseMatrix} The sparse matrix, where the i-th column is the sparse feature vector of the i-th record in rs.
	*/
 exports.FeatureSpace.prototype.extractSparseMatrix = function (rs) {};
/**
	* Extracts the feature vectors from the recordset and returns them as columns of a dense matrix.
	* @param {module:qm.RecordSet} rs - The given record set.
	* @returns {module:la.Matrix} The dense matrix, where the i-th column is the feature vector of the i-th record in rs.
	*/
 exports.FeatureSpace.prototype.extractMatrix = function (rs) {};
/**
	* Gives the name of feature extractor at given position.
	* @param {number} idx - The index of the feature extractor in feature space (zero based).
	* @returns {String} The name of the feature extractor at position idx.
	*/
 exports.FeatureSpace.prototype.getFeatureExtractor = function (idx) {};
/**
	* Gives the name of the feature at the given position.
	* @param {number} idx - The index of the feature in feature space (zero based).
	* @returns {String} The name of the feature at the position idx.
	*/
 exports.FeatureSpace.prototype.getFeature = function (idx) {};
/**
	* Filters the vector to keep only the elements from the feature extractor.
	* @param {(module:la.Vector | module:la.SparseVector)} vec - The vector from where the function filters the elements.
	* @param {number} idx - The index of the feature extractor.
	* @param {boolean} [keepOffset = 'true'] - For keeping the original indexing in the new vector.
	* @returns {(module:la.Vector | module:la.SparseVector)} 
	* <br>1. module:la.Vector, if vec is of type module:la.Vector.
	* <br>2. module:la.SparseVector, if vec is of type module:la.SparseVector.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new feature space
	* var ftr = new qm.FeatureSpace(base, [
    *     { type: "numeric", source: "FtrSpaceTest", field: "Value" },
    *     { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
	*     { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] }
	*     ]);
	* // create a new dense vector
	* var vec = new qm.la.Vector([1, 0, 1, 0, 1, 0, 0, 1, 0, 0]);
	* // filter the elements from the second feature extractor
	* var vec2 = ftr.filter(vec, 1); // returns vector [0, 0, 1, 0, 0, 0, 0, 0, 0, 0]
	* // filter the elements from the second feature extractor, without keeping the offset
	* var vec3 = ftr.filter(vec, 1, false); // returns vector [0, 1, 0]
	* // create a new sparse vector
	* var spVec = new qm.la.SparseVector([[0, 1], [2, 1], [4, 1], [7, 1]]);
	* // filter the elements from the second feature extractor
	* var spVec2 = ftr.filter(spVec, 1); // returns sparse vector [[2, 1]]
	* // filter the elements from the second feature extractor, without keeping the offset
	* var spVec3 = ftr.filter(spVec, 1, false); // returns sparse vector [[1, 1]]
	*/
 exports.FeatureSpace.prototype.filter = function (vec, idx, keepOffset) {};
