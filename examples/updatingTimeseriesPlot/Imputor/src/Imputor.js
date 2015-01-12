time = require('time.js');

var importData = function (limit) {
    var count = 0; //counter used for counting iterations when limit is defined
    var tm = time.now;
    var tmshift = tm.clone();
    var periodInSeconds = 60;

    var respCallBack = function (resp) {
        tmshift.add(1, 'second');
        var valshift = Math.sin(2 * Math.PI * tmshift.timestamp / periodInSeconds);

        var rec = { Time: tmshift.string, Val: valshift }

        console.log("Response: " + resp);
        //eval(breakpoint)
        process.sleep(2000)

        sendData(rec);
    }

    var errCallBack = function (err) { console.log(err); eval(breakpoint); sendData(); }

    var sendData = function (_data) {
        // If input parameter limit is defined
        if (limit != null) {
            if (count > limit) {
                console.log("Reached count limit at " + limit);
                return;
            } else count++
        }

        var root = "http://localhost:8080/updatingTsPlot/import?data=";
        var data = (_data == null) ? null : JSON.stringify(_data);
        var url = root + data;
        console.log("Sending data...\n" + JSON.stringify(_data, undefined, 2));

        http.getStr(url, respCallBack, errCallBack);
    }

    sendData() //Start the process
}

importData(1000)
//importData(1000)

// DEBUGGING
//console.log("Console mode...")
//eval(breakpoint)