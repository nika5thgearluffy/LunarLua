
if "%1"=="" SET CONFIG=Release
if NOT "%1"=="" SET CONFIG=%1

"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" LunaDll.sln /m /nologo /p:Configuration="%CONFIG%" /p:Platform="Win32"
