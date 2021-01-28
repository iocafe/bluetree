mkdir tmp64
cd tmp64
cmake -G "Visual Studio 16 2019" -A x64 ..

change "Runtime library" to Multi-threaded (/MT) in Visual Studio
change debug to C7 compatible

mkdir tmp32
cd tmp323
cmake -G "Visual Studio 16 2019" -A Win32 ..