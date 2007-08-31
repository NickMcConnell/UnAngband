mkdir unangband-062-src
mkdir unangband-062-src\lib
mkdir unangband-062-src\lib\apex
mkdir unangband-062-src\lib\bone
mkdir unangband-062-src\lib\data
mkdir unangband-062-src\lib\edit
mkdir unangband-062-src\lib\file
mkdir unangband-062-src\lib\help
mkdir unangband-062-src\lib\info
mkdir unangband-062-src\lib\pref
mkdir unangband-062-src\lib\save
mkdir unangband-062-src\lib\user
mkdir unangband-062-src\lib\xtra
mkdir unangband-062-src\lib\xtra\font
mkdir unangband-062-src\lib\xtra\graf
mkdir unangband-062-src\lib\xtra\music
mkdir unangband-062-src\lib\xtra\sound
mkdir unangband-062-src\lib\xtra\help
mkdir unangband-062-src\src

copy lib\apex\delete.me unangband-062-src\lib\apex
copy lib\bone\delete.me unangband-062-src\lib\bone
copy lib\data\delete.me unangband-062-src\lib\data
copy lib\save\delete.me unangband-062-src\lib\save
copy lib\user\delete.me unangband-062-src\lib\user
copy lib\xtra\font\delete.me unangband-062-src\lib\xtra\font
copy lib\xtra\graf\delete.me unangband-062-src\lib\xtra\graf
copy lib\xtra\music\delete.me unangband-062-src\lib\xtra\music

copy readme.txt unangband-062-src
copy changes.txt unangband-062-src

copy lib\edit\*.txt unangband-062-src\lib\edit

copy lib\file\*.txt unangband-062-src\lib\file
copy lib\info\*.txt unangband-062-src\lib\info

copy lib\help\*.txt unangband-062-src\lib\help
copy lib\help\*.hlp unangband-062-src\lib\help

copy lib\pref\*.prf unangband-062-src\lib\pref

copy lib\xtra\sound\sound.cfg unangband-062-src\lib\xtra\sound

copy lib\xtra\help\angband.hlp unangband-062-src\lib\xtra\help
copy lib\xtra\help\angband.cnt unangband-062-src\lib\xtra\help

copy src\*.h unangband-062-src\src
copy src\*.c unangband-062-src\src
copy src\*.inc unangband-062-src\src
copy src\*.rc unangband-062-src\src
copy src\*.ico unangband-062-src\src
copy src\Makefile.* unangband-062-src\src

7z a -tzip -r unangband-062-src.zip unangband-062-src

rmdir /q /s unangband-062-src

