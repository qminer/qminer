node makedoc.js ../src/nodejs/ht/ht_nodejs.h ../src/nodejs/scripts/ht.js ../nodedoc/htdoc.js ../src/nodejs/ht/htDocHead.js ../src/nodejs/ht/StrStrDocData.js ../src/nodejs/ht/StrIntDocData.js ../src/nodejs/ht/StrFltDocData.js ../src/nodejs/ht/IntStrDocData.js ../src/nodejs/ht/IntIntDocData.js ../src/nodejs/ht/IntFltDocData.js

node makedoc.js ../src/nodejs/fs/fs_nodejs.h ../src/nodejs/scripts/fs.js ../nodedoc/fsdoc.js
node makedoc.js ../src/nodejs/analytics/analytics.h ../src/nodejs/scripts/analytics.js ../nodedoc/analyticsdoc.js

node makedoc.js ../src/nodejs/la/la_nodejs.h "" ../nodedoc/ladoc_module.js
node makedoc.js ../src/nodejs/la/la_structures_nodejs.h ../src/nodejs/scripts/la.js ../nodedoc/ladoc_structures.js ../nodedoc/ladoc_module.js
node makedoc.js ../src/nodejs/la/la_vector_nodejs.h "" ../nodedoc/ladoc.js ../nodedoc/ladoc_structures.js ../src/nodejs/la/VectorDocData.js ../src/nodejs/la/StrVectorDocData.js ../src/nodejs/la/IntVectorDocData.js ../src/nodejs/la/BoolVectorDocData.js

node makedoc.js ../src/nodejs/qm/qm_nodejs.h ../src/nodejs/scripts/qm.js ../nodedoc/qminerdoc.js
node makedoc.js ../src/nodejs/qm/qm_nodejs_streamaggr.h "" ../nodedoc/qminer_aggrdoc.js

node makedoc.js ../src/nodejs/statistics/stat_nodejs.h ../src/nodejs/scripts/statistics.js ../nodedoc/statdoc.js

node appendIntellisense ../nodedoc/fsdoc.js ../src/nodejs/intellisense/fs_intellisense.js "exports = {}; require.modules.qminer_fs = exports;"
node appendIntellisense ../nodedoc/ladoc.js ../src/nodejs/intellisense/la_intellisense.js "exports = {}; require.modules.qminer_la = exports;"
node appendIntellisense ../nodedoc/htdoc.js ../src/nodejs/intellisense/ht_intellisense.js "exports = {}; require.modules.qminer_ht = exports;"
node appendIntellisense ../nodedoc/statdoc.js ../src/nodejs/intellisense/statistics_intellisense.js "exports = {}; require.modules.qminer_stat = exports;"
node appendIntellisense ../nodedoc/analyticsdoc.js ../src/nodejs/intellisense/analytics_intellisense.js "exports = {}; require.modules.qminer_analytics = exports;"
node appendIntellisense ../nodedoc/qminerdoc.js ../src/nodejs/intellisense/qminer_intellisense.js "// this file mimicks the qminer module index.js file\nexports = {}; require.modules.qminer = exports;\nexports.la = require('qminer_la');\nexports.fs = require('qminer_fs');\nexports.analytics = require('qminer_analytics');\nexports.ht= require('qminer_ht');\nexports.statistics= require('qminer_stat');"

jsdoc --template ../node_modules/jsdoc-baseline ../nodedoc/htdoc.js ../nodedoc/fsdoc.js ../nodedoc/analyticsdoc.js  ../nodedoc/ladoc.js ../nodedoc/qminerdoc.js ../nodedoc/statdoc.js ../nodedoc/qminer_aggrdoc.js -d ../nodedoc

node factorydoc.js ../nodedoc/module-qm.html
node factorydoc.js ../nodedoc/module-qm.Iterator.html
node factorydoc.js ../nodedoc/module-qm.Record.html
node factorydoc.js ../nodedoc/module-qm.RecordSet.html
node factorydoc.js ../nodedoc/module-qm.Store.html

node removeTimestamp.js ../nodedoc

cp -r pictures/ ../nodedoc/pictures/

node changeDocHeader.js ../nodedoc/

node createExampleTests.js ../nodedoc/ ../test/nodejs/
