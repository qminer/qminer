/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

/////////////////////////////////////////////////
// GUID
class TGuid {
public:
  // create GUID string, something like "1b4e28ba-2fa1-11d2-883f-0016d3cca427"
  static TStr GenGuid();
  // create GUID string which is a valid variable name
  static TStr GenSafeGuid();
};
