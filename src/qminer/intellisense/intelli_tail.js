
// Manual override
_vec[0] = 1; // vec is indexed and returns numbers
_intVec[0] = 1; // vec is indexed and returns numbers
_addIntellisenseVar("_vec2", "_vec");
_addIntellisenseVar("_vec3", "_vec");
_addIntellisenseVar("_valVec", "_vec");
_addIntellisenseVar("_intVec2", "_intVec");
_addIntellisenseVar("_rowIdxVec", "_intVec");
_addIntellisenseVar("_colIdxVec", "_intVec");
_addIntellisenseVar("_mat2", "_mat");


// implement ignore
intellisense.addEventListener('statementcompletion', function (event) {
    if (event.targetName === "this") return;
    event.items = event.items.filter(function (item) {
        return !intellisenseIgnore.hasOwnProperty(item.name);
    });
});