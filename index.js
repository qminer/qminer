var qminer = null;
 
// Load the precompiled binary for windows.
if(process.platform == "win32" && process.arch == "x64") {
robot = require('./bin/winx64/robot');
} else if(process.platform == "win32" && process.arch == "ia32") {
robot = require('./bin/winx86/robot');
} else {
	// Load the new built binary for other platforms.
qminer = require('./build/Release/robot');
}
 
module.exports = qminer;
