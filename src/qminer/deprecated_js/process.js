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

//////////////////////////////////////////
// process wrapper 

//#- `strArr = process.args` -- array of command-line arguments 
//#     used to start current QMiner instance
process.args = qm.args;

//#- `bool = process.isArg(argStr)` -- returns true when `argStr` among the 
//#     command-line arguments used to start current QMiner instance
process.isArg = function(arg) {
    for (var i = 0; i < process.args.length; i++) {
        if (process.args[i] == arg) { return true; }
	}
	return false;
}

//#- `str = process.getArg(argStr)` -- returns the value of command-line 
//      argument with prefix `argStr`
process.getArg = function(arg, defValue) {
    for (var i = 0; i < process.args.length; i++) {
        if (process.args[i].indexOf(arg) == 0) { 
            return process.args[i].slice(arg.length);
        }
	}
	return defValue; 
}

//#- `num = process.getArgFloat(argStr)` -- returns the value of command-line 
//      argument with prefix `argStr`, and transform it to float before return
process.getArgFloat = function(arg, defValue) {
    var val = process.getArg(arg);
	return val ? parseFloat(val) : defValue; 
}

//#- `num = process.getArgInt(argStr)` -- returns the value of command-line 
//      argument with prefix `argStr`, and transform it to integer before return
process.getArgInt = function(arg, defValue) {
    var val = process.getArg(arg);
	return val ? parseInt(val) : defValue; 
}

//#- `sysStatJson = process.sysStat` -- statistics about system and qminer
//      process (E.g. memory consumption). Currently only works on LINUX.
process.sysStat = qm.sysStat;