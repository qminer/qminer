var qminer = null;
 
// Load the precompiled binary for windows.
if(process.platform == "win32" && process.arch == "x64") {
	qminer = require('./bin/winx64/qm.node');
} else if(process.platform == "win32" && process.arch == "ia32") {
	throw 'unsupported architecture';
} else {
	// Load the new built binary for other platforms.
	qminer = require('./build/Debug/qm.node');
}
 
module.exports = qminer;
