exports.addToProcess = function (proc) {
    proc.isArg = function (arg) {
        return process.argv.indexOf(arg) >= 0;
    }
};