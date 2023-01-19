@echo on
del *.sdf /s /q
del *.aps /s /q

rd  x64 /s /q
rd  .VS /s /q
rd  Debug /s /q
rd  LET_AlignClient\Debug /s /q
rd  LET_AlignClient\Release /s /q
rd  LET_AlignClient\x64 /s /q

rd  LET_Communicator\\Debug /s /q