exports.silent = false;

// prints success if it comes to this
exports.run = function(test, msg) {
    if (!this.silent) { console.say("Test", "PASS(ok): " + msg); }
}

// checks if condition is true
exports.ok = function(test, msg) {
    if (test) {
        if (!this.silent) { console.say("Test", "PASS(ok): " + msg); }
    } else {
        console.say("Test", "FAIL(ok): " + msg);
    }
}

// checks if object exists (!=null)
exports.exists = function(obj, msg) {
    if (obj != null) {
        if (!this.silent) { console.say("Test", "PASS(exists): " + msg); }
    } else {
        console.say("Test", "FAIL(exists): " + msg);
    }
}

// checks if condition is true
exports.equals = function(test, ok, msg) {
    if (test == ok) {
        if (!this.silent) { console.say("Test", "PASS(equals:" + test + "=" + ok + "): " + msg); }
    } else {
        console.say("Test", "FAIL(equals:" + test + "=" + ok + "): " + msg);
    }
}
