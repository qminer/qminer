//intellisense start
// this file mimicks the qminer module index.js file
exports = {}; require.modules.qminer = exports;
exports.la = require('qminer_la');
exports.fs = require('qminer_fs');
exports.analytics = require('qminer_analytics');
exports.ht= require('qminer_ht');
exports.statistics= require('qminer_stat');
exports.datasets= require('qminer_datasets');
//intellisense end
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
	* @property {boolean} debug - True if QMiner was compiled in debug mode, else false.
	*/
 exports.debug = false;
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
*       type: "Movies",
*       type: [{ name: "title", type: "string" }]
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
*   Vector: [[0,1], [1,1]]});
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
* @property {module:qm~FeatureExtractorDateWindow} dateWindow - The date window type.
* @property {module:qm~FeatureExtractorSparseVector} sparseVector - The sparse vector type.
*
*/
/**
* @typedef {Object} FeatureExtractorConstant
* The feature extractor of type 'contant'.
* @property {string} type - The type of the extractor. It must be equal <b>'constant'</b>.
* @property {number} [const = 1.0] - A constant number. 
* @property {module:qm~FeatureSource} source - The source of the extractor.
* @example
* var qm = require('qminer');
* // create a simple base, where each record contains only a persons name
* var base = new qm.Base({
*   mode: 'createClean',
*   schema: [{
*      type: "Person",
*      type: [{ type: "Name", type: "string" }]
*   }]
* });
* // create a feature space containing the constant extractor, where the constant is equal 5
* var ftr = qm.FeatureSpace(base, { type: "constant", source: "Person", const: 5 });
*/
/**
* @typedef {Object} FeatureExtractorRandom
* The feature extractor of type 'random'.
* @property {string} type - The type of the extractor. It must be equal <b>'random'</b>.
* @property {number} [seed = 0] - The seed number used to construct the random number.
* @property {module:qm~FeatureSource} source - The source of the extractor.
* @example
* var qm = require('qminer');
* // create a simple base, where each record contains only a persons name
* var base = new qm.Base({
*   mode: 'createClean',
*   schema: [{
*      type: "Person",
*      type: [{ type: "Name", type: "string" }]
*   }]
* });
* // create a feature space containing the random extractor
* var ftr = qm.FeatureSpace(base, { type: "random", source: "Person" });
*/
/**
* @typedef {Object} FeatureExtractorNumeric 
* The feature extractor of type 'numeric'.
* @property {string} type - The type of the extractor. It must be equal <b>'numeric'</b>.
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
*       type: "Class",
*       type: [
*          { type: "Name", type: "string" },
*          { type: "Grade", type: "number" }
*       ]
*    }]
* });
* // create a feature space containing the numeric extractor, where the values are 
* // normalized, the values are taken from the field "Grade"
* var ftr = qm.FeatureSpace(base, { type: "numeric", source: "Class", normalize: true, field: "Grade" });
*/
/**
 * @typedef {Object} FeatureExtractorSparseVector
 * The feature extractor of type 'num_sp_v'.
 * @property {string} type - The type of the extractor. It must be equal <b>'num_sp_v'</b>.
 * @property {number} [dimension = 0] - Dimensionality of sparse vectors.
 * @property {boolean} [normalize = false] - Normalize vectors to L2 norm of 1.0.
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
 *          { "name": "Features", "type": "num_sp_v" }
 *       ]
 *    }]
 * });
 * // create a feature space containing the numeric extractor, where the values are
 * // normalized, the values are taken from the field "Grade"
 * var ftr = qm.FeatureSpace(base, { type: "num_sp_v", source: "Class", normalize: false, field: "Features" });
 */
/**
* @typedef {Object} FeatureExtractorCategorical
* The feature extractor of type 'categorical'.
* @property {string} type - The type of the extractor. It must be equal <b>'categorical'</b>.
* @property {Array.<Object>} [values] - A fixed set of values, which form a fixed feature set. No dimensionality changes if new values are seen in the upgrades.
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
*       type: "Class",
*       type: [
*          { type: "Name", type: "string" },
*          { type: "StudyGroup", type: "string" }
*       ]
*    }]
* });
* // create a feature space containing the categorical extractor, where the it's values
* // are taken from the field "StudyGroup": "A", "B", "C" and "D"
* var ftr = qm.FeatureSpace(base, { type: "categorical", source: "Class", field: "StudyGroup", values: ["A", "B", "C", "D"] });
*/
/**
* @typedef {Object} FeatureExtractorMultinomial
* The feature extractor of type 'multinomial'.
* @property {string} type - The type of the extractor. It must be equal <b>'multinomial'</b>.
* @property {boolean} [normalize = 'false'] - Normalize the resulting vector of the extractor to have L2 norm 1.0.
* @property {Array.<Object>} [values] - A fixed set of values, which form a fixed feature set, no dimensionality changes if new values are seen in the updates. Cannot be used the same time as datetime.
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
*       type: "Class",
*       type: [
*          { type: "Name", type: "string" },
*          { type: "StudyGroups", type: "string_v" }
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
* @typedef {Object} FeatureExtractorText
* The feature extractor of type 'text'.
* @property {string} type - The type of the extractor. It must be equal <b>'text'</b>.
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
*       type: "Articles",
*       type: [
*          { type: "Title", type: "string" },
*          { type: "Text", type: "string" }
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
* @typedef {Object} FeatureExtractorJoin
* The feature extractor of type 'join'.
* @property {string} type - The type of the extractor. It must be equal <b>'join'</b>.
* @property {number} [bucketSize = 1] - The size of the bucket in which we group consecutive records.
* @property {module:qm~FeatureSource} source - The source of the extractor.
* @example
* // import qm module
* var qm = require('qminer');
*/
/**
* @typedef {Object} FeatureExtractorPair
* The feature extractor of type 'pair'.
* @property {string} type - The type of the extractor. It must be equal <b>'pair'</b>.
* @property {module:qm~FeatureExtractors} first - The first feature extractor.
* @property {module:qm~FeatureExtractors} second - The second feature extractor.
* @property {module:qm~FeatureSource} source - The source of the extractor.
* @example
* var qm = require('qminer');
*/
/** 
* @typedef {Object} FeatureExtractorDateWindow
* The feature extractor of type 'dateWindow'.
* @property {string} type - The type of the extractor. It must be equal <b>'dateWindow'</b>.
* @property {string} [unit = 'day'] - How granular is the time window. The options are: 'day', 'week', 'month', 'year', '12hours', '6hours', '4hours', '2hours',
* 'hour', '30minutes', '15minutes', '10minutes', 'minute', 'second'.
* @property {number} [window = 1] - The size of the window.
* @property {boolean} [normalize = 'false'] - Normalize the resulting vector of the extractor to have L2 norm 1.0. //TODO
* @property {number} start - //TODO
* @property {number} end - //TODO
* @property {module:qm~FeatureSource} source - The source of the extractor.
* @example
* // import qm module
* var qm = require('qminer');
*/
/**
* @typedef {Object} FeatureExtractorJsfunc
* The feature extractor of type 'jsfunc'.
* @property {string} type - The type of the extractor. It must be equal <b>'jsfunc'</b>.
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
*       type: "Class",
*       type: [
*          { type: "Name", type: "string" },
*          { type: "StudyGroups", type: "string_v" }
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
		/** Sets the product of the tf and idf frequency. */
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
* @classdesc Represents the database and holds stores. The base object can be opened in multiple
* modes: 'create' - create a new database, 'createClean' - force create, and 'openReadOnly' - open in read-only mode.
* @class
* @param {module:qm~BaseConstructorParam} paramObj - The base constructor parameter object.
* @property {String} paramObj.mode - The mode in which base is opened.
* @property [String] paramObj.dbPath - The path to the location of the database.
* @property [Object] paramObj.schema - The database schema.
* @example
* // import qm module
* var qm = require('qminer');
* // using a constructor, in open mode
* var base = new qm.Base({mode: 'open'});
*/
 exports.Base = function (paramObj) { return Object.create(require('qminer').Base.prototype); };
/**
	* Closes the database.
	* @returns {null}
	*/
 exports.Base.prototype.close = function () { return null; }
/**
	 * Returns the store with the specified name.
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
	 *        name: "KwikEMart",
	 *        fields: [
	 *            { name: "Worker", type: "string" },
	 *            { name: "Groceries", type: "string_v" }
	 *        ]
	 *    },
	 *    {
	 *        name: "NuclearPowerplant",
	 *        fields: [
	 *            { name: "Owner", type: "string" },
	 *            { name: "NumberOfAccidents", type: "int" },
	 *            { name: "Workers", type: "string_v" }
	 *        ]
	 *    }]
	 * });
	 * // get the "KwikEMart" store 
	 * var store = base.store("KwikEMart");	// returns the store with the name "KwikEMart"
	 */
 exports.Base.prototype.store = function (name) { return Object.create(require('qminer').Store.prototype); }
/**
	 * Returns a list of store descriptors.
	 * @returns {Object[]} The list of store descriptors.
	 */
 exports.Base.prototype.getStoreList = function () { return [{storeId:'', storeName:'', storeRecords:'', fields: [], keys: [], joins: []}]; }
/**
	* Creates a new store.
	* @param {Array.<module:qm~SchemaDefinition>} storeDef - The definition of the store(s).
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
	*        name: "Superheroes",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Superpowers", type: "string_v" },
	*            { name: "YearsActive", type: "int" }
	*        ]
	*    }]
	* });
	* // create a new store called "Supervillains" in the base
	* base.createStore({
	*    name: "Supervillians",
	*    fields: [
	*        { name: "Name", type: "string" },
	*        { name: "Superpowers", type: "string_v" },
	*        { name: "YearsActive", type: "int" }
	*    ]
	* });
	* // create two new stores called "Cities" and "Leagues"
	* base.createStore([
	*    {
	*        name: "Cities",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "Population", type: "int" }
	*        ]
	*    },
	*    {
	*        name: "Leagues",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Members", type: "string_v" }
	*        ]
	*    }
	* ]);
	*/
 exports.Base.prototype.createStore = function (storeDef, storeSizeInMB) { return storeDef instanceof Array ? [Object.create(require('qminer').Store.prototype)] : Object.create(require('qminer').Store.prototype) ;}
/**
	* Creates a new store.
	* @param {module:qm~QueryObject} query - query language JSON object	
	* @returns {module:qm.RecordSet} - Returns the record set that matches the search criterion
	*/
 exports.Base.prototype.search = function (query) { return Object.create(require('qminer').RecordSet.prototype); }
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
	* Gets the stream aggregate of the given name.
	* @param {string} saName - The name of the stream aggregate.
	* @returns {module:qm.StreamAggr} The stream aggregate whose name is saName.
	*/
 exports.Base.prototype.getStreamAggr = function (saName) { return Object.create(require('qminer').StreamAggr.prototype); }
/**
	* Gets an array of the stream aggregate names in the base.
	* @returns {Array.<string>} The array containing the stream aggregat names.
	*/
 exports.Base.prototype.getStreamAggrNames = function () { return [""]; }
/**
* Store (factory pattern result) 
* @namespace
* @example
* // import qm module
* var qm = require('qminer');
* // factory based construction using base.createStore
* var base = qm.create('qm.conf', "", true);
* base.createStore([{
*    name: "People",
*    fields: [
*        { name: "Name", type: "string", primary: true },
*        { name: "Gender", type: "string", shortstring: true },
*        { name: "Age", type: "int" }
*    ],
*    joins: [
*        { name: "ActedIn", type: "index", store: "Movies", inverse: "Actor" },
*        { name: "Directed", type: "index", store: "Movies", inverse: "Director" }
*    ],
*    keys: [
*        { field: "Name", type: "text" },
*        { field: "Gender", type: "value" }
*    ]
* },
* {
*    name: "Movies",
*    fields: [
*        { name: "Title", type: "string", primary: true },
*        { name: "Plot", type: "string", store: "cache" },
*        { name: "Year", type: "int" },
*        { name: "Rating", type: "float" },
*        { name: "Genres", type: "string_v", codebook: true }
*    ],
*    joins: [
*        { name: "Actor", type: "index", store: "People", inverse: "ActedIn" },
*        { name: "Director", type: "index", store: "People", inverse: "Directed" }
*    ],
*    keys: [
*        { field: "Title", type: "value" },
*        { field: "Plot", type: "text", vocabulary: "voc_01" },
*        { field: "Genres", type: "value" }
*    ]
* }]);
* // using the base constructor
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [{
*        name: "Class",
*        fields: [
*            { name: "Name", type: "string" },
*            { name: "StudyGroup", type: "string" }
*        ]
*    }]
* });
*/
 exports.Store = function (base, storeDef) { return Object.create(require('qminer').Store.prototype); };
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
	*        name: "Class",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "StudyGroup", type: "string" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Class").push({ Name: "Dean", StudyGroup: "A" });
	* base.store("Class").push({ Name: "Chang", StudyGroup: "D" });
	* base.store("Class").push({ Name: "Magnitude", StudyGroup: "C" });
	* base.store("Class").push({ Name: "Leonard", StudyGroup: "B" });
	* // get the record with the name "Magnitude"
	* var record = base.store("Class").rec("Magnitude");
	*/
 exports.Store.prototype.recordByName = function (recName) { return Object.create(require('qminer').Record.prototype); };
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
	*        name: "Class",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "StudyGroup", type: "string" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Class").push({ Name: "Abed", StudyGroup: "A" });
	* base.store("Class").push({ Name: "Annie", StudyGroup: "B" });
	* base.store("Class").push({ Name: "Britta", StudyGroup: "C" });
	* base.store("Class").push({ Name: "Jeff", StudyGroup: "A" });
	* // change the StudyGroup of all records of store Class to A
	* base.store("Class").each(function (rec) { rec.StudyGroup = "A"; });	// all records in Class are now in study group A
	*/
 exports.Store.prototype.each = function (callback) { return Object.create(require('qminer').Store.prototype); }
/**
	* Creates an array of function outputs created from the store records.
	* @param {function} callback - Function that generates the array. It takes two parameters:
	* <br>rec - The current record.
	* <br>[idx] - The index of the current record.
	* @returns {Array<Object>} The array created by the callback function.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing the store Class
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Class",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "StudyGroup", type: "string" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Class").push({ Name: "Shirley", StudyGroup: "A" });
	* base.store("Class").push({ Name: "Troy", StudyGroup: "B" });
	* base.store("Class").push({ Name: "Chang", StudyGroup: "C" });
	* base.store("Class").push({ Name: "Pierce", StudyGroup: "A" });
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
	*        name: "Superheroes",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Superpowers", type: "string_v" }
	*        ]
	*    },
	*    {
	*        name: "Supervillians",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Superpowers", type: "string_v" }
	*        ]
	*    }]
	* });
	* // add a new superhero to the Superheroes store
	* base.store("Superheroes").push({ Name: "Superman", Superpowers: ["flight", "heat vision", "bulletproof"] }); // returns 0
	* // add a new supervillian to the Supervillians store
	* base.store("Supervillians").push({ Name: "Lex Luthor", Superpowers: ["expert engineer", "genius-level intellect", "money"] }); // returns 0
	*/
 exports.Store.prototype.push = function (rec) { return 0; }
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
	*        name: "Planets",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Diameter", type: "int" },
	*            { name: "NearestStars", type: "string_v" }
	*        ]
	*    }]
	* });
	* // add a new planet in the store
	* base.store("Planets").push({ Name: "Earth", Diameter: 299196522, NearestStars: ["Sun"] });
	* // create a record of a planet (not added to the Planets store)
	* var planet = base.store("Planets").newRecord({ Name: "Tatooine", Diameter: 10465, NearestStars: ["Tatoo 1", "Tatoo 2"] });
	*/
 exports.Store.prototype.newRecord = function (json) { return Object.create(require('qminer').Record.prototype); };
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
	*        name: "Superheroes",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Superpowers", type: "string_v" }
	*        ]
	*    }]
	* });
	* // add some new records to the store
	* base.store("Superheroes").push({ Name: "Superman", Superpowers: ["Superhuman strength, speed, hearing", "Flight", "Heat vision"] });
	* base.store("Superheroes").push({ Name: "Batman", Superpowers: ["Genius-level intellect", "Peak physical and mental conditioning", "Master detective"] });
	* base.store("Superheroes").push({ Name: "Thor", Superpowers: ["Superhuman strength, endurance and longevity", "Abilities via Mjolnir"] });
	* base.store("Superheroes").push({ Name: "Wonder Woman", Superpowers: ["Superhuman strength, agility and endurance", "Flight", "Highly skilled hand-to-hand combatant"] });
	* // create a new record set containing only the DC Comic superheroes (those with the record ids 0, 1 and 3)
	* var intVec = new qm.la.IntVector([0, 1, 3]);
	* var DCHeroes = base.store("Superheroes").newRecordSet(intVec);
	*/
 exports.Store.prototype.newRecordSet = function (idVec) { return Object.create(require('qminer').RecordSet.prototype); };
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
	*        name: "TVSeries",
	*        fields: [
	*            { name: "Title", type: "string", primary: true },
	*            { name: "NumberOfEpisodes", type: "int" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("TVSeries").push({ Title: "Archer", NumberOfEpisodes: 75 });
	* base.store("TVSeries").push({ Title: "The Simpsons", NumberOfEpisodes: 574 });
	* base.store("TVSeries").push({ Title: "New Girl", NumberOfEpisodes: 94 });
	* base.store("TVSeries").push({ Title: "Rick and Morty", NumberOfEpisodes: 11 });
	* base.store("TVSeries").push({ Title: "Game of Thrones", NumberOfEpisodes: 47 });
	* // create a sample record set containing 3 records
	* var randomRecordSet = base.store("TVSeries").sample(3); // contains 3 random records from the TVSeries store
	*/
 exports.Store.prototype.sample = function (sampleSize) { return Object.create(require('qminer').RecordSet.prototype); };
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
	*        name: "People",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "Gender", type: "string" },
	*            { name: "Age", type: "int" }
	*        ]
	*    }]
	* });
	* // get the details of the field "Name" of store "People"
	* // it returns a JSON object:
	* // { id: 0, name: "Name", type: "string", primary: true }
	* var details = base.store("People").field("Name");
	*/
 exports.Store.prototype.field = function (fieldName) { return { id: 0, name:'', type:'', primary:'' }; }; 
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
	*        name: "TVSeries",
	*        fields: [
	*            { name: "Title", type: "string", primary: true },
	*            { name: "NumberOfEpisodes", type: "int" }
	*        ]
	*    }]
	* });
	* // check if the field "Title" is of numeric type
	* var isTitleNumeric = base.store("TVSeries").isNumeric("Title"); // returns false
	* // check if the field "NumberOfEpisodes" is of numeric type
	* var isNumberOfEpisodesNumeric = base.store("TVSeries").isNumeric("NumberOfEpisodes"); // returns true
	*/
 exports.Store.prototype.isNumeric = function (fieldName) { return true; };
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
	*        name: "People",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "Gender", type: "string" },
	*            { name: "Age", type: "int" }
	*        ]
	*    }]
	* });
	* // check if the field "Name" is of string type
	* var isNameString = base.store("People").isString("Name"); // returns true
	* // check if the field "Age" is of string type
	* var isAgeString = base.store("People").isString("Age"); // returns false
	*/
 exports.Store.prototype.isString = function (fieldName) { return true; }; 
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
	*        name: "BasketballPlayers",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "SeasonScore", type: "int_v" },
	*            { name: "DateOfBirth", type: "datetime" }
	*        ]
	*    }]
	* });
	* // check if the SeasonScore field is of type Date
	* var isSeasonScoreDate = base.store("BasketballPlayers").isDate("SeasonScore"); // returns false
	* // check if the FirstPlayed field is of type Date
	* var isFirstPlayedDate = base.store("BasketballPlayers").isDate("DateOfBirth"); // returns true
	*/
 exports.Store.prototype.isDate = function (fieldName) { return true; }
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
	*        name: "Countries",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "Population", type: "int" },
	*            { name: "Continent", type: "string" }
	*        ],
	*        keys: [
	*            { field: "Name", type: "text" },
	*            { field: "Continent", type: "value" }
	*        ]
	*    }]
	* });
	* // get the details of the key of the field "Continent"
	* // returns a JSON object containing the details of the key:
	* // { fq: { length: 0 }, vocabulary: { length: 0 }, name: 'Continent', store: { name: 'Countries', ... }}
	* var details = base.store("Countries").key("Continent");
	*/
 exports.Store.prototype.key = function (keyName) { return { fq: {}, vocabulary: {}, name:'', store: {} }; };
