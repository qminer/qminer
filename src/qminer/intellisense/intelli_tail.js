
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

// json objects
_addIntellisenseVar("_obj", "{}");
_addIntellisenseVar("_objJSON", "{}");
_addIntellisenseVar("_paramJSON", "{}");
_addIntellisenseVar("_aggrsJSON", "{}");
_addIntellisenseVar("_aggrQueryJSON", "{}");

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


require = function (libName) {
    if (libName === 'analytics.js') return _analytics;
    if (libName === 'utilities.js') return _utilities;
    if (libName === 'assert.js') return _assert;
    if (libName === 'twitter.js') return _twitter;
};

intellisenseIgnore["_analytics"] = "{}";
intellisenseIgnore["_utilities"] = "{}";
intellisenseIgnore["_assert"] = "{}";
intellisenseIgnore["_twitter"] = "{}";


// implement ignore
intellisense.addEventListener('statementcompletion', function (event) {
    if (event.targetName === "this") return;
    event.items = event.items.filter(function (item) {
        return !intellisenseIgnore.hasOwnProperty(item.name);
    });
});

