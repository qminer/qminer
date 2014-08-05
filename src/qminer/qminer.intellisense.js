
// Manual override
// contains keys of variables that should not show in intellisense
intellisenseIgnore = {};
// creates global variables that can be seen in visual studio js file if this file is referenced
function _addIntellisenseVar(key, val, hideVar) {
    if (typeof hideVar == 'undefined')
        hideVar = true;
    if (hideVar) {
        intellisenseIgnore[key] = val;
    }
    eval(key + "=" + val);
}

// ignore suggest list! (all non global variables)
// locals (js)
// num
_addIntellisenseVar("_idx", "1");
_addIntellisenseVar("_rowIdx", "1");
_addIntellisenseVar("_colIdx", "1");
_addIntellisenseVar("_recSetIdx", "1");
_addIntellisenseVar("_num", "1");
_addIntellisenseVar("_num2", "1");
_addIntellisenseVar("_len", "1");
_addIntellisenseVar("_dim", "1");
_addIntellisenseVar("_cols", "1");
_addIntellisenseVar("_rows", "1");
_addIntellisenseVar("_k", "1");
_addIntellisenseVar("_limit", "1");
_addIntellisenseVar("_recId", "1");
_addIntellisenseVar("_recFq", "1");
_addIntellisenseVar("_sampleSize", "1");
_addIntellisenseVar("_minVal", "1");
_addIntellisenseVar("_maxVal", "1");
_addIntellisenseVar("_minId", "1");
_addIntellisenseVar("_maxId", "1");
_addIntellisenseVar("_minFq", "1");
_addIntellisenseVar("_maxFq", "1");
_addIntellisenseVar("_seed", "1");
_addIntellisenseVar("_joinFrequency", "1");
_addIntellisenseVar("_millis", "1");
_addIntellisenseVar("_thresh", "1");
_addIntellisenseVar("_iter", "1");
_addIntellisenseVar("_statusCode", "1");

// arrays
_addIntellisenseVar("_array", "[]");
_addIntellisenseVar("_arr", "[1]");
_addIntellisenseVar("_numArr", "[1]");
_addIntellisenseVar("_objArr", "[{}]");
_addIntellisenseVar("_strArr", "['']");
_addIntellisenseVar("_nestedArr", "[_arr]");
_addIntellisenseVar("_doubleNestedArr", "[_nestedArr]");
// bools
_addIntellisenseVar("_bool", "true");
_addIntellisenseVar("_asc", "true");
// strings
_addIntellisenseVar("_str", "''");
_addIntellisenseVar("_char", "''");
_addIntellisenseVar("_argStr", "''");
_addIntellisenseVar("_line", "''");
_addIntellisenseVar("_labelStr", "''");
_addIntellisenseVar("_message", "''");
_addIntellisenseVar("_fileName", "''");
_addIntellisenseVar("_recName", "''");
_addIntellisenseVar("_storeName", "''");
_addIntellisenseVar("_keyName", "''");
_addIntellisenseVar("_typeName", "''");
_addIntellisenseVar("_saName", "''");
_addIntellisenseVar("_joinName", "''");
_addIntellisenseVar("_ftrName", "''");
_addIntellisenseVar("_scriptNm", "''");
_addIntellisenseVar("_scriptFNm", "''");
_addIntellisenseVar("_prefixStr", "''");
_addIntellisenseVar("_dirName", "''");
_addIntellisenseVar("_alAnswer", "''");
_addIntellisenseVar("_url", "''");
_addIntellisenseVar("_mimeType", "''");
_addIntellisenseVar("_dataStr", "''");

// json objects
_addIntellisenseVar("_obj", "{}");
_addIntellisenseVar("_objJSON", "{}");
_addIntellisenseVar("_paramJSON", "{}");
_addIntellisenseVar("_aggrsJSON", "{}");
_addIntellisenseVar("_aggrQueryJSON", "{}");
_addIntellisenseVar("_dataJSON", "{}");
_addIntellisenseVar("_tmJSON", "{year:_num, month:_num, day:_num, hour:_num, minute:_num, second:_num}");

// other structures
_addIntellisenseVar("_sortRes", "{ vec: _vec, perm: _intVec }");
_addIntellisenseVar("_vecCtrParam", "{ vals: 1, mxvals: 1 }");
_addIntellisenseVar("_svdRes", "{ U: _mat, V: _mat, s: _vec }");
_addIntellisenseVar("_storeDef", "{ id: '', name: '', fields: [], joins: [], keys: []}");
_addIntellisenseVar("_trigger", "{ onAdd: function(_rec) {}, onUpdate: function(_rec) {}, onDelete: function(_rec) {}}");
_addIntellisenseVar("_name", "{ name:_str, onAdd: function(_rec) {}, onUpdate: function(_rec) {}, onDelete: function(_rec) {}}");
_addIntellisenseVar("_field", "{ id: 1, name: '', type: '', nullable: false, internal: false, primary: false}");
_addIntellisenseVar("_mapCallback", "function (_rec, _idx) {}");
_addIntellisenseVar("_filterCallback", "function (_rec) { return _bool}");
_addIntellisenseVar("_comparatorCallback", "function (_rec, _rec2) { return _bool}");
_addIntellisenseVar("_langOptionsJson", "{stemmer: _strArr , stopwords: _strArr}");
_addIntellisenseVar("_scoreArr", "{}");
_addIntellisenseVar("_perceptronParam", "{w: _vec , b: _num}");
_addIntellisenseVar("_fileInfoJson", "{createTime:_str, lastAccessTime:_str, lastWriteTime:_str, size:_num}");
_addIntellisenseVar("_httpStrSuccessCallback", "function (_str) {}");
_addIntellisenseVar("_httpJsonSuccessCallback", "function (_objJSON) {}");
_addIntellisenseVar("_httpErrorCallback", "function (_message) {}");

_addIntellisenseVar("_httpRequestCallback", "function (_httpRequest,_httpResponse) {}");
_addIntellisenseVar("_httpRequest", "{host:_str, connection:_str, cache-control:_str, accept:_str, user-agent:_str, accept-language:_str, method:_str, scheme:_str, path:_str, args:_obj, data:_str, params:_obj}");




//// globals like `la` and `qm` C++ (without _): do nothing here, add them to procintelli.py
// special case
_addIntellisenseVar("_addIntellisenseVar", "_addIntellisenseVar"); // eval doesn't change _addIntellisenseVar, we just add the variable "_addIntellisenseVar" to ignore list
_addIntellisenseVar("intellisenseIgnore", "intellisenseIgnore"); // eval doesn't change intellisenseIgnore, we just add the variable "intellisenseIgnore" to ignore list

// locals (just hide, do not overwrite) (c++)
intellisenseIgnore["_vec"] = "{}";
intellisenseIgnore["_spVec"] = "{}";
intellisenseIgnore["_intVec"] = "{}";
intellisenseIgnore["_mat"] = "{}";
intellisenseIgnore["_spMat"] = "{}";
intellisenseIgnore["_rec"] = "{}"; // record
intellisenseIgnore["_rs"] = "{}"; // record set
intellisenseIgnore["_store"] = "{}";
intellisenseIgnore["_key"] = "{}";
intellisenseIgnore["_tm"] = "{}";
intellisenseIgnore["_svmModel"] = "{}";
intellisenseIgnore["_nnModel"] = "{}";
intellisenseIgnore["_recLinRegModel"] = "{}";
intellisenseIgnore["_htModel"] = "{}";
intellisenseIgnore["_fsp"] = "{}";
intellisenseIgnore["_twitterParser"] = "{}";
intellisenseIgnore["_twitterDef"] = "{}";
intellisenseIgnore["_alModel"] = "{}";
intellisenseIgnore["_batchModel"] = "{}";
intellisenseIgnore["_fin"] = "{}";
intellisenseIgnore["_fout"] = "{}";
intellisenseIgnore["_hashTable"] = "{}";
intellisenseIgnore["_httpResponse"] = "{}";
intellisenseIgnore["_lloydModel"] = "{}";
intellisenseIgnore["_model"] = "{}";
intellisenseIgnore["_perceptronModel"] = "{}";
intellisenseIgnore["_result"] = "{}";
intellisenseIgnore["_ridgeRegressionModel"] = "{}";
intellisenseIgnore["_sw"] = "{}";

_addIntellisenseVar("_vec2", "_vec");
_addIntellisenseVar("_vec3", "_vec");
_addIntellisenseVar("_inVec", "_vec");
_addIntellisenseVar("_outVec", "_vec");
_addIntellisenseVar("_valVec", "_vec");
_addIntellisenseVar("_spVec2", "_spVec");
_addIntellisenseVar("_spVec3", "_spVec");
_addIntellisenseVar("_intVec2", "_intVec");
_addIntellisenseVar("_intVec3", "_intVec");
_addIntellisenseVar("_idxVec", "_intVec");
_addIntellisenseVar("_idVec", "_intVec");
_addIntellisenseVar("_rowIdxVec", "_intVec");
_addIntellisenseVar("_colIdxVec", "_intVec");
_addIntellisenseVar("_mat2", "_mat");
_addIntellisenseVar("_mat3", "_mat");
_addIntellisenseVar("_spMat2", "_spMat");
_addIntellisenseVar("_spMat3", "_spMat");
_addIntellisenseVar("_rs2", "_rs");
_addIntellisenseVar("_rs3", "_rs");
_addIntellisenseVar("_joinRec", "_rec");
_addIntellisenseVar("_tm2", "_tm");

require = function (libName) {
    if (libName === 'analytics.js') return _analytics;
    if (libName === 'utilities.js') return _utilities;
    if (libName === 'assert.js') return _assert;
    if (libName === 'twitter.js') return _twitter;
    if (libName === 'time.js') return _tm;
};

intellisenseIgnore["_analytics"] = "{}";
intellisenseIgnore["_utilities"] = "{}";
intellisenseIgnore["_assert"] = "{}";
intellisenseIgnore["_twitter"] = "{}";
intellisenseIgnore["_tm"] = "{}";


// implement ignore
intellisense.addEventListener('statementcompletion', function (event) {
    if (event.targetName === "this") return;
    event.items = event.items.filter(function (item) {
        return !intellisenseIgnore.hasOwnProperty(item.name);
    });
});

// Autogenerated below

var _result = new function () {
	this.report = function () {
	/// <signature>
	/// <summary> prints basic report on to the console</summary>
	/// <returns value =""/>
	/// </signature>

	};

	this.reportCSV = function () {
	/// <signature>
	/// <summary> prints CSV output to the `fout` output stream</summary>
	/// <param name="_fout" value="_fout">param</param>
	/// <returns value =""/>
	/// </signature>

	};

}

