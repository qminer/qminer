var _store = {
	key: function () {
	/// <signature>
	/// <summary> get [index key](#index-key) named `keyName`</summary>
	/// <param name="_keyName" value="_keyName">param</param>
	/// <returns value ="_key"/>
	/// </signature>

	},

	rec: function () {
	/// <signature>
	/// <summary> get record named `recName`;</summary>
	/// <param name="_recName" value="_recName">param</param>
	/// <returns value ="_rec"/>
	/// </signature>

	},

	addTrigger: function () {
	/// <signature>
	/// <summary> add `trigger` to the store triggers. Trigger is a JS object with three properties `onAdd`, `onUpdate`, `onDelete` whose values are callbacks</summary>
	/// <param name="_trigger" value="_trigger">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	/// <field value = "_objArr"> array of all the [index keys](#index-key) objects</field>
	keys: _objArr,
	/// <field value = "_objArr"> array of all the field descriptor JSON objects</field>
	fields: _objArr,
	/// <field value = "_rs"> create a record set containing all the records from the store</field>
	recs: _rs,
	addStreamAggr: function () {
	/// <signature>
	/// <summary> add new [Stream Aggregate](Stream-Aggregates). Stream aggregate is defined by `paramJSON` object</summary>
	/// <param name="_paramJSON" value="_paramJSON">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	getStreamAggr: function () {
	/// <signature>
	/// <summary> returns current JSON value of stream aggregate `saName`</summary>
	/// <param name="_saName" value="_saName">param</param>
	/// <returns value ="_objJSON"/>
	/// </signature>

	},

	newRec: function () {
	/// <signature>
	/// <summary> creates new record `rec` by (JSON) value `recordJson` (not added to the store)</summary>
	/// <param name="_recordJson" value="_recordJson">param</param>
	/// <returns value ="_rec"/>
	/// </signature>

	},

	/// <field value = "_str"> name of the store</field>
	name: _str,
	sample: function () {
	/// <signature>
	/// <summary> create a record set containing a random</summary>
	/// <param name="_sampleSize" value="_sampleSize">param</param>
	/// <returns value ="_rs"/>
	/// </signature>

	},

	field: function () {
	/// <signature>
	/// <summary> get details of field named `fieldName`</summary>
	/// <param name="_fieldName" value="_fieldName">param</param>
	/// <returns value ="_field"/>
	/// </signature>

	},

	add: function () {
	/// <signature>
	/// <summary> add record `rec` to the store and return its ID `recId`</summary>
	/// <param name="_rec" value="_rec">param</param>
	/// <returns value ="_recId"/>
	/// </signature>

	},

	/// <field value = "_objArr"> array of all the join names</field>
	joins: _objArr,
	newRecSet: function () {
	/// <signature>
	/// <summary> creates new record set from an integer vector record IDs `idVec` (type la.newIntVec);</summary>
	/// <param name="_idVec" value="_idVec">param</param>
	/// <returns value ="_rs"/>
	/// </signature>

	},

	/// <field value = "_len"> number of records in the store</field>
	length: _len,
	/// <field value = "_bool"> `bool = true` when store is empty</field>
	empty: _bool,
	getStreamAggrNames: function () {
	/// <signature>
	/// <summary> returns the names of all stream aggregators as an array of strings `strArr`</summary>
	/// <returns value ="_strArr"/>
	/// </signature>

	},

}

var process = {
	/// <field value = "_str"> Returns the name of the script.</field>
	scriptNm: _str,
	/// <field value = "_str"> Returns absolute script file path.</field>
	scriptFNm: _str,
	isArg: function () {
	/// <signature>
	/// <summary> returns true when `argStr` among the</summary>
	/// <param name="_argStr" value="_argStr">param</param>
	/// <returns value ="_bool"/>
	/// </signature>

	},

	/// <field value = "_a"> array of command-line arguments</field>
	args: _a,
	stop: function () {
	/// <signature>
	/// <summary> Stops the current process.</summary>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> Stops the current process and returns `returnCode</summary>
	/// <param name="_returnCode" value="_returnCode">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	/// <field value = "_objJSON"> statistics about system and qminer process (E.g. memory consumption).</field>
	sysStat: _objJSON,
	sleep: function () {
	/// <signature>
	/// <summary> Halts execution for the given amount of milliseconds `millis`.</summary>
	/// <param name="_millis" value="_millis">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	getGlobals: function () {
	/// <signature>
	/// <summary> Returns an array of all global variable names</summary>
	/// <returns value ="_globalVarNames"/>
	/// </signature>

	},

}

var _vec = {
	multiply: function () {
	/// <signature>
	/// <summary>`vec2` is a vector obtained by multiplying vector `vec` with a scalar (number) `num`. Implemented for dense float vectors only.</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_vec2"/>
	/// </signature>

	},

	at: function () {
	/// <signature>
	/// <summary> gets the value `num` of vector `vec` at index `idx`  (0-based indexing)</summary>
	/// <param name="_idx" value="_idx">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	},

	normalize: function () {
	/// <signature>
	/// <summary> normalizes the vector `vec` (inplace operation). Implemented for dense float vectors only.</summary>
	/// <returns value =""/>
	/// </signature>

	},

	outer: function () {
	/// <signature>
	/// <summary> the dense matrix `mat` is a rank-1 matrix obtained by multiplying `vec * vec2^T`. Implemented for dense float vectors only.</summary>
	/// <param name="_vec2" value="_vec2">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	},

	toMat: function () {
	/// <signature>
	/// <summary> `mat` is a matrix with a single column that is equal to dense vector `vec`.</summary>
	/// <returns value ="_mat"/>
	/// </signature>

	},

	pushV: function () {
	/// <signature>
	/// <summary> append vector `vec2` to vector `vec`.</summary>
	/// <param name="_vec2" value="_vec2">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	diag: function () {
	/// <signature>
	/// <summary> `mat` is a diagonal dense matrix whose diagonal equals `vec`. Implemented for dense float vectors only.</summary>
	/// <returns value ="_mat"/>
	/// </signature>

	},

	sum: function () {
	/// <signature>
	/// <summary> return `num`: the sum of elements of vector `vec`</summary>
	/// <returns value ="_num"/>
	/// </signature>

	},

	unshift: function () {
	/// <signature>
	/// <summary> insert value `num` to the begining of vector `vec`. Returns the length of the modified array.</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_len"/>
	/// </signature>

	},

	inner: function () {
	/// <signature>
	/// <summary> `num` is the standard dot product between vectors `vec` and `vec2`. Implemented for dense float vectors only.</summary>
	/// <param name="_vec2" value="_vec2">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	},

	spDiag: function () {
	/// <signature>
	/// <summary> `spMat` is a diagonal sparse matrix whose diagonal equals `vec`. Implemented for dense float vectors only.</summary>
	/// <returns value ="_spMat"/>
	/// </signature>

	},

	print: function () {
	/// <signature>
	/// <summary> print vector in console</summary>
	/// <returns value =""/>
	/// </signature>

	},

	subVec: function () {
	/// <signature>
	/// <summary> gets the subvector based on an index vector `intVec` (indices can repeat, 0-based indexing)</summary>
	/// <param name="_intVec" value="_intVec">param</param>
	/// <returns value ="_vec2"/>
	/// </signature>

	},

	sort: function () {
	/// <signature>
	/// <summary> `vec2` is a sorted copy of `vec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order</summary>
	/// <param name="_asc" value="_asc">param</param>
	/// <returns value ="_vec2"/>
	/// </signature>

	},

	sortPerm: function () {
	/// <signature>
	/// <summary> returns a sorted copy of the vector in `sortRes.vec` and the permutation `sortRes.perm`. `asc=true` sorts in ascending order (equivalent `sortPerm()`), `asc`=false sorts in descending order.</summary>
	/// <param name="_asc" value="_asc">param</param>
	/// <returns value ="_sortRes"/>
	/// </signature>

	},

	put: function () {
	/// <signature>
	/// <summary> set value of vector `vec` at index `idx` to `num` (0-based indexing)</summary>
	/// <param name="_idx" value="_idx">param</param>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	getMaxIdx: function () {
	/// <signature>
	/// <summary> returns the integer index `idx` of the maximal element in vector `vec`</summary>
	/// <returns value ="_idx"/>
	/// </signature>

	},

	/// <field value = "_len"> integer `len` is the length of vector `vec`</field>
	length: _len,
	plus: function () {
	/// <signature>
	/// <summary>`vec3` is the sum of vectors `vec` and `vec2`. Implemented for dense float vectors only.</summary>
	/// <param name="_vec2" value="_vec2">param</param>
	/// <returns value ="_vec3"/>
	/// </signature>

	},

	norm: function () {
	/// <signature>
	/// <summary> `num` is the Euclidean norm of `vec`. Implemented for dense float vectors only.</summary>
	/// <returns value ="_num"/>
	/// </signature>

	},

	sparse: function () {
	/// <signature>
	/// <summary> `spVec` is a sparse vector representation of dense vector `vec`. Implemented for dense float vectors only.</summary>
	/// <returns value ="_spVec"/>
	/// </signature>

	},

	push: function () {
	/// <signature>
	/// <summary> append value `num` to vector `vec`. Returns `len` - the length  of the modified array</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_len"/>
	/// </signature>

	},

	minus: function () {
	/// <signature>
	/// <summary>`vec3` is the difference of vectors `vec` and `vec2`. Implemented for dense float vectors only.</summary>
	/// <param name="_vec2" value="_vec2">param</param>
	/// <returns value ="_vec3"/>
	/// </signature>

	},

}

var _ridgeRegressionModel = {
	compute: function () {
	/// <signature>
	/// <summary> computes the model parameters `vec2`, given</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_vec2"/>
	/// </signature>

	/// <signature>
	/// <summary> computes the model parameters `vec2`, given</summary>
	/// <param name="_spMat" value="_spMat">param</param>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_vec2"/>
	/// </signature>

	},

	forget: function () {
	/// <signature>
	/// <summary> deletes first `n` (integer) examples from the training set</summary>
	/// <param name="_n" value="_n">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	getModel: function () {
	/// <signature>
	/// <summary> returns the parameter vector `vec` (dense vector)</summary>
	/// <returns value ="_vec"/>
	/// </signature>

	},

	update: function () {
	/// <signature>
	/// <summary> recomputes the model</summary>
	/// <returns value =""/>
	/// </signature>

	},

	addupdate: function () {
	/// <signature>
	/// <summary> adds a vector `vec` and target `num` (number) to the training set and retrains the model</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	add: function () {
	/// <signature>
	/// <summary> adds a vector `vec` and target `num` (number) to the training set</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	},

}

var _hashTable = {
	/// <field value = "_strArr"> array of keys (strings)</field>
	keys: _strArr,
	put: function () {
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

	},

	contains: function () {
	/// <signature>
	/// <summary> does the table contain the key?</summary>
	/// <param name="_key" value="_key">param</param>
	/// <returns value ="_bool"/>
	/// </signature>

	},

	/// <field value = "_array"> array of values</field>
	vals: _array,
	get: function () {
	/// <signature>
	/// <summary> get data</summary>
	/// <param name="_key" value="_key">param</param>
	/// <returns value ="_dat"/>
	/// </signature>

	},

}

var _rec = {
	delJoin: function () {
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

	},

	addJoin: function () {
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

	},

	/// <field value = "_recId"> returns record ID</field>
	$id: _recId,
	/// <field value = "_recFq"> returns record frequency (used for randomized joins)</field>
	$fq: _recFq,
	/// <field value = "_recName"> returns record name</field>
	$name: _recName,
	toJSON: function () {
	/// <signature>
	/// <summary> provide json version of record, useful when calling JSON.stringify</summary>
	/// <returns value ="_objJSON"/>
	/// </signature>

	},

}

var _fin = {
	getCh: function () {
	/// <signature>
	/// <summary> reads a character</summary>
	/// <returns value ="_char"/>
	/// </signature>

	},

	peekCh: function () {
	/// <signature>
	/// <summary> peeks a character</summary>
	/// <returns value ="_char"/>
	/// </signature>

	},

	/// <field value = "_len"> returns the length of input stream</field>
	length: _len,
	/// <field value = "_bool"> end of stream?</field>
	eof: _bool,
	readLine: function () {
	/// <signature>
	/// <summary> reads a line</summary>
	/// <returns value ="_line"/>
	/// </signature>

	},

}

var _analytics = {
	newBatchModel: function () {
	/// <signature>
	/// <summary> learns a new batch model</summary>
	/// <param name="_rs" value="_rs">param</param>
	/// <param name="_fsp" value="_fsp">param</param>
	/// <param name="_target" value="_target">param</param>
	/// <returns value ="_batchModel"/>
	/// </signature>

	},

	newRecLinReg: function () {
	/// <signature>
	/// <summary> create new recursive linear regression</summary>
	/// <param name="_recLinRegParameters" value="_recLinRegParameters">param</param>
	/// <returns value ="_recLinRegModel"/>
	/// </signature>

	},

	loadRecLinRegModel: function () {
	/// <signature>
	/// <summary> load serialized linear model</summary>
	/// <param name="_fin" value="_fin">param</param>
	/// <returns value ="_recLinRegModel"/>
	/// </signature>

	},

	computeKmeans: function () {
	/// <signature>
	/// <summary> solves the k-means algorithm based on a training</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <param name="_k" value="_k">param</param>
	/// <param name="_iter" value="_iter">param</param>
	/// <returns value ="_mat2"/>
	/// </signature>

	/// <signature>
	/// <summary> solves the k-means algorithm based on a training</summary>
	/// <param name="_spMat" value="_spMat">param</param>
	/// <param name="_k" value="_k">param</param>
	/// <param name="_iter" value="_iter">param</param>
	/// <returns value ="_mat2"/>
	/// </signature>

	},

	newHoeffdingTree: function () {
	/// <signature>
	/// <summary> create new</summary>
	/// <param name="_jsonStream" value="_jsonStream">param</param>
	/// <param name="_htJsonParams" value="_htJsonParams">param</param>
	/// <returns value ="_htModel"/>
	/// </signature>

	},

	loadSvmModel: function () {
	/// <signature>
	/// <summary> load serialized linear model</summary>
	/// <param name="_fin" value="_fin">param</param>
	/// <returns value ="_svmModel"/>
	/// </signature>

	},

	getLanguageOptions: function () {
	/// <signature>
	/// <summary> get options for text parsing</summary>
	/// <returns value ="_langOptionsJson"/>
	/// </signature>

	},

	newNN: function () {
	/// <signature>
	/// <summary> create new neural network</summary>
	/// <param name="_nnParameters" value="_nnParameters">param</param>
	/// <returns value ="_nnModel"/>
	/// </signature>

	},

	loadFeatureSpace: function () {
	/// <signature>
	/// <summary> load serialized feature</summary>
	/// <param name="_fin" value="_fin">param</param>
	/// <returns value ="_fsp"/>
	/// </signature>

	},

	newFeatureSpace: function () {
	/// <signature>
	/// <summary> create new</summary>
	/// <param name="_featureExtractors" value="_featureExtractors">param</param>
	/// <returns value ="_fsp"/>
	/// </signature>

	},

	trainSvmRegression: function () {
	/// <signature>
	/// <summary> trains</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <param name="_vec" value="_vec">param</param>
	/// <param name="_svmRegParameters" value="_svmRegParameters">param</param>
	/// <returns value ="_svmModel"/>
	/// </signature>

	},

	newLloyd: function () {
	/// <signature>
	/// <summary> online clustering based on the Lloyd alogrithm. The model intialization</summary>
	/// <param name="_dim" value="_dim">param</param>
	/// <param name="_k" value="_k">param</param>
	/// <returns value ="_lloydModel"/>
	/// </signature>

	},

	newActiveLearner: function () {
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

	},

	loadBatchModel: function () {
	/// <signature>
	/// <summary> loads batch model frm input stream `fin`</summary>
	/// <param name="_fin" value="_fin">param</param>
	/// <returns value ="_batchModel"/>
	/// </signature>

	},

	trainSvmClassify: function () {
	/// <signature>
	/// <summary> trains binary</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <param name="_vec" value="_vec">param</param>
	/// <param name="_svmParameters" value="_svmParameters">param</param>
	/// <returns value ="_svmModel"/>
	/// </signature>

	},

	newPerceptron: function () {
	/// <signature>
	/// <summary> the perceptron learning algorithm initialization requires</summary>
	/// <param name="_dim" value="_dim">param</param>
	/// <param name="_use_bias" value="_use_bias">param</param>
	/// <returns value ="_perceptronModel"/>
	/// </signature>

	},

	newRidgeRegression: function () {
	/// <signature>
	/// <summary> solves a regularized ridge</summary>
	/// <param name="_kappa" value="_kappa">param</param>
	/// <param name="_dim" value="_dim">param</param>
	/// <param name="_buffer" value="_buffer">param</param>
	/// <returns value ="_ridgeRegressionModel"/>
	/// </signature>

	},

}

var console = {
	startx: function () {
	/// <signature>
	/// <summary> useful for debugging;</summary>
	/// <param name="_evalFun" value="_evalFun">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	pause: function () {
	/// <signature>
	/// <summary> waits until enter is pressed</summary>
	/// <returns value =""/>
	/// </signature>

	},

	log: function () {
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

	},

	start: function () {
	/// <signature>
	/// <summary> start interactive console; does not see local variables.</summary>
	/// <returns value =""/>
	/// </signature>

	},

	getln: function () {
	/// <signature>
	/// <summary> reads a line from command line and returns</summary>
	/// <returns value ="_line"/>
	/// </signature>

	},

	println: function () {
	/// <signature>
	/// <summary> `print(str); print("\n")`</summary>
	/// <param name="_str" value="_str">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	print: function () {
	/// <signature>
	/// <summary> prints a string to standard output</summary>
	/// <param name="_str" value="_str">param</param>
	/// <returns value =""/>
	/// </signature>

	},

}

var la = {
	newVec: function () {
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

	},

	square: function () {
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

	},

	printArray: function () {
	/// <signature>
	/// <summary> print the javascript array `arr` in the console</summary>
	/// <param name="_arr" value="_arr">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	newSpMat: function () {
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

	},

	genRandomMatrix: function () {
	/// <signature>
	/// <summary> `mat` is a dense matrix whose elements are independent samples from a standard normal random variable, with `rows` rows and `cols` columns (integers)</summary>
	/// <param name="_rows" value="_rows">param</param>
	/// <param name="_cols" value="_cols">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	},

	copyFltArrayToVec: function () {
	/// <signature>
	/// <summary> copies a JS array of numbers `arr` into a float vector `vec`</summary>
	/// <param name="_arr" value="_arr">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	},

	genRandomVector: function () {
	/// <signature>
	/// <summary> `vec` is a dense vector whose elements are independent samples from a standard normal random variable and whos dimension is `dim`</summary>
	/// <param name="_dim" value="_dim">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	},

	repmat: function () {
	/// <signature>
	/// <summary> creates a matrix `mat2` consisting of an `m`-by-`n` tiling of copies of `mat`</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <param name="_m" value="_m">param</param>
	/// <param name="_n" value="_n">param</param>
	/// <returns value ="_mat2"/>
	/// </signature>

	},

	randIntVec: function () {
	/// <signature>
	/// <summary> returns a JS array `vec`, which is a sample of `k` numbers from `[0,...,num]`, sampled without replacement. `k` must be smaller or equal to `num`</summary>
	/// <param name="_num" value="_num">param</param>
	/// <param name="_k" value="_k">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	},

	rangeVec: function () {
	/// <signature>
	/// <summary> `intVec` is an integer vector: `[num, num + 1, ..., num2].</summary>
	/// <param name="_num" value="_num">param</param>
	/// <param name="_num2" value="_num2">param</param>
	/// <returns value ="_intVec"/>
	/// </signature>

	},

	repvec: function () {
	/// <signature>
	/// <summary> creates a matrix `mat2` consisting of an `m`-by-`n` tiling of copies of `vec`</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <param name="_m" value="_m">param</param>
	/// <param name="_n" value="_n">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	},

	printSpFeatVec: function () {
	/// <signature>
	/// <summary> Print a sparse feature vector `spVec` along with feature names based on feature space `fsp`. If third parameter is ommited, the elements are sorted by dimension number. If boolean parameter `asc` is used, then the rows are sorted by (non-zero) vector values. Use `asc=true` for sorting in ascending order and `asc=false` for sorting in descending order.</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <param name="_fsp" value="_fsp">param</param>
	/// <param name="_asc" value="_asc">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	randInt: function () {
	/// <signature>
	/// <summary> returns an integer `num2` which is randomly selected from the set of integers `[0, ..., num]`</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_num2"/>
	/// </signature>

	},

	newSpVec: function () {
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

	},

	pdist2: function () {
	/// <signature>
	/// <summary> computes the pairwise squared euclidean distances between columns of `mat` and `mat2`. mat3[i,j] = ||mat(:,i) - mat2(:,j)||^2</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <param name="_mat2" value="_mat2">param</param>
	/// <returns value ="_mat3"/>
	/// </signature>

	},

	printVec: function () {
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

	},

	newIntVec: function () {
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

	},

	speye: function () {
	/// <signature>
	/// <summary> `spMat` is a `dim`-by-`dim` sparse identity matrix</summary>
	/// <param name="_dim" value="_dim">param</param>
	/// <returns value ="_spMat"/>
	/// </signature>

	},

	printMat: function () {
	/// <signature>
	/// <summary> print the matrix `mat` in the console</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	ones: function () {
	/// <signature>
	/// <summary> `vec` is a `k`-dimensional vector whose entries are set to `1.0`.</summary>
	/// <param name="_k" value="_k">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	},

	eye: function () {
	/// <signature>
	/// <summary> `mat` is a `dim`-by-`dim` identity matrix</summary>
	/// <param name="_dim" value="_dim">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	},

	saveMat: function () {
	/// <signature>
	/// <summary> writes a dense matrix `mat` to output file stream `fout`</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <param name="_fout" value="_fout">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	genRandomPerm: function () {
	/// <signature>
	/// <summary> returns a permutation of `k` elements. `arr` is a javascript array of integers</summary>
	/// <param name="_k" value="_k">param</param>
	/// <returns value ="_arr"/>
	/// </signature>

	},

	conjgrad: function () {
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

	},

	newMat: function () {
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

	},

	genRandom: function () {
	/// <signature>
	/// <summary> `num` is a sample from a standard normal random variable</summary>
	/// <returns value ="_num"/>
	/// </signature>

	},

	copyIntArrayToVec: function () {
	/// <signature>
	/// <summary> copies a JS array of integers `arr` into an integer vector `intVec`</summary>
	/// <param name="_arr" value="_arr">param</param>
	/// <returns value ="_intVec"/>
	/// </signature>

	},

	findMaxIdx: function () {
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

	},

	printFeatVec: function () {
	/// <signature>
	/// <summary> Print a feature vector `vec` along with feature names based on feature space `fsp`. The parameter `limit` (integer) is optional and limits the number of rows printed (prints all values by default). If the fourth parameter is ommited, the elements are sorted by dimension number. If boolean parameter `asc` is used, then the rows are sorted by (non-zero) vector values. Use `asc=true` for sorting in ascending order and `asc=false` for sorting in descending order.</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <param name="_fsp" value="_fsp">param</param>
	/// <param name="_limit" value="_limit">param</param>
	/// <param name="_asc" value="_asc">param</param>
	/// <returns value =""/>
	/// </signature>

	},

}

var _spMat = {
	load: function () {
	/// <signature>
	/// <summary> load `spMat` (sparse matrix) from input steam `fin`. `spMat` has to be initialized first, for example using `spMat = la.newSpMat()`.</summary>
	/// <param name="_fin" value="_fin">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	full: function () {
	/// <signature>
	/// <summary> get dense matrix representation `mat` of `spMat (sparse column matrix)`</summary>
	/// <returns value ="_mat"/>
	/// </signature>

	},

	normalizeCols: function () {
	/// <signature>
	/// <summary> normalizes each column of a sparse matrix `spMat` (inplace operation)</summary>
	/// <returns value =""/>
	/// </signature>

	},

	push: function () {
	/// <signature>
	/// <summary> attaches a column `spVec` (sparse vector) to `spMat` (sparse matrix)</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	transpose: function () {
	/// <signature>
	/// <summary> `spMat2` (sparse matrix) is `spMat` (sparse matrix) transposed</summary>
	/// <returns value ="_spMat2"/>
	/// </signature>

	},

	frob: function () {
	/// <signature>
	/// <summary> number `num` is the Frobenious norm of `spMat` (sparse matrix)</summary>
	/// <returns value ="_num"/>
	/// </signature>

	},

	put: function () {
	/// <signature>
	/// <summary> Sets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing.</summary>
	/// <param name="_rowIdx" value="_rowIdx">param</param>
	/// <param name="_colIdx" value="_colIdx">param</param>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	/// <field value = "_num"> integer `num` corresponds to the number of columns of `spMat` (sparse matrix)</field>
	cols: _num,
	multiplyT: function () {
	/// <signature>
	/// <summary> Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_spMat2"/>
	/// </signature>

	/// <signature>
	/// <summary> Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_vec2"/>
	/// </signature>

	/// <signature>
	/// <summary> Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	/// <signature>
	/// <summary> Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <returns value ="_mat2"/>
	/// </signature>

	/// <signature>
	/// <summary> Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient.</summary>
	/// <param name="_spMat2" value="_spMat2">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	},

	print: function () {
	/// <signature>
	/// <summary> print `spMat` (sparse matrix) to console</summary>
	/// <returns value =""/>
	/// </signature>

	},

	nnz: function () {
	/// <signature>
	/// <summary> `num` is the number of non-zero elements of sparse column matrix `spMat`</summary>
	/// <returns value ="_num"/>
	/// </signature>

	},

	/// <field value = "_num"> integer `num` corresponds to the number of rows of `spMat` (sparse matrix)</field>
	rows: _num,
	plus: function () {
	/// <signature>
	/// <summary> `spMat3` is the sum of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)</summary>
	/// <param name="_spMat2" value="_spMat2">param</param>
	/// <returns value ="_spMat3"/>
	/// </signature>

	},

	at: function () {
	/// <signature>
	/// <summary> Gets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.</summary>
	/// <param name="_rowIdx" value="_rowIdx">param</param>
	/// <param name="_colIdx" value="_colIdx">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	},

	multiply: function () {
	/// <signature>
	/// <summary> Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_spMat2"/>
	/// </signature>

	/// <signature>
	/// <summary> Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_vec2"/>
	/// </signature>

	/// <signature>
	/// <summary> Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	/// <signature>
	/// <summary> Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <returns value ="_mat2"/>
	/// </signature>

	/// <signature>
	/// <summary> Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix</summary>
	/// <param name="_spMat2" value="_spMat2">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	},

	toString: function () {
	/// <signature>
	/// <summary> returns a string displaying rows, columns and number of non-zero elements of a sparse column matrix `spMat`</summary>
	/// <returns value ="_str"/>
	/// </signature>

	},

	save: function () {
	/// <signature>
	/// <summary> print `spMat` (sparse matrix) to output stream `fout`</summary>
	/// <param name="_fout" value="_fout">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	minus: function () {
	/// <signature>
	/// <summary> `spMat3` is the difference of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)</summary>
	/// <param name="_spMat2" value="_spMat2">param</param>
	/// <returns value ="_spMat3"/>
	/// </signature>

	},

	colNorms: function () {
	/// <signature>
	/// <summary> `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `spMat`</summary>
	/// <returns value ="_vec"/>
	/// </signature>

	},

}

var _batchModel = {
	predict: function () {
	/// <signature>
	/// <summary> creates feature vector from record `rec`, sends it</summary>
	/// <param name="_rec" value="_rec">param</param>
	/// <returns value ="_scoreArr"/>
	/// </signature>

	},

	save: function () {
	/// <signature>
	/// <summary> saves the model to `fout` output stream</summary>
	/// <param name="_fout" value="_fout">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	/// <field value = "_strArr"> array of categories for which we have models</field>
	target: _strArr,
	predictTop: function () {
	/// <signature>
	/// <summary> creates feature vector from record `rec`,</summary>
	/// <param name="_rec" value="_rec">param</param>
	/// <returns value ="_labelStr"/>
	/// </signature>

	},

}

var _httpResponse = {
	setStatusCode: function () {
	/// <signature>
	/// <summary> sets status code (integer)</summary>
	/// <param name="_statusCode" value="_statusCode">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	close: function () {
	/// <signature>
	/// <summary> closes and executes the response</summary>
	/// <returns value =""/>
	/// </signature>

	},

	add: function () {
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

	},

	send: function () {
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

	},

	setContentType: function () {
	/// <signature>
	/// <summary> sets content type (string)</summary>
	/// <param name="_mimeType" value="_mimeType">param</param>
	/// <returns value =""/>
	/// </signature>

	},

}

var _svmModel = {
	predict: function () {
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

	},

	save: function () {
	/// <signature>
	/// <summary> saves model to output stream `fout`</summary>
	/// <param name="_fout" value="_fout">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	/// <field value = "_vec"> weights of the SVM linear model as a full vector `vec`</field>
	weights: _vec,
}

var _utilities = {
	isArray: function () {
	/// <signature>
	/// <summary> is parameter an array?</summary>
	/// <param name="_arg" value="_arg">param</param>
	/// <returns value ="_bool"/>
	/// </signature>

	},

	isString: function () {
	/// <signature>
	/// <summary> is `s` a string?</summary>
	/// <param name="_s" value="_s">param</param>
	/// <returns value ="_bool"/>
	/// </signature>

	},

	arraysIdentical: function () {
	/// <signature>
	/// <summary> `bool` is true if array `arr` is identical to array `arr2`</summary>
	/// <param name="_arr" value="_arr">param</param>
	/// <param name="_arr2" value="_arr2">param</param>
	/// <returns value ="_bool"/>
	/// </signature>

	},

	isInArray: function () {
	/// <signature>
	/// <summary> is element in an array?</summary>
	/// <param name="_array" value="_array">param</param>
	/// <param name="_value" value="_value">param</param>
	/// <returns value ="_bool"/>
	/// </signature>

	},

	ifNull: function () {
	/// <signature>
	/// <summary> checks if `val` is null and returns default value `defVal`</summary>
	/// <param name="_val" value="_val">param</param>
	/// <param name="_defVal" value="_defVal">param</param>
	/// <returns value ="_returnVal"/>
	/// </signature>

	},

	newStopWatch: function () {
	/// <signature>
	/// <summary> creates a stop watch object `sw`</summary>
	/// <returns value ="_sw"/>
	/// </signature>

	},

	newHashTable: function () {
	/// <signature>
	/// <summary> creates a hash table</summary>
	/// <returns value ="_hashTable"/>
	/// </signature>

	},

	isNumber: function () {
	/// <signature>
	/// <summary> is `n` a number?</summary>
	/// <param name="_n" value="_n">param</param>
	/// <returns value ="_bool"/>
	/// </signature>

	},

	isObject: function () {
	/// <signature>
	/// <summary> is parameter an object?</summary>
	/// <param name="_arg" value="_arg">param</param>
	/// <returns value ="_bool"/>
	/// </signature>

	},

}

var _fout = {
	write: function () {
	/// <signature>
	/// <summary> writes to output stream. `data` can be a number, a json object or a string.</summary>
	/// <param name="_data" value="_data">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	close: function () {
	/// <signature>
	/// <summary> closes output stream</summary>
	/// <returns value =""/>
	/// </signature>

	},

	writeLine: function () {
	/// <signature>
	/// <summary> writes data to output stream and adds newline</summary>
	/// <param name="_data" value="_data">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	flush: function () {
	/// <signature>
	/// <summary> flushes output stream</summary>
	/// <returns value =""/>
	/// </signature>

	},

}

var _rs = {
	/// <field value = "_bool"> `bool = true` when records in the set are assigned weights</field>
	weighted: _bool,
	shuffle: function () {
	/// <signature>
	/// <summary> shuffle order using random integer seed `seed`. Inplace operation.</summary>
	/// <param name="_seed" value="_seed">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	sample: function () {
	/// <signature>
	/// <summary> create new record set by randomly sampling `num` records.</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_rs2"/>
	/// </signature>

	},

	sortByField: function () {
	/// <signature>
	/// <summary> sort records according to value of field `fieldName`; if `asc > 0` sorted in ascending order. Inplace operation.</summary>
	/// <param name="_fieldName" value="_fieldName">param</param>
	/// <param name="_asc" value="_asc">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	toJSON: function () {
	/// <signature>
	/// <summary> provide json version of record set, useful when calling JSON.stringify</summary>
	/// <returns value ="_objsJSON"/>
	/// </signature>

	},

	setdiff: function () {
	/// <signature>
	/// <summary> returns the set difference (record set) `rs3`=`rs``rs2`  between two record sets `rs` and `rs1`, which should point to the same store.</summary>
	/// <param name="_rs2" value="_rs2">param</param>
	/// <returns value ="_rs3"/>
	/// </signature>

	},

	deleteRecs: function () {
	/// <signature>
	/// <summary> delete from `rs` records that are also in `rs2`. Inplace operation.</summary>
	/// <param name="_rs2" value="_rs2">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	setunion: function () {
	/// <signature>
	/// <summary> returns the union (record set) `rs3` between two record sets `rs` and `rs2`, which should point to the same store.</summary>
	/// <param name="_rs2" value="_rs2">param</param>
	/// <returns value ="_rs3"/>
	/// </signature>

	},

	filterById: function () {
	/// <signature>
	/// <summary> keeps only records with ids between `minId` and `maxId`. Inplace operation.</summary>
	/// <param name="_minId" value="_minId">param</param>
	/// <param name="_maxId" value="_maxId">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	filterByField: function () {
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

	},

	setintersect: function () {
	/// <signature>
	/// <summary> returns the intersection (record set) `rs3` between two record sets `rs` and `rs2`, which should point to the same store.</summary>
	/// <param name="_rs2" value="_rs2">param</param>
	/// <returns value ="_rs3"/>
	/// </signature>

	},

	sortById: function () {
	/// <signature>
	/// <summary> sort records according to record id; if `asc > 0` sorted in ascending order. Inplace operation.</summary>
	/// <param name="_asc" value="_asc">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	/// <field value = "_storeName"> store of the records</field>
	store: _storeName,
	sort: function () {
	/// <signature>
	/// <summary> sort records according to `comparator` callback. Example: rs.sort(function(rec,rec2) {return rec.Val < rec2.Val;} ) sorts rs in ascending order (field Val is assumed to be a num). Inplace operation.</summary>
	/// <param name="_comparatorCallback" value="_comparatorCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	map: function () {
	/// <signature>
	/// <summary> iterates through the record set and executes the callback function `mapCallback` on each element:</summary>
	/// <param name="_mapCallback" value="_mapCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	sortByFq: function () {
	/// <signature>
	/// <summary> sort records according to weight; if `asc > 0` sorted in ascending order. Inplace operation.</summary>
	/// <param name="_asc" value="_asc">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	clone: function () {
	/// <signature>
	/// <summary> creates new instance of record set</summary>
	/// <returns value ="_rs2"/>
	/// </signature>

	},

	filterByFq: function () {
	/// <signature>
	/// <summary> keeps only records with weight between `minFq` and `maxFq`. Inplace operation.</summary>
	/// <param name="_minFq" value="_minFq">param</param>
	/// <param name="_maxFq" value="_maxFq">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	trunc: function () {
	/// <signature>
	/// <summary> truncate to first `num` record. Inplace operation.</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	join: function () {
	/// <signature>
	/// <summary> executes a join `joinName` on the records in the set, result is another record set `rs2`.</summary>
	/// <param name="_joinName" value="_joinName">param</param>
	/// <returns value ="_rs2"/>
	/// </signature>

	/// <signature>
	/// <summary> executes a join `joinName` on a sample of `sampleSize` records in the set, result is another record set `rs2`.</summary>
	/// <param name="_joinName" value="_joinName">param</param>
	/// <param name="_sampleSize" value="_sampleSize">param</param>
	/// <returns value ="_rs2"/>
	/// </signature>

	},

	reverse: function () {
	/// <signature>
	/// <summary> reverse record order. Inplace operation.</summary>
	/// <returns value =""/>
	/// </signature>

	},

	filter: function () {
	/// <signature>
	/// <summary> keeps only records that pass `filterCallback` function</summary>
	/// <param name="_filterCallback" value="_filterCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	/// <field value = "_len"> number of records in the set</field>
	length: _len,
	/// <field value = "_bool"> `bool = true` when record set is empty</field>
	empty: _bool,
	aggr: function () {
	/// <signature>
	/// <summary> returns an object where keys are aggregate names and values are JSON serialized aggregate values of all the aggregates contained in the records set</summary>
	/// <returns value ="_aggrsJSON"/>
	/// </signature>

	/// <signature>
	/// <summary> computes the aggregates based on the `aggrQueryJSON` parameter JSON object. If only one aggregate is involved and an array of JSON objects when more than one are returned.</summary>
	/// <param name="_aggrQueryJSON" value="_aggrQueryJSON">param</param>
	/// <returns value ="_aggr"/>
	/// </signature>

	},

}

var _spVec = {
	normalize: function () {
	/// <signature>
	/// <summary> normalizes the vector spVec (inplace operation)</summary>
	/// <returns value =""/>
	/// </signature>

	},

	/// <field value = "_num"> gets the dimension `num` (-1 means that it is unknown)</field>
	dim: _num,
	full: function () {
	/// <signature>
	/// <summary>  returns `vec` - a dense vector representation of sparse vector `spVec`.</summary>
	/// <returns value ="_vec"/>
	/// </signature>

	},

	sum: function () {
	/// <signature>
	/// <summary> `num` is the sum of elements of `spVec`</summary>
	/// <returns value ="_num"/>
	/// </signature>

	},

	put: function () {
	/// <signature>
	/// <summary> Set the element of a sparse vector `spVec`. Inputs: index `idx` (integer), value `num` (number). Uses 0-based indexing</summary>
	/// <param name="_idx" value="_idx">param</param>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	idxVec: function () {
	/// <signature>
	/// <summary>  returns `idxVec` - a dense (int) vector of indices (0-based) of nonzero elements of `spVec`.</summary>
	/// <returns value ="_idxVec"/>
	/// </signature>

	},

	print: function () {
	/// <signature>
	/// <summary> prints the vector to console</summary>
	/// <returns value =""/>
	/// </signature>

	},

	/// <field value = "_num"> gets the number of nonzero elements `num` of vector `spVec`</field>
	nnz: _num,
	inner: function () {
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

	},

	multiply: function () {
	/// <signature>
	/// <summary> `spVec2` is sparse vector, a product between `num` (number) and vector `spVec`</summary>
	/// <param name="_a" value="_a">param</param>
	/// <returns value ="_spVec2"/>
	/// </signature>

	},

	valVec: function () {
	/// <signature>
	/// <summary>  returns `valVec` - a dense (double) vector of values of nonzero elements of `spVec`.</summary>
	/// <returns value ="_valVec"/>
	/// </signature>

	},

	norm: function () {
	/// <signature>
	/// <summary> returns `num` - the norm of `spVec`</summary>
	/// <returns value ="_num"/>
	/// </signature>

	},

	at: function () {
	/// <signature>
	/// <summary> Gets the element of a sparse vector `spVec`. Input: index `idx` (integer). Output: value `num` (number). Uses 0-based indexing</summary>
	/// <param name="_idx" value="_idx">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	},

}

var _lloydModel = {
	getCentroidIdx: function () {
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

	},

	setC: function () {
	/// <signature>
	/// <summary> sets the centroid matrix to matrix `mat`</summary>
	/// <param name="_mat" value="_mat">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	getC: function () {
	/// <signature>
	/// <summary> returns the centroid matrix `mat`</summary>
	/// <returns value ="_mat"/>
	/// </signature>

	},

	update: function () {
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

	},

	init: function () {
	/// <signature>
	/// <summary> initializes the model with random centroids</summary>
	/// <returns value =""/>
	/// </signature>

	},

	getCentroid: function () {
	/// <signature>
	/// <summary> returns the centroid `vec2` (dense vector) that is the closest to vector `vec`</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_vec2"/>
	/// </signature>

	/// <signature>
	/// <summary> returns the centroid `vec2` (dense vector) that is the closest to sparse vector `spVec`</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <returns value ="_vec2"/>
	/// </signature>

	},

}

var http = {
	onDelete: function () {
	/// <signature>
	/// <summary> path: function path without server name and script name. Example: `http.onDelete("test", function (req, resp) { })` executed from `script.js` on localhost will execute a delete request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }</summary>
	/// <param name="_path" value="_path">param</param>
	/// <param name="_httpRequestCallback" value="_httpRequestCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	onRequest: function () {
	/// <signature>
	/// <summary> path: function path without server name and script name. Example: `http.onRequest("test", "GET", function (req, resp) { })` executed from `script.js` on localhost will execute a get request from `http://localhost/script/test`. `verb` can be one of the following {"GET","POST","PUT","DELETE","PATCH"}. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }</summary>
	/// <param name="_path" value="_path">param</param>
	/// <param name="_verb" value="_verb">param</param>
	/// <param name="_httpRequestCallback" value="_httpRequestCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	get: function () {
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

	},

	onPut: function () {
	/// <signature>
	/// <summary> path: function path without server name and script name. Example: `http.onPut("test", function (req, resp) { })` executed from `script.js` on localhost will execute a put request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }</summary>
	/// <param name="_path" value="_path">param</param>
	/// <param name="_httpRequestCallback" value="_httpRequestCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	getStr: function () {
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

	},

	postStr: function () {
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

	},

	jsonp: function () {
	/// <signature>
	/// <summary> packaging reply as jsonp when callback parameter is provided in URL</summary>
	/// <param name="_httpRequest" value="_httpRequest">param</param>
	/// <param name="_httpResponse" value="_httpResponse">param</param>
	/// <param name="_dataJSON" value="_dataJSON">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	onPatch: function () {
	/// <signature>
	/// <summary> path: function path without server name and script name. Example: `http.onPatch("test", function (req, resp) { })` executed from `script.js` on localhost will execute a patch request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }</summary>
	/// <param name="_path" value="_path">param</param>
	/// <param name="_httpRequestCallback" value="_httpRequestCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	post: function () {
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

	},

	onPost: function () {
	/// <signature>
	/// <summary> path: function path without server name and script name. Example: `http.onPost("test", function (req, resp) { })` executed from `script.js` on localhost will execute a post request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }</summary>
	/// <param name="_path" value="_path">param</param>
	/// <param name="_httpRequestCallback" value="_httpRequestCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	onGet: function () {
	/// <signature>
	/// <summary> path: function path without server name and script name. Example: `http.onGet("test", function (req, resp) { })` executed from `script.js` on localhost will execute a get request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }</summary>
	/// <param name="_path" value="_path">param</param>
	/// <param name="_httpRequestCallback" value="_httpRequestCallback">param</param>
	/// <returns value =""/>
	/// </signature>

	},

}

var _recLinRegModel = {
	predict: function () {
	/// <signature>
	/// <summary> sends vector `vec` through the</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	},

	/// <field value = "_num"> dimensionality of the feature space on which this model works</field>
	dim: _num,
	save: function () {
	/// <signature>
	/// <summary> saves model to output stream `fout`</summary>
	/// <param name="_fout" value="_fout">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	/// <field value = "_vec"> weights of the linear model as a full vector `vec`</field>
	weights: _vec,
	learn: function () {
	/// <signature>
	/// <summary> updates the model using full vector `vec` and target number `num`as training data</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	},

}

var _twitterParser = {
	rawJsonToStoreJson: function () {
	/// <signature>
	/// <summary> transforms a raw JSON object (result of twitter crawler) `rawTweetJSON` to `twitter.getTwitterStore()` compatible json object `objJSON`</summary>
	/// <param name="_rawTweetJSON" value="_rawTweetJSON">param</param>
	/// <returns value ="_objJSON"/>
	/// </signature>

	},

}

var _tm = {
	/// <field value = "_str"> day of week (string)</field>
	dayOfWeek: _str,
	sub: function () {
	/// <signature>
	/// <summary> subtracts `val` from the time; `unit` defintes the unit of `val`. options are `second` (default), `minute`, `hour`, and `day`.</summary>
	/// <param name="_val" value="_val">param</param>
	/// <param name="_unit" value="_unit">param</param>
	/// <returns value ="_tm2"/>
	/// </signature>

	},

	toJSON: function () {
	/// <signature>
	/// <summary> returns json representation of time</summary>
	/// <returns value ="_tmJSON"/>
	/// </signature>

	},

	/// <field value = "_str"> string representation of time (e.g. 2014-05-29T10:09:12)</field>
	string: _str,
	/// <field value = "_num"> hour (number)</field>
	hour: _num,
	/// <field value = "_num"> unix timestamp representation of time (seconds since 1970)</field>
	timestamp: _num,
	/// <field value = "_tm2"> returns new time object represented current UTC time</field>
	nowUTC: _tm2,
	/// <field value = "_str"> string representation of date (e.g. 2014-05-29)</field>
	dateString: _str,
	/// <field value = "_num"> month (number)</field>
	month: _num,
	parse: function () {
	/// <signature>
	/// <summary> parses string `str` in weblog format (example: `2014-05-29T10:09:12`)  and returns a date time object. Weblog format uses `T` to separate date and time, uses `-` for date units separation and `:` for time units separation (`YYYY-MM-DDThh-mm-ss`).</summary>
	/// <param name="_str" value="_str">param</param>
	/// <returns value ="_tm2"/>
	/// </signature>

	},

	add: function () {
	/// <signature>
	/// <summary> adds `val` to the time; `unit` defines the unit</summary>
	/// <param name="_val" value="_val">param</param>
	/// <param name="_unit" value="_unit">param</param>
	/// <returns value ="_tm2"/>
	/// </signature>

	},

	/// <field value = "_num"> year (number)</field>
	year: _num,
	/// <field value = "_num"> millisecond (number)</field>
	milisecond: _num,
	/// <field value = "_tm2"> returns new time object representing current local time</field>
	now: _tm2,
	/// <field value = "_num"> day (number)</field>
	day: _num,
	/// <field value = "_num"> minute (number)</field>
	minute: _num,
	/// <field value = "_num"> second (number)</field>
	second: _num,
}

var _htModel = {
	process: function () {
	/// <signature>
	/// <summary> processes the stream example; `strArr` is an array of discrete attribute values (strings);</summary>
	/// <param name="_strArr" value="_strArr">param</param>
	/// <param name="_numArr" value="_numArr">param</param>
	/// <param name="_labelStr" value="_labelStr">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> processes the stream example; `line` is comma-separated string of attribute values (for example "a1,a2,c", where c is the class label); returns nothing;</summary>
	/// <param name="_line" value="_line">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	exportModel: function () {
	/// <signature>
	/// <summary> writes the current model into file `htOutParams.file` in format `htOutParams.type`;</summary>
	/// <param name="_htOutParams" value="_htOutParams">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	classify: function () {
	/// <signature>
	/// <summary> classifies the stream example; `strArr` is an array of discrete attribute values (strings); `numArr` is an array of numeric attribute values (numbers); returns the class label</summary>
	/// <param name="_strArr" value="_strArr">param</param>
	/// <param name="_numArr" value="_numArr">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> classifies the stream example; `line` is comma-separated string of attribute values; returns the class label</summary>
	/// <param name="_line" value="_line">param</param>
	/// <returns value =""/>
	/// </signature>

	},

}

var _alModel = {
	selectQuestion: function () {
	/// <signature>
	/// <summary> returns `recSetIdx` - the index of the record in `recSet`, whose class is unknonw and requires user input</summary>
	/// <returns value ="_recSetIdx"/>
	/// </signature>

	},

	getAnswer: function () {
	/// <signature>
	/// <summary> given user input `ALAnswer` (string) and `recSetIdx` (integer, result of model.selectQuestion) the training set is updated.</summary>
	/// <param name="_alAnswer" value="_alAnswer">param</param>
	/// <param name="_recSetIdx" value="_recSetIdx">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	getPos: function () {
	/// <signature>
	/// <summary> given a `threshold` (number) return the indexes of records classified above it as a javascript array of numbers. Must be in SVM mode.</summary>
	/// <param name="_thresh" value="_thresh">param</param>
	/// <returns value ="_numArr"/>
	/// </signature>

	},

	setc: function () {
	/// <signature>
	/// <summary> sets the SVM c parameter to the provided value.</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	setj: function () {
	/// <signature>
	/// <summary> sets the SVM j parameter to the provided value.</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	startLoop: function () {
	/// <signature>
	/// <summary> starts the active learning loop in console</summary>
	/// <returns value =""/>
	/// </signature>

	},

	getnpos: function () {
	/// <signature>
	/// <summary> return the  number of examples marked as positive.</summary>
	/// <returns value ="_num"/>
	/// </signature>

	},

	saveSvmModel: function () {
	/// <signature>
	/// <summary> saves the binary SVM model to an output stream `fout`. The algorithm must be in SVM mode.</summary>
	/// <param name="_fout" value="_fout">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	getQueryMode: function () {
	/// <signature>
	/// <summary> returns true if in query mode, false otherwise (SVM mode)</summary>
	/// <returns value ="_bool"/>
	/// </signature>

	},

	getnneg: function () {
	/// <signature>
	/// <summary> return the  number of examples marked as negative.</summary>
	/// <returns value ="_num"/>
	/// </signature>

	},

}

var _model = {
	predict: function () {
	/// <signature>
	/// <summary> predicts the target `num` (number), given feature vector `vec` based on the internal model parameters.</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	},

}

var _mat = {
	/// <field value = "_num"> integer `num` corresponds to the number of columns of `mat`</field>
	cols: _num,
	put: function () {
	/// <signature>
	/// <summary> Sets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing.</summary>
	/// <param name="_rowIdx" value="_rowIdx">param</param>
	/// <param name="_colIdx" value="_colIdx">param</param>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	normalizeCols: function () {
	/// <signature>
	/// <summary> normalizes each column of matrix `mat` (inplace operation)</summary>
	/// <returns value =""/>
	/// </signature>

	},

	/// <field value = "_num"> integer `num` corresponds to the number of rows of `mat`</field>
	rows: _num,
	diag: function () {
	/// <signature>
	/// <summary> Returns the diagonal of matrix `mat` as `vec` (dense vector).</summary>
	/// <returns value ="_vec"/>
	/// </signature>

	},

	frob: function () {
	/// <signature>
	/// <summary> number `num` is the Frobenious norm of matrix `mat`</summary>
	/// <returns value ="_num"/>
	/// </signature>

	},

	multiplyT: function () {
	/// <signature>
	/// <summary> Matrix transposed multiplication: `num` is a number, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_mat2"/>
	/// </signature>

	/// <signature>
	/// <summary> Matrix transposed multiplication: `vec` is a vector, `vec2` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_vec2"/>
	/// </signature>

	/// <signature>
	/// <summary> Matrix transposed multiplication: `spVec` is a sparse vector, `vec` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	/// <signature>
	/// <summary> Matrix transposed multiplication: `mat2` is a matrix, `mat3` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
	/// <param name="_mat2" value="_mat2">param</param>
	/// <returns value ="_mat3"/>
	/// </signature>

	/// <signature>
	/// <summary> Matrix transposed multiplication: `spMat` is a sparse matrix, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient</summary>
	/// <param name="_spMat" value="_spMat">param</param>
	/// <returns value ="_mat2"/>
	/// </signature>

	},

	getRow: function () {
	/// <signature>
	/// <summary> `vec` corresponds to the `rowIdx`-th row of dense matrix `mat`. `rowIdx` must be an integer.</summary>
	/// <param name="_rowIdx" value="_rowIdx">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	},

	printStr: function () {
	/// <signature>
	/// <summary> print matrix `mat` to a string `str`</summary>
	/// <returns value ="_str"/>
	/// </signature>

	},

	multiply: function () {
	/// <signature>
	/// <summary> Matrix multiplication: `num` is a number, `mat2` is a matrix</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_mat2"/>
	/// </signature>

	/// <signature>
	/// <summary> Matrix multiplication: `vec` is a vector, `vec2` is a vector</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_vec2"/>
	/// </signature>

	/// <signature>
	/// <summary> Matrix multiplication: `spVec` is a sparse vector, `vec` is a vector</summary>
	/// <param name="_spVec" value="_spVec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	/// <signature>
	/// <summary> Matrix multiplication: `mat2` is a matrix, `mat3` is a matrix</summary>
	/// <param name="_mat2" value="_mat2">param</param>
	/// <returns value ="_mat3"/>
	/// </signature>

	/// <signature>
	/// <summary> Matrix multiplication: `spMat` is a sparse matrix, `mat2` is a matrix</summary>
	/// <param name="_spMat" value="_spMat">param</param>
	/// <returns value ="_mat2"/>
	/// </signature>

	},

	print: function () {
	/// <signature>
	/// <summary> print matrix `mat` to console</summary>
	/// <returns value =""/>
	/// </signature>

	},

	plus: function () {
	/// <signature>
	/// <summary> `mat3` is the sum of matrices `mat` and `mat2`</summary>
	/// <param name="_mat2" value="_mat2">param</param>
	/// <returns value ="_mat3"/>
	/// </signature>

	},

	colNorms: function () {
	/// <signature>
	/// <summary> `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `mat`</summary>
	/// <returns value ="_vec"/>
	/// </signature>

	},

	transpose: function () {
	/// <signature>
	/// <summary> matrix `mat2` is matrix `mat` transposed</summary>
	/// <returns value ="_mat2"/>
	/// </signature>

	},

	setCol: function () {
	/// <signature>
	/// <summary> Sets the column of a dense matrix `mat`. `colIdx` must be an integer, `vec` must be a dense vector.</summary>
	/// <param name="_colIdx" value="_colIdx">param</param>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	colMaxIdx: function () {
	/// <signature>
	/// <summary>: get the index `rowIdx` of the maximum element in column `colIdx` of dense matrix `mat`</summary>
	/// <param name="_colIdx" value="_colIdx">param</param>
	/// <returns value ="_rowIdx"/>
	/// </signature>

	},

	rowNorms: function () {
	/// <signature>
	/// <summary> `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th row of `mat`</summary>
	/// <returns value ="_vec"/>
	/// </signature>

	},

	getCol: function () {
	/// <signature>
	/// <summary> `vec` corresponds to the `colIdx`-th column of dense matrix `mat`. `colIdx` must be an integer.</summary>
	/// <param name="_colIdx" value="_colIdx">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	},

	rowMaxIdx: function () {
	/// <signature>
	/// <summary>: get the index `colIdx` of the maximum element in row `rowIdx` of dense matrix `mat`</summary>
	/// <param name="_rowIdx" value="_rowIdx">param</param>
	/// <returns value ="_colIdx"/>
	/// </signature>

	},

	solve: function () {
	/// <signature>
	/// <summary> vector `vec2` is the solution to the linear system `mat * vec2 = vec`</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_vec2"/>
	/// </signature>

	},

	sparse: function () {
	/// <signature>
	/// <summary> get sparse column matrix representation `spMat` of dense matrix `mat`</summary>
	/// <returns value ="_spMat"/>
	/// </signature>

	},

	setRow: function () {
	/// <signature>
	/// <summary> Sets the row of a dense matrix `mat`. `rowIdx` must be an integer, `vec` must be a dense vector.</summary>
	/// <param name="_rowIdx" value="_rowIdx">param</param>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	minus: function () {
	/// <signature>
	/// <summary> `mat3` is the difference of matrices `mat` and `mat2`</summary>
	/// <param name="_mat2" value="_mat2">param</param>
	/// <returns value ="_mat3"/>
	/// </signature>

	},

	at: function () {
	/// <signature>
	/// <summary> Gets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.</summary>
	/// <param name="_rowIdx" value="_rowIdx">param</param>
	/// <param name="_colIdx" value="_colIdx">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	},

}

var _key = {
	/// <field value = "_strArr"> gets the array of weights (as strings) in the vocabulary</field>
	fq: _strArr,
	/// <field value = "_keyName"> gets the key name</field>
	name: _keyName,
	/// <field value = "_storeName"> gets the store name `storeName`</field>
	store: _storeName,
	/// <field value = "_strArr"> gets the array of words (as strings) in the vocabulary</field>
	voc: _strArr,
}

var qm = {
	search: function () {
	/// <signature>
	/// <summary> execute `query` (Json) specified in [QMiner Query Language](Query Language)</summary>
	/// <param name="_query" value="_query">param</param>
	/// <returns value ="_rs"/>
	/// </signature>

	},

	gc: function () {
	/// <signature>
	/// <summary> start garbage collection to remove records outside time windows</summary>
	/// <returns value =""/>
	/// </signature>

	},

	printStreamAggr: function () {
	/// <signature>
	/// <summary> prints all current field values of every stream aggregate attached to the store `store`</summary>
	/// <param name="_store" value="_store">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	createStore: function () {
	/// <signature>
	/// <summary> create new store(s) based on given `storeDef` (Json) [definition](Store Definition)</summary>
	/// <param name="_storeDef" value="_storeDef">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	store: function () {
	/// <signature>
	/// <summary> store with name `storeName`; `store = null` when no such store</summary>
	/// <param name="_storeName" value="_storeName">param</param>
	/// <returns value ="_store"/>
	/// </signature>

	},

	getStoreList: function () {
	/// <signature>
	/// <summary> an array of strings listing all existing stores</summary>
	/// <returns value ="_strArr"/>
	/// </signature>

	},

}

var fs = {
	rename: function () {
	/// <signature>
	/// <summary> rename file</summary>
	/// <param name="_fromFileName" value="_fromFileName">param</param>
	/// <param name="_toFileName" value="_toFileName">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	openWrite: function () {
	/// <signature>
	/// <summary> open file in write mode and return file output stream `fout`</summary>
	/// <param name="_fileName" value="_fileName">param</param>
	/// <returns value ="_fout"/>
	/// </signature>

	},

	exists: function () {
	/// <signature>
	/// <summary> does file exist?</summary>
	/// <param name="_fileName" value="_fileName">param</param>
	/// <returns value ="_bool"/>
	/// </signature>

	},

	openRead: function () {
	/// <signature>
	/// <summary> open file in read mode and return file input stream `fin`</summary>
	/// <param name="_fileName" value="_fileName">param</param>
	/// <returns value ="_fin"/>
	/// </signature>

	},

	openAppend: function () {
	/// <signature>
	/// <summary> open file in append mode and return file output stream `fout`</summary>
	/// <param name="_fileName" value="_fileName">param</param>
	/// <returns value ="_fout"/>
	/// </signature>

	},

	move: function () {
	/// <signature>
	/// <summary> move file</summary>
	/// <param name="_fromFileName" value="_fromFileName">param</param>
	/// <param name="_toFileName" value="_toFileName">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	mkdir: function () {
	/// <signature>
	/// <summary> make folder</summary>
	/// <param name="_dirName" value="_dirName">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	listFile: function () {
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

	},

	del: function () {
	/// <signature>
	/// <summary> delete file</summary>
	/// <param name="_fileName" value="_fileName">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	rmdir: function () {
	/// <signature>
	/// <summary> delete folder</summary>
	/// <param name="_dirName" value="_dirName">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	fileInfo: function () {
	/// <signature>
	/// <summary> returns file info as a json object {createTime:str, lastAccessTime:str, lastWriteTime:str, size:num}.</summary>
	/// <param name="_fileName" value="_fileName">param</param>
	/// <returns value ="_fileInfoJson"/>
	/// </signature>

	},

	copy: function () {
	/// <signature>
	/// <summary> copy file</summary>
	/// <param name="_fromFileName" value="_fromFileName">param</param>
	/// <param name="_toFileName" value="_toFileName">param</param>
	/// <returns value =""/>
	/// </signature>

	},

}

var _perceptronModel = {
	predict: function () {
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

	},

	getModel: function () {
	/// <signature>
	/// <summary> returns an object `perceptronParam` where `perceptronParam.w` (vector) and `perceptronParam.b` (bias) are the separating hyperplane normal and bias.</summary>
	/// <returns value ="_perceptronParam"/>
	/// </signature>

	},

	update: function () {
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

	},

}

var _fsp = {
	/// <field value = "_num"> dimensionality of feature space</field>
	dim: _num,
	ftrSpColMat: function () {
	/// <signature>
	/// <summary> extracts sparse feature vectors from</summary>
	/// <param name="_rs" value="_rs">param</param>
	/// <returns value ="_spMat"/>
	/// </signature>

	},

	extractStrings: function () {
	/// <signature>
	/// <summary> use feature extractors to extract string</summary>
	/// <param name="_rec" value="_rec">param</param>
	/// <returns value ="_strArr"/>
	/// </signature>

	},

	getFtr: function () {
	/// <signature>
	/// <summary> returns the name `ftrName` (string) of `idx`-th feature in feature space `fsp`</summary>
	/// <param name="_idx" value="_idx">param</param>
	/// <returns value ="_ftrName"/>
	/// </signature>

	},

	updateRecord: function () {
	/// <signature>
	/// <summary> update feature space definitions and extractors</summary>
	/// <param name="_rec" value="_rec">param</param>
	/// <returns value =""/>
	/// </signature>

	/// <signature>
	/// <summary> update feature space definitions and extractors</summary>
	/// <param name="_rs" value="_rs">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	ftrSpVec: function () {
	/// <signature>
	/// <summary> extracts sparse feature vector `spVec` from record `rec`</summary>
	/// <param name="_rec" value="_rec">param</param>
	/// <returns value ="_spVec"/>
	/// </signature>

	},

	ftrVec: function () {
	/// <signature>
	/// <summary> extracts feature vector `vec` from record  `rec`</summary>
	/// <param name="_rec" value="_rec">param</param>
	/// <returns value ="_vec"/>
	/// </signature>

	},

	ftrColMat: function () {
	/// <signature>
	/// <summary> extracts feature vectors from</summary>
	/// <param name="_rs" value="_rs">param</param>
	/// <returns value ="_mat"/>
	/// </signature>

	},

	save: function () {
	/// <signature>
	/// <summary> serialize feature space to `fout` output stream</summary>
	/// <param name="_fout" value="_fout">param</param>
	/// <returns value =""/>
	/// </signature>

	},

}

var _nnModel = {
	predict: function () {
	/// <signature>
	/// <summary> sends vector `vec` through the model and returns the prediction as a vector `vec2`</summary>
	/// <param name="_vec" value="_vec">param</param>
	/// <returns value ="_vec2"/>
	/// </signature>

	},

	learn: function () {
	/// <signature>
	/// <summary> uses a pair of input `inVec` and output `outVec` to perform one step of learning with backpropagation.</summary>
	/// <param name="_inVec" value="_inVec">param</param>
	/// <param name="_outVec" value="_outVec">param</param>
	/// <returns value =""/>
	/// </signature>

	},

}

var _intVec = {
	sort: function () {
	/// <signature>
	/// <summary> integer vector `intVec2` is a sorted copy of integer vector `intVec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order</summary>
	/// <param name="_asc" value="_asc">param</param>
	/// <returns value ="_intVec2"/>
	/// </signature>

	},

	toMat: function () {
	/// <signature>
	/// <summary> `mat` is a matrix with a single column that is equal to dense integer vector `intVec`.</summary>
	/// <returns value ="_mat"/>
	/// </signature>

	},

	getMaxIdx: function () {
	/// <signature>
	/// <summary> returns the integer index `idx` of the maximal element in integer vector `vec`</summary>
	/// <returns value ="_idx"/>
	/// </signature>

	},

	pushV: function () {
	/// <signature>
	/// <summary> append integer vector `intVec2` to integer vector `intVec`.</summary>
	/// <param name="_intVec2" value="_intVec2">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	unshift: function () {
	/// <signature>
	/// <summary> insert value `num` to the begining of integer vector `intVec`. Returns the length of the modified array.</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_len"/>
	/// </signature>

	},

	sum: function () {
	/// <signature>
	/// <summary> return `num`: the sum of elements of integer vector `intVec`</summary>
	/// <returns value ="_num"/>
	/// </signature>

	},

	put: function () {
	/// <signature>
	/// <summary> set value of integer vector `intVec` at index `idx` to `num` (0-based indexing)</summary>
	/// <param name="_idx" value="_idx">param</param>
	/// <param name="_num" value="_num">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	push: function () {
	/// <signature>
	/// <summary> append value `num` to integer vector `intVec`. Returns `len` - the length  of the modified array</summary>
	/// <param name="_num" value="_num">param</param>
	/// <returns value ="_len"/>
	/// </signature>

	},

	/// <field value = "_len"> integer `len` is the length of integer vector `vec`</field>
	length: _len,
	at: function () {
	/// <signature>
	/// <summary> gets the value `num` of integer vector `intVec` at index `idx`  (0-based indexing)</summary>
	/// <param name="_idx" value="_idx">param</param>
	/// <returns value ="_num"/>
	/// </signature>

	},

	print: function () {
	/// <signature>
	/// <summary> print integer vector in console</summary>
	/// <returns value =""/>
	/// </signature>

	},

	subVec: function () {
	/// <signature>
	/// <summary> gets the subvector based on an index vector `intVec` (indices can repeat, 0-based indexing)</summary>
	/// <param name="_intVec" value="_intVec">param</param>
	/// <returns value ="_intVec2"/>
	/// </signature>

	},

}

var _twitter = {
	getTwitterStoreJson: function () {
	/// <signature>
	/// <summary> returns a Twitter store definition JSON object `twitterDef`. The JSON array contains four store definitions: Tweets, Users, HashTags and Pages</summary>
	/// <returns value ="_twitterDef"/>
	/// </signature>

	},

	newParser: function () {
	/// <signature>
	/// <summary> creates an object that converts between raw Twitter JSON objects and qminer store compatible JSON objects. Exposes:</summary>
	/// <returns value ="_twitterParser"/>
	/// </signature>

	},

	RawToStore: function () {
	/// <signature>
	/// <summary> converts twitter JSON lines to `twitter.getTwitterStoreJson()` compatible JSON lines, given input stream `fin` (raw JSON lines) and output stream `fout` (store JSON lines file)</summary>
	/// <param name="_fin" value="_fin">param</param>
	/// <param name="_fout" value="_fout">param</param>
	/// <returns value =""/>
	/// </signature>

	},

}

var _sw = {
	reset: function () {
	/// <signature>
	/// <summary> resets</summary>
	/// <returns value =""/>
	/// </signature>

	},

	saytime: function () {
	/// <signature>
	/// <summary> displays elpased time from tic</summary>
	/// <param name="_message" value="_message">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	stop: function () {
	/// <signature>
	/// <summary> stops the stopwatch</summary>
	/// <returns value =""/>
	/// </signature>

	},

	start: function () {
	/// <signature>
	/// <summary> starts the stopwatch</summary>
	/// <returns value =""/>
	/// </signature>

	},

	time: function () {
	/// <signature>
	/// <summary> returns unix epoch time in milliseconds</summary>
	/// <returns value ="_num"/>
	/// </signature>

	},

	toc: function () {
	/// <signature>
	/// <summary> displays time from tic and message `str`</summary>
	/// <param name="_str" value="_str">param</param>
	/// <returns value =""/>
	/// </signature>

	},

	tic: function () {
	/// <signature>
	/// <summary> resets and starts the stop watch</summary>
	/// <returns value =""/>
	/// </signature>

	},

}


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

_vec[0] = 1; // vec is indexed and returns numbers
_intVec[0] = 1; // vec is indexed and returns numbers
_spMat[0] = _spVec; // spMat is indexed and returns sparse column vectors
_rs[0] = _rec; // record set at index returns a record

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

