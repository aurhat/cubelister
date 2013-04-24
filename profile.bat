@SET PT="%PROGRAMFILES%\Microsoft Visual Studio 10.0\Team Tools\Performance Tools"

%PT%\vsperfcmd /start:sample /output:profile\profile.vsp
%PT%\vsperfcmd /launch:csl.exe
%PT%\vsperfcmd /shutdown

%PT%\vsperfreport /summary:all /output:profile\ profile\profile.vsp

@PAUSE
