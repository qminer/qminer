///////////////////////////////////////////////////////////
// Creating the base that contains the text

var qm = require('qminer');
var svcBase = new qm.Base({
    mode: 'createClean',
    schema: [{
        name: 'TestText',
        fields: [
            { name: 'Text', type: 'string' },
            { name: 'Clasification', type: 'int' }
        ]
    }
    ]
});

// the text feature
var ftr = new qm.FeatureSpace(svcBase,
    { type: 'text', source: 'TestText', field: 'Text', normalize: true, weight: 'tfidf', tokenizer: { type: 'simple', stopwords: 'en' } },
    { type: 'numeric', source: 'TestText', field: 'Clasification' }
    );



///////////////////////////////////////////////////////////
// SVC Support Vector Machine Clasifier

///////////////////////////////////////////////////////////
// Server

// create the server
var path = require('path');
var express = require('express');
var app = express();
var http = require('http').Server(app);
var io = require('socket.io')(http);

// adding the css files (or static files) to the server
app.use(express.static(path.join(__dirname + '/public')));

// when the client calls the server, it calls the dataSystemStats() and the example.html
app.get('/', function (request, response) {
    response.sendFile(__dirname + '/svc.html');
});

// creates the server listener, so that we know that it's connected
http.listen(2000, '127.0.0.1');
console.log('Server running at http://127.0.0.1:2000/');

///////////////////////////////////////////////////////////
// Socket functions

io.on('connection', function (socket) {

    socket.on('InputText', function (data) {
        console.log('Got input text!');
        svcBase.store('TestText').push({ Text: data.text, Clasification: parseInt(data.clasification) });
    });

    socket.on('Predict', function (data) {

        // start of prediction
        var SVC = new qm.analytics.SVC();
        var features = ftr.extractMatrix(svcBase.store('TestText').allRecords);

        var clasification = features.getRow(features.rows - 1);

        SVC.fit(matrix);

    })

})