"use strict";

var fs = require("fs");
var path = require("path");
var cp = require("child_process");

var is_win = process.platform.startsWith("win");
var exe_name = "qminer-test" + (is_win ? ".exe" : "");
var release_dir = __dirname + "/../../build/Release"
var debug_dir = __dirname + "/../../build/Debug"

var target = "";


if (fs.existsSync(release_dir)) {
    target = path.join(release_dir, exe_name);
} else {
    target = path.join(debug_dir, exe_name);
}

//cp.execSync(target);

var child = cp.spawn(target);

child.stdout.on('data', function (data) {
  console.log(data.toString());
});

child.stderr.on('data', function (data) {
  console.log(data.toString());
});

child.on('exit', function (code) {
  console.log('child process exited with code ' + code.toString());
  process.exit(code);
});