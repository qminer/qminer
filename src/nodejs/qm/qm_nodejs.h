/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef QMINER_QM_NODEJS
#define QMINER_QM_NODEJS

#include <node.h>
#include <node_object_wrap.h>
#include <qminer.h>
#include "../la/la_nodejs.h"
#include "../fs/fs_nodejs.h"
#include "../nodeutil.h"

#include "qm_nodejs_streamaggr.h"

///////////////////////////////
// NodeJs QMiner.
// A factory of base objects
// The only part of node framework: Init


/**
* Qminer module.
* @module qm
* @example 
* // import module
* var qm = require('qminer');
*/
class TNodeJsQm : public node::ObjectWrap {
public:
	// Node framework
	static void Init(v8::Handle<v8::Object> exports);	
	// TNodeJsRec needs this to select a template. TODO remove, see comment in
    //   v8::Local<v8::Object> TNodeJsRec::New(const TQm::TRec& Rec, const TInt& _Fq)
	static THash<TStr, TUInt> BaseFPathToId;
private:
	/*
	* Creates a directory structure.
	* @param {string} [configPath='qm.conf'] - The path to configuration file.
	* @param {boolean} [overwrite=false] - If you want to overwrite the configuration file.
	* @param {number} [portN=8080] - The number of the port. Currently not used.
	* @param {number} [cacheSize=1024] - Sets available memory for indexing (in MB).
	*/
	// exports.config = function(configPath, overwrite, portN, cacheSize) {}
	JsDeclareFunction(config);

	/*
	* Creates an empty base.
	* @param {string} [configPath='qm.conf'] - Configuration file path.
	* @param {string} [schemaPath=''] - Schema file path.
	* @param {boolean} [clear=false] - Clear the existing db folder.
	* @returns {module:qm.Base}
	*/
	// exports.create = function (configPath, schemaPath, clear) { return Object.create(require('qminer').Base.prototype); }
	JsDeclareFunction(create);

	/*
	* Opens a base.
	* @param {string} [configPath='qm.conf'] - The configuration file path.
	* @param {boolean} [readOnly=false] - Open in read only mode?
	* @returns {module:qm.Base}
	*/
	// exports.open = function (configPath, readOnly) { return Object.create(require('qminer').Base.prototype); }
	JsDeclareFunction(open);
    
	/**
	* Set verbosity of QMiner internals.
	* @param {number} [level=0] - verbosity level: 0 = no output, 1 = log output, 2 = log and debug output.
	*/
	//# exports.verbosity = function (level) { }
	JsDeclareFunction(verbosity);
};

///////////////////////////////
// NodeJs QMiner Base

/**
* Base access modes.
* @readonly
* @enum {string}
*/
//# var baseModes = {
//#    /** sets up the db folder */
//#    create: 'create',
//#    /** cleans the db folder and calls create */
//#    createClean: 'createClean',
//#    /** opens with write permissions */
//#    open: 'open',
//#    /** opens in read-only mode */
//#    openReadOnly: 'openReadOnly'
//# }

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
//# var fieldTypes = {
//#    /** signed 32-bit integer */
//#    int: 'int', 
//#    /** vector of signed 32-bit integers */
//#    int_v: 'int_v', 
//# /** string */
//# string : 'string',
//# /** vector of strings */
//# string_v : 'string_v',
//# /** boolean */
//# bool : 'bool',
//# /** double precision floating point number */
//# float : 'float',
//# /** a pair of floats, useful for storing geo coordinates */
//# float_pair : 'float_pair',
//# /** vector of floats */
//# float_v : 'float_v',
//# /** date and time format, stored in a form of milliseconds since 1600 */
//# datetime : 'datetime',
//# /** sparse vector(same format as used by QMiner JavaScript linear algebra library) */
//# num_sp_v : 'num_sp_v',
//# }

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
//# var FeatureWeight = {
//# /** Sets 1 if term occurs, 0 otherwise. */
//# none: 'none',
//# /** Sets the term frequency in the document. */
//# tf: 'tf',
//# /** Sets the inverse document frequency in the document. */
//# idf: 'idf',
//# /** Sets the product of the tf and idf score. */
//# tfidf: 'tfidf'
//# }

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
//# var FeatureTokenizerType = {
//#	/** The simple encoding. */
//#	simple: 'simple',
//#	/** The html encoding. */
//# html: 'html',
//# /** The unicode encoding. */
//# unicode: 'unicode'
//# }

/**
* THe stopwords used for extraction.
* @readonly
* @enum {Object}
*/
//# var FeatureTokenizerStopwords = {
//# /** The pre-defined stopword list (none). */
//# none: 'none',
//# /** The pre-defined stopword list (english). */
//# en: 'en',
//# /** The pre-defined stopword list (slovene). */
//# si: 'si',
//# /** The pre-defined stopword list (spanish). */
//# es: 'es',
//# /** The pre-defined stopword list (german). */
//# de: 'de',
//# /** An array of stopwords. The array must be given as a parameter instead of 'array'! */
//# array: 'array'
//# }

/**
* The steemer used for extraction.
* @readonly
* @enum {Object}
*/
//# var FeatureTokenizerStemmer = {
//# /** For using the porter stemmer. */
//# boolean: 'true',
//# /** For using the porter stemmer. */
//# porter: 'porter',
//# /** For using no stemmer. */
//# none: 'none',
//# }

/**
* How are multi-record cases combined into a single vector. //TODO not implemented for join record cases (works only if the start store and the 
* feature store are the same)
* @readonly
* @enum {string}
*/
//# var FeatureMode = {
//#		/** Multi-record cases are merged into one document. */
//#		concatenate: 'concatenate', 
//#		/** Treat each case as a separate document. */
//#		centroid: 'centroid', 
//#		/** //TODO (Use the tokenizer option) */
//#		tokenized : 'tokenized'
//# }

/**
* Details on forgetting old IDFs when running on stream.
* @readonly
* @enum {string}
*/
//# var FeatureStream = {
//# /** (optional) Field name which is providing timestamp, if missing system time is used. */
//# field: 'field',
//# /** Forgetting factor, by which the old IDFs are multiplied after each iteration. */
//# factor: 'factor',
//# /** The time between iterations when the factor is applied, standard JSon time format is used to specify the interval duration. */
//# interval: 'interval'
//# }

class TNodeJsBaseWatcher {
private:
	// smart pointer
	TCRef CRef;
	friend class TPt<TNodeJsBaseWatcher>;
public:
	bool OpenP;
	TNodeJsBaseWatcher() { OpenP = true; }
	static TPt<TNodeJsBaseWatcher> New() { return new TNodeJsBaseWatcher; }
	void AssertOpen() { EAssertR(OpenP, "Base is closed!"); }
	void Close() { OpenP = false; }
};
typedef TPt<TNodeJsBaseWatcher> PNodeJsBaseWatcher;

