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


function isObjectAndNotArray(object) {
    return (typeof object === 'object' && !Array.isArray(object));
}

// 'createNew' defaults to false
function overwriteKeys(baseObject, overrideObject, createNew) {
    if (createNew) {
        baseObject = JSON.parse(JSON.stringify(baseObject));
    }
    Object.keys(overrideObject).forEach(function (key) {
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