var request = require("supertest");
var makeServer = require('../server');

describe('Streamaggregate example test', function () {
	// run server before each test
	var server;
	beforeEach(function () {
		server = makeServer();		
	});
	
	// close server after each test
    afterEach(function (done) {
    	server.close(done);
    });
  
    // test server
	it('should responds to /', function (done) {
		request(server)
	    	.get('/')
	    	.expect(200, done);
	});
	
	it('testing fail', function (done) {
		request(server)
      		.get('/foo/bar')
    		.expect(200, done);
	})
		
	// test server
  	it('should repond 404 to everything else', function (done) {
    	request(server)
      		.get('/foo/bar')
    		.expect(404, done);
	});

});