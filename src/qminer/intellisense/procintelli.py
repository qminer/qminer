import re
import shutil
import sys
import copy
import pprint

fr = open('intellisense.js', 'r')
fw = open('intelli_body.js', 'w')
fwProblems = open('problems.js', 'w');

globalVars = set(['qm', 'la', 'process', 'http', 'console', 'fs'])

funDict = {};

for line in fr:
    line = line.strip()
    splitLine = line.split('$SEPARATOR$');
    line = splitLine[0];
    commentLine = splitLine[1].strip().strip('-');
    if len(line) == 0: continue
    # currently ignore indexed property []
    if "[" in line:
        # not handled automatically (add a line in the intellisense tail file)
        fwProblems.write(line + '\n')
        continue
    # currently ignore functions that take json objects
    if "{" in line:
        # not handled yet 
        fwProblems.write(line + '\n')
        continue
    if "'" in line:
        # not handled yet 
        fwProblems.write(line + '\n')
        continue
    if "\"" in line:
        # not handled yet 
        fwProblems.write(line + '\n')
        continue
    # determine if it is a prop or func? check last char
    objName = ""
    funPathNoObj = ""
    if (line[-1] != ')'):
        print("prop: " + line)
        # properties must contain '='
        if "=" in line:
            # len = vec.length
            # _vec.length = _len
            temp = line.split('=')
            lhs = temp[0].strip()
            rhs = temp[1].strip()
            temp2 = rhs.split('.')
            if len(temp2) > 0:
                funPathNoObj = rhs[rhs.index('.')+1 :]
            if temp2[0] not in globalVars:
                rhs = '_' + rhs
                objName = '_' + temp2[0]
            else:
                objName = temp2[0]           
            if lhs not in globalVars:
                lhs = '_' + lhs
            fw.write(rhs + " = " + lhs + '\n')
            if rhs not in funDict:
                funDict[rhs] = [];
                #({'return':lhs, 'comment':commentLine, 'funArgs':fixedFunArgs, 'isFun':true})
            funDict[rhs].append({'noObjPath': funPathNoObj, 'rhs':rhs, 'return':lhs, 'obj':objName, 'comment':commentLine, 'isFun':False, 'zline':line})
        else:
            print("not OK!: properties should have a return value. Fix doc!)")
            fwProblems.write(line + '\n')
    else:
        print("fun: " + line)
        # empty body by default
        funBody = ' {\n' + '/// <summary>' + commentLine + "</summary>"  + '\n}'
        #funBody = ' { }'
        rhs = ''
        lhs = ''
        if "=" in line:
            # body will have a return statement
            temp = line.split('=')
            lhs = temp[0].strip()
            if lhs not in globalVars:
                lhs = '_' + lhs;
                funBody = ' {\n' + '/// <summary>' + commentLine + "</summary>"  + '\nreturn ' + lhs + ';\n}'
                #funBody = ' { return _' + lhs + '; }'
            else:
                print("not OK!: globals should be unique and we are returning them. Fix doc!)")
                sys.exit()
            # prepare rhs
            rhs = temp[1].strip()
        else:
            # no lhs
            rhs = line
        # example: num = vec.inner(vec2)
        # we have function body funBody = { return _lhs; } or { }
        # attach _ to rhs if it is not a global var
        temp2 = rhs.split('.')        
        if len(temp2) > 1:
            funPathNoObj = rhs[rhs.index('.')+1 : rhs.index('(')]
        else:
            # not handled yet
            fwProblems.write(line + '\n')
            continue
        objName = temp2[0]
        if temp2[0] not in globalVars:
            rhs = '_' + rhs
            objName = '_' + temp2[0]
        else:
            objName = temp2[0]
        # so far we have _vec.inner(vec2) and { return _num; }
        # find first occurrence of (
        # "_vec.inner" + " = function " + "(vec2)" + " { return _num; }
        funArgs = rhs[rhs.index('(')+1 : -1]
        splitFunArgs = funArgs.split(',');
        fixedFunArgs = "";
        for arg in splitFunArgs:
            arg = arg.strip();
            if len(arg) > 0:
                # attach _
                if arg not in globalVars:
                    arg = '_' + arg;
                fixedFunArgs += arg + ', '
        # remove last comma and space
        fixedFunArgs = fixedFunArgs[:-2]
        funPath = rhs[:rhs.index('(')]
        funStr = rhs[:rhs.index('(')] + " = function (" + fixedFunArgs + ")" + funBody
        # todo: alter args        
        fw.write(funStr + '\n')  
        
        if funPath not in funDict:
            funDict[funPath] = []
        funDict[funPath].append({'noObjPath': funPathNoObj, 'funPath': rhs[:rhs.index('(')], 'rhs':rhs, 'return':lhs, 'obj':objName, 'comment':commentLine, 'funArgs':fixedFunArgs, 'isFun':True, 'zline':line})
    