/**
	* Returns the stream aggregate with the given name.
	* @param {string} saName - The name of the stream aggregate.
	* @returns {module:qm.StreamAggr} The stream aggregate with the saName.
	*/
 exports.Store.prototype.getStreamAggr = function (saName) { return Object.create(require('qminer').StreamAggr.prototype); }
/**
	* Returns an array of the stream aggregates names connected to the store.
	* @returns {Array.<string>} An array of stream aggregates names.
	*/
 exports.Store.prototype.getStreamAggrNames = function () { return [""]; }
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
	*        name: "FootballPlayers",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "FootballClubs", type: "string_v" },
	*            { name: "GoalsPerSeason", type: "int_v" },
	*        ]
	*    }]
	* });
	* // get the store as a JSON object
	* // the returned JSON object is:
	* // { storeId: 0, storeName: 'FootballPlayers', storeRecords: 0, fields: [...], keys: [], joins: [] }
	* var json = base.store("FootballPlayers").toJSON();
	*/
 exports.Store.prototype.toJSON = function () { return { storeId:0, storeName:'', storeRecords:'', fields:[], keys:[], joins:[] }; };
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
	*        name: "TVSeries",
	*        fields: [
	*            { name: "Title", type: "string", primary: true },
	*            { name: "NumberOfEpisodes", type: "int" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("TVSeries").push({ Title: "Archer", NumberOfEpisodes: 75 });
	* base.store("TVSeries").push({ Title: "The Simpsons", NumberOfEpisodes: 574 });
	* base.store("TVSeries").push({ Title: "New Girl", NumberOfEpisodes: 94 });
	* base.store("TVSeries").push({ Title: "Rick and Morty", NumberOfEpisodes: 11 });
	* base.store("TVSeries").push({ Title: "Game of Thrones", NumberOfEpisodes: 47 });
	* // deletes the first 2 records (Archer and The Simpsons) in TVSeries
	* base.store("TVSeries").clear(2); // returns 3
	* // delete all remaining records in TVStore
	* base.store("TVSeries").clear();  // returns 0
	*/
 exports.Store.prototype.clear = function (num) { return 0; };
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
	*        name: "Companies",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "Location", type: "string" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Companies").push({ Name: "DC Comics", Location: "Burbank, California" });
	* base.store("Companies").push({ Name: "DC Shoes", Location: "Huntington Beach, California" });
	* base.store("Companies").push({ Name: "21st Century Fox", Location: "New York City, New York" });
	* // get the vector of company names
	* var companyNames = base.store("Companies").getVector("Name");	// returns a vector ["DC Comics", "DC Shoes", "21st Century Fox"]
	*/
 exports.Store.prototype.getVector = function (fieldName) { return Object.create(require('qminer').la.Vector.prototype); };
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
	*        name: "ArcheryChampionship",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "ScorePerRound", type: "float_v" }
	*        ]
	*    }]
	* });
	* // set new records in the store
	* base.store("ArcheryChampionship").push({ Name: "Robin Hood", ScorePerRound: [50, 48, 48] });
	* base.store("ArcheryChampionship").push({ Name: "Oliver Queen", ScorePerRound: [44, 46, 44] });
	* base.store("ArcheryChampionship").push({ Name: "Legolas", ScorePerRound: [50, 50, 48] });
	* // get the matrix containing the "score per round" values
	* // The values of the i-th column are the values of the i-th record.
	* // The function will give the matrix:
	* // 50  44  50
	* // 48  46  50
	* // 48  44  48
	* var matrix = base.store("ArcheryChampionship").getMatrix("ScorePerRound");
	*/
 exports.Store.prototype.getMatrix = function (fieldName) { return Object.create(require('qminer').la.Matrix.prototype); };
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
	*        name: "Festivals",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Type", type: "string" },
	*            { name: "Location", type: "string" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("Festivals").push({ Name: "Metaldays", Type: "music", Location: "Tolmin, Slovenia" });
	* base.store("Festivals").push({ Name: "Festival de Cannes", Type: "movie", Location: "Cannes, France" });
	* base.store("Festivals").push({ Name: "The Festival of Chocolate", Type: "food", Location: "Hillsborough, USA" });
	* // get the field value of the second record for field "Type"
	* var fieldValue = base.store("Festivals").cell(1, "Type"); // returns "movie"
	*/
 exports.Store.prototype.cell = function (recId, fieldName) {};
/**
	* Gives the name of the store.
	*/
 exports.Store.prototype.name = "";
/**
	* Checks if the store is empty.
	*/
 exports.Store.prototype.empty = true;
/**
	* Gives the number of records.
	*/
 exports.Store.prototype.length = 0;
/**
	* Creates a record set containing all the records from the store.
	*/
 exports.Store.prototype.allRecords = Object.create(require('qminer').RecordSet.prototype);
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
 exports.Store.prototype.first = Object.create(require('qminer').Record.prototype);
/**
	* Returns the last record of the store.
	*/
 exports.Store.prototype.last = Object.create(require('qminer').Record.prototype);
/**
	* Returns an iterator for iterating over the store from start to end.
	*/
 exports.Store.prototype.forwardIter = Object.create(require('qminer').Iterator.prototype);
/**
	* Returns an iterator for iterating over the store form end to start.
	*/
 exports.Store.prototype.backwardIter = Object.create(require('qminer').Iterator.prototype);
/**
	* Gets the record with the given ID.
	* @param {number} recId - The id of the record.
	* @returns {module:qm.Record} The record with the ID equal to recId.
	* @ignore
	*/
 exports.Store.prototype.store = function (recId) { };
/**
	* Returns the base, in which the store is contained.
	*/
 exports.Store.prototype.base = Object.create(require('qminer').Base.prototype);
/**
* Record (factory pattern). The records are used for storing data in {@link module:qm.Store}.
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
	*        name: "StarWarsMovies",
	*        fields: [
	*            { name: "Title", type: "string" },
	*            { name: "ReleseDate", type: "datetime" },
	*            { name: "Length", type: "int" }
	*        ]
	*    }]
	* });
	* // create some records in the new store
	* base.store("StarWarsMovies").push({ Title: "Attack of the Clones", ReleseDate: "2002-05-16T00:00:00", Length: 142 });
	* base.store("StarWarsMovies").push({ Title: "The Empire Strikes Back", ReleseDate: "1980-06-20T00:00:00", Length: 124 });
	* base.store("StarWarsMovies").push({ Title: "Return of the Jedi", ReleseDate: "1983-05-25T00:00:00", Length: 134 });
	* // create a clone of the "Attack of the Clones" record
	* var clone = base.store("StarWarsMovies")[0].$clone();
	*/
 exports.Record.prototype.$clone = function () { return Object.create(require('qminer').Record.prototype); };
/**
     * Provide json version of record, useful when calling JSON.stringify
     *
     * @param {Boolean} - ???
     * @param {Boolean} - ???
     * @param {Boolean} [sysFields=true] - if set to true system fields, like $id, will be included
     */
/**
	* Adds a join record `joinRecord` to join `jonName` (string) with join frequency `joinFrequency`
	* @param {string} joinName - join name
	* @param {(module:qm.Record | number)} joinRecord - joined record or its ID
	* @param {number} [joinFrequency=1] - frequency attached to the join
	* @returns {module:qm.Record} Record.
    * @example
    * //TODO
	*/
 exports.Record.prototype.$addJoin = function (joinName, joinRecord, joinFrequency) { return Object.create(require('qminer').Record.prototype); }
/**
	* Deletes join record `joinRecord` from join `joinName` (string) with join frequency `joinFrequency`.
	* @param {string} joinName - join name
	* @param {(module:qm.Record | number)} joinRecord - joined record or its ID
	* @param {number} [joinFrequency=1] - frequency attached to the join
	* @returns {module:qm.Record} Record.
    * @example
    * //TODO
	*/
 exports.Record.prototype.$delJoin = function (joinName, joinRecord, joinFrequency) { return Object.create(require('qminer').Record.prototype); }
/**
	* Creates a JSON version of the record.
	*
    * @param {Boolean} - ???
    * @param {Boolean} - ???
    * @param {Boolean} [sysFields=true] - if set to true system fields, like $id, will be included
	* @returns {Object} The JSON version of the record.
	*
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Musicians",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "DateOfBirth", type: "datetime" },
	*            { name: "GreatestHits", type: "string_v" }
	*        ]
	*    }]
	* });
	* // create some records
	* base.store("Musicians").push({ Name: "Jimmy Page", DateOfBirth:  "1944-01-09T00:00:00", GreatestHits: ["Stairway to Heaven", "Whole Lotta Love"] });
	* base.store("Musicians").push({ Name: "Beyonce", DateOfBirth: "1981-09-04T00:00:00", GreatestHits: ["Single Ladies (Put a Ring on It)"] });
	* // get a JSON version of the "Beyonce" record 
	* // The JSON object for this example si:
	* // { '$id': 1, Name: 'Beyonce', ActiveSince: '1981-09-04T00:00:00', GreatestHits: ['Single Ladies (Put a Ring on It)'] }
	* var json = base.store("Musicians").rec("Beyonce").toJSON();
	*/
 exports.Record.prototype.toJSON = function () {};
/**
	* Returns the id of the record.
	*/
 exports.Record.prototype.$id = 0;
/**
	* Returns the name of the record.
	*/
 exports.Record.prototype.$name = "";
/**
	* Returns the frequency of the record.
	*/
 exports.Record.prototype.$fq = 0;
/**
	* Returns the store the record belongs to.
	*/
 exports.Record.prototype.store = Object.create('qminer').Store.prototype;
/**
* Record Set (factory pattern). The Record Set is a set of records.
* @namespace
* @example
* // import qm module
* var qm = require('qminer');
* // factory based construction using store.allRecords
* var rs = store.allRecords;
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
	*        name: "Philosophers",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Era", type: "string" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("Philosophers").push({ Name: "Plato", Era: "Ancient philosophy" });
	* base.store("Philosophers").push({ Name: "Immanuel Kant", Era: "18th-century philosophy" });
	* base.store("Philosophers").push({ Name: "Emmanuel Levinas", Era: "20th-century philosophy" });
	* base.store("Philosophers").push({ Name: "Rene Descartes", Era: "17th-century philosophy" });
	* base.store("Philosophers").push({ Name: "Confucius", Era: "Ancient philosophy" });
	* // create a record set out of the records in store
	* var recordSet = base.store("Philosophers").allRecords;
	* // clone the record set of the "Philosophers" store
	* var philosophers = recordSet.clone();
	*/
 exports.RecordSet.prototype.clone = function () { return Object.create(require(qminer).RecordSet.prototype); };
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
	*        name: "Musicians",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Instruments", type: "string_v" }
	*        ],
	*        joins: [
	*            { name: "PlaysIn", type: "index", store: "Bands", inverse: "Members" }
	*        ]
	*    },
	*    {
	*        name: "Bands",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Genre", type: "string" }
	*        ],
	*        joins: [
	*            { name: "Members", type: "index", store: "Musicians", inverse: "PlaysIn" }
	*        ]
	*    }]
	* });
	* // add some new records to both stores
	* base.store("Musicians").push({ Name: "Robert Plant", Instruments: ["Vocals"], PlaysIn: [{Name: "Led Zeppelin", "Genre": "Rock" }] });
	* base.store("Musicians").push({ Name: "Jimmy Page", Instruments: ["Guitar"], PlaysIn: [{Name: "Led Zeppelin", "Genre": "Rock" }] });
	* base.store("Bands").push({ Name: "The White Stripes", Genre: "Rock" });
	* // create a record set containing the musicians, that are members of some bend
	* // returns a record set containing the records of "Robert Plant" and "Jimmy Page"
	* var ledZeppelin = base.store("Bands").allRecords.join("Members");
	* // create a record set containing the first musician, that is a member of some band
	* // returns a record set containing only one record, which is "Robert Plant" or "Jimmy Page"
	* var ledMember = base.store("Bands").allRecords.join("Members", 1);
	*/
 exports.RecordSet.prototype.join = function (joinName, sampleSize) { return Object.create(require('qminer').RecordSet.prototype); };
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
	*        name: "Philosophers",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Era", type: "string" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("Philosophers").push({ Name: "Plato", Era: "Ancient philosophy" });
	* base.store("Philosophers").push({ Name: "Immanuel Kant", Era: "18th-century philosophy" });
	* base.store("Philosophers").push({ Name: "Emmanuel Levinas", Era: "20th-century philosophy" });
	* base.store("Philosophers").push({ Name: "Rene Descartes", Era: "17th-century philosophy" });
	* base.store("Philosophers").push({ Name: "Confucius", Era: "Ancient philosophy" });
	* // create two identical record sets of the "Philosophers" store
	* var recordSet1 = base.store("Philosophers").allRecords;
	* var recordSet2 = base.store("Philosophers").allRecords;
	* // truncate the first 3 records in recordSet1
	* recordSet1.trunc(3); // return self, containing only the first 3 records ("Plato", "Immanuel Kant", "Emmanuel Levinas")
	* // truncate the first 2 records in recordSet2, starting with "Emmanuel Levinas"
	* recordSet2.trunc(2, 2); // returns self, containing only the 2 records ("Emmanuel Levinas", "Rene Descartes")
	*/
 exports.RecordSet.prototype.trunc = function (limit_num, offset_num) { return Object.create(require('qminer').RecordSet.prototype); };
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
	*        name: "Movies",
	*        fields: [
	*            { name: "Title", type: "string" },
	*            { name: "Length", type: "int" },
	*            { name: "Director", type: "string" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("Movies").push({ Title: "The Nightmare Before Christmas", Length: 76, Director: "Henry Selick" });
	* base.store("Movies").push({ Title: "Jurassic Part", Length: 127, Director: "Steven Spielberg" });
	* base.store("Movies").push({ Title: "The Avengers", Length: 143, Director: "Joss Whedon" });
	* base.store("Movies").push({ Title: "The Clockwork Orange", Length: 136, Director: "Stanley Kubrick" });
	* base.store("Movies").push({ Title: "Full Metal Jacket", Length: 116, Director: "Stanely Kubrick" });
	* // create a sample record set of containing 3 records from the "Movies" store
	* var sample = base.store("Movies").allRecords.sample(3);
	*/
 exports.RecordSet.prototype.sample = function (num) { return Object.create(require('qminer').RecordSet.prototype); };
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
	*        name: "WeatherForcast",
	*        fields: [
	*            { name: "Weather", type: "string" },
	*            { name: "Date", type: "datetime" },
	*            { name: "TemperatureDegrees", type: "int" }
	*        ]
	*    }]
	* });
	* // put some records in the "WeatherForecast" store
	* base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-27T11:00:00", TemperatureDegrees: 19 });
	* base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-28T11:00:00", TemperatureDegrees: 22 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-29T11:00:00", TemperatureDegrees: 25 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-30T11:00:00", TemperatureDegrees: 25 });
	* base.store("WeatherForcast").push({ Weather: "Scattered Showers", Date: "2015-05-31T11:00:00", TemperatureDegrees: 24 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-06-01T11:00:00", TemperatureDegrees: 27 });
	* // get the record set containing the records from the "WeatherForcast" store
	* var recordSet = base.store("WeatherForcast").allRecords;
	* // shuffle the records in the newly created record set. Use the number 100 as the seed for the shuffle
	* recordSet.shuffle(100); // returns self, the records in the record set are shuffled
	*/
 exports.RecordSet.prototype.shuffle = function (seed) { return Object.create(require('qminer').RecordSet.prototype); };
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
	*        name: "WeatherForcast",
	*        fields: [
	*            { name: "Weather", type: "string" },
	*            { name: "Date", type: "datetime" },
	*            { name: "TemperatureDegrees", type: "int" },
	*        ]
	*    }]
	* });
	* // put some records in the "WeatherForecast" store
	* base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-27T11:00:00", TemperatureDegrees: 19 });
	* base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-28T11:00:00", TemperatureDegrees: 22 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-29T11:00:00", TemperatureDegrees: 25 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-30T11:00:00", TemperatureDegrees: 25 });
	* base.store("WeatherForcast").push({ Weather: "Scattered Showers", Date: "2015-05-31T11:00:00", TemperatureDegrees: 24 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-06-01T11:00:00", TemperatureDegrees: 27 });
	* // get the record set containing the records from the "WeatherForcast" store
	* var recordSet = base.store("WeatherForcast").allRecords;
	* // reverse the record order in the record set
	* recordSet.reverse(); // returns self, the records in the record set are in the reverse order
	*/
 exports.RecordSet.prototype.reverse = function () { return Object.create(require('qminer').RecordSet.prototype); };
/**
	* Sorts the records according to record id.
	* @param {number} [asc=-1] - If asc > 0, it sorts in ascending order. Otherwise, it sorts in descending order.  
	* @returns {module:qm.RecordSet} Self. Records are sorted according to record id and asc.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Tea",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Type", type: "string"},
	*            { name: "Origin", type: "string", "null": true  }
	*        ]
	*    }]
	* });
	* // put some records in the "Tea" store
	* base.store("Tea").push({ Name: "Tanyang Gongfu", Type: "Black", Origin: "Tanyang" });
	* base.store("Tea").push({ Name: "Rou Gui", Type: "White" });
	* base.store("Tea").push({ Name: "Tieluohan Tea", Type: "Wuyi", Origin: "Northern Fujian" });
	* base.store("Tea").push({ Name: "Red Robe", Type: "Oolong", Origin: "Wuyi Mountains" });
	* // get the records of the "Tea" store as a record set
	* var recordSet = base.store("Tea").allRecords;
	* // sort the records in the record set by their id in descending order
	* recordSet.sortById(); // returns self, the records are sorted in descending order (default)
	* // sort the records in the record set by their id in ascending order
	* recordSet.sortById(1); // returns self, the records are sorted in ascending order
	*/
 exports.RecordSet.prototype.sortById = function (asc) { return Object.create(require('qminer').RecordSet.prototype); }; 
/**
	* Sorts the records according to their weight.
	* @param {number} [asc=1] - If asc > 0, it sorts in ascending order. Otherwise, it sorts in descending order.
	* @returns {module:qm.RecordSet} Self. Records are sorted according to record weight and asc.
	* @ignore
	*/
 exports.RecordSet.prototype.sortByFq = function (asc) { return Object.create(require('qminer').RecordSet.prototype); }; 
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
	*        name: "TVSeries",
	*        fields: [
	*            { name: "Title", type: "string", primary: true },
	*            { name: "NumberOfEpisodes", type: "int" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("TVSeries").push({ Title: "Archer", NumberOfEpisodes: 75 });
	* base.store("TVSeries").push({ Title: "The Simpsons", NumberOfEpisodes: 574 });
	* base.store("TVSeries").push({ Title: "New Girl", NumberOfEpisodes: 94 });
	* base.store("TVSeries").push({ Title: "Rick and Morty", NumberOfEpisodes: 11 });
	* base.store("TVSeries").push({ Title: "Game of Thrones", NumberOfEpisodes: 47 });
	* // get the records of the "TVSeries" store as a record set
	* var recordSet = base.store("TVSeries").allRecords;
	* // sort the records by their "Title" field in ascending order 
	* recordSet.sortByField("Title", true); // returns self, record are sorted by their "Title"
	*/
 exports.RecordSet.prototype.sortByField = function (fieldName, asc) { return Object.create(require('qminer').RecordSet.prototype); };
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
	*        name: "TVSeries",
	*        fields: [
	*            { name: "Title", type: "string", primary: true },
	*            { name: "NumberOfEpisodes", type: "int" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("TVSeries").push({ Title: "Archer", NumberOfEpisodes: 75 });
	* base.store("TVSeries").push({ Title: "The Simpsons", NumberOfEpisodes: 574 });
	* base.store("TVSeries").push({ Title: "New Girl", NumberOfEpisodes: 94 });
	* base.store("TVSeries").push({ Title: "Rick and Morty", NumberOfEpisodes: 11 });
	* base.store("TVSeries").push({ Title: "Game of Thrones", NumberOfEpisodes: 47 });
	* // get the records of the "TVSeries" store as a record set
	* var recordSet = base.store("TVSeries").allRecords;
	* // sort the records by their number of episodes
	* recordSet.sort(function (rec, rec2) { return rec.NumberOfEpisodes < rec2.NumberOfEpisodes; }); // returns self, records are sorted by the number of episodes
	*/
 exports.RecordSet.prototype.sort = function (callback) { return Object.create(require('qminer').RecordSet.prototype); };
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
	*        name: "FrankSinatraGreatestHits",
	*        fields: [
	*            { name: "Title", type: "string" },
	*            { name: "Length", type: "int" }
	*        ]
	*    }]
	* });
	* // put some records in the "FrankSinatraGreatesHits" store
	* base.store("FrankSinatraGreatestHits").push({ Title: "Strangers in the Night", Length: 145 });
	* base.store("FrankSinatraGreatestHits").push({ Title: "Summer Wind", Length: 173 });
	* base.store("FrankSinatraGreatestHits").push({ Title: "It Was a Very Good Year", Length: 265 });
	* base.store("FrankSinatraGreatestHits").push({ Title: "Somewhere in Your Heart", Length: 146 });
	* base.store("FrankSinatraGreatestHits").push({ Title: "Forget Domani", Length: 156 });
	* base.store("FrankSinatraGreatestHits").push({ Title: "Somethin' Stupid", Length: 155 });
	* base.store("FrankSinatraGreatestHits").push({ Title: "This Town", Length: 186 });
	* // get the records of the store as a record set
	* var recordSet = base.store("FrankSinatraGreatestHits").allRecords;
	* // from the record set keep the records with indeces between or equal 2 and 5
	* recordSet.filterById(2, 5);
	*/
 exports.RecordSet.prototype.filterById = function (minId, maxId) { return Object.create(require('qminer').RecordSet.prototype); };
/**
	* Keeps only the records with weight between two values.
	* @param {number} [minFq] - The minimum value.
	* @param {number} [maxFq] - The maximum value.
	* @returns {module:qm.RecordSet} Self.
	* <br>1. Contains only the records of the original with weights between minFq and maxFq, if parameters are given.
	* <br>2. Contains all the records of the original, if no parameter is given.
	* @ignore
	*/
 exports.RecordSet.prototype.filterByFq = function (minFq, maxFq) { return Object.create(require('qminer').RecordSet.prototype); };
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
	*        name: "WeatherForcast",
	*        fields: [
	*            { name: "Weather", type: "string" },
	*            { name: "Date", type: "datetime" },
	*            { name: "TemperatureDegrees", type: "int" },
	*        ]
	*    }]
	* });
	* // put some records in the "WeatherForecast" store
	* base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-27T11:00:00", TemperatureDegrees: 19 });
	* base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-28T11:00:00", TemperatureDegrees: 22 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-29T11:00:00", TemperatureDegrees: 25 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-30T11:00:00", TemperatureDegrees: 25 });
	* base.store("WeatherForcast").push({ Weather: "Scattered Showers", Date: "2015-05-31T11:00:00", TemperatureDegrees: 24 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-06-01T11:00:00", TemperatureDegrees: 27 });
	* // get the record set containing the records from the "WeatherForcast" store
	* var recordSet = base.store("WeatherForcast").allRecords;
	* // filter only the records, where the weather is Mostly Cloudy
	* recordSet.filterByField("Weather", "Mostly Cloudy"); // returns self, containing only the records, where the weather is "Mostly Cloudy"
	*/
 exports.RecordSet.prototype.filterByField = function (fieldName, minVal, maxVal) { return Object.create(require('qminer').RecordSet.prototype); };
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
	*        name: "ArcheryChampionship",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "ScorePerRound", type: "float_v" }
	*        ]
	*    }]
	* });
	* // set new records in the store
	* base.store("ArcheryChampionship").push({ Name: "Robin Hood", ScorePerRound: [50, 48, 48] });
	* base.store("ArcheryChampionship").push({ Name: "Oliver Queen", ScorePerRound: [44, 46, 44] });
	* base.store("ArcheryChampionship").push({ Name: "Legolas", ScorePerRound: [50, 50, 48] });
	* // create a record set out of the records of the store
	* var recordSet = base.store("ArcheryChampionship").allRecords;
	* // filter the records: which archers have scored 48 points in the third round
	* recordSet.filter(function (rec) { return rec.ScorePerRound[2] == 48; }); // keeps only the records, where the score of the third round is equal 48
	*/
 exports.RecordSet.prototype.filter = function (callback) { return Object.create(require('qminer').RecordSet.prototype); }; 
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
	*        name: "SocialGames",
	*        fields: [
	*            { name: "Title", type: "string" },
	*            { name: "Type", type: "string" },
	*            { name: "MinPlayers", type: "int" },
	*            { name: "MaxPlayers", type: "int" }
	*        ]
	*    }]
	* });
	* // set new records in the store
	* base.store("SocialGames").push({ Title: "DungeonsAndDragons", Type: "Role-Playing", MinPlayers: 5, MaxPlayers: 5 });
	* base.store("SocialGames").push({ Title: "Dobble", Type: "Card", MinPlayers: 2, MaxPlayers: 8 });
	* base.store("SocialGames").push({ Title: "Settlers of Catan", Type: "Board", MinPlayers: 3, MaxPlayers: 4 });
	* base.store("SocialGames").push({ Title: "Munchkin", Type: "Card", MinPlayers: 3, MaxPlayers: 6 });
	* // create a record set out of the records of the store
	* var recordSet = base.store("SocialGames").allRecords;
	* // sort the records by MinPlayers in ascending order
	* recordSet.sortByField("MinPlayers", true);
	* // split the record set by the minimum number of players
	* // returns an array containing three record sets: the first containing the "DungeonsAndDragons" record,
	* // the second containing the "Settlers of Catan" and "Munchkin" records and the third containing the 
	* // "Dobble" record
	* var arr = recordSet.split(function (rec, rec2) { return rec.MinPlayers < rec2.MinPlayers; });
	*/
 exports.RecordSet.prototype.split = function (callback) {return [Object.create(require('qminer').RecordSet.prototype)]; };
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
	*        name: "BookWriters",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Genre", type: "string" },
	*            { name: "Books", type: "string_v" }
	*        ]
	*    }]
	* });
	* // set new records in the store
	* base.store("BookWriters").push({ Name: "Terry Pratchett", Genre: "Fantasy", Books: ["The Colour of Magic", "Going Postal", "Mort", "Guards! Guards!"] });
	* base.store("BookWriters").push({ Name: "Douglas Adams", Genre: "Sci-fi", Books: ["The Hitchhiker's Guide to the Galaxy", "So Long, and Thanks for All the Fish"] });
	* base.store("BookWriters").push({ Name: "Fyodor Dostoyevsky", Genre: "Drama", Books: ["Crime and Punishment", "Demons"] });
	* base.store("BookWriters").push({ Name: "J.R.R. Tolkien", Genre: "Fantasy", Books: ["The Hobbit", "The Two Towers", "The Silmarillion" ] });
	* base.store("BookWriters").push({ Name: "George R.R. Martin", Genre: "Fantasy", Books: ["A Game of Thrones", "A Feast of Crows"] });
	* base.store("BookWriters").push({ Name: "J. K. Rowling", Genre: "Fantasy", Books: ["Harry Potter and the Philosopher's Stone"] });
	* base.store("BookWriters").push({ Name: "Ivan Cankar", Genre: "Drama", Books: ["On the Hill", "The King of Betajnova", "The Serfs"] });
	* // create one record set containing all records of store
	* var recordSet = base.store("BookWriters").allRecords;
	* // create one record set containing the records with genre "Fantasy"
	* var fantasy = base.store("BookWriters").allRecords.filterByField("Genre", "Fantasy");
	* // delete the records in recordSet, that are also in fantasy
	* recordSet.deleteRecords(fantasy); // returns self, containing only three records: "Douglas Adams", "Fyodor Dostoyevsky" and "Ivan Cankar"
	*/
 exports.RecordSet.prototype.deleteRecords = function (rs) { return Object.create(require('qminer').RecordSet.prototype); }; 
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
	*        name: "Musicians",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "DateOfBirth", type: "datetime" },
	*            { name: "GreatestHits", type: "string_v" }
	*        ]
	*    }]
	* });
	* // create some records
	* base.store("Musicians").push({ Name: "Jimmy Page", DateOfBirth:  "1944-01-09T00:00:00", GreatestHits: ["Stairway to Heaven", "Whole Lotta Love"] });
	* base.store("Musicians").push({ Name: "Beyonce", DateOfBirth: "1981-09-04T00:00:00", GreatestHits: ["Single Ladies (Put a Ring on It)"] });
	* // create a record set out of the records in the "Musicians" store
	* var recordSet = base.store("Musicians").allRecords;
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
	*        name: "People",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Gender", type: "string" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("People").push({ Name: "Eric Sugar", Gender: "Male" });
	* base.store("People").push({ Name: "Jane Tokyo", Gender: "Female" });
	* base.store("People").push({ Name: "Mister Tea", Gender: "Male" });
	* // create a record set out of the records of the store
	* var recordSet = base.store("People").allRecords;
	* // change the Name of all records into "Anonymous"
	* recordSet.each(function (rec) { rec.Name = "Anonymous"; }); // returns self, all record's Name are "Anonymous"
	*/
 exports.RecordSet.prototype.each = function (callback) { return Object.create(require('qminer').RecordSet.prototype); }
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
	*        name: "People",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Gender", type: "string" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("People").push({ Name: "Eric Sugar", Gender: "Male" });
	* base.store("People").push({ Name: "Jane Tokyo", Gender: "Female" });
	* base.store("People").push({ Name: "Mister Tea", Gender: "Male" });
	* // create a record set out of the records of the store
	* var recordSet = base.store("People").allRecords;
	* // make an array of record Names
	* var arr = recordSet.map(function (rec) { return rec.Name; }); // returns an array: ["Eric Sugar", "Jane Tokyo", "Mister Tea"]
	*/
 exports.RecordSet.prototype.map = function (callback) { return [Object];  }
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
	*        name: "Movies",
	*        fields: [
	*            { name: "Title", type: "string" },
	*            { name: "Length", type: "int" },
	*            { name: "Director", type: "string" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("Movies").push({ Title: "The Nightmare Before Christmas", Length: 76, Director: "Henry Selick" });
	* base.store("Movies").push({ Title: "Jurassic Part", Length: 127, Director: "Steven Spielberg" });
	* base.store("Movies").push({ Title: "The Avengers", Length: 143, Director: "Joss Whedon" });
	* base.store("Movies").push({ Title: "The Clockwork Orange", Length: 136, Director: "Stanley Kubrick" });
	* base.store("Movies").push({ Title: "Full Metal Jacket", Length: 116, Director: "Stanely Kubrick" });
	* // create a record set out of the records in store, where length of the movie is greater than 110
	* var greaterSet = base.store("Movies").allRecords.filterByField("Length", 110, 150);
	* // create a record set out of the records in store, where the length of the movie is lesser than 130
	* var lesserSet = base.store("Movies").allRecords.filterByField("Length", 0, 130);
	* // get the intersection of greaterSet and lesserSet
	* var intersection = greaterSet.setIntersect(lesserSet); // returns a record set, containing the movies with lengths between 110 and 130
	*/
 exports.RecordSet.prototype.setIntersect = function (rs) { return Object.create(require('qminer').RecordSet.prototype); };
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
	*        name: "TVSeries",
	*        fields: [
	*            { name: "Title", type: "string", "primary": true },
	*            { name: "NumberOfEpisodes", type: "int" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("TVSeries").push({ Title: "Archer", NumberOfEpisodes: 75 });
	* base.store("TVSeries").push({ Title: "The Simpsons", NumberOfEpisodes: 574 });
	* base.store("TVSeries").push({ Title: "New Girl", NumberOfEpisodes: 94 });
	* base.store("TVSeries").push({ Title: "Rick and Morty", NumberOfEpisodes: 11 });
	* base.store("TVSeries").push({ Title: "Game of Thrones", NumberOfEpisodes: 47 });
	* // create a record set out of the records in store, where the number of episodes is lesser than 47
	* var lesserSet = base.store("TVSeries").allRecords.filterByField("NumberOfEpisodes", 0, 47);
	* // create a record set out of the records in store, where the number of episodes is greater than 100
	* var greaterSet = base.store("TVSeries").allRecords.filterByField("NumberOfEpisodes", 100, 600);
	* // get the union of lesserSet and greaterSet
	* var union = lesserSet.setUnion(greaterSet); // returns a record set, which is the union of the two record sets
	*/
 exports.RecordSet.prototype.setUnion = function (rs) { return Object.create(require('qminer').RecordSet.prototype); };
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
	*        name: "BookWriters",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Genre", type: "string" },
	*            { name: "Books", type: "string_v" }
	*        ]
	*    }]
	* });
	* // set new records in the store
	* base.store("BookWriters").push({ Name: "Terry Pratchett", Genre: "Fantasy", Books: ["The Colour of Magic", "Going Postal", "Mort", "Guards! Guards!"] });
	* base.store("BookWriters").push({ Name: "Douglas Adams", Genre: "Sci-fi", Books: ["The Hitchhiker's Guide to the Galaxy", "So Long, and Thanks for All the Fish"] });
	* base.store("BookWriters").push({ Name: "Fyodor Dostoyevsky", Genre: "Drama", Books: ["Crime and Punishment", "Demons"] });
	* base.store("BookWriters").push({ Name: "J.R.R. Tolkien", Genre: "Fantasy", Books: ["The Hobbit", "The Two Towers", "The Silmarillion" ] });
	* base.store("BookWriters").push({ Name: "George R.R. Martin", Genre: "Fantasy", Books: ["A Game of Thrones", "A Feast of Crows"] });
	* base.store("BookWriters").push({ Name: "J. K. Rowling", Genre: "Fantasy", Books: ["Harry Potter and the Philosopher's Stone"] });
	* base.store("BookWriters").push({ Name: "Ivan Cankar", Genre: "Drama", Books: ["On the Hill", "The King of Betajnova", "The Serfs"] });
	* // create one record set containing all records of store
	* var recordSet = base.store("BookWriters").allRecords;
	* // create one record set containing the records with genre "Fantasy"
	* var fantasy = base.store("BookWriters").allRecords.filterByField("Genre", "Fantasy");
	* // create a new record set containing the difference of recordSet and fantasy
	* var difference = recordSet.setDiff(fantasy); // returns a record set, containing the records of Douglas Adams, Fyodor Dostoyevsky and Ivan Cankar
	*/
 exports.RecordSet.prototype.setDiff = function (rs) { return Object.create(require('qminer').RecordSet.prototype); }; 
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
	*        name: "TVSeries",
	*        fields: [
	*            { name: "Title", type: "string", "primary": true },
	*            { name: "NumberOfEpisodes", type: "int" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("TVSeries").push({ Title: "Archer", NumberOfEpisodes: 75 });
	* base.store("TVSeries").push({ Title: "The Simpsons", NumberOfEpisodes: 574 });
	* base.store("TVSeries").push({ Title: "New Girl", NumberOfEpisodes: 94 });
	* base.store("TVSeries").push({ Title: "Rick and Morty", NumberOfEpisodes: 11 });
	* base.store("TVSeries").push({ Title: "Game of Thrones", NumberOfEpisodes: 47 });
	* // create a record set of the records of store
	* var recordSet = base.store("TVSeries").allRecords;
	* // create a vector containing the number of episodes for each series
	* // the vector will look like [75, 574, 94, 11, 47]
	* var vector = recordSet.getVector("NumberOfEpisodes");
	*/
 exports.RecordSet.prototype.getVector = function (fieldName) { return Object.create(require('qminer').la.Vector.prototype); }; 
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
	*        name: "ArcheryChampionship",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "ScorePerRound", type: "float_v" }
	*        ]
	*    }]
	* });
	* // set new records in the store
	* base.store("ArcheryChampionship").push({ Name: "Robin Hood", ScorePerRound: [50, 48, 48] });
	* base.store("ArcheryChampionship").push({ Name: "Oliver Queen", ScorePerRound: [44, 46, 44] });
	* base.store("ArcheryChampionship").push({ Name: "Legolas", ScorePerRound: [50, 50, 48] });
	* // create a record set of the records in store
	* var recordSet = base.store("ArcheryChampionship").allRecords;
	* // create a matrix from the "ScorePerRound" field
	* // the i-th column of the matrix is the data of the i-th record in record set
	* // the matrix will look like
	* // 50  44  50
	* // 48  46  50
	* // 48  44  48
	* var matrix = recordSet.getMatrix("ScorePerRound");
	*/
 exports.RecordSet.prototype.getMatrix = function (fieldName) { return Object.create(require('qminer').la.Matrix.prototype); };
