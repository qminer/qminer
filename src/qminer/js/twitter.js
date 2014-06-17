/**
 * QMiner - Open Source Analytics Platform
 * 
 * Copyright (C) 2014 Jozef Stefan Institute
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

///////////////////////////////
// Twitter utilities
//#
//# ### twitter.js (use require)
//#
//# Utilities and store definition for working with Twitter data. 
//# The library must be loaded using `var twitter = require('twitter.js');`.
//# 
//# **Functions and properties:**
//#
//#- `var twitterDef = new twitter.TwitterStore()` -- returns a Twitter store definition JSON object `twitterDef`. The JSON array contains four store definitions: Tweets, Users, HashTags and Pages
exports.TwitterStore = function () {
    var defJSON = [
    {
        "name": "Tweets",
        "fields": [
            {
                "name": "ID",
                "type": "string",
                "primary": true
            },
            {
                "name": "Date",
                "type": "datetime"
            },
            {
                "name": "Text",
                "type": "string",
                "store": "cache"
            },
            {
                "name": "Geo",
                "type": "float_pair",
                "null": true
            },
            {
                "name": "Category",
                "type": "int",
                "null": true
            }
        ],
        "joins": [
            {
                "name": "author",
                "type": "field",
                "store": "Users",
                "inverse": "author"
            },
            {
                "name": "hasUser",
                "type": "index",
                "store": "Users",
                "inverse": "hasUser"
            },
            {
                "name": "hasHashtag",
                "type": "index",
                "store": "Hashtags",
                "inverse": "hasHashtag"
            },
            {
                "name": "hasPages",
                "type": "index",
                "store": "Pages",
                "inverse": "hasPages"
            },
            {
                "name": "hasMedia",
                "type": "index",
                "store": "Pages",
                "inverse": "hasMedia"
            }
        ],
        "keys": [
            {
                "field": "Text",
                "type": "text"
            },
            {
                "field": "Geo",
                "type": "location"
            }
        ]
    },
    {
        "name": "Users",
        "fields": [
            {
                "name": "Username",
                "type": "string",
                "primary": true
            },
            {
                "name": "Name",
                "type": "string",
                "null": true
            },
            {
                "name": "Location",
                "type": "string",
                "null": true
            },
            {
                "name": "Followers",
                "type": "int",
                "null": true
            },
            {
                "name": "Friends",
                "type": "int",
                "null": true
            }
        ],
        "joins": [
            {
                "name": "author",
                "type": "index",
                "store": "Tweets",
                "inverse": "author"
            },
            {
                "name": "hasUser",
                "type": "index",
                "store": "Tweets",
                "inverse": "hasUser"
            }
        ],
        "keys": [
            {
                "field": "Location",
                "type": "value"
            }
        ]
    },
    {
        "name": "Hashtags",
        "fields": [
            {
                "name": "Name",
                "type": "string",
                "primary": true
            }
        ],
        "joins": [
            {
                "name": "hasHashtag",
                "type": "index",
                "store": "Tweets",
                "inverse": "hasHashtag"
            }
        ]
    },
    {
        "name": "Pages",
        "fields": [
            {
                "name": "URL",
                "type": "string",
                "primary": true
            }
        ],
        "joins": [
            {
                "name": "hasPages",
                "type": "index",
                "store": "Tweets",
                "inverse": "hasPages"
            },
            {
                "name": "hasMedia",
                "type": "index",
                "store": "Tweets",
                "inverse": "hasMedia"
            }
        ]
    }
    ];
    return defJSON;
}
//#- `var Parser = new twitter.Parser()` -- creates an object that converts between raw Twitter JSON objects and qminer store compatible JSON objects. Exposes:
exports.Parser = function () {
    // support functions for parsing new tweets
    var months = {};
    months["Jan"] = 1; months["Feb"] = 2; months["Mar"] = 3; months["Apr"] = 4;
    months["May"] = 5; months["Jun"] = 6; months["Jul"] = 7; months["Aug"] = 8;
    months["Sep"] = 9; months["Oct"] = 10; months["Nov"] = 11; months["Dec"] = 12;

    var _parseTwitterTime = function (a) { return (a < 10) ? ("0" + a) : a; }

    var parseTwitterTime = function (date_str) {
        // parse date
        var date_part = date_str.split(' ');
        var year = parseInt(date_part[5]);
        var month = months[date_part[1]];
        var day = parseInt(date_part[2]);
        // prase time
        var time_part = date_part[3].split(':');
        var hour = parseInt(time_part[0]);
        var minute = parseInt(time_part[1]);
        var second = parseInt(time_part[2]);
        // return qminer time string
        return year + "-" +
            _parseTwitterTime(month) + "-" +
            _parseTwitterTime(day) + "T" +
            _parseTwitterTime(hour) + ":" +
            _parseTwitterTime(minute) + ":" +
            _parseTwitterTime(second);
    };

    //#   - `obj = twitter.Parser.rawJsonToStoreJson(raw)` -- transforms a raw JSON object (result of twitter crawler) `raw` to `twitter.TwitterStore()` compatible json object `obj`
    this.rawJsonToStoreJson = function (raw) {
        var tweet = {};
        tweet.ID = raw.id_str;
        tweet.Date = parseTwitterTime(raw.created_at);
        tweet.Text = raw.text;
        // get user
        var user = {};
        user.Username = raw.user.screen_name;
        user.Name = raw.user.name;
        user.Location = raw.user.location;
        user.Followers = raw.user.followers_count;
        user.Friends = raw.user.friends_count;
        tweet.author = user;
        // get location
        try { tweet.Geo = raw.geo.coordinates; }
        catch (err) { /* no location */ }
        // entities
        if (raw.entities) {
            if (raw.entities.hashtags) {
                var _hashtags = raw.entities.hashtags;
                var hashtags = [];
                for (var i = 0; i < _hashtags.length; i++) {
                    hashtags.push({ Name: _hashtags[i].text });
                }
                tweet.hasHashtag = hashtags;
            }
            if (raw.entities.user_mentions) {
                var _user_mentions = raw.entities.user_mentions;
                var user_mentions = [];
                for (var i = 0; i < _user_mentions.length; i++) {
                    user_mentions.push({ Username: _user_mentions[i].screen_name });
                }
                tweet.hasUser = user_mentions;
            }
            if (raw.entities.urls) {
                var _pages = raw.entities.urls;
                var pages = [];
                for (var i = 0; i < _pages.length; i++) {
                    pages.push({ URL: _pages[i].expanded_url });
                }
                tweet.hasPages = pages;
            }
            if (raw.entities.media) {
                var _media = raw.entities.media;
                var media = [];
                for (var i = 0; i < _media.length; i++) {
                    media.push({ URL: _media[i].expanded_url });
                }
                tweet.hasMedia = media;
            }
        }
        return tweet;
    };
};

//#- `twitter.RawToStore(fin, fout)` -- converts twitter JSON lines to `twitter.TwitterStore()` compatible JSON lines, given input stream `fin` (raw JSON lines) and output stream `fout` (store JSON lines file)
exports.RawToStore = function (fin, fout) {
    var Parser = new exports.Parser();
    var count = 0;
    while (!fin.eof) {
        var line = fin.getNextLn();
        if (line == "") { continue; }
        try {
            //console.startx(function (x) { return eval(x); }); // debug breakpoint
            var rawJson = JSON.parse(line);   
            var storeJson = Parser.rawJsonToStoreJson(rawJson);
            fout.writeLine(JSON.stringify(storeJson));
            count++;
        } catch (err) {
            console.log("Error parsing [" + line + "]: " + err)
        }
    }
    console.log("Transformed " + count + " lines.");
    return count;
};