This is a hack to make intellisense work with qminer node mule.

1.
qminer_intellisense.js includes:
exports = {}; require.modules.qminer = exports;

This together with require_intellisense.js links require('qminer') with the exports object in qminer_intellisense.js

2.
Analytics example. analyticsdoc.js (generated with makedoc) gets prepended
with "exports = {}; require.modules.qminer_analytics = exports;" and saved to analytics_intellisense.js

This together with require_intellisense.js links require('qminer_analytics') with the exports object in analytics_intellisense.js 

3.
Then in qminer_intellisense.js we set:
exports.analytics = require('qminer_analytics');


This makes require('qminer') return the object that has
the analytics property, which links to the exports object in analytics_intellisense.js


/////////////////////

To use in Visual Studio 2013:
Tools -> Options -> Text Editor -> JavaScript -> Intellisense -> References

Select Reference Group: Implicit Web
Add all js files in src/nodejs/intellisense
Make sure require_intellisense.js is above others (order matters) and qminer_intellisense.js is below others !