/**
* Base
* @classdesc Represents the database and holds stores.
* @class
* @param {module:qm~BaseConstructorParam} paramObj - The base constructor parameter object
* @example
* // import qm module
* var qm = require('qminer');
* // using a constructor, in open mode:
* var base = new qm.Base({mode: 'open'});
*/
//# exports.Base = function (paramObj) {};
class TNodeJsBase : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	static v8::Persistent<v8::Function> Constructor;
public:
	static void Init(v8::Handle<v8::Object> Exports);
	static const TStr ClassId;
	// wrapped C++ object
	TWPt<TQm::TBase> Base;
	// C++ constructor
	TNodeJsBase(const TWPt<TQm::TBase>& Base_) : Base(Base_) { Watcher = TNodeJsBaseWatcher::New(); }
	TNodeJsBase(const TStr& DbPath, const TStr& SchemaFNm, const PJsonVal& Schema, const bool& Create, const bool& ForceCreate, const bool& ReadOnly, const TInt& IndexCache, const TInt& StoreCache);
	// Object that knows if Base is valid
	PNodeJsBaseWatcher Watcher;
private:		
	// parses arguments, called by javascript constructor 
	static TNodeJsBase* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);
private:
	/**
	* Closes the database.
	* @returns {null}
	*/
	//# exports.Base.prototype.close = function () {}
	JsDeclareFunction(close);

	/**
	 * Returns the store with the specified name.
	 *
	 * @param {string} name - Name of the store.
	 * @returns {module:qm.Store} The store.
	 */
	//# exports.Base.prototype.store = function (name) { return Object.create(require('qminer').Store.prototype); }
	JsDeclareFunction(store);

	/**
	 * Returns a list of store descriptors.
	 *
	 * @returns {Object[]}
	 */
	//# exports.Base.prototype.getStoreList = function () { return [{storeId:'', storeName:'', storeRecords:'', fields: [], keys: [], joins: []}]; }
	JsDeclareFunction(getStoreList);
	/**
	* Creates a new store.
	* @param {Array<module:qm~SchemaDefinition>} storeDef - The definition of the store(s)
	* @param {number} [storeSizeInMB = 1024] - The reserved size of the store(s).
	* @returns {(module:qm.Store | module:qm.Store[])} - Returns a store or an array of stores (if the schema definition was an array)
	*/
	//# exports.Base.prototype.createStore = function (storeDef, storeSizeInMB) { return storeDef instanceof Array ? [Object.create(require('qminer').Store.prototype)] : Object.create(require('qminer').Store.prototype) ;}
	JsDeclareFunction(createStore);
	/**
	* Creates a new store.
	* @param {module:qm~QueryObject} query - query language JSON object	
	* @returns {module:qm.RecSet} - Returns the record set that matches the search criterion
	*/
	//# exports.Base.prototype.search = function (query) { return Object.create(require('qminer').RecSet.prototype);}
	JsDeclareFunction(search);   
	/**
	* Calls qminer garbage collector to remove records outside time windows.
	*/
	//# exports.Base.prototype.garbageCollect = function () { }
	JsDeclareFunction(garbageCollect);
	/**
	* Calls qminer partial flush - base saves dirty data given some time-window.
	* @param {number} window - Length of available time-window in msec. Default 500.
	*/
	//# exports.Base.prototype.partialFlush = function () { }
	JsDeclareFunction(partialFlush);
	/**
	* Retrieves performance statistics for qminer.
	*/
	//# exports.Base.prototype.getStats = function () { }
	JsDeclareFunction(getStats);

	//!- `sa = base.getStreamAggr(saName)` -- gets the stream aggregate `sa` given name (string).
	JsDeclareFunction(getStreamAggr);
	//!- `strArr = base.getStreamAggrNames()` -- gets the stream aggregate names of stream aggregates in the default stream aggregate base.
	JsDeclareFunction(getStreamAggrNames);	
	//!JSIMPLEMENT:src/qminer/qminer.js    
};


///////////////////////////////
// NodeJs-Qminer-Store

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
//# exports.Store = function (base, storeDef) {};
class TNodeJsStore : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	// Node framework
	static v8::Persistent<v8::Function> Constructor;
public:	
	// Node framework 
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr ClassId;
	// Wrapped C++ object
	TWPt<TQm::TStore> Store;
	// Object that knows if Base is valid
	PNodeJsBaseWatcher Watcher;
	// C++ constructors	
	TNodeJsStore(TWPt<TQm::TStore> _Store, PNodeJsBaseWatcher& _Watcher) : Store(_Store), Watcher(_Watcher) { }

	// Field accessors
	static v8::Local<v8::Value> Field(const TQm::TRec& Rec, const int FieldId);
	static v8::Local<v8::Value> Field(const TWPt<TQm::TStore>& Store, const uint64& RecId, const int FieldId);
