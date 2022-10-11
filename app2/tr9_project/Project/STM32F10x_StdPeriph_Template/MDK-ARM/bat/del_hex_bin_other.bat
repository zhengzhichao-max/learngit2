::del /Q ..\tr9_hex_bin\*.htm
::del /Q ..\tr9_hex_bin\*.fed
::del /Q ..\tr9_hex_bin\*.l2p
::del /Q ..\tr9_hex_bin\*.lnp
::del /Q ..\tr9_hex_bin\*.axf
::del /Q ..\tr9_hex_bin\*.map
::del /Q ..\tr9_hex_bin\*.sct

del /Q .\tr9_hex_bin\*.htm
del /Q .\tr9_hex_bin\*.fed
del /Q .\tr9_hex_bin\*.l2p
del /Q .\tr9_hex_bin\*.lnp
::del /Q .\tr9_hex_bin\*.axf
del /Q .\tr9_hex_bin\*.map
del /Q .\tr9_hex_bin\*.sct

%rd /Q /S Project\Obj%
%rd /Q /S Project\List%