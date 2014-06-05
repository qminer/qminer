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
// Console 

//#- `console.start()` - start interactive console; does not see local variables.
//#     Receiving empty imput (Enter was pressed) exits the interactive mode.
console.start = function () {
    while (true) {
        var ead1042dc6554fcc8d47d1070268184f = console.getln();
        if (ead1042dc6554fcc8d47d1070268184f == "") break;
        try {
            console.log("" + eval(ead1042dc6554fcc8d47d1070268184f));
        } catch (err) {
            console.log("Error: " + err.message);
        }
    }
}

//#- `console.startx(evalFun)` -- useful for debugging;
//#     insert in code: `console.startx(function (x) { return eval(x); })`
//#     in order for console to see and interact with local variables.
//#     Receiving empty imput (Enter was pressed) exits the interactive mode.
console.startx = function (x) {
    while (true) {
        var ead1042dc6554fcc8d47d1070268184f = console.getln();
        if (ead1042dc6554fcc8d47d1070268184f == "") break;
        try {
            console.log("" + x(ead1042dc6554fcc8d47d1070268184f));
        } catch (err) {
            console.log("Error: " + err.message);
        }
    }
}

//#- `console.pause()` -- waits until enter is pressed
console.pause = function() {console.getln()};