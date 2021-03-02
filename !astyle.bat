@echo off
set ASTYLEEXE=c:\bin\astyle.exe
set ASTYLEOPT=-n --options=astyle-cpp-options
%ASTYLEEXE% %ASTYLEOPT% *.h *.cpp --exclude=Resource.h --exclude=stdafx.h --exclude=stdafx.cpp
%ASTYLEEXE% %ASTYLEOPT% RenderVfw\*.cpp --exclude=stdafx.h --exclude=stdafx.cpp
%ASTYLEEXE% %ASTYLEOPT% RenderDX9\*.cpp --exclude=stdafx.h --exclude=stdafx.cpp
%ASTYLEEXE% %ASTYLEOPT% RenderOpenGL\*.cpp --exclude=stdafx.h --exclude=stdafx.cpp
%ASTYLEEXE% %ASTYLEOPT% RenderSDL\*.cpp --exclude=stdafx.h --exclude=stdafx.cpp
