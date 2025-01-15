REM pushd ..
set SOLUTION_FILE_PATH=..\build\hermod.sln
set BUILD_CONFIGURATION=Debug
bootstrap.bat && "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86_x64 && msbuild /m /p:Configuration=%BUILD_CONFIGURATION% %SOLUTION_FILE_PATH%
REM popd