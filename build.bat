set exit_code=1

echo Remember the current folder
set start_dir=%cd%

echo Change into the script's folder
cd %~dp0

echo Set the environment variables
call "%vs140cmntools%vsvars32.bat"

echo Build the Solution
msbuild VisualStudio\NDTable.sln /t:Clean,Build /p:Configuration=Release /p:Platform=x64

echo Run the tests
VisualStudio\x64\Release\NDTable_test.exe
set exit_code=%errorlevel%

echo Change back to the original folder
cd %start_dir%

EXIT /B %exit_code%
