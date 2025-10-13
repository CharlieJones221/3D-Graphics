del /s /q Demo\*.*
rd Demo
md Demo
md Demo\Data

Xcopy 3DG\External\bin Demo\
Xcopy /E /I 3DG\Data Demo\Data\
copy x64\Release\3DG.exe Demo\3DG.exe