private:
	//! 
	//! **Functions and properties:**
	//!
	//!- `rec = store.rec(recName)` -- get record named `recName`; 
	//!     returns `null` when no such record exists
	/**
	* Returns a record form the store.
	* @param {string} recName - Record name.
	* @returns {Object} Returns the record. If the record doesn't exist, it returns null. //TODO
	*/
	//# exports.Store.prototype.rec = function (recName) {};
	JsDeclareFunction(rec);

	//!- `store = store.each(callback)` -- iterates through the store and executes the callback function `callback` on each record. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Returns self. Examples:
	//!  - `store.each(function (rec) { console.log(JSON.stringify(rec)); })`
	//!  - `store.each(function (rec, idx) { console.log(JSON.stringify(rec) + ', ' + idx); })`
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
	* base.store("Class").add({ "Name": "Bob", "StudyGroup": "A" });
	* base.store("Class").add({ "Name": "John", "StudyGroup": "B" });
	* base.store("Class").add({ "Name": "Marco", "StudyGroup": "C" });
	* base.store("Class").add({ "Name": "Dana", "StudyGroup": "A" });
	* // change the StudyGroup of all records of store Class to "A"
	* base.store("Class").each(function (rec) { rec.StudyGroup = "A"; });	// all records in Class are now in study group A
	*/
	//# exports.Store.prototype.each = function (callback) {}
	JsDeclareFunction(each);

	//!- `arr = store.map(callback)` -- iterates through the store, applies callback function `callback` to each record and returns new array with the callback outputs. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Examples:
	//!  - `arr = store.map(function (rec) { return JSON.stringify(rec); })`
	//!  - `arr = store.map(function (rec, idx) {  return JSON.stringify(rec) + ', ' + idx; })`
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
	* base.store("Class").add({ "Name": "Bob", "StudyGroup": "A" });
	* base.store("Class").add({ "Name": "John", "StudyGroup": "B" });
	* base.store("Class").add({ "Name": "Marco", "StudyGroup": "C" });
	* base.store("Class").add({ "Name": "Dana", "StudyGroup": "A" });
	* // make an array of record names
	* var arr = base.store("Class").map(function (rec) { return rec.Name; }); // returns an array ["Bob", "John", "Marco", "Dana"]
	*/
	//# exports.Store.prototype.map = function (callback) {}
	JsDeclareFunction(map);

	//!- `recId = store.push(rec)` -- add record `rec` to the store and return its ID `recId`
	/**
	* Adds a record to the store.
	* @param {Object} rec - The added record. //TODO
	* @returns {number} The ID of the added record.
	*/
	//# exports.Store.prototype.push = function (rec) {}
	JsDeclareFunction(push);

	//!- `rec = store.newRecord(recordJson)` -- creates new record `rec` by (JSON) value `recordJson` (not added to the store)
	/**
	* Creates a new record of given store. The record is not added to the store.
	* @param {Object} json - A JSON value of the record.
	* @returns {module:qm.Record} The record created by the JSON value and the store.
	*/
	//# exports.Store.prototype.newRecord = function (json) {};
	JsDeclareFunction(newRecord);

	//!- `rs = store.newRecordSet(idVec)` -- creates new record set from an integer vector record IDs `idVec` (type la.newIntVec);
	/**
	* Creates a new record set out of the records in store.
	* @param {module:la.IntVector} idVec - The integer vector containing the ids of selected vectors.
	* @returns {module:qm.RecSet} The record set that contains the records gained with idVec.
	*/
	//# exports.Store.prototype.newRecordSet = function (idVec) {};
	JsDeclareFunction(newRecordSet);

	//!- `rs = store.sample(sampleSize)` -- create a record set containing a random 
	//!     sample of `sampleSize` records
	/**
	* Creates a record set containing random records from store.
	* @param {number} sampleSize - The size of the record set.
	* @returns {module:qm.RecSet} Returns a record set containing random records.
	*/
	//# exports.Store.prototype.sample = function (sampleSize) {};
	JsDeclareFunction(sample);

	//!- `field = store.field(fieldName)` -- get details of field named `fieldName`
	/**
	* Gets the details of the selected field.
	* @param {string} fieldName - The name of the field.
	* @returns {Object} The JSON object containing the details of the field. //TODO
	*/
	//# exports.Store.prototype.field = function (fieldName) {}; 
	JsDeclareFunction(field);

	//!- `bool = store.isNumeric(fieldName)` -- returns true if the field is of numeric type
	/**
	* Checks if the field is of numeric type.
	* @param {string} fieldName - The checked field.
	* @returns {boolean} True, if the field is of numeric type. Otherwise, false.
	*/
	//# exports.Store.prototype.isNumeric = function (fieldName) {};
	JsDeclareFunction(isNumeric)

	//!- `bool = store.isString(fieldName)` -- returns true if the field is of String type
	/**
	* Checks if the field is of string type.
	* @param {string} fieldName - The checked field.
	* @returns {boolean} True, if the field is of the string type. Otherwise, false.
	*/
	//# exports.Store.prototype.isString = function (fieldName) {}; 
	JsDeclareFunction(isString)

	//!- `bool = store.isDate(fieldName)` -- returns true if the field is of type Date
	JsDeclareFunction(isDate)

	//!- `key = store.key(keyName)` -- get [index key](#index-key) named `keyName`
	/**
	* Returns the details of the selected key.
	* @param {string} keyName - The selected key.
	* @returns {Object} The JSON object containing the details of the key. //TODO
	*/
	//# exports.Store.prototype.key = function (keyName) {};
	JsDeclareFunction(key);

	////!- `store.addTrigger(trigger)` -- add `trigger` to the store triggers. Trigger is a JS object with three properties `onAdd`, `onUpdate`, `onDelete` whose values are callbacks
	//JsDeclareFunction(addTrigger); Deprecated - use new qm.sa(...) instead
	//!- `sa = store.getStreamAggr(saName)` -- returns a stream aggregate `sa` whose name is `saName`
	JsDeclareFunction(getStreamAggr);
	//!- `strArr = store.getStreamAggrNames()` -- returns the names of all stream aggregators listening on the store as an array of strings `strArr`
	JsDeclareFunction(getStreamAggrNames);
	//!- `objJSON = store.toJSON()` -- returns the store as a JSON
	/**
	* Returns the store as a JSON.
	* @returns {Object} The store as a JSON.
	*/
	//# exports.Store.prototype.toJSON = function () {};
	JsDeclareFunction(toJSON);

	//!- `store.clear()` -- deletes all records
	//!- `len = store.clear(num)` -- deletes the first `num` records and returns new length `len`
	/**
	* Deletes the records in the store.
	* @param {number} [num] - The number of deleted records.
	* @returns {number} The number of remaining records in the store.
	* @example
	* // delete all records in store
	* store.clear();	// returns 0
	* // deletes the first 10 records
	* store.clear(10);
	*/
	//# exports.Store.prototype.clear = function (num) {};
	JsDeclareFunction(clear);

	//!- `vec = store.getVector(fieldName)` -- gets the `fieldName` vector - the corresponding field type must be one-dimensional, e.g. float, int, string,...
	/**
	* Gives a vector containing the field value of each record.
	* @param {string} fieldName - The field name. Field must be of one-dimensional type, e.g. int, float, string...
	* @returns {module:la.Vector} The vector containing the field values of each record.
	*/
	//# exports.Store.prototype.getVector = function (fieldName) {};
	JsDeclareFunction(getVector);

	//!- `mat = store.getMatrix(fieldName)` -- gets the `fieldName` matrix - the corresponding field type must be float_v or num_sp_v
	/**
	* Gives a matrix containing the field values of each record.
	* @param {string} fieldName - The field name. Field mustn't be of type string.
	* @returns {(module:la.Matrix | module:la.SparseMatrix)} The matrix containing the field values. 
	*/
	//# exports.Store.prototype.getMatrix = function (fieldName) {};
	JsDeclareFunction(getMatrix);

	//!- `val = store.cell(recId, fieldId)` -- if fieldId (int) corresponds to fieldName, this is equivalent to store[recId][fieldName]
	//!- `val = store.cell(recId, fieldName)` -- equivalent to store[recId][fieldName]
	/**
	* Gives the field value of a specific record.
	* @param {number} recId - The record id.
	* @param {string} fieldName - The field's name.
	* @returns {Object} The fieldName value of the record with recId.
	*/
	//# exports.Store.prototype.cell = function (recId, fieldName) {};
	JsDeclareFunction(cell);

	//!- `str = store.name` -- name of the store
	/**
	* Gives the name of the store.
	*/
	//# exports.Store.prototype.name = undefined;
	JsDeclareProperty(name);

	//!- `bool = store.empty` -- `bool = true` when store is empty
	/**
	* Checks if the store is empty.
	*/
	//# exports.Store.prototype.empty = undefined;
	JsDeclareProperty(empty);

	//!- `len = store.length` -- number of records in the store
	/**
	* Gives the number of records.
	*/
	//# exports.Store.prototype.length = 0;
	JsDeclareProperty(length);

	//!- `rs = store.recs` -- create a record set containing all the records from the store
	/**
	* Creates a record set containing all the records from the store.
	*/
	//# exports.Store.prototype.recs = undefined;
	JsDeclareProperty(recs);

	//!- `objArr = store.fields` -- array of all the field descriptor JSON objects
	/**
	* Gives an array of all field descriptor JSON objects.
	*/
	//# exports.Store.prototype.fields = undefinied;
	JsDeclareProperty(fields);

	//!- `objArr = store.joins` -- array of all the join names
	/**
	* Gives an array of all join descriptor JSON objects.
	*/
	//# exports.Store.prototype.joins = undefined;
	JsDeclareProperty(joins);

	//!- `objArr = store.keys` -- array of all the [index keys](#index-key) objects    
	/**
	* Gives an array of all key descriptor JSON objects.
	*/
	//# exports.Store.prototype.keys = undefined;
	JsDeclareProperty(keys);

	//!- `rec = store.first` -- first record from the store
	/**
	* Returns the first record of the store.
	*/
	//# exports.Store.prototype.first = undefined;
	JsDeclareProperty(first);

	//!- `rec = store.last` -- last record from the store
	/**
	* Returns the last record of the store.
	*/
	//# exports.Store.prototype.last = undefined;
	JsDeclareProperty(last);

	//!- `iter = store.forwardIter` -- returns iterator for iterating over the store from start to end
	/**
	* Returns an iterator for iterating over the store from start to end.
	*/
	//# exports.Store.prototype.forwardIter = undefined;
	JsDeclareProperty(forwardIter);

	//!- `iter = store.backwardIter` -- returns iterator for iterating over the store from end to start
	/**
	* Returns an iterator for iterating over the store form end to start.
	*/
	//# exports.Store.prototype.backwardIter = undefined;
	JsDeclareProperty(backwardIter);

	//!- `rec = store[recId]` -- get record with ID `recId`; 
	//!     returns `null` when no such record exists
	JsDeclIndexedProperty(indexId);	

	//!- `base = store.base` -- get store base; 
	JsDeclareProperty(base);
	//!JSIMPLEMENT:src/qminer/store.js
};

