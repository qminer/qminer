SET CURRENT_PATH=%CD%
SET TEST_PATH=%~dp0

cd %TEST_PATH%

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