describe("${describe}', number ${number}", function () {
	it("should make test number ${number}", function (done) {
		try {
			${content}
			done();
		} catch (e) {
			done(e);
		}
	});
});