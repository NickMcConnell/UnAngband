mkdir angband
mkdir angband\lib
mkdir angband\lib\apex
mkdir angband\lib\bone
mkdir angband\lib\data
mkdir angband\lib\edit
mkdir angband\lib\file
mkdir angband\lib\help
mkdir angband\lib\info
mkdir angband\lib\pref
mkdir angband\lib\save
mkdir angband\lib\user
mkdir angband\lib\xtra
mkdir angband\lib\xtra\font
mkdir angband\lib\xtra\graf
mkdir angband\lib\xtra\music
mkdir angband\lib\xtra\sound
mkdir angband\lib\xtra\help
mkdir angband\src

copy lib\apex\delete.me angband\lib\apex
copy lib\bone\delete.me angband\lib\bone
copy lib\data\delete.me angband\lib\data
copy lib\info\delete.me angband\lib\info
copy lib\save\delete.me angband\lib\save
copy lib\user\delete.me angband\lib\user
copy lib\xtra\font\delete.me angband\lib\xtra\font
copy lib\xtra\graf\delete.me angband\lib\xtra\graf
copy lib\xtra\music\delete.me angband\lib\xtra\music

copy readme.txt angband
copy changes.txt angband

copy lib\edit\*.txt angband\lib\edit

copy lib\file\*.txt angband\lib\file

copy lib\help\*.txt angband\lib\help
copy lib\help\*.hlp angband\lib\help

copy lib\pref\*.prf angband\lib\pref

copy lib\xtra\sound\sound.cfg angband\lib\xtra\sound

copy lib\xtra\help\angband.hlp angband\lib\xtra\help
copy lib\xtra\help\angband.cnt angband\lib\xtra\help

copy src\*.h angband\src
copy src\*.c angband\src
copy src\Makefile.* angband\src

7z a -tzip -r unangband-061-src.zip angband

rmdir /q /s angband

