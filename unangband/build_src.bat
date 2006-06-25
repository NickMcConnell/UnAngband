mkdir unangband-061-src
mkdir unangband-061-src\lib
mkdir unangband-061-src\lib\apex
mkdir unangband-061-src\lib\bone
mkdir unangband-061-src\lib\data
mkdir unangband-061-src\lib\edit
mkdir unangband-061-src\lib\file
mkdir unangband-061-src\lib\help
mkdir unangband-061-src\lib\info
mkdir unangband-061-src\lib\pref
mkdir unangband-061-src\lib\save
mkdir unangband-061-src\lib\user
mkdir unangband-061-src\lib\xtra
mkdir unangband-061-src\lib\xtra\font
mkdir unangband-061-src\lib\xtra\graf
mkdir unangband-061-src\lib\xtra\music
mkdir unangband-061-src\lib\xtra\sound
mkdir unangband-061-src\lib\xtra\help
mkdir unangband-061-src\src

copy lib\apex\delete.me unangband-061-src\lib\apex
copy lib\bone\delete.me unangband-061-src\lib\bone
copy lib\data\delete.me unangband-061-src\lib\data
copy lib\info\delete.me unangband-061-src\lib\info
copy lib\save\delete.me unangband-061-src\lib\save
copy lib\user\delete.me unangband-061-src\lib\user
copy lib\xtra\font\delete.me unangband-061-src\lib\xtra\font
copy lib\xtra\graf\delete.me unangband-061-src\lib\xtra\graf
copy lib\xtra\music\delete.me unangband-061-src\lib\xtra\music

copy readme.txt unangband-061-src
copy changes.txt unangband-061-src

copy lib\edit\*.txt unangband-061-src\lib\edit

copy lib\file\*.txt unangband-061-src\lib\file

copy lib\help\*.txt unangband-061-src\lib\help
copy lib\help\*.hlp unangband-061-src\lib\help

copy lib\pref\*.prf unangband-061-src\lib\pref

copy lib\xtra\sound\sound.cfg unangband-061-src\lib\xtra\sound

copy lib\xtra\help\angband.hlp unangband-061-src\lib\xtra\help
copy lib\xtra\help\angband.cnt unangband-061-src\lib\xtra\help

copy src\*.h unangband-061-src\src
copy src\*.c unangband-061-src\src
copy src\Makefile.* unangband-061-src\src

7z a -tzip -r unangband-061-src.zip unangband-061-src

rmdir /q /s unangband-061-src

