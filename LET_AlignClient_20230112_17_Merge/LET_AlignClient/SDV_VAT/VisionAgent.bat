set CURPATH=%cd%
cd /d C:\Windows\Microsoft.NET\Framework64\v4.0.30319
regasm %CURPATH%\Vision.Agent.dll /tlb:Vision.Agent.tlb
pause