///////////////////////////////
// NodeJs QMiner Record

/**
* Record (factory pattern).
* @namespace
*/
//# exports.Record = function () {}; 
class TNodeJsRec: public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	// Modified node framework: one record template per each base,storeId combination 
	static TVec<TVec<v8::Persistent<v8::Function> > > BaseStoreIdConstructor;
public:
	// Node framework 
	static void Init(const TWPt<TQm::TStore>& Store);
	static const TStr ClassId;

	// when reseting a db we have to clear the old record templates
	static void Clear(const int& BaseId);
	// Object that knows if Base is valid
	PNodeJsBaseWatcher Watcher;
	// C++ wrapped object
	TQm::TRec Rec;
	TInt Fq;
	// C++ constructors	
	TNodeJsRec(PNodeJsBaseWatcher _Watcher, const TQm::TRec& _Rec, const TInt& _Fq = 0) : Watcher(_Watcher), Rec(_Rec), Fq(_Fq) {}
	// Not typical (records have multiple templates), simpler objects get this method from TNodeJsUtil
	static v8::Local<v8::Object> NewInstance(TNodeJsRec* Obj);
	
private:
	//!
	//! **Functions and properties:**
	//!

    //!- `rec2 = rec.$clone()` -- create a clone of JavaScript wrapper with same record inside
	/**
	* Clones the record.
	* @returns {module:qm.Record} The clone of the record.
	*/
	//# exports.Record.prototype.$clone = function () {};
    JsDeclareFunction(clone);

    //!- `rec = rec.addJoin(joinName, (joinRecord | joinRecordId))` -- adds a join record `joinRecord` (or given id, joinRecordId) to join `jonName` (string). Returns self.
    //!- `rec = rec.addJoin(joinName, (joinRecord | joinRecordId), joinFrequency)` -- adds a join record `joinRecord` (or given id, joinRecordId) to join `jonName` (string) with join frequency `joinFrequency`. Returns self.
    JsDeclareFunction(addJoin);
    //!- `rec = rec.delJoin(joinName, (joinRecord | joinRecordId))` -- deletes join record `joinRecord` (or given id, joinRecordId) from join `joinName` (string). Returns self.
    //!- `rec = rec.delJoin(joinName, (joinRecord | joinRecordId), joinFrequency)` -- deletes join record `joinRecord` (or given id, joinRecordId) from join `joinName` (string) with join frequency `joinFrequency`. Return self.
    JsDeclareFunction(delJoin);

    //!- `objJSON = rec.toJSON()` -- provide json version of record, useful when calling JSON.stringify
	/**
	* Creates a JSON version of the record.
	* @returns {Object} The JSON version of the record.
	*/
	//# exports.Record.prototype.toJSON = function () {};
    JsDeclareFunction(toJSON);

	//!- `recId = rec.$id` -- returns record ID
	/**
	* Returns the id of the record.
	*/
	//# exports.Record.prototype.$id = undefined;
	JsDeclareProperty(id);

	//!- `recName = rec.$name` -- returns record name
	/**
	* Returns the name of the record.
	*/
	//# exports.Record.prototype.$name = undefined;
	JsDeclareProperty(name);

	//!- `recFq = rec.$fq` -- returns record frequency (used for randomized joins)
	/**
	* Returns the frequency of the record.
	*/
	//# exports.Record.prototype.$fq = undefined;
	JsDeclareProperty(fq);

	//!- `recStore = rec.$store` -- returns record store
	/**
	* Returns the store the record belongs to.
	*/
	//# exports.Record.prototype.store = undefined;
	JsDeclareProperty(store);

	//!- `rec['fieldName'] = val` -- sets the record's field `fieldName` to `val`. Equivalent: `rec.fieldName = val`.
	//!- `val = rec['fieldName']` -- gets the value `val` at field `fieldName`. Equivalent: `val = rec.fieldName`.
	JsDeclareSetProperty(getField, setField);
	//!- `rs = rec['joinName']` -- gets the record set if `joinName` is an index join. Equivalent: `rs = rec.joinName`. No setter currently.
	//!- `rec2 = rec['joinName']` -- gets the record `rec2` is the join `joinName` is a field join. Equivalent: `rec2 = rec.joinName`. No setter currently.
	JsDeclareProperty(join);
	JsDeclareProperty(sjoin);
};

///////////////////////////////
// NodeJs QMiner Record Set

/**
* Record Set (factory pattern)
* @namespace
* @example
* // import qm module
* var qm = require('qminer');
* // factory based construction using store.recs
* var rs = store.recs;
*/
//# exports.RecSet = function () {}
class TNodeJsRecSet: public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	// Node framework
	static v8::Persistent<v8::Function> Constructor;
public:
	// Node framework 
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr ClassId;
	// C++ wrapped object
	TQm::PRecSet RecSet;
	// Object that knows if Base is valid
	PNodeJsBaseWatcher Watcher;
	// C++ constructors
	TNodeJsRecSet(const TQm::PRecSet& _RecSet, PNodeJsBaseWatcher& _Watcher) : RecSet(_RecSet), Watcher(_Watcher) {}
