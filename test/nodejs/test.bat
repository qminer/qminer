SET CURRENT_PATH=%CD%
SET TEST_PATH=%~dp0

cd %TEST_PATH%

call npm install -g mocha
call mocha --timeout 3000 *.js

SET EXAMPLES_PATH=.\..\..\examples\streamaggregate
cd %EXAMPLES_PATH%

call npm install
call npm test

cd %CURRENT_PATH%