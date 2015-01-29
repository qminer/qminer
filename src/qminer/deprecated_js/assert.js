// QMiner - Open Source Analytics Platform
// 
// Copyright (C) 2014 Jozef Stefan Institute
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License, version 3,
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

exports.silent = false;
exports.consoleTitle = "Test";

//#- `assert.run(test, msg)` -- prints success if this code is reached
exports.run = function(test, msg) {
    if (!this.silent) { console.say(this.consoleTitle, "PASS(ok): " + msg); }
}

// checks if condition is true
//#- `assert.ok(test, msg)` --  `test` is boolean, prints message `msg` if test is true
exports.ok = function(test, msg) {
    if (test) {
        if (!this.silent) { console.say(this.consoleTitle, "PASS(ok): " + msg); }
    } else {
        console.say(this.consoleTitle, "FAIL(ok): " + msg);
        process.returnCode = 11;
    }
}

// checks if object exists (!=null)
//#- `assert.exists(obj, msg)` --  checks if object `obj` exists (!=null) and prints message `msg`
exports.exists = function(obj, msg) {
    if (obj != null) {
        if (!this.silent) { console.say(this.consoleTitle, "PASS(exists): " + msg); }
    } else {
        console.say(this.consoleTitle, "FAIL(exists): " + msg);
        process.returnCode = 11;
    }
}

// checks if condition is true
//#- `assert.equals(obj, obj2,  msg)` --  checks if object `obj==obj2` and prints message `msg`
exports.equals = function(test, ok, msg) {
    if (test == ok) {
        if (!this.silent) { console.say(this.consoleTitle, "PASS(equals:" + test + "=" + ok + "): " + msg); }
    } else {
        console.say(this.consoleTitle, "FAIL(equals:" + test + "=" + ok + "): " + msg);
        process.returnCode = 11;
    }
}