private:
	//! 
	//! **Functions and properties:**
	//!   
	//!- `rs2 = rs.clone()` -- creates new instance of record set
	/**
	* Creates a new instance of the record set.
	* @returns {module:qm.RecSet} A copy of the record set.
	*/
	//# exports.RecSet.prototype.clone = function () {};
	JsDeclareFunction(clone);

	//!- `rs2 = rs.join(joinName)` -- executes a join `joinName` on the records in the set, result is another record set `rs2`.
	//!- `rs2 = rs.join(joinName, sampleSize)` -- executes a join `joinName` on a sample of `sampleSize` records in the set, result is another record set `rs2`.
	/**
	* Creates a new record set out of the join attribute of records.
	* @param {string} joinName - The name of the join attribute.
	* @param {number} [sampleSize] - The number of records to be used for construction of the record set.
	* @returns {module:qm.RecSet} The record set containing the join records.
	*/
	//# exports.RecSet.prototype.join = function (joinName, sampleSize) {};
	JsDeclareFunction(join);

	//!- `aggrsJSON = rs.aggr()` -- returns an object where keys are aggregate names and values are JSON serialized aggregate values of all the aggregates contained in the records set
	//!- `aggr = rs.aggr(aggrQueryJSON)` -- computes the aggregates based on the `aggrQueryJSON` parameter JSON object. If only one aggregate is involved and an array of JSON objects when more than one are returned.
	JsDeclareFunction(aggr);
    
	//!- `rs = rs.trunc(limit_num)` -- truncate to first `limit_num` record and return self.
	//!- `rs = rs.trunc(limit_num, offset_num)` -- truncate to `limit_num` record starting with `offset_num` and return self.
	/**
	* Truncates the first records.
	* @param {number} limit_num - How many records to truncate.
	* @param {number} [offset_num] - Where to start to truncate.
	* @returns {module:qm.RecSet} Self.
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
	//# exports.RecSet.prototype.trunc = function (limit_num, offset_num) {};
	JsDeclareFunction(trunc);

	//!- `rs2 = rs.sample(num)` -- create new record set by randomly sampling `num` records.
	/**
	* Creates a sample of records of the record set.
	* @param {number} num - The number of records in the sample.
	* @returns {module:qm.RecSet} A record set containing the sample records.
	*/
	//# exports.RecSet.prototype.sample = function (num) {};
	JsDeclareFunction(sample);

	//!- `rs = rs.shuffle(seed)` -- shuffle order using random integer seed `seed`. Returns self.
	/**
	* Shuffles the order of records in the record set.
	* @param {number} [seed] - Integer.
	* @returns {module:qm.RecSet} Self.
	*/
	//# exports.RecSet.prototype.shuffle = function (seed) {};
	JsDeclareFunction(shuffle);

	//!- `rs = rs.reverse()` -- reverse record order. Returns self.
	/**
	* It reverses the record order.
	* @returns {module:qm.RecSet} Self. Records are in reversed order.
	*/
	//# exports.RecSet.prototype.reverse = function () {};
	JsDeclareFunction(reverse);

	//!- `rs = rs.sortById(asc)` -- sort records according to record id; if `asc > 0` sorted in ascending order. Returns self.
	/**
	* Sorts the records according to record id.
	* @param {number} [asc=1] - If asc > 0, it sorts in ascending order. Otherwise, it sorts in descending order.  
	* @returns {module:qm.RecSet} Self. Records are sorted according to record id and asc.
	*/
	//# exports.RecSet.prototype.sortById = function (asc) {}; 
	JsDeclareFunction(sortById);

	//!- `rs = rs.sortByFq(asc)` -- sort records according to weight; if `asc > 0` sorted in ascending order. Returns self.
	JsDeclareFunction(sortByFq);

	//!- `rs = rs.sortByField(fieldName, asc)` -- sort records according to value of field `fieldName`; if `asc > 0` sorted in ascending order (default is desc). Returns self.
	/**
	* Sorts the records according to a specific record field.
	* @param {string} fieldName - The field by which the sort will work.
	* @param {number} [arc=-1] - if asc > 0, it sorts in ascending order. Otherwise, it sorts in descending order.
	* @returns {module:qm.RecSet} Self. Records are sorted according to fieldName and arc.
	*/
	//# exports.RecSet.prototype.sortByField = function (fieldName, asc) {};
	JsDeclareFunction(sortByField);

	//!- `rs = rs.sort(comparatorCallback)` -- sort records according to `comparator` callback. Example: rs.sort(function(rec,rec2) {return rec.Val < rec2.Val;} ) sorts rs in ascending order (field Val is assumed to be a num). Returns self.
	/**
	* Sorts the records according to the given callback function.
	* @param {function} callback - The function used to sort the records. It takes two parameters:
	* <br>1. rec - The first record.
	* <br>2. rec2 - The second record.
	* <br>It returns a boolean object.
	* @returns {module:qm.RecSet} Self. The records are sorted according to the callback function.
	* @example
	* // import qm module
	* qm = require('qminer');
	* // construct a new record set of movies (one field is it's Rating)
	* var rs = //TODO
	* // sort the records by their rating
	* rs.sort(function (rec, rec2) { return rec.Rating < rec2.Rating ;});
	*/
	//# exports.RecSet.prototype.sort = function (callback) {};
	JsDeclareFunction(sort);

	//!- `rs = rs.filterById(minId, maxId)` -- keeps only records with ids between `minId` and `maxId`. Returns self.
	/**
	* Keeps only records with ids between two values.
	* @param {number} [minId] - The minimum id.
	* @param {number} [maxId] - The maximum id.
	* @returns {module:qm.RecSet} Self. 
	* <br>1. Contains only the records of the original with ids between minId and maxId, if parameters are given.
	* <br>2. Contains all the records of the original, if no parameter is given.
	*/
	//# exports.RecSet.prototype.filterById = function (minId, maxId) {};
	JsDeclareFunction(filterById);
	
	//!- `rs = rs.filterByFq(minFq, maxFq)` -- keeps only records with weight between `minFq` and `maxFq`. Returns self.
	JsDeclareFunction(filterByFq);

	//!- `rs = rs.filterByField(fieldName, minVal, maxVal)` -- keeps only records with numeric value of field `fieldName` between `minVal` and `maxVal`. Returns self.
	//!- `rs = rs.filterByField(fieldName, minTm, maxTm)` -- keeps only records with value of time field `fieldName` between `minVal` and `maxVal`. Returns self.
	//!- `rs = rs.filterByField(fieldName, str)` -- keeps only records with string value of field `fieldName` equal to `str`. Returns self.
	/**
	* Keeps only the records with a specific value of some field.
	* @param {string} fieldName - The field by which the records will be filtered.
	* @param {(string | number)} minVal -  
	* <br>1. Is a string, if the field type is a string. The exact string to compare.
	* <br>2. Is a number, if the field type is a number. The minimal value for comparison.
	* <br>3. TODO Time field
	* @param {number} maxVal - Only in combination with minVal for non-string fields. The maximal value for comparison.
	* @returns {module:qm.RecSet} Self. Containing only the records with the fieldName value between minVal and maxVal. If the fieldName type is string,
	* it contains only the records with fieldName equal to minVal.
	*/
	//# exports.RecSet.prototype.filterByField = function (fieldName, minVal, maxVal) {};
	JsDeclareFunction(filterByField);

	//!- `rs = rs.filter(filterCallback)` -- keeps only records that pass `filterCallback` function. Returns self.
	/**
	* Keeps only the records that pass the callback function.
	* @param {function} callback - The filter function. It takes one parameter and return a boolean object.
	* @returns {module:qm.RecSet} Self. Containing only the record that pass the callback function.
	* @example
	* // import qm module
	* qm = require('qminer');
	* // construct a record set of kitchen appliances
	* var rs = //TODO
	* // filter by the field price
	* rs.filter(function (rec) { return rec.Price > 10000; }); // keeps only the records, where their Price is more than 10000
	*/
	//# exports.RecSet.prototype.filter = function (callback) {}; 
	JsDeclareFunction(filter);

	//!- `rsArr = rs.split(splitterCallback)` -- split records according to `splitter` callback. Example: rs.split(function(rec,rec2) {return (rec2.Val - rec2.Val) > 10;} ) splits rs in whenever the value of field Val increases for more than 10. Result is an array of record sets. 
	/**
	* Splits the record set into smaller record sets.
	* @param {function} callback - The splitter function. It takes two parameters (records) and returns a boolean object.
	* @returns {Array.<module:qm.RecSet>} An array containing the smaller record sets. The records are split according the callback function.
	*/
	//# exports.RecSet.prototype.split = function (callback) {};
	JsDeclareFunction(split);

	//!- `rs = rs.deleteRecords(rs2)` -- delete from `rs` records that are also in `rs2`. Returns self.
	/**
	* Deletes the records, that are also in the other record set.
	* @param {module:qm.RecSet} rs - The other record set.
	* @returns {module:qm.RecSet} Self. Contains only the records, that are not in rs.
	*/
	//# exports.RecSet.prototype.deleteRecords = function (rs) {}; 
	JsDeclareFunction(deleteRecords);

	//!- `objsJSON = rs.toJSON()` -- provide json version of record set, useful when calling JSON.stringify
	/**
	* Returns the record set as a JSON.
	* @returns {Object} The record set as a JSON.
	*/
	//# exports.RecSet.prototype.toJSON = function () {};
	JsDeclareFunction(toJSON);

	//!- `rs = rs.each(callback)` -- iterates through the record set and executes the callback function `callback` on each element. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Returns self. Examples:
	//!  - `rs.each(function (rec) { console.log(JSON.stringify(rec)); })`
	//!  - `rs.each(function (rec, idx) { console.log(JSON.stringify(rec) + ', ' + idx); })`
	/**
	* Executes a function on each record in record set.
	* @param {function} callback - Function to be executed. It takes two parameters:
	* <br>rec - The current record.
	* <br>[idx] - The index of the current record.
	* @returns {module:qm.RecSet} Self.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a record set with some people with fields Name and Gender
	* var rs = //TODO
	* // change the gender of all records to "Extraterrestrial"
	* rs.each(function (rec) { rec.Gender = "Extraterrestrial"; });
	*/
	//# exports.RecSet.prototype.each = function (callback) {}
	JsDeclareFunction(each);

	//!- `arr = rs.map(callback)` -- iterates through the record set, applies callback function `callback` to each element and returns new array with the callback outputs. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Examples:
	//!  - `arr = rs.map(function (rec) { return JSON.stringify(rec); })`
	//!  - `arr = rs.map(function (rec, idx) {  return JSON.stringify(rec) + ', ' + idx; })`
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
	//# exports.RecSet.prototype.map = function (callback) {}
	JsDeclareFunction(map);

	//!- `rs3 = rs.setIntersect(rs2)` -- returns the intersection (record set) `rs3` between two record sets `rs` and `rs2`, which should point to the same store.
	/**
	* Creates the set intersection of two record sets.
	* @param {module:qm.RecSet} rs - The other record set.
	* @returns {module:qm.RecSet} The intersection of the two record sets.
	*/
	//# exports.RecSet.prototype.setIntersect = function (rs) {};
	JsDeclareFunction(setIntersect);

	//!- `rs3 = rs.setUnion(rs2)` -- returns the union (record set) `rs3` between two record sets `rs` and `rs2`, which should point to the same store.
	/**
	* Creates the set union of two record sets.
	* @param {module:qm.RecSet} rs - The other record set.
	* @returns {module:qm.RecSet} The union of the two record sets.
	*/
	//# exports.RecSet.prototype.setUnion = function (rs) {};
	JsDeclareFunction(setUnion);

	//!- `rs3 = rs.setDiff(rs2)` -- returns the set difference (record set) `rs3`=`rs`\`rs2`  between two record sets `rs` and `rs1`, which should point to the same store.
	/**
	* Creates the set difference between two record sets.
	* @param {module:qm.RecSet} rs - The other record set.
	* @returns {module:qm.RecSet} The difference between the two record sets.
	*/
	//# exports.RecSet.prototype.setDiff = function (rs) {}; 
	JsDeclareFunction(setDiff);

	//!- `vec = rs.getVector(fieldName)` -- gets the `fieldName` vector - the corresponding field type must be one-dimensional, e.g. float, int, string,...
	/**
	* Creates a vector containing the field values of records.
	* @param {string} fieldName - The field from which to take the values. It's type must be one-dimensional, e.g. float, int, string,...
	* @returns {module:la.Vector} The vector containing the field values of records. The type it contains is dependant of the field type.
	*/
	//# exports.RecSet.prototype.getVector = function (fieldName) {}; 
	JsDeclareFunction(getVector);

	//!- `vec = rs.getMatrix(fieldName)` -- gets the `fieldName` matrix - the corresponding field type must be float_v or num_sp_v
	/**
	* Creates a vector containing the field values of records.
	* @param {string} fieldName - The field from which to take the values. It's type must be numeric, e.g. float, int, float_v, num_sp_v,...
	* @returns {(module:la.Matrix|module:la.SparseMatrix)} The matrix containing the field values of records.
	*/
	//# exports.RecSet.prototype.getMatrix = function (fieldName) {};
	JsDeclareFunction(getMatrix);
	
	//!- `storeName = rs.store` -- store of the records
	/**
	* Returns the store, where the records in the record set are stored.
	*/
	//# exports.RecSet.prototype.store = undefined;
	JsDeclareProperty(store);

	//!- `len = rs.length` -- number of records in the set
	/**
	* Returns the number of records in record set.
	*/
	//# exports.RecSet.prototype.length = undefined;
	JsDeclareProperty(length);

	//!- `bool = rs.empty` -- `bool = true` when record set is empty
	/**
	* Checks if the record set is empty. If the record set is empty, then it returns true. Otherwise, it returns false.
	*/
	//# exports.RecSet.prototype.empty = undefined;
	JsDeclareProperty(empty);

	//!- `bool =  rs.weighted` -- `bool = true` when records in the set are assigned weights
	/**
	* Checks if the record set is weighted. If the record set is weighted, then it returns true. Otherwise, it returns false.
	*/
	//# exports.RecSet.prototype.weighted = undefined;
	JsDeclareProperty(weighted);

	//!- `rec = rs[n]` -- return n-th record from the record set
	JsDeclIndexedProperty(indexId);
};

