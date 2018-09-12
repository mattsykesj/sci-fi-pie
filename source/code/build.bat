@echo off

mkdir ..\..\build\debug
pushd ..\..\build\debug
cl -FC -Zi ..\..\source\code\win32_Sci_Fi_Pie.cpp User32.lib Gdi32.lib Winmm.lib Xinput.lib XAudio2.lib
popd 