/**
	* Returns the store, where the records in the record set are stored.
	*/
 exports.RecordSet.prototype.store = Object.create(require('qminer').Store.prototype);
/**
	* Returns the number of records in record set.
	*/
 exports.RecordSet.prototype.length = 0;
/**
	* Checks if the record set is empty. If the record set is empty, then it returns true. Otherwise, it returns false.
	*/
 exports.RecordSet.prototype.empty = true;
/**
	* Checks if the record set is weighted. If the record set is weighted, then it returns true. Otherwise, it returns false.
	*/
 exports.RecordSet.prototype.weighted = true;
/**
* Store Iterator (factory pattern)
* @namespace
* @example
* // import qm module
* qm = require('qminer');
* // create a new base with a simple store
* var base = new qm.Base({ mode: "createClean" });
* base.createStore({
*     name: "People",
*     fields: [
*         { name: "Name", type: "string" },
*         { name: "Gendre", type: "string" }
*     ]
* });
* // add new records to the store
* base.store("People").push({ Name: "Geronimo", Gender: "Male" });
* base.store("People").push({ Name: "Pochahontas", Gender: "Female" });
* base.store("People").push({ Name: "John Rolfe", Gender: "Male" });
* base.store("People").push({ Name: "John Smith", Gender: "Male"});
* // factory based construction with forwardIter
* var iter = base.store("People").forwardIter;
*/
 exports.Iterator = function () { return Object.create(require('qminer').Iterator.prototype); };
