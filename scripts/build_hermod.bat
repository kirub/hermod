pushd ..
bootstrap.bat && "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86_x64 && msbuild hermod.sln
popd