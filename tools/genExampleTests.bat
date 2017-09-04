node makedoc.js ../src/nodejs/ht/ht_nodejs.h ../src/nodejs/scripts/ht.js ../nodedoc/htdoc.js ../src/nodejs/ht/htDocHead.js ../src/nodejs/ht/StrStrDocData.js ../src/nodejs/ht/StrIntDocData.js ../src/nodejs/ht/StrFltDocData.js ../src/nodejs/ht/IntStrDocData.js ../src/nodejs/ht/IntIntDocData.js ../src/nodejs/ht/IntFltDocData.js
node makedoc.js ../src/nodejs/fs/fs_nodejs.h "" ../nodedoc/fsdoc.js
node makedoc.js ../src/nodejs/analytics/analytics.h ../src/nodejs/scripts/analytics.js ../nodedoc/analyticsdoc.js
node makedoc.js ../src/nodejs/la/la_structures_nodejs.h ../src/nodejs/scripts/la.js ../nodedoc/ladoc_structures.js
node makedoc.js ../src/nodejs/la/la_vector_nodejs.h "" ../nodedoc/ladoc.js ../nodedoc/ladoc_structures.js ../src/nodejs/la/VectorDocData.js ../src/nodejs/la/StrVectorDocData.js ../src/nodejs/la/IntVectorDocData.js ../src/nodejs/la/BoolVectorDocData.js
node makedoc.js ../src/nodejs/qm/qm_nodejs.h ../src/nodejs/scripts/qm.js ../nodedoc/qminerdoc.js
node makedoc.js ../src/nodejs/qm/qm_nodejs_streamaggr.h "" ../nodedoc/qminer_aggrdoc.js
node makedoc.js ../src/nodejs/statistics/stat_nodejs.h "" ../nodedoc/statdoc.js
echo node makedoc.js "" ../src/nodejs/datasets/datasets.js ../nodedoc/datasetsdoc.js

node createExampleTests.js ../nodedoc/ ../test/nodejs/examples/