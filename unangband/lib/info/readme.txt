This readme contains information about how to populate this directory with additional
help tips. Please feel free to contribute to the context sensitive information held
here if you found the existing stuff useful.

In order to generate a context sensitive help file, the Unangband program will check
this directory for files of a specific format and display them using the built-in
help browser.

The names of the files are used to determine which file is displayed in which
context. The following file names are currently supported:

kindnnn.txt    The player has recently become aware of an object of kind nnn.
               See the object.txt file for which nnn corresponds to which object.
               
tvalnnn.txt    The player has recently become aware of the first object of tval nnn.
			   See the object.txt file for which tvals are used by which object.

tvalaa-bb.txt  The player has recently become aware of the bbth object of tval aa.
			   See the object.txt file for which tvals are used by which object.

egonnn.txt     The player has recently become aware of the first ego item of nnn.
			   See the egoitem.txt file for which tvals are used by which object.

artnnn.txt     The player has recently become aware of the first artifact of nnn.
			   See the artifact.txt file for which tvals are used by which object.

looknnn.txt    The player has recently become aware of the first creature of race nnn.
               See the monster.txt file for which nnn corresponds to which object.
               
killnnn.txt    The player has recently killed the first creature of race nnn.
               See the monster.txt file for which nnn corresponds to which object.
               
rflagnnn.txt   The player has recently become aware of the race flag nnn for the
               first time.
               See defines.h for the order that race flags are given, and use
               rfx_flag to determine 32 * (x-1) + y = nnn. (Not yet implemented)

oflagnnn.txt   The player has recently become aware of the object flag nnn for the
               first time.
               See defines.h for the order that race flags are given, and use
               trx_flag to determine 32 * (x-1) + y = nnn. (Not yet implemented)

birthnn.txt    The player has just been born. These tips are shown every time in nn
               order, but only for beginners.

levelnn.txt    The player has reached level nn for the first time this game.

depthaa-bb.txt The player has reached depth bb in dungeon aa for the first time this
               game.

racenn.txt     The player has just been born as race nn. See p_race.txt for which nn
			   corresponds to which race.

raceaa-bb.txt  The player has reached level bb as race aa for the first time this game.

classnn.txt    The player has just been born as class nn. See p_class.txt for which nn
			   corresponds to which class.
			   
classaa-bb.txt The player has reached level bb as class aa for the first time this game.

stylenn.txt    The player has just been born as style nn.  See defines.h for which nn
               corresponds to which style.

wsaa-bb-cc.txt The player has reached level cc as class aa, style bb for the first
               time this game.
