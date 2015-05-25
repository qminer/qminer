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
*		"name": "People",
*		"fields": [
*				{ "name": "Name", "type": "string", "primary": true },
*				{ "name": "Gender", "type": "string", "shortstring": true },
*				{ "name": "Age", "type": "int" }
*		],
*		"joins": [
*				{ "name": "ActedIn", "type": "index", "store": "Movies", "inverse": "Actor" },
*				{ "name": "Directed", "type": "index", "store": "Movies", "inverse": "Director" }
*		],
*		"keys": [
*				{ "field": "Name", "type": "text" },
*				{ "field": "Gender", "type": "value" }
*		]
*	},
*	{
*		"name": "Movies",
*		"fields": [
*				{ "name": "Title", "type": "string", "primary": true },
*				{ "name": "Plot", "type": "string", "store": "cache" },
*				{ "name": "Year", "type": "int" },
*				{ "name": "Rating", "type": "float" },
*				{ "name": "Genres", "type": "string_v", "codebook": true }
*		],
*		"joins": [
*				{ "name": "Actor", "type": "index", "store": "People", "inverse": "ActedIn" },
*				{ "name": "Director", "type": "index", "store": "People", "inverse": "Directed" }
*		],
*		"keys": [
*				{ "field": "Title", "type": "value" },
*				{ "field": "Plot", "type": "text", "vocabulary": "voc_01" },
*				{ "field": "Genres", "type": "value" }
*		]
*	},
* // using a constructor
* var base = new qm.Base({
*		mode: "createClean",
*		schema: [{
*			"name": "Class",
*			"fields": [
*				{ "name", "Name", "type": "string" },
*				{ "name", "StudyGroup", "type": "string" }
*			]
*		}]
*	 })
*/
 exports.Store = function (base, storeDef) {};
/**
	* Returns a record from the store.
	* @param {string} recName - Record name.
	* @returns {Object} Returns the record. If the record doesn't exist, it returns null. //TODO
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
	*	mode: "createClean",
	*	schema: [{
	*		"name": "Class",
	*		"fields": [
	*			{ "name", "Name", "type": "string" },
	*			{ "name", "StudyGroup", "type": "string" }
	*		]
	*	}]
	* })
	* // add some records to the store
	* base.store("Class").add({ "Name": "Abed", "StudyGroup": "A" });
	* base.store("Class").add({ "Name": "Annie", "StudyGroup": "B" });
	* base.store("Class").add({ "Name": "Britta", "StudyGroup": "C" });
	* base.store("Class").add({ "Name": "Jeff", "StudyGroup": "A" });
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
	*	mode: "createClean",
	*	schema: [{
	*		"name": "Class",
	*		"fields": [
	*			{ "name", "Name", "type": "string" },
	*			{ "name", "StudyGroup", "type": "string" }
	*		]
	*	}]
	* })
	* // add some records to the store
	* base.store("Class").add({ "Name": "Shirley", "StudyGroup": "A" });
	* base.store("Class").add({ "Name": "Troy", "StudyGroup": "B" });
	* base.store("Class").add({ "Name": "Chang", "StudyGroup": "C" });
	* base.store("Class").add({ "Name": "Pierce", "StudyGroup": "A" });
	* // make an array of record names
	* var arr = base.store("Class").map(function (rec) { return rec.Name; }); // returns an array ["Shirley", "Troy", "Chang", "Pierce"]
	*/
 exports.Store.prototype.map = function (callback) {}
/**
	* Adds a record to the store.
	* @param {Object} rec - The added record. //TODO
	* @returns {number} The ID of the added record.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base with one two stores
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
	* })
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
	*    mode: "createClear",
	*    schema: [{
	*        "name": "Planets",
	*        "fields": [
	*            { "name": "Name", "type": "string" },
	*            { "name": "Diameter", "type": "int" },
	*            { "name": "NearestStars", "type": "string_v" }
	*        ]
	*    }]
	* })
	* // add a new planet in the store
	* base.store("Planets").push({ "Name": "Earth", "Diameter": 299196522, "NearestStars": ["Sun"] });
	* // create a record of a planet (not added to the Planets store)
	* var planet = base.store("Planets").newRecord({ "Name": "Tatooine", "Diameter": 10465, "NearestStars": ["Tatoo 1", "Tatoo 2"] });
	*/
 exports.Store.prototype.newRecord = function (json) {};
/**
	* Creates a new record set out of the records in store.
	* @param {module:la.IntVector} idVec - The integer vector containing the ids of selected vectors.
	* @returns {module:qm.RecordSet} The record set that contains the records gained with idVec.
	*/
 exports.Store.prototype.newRecordSet = function (idVec) {};
/**
	* Creates a record set containing random records from store.
	* @param {number} sampleSize - The size of the record set.
	* @returns {module:qm.RecordSet} Returns a record set containing random records.
	*/
 exports.Store.prototype.sample = function (sampleSize) {};
