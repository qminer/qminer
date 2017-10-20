function isObjectAndNotArray(object) {
    return (typeof object === 'object' && !Array.isArray(object));
}

// 'createNew' defaults to false
function overwriteKeys(baseObject, overrideObject, createNew) {
  if (!baseObject) {
    baseObject = {};
  }
  if (createNew) {
    baseObject = JSON.parse(JSON.stringify(baseObject));
  }
  Object.keys(overrideObject).forEach(function(key) {
    if (isObjectAndNotArray(baseObject[key]) && isObjectAndNotArray(overrideObject[key])) {
      overwriteKeys(baseObject[key], overrideObject[key]);
    }
    else {
      baseObject[key] = overrideObject[key];
    }
  });
  return baseObject;
}

module.exports = overwriteKeys;