/**
	* Moves to the next record.
	* @returns {boolean} 
	* <br>1. True, if the iteration successfully moves to the next record.
	* <br>2. False, if there is no record left.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "TheWitcherSaga",
	*        fields: [
	*            { name: "Title", type: "string" },
	*            { name: "YearOfRelease", type: "int" },
	*            { name: "EnglishEdition", type: "bool" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("TheWitcherSaga").push({ Title: "Blood of Elves", YearOfRelease: 1994, EnglishEdition: true });
	* base.store("TheWitcherSaga").push({ Title: "Time of Contempt", YearOfRelease: 1995, EnglishEdition: true });
	* base.store("TheWitcherSaga").push({ Title: "Baptism of Fire", YearOfRelease: 1996, EnglishEdition: true });
	* base.store("TheWitcherSaga").push({ Title: "The Swallow's Tower", YearOfRelease: 1997, EnglishEdition: false });
	* base.store("TheWitcherSaga").push({ Title: "Lady of the Lake", YearOfRelease: 1999, EnglishEdition: false });
	* base.store("TheWitcherSaga").push({ Title: "Season of Storms", YearOfRelease: 2013, EnglishEdition: false });
	* // create an iterator for the store
	* var iter = base.store("TheWitcherSaga").forwardIter;
	* // go to the first record in the store
	* iter.next(); // returns true
	*/
 exports.Iterator.prototype.next = function () { return true; };
/**
	* Gives the store of the iterator.
	*/
 exports.Iterator.prototype.store = Object.create(require('qminer').Store.prototype);
/**
	* Gives the current record.
	*/
 exports.Iterator.prototype.record = Object.create(require('qminer').Record.prototype);
/**
* Feature Space
* @classdesc Represents the feature space. It contains any of the {@link module:qm~FeatureExtractors}.
* @class
* @param {module:qm.Base} base - The base where the features are extracted from.
* @param {(Array.<Object> | module:fs.FIn)} param - Array with definiton of extractors or input stream.
* @example
* // import qm module
* var qm = require('qminer');
* // construct a base with the store
* var base = new qm.Base({
*   mode: "createClean",
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
 exports.FeatureSpace = function (base, extractors) { return Object.create(require('qminer').FeatureSpace.prototype); };
/**
	* Returns the dimension of the feature space.
	*/
 exports.FeatureSpace.prototype.dim = 0;
/**
	* Returns an array of the dimensions of each feature extractor in the feature space.
	*/
 exports.FeatureSpace.prototype.dims = [0];
/**
	* Serialize the feature space to an output stream.
	* @param {module:fs.FOut} fout - The output stream.
	* @returns {module:fs.FOut} The output stream.
	*/
 exports.FeatureSpace.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); };
/**
	* Adds a new feature extractor to the feature space.
	* @param {Object} obj - The added feature extractor. It must be given as a JSON object.
	* @returns {module:qm.FeatureSpace} Self.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "WeatherForcast",
	*        fields: [
	*            { name: "Weather", type: "string" },
	*            { name: "Date", type: "datetime" },
	*            { name: "TemperatureDegrees", type: "int" }
	*        ]
	*    }]
	* });
	* // put some records in the "WeatherForecast" store
	* base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-27T11:00:00", TemperatureDegrees: 19 });
	* base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-28T11:00:00", TemperatureDegrees: 22 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-29T11:00:00", TemperatureDegrees: 25 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-30T11:00:00", TemperatureDegrees: 25 });
	* base.store("WeatherForcast").push({ Weather: "Scattered Showers", Date: "2015-05-31T11:00:00", TemperatureDegrees: 24 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-06-01T11:00:00", TemperatureDegrees: 27 });
	* // create a feature space 
	* var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "WeatherForcast", field: "TemperatureDegrees" });
	* // add a new feature extractor to the feature space
	* // it adds the new feature extractor to the pre-existing feature extractors in the feature space
	* ftr.addFeatureExtractor({ type: "text", source: "WeatherForcast", field: "Weather", normalize: true, weight: "tfidf" });      
	*/
 exports.FeatureSpace.prototype.addFeatureExtractor = function (obj) { return Object.create(require('qminer').FeatureSpace.prototype); };
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
    *   mode: "createClean",
    *   schema: {
    *     name: "FtrSpace",
    *     fields: [
    *       { name: "Value", type: "float" },
    *       { name: "Category", type: "string" },
    *       { name: "Categories", type: "string_v" },
    *     ]
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
	*   { type: "numeric", source: "FtrSpace", normalize: true, field: "Value" },
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
 exports.FeatureSpace.prototype.updateRecord = function (rec) { return Object.create(require('qminer').FeatureSpace.prototype); };
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
    *   mode: "createClean",
    *   schema: {
    *     name: "FtrSpace",
    *     fields: [
    *       { name: "Value", type: "float" },
    *       { name: "Category", type: "string" },
    *       { name: "Categories", type: "string_v" },
    *     ]
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
	*     { type: "numeric", source: "FtrSpace", normalize: true, field: "Value" },
	*     { type: "categorical", source: "FtrSpace", field: "Category", values: ["a", "b", "c"] },
	*     { type: "multinomial", source: "FtrSpace", field: "Categories", normalize: true, values: ["a", "b", "c", "q", "w", "e"] }
	* ]);
	* // update the feature space with the record set 
	* ftr.updateRecords(Store.allRecords);
	* // get the feature vectors of these records
	* ftr.extractVector(Store[0]); // returns the vector [0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2), 0, 0]
	* ftr.extractVector(Store[1]); // returns the vector [1/3, 0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2), 0]
	* ftr.extractVector(Store[2]); // returns the vector [2/3, 0, 0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2)]
	* ftr.extractVector(Store[3]); // returns the vector [1, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2), 0, 0]
	*/
 exports.FeatureSpace.prototype.updateRecords = function (rs) { return Object.create(require('qminer').FeatureSpace.prototype); };
/**
	* Creates a sparse feature vector from the given record.
	* @param {module:qm.Record} rec - The given record.
	* @returns {module:la.SparseVector} The sparse feature vector gained from rec.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing the store Class. Let the Name field be the primary field. 
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Class",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "StudyGroup", type: "string" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Class").push({ Name: "Dean", StudyGroup: "A" });
	* base.store("Class").push({ Name: "Chang", StudyGroup: "D" });
	* base.store("Class").push({ Name: "Magnitude", StudyGroup: "C" });
	* base.store("Class").push({ Name: "Leonard", StudyGroup: "B" });
	* // create a new feature space
	* // the feature space is of dimensions [0, 4]; 4 is the dimension of the categorical feature extractor
	* // and 0 is the dimension of text feature extractor (the text feature extractor doesn't have any words,
	* // need to be updated for use).
	* var ftr = new qm.FeatureSpace(base, [
	*    { type: "text", source: "Class", field: "Name", normalize: false },
	*    { type: "categorical", source: "Class", field: "StudyGroup", values: ["A", "B", "C", "D"] } 
	* ]);
	* // get the sparse extractor vector for the first record in store
	* // the sparse vector will be [(0, 1)] - uses only the categorical feature extractor. There are no
	* // features in the text feature extractor.
	* var vec = ftr.extractSparseVector(base.store("Class")[0]);
	*/
 exports.FeatureSpace.prototype.extractSparseVector = function (rec) { return Object.create(require('qminer').la.SparseVector.prototype); }
/**
	* Creates a feature vector from the given record.
	* @param {module:qm.Record} rec - The given record.
	* @returns {module:la.Vector} The feature vector gained from rec.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing the store Class. Let the Name field be the primary field.
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Class",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "StudyGroup", type: "string" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Class").push({ Name: "Jeff", StudyGroup: "A" });
	* base.store("Class").push({ Name: "Britta", StudyGroup: "D" });
	* base.store("Class").push({ Name: "Abed", StudyGroup: "C" });
	* base.store("Class").push({ Name: "Annie", StudyGroup: "B" });
	* // create a new feature space
	* // the feature space is of dimensions [0, 4]; 4 is the dimension of the categorical feature extractor
	* // and 0 is the dimension of text feature extractor (the text feature extractor doesn't have any words,
	* // need to be updated for use).
	* var ftr = new qm.FeatureSpace(base, [
	*    { type: "text", source: "Class", field: "Name", normalize: false },
	*    { type: "categorical", source: "Class", field: "StudyGroup", values: ["A", "B", "C", "D"] }
	* ]);
	* // get the extractor vector for the first record in store
	* // the sparse vector will be [1, 0, 0, 0] - uses only the categorical feature extractor. There are no
	* // features in the text feature extractor.
	* var vec = ftr.extractVector(base.store("Class")[0]);
	*/
 exports.FeatureSpace.prototype.extractVector = function (rec) { return Object.create(require('qminer').la.Vector.prototype); };
/**
	* Performs the inverse operation of ftrVec. Works only for numeric feature extractors.
	* @param {(module:la.Vector | Array.<Object>)} ftr - The feature vector or an array with feature values.
	* @returns {module:la.Vector} The inverse of ftr as vector.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "TheWitcherSaga",
	*        fields: [
	*            { name: "Title", type: "string" },
	*            { name: "YearOfRelease", type: "int" },
	*            { name: "EnglishEdition", type: "bool" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("TheWitcherSaga").push({ Title: "Blood of Elves", YearOfRelease: 1994, EnglishEdition: true });
	* base.store("TheWitcherSaga").push({ Title: "Time of Contempt", YearOfRelease: 1995, EnglishEdition: true });
	* base.store("TheWitcherSaga").push({ Title: "Baptism of Fire", YearOfRelease: 1996, EnglishEdition: true });
	* base.store("TheWitcherSaga").push({ Title: "The Swallow's Tower", YearOfRelease: 1997, EnglishEdition: false });
	* base.store("TheWitcherSaga").push({ Title: "Lady of the Lake", YearOfRelease: 1999, EnglishEdition: false });
	* base.store("TheWitcherSaga").push({ Title: "Season of Storms", YearOfRelease: 2013, EnglishEdition: false });
	* // create a feature space with the numeric feature extractor and update the feature space with the records in store
	* // for update, look the method updateRecords in feature space
	* var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "TheWitcherSaga", field: "YearOfRelease", normalize: true });
	* ftr.updateRecords(base.store("TheWitcherSaga").allRecords);
	* // get a feature vector for the second record
	* // because of the numeric feature extractor having normalize: true and of the records update of feature space, the values
	* // are not equal to those of the records, i.e. the value 1995 is now 0.105263 
	* var ftrVec = ftr.extractVector(base.store("TheWitcherSaga")[1]);
	* // get the inverse of the feature vector
	* // the function returns the values to their first value, i.e. 0.105263 returns to 1995
	* var inverse = ftr.invertFeatureVector(ftrVec); // returns a vector [1995]
	*/
 exports.FeatureSpace.prototype.invertFeatureVector = function (ftr) { return Object.create(require('qminer').la.Vector.prototype); };
/**
	* Calculates the inverse of a single feature using a specific feature extractor.
	* @param {number} idx - The index of the specific feature extractor.
	* @param {Object} val - The value to be inverted.
	* @returns {Object} The inverse of val using the feature extractor with index idx.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "TheWitcherSaga",
	*        fields: [
	*            { name: "Title", type: "string" },
	*            { name: "YearOfRelease", type: "int" },
	*            { name: "EnglishEdition", type: "bool" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("TheWitcherSaga").push({ Title: "Blood of Elves", YearOfRelease: 1994, EnglishEdition: true });
	* base.store("TheWitcherSaga").push({ Title: "Time of Contempt", YearOfRelease: 1995, EnglishEdition: true });
	* base.store("TheWitcherSaga").push({ Title: "Baptism of Fire", YearOfRelease: 1996, EnglishEdition: true });
	* base.store("TheWitcherSaga").push({ Title: "The Swallow's Tower", YearOfRelease: 1997, EnglishEdition: false });
	* base.store("TheWitcherSaga").push({ Title: "Lady of the Lake", YearOfRelease: 1999, EnglishEdition: false });
	* base.store("TheWitcherSaga").push({ Title: "Season of Storms", YearOfRelease: 2013, EnglishEdition: false });
	* // create a feature space with the numeric feature extractor and update the feature space with the records in store
	* // for update, look the method updateRecords in feature space
	* var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "TheWitcherSaga", field: "YearOfRelease", normalize: true });
	* ftr.updateRecords(base.store("TheWitcherSaga").allRecords);
	* // because of the numeric feature extractor having normalize: true and of the records update of feature space, 
	* // the values are not equal to those of the records 
	* // invert the value 0 using the numeric feature extractor
	* var inverse = ftr.invertFeature(0, 0); // returns the value 1994
	*/
 exports.FeatureSpace.prototype.invertFeature = function (idx, val) {};
/**
	* Extracts the sparse feature vectors from the record set and returns them as columns of the sparse matrix.
	* @param {module:qm.RecordSet} rs - The given record set.
	* @returns {module:la.SparseMatrix} The sparse matrix, where the i-th column is the sparse feature vector of the i-th record in rs.
	* @example
	* // import qm module
	* var qm = require("qminer");
	* // create a base containing the store Class. Let the Name field be the primary field. 
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Class",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "StudyGroups", type: "string_v" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Class").push({ Name: "Dean", StudyGroups: ["A", "D"] });
	* base.store("Class").push({ Name: "Chang", StudyGroups: ["B", "D"] });
	* base.store("Class").push({ Name: "Magnitude", StudyGroups: ["B", "C"] });
	* base.store("Class").push({ Name: "Leonard", StudyGroups: ["A", "B"] });
	* // create a feature space containing the multinomial feature extractor
	* var ftr = new qm.FeatureSpace(base, { type: "multinomial", source: "Class", field: "StudyGroups", values: ["A", "B", "C", "D"] });
	* // create a sparse feature matrix out of the records of the store by using the feature space
	* // returns a sparse matrix equal to 
	* // [[(0, 1), (3, 1)], [(1, 1), (3, 1)], [(1, 1), (2, 1)], [(0, 1), (1, 1)]]
	* var sparseMatrix = ftr.extractSparseMatrix(base.store("Class").allRecords);
	*/
 exports.FeatureSpace.prototype.extractSparseMatrix = function (rs) { return Object.create(require('qminer').la.SparseMatrix.prototype); };
/**
	* Extracts the feature vectors from the recordset and returns them as columns of a dense matrix.
	* @param {module:qm.RecordSet} rs - The given record set.
	* @returns {module:la.Matrix} The dense matrix, where the i-th column is the feature vector of the i-th record in rs.
	* @example
	* // import qm module
	* var qm = require("qminer");
	* // create a base containing the store Class. Let the Name field be the primary field.
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Class",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "StudyGroups", type: "string_v" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Class").push({ Name: "Dean", StudyGroups: ["A", "D"] });
	* base.store("Class").push({ Name: "Chang", StudyGroups: ["B", "D"] });
	* base.store("Class").push({ Name: "Magnitude", StudyGroups: ["B", "C"] });
	* base.store("Class").push({ Name: "Leonard", StudyGroups: ["A", "B"] });
	* // create a feature space containing the multinomial feature extractor
	* var ftr = new qm.FeatureSpace(base, { type: "multinomial", source: "Class", field: "StudyGroups", values: ["A", "B", "C", "D"] });
	* // create a feature matrix out of the records of the store by using the feature space
	* // returns a sparse matrix equal to
	* // 1  0  0  1
	* // 0  1  0  1
	* // 0  0  1  0
	* // 1  1  0  0
	* var matrix = ftr.extractMatrix(base.store("Class").allRecords);
	*/
 exports.FeatureSpace.prototype.extractMatrix = function (rs) { return Object.create(require('qminer').la.Matrix.prototype); };
/**
	* Gives the name of feature extractor at given position.
	* @param {number} idx - The index of the feature extractor in feature space (zero based).
	* @returns {String} The name of the feature extractor at position idx.
	* @example
	* // import qm module
	* var qm = require("qminer");
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "People",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Gendre", type: "string" },
	*            { name: "Age", type: "int" }
	*        ]
	*    }]
	* });
	* // create a feature space containing a categorical and numeric feature extractor
	* var ftr = new qm.FeatureSpace(base, [
	*    { type: "numeric", source: "People", field: "Age" },
	*    { type: "categorical", source: "People", field: "Gendre", values: ["Male", "Female"] }
	* ]);
	* // get the name of the feature extractor with index 1
	* var extractorName = ftr.getFeatureExtractor(1); // returns "Categorical[Gendre]"
	*/
 exports.FeatureSpace.prototype.getFeatureExtractor = function (idx) { return ''; };
