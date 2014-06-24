// right of --  -> summary
// [] ignore for now

// primitive
_idx = 1; _num = 1; _len = 1;
// _bool = true;
// _str = "";
// _obj = {};
// _null = null;
/// ignore this: _undef = undefined;


// globals without _, locals with _
la = {}; // global linalg
_vec = {}; // vector
_intVec = {}; // int vector
// alternatives
_vec2 = _vec3 = _vec;
_intVec2 = _intVec;

// result structures
_sortRes = { vec: _vec, perm: _intVec };

// ignore suggest list! (all non global variables)
var intellisenseIgnore = {
    "_vec" : 1,
    "_vec2" : 1,
    "_vec3" : 1,
    "_intVec" : 1,
    "_intVec2" : 1,
    "_sortRes" : 1,
    "intellisenseIgnore" : 1
}

// implement ignore
intellisense.addEventListener('statementcompletion', function (event) {
    if (event.targetName === "this") return;
    event.items = event.items.filter(function (item) {
        return !intellisenseIgnore.hasOwnProperty(item.name);
    });
});



/// automatic !

// vec.print()
_vec.print = function () { }
// intVec.print()
_intVec.print = function () { }

// vec.put(idx, num)
_vec.put = function (_idx, _num) { }
// intVec.put(idx, num)
_intVec.put = function (_idx, _num) { }


// num = vec.at(idx)
_vec.at = function (_idx) { return _num; }
// num = intVec.at(idx)
_intVec.at = function (_idx) { return _num; }

// vec = la.newVec();
la.newVec = function () { return _vec; }
// intVec = la.newIntVec();
la.newIntVec = function () { return _intVec; }



