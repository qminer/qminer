/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// JavaScript source code
var la = require('../../index.js').la;
var analytics = require('../../index.js').analytics;
var assert = require("../../src/nodejs/scripts/assert.js");
//Unit test for tokenizer

describe("Tokenizer tests", function () {
    beforeEach(function () {

    });
    afterEach(function () {

    });
    describe("constructor test", function () {
        it("should not throw an exception, type simple", function () {
            assert.doesNotThrow(function () {
                var tokenizer = new analytics.Tokenizer({ type: "simple" });
            });
        });
        it("should not throw an exception, type Html", function () {
            assert.doesNotThrow(function () {
                var tokenizer = new analytics.Tokenizer({ type: "html" });
            });
        });
        it("should not throw an exception, type Html-Unicode", function () {
            assert.doesNotThrow(function () {
                var tokenizer = new analytics.Tokenizer({ type: "unicode" });
           });
        });
    });
    describe("getTokens test", function () {
        it("should not throw an exception using getTokens", function () {
            var tokenizer = new analytics.Tokenizer({ type: "simple" });
            var string = "Sunny Day";
            assert.doesNotThrow(function () {
                var tokens = tokenizer.getTokens(string);
            });
        });
        it("should return tokens of a string", function () {
            var tokenizer = new analytics.Tokenizer({ type: "simple" });
            var string = "It is alright!";
            var tokens = tokenizer.getTokens(string)
            assert.strictEqual(tokens[0], "IT");
            assert.strictEqual(tokens[1], "IS");
            assert.strictEqual(tokens[2], "ALRIGHT");
        });
        it("should return tokens of a string using type Html", function () {
            var tokenizer = new analytics.Tokenizer({ type: "html" });
            var string = "It is alright!";
            var tokens = tokenizer.getTokens(string)
            assert.strictEqual(tokens[0], "it");
            assert.strictEqual(tokens[1], "is");
            assert.strictEqual(tokens[2], "alright");
        });
        it("should return tokens using contractions", function () {
            var tokenizer = new analytics.Tokenizer({ type: "simple" });
            var string = "It's alright!";
            var tokens = tokenizer.getTokens(string)
            assert.strictEqual(tokens[0], "IT'S");
            assert.strictEqual(tokens[1], "ALRIGHT");
        });
        it("should return tokens using contractions with type Html", function () {
            var tokenizer = new analytics.Tokenizer({ type: "html" });
            var string = "It's alright.";
            var tokens = tokenizer.getTokens(string)
            assert.strictEqual(tokens[0], "it");
            assert.strictEqual(tokens[1], "s");
            assert.strictEqual(tokens[2], "alright");
        });
        it("should return tokens ignoring extra punctuation", function () {
            var tokenizer = new analytics.Tokenizer({ type: "simple" });
            var string = "It, ./ is,) alright!!";
            var tokens = tokenizer.getTokens(string)
            assert.strictEqual(tokens[0], "IT");
            assert.strictEqual(tokens[1], "IS");
            assert.strictEqual(tokens[2], "ALRIGHT");
        });
        it("should return tokens ignoring extra punctuation using type Html", function () {
            var tokenizer = new analytics.Tokenizer({ type: "html" });
            var string = "It, ./ is,) alright!!";
            var tokens = tokenizer.getTokens(string)
            assert.strictEqual(tokens[0], "it");
            assert.strictEqual(tokens[1], "is");
            assert.strictEqual(tokens[2], "alright");
        });
        it("should throw an exception if input is not a string", function () {
            var tokenizer = new analytics.Tokenizer({ type: "simple" });
            var object = { type: "Fiat", model: 500, color: "white" };
            assert.throws(function () {
                var tokens = tokenizer.getTokens(object);
            });
        });
        it("should not throw an exception if input string is empty", function () {
            var tokenizer = new analytics.Tokenizer({ type: "simple" });
            var emptyString = "";
            var tokens = tokenizer.getTokens(emptyString);
            assert.strictEqual(tokens[0], undefined);
        });
    });
    describe("getSentences test", function () {
        it("should not throw an exception using getSentences", function () {
            var tokenizer = new analytics.Tokenizer({ type: "simple" });
            var string = "It is alright. Do not worry!";
            assert.doesNotThrow(function () {
                var sentences = tokenizer.getSentences(string);
            });
        });
        it("should return sentences of text", function () {
            var tokenizer = new analytics.Tokenizer({ type: "simple" });
            var text = "It is alright. Do not worry!";
            var sentences = tokenizer.getSentences(text);
            assert.strictEqual(sentences[0], "It is alright");
            assert.strictEqual(sentences[1], " Do not worry");
        });
        it("should return sentences of text using contractions", function () {
            var tokenizer = new analytics.Tokenizer({ type: "simple" });
            var text = "It's alright. Don't worry!";
            var sentences = tokenizer.getSentences(text);
            assert.strictEqual(sentences[0], "It's alright");
            assert.strictEqual(sentences[1], " Don't worry");
        });
        it("should return sentences of text using extra contractions", function () {
            var tokenizer = new analytics.Tokenizer({ type: "simple" });
            var text = "Listen, my idea was better. Please, don't take this personally...";
            var sentences = tokenizer.getSentences(text);
            assert.strictEqual(sentences[0], "Listen, my idea was better");
            assert.strictEqual(sentences[1], " Please, don't take this personally");
        });
        it("should throw an exception if input is not a string", function () {
            var tokenizer = new analytics.Tokenizer({ type: "simple" });
            var array = new la.Vector([1, 2]);
            assert.throws(function () {
                var tokens = tokenizer.getSentences(array);
            });
        });
    });
    describe("getParagraphs test", function () {
        it("should not throw an exception using getParagraphs", function () {
            var tokenizer = new analytics.Tokenizer({ type: "simple" });
            var string = "My car is badass. \n But so is my life!";
            assert.doesNotThrow(function () {
                var paragraphs = tokenizer.getParagraphs(string);
            });
        });
        it("should throw an exception if input is not a string", function () {
            var tokenizer = new analytics.Tokenizer({ type: "simple" });
            var array = new la.Vector([1, 2]);
            assert.throws(function () {
                var tokens = tokenizer.getParagraphs(array);
            });
        });
        it("should break text into paragraphs", function () {
            var tokenizer = new analytics.Tokenizer({ type: "simple" });
            var text = "Yes!\t No?\n Maybe...";
            var paragraphs = tokenizer.getParagraphs(text);
            assert.strictEqual(paragraphs[0], "Yes");
            assert.strictEqual(paragraphs[1], " No");
            assert.strictEqual(paragraphs[2], " Maybe");
        });
        it("should not throw an exception if the string is empty", function () {
            var tokenizer = new analytics.Tokenizer({ type: "simple" });
            var string = "\n";
            assert.doesNotThrow(function () {
                var paragraphs = tokenizer.getParagraphs(string);
            });
        });
    });
});