/**
	* Gets the details of the selected field.
	* @param {string} fieldName - The name of the field.
	* @returns {Object} The JSON object containing the details of the field. //TODO
	*/
 exports.Store.prototype.field = function (fieldName) {}; 
/**
	* Checks if the field is of numeric type.
	* @param {string} fieldName - The checked field.
	* @returns {boolean} True, if the field is of numeric type. Otherwise, false.
	*/
 exports.Store.prototype.isNumeric = function (fieldName) {};
/**
	* Checks if the field is of string type.
	* @param {string} fieldName - The checked field.
	* @returns {boolean} True, if the field is of string type. Otherwise, false.
	*/
 exports.Store.prototype.isString = function (fieldName) {}; 
/**
	* Returns the details of the selected key.
	* @param {string} keyName - The selected key.
	* @returns {Object} The JSON object containing the details of the key. //TODO
	*/
 exports.Store.prototype.key = function (keyName) {};
/**
	* Returns the store as a JSON.
	* @returns {Object} The store as a JSON.
	*/
 exports.Store.prototype.toJSON = function () {};
/**
	* Deletes the records in the store.
	* @param {number} [num] - The number of deleted records.
	* @returns {number} The number of remaining records in the store.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base with one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "TVSeries",
	*        "fields": [
	*            { "name": "Title", "type": "string", "primary": true },
	*            { "name": "NumberOfEpisodes", "type": "int" }
	*        ]
	*    }]
	* })
	* // add some records in the store
	* base.store("TVSeries").push({ "Title": "Archer", "NumberOfEpisodes": 75 });
	* base.store("TVSeries").push({ "Title": "The Simpsons", "NumberOfEpisodes": 574 });
	* base.store("TVSeries").push({ "Title": "New Girl", "NumberOfEpisodes": 94);
	* base.store("TVSeries").push({ "Title": "Rick and Morty", "NumberOfEpisodes": 11});
	* base.store("TVSeries").push({ "Title": "Game of Thrones", "NumberOfEpisodes": 47 });
	* // deletes the first 2 records (Archer and The Simpsons) in TVSeries
	* store.clear(2); // returns 3
	* // delete all remaining records in TVStore
	* store.clear();  // returns 0
	*/
 exports.Store.prototype.clear = function (num) {};
/**
	* Gives a vector containing the field value of each record.
	* @param {string} fieldName - The field name. Field must be of one-dimensional type, e.g. int, float, string...
	* @returns {module:la.Vector} The vector containing the field values of each record.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base with one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        "name": "Companies",
	*        "fields": [
	*            { "name": "Name", "type": "string", "primary": true },
	*            { "name": "Location", "type": "string" }
	*        ]
	*    }]
	* })
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
	*/
 exports.Store.prototype.getMatrix = function (fieldName) {};
/**
	* Gives the field value of a specific record.
	* @param {number} recId - The record id.
	* @param {string} fieldName - The field's name.
	* @returns {Object} The fieldName value of the record with recId.
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
* Record (factory pattern).
* @namespace
*/
 exports.Record = function () {}; 
/**
	* Clones the record.
	* @returns {module:qm.Record} The clone of the record.
	*/
 exports.Record.prototype.$clone = function () {};
/**
	* Creates a JSON version of the record.
	* @returns {Object} The JSON version of the record.
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
	*/
 exports.RecordSet.prototype.clone = function () {};
/**
	* Creates a new record set out of the join attribute of records.
	* @param {string} joinName - The name of the join attribute.
	* @param {number} [sampleSize] - The number of records to be used for construction of the record set.
	* @returns {module:qm.RecordSet} The record set containing the join records.
	*/
 exports.RecordSet.prototype.join = function (joinName, sampleSize) {};
/**
	* Truncates the first records.
	* @param {number} limit_num - How many records to truncate.
	* @param {number} [offset_num] - Where to start to truncate.
	* @returns {module:qm.RecordSet} Self.
	* @example
	* // import qm module
	* qm = require('qminer');
	* // construct a record set with 20 records
	* rs = //TODO
	* rs2 = //TODO
	* // truncate the first 10 records
	* rs.trunc(10); // returns self, only with the first 10 records
	* // truncate the first 10 records starting with the 5th
	* rs2.trunc(10, 4);
	*/
 exports.RecordSet.prototype.trunc = function (limit_num, offset_num) {};
/**
	* Creates a sample of records of the record set.
	* @param {number} num - The number of records in the sample.
	* @returns {module:qm.RecordSet} A record set containing the sample records.
	*/
 exports.RecordSet.prototype.sample = function (num) {};
/**
	* Shuffles the order of records in the record set.
	* @param {number} [seed] - Integer.
	* @returns {module:qm.RecordSet} Self.
	*/
 exports.RecordSet.prototype.shuffle = function (seed) {};
/**
	* It reverses the record order.
	* @returns {module:qm.RecordSet} Self. Records are in reversed order.
	*/
 exports.RecordSet.prototype.reverse = function () {};