/**
	* Gives the name of the feature at the given position.
	* @param {number} idx - The index of the feature in feature space (zero based).
	* @returns {String} The name of the feature at the position idx.
	* @example
	* // import qm module
	* var qm = require("qminer");
	* // create a base containing the store Class. Let the Name field be the primary field.
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Class",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "StudyGroups", type: "string_v" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Class").push({ Name: "Dean", StudyGroups: ["A", "D"] });
	* base.store("Class").push({ Name: "Chang", StudyGroups: ["B", "D"] });
	* base.store("Class").push({ Name: "Magnitude", StudyGroups: ["B", "C"] });
	* base.store("Class").push({ Name: "Leonard", StudyGroups: ["A", "B"] });
	* // create a feature space containing the multinomial feature extractor
	* var ftr = new qm.FeatureSpace(base, [
	* { type: "text", source: "Class", field: "Name" },
	* { type: "multinomial", source: "Class", field: "StudyGroups", values: ["A", "B", "C", "D"] }
	* ]);
	* // get the feature at position 2
	* var feature = ftr.getFeature(2); // returns "C", because the text extractor has no features at the moment
	* // update the feature space with the records of the store; see the method updateRecords
	* ftr.updateRecords(base.store("Class").allRecords);
	* // get the feature at position 2
	* var feature2 = ftr.getFeature(2); // returns "magnitude"
	*/
 exports.FeatureSpace.prototype.getFeature = function (idx) { return ''; };
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
	* // create a new base with one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Academics",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Age", type: "int" },
	*            { name: "Gendre", type: "string" },
	*            { name: "Skills", type: "string_v" }
	*        ]
	*    }]
	* });
	* // create a new feature space
	* var ftr = new qm.FeatureSpace(base, [
    *     { type: "numeric", source: "Academics", field: "Age" },
    *     { type: "categorical", source: "Academics", field: "Gendre", values: ["Male", "Female"] },
	*     { type: "multinomial", source: "Academics", field: "Skills", values: ["Mathematics", "Programming", "Philosophy", "Languages", "Politics", "Cooking"] }
	*     ]);
	* // create a new dense vector
	* var vec = new qm.la.Vector([40, 0, 1, 0, 1, 1, 1, 0, 0]);
	* // filter the elements from the second feature extractor
	* var vec2 = ftr.filter(vec, 1); // returns vector [0, 0, 1, 0, 0, 0, 0, 0, 0]
	* // filter the elements from the second feature extractor, without keeping the offset
	* var vec3 = ftr.filter(vec, 1, false); // returns vector [0, 1]
	* // create a new sparse vector
	* var spVec = new qm.la.SparseVector([[0, 40], [2, 1], [4, 1], [5, 1], [6, 1]]);
	* // filter the elements from the second feature extractor
	* var spVec2 = ftr.filter(spVec, 1); // returns sparse vector [[2, 1]]
	* // filter the elements from the second feature extractor, without keeping the offset
	* var spVec3 = ftr.filter(spVec, 1, false); // returns sparse vector [[1, 1]]
	*/
 exports.FeatureSpace.prototype.filter = function (vec, idx, keepOffset) { return (vec instanceof require('qminer').la.Vector) ? require('qminer').la.Vector : require('qminer').la.SparseVector; };
