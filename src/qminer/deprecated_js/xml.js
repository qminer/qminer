/*	This work is licensed under Creative Commons GNU LGPL License.

	License: http://creativecommons.org/licenses/LGPL/2.1/
   Version: 0.9
	Author:  Stefan Goessner/2006
	Web:     http://goessner.net/ 
*/

///////////////////////////////
// XML utilities
//#
//# ### xml.js (use require)
//#
//# Utilities for transforming XML to JSON and JSON to XML.
//# Code is taken from [http://goessner.net/download/prj/jsonxml/]
//# 
//# **Functions and properties:**
//#

exports.escapeXml = function(str) {
    return str.replace(/&/g, '&amp;')
              .replace(/</g, '&lt;')
              .replace(/>/g, '&gt;')
              .replace(/"/g, '&quot;')
              .replace(/'/g, '&apos;');
}

//# - str = json2xml(json, nl) -- takes input object 'json' and transforms it to XML; `nl` controls new lines after tags
exports.json2xml = function (o, nl) {
   var toXml = function(v, name) {
      var xml = "";
      if (v instanceof Array) {
         for (var i=0, n=v.length; i<n; i++)
            xml += toXml(v[i], name);
      }
      else if (typeof(v) == "object") {
         var hasChild = false;
         xml += "<" + name;
         for (var m in v) {
            if (m.charAt(0) == "@")
               xml += " " + m.substr(1) + "=\"" + v[m].toString() + "\"";
            else
               hasChild = true;
         }
         xml += hasChild ? ">\n" : "/>\n";
         if (hasChild) {
            for (var m in v) {
               if (m == "#text")
                  xml += v[m];
               else if (m == "#cdata")
                  xml += "\n<![CDATA[" + v[m] + "]]>";
               else if (m.charAt(0) != "@")
                  xml += toXml(v[m], m);
            }
            xml += "</" + name + ">\n";
         }
      }
      else {
         xml += "<" + name + ">\n" + exports.escapeXml(v.toString()) +  "\n</" + name + ">\n";
      }
      return xml;
   }, xml="";
   for (var m in o)
      xml += toXml(o[m], m);
   return nl ? xml : (xml.replace(/\n</g, "<").replace(/>\n/g, ">"));
};


