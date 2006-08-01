mkdir unangband-061-win
mkdir unangband-061-win\lib
mkdir unangband-061-win\lib\apex
mkdir unangband-061-win\lib\bone
mkdir unangband-061-win\lib\data
mkdir unangband-061-win\lib\edit
mkdir unangband-061-win\lib\file
mkdir unangband-061-win\lib\help
mkdir unangband-061-win\lib\info
mkdir unangband-061-win\lib\pref
mkdir unangband-061-win\lib\save
mkdir unangband-061-win\lib\script
mkdir unangband-061-win\lib\user
mkdir unangband-061-win\lib\xtra
mkdir unangband-061-win\lib\xtra\font
mkdir unangband-061-win\lib\xtra\graf
mkdir unangband-061-win\lib\xtra\music
mkdir unangband-061-win\lib\xtra\sound
mkdir unangband-061-win\lib\xtra\help

copy lib\apex\delete.me unangband-061-win\lib\apex
copy lib\bone\delete.me unangband-061-win\lib\bone
copy lib\data\delete.me unangband-061-win\lib\data
copy lib\info\delete.me unangband-061-win\lib\info
copy lib\save\delete.me unangband-061-win\lib\save
copy lib\user\delete.me unangband-061-win\lib\user
copy lib\xtra\music\delete.me unangband-061-win\lib\xtra\music

copy unangband.exe unangband-061-win
copy readme.txt unangband-061-win
copy changes.txt unangband-061-win

copy lib\edit\*.txt unangband-061-win\lib\edit

copy lib\file\*.txt unangband-061-win\lib\file

copy lib\help\*.txt unangband-061-win\lib\help
copy lib\help\*.hlp unangband-061-win\lib\help

copy lib\pref\*.prf unangband-061-win\lib\pref

copy lib\xtra\font\*.fon unangband-061-win\lib\xtra\font

copy lib\xtra\graf\*.bmp unangband-061-win\lib\xtra\graf

copy lib\xtra\sound\sound.cfg unangband-061-win\lib\xtra\sound
copy lib\xtra\sound\*.wav unangband-061-win\lib\xtra\sound

copy lib\xtra\help\angband.hlp unangband-061-win\lib\xtra\help
copy lib\xtra\help\angband.cnt unangband-061-win\lib\xtra\help

upx -9 unangband-061-win\unangband.exe

7z a -tzip -r unangband-061-win.zip unangband-061-win

rmdir /q /s unangband-061-win

