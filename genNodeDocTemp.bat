mkdir nodetempdoc
node makedoc.js src/nodejs/ht/ht_nodejs.h src/nodejs/scripts/ht.js nodetempdoc/htdoc.js ./src/nodejs/ht/htDocHead.js ./src/nodejs/ht/StrStrDocData.js ./src/nodejs/ht/StrIntDocData.js ./src/nodejs/ht/StrFltDocData.js ./src/nodejs/ht/IntStrDocData.js ./src/nodejs/ht/IntIntDocData.js ./src/nodejs/ht/IntFltDocData.js

node makedoc.js src/nodejs/fs/fs_nodejs.h "" nodetempdoc/fsdoc.js
node makedoc.js src/nodejs/analytics/analytics.h src/nodejs/scripts/analytics.js nodetempdoc/analytics.js

node makedoc.js src/nodejs/la/la_structures_nodejs.h src/nodejs/scripts/la.js ./nodetempdoc/ladoc_structures.js

node makedoc.js src/nodejs/la/la_vector_nodejs.h "" ./nodetempdoc/ladoc.js ./nodetempdoc/ladoc_structures.js ./src/nodejs/la/VectorDocData.js ./src/nodejs/la/StrVectorDocData.js ./src/nodejs/la/IntVectorDocData.js ./src/nodejs/la/BoolVectorDocData.js

node makedoc.js src/nodejs/qm/qm_nodejs.h "" ./nodetempdoc/qminerdoc.js

jsdoc nodetempdoc/htdoc.js nodetempdoc/fsdoc.js nodetempdoc/analytics.js  nodetempdoc/ladoc.js nodetempdoc/qminerdoc.js -d nodetempdoc