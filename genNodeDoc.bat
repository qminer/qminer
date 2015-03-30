node makedoc.js src/nodejs/ht/ht_nodejs.h src/nodejs/scripts/ht.js nodedoc/htdoc.js ./src/nodejs/ht/htDocHead.js ./src/nodejs/ht/StrStrDocData.js ./src/nodejs/ht/StrIntDocData.js ./src/nodejs/ht/StrFltDocData.js ./src/nodejs/ht/IntStrDocData.js ./src/nodejs/ht/IntIntDocData.js ./src/nodejs/ht/IntFltDocData.js

node makedoc.js src/nodejs/fs/fs_nodejs.h "" nodedoc/fsdoc.js
node makedoc.js src/nodejs/analytics/analytics.h src/nodejs/scripts/analytics.js nodedoc/analyticsdoc.js

node makedoc.js src/nodejs/la/la_structures_nodejs.h src/nodejs/scripts/la.js ./nodedoc/ladoc_structures.js

node makedoc.js src/nodejs/la/la_vector_nodejs.h "" ./nodedoc/ladoc.js ./nodedoc/ladoc_structures.js ./src/nodejs/la/VectorDocData.js ./src/nodejs/la/StrVectorDocData.js ./src/nodejs/la/IntVectorDocData.js ./src/nodejs/la/BoolVectorDocData.js

node appendIntellisense nodedoc/fsdoc.js src/nodejs/intellisense/fs_intellisense.js "exports = {}; require.modules.qminer_fs = exports;"
node appendIntellisense nodedoc/ladoc.js src/nodejs/intellisense/la_intellisense.js "exports = {}; require.modules.qminer_la = exports;"
node appendIntellisense nodedoc/htdoc.js src/nodejs/intellisense/ht_intellisense.js "exports = {}; require.modules.qminer_ht = exports;"
node appendIntellisense nodedoc/analyticsdoc.js src/nodejs/intellisense/analytics_intellisense.js "exports = {}; require.modules.qminer_analytics = exports;"

jsdoc nodedoc/htdoc.js nodedoc/fsdoc.js nodedoc/analyticsdoc.js  nodedoc/ladoc.js -d nodedoc