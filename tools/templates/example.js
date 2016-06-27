
	function (callback) {
		describe("${describe}', number ${number}", function () {
			it("should make test number ${number}", function (done) {
				try {
					${content}
					done();
					callback();
				} catch (e) {
					done(e);
					callback(e);
				}
			});
		});
	},