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

//#- `a = process.args` -- array of command-line arguments 
//#     used to start current QMiner instance
process.args = qm.args;

//#- `process.isArg(arg)` -- returns true when `arg` among the 
//#     command-line arguments used to start current QMiner instance
process.isArg = function(arg) {
    for (var i = 0; i < process.args.length; i++) {
        if (process.args[i] == arg) { return true; }
	}
	return false;
}

//#- `process.sysStat` -- statistics about system and qminer process (E.g. memory consumption).
process.sysStat = qm.sysStat;