///////////////////////////////
// NodeJs QMiner Store Iterator
//! 
//! ### Store iterator
//! 
/**
* Store Iterator (factory pattern)
* @namespace
* @example
* // import qm module
* qm = require('qminer');
* // factory based construction with store.forwardIter
* var iter = store.forwardIter;
*/
//# exports.Iterator = function () {};
class TNodeJsStoreIter: public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	// Node framework
	static v8::Persistent<v8::Function> Constructor;
public:
	// Node framework 
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr ClassId;

	// C++ wrapped object
	TWPt<TQm::TStore> Store;
	TQm::PStoreIter Iter;	
	TNodeJsRec* JsRec;
	// placeholder for last object
	v8::Persistent<v8::Object> RecObj;
	// Object that knows if Base is valid
	PNodeJsBaseWatcher Watcher;
	

	// C++ constructors
	TNodeJsStoreIter(const TWPt<TQm::TStore>& _Store, const TQm::PStoreIter& _Iter, PNodeJsBaseWatcher& _Watcher) : Store(_Store), Iter(_Iter), JsRec(nullptr), Watcher(_Watcher) {}
	
public:
	
    // delete placeholder
    ~TNodeJsStoreIter() { RecObj.Reset(); }
	
    //!
	//! **Functions and properties:**
	//!   
	//!- `bool = iter.next()` -- moves to the next record or returns false if no record left; must be called at least once before `iter.rec` is available
	/**
	* Moves to the next record.
	* @returns {boolean} 
	* <br>1. True, if the iteration successfully moves to the next record.
	* <br>2. False, if there is no record left.
	*/
	//# exports.Iterator.prototype.next = function () {};
	JsDeclareFunction(next);

	//!- `store = iter.store` -- get the store
	/**
	* Gives the store of the iterator.
	*/
	//# exports.Iterator.prototype.store = undefined;
	JsDeclareProperty(store);

	//!- `rec = iter.record` -- get current record; reuses JavaScript record wrapper, need to call `rec.$clone()` on it to if there is any wish to store intermediate records.
	/**
	* Gives the current record.
	*/
	//# exports.Iterator.prototype.record = undefined;
	JsDeclareProperty(record);
};