fw.close()   
# write

#{'comment': '',
# 'isFun': False,
# 'noObjPath': 'length',
# 'obj': '_vec',
# 'return': '_len',
# 'rhs': '_vec.length',
# 'zline': 'len = vec.length'}
#{'comment': '',
# 'funArgs': '_vec2',
# 'funPath': '_vec.minus',
# 'isFun': True,
# 'noObjPath': 'minus',
# 'obj': '_vec',
# 'return': '_vec3',
# 'rhs': '_vec.minus(vec2)',
# 'zline': 'vec3 = vec.minus(vec2)'}
#var _vec = {
#    /// <field value = '_len'>lenz</field>
#    length: _len,
#    minus: function (_vec2) {
#        /// <signature>
#        /// <summary> minuz</summary>
#        /// <param name="_vec2" value="_vec2">vecz</param>
#        /// </signature>
#        return _vec3;
#    }
#} 

# collection of objects, each is a collection of noObjPaths, each is a collection of sigs
# objDict: keys are objects, values are noObjPath 
objDict = {};
for key in sorted(funDict.keys()):        
    for sig in funDict[key]:
        # ignore nested
        if sig["rhs"].count('.') != 1:
            continue
        # hashmap of funPaths
        # create hashmap of all objects
        obj = sig["obj"]
        fun = sig["noObjPath"]

        if obj not in objDict:
            objDict[obj] = {}
        if fun not in objDict[obj]:
            objDict[obj][fun] = []
        objDict[obj][fun].append(sig)

fw3 = open('intelli_body2.js', 'w')
pprint.pprint(objDict)
#objDict[sig["obj"]].append(sig)
for obj in objDict:
    fw3.write('var ' + obj + ' = {\n')
    for fun in objDict[obj]:
        isFun = objDict[obj][fun][0]["isFun"]
        if not isFun:
# 'isFun': False,
# 'noObjPath': 'length',
# 'obj': '_vec',
# 'return': '_len',
            sig = objDict[obj][fun][0]
            #/// <field value = '_len'>lenz</field>
            fw3.write("\t/// <field value = \"" + sig["return"] + "\">" + sig["comment"] + "</field>\n")  
            #length: _len,
            fw3.write("\t" + sig["noObjPath"] + ": " + sig["return"] +  ",\n")
        else:
#    minus: function (_vec2) {
            sig = objDict[obj][fun][0]
            fw3.write("\t" + sig["noObjPath"] + ": function () {\n")
            for sig in objDict[obj][fun]:
                fw3.write("\t/// <signature>\n")
                fw3.write("\t/// <summary>" + sig["comment"] + "</summary>\n")
                params = sig["funArgs"].split(",");
                #        /// <param name="_vec2" value="_vec2">vecz</param>
                for param in params:
                    if len(param.strip()) > 0:
                        fw3.write("\t/// <param name=\"" + param.strip() + "\" value=\"" + param.strip() + "\">param</param>\n")
                fw3.write("\t/// <returns value =\"" + sig["return"] + "\"/>\n")
                fw3.write("\t/// </signature>\n\n")
            fw3.write('\t},\n\n')
            
# 'isFun': True,
# 'funArgs': '_vec2',
# 'noObjPath': 'minus',
# 'obj': '_vec',
# 'return': '_vec3',
    fw3.write('}\n\n')
fw3.close()


# Combine autogenerated with tail
# head (skip)
shutil.copy2('intelli_tail.js', '../qminer.intellisense.js')
fw2 = open('../qminer.intellisense.js', 'a')
# body
fr = open('intelli_body2.js', 'r')
for line in fr:
    fw2.write(line)
## tail
#fr = open('intelli_tail.js', 'r')
#for line in fr:
#    fw2.write(line)
#    

