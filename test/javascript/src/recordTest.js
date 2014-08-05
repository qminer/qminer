var assert = require('assert.js');

console.say("Record", "Testing record serialization/deserilization/by value");

// only report failours
assert.silent = !process.isArg("-verbose");
// name of the debug process
assert.consoleTitle = "Record";

qm.createStore({
    "name": "RecordTest", 
    "fields": [ 
      { "name": "Int", "type": "int" },
      { "name": "IntV", "type": "int_v", "null": true },
      { "name": "UInt64", "type": "uint64", "null": true },
      { "name": "Str", "type": "string", "null": true },
      { "name": "StrV", "type": "string_v", "null": true },
      { "name": "Bool", "type": "bool", "null": true },
      { "name": "Flt", "type": "float", "null": true },
      { "name": "FltPr", "type": "float_pair", "null": true },
      { "name": "FltV", "type": "float_v", "null": true },
      { "name": "Tm", "type": "datetime", "null": true },
      { "name": "SpV", "type": "num_sp_v", "null": true }
    ], 
    "joins": [ ],
    "keys": [ ]
  });

/*
FieldTypeMap.AddDat("int") = oftInt;
FieldTypeMap.AddDat("int_v") = oftIntV;
FieldTypeMap.AddDat("uint64") = oftUInt64;
FieldTypeMap.AddDat("string") = oftStr;
FieldTypeMap.AddDat("string_v") = oftStrV;
FieldTypeMap.AddDat("bool") = oftBool;
FieldTypeMap.AddDat("float") = oftFlt;
FieldTypeMap.AddDat("float_pair") = oftFltPr;
FieldTypeMap.AddDat("float_v") = oftFltV;
FieldTypeMap.AddDat("datetime") = oftTm;
FieldTypeMap.AddDat("num_sp_v") = oftNumSpV;
FieldTypeMap.AddDat("bow_sp_v") = oftBowSpV;
 */

assert.exists(qm.store("RecordTest"), "Retrieve 'RecordTest' store");
var RecordTest = qm.store("RecordTest");

// adding record
var rec = { 
    Int: 123,
    UInt64: 123,
    Str: "test",
    StrV: ["test1", "test2"],
    Bool: false,
    Flt: 1.23
};
// check addition
assert.equals(RecordTest.add(rec), 0, "RecordTest.add");
// check getters
var recByRef = RecordTest[0];
assert.exists(recByRef, "RecordTest[0]");
assert.equals(recByRef.Int, 123, "recByRef.Int");
assert.equals(recByRef.UInt64, 123, "recByRef.UInt64");
assert.equals(recByRef.Str, "test", "recByRef.Str");
assert.equals(recByRef.Bool, false, "recByRef.Bool");
assert.equals(recByRef.Flt, 1.23, "recByRef.Flt");
// check setters
recByRef.Int = 124;
assert.equals(recByRef.Int, 124, "recByRef.Int");
recByRef.UInt64 = 124;
assert.equals(recByRef.UInt64, 124, "recByRef.UInt64");
recByRef.Str = "tset";
assert.equals(recByRef.Str, "tset", "recByRef.Str");
recByRef.Bool = true;
assert.equals(recByRef.Bool, true, "recByRef.Bool");
recByRef.Flt = 1.24;
assert.equals(recByRef.Flt, 1.24, "recByRef.Flt");

// check by value 
var recByVal = RecordTest.newRec(rec);
assert.exists(recByVal, "RecordTest.newRec(rec)");
assert.equals(recByVal.Int, 123, "recByVal.Int");
assert.equals(recByVal.UInt64, 123, "recByVal.UInt64");
assert.equals(recByVal.Str, "test", "recByVal.Str");
assert.exists(recByVal.StrV, "recByVal.StrV");
console.log("Record", JSON.stringify(recByVal.StrV));
assert.equals(recByVal.Bool, false, "recByVal.Bool");
assert.equals(recByVal.Flt, 1.23, "recByVal.Flt");