/**
	* Extracts string features from the record.
	* @param {module:qm.Record} rec
	* @returns {Array.<string>} An array containing the strings gained by the extractor.
	* @ignore
	*/
 exports.FeatureSpace.prototype.extractStrings = function (rec) {return ['']; }; 

    exports.preprocessing = new function() {
        this.binarize = function (y, labelId) {
            var target = new la.Vector();
            for (var i = 0; i < y.length; i++) {
                target.push(y[i] === labelId ? 1 : -1);
            }
            return target;
        };

        this.applyModel = function (model, X) {
            var target = new la.Vector();
            for (var i = 0; i < X.cols; i++) {
                target.push(model.decisionFunction(X[i]));
            }
            return target;
        }
    };

    /**
    * SVM model.
    * @typedef {Object} svmModel
    * @property  {module:la.Vector} [svmModel.weigths] - SVM normal vector.
    */
    /**
	* Get SVC model.
	* @returns {module:analytics~svmModel} The current SVM model.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a SVC model
    * var SVC = new analytics.SVC();
    * // get the properties of the model
    * var model = SVC.getModel(); // returns { weight: new require('qminer').la.Vector(); }
	*/
    exports.SVC.prototype.getModel = function() { return { weights: this.weights }; }
    /**
	* Get SVR model
	* @returns {module:analytics~svmModel} Get current SVM model
	*/
    exports.SVR.prototype.getModel = function() { return { weights: this.weights }; }

    // Ridge Regression
    /**
    * Gets RidgeReg model.
    * @returns {module:analytics~ridgeRegModel} Get current RidgeReg model.
    */
    exports.RidgeReg.prototype.getModel = function () { return { weights: this.weights }; }


    // var model = new OneVsAll({
    //     model : analytics.SVC,
    //     modelParam: { c: 10, j: 10, maxTime: 123 },
    //     cats : 123
    // });
    //
    // var X = featureSpace.extractSparseMatrix(recordSet);
    // var y = store.getCol("label");
    // model.fit(X, y);
    //
    // model.predict(featureSpace.extractSparseVector(record));

    /**
    * @typedef {Object} oneVsAllParam
    * The parameter given to the OneVsAll object. A Json object containing the parameter keys with values.
    * @param {function} [model] - Constructor for binary model to be
    * used internaly. Constructor should expect only one parameter.
    * @param {Object} [modelParam] - Parameter for oneVsAllParam.model constructor.
    * @param {number} [categories] - Number of categories.
    * @param {boolean} [verbose = false] - If false, the console output is supressed.
    */

    /**
    * @classdesc One vs. all model for multiclass prediction. Builds binary model
    * for each category and predicts the one with the highest score. Binary model is
    * provided as part of the constructor.
    * @class
    * @param {module:analytics~oneVsAllParam} [oneVsAllParam] - Constructor parameters.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a new OneVsAll object with the model analytics.SVC
    * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
    */
    exports.OneVsAll = function (oneVsAllParam) {
        // remember parameters
        var model = oneVsAllParam.model;
        var modelParam = oneVsAllParam.modelParam;
        var cats = oneVsAllParam.cats;
        var verbose = oneVsAllParam.verbose == undefined ? false : oneVsAllParam.verbose;
        // trained models
        var models = [ ];

        /**
        * Gets the parameters.
        * @returns {Object} Json object containing the parameters.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // create a new OneVsAll object with the model analytics.SVC
        * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
        * // get the parameters
        * // returns the JSon object
        * // { model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2, models: [] }
        * var params = onevsall.getParams();
        */
        this.getParams = function () {
            return { model: model, modelParam: modelParam, cats: cats, models: models }
        };

        /**
        * Sets the parameters.
        * @returns {module:analytics.OneVsAll} Self. The parameters are changed.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // create a new OneVsAll object with the model analytics.SVC
        * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
        * // set the parameters
        * var params = onevsall.setParams({ model: analytics.SVR, modelParam: { c: 12, maxTime: 10000}, cats: 3, verbose: true });
        */
        this.setParams = function (oneVsAllParam) {
            model = oneVsAllParam.model == undefined ? model : oneVsAllParam.model;
            modelParam = oneVsAllParam.modelParam == undefined ? modelParam : oneVsAllParam.modelParam;
            cats = oneVsAllParam.cats == undefined ? cats : oneVsAllParam.cats;
            verbose = oneVsAllParam.verbose == undefined ? verbose : oneVsAllParam.verbose;
        }

        /**
         * Apply all models to the given vector and returns a vector of scores, one for each category.
         * Semantic of scores depand on the provided binary model.
         * @param {module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix} X -
         * Input feature vector or matrix with feature vectors as columns.
         * @returns {module:la.Vector | module:la.Matrix} The score and label of the input:
         * <br>1. {@link module:la.Vector} of scores, if X is of type {@link module:la.Vector} or {@link module:la.SparseVector}.
         * <br>2. {@link module:la.Matrix} with columns corresponding to instances, and rows corresponding to labels, if X is of type {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
         * @example
         * // import modules
         * var analytics = require('qminer').analytics;
         * var la = require('qminer').la;
         * // create a new OneVsAll object with the model analytics.SVC
         * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
         * // create the data (matrix and vector) used to fit the model
         * var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
         * var vector = new la.Vector([0, 0, 1, 1]);
         * // fit the model
         * onevsall.fit(matrix, vector);
         * // create the vector for the decisionFunction
         * var test = new la.Vector([1, 2]);
         * // give the vector to the decision function
         * var prediction = onevsall.predict(test); // returns the vector of scores 
         */
        this.decisionFunction = function(X) {
            // check what is our input
            if (X instanceof la.Vector || X instanceof la.SparseVector) {
                // evaluate all models
                var scores = new la.Vector();
                for (var cat = 0; cat < cats; cat++) {
                    scores.push(models[cat].decisionFunction(X));
                }
                return scores;
            } else if (X instanceof la.Matrix || X instanceof la.SparseMatrix) {
                // create matrix where cols are instances and rows are scores for categories
                var scores = new la.Matrix({rows: cats, cols: X.cols});
                for (var i = 0; i < X.cols; i++) {
                    var x_i = X.getCol(i);
                    for (var cat = 0; cat < cats; cat++) {
                        scores.put(cat, i, models[cat].decisionFunction(x_i));
                    }
                }
                return scores;
            } else {
                throw "analytics.OneVsAll.decisionFunction: Input data of unsupported type!";
            }
        }

        /**
         * Apply all models to the given vector and returns category with the highest score.
         * @param {module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix} X -
         * Input feature vector or matrix with feature vectors as columns.
         * @returns {number | module:la.IntVector} Returns:
         * <br>1. number of the category with the higher score, if X is {@link module:la.Vector} or {@link module:la.SparseVector}.
         * <br>2. {@link module:la.IntVector} of categories with the higher score for each column of X, if X is {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
         * @example
         * // import modules
         * var analytics = require('qminer').analytics;
         * var la = require('qminer').la;
         * // create a new OneVsAll object with the model analytics.SVC
         * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
         * // create the data (matrix and vector) used to fit the model
         * var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
         * var vector = new la.Vector([0, 0, 1, 1]);
         * // fit the model
         * onevsall.fit(matrix, vector);
         * // create the vector for the prediction
         * var test = new la.Vector([1, 2]);
         * // get the prediction of the vector
         * var prediction = onevsall.predict(test); // returns 0
         */
        this.predict = function(X) {
            // evaluate all models
            var scores = this.decisionFunction(X);
            // select maximal one
            if (scores instanceof la.Vector) {
                return scores.getMaxIdx();
            } else if (scores instanceof la.Matrix) {
                var predictions = new la.IntVector();
                for (var i = 0; i < scores.cols; i++) {
                    predictions.push(scores.getCol(i).getMaxIdx());
                }
                return predictions;
            } else {
                throw "analytics.OneVsAll.predict: decisionFunction returns unsupported type!";
            }
        }

        // X = feature matrix
        // y = target label from 0..cats
        /**
         * Apply all models to the given vector and returns category with the highest score.
         * @param {module:la.Matrix | module:la.SparseMatrix} X - training instance feature vectors.
         * @param {module:la.Vector} y - target category for each training instance. Categories must
         * be integer numbers between 0 and oneVsAllParam.categories - 1.
         * @returns {module:analytics.OneVsAll} Self. The models are now fitted.
         * @example
         * // import modules
         * var analytics = require('qminer').analytics;
         * var la = require('qminer').la;
         * // create a new OneVsAll object with the model analytics.SVC
         * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
         * // create the data (matrix and vector) used to fit the model
         * var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
         * var vector = new la.Vector([0, 0, 1, 1]);
         * // fit the model
         * onevsall.fit(matrix, vector);
         */        
        this.fit = function(X, y) {
            models = [ ];
            // make model for each category
            for (var cat = 0; cat < cats; cat++) {
                if (verbose) {
                    console.log("Fitting label", (cat + 1), "/", cats);
                };
                // prepare targert vector for current category
                var target = exports.preprocessing.binarize(y, cat);
                // get the model
                var catModel = new model(modelParam);
                models.push(catModel.fit(X, target));
            }
            if (verbose) {
                console.log("Done!");
            };
            return this;
        }
    };

    exports.ThresholdModel = function(params) {
        // what do we optimize
        this.target = params.target;
        if (this.target === "recall" || this.target === "precision") {
            this.level = params.level;
        }
        // threshold model
        this.model = null;

        // apply all models to the given vector and return distance to the class boundary
        // x = dense vector with prediction score for each class
        // result = traslated predictions based on thresholds
        this.decisionFunction = function(x) {
            if (x instanceof Number) {
                // just transate based on the model's threshold
                return x - this.model;
            } else if (x instanceof la.Vector) {
                // each element is a new instance
                var scores = new la.Vector();
                for (var i = 0; i < x.length; i++) {
                    scores.push(x[i] - this.model);
                }
                return scores;
            } else {
                throw "analytics.ThresholdModel.decisionFunction: Input data of unsupported type!";
            }
        }

        // return the most likely category
        // x = dense vector with prediction score for each class
        // result = array of positive label ids
        this.predict = function(x) {
            // evaluate all models
            var scores = this.decisionFunction(x)
            // check what we get
            if (scores instanceof la.Vector) {
                return res = new la.Vector();
                for (var i = 0; i < scores.length; i++) {
                    res.push(scores[i] > 0 ? 1 : -1);
                }
                return res;
            } else {
                return scores > 0 ? 1 : -1;
            }
        }

        // X = vector of predictions for each instance (output of decision_funcition)
        // y = target labels (1 or -1)
        this.fit = function(X, y) {
            if (this.target === "f1") {
                // find threshold that maximizes F1 measure
                this.model = exports.metrics.bestF1Threshold(y, X);
            } else if (this.target === "recall") {
                // find threshold that results in desired recall
                this.model = exports.metrics.desiredRecallThreshold(y, X, this.level);
            } else if (this.target === "precision") {
                // find threshold that results in desired precision
                this.model = exports.metrics.desiredPrecisionThreshold(y, X, this.level);
            } else {
                throw "Unknown threshold model target: " + this.target;
            }
        }
    }

    exports.metrics = new function() {
        // For evaluating provided categories (precision, recall, F1).
        this.ClassificationScore  = function (yTrue, yPred) {
            this.scores = {
                count: 0, predictionCount: 0,
                TP: 0, TN: 0, FP: 0, FN: 0,
                all: function () { return this.TP + this.FP + this.TN + this.FN; },
                precision: function () { return (this.FP == 0) ? 1 : this.TP / (this.TP + this.FP); },
                recall: function () { return this.TP / (this.TP + this.FN); },
                f1: function () { return 2 * this.precision() * this.recall() / (this.precision() + this.recall()); },
                accuracy: function () { return (this.TP + this.TN) / this.all(); }
            };

            // adds prediction to the current statistics. `correct` corresponds to the correct
            // label(s), `predicted` correspond to predicted lable(s). Labels can be either integers
            // or integer array (when there are zero or more then one lables).
            this.push = function (correct, predicted) {
                var catCorrect = (correct > 0);
                var catPredicted = (predicted > 0);
                // update counts for correct categories
                if (catCorrect) { this.scores.count++; }
                // update counts for how many times category was predicted
                if (catPredicted) { this.scores.predictionCount++; }
                // update true/false positive/negative count
                if (catCorrect && catPredicted) {
                    // both predicted and correct say true
                    this.scores.TP++;
                } else if (catCorrect) {
                    // this was only correct but not predicted
                    this.scores.FN++;
                } else if (catPredicted) {
                    // this was only predicted but not correct
                    this.scores.FP++;
                } else {
                    // both predicted and correct say false
                    this.scores.TN++;
                }
            };

            // initialize if we are passed the data
            if (arguments.length >= 2) {
                for (var i = 0; i < yTrue.length; i++) {
                    this.push(yTrue[i], yPred[i]);
                }
            }
        };

        this.accuracyScore = function (yTrue, yPred) {
            return new this.ClassificationScore (yTrue, yPred).scores.accuracy();
        };

        this.precisionScore = function (yTrue, yPred) {
            return new this.ClassificationScore (yTrue, yPred).scores.precision();
        };

        this.recallScore = function (yTrue, yPred) {
            return new this.ClassificationScore (yTrue, yPred).scores.recall();
        };

        this.f1Score = function (yTrue, yPred) {
            return new this.ClassificationScore (yTrue, yPred).scores.accuracy();
        };

        // used for computing ROC curve and other related measures such as AUC;
        this.PredictionCurve = function (yTrue, yPred) {
            // count of all examples
            this.length = 0;
            // count of all the positive and negative examples
    		this.allPositives = 0;
    		this.allNegatives = 0;
    		// store of predictions and ground truths
    		this.grounds = new la.Vector();
    		this.predictions = new la.Vector();

            // add new measurement with ground score (1 or -1) and predicted value
            this.push = function (ground, predict) {
                // remember the scores
                this.grounds.push(ground)
                this.predictions.push(predict);
                // update counts
                this.length++;
                if (ground > 0) {
                    this.allPositives++;
                } else {
                    this.allNegatives++;
                }
            };

            // initialize if we are given data
            if (arguments.length >= 2) {
                for (var i = 0; i < yTrue.length; i++) {
                    this.push(yTrue[i], yPred[i]);
                }
            }

            // get ROC parametrization sampled on `sample' points
    		this.roc = function (sample) {
    			// default sample size is 10
    			sample = sample || 10;
    			// sort according to predictions
    			var perm = this.predictions.sortPerm(false);
    			// maintaining the results as we go along
    			var TP = 0, FP = 0, ROC = [[0, 0]];
    			// for figuring out when to dump a new ROC sample
    			var next = Math.floor(perm.perm.length / sample);
    			// go over the sorted results
    			for (var i = 0; i < perm.perm.length; i++) {
    				// get the ground
    				var ground = this.grounds[perm.perm[i]];
    				// update TP/FP counts according to the ground
    				if (ground > 0) { TP++ } else { FP++; }
    				// see if time to do next save
    				next = next - 1;
    				if (next <= 0) {
    					// add new datapoint to the curve
    					ROC.push([FP/this.allNegatives, TP/this.allPositives]);
    					// setup next timer
    					next = Math.floor(perm.perm.length / sample);
    				}
    			}
    			// add the last point
    			ROC.push([1,1]);
    			// return ROC
    			return ROC;
    		}

            // get AUC of the current curve
    		this.auc = function (sample) {
    			// default sample size is 10
    			sample = sample || 10;
    	        // get the curve
    	        var curve = this.curve(sample);
    	        // compute the area
    	        var result = 0;
    	        for (var i = 1; i < curve.length; i++) {
    	            // get edge points
    	            var left = curve[i-1];
    	            var right = curve[i];
    	            // first the rectangle bellow
    	            result = result + (right[0] - left[0]) * left[1];
    	            // an then the triangle above
    	            result = result + (right[0] - left[0]) * (right[1] - left[1]) / 2;
    	        }
    	        return result;
    	    }

            this.evalPrecisionRecall = function (callback) {
                // sort according to predictions
                var perm = this.predictions.sortPerm(false);
                // maintaining the results as we go along
                var TP = 0, FP = 0, TN = this.allNegatives, FN = this.allPositives;
                // go over the sorted results
                for (var i = 0; i < perm.perm.length; i++) {
                    // get the ground
                    var ground = this.grounds[perm.perm[i]];
                    // update TP/FP counts according to the ground
                    if (ground > 0) { TP++; FN--; } else { FP++; TN--; }
                    // do the update
                    if ((TP + FP) > 0 && (TP + FN) > 0 && TP > 0) {
                        // compute current precision and recall
                        var precision = TP / (TP + FP);
                        var recall = TP / (TP + FN);
                        // see if we need to update current bep
                        callback.update(ground, perm.vec[i], precision, recall);
                    }
                }
                return callback.finish();
            }

            // get precision recall curve sampled on `sample' points
            this.precisionRecallCurve = function (sample) {
                return this.evalPrecisionRecall(new function (sample, length) {
                    // default sample size is 10
                    this.sample = sample || 10;
                    // curve
                    this.curve = [[0, 1]];
                    // for figuring out when to dump a new ROC sample
                    this.next = Math.floor(length / (this.sample));
                    this.counter = this.next;
                    console.log(length, this.sample, this.next);
                    // keep last value
                    this.precision = 0; this.recall = 0;
                    // handlers
                    this.update = function (yTrue, yPred, precision, recall) {
                        this.counter = this.counter - 1;
                        if (this.counter <= 0) {
                            // add to the curve
                            this.curve.push([recall, precision]);
                            // setup next timer
                            this.counter = this.next;
                        }
                        // always remember last value
                        this.precision = precision; this.recall = recall;
                    }
                    this.finish = function () {
                        // add the last point
                        this.curve.push([this.recall, this.precision]);
                        return this.curve;
                    }
                }(sample, this.length));
            };

            // get break-even point, the value where precision and recall intersect
            this.breakEvenPoint = function () {
                return this.evalPrecisionRecall(new function () {
                    this.minDiff = 1.0; this.bep = -1.0;
                    this.update = function (yTrue, yPred, precision, recall) {
                        var diff = Math.abs(precision - recall);
                        if (diff < minDiff) { minDiff = diff; bep = (precision + recall) / 2; }
                    }
                    this.finish = function () { return this.bep; }
                }());
            }

            // gets threshold for prediction score, which results in the highest F1
            this.bestF1 = function () {
                return this.evalPrecisionRecall(new function () {
                    this.maxF1 = 0.0; this.threshold = 0.0;
                    this.update = function (yTrue, yPred, precision, recall) {
                        var f1 = 2 * precision * recall / (precision + recall);
                        if (f1 > this.maxF1) {
                            this.maxF1 = f1;
                            this.threshold = yPred;
                        }
                    }
                    this.finish = function () { return this.threshold; }
                }());
            }

            // gets threshold for prediction score, nearest to specified recall
            this.desiredRecall = function (desiredRecall) {
                return this.evalPrecisionRecall(new function () {
                    this.recallDiff = 1.0; this.threshold = 0.0;
                    this.update = function (yTrue, yPred, precision, recall) {
                        var diff = Math.abs(desiredRecall - recall);
                        if (diff < this.recallDiff) {
                            this.recallDiff = diff;
                            this.threshold = yPred;
                        }
                    }
                    this.finish = function () { return this.threshold; }
                }());
            }

            // gets threshold for prediction score, nearest to specified recall
            this.desiredPrecision = function (desiredPrecision) {
                return this.evalPrecisionRecall(new function () {
                    this.precisionDiff = 1.0; this.threshold = 0.0;
                    this.update = function (yTrue, yPred, precision, recall) {
                        var diff = Math.abs(desiredPrecision - precision);
                        if (diff < this.precisionDiff) {
                            this.precisionDiff = diff;
                            this.threshold = yPred;
                        }
                    }
                    this.finish = function () { return this.threshold; }
                }());
            }
        };

        this.rocCurve = function (yTrue, yPred, sample) {
            return new this.PredictionCurve(yTrue, yPred).roc(sample);
        };

        this.rocAucScore = function (yTrue, yPred, sample) {
            return new this.PredictionCurve(yTrue, yPred).roc(sample);
        };

        this.precisionRecallCurve = function (yTrue, yPred, sample) {
            return new this.PredictionCurve(yTrue, yPred).precisionRecallCurve(sample);
        };

        this.breakEventPointScore = function (yTrue, yPred) {
            return new this.PredictionCurve(yTrue, yPred).breakEvenPoint();
        };

        this.bestF1Threshold = function (yTrue, yPred) {
            return new this.PredictionCurve(yTrue, yPred).bestF1();
        };

        this.desiredRecallThreshold = function (yTrue, yPred, desiredRecall) {
            return new this.PredictionCurve(yTrue, yPred).desiredRecall(desiredRecall);
        };

        this.desiredPrecisionThreshold = function (yTrue, yPred, desiredPrecision) {
            return new this.PredictionCurve(yTrue, yPred).desiredPrecision(desiredPrecision);
        };
    };

    /**
    * @typedef {Object} detectorParam
    * A Json object used for the creation of the {@link module:analytics.NearestNeighborAD}.
    * @param {number} [rate= 0.05] - The expected fracton of emmited anomalies (0.05 -> 5% of cases will be classified as anomalies).
    */

    /**
    * @classdesc Anomaly detector that checks if the test point is too far from
    * the nearest known point.
    * @class
    * @param {module:analytics~detectorParam} [detectorParam] - Constructor parameters.
    * @example
    * // import modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * // create a new NearestNeighborAD object
    * var neighbor = new analytics.NearestNeighborAD({ rate: 0.1 });
    * // create a matrix 
    * var matrix = new la.Matrix([[1, -2, 0], [2, 3, 1]]);
    * // fit the model with the matrix
    * neighbor.fit(matrix);
    * // create a new vector
    * var vector = new la.Vector([4, 0]);
    * // predict if the vector is an anomaly or not
    * var prediction = neighbor.predict(vector);
    */
    exports.NearestNeighborAD = function (detectorParam) {
        // Parameters
        var param = detectorParam == undefined ? {} : detectorParam;
        param.rate = param.rate == undefined ? 0.05 : param.rate;
        // model param
        var rate = param.rate == undefined ? 0.05 : param.rate;
        assert(rate > 0 && rate <= 1.0, 'rate parameter not in range (0,1]');
        // default model
        var thresh = 0;

        /**
        * Returns the model.
        * @returns {Object} Json object whose keys are: 
        * <br> 1. rate - The expected fraction of emmited anomalies.
        * <br> 2. thresh - Maximal squared distance to the nearest neighbor that is not anomalous.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // create a new NearestNeighborAD object
        * var neighbor = new analytics.NearestNeighborAD({ rate: 0.1 });
        * // get the model of the object
        * // returns a json object { rate: 0.1, thresh: 0 }
        * var model = neighbor.getModel();
        */
        this.getModel = function () { return { rate: rate, thresh: thresh }; }

        /**
        * Sets parameters.
        * @param {module:analytics~detectorParam} newParams - The Json object containing the new rate value.
        * @returns {module:analytics.NearestNeighborAD} Self. The parameters are updated with newParams.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // create a new NearestNeighborAD object
        * var neighbor = new analytics.NearestNeighborAD();
        * // set it's parameters to rate: 0.1
        * neighbor.setParams({ rate: 0.1 });
        */
        this.setParams = function (newParams) {
            param = newParams;
            assert(param.rate > 0 && param.rate <= 1.0, 'rate parameter not in range (0,1]');
            // Parameters
            rate = param.rate == undefined ? param : newParams.rate;
            
        }

        /**
        * Returns parameters.
        * @returns {module:analytics~detectorParam} The Json object containing the rate value. 
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // create a new NearestNeighborAD object
        * var neighbor = new analytics.NearestNeighborAD();
        * // get the parameters of the object
        * // returns a json object { rate: 0.05 }
        * var params = neighbor.getParams();
        */
        this.getParams = function () {
            return param;
        }

        /**
        * Gets the 100*(1-rate) percentile.
        * @param {module:la.Vector} vector - Vector of values.
        * @returns {number} The 100*(1-rate) percentile.
        */
        function getThreshold(vector, rate) {
            var sorted = vector.sortPerm().vec;
            var idx = Math.floor((1 - rate) * sorted.length);
            return sorted[idx];
        }
        //var neighborDistances = undefined;

        /**
        * Analyzes the nearest neighbor distances and computes the detector threshold based on the rate parameter.
        * @param {module:la.Matrix} A - Matrix whose columns correspond to known examples. Gets saved as it is part of
        * the model.
        * @returns {module:analytics.NearestNeighborAD} Self. The model is set by the matrix A.
        * @example
        * // import modules
        * var analytics = require('qminer').analytics;
        * var la = require('qminer').la;
        * // create a new NearestNeighborAD object
        * var neighbor = new analytics.NearestNeighborAD();
        * // create a new matrix
        * var matrix = new la.Matrix([[1, -2, 0], [2, 3, 1]]);
        * // fit the model with the matrix
        * neighbor.fit(matrix);
        */
        this.fit = function (A) {
            this.X = A;
            // distances
            var D = la.pdist2(A, A);
            // add big numbers on the diagonal (exclude the point itself from the nearest point calcualtion)
            var E = D.plus(D.multiply(la.ones(D.rows)).diag()).multiply(-1);
            var neighborDistances = new la.Vector({ vals: D.rows });
            for (var i = 0; i < neighborDistances.length; i++) {
                // nearest neighbour squared distance
                neighborDistances[i] = D.at(i, E.rowMaxIdx(i));
            }
            thresh = getThreshold(neighborDistances, rate);
        }

        /**
        * Compares the point to the known points and returns 1 if it's too far away (based on the precomputed threshold).
        * @param {module:la.Vector} x - Test vector.
        * @returns {number} Returns 1.0 if the vector x is an anomaly and 0.0 otherwise.
        * @example
        * // import modules
        * var analytics = require('qminer').analytics;
        * var la = require('qminer').la;
        * // create a new NearestNeighborAD object
        * var neighbor = new analytics.NearestNeighborAD();
        * // create a new matrix
        * var matrix = new la.Matrix([[1, -2, 0], [2, 3, 1]]);
        * // fit the model with the matrix
        * neighbor.fit(matrix);
        * // create a new vector 
        * var vector = new la.Vector([4, 0]);
        * // check if the vector is an anomaly
        * var prediction = neighbor.predict(vector); // returns 1
        */
        this.predict = function (x) {
            // compute squared dist and compare to thresh
            var d = la.pdist2(this.X, x.toMat()).getCol(0);
            var idx = d.multiply(-1).getMaxIdx();
            var p = d[idx];
            //console.log(p)
            return p > thresh ? 1 : 0;
        }

        /**
        * Adds a new point (or points) to the known points and recomputes the threshold.
        * @param {(module:la.Vector | module:la.Matrix)} x - Test example (vector input) or column examples (matrix input).
        * @returns {module:analytics.NearestNeighborAD} Self. The model is updated.
         * @example
        * // import modules
        * var analytics = require('qminer').analytics;
        * var la = require('qminer').la;
        * // create a new NearestNeighborAD object
        * var neighbor = new analytics.NearestNeighborAD();
        * // create a new matrix
        * var matrix = new la.Matrix([[1, -2, 0], [2, 3, 1]]);
        * // fit the model with the matrix
        * neighbor.fit(matrix);
        * // create a new vector
        * var vector = new la.Vector([2, 5]);
        * // update the model with the vector
        * neighbor.update(vector);
        */
        this.update = function (x) {
            // append x to known examples and retrain (slow version)
            // speedup 1: don't reallocate X every time (fixed window, circular buffer)
            // speedup 2: don't recompute distances d(X,X), just d(X, y), get the minimum
            // and append to neighborDistances
            this.fit(la.cat([[this.X, x.toMat()]]));
            //console.log('new threshold ' + this.thresh);
        }
    }

    /**
    * @classdesc Principal components analysis
    * @class
    */
    exports.PCA = function (param) {
        param = param == undefined ? {} : param;

        // Fit params
        var iter = param.iter == undefined ? 100 : param.iter;
        var k = param.k; // can be undefined

        /**
        * Returns the model
        * @returns {Object} The model object whose keys are: P (eigenvectors), lambda (eigenvalues) and mu (mean)
        */
        this.getModel = function () {
            return { P: this.P, mu: this.mu, lambda: this.lambda };
        }

        /**
        * Sets parameters
        * @param {p} Object whose keys are: k (number of eigenvectors) and iter (maximum iterations)
        */
        this.setParams = function (p) {
            param = p;

            iter = param.iter == undefined ? iter : param.iter;
            k = param.k == undefined ? k : param.iter; 
        }

        /**
        * Gets parameters
        * @returns Object whose keys are: k (number of eigenvectors) and iter (maximum iterations)
        */
        this.getParams = function () {
            return param;
        }

        /**
        * Finds the eigenvectors of the variance matrix.
        * @param {module:la.Matrix} A - Matrix whose columns correspond to examples.
        */
        this.fit = function (A) {
            var rows = A.rows;
            var cols = A.cols;

            k = k == undefined ? rows : k;
            //iter = iter == undefined ? -1 : iter;

            var mu = stat.mean(A, 2);
            // cov(A) = 1/(n-1) A A' - mu mu'

            // center data (same as matlab)
            var cA = A.minus(la.ones(cols).outer(mu));
            var C = cA.multiply(cA.transpose()).multiply(1 / (cols - 1));
            // alternative computation:
            //var C = (A.multiply(A.transpose()).multiply(1 / (cols - 1))).minus(mu.outer(mu));
            var res = la.svd(C, k, { iter: iter });

            this.P = res.U;
            this.lambda = res.s;
            this.mu = mu;
        }

        /**
        * Projects the example(s) and expresses them as coefficients in the eigenvector basis this.P.
        * Recovering the data in the original space: (this.P).multiply(p), where p's rows are the coefficients
        * in the eigenvector basis.
        * @param {(module:la.Vector | module:la.Matrix)} x - Test vector or matrix with column examples
        * @returns {(module:la.Vector | module:la.Matrix)} Returns projected vector or matrix
        */
        this.transform = function (x) {
            if (x.constructor.name == 'Matrix') {
                // P * (x - mu*ones(1, size(x,2))
                return this.P.multiplyT(x.minus(this.mu.outer(la.ones(x.cols))));

            } else if (x.constructor.name == 'Vector') {
                // P * (x - mu)
                return this.P.multiplyT(x.minus(this.mu));
            }
        }

        /**
        * Reconstructs the vector in the original space, reverses centering
        * @param {(module:la.Vector | module:la.Matrix)} x - Test vector or matrix with column examples, in the PCA space
        * @returns {(module:la.Vector | module:la.Matrix)} Returns the reconstruction
        */
        this.inverseTransform = function (x) {
            if (x.constructor.name == 'Matrix') {
                // P x + mu*ones(1, size(x,2)
                return (this.P.multiply(x)).plus(this.mu.outer(la.ones(x.cols)));
            } else if (x.constructor.name == 'Vector') {
                // P x + mu
                return (this.P.multiply(x)).plus(this.mu);
            }
        }
    }

    /**
    * @classdesc KMeans clustering
    * @class
    * @property {number} iter - The maximum number of iterations.
    * @property {number} k - The number of centroids.
    * @property {boolean} verbose - If false, the console output is supressed.
    * @example
    * // import analytics and la modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * // create a KMeans object
    * var KMeans = new analytics.KMeans();
    * // create the matrix to be fitted
    * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
    * // create the model 
    * KMeans.fit(X);
    */
    exports.KMeans = function (param) {
        param = param == undefined ? {} : param;

        // Fit params
        var iter = param.iter == undefined ? 100 : param.iter;
        var k = param.k == undefined ? 2 : param.k;
        var verbose = param.verbose == undefined ? false : param.verbose;
        var fitIdx = param.fitIdx == undefined ? undefined : param.fitIdx;

        // Model
        var C = undefined;
        var idxv = undefined;
        var norC2 = undefined;

        /**
        * Permutes centroid with given mapping.
        @param {object} mapping - object that contains the mappping. E.g. mapping[4]=2 means "map cluster 4 into cluster 2"
        */
        this.permuteCentroids = function (mapping) {
            var cl_count = C.cols;
            var perm_matrix = la.zeros(cl_count, cl_count);
            for (var i = 0; i < cl_count; i++) {
                perm_matrix.put(i, mapping[i], 1);
            }
            var C_new = C.multiply(perm_matrix);
            var idxv_new = new la.Vector(idxv);
            for (var i = 0; i < idxv_new.length; i++) {
                idxv_new[i] = mapping[idxv[i]]
            }
            C = C_new;
            norC2 = la.square(C.colNorms());
            idxv = idxv_new;
        }
        /**
        * Returns the model
        * @returns {Object} The model object whose keys are: C (centroids), norC2 (centroid norms squared) and idxv (cluster ids of the training data)
        * Returns the model.
        * @returns {Object} The model object whose keys are: C (centroids) and idxv (cluster ids of the training data).
        * @example
        * // import modules
        * var analytics = require('qminer').analytics;
        * var la = require('qminer').la;
        * // create the KMeans object
        * var KMeans = new analytics.KMeans({ iter: 1000 });
        * // create a matrix to be fitted
        * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
        * // create the model
        * KMeans.fit(X);
        * // get the model
        * var model = KMeans.getModel();
        */
        this.getModel = function () {
            return { C: C, idxv: idxv };
        }

        /**
        * Sets the parameters.
        * @param {Object} p - Object whose keys are: k (number of centroids), iter (maximum iterations) and verbose (if false, console output is supressed).
        * @returns {module:analytics.KMeans} Self.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // create a new KMeans object
        * var KMeans = new analytics.KMeans();
        * // change the parameters of the KMeans object
        * KMeans.setParams({ iter: 1000, k: 5 });
        */
        this.setParams = function (p) {
            param = p;

            iter = param.iter == undefined ? iter : param.iter;
            k = param.k == undefined ? k : param.k;
            verbose = param.verbose == undefined ? verbose : param.verbose;
            fitIdx = param.fitIdx == undefined ? fitIdx : param.fitIdx;
        }

        /**
        * Returns the parameters.
        * @returns Object whose keys are: k (number of centroids), iter (maximum iterations) and verbose (if false, console output is supressed).
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // create a new KMeans object
        * var KMeans = new analytics.KMeans({ iter: 1000, k: 5 });
        * // get the parameters
        * var json = KMeans.getParams();
        */
        this.getParams = function () {
            return param;
        }

        /**
        * Computes the centroids
        * @param {(module:la.Matrix | module:la.SparseMatrix)} A - Matrix whose columns correspond to examples.
        * @returns {module:analytics.KMeans} Self. It stores the info about the new model.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // create a new KMeans object
        * var KMeans = new analytics.KMeans({ iter: 1000, k: 3 });
        * // create a matrix to be fitted
        * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
        * // create the model with the matrix X
        * KMeans.fit(X);
        */
        this.fit = function (X) {
            // select random k columns of X, returns a dense C++ matrix
            var selectCols = function (X, k) {
                var idx;
                if (fitIdx == undefined) {
                    idx = la.randi(X.cols, k);
                } else {
                    assert(fitIdx.length == k, "Error: fitIdx is not of length k!");
                    assert(Math.max.apply(Math, fitIdx) < X.cols, "Error: fitIdx contains index greater than number of columns in matrix. Index out of range!");
                    idx = fitIdx;
                }
                var idxMat = new la.SparseMatrix({ cols: 0, rows: X.cols });
                for (var i = 0; i < idx.length; i++) {
                    var spVec = new la.SparseVector([[idx[i], 1.0]], X.cols);
                    idxMat.push(spVec);
                }
                var C = X.multiply(idxMat);
                var result = {};
                result.C = C;
                result.idx = idx;
                return result;
            };

            // modified k-means algorithm that avoids empty centroids
            // A Modified k-means Algorithm to Avoid Empty Clusters, Malay K. Pakhira
            // http://www.academypublisher.com/ijrte/vol01/no01/ijrte0101220226.pdf
            var getCentroids = function (X, idx, oldC) {
                // select random k columns of X, returns a dense matrix
                // 1. construct a sparse matrix (coordinate representation) that encodes the closest centroids
                var idxvec = new la.IntVector(idx);
                var rangeV = la.rangeVec(0, X.cols - 1);
                var ones_cols = la.ones(X.cols);
                var idxMat = new la.SparseMatrix(idxvec, rangeV, ones_cols, X.cols);
                idxMat = idxMat.transpose();
                var ones_n = la.ones(X.cols);
                // 2. compute the number of points that belong to each centroid, invert
                var colSum = idxMat.multiplyT(ones_n);
                for (var i = 0; i < colSum.length; i++) {
                    var val = 1.0 / (1.0 + colSum.at(i)); // modification
                    colSum.put(i, val);
                }
                // 3. compute the centroids
                //var w = new qm_util.clsStopwatch();
                //w.tic();
                var sD = colSum.spDiag();
                var C = oldC;
                if (idxMat.cols == oldC.cols)
                    C = ((X.multiply(idxMat)).plus(oldC)).multiply(sD); // modification
                return C;
            };


            // X: column examples
            // k: number of centroids
            // iter: number of iterations
            assert(k <= X.cols, "k <= X.cols");
            var w = new qm_util.clsStopwatch();
            var norX2 = la.square(X.colNorms());
            var initialCentroids = selectCols(X, k);
            C = initialCentroids.C;
            var idxvOld = initialCentroids.idx;
            //printArray(idxvOld); // DEBUG
            var ones_n = la.ones(X.cols).multiply(0.5);
            var ones_k = la.ones(k).multiply(0.5);
            w.tic();
            for (var i = 0; i < iter; i++) {
                //console.say("iter: " + i);
                norC2 = la.square(C.colNorms());
                //D =  full(C'* X) - norC2' * (0.5* ones(1, n)) - (0.5 * ones(k,1) )* norX2';
                var D = C.multiplyT(X).minus(norC2.outer(ones_n)).minus(ones_k.outer(norX2));
                idxv = la.findMaxIdx(D);

                if (verbose) {
                    var energy = 0.0;
                    for (var j = 0; j < X.cols; j++) {
                        if (D.at(idxv[j], j) < 0) {
                            energy += Math.sqrt(-2 * D.at(idxv[j], j));
                        }
                    }
                    console.log("energy: " + 1.0 / X.cols * energy);
                }
                if (qm_util.arraysIdentical(idxv, idxvOld)) {
                    if (verbose) {
                        console.say("converged at iter: " + i); //DEBUG
                    }
                    break;
                }
                idxvOld = idxv.slice();
                C = getCentroids(X, idxv, C); //drag
            }
            if (verbose) {
                w.toc("end");
            }
            norC2 = la.square(C.colNorms());
        };

        /**
        * Returns an vector of cluster id assignments
        * @param {(module:la.Matrix | module:la.SparseMatrix)} A - Matrix whose columns correspond to examples.
        * @returns {module:la.IntVector} Vector of cluster assignments.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // create a new KMeans object
        * var KMeans = new analytics.KMeans({ iter: 1000, k: 3 });
        * // create a matrix to be fitted
        * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
        * // create the model with the matrix X
        * KMeans.fit(X);
        * // create the matrix of the prediction vectors
        * var pred = new la.Matrix([[2, -1, 1], [1, 0, -3]]);
        * // predict the values
        * var prediction = KMeans.predict(pred);
        */
        this.predict = function (X) {
            var ones_n = la.ones(X.cols).multiply(0.5);
            var ones_k = la.ones(k).multiply(0.5);
            var norX2 = la.square(X.colNorms());
            var D = C.multiplyT(X).minus(norC2.outer(ones_n)).minus(ones_k.outer(norX2));
            return la.findMaxIdx(D);
        }

        /**
        * Transforms the points to vectors of squared distances to centroids.
        * @param {(module:la.Matrix | module:la.SparseMatrix)} A - Matrix whose columns correspond to examples.
        * @returns {module:la.Matrix} Matrix where each column represents the squared distances to the centroid vectors.
        * @example
        * // import modules
        * var analytics = require('qminer').analytics;
        * var la = require('qminer').la;
        * // create a new KMeans object
        * var KMeans = new analytics.KMeans({ iter: 1000, k: 3 });
        * // create a matrix to be fitted
        * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
        * // create the model with the matrix X
        * KMeans.fit(X);
        * // create the matrix of the transform vectors
        * var matrix = new la.Matrix([[-2, 0], [0, -3]]);
        * // get the transform values of matrix
        * // returns the matrix
        * //  10    17
        * //   1    20
        * //  10     1
        * KMeans.transform(matrix);
        */
        this.transform = function (X) {
            var ones_n = la.ones(X.cols).multiply(0.5);
            var ones_k = la.ones(k).multiply(0.5);
            var norX2 = la.square(X.colNorms());
            var D = C.multiplyT(X).minus(norC2.outer(ones_n)).minus(ones_k.outer(norX2));
            D = D.multiply(-2);
            return D;
        }
		/**
        * Saves KMeans internal state into (binary) file
        * @param {string} fname - Name of the file to write into.
        */
        this.save = function(fname){
			if (!C) {
				throw new Error("KMeans.save() - model not created yet");
			}

			var params_vec = new la.Vector();
			params_vec.push(iter);
			params_vec.push(k);
			params_vec.push(verbose ? 1.0 : 0.0);

			var xfs = qm.fs;
			var fout = xfs.openWrite(fname);
			C.save(fout);
			norC2.save(fout);
			(new la.Vector(idxv)).save(fout);
			params_vec.save(fout);
			fout.close();
			fout = null;
		}
		/**
        * Loads KMeans internal state from (binary) file
        * @param {string} fname - Name of the file to read from.
        */
        this.load = function (fname) {
		    var xfs = qm.fs;
		    var fin = xfs.openRead(fname);

		    C = new la.Matrix();
		    C.load(fin);
		    norC2 = new la.Vector();
		    norC2.load(fin);

		    var idxvtmp = new la.Vector();
		    idxvtmp.load(fin);
		    idxv = idxvtmp; // make normal vector (?)

		    var params_vec = new la.Vector();
		    params_vec.load(fin);
		    iter = params_vec[0];
		    k = params_vec[1];
		    verbose = (params_vec[2] != 0);

		    fin = null;
		}
    }

    ///////////////////////////////
    ////// code below not yet ported or verified for scikit
    ///////////////////////////////

    function defarg(arg, defaultval) {
        return arg == undefined ? defaultval : arg;
    }

    function createBatchModel(featureSpace, models) {
        this.featureSpace = featureSpace;
        this.models = models;
        // get targets
        this.target = [];
        for (var cat in this.models) { this.target.push(cat); }
        // serialize to stream
        this.save = function (sout) {
            // save list
            sout.writeLine(this.models);
            // save feature space
            this.featureSpace.save(sout);
            // save models
            for (var cat in this.models) {
                this.models[cat].model.save(sout);
            }
            return sout;
        }

        this.predict = function (record) {
            var vec = this.featureSpace.extractSparseVector(record);
            var result = {};
            for (var cat in this.models) {
                result[cat] = this.models[cat].model.predict(vec);
            }
            return result;
        }

        this.predictLabels = function (record) {
            var result = this.predict(record);
            var labels = [];
            for (var cat in result) {
                if (result[cat] > 0.0) {
                    labels.push(cat);
                }
            }
            return labels;
        }

        this.predictTop = function (record) {
            var result = this.predict(record);
            var top = null;
            for (var cat in result) {
                if (top) {
                    if (top.weight > result[cat]) {
                        top.category = cat;
                        top.weight = result[cat];
                    }
                } else {
                    top = { category: cat, weight: result[cat] }
                }
            }
            return top.category;
        }
        return this;
    }

    //!- `batchModel = analytics.newBatchModel(rs, features, target)` -- learns a new batch model
    //!     using record set `rs` as training data and `features`; `target` is
    //!     a field descriptor JSON object for the records which we are trying to predict
	//!     (obtained by calling store.field("Rating");
    //!     if target field string or string vector, the result is a SVM classification model,
    //!     and if target field is a float, the result is a SVM regression model; resulting
    //!     model has the following functions:
    //!   - `strArr = batchModel.target` -- array of categories for which we have models
    //!   - `scoreArr = batchModel.predict(rec)` -- creates feature vector from record `rec`, sends it
    //!     through the model and returns the result as a dictionary where labels are keys and scores (numbers) are values.
    //!   - `labelArr = batchModel.predictLabels(rec)` -- creates feature vector from record `rec`,
    //!     sends it through the model and returns the labels with positive weights as `labelArr`.
    //!   - `labelStr = batchModel.predictTop(rec)` -- creates feature vector from record `rec`,
    //!     sends it through the model and returns the top ranked label `labelStr`.
    //!   - `batchModel.save(fout)` -- saves the model to `fout` output stream
    exports.newBatchModel = function (records, features, target, limitCategories) {
        console.log("newBatchModel", "Start");
        // prepare feature space
        console.log("newBatchModel", "  creating feature space");
        var featureSpace = new qm.FeatureSpace(records.store.base, features);
        // initialize features
        featureSpace.updateRecords(records);
        console.log("newBatchModel", "  number of dimensions = " + featureSpace.dim);
        // prepare spare vectors
        console.log("newBatchModel", "  preparing feature vectors");
        var sparseVecs = featureSpace.extractSparseMatrix(records);
        // prepare target vectors
        var targets = {};
        // figure out if new category name, or update count
        function initCats(categories, catName) {
            if (categories[catName]) {
                categories[catName].count++;
            } else {
                // check if we should ignore this category
                if (limitCategories && !qm_util.isInArray(limitCategories, catName)) { return; }
                // check if we should ignore this category
                categories[catName] = {
                    name: catName,
                    type: "classification",
                    count: 1,
                    target: new la.Vector({ mxVals: records.length })
                };
            }
        }
        // initialize targets
        console.log("newBatchModel", "  preparing target vectors");
        if (target.type === "string_v") {
            // get all possible values for the field
            for (var i = 0; i < records.length; i++) {
                var cats = records[i][target.name];
                for (var j = 0; j < cats.length; j++) {
                    initCats(targets, cats[j]);
                }
            }
            // initialized with +1 or -1 for each category
            for (var i = 0; i < records.length; i++) {
                var cats = la.copyVecToArray(records[i][target.name]);
                for (var cat in targets) {
                    targets[cat].target.push(qm_util.isInArray(cats, cat) ? 1.0 : -1.0);
                }
            }
        } else if (target.type === "string") {
            // get all possible values for the field
            for (var i = 0; i < records.length; i++) {
                var recCat = records[i][target.name];
                initCats(targets, recCat);
            }
            // initialized with +1 or -1 for each category
            for (var i = 0; i < records.length; i++) {
                var recCat = records[i][target.name];
                for (var cat in targets) {
                    targets[cat].target.push((recCat === cat) ? 1.0 : -1.0);
                }
            }
        } else if (target.type === "float") {
            // initialized with +1 or -1 for each category
            targets[target.name] = {
                name: target.name,
                type: "regression",
                count: records.length,
                target: new la.Vector({ mxVals: records.length })

            };
            for (var i = 0; i < records.length; i++) {
                targets[target.name].target.push(records[i][target.name]);
            }
        }
        // training model for each category
        console.log("newBatchModel", "  training SVM");
        var models = {};
        for (var cat in targets) {
            if (targets[cat].count >= 50) {
                models[cat] = {
                    name: targets[cat].name,
                    type: targets[cat].type,
                };
                if (targets[cat].type === "classification") {
                    console.log("newBatchModel", "    ... " + cat + " (classification)");
                    models[cat].model = new exports.SVC({ c: 1, j: 10, batchSize: 10000, maxIterations: 100000, maxTime: 1800, minDiff: 0.001 });
                    models[cat].model.fit(sparseVecs, targets[cat].target);
                } else if (targets[cat].type === "regression") {
                    console.log("newBatchModel", "    ... " + cat + " (regression)");
                    models[cat].model = new exports.SVR({ c: 1, eps: 1e-2, batchSize: 10000, maxIterations: 100000, maxTime: 1800, minDiff: 0.001 });
                    models[cat].model.fit(sparseVecs, targets[cat].target);
                }
            }
        }
        // done
        console.log("newBatchModel", "Done");
        // we finished the constructor
        return new createBatchModel(featureSpace, models);
    };

    //!- `batchModel = analytics.loadBatchModel(base, fin)` -- loads batch model frm input stream `fin`
    exports.loadBatchModel = function (base, sin) {
        var models = JSON.parse(sin.readLine());
        var featureSpace = new qm.FeatureSpace(base, sin);
        for (var cat in models) {
            models[cat].model = new exports.SVC(sin);
        }
        // we finished the constructor
        return new createBatchModel(featureSpace, models);
    };

	//!- `result = analytics.crossValidation(rs, features, target, folds)` -- creates a batch
    //!     model for records from record set `rs` using `features; `target` is the
    //!     target field and is assumed discrete; the result is a results object
    //!     with the following API:
    //!     - `result.target` -- an object with categories as keys and the following
    //!       counts as members of these keys: `count`, `TP`, `TN`, `FP`, `FN`,
    //!       `all()`, `precision()`, `recall()`, `accuracy()`.
    //!     - `result.confusion` -- confusion matrix between categories
    //!     - `result.report()` -- prints basic report on to the console
    //!     - `result.reportCSV(fout)` -- prints CSV output to the `fout` output stream
    exports.crossValidation = function (records, features, target, folds, limitCategories) {
        // create empty folds
        var fold = [];
        for (var i = 0; i < folds; i++) {
            fold.push(new la.IntVector());
        }
        // split records into folds
        records.shuffle(1);
        var fold_i = 0;
        for (var i = 0; i < records.length; i++) {
            fold[fold_i].push(records[i].$id);
            fold_i++; if (fold_i >= folds) { fold_i = 0; }
        }
        // do cross validation
        var cfyRes = null;
        for (var fold_i = 0; fold_i < folds; fold_i++) {
            // prepare train and test record sets
            var train = new la.IntVector();
            var test = new la.IntVector();
            for (var i = 0; i < folds; i++) {
                if (i == fold_i) {
                    test.pushV(fold[i]);
                } else {
                    train.pushV(fold[i]);
                }
            }
            var trainRecs = records.store.newRecSet(train);
            var testRecs = records.store.newRecSet(test);
            console.log("Fold " + fold_i + ": " + trainRecs.length + " training and " + testRecs.length + " testing");
            // create model for the fold
            var model = exports.newBatchModel(trainRecs, features, target, limitCategories);
            // prepare test counts for each target
            if (!cfyRes) { cfyRes = new exports.ClassificationScore (model.target); }
            // evaluate predictions
            for (var i = 0; i < testRecs.length; i++) {
                var correct = testRecs[i][target.name];
                var predicted = model.predictLabels(testRecs[i]);
                cfyRes.count(correct, predicted);
            }
            // report
            cfyRes.report();
        }
        return cfyRes;
    };

    //!- `alModel = analytics.newActiveLearner(query, qRecSet, fRecSet, ftrSpace, settings)` -- initializes the
    //!    active learning. The algorihm is run by calling `model.startLoop()`. The algorithm has two stages: query mode, where the algorithm suggests potential
    //!    positive and negative examples based on the query text, and SVM mode, where the algorithm keeps
    //!   selecting examples that are closest to the SVM margin (every time an example is labeled, the SVM
    //!   is retrained.
    //!   The inputs are: query (text), record set `qRecSet`, record set `fRecSet`,  the feature space `ftrSpace` and a
    //!   `settings`JSON object. The settings object specifies:`textField` (string) which is the name
    //!    of the field in records that is used to create feature vectors, `nPos` (integer) and `nNeg` (integer) set the number of positive and negative
    //!    examples that have to be identified in the query mode before the program enters SVM mode.
    //!   We can set two additional parameters `querySampleSize` and `randomSampleSize` which specify the sizes of subsamples of qRecSet and fRecSet, where the rest of the data is ignored in the active learning.
    //!   Final parameters are all SVM parameters (c, j, batchSize, maxIterations, maxTime, minDiff, verbose).
    exports.newActiveLearner = function (query, qRecSet, fRecSet, ftrSpace, stts) {
        return new exports.ActiveLearner(query, qRecSet, fRecSet, ftrSpace, stts);
    }

    exports.ActiveLearner = function (query, qRecSet, fRecSet, ftrSpace, stts) {
        var settings = defarg(stts, {});
        settings.nPos = defarg(stts.nPos, 2);
        settings.nNeg = defarg(stts.nNeg, 2);
        settings.textField = defarg(stts.textField, "Text");
        settings.querySampleSize = defarg(stts.querySampleSize, -1);
        settings.randomSampleSize = defarg(stts.randomSampleSize, -1);
        settings.c = defarg(stts.c, 1.0);
        settings.j = defarg(stts.j, 1.0);
        settings.batchSize = defarg(stts.batchSize, 100);
        settings.maxIterations = defarg(stts.maxIterations, 100000);
        settings.maxTime = defarg(stts.maxTime, 1); // 1 second for computation by default
        settings.minDiff = defarg(stts.minDiff, 1e-6);
        settings.verbose = defarg(stts.verbose, false);

        // compute features or provide them
        settings.extractFeatures = defarg(stts.extractFeatures, true);

        if (!settings.extractFeatures) {
            if (stts.uMat == null) { throw 'settings uMat not provided, extractFeatures = false'; }
            if (stts.uRecSet == null) { throw 'settings uRecSet not provided, extractFeatures = false'; }
            if (stts.querySpVec == null) { throw 'settings querySpVec not provided, extractFeatures = false'; }
        }

        // QUERY MODE
        var queryMode = true;
        // bow similarity between query and training set

        var querySpVec;
        var uRecSet;
        var uMat;

        if (settings.extractFeatures) {
            var temp = {}; temp[settings.textField] = query;
            var queryRec = qRecSet.store.newRecord(temp); // record
            querySpVec = ftrSpace.extractSparseVector(queryRec);
            // use sampling?
            var sq = qRecSet;
            if (settings.querySampleSize >= 0 && qRecSet != undefined) {
                sq = qRecSet.sample(settings.querySampleSize);
            }
            var sf = fRecSet;
            if (settings.randomSampleSize >= 0 && fRecSet != undefined) {
                sf = fRecSet.sample(settings.randomSampleSize);
            }
            // take a union or just qset or just fset if some are undefined
            uRecSet = (sq != undefined) ? ((sf != undefined) ? sq.setunion(sf) : sq) : sf;
            if (uRecSet == undefined) { throw 'undefined record set for active learning!';}
            uMat = ftrSpace.extractSparseMatrix(uRecSet);

        } else {
            querySpVec = stts.querySpVec;
            uRecSet = stts.uRecSet;
            uMat = stts.uMat;
        }


        querySpVec.normalize();
        uMat.normalizeCols();

        var X = new la.SparseMatrix();
        var y = new la.Vector();
        var simV = uMat.multiplyT(querySpVec); //similarities (q, recSet)
        var sortedSimV = simV.sortPerm(); //ascending sort
        var simVs = sortedSimV.vec; //sorted similarities (q, recSet)
        var simVp = sortedSimV.perm; //permutation of sorted similarities (q, recSet)
        //// counters for questions in query mode
        var nPosQ = 0; //for traversing simVp from the end
        var nNegQ = 0; //for traversing simVp from the start


        // SVM MODE
        var svm;
        var posIdxV = new la.IntVector(); //indices in recordSet
        var negIdxV = new la.IntVector(); //indices in recordSet

        var posRecIdV = new la.IntVector(); //record IDs
        var negRecIdV = new la.IntVector(); //record IDs

        var classVec = new la.Vector({ "vals": uRecSet.length }); //svm scores for record set
        var resultVec = new la.Vector({ "vals": uRecSet.length }); // non-absolute svm scores for record set


        //!   - `rs = alModel.getRecSet()` -- returns the record set that is being used (result of sampling)
        this.getRecSet = function () { return uRecSet };

        //!   - `idx = alModel.selectedQuestionIdx()` -- returns the index of the last selected question in alModel.getRecSet()
        this.selectedQuestionIdx = -1;

        //!   - `bool = alModel.getQueryMode()` -- returns true if in query mode, false otherwise (SVM mode)
        this.getQueryMode = function () { return queryMode; };

        //!   - `numArr = alModel.getPos(thresh)` -- given a `threshold` (number) return the indexes of records classified above it as a javascript array of numbers. Must be in SVM mode.
        this.getPos = function (threshold) {
            if (this.queryMode) { return null; } // must be in SVM mode to return results
            if (!threshold) { threshold = 0; }
            var posIdxArray = [];
            for (var recN = 0; recN < uRecSet.length; recN++) {
                if (resultVec[recN] >= threshold) {
                    posIdxArray.push(recN);
                }
            }
            return posIdxArray;
        };

        this.debug = function () { debugger; }

        this.getTop = function (limit) {
            if (this.queryMode) { return null; } // must be in SVM mode to return results
            if (!limit) { limit = 20; }
            var idxArray = [];
            var marginArray = [];
            var sorted = resultVec.sortPerm(false);
            for (var recN = 0; recN < uRecSet.length && recN < limit; recN++) {
                idxArray.push(sorted.perm[recN]);
                var val = sorted.vec[recN];
                val = val == Number.POSITIVE_INFINITY ? Number.MAX_VALUE : val;
                val = val == Number.NEGATIVE_INFINITY ? -Number.MAX_VALUE : val;
                marginArray.push(val);
            }
            return { posIdx: idxArray, margins: marginArray };
        };

        //!   - `objJSON = alModel.getSettings()` -- returns the settings object
        this.getSettings = function () { return settings; }

        // returns record set index of the unlabeled record that is closest to the margin
        //!   - `recSetIdx = alModel.selectQuestion()` -- returns `recSetIdx` - the index of the record in `recSet`, whose class is unknonw and requires user input
        this.selectQuestion = function () {
            if (posRecIdV.length >= settings.nPos && negRecIdV.length >= settings.nNeg) { queryMode = false; }
            if (queryMode) {
                if (posRecIdV.length < settings.nPos && nPosQ + 1 < uRecSet.length) {
                    nPosQ = nPosQ + 1;
                    console.log("query mode, try to get pos");
                    this.selectedQuestionIdx = simVp[simVp.length - 1 - (nPosQ - 1)];
                    return this.selectedQuestionIdx;
                }
                if (negRecIdV.length < settings.nNeg && nNegQ + 1 < uRecSet.length) {
                    nNegQ = nNegQ + 1;
                    // TODO if nNegQ == rRecSet.length, find a new sample
                    console.log("query mode, try to get neg");
                    this.selectedQuestionIdx = simVp[nNegQ - 1];
                    return this.selectedQuestionIdx;
                }
            }
            else {
                ////call svm, get record closest to the margin
                svm = new exports.SVC(settings);
                svm.fit(X, y);//column examples, y float vector of +1/-1, default svm paramvals

                // mark positives
                for (var i = 0; i < posIdxV.length; i++) {
                    classVec[posIdxV[i]] = Number.POSITIVE_INFINITY;
                    resultVec[posIdxV[i]] = Number.POSITIVE_INFINITY;
                }
                // mark negatives
                for (var i = 0; i < negIdxV.length; i++) {
                    classVec[negIdxV[i]] = Number.POSITIVE_INFINITY;
                    resultVec[negIdxV[i]] = Number.NEGATIVE_INFINITY;
                }
                var posCount = posIdxV.length;
                var negCount = negIdxV.length;
                // classify unlabeled
                for (var recN = 0; recN < uRecSet.length; recN++) {
                    if (classVec[recN] !== Number.POSITIVE_INFINITY) {
                        var svmMargin = svm.predict(uMat.getCol(recN));
                        if (svmMargin > 0) {
                            posCount++;
                        } else {
                            negCount++;
                        }
                        classVec[recN] = Math.abs(svmMargin);
                        resultVec[recN] = svmMargin;
                    }
                }
                var sorted = classVec.sortPerm();
                console.log("svm mode, margin: " + sorted.vec[0] + ", npos: " + posCount + ", nneg: " + negCount);
                this.selectedQuestionIdx = sorted.perm[0];
                return this.selectedQuestionIdx;
            }

        };
        // asks the user for class label given a record set index
        //!   - `alModel.getAnswer(ALAnswer, recSetIdx)` -- given user input `ALAnswer` (string) and `recSetIdx` (integer, result of model.selectQuestion) the training set is updated.
        //!      The user input should be either "y" (indicating that recSet[recSetIdx] is a positive example), "n" (negative example).
        this.getAnswer = function (ALanswer, recSetIdx) {
            //todo options: ?newQuery
            if (ALanswer === "y") {
                posIdxV.push(recSetIdx);
                posRecIdV.push(uRecSet[recSetIdx].$id);
                //X.push(ftrSpace.extractSparseVector(uRecSet[recSetIdx]));
                X.push(uMat.getCol(recSetIdx));
                y.push(1.0);
            } else {
                negIdxV.push(recSetIdx);
                negRecIdV.push(uRecSet[recSetIdx].$id);
                //X.push(ftrSpace.extractSparseVector(uRecSet[recSetIdx]));
                X.push(uMat.getCol(recSetIdx));
                y.push(-1.0);
            }
            // +k query // rank unlabeled according to query, ask for k most similar
            // -k query // rank unlabeled according to query, ask for k least similar
        };
        //!   - `alModel.startLoop()` -- starts the active learning loop in console
        this.startLoop = function () {
            while (true) {
                var recSetIdx = this.selectQuestion();
                var ALanswer = sget(uRecSet[recSetIdx].Text + ": y/(n)/s? Command s stops the process").trim();
                if (ALanswer == "s") { break; }
                if (posIdxV.length + negIdxV.length == uRecSet.length) { break; }
                this.getAnswer(ALanswer, recSetIdx);
            }
        };
        //!   - `alModel.saveSvmModel(fout)` -- saves the binary SVM model to an output stream `fout`. The algorithm must be in SVM mode.
        this.saveSvmModel = function (outputStream) {
            // must be in SVM mode
            if (queryMode) {
                console.log("AL.save: Must be in svm mode");
                return;
            }
            svm.save(outputStream);
        };

        this.getWeights = function () {
            return svm.weights;
        }
        //this.saveLabeled
        //this.loadLabeled
    };


	//////////// RIDGE REGRESSION
	// solve a regularized least squares problem
	//!- `ridgeRegressionModel = new analytics.RidgeRegression(kappa, dim, buffer)` -- solves a regularized ridge
	//!  regression problem: min|X w - y|^2 + kappa |w|^2. The inputs to the algorithm are: `kappa`, the regularization parameter,
	//!  `dim` the dimension of the model and (optional) parameter `buffer` (integer) which specifies
	//!  the length of the window of tracked examples (useful in online mode). The model exposes the following functions:
	exports.RidgeRegression = function (kappa, dim, buffer) {
	    var X = [];
	    var y = [];
	    buffer = typeof buffer !== 'undefined' ? buffer : -1;
	    var w = new la.Vector({ "vals": dim });
	    //!   - `ridgeRegressionModel.add(vec, num)` -- adds a vector `vec` and target `num` (number) to the training set
	    this.add = function (x, target) {
	        X.push(x);
	        y.push(target);
	        if (buffer > 0) {
	            if (X.length > buffer) {
	                this.forget(X.length - buffer);
	            }
	        }
	    };
	    //!   - `ridgeRegressionModel.addupdate(vec, num)` -- adds a vector `vec` and target `num` (number) to the training set and retrains the model
	    this.addupdate = function (x, target) {
	        this.add(x, target);
	        this.update();
	    }
	    //!   - `ridgeRegressionModel.forget(n)` -- deletes first `n` (integer) examples from the training set
	    this.forget = function (ndeleted) {
	        ndeleted = typeof ndeleted !== 'undefined' ? ndeleted : 1;
	        ndeleted = Math.min(X.length, ndeleted);
	        X.splice(0, ndeleted);
	        y.splice(0, ndeleted);
	    };
	    //!   - `ridgeRegressionModel.update()` -- recomputes the model
	    this.update = function () {
	        var A = this.getMatrix();
	        var b = new la.Vector(y);
	        w = this.compute(A, b);
	    };
	    //!   - `vec = ridgeRegressionModel.getModel()` -- returns the parameter vector `vec` (dense vector)
	    this.getModel = function () {
	        return w;
	    };
	    this.getMatrix = function () {
	        if (X.length > 0) {
	            var A = new la.Matrix({ "cols": X[0].length, "rows": X.length });
	            for (var i = 0; i < X.length; i++) {
	                A.setRow(i, X[i]);
	            }
	            return A;
	        }
	    };
	    //!   - `vec2 = ridgeRegressionModel.compute(mat, vec)` -- computes the model parameters `vec2`, given
	    //!    a row training example matrix `mat` and target vector `vec` (dense vector). The vector `vec2` solves min_vec2 |mat' vec2 - vec|^2 + kappa |vec2|^2.
	    //!   - `vec2 = ridgeRegressionModel.compute(spMat, vec)` -- computes the model parameters `vec2`, given
	    //!    a row training example sparse matrix `spMat` and target vector `vec` (dense vector). The vector `vec2` solves min_vec2 |spMat' vec2 - vec|^2 + kappa |vec2|^2.
	    this.compute = function (A, b) {
	        var I = la.eye(A.cols);
	        var coefs = (A.transpose().multiply(A).plus(I.multiply(kappa))).solve(A.transpose().multiply(b));
	        return coefs;
	    };
	    //!   - `num = model.predict(vec)` -- predicts the target `num` (number), given feature vector `vec` based on the internal model parameters.
	    this.predict = function (x) {
	        return w.inner(x);
	    };
	};


    /**
     * StreamStory.
     * @class
     * @param {opts} HierarchMarkovParam - parameters. TODO typedef and describe
     */
    exports.HierarchMarkov = function (opts) {
    	// constructor
    	if (opts == null) throw 'Missing parameters!';
    	if (opts.base == null) throw 'Missing parameter base!';

    	// create model and feature space
    	var mc;
    	var obsFtrSpace;
    	var controlFtrSpace;

    	if (opts.hmcConfig != null && opts.obsFields != null &&
    			opts.contrFields != null && opts.base != null) {

    		mc = opts.sequenceEndV != null ? new exports.HMC(opts.hmcConfig, opts.sequenceEndV) : new exports.HMC(opts.hmcConfig);

    		obsFtrSpace = new qm.FeatureSpace(opts.base, opts.obsFields);
    		controlFtrSpace = new qm.FeatureSpace(opts.base, opts.contrFields);
    	}
    	else if (opts.hmcFile != null) {
    		var fin = new fs.FIn(opts.hmcFile);
    		mc = new exports.HMC(fin);
    		obsFtrSpace = new qm.FeatureSpace(opts.base, fin);
    		controlFtrSpace = new qm.FeatureSpace(opts.base, fin);
    	}
    	else {
    		throw 'Parameters missing: ' + JSON.stringify(opts);
    	}

    	function getFtrNames(ftrSpace) {
    		var names = [];

    		var dims = ftrSpace.dims;
    		for (var i = 0; i < dims.length; i++) {
				names.push(ftrSpace.getFeature(i));
			}

    		return names;
    	}

    	function getObsFtrCount() {
			return obsFtrSpace.dims.length;
		}

    	function getObsFtrNames() {
    		return getFtrNames(obsFtrSpace);
    	}

    	function getControlFtrNames() {
    		return getFtrNames(controlFtrSpace);
    	}

    	function getFtrDescriptions(stateId) {
    		var observations = [];
    		var controls = [];

			var coords = mc.fullCoords(stateId);
			var obsFtrNames = getObsFtrNames();
			var invObsCoords = obsFtrSpace.invertFeatureVector(coords);
			for (var i = 0; i < invObsCoords.length; i++) {
				observations.push({name: obsFtrNames[i], value: invObsCoords.at(i)});
			}

			var controlCoords = mc.fullCoords(stateId, false);
			var contrFtrNames = getControlFtrNames();
			var invControlCoords = controlFtrSpace.invertFeatureVector(controlCoords);
			for (var i = 0; i < invControlCoords.length; i++) {
				controls.push({name: contrFtrNames[i], value: invControlCoords.at(i)});
			}

			return {
				observations: observations,
				controls: controls
			};
    	}

    	function getFtrCoord(stateId, ftrIdx) {
    		if (ftrIdx < obsFtrSpace.dims.length) {
    			return obsFtrSpace.invertFeatureVector(mc.fullCoords(stateId))[ftrIdx];
    		} else {
    			return controlFtrSpace.invertFeatureVector(mc.fullCoords(stateId, false))[ftrIdx - obsFtrSpace.dims.length];
    		}
    	}

    	// public methods
    	var that = {
    		/**
    		 * Creates a new model out of the record set.
    		 */
    		fit: function (opts) {
    			var recSet = opts.recSet;
    			var batchEndV = opts.batchEndV;
    			var timeField = opts.timeField;

    			log.info('Updating feature space ...');
    			obsFtrSpace.updateRecords(recSet);
    			controlFtrSpace.updateRecords(recSet);

    			var obsColMat = obsFtrSpace.extractMatrix(recSet);
    			var contrColMat = controlFtrSpace.extractMatrix(recSet);
    			var timeV = recSet.getVector(timeField);

    			log.info('Creating model ...');
    			mc.fit({
    				observations: obsColMat,
    				controls: contrColMat,
    				times: timeV,
    				batchV: batchEndV
    			});
    			log.info('Done!');

    			return that;
    		},

    		/**
    		 * Adds a new record. Doesn't update the models statistics.
    		 */
    		update: function (rec) {
    			if (rec == null) return;

    			var obsFtrVec = obsFtrSpace.extractVector(rec);
    			var contFtrVec = controlFtrSpace.extractVector(rec);
    			var timestamp = rec.time.getTime();

    			mc.update(obsFtrVec, contFtrVec, timestamp);
    		},

    		/**
    		 * Saves the feature space and model into the specified files.
    		 */
    		save: function (mcFName) {
    			try {
    				console.log('Saving Markov chain ...');

    				var fout = new fs.FOut(mcFName);

	    			mc.save(fout);
	    			obsFtrSpace.save(fout);
	    			controlFtrSpace.save(fout);

	    			fout.flush();
	    			fout.close();

	    			console.log('Done!');
    			} catch (e) {
    				console.log('Failed to save the model!!' + e.message);
    			}
    		},

    		/**
    		 * Returns the state used in the visualization.
    		 */
    		getVizState: function () {
    			log.debug('Fetching visualization ...');
    			return mc.toJSON();
    		},

    		/**
    		 * Returns the hierarchical Markov chain model.
    		 */
    		getModel: function () {
    			return mc;
    		},

    		/**
    		 * Returns the feature space.
    		 */
    		getFtrSpace: function () {
    			return { observations: obsFtrSpace, controls: controlFtrSpace };
    		},

    		/**
    		 * Returns the current state at the specified height. If the height is not specified it
    		 * returns the current states through the hierarchy.
    		 */
    		currState: function (height) {
    			return mc.currState(height);
    		},

    		/**
    		 * Returns the most likely future states.
    		 */
    		futureStates: function (level, state, time) {
    			return mc.futureStates(level, state, time);
    		},

    		/**
    		 * Returns the most likely future states.
    		 */
    		pastStates: function (level, state, time) {
    			return mc.pastStates(level, state, time);
    		},

    		getFtrNames: function () {
    			return {
    				observation: getObsFtrNames(),
    				control: getControlFtrNames()
    			}
    		},

    		/**
    		 * Returns state details as a Javascript object.
    		 */
    		stateDetails: function (stateId, height) {
    			var futureStates = mc.futureStates(height, stateId);
    			var pastStates = mc.pastStates(height, stateId);
    			var isTarget = mc.isTarget(stateId, height);
    			var stateNm = mc.getStateName(stateId);
    			var wgts = mc.getStateWgtV(stateId);

    			var features = getFtrDescriptions(stateId);

    			return {
    				id: stateId,
    				name: stateNm.length > 0 ? stateNm : null,
    				isTarget: isTarget,
    				features: features,
    				futureStates: futureStates,
    				pastStates: pastStates,
    				featureWeights: wgts
    			};
    		},

    		/**
    		 * Returns a histogram for the desired feature in the desired state.
    		 */
    		histogram: function (stateId, ftrIdx) {
    			var hist = mc.histogram(stateId, ftrIdx);

    			var nObsFtrs = getObsFtrCount();

    			if (ftrIdx < nObsFtrs) {
	    			for (var i = 0; i < hist.binStartV.length; i++) {
	    				hist.binStartV[i] = obsFtrSpace.invertFeature(ftrIdx, hist.binStartV[i]);
	    			}
    			} else {
    				for (var i = 0; i < hist.binStartV.length; i++) {
	    				hist.binStartV[i] = controlFtrSpace.invertFeature(ftrIdx - nObsFtrs, hist.binStartV[i]);
	    			}
    			}

    			return hist;
    		},

    		/**
    		 * Callback when the current state changes.
    		 */
    		onStateChanged: function (callback) {
    			mc.onStateChanged(callback);
    		},

    		/**
    		 * Callback when an anomaly is detected.
    		 */
    		onAnomaly: function (callback) {
    			mc.onAnomaly(callback);
    		},

    		onOutlier: function (callback) {
    			mc.onOutlier(function (ftrV) {
    				var invFtrV = obsFtrSpace.invertFeatureVector(ftrV);

    				var features = [];
    				for (var i = 0; i < invFtrV.length; i++) {
    					features.push({name: obsFtrSpace.getFeature(i), value: invFtrV.at(i)});
    				}

    				callback(features);
    			});
    		},

    		onPrediction: function (callback) {
    			mc.onPrediction(callback);
    		},

    		/**
    		 * Returns the distribution of features accross the states on the
    		 * specified height.
    		 */
    		getFtrDist: function (height, ftrIdx) {
    			var stateIds = mc.stateIds(height);

    			var result = [];
    			for (var i = 0; i < stateIds.length; i++) {
    				var stateId = stateIds[i];
    				var coord = getFtrCoord(stateId, ftrIdx);
    				result.push({ state: stateId, value: coord });
    			}

    			return result;
    		},

    		setControl: function (ftrIdx, factor) {
    			var controlFtrIdx = ftrIdx - obsFtrSpace.dims.length;
    			mc.setControlFactor(controlFtrIdx, factor);
    		}
    	};

    	return that;
    };
    
