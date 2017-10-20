/**
 * github.com/bucaran/sget
 *
 * sget. Async / Sync read line for Node.
 *
 * @copyright (c) 2014 Jorge Bucaran
 * @license MIT
 */
var fs = require('fs'),
    rl = require('readline');
/**
 * Read a line from stdin sync. If callback is undefined reads it async.
 *
 * @param {String} message Message to log before reading stdin.
 * @param {Function} callback If specified, reads the stdin async.
 */
var sget = module.exports = function(message, callback) {
  win32 = function() {
    return ('win32' === process.platform);
  },
  readSync = function(buffer) {
    var fd = win32() ? process.stdin.fd : fs.openSync('/dev/stdin', 'rs');
    var bytes = fs.readSync(fd, buffer, 0, buffer.length);
    if (!win32()) fs.closeSync(fd);
    return bytes;
  };
  message = message || '';
  if (callback) {
    var cli = rl.createInterface(process.stdin, process.stdout);
    console.log(message);
    cli.prompt();
    cli.on('line', function(data) {
      cli.close();
      callback(data);
    });
  } else {
    return (function(buffer) {
      try {
        console.log(message);
        return buffer.toString(null, 0, readSync(buffer));
      } catch (e) {
        throw e;
      }
    }(new Buffer(sget.bufferSize)));
  }
};
/**
 * @type {Number} Size of the buffer to read.
 */
sget.bufferSize = 256;
