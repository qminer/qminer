SET CURRENT_PATH=%CD%
SET TEST_PATH=%~dp0

cd %TEST_PATH%

cd examples
call mocha -b --timeout 30000 exampleanalyticsdoc.js
call mocha -b --timeout 30000 examplefsdoc.js
call mocha -b --timeout 30000 examplehtdoc.js
call mocha -b --timeout 30000 exampleladoc.js
call mocha -b --timeout 30000 exampleladoc_module.js
call mocha -b --timeout 30000 exampleladoc_structures.js
call mocha -b --timeout 30000 exampleqminer_aggrdoc.js
call mocha -b --timeout 30000 exampleqminerdoc.js
call mocha -b --timeout 30000 examplestatdoc.js
cd ..

call npm install -g mocha
call mocha -b --timeout 30000 *.js
if errorlevel 1 (
   echo Failure Reason Given is %errorlevel%
   cd %CURRENT_PATH%
   exit /b %errorlevel%
)

SET EXAMPLES_PATH=.\..\..\examples\streamaggregate
cd %EXAMPLES_PATH%

call npm install --silent > npm_install.log
call npm test
if errorlevel 1 (
   echo Failure Reason Given is %errorlevel%
   cd %CURRENT_PATH%
   exit /b %errorlevel%
)

cd %CURRENT_PATH%