///////////////////////////////
// NodeJs QMiner Record Filter
class TJsRecFilter {
private:
	TWPt<TQm::TStore> Store;
	// Callbacks
	v8::Persistent<v8::Function> Callback;

public:
	~TJsRecFilter(){
		Callback.Reset();
	}
	TJsRecFilter(TWPt<TQm::TStore> _Store, v8::Handle<v8::Function> _Callback) : Store(_Store) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);
		// set persistent object
		Callback.Reset(Isolate, _Callback);
	}
	
	bool operator()(const TUInt64IntKd& RecIdWgt) const;
};

///////////////////////////////
// NodeJs QMiner Record Filter (record splitter, record comparator)
class TJsRecPairFilter {
private:
	TWPt<TQm::TStore> Store;
	// Callbacks
    v8::Persistent<v8::Function> Callback;
public:
	~TJsRecPairFilter(){
		Callback.Reset();
	}
	TJsRecPairFilter(TWPt<TQm::TStore> _Store, v8::Handle<v8::Function> _Callback) : Store(_Store) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);
		// set persistent object
		Callback.Reset(Isolate, _Callback);
	}

	bool operator()(const TUInt64IntKd& RecIdWgt1, const TUInt64IntKd& RecIdWgt2) const;
};

///////////////////////////////
// NodeJs QMiner Index Key
//!
//! ### Index key
//!
class TNodeJsIndexKey: public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
    // Node framework
    static v8::Persistent<v8::Function> Constructor;
public:
	// Node framework
    static void Init(v8::Handle<v8::Object> exports);
	static const TStr ClassId;
	// C++ wrapped object
	TWPt<TQm::TStore> Store;
	TQm::TIndexKey IndexKey;
	// Object that knows if Base is valid
	PNodeJsBaseWatcher Watcher;
    // C++ constructors
	TNodeJsIndexKey(const TWPt<TQm::TStore>& _Store, const TQm::TIndexKey& _IndexKey, PNodeJsBaseWatcher& _Watcher) :
		Store(_Store), IndexKey(_IndexKey), Watcher(_Watcher) { }
    
public:
    //!
	//! **Functions and properties:**
	//!   
	//!- `store = key.store` -- gets the key's store
	JsDeclareProperty(store);
	//!- `keyName = key.name` -- gets the key's name
	JsDeclareProperty(name);
	//!- `strArr = key.vocabulary` -- gets the array of words (as strings) in the vocabulary
	JsDeclareProperty(vocabulary);
	//!- `strArr = key.fq` -- gets the array of weights (as ints) in the vocabulary
	JsDeclareProperty(fq);
};


///////////////////////////////////////////////
/// Javscript Function Feature Extractor.
//-
//- ## Javascript Feature Extractor
//-
class TNodeJsFuncFtrExt : public TQm::TFtrExt {
private:
	// private constructor
	TNodeJsFuncFtrExt(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal,
        const v8::Handle<v8::Function> _Fun, v8::Isolate* Isolate);
	~TNodeJsFuncFtrExt() { Fun.Reset(); }
public:
	// public smart pointer
	static TQm::PFtrExt NewFtrExt(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal,
        const v8::Handle<v8::Function>& Fun, v8::Isolate* Isolate);
private:
	// Core part
	TInt Dim;
	TStr Name;
	// callback
	v8::Persistent<v8::Function> Fun;

	double ExecuteFunc(const TQm::TRec& FtrRec) const;
	void ExecuteFuncVec(const TQm::TRec& FtrRec, TFltV& Vec) const;
    // will throw exception (saving, loading not supported)
	TNodeJsFuncFtrExt(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal);
    // will throw exception (saving, loading not supported)
	TNodeJsFuncFtrExt(const TWPt<TQm::TBase>& Base, TSIn& SIn);
public:
    // will throw exception (saving, loading not supported)
	static TQm::PFtrExt New(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal);
    // will throw exception (saving, loading not supported)
	static TQm::PFtrExt Load(const TWPt<TQm::TBase>& Base, TSIn& SIn);
	void Save(TSOut& SOut) const;

	TStr GetNm() const { return Name; }
	int GetDim() const { return Dim; }
	TStr GetFtr(const int& FtrN) const { return TStr::Fmt("%s[%d]", GetNm().CStr(), FtrN) ; }

	void Clr() { };
	bool Update(const TQm::TRec& Rec) { return false; }
	void AddSpV(const TQm::TRec& Rec, TIntFltKdV& SpV, int& Offset) const;
	void AddFullV(const TQm::TRec& Rec, TFltV& FullV, int& Offset) const;
	void InvFullV(const TFltV& FullV, int& Offset, TFltV& InvV) const {
		throw TExcept::New("Not implemented yet!", "TJsFuncFtrExt::InvFullV"); }

	// flat feature extraction
	void ExtractFltV(const TQm::TRec& FtrRec, TFltV& FltV) const;
	// feature extractor type name
	static TStr GetType() { return "jsfunc"; }
};

///////////////////////////////
// NodeJs QMiner Feature Space

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
//# exports.FeatureSpace = function (base, extractors) {};

class TNodeJsFtrSpace : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	// Node framework
	static v8::Persistent<v8::Function> constructor;

	TQm::PFtrSpace FtrSpace;

	TNodeJsFtrSpace(const TQm::PFtrSpace& FtrSpace);
	TNodeJsFtrSpace(const TWPt<TQm::TBase> Base, TSIn& SIn);

	static v8::Local<v8::Object> WrapInst(const v8::Local<v8::Object> Obj, const TQm::PFtrSpace& FtrSpace);
	static v8::Local<v8::Object> WrapInst(const v8::Local<v8::Object> Obj, const TWPt<TQm::TBase> Base, TSIn& SIn);

