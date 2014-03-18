exports.silent = false;
exports.consoleTitle = "Test";

// prints success if it comes to this
exports.run = function(test, msg) {
    if (!this.silent) { console.say(this.consoleTitle, "PASS(ok): " + msg); }
}

// checks if condition is true
exports.ok = function(test, msg) {
    if (test) {
        if (!this.silent) { console.say(this.consoleTitle, "PASS(ok): " + msg); }
    } else {
        console.say(this.consoleTitle, "FAIL(ok): " + msg);
    }
}

// checks if object exists (!=null)
exports.exists = function(obj, msg) {
    if (obj != null) {
        if (!this.silent) { console.say(this.consoleTitle, "PASS(exists): " + msg); }
    } else {
        console.say(this.consoleTitle, "FAIL(exists): " + msg);
    }
}

// checks if condition is true
exports.equals = function(test, ok, msg) {
    if (test == ok) {
        if (!this.silent) { console.say(this.consoleTitle, "PASS(equals:" + test + "=" + ok + "): " + msg); }
    } else {
        console.say(this.consoleTitle, "FAIL(equals:" + test + "=" + ok + "): " + msg);
    }
}
