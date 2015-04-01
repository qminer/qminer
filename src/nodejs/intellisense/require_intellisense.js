// trick from https://github.com/ishisaka/nodeintellisense/
function require(name) {
    return require.modules[name];
};

// Contains modules recognized by require()
// Add a property to this object to support
// other modules.
require.modules = {};