/**
	* Sorts the records according to record id.
	* @param {number} [asc=1] - If asc > 0, it sorts in ascending order. Otherwise, it sorts in descending order.  
	* @returns {module:qm.RecordSet} Self. Records are sorted according to record id and asc.
	*/
 exports.RecordSet.prototype.sortById = function (asc) {}; 
/**
	* Sorts the records according to a specific record field.
	* @param {string} fieldName - The field by which the sort will work.
	* @param {number} [arc=-1] - if asc > 0, it sorts in ascending order. Otherwise, it sorts in descending order.
	* @returns {module:qm.RecordSet} Self. Records are sorted according to fieldName and arc.
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
	* qm = require('qminer');
	* // construct a new record set of movies (one field is it's Rating)
	* var rs = //TODO
	* // sort the records by their rating
	* rs.sort(function (rec, rec2) { return rec.Rating < rec2.Rating ;});
	*/
 exports.RecordSet.prototype.sort = function (callback) {};
/**
	* Keeps only records with ids between two values.
	* @param {number} [minId] - The minimum id.
	* @param {number} [maxId] - The maximum id.
	* @returns {module:qm.RecordSet} Self. 
	* <br>1. Contains only the records of the original with ids between minId and maxId, if parameters are given.
	* <br>2. Contains all the records of the original, if no parameter is given.
	*/
 exports.RecordSet.prototype.filterById = function (minId, maxId) {};
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
	*/
 exports.RecordSet.prototype.filterByField = function (fieldName, minVal, maxVal) {};
/**
	* Keeps only the records that pass the callback function.
	* @param {function} callback - The filter function. It takes one parameter and return a boolean object.
	* @returns {module:qm.RecordSet} Self. Containing only the record that pass the callback function.
	* @example
	* // import qm module
	* qm = require('qminer');
	* // construct a record set of kitchen appliances
	* var rs = //TODO
	* // filter by the field price
	* rs.filter(function (rec) { return rec.Price > 10000; }); // keeps only the records, where their Price is more than 10000
	*/
 exports.RecordSet.prototype.filter = function (callback) {}; 
/**
	* Splits the record set into smaller record sets.
	* @param {function} callback - The splitter function. It takes two parameters (records) and returns a boolean object.
	* @returns {Array.<module:qm.RecordSet>} An array containing the smaller record sets. The records are split according the callback function.
	*/
 exports.RecordSet.prototype.split = function (callback) {};
/**
	* Deletes the records, that are also in the other record set.
	* @param {module:qm.RecordSet} rs - The other record set.
	* @returns {module:qm.RecordSet} Self. Contains only the records, that are not in rs.
	*/
 exports.RecordSet.prototype.deleteRecords = function (rs) {}; 
/**
	* Returns the record set as a JSON.
	* @returns {Object} The record set as a JSON.
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
	* // create a record set with some people with fields Name and Gender
	* var rs = //TODO
	* // change the gender of all records to "Extraterrestrial"
	* rs.each(function (rec) { rec.Gender = "Extraterrestrial"; });
	*/
 exports.RecordSet.prototype.each = function (callback) {}
/**
	* Creates an array of function outputs created from the records in record set.
	* @param {function} callback - Function that generates the array. It takes two parameters:
	* <br>rec - The current record.
	* <br>[idx] - The index of the current record.
	* @returns {Array<Object>} The array created by the callback function. //TODO
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a record set with some people with fields Name and Gender
	* var rs = //TODO
	* // make an array of record names
	* var arr = rs.map(function (rec) { return rec.Name; });
	*/
 exports.RecordSet.prototype.map = function (callback) {}
/**
	* Creates the set intersection of two record sets.
	* @param {module:qm.RecordSet} rs - The other record set.
	* @returns {module:qm.RecordSet} The intersection of the two record sets.
	*/
 exports.RecordSet.prototype.setIntersect = function (rs) {};
/**
	* Creates the set union of two record sets.
	* @param {module:qm.RecordSet} rs - The other record set.
	* @returns {module:qm.RecordSet} The union of the two record sets.
	*/
 exports.RecordSet.prototype.setUnion = function (rs) {};
/**
	* Creates the set difference between two record sets.
	* @param {module:qm.RecordSet} rs - The other record set.
	* @returns {module:qm.RecordSet} The difference between the two record sets.
	*/
 exports.RecordSet.prototype.setDiff = function (rs) {}; 
/**
	* Creates a vector containing the field values of records.
	* @param {string} fieldName - The field from which to take the values. It's type must be one-dimensional, e.g. float, int, string,...
	* @returns {module:la.Vector} The vector containing the field values of records. The type it contains is dependant of the field type.
	*/
 exports.RecordSet.prototype.getVector = function (fieldName) {}; 
/**
	* Creates a vector containing the field values of records.
	* @param {string} fieldName - The field from which to take the values. It's type must be numeric, e.g. float, int, float_v, num_sp_v,...
	* @returns {(module:la.Matrix|module:la.SparseMatrix)} The matrix containing the field values of records.
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