var _store = new function () {
	/// <field name = "joins" value = "_objArr"> array of all the join names</field>
	this.joins = _objArr;
	this.sample = function () {
	/// <signature>
	/// <summary> create a record set containing a random</summary>
	/// <param name="_sampleSize" value="_sampleSize">param</param>
	/// <returns value ="_rs"/>
	/// </signature>

	};

	this.getStreamAggr = function () {
	/// <signature>
	/// <summary> returns a stream aggregate `sa` whose name is `saName`</summary>
	/// <param name="_saName" value="_saName">param</param>
	/// <returns value ="_sa"/>
	/// </signature>

	};

	this.newRec = function () {
	/// <signature>
	/// <summary> creates new record `rec` by (JSON) value `recordJson` (not added to the store)</summary>
	/// <param name="_recordJson" value="_recordJson">param</param>
	/// <returns value ="_rec"/>
	/// </signature>

	};

	this.newRecSet = function () {
	/// <signature>
	/// <summary> creates new record set from an integer vector record IDs `idVec` (type la.newIntVec);</summary>
	/// <param name="_idVec" value="_idVec">param</param>
	/// <returns value ="_rs"/>
	/// </signature>

	};

	/// <field name = "backwardIter" value = "_iter"> returns iterator for iterating over the store from end to start</field>
	this.backwardIter = _iter;
	/// <field name = "forwardIter" value = "_iter"> returns iterator for iterating over the store from start to end</field>
	this.forwardIter = _iter;
	this.field = function () {
	/// <signature>
	/// <summary> get details of field named `fieldName`</summary>
	/// <param name="_fieldName" value="_fieldName">param</param>
	/// <returns value ="_field"/>
	/// </signature>

	};

	this.add = function () {
	/// <signature>
	/// <summary> add record `rec` to the store and return its ID `recId`</summary>
	/// <param name="_rec" value="_rec">param</param>
	/// <returns value ="_recId"/>
	/// </signature>

	};

	this.rec = function () {
	/// <signature>
	/// <summary> get record named `recName`;</summary>
	/// <param name="_recName" value="_recName">param</param>
	/// <returns value ="_rec"/>
	/// </signature>

	};

	/// <field name = "empty" value = "_bool"> `bool = true` when store is empty</field>
	this.empty = _bool;
	/// <field name = "keys" value = "_objArr"> array of all the [index keys](#index-key) objects</field>
	this.keys = _objArr;
	this.addStreamAggr = function () {
	/// <signature>
	/// <summary> creates a new stream aggregate `sa` and registers it to the store</summary>
	/// <param name="_param" value="_param">param</param>
	/// <returns value ="_sa"/>
	/// </signature>

	};

	this.key = function () {
	/// <signature>
	/// <summary> get [index key](#index-key) named `keyName`</summary>
	/// <param name="_keyName" value="_keyName">param</param>
	/// <returns value ="_key"/>
	/// </signature>

	};

	/// <field name = "recs" value = "_rs"> create a record set containing all the records from the store</field>
	this.recs = _rs;
	/// <field name = "name" value = "_str"> name of the store</field>
	this.name = _str;
	/// <field name = "last" value = "_rec"> last record from the store</field>
	this.last = _rec;
	this.addTrigger = function () {
	/// <signature>
	/// <summary> add `trigger` to the store triggers. Trigger is a JS object with three properties `onAdd`, `onUpdate`, `onDelete` whose values are callbacks</summary>
	/// <param name="_trigger" value="_trigger">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	/// <field name = "fields" value = "_objArr"> array of all the field descriptor JSON objects</field>
	this.fields = _objArr;
	/// <field name = "length" value = "_len"> number of records in the store</field>
	this.length = _len;
	/// <field name = "first" value = "_rec"> first record from the store</field>
	this.first = _rec;
	this.getStreamAggrNames = function () {
	/// <signature>
	/// <summary> returns the names of all stream aggregators listening on the store as an array of strings `strArr`</summary>
	/// <returns value ="_strArr"/>
	/// </signature>

	};

}

var process = new function () {
	/// <field name = "scriptNm" value = "_str"> Returns the name of the script.</field>
	this.scriptNm = _str;
	/// <field name = "scriptFNm" value = "_str"> Returns absolute script file path.</field>
	this.scriptFNm = _str;
	this.isArg = function () {
	/// <signature>
	/// <summary> returns true when `argStr` among the</summary>
	/// <param name="_argStr" value="_argStr">param</param>
	/// <returns value ="_bool"/>
	/// </signature>

	};

	/// <field name = "args" value = "_a"> array of command-line arguments</field>
	this.args = _a;
	this.stop = function () {
	/// <signature>
	/// <summary> Stops the current process.</summary>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> Stops the current process and returns `returnCode</summary>
	/// <param name="_returnCode" value="_returnCode">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	/// <field name = "sysStat" value = "_objJSON"> statistics about system and qminer process (E.g. memory consumption).</field>
	this.sysStat = _objJSON;
	this.sleep = function () {
	/// <signature>
	/// <summary> Halts execution for the given amount of milliseconds `millis`.</summary>
	/// <param name="_millis" value="_millis">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.getGlobals = function () {
	/// <signature>
	/// <summary> Returns an array of all global variable names</summary>
	/// <returns value ="_globalVarNames"/>
	/// </signature>

	};

	this.exitScript = function () {
	/// <signature>
	/// <summary> Exits the current script</summary>
	/// <returns value =""/>
	/// </signature>

	};

}

var _vec = new function () {
	this.multiply = function () {
	/// <signature>
	/// <summary>`vec2` is a vector obtained by multiplying vector `vec` with a scalar (number) `num`. Implemented for dense float vectors only.</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.at = function () {
	/// <signature>
	/// <summary> gets the value `num` of vector `vec` at index `idx`  (0-based indexing)</summary>
	/// <param name="_idx" value="_idx">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	};

	this.normalize = function () {
	/// <signature>
	/// <summary> normalizes the vector `vec` (inplace operation). Implemented for dense float vectors only.</summary>
	/// <returns value =""/>
	/// </signature>

	};

	this.outer = function () {
	/// <signature>
	/// <summary> the dense matrix `mat` is a rank-1 matrix obtained by multiplying `vec * vec2^T`. Implemented for dense float vectors only.</summary>
	/// <param name="_vec2" value="_vec2">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.toMat = function () {
	/// <signature>
	/// <summary> `mat` is a matrix with a single column that is equal to dense vector `vec`.</summary>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.pushV = function () {
	/// <signature>
	/// <summary> append vector `vec2` to vector `vec`.</summary>
	/// <param name="_vec2" value="_vec2">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.diag = function () {
	/// <signature>
	/// <summary> `mat` is a diagonal dense matrix whose diagonal equals `vec`. Implemented for dense float vectors only.</summary>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.sum = function () {
	/// <signature>
	/// <summary> return `num`: the sum of elements of vector `vec`</summary>
	/// <returns value ="_num"/>
	/// </signature>

	};

	this.unshift = function () {
	/// <signature>
	/// <summary> insert value `num` to the begining of vector `vec`. Returns the length of the modified array.</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_len"/>
	/// </signature>

	};

	this.inner = function () {
	/// <signature>
	/// <summary> `num` is the standard dot product between vectors `vec` and `vec2`. Implemented for dense float vectors only.</summary>
	/// <param name="_vec2" value="_vec2">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	};

	this.spDiag = function () {
	/// <signature>
	/// <summary> `spMat` is a diagonal sparse matrix whose diagonal equals `vec`. Implemented for dense float vectors only.</summary>
	/// <returns value ="_spMat"/>
	/// </signature>

	};

	this.print = function () {
	/// <signature>
	/// <summary> print vector in console</summary>
	/// <returns value =""/>
	/// </signature>

	};

	this.subVec = function () {
	/// <signature>
	/// <summary> gets the subvector based on an index vector `intVec` (indices can repeat, 0-based indexing)</summary>
	/// <param name="_intVec" value="_intVec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.sort = function () {
	/// <signature>
	/// <summary> `vec2` is a sorted copy of `vec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order</summary>
	/// <param name="_asc" value="_asc">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.sortPerm = function () {
	/// <signature>
	/// <summary> returns a sorted copy of the vector in `sortRes.vec` and the permutation `sortRes.perm`. `asc=true` sorts in ascending order (equivalent `sortPerm()`), `asc`=false sorts in descending order.</summary>
	/// <param name="_asc" value="_asc">param</param>
	/// <returns value ="_sortRes"/>
	/// </signature>

	};

	this.put = function () {
	/// <signature>
	/// <summary> set value of vector `vec` at index `idx` to `num` (0-based indexing)</summary>
	/// <param name="_idx" value="_idx">param</param>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.getMaxIdx = function () {
	/// <signature>
	/// <summary> returns the integer index `idx` of the maximal element in vector `vec`</summary>
	/// <returns value ="_idx"/>
	/// </signature>

	};

	/// <field name = "length" value = "_len"> integer `len` is the length of vector `vec`</field>
	this.length = _len;
	this.plus = function () {
	/// <signature>
	/// <summary>`vec3` is the sum of vectors `vec` and `vec2`. Implemented for dense float vectors only.</summary>
	/// <param name="_vec2" value="_vec2">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.norm = function () {
	/// <signature>
	/// <summary> `num` is the Euclidean norm of `vec`. Implemented for dense float vectors only.</summary>
	/// <returns value ="_num"/>
	/// </signature>

	};

	this.sparse = function () {
	/// <signature>
	/// <summary> `spVec` is a sparse vector representation of dense vector `vec`. Implemented for dense float vectors only.</summary>
	/// <returns value ="_spVec"/>
	/// </signature>

	};

	this.push = function () {
	/// <signature>
	/// <summary> append value `num` to vector `vec`. Returns `len` - the length  of the modified array</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_len"/>
	/// </signature>

	};

	this.minus = function () {
	/// <signature>
	/// <summary>`vec3` is the difference of vectors `vec` and `vec2`. Implemented for dense float vectors only.</summary>
	/// <param name="_vec2" value="_vec2">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	};

}

var _ridgeRegressionModel = new function () {
	this.compute = function () {
	/// <signature>
	/// <summary> computes the model parameters `vec2`, given</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	/// <signature>
	/// <summary> computes the model parameters `vec2`, given</summary>
	/// <param name="_spMat" value="_spMat">param</param>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.forget = function () {
	/// <signature>
	/// <summary> deletes first `n` (integer) examples from the training set</summary>
	/// <param name="_n" value="_n">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.getModel = function () {
	/// <signature>
	/// <summary> returns the parameter vector `vec` (dense vector)</summary>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.update = function () {
	/// <signature>
	/// <summary> recomputes the model</summary>
	/// <returns value =""/>
	/// </signature>

	};

	this.addupdate = function () {
	/// <signature>
	/// <summary> adds a vector `vec` and target `num` (number) to the training set and retrains the model</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.add = function () {
	/// <signature>
	/// <summary> adds a vector `vec` and target `num` (number) to the training set</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	};

}

var _assert = new function () {
	this.run = function () {
	/// <signature>
	/// <summary> prints success if this code is reached</summary>
	/// <param name="_test" value="_test">param</param>
	/// <param name="_msg" value="_msg">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.ok = function () {
	/// <signature>
	/// <summary>  `test` is boolean, prints message `msg` if test is true</summary>
	/// <param name="_test" value="_test">param</param>
	/// <param name="_msg" value="_msg">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.equals = function () {
	/// <signature>
	/// <summary>  checks if object `obj==obj2` and prints message `msg`</summary>
	/// <param name="_obj" value="_obj">param</param>
	/// <param name="_obj2" value="_obj2">param</param>
	/// <param name="_msg" value="_msg">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.exists = function () {
	/// <signature>
	/// <summary>  checks if object `obj` exists (!=null) and prints message `msg`</summary>
	/// <param name="_obj" value="_obj">param</param>
	/// <param name="_msg" value="_msg">param</param>
	/// <returns value =""/>
	/// </signature>

	};

}

var _hashTable = new function () {
	/// <field name = "keys" value = "_strArr"> array of keys (strings)</field>
	this.keys = _strArr;
	this.put = function () {
	/// <signature>
	/// <summary> add a key</summary>
	/// <param name="_key" value="_key">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> add key-dat</summary>
	/// <param name="_key" value="_key">param</param>
	/// <param name="_dat" value="_dat">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.contains = function () {
	/// <signature>
	/// <summary> does the table contain the key?</summary>
	/// <param name="_key" value="_key">param</param>
	/// <returns value ="_bool"/>
	/// </signature>

	};

	/// <field name = "vals" value = "_array"> array of values</field>
	this.vals = _array;
	this.get = function () {
	/// <signature>
	/// <summary> get data</summary>
	/// <param name="_key" value="_key">param</param>
	/// <returns value ="_dat"/>
	/// </signature>

	};

}

var _rec = new function () {
	this.delJoin = function () {
	/// <signature>
	/// <summary> deletes join record `joinRecord` from join `joinName` (string)</summary>
	/// <param name="_joinName" value="_joinName">param</param>
	/// <param name="_joinRecord" value="_joinRecord">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> deletes join record `joinRecord` from join `joinName` (string) with join frequency `joinFrequency`</summary>
	/// <param name="_joinName" value="_joinName">param</param>
	/// <param name="_joinRecord" value="_joinRecord">param</param>
	/// <param name="_joinFrequency" value="_joinFrequency">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.addJoin = function () {
	/// <signature>
	/// <summary> adds a join record `joinRecord` to join `jonName` (string)</summary>
	/// <param name="_joinName" value="_joinName">param</param>
	/// <param name="_joinRecord" value="_joinRecord">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> adds a join record `joinRecord` to join `jonName` (string) with join frequency `joinFrequency`</summary>
	/// <param name="_joinName" value="_joinName">param</param>
	/// <param name="_joinRecord" value="_joinRecord">param</param>
	/// <param name="_joinFrequency" value="_joinFrequency">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	/// <field name = "$id" value = "_recId"> returns record ID</field>
	this.$id = _recId;
	/// <field name = "$store" value = "_recStore"> returns record store</field>
	this.$store = _recStore;
	/// <field name = "$fq" value = "_recFq"> returns record frequency (used for randomized joins)</field>
	this.$fq = _recFq;
	/// <field name = "$name" value = "_recName"> returns record name</field>
	this.$name = _recName;
	this.toJSON = function () {
	/// <signature>
	/// <summary> provide json version of record, useful when calling JSON.stringify</summary>
	/// <returns value ="_objJSON"/>
	/// </signature>

	};

}

var _iter = new function () {
	/// <field name = "rec" value = "_rec"> get current record</field>
	this.rec = _rec;
	/// <field name = "store" value = "_store"> get the store</field>
	this.store = _store;
	this.next = function () {
	/// <signature>
	/// <summary> moves to the next record or returns false if no record left; must be called at least once before `iter.rec` is available</summary>
	/// <returns value ="_bool"/>
	/// </signature>

	};

}

var _fin = new function () {
	this.getCh = function () {
	/// <signature>
	/// <summary> reads a character</summary>
	/// <returns value ="_char"/>
	/// </signature>

	};

	this.peekCh = function () {
	/// <signature>
	/// <summary> peeks a character</summary>
	/// <returns value ="_char"/>
	/// </signature>

	};

	/// <field name = "length" value = "_len"> returns the length of input stream</field>
	this.length = _len;
	/// <field name = "eof" value = "_bool"> end of stream?</field>
	this.eof = _bool;
	this.readLine = function () {
	/// <signature>
	/// <summary> reads a line</summary>
	/// <returns value ="_line"/>
	/// </signature>

	};

}

var _analytics = new function () {
	this.newBatchModel = function () {
	/// <signature>
	/// <summary> learns a new batch model</summary>
	/// <param name="_rs" value="_rs">param</param>
	/// <param name="_features" value="_features">param</param>
	/// <param name="_target" value="_target">param</param>
	/// <returns value ="_batchModel"/>
	/// </signature>

	};

	this.newLloyd = function () {
	/// <signature>
	/// <summary> online clustering based on the Lloyd alogrithm. The model intialization</summary>
	/// <param name="_dim" value="_dim">param</param>
	/// <param name="_k" value="_k">param</param>
	/// <returns value ="_lloydModel"/>
	/// </signature>

	};

	this.loadRecLinRegModel = function () {
	/// <signature>
	/// <summary> load serialized linear model</summary>
	/// <param name="_fin" value="_fin">param</param>
	/// <returns value ="_recLinRegModel"/>
	/// </signature>

	};

	this.computeKmeans = function () {
	/// <signature>
	/// <summary> solves the k-means algorithm based on a training</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <param name="_k" value="_k">param</param>
	/// <param name="_iter" value="_iter">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	/// <signature>
	/// <summary> solves the k-means algorithm based on a training</summary>
	/// <param name="_spMat" value="_spMat">param</param>
	/// <param name="_k" value="_k">param</param>
	/// <param name="_iter" value="_iter">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.crossValidation = function () {
	/// <signature>
	/// <summary> creates a batch</summary>
	/// <param name="_rs" value="_rs">param</param>
	/// <param name="_features" value="_features">param</param>
	/// <param name="_target" value="_target">param</param>
	/// <returns value ="_result"/>
	/// </signature>

	};

	this.loadSvmModel = function () {
	/// <signature>
	/// <summary> load serialized linear model</summary>
	/// <param name="_fin" value="_fin">param</param>
	/// <returns value ="_svmModel"/>
	/// </signature>

	};

	this.getLanguageOptions = function () {
	/// <signature>
	/// <summary> get options for text parsing</summary>
	/// <returns value ="_langOptionsJson"/>
	/// </signature>

	};

	this.newNN = function () {
	/// <signature>
	/// <summary> create new neural network</summary>
	/// <param name="_nnParameters" value="_nnParameters">param</param>
	/// <returns value ="_nnModel"/>
	/// </signature>

	};

	this.loadFeatureSpace = function () {
	/// <signature>
	/// <summary> load serialized feature</summary>
	/// <param name="_fin" value="_fin">param</param>
	/// <returns value ="_fsp"/>
	/// </signature>

	};

	this.newFeatureSpace = function () {
	/// <signature>
	/// <summary> create new</summary>
	/// <param name="_featureExtractors" value="_featureExtractors">param</param>
	/// <returns value ="_fsp"/>
	/// </signature>

	};

	this.trainSvmRegression = function () {
	/// <signature>
	/// <summary> trains</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <param name="_vec" value="_vec">param</param>
	/// <param name="_svmRegParameters" value="_svmRegParameters">param</param>
	/// <returns value ="_svmModel"/>
	/// </signature>

	};

	this.trainSvmClassify = function () {
	/// <signature>
	/// <summary> trains binary</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <param name="_vec" value="_vec">param</param>
	/// <param name="_svmParameters" value="_svmParameters">param</param>
	/// <returns value ="_svmModel"/>
	/// </signature>

	};

	this.newKNearestNeighbors = function () {
	/// <signature>
	/// <summary> online regression based on knn alogrithm. The model intialization</summary>
	/// <param name="_k" value="_k">param</param>
	/// <param name="_buffer" value="_buffer">param</param>
	/// <param name="_power" value="_power">param</param>
	/// <returns value ="_kNearestNeighbors"/>
	/// </signature>

	};

	this.newActiveLearner = function () {
	/// <signature>
	/// <summary> initializes the</summary>
	/// <param name="_fsp" value="_fsp">param</param>
	/// <param name="_textField" value="_textField">param</param>
	/// <param name="_rs" value="_rs">param</param>
	/// <param name="_nPos" value="_nPos">param</param>
	/// <param name="_nNeg" value="_nNeg">param</param>
	/// <param name="_query" value="_query">param</param>
	/// <param name="_c" value="_c">param</param>
	/// <param name="_j" value="_j">param</param>
	/// <returns value ="_alModel"/>
	/// </signature>

	};

	this.loadBatchModel = function () {
	/// <signature>
	/// <summary> loads batch model frm input stream `fin`</summary>
	/// <param name="_fin" value="_fin">param</param>
	/// <returns value ="_batchModel"/>
	/// </signature>

	};

	this.newRecLinReg = function () {
	/// <signature>
	/// <summary> create new recursive linear regression</summary>
	/// <param name="_recLinRegParameters" value="_recLinRegParameters">param</param>
	/// <returns value ="_recLinRegModel"/>
	/// </signature>

	};

	this.newPerceptron = function () {
	/// <signature>
	/// <summary> the perceptron learning algorithm initialization requires</summary>
	/// <param name="_dim" value="_dim">param</param>
	/// <param name="_use_bias" value="_use_bias">param</param>
	/// <returns value ="_perceptronModel"/>
	/// </signature>

	};

	this.newRidgeRegression = function () {
	/// <signature>
	/// <summary> solves a regularized ridge</summary>
	/// <param name="_kappa" value="_kappa">param</param>
	/// <param name="_dim" value="_dim">param</param>
	/// <param name="_buffer" value="_buffer">param</param>
	/// <returns value ="_ridgeRegressionModel"/>
	/// </signature>

	};

	this.newHoeffdingTree = function () {
	/// <signature>
	/// <summary> create new</summary>
	/// <param name="_jsonStream" value="_jsonStream">param</param>
	/// <param name="_htJsonParams" value="_htJsonParams">param</param>
	/// <returns value ="_htModel"/>
	/// </signature>

	};

}

var console = new function () {
	this.startx = function () {
	/// <signature>
	/// <summary> useful for debugging;</summary>
	/// <param name="_evalFun" value="_evalFun">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.pause = function () {
	/// <signature>
	/// <summary> waits until enter is pressed</summary>
	/// <returns value =""/>
	/// </signature>

	};

	this.log = function () {
	/// <signature>
	/// <summary> writes `message` to standard output, using</summary>
	/// <param name="_message" value="_message">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> writes `message` to standard output,</summary>
	/// <param name="_prefixStr" value="_prefixStr">param</param>
	/// <param name="_message" value="_message">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.start = function () {
	/// <signature>
	/// <summary> start interactive console; does not see local variables.</summary>
	/// <returns value =""/>
	/// </signature>

	};

	this.getln = function () {
	/// <signature>
	/// <summary> reads a line from command line and returns</summary>
	/// <returns value ="_line"/>
	/// </signature>

	};

	this.println = function () {
	/// <signature>
	/// <summary> `print(str); print("\n")`</summary>
	/// <param name="_str" value="_str">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.print = function () {
	/// <signature>
	/// <summary> prints a string to standard output</summary>
	/// <param name="_str" value="_str">param</param>
	/// <returns value =""/>
	/// </signature>

	};

}

var la = new function () {
	this.newVec = function () {
	/// <signature>
	/// <summary> generate an empty float vector</summary>
	/// <returns value ="_vec"/>
	/// </signature>

	/// <signature>
	/// <summary> copy a javascript number array `arr`</summary>
	/// <param name="_arr" value="_arr">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	/// <signature>
	/// <summary> clone a float vector `vec2`</summary>
	/// <param name="_vec2" value="_vec2">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.square = function () {
	/// <signature>
	/// <summary> squares all elements of a vector `vec` (inplace).</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> returns `sq` which is the quare of number `num`.</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	};

	this.printArray = function () {
	/// <signature>
	/// <summary> print the javascript array `arr` in the console</summary>
	/// <param name="_arr" value="_arr">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.newSpMat = function () {
	/// <signature>
	/// <summary> creates an empty sparse matrix `spMat`</summary>
	/// <returns value ="_spMat"/>
	/// </signature>

	/// <signature>
	/// <summary> creates an sparse matrix based on two int vectors `rowIdxVec` (row indices) and `colIdxVec` (column indices) and float vector of values `valVec`</summary>
	/// <param name="_rowIdxVec" value="_rowIdxVec">param</param>
	/// <param name="_colIdxVec" value="_colIdxVec">param</param>
	/// <param name="_valVec" value="_valVec">param</param>
	/// <returns value ="_spMat"/>
	/// </signature>

	/// <signature>
	/// <summary> creates an sparse matrix with `rows` rows (optional parameter), where `doubleNestedArr` is a javascript array of arrays that correspond to sparse matrix columns and each column is a javascript array of arrays corresponding to nonzero elements. Each element is an array of size 2, where the first number is an int (row index) and the second value is a number (value). Example: `spMat = linalg.newSpMat([[[0, 1.1], [1, 2.2], [3, 3.3]], [[2, 1.2]]], { "rows": 4 });`</summary>
	/// <param name="_doubleNestedArr" value="_doubleNestedArr">param</param>
	/// <param name="_rows" value="_rows">param</param>
	/// <returns value ="_spMat"/>
	/// </signature>

	};

	this.genRandomMatrix = function () {
	/// <signature>
	/// <summary> `mat` is a dense matrix whose elements are independent samples from a standard normal random variable, with `rows` rows and `cols` columns (integers)</summary>
	/// <param name="_rows" value="_rows">param</param>
	/// <param name="_cols" value="_cols">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.copyFltArrayToVec = function () {
	/// <signature>
	/// <summary> copies a JS array of numbers `arr` into a float vector `vec`</summary>
	/// <param name="_arr" value="_arr">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.genRandomVector = function () {
	/// <signature>
	/// <summary> `vec` is a dense vector whose elements are independent samples from a standard normal random variable and whos dimension is `dim`</summary>
	/// <param name="_dim" value="_dim">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.repmat = function () {
	/// <signature>
	/// <summary> creates a matrix `mat2` consisting of an `m`-by-`n` tiling of copies of `mat`</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <param name="_m" value="_m">param</param>
	/// <param name="_n" value="_n">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.randIntVec = function () {
	/// <signature>
	/// <summary> returns a JS array `vec`, which is a sample of `k` numbers from `[0,...,num]`, sampled without replacement. `k` must be smaller or equal to `num`</summary>
	/// <param name="_num" value="_num">param</param>
	/// <param name="_k" value="_k">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.rangeVec = function () {
	/// <signature>
	/// <summary> `intVec` is an integer vector: `[num, num + 1, ..., num2].</summary>
	/// <param name="_num" value="_num">param</param>
	/// <param name="_num2" value="_num2">param</param>
	/// <returns value ="_intVec"/>
	/// </signature>

	};

	this.repvec = function () {
	/// <signature>
	/// <summary> creates a matrix `mat2` consisting of an `m`-by-`n` tiling of copies of `vec`</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <param name="_m" value="_m">param</param>
	/// <param name="_n" value="_n">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.printSpFeatVec = function () {
	/// <signature>
	/// <summary> Print a sparse feature vector `spVec` along with feature names based on feature space `fsp`. If third parameter is ommited, the elements are sorted by dimension number. If boolean parameter `asc` is used, then the rows are sorted by (non-zero) vector values. Use `asc=true` for sorting in ascending order and `asc=false` for sorting in descending order.</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <param name="_fsp" value="_fsp">param</param>
	/// <param name="_asc" value="_asc">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.randInt = function () {
	/// <signature>
	/// <summary> returns an integer `num2` which is randomly selected from the set of integers `[0, ..., num]`</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	};

	this.newSpVec = function () {
	/// <signature>
	/// <summary> creates an empty sparse vector `spVec`, where `len` is an optional (-1 by default) integer parameter that sets the dimension</summary>
	/// <param name="_len" value="_len">param</param>
	/// <returns value ="_spVec"/>
	/// </signature>

	/// <signature>
	/// <summary> creats a sparse vector `spVec` from a javascript array `nestedArr`, whose elements are javascript arrays with two elements (integer row index and double value). `len` is optional and sets the dimension</summary>
	/// <param name="_nestedArr" value="_nestedArr">param</param>
	/// <param name="_len" value="_len">param</param>
	/// <returns value ="_spVec"/>
	/// </signature>

	};

	this.pdist2 = function () {
	/// <signature>
	/// <summary> computes the pairwise squared euclidean distances between columns of `mat` and `mat2`. mat3[i,j] = ||mat(:,i) - mat2(:,j)||^2</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <param name="_mat2" value="_mat2">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.zeros = function () {
	/// <signature>
	/// <summary> `mat` is a `rows`-by-`cols` sparse zero matrix</summary>
	/// <param name="_rows" value="_rows">param</param>
	/// <param name="_cols" value="_cols">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.printVec = function () {
	/// <signature>
	/// <summary> print the vector `vec` in the console</summary>
	/// <param name="_vecec" value="_vecec">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> print the int vector `intVec` in the console</summary>
	/// <param name="_intVec" value="_intVec">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.newIntVec = function () {
	/// <signature>
	/// <summary> generate an empty float vector</summary>
	/// <returns value ="_intVec"/>
	/// </signature>

	/// <signature>
	/// <summary> copy a javascript int array `arr`</summary>
	/// <param name="_arr" value="_arr">param</param>
	/// <returns value ="_intVec"/>
	/// </signature>

	/// <signature>
	/// <summary> clone an int vector `vec2`</summary>
	/// <param name="_vec2" value="_vec2">param</param>
	/// <returns value ="_intVec"/>
	/// </signature>

	};

	this.speye = function () {
	/// <signature>
	/// <summary> `spMat` is a `dim`-by-`dim` sparse identity matrix</summary>
	/// <param name="_dim" value="_dim">param</param>
	/// <returns value ="_spMat"/>
	/// </signature>

	};

	this.printMat = function () {
	/// <signature>
	/// <summary> print the matrix `mat` in the console</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.ones = function () {
	/// <signature>
	/// <summary> `vec` is a `k`-dimensional vector whose entries are set to `1.0`.</summary>
	/// <param name="_k" value="_k">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.eye = function () {
	/// <signature>
	/// <summary> `mat` is a `dim`-by-`dim` identity matrix</summary>
	/// <param name="_dim" value="_dim">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.saveMat = function () {
	/// <signature>
	/// <summary> writes a dense matrix `mat` to output file stream `fout`</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <param name="_fout" value="_fout">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.genRandomPerm = function () {
	/// <signature>
	/// <summary> returns a permutation of `k` elements. `arr` is a javascript array of integers</summary>
	/// <param name="_k" value="_k">param</param>
	/// <returns value ="_arr"/>
	/// </signature>

	};

	this.conjgrad = function () {
	/// <signature>
	/// <summary> solves the psd symmetric system mat * vec2 = vec, where `mat` is a matrix and `vec` and `vec2` are dense vectors</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <param name="_vec" value="_vec">param</param>
	/// <param name="_vec2" value="_vec2">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> solves the psd symmetric system spMat * vec2 = vec, where `spMat` is a matrix and `vec` and `vec2` are dense vectors</summary>
	/// <param name="_spMat" value="_spMat">param</param>
	/// <param name="_vec" value="_vec">param</param>
	/// <param name="_vec2" value="_vec2">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.newMat = function () {
	/// <signature>
	/// <summary> generates a 0x0 matrix</summary>
	/// <returns value ="_mat"/>
	/// </signature>

	/// <signature>
	/// <summary> generates a matrix from a javascript array `nestedArr`, whose elements are arrays of numbers which correspond to matrix rows (row-major dense matrix)</summary>
	/// <param name="_nestedArr" value="_nestedArr">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	/// <signature>
	/// <summary> clones a dense matrix `mat2`</summary>
	/// <param name="_mat2" value="_mat2">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.elementByElement = function () {
	/// <signature>
	/// <summary> performs element-by-element operation of `mat` or `vec`, defined in `callback` function. Example: `mat3 = la.elementByElement(mat, mat2, function (a, b) { return a*b } )`</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <param name="_mat2" value="_mat2">param</param>
	/// <param name="_callback" value="_callback">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.genRandom = function () {
	/// <signature>
	/// <summary> `num` is a sample from a standard normal random variable</summary>
	/// <returns value ="_num"/>
	/// </signature>

	};

	this.copyIntArrayToVec = function () {
	/// <signature>
	/// <summary> copies a JS array of integers `arr` into an integer vector `intVec`</summary>
	/// <param name="_arr" value="_arr">param</param>
	/// <returns value ="_intVec"/>
	/// </signature>

	};

	this.sparse = function () {
	/// <signature>
	/// <summary> `spMat` is a `rows`-by-`cols` sparse zero matrix</summary>
	/// <param name="_rows" value="_rows">param</param>
	/// <param name="_cols" value="_cols">param</param>
	/// <returns value ="_spMat"/>
	/// </signature>

	};

	this.findMaxIdx = function () {
	/// <signature>
	/// <summary> returns a JS array of indices `idxArray` that correspond to the max elements in each column of dense matrix `mat`.</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <returns value ="_arr"/>
	/// </signature>

	/// <signature>
	/// <summary> returns a JS array of indices `idxArray` that correspond to the max elements in each column of dense matrix `vec`. The resulting array has one element.</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_arr"/>
	/// </signature>

	};

	this.printFeatVec = function () {
	/// <signature>
	/// <summary> Print a feature vector `vec` along with feature names based on feature space `fsp`. The parameter `limit` (integer) is optional and limits the number of rows printed (prints all values by default). If the fourth parameter is ommited, the elements are sorted by dimension number. If boolean parameter `asc` is used, then the rows are sorted by (non-zero) vector values. Use `asc=true` for sorting in ascending order and `asc=false` for sorting in descending order.</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <param name="_fsp" value="_fsp">param</param>
	/// <param name="_limit" value="_limit">param</param>
	/// <param name="_asc" value="_asc">param</param>
	/// <returns value =""/>
	/// </signature>

	};

}

var _sa = new function () {
	this.saveJson = function () {
	/// <signature>
	/// <summary> executes saveJson given an optional number parameter `limit`, whose meaning is specific to each type of stream aggregate</summary>
	/// <param name="_limit" value="_limit">param</param>
	/// <returns value ="_objJSON"/>
	/// </signature>

	};

	this.onDelete = function () {
	/// <signature>
	/// <summary> executes onDelete function given an input record `rec`</summary>
	/// <param name="_rec" value="_rec">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	/// <field name = "name" value = "_str"> returns the name (unique) of the stream aggregate</field>
	this.name = _str;
	/// <field name = "val" value = "_objJSON"> same as sa.saveJson(-1)</field>
	this.val = _objJSON;
	this.onUpdate = function () {
	/// <signature>
	/// <summary> executes onUpdate function given an input record `rec`</summary>
	/// <param name="_rec" value="_rec">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.onAdd = function () {
	/// <signature>
	/// <summary> executes onAdd function given an input record `rec`</summary>
	/// <param name="_rec" value="_rec">param</param>
	/// <returns value =""/>
	/// </signature>

	};

}

var _spMat = new function () {
	this.load = function () {
	/// <signature>
	/// <summary> load `spMat` (sparse matrix) from input steam `fin`. `spMat` has to be initialized first, for example using `spMat = la.newSpMat()`.</summary>
	/// <param name="_fin" value="_fin">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.full = function () {
	/// <signature>
	/// <summary> get dense matrix representation `mat` of `spMat (sparse column matrix)`</summary>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.normalizeCols = function () {
	/// <signature>
	/// <summary> normalizes each column of a sparse matrix `spMat` (inplace operation)</summary>
	/// <returns value =""/>
	/// </signature>

	};

	this.push = function () {
	/// <signature>
	/// <summary> attaches a column `spVec` (sparse vector) to `spMat` (sparse matrix)</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.transpose = function () {
	/// <signature>
	/// <summary> `spMat2` (sparse matrix) is `spMat` (sparse matrix) transposed</summary>
	/// <returns value ="_spMat"/>
	/// </signature>

	};

	this.frob = function () {
	/// <signature>
	/// <summary> number `num` is the Frobenious norm of `spMat` (sparse matrix)</summary>
	/// <returns value ="_num"/>
	/// </signature>

	};

	this.put = function () {
	/// <signature>
	/// <summary> Sets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing.</summary>
	/// <param name="_rowIdx" value="_rowIdx">param</param>
	/// <param name="_colIdx" value="_colIdx">param</param>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	/// <field name = "cols" value = "_num"> integer `num` corresponds to the number of columns of `spMat` (sparse matrix)</field>
	this.cols = _num;
	this.multiplyT = function () {
	/// <signature>
	/// <summary> Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_spMat"/>
	/// </signature>

	/// <signature>
	/// <summary> Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	/// <signature>
	/// <summary> Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	/// <signature>
	/// <summary> Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	/// <signature>
	/// <summary> Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient.</summary>
	/// <param name="_spMat2" value="_spMat2">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.print = function () {
	/// <signature>
	/// <summary> print `spMat` (sparse matrix) to console</summary>
	/// <returns value =""/>
	/// </signature>

	};

	this.nnz = function () {
	/// <signature>
	/// <summary> `num` is the number of non-zero elements of sparse column matrix `spMat`</summary>
	/// <returns value ="_num"/>
	/// </signature>

	};

	/// <field name = "rows" value = "_num"> integer `num` corresponds to the number of rows of `spMat` (sparse matrix)</field>
	this.rows = _num;
	this.plus = function () {
	/// <signature>
	/// <summary> `spMat3` is the sum of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)</summary>
	/// <param name="_spMat2" value="_spMat2">param</param>
	/// <returns value ="_spMat"/>
	/// </signature>

	};

	this.at = function () {
	/// <signature>
	/// <summary> Gets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.</summary>
	/// <param name="_rowIdx" value="_rowIdx">param</param>
	/// <param name="_colIdx" value="_colIdx">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	};

	this.multiply = function () {
	/// <signature>
	/// <summary> Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_spMat"/>
	/// </signature>

	/// <signature>
	/// <summary> Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	/// <signature>
	/// <summary> Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	/// <signature>
	/// <summary> Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	/// <signature>
	/// <summary> Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix</summary>
	/// <param name="_spMat2" value="_spMat2">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.toString = function () {
	/// <signature>
	/// <summary> returns a string displaying rows, columns and number of non-zero elements of a sparse column matrix `spMat`</summary>
	/// <returns value ="_str"/>
	/// </signature>

	};

	this.save = function () {
	/// <signature>
	/// <summary> print `spMat` (sparse matrix) to output stream `fout`</summary>
	/// <param name="_fout" value="_fout">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.minus = function () {
	/// <signature>
	/// <summary> `spMat3` is the difference of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)</summary>
	/// <param name="_spMat2" value="_spMat2">param</param>
	/// <returns value ="_spMat"/>
	/// </signature>

	};

	this.colNorms = function () {
	/// <signature>
	/// <summary> `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `spMat`</summary>
	/// <returns value ="_vec"/>
	/// </signature>

	};

}

var _batchModel = new function () {
	this.predict = function () {
	/// <signature>
	/// <summary> creates feature vector from record `rec`, sends it</summary>
	/// <param name="_rec" value="_rec">param</param>
	/// <returns value ="_scoreArr"/>
	/// </signature>

	};

	this.save = function () {
	/// <signature>
	/// <summary> saves the model to `fout` output stream</summary>
	/// <param name="_fout" value="_fout">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.predictLabels = function () {
	/// <signature>
	/// <summary> creates feature vector from record `rec`,</summary>
	/// <param name="_rec" value="_rec">param</param>
	/// <returns value ="_labelArr"/>
	/// </signature>

	};

	/// <field name = "target" value = "_strArr"> array of categories for which we have models</field>
	this.target = _strArr;
	this.predictTop = function () {
	/// <signature>
	/// <summary> creates feature vector from record `rec`,</summary>
	/// <param name="_rec" value="_rec">param</param>
	/// <returns value ="_labelStr"/>
	/// </signature>

	};

}

var _httpResponse = new function () {
	this.setStatusCode = function () {
	/// <signature>
	/// <summary> sets status code (integer)</summary>
	/// <param name="_statusCode" value="_statusCode">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.close = function () {
	/// <signature>
	/// <summary> closes and executes the response</summary>
	/// <returns value =""/>
	/// </signature>

	};

	this.add = function () {
	/// <signature>
	/// <summary> adds `dataStr` (string) to request body</summary>
	/// <param name="_dataStr" value="_dataStr">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> adds `dataJSON` (JSON object) to request body</summary>
	/// <param name="_dataJSON" value="_dataJSON">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.send = function () {
	/// <signature>
	/// <summary> adds `dataStr` (string) and closes the response</summary>
	/// <param name="_dataStr" value="_dataStr">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> adds `dataJSON` (JSON object) and closes the response</summary>
	/// <param name="_dataJSON" value="_dataJSON">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.setContentType = function () {
	/// <signature>
	/// <summary> sets content type (string)</summary>
	/// <param name="_mimeType" value="_mimeType">param</param>
	/// <returns value =""/>
	/// </signature>

	};

}

var _svmModel = new function () {
	this.predict = function () {
	/// <signature>
	/// <summary> sends vector `vec` through the model and returns the prediction as a real number `num` (-1 or 1 for classification)</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	/// <signature>
	/// <summary> sends sparse vector `spVec` through the model and returns the prediction as a real number `num` (-1 or 1 for classification)</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	};

	this.save = function () {
	/// <signature>
	/// <summary> saves model to output stream `fout`</summary>
	/// <param name="_fout" value="_fout">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	/// <field name = "weights" value = "_vec"> weights of the SVM linear model as a full vector `vec`</field>
	this.weights = _vec;
}

var _kNearestNeighbors = new function () {
	this.predict = function () {
	/// <signature>
	/// <summary> predicts the target `num` (number), given feature vector `vec` based on k nearest neighburs,</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	/// <signature>
	/// <summary> findes k nearest neighbors. Returns object with two vectors: indexes `perm` (intVec) and values `vec` (vector)</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_object"/>
	/// </signature>

	};

	this.update = function () {
	/// <signature>
	/// <summary> adds a vector `vec` and target `num` (number) to the "training" set</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	};

}

var _utilities = new function () {
	this.isArray = function () {
	/// <signature>
	/// <summary> is parameter an array?</summary>
	/// <param name="_arg" value="_arg">param</param>
	/// <returns value ="_bool"/>
	/// </signature>

	};

	this.isString = function () {
	/// <signature>
	/// <summary> is `s` a string?</summary>
	/// <param name="_s" value="_s">param</param>
	/// <returns value ="_bool"/>
	/// </signature>

	};

	this.arraysIdentical = function () {
	/// <signature>
	/// <summary> `bool` is true if array `arr` is identical to array `arr2`</summary>
	/// <param name="_arr" value="_arr">param</param>
	/// <param name="_arr2" value="_arr2">param</param>
	/// <returns value ="_bool"/>
	/// </signature>

	};

	this.isInArray = function () {
	/// <signature>
	/// <summary> is element in an array?</summary>
	/// <param name="_array" value="_array">param</param>
	/// <param name="_value" value="_value">param</param>
	/// <returns value ="_bool"/>
	/// </signature>

	};

	this.ifNull = function () {
	/// <signature>
	/// <summary> checks if `val` is null and returns default value `defVal`</summary>
	/// <param name="_val" value="_val">param</param>
	/// <param name="_defVal" value="_defVal">param</param>
	/// <returns value ="_returnVal"/>
	/// </signature>

	};

	this.newStopWatch = function () {
	/// <signature>
	/// <summary> creates a stop watch object `sw`</summary>
	/// <returns value ="_sw"/>
	/// </signature>

	};

	this.newHashTable = function () {
	/// <signature>
	/// <summary> creates a hash table</summary>
	/// <returns value ="_hashTable"/>
	/// </signature>

	};

	this.isNumber = function () {
	/// <signature>
	/// <summary> is `n` a number?</summary>
	/// <param name="_n" value="_n">param</param>
	/// <returns value ="_bool"/>
	/// </signature>

	};

	this.isObject = function () {
	/// <signature>
	/// <summary> is parameter an object?</summary>
	/// <param name="_arg" value="_arg">param</param>
	/// <returns value ="_bool"/>
	/// </signature>

	};

	this.numberWithCommas = function () {
	/// <signature>
	/// <summary> format number 1234 to 1,234</summary>
	/// <param name="_number" value="_number">param</param>
	/// <returns value ="_string"/>
	/// </signature>

	};

}

var _fout = new function () {
	this.write = function () {
	/// <signature>
	/// <summary> writes to output stream. `data` can be a number, a json object or a string.</summary>
	/// <param name="_data" value="_data">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.close = function () {
	/// <signature>
	/// <summary> closes output stream</summary>
	/// <returns value =""/>
	/// </signature>

	};

	this.writeLine = function () {
	/// <signature>
	/// <summary> writes data to output stream and adds newline</summary>
	/// <param name="_data" value="_data">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.flush = function () {
	/// <signature>
	/// <summary> flushes output stream</summary>
	/// <returns value =""/>
	/// </signature>

	};

}

var _rs = new function () {
	/// <field name = "weighted" value = "_bool"> `bool = true` when records in the set are assigned weights</field>
	this.weighted = _bool;
	this.shuffle = function () {
	/// <signature>
	/// <summary> shuffle order using random integer seed `seed`. Inplace operation.</summary>
	/// <param name="_seed" value="_seed">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.sample = function () {
	/// <signature>
	/// <summary> create new record set by randomly sampling `num` records.</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_rs"/>
	/// </signature>

	};

	this.sortByField = function () {
	/// <signature>
	/// <summary> sort records according to value of field `fieldName`; if `asc > 0` sorted in ascending order. Inplace operation.</summary>
	/// <param name="_fieldName" value="_fieldName">param</param>
	/// <param name="_asc" value="_asc">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.toJSON = function () {
	/// <signature>
	/// <summary> provide json version of record set, useful when calling JSON.stringify</summary>
	/// <returns value ="_objsJSON"/>
	/// </signature>

	};

	this.setdiff = function () {
	/// <signature>
	/// <summary> returns the set difference (record set) `rs3`=`rs``rs2`  between two record sets `rs` and `rs1`, which should point to the same store.</summary>
	/// <param name="_rs2" value="_rs2">param</param>
	/// <returns value ="_rs"/>
	/// </signature>

	};

	this.deleteRecs = function () {
	/// <signature>
	/// <summary> delete from `rs` records that are also in `rs2`. Inplace operation.</summary>
	/// <param name="_rs2" value="_rs2">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.setunion = function () {
	/// <signature>
	/// <summary> returns the union (record set) `rs3` between two record sets `rs` and `rs2`, which should point to the same store.</summary>
	/// <param name="_rs2" value="_rs2">param</param>
	/// <returns value ="_rs"/>
	/// </signature>

	};

	this.filterById = function () {
	/// <signature>
	/// <summary> keeps only records with ids between `minId` and `maxId`. Inplace operation.</summary>
	/// <param name="_minId" value="_minId">param</param>
	/// <param name="_maxId" value="_maxId">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.filterByField = function () {
	/// <signature>
	/// <summary> keeps only records with numeric value of field `fieldName` between `minVal` and `maxVal`. Inplace operation.</summary>
	/// <param name="_fieldName" value="_fieldName">param</param>
	/// <param name="_minVal" value="_minVal">param</param>
	/// <param name="_maxVal" value="_maxVal">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> keeps only records with value of time field `fieldName` between `minVal` and `maxVal`. Inplace operation.</summary>
	/// <param name="_fieldName" value="_fieldName">param</param>
	/// <param name="_minTm" value="_minTm">param</param>
	/// <param name="_maxTm" value="_maxTm">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> keeps only records with string value of field `fieldName` equal to `str`. Inplace operation.</summary>
	/// <param name="_fieldName" value="_fieldName">param</param>
	/// <param name="_str" value="_str">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.setintersect = function () {
	/// <signature>
	/// <summary> returns the intersection (record set) `rs3` between two record sets `rs` and `rs2`, which should point to the same store.</summary>
	/// <param name="_rs2" value="_rs2">param</param>
	/// <returns value ="_rs"/>
	/// </signature>

	};

	this.split = function () {
	/// <signature>
	/// <summary> split records according to `splitter` callback. Example: rs.split(function(rec,rec2) {return (rec2.Val - rec2.Val) > 10;} ) splits rs in whenever the value of field Val increases for more then 10. Result is an array of record sets.</summary>
	/// <param name="_splitterCallback" value="_splitterCallback">param</param>
	/// <returns value ="_rsArr"/>
	/// </signature>

	};

	this.sortById = function () {
	/// <signature>
	/// <summary> sort records according to record id; if `asc > 0` sorted in ascending order. Inplace operation.</summary>
	/// <param name="_asc" value="_asc">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	/// <field name = "empty" value = "_bool"> `bool = true` when record set is empty</field>
	this.empty = _bool;
	this.sort = function () {
	/// <signature>
	/// <summary> sort records according to `comparator` callback. Example: rs.sort(function(rec,rec2) {return rec.Val < rec2.Val;} ) sorts rs in ascending order (field Val is assumed to be a num). Inplace operation.</summary>
	/// <param name="_comparatorCallback" value="_comparatorCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.map = function () {
	/// <signature>
	/// <summary> iterates through the record set and executes the callback function `mapCallback` on each element:</summary>
	/// <param name="_mapCallback" value="_mapCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.sortByFq = function () {
	/// <signature>
	/// <summary> sort records according to weight; if `asc > 0` sorted in ascending order. Inplace operation.</summary>
	/// <param name="_asc" value="_asc">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.clone = function () {
	/// <signature>
	/// <summary> creates new instance of record set</summary>
	/// <returns value ="_rs"/>
	/// </signature>

	};

	this.filterByFq = function () {
	/// <signature>
	/// <summary> keeps only records with weight between `minFq` and `maxFq`. Inplace operation.</summary>
	/// <param name="_minFq" value="_minFq">param</param>
	/// <param name="_maxFq" value="_maxFq">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.trunc = function () {
	/// <signature>
	/// <summary> truncate to first `num` record. Inplace operation.</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.join = function () {
	/// <signature>
	/// <summary> executes a join `joinName` on the records in the set, result is another record set `rs2`.</summary>
	/// <param name="_joinName" value="_joinName">param</param>
	/// <returns value ="_rs"/>
	/// </signature>

	/// <signature>
	/// <summary> executes a join `joinName` on a sample of `sampleSize` records in the set, result is another record set `rs2`.</summary>
	/// <param name="_joinName" value="_joinName">param</param>
	/// <param name="_sampleSize" value="_sampleSize">param</param>
	/// <returns value ="_rs"/>
	/// </signature>

	};

	this.reverse = function () {
	/// <signature>
	/// <summary> reverse record order. Inplace operation.</summary>
	/// <returns value =""/>
	/// </signature>

	};

	this.filter = function () {
	/// <signature>
	/// <summary> keeps only records that pass `filterCallback` function</summary>
	/// <param name="_filterCallback" value="_filterCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	/// <field name = "length" value = "_len"> number of records in the set</field>
	this.length = _len;
	/// <field name = "store" value = "_storeName"> store of the records</field>
	this.store = _storeName;
	this.aggr = function () {
	/// <signature>
	/// <summary> returns an object where keys are aggregate names and values are JSON serialized aggregate values of all the aggregates contained in the records set</summary>
	/// <returns value ="_aggrsJSON"/>
	/// </signature>

	/// <signature>
	/// <summary> computes the aggregates based on the `aggrQueryJSON` parameter JSON object. If only one aggregate is involved and an array of JSON objects when more than one are returned.</summary>
	/// <param name="_aggrQueryJSON" value="_aggrQueryJSON">param</param>
	/// <returns value ="_aggr"/>
	/// </signature>

	};

}

var _spVec = new function () {
	this.normalize = function () {
	/// <signature>
	/// <summary> normalizes the vector spVec (inplace operation)</summary>
	/// <returns value =""/>
	/// </signature>

	};

	/// <field name = "dim" value = "_num"> gets the dimension `num` (-1 means that it is unknown)</field>
	this.dim = _num;
	this.full = function () {
	/// <signature>
	/// <summary>  returns `vec` - a dense vector representation of sparse vector `spVec`.</summary>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.sum = function () {
	/// <signature>
	/// <summary> `num` is the sum of elements of `spVec`</summary>
	/// <returns value ="_num"/>
	/// </signature>

	};

	this.put = function () {
	/// <signature>
	/// <summary> Set the element of a sparse vector `spVec`. Inputs: index `idx` (integer), value `num` (number). Uses 0-based indexing</summary>
	/// <param name="_idx" value="_idx">param</param>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.idxVec = function () {
	/// <signature>
	/// <summary>  returns `idxVec` - a dense (int) vector of indices (0-based) of nonzero elements of `spVec`.</summary>
	/// <returns value ="_idxVec"/>
	/// </signature>

	};

	this.print = function () {
	/// <signature>
	/// <summary> prints the vector to console</summary>
	/// <returns value =""/>
	/// </signature>

	};

	/// <field name = "nnz" value = "_num"> gets the number of nonzero elements `num` of vector `spVec`</field>
	this.nnz = _num;
	this.inner = function () {
	/// <signature>
	/// <summary> `num` is the inner product between `spVec` and dense vector `vec`.</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	/// <signature>
	/// <summary> `num` is the inner product between `spVec` and sparse vector `spVec`.</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	};

	this.multiply = function () {
	/// <signature>
	/// <summary> `spVec2` is sparse vector, a product between `num` (number) and vector `spVec`</summary>
	/// <param name="_a" value="_a">param</param>
	/// <returns value ="_spVec"/>
	/// </signature>

	};

	this.valVec = function () {
	/// <signature>
	/// <summary>  returns `valVec` - a dense (double) vector of values of nonzero elements of `spVec`.</summary>
	/// <returns value ="_valVec"/>
	/// </signature>

	};

	this.norm = function () {
	/// <signature>
	/// <summary> returns `num` - the norm of `spVec`</summary>
	/// <returns value ="_num"/>
	/// </signature>

	};

	this.at = function () {
	/// <signature>
	/// <summary> Gets the element of a sparse vector `spVec`. Input: index `idx` (integer). Output: value `num` (number). Uses 0-based indexing</summary>
	/// <param name="_idx" value="_idx">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	};

}

var _lloydModel = new function () {
	this.getCentroidIdx = function () {
	/// <signature>
	/// <summary> returns the centroid index `idx` (integer) that corresponds to the centroid that is the closest to vector `vec`</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_idx"/>
	/// </signature>

	/// <signature>
	/// <summary> returns the centroid index `idx` (integer) that corresponds to the centroid that is the closest to sparse vector `spVec`</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <returns value ="_idx"/>
	/// </signature>

	};

	this.setC = function () {
	/// <signature>
	/// <summary> sets the centroid matrix to matrix `mat`</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.getC = function () {
	/// <signature>
	/// <summary> returns the centroid matrix `mat`</summary>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.update = function () {
	/// <signature>
	/// <summary> updates the model with a vector `vec`</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> updates the model with a sparse vector `spVec`</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.init = function () {
	/// <signature>
	/// <summary> initializes the model with random centroids</summary>
	/// <returns value =""/>
	/// </signature>

	};

	this.getCentroid = function () {
	/// <signature>
	/// <summary> returns the centroid `vec2` (dense vector) that is the closest to vector `vec`</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	/// <signature>
	/// <summary> returns the centroid `vec2` (dense vector) that is the closest to sparse vector `spVec`</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	};

}

var http = new function () {
	this.onDelete = function () {
	/// <signature>
	/// <summary> path: function path without server name and script name. Example: `http.onDelete("test", function (req, resp) { })` executed from `script.js` on localhost will execute a delete request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }</summary>
	/// <param name="_path" value="_path">param</param>
	/// <param name="_httpRequestCallback" value="_httpRequestCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.onRequest = function () {
	/// <signature>
	/// <summary> path: function path without server name and script name. Example: `http.onRequest("test", "GET", function (req, resp) { })` executed from `script.js` on localhost will execute a get request from `http://localhost/script/test`. `verb` can be one of the following {"GET","POST","PUT","DELETE","PATCH"}. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }</summary>
	/// <param name="_path" value="_path">param</param>
	/// <param name="_verb" value="_verb">param</param>
	/// <param name="_httpRequestCallback" value="_httpRequestCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.get = function () {
	/// <signature>
	/// <summary> gets url, but does nothing with response</summary>
	/// <param name="_url" value="_url">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> gets url and executes httpJsonSuccessCallback, a function with signature: function (objJson) {} on success. Error will occour if objJson is not a JSON object.</summary>
	/// <param name="_url" value="_url">param</param>
	/// <param name="_httpJsonSuccessCallback" value="_httpJsonSuccessCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> gets url and executes httpJsonSuccessCallback (signature: function (objJson) {}) on success or httpErrorCallback (signature: function (message) {}) on error. Error will occour if objJson is not a JSON object.</summary>
	/// <param name="_url" value="_url">param</param>
	/// <param name="_httpJsonSuccessCallback" value="_httpJsonSuccessCallback">param</param>
	/// <param name="_httpErrorCallback" value="_httpErrorCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.onPut = function () {
	/// <signature>
	/// <summary> path: function path without server name and script name. Example: `http.onPut("test", function (req, resp) { })` executed from `script.js` on localhost will execute a put request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }</summary>
	/// <param name="_path" value="_path">param</param>
	/// <param name="_httpRequestCallback" value="_httpRequestCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.getStr = function () {
	/// <signature>
	/// <summary> gets url, but does nothing with response</summary>
	/// <param name="_url" value="_url">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> gets url and executes httpStrSuccessCallback, a function with signature: function (str) {} on success.</summary>
	/// <param name="_url" value="_url">param</param>
	/// <param name="_httpStrSuccessCallback" value="_httpStrSuccessCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> gets url and executes httpJsonSuccessCallback (signature: function (str) {}) on success or httpErrorCallback (signature: function (message) {}) on error.</summary>
	/// <param name="_url" value="_url">param</param>
	/// <param name="_httpStrSuccessCallback" value="_httpStrSuccessCallback">param</param>
	/// <param name="_httpErrorCallback" value="_httpErrorCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.postStr = function () {
	/// <signature>
	/// <summary> post to `url` (string) using `mimeType` (string), where the request body is `data` (string)</summary>
	/// <param name="_url" value="_url">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> post to `url` (string) using `mimeType` (string), where the request body is `data` (string). executes httpStrSuccessCallback, a function with signature: function (str) {} on success.</summary>
	/// <param name="_url" value="_url">param</param>
	/// <param name="_mimeType" value="_mimeType">param</param>
	/// <param name="_data" value="_data">param</param>
	/// <param name="_httpStrSuccessCallback" value="_httpStrSuccessCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> post to `url` (string) using `mimeType` (string), where the request body is `data` (string). executes httpStrSuccessCallback, a function with signature: function (str) {} on success or httpErrorCallback (signature: function (message) {}) on error.</summary>
	/// <param name="_url" value="_url">param</param>
	/// <param name="_mimeType" value="_mimeType">param</param>
	/// <param name="_data" value="_data">param</param>
	/// <param name="_httpStrSuccessCallback" value="_httpStrSuccessCallback">param</param>
	/// <param name="_httpErrorCallback" value="_httpErrorCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.jsonp = function () {
	/// <signature>
	/// <summary> packaging reply as jsonp when callback parameter is provided in URL</summary>
	/// <param name="_httpRequest" value="_httpRequest">param</param>
	/// <param name="_httpResponse" value="_httpResponse">param</param>
	/// <param name="_dataJSON" value="_dataJSON">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.onPatch = function () {
	/// <signature>
	/// <summary> path: function path without server name and script name. Example: `http.onPatch("test", function (req, resp) { })` executed from `script.js` on localhost will execute a patch request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }</summary>
	/// <param name="_path" value="_path">param</param>
	/// <param name="_httpRequestCallback" value="_httpRequestCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.post = function () {
	/// <signature>
	/// <summary> post to `url` (string) using `mimeType` (string), where the request body is `data` (string)</summary>
	/// <param name="_url" value="_url">param</param>
	/// <param name="_mimeType" value="_mimeType">param</param>
	/// <param name="_data" value="_data">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> post to `url` (string) using `mimeType` (string), where the request body is `data` (string). executes httpJsonSuccessCallback, a function with signature: function (objJson) {} on success. Error will occour if objJson is not a JSON object.</summary>
	/// <param name="_url" value="_url">param</param>
	/// <param name="_mimeType" value="_mimeType">param</param>
	/// <param name="_data" value="_data">param</param>
	/// <param name="_httpJsonSuccessCallback" value="_httpJsonSuccessCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> post to `url` (string) using `mimeType` (string), where the request body is `data` (string). executes httpJsonSuccessCallback, a function with signature: function (objJson) {} on success or httpErrorCallback (signature: function (message) {}) on error. Error will occour if objJson is not a JSON object.</summary>
	/// <param name="_url" value="_url">param</param>
	/// <param name="_mimeType" value="_mimeType">param</param>
	/// <param name="_data" value="_data">param</param>
	/// <param name="_httpJsonSuccessCallback" value="_httpJsonSuccessCallback">param</param>
	/// <param name="_httpErrorCallback" value="_httpErrorCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.onPost = function () {
	/// <signature>
	/// <summary> path: function path without server name and script name. Example: `http.onPost("test", function (req, resp) { })` executed from `script.js` on localhost will execute a post request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }</summary>
	/// <param name="_path" value="_path">param</param>
	/// <param name="_httpRequestCallback" value="_httpRequestCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.onGet = function () {
	/// <signature>
	/// <summary> path: function path without server name and script name. Example: `http.onGet("test", function (req, resp) { })` executed from `script.js` on localhost will execute a get request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }</summary>
	/// <param name="_path" value="_path">param</param>
	/// <param name="_httpRequestCallback" value="_httpRequestCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	};

}

var _recLinRegModel = new function () {
	this.predict = function () {
	/// <signature>
	/// <summary> sends vector `vec` through the</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	};

	/// <field name = "dim" value = "_num"> dimensionality of the feature space on which this model works</field>
	this.dim = _num;
	this.save = function () {
	/// <signature>
	/// <summary> saves model to output stream `fout`</summary>
	/// <param name="_fout" value="_fout">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	/// <field name = "weights" value = "_vec"> weights of the linear model as a full vector `vec`</field>
	this.weights = _vec;
	this.learn = function () {
	/// <signature>
	/// <summary> updates the model using full vector `vec` and target number `num`as training data</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	};

}

var _twitterParser = new function () {
	this.rawJsonToStoreJson = function () {
	/// <signature>
	/// <summary> transforms a raw JSON object (result of twitter crawler) `rawTweetJSON` to `twitter.getTwitterStore()` compatible json object `objJSON`</summary>
	/// <param name="_rawTweetJSON" value="_rawTweetJSON">param</param>
	/// <returns value ="_objJSON"/>
	/// </signature>

	};

}

var _tm = new function () {
	/// <field name = "dayOfWeek" value = "_str"> day of week (string)</field>
	this.dayOfWeek = _str;
	this.sub = function () {
	/// <signature>
	/// <summary> subtracts `val` from the time; `unit` defintes the unit of `val`. options are `second` (default), `minute`, `hour`, and `day`.</summary>
	/// <param name="_val" value="_val">param</param>
	/// <param name="_unit" value="_unit">param</param>
	/// <returns value ="_tm"/>
	/// </signature>

	};

	this.toJSON = function () {
	/// <signature>
	/// <summary> returns json representation of time</summary>
	/// <returns value ="_tmJSON"/>
	/// </signature>

	};

	/// <field name = "string" value = "_str"> string representation of time (e.g. 2014-05-29T10:09:12)</field>
	this.string = _str;
	/// <field name = "hour" value = "_num"> hour (number)</field>
	this.hour = _num;
	/// <field name = "timestamp" value = "_num"> unix timestamp representation of time (seconds since 1970)</field>
	this.timestamp = _num;
	/// <field name = "nowUTC" value = "_tm"> returns new time object represented current UTC time</field>
	this.nowUTC = this;
	/// <field name = "dateString" value = "_str"> string representation of date (e.g. 2014-05-29)</field>
	this.dateString = _str;
	/// <field name = "month" value = "_num"> month (number)</field>
	this.month = _num;
	this.parse = function () {
	/// <signature>
	/// <summary> parses string `str` in weblog format (example: `2014-05-29T10:09:12`)  and returns a date time object. Weblog format uses `T` to separate date and time, uses `-` for date units separation and `:` for time units separation (`YYYY-MM-DDThh-mm-ss`).</summary>
	/// <param name="_str" value="_str">param</param>
	/// <returns value ="_tm"/>
	/// </signature>

	};

	this.add = function () {
	/// <signature>
	/// <summary> adds `val` to the time; `unit` defines the unit</summary>
	/// <param name="_val" value="_val">param</param>
	/// <param name="_unit" value="_unit">param</param>
	/// <returns value ="_tm"/>
	/// </signature>

	};

	/// <field name = "year" value = "_num"> year (number)</field>
	this.year = _num;
	/// <field name = "milisecond" value = "_num"> millisecond (number)</field>
	this.milisecond = _num;
	/// <field name = "now" value = "_tm"> returns new time object representing current local time</field>
	this.now = this;
	/// <field name = "day" value = "_num"> day (number)</field>
	this.day = _num;
	/// <field name = "minute" value = "_num"> minute (number)</field>
	this.minute = _num;
	/// <field name = "second" value = "_num"> second (number)</field>
	this.second = _num;
}

var _htModel = new function () {
	this.process = function () {
	/// <signature>
	/// <summary> processes the stream example; `strArr` is an array of discrete attribute values (strings);</summary>
	/// <param name="_strArr" value="_strArr">param</param>
	/// <param name="_numArr" value="_numArr">param</param>
	/// <param name="_labelStr" value="_labelStr">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> processes the stream example; `line` is comma-separated string of attribute values (for example `"a1,a2,c"`, where `c` is the class label); the function returns nothing.</summary>
	/// <param name="_line" value="_line">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.exportModel = function () {
	/// <signature>
	/// <summary> writes the current model into file `htOutParams.file` in format `htOutParams.type`.</summary>
	/// <param name="_htOutParams" value="_htOutParams">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.classify = function () {
	/// <signature>
	/// <summary> classifies the stream example; `strArr` is an array of discrete attribute values (strings); `numArr` is an array of numeric attribute values (numbers); returns the class label `labelStr`.</summary>
	/// <param name="_strArr" value="_strArr">param</param>
	/// <param name="_numArr" value="_numArr">param</param>
	/// <returns value ="_labelStr"/>
	/// </signature>

	/// <signature>
	/// <summary> classifies the stream example; `line` is comma-separated string of attribute values; returns the class label `labelStr`.</summary>
	/// <param name="_line" value="_line">param</param>
	/// <returns value ="_labelStr"/>
	/// </signature>

	};

}

var _alModel = new function () {
	this.selectQuestion = function () {
	/// <signature>
	/// <summary> returns `recSetIdx` - the index of the record in `recSet`, whose class is unknonw and requires user input</summary>
	/// <returns value ="_recSetIdx"/>
	/// </signature>

	};

	this.getAnswer = function () {
	/// <signature>
	/// <summary> given user input `ALAnswer` (string) and `recSetIdx` (integer, result of model.selectQuestion) the training set is updated.</summary>
	/// <param name="_alAnswer" value="_alAnswer">param</param>
	/// <param name="_recSetIdx" value="_recSetIdx">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.getPos = function () {
	/// <signature>
	/// <summary> given a `threshold` (number) return the indexes of records classified above it as a javascript array of numbers. Must be in SVM mode.</summary>
	/// <param name="_thresh" value="_thresh">param</param>
	/// <returns value ="_numArr"/>
	/// </signature>

	};

	this.setc = function () {
	/// <signature>
	/// <summary> sets the SVM c parameter to the provided value.</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.setj = function () {
	/// <signature>
	/// <summary> sets the SVM j parameter to the provided value.</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.startLoop = function () {
	/// <signature>
	/// <summary> starts the active learning loop in console</summary>
	/// <returns value =""/>
	/// </signature>

	};

	this.getnpos = function () {
	/// <signature>
	/// <summary> return the  number of examples marked as positive.</summary>
	/// <returns value ="_num"/>
	/// </signature>

	};

	this.saveSvmModel = function () {
	/// <signature>
	/// <summary> saves the binary SVM model to an output stream `fout`. The algorithm must be in SVM mode.</summary>
	/// <param name="_fout" value="_fout">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.getQueryMode = function () {
	/// <signature>
	/// <summary> returns true if in query mode, false otherwise (SVM mode)</summary>
	/// <returns value ="_bool"/>
	/// </signature>

	};

	this.getnneg = function () {
	/// <signature>
	/// <summary> return the  number of examples marked as negative.</summary>
	/// <returns value ="_num"/>
	/// </signature>

	};

}

var _model = new function () {
	this.predict = function () {
	/// <signature>
	/// <summary> predicts the target `num` (number), given feature vector `vec` based on the internal model parameters.</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	};

}

var _mat = new function () {
	/// <field name = "cols" value = "_num"> integer `num` corresponds to the number of columns of `mat`</field>
	this.cols = _num;
	this.put = function () {
	/// <signature>
	/// <summary> Sets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing.</summary>
	/// <param name="_rowIdx" value="_rowIdx">param</param>
	/// <param name="_colIdx" value="_colIdx">param</param>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.normalizeCols = function () {
	/// <signature>
	/// <summary> normalizes each column of matrix `mat` (inplace operation)</summary>
	/// <returns value =""/>
	/// </signature>

	};

	/// <field name = "rows" value = "_num"> integer `num` corresponds to the number of rows of `mat`</field>
	this.rows = _num;
	this.diag = function () {
	/// <signature>
	/// <summary> Returns the diagonal of matrix `mat` as `vec` (dense vector).</summary>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.frob = function () {
	/// <signature>
	/// <summary> number `num` is the Frobenious norm of matrix `mat`</summary>
	/// <returns value ="_num"/>
	/// </signature>

	};

	this.multiplyT = function () {
	/// <signature>
	/// <summary> Matrix transposed multiplication: `num` is a number, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	/// <signature>
	/// <summary> Matrix transposed multiplication: `vec` is a vector, `vec2` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	/// <signature>
	/// <summary> Matrix transposed multiplication: `spVec` is a sparse vector, `vec` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	/// <signature>
	/// <summary> Matrix transposed multiplication: `mat2` is a matrix, `mat3` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
	/// <param name="_mat2" value="_mat2">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	/// <signature>
	/// <summary> Matrix transposed multiplication: `spMat` is a sparse matrix, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
	/// <param name="_spMat" value="_spMat">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.getRow = function () {
	/// <signature>
	/// <summary> `vec` corresponds to the `rowIdx`-th row of dense matrix `mat`. `rowIdx` must be an integer.</summary>
	/// <param name="_rowIdx" value="_rowIdx">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.printStr = function () {
	/// <signature>
	/// <summary> print matrix `mat` to a string `str`</summary>
	/// <returns value ="_str"/>
	/// </signature>

	};

	this.multiply = function () {
	/// <signature>
	/// <summary> Matrix multiplication: `num` is a number, `mat2` is a matrix</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	/// <signature>
	/// <summary> Matrix multiplication: `vec` is a vector, `vec2` is a vector</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	/// <signature>
	/// <summary> Matrix multiplication: `spVec` is a sparse vector, `vec` is a vector</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	/// <signature>
	/// <summary> Matrix multiplication: `mat2` is a matrix, `mat3` is a matrix</summary>
	/// <param name="_mat2" value="_mat2">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	/// <signature>
	/// <summary> Matrix multiplication: `spMat` is a sparse matrix, `mat2` is a matrix</summary>
	/// <param name="_spMat" value="_spMat">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.print = function () {
	/// <signature>
	/// <summary> print matrix `mat` to console</summary>
	/// <returns value =""/>
	/// </signature>

	};

	this.plus = function () {
	/// <signature>
	/// <summary> `mat3` is the sum of matrices `mat` and `mat2`</summary>
	/// <param name="_mat2" value="_mat2">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.colNorms = function () {
	/// <signature>
	/// <summary> `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `mat`</summary>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.transpose = function () {
	/// <signature>
	/// <summary> matrix `mat2` is matrix `mat` transposed</summary>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.setCol = function () {
	/// <signature>
	/// <summary> Sets the column of a dense matrix `mat`. `colIdx` must be an integer, `vec` must be a dense vector.</summary>
	/// <param name="_colIdx" value="_colIdx">param</param>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.colMaxIdx = function () {
	/// <signature>
	/// <summary>: get the index `rowIdx` of the maximum element in column `colIdx` of dense matrix `mat`</summary>
	/// <param name="_colIdx" value="_colIdx">param</param>
	/// <returns value ="_rowIdx"/>
	/// </signature>

	};

	this.rowNorms = function () {
	/// <signature>
	/// <summary> `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th row of `mat`</summary>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.getCol = function () {
	/// <signature>
	/// <summary> `vec` corresponds to the `colIdx`-th column of dense matrix `mat`. `colIdx` must be an integer.</summary>
	/// <param name="_colIdx" value="_colIdx">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.rowMaxIdx = function () {
	/// <signature>
	/// <summary>: get the index `colIdx` of the maximum element in row `rowIdx` of dense matrix `mat`</summary>
	/// <param name="_rowIdx" value="_rowIdx">param</param>
	/// <returns value ="_colIdx"/>
	/// </signature>

	};

	this.solve = function () {
	/// <signature>
	/// <summary> vector `vec2` is the solution to the linear system `mat * vec2 = vec`</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.sparse = function () {
	/// <signature>
	/// <summary> get sparse column matrix representation `spMat` of dense matrix `mat`</summary>
	/// <returns value ="_spMat"/>
	/// </signature>

	};

	this.setRow = function () {
	/// <signature>
	/// <summary> Sets the row of a dense matrix `mat`. `rowIdx` must be an integer, `vec` must be a dense vector.</summary>
	/// <param name="_rowIdx" value="_rowIdx">param</param>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.minus = function () {
	/// <signature>
	/// <summary> `mat3` is the difference of matrices `mat` and `mat2`</summary>
	/// <param name="_mat2" value="_mat2">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.at = function () {
	/// <signature>
	/// <summary> Gets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.</summary>
	/// <param name="_rowIdx" value="_rowIdx">param</param>
	/// <param name="_colIdx" value="_colIdx">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	};

}

var _key = new function () {
	/// <field name = "fq" value = "_strArr"> gets the array of weights (as strings) in the vocabulary</field>
	this.fq = _strArr;
	/// <field name = "name" value = "_keyName"> gets the key name</field>
	this.name = _keyName;
	/// <field name = "store" value = "_storeName"> gets the store name `storeName`</field>
	this.store = _storeName;
	/// <field name = "voc" value = "_strArr"> gets the array of words (as strings) in the vocabulary</field>
	this.voc = _strArr;
}

var qm = new function () {
	this.search = function () {
	/// <signature>
	/// <summary> execute `query` (Json) specified in [QMiner Query Language](Query Language)</summary>
	/// <param name="_query" value="_query">param</param>
	/// <returns value ="_rs"/>
	/// </signature>

	};

	this.getStreamAggr = function () {
	/// <signature>
	/// <summary> gets the stream aggregate `sa` given name (string).</summary>
	/// <param name="_saName" value="_saName">param</param>
	/// <returns value ="_sa"/>
	/// </signature>

	};

	this.newStreamAggr = function () {
	/// <signature>
	/// <summary> create a new [Stream Aggregate](Stream-Aggregates) object `sa`. The constructor parameters are stored in `paramJSON` object. `paramJSON` must contain field `type` which defines the type of the aggregate.</summary>
	/// <param name="_paramJSON" value="_paramJSON">param</param>
	/// <returns value ="_sa"/>
	/// </signature>

	/// <signature>
	/// <summary> create a new [Stream Aggregate](Stream-Aggregates) object `sa`. The constructor parameters are stored in `paramJSON` object. `paramJSON` must contain field `type` which defines the type of the aggregate. Second parameter `storeName` is used to register the stream aggregate for events on the appropriate store.</summary>
	/// <param name="_paramJSON" value="_paramJSON">param</param>
	/// <param name="_storeName" value="_storeName">param</param>
	/// <returns value ="_sa"/>
	/// </signature>

	/// <signature>
	/// <summary> create a new [Stream Aggregate](Stream-Aggregates) object `sa`. The constructor parameters are stored in `paramJSON` object. `paramJSON` must contain field `type` which defines the type of the aggregate. Second parameter `storeNameArr` is an array of store names, where the stream aggregate will be registered.</summary>
	/// <param name="_paramJSON" value="_paramJSON">param</param>
	/// <param name="_storeNameArr" value="_storeNameArr">param</param>
	/// <returns value ="_sa"/>
	/// </signature>

	/// <signature>
	/// <summary> create a new [Stream Aggregate](Stream-Aggregates). The function object `funObj` defines the aggregate name and four callbacks: onAdd (takes record as input), onUpdate (takes record as input), onDelete (takes record as input) and saveJson (takes one numeric parameter - limit) callbacks. An example: `funObj = new function () {this.name = 'aggr1'; this.onAdd = function (rec) { }; this.onUpdate = function (rec) { }; this.onDelete = function (rec) { };  this.saveJson = function (limit) { return {}; } }`.</summary>
	/// <param name="_funObj" value="_funObj">param</param>
	/// <returns value ="_sa"/>
	/// </signature>

	/// <signature>
	/// <summary> create a new [Stream Aggregate](Stream-Aggregates). The function object `funObj` defines the aggregate name and four callbacks: onAdd (takes record as input), onUpdate (takes record as input), onDelete (takes record as input) and saveJson (takes one numeric parameter - limit) callbacks. An example: `funObj = new function () {this.name = 'aggr1'; this.onAdd = function (rec) { }; this.onUpdate = function (rec) { }; this.onDelete = function (rec) { };  this.saveJson = function (limit) { return {}; } }`.  Second parameter `storeName` is used to register the stream aggregate for events on the appropriate store.</summary>
	/// <param name="_funObj" value="_funObj">param</param>
	/// <param name="_storeName" value="_storeName">param</param>
	/// <returns value ="_sa"/>
	/// </signature>

	/// <signature>
	/// <summary> create a new [Stream Aggregate](Stream-Aggregates). The function object `funObj` defines the aggregate name and four callbacks: onAdd (takes record as input), onUpdate (takes record as input), onDelete (takes record as input) and saveJson (takes one numeric parameter - limit) callbacks. An example: `funObj = new function () {this.name = 'aggr1'; this.onAdd = function (rec) { }; this.onUpdate = function (rec) { }; this.onDelete = function (rec) { };  this.saveJson = function (limit) { return {}; } }`.  Second parameter `storeNameArr` is an array of store names, where the stream aggregate will be registered.</summary>
	/// <param name="_funObj" value="_funObj">param</param>
	/// <param name="_storeNameArr" value="_storeNameArr">param</param>
	/// <returns value ="_sa"/>
	/// </signature>

	/// <signature>
	/// <summary> create a new [Stream Aggregate](Stream-Aggregates). The `ftrExtObj = {type : 'ftrext', name : 'aggr1', featureSpace: fsp }` object has three parameters: `type='ftrext'`,`name` (string) and feature space `featureSpace` whose value is a feature space object.</summary>
	/// <param name="_ftrExtObj" value="_ftrExtObj">param</param>
	/// <returns value ="_sa"/>
	/// </signature>

	/// <signature>
	/// <summary> create a new [Stream Aggregate](Stream-Aggregates). The `ftrExtObj = {type : 'ftrext', name : 'aggr1', featureSpace: fsp }` object has three parameters: `type='ftrext'`,`name` (string) and feature space `featureSpace` whose value is a feature space object.  Second parameter `storeName` is used to register the stream aggregate for events on the appropriate store.</summary>
	/// <param name="_ftrExtObj" value="_ftrExtObj">param</param>
	/// <param name="_storeName" value="_storeName">param</param>
	/// <returns value ="_sa"/>
	/// </signature>

	/// <signature>
	/// <summary> create a new [Stream Aggregate](Stream-Aggregates). The `ftrExtObj = {type : 'ftrext', name : 'aggr1', featureSpace: fsp }` object has three parameters: `type='ftrext'`,`name` (string) and feature space `featureSpace` whose value is a feature space object.  Second parameter `storeNameArr` is an array of store names, where the stream aggregate will be registered.</summary>
	/// <param name="_ftrExtObj" value="_ftrExtObj">param</param>
	/// <param name="_storeNameArr" value="_storeNameArr">param</param>
	/// <returns value ="_sa"/>
	/// </signature>

	};

	this.printStreamAggr = function () {
	/// <signature>
	/// <summary> prints all current field values of every stream aggregate attached to the store `store`</summary>
	/// <param name="_store" value="_store">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.gc = function () {
	/// <signature>
	/// <summary> start garbage collection to remove records outside time windows</summary>
	/// <returns value =""/>
	/// </signature>

	};

	this.getStoreList = function () {
	/// <signature>
	/// <summary> an array of strings listing all existing stores</summary>
	/// <returns value ="_strArr"/>
	/// </signature>

	};

	this.createStore = function () {
	/// <signature>
	/// <summary> create new store(s) based on given `storeDef` (Json) [definition](Store Definition)</summary>
	/// <param name="_storeDef" value="_storeDef">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> create new store(s) based on given `storeDef` (Json) [definition](Store Definition)</summary>
	/// <param name="_storeDef" value="_storeDef">param</param>
	/// <param name="_storeSizeInMB" value="_storeSizeInMB">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.store = function () {
	/// <signature>
	/// <summary> store with name `storeName`; `store = null` when no such store</summary>
	/// <param name="_storeName" value="_storeName">param</param>
	/// <returns value ="_store"/>
	/// </signature>

	};

	this.getStreamAggrNames = function () {
	/// <signature>
	/// <summary> gets the stream aggregate names of stream aggregates in the default stream aggregate base.</summary>
	/// <returns value ="_strArr"/>
	/// </signature>

	};

}

var fs = new function () {
	this.rename = function () {
	/// <signature>
	/// <summary> rename file</summary>
	/// <param name="_fromFileName" value="_fromFileName">param</param>
	/// <param name="_toFileName" value="_toFileName">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.openWrite = function () {
	/// <signature>
	/// <summary> open file in write mode and return file output stream `fout`</summary>
	/// <param name="_fileName" value="_fileName">param</param>
	/// <returns value ="_fout"/>
	/// </signature>

	};

	this.exists = function () {
	/// <signature>
	/// <summary> does file exist?</summary>
	/// <param name="_fileName" value="_fileName">param</param>
	/// <returns value ="_bool"/>
	/// </signature>

	};

	this.openRead = function () {
	/// <signature>
	/// <summary> open file in read mode and return file input stream `fin`</summary>
	/// <param name="_fileName" value="_fileName">param</param>
	/// <returns value ="_fin"/>
	/// </signature>

	};

	this.openAppend = function () {
	/// <signature>
	/// <summary> open file in append mode and return file output stream `fout`</summary>
	/// <param name="_fileName" value="_fileName">param</param>
	/// <returns value ="_fout"/>
	/// </signature>

	};

	this.move = function () {
	/// <signature>
	/// <summary> move file</summary>
	/// <param name="_fromFileName" value="_fromFileName">param</param>
	/// <param name="_toFileName" value="_toFileName">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.mkdir = function () {
	/// <signature>
	/// <summary> make folder</summary>
	/// <param name="_dirName" value="_dirName">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.listFile = function () {
	/// <signature>
	/// <summary> returns list of files in directory given file extension</summary>
	/// <param name="_dirName" value="_dirName">param</param>
	/// <param name="_fileExtension" value="_fileExtension">param</param>
	/// <returns value ="_strArr"/>
	/// </signature>

	/// <signature>
	/// <summary> returns list of files in directory given extension. `recursive` is a boolean</summary>
	/// <param name="_dirName" value="_dirName">param</param>
	/// <param name="_fileExtension" value="_fileExtension">param</param>
	/// <param name="_recursive" value="_recursive">param</param>
	/// <returns value ="_strArr"/>
	/// </signature>

	};

	this.del = function () {
	/// <signature>
	/// <summary> delete file</summary>
	/// <param name="_fileName" value="_fileName">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.rmdir = function () {
	/// <signature>
	/// <summary> delete folder</summary>
	/// <param name="_dirName" value="_dirName">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.fileInfo = function () {
	/// <signature>
	/// <summary> returns file info as a json object {createTime:str, lastAccessTime:str, lastWriteTime:str, size:num}.</summary>
	/// <param name="_fileName" value="_fileName">param</param>
	/// <returns value ="_fileInfoJson"/>
	/// </signature>

	};

	this.copy = function () {
	/// <signature>
	/// <summary> copy file</summary>
	/// <param name="_fromFileName" value="_fromFileName">param</param>
	/// <param name="_toFileName" value="_toFileName">param</param>
	/// <returns value =""/>
	/// </signature>

	};

}

var _perceptronModel = new function () {
	this.predict = function () {
	/// <signature>
	/// <summary> returns the prediction (0 or 1) for a vector `vec`</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	/// <signature>
	/// <summary> returns the prediction (0 or 1) for a sparse vector `spVec`</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	};

	this.getModel = function () {
	/// <signature>
	/// <summary> returns an object `perceptronParam` where `perceptronParam.w` (vector) and `perceptronParam.b` (bias) are the separating hyperplane normal and bias.</summary>
	/// <returns value ="_perceptronParam"/>
	/// </signature>

	};

	this.update = function () {
	/// <signature>
	/// <summary> updates the internal parameters `w` and `b` based on the training feature vector `vec` and target class `num` (0 or 1)!</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> updates the internal parameters `w` and `b` based on the training sparse feature vector `spVec` and target class `num` (0 or 1)!</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	};

}

var _fsp = new function () {
	/// <field name = "dim" value = "_num"> dimensionality of feature space</field>
	this.dim = _num;
	this.ftrSpColMat = function () {
	/// <signature>
	/// <summary> extracts sparse feature vectors from</summary>
	/// <param name="_rs" value="_rs">param</param>
	/// <returns value ="_spMat"/>
	/// </signature>

	};

	this.extractStrings = function () {
	/// <signature>
	/// <summary> use feature extractors to extract string</summary>
	/// <param name="_rec" value="_rec">param</param>
	/// <returns value ="_strArr"/>
	/// </signature>

	};

	this.getFtr = function () {
	/// <signature>
	/// <summary> returns the name `ftrName` (string) of `idx`-th feature in feature space `fsp`</summary>
	/// <param name="_idx" value="_idx">param</param>
	/// <returns value ="_ftrName"/>
	/// </signature>

	};

	this.updateRecord = function () {
	/// <signature>
	/// <summary> update feature space definitions and extractors</summary>
	/// <param name="_rec" value="_rec">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.ftrSpVec = function () {
	/// <signature>
	/// <summary> extracts sparse feature vector `spVec` from record `rec`</summary>
	/// <param name="_rec" value="_rec">param</param>
	/// <returns value ="_spVec"/>
	/// </signature>

	};

	this.ftrVec = function () {
	/// <signature>
	/// <summary> extracts feature vector `vec` from record  `rec`</summary>
	/// <param name="_rec" value="_rec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.ftrColMat = function () {
	/// <signature>
	/// <summary> extracts feature vectors from</summary>
	/// <param name="_rs" value="_rs">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.save = function () {
	/// <signature>
	/// <summary> serialize feature space to `fout` output stream</summary>
	/// <param name="_fout" value="_fout">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.updateRecords = function () {
	/// <signature>
	/// <summary> update feature space definitions and extractors</summary>
	/// <param name="_rs" value="_rs">param</param>
	/// <returns value =""/>
	/// </signature>

	};

}

var _nnModel = new function () {
	this.predict = function () {
	/// <signature>
	/// <summary> sends vector `vec` through the model and returns the prediction as a vector `vec2`</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	};

	this.learn = function () {
	/// <signature>
	/// <summary> uses a pair of input `inVec` and output `outVec` to perform one step of learning with backpropagation.</summary>
	/// <param name="_inVec" value="_inVec">param</param>
	/// <param name="_outVec" value="_outVec">param</param>
	/// <returns value =""/>
	/// </signature>

	};

}

var _intVec = new function () {
	this.sort = function () {
	/// <signature>
	/// <summary> integer vector `intVec2` is a sorted copy of integer vector `intVec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order</summary>
	/// <param name="_asc" value="_asc">param</param>
	/// <returns value ="_intVec"/>
	/// </signature>

	};

	this.toMat = function () {
	/// <signature>
	/// <summary> `mat` is a matrix with a single column that is equal to dense integer vector `intVec`.</summary>
	/// <returns value ="_mat"/>
	/// </signature>

	};

	this.getMaxIdx = function () {
	/// <signature>
	/// <summary> returns the integer index `idx` of the maximal element in integer vector `vec`</summary>
	/// <returns value ="_idx"/>
	/// </signature>

	};

	this.pushV = function () {
	/// <signature>
	/// <summary> append integer vector `intVec2` to integer vector `intVec`.</summary>
	/// <param name="_intVec2" value="_intVec2">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.unshift = function () {
	/// <signature>
	/// <summary> insert value `num` to the begining of integer vector `intVec`. Returns the length of the modified array.</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_len"/>
	/// </signature>

	};

	this.sum = function () {
	/// <signature>
	/// <summary> return `num`: the sum of elements of integer vector `intVec`</summary>
	/// <returns value ="_num"/>
	/// </signature>

	};

	this.put = function () {
	/// <signature>
	/// <summary> set value of integer vector `intVec` at index `idx` to `num` (0-based indexing)</summary>
	/// <param name="_idx" value="_idx">param</param>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.push = function () {
	/// <signature>
	/// <summary> append value `num` to integer vector `intVec`. Returns `len` - the length  of the modified array</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_len"/>
	/// </signature>

	};

	/// <field name = "length" value = "_len"> integer `len` is the length of integer vector `vec`</field>
	this.length = _len;
	this.at = function () {
	/// <signature>
	/// <summary> gets the value `num` of integer vector `intVec` at index `idx`  (0-based indexing)</summary>
	/// <param name="_idx" value="_idx">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	};

	this.print = function () {
	/// <signature>
	/// <summary> print integer vector in console</summary>
	/// <returns value =""/>
	/// </signature>

	};

	this.subVec = function () {
	/// <signature>
	/// <summary> gets the subvector based on an index vector `intVec` (indices can repeat, 0-based indexing)</summary>
	/// <param name="_intVec" value="_intVec">param</param>
	/// <returns value ="_intVec"/>
	/// </signature>

	};

}

var _twitter = new function () {
	this.getTwitterStoreJson = function () {
	/// <signature>
	/// <summary> returns a Twitter store definition JSON object `twitterDef`. The JSON array contains four store definitions: Tweets, Users, HashTags and Pages</summary>
	/// <returns value ="_twitterDef"/>
	/// </signature>

	};

	this.newParser = function () {
	/// <signature>
	/// <summary> creates an object that converts between raw Twitter JSON objects and qminer store compatible JSON objects. Exposes:</summary>
	/// <returns value ="_twitterParser"/>
	/// </signature>

	};

	this.RawToStore = function () {
	/// <signature>
	/// <summary> converts twitter JSON lines to `twitter.getTwitterStoreJson()` compatible JSON lines, given input stream `fin` (raw JSON lines) and output stream `fout` (store JSON lines file)</summary>
	/// <param name="_fin" value="_fin">param</param>
	/// <param name="_fout" value="_fout">param</param>
	/// <returns value =""/>
	/// </signature>

	};

}

var _sw = new function () {
	this.reset = function () {
	/// <signature>
	/// <summary> resets</summary>
	/// <returns value =""/>
	/// </signature>

	};

	this.saytime = function () {
	/// <signature>
	/// <summary> displays elpased time from tic</summary>
	/// <param name="_message" value="_message">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.stop = function () {
	/// <signature>
	/// <summary> stops the stopwatch</summary>
	/// <returns value =""/>
	/// </signature>

	};

	this.start = function () {
	/// <signature>
	/// <summary> starts the stopwatch</summary>
	/// <returns value =""/>
	/// </signature>

	};

	this.time = function () {
	/// <signature>
	/// <summary> returns unix epoch time in milliseconds</summary>
	/// <returns value ="_num"/>
	/// </signature>

	};

	this.toc = function () {
	/// <signature>
	/// <summary> displays time from tic and message `str`</summary>
	/// <param name="_str" value="_str">param</param>
	/// <returns value =""/>
	/// </signature>

	};

	this.tic = function () {
	/// <signature>
	/// <summary> resets and starts the stop watch</summary>
	/// <returns value =""/>
	/// </signature>

	};

}


// Manual override

_vec[0] = 1; // vec is indexed and returns numbers
_intVec[0] = 1; // vec is indexed and returns numbers
_spMat[0] = _spVec; // spMat is indexed and returns sparse column vectors
_rs[0] = _rec; // record set at index returns a record
_store[0] = _rec; // store index operator returns a record
_addIntellisenseVar("_rsArr", "[_rs]");