public:
	static v8::Local<v8::Object> New(const TQm::PFtrSpace& FtrSpace);
	static v8::Local<v8::Object> New(const TWPt<TQm::TBase> Base, TSIn& SIn);

	// Node framework
	static void Init(v8::Handle<v8::Object> exports);

	TQm::PFtrSpace GetFtrSpace() { return FtrSpace; }
	//!
	//! **Constructor:**
	//!
	//!- `fsp = new qm.FeatureSpace(base, fin)` -- construct a new feature space by providing the base and input stream object
	//!- `fsp = new qm.FeatureSpace(base, params)` -- construct a new feature space by providing the base and parameter JSON object
	JsDeclareFunction(New);
	//!
	//! **Functions and properties:**
	//!
    
	//!- `num = fsp.dim` -- dimensionality of feature space
	/**
	* Returns the dimension of the feature space.
	*/
	//# exports.FeatureSpace.prototype.dim = undefined;
    JsDeclareProperty(dim);
    
	//!- `num_array = fsp.dims` -- dimensionality of feature space for each of the internal feature extarctors
	/**
	* Returns an array of the dimensions of each feature extractor in the feature space.
	*/
	//# exports.FeatureSpace.prototype.dims = undefined;
    JsDeclareProperty(dims);

    //!- `fout = fsp.save(fout)` -- serialize feature space to `fout` output stream. Returns `fout`.
	/**
	* Serialize the feature space to an output stream.
	* @param {module:fs.FOut} fout - The output stream.
	* @returns {module:fs.FOut} The output stream.
	*/
	//# exports.FeatureSpace.prototype.save = function (fout) {};
    JsDeclareFunction(save);

	//!- `fsp = fsp.addFeatureExtractor(objJson)` -- add a feature extractor parametrized by `objJson`
	/**
	* Adds a new feature extractor to the feature space.
	* @param {Object} obj - The added feature extracture.
	* @returns {module:qm.FeatureSpace} Self.
	*/
	//# exports.FeatureSpace.prototype.addFeatureExtractor = function (obj) {};
	JsDeclareFunction(addFeatureExtractor);

    //!- `fsp = fsp.updateRecord(rec)` -- update feature space definitions and extractors
    //!     by exposing them to record `rec`. Returns self. For example, this can update the vocabulary
    //!     used by bag-of-words extractor by taking into account new text.
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
	//# exports.FeatureSpace.prototype.updateRecord = function (rec) {};
	JsDeclareFunction(updateRecord);

    //!- `fsp = fsp.updateRecords(rs)` -- update feature space definitions and extractors
    //!     by exposing them to records from record set `rs`. Returns self. For example, this can update
    //!     the vocabulary used by bag-of-words extractor by taking into account new text.
	/**
	* Updates the feature space definitions and extractors by adding all the records of a record set.
	* <br> For text feature extractors, it can update it's vocabulary by taking into account the new text.
	* <br> For numeric feature extractors, it can update the minimal and maximal values used to form the normalization.
	* <br> For jsfunc feature extractors, it can update a parameter used in it's function.
	* <br> For dateWindow feature extractor, it can update the start and the end of the window period to form the normalization.
	* @param {module:qm.RecSet} rs - The record set, which updates the feature space.
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
	//# exports.FeatureSpace.prototype.updateRecords = function (rs) {};
	JsDeclareFunction(updateRecords);

	//!- `spVec = fsp.extractSparseVector(rec)` -- extracts sparse feature vector `spVec` from record `rec`
	/**
	* Creates a sparse feature vector from the given record.
	* @param {module:qm.Record} rec - The given record.
	* @returns {module:la.SparseVector} The sparse feature vector gained from rec.
	*/
	//# exports.FeatureSpace.prototype.extractSparseVector = function (rec) {}
    JsDeclareFunction(extractSparseVector);

    //!- `vec = fsp.extractVector(rec)` -- extracts feature vector `vec` from record  `rec`
	/**
	* Creates a feature vector from the given record.
	* @param {module:qm.Record} rec - The given record.
	* @returns {module:la.Vector} The feature vector gained from rec.
	*/
	//# exports.FeatureSpace.prototype.extractVector = function (rec) {};
	JsDeclareFunction(extractVector);
    
	//!- `vec = fsp.invertFeatureVector(ftrVec)` -- performs the inverse operation of ftrVec, returns the results in
    //!- 	an array
	/**
	* Performs the inverse operation of ftrVec. Works only for numeric feature extractors.
	* @param {(module:qm.Vector | Array.<Object>)} ftr - The feature vector or an array with feature values.
	* @returns {module:qm.Vector} The inverse of ftr as vector.
	*/
	//# exports.FeatureSpace.prototype.invertFeatureVector = function (ftr) {};
	JsDeclareFunction(invertFeatureVector);

	//!- `val = fsp.invertFeature(ftrIdx, val)` -- inverts a single feature using the feature
	//!- 	extractor on index `ftrIdx`
	/**
	* Calculates the inverse of a single feature using a specific feature extractor.
	* @param {number} idx - The index of the specific feature extractor.
	* @param {Object} val - The value to be inverted.
	* @returns {Object} The inverse of val using the feature extractor with index idx.
	*/
	//# exports.FeatureSpace.prototype.invertFeature = function (idx, val) {};
	JsDeclareFunction(invertFeature);

    //!- `spMat = fsp.extractSparseMatrix(rs)` -- extracts sparse feature vectors from
    //!     record set `rs` and returns them as columns in a sparse matrix `spMat`.
	/**
	* Extracts the sparse feature vectors from the record set and returns them as columns of the sparse matrix.
	* @param {module:qm.RecSet} rs - The given record set.
	* @returns {module:la.SparseMatrix} The sparse matrix, where the i-th column is the sparse feature vector of the i-th record in rs.
	*/
	//# exports.FeatureSpace.prototype.extractSparseMatrix = function (rs) {};
	JsDeclareFunction(extractSparseMatrix);

    //!- `mat = fsp.extractMatrix(rs)` -- extracts feature vectors from
    //!     record set `rs` and returns them as columns in a matrix `mat`.
	/**
	* Extracts the feature vectors from the recordset and returns them as columns of a dense matrix.
	* @param {module:qm.RecSet} rs - The given record set.
	* @returns {module:la.Matrix} The dense matrix, where the i-th column is the feature vector of the i-th record in rs.
	*/
	//# exports.FeatureSpace.prototype.extractMatrix = function (rs) {};
    JsDeclareFunction(extractMatrix);

	//!- `name = fsp.getFeatureExtractor(ftrExtractor)` -- returns the name `name` (string) of `ftrExtractor`-th feature extractor in feature space `fsp`
	/**
	* Gives the name of feature extractor at given position.
	* @param {number} idx - The index of the feature extractor in feature space (zero based).
	* @returns {String} The name of the feature extractor at position idx.
	*/
	//# exports.FeatureSpace.prototype.getFeatureExtractor = function (idx) {};
	JsDeclareFunction(getFeatureExtractor);

	//!- `ftrName = fsp.getFeature(idx)` -- returns the name `ftrName` (string) of `idx`-th feature in feature space `fsp`
	/**
	* Gives the name of the feature at the given position.
	* @param {number} idx - The index of the feature in feature space (zero based).
	* @returns {String} The name of the feature at the position idx.
	*/
	//# exports.FeatureSpace.prototype.getFeature = function (idx) {};
	JsDeclareFunction(getFeature);
    
	//!- `out_vec = fsp.filter(in_vec, ftrExtractor)` -- filter the vector to keep only elements from the feature extractor ID `ftrExtractor`
    //!- `out_vec = fsp.filter(in_vec, ftrExtractor, keepOffset)` -- filter the vector to keep only elements from the feature extractor ID `ftrExtractor`.
    //!     If `keepOffset` == `true`, then original feature ID offset is kept, otherwise the first feature of `ftrExtractor` starts with position 0.
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
	//# exports.FeatureSpace.prototype.filter = function (vec, idx, keepOffset) {};
    JsDeclareFunction(filter);

	//!- `strArr = fsp.extractStrings(rec)` -- use feature extractors to extract string
    //!     features from record `rec` (e.g. words from string fields); results are returned
    //!     as a string array
    JsDeclareFunction(extractStrings);

private:
    static TQm::PFtrExt NewFtrExtFromFunc(const TWPt<TQm::TBase>& Base, v8::Local<v8::Object>& Settings, v8::Isolate* Isolate) {
    	PJsonVal ParamVal = TNodeJsUtil::GetObjProps(Settings);
    	v8::Handle<v8::Function> Func = v8::Handle<v8::Function>::Cast(Settings->Get(v8::String::NewFromUtf8(Isolate, "fun")));
    	return TNodeJsFuncFtrExt::NewFtrExt(Base, ParamVal, Func, Isolate);
    }
};

#endif
