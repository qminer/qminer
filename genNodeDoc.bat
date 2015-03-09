node makedoc.js src/nodejs/ht/ht_nodejs.h src/nodejs/scripts/ht.js nodedoc/htdoc.js
node makedoc.js src/nodejs/fs/fs_nodejs.h "" nodedoc/fsdoc.js
jsdoc nodedoc/htdoc.js nodedoc/fsdoc.js -d nodedoc