/* File: randart.c */


/*
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"

#include "init.h"

#define LOG_PRINT(string) \
	do { if (randart_verbose) \
		fprintf(randart_log, (string)); \
	} while (0);

#define LOG_PRINT1(string, value) \
	do { if (randart_verbose) \
		fprintf(randart_log, (string), (int)(value)); \
	} while (0);

#define LOG_PRINT2(string, val1, val2) \
	do { if (randart_verbose) \
		fprintf(randart_log, (string), (int)(val1), (int)(val2)); \
	} while (0);

/*
 * Original random artifact generator (randart) by Greg Wooledge.
 * Updated by Chris Carr / Chris Robertson in 2001.
 *
 * The external "names.txt" file was sucked into this file for simplicity.
 */

#ifdef GJW_RANDART

static cptr names_list =
"adanedhel\n"
"adurant\n"
"aeglos\n"
"aegnor\n"
"aelin\n"
"aeluin\n"
"aerandir\n"
"aerin\n"
"agarwaen\n"
"aglareb\n"
"aglarond\n"
"aglon\n"
"ainulindale\n"
"ainur\n"
"alcarinque\n"
"aldaron\n"
"aldudenie\n"
"almaren\n"
"alqualonde\n"
"aman\n"
"amandil\n"
"amarie\n"
"amarth\n"
"amlach\n"
"amon\n"
"amras\n"
"amrod\n"
"anach\n"
"anar\n"
"anarion\n"
"ancalagon\n"
"ancalimon\n"
"anarrima\n"
"andor\n"
"andram\n"
"androth\n"
"anduin\n"
"andunie\n"
"anfauglir\n"
"anfauglith\n"
"angainor\n"
"angband\n"
"anghabar\n"
"anglachel\n"
"angrenost\n"
"angrim\n"
"angrist\n"
"angrod\n"
"anguirel\n"
"annael\n"
"annatar\n"
"annon\n"
"annuminas\n"
"apanonar\n"
"aradan\n"
"aragorn\n"
"araman\n"
"aranel\n"
"aranruth\n"
"aranwe\n"
"aras\n"
"aratan\n"
"aratar\n"
"arathorn\n"
"arda\n"
"ard-galen\n"
"aredhel\n"
"ar-feiniel\n"
"argonath\n"
"arien\n"
"armenelos\n"
"arminas\n"
"arnor\n"
"aros\n"
"arossiach\n"
"arthad\n"
"arvernien\n"
"arwen\n"
"ascar\n"
"astaldo\n"
"atalante\n"
"atanamir\n"
"atanatari\n"
"atani\n"
"aule\n"
"avallone\n"
"avari\n"
"avathar\n"
"balan\n"
"balar\n"
"balrog\n"
"barad\n"
"baragund\n"
"barahir\n"
"baran\n"
"baranduin\n"
"bar\n"
"bauglir\n"
"beleg\n"
"belegaer\n"
"belegost\n"
"belegund\n"
"beleriand\n"
"belfalas\n"
"belthil\n"
"belthronding\n"
"beor\n"
"beraid\n"
"bereg\n"
"beren\n"
"boromir\n"
"boron\n"
"bragollach\n"
"brandir\n"
"bregolas\n"
"bregor\n"
"brethil\n"
"brilthor\n"
"brithiach\n"
"brithombar\n"
"brithon\n"
"cabed\n"
"calacirya\n"
"calaquendi\n"
"calenardhon\n"
"calion\n"
"camlost\n"
"caragdur\n"
"caranthir\n"
"carcharoth\n"
"cardolan\n"
"carnil\n"
"celeborn\n"
"celebrant\n"
"celebrimbor\n"
"celebrindal\n"
"celebros\n"
"celegorm\n"
"celon\n"
"cirdan\n"
"cirith\n"
"cirth\n"
"ciryatan\n"
"ciryon\n"
"coimas\n"
"corollaire\n"
"crissaegrim\n"
"cuarthal\n"
"cuivienen\n"
"culurien\n"
"curufin\n"
"curufinwe\n"
"curunir\n"
"cuthalion\n"
"daedeloth\n"
"daeron\n"
"dagnir\n"
"dagor\n"
"dagorlad\n"
"dairuin\n"
"danwedh\n"
"delduwath\n"
"denethor\n"
"dimbar\n"
"dimrost\n"
"dinen\n"
"dior\n"
"dirnen\n"
"dolmed\n"
"doriath\n"
"dorlas\n"
"dorthonion\n"
"draugluin\n"
"drengist\n"
"duath\n"
"duinath\n"
"duilwen\n"
"dunedain\n"
"dungortheb\n"
"earendil\n"
"earendur\n"
"earnil\n"
"earnur\n"
"earrame\n"
"earwen\n"
"echor\n"
"echoriath\n"
"ecthelion\n"
"edain\n"
"edrahil\n"
"eglador\n"
"eglarest\n"
"eglath\n"
"eilinel\n"
"eithel\n"
"ekkaia\n"
"elbereth\n"
"eldalie\n"
"eldalieva\n"
"eldamar\n"
"eldar\n"
"eledhwen\n"
"elemmire\n"
"elende\n"
"elendil\n"
"elendur\n"
"elenna\n"
"elentari\n"
"elenwe\n"
"elerrina\n"
"elleth\n"
"elmoth\n"
"elostirion\n"
"elrond\n"
"elros\n"
"elu\n"
"eluchil\n"
"elured\n"
"elurin\n"
"elwe\n"
"elwing\n"
"emeldir\n"
"endor\n"
"engrin\n"
"engwar\n"
"eol\n"
"eonwe\n"
"ephel\n"
"erchamion\n"
"ereb\n"
"ered\n"
"erech\n"
"eregion\n"
"ereinion\n"
"erellont\n"
"eressea\n"
"eriador\n"
"eru\n"
"esgalduin\n"
"este\n"
"estel\n"
"estolad\n"
"ethir\n"
"ezellohar\n"
"faelivrin\n"
"falas\n"
"falathar\n"
"falathrim\n"
"falmari\n"
"faroth\n"
"fauglith\n"
"feanor\n"
"feanturi\n"
"felagund\n"
"finarfin\n"
"finduilas\n"
"fingolfin\n"
"fingon\n"
"finwe\n"
"firimar\n"
"formenos\n"
"fornost\n"
"frodo\n"
"fuin\n"
"fuinur\n"
"gabilgathol\n"
"galad\n"
"galadriel\n"
"galathilion\n"
"galdor\n"
"galen\n"
"galvorn\n"
"gandalf\n"
"gaurhoth\n"
"gelion\n"
"gelmir\n"
"gelydh\n"
"gil\n"
"gildor\n"
"giliath\n"
"ginglith\n"
"girith\n"
"glaurung\n"
"glingal\n"
"glirhuin\n"
"gloredhel\n"
"glorfindel\n"
"golodhrim\n"
"gondolin\n"
"gondor\n"
"gonnhirrim\n"
"gorgoroth\n"
"gorlim\n"
"gorthaur\n"
"gorthol\n"
"gothmog\n"
"guilin\n"
"guinar\n"
"guldur\n"
"gundor\n"
"gurthang\n"
"gwaith\n"
"gwareth\n"
"gwindor\n"
"hadhodrond\n"
"hador\n"
"haladin\n"
"haldad\n"
"haldan\n"
"haldar\n"
"haldir\n"
"haleth\n"
"halmir\n"
"handir\n"
"harad\n"
"hareth\n"
"hathaldir\n"
"hathol\n"
"haudh\n"
"helcar\n"
"helcaraxe\n"
"helevorn\n"
"helluin\n"
"herumor\n"
"herunumen\n"
"hildorien\n"
"himlad\n"
"himring\n"
"hirilorn\n"
"hisilome\n"
"hithaeglir\n"
"hithlum\n"
"hollin\n"
"huan\n"
"hunthor\n"
"huor\n"
"hurin\n"
"hyarmendacil\n"
"hyarmentir\n"
"iant\n"
"iaur\n"
"ibun\n"
"idril\n"
"illuin\n"
"ilmare\n"
"ilmen\n"
"iluvatar\n"
"imlach\n"
"imladris\n"
"indis\n"
"ingwe\n"
"irmo\n"
"isil\n"
"isildur\n"
"istari\n"
"ithil\n"
"ivrin\n"
"kelvar\n"
"kementari\n"
"ladros\n"
"laiquendi\n"
"lalaith\n"
"lamath\n"
"lammoth\n"
"lanthir\n"
"laurelin\n"
"leithian\n"
"legolin\n"
"lembas\n"
"lenwe\n"
"linaewen\n"
"lindon\n"
"lindorie\n"
"loeg\n"
"lomelindi\n"
"lomin\n"
"lomion\n"
"lorellin\n"
"lorien\n"
"lorindol\n"
"losgar\n"
"lothlann\n"
"lothlorien\n"
"luin\n"
"luinil\n"
"lumbar\n"
"luthien\n"
"mablung\n"
"maedhros\n"
"maeglin\n"
"maglor\n"
"magor\n"
"mahanaxar\n"
"mahtan\n"
"maiar\n"
"malduin\n"
"malinalda\n"
"mandos\n"
"manwe\n"
"mardil\n"
"melian\n"
"melkor\n"
"menegroth\n"
"meneldil\n"
"menelmacar\n"
"meneltarma\n"
"minas\n"
"minastir\n"
"mindeb\n"
"mindolluin\n"
"mindon\n"
"minyatur\n"
"mirdain\n"
"miriel\n"
"mithlond\n"
"mithrandir\n"
"mithrim\n"
"mordor\n"
"morgoth\n"
"morgul\n"
"moria\n"
"moriquendi\n"
"mormegil\n"
"morwen\n"
"nahar\n"
"naeramarth\n"
"namo\n"
"nandor\n"
"nargothrond\n"
"narog\n"
"narsil\n"
"narsilion\n"
"narya\n"
"nauglamir\n"
"naugrim\n"
"ndengin\n"
"neithan\n"
"neldoreth\n"
"nenar\n"
"nenning\n"
"nenuial\n"
"nenya\n"
"nerdanel\n"
"nessa\n"
"nevrast\n"
"nibin\n"
"nienna\n"
"nienor\n"
"nimbrethil\n"
"nimloth\n"
"nimphelos\n"
"nimrais\n"
"nimras\n"
"ningloron\n"
"niniel\n"
"ninniach\n"
"ninquelote\n"
"niphredil\n"
"nirnaeth\n"
"nivrim\n"
"noegyth\n"
"nogrod\n"
"noldolante\n"
"noldor\n"
"numenor\n"
"nurtale\n"
"obel\n"
"ohtar\n"
"oiolosse\n"
"oiomure\n"
"olorin\n"
"olvar\n"
"olwe\n"
"ondolinde\n"
"orfalch\n"
"ormal\n"
"orocarni\n"
"orodreth\n"
"orodruin\n"
"orome\n"
"oromet\n"
"orthanc\n"
"osgiliath\n"
"osse\n"
"ossiriand\n"
"palantir\n"
"pelargir\n"
"pelori\n"
"periannath\n"
"quendi\n"
"quenta\n"
"quenya\n"
"radagast\n"
"radhruin\n"
"ragnor\n"
"ramdal\n"
"rana\n"
"rathloriel\n"
"rauros\n"
"region\n"
"rerir\n"
"rhovanion\n"
"rhudaur\n"
"rhun\n"
"rhunen\n"
"rian\n"
"ringil\n"
"ringwil\n"
"romenna\n"
"rudh\n"
"rumil\n"
"saeros\n"
"salmar\n"
"saruman\n"
"sauron\n"
"serech\n"
"seregon\n"
"serinde\n"
"shelob\n"
"silmarien\n"
"silmaril\n"
"silpion\n"
"sindar\n"
"singollo\n"
"sirion\n"
"soronume\n"
"sul\n"
"sulimo\n"
"talath\n"
"taniquetil\n"
"tar\n"
"taras\n"
"tarn\n"
"tathren\n"
"taur\n"
"tauron\n"
"teiglin\n"
"telchar\n"
"telemnar\n"
"teleri\n"
"telperion\n"
"telumendil\n"
"thalion\n"
"thalos\n"
"thangorodrim\n"
"thargelion\n"
"thingol\n"
"thoronath\n"
"thorondor\n"
"thranduil\n"
"thuringwethil\n"
"tilion\n"
"tintalle\n"
"tinuviel\n"
"tirion\n"
"tirith\n"
"tol\n"
"tulkas\n"
"tumhalad\n"
"tumladen\n"
"tuna\n"
"tuor\n"
"turambar\n"
"turgon\n"
"turin\n"
"uial\n"
"uilos\n"
"uinen\n"
"ulairi\n"
"ulmo\n"
"ulumuri\n"
"umanyar\n"
"umarth\n"
"umbar\n"
"ungoliant\n"
"urthel\n"
"uruloki\n"
"utumno\n"
"vaire\n"
"valacirca\n"
"valandil\n"
"valaquenta\n"
"valar\n"
"valaraukar\n"
"valaroma\n"
"valier\n"
"valimar\n"
"valinor\n"
"valinoreva\n"
"valmar\n"
"vana\n"
"vanyar\n"
"varda\n"
"vasa\n"
"vilya\n"
"vingilot\n"
"vinyamar\n"
"voronwe\n"
"wethrin\n"
"wilwarin\n"
"yavanna\n"
;

#define MAX_TRIES 200
#define BUFLEN 1024

#define MIN_NAME_LEN 5
#define MAX_NAME_LEN 9
#define S_WORD 26
#define E_WORD S_WORD

#define sign(x)	((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))

/* Total number of different slay types used */

#define SLAY_MAX 0x00200000L

/*
 * Average damage for good ego ammo of various types, used for balance
 * The current values assume normal (non-seeker) ammo enchanted to +9
 */

#define AVG_SLING_AMMO_DAMAGE 11
#define AVG_BOW_AMMO_DAMAGE 12
#define AVG_XBOW_AMMO_DAMAGE 12

/* Inhibiting factors for large bonus values */
#define INHIBIT_STRONG 6
#define INHIBIT_WEAK 2

/*
 * Numerical index values for the different learned probabilities
 * These are to make the code more readable.
 */

#define ART_IDX_BOW_SHOTS 0
#define ART_IDX_BOW_MIGHT 1
#define ART_IDX_WEAPON_HIT 2
#define ART_IDX_WEAPON_DAM 3
#define ART_IDX_NONWEAPON_HIT 4
#define ART_IDX_NONWEAPON_DAM 5

#define ART_IDX_MELEE_BLESS 6
#define ART_IDX_MELEE_BRAND_SLAY 7
#define ART_IDX_MELEE_SINV 8
#define ART_IDX_MELEE_BLOWS 9
#define ART_IDX_MELEE_AC 10
#define ART_IDX_MELEE_DICE 11
#define ART_IDX_MELEE_WEIGHT 12
#define ART_IDX_MELEE_TUNN 13

#define ART_IDX_ALLARMOR_WEIGHT 14

#define ART_IDX_BOOT_AC 15
#define ART_IDX_BOOT_FEATHER 16
#define ART_IDX_BOOT_STEALTH 17
#define ART_IDX_BOOT_SPEED 18

#define ART_IDX_GLOVE_AC 19
#define ART_IDX_GLOVE_FA 20
#define ART_IDX_GLOVE_DEX 21

#define ART_IDX_HELM_AC 22
#define ART_IDX_HELM_RBLIND 23
#define ART_IDX_HELM_ESP 24
#define ART_IDX_HELM_SINV 25
#define ART_IDX_HELM_WIS 26
#define ART_IDX_HELM_INT 27

#define ART_IDX_SHIELD_AC 28
#define ART_IDX_SHIELD_LRES 29

#define ART_IDX_CLOAK_AC 30
#define ART_IDX_CLOAK_STEALTH 31

#define ART_IDX_ARMOR_AC 32
#define ART_IDX_ARMOR_STEALTH 33
#define ART_IDX_ARMOR_HLIFE 34
#define ART_IDX_ARMOR_CON 35
#define ART_IDX_ARMOR_LRES 36
#define ART_IDX_ARMOR_ALLRES 37
#define ART_IDX_ARMOR_HRES 38

#define ART_IDX_GEN_STAT 39
#define ART_IDX_GEN_SUST 40
#define ART_IDX_GEN_STEALTH 41
#define ART_IDX_GEN_SEARCH 42
#define ART_IDX_GEN_INFRA 43
#define ART_IDX_GEN_SPEED 44
#define ART_IDX_GEN_IMMUNE 45
#define ART_IDX_GEN_FA 46
#define ART_IDX_GEN_HLIFE 47
#define ART_IDX_GEN_FEATHER 48
#define ART_IDX_GEN_LITE 49
#define ART_IDX_GEN_SINV 50
#define ART_IDX_GEN_ESP 51
#define ART_IDX_GEN_SDIG 52
#define ART_IDX_GEN_REGEN 53
#define ART_IDX_GEN_LRES 54
#define ART_IDX_GEN_RPOIS 55
#define ART_IDX_GEN_RFEAR 56
#define ART_IDX_GEN_RLITE 57
#define ART_IDX_GEN_RDARK 58
#define ART_IDX_GEN_RBLIND 59
#define ART_IDX_GEN_RCONF 60
#define ART_IDX_GEN_RSOUND 61
#define ART_IDX_GEN_RSHARD 62
#define ART_IDX_GEN_RNEXUS 63
#define ART_IDX_GEN_RNETHER 64
#define ART_IDX_GEN_RCHAOS 65
#define ART_IDX_GEN_RDISEN 66
#define ART_IDX_GEN_AC 67
#define ART_IDX_GEN_TUNN 68

/* Supercharged abilities - treated differently in algorithm */

#define ART_IDX_MELEE_DICE_SUPER 69
#define ART_IDX_BOW_SHOTS_SUPER 70
#define ART_IDX_BOW_MIGHT_SUPER 71
#define ART_IDX_GEN_SPEED_SUPER 72

/* Aggravation - weapon and nonweapon */
#define ART_IDX_WEAPON_AGGR 73
#define ART_IDX_NONWEAPON_AGGR 74

/* Start of ESP_IDX defines ARD_ESP */
#define ART_IDX_MELEE_SENSE 75
#define ART_IDX_BOW_SENSE 76
#define ART_IDX_NONWEAPON_SENSE 77

#define ART_IDX_MELEE_RESTRICT 78
#define ART_IDX_BOW_RESTRICT 79
#define ART_IDX_NONWEAPON_RESTRICT 80

#define ART_IDX_GEN_SAVE 81
#define ART_IDX_GEN_DEVICE 82

/* Total of abilities */
#define ART_IDX_TOTAL 83

/* End of ESP_IDX defines ARD_ESP */

/* Tallies of different ability types */
#define ART_IDX_BOW_COUNT 3
#define ART_IDX_WEAPON_COUNT 3
#define ART_IDX_NONWEAPON_COUNT 4
#define ART_IDX_MELEE_COUNT 9
#define ART_IDX_ALLARMOR_COUNT 1
#define ART_IDX_BOOT_COUNT 4
#define ART_IDX_GLOVE_COUNT 3
#define ART_IDX_HELM_COUNT 6
#define ART_IDX_SHIELD_COUNT 2
#define ART_IDX_CLOAK_COUNT 2
#define ART_IDX_ARMOR_COUNT 7
#define ART_IDX_GEN_COUNT 30
#define ART_IDX_HIGH_RESIST_COUNT 12

/* Arrays of indices by item type, used in frequency generation */

/* ARD_ESP -- following changed to include ARD_IDX_BOW_SENSE */
static s16b art_idx_bow[] =
	{ART_IDX_BOW_SHOTS, ART_IDX_BOW_MIGHT, ART_IDX_BOW_SENSE, ART_IDX_BOW_RESTRICT};
static s16b art_idx_weapon[] =
	{ART_IDX_WEAPON_HIT, ART_IDX_WEAPON_DAM, ART_IDX_WEAPON_AGGR};
/* ARD_ESP -- following changed to include ARD_IDX_NONWEAPON_SENSE */
static s16b art_idx_nonweapon[] =
	{ART_IDX_NONWEAPON_HIT, ART_IDX_NONWEAPON_DAM, ART_IDX_NONWEAPON_AGGR,
	ART_IDX_NONWEAPON_SENSE, ART_IDX_NONWEAPON_RESTRICT};
/* ARD_ESP -- following changed to include ARD_IDX_MELEE_SENSE */
static s16b art_idx_melee[] =
	{ART_IDX_MELEE_BLESS, ART_IDX_MELEE_BRAND_SLAY, ART_IDX_MELEE_SINV,
	ART_IDX_MELEE_BLOWS, ART_IDX_MELEE_AC, ART_IDX_MELEE_DICE,
	ART_IDX_MELEE_WEIGHT, ART_IDX_MELEE_TUNN, ART_IDX_MELEE_SENSE, ART_IDX_MELEE_RESTRICT};
static s16b art_idx_allarmor[] =
	{ART_IDX_ALLARMOR_WEIGHT};
static s16b art_idx_boot[] =
	{ART_IDX_BOOT_AC, ART_IDX_BOOT_FEATHER, ART_IDX_BOOT_STEALTH, ART_IDX_BOOT_SPEED};
static s16b art_idx_glove[] =
	{ART_IDX_GLOVE_AC, ART_IDX_GLOVE_FA, ART_IDX_GLOVE_DEX};
static s16b art_idx_headgear[] =
	{ART_IDX_HELM_AC, ART_IDX_HELM_RBLIND, ART_IDX_HELM_ESP, ART_IDX_HELM_SINV,
	ART_IDX_HELM_WIS, ART_IDX_HELM_INT};
static s16b art_idx_shield[] =
	{ART_IDX_SHIELD_AC, ART_IDX_SHIELD_LRES};
static s16b art_idx_cloak[] =
	{ART_IDX_CLOAK_AC, ART_IDX_CLOAK_STEALTH};
static s16b art_idx_armor[] =
	{ART_IDX_ARMOR_AC, ART_IDX_ARMOR_STEALTH, ART_IDX_ARMOR_HLIFE, ART_IDX_ARMOR_CON,
	ART_IDX_ARMOR_LRES, ART_IDX_ARMOR_ALLRES, ART_IDX_ARMOR_HRES};
static s16b art_idx_gen[] =
	{ART_IDX_GEN_STAT, ART_IDX_GEN_SUST, ART_IDX_GEN_STEALTH,
	ART_IDX_GEN_SEARCH, ART_IDX_GEN_INFRA, ART_IDX_GEN_SPEED,
	ART_IDX_GEN_IMMUNE, ART_IDX_GEN_FA, ART_IDX_GEN_HLIFE,
	ART_IDX_GEN_FEATHER, ART_IDX_GEN_LITE, ART_IDX_GEN_SINV,
	ART_IDX_GEN_ESP, ART_IDX_GEN_SDIG, ART_IDX_GEN_REGEN,
	ART_IDX_GEN_LRES, ART_IDX_GEN_RPOIS, ART_IDX_GEN_RFEAR,
	ART_IDX_GEN_RLITE, ART_IDX_GEN_RDARK, ART_IDX_GEN_RBLIND,
	ART_IDX_GEN_RCONF, ART_IDX_GEN_RSOUND, ART_IDX_GEN_RSHARD,
	ART_IDX_GEN_RNEXUS, ART_IDX_GEN_RNETHER, ART_IDX_GEN_RCHAOS,
	ART_IDX_GEN_RDISEN, ART_IDX_GEN_AC, ART_IDX_GEN_TUNN,
	ART_IDX_GEN_SAVE, ART_IDX_GEN_DEVICE};
static s16b art_idx_high_resist[] =
	{ART_IDX_GEN_RPOIS, ART_IDX_GEN_RFEAR,
	ART_IDX_GEN_RLITE, ART_IDX_GEN_RDARK, ART_IDX_GEN_RBLIND,
	ART_IDX_GEN_RCONF, ART_IDX_GEN_RSOUND, ART_IDX_GEN_RSHARD,
	ART_IDX_GEN_RNEXUS, ART_IDX_GEN_RNETHER, ART_IDX_GEN_RCHAOS,
	ART_IDX_GEN_RDISEN};


/* Initialize the data structures for learned probabilities */

static unsigned short lprobs[S_WORD+1][S_WORD+1][S_WORD+1];
static unsigned short ltotal[S_WORD+1][S_WORD+1];
static s16b artprobs[ART_IDX_TOTAL];
static s16b art_bow_total = 0;
static s16b art_melee_total = 0;
static s16b art_boot_total = 0;
static s16b art_glove_total = 0;
static s16b art_headgear_total = 0;
static s16b art_shield_total = 0;
static s16b art_cloak_total = 0;
static s16b art_armor_total = 0;
static s16b art_other_total = 0;
static s16b art_total = 0;

/*
 * Working array for holding frequency values - global to avoid repeated
 * allocation of memory
 */

static s16b art_freq[ART_IDX_TOTAL];
/*
 * Mean start and increment values for to_hit, to_dam and AC.  Update these
 * if the algorithm changes.  They are used in frequency generation.
 */

static s16b mean_hit_increment = 3;
static s16b mean_dam_increment = 3;
static s16b mean_hit_startval = 8;
static s16b mean_dam_startval = 8;
static s16b mean_ac_startval = 15;
static s16b mean_ac_increment = 5;

/*
 * Pointer for logging file
 */

static FILE *randart_log;

/*
 * Cache the results of lookup_kind(), which is expensive and would
 * otherwise be called much too often.
 */
static s16b *kinds;

/*
 * Store the original artifact power ratings
 */
static s32b *base_power;

/*
 * Store the original base item levels
 */
static byte *base_item_level;

/*
 * Store the original base item rarities
 */
static byte *base_item_rarity;

/*
 * Store the original artifact rarities
 */
static byte *base_art_rarity;

/*
 * Cache the results of slay_value(), which is expensive and would
 * otherwise be called much too often.
 */
static s32b *slays;

/*
 * Store the power ratings for the various monsters
 */

static long *mon_power;

/* Global just for convenience. */
static int randart_verbose = 1;


/*ARD_RAND - Extra global variable.
 * Used to create random artifacts along side existing artifacts.
 *
 * a_max is the old z_info->a_max.
 */
byte a_max;

/*
 * Use W. Sheldon Simms' random name generator.  This function builds
 * probability tables which are used later on for letter selection.  It
 * relies on the ASCII character set.
 */
static void build_prob(cptr learn)
{
	int c_prev, c_cur, c_next;

	/* Build raw frequencies */
	do
	{
		c_prev = c_cur = S_WORD;

		do
		{
			c_next = *learn++;
		} while (!isalpha(c_next) && (c_next != '\0'));

		if (c_next == '\0') break;

		do
		{
			c_next = A2I(tolower(c_next));
			lprobs[c_prev][c_cur][c_next]++;
			ltotal[c_prev][c_cur]++;
			c_prev = c_cur;
			c_cur = c_next;
			c_next = *learn++;
		} while (isalpha(c_next));

		lprobs[c_prev][c_cur][E_WORD]++;
		ltotal[c_prev][c_cur]++;
	}
	while (c_next != '\0');
}


/*
 * Use W. Sheldon Simms' random name generator.  Generate a random word using
 * the probability tables we built earlier.  Relies on the ASCII character
 * set.  Relies on European vowels (a, e, i, o, u).  The generated name should
 * be copied/used before calling this function again.
 */
static char *make_word(void)
{
	static char word_buf[90];
	int r, totalfreq;
	int tries, lnum, vow;
	int c_prev, c_cur, c_next;
	char *cp;

startover:
	vow = 0;
	lnum = 0;
	tries = 0;
	cp = word_buf;
	c_prev = c_cur = S_WORD;

	while (1)
	{
	    getletter:
		c_next = 0;
		r = rand_int(ltotal[c_prev][c_cur]);
		totalfreq = lprobs[c_prev][c_cur][c_next];

		while (totalfreq <= r)
		{
			c_next++;
			totalfreq += lprobs[c_prev][c_cur][c_next];
		}

		if (c_next == E_WORD)
		{
			if ((lnum < MIN_NAME_LEN) || vow == 0)
			{
				tries++;
				if (tries < 10) goto getletter;
				goto startover;
			}
			*cp = '\0';
			break;
		}

		if (lnum >= MAX_NAME_LEN) goto startover;

		*cp = I2A(c_next);

		if (is_a_vowel(*cp)) vow++;

		cp++;
		lnum++;
		c_prev = c_cur;
		c_cur = c_next;
	}

	word_buf[0] = toupper(word_buf[0]);

	return (word_buf);
}


/*
 * Use W. Sheldon Simms' random name generator.
 */
static errr init_names(void)
{
	char buf[BUFLEN];
	size_t name_size;
	char *a_base;
	char *a_next;
	int i;

	/* Temporary space for names, while reading and randomizing them. */
	cptr *names;

	build_prob(names_list);

	/* Allocate the "names" array */
	/* ToDo: Make sure the memory is freed correctly in case of errors */
	C_MAKE(names, z_info->a_max, cptr);

	for (i = 1 ; i < z_info->a_max; i++)
	{
		char *word = make_word();

/* Start of ARD_RAND changes for artifact names */

		if (i == ART_POWER)
		{
			names[i-1] = string_make("of Power (The One Ring)");
			continue;
		}

		if (i == ART_GROND)
		{
			names[i-1] = string_make("'Grond'");
			continue;
		}

		if (i == ART_MORGOTH)
		{
			names[i-1] = string_make("of Morgoth");
			continue;
		}
		if ((!adult_rand_artifacts) && (i<a_max))
		{
			names[i-1] = string_make(a_name+a_info[i].name);

			continue;
		}
/* End ARD_RAND
 */

		if (rand_int(3) == 0)
			sprintf(buf, "'%s'", word);
		else
			sprintf(buf, "of %s", word);

		names[i-1] = string_make(buf);
	}

	/* Convert our names array into an a_name structure for later use. */
	name_size = 2;

	for (i = 1; i < z_info->a_max; i++)
	{
		name_size += strlen(names[i-1]) + 1;
	}

	C_MAKE(a_base, name_size, char);

	a_next = a_base + 1;	/* skip first char */

	for (i = 1; i < z_info->a_max; i++)
	{
		strcpy(a_next, names[i-1]);
		if (a_info[i].tval > 0)
		{
			a_info[i].name = a_next - a_base;
		}
		a_next += strlen(names[i-1]) + 1;
	}

	/* Free the old names */
	KILL(a_name);

	for (i = 1; i < z_info->a_max; i++)
	{
		string_free(names[i-1]);
	}

	/* Free the "names" array */
	KILL(names);

	/* Store the names */
	a_head.name_ptr = a_name = a_base;

	/* Success */
	return (0);
}

/*
 * Go through the attack types for this monster.
 * We look for the maximum possible maximum damage that this
 * monster can inflict in 10 game turns.  For melee
 * attacks we use the maximum damage assuming all attacks hit.
 * Spells are handled on a case by case basis.  For random
 * damage spells with multiple dice, we take the maximum.
 * For breaths we assume the monster has maximum HP.  In general
 * we assume all low and high resists - but assume maximum
 * resisted damage for the high resists.  Special spells like
 * summoning that don't cause damage are arbitrarily assigned a
 * 'fake' damage rating.
 *
 * To do: Add a factor for attack types that have side effects,
 * like confusion / blindness / nether / etc.
 */

static long eval_max_dam(int r_idx)
{
	int i, x;
	u32b dam = 1;
	u32b hp;
	u32b melee_dam, atk_dam, spell_dam;
	byte rlev;
	monster_race *r_ptr;
	u32b flag, breath_mask, attack_mask;
	u32b flag_counter;

	r_ptr = &r_info[r_idx];

	/*clear the counters*/
	melee_dam = atk_dam = spell_dam = 0;

	/* Evaluate average HP for this monster */
	if (r_ptr->flags1 & (RF1_FORCE_MAXHP)) hp = r_ptr->hdice * r_ptr->hside;
	else hp = r_ptr->hdice * (r_ptr->hside + 1) / 2;

	/* Extract the monster level, force 1 for town monsters */
	rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);

	for (x = 0; x < 4; x++)
	{

		/*Get the flags 4 monster flags and masks*/
		switch (x)
		{
			case 0:
			{
		 		flag = r_ptr->flags4;
				attack_mask = RF4_ATTACK_MASK;
				breath_mask = RF4_BREATH_MASK;
				break;
			}
			case 1:
			{
		 		flag = r_ptr->flags5;
				attack_mask = RF5_ATTACK_MASK;
				breath_mask = RF5_BREATH_MASK;
				break;
			}
			case 2:
			{
		 		flag = r_ptr->flags6;
				attack_mask = RF6_ATTACK_MASK;
				breath_mask = RF6_BREATH_MASK;
				break;
			}
			case 3:
			default:
			{
		 		flag = r_ptr->flags7;
				attack_mask = RF7_ATTACK_MASK;
				breath_mask = RF7_BREATH_MASK;
				break;
			}
		}

		/*no spells here, continue*/
		if (!flag) continue;

		flag_counter = 0x00000001;

		/* using 32 assumes a u32b flag size*/
		for (i = 0; i < 32; i++)
		{
			u16b this_dam = 0;

			/* First make sure monster has the flag*/
			if (flag & flag_counter)
			{
				/*Is it a breath? Should only be flag 4*/
				if (breath_mask & flag_counter)
				{
					int which_gf = 0;
					int mult = 1;
					int div_by = 1;

					/*hack - all breaths are in flag 4*/

					if (flag_counter == RF4_BRTH_ACID) 		which_gf = GF_ACID;
					else if (flag_counter == RF4_BRTH_ELEC) which_gf = GF_ELEC;
					else if (flag_counter == RF4_BRTH_FIRE) which_gf = GF_FIRE;
					else if (flag_counter == RF4_BRTH_COLD) which_gf = GF_COLD;
					else if (flag_counter == RF4_BRTH_POIS)
					{
						which_gf = GF_POIS;
						mult = 10;
						div_by = 9;
					}
					else if (flag_counter == RF4_BRTH_PLAS)
					{
						which_gf = GF_PLASMA;
						mult = 5;
						div_by = 4;
					}
					else if (flag_counter == RF4_BRTH_LITE)
					{
						which_gf = GF_LITE;
						mult = 5;
						div_by = 4;
					}
					else if (flag_counter == RF4_BRTH_DARK)
					{
						which_gf = GF_DARK;
						mult = 5;
						div_by = 4;
					}
					else if (flag_counter == RF4_BRTH_CONFU)
					{
						which_gf = GF_CONFUSION;
						mult = 4;
						div_by = 3;
					}
					else if (flag_counter == RF4_BRTH_SOUND)
					{
						which_gf = GF_SOUND;
						mult = 6;
						div_by = 5;
					}
					else if (flag_counter == RF4_BRTH_SHARD)
					{
						which_gf = GF_SHARD;
						mult = 8;
						div_by = 7;
					}
					else if (flag_counter == RF4_BRTH_INER)
					{
						which_gf = GF_INERTIA;
						mult = 3;
						div_by = 2;
					}
					else if (flag_counter == RF4_BRTH_GRAV)
					{
						which_gf = GF_GRAVITY;
						mult = 3;
						div_by = 2;
					}
					else if (flag_counter == RF4_BRTH_FORCE)
					{
						which_gf = GF_FORCE;
						mult = 6;
						div_by = 5;
					}
					else if (flag_counter == RF4_BRTH_NEXUS)
					{
						which_gf = GF_NEXUS;
						mult = 5;
						div_by = 4;
					}
					else if (flag_counter == RF4_BRTH_NETHR)
					{
						which_gf = GF_NETHER;
						mult = 5;
						div_by = 4;
					}
					else if (flag_counter == RF4_BRTH_CHAOS)
					{
						which_gf = GF_CHAOS;
						mult = 4;
						div_by = 3;
					}
					else if (flag_counter == RF4_BRTH_DISEN)
					{
						which_gf = GF_DISENCHANT;
						mult = 4;
						div_by = 3;
					}
					else if (flag_counter == RF4_BRTH_TIME)
					{
						which_gf = GF_TIME;
						mult = 3;
						div_by = 2;
					}
					else if (flag_counter == RF4_BRTH_MANA) which_gf = GF_MANA;

					if (which_gf)
					{
						this_dam = get_breath_dam(hp, which_gf,
									(r_ptr->flags2 & (RF2_POWERFUL) ? TRUE : FALSE));

						/* handle elemental breaths*/
						switch (which_gf)
					    {
							case GF_ACID:
							case GF_FIRE:
							case GF_COLD:
							case GF_ELEC:
							case GF_POIS:
							{
								/* Lets just pretend the player has the right base resist*/
								this_dam /= 3;

								break;
							}

							default: break;
						}

						this_dam = (this_dam * mult) / div_by;

						/*slight bonus for cloud_surround*/
						if (r_ptr->flags2 & RF2_HAS_AURA) this_dam = this_dam * 11 / 10;
					}
				}

				/*Is it an arrow, bolt, beam, or ball?*/
				else if (attack_mask & flag_counter)
				{
					switch (x)
					{
						case 0:
						{
							this_dam = r_ptr->spell_power * spell_info_RF4[i][COL_SPELL_DAM_MULT];
							this_dam /=  MAX(1, spell_info_RF4[i][COL_SPELL_DAM_DIV]);
							break;
						}
						case 1:
						{
							this_dam = r_ptr->spell_power * spell_info_RF5[i][COL_SPELL_DAM_MULT];
							this_dam /=  MAX(1, spell_info_RF5[i][COL_SPELL_DAM_DIV]);
							break;
						}
						case 2:
						{
							this_dam = r_ptr->spell_power * spell_info_RF6[i][COL_SPELL_DAM_MULT];
							this_dam /=  MAX(1, spell_info_RF6[i][COL_SPELL_DAM_DIV]);
							break;
						}
						case 3:
						{
							this_dam = r_ptr->spell_power * spell_info_RF7[i][COL_SPELL_DAM_MULT];
							this_dam /=  MAX(1, spell_info_RF7[i][COL_SPELL_DAM_DIV]);
							break;
						}
					}
				}

				else switch (x)
				{
					/*Misc flag4 flags*/
					case 0:
					{
						if (flag_counter == RF4_SHRIEK) this_dam = rlev / 2;
						break;
					}

					case 1:
					{
						/*Right now all flag5 are attack mask spells*/
						break;
					}

					case 2:
					{
						/*Misc flag6 flags*/
						if (flag_counter == RF6_ADD_MANA) this_dam = MAX(r_ptr->mana, 30);
						else if (flag_counter == RF6_BLINK) this_dam = rlev / 3;
						else if (flag_counter == RF6_TELE_SELF_TO) this_dam = rlev * 2;
						else if (flag_counter == RF6_TELE_TO) this_dam = rlev;
						else if (flag_counter == RF6_DARKNESS) this_dam = rlev;
						else if (flag_counter == RF6_TRAPS) this_dam = rlev;
						else if (flag_counter == RF6_FORGET) this_dam = rlev / 3;
						else if (flag_counter == RF6_DRAIN_MANA) this_dam = rlev * 2;
						else if (flag_counter == RF6_HUNGER) this_dam = rlev;
						else if (flag_counter == RF6_SCARE) this_dam = rlev;
						else if (flag_counter == RF6_BLIND) this_dam = rlev;
						else if (flag_counter == RF6_CONF) this_dam = rlev;
						else if (flag_counter == RF6_SLOW) this_dam = rlev;
						else if (flag_counter == RF6_HOLD) this_dam = 25;
						break;
					}
					/*All flag7 flags*/
					case 3:
					{
						/*Right now all flag7 are summon spells*/
						/* All summons are assigned arbitrary values according to their levels*/
						if 		(flag_counter == RF7_S_KIN) 	this_dam = rlev * 2;
						else if (flag_counter == RF7_S_MONSTER)	this_dam = rlev * 2 / 5;
						else if (flag_counter == RF7_S_MONSTERS)this_dam = rlev * 4 / 5;
						else if (flag_counter == RF7_S_ANT)		this_dam = rlev / 5;
						else if (flag_counter == RF7_S_SPIDER)	this_dam = rlev / 5;
						else if (flag_counter == RF7_S_HOUND)	this_dam = rlev;
						else if (flag_counter == RF7_S_ANIMAL)	this_dam = rlev / 2;
						else if (flag_counter == RF7_S_HYDRA)	this_dam = rlev * 3 / 2;
						else if (flag_counter == RF7_S_THIEF)	this_dam = rlev / 2;
						else if (flag_counter == RF7_S_BERTBILLTOM)	this_dam = rlev * 3 / 4;
						else if (flag_counter == RF7_S_MAIA)	this_dam = rlev * 3 / 2;
						else if (flag_counter == RF7_S_DRAGON)	this_dam = rlev * 3 / 2;
						else if (flag_counter == RF7_S_HI_DRAGON) this_dam = rlev * 4;
						else if (flag_counter == RF7_S_DEMON)	this_dam = rlev * 3 / 2;
						else if (flag_counter == RF7_S_HI_DEMON)this_dam = rlev * 3;
						else if (flag_counter == RF7_S_UNDEAD)	this_dam = rlev * 3 / 2;
						else if (flag_counter == RF7_S_HI_UNDEAD)this_dam = rlev * 4;
						else if (flag_counter == RF7_S_WRAITH)	this_dam = rlev * 9 / 2;
						else if (flag_counter == RF7_S_UNIQUE)	this_dam = rlev * 3;
						else if (flag_counter == RF7_S_HI_UNIQUE)	this_dam = rlev * 5;
						break;
					}
				}

			}

			if (this_dam > spell_dam) spell_dam = this_dam;

			/*shift one bit*/
			flag_counter = flag_counter << 1;
		}
	}

	/* Only do if it has attacks */
	if (!(r_ptr->flags1 & (RF1_NEVER_BLOW)))
	{
		for (i = 0; i < 4; i++)
		{
			/* Extract the attack infomation */
			int effect = r_ptr->blow[i].effect;
			int method = r_ptr->blow[i].method;
			int d_dice = r_ptr->blow[i].d_dice;
			int d_side = r_ptr->blow[i].d_side;

			/* Hack -- no more attacks */
			if (!method) continue;

			/* Assume average damage*/
			atk_dam = d_dice * (d_side + 1) / 2;

			switch (method)
			{
				/*possible stun*/
				case RBM_HIT:
				{
					if ((effect == GF_WOUND) || (effect == GF_BATTER))
					{
						atk_dam *= 5;
						atk_dam /= 4;
					}
					break;
				}
				/*stun definitely most dangerous*/
				case RBM_PUNCH:
				case RBM_KICK:
				case RBM_BUTT:
				case RBM_CRUSH:
				{
					atk_dam *= 4;
					atk_dam /= 3;
					break;
				}
				/*cut*/
				case RBM_CLAW:
				case RBM_BITE:
				case RBM_PECK:
				{
					atk_dam *= 7;
					atk_dam /= 5;
					break;
				}
				default: break;
			}

			switch (effect)
			{
				/*other bad effects - minor*/
				case GF_EAT_GOLD:
				case GF_EAT_ITEM:
				case GF_EAT_FOOD:
				case GF_HUNGER:
				case GF_EAT_LITE:
				case GF_TERRIFY:
				{
					atk_dam *= 11;
					atk_dam /= 10;
					break;
				}
				/*other bad effects - major*/
				case GF_UN_BONUS:
				case GF_UN_POWER:
				case GF_LOSE_MANA:
				case GF_POIS:
				case GF_ACID:
				case GF_ELEC:
				case GF_FIRE:
				case GF_COLD:
				case GF_BLIND:
				case GF_CONFUSION:
				case GF_PARALYZE:
				case GF_DISEASE:
				case GF_LOSE_STR:
				case GF_LOSE_INT:
				case GF_LOSE_WIS:
				case GF_LOSE_DEX:
				case GF_LOSE_CON:
				case GF_LOSE_CHR:
				case GF_LOSE_ALL:
				case GF_EXP_10:
				case GF_EXP_20:
				case GF_EXP_40:
				case GF_EXP_80:
				case GF_HALLU:
				{
					atk_dam *= 5;
					atk_dam /= 4;
					break;
				}
				/*Earthquakes*/
				case GF_SHATTER:
				{
					atk_dam *= 7;
					atk_dam /= 6;
					break;
				}
				/*nothing special*/
				default: break;
			}

			/*keep a running total*/
			melee_dam += atk_dam;
		}

		/*Reduce damamge potential for monsters that move randomly*/
		if ((r_ptr->flags1 & (RF1_RAND_25)) || (r_ptr->flags1 & (RF1_RAND_50)))
		{
			int reduce = 100;

			if (r_ptr->flags1 & (RF1_RAND_25)) reduce -= 25;
			if (r_ptr->flags1 & (RF1_RAND_50)) reduce -= 50;

			/*even moving randomly one in 8 times will hit the player*/
			reduce += (100 - reduce) / 8;

			/* adjust the melee damage*/
			melee_dam = (melee_dam * reduce) / 100;
		}

		/*monsters who can't move aren't nearly as much of a combat threat*/
		if (r_ptr->flags1 & (RF1_NEVER_MOVE))
		{
			melee_dam /= 4;
		}

		/*but keep a minimum*/
		if (melee_dam < 1) melee_dam = 1;
	}

	/*
	 * Get the max damage attack
	 */

	if (dam < spell_dam) dam = spell_dam;
	if (dam < melee_dam) dam = melee_dam;

	/*
	 * Adjust for speed.  Monster at speed 120 will do double damage,
	 * monster at speed 100 will do half, etc.  Bonus for monsters who can haste self.
	 */
	dam = (dam * extract_energy[r_ptr->speed + (r_ptr->flags6 & RF6_HASTE ? 5 : 0)]) / 10;

	/*but deep in a minimum*/
	if (dam < 1) dam  = 1;

	/* We're done */
	return (dam);
}
/*
 * Initialize the data structures for the monster power ratings
 * ToDo: Add handling and return codes for error conditions if any.
 */

static bool init_mon_power (void)
{
	int i, j;
	byte lvl;
	long hp, av_hp, av_dam;
	long tot_hp[MAX_DEPTH];
	long dam, dmg;
	long tot_dam[MAX_DEPTH];
	s16b mon_count[MAX_DEPTH];
	monster_race *rptr;

	/* Reset the sum of all monster power values */

	tot_mon_power = 0;

	/* Make sure all arrays start at zero */

	for (i = 0; i < MAX_DEPTH; i++)
	{
		tot_hp[i] = 0;
		tot_dam[i] = 0;
		mon_count[i] = 0;
	}

	/*
	 * Go through r_info and evaluate power ratings.
	 */

	for (i = 0; i < z_info->r_max; i++)
	{
		rptr = &r_info[i];

		/* Set the current level */

		lvl = rptr->level;

		/* Evaluate average HP for this monster */

		if (rptr->flags1 & (RF1_FORCE_MAXHP)) hp = rptr->hdice * rptr->hside;
		else hp = rptr->hdice * (rptr->hside + 1) / 2;

		/* Maximum damage this monster can do in 10 game turns */

		dam = eval_max_dam(i);

		/* Define the power rating */

		mon_power[i] = hp * dam;

		/* Adjust for group monsters.  Average in-level group size is 7 */

		if (rptr->flags1 & RF1_FRIENDS) mon_power[i] *= 7;

		/* Adjust for rarity.  Monsters with rarity > 1 appear less often */
		/* Paranoia */
		if (rptr->rarity != 0) mon_power[i] /= rptr->rarity;

		/*
		 * Update the running totals - these will be used as divisors later
		 * Total HP / dam / count for everything up to the current level
		 */

		for (j = lvl; j < MAX_DEPTH; j++)
		{

			tot_hp[j] += hp;
			tot_dam[j] += dam;

			/*
			 * Hack - if it's a group monster, add several to the count
			 * so that the averages don't get thrown off
			 */
			if (rptr->flags1 & RF1_FRIENDS) mon_count[j] += 7;
			else mon_count[j] += 1;
		}

	}

	/* Apply divisors now */

	for (i = 0; i < z_info->r_max; i++)
	{
		rptr = &r_info[i];

		/* Extract level */

		lvl = rptr->level;

		/* Paranoia */
		if (tot_hp[lvl] != 0 && tot_dam[lvl] != 0)
		{

			/* Divide by average HP and av damage for all in-level monsters */
			av_hp = tot_hp[lvl] / mon_count[lvl];
			av_dam = tot_dam[lvl] / mon_count[lvl];

			mon_power[i] = mon_power[i] / (av_hp * av_dam);

			/* Never less than 1 */
			if (mon_power[i] < 1) mon_power[i] = 1;

			/* Now we have the proper rating, add it to the total */

			tot_mon_power += mon_power[i];
		}

		if (randart_verbose)
		{
			/* Write the monster power rating, hp and damage to file */
			fprintf(randart_log, "Power rating for monster %d: ", i);
			dmg = eval_max_dam(i);
			fprintf(randart_log, "Max dam: %i ", (int)dmg);
			fprintf(randart_log, "Rating: %i\n", (int)mon_power[i]);
			fflush(randart_log);
		}

	}

	/* Now we have all the ratings */

	return TRUE;
}


/*
 * Calculate the rating for a given slay combination
 */

static s32b slay_power(u32b s_index)
{
	s32b sv;
	int i;
	int mult;
	monster_race *r_ptr;

	/* s_index combines the slay bytes into an index value
	 * For now we do not support the two undefined slays (XXX),
	 * but this could be added
	 */

	/* Look in the cache to see if we know this one yet */

	sv = slays[s_index];

	/* If it's cached, return its value */

	if (sv) return slays[s_index];

	/* Otherwise we need to calculate the expected average multiplier
	 * for this combination (multiplied by the total number of
	 * monsters, which we'll divide out later).
	 */

	sv = 0;

	for(i = 0; i < z_info->r_max; i++) {

		mult = 1;

		r_ptr = &r_info[i];

		/*
		 * Do the following in ascending order so that the best
		 * multiple is retained
		 */

		if ( (r_ptr->flags3 & (RF3_ANIMAL | RF3_INSECT | RF3_PLANT))
			&& (s_index & 0x00000001L) )
				mult = 2;

		/* New brand - brand_lite */
		if ( (r_ptr->flags3 & (RF3_HURT_LITE))
			&& (s_index & 0x00010000L) )
				mult = 2;

		/* New brand - brand_dark */
		if ( ((r_ptr->flags4 & (RF4_BRTH_DARK)) || (r_ptr->flags3 & (RF3_ORC)))
			&& (s_index & 0x00020000L) )
				mult = 2;

		if ( (r_ptr->flags3 & RF3_EVIL)
			&& (s_index & 0x00000002L) )
				mult = 2;
		if ( (r_ptr->flags3 & RF3_UNDEAD)
			&& (s_index & 0x00000004L) )
				mult = 3;
		if ( (r_ptr->flags3 & RF3_DEMON)
			&& (s_index & 0x00000008L) )
				mult = 3;
		if ( (r_ptr->flags3 & RF3_ORC)
			&& (s_index & 0x00000010L) )
				mult = 3;
		if ( (r_ptr->flags3 & RF3_TROLL)
			&& (s_index & 0x00000020L) )
				mult = 3;
		if ( (r_ptr->flags3 & RF3_GIANT)
			&& (s_index & 0x00000040L) )
				mult = 3;
		if ( (r_ptr->flags3 & RF3_DRAGON)
			&& (s_index & 0x00000080L) )
				mult = 3;

		/* New slay - slay man */
		if ( (strchr("pqt", r_ptr->d_char))
			&& (s_index & 0x00040000L) )
				mult = 3;

		/* New slay - slay elf */
		if ( (strchr("l", r_ptr->d_char))
			&& (s_index & 0x00080001L) )
				mult = 3;

		/* New slay - slay dwarf */
		if ( (strchr("h", r_ptr->d_char))
			&& ((strstr(r_name + r_ptr->name, "warf")) || (strstr(r_name + r_ptr->name, "warven")))
			&& (s_index & 0x00100000L) )
				mult = 3;



		/* Brands get the multiple if monster is NOT resistant */

		if ( !(r_ptr->flags3 & RF3_IM_ACID)
			&& (s_index & 0x00001000L) )
				mult = 3;
		if ( !(r_ptr->flags3 & RF3_IM_FIRE)
			&& (s_index & 0x00002000L) )
				mult = 3;
		if ( !(r_ptr->flags3 & RF3_IM_COLD)
			&& (s_index & 0x00004000L) )
				mult = 3;
		if ( !(r_ptr->flags3 & RF3_IM_ELEC)
			&& (s_index & 0x00008000L) )
				mult = 3;
		if ( !(r_ptr->flags3 & RF3_IM_POIS)
			&& (s_index & 0x00000800L) )
				mult = 3;

		/* Do kill flags last since they have the highest multiplier */

		if ( (r_ptr->flags3 & RF3_DRAGON)
			&& (s_index & 0x00000100L) )
				mult = 5;
		if ( (r_ptr->flags3 & RF3_DEMON)
			&& (s_index & 0x00000200L) )
				mult = 5;
		if ( (r_ptr->flags3 & RF3_UNDEAD)
			&& (s_index & 0x00000400L) )
				mult = 5;

		/* Add the multiple to sv */

		sv += mult * mon_power[i];

		/* End loop */
	}

	/*
	 * To get the expected damage for this weapon, multiply the
	 * average damage from base dice by sv, and divide by the
	 * total number of monsters.
	 */

	if (randart_verbose)
	{
		/* Write info about the slay combination and multiplier */
		fprintf(randart_log,"Slay multiplier for:");

		if (s_index & 0x00000002L) fprintf(randart_log,"Evl ");
		if (s_index & 0x00000100L) fprintf(randart_log,"XDr ");
		if (s_index & 0x00000200L) fprintf(randart_log,"XDm ");
		if (s_index & 0x00000400L) fprintf(randart_log,"XUn ");
		if (s_index & 0x00000001L) fprintf(randart_log,"Nat ");
		if (s_index & 0x00000004L) fprintf(randart_log,"Und ");
		if (s_index & 0x00000008L) fprintf(randart_log,"Drg ");
		if (s_index & 0x00000010L) fprintf(randart_log,"Dmn ");
		if (s_index & 0x00000020L) fprintf(randart_log,"Tro ");
		if (s_index & 0x00000040L) fprintf(randart_log,"Orc ");
		if (s_index & 0x00000080L) fprintf(randart_log,"Gia ");

		if (s_index & 0x00001000L) fprintf(randart_log,"Acd ");
		if (s_index & 0x00002000L) fprintf(randart_log,"Elc ");
		if (s_index & 0x00004000L) fprintf(randart_log,"Fir ");
		if (s_index & 0x00008000L) fprintf(randart_log,"Cld ");
		if (s_index & 0x00000800L) fprintf(randart_log,"Poi ");

		if (s_index & 0x00010000L) fprintf(randart_log,"Lit ");
		if (s_index & 0x00020000L) fprintf(randart_log,"Drk ");
		if (s_index & 0x00040000L) fprintf(randart_log,"Man ");
		if (s_index & 0x00080000L) fprintf(randart_log,"Elf ");
		if (s_index & 0x00100000L) fprintf(randart_log,"Dwf ");

		fprintf(randart_log,"times 1000 is: %d\n", (int)((1000 * sv) / tot_mon_power));
		fflush(randart_log);
	}

	/* Add to the cache */

	slays[s_index] = sv;

	return sv;

	/* End method */
}

/*
 * Calculate the multiplier we'll get with a given bow type.
 */
static int bow_multiplier(int sval)
{
	switch (sval)
	{
		case SV_SLING:
		case SV_SHORT_BOW:
			return (2);
		case SV_LONG_BOW:
		case SV_LIGHT_XBOW:
			return (3);
		case SV_HEAVY_XBOW:
			return (4);
		default:
			msg_format("Illegal bow sval %s", sval);
	}

	return (0);
}


static void remove_contradictory(artifact_type *a_ptr)
{
	if (a_ptr->flags3 & TR3_AGGRAVATE) a_ptr->flags1 &= ~(TR1_STEALTH);
	if (a_ptr->flags2 & TR2_IM_ACID) a_ptr->flags2 &= ~(TR2_RES_ACID);
	if (a_ptr->flags2 & TR2_IM_ELEC) a_ptr->flags2 &= ~(TR2_RES_ELEC);
	if (a_ptr->flags2 & TR2_IM_FIRE) a_ptr->flags2 &= ~(TR2_RES_FIRE);
	if (a_ptr->flags2 & TR2_IM_COLD) a_ptr->flags2 &= ~(TR2_RES_COLD);

	if (a_ptr->flags2 & TR2_IM_ACID) a_ptr->flags4 &= ~(TR4_HURT_ACID);
	if (a_ptr->flags2 & TR2_IM_ELEC) a_ptr->flags4 &= ~(TR4_HURT_ELEC);
	if (a_ptr->flags2 & TR2_IM_FIRE) a_ptr->flags4 &= ~(TR4_HURT_FIRE);
	if (a_ptr->flags2 & TR2_IM_COLD) a_ptr->flags4 &= ~(TR4_HURT_COLD);

	if (a_ptr->flags2 & TR2_RES_ACID) a_ptr->flags4 &= ~(TR4_HURT_ACID);
	if (a_ptr->flags2 & TR2_RES_ELEC) a_ptr->flags4 &= ~(TR4_HURT_ELEC);
	if (a_ptr->flags2 & TR2_RES_FIRE) a_ptr->flags4 &= ~(TR4_HURT_FIRE);
	if (a_ptr->flags2 & TR2_RES_COLD) a_ptr->flags4 &= ~(TR4_HURT_COLD);
	if (a_ptr->flags2 & TR2_RES_LITE) a_ptr->flags4 &= ~(TR4_HURT_LITE);

	if (a_ptr->pval < 0)
	{
		if (a_ptr->flags1 & TR1_STR) a_ptr->flags2 &= ~(TR2_SUST_STR);
		if (a_ptr->flags1 & TR1_INT) a_ptr->flags2 &= ~(TR2_SUST_INT);
		if (a_ptr->flags1 & TR1_WIS) a_ptr->flags2 &= ~(TR2_SUST_WIS);
		if (a_ptr->flags1 & TR1_DEX) a_ptr->flags2 &= ~(TR2_SUST_DEX);
		if (a_ptr->flags1 & TR1_CON) a_ptr->flags2 &= ~(TR2_SUST_CON);
		if (a_ptr->flags1 & TR1_CHR) a_ptr->flags2 &= ~(TR2_SUST_CHR);
		a_ptr->flags1 &= ~(TR1_BLOWS);
	}

	if (a_ptr->flags3 & TR3_LIGHT_CURSE) a_ptr->flags3 &= ~(TR3_BLESSED);
	if (a_ptr->flags1 & TR1_KILL_DRAGON) a_ptr->flags1 &= ~(TR1_SLAY_DRAGON);
	if (a_ptr->flags1 & TR1_KILL_DEMON) a_ptr->flags1 &= ~(TR1_SLAY_DEMON);
	if (a_ptr->flags1 & TR1_KILL_UNDEAD) a_ptr->flags1 &= ~(TR1_SLAY_UNDEAD);
	if (a_ptr->flags3 & TR3_DRAIN_EXP) a_ptr->flags3 &= ~(TR3_HOLD_LIFE);
	if (a_ptr->flags3 & TR3_DRAIN_HP) a_ptr->flags3 &= ~(TR3_REGEN);
	if (a_ptr->flags3 & TR3_DRAIN_MANA) a_ptr->flags3 &= ~(TR3_REGEN);

	if (!(a_ptr->flags4 & TR4_EVIL))
	{
		if (a_ptr->flags1 & TR1_SLAY_NATURAL) a_ptr->flags4 &= ~(TR4_ANIMAL);
		if (a_ptr->flags1 & TR1_SLAY_EVIL) a_ptr->flags4 &= ~(TR4_EVIL);
		if (a_ptr->flags1 & TR1_SLAY_UNDEAD) a_ptr->flags4 &= ~(TR4_UNDEAD);
		if (a_ptr->flags1 & TR1_SLAY_DEMON) a_ptr->flags4 &= ~(TR4_DEMON);
		if (a_ptr->flags1 & TR1_SLAY_ORC) a_ptr->flags4 &= ~(TR4_ORC);
		if (a_ptr->flags1 & TR1_SLAY_TROLL) a_ptr->flags4 &= ~(TR4_TROLL);
		if (a_ptr->flags1 & TR1_SLAY_GIANT) a_ptr->flags4 &= ~(TR4_GIANT);
		if (a_ptr->flags1 & TR1_SLAY_DRAGON) a_ptr->flags4 &= ~(TR4_DRAGON);
		if (a_ptr->flags1 & TR1_KILL_DRAGON) a_ptr->flags4 &= ~(TR4_DRAGON);
		if (a_ptr->flags1 & TR1_KILL_DEMON) a_ptr->flags4 &= ~(TR4_DEMON);
		if (a_ptr->flags1 & TR1_KILL_UNDEAD) a_ptr->flags4 &= ~(TR4_UNDEAD);

		if (a_ptr->flags1 & TR1_SLAY_ORC) a_ptr->flags4 &= ~(TR4_SLAY_ELF);
		if (a_ptr->flags1 & TR1_SLAY_GIANT) a_ptr->flags4 &= ~(TR4_SLAY_DWARF);

		if (a_ptr->flags4 & TR4_SLAY_MAN) a_ptr->flags4 &= ~(TR4_MAN);
		if (a_ptr->flags4 & TR4_SLAY_ELF) a_ptr->flags4 &= ~(TR4_ELF);
		if (a_ptr->flags4 & TR4_SLAY_DWARF) a_ptr->flags4 &= ~(TR4_DWARF);
	}

	if (a_ptr->flags1 & TR1_BRAND_POIS) a_ptr->flags4 &= ~(TR4_HURT_POIS);
	if (a_ptr->flags1 & TR1_BRAND_ACID) a_ptr->flags4 &= ~(TR4_HURT_ACID);
	if (a_ptr->flags1 & TR1_BRAND_ELEC) a_ptr->flags4 &= ~(TR4_HURT_ELEC);
	if (a_ptr->flags1 & TR1_BRAND_FIRE) a_ptr->flags4 &= ~(TR4_HURT_FIRE);
	if (a_ptr->flags1 & TR1_BRAND_COLD) a_ptr->flags4 &= ~(TR4_HURT_COLD);

	if (a_ptr->flags4 & TR4_BRAND_LITE) a_ptr->flags4 &= ~(TR4_HURT_LITE);

	if (a_ptr->flags3 & TR3_TELEPORT) a_ptr->flags4 &= ~(TR4_ANCHOR);

}


/*
 * Evaluate the artifact's overall power level.
 */
static s32b artifact_power(int a_idx)
{
	const artifact_type *a_ptr = &a_info[a_idx];
	s32b p = 0;
	s16b k_idx;
	object_kind *k_ptr;
	int immunities = 0;
	int sustains = 0;
	int low_resists = 0;
	int high_resists = 0;

	LOG_PRINT("********** ENTERING EVAL POWER ********\n");
	LOG_PRINT1("Artifact index is %d\n", a_idx);

	/* Try to use the cache */
	k_idx = kinds[a_idx];

	/* Lookup the item if not yet cached */
	if (!k_idx)
	{
		k_idx = lookup_kind(a_ptr->tval, a_ptr->sval);

		/* Cache the object index */
		kinds[a_idx] = k_idx;

		/* Paranoia */
		if (!k_idx)
		{
			quit_fmt("Illegal tval(%d)/sval(%d) value for artifact %d!", a_ptr->tval, a_ptr->sval,a_idx);
		}
	}

	k_ptr = &k_info[k_idx];

	if (a_idx >= ART_MIN_NORMAL)
	{
		LOG_PRINT1("Initial power level is %d\n", p);
	}

	/* Evaluate certain abilities based on type of object. */
	switch (a_ptr->tval)
	{
		case TV_BOW:
		{
			int mult;

			/*
			 * Add the average damage of fully enchanted (good) ammo for this
			 * weapon.  Could make this dynamic based on k_info if desired.
			 */

			if (a_ptr->sval == SV_SLING)
			{
				p += AVG_SLING_AMMO_DAMAGE;
			}
			else if (a_ptr->sval == SV_SHORT_BOW ||
				a_ptr->sval == SV_LONG_BOW)
			{
				p += AVG_BOW_AMMO_DAMAGE;
			}
			else if (a_ptr->sval == SV_LIGHT_XBOW ||
				a_ptr->sval == SV_HEAVY_XBOW)
			{
				p += AVG_XBOW_AMMO_DAMAGE;
			}

			LOG_PRINT1("Adding power from ammo, total is %d\n", p);

			mult = bow_multiplier(a_ptr->sval);
			LOG_PRINT1("Base multiplier for this weapon is %d\n", mult);

			if (a_ptr->to_h > 9)
			{
				p+= (a_ptr->to_h - 9) * 2 / 3;
				LOG_PRINT1("Adding power from high to_hit, total is %d\n", p);
			}
			else
			{
				p += 6;
				LOG_PRINT1("Adding power from base to_hit, total is %d\n", p);
			}

			if (a_ptr->flags1 & TR1_MIGHT)
			{
				if (a_ptr->pval > 3 || a_ptr->pval < 0)
				{
					p += 20000;	/* inhibit */
					mult = 1;	/* don't overflow */
				}
				else
				{
					mult += a_ptr->pval;
				}

				LOG_PRINT1("Extra might multiple is %d\n", mult);
			}
			p *= mult;

			LOG_PRINT2("Multiplying power by %d, total is %d\n", mult, p);

			/*
			 * Damage multiplier for bows should be weighted less than that
			 * for melee weapons, since players typically get fewer shots
			 * than hits (note, however, that the multipliers are applied
			 * afterwards in the bow calculation, not before as for melee
			 * weapons, which tends to bring these numbers back into line).
			 */

			if (a_ptr->to_d > 9)
			{
				p += a_ptr->to_d;
				LOG_PRINT1("Adding power from to_dam greater than 9, total is %d\n", p);
			}
			else
			{
				p += 9;
				LOG_PRINT1("Adding power for base to_dam, total is %d\n", p);
			}

			LOG_PRINT1("Adding power from to_dam, total is %d\n", p);

			if (a_ptr->flags1 & TR1_SHOTS)
			{
				/*
				 * Extra shots are calculated differently for bows than for
				 * slings or crossbows, because of rangers ... not any more CC 13/8/01
				 */

				LOG_PRINT1("Extra shots: %d\n", a_ptr->pval);

				if (a_ptr->pval > 3 || a_ptr->pval < 0)
				{
					p += 20000;	/* inhibit */
					LOG_PRINT("INHIBITING - more than 3 extra shots\n");
				}
				else if (a_ptr->pval > 0)
				{
					if (a_ptr->sval == SV_SHORT_BOW ||
						a_ptr->sval == SV_LONG_BOW)
					{
						p = (p * (1 + a_ptr->pval));
					}
					else
					{
						p = (p * (1 + a_ptr->pval));
					}
					LOG_PRINT2("Multiplying power by 1 + %d, total is %d\n", a_ptr->pval, p);
				}

			}

			/* Normalise power back */
			/* We now only count power as 'above' having the basic weapon at the same level */
			if (a_ptr->sval == SV_SLING)
			{
				if (ABS(p) > AVG_SLING_AMMO_DAMAGE)
					p -= sign(p) * AVG_SLING_AMMO_DAMAGE * bow_multiplier(k_ptr->sval);
				else
					p = 0;
			}
			else if (a_ptr->sval == SV_SHORT_BOW ||
				a_ptr->sval == SV_LONG_BOW)
			{
				if (ABS(p) > AVG_BOW_AMMO_DAMAGE)
					p -= sign(p) * AVG_BOW_AMMO_DAMAGE * bow_multiplier(k_ptr->sval);
				else
					p = 0;
			}
			else if (a_ptr->sval == SV_LIGHT_XBOW ||
				a_ptr->sval == SV_HEAVY_XBOW)
			{
				if (ABS(p) > AVG_XBOW_AMMO_DAMAGE)
					p -= sign(p) * AVG_XBOW_AMMO_DAMAGE * bow_multiplier(k_ptr->sval);
				else
					p = 0;
			}

			if (a_ptr->weight < k_ptr->weight)
			{
				p++;
				LOG_PRINT("Incrementing power by one for low weight\n");
			}

			/*
			 * Correction to match ratings to melee damage ratings.
			 * We multiply all missile weapons by 1.5 in order to compare damage.
			 * (CR 11/20/01 - changed this to 1.25).
			 * Melee weapons assume 5 attacks per turn, so we must also divide
			 * by 5 to get equal ratings.
			 */

			if (a_ptr->sval == SV_SHORT_BOW ||
				a_ptr->sval == SV_LONG_BOW)
			{
				p = sign(p) * (ABS(p) / 4);
				LOG_PRINT1("Rescaling bow power, total is %d\n", p);
			}
			else
			{
				p = sign(p) * (ABS(p) / 4);
				LOG_PRINT1("Rescaling xbow/sling power, total is %d\n", p);
			}

			/* Ignore theft / acid / fire -- Slight bonus as may choose to swap bows */
			p += 1;
			LOG_PRINT1("Adding power for ignore theft / acid / fire, total is %d\n", p);

			break;
		}
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		{
			/* Not this is 'uncorrected' */
			p += a_ptr->dd * (a_ptr->ds + 1);
			LOG_PRINT1("Adding power for dam dice, total is %d\n", p);

			/* Apply the correct slay multiplier */

			p = (p * slay_power(slay_index(a_ptr->flags1, a_ptr->flags2, a_ptr->flags3, a_ptr->flags4))) / tot_mon_power;
			LOG_PRINT1("Adjusted for slay power, total is %d\n", p);

			p /= 2;
			LOG_PRINT1("Correcting damage, total is %d\n", p);

			/* To hit uses a percentage - up to 65%*/
			if (a_ptr->to_h > 9)
			{
				p+= a_ptr->to_h * 2 / 3;
				LOG_PRINT1("Adding power for to_hit greater than 9, total is %d\n", p);
			}
			else
			{
				p+= 6;
				LOG_PRINT1("Adding power for base to_hit, total is %d\n", p);
			}

			if (a_ptr->to_d > 9)
			{
				p += a_ptr->to_d;
				LOG_PRINT1("Adding power for to_dam greater than 9, total is %d\n", p);
			}
			else
			{
				p+= 9;
				LOG_PRINT1("Adding power for base to_dam, total is %d\n", p);
			}


			if (a_ptr->flags1 & TR1_BLOWS)
			{
				LOG_PRINT1("Extra blows: %d\n", a_ptr->pval);
				if (a_ptr->pval > 3 || a_ptr->pval < 0)
				{
					p += 20000;	/* inhibit */
					LOG_PRINT("INHIBITING, more than 3 extra blows or a negative number\n");
				}
				else if (a_ptr->pval > 0)
				{
					p = sign(p) * ((ABS(p) * (5 + a_ptr->pval)) / 5);
					/* Add an extra +5 per blow to account for damage rings */
					/* (The +5 figure is a compromise here - could be adjusted) */
					p += 5 * a_ptr->pval;
					LOG_PRINT1("Adding power for blows, total is %d\n", p);
				}
			}

			/* Normalise power back */
			/* We remove the weapon base damage to get 'true' power */
			/* This makes e.g. a sword that provides fire immunity the same value as
			   a ring that provides fire immunity */
			if (ABS(p) > k_ptr->dd * (k_ptr->ds + 1) / 2 + 15)
				p -= sign(p) * (k_ptr->dd * (k_ptr->ds + 1) / 2 + 15);
			else
				p = 0;

			LOG_PRINT1("Normalising power against base item, total is %d\n", p);

			if (a_ptr->ac != k_ptr->ac)
			{
				p += a_ptr->ac - k_ptr->ac;
				LOG_PRINT1("Adding power for non-standard AC value, total is %d\n", p);
			}

			/* Remember, weight is in 0.1 lb. units. */
			if (a_ptr->weight != k_ptr->weight)
			{
			/*	p += (k_ptr->weight - a_ptr->weight) / 20; */
				LOG_PRINT1("Adding power for low weight, total is %d\n", p);
			}

			/* Ignore theft / acid / fire -- Bonus as may choose to swap melee weapons */
			p += 3;
			LOG_PRINT1("Adding power for ignore theft / acid / fire, total is %d\n", p);

			break;
		}
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{
			if (a_ptr->ac != k_ptr->ac)
			{
				p += a_ptr->ac - k_ptr->ac;
			}
			LOG_PRINT1("Adding power for non-standard AC value, total is %d\n", p);

			p += sign(a_ptr->to_h) * ((ABS(a_ptr->to_h) * 2) / 3);
			LOG_PRINT1("Adding power for to_hit, total is %d\n", p);

			p += a_ptr->to_d * 2;
			LOG_PRINT1("Adding power for to_dam, total is %d\n", p);

			if (a_ptr->weight < k_ptr->weight)
			{
				p += (k_ptr->weight - a_ptr->weight) / 10;
				LOG_PRINT1("Adding power for low weight, total is %d\n", p);
			}

			/* Ignore theft / acid / fire -- Big bonus for ignore acid */
			p += 5;
			LOG_PRINT1("Adding power for ignore theft / *acid* / fire, total is %d\n", p);

			break;
		}
		case TV_LITE:
		{
			p += 10;
			LOG_PRINT("Artifact light source, adding 10 as base\n");

			p += sign(a_ptr->to_h) * ((ABS(a_ptr->to_h) * 2) / 3);
			LOG_PRINT1("Adding power for to_hit, total is %d\n", p);

			p += a_ptr->to_d * 2;
			LOG_PRINT1("Adding power for to_dam, total is %d\n", p);

			break;
		}
		case TV_RING:
		case TV_AMULET:
		{
			LOG_PRINT("Artifact jewellery, adding 0 as base\n");

			p += sign(a_ptr->to_h) * ((ABS(a_ptr->to_h) * 2) / 3);
			LOG_PRINT1("Adding power for to_hit, total is %d\n", p);

			p += a_ptr->to_d * 2;
			LOG_PRINT1("Adding power for to_dam, total is %d\n", p);

			/* Ignore theft -- Very slight bonus for ignore ignore */
			p += 1;
			LOG_PRINT1("Adding power for ignore theft, total is %d\n", p);

			break;
		}
	}

	/* Compute weight discount percentage */
	/*
	 * If an item weighs more than 5 pounds, we discount its power.
	 *
	 * This figure is from 30 lb weight limit for mages divided by 6 slots;
	 * but we do it for all items as they may be used as swap items, but use a divisor of 10 lbs instead.
	 */

	/* Evaluate ac bonus and weight differentl for armour and non-armour. */
	switch (a_ptr->tval)
	{
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{
			if (a_ptr->weight >= 50)
			{
				p -= a_ptr->weight / 50; 

				LOG_PRINT2("Reducing power for armour weight of %d, total is %d\n", a_ptr->weight, p);
			}

			if (a_ptr->to_a > 9)
			{
				p+= (a_ptr->to_a - 9);
				LOG_PRINT2("Adding power for to_ac of %d, total is %d\n", a_ptr->to_a, p);
			}

			if (a_ptr->to_a > 19)
			{
				p += (a_ptr->to_a - 19);
				LOG_PRINT1("Adding power for high to_ac value, total is %d\n", p);
			}

			if (a_ptr->to_a > 29)
			{
				p += (a_ptr->to_a - 29);
				LOG_PRINT1("Adding power for very high to_ac value, total is %d\n", p);
			}

			if (a_ptr->to_a > 39)
			{
				p += 20000;	/* inhibit */
				LOG_PRINT("INHIBITING: AC bonus too high\n");
			}
			break;

		default:
			if (a_ptr->weight >= 100)
			{
				p -= a_ptr->weight / 100; 

				LOG_PRINT2("Reducing power for swap item weight of %d, total is %d\n", a_ptr->weight, p);

			}

			p += sign(a_ptr->to_a) * (ABS(a_ptr->to_a) / 2);

			if (a_ptr->to_a > 9)
			{
				p+= (a_ptr->to_a - 9);
				LOG_PRINT2("Adding power for high to_ac of %d, total is %d\n", a_ptr->to_a, p);

			}

			if (a_ptr->to_a > 19)
			{
				p += (a_ptr->to_a - 19);
				LOG_PRINT1("Adding power for very high to_ac value, total is %d\n", p);
			}

			if (a_ptr->to_a > 29)
			{
				p += 20000;	/* inhibit */
				LOG_PRINT("INHIBITING: AC bonus too high\n");
			}
			break;
		}
	}

	/* Other abilities are evaluated independent of the object type. */
	if (a_ptr->pval > 0)
	{
		if (a_ptr->flags1 & TR1_STR)
		{
			p += 3 * a_ptr->pval * a_ptr->pval / 4;
			LOG_PRINT2("Adding power for STR bonus %d, total is %d\n", a_ptr->pval, p);
		}
		if (a_ptr->flags1 & TR1_INT)
		{
			p += a_ptr->pval * a_ptr->pval / 2;
			LOG_PRINT2("Adding power for INT bonus %d, total is %d\n", a_ptr->pval, p);
		}
		if (a_ptr->flags1 & TR1_WIS)
		{
			p += a_ptr->pval * a_ptr->pval / 2;
			LOG_PRINT2("Adding power for WIS bonus %d, total is %d\n", a_ptr->pval, p);
		}
		if (a_ptr->flags1 & TR1_DEX)
		{
			p += 3 * a_ptr->pval * a_ptr->pval / 4;
			LOG_PRINT2("Adding power for DEX bonus %d, total is %d\n", a_ptr->pval, p);
		}
		if (a_ptr->flags1 & TR1_CON)
		{
			p += a_ptr->pval * a_ptr->pval;
			LOG_PRINT2("Adding power for CON bonus %d, total is %d\n", a_ptr->pval, p);
		}
		if (a_ptr->flags1 & TR1_SAVE)
		{
			p += a_ptr->pval * a_ptr->pval / 4;
			LOG_PRINT2("Adding power for save bonus %d, total is %d\n", a_ptr->pval, p);
		}
		if (a_ptr->flags1 & TR1_DEVICE)
		{
			p += a_ptr->pval * a_ptr->pval / 6;
			LOG_PRINT2("Adding power for device bonus %d, total is %d\n", a_ptr->pval, p);
		}
		if (a_ptr->flags1 & TR1_STEALTH)
		{
			p += a_ptr->pval * a_ptr->pval / 4;
			LOG_PRINT2("Adding power for stealth bonus %d, total is %d\n", a_ptr->pval, p);
		}
		if (a_ptr->flags1 & TR1_TUNNEL)
		{
			p += a_ptr->pval * a_ptr->pval / 6;
			LOG_PRINT2("Adding power for tunnel bonus %d, total is %d\n", a_ptr->pval, p);
		}
		/* For now add very small amount for searching */
		if (a_ptr->flags1 & TR1_SEARCH)
		{
			p += a_ptr->pval * a_ptr->pval / 24;
			LOG_PRINT2("Adding power for searching bonus %d, total is %d\n", a_ptr->pval , p);
		}
	}
	else if (a_ptr->pval < 0)	/* hack: don't give large negatives */
	{
		if (a_ptr->flags1 & TR1_STR) p += 4 * a_ptr->pval;
		if (a_ptr->flags1 & TR1_INT) p += 2 * a_ptr->pval;
		if (a_ptr->flags1 & TR1_WIS) p += 2 * a_ptr->pval;
		if (a_ptr->flags1 & TR1_DEX) p += 3 * a_ptr->pval;
		if (a_ptr->flags1 & TR1_CON) p += 4 * a_ptr->pval;
		if (a_ptr->flags1 & TR1_STEALTH) p += a_ptr->pval;
		LOG_PRINT1("Subtracting power for negative ability values, total is %d\n", p);
	}
	if (a_ptr->flags1 & TR1_CHR)
	{
		p += a_ptr->pval;
		LOG_PRINT2("Adding power for CHR bonus/penalty %d, total is %d\n", a_ptr->pval, p);
	}
	if (a_ptr->flags1 & TR1_INFRA)
	{
		p += a_ptr->pval;
		LOG_PRINT2("Adding power for infra bonus/penalty %d, total is %d\n", a_ptr->pval, p);
	}
	if (a_ptr->flags1 & TR1_SPEED)
	{
		p += 5 * a_ptr->pval;
		LOG_PRINT2("Adding power for speed bonus/penalty %d, total is %d\n", a_ptr->pval, p);
	}

#define ADD_POWER(string, val, flag, flgnum, extra) \
	if (a_ptr->flags##flgnum & flag) { \
		p += (val); \
		extra; \
		LOG_PRINT1("Modifying power for " string ", total is %d\n", p); \
	}

	ADD_POWER("sustain STR",	 5, TR2_SUST_STR, 2,sustains++);
	ADD_POWER("sustain INT",	 2, TR2_SUST_INT, 2,sustains++);
	ADD_POWER("sustain WIS",	 2, TR2_SUST_WIS, 2,sustains++);
	ADD_POWER("sustain DEX",	 4, TR2_SUST_DEX, 2,sustains++);
	ADD_POWER("sustain CON",	 3, TR2_SUST_CON, 2,sustains++);
	ADD_POWER("sustain CHR",	 0, TR2_SUST_CHR, 2,sustains++);

	/* Add bonus for sustains getting 'sustain-lock' */
	if (sustains > 1)
	{
		p += (sustains -1);
		LOG_PRINT1("Adding power for multiple sustains, total is %d\n", p); \
	}

	ADD_POWER("acid immunity",	17, TR2_IM_ACID, 2, immunities++);
	ADD_POWER("elec immunity",	14, TR2_IM_ELEC, 2, immunities++);
	ADD_POWER("fire immunity",	22, TR2_IM_FIRE, 2, immunities++);
	ADD_POWER("cold immunity",	17, TR2_IM_COLD, 2, immunities++);

	if (immunities > 1)
	{
		p += 15;
		LOG_PRINT1("Adding power for multiple immunities, total is %d\n", p);
	}
	if (immunities > 2)
	{
		p += 45;
		LOG_PRINT1("Adding power for three or more immunities, total is %d\n", p);
	}
	if (immunities > 3)
	{
		p += 20000;		/* inhibit */
		LOG_PRINT("INHIBITING: Too many immunities\n");
	}

	low_resists += immunities;

	ADD_POWER("free action",	 7, TR3_FREE_ACT, 3, high_resists++);
	ADD_POWER("hold life",		 6, TR3_HOLD_LIFE, 3, high_resists++);
	ADD_POWER("feather fall",	 1, TR3_FEATHER, 3,); /* was 2 */
	ADD_POWER("permanent light",     4, TR3_LITE, 3,); /* was 2 */

	ADD_POWER("see invisible",	 4, TR3_SEE_INVIS, 3,);
	ADD_POWER("sense orcs",	  	3, TR3_ESP_ORC, 3,);
	ADD_POWER("sense trolls",	3, TR3_ESP_TROLL, 3,);
	ADD_POWER("sense giants",	3, TR3_ESP_GIANT, 3,);
	ADD_POWER("sense demons",	4, TR3_ESP_DEMON, 3,);
	ADD_POWER("sense undead",	5, TR3_ESP_UNDEAD, 3,);
	ADD_POWER("sense dragons",       5, TR3_ESP_DRAGON, 3,);
	ADD_POWER("sense nature",	4, TR3_ESP_NATURE, 3,);
	ADD_POWER("telepathy",	  18, TR3_TELEPATHY, 3,);
	ADD_POWER("slow digestion",	 1, TR3_SLOW_DIGEST, 3,);

	/* Digging moved to general section since it can be on anything now */
	ADD_POWER("tunnelling",	 a_ptr->pval, TR1_TUNNEL, 1,);

	ADD_POWER("resist acid",	 2, TR2_RES_ACID, 2, low_resists++);
	ADD_POWER("resist elec",	 3, TR2_RES_ELEC, 2, low_resists++);
	ADD_POWER("resist fire",	 3, TR2_RES_FIRE, 2, low_resists++);
	ADD_POWER("resist cold",	 3, TR2_RES_COLD, 2, low_resists++);

	/* Add bonus for getting 'low resist-lock' */
	if (low_resists > 1)
	{
		p += (low_resists-1) * 2;
		LOG_PRINT1("Adding power for multiple low resists, total is %d\n", p); \
	}

	ADD_POWER("resist poison",	14, TR2_RES_POIS, 2, high_resists++);
	ADD_POWER("resist light",	 3, TR2_RES_LITE, 2, high_resists++);
	ADD_POWER("resist dark",	 8, TR2_RES_DARK, 2, high_resists++);
	ADD_POWER("resist blindness",	 8, TR2_RES_BLIND, 2, high_resists++);
	ADD_POWER("resist confusion",	12, TR2_RES_CONFU, 2, high_resists++);
	ADD_POWER("resist sound",	 7, TR2_RES_SOUND, 2, high_resists++);
	ADD_POWER("resist shards",	 4, TR2_RES_SHARD, 2, high_resists++);
	ADD_POWER("resist nexus",	 5, TR2_RES_NEXUS, 2, high_resists++);
	ADD_POWER("resist nether",	10, TR2_RES_NETHR, 2, high_resists++);
	ADD_POWER("resist chaos",	10, TR2_RES_CHAOS, 2, high_resists++);
	ADD_POWER("resist disenchantment", 10, TR2_RES_DISEN, 2, high_resists++);

	/* Add bonus for getting 'high resist-lock' */
	if (high_resists > 1)
	{
		p += (high_resists-1) * 3;
		LOG_PRINT1("Adding power for multiple high resists, total is %d\n", p); \
	}

	ADD_POWER("regeneration",	 4, TR3_REGEN, 3,);
	ADD_POWER("blessed",		 1, TR3_BLESSED, 3,);

	ADD_POWER("blood vampire",	 25, TR4_VAMP_HP, 4,);
	ADD_POWER("mana vampire",	 14, TR4_VAMP_MANA, 4,);

	ADD_POWER("teleportation",	 -40, TR3_TELEPORT, 3,);
	ADD_POWER("drain experience",	 -20, TR3_DRAIN_EXP, 3,);

	ADD_POWER("drain health",	 -20, TR3_DRAIN_HP, 3,);
	ADD_POWER("drain mana",	 	 -10, TR3_DRAIN_MANA, 3,);
	ADD_POWER("aggravation",	 -15, TR3_AGGRAVATE, 3,);
	ADD_POWER("light curse",	 -1,  TR3_LIGHT_CURSE, 3,);
	ADD_POWER("heavy curse",	 -4, TR3_HEAVY_CURSE, 3,);
/*	ADD_POWER("permanent curse",	 -40, TR3_PERMA_CURSE, 3,);*/
	ADD_POWER("light vulneribility", -30, TR4_HURT_LITE, 4,);
	ADD_POWER("water vulneribility", -30, TR4_HURT_WATER, 4,);
	ADD_POWER("hunger",	 	 -15, TR4_HUNGER, 4,);
	ADD_POWER("anchor",	 	 -4, TR4_ANCHOR, 4,);
	ADD_POWER("silent",	 	 -20, TR4_SILENT, 4,);
	ADD_POWER("static",	 	 -15, TR4_STATIC, 4,);
	ADD_POWER("windy",	 	 -15, TR4_WINDY, 4,);
	ADD_POWER("animal",	 	 -5, TR4_ANIMAL, 4,);
	ADD_POWER("evil",	 	 -5, TR4_EVIL, 4,);
	ADD_POWER("undead",	 	 -10, TR4_UNDEAD, 4,);
	ADD_POWER("demon",	 	 -10, TR4_DEMON, 4,);
	ADD_POWER("orc",	 	 -3, TR4_ORC, 4,);
	ADD_POWER("troll",	 	 -3, TR4_TROLL, 4,);
	ADD_POWER("giant",	 	 -5, TR4_GIANT, 4,);
	ADD_POWER("dragon",	 	 -10, TR4_DRAGON, 4,);
	ADD_POWER("man",	 	 -3, TR4_MAN, 4,);
	ADD_POWER("dwarf",	 	 -3, TR4_DWARF, 4,);
	ADD_POWER("elf",	 	 -3, TR4_ELF, 4,);
	ADD_POWER("hurt poison",	 -50, TR4_HURT_POIS, 4,);
	ADD_POWER("hurt acid",	 	 -30, TR4_HURT_ACID, 4,);
	ADD_POWER("hurt lightning",	 -40, TR4_HURT_ELEC, 4,);
	ADD_POWER("hurt fire",	 -40, TR4_HURT_FIRE, 4,);
	ADD_POWER("hurt cold",	 -40, TR4_HURT_COLD, 4,);

	return (p);

}


/*
 * Store the original artifact power ratings as a baseline
 */

static void store_base_power (void)
{
	int i;
	artifact_type *a_ptr;
	object_kind *k_ptr;
	s16b k_idx;

	for(i = 0; i < z_info->a_max; i++)
	{
		a_ptr = &a_info[i];
		remove_contradictory(a_ptr);
		base_power[i] = artifact_power(i);
		a_ptr->power = base_power[i];
	}

	for(i = 0; i < z_info->a_max; i++)
	{
		/* Kinds array was populated in the above step */
		k_idx = kinds[i];
		k_ptr = &k_info[k_idx];
		a_ptr = &a_info[i];
		base_item_level[i] = k_ptr->level;
		base_item_rarity[i] = k_ptr->chance[0];
		base_art_rarity[i] = a_ptr->rarity;
	}

	/* Store the number of different types, for use later */

	for (i = 0; i < z_info->a_max; i++)
	{
		switch (a_info[i].tval)
		{
			case TV_SWORD:
			case TV_POLEARM:
			case TV_HAFTED:
				art_melee_total++; break;
			case TV_BOW:
				art_bow_total++; break;
			case TV_SOFT_ARMOR:
			case TV_HARD_ARMOR:
			case TV_DRAG_ARMOR:
				art_armor_total++; break;
			case TV_SHIELD:
				art_shield_total++; break;
			case TV_CLOAK:
				art_cloak_total++; break;
			case TV_HELM:
			case TV_CROWN:
				art_headgear_total++; break;
			case TV_GLOVES:
				art_glove_total++; break;
			case TV_BOOTS:
				art_boot_total++; break;
			default:
				art_other_total++;
		}
	}
	art_total = art_melee_total + art_bow_total + art_armor_total +
		art_shield_total + art_cloak_total + art_headgear_total +
		art_glove_total + art_boot_total + art_other_total;
}

static struct item_choice {
	int threshold;
	int tval;
	char *report;
} item_choices[] = {
	{  6, TV_BOW,		"a missile weapon"},
	{  9, TV_DIGGING,	"a digger"},
	{ 19, TV_HAFTED,	"a blunt weapon"},
	{ 33, TV_SWORD,		"an edged weapon"},
	{ 42, TV_POLEARM,	"a polearm"},
	{ 64, TV_SOFT_ARMOR,	"body armour"},
	{ 71, TV_BOOTS,		"footwear"},
	{ 78, TV_GLOVES,	"gloves"},
	{ 87, TV_HELM,		"a hat"},
	{ 94, TV_SHIELD,	"a shield"},
	{100, TV_CLOAK,		"a cloak"}
};

/*
 * Randomly select a base item type (tval,sval).  Assign the various fields
 * corresponding to that choice.
 *
 * The return value gives the index of the new item type.  The method is
 * passed a pointer to a rarity value in order to return the rarity of the
 * new item.
 */
static s16b choose_item(int a_idx)
{
	artifact_type *a_ptr = &a_info[a_idx];
	int tval, sval=0;
	object_kind *k_ptr;
	int r, i;
	s16b k_idx, r2;
	byte target_level;

	/*
	 * Look up the original artifact's base object kind to get level.
	 */
	k_idx = kinds[a_idx];
	k_ptr = &k_info[k_idx];
	target_level = base_item_level[a_idx];
	LOG_PRINT1("Base item level is: %d\n", target_level);


	/*
	 * If the artifact level is higher then we use that instead.  Note that
	 * we can get away with reusing the artifact rarities here since we
	 * don't change them.  If artifact rarities are changed then the
	 * original values will need to be stored, as for base items.
	 */

	if(a_ptr->level > target_level) target_level = a_ptr->level;
	LOG_PRINT1("Target level is: %d\n", target_level);


	/*
	 * Pick a category (tval) of weapon randomly.  Within each tval, roll
	 * an sval (specific item) based on the target level.  The number we
	 * roll should be a bell curve.  The mean and standard variation of the
	 * bell curve are based on the target level; the distribution of
	 * kinds versus the bell curve is hand-tweaked. :-(
	 * CR 8/11/01 - reworked some of these lists
	 */
	r = rand_int(100);
	r2 = Rand_normal(target_level * 2, target_level);
	LOG_PRINT2("r is: %d, r2 is: %d\n", r, r2);


	i = 0;
	while (r >= item_choices[i].threshold)
	{
		i++;
	}

	tval = item_choices[i].tval;
	LOG_PRINT(format("Creating item: %s\n", item_choices[i].report));


	switch (tval)
	{
	case TV_BOW:
		if (r2 <10) sval = SV_SLING;
		else if (r2 < 30) sval = SV_SHORT_BOW;
		else if (r2 < 60) sval = SV_LONG_BOW;
		else if (r2 < 90) sval = SV_LIGHT_XBOW;
		else sval = SV_HEAVY_XBOW;
		break;

	case TV_DIGGING:
		if (r2 < 15) sval = SV_SHOVEL;
		else if (r2 < 30) sval = SV_PICK;
		else if (r2 < 60) sval = SV_GNOMISH_SHOVEL;
		else if (r2 < 90) sval = SV_ORCISH_PICK;
		else if (r2 < 120) sval = SV_DWARVEN_SHOVEL;
		else sval = SV_DWARVEN_PICK;
		break;

	case TV_HAFTED:
		if (r2 < 3) sval = SV_WHIP;
		else if (r2 < 5) sval = SV_BATON;
		else if (r2 < 8) sval = SV_MACE;
		else if (r2 < 15) sval = SV_WAR_HAMMER;
		else if (r2 < 22) sval = SV_QUARTERSTAFF;
		else if (r2 < 28) sval = SV_LUCERN_HAMMER;
		else if (r2 < 35) sval = SV_MORNING_STAR;
		else if (r2 < 45) sval = SV_FLAIL;
		else if (r2 < 60) sval = SV_LEAD_FILLED_MACE;
		else if (r2 < 80) sval = SV_BALL_AND_CHAIN;
		else if (r2 < 100) sval = SV_TWO_HANDED_MACE;
		else if (r2 < 130) sval = SV_TWO_HANDED_FLAIL;
		else sval = SV_MACE_OF_DISRUPTION;
		break;

	case TV_SWORD:
		if (r2 < -5) sval = SV_BROKEN_DAGGER;
		else if (r2 < 0) sval = SV_BROKEN_SWORD;
		else if (r2 < 4) sval = SV_DAGGER;
		else if (r2 < 8) sval = SV_MAIN_GAUCHE;
		else if (r2 < 12) sval = SV_RAPIER;	/* or at least pointy ;-) */
		else if (r2 < 15) sval = SV_SMALL_SWORD;
		else if (r2 < 18) sval = SV_SHORT_SWORD;
		else if (r2 < 22) sval = SV_SABRE;
		else if (r2 < 26) sval = SV_CUTLASS;
		else if (r2 < 30) sval = SV_TULWAR;
		else if (r2 < 35) sval = SV_BROAD_SWORD;
		else if (r2 < 41) sval = SV_LONG_SWORD;
		else if (r2 < 45) sval = SV_SCIMITAR;
		else if (r2 < 51) sval = SV_BASTARD_SWORD;
		else if (r2 < 64) sval = SV_KATANA;
		else if (r2 < 90) sval = SV_TWO_HANDED_SWORD;
		else if (r2 < 120) sval = SV_EXECUTIONERS_SWORD;
		else sval = SV_BLADE_OF_CHAOS;
		break;

	case TV_POLEARM:
		if (r2 < 3) sval = SV_DART;
		else if (r2 < 8) sval = SV_JAVELIN;
		else if (r2 < 17) sval = SV_SPEAR;
		else if (r2 < 22) sval = SV_TWO_HANDED_SPEAR;
		else if (r2 < 30) sval = SV_AWL_PIKE;
		else if (r2 < 37) sval = SV_PIKE;
		else if (r2 < 45) sval = SV_BEAKED_AXE;
		else if (r2 < 54) sval = SV_BROAD_AXE;
		else if (r2 < 64) sval = SV_BATTLE_AXE;
		else if (r2 < 75) sval = SV_GLAIVE;
		else if (r2 < 83) sval = SV_TRIDENT;
		else if (r2 < 91) sval = SV_HALBERD;
		else if (r2 < 96) sval = SV_LANCE;
		else if (r2 < 106) sval = SV_GREAT_AXE;
		else if (r2 < 115) sval = SV_SCYTHE;
		else if (r2 < 130) sval = SV_LOCHABER_AXE;
		else sval = SV_SCYTHE_OF_SLICING;
		break;

	case TV_SOFT_ARMOR:
		/* Hack - multiply r2 by 3/2 (armor has deeper base levels than other types) */
		r2 = sign(r2) * ((ABS(r2) * 3) / 2);

		/* Adjust tval, as all armour is done together */
		if (r2 < 30) tval = TV_SOFT_ARMOR;
		else if (r2 < 106) tval = TV_HARD_ARMOR;
		else tval = TV_DRAG_ARMOR;

		/* Soft stuff. */
		if (r2 < 0) sval = SV_FILTHY_RAG;
		else if (r2 < 5) sval = SV_ROBE;
		else if (r2 < 10) sval = SV_SOFT_LEATHER_ARMOR;
		else if (r2 < 15) sval = SV_SOFT_STUDDED_LEATHER;
		else if (r2 < 20) sval = SV_HARD_LEATHER_ARMOR;
		else if (r2 < 25) sval = SV_HARD_STUDDED_LEATHER;
		else if (r2 < 30) sval = SV_LEATHER_SCALE_MAIL;

		/* Hard stuff. */
		else if (r2 < 33) sval = SV_RUSTY_CHAIN_MAIL;
		else if (r2 < 38) sval = SV_METAL_SCALE_MAIL;
		else if (r2 < 44) sval = SV_CHAIN_MAIL;
		else if (r2 < 49) sval = SV_AUGMENTED_CHAIN_MAIL;
		else if (r2 < 54) sval = SV_DOUBLE_CHAIN_MAIL;
		else if (r2 < 59) sval = SV_BAR_CHAIN_MAIL;
		else if (r2 < 64) sval = SV_METAL_BRIGANDINE_ARMOUR;
		else if (r2 < 69) sval = SV_PARTIAL_PLATE_ARMOUR;
		else if (r2 < 74) sval = SV_METAL_LAMELLAR_ARMOUR;
		else if (r2 < 80) sval = SV_FULL_PLATE_ARMOUR;
		else if (r2 < 85) sval = SV_RIBBED_PLATE_ARMOUR;
		else if (r2 < 92) sval = SV_MITHRIL_CHAIN_MAIL;
		else if (r2 < 99) sval = SV_MITHRIL_PLATE_MAIL;
		else if (r2 < 106) sval = SV_ADAMANTITE_PLATE_MAIL;

		/* DSM - CC 18/8/01 */
		else if (r2 < 111) sval = SV_DRAGON_BLACK;
		else if (r2 < 116) sval = SV_DRAGON_BLUE;
		else if (r2 < 121) sval = SV_DRAGON_WHITE;
		else if (r2 < 126) sval = SV_DRAGON_RED;
		else if (r2 < 132) sval = SV_DRAGON_GREEN;
		else if (r2 < 138) sval = SV_DRAGON_MULTIHUED;
		else if (r2 < 144) sval = SV_DRAGON_ETHER;
		else if (r2 < 152) sval = SV_DRAGON_LAW;
		else if (r2 < 159) sval = SV_DRAGON_BRONZE;
		else if (r2 < 166) sval = SV_DRAGON_GOLD;
		else if (r2 < 174) sval = SV_DRAGON_CHAOS;
		else if (r2 < 183) sval = SV_DRAGON_BALANCE;
		else sval = SV_DRAGON_POWER;
		break;

	case TV_BOOTS:
		if (r2 < 15) sval = SV_PAIR_OF_SOFT_LEATHER_BOOTS;
		else if (r2 < 60) sval = SV_PAIR_OF_HARD_LEATHER_BOOTS;
		else sval = SV_PAIR_OF_METAL_SHOD_BOOTS;
		break;

	case TV_GLOVES:
		if (r2 < 15) sval = SV_SET_OF_LEATHER_GLOVES;
		else if (r2 < 60) sval = SV_SET_OF_GAUNTLETS;
		else sval = SV_SET_OF_CESTI;
		break;

	case TV_HELM:
		/* Adjust tval to handle crowns and helms here */
		if (r2 < 50) tval = TV_HELM; else tval = TV_CROWN;

		if (r2 < 9) sval = SV_HARD_LEATHER_CAP;
		else if (r2 < 20) sval = SV_METAL_CAP;
		else if (r2 < 40) sval = SV_IRON_HELM;
		else if (r2 < 50) sval = SV_STEEL_HELM;

		else if (r2 < 70) sval = SV_IRON_CROWN;
		else if (r2 < 90) sval = SV_GOLDEN_CROWN;
		else sval = SV_JEWELED_CROWN;
		break;

	case TV_SHIELD:
		if (r2 < 15) sval = SV_SMALL_LEATHER_SHIELD;
		else if (r2 < 40) sval = SV_SMALL_METAL_SHIELD;
		else if (r2 < 70) sval = SV_LARGE_LEATHER_SHIELD;
		else if (r2 < 120) sval = SV_LARGE_METAL_SHIELD;
		else sval = SV_SHIELD_OF_DEFLECTION;
		break;

	case TV_CLOAK:
		if (r2 < 100) sval = SV_CLOAK;
		else sval = SV_SHADOW_CLOAK;
		break;
	}

	k_idx = lookup_kind(tval, sval);
	k_ptr = &k_info[k_idx];
	kinds[a_idx] = k_idx;

	a_ptr->tval = k_ptr->tval;
	a_ptr->sval = k_ptr->sval;
	a_ptr->pval = k_ptr->pval;
	a_ptr->to_h = k_ptr->to_h;
	a_ptr->to_d = k_ptr->to_d;
	a_ptr->to_a = k_ptr->to_a;
	a_ptr->ac = k_ptr->ac;
	a_ptr->dd = k_ptr->dd;
	a_ptr->ds = k_ptr->ds;
	a_ptr->weight = k_ptr->weight;
	a_ptr->flags1 = k_ptr->flags1;
	a_ptr->flags2 = k_ptr->flags2;
	a_ptr->flags3 = k_ptr->flags3;
	a_ptr->flags4 = k_ptr->flags4;

	/*
	 * Dragon armor: remove activation flag.  This is because the current
	 * code doesn't support standard DSM activations for artifacts very
	 * well.  If it gets an activation from the base artifact it will be
	 * reset later.
	 */
	if (a_ptr->tval == TV_DRAG_ARMOR)
	{
		a_ptr->flags3 &= ~TR3_ACTIVATE;
		a_ptr->pval = 0;
	}

	/* Artifacts ignore everything */
	a_ptr->flags2 |= TR2_IGNORE_MASK;

	/* Assign basic stats to the artifact based on its artifact level. */
	/*
	 * CR, 2001-09-03: changed to a simpler version to match the hit-dam
	 * parsing algorithm. We use random ranges averaging mean_hit_startval
	 * and mean_dam_startval, but permitting variation of 50% to 150%.
	 * Level-dependent term has been removed for the moment.
	 */
	switch (a_ptr->tval)
	{
		case TV_BOW:
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_SWORD:
		case TV_POLEARM:
			a_ptr->to_h += (s16b)(mean_hit_startval / 2 +
				randint( mean_hit_startval ) );
			a_ptr->to_d += (s16b)(mean_dam_startval / 2 +
				randint( mean_dam_startval ) );
			LOG_PRINT2("Assigned basic stats, to_hit: %d, to_dam: %d\n", a_ptr->to_h, a_ptr->to_d);
			break;
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
			/* CR: adjusted this to work with parsing logic */
			a_ptr->to_a += (s16b)(mean_ac_startval / 2 +
				randint( mean_ac_startval ) );

			LOG_PRINT1("Assigned basic stats, AC bonus: %d\n", a_ptr->to_a);

			break;
	}

	/* Done - return the index of the new object kind. */
	return k_idx;
}


/*
 * We've just added an ability which uses the pval bonus.  Make sure it's
 * not zero.  If it's currently negative, leave it negative (heh heh).
 */
static void do_pval(artifact_type *a_ptr)
{
	int factor = 1;
	/* Track whether we have blows, might or shots on this item */
	if (a_ptr->flags1 & TR1_BLOWS) factor++;
	if (a_ptr->flags1 & TR1_MIGHT) factor++;
	if (a_ptr->flags1 & TR1_SHOTS) factor++;

	if (a_ptr->pval == 0)
	{
		/* Blows, might, shots handled separately */
		if (factor > 1)
		{
			a_ptr->pval = (s16b)(1 + rand_int(2));
			/* Give it a shot at +3 */
			if (rand_int(INHIBIT_STRONG) == 0) a_ptr->pval = 3;
		}
		else a_ptr->pval = (s16b)(1 + rand_int(5));
		LOG_PRINT1("Assigned initial pval, value is: %d\n", a_ptr->pval);
	}
	else if (a_ptr->pval < 0)
	{
		if (rand_int(2) == 0)
		{
			a_ptr->pval--;
			LOG_PRINT1("Decreasing pval by 1, new value is: %d\n", a_ptr->pval);
		}
	}
	else if (rand_int(a_ptr->pval * factor) == 0)
	{
		/*
		 * CR: made this a bit rarer and diminishing with higher pval -
		 * also rarer if item has blows/might/shots already
		 */
		a_ptr->pval++;
		LOG_PRINT1("Increasing pval by 1, new value is: %d\n", a_ptr->pval);
	}
}


/*
 * Adjust the parsed frequencies for any peculiarities of the
 * algorithm.  For example, if stat bonuses and sustains are
 * being added in a correlated fashion, it will tend to push
 * the frequencies up for both of them.  In this method we
 * compensate for cases like this by applying corrective
 * scaling.
 */

static void adjust_freqs()
{
	/*
	 * Enforce minimum values for any frequencies that might potentially
	 * be missing in the standard set, especially supercharged ones.
	 * Numbers here represent the average number of times this ability
	 * would appear if the entire randart set was eligible to receive
	 * it (so in the case of a bow ability: if the set was all bows).
	 *
	 * Note that low numbers here for very specialized abilities could
	 * mean that there's a good chance this ability will not appear in
	 * a given randart set.  If this is a problem, raise the number.
	 */
	if (artprobs[ART_IDX_GEN_RFEAR] < 5)
		artprobs[ART_IDX_GEN_RFEAR] = 5;
	if (artprobs[ART_IDX_MELEE_DICE_SUPER] < 5)
		artprobs[ART_IDX_MELEE_DICE_SUPER] = 5;
	if (artprobs[ART_IDX_BOW_SHOTS_SUPER] < 5)
		artprobs[ART_IDX_BOW_SHOTS_SUPER] = 5;
	if (artprobs[ART_IDX_BOW_MIGHT_SUPER] < 5)
		artprobs[ART_IDX_BOW_MIGHT_SUPER] = 5;
	if (artprobs[ART_IDX_GEN_SPEED_SUPER] < 2)
		artprobs[ART_IDX_GEN_SPEED_SUPER] = 2;
	if (artprobs[ART_IDX_GEN_AC] < 5)
		artprobs[ART_IDX_GEN_AC] = 5;
	if (artprobs[ART_IDX_GEN_TUNN] < 5)
		artprobs[ART_IDX_GEN_TUNN] = 5;

	/* Cut aggravation frequencies in half since they're used twice */
	artprobs[ART_IDX_NONWEAPON_AGGR] /= 2;
	artprobs[ART_IDX_WEAPON_AGGR] /= 2;
}

/*
 * Parse the list of artifacts and count up the frequencies of the various
 * abilities.  This is used to give dynamic generation probabilities.
 */

static void parse_frequencies ()
{
	int i;
	const artifact_type *a_ptr;
	object_kind *k_ptr;
	s32b temp;
	s16b k_idx;


	LOG_PRINT("\n****** BEGINNING GENERATION OF FREQUENCIES\n\n");

	/* Zero the frequencies */

	for(i = 0; i < ART_IDX_TOTAL; i++)
	{
		artprobs[i] = 0;
	}

	/* Go through the list of all artifacts */

	for(i = 0; i < z_info->a_max; i++)
	{

		LOG_PRINT1("Current artifact index is %d\n", i);

		a_ptr = &a_info[i];

		/* Special cases -- don't parse these! */
		if ((i == ART_POWER) ||
			(i == ART_GROND) ||
			(i == ART_MORGOTH))
			continue;

		/* Also don't parse cursed items */
		if (base_power[i] < 0) continue;

		/* Get a pointer to the base item for this artifact */
		k_idx = kinds[i];
		k_ptr = &k_info[k_idx];

		/* Count up the abilities for this artifact */

		if (a_ptr->tval == TV_BOW)
		{
			if (a_ptr->flags1 & TR1_SHOTS)
			{
				/* Do we have 3 or more extra shots? (Unlikely) */
				if(a_ptr->pval > 2)
				{
					LOG_PRINT("Adding 1 for supercharged shots (3 or more!)\n");

					(artprobs[ART_IDX_BOW_SHOTS_SUPER])++;
				}
				else {
					LOG_PRINT("Adding 1 for extra shots\n");

					(artprobs[ART_IDX_BOW_SHOTS])++;
				}
			}

			if (a_ptr->flags1 & TR1_MIGHT)
			{
				/* Do we have 3 or more extra might? (Unlikely) */
				if(a_ptr->pval > 2)
				{
					LOG_PRINT("Adding 1 for extra shots\n");

					(artprobs[ART_IDX_BOW_MIGHT_SUPER])++;
				}
				else {
					LOG_PRINT("Adding 1 for extra might\n");

					(artprobs[ART_IDX_BOW_MIGHT])++;
				}
			}

			if (a_ptr->flags3 & 0x00300F00)
			{
				/* We have some sensing - count them */
				temp = 0;
				if (a_ptr->flags3 & TR3_ESP_ORC) temp++;
				if (a_ptr->flags3 & TR3_ESP_TROLL) temp++;
				if (a_ptr->flags3 & TR3_ESP_GIANT) temp++;
				if (a_ptr->flags3 & TR3_ESP_DRAGON) temp++;
				if (a_ptr->flags3 & TR3_ESP_DEMON) temp++;
				if (a_ptr->flags3 & TR3_ESP_UNDEAD) temp++;
				if (a_ptr->flags3 & TR3_ESP_NATURE) temp++;

				/* Add these to the frequency count */
				artprobs[ART_IDX_BOW_SENSE] += temp;

				LOG_PRINT1("Adding %d instances of sensing for bows\n", temp);
			}

			if (a_ptr->flags4 & 0x07FF0000)
			{
				/* We have some restrictions - count them */
				temp = 0;
				if (a_ptr->flags4 & TR4_ANIMAL) temp++;
				if (a_ptr->flags4 & TR4_EVIL) temp++;
				if (a_ptr->flags4 & TR4_UNDEAD) temp++;
				if (a_ptr->flags4 & TR4_DEMON) temp++;
				if (a_ptr->flags4 & TR4_ORC) temp++;
				if (a_ptr->flags4 & TR4_TROLL) temp++;
				if (a_ptr->flags4 & TR4_GIANT) temp++;
				if (a_ptr->flags4 & TR4_DRAGON) temp++;
				if (a_ptr->flags4 & TR4_MAN) temp++;
				if (a_ptr->flags4 & TR4_DWARF) temp++;
				if (a_ptr->flags4 & TR4_ELF) temp++;

				/* Add these to the frequency count */
				artprobs[ART_IDX_BOW_RESTRICT] += temp;

				LOG_PRINT1("Adding %d restrictions for bows\n", temp);
			}		
		}

		/* Handle hit / dam ratings - are they higher than normal? */
		/* Also handle other weapon/nonweapon abilities */
		if (a_ptr->tval == TV_BOW || a_ptr->tval == TV_DIGGING ||
			a_ptr->tval == TV_HAFTED || a_ptr->tval == TV_POLEARM ||
			a_ptr->tval == TV_SWORD)
		{
			if (a_ptr->to_h - k_ptr->to_h - mean_hit_startval > 0)
			{
				temp = (a_ptr->to_d - k_ptr->to_d - mean_dam_startval) /
					mean_dam_increment;
				if (temp > 0)
				{
					LOG_PRINT1("Adding %d instances of extra to-hit bonus for weapon\n", temp);

					(artprobs[ART_IDX_WEAPON_HIT]) += temp;
				}
			}
			else if (a_ptr->to_h - k_ptr->to_h - mean_hit_startval < 0)
			{
				temp = ( -(a_ptr->to_d - k_ptr->to_d - mean_dam_startval) ) /
					mean_dam_increment;
				if (temp > 0)
				{
					LOG_PRINT1("Subtracting %d instances of extra to-hit bonus for weapon\n", temp);

					(artprobs[ART_IDX_WEAPON_HIT]) -= temp;
				}
			}
			if (a_ptr->to_d - k_ptr->to_d - mean_dam_startval > 0)
			{
				temp = (a_ptr->to_d - k_ptr->to_d - mean_dam_startval) /
					mean_dam_increment;
				if (temp > 0)
				{
					LOG_PRINT1("Adding %d instances of extra to-dam bonus for weapon\n", temp);

					(artprobs[ART_IDX_WEAPON_DAM]) += temp;
				}
			}
			else if (a_ptr->to_d - k_ptr->to_d - mean_dam_startval < 0)
			{
				temp = ( -(a_ptr->to_d - k_ptr->to_d - mean_dam_startval)) /
					mean_dam_increment;
				if (temp > 0)
				{
					LOG_PRINT1("Subtracting %d instances of extra to-dam bonus for weapon\n", temp);

					(artprobs[ART_IDX_WEAPON_DAM]) -= temp;
				}
			}

			/* Aggravation */
			if (a_ptr->flags3 & TR3_AGGRAVATE)
			{
				LOG_PRINT("Adding 1 for aggravation - weapon\n");
				(artprobs[ART_IDX_WEAPON_AGGR])++;
			}

			/* End weapon stuff */
		}
		else
		{
			if (a_ptr->to_h - k_ptr->to_h > 0)
			{
				temp = (a_ptr->to_d - k_ptr->to_d) / mean_dam_increment;
				if (temp > 0)
				{
					LOG_PRINT1("Adding %d instances of extra to-hit bonus for non-weapon\n", temp);

					(artprobs[ART_IDX_NONWEAPON_HIT]) += temp;
				}
			}
			if (a_ptr->to_d - k_ptr->to_d > 0)
			{
				temp = (a_ptr->to_d - k_ptr->to_d) / mean_dam_increment;
				if (temp > 0)
				{
					LOG_PRINT1("Adding %d instances of extra to-dam bonus for non-weapon\n", temp);

					(artprobs[ART_IDX_NONWEAPON_DAM]) += temp;
				}
			}
			/* Aggravation */
			if (a_ptr->flags3 & TR3_AGGRAVATE)
			{
				LOG_PRINT("Adding 1 for aggravation - nonweapon\n");
				(artprobs[ART_IDX_NONWEAPON_AGGR])++;
			}

			if (a_ptr->flags3 & 0x00300F00)
			{
				/* We have some sensing - count them */
				temp = 0;
				if (a_ptr->flags3 & TR3_ESP_ORC) temp++;
				if (a_ptr->flags3 & TR3_ESP_TROLL) temp++;
				if (a_ptr->flags3 & TR3_ESP_GIANT) temp++;
				if (a_ptr->flags3 & TR3_ESP_DRAGON) temp++;
				if (a_ptr->flags3 & TR3_ESP_DEMON) temp++;
				if (a_ptr->flags3 & TR3_ESP_UNDEAD) temp++;
				if (a_ptr->flags3 & TR3_ESP_NATURE) temp++;

				LOG_PRINT1("Adding %d instances of sensing for nonweapons\n", temp);

				/* Add these to the frequency count */
				artprobs[ART_IDX_NONWEAPON_SENSE] += temp;
			}

			if ((a_ptr->flags1 & 0x00FF0000) || (a_ptr->flags4 & 0x07FF0E00))
			{
				/* We have some restrictions - count them */
				/* Note that the only reason slay flags are on non-weapons is for restrictions */

				temp = 0;
				if (a_ptr->flags1 & TR1_SLAY_NATURAL) temp++;
				if (a_ptr->flags1 & TR1_SLAY_EVIL) temp++;
				if (a_ptr->flags1 & TR1_SLAY_UNDEAD) temp++;
				if (a_ptr->flags1 & TR1_SLAY_DEMON) temp++;
				if (a_ptr->flags1 & TR1_SLAY_ORC) temp++;
				if (a_ptr->flags1 & TR1_SLAY_TROLL) temp++;
				if (a_ptr->flags1 & TR1_SLAY_GIANT) temp++;
				if (a_ptr->flags1 & TR1_SLAY_DRAGON) temp++;
				if (a_ptr->flags4 & TR4_SLAY_MAN) temp++;
				if (a_ptr->flags4 & TR4_SLAY_ELF) temp++;
				if (a_ptr->flags4 & TR4_SLAY_DWARF) temp++;
				if (a_ptr->flags4 & TR4_ANIMAL) temp++;
				if (a_ptr->flags4 & TR4_EVIL) temp++;
				if (a_ptr->flags4 & TR4_UNDEAD) temp++;
				if (a_ptr->flags4 & TR4_DEMON) temp++;
				if (a_ptr->flags4 & TR4_ORC) temp++;
				if (a_ptr->flags4 & TR4_TROLL) temp++;
				if (a_ptr->flags4 & TR4_GIANT) temp++;
				if (a_ptr->flags4 & TR4_DRAGON) temp++;
				if (a_ptr->flags4 & TR4_MAN) temp++;
				if (a_ptr->flags4 & TR4_DWARF) temp++;
				if (a_ptr->flags4 & TR4_ELF) temp++;

				LOG_PRINT1("Adding %d restrictions for nonweapons\n", temp);

				/* Add these to the frequency count */
				artprobs[ART_IDX_NONWEAPON_RESTRICT] += temp;
			}
		}

		if (a_ptr->tval == TV_DIGGING || a_ptr->tval == TV_HAFTED ||
			a_ptr->tval == TV_POLEARM || a_ptr->tval == TV_SWORD)
		{
			/* Blessed weapon Y/N */

			if(a_ptr->flags3 & TR3_BLESSED)
			{
				LOG_PRINT("Adding 1 for blessed weapon\n");

				(artprobs[ART_IDX_MELEE_BLESS])++;
			}

			/*
			 * Brands or slays - count all together
			 * We will need to add something here unless the weapon has
			 * nothing at all
			 */

			if ((a_ptr->flags1 & 0xFFFF0000) || (a_ptr->flags4 & 0x00000E03))
			{
				/* We have some brands or slays - count them */

				temp = 0;
				if (a_ptr->flags1 & TR1_SLAY_EVIL) temp++;
				if (a_ptr->flags1 & TR1_KILL_DRAGON) temp++;
				if (a_ptr->flags1 & TR1_KILL_DEMON) temp++;
				if (a_ptr->flags1 & TR1_KILL_UNDEAD) temp++;
				if (a_ptr->flags1 & TR1_SLAY_NATURAL) temp++;
				if (a_ptr->flags1 & TR1_SLAY_UNDEAD) temp++;
				if (a_ptr->flags1 & TR1_SLAY_DRAGON) temp++;
				if (a_ptr->flags1 & TR1_SLAY_DEMON) temp++;
				if (a_ptr->flags1 & TR1_SLAY_TROLL) temp++;
				if (a_ptr->flags1 & TR1_SLAY_ORC) temp++;
				if (a_ptr->flags1 & TR1_SLAY_GIANT) temp++;
				if (a_ptr->flags1 & TR1_BRAND_ACID) temp++;
				if (a_ptr->flags1 & TR1_BRAND_ELEC) temp++;
				if (a_ptr->flags1 & TR1_BRAND_FIRE) temp++;
				if (a_ptr->flags1 & TR1_BRAND_COLD) temp++;

				if (a_ptr->flags4 & TR4_BRAND_LITE) temp++;
				if (a_ptr->flags4 & TR4_BRAND_DARK) temp++;

				if (a_ptr->flags4 & TR4_SLAY_MAN) temp++;
				if (a_ptr->flags4 & TR4_SLAY_ELF) temp++;
				if (a_ptr->flags4 & TR4_SLAY_DWARF) temp++;

				LOG_PRINT1("Adding %d for slays and brands\n", temp);

				/* Add these to the frequency count */
				artprobs[ART_IDX_MELEE_BRAND_SLAY] += temp;
			}

			if (a_ptr->flags3 & 0x00300F00)
			{
				/* We have some sensing - count them */
				temp = 0;
				if (a_ptr->flags3 & TR3_ESP_ORC) temp++;
				if (a_ptr->flags3 & TR3_ESP_TROLL) temp++;
				if (a_ptr->flags3 & TR3_ESP_GIANT) temp++;
				if (a_ptr->flags3 & TR3_ESP_DRAGON) temp++;
				if (a_ptr->flags3 & TR3_ESP_DEMON) temp++;
				if (a_ptr->flags3 & TR3_ESP_UNDEAD) temp++;
				if (a_ptr->flags3 & TR3_ESP_NATURE) temp++;

				LOG_PRINT1("Adding %d instances of sensing for melee weapons\n", temp);

				/* Add these to the frequency count */
				artprobs[ART_IDX_MELEE_SENSE] += temp;
			}

			if (a_ptr->flags4 & 0x07FF0000)
			{
				/* We have some restrictions - count them */
				temp = 0;
				if (a_ptr->flags4 & TR4_ANIMAL) temp++;
				if (a_ptr->flags4 & TR4_EVIL) temp++;
				if (a_ptr->flags4 & TR4_UNDEAD) temp++;
				if (a_ptr->flags4 & TR4_DEMON) temp++;
				if (a_ptr->flags4 & TR4_ORC) temp++;
				if (a_ptr->flags4 & TR4_TROLL) temp++;
				if (a_ptr->flags4 & TR4_GIANT) temp++;
				if (a_ptr->flags4 & TR4_DRAGON) temp++;
				if (a_ptr->flags4 & TR4_MAN) temp++;
				if (a_ptr->flags4 & TR4_DWARF) temp++;
				if (a_ptr->flags4 & TR4_ELF) temp++;

				LOG_PRINT1("Adding %d restrictions for melee weapons\n", temp);

				/* Add these to the frequency count */
				artprobs[ART_IDX_MELEE_RESTRICT] += temp;
			}

			/* See invisible? */
			if(a_ptr->flags3 & TR3_SEE_INVIS)
			{
				LOG_PRINT("Adding 1 for see invisible (weapon case)\n");

				(artprobs[ART_IDX_MELEE_SINV])++;
			}

			/* Does this weapon have extra blows? */
			if (a_ptr->flags1 & TR1_BLOWS)
			{
				LOG_PRINT("Adding 1 for extra blows\n");

				(artprobs[ART_IDX_MELEE_BLOWS])++;
			}

			/* Does this weapon have an unusual bonus to AC? */
			if ( (a_ptr->to_a - k_ptr->to_a) > 0)
			{
				temp = (a_ptr->to_a - k_ptr->to_a) / mean_ac_increment;
				if (temp > 0)
				{
					LOG_PRINT1("Adding %d instances of extra AC bonus for weapon\n", temp);

					(artprobs[ART_IDX_MELEE_AC]) += temp;
				}
			}

			/* Check damage dice - are they more than normal? */
			if (a_ptr->dd > k_ptr->dd)
			{
				/* Difference of 3 or more? */
				if ( (a_ptr->dd - k_ptr->dd) > 2)
				{
					LOG_PRINT("Adding 1 for super-charged damage dice!\n");

					(artprobs[ART_IDX_MELEE_DICE_SUPER])++;
				}
				else
				{
					LOG_PRINT("Adding 1 for extra damage dice.\n");

					(artprobs[ART_IDX_MELEE_DICE])++;
				}
			}

			/* Check weight - is it different from normal? */
			if (a_ptr->weight != k_ptr->weight)
			{
				LOG_PRINT("Adding 1 for unusual weight.\n");

				(artprobs[ART_IDX_MELEE_WEIGHT])++;
			}

			/* Check for tunnelling ability */
			if (a_ptr->flags1 & TR1_TUNNEL)
			{
				LOG_PRINT("Adding 1 for tunnelling bonus.\n");

				(artprobs[ART_IDX_MELEE_TUNN])++;
			}

			/* End of weapon-specific stuff */
		}
		else
		{
			/* Check for tunnelling ability */
			if (a_ptr->flags1 & TR1_TUNNEL)
			{
				LOG_PRINT("Adding 1 for tunnelling bonus - general.\n");

				(artprobs[ART_IDX_GEN_TUNN])++;
			}

		}
		/*
		 * Count up extra AC bonus values.
		 * Could also add logic to subtract for lower values here, but it's
		 * probably not worth the trouble since it's so rare.
		 */

		if ( (a_ptr->to_a - k_ptr->to_a - mean_ac_startval) > 0)
		{
			temp = (a_ptr->to_a - k_ptr->to_a - mean_ac_startval) /
				mean_ac_increment;
			if (temp > 0)
			{
				if (a_ptr->tval == TV_BOOTS)
				{
					LOG_PRINT1("Adding %d for AC bonus - boots\n", temp);
					(artprobs[ART_IDX_BOOT_AC]) += temp;
				}
				else if (a_ptr->tval == TV_GLOVES)
				{
					LOG_PRINT1("Adding %d for AC bonus - gloves\n", temp);
					(artprobs[ART_IDX_GLOVE_AC]) += temp;
				}
				else if (a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN)
				{
					LOG_PRINT1("Adding %d for AC bonus - headgear\n", temp);
					(artprobs[ART_IDX_HELM_AC]) += temp;
				}
				else if (a_ptr->tval == TV_SHIELD)
				{
					LOG_PRINT1("Adding %d for AC bonus - shield\n", temp);
					(artprobs[ART_IDX_SHIELD_AC]) += temp;
				}
				else if (a_ptr->tval == TV_CLOAK)
				{
					LOG_PRINT1("Adding %d for AC bonus - cloak\n", temp);
					(artprobs[ART_IDX_CLOAK_AC]) += temp;
				}
				else if (a_ptr->tval == TV_SOFT_ARMOR || a_ptr->tval == TV_HARD_ARMOR ||
					a_ptr->tval == TV_DRAG_ARMOR)
				{
					LOG_PRINT1("Adding %d for AC bonus - body armor\n", temp);
					(artprobs[ART_IDX_ARMOR_AC]) += temp;
				}
				else
				{
					LOG_PRINT1("Adding %d for AC bonus - general\n", temp);
					(artprobs[ART_IDX_GEN_AC]) += temp;
				}
			}
		}

		/* Generic armor abilities */
		if ( a_ptr->tval == TV_BOOTS || a_ptr->tval == TV_GLOVES ||
			a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN ||
			a_ptr->tval == TV_SHIELD || a_ptr->tval == TV_CLOAK ||
			a_ptr->tval == TV_SOFT_ARMOR || a_ptr->tval == TV_HARD_ARMOR ||
			a_ptr->tval == TV_DRAG_ARMOR)
		{
			/* Check weight - is it different from normal? */
			if (a_ptr->weight != k_ptr->weight)
			{
				LOG_PRINT("Adding 1 for unusual weight.\n");

				(artprobs[ART_IDX_ALLARMOR_WEIGHT])++;
			}

			/* Done with generic armor abilities */
		}

		/*
		 * General abilities.  This section requires a bit more work
		 * than the others, because we have to consider cases where
		 * a certain ability might be found in a particular item type.
		 * For example, ESP is commonly found on headgear, so when
		 * we count ESP we must add it to either the headgear or
		 * general tally, depending on the base item.  This permits
		 * us to have general abilities appear more commonly on a
		 * certain item type.
		 */

		if ( (a_ptr->flags1 & TR1_STR) || (a_ptr->flags1 & TR1_INT) ||
			(a_ptr->flags1 & TR1_WIS) || (a_ptr->flags1 & TR1_DEX) ||
			(a_ptr->flags1 & TR1_CON) || (a_ptr->flags1 & TR1_CHR) )
		{
			/* Stat bonus case.  Add up the number of individual bonuses */

			temp = 0;
			if (a_ptr->flags1 & TR1_STR) temp++;
			if (a_ptr->flags1 & TR1_INT) temp++;
			if (a_ptr->flags1 & TR1_WIS) temp++;
			if (a_ptr->flags1 & TR1_DEX) temp++;
			if (a_ptr->flags1 & TR1_CON) temp++;
			if (a_ptr->flags1 & TR1_CHR) temp++;

			/* Handle a few special cases separately. */

			if((a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN) &&
				((a_ptr->flags1 & TR1_WIS) || (a_ptr->flags1 & TR1_INT)))
			{
				/* Handle WIS and INT on helms and crowns */
				if(a_ptr->flags1 & TR1_WIS)
				{
					LOG_PRINT("Adding 1 for WIS bonus on headgear.\n");

					(artprobs[ART_IDX_HELM_WIS])++;
					/* Counted this one separately so subtract it here */
					temp--;
				}
				if(a_ptr->flags1 & TR1_INT)
				{
					LOG_PRINT("Adding 1 for INT bonus on headgear.\n");

					(artprobs[ART_IDX_HELM_INT])++;
					/* Counted this one separately so subtract it here */
					temp--;
				}
			}
			else if ((a_ptr->tval == TV_SOFT_ARMOR ||
				a_ptr->tval == TV_HARD_ARMOR ||
				a_ptr->tval == TV_DRAG_ARMOR) && a_ptr->flags1 & TR1_CON)
			{
				/* Handle CON bonus on armor */
				LOG_PRINT("Adding 1 for CON bonus on body armor.\n");

				(artprobs[ART_IDX_ARMOR_CON])++;
				/* Counted this one separately so subtract it here */
				temp--;
			}
			else if ((a_ptr->tval == TV_GLOVES) && (a_ptr->flags1 & TR1_DEX))
			{
				/* Handle DEX bonus on gloves */
				LOG_PRINT("Adding 1 for DEX bonus on gloves.\n");

				(artprobs[ART_IDX_GLOVE_DEX])++;
				/* Counted this one separately so subtract it here */
				temp--;
			}

			/* Now the general case */

			if (temp > 0)
			{
				/* There are some bonuses that weren't handled above */
				LOG_PRINT1("Adding %d for stat bonuses - general.\n", temp);

				(artprobs[ART_IDX_GEN_STAT]) += temp;

			/* Done with stat bonuses */
			}
		}

		if ( (a_ptr->flags2 & TR2_SUST_STR) || (a_ptr->flags2 & TR2_SUST_INT) ||
			(a_ptr->flags2 & TR2_SUST_WIS) || (a_ptr->flags2 & TR2_SUST_DEX) ||
			(a_ptr->flags2 & TR2_SUST_CON) || (a_ptr->flags2 & TR2_SUST_CHR) )
		{
			/* Now do sustains, in a similar manner */
			temp = 0;
			if (a_ptr->flags2 & TR2_SUST_STR) temp++;
			if (a_ptr->flags2 & TR2_SUST_INT) temp++;
			if (a_ptr->flags2 & TR2_SUST_WIS) temp++;
			if (a_ptr->flags2 & TR2_SUST_DEX) temp++;
			if (a_ptr->flags2 & TR2_SUST_CON) temp++;
			if (a_ptr->flags2 & TR2_SUST_CHR) temp++;

			LOG_PRINT1("Adding %d for stat sustains.\n", temp);

			(artprobs[ART_IDX_GEN_SUST]) += temp;
		}

		if (a_ptr->flags1 & TR1_SAVE)
		{
			/* General case */
			LOG_PRINT("Adding 1 for spell resistance bonus - general.\n");

			(artprobs[ART_IDX_GEN_SAVE])++;

			/* Done with magical devices */
		}

		if (a_ptr->flags1 & TR1_DEVICE)
		{
			/* General case */
			LOG_PRINT("Adding 1 for magical device bonus - general.\n");

			(artprobs[ART_IDX_GEN_DEVICE])++;

			/* Done with magical devices */
		}


		if (a_ptr->flags1 & TR1_STEALTH)
		{
			/* Handle stealth, including a couple of special cases */
			if(a_ptr->tval == TV_BOOTS)
			{
				LOG_PRINT("Adding 1 for stealth bonus on boots.\n");

				(artprobs[ART_IDX_BOOT_STEALTH])++;
			}
			else if (a_ptr->tval == TV_CLOAK)
			{
				LOG_PRINT("Adding 1 for stealth bonus on cloak.\n");

				(artprobs[ART_IDX_CLOAK_STEALTH])++;
			}
			else if (a_ptr->tval == TV_SOFT_ARMOR ||
				a_ptr->tval == TV_HARD_ARMOR || a_ptr->tval == TV_DRAG_ARMOR)
			{
				LOG_PRINT("Adding 1 for stealth bonus on armor.\n");

				(artprobs[ART_IDX_ARMOR_STEALTH])++;
			}
			else
			{
				/* General case */
				LOG_PRINT("Adding 1 for stealth bonus - general.\n");

				(artprobs[ART_IDX_GEN_STEALTH])++;
			}
			/* Done with stealth */
		}

		if (a_ptr->flags1 & TR1_SEARCH)
		{
			/* Handle searching bonus - fully generic this time */
			LOG_PRINT("Adding 1 for search bonus - general.\n");

			(artprobs[ART_IDX_GEN_SEARCH])++;
		}

		if (a_ptr->flags1 & TR1_INFRA)
		{
			/* Handle infravision bonus - fully generic */
			LOG_PRINT("Adding 1 for infravision bonus - general.\n");

			(artprobs[ART_IDX_GEN_INFRA])++;
		}

		if (a_ptr->flags1 & TR1_SPEED)
		{
			/*
			 * Speed - boots handled separately.
			 * This is something of a special case in that we use the same
			 * frequency for the supercharged value and the normal value.
			 * We get away with this by using a somewhat lower average value
			 * for the supercharged ability than in the basic set (around
			 * +7 or +8 - c.f. Ringil and the others at +10 and upwards).
			 * This then allows us to add an equal number of
			 * small bonuses around +3 or so without unbalancing things.
			 */

			if (a_ptr->pval > 6)
			{
				/* Supercharge case */
				LOG_PRINT("Adding 1 for supercharged speed bonus!\n");

				(artprobs[ART_IDX_GEN_SPEED_SUPER])++;
			}
			else if(a_ptr->tval == TV_BOOTS)
			{
				/* Handle boots separately */
				LOG_PRINT("Adding 1 for normal speed bonus on boots.\n");

				(artprobs[ART_IDX_BOOT_SPEED])++;
			}
			else
			{
				LOG_PRINT("Adding 1 for normal speed bonus - general.\n");

				(artprobs[ART_IDX_GEN_SPEED])++;
			}
			/* Done with speed */
		}

		if ( (a_ptr->flags2 & TR2_IM_ACID) || (a_ptr->flags2 & TR2_IM_ELEC) ||
			(a_ptr->flags2 & TR2_IM_FIRE) || (a_ptr->flags2 & TR2_IM_COLD) )
		{
			/* Count up immunities for this item, if any */
			temp = 0;
			if (a_ptr->flags2 & TR2_IM_ACID) temp++;
			if (a_ptr->flags2 & TR2_IM_ELEC) temp++;
			if (a_ptr->flags2 & TR2_IM_FIRE) temp++;
			if (a_ptr->flags2 & TR2_IM_COLD) temp++;

			LOG_PRINT1("Adding %d for immunities.\n", temp);

			(artprobs[ART_IDX_GEN_IMMUNE]) += temp;
		}

		if (a_ptr->flags3 & TR3_FREE_ACT)
		{
			/* Free action - handle gloves separately */
			if(a_ptr->tval == TV_GLOVES)
			{
				LOG_PRINT("Adding 1 for free action on gloves.\n");

				(artprobs[ART_IDX_GLOVE_FA])++;
			}
			else
			{
				LOG_PRINT("Adding 1 for free action - general.\n");

				(artprobs[ART_IDX_GEN_FA])++;
			}
		}

		if (a_ptr->flags3 & TR3_HOLD_LIFE)
		{
			/* Hold life - do body armor separately */
			if( (a_ptr->tval == TV_SOFT_ARMOR) || (a_ptr->tval == TV_HARD_ARMOR) ||
				(a_ptr->tval == TV_DRAG_ARMOR))
			{
				LOG_PRINT("Adding 1 for hold life on armor.\n");

				(artprobs[ART_IDX_ARMOR_HLIFE])++;
			}
			else
			{
				LOG_PRINT("Adding 1 for hold life - general.\n");

				(artprobs[ART_IDX_GEN_HLIFE])++;
			}
		}

		if (a_ptr->flags3 & TR3_FEATHER)
		{
			/* Feather fall - handle boots separately */
			if(a_ptr->tval == TV_BOOTS)
			{
				LOG_PRINT("Adding 1 for feather fall on boots.\n");

				(artprobs[ART_IDX_BOOT_FEATHER])++;
			}
			else
			{
				LOG_PRINT("Adding 1 for feather fall - general.\n");

				(artprobs[ART_IDX_GEN_FEATHER])++;
			}
		}

		if (a_ptr->flags3 & TR3_LITE)
		{
			/* Handle permanent light */
			LOG_PRINT("Adding 1 for permanent light - general.\n");

			(artprobs[ART_IDX_GEN_LITE])++;
		}

		if (a_ptr->flags3 & TR3_SEE_INVIS)
		{
			/*
			 * Handle see invisible - do helms / crowns separately
			 * (Weapons were done already so exclude them)
			 */
			if( !(a_ptr->tval == TV_DIGGING || a_ptr->tval == TV_HAFTED ||
			a_ptr->tval == TV_POLEARM || a_ptr->tval == TV_SWORD))
			{
				if (a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN)
				{
					LOG_PRINT("Adding 1 for see invisible - headgear.\n");

					(artprobs[ART_IDX_HELM_SINV])++;
				}
				else
				{
					LOG_PRINT("Adding 1 for see invisible - general.\n");

					(artprobs[ART_IDX_GEN_SINV])++;
				}
			}
		}

		if (a_ptr->flags3 & TR3_TELEPATHY)
		{
			/* ESP case.  Handle helms/crowns separately. */
			if(a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN)
			{
				LOG_PRINT("Adding 1 for ESP on headgear.\n");

				(artprobs[ART_IDX_HELM_ESP])++;
			}
			else
			{
				LOG_PRINT("Adding 1 for ESP - general.\n");

				(artprobs[ART_IDX_GEN_ESP])++;
			}
		}

		if (a_ptr->flags3 & TR3_SLOW_DIGEST)
		{
			/* Slow digestion case - generic. */
			LOG_PRINT("Adding 1 for slow digestion - general.\n");

			(artprobs[ART_IDX_GEN_SDIG])++;
		}

		if (a_ptr->flags3 & TR3_REGEN)
		{
			/* Regeneration case - generic. */
			LOG_PRINT("Adding 1 for regeneration - general.\n");

			(artprobs[ART_IDX_GEN_REGEN])++;
		}

		if ( (a_ptr->flags2 & TR2_RES_ACID) || (a_ptr->flags2 & TR2_RES_ELEC) ||
			(a_ptr->flags2 & TR2_RES_FIRE) || (a_ptr->flags2 & TR2_RES_COLD) )
		{
			/* Count up low resists (not the type, just the number) */
			temp = 0;
			if (a_ptr->flags2 & TR2_RES_ACID) temp++;
			if (a_ptr->flags2 & TR2_RES_ELEC) temp++;
			if (a_ptr->flags2 & TR2_RES_FIRE) temp++;
			if (a_ptr->flags2 & TR2_RES_COLD) temp++;

			/* Shields treated separately */
			if (a_ptr->tval == TV_SHIELD)
			{
				LOG_PRINT1("Adding %d for low resists on shield.\n", temp);

				(artprobs[ART_IDX_SHIELD_LRES]) += temp;
			}
			else if (a_ptr->tval == TV_SOFT_ARMOR ||
				a_ptr->tval == TV_HARD_ARMOR || a_ptr->tval == TV_DRAG_ARMOR)
			{
				/* Armor also treated separately */
				if (temp == 4)
				{
					/* Special case: armor has all four low resists */
					LOG_PRINT("Adding 1 for ALL LOW RESISTS on body armor.\n");

					(artprobs[ART_IDX_ARMOR_ALLRES])++;
				}
				else
				{
					/* Just tally up the resists as usual */
					LOG_PRINT1("Adding %d for low resists on body armor.\n", temp);

					(artprobs[ART_IDX_ARMOR_LRES]) += temp;
				}
			}
			else
			{
				/* General case */
				LOG_PRINT1("Adding %d for low resists - general.\n", temp);

				(artprobs[ART_IDX_GEN_LRES]) += temp;
			}

			/* Done with low resists */
		}

		/*
		 * If the item is body armor then count up all the high resists before
		 * going through them individually.  High resists are an important
		 * component of body armor so we track probability for them separately.
		 * The proportions of the high resists will be determined by the
		 * generic frequencies - this number just tracks the total.
		 */
		if (a_ptr->tval == TV_SOFT_ARMOR ||
			a_ptr->tval == TV_HARD_ARMOR || a_ptr->tval == TV_DRAG_ARMOR)
		{
			temp = 0;
			if (a_ptr->flags2 & TR2_RES_POIS) temp++;
			if (a_ptr->flags2 & TR2_RES_FEAR) temp++;
			if (a_ptr->flags2 & TR2_RES_LITE) temp++;
			if (a_ptr->flags2 & TR2_RES_DARK) temp++;
			if (a_ptr->flags2 & TR2_RES_BLIND) temp++;
			if (a_ptr->flags2 & TR2_RES_CONFU) temp++;
			if (a_ptr->flags2 & TR2_RES_SOUND) temp++;
			if (a_ptr->flags2 & TR2_RES_SHARD) temp++;
			if (a_ptr->flags2 & TR2_RES_NEXUS) temp++;
			if (a_ptr->flags2 & TR2_RES_NETHR) temp++;
			if (a_ptr->flags2 & TR2_RES_CHAOS) temp++;
			if (a_ptr->flags2 & TR2_RES_DISEN) temp++;

			LOG_PRINT1("Adding %d for high resists on body armor.\n", temp);

			(artprobs[ART_IDX_ARMOR_HRES]) += temp;
		}

		/* Now do the high resists individually */
		if (a_ptr->flags2 & TR2_RES_POIS)
		{
			/* Resist poison ability */
			LOG_PRINT("Adding 1 for resist poison - general.\n");

			(artprobs[ART_IDX_GEN_RPOIS])++;
		}

		if (a_ptr->flags2 & TR2_RES_FEAR)
		{
			/* Resist fear ability */
			LOG_PRINT("Adding 1 for resist fear - general.\n");

			(artprobs[ART_IDX_GEN_RFEAR])++;
		}

		if (a_ptr->flags2 & TR2_RES_LITE)
		{
			/* Resist light ability */
			LOG_PRINT("Adding 1 for resist light - general.\n");

			(artprobs[ART_IDX_GEN_RLITE])++;
		}

		if (a_ptr->flags2 & TR2_RES_DARK)
		{
			/* Resist dark ability */
			LOG_PRINT("Adding 1 for resist dark - general.\n");

			(artprobs[ART_IDX_GEN_RDARK])++;
		}

		if (a_ptr->flags2 & TR2_RES_BLIND)
		{
			/* Resist blind ability - helms/crowns are separate */
			if(a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN)
			{
				LOG_PRINT("Adding 1 for resist blindness - headgear.\n");

				(artprobs[ART_IDX_HELM_RBLIND])++;
			}
			else
			{
				/* General case */
				LOG_PRINT("Adding 1 for resist blindness - general.\n");

				(artprobs[ART_IDX_GEN_RBLIND])++;
			}
		}

		if (a_ptr->flags2 & TR2_RES_CONFU)
		{
			/* Resist confusion ability */
			LOG_PRINT("Adding 1 for resist confusion - general.\n");

			(artprobs[ART_IDX_GEN_RCONF])++;
		}

		if (a_ptr->flags2 & TR2_RES_SOUND)
		{
			/* Resist sound ability */
			LOG_PRINT("Adding 1 for resist sound - general.\n");

			(artprobs[ART_IDX_GEN_RSOUND])++;
		}

		if (a_ptr->flags2 & TR2_RES_SHARD)
		{
			/* Resist shards ability */
			LOG_PRINT("Adding 1 for resist shards - general.\n");

			(artprobs[ART_IDX_GEN_RSHARD])++;
		}

		if (a_ptr->flags2 & TR2_RES_NEXUS)
		{
			/* Resist nexus ability */
			LOG_PRINT("Adding 1 for resist nexus - general.\n");

			(artprobs[ART_IDX_GEN_RNEXUS])++;
		}

		if (a_ptr->flags2 & TR2_RES_NETHR)
		{
			/* Resist nether ability */
			LOG_PRINT("Adding 1 for resist nether - general.\n");

			(artprobs[ART_IDX_GEN_RNETHER])++;
		}

		if (a_ptr->flags2 & TR2_RES_CHAOS)
		{
			/* Resist chaos ability */
			LOG_PRINT("Adding 1 for resist chaos - general.\n");

			(artprobs[ART_IDX_GEN_RCHAOS])++;
		}

		if (a_ptr->flags2 & TR2_RES_DISEN)
		{
			/* Resist disenchantment ability */
			LOG_PRINT("Adding 1 for resist disenchantment - general.\n");

			(artprobs[ART_IDX_GEN_RDISEN])++;
		}
		/* Done with parsing of frequencies for this item */
	}
	/* End for loop */

	if(randart_verbose)
	{
	/* Print out some of the abilities, to make sure that everything's fine */
		for(i=0; i<ART_IDX_TOTAL; i++)
		{
			fprintf(randart_log, "Frequency of ability %d: %d\n", i, artprobs[i]);
		}
	}

	/*
	 * Rescale the abilities so that dependent / independent abilities are
	 * comparable.  We do this by rescaling the frequencies for item-dependent
	 * abilities as though the entire set was made up of that item type.  For
	 * example, if one bow out of three has extra might, and there are 120
	 * artifacts in the full set, we rescale the frequency for extra might to
	 * 40 (if we had 120 randart bows, about 40 would have extra might).
	 *
	 * This will allow us to compare the frequencies of all ability types,
	 * no matter what the dependency.  We assume that generic abilities (like
	 * resist fear in the current version) don't need rescaling.  This
	 * introduces some inaccuracy in cases where specific instances of an
	 * ability (like INT bonus on helms) have been counted separately -
	 * ideally we should adjust for this in the general case.  However, as
	 * long as this doesn't occur too often, it shouldn't be a big issue.
	 *
	 * The following loops look complicated, but they are simply equivalent
	 * to going through each of the relevant ability types one by one.
	 */

	/* Bow-only abilities */
	for (i = 0; i < ART_IDX_BOW_COUNT; i++)
	{
		artprobs[art_idx_bow[i]] = (artprobs[art_idx_bow[i]] * art_total)
			/ art_bow_total;
	}

	/* All weapon abilities */
	for (i = 0; i < ART_IDX_WEAPON_COUNT; i++)
	{
		artprobs[art_idx_weapon[i]] = (artprobs[art_idx_weapon[i]] *
			art_total) / (art_bow_total + art_melee_total);
	}

	/* Corresponding non-weapon abilities */
	temp = art_total - art_melee_total - art_bow_total;
	for (i = 0; i < ART_IDX_NONWEAPON_COUNT; i++)
	{
		artprobs[art_idx_nonweapon[i]] = (artprobs[art_idx_nonweapon[i]] *
			art_total) / temp;
	}

	/* All melee weapon abilities */
	for (i = 0; i < ART_IDX_MELEE_COUNT; i++)
	{
		artprobs[art_idx_melee[i]] = (artprobs[art_idx_melee[i]] *
			art_total) / art_melee_total;
	}

	/* All general armor abilities */
	temp = art_armor_total + art_boot_total + art_shield_total +
		art_headgear_total + art_cloak_total + art_glove_total;
	for (i = 0; i < ART_IDX_ALLARMOR_COUNT; i++)
	{
		artprobs[art_idx_allarmor[i]] = (artprobs[art_idx_allarmor[i]] *
			art_total) / temp;
	}

	/* Boots */
	for (i = 0; i < ART_IDX_BOOT_COUNT; i++)
	{
		artprobs[art_idx_boot[i]] = (artprobs[art_idx_boot[i]] *
			art_total) / art_boot_total;
	}

	/* Gloves */
	for (i = 0; i < ART_IDX_GLOVE_COUNT; i++)
	{
		artprobs[art_idx_glove[i]] = (artprobs[art_idx_glove[i]] *
			art_total) / art_glove_total;
	}

	/* Headgear */
	for (i = 0; i < ART_IDX_HELM_COUNT; i++)
	{
		artprobs[art_idx_headgear[i]] = (artprobs[art_idx_headgear[i]] *
			art_total) / art_headgear_total;
	}

	/* Shields */
	for (i = 0; i < ART_IDX_SHIELD_COUNT; i++)
	{
		artprobs[art_idx_shield[i]] = (artprobs[art_idx_shield[i]] *
			art_total) / art_shield_total;
	}

	/* Cloaks */
	for (i = 0; i < ART_IDX_CLOAK_COUNT; i++)
	{
		artprobs[art_idx_cloak[i]] = (artprobs[art_idx_cloak[i]] *
			art_total) / art_cloak_total;
	}

	/* Body armor */
	for (i = 0; i < ART_IDX_ARMOR_COUNT; i++)
	{
		artprobs[art_idx_armor[i]] = (artprobs[art_idx_armor[i]] *
			art_total) / art_armor_total;
	}

	/*
	 * All others are general case and don't need to be rescaled,
	 * unless the algorithm is getting too clever about separating
	 * out individual cases (in which case some logic should be
	 * added for them in the following method call).
	 */

	/* Perform any additional rescaling and adjustment, if required. */
	adjust_freqs();

	/* Log the final frequencies to check that everything's correct */
	for(i=0; i<ART_IDX_TOTAL; i++)
	{
		LOG_PRINT2( "Rescaled frequency of ability %d: %d\n", i, artprobs[i]);
	}

}

static bool add_str(artifact_type *a_ptr)
{
	if(a_ptr->flags1 & TR1_STR) return FALSE;
	a_ptr->flags1 |= TR1_STR;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: STR (now %+d)\n", a_ptr->pval);
	return TRUE;
}

static bool add_int(artifact_type *a_ptr)
{
	if(a_ptr->flags1 & TR1_INT) return FALSE;
	a_ptr->flags1 |= TR1_INT;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: INT (now %+d)\n", a_ptr->pval);
	return TRUE;
}

static bool add_wis(artifact_type *a_ptr)
{
	if(a_ptr->flags1 & TR1_WIS) return FALSE;
	a_ptr->flags1 |= TR1_WIS;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: WIS (now %+d)\n", a_ptr->pval);
	return TRUE;
}

static bool add_dex(artifact_type *a_ptr)
{
	if(a_ptr->flags1 & TR1_DEX) return FALSE;
	a_ptr->flags1 |= TR1_DEX;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: DEX (now %+d)\n", a_ptr->pval);
	return TRUE;
}

static bool add_con(artifact_type *a_ptr)
{
	if(a_ptr->flags1 & TR1_CON) return FALSE;
	a_ptr->flags1 |= TR1_CON;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: CON (now %+d)\n", a_ptr->pval);
	return TRUE;
}

static bool add_chr(artifact_type *a_ptr)
{
	if(a_ptr->flags1 & TR1_CHR) return FALSE;
	a_ptr->flags1 |= TR1_CHR;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: CHR (now %+d)\n", a_ptr->pval);
	return TRUE;
}

static void add_stat(artifact_type *a_ptr)
{
	int r;
	bool success = FALSE;

	/* Hack: break out if all stats are raised to avoid an infinite loop */
	if ((a_ptr->flags1 & TR1_STR) && (a_ptr->flags1 & TR1_INT) &&
		(a_ptr->flags1 & TR1_WIS) && (a_ptr->flags1 & TR1_DEX) &&
		(a_ptr->flags1 & TR1_CON) && (a_ptr->flags1 & TR1_CHR))
			return;

	/* Make sure we add one that hasn't been added yet */
	while (!success)
	{
		r = rand_int(6);
		if (r == 0) success = add_str(a_ptr);
		else if (r == 1) success = add_int(a_ptr);
		else if (r == 2) success = add_wis(a_ptr);
		else if (r == 3) success = add_dex(a_ptr);
		else if (r == 4) success = add_con(a_ptr);
		else if (r == 5) success = add_chr(a_ptr);
	}
}

static bool add_sus_str(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_SUST_STR) return FALSE;
	a_ptr->flags2 |= TR2_SUST_STR;
	LOG_PRINT("Adding ability: sustain STR\n");
	return TRUE;
}

static bool add_sus_int(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_SUST_INT) return FALSE;
	a_ptr->flags2 |= TR2_SUST_INT;
	LOG_PRINT("Adding ability: sustain INT\n");
	return TRUE;
}

static bool add_sus_wis(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_SUST_WIS) return FALSE;
	a_ptr->flags2 |= TR2_SUST_WIS;
	LOG_PRINT("Adding ability: sustain WIS\n");
	return TRUE;
}

static bool add_sus_dex(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_SUST_DEX) return FALSE;
	a_ptr->flags2 |= TR2_SUST_DEX;
	LOG_PRINT("Adding ability: sustain DEX\n");
	return TRUE;
}

static bool add_sus_con(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_SUST_CON) return FALSE;
	a_ptr->flags2 |= TR2_SUST_CON;
	LOG_PRINT("Adding ability: sustain CON\n");
	return TRUE;
}

static bool add_sus_chr(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_SUST_CHR) return FALSE;
	a_ptr->flags2 |= TR2_SUST_CHR;
	LOG_PRINT("Adding ability: sustain CHR\n");
	return TRUE;
}

static void add_sustain(artifact_type *a_ptr)
{
	int r;
	bool success = FALSE;

	/* Hack: break out if all stats are sustained to avoid an infinite loop */
	if ((a_ptr->flags2 & TR2_SUST_STR) && (a_ptr->flags2 & TR2_SUST_INT) &&
		(a_ptr->flags2 & TR2_SUST_WIS) && (a_ptr->flags2 & TR2_SUST_DEX) &&
		(a_ptr->flags2 & TR2_SUST_CON) && (a_ptr->flags2 & TR2_SUST_CHR))
			return;

	while (!success)
	{
		r = rand_int(6);
		if (r == 0) success = add_sus_str(a_ptr);
		else if (r == 1) success = add_sus_int(a_ptr);
		else if (r == 2) success = add_sus_wis(a_ptr);
		else if (r == 3) success = add_sus_dex(a_ptr);
		else if (r == 4) success = add_sus_con(a_ptr);
		else if (r == 5) success = add_sus_chr(a_ptr);
	}
}

static void add_spell_resistance(artifact_type *a_ptr)
{
	a_ptr->flags1 |= TR1_SAVE;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: spell resistance (now %+d)\n", a_ptr->pval);
}

static void add_magical_device(artifact_type *a_ptr)
{
	a_ptr->flags1 |= TR1_DEVICE;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: magical devices (now %+d)\n", a_ptr->pval);
}

static void add_stealth(artifact_type *a_ptr)
{
	a_ptr->flags1 |= TR1_STEALTH;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: stealth (now %+d)\n", a_ptr->pval);
}

static void add_search(artifact_type *a_ptr)
{
	a_ptr->flags1 |= TR1_SEARCH;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: searching (now %+d)\n", a_ptr->pval);
}

static void add_infravision(artifact_type *a_ptr)
{
	a_ptr->flags1 |= TR1_INFRA;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: infravision (now %+d)\n", a_ptr->pval);
}

static void add_tunnelling(artifact_type *a_ptr)
{
	a_ptr->flags1 |= TR1_TUNNEL;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: tunnelling (new bonus is %+d)\n", a_ptr->pval);
}

static void add_speed(artifact_type *a_ptr)
{
	a_ptr->flags1 |= TR1_SPEED;
	if (a_ptr->pval == 0)
	{
		a_ptr->pval = (s16b)(1 + rand_int(4));
		LOG_PRINT1("Adding ability: speed (first time) (now %+d)\n", a_ptr->pval);
	}
	else
	{
		do_pval(a_ptr);
		LOG_PRINT1("Adding ability: speed (now %+d)\n", a_ptr->pval);
	}
}

static void add_shots(artifact_type *a_ptr)
{
	a_ptr->flags1 |= TR1_SHOTS;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: extra shots (now %+d)\n", a_ptr->pval);
}

static void add_blows(artifact_type *a_ptr)
{
	a_ptr->flags1 |= TR1_BLOWS;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: extra blows (%d additional blows)\n", a_ptr->pval);
}

static void add_might(artifact_type *a_ptr)
{
	a_ptr->flags1 |= TR1_MIGHT;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: extra might (now %+d)\n", a_ptr->pval);
}

static bool add_resist_acid(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_ACID) return FALSE;
	a_ptr->flags2 |= TR2_RES_ACID;
	LOG_PRINT("Adding ability: resist acid\n");
	return TRUE;
}

static bool add_resist_lightning(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_ELEC) return FALSE;
	a_ptr->flags2 |= TR2_RES_ELEC;
	LOG_PRINT("Adding ability: resist lightning\n");
	return TRUE;
}

static bool add_resist_fire(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_FIRE) return FALSE;
	a_ptr->flags2 |= TR2_RES_FIRE;
	LOG_PRINT("Adding ability: resist fire\n");
	return TRUE;
}

static bool add_resist_cold(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_COLD) return FALSE;
	a_ptr->flags2 |= TR2_RES_COLD;
	LOG_PRINT("Adding ability: resist cold\n");
	return TRUE;
}

static void add_low_resist(artifact_type *a_ptr)
{
	int r;
	bool success = FALSE;

	/* Hack - if all low resists added already, exit to avoid infinite loop */
	if( (a_ptr->flags2 & TR2_RES_ACID) && (a_ptr->flags2 & TR2_RES_ELEC) &&
		(a_ptr->flags2 & TR2_RES_FIRE) && (a_ptr->flags2 & TR2_RES_COLD) )
			return;

	/* Hack -- branded weapons prefer same kinds of resist as brand */
	r = rand_int(4);
	if ((r == 0) && (a_ptr->flags1 & (TR1_BRAND_ACID))) success = add_resist_acid(a_ptr);
	else if ((r == 1) && (a_ptr->flags1 & (TR1_BRAND_ELEC))) success = add_resist_lightning(a_ptr);
	else if ((r == 2) && (a_ptr->flags1 & (TR1_BRAND_FIRE))) success = add_resist_fire(a_ptr);
	else if ((r == 3) && (a_ptr->flags1 & (TR1_BRAND_COLD))) success = add_resist_cold(a_ptr);

	while (!success)
	{
		r = rand_int(4);
		if (r == 0) success = add_resist_acid(a_ptr);
		else if (r == 1) success = add_resist_lightning(a_ptr);
		else if (r == 2) success = add_resist_fire(a_ptr);
		else if (r == 3) success = add_resist_cold(a_ptr);
	}
}

static bool add_resist_poison(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_POIS) return FALSE;
	a_ptr->flags2 |= TR2_RES_POIS;
	LOG_PRINT("Adding ability: resist poison\n");
	return TRUE;
}

static bool add_resist_fear(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_FEAR) return FALSE;
	a_ptr->flags2 |= TR2_RES_FEAR;
	LOG_PRINT("Adding ability: resist fear\n");
	return TRUE;
}

static bool add_resist_light(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_LITE) return FALSE;
	a_ptr->flags2 |= TR2_RES_LITE;
	LOG_PRINT("Adding ability: resist light\n");
	return TRUE;
}

static bool add_resist_dark(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_DARK) return FALSE;
	a_ptr->flags2 |= TR2_RES_DARK;
	LOG_PRINT("Adding ability: resist dark\n");
	return TRUE;
}

static bool add_resist_blindness(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_BLIND) return FALSE;
	a_ptr->flags2 |= TR2_RES_BLIND;
	LOG_PRINT("Adding ability: resist blindness\n");
	return TRUE;
}

static bool add_resist_confusion(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_CONFU) return FALSE;
	a_ptr->flags2 |= TR2_RES_CONFU;
	LOG_PRINT("Adding ability: resist confusion\n");
	return TRUE;
}

static bool add_resist_sound(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_SOUND) return FALSE;
	a_ptr->flags2 |= TR2_RES_SOUND;
	LOG_PRINT("Adding ability: resist sound\n");
	return TRUE;
}

static bool add_resist_shards(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_SHARD) return FALSE;
	a_ptr->flags2 |= TR2_RES_SHARD;
	LOG_PRINT("Adding ability: resist sound\n");
	return TRUE;
}

static bool add_resist_nexus(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_NEXUS) return FALSE;
	a_ptr->flags2 |= TR2_RES_NEXUS;
	LOG_PRINT("Adding ability: resist nexus\n");
	return TRUE;
}

static bool add_resist_nether(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_NETHR) return FALSE;
	a_ptr->flags2 |= TR2_RES_NETHR;
	LOG_PRINT("Adding ability: resist nether\n");
	return TRUE;
}

static bool add_resist_chaos(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_CHAOS) return FALSE;
	a_ptr->flags2 |= TR2_RES_CHAOS;
	LOG_PRINT("Adding ability: resist chaos\n");
	return TRUE;
}

static bool add_resist_disenchantment(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_DISEN) return FALSE;
	a_ptr->flags2 |= TR2_RES_DISEN;
	LOG_PRINT("Adding ability: resist chaos\n");
	return TRUE;
}

static void add_high_resist(artifact_type *a_ptr)
{
	/* Add a high resist, according to the generated frequency distribution. */
	int r, i, temp;
	int count = 0;
	bool success = FALSE;

	temp = 0;
	for (i = 0; i < ART_IDX_HIGH_RESIST_COUNT; i++)
	{
		temp += artprobs[art_idx_high_resist[i]];
	}

	/* Hack -- brands prefer same kind of resist as their brand */
	r = rand_int(3);
	if ((r == 0) && (a_ptr->flags1 & (TR1_BRAND_POIS))) success = add_resist_poison(a_ptr);
	else if ((r == 1) && (a_ptr->flags4 & (TR4_BRAND_LITE))) success = add_resist_light(a_ptr);
	else if ((r == 2) && (a_ptr->flags4 & (TR4_BRAND_DARK))) success = add_resist_dark(a_ptr);

	/* The following will fail (cleanly) if all high resists already added */
	while ( (!success) && (count < MAX_TRIES) )
	{
		/* Randomize from 1 to this total amount */
		r = randint(temp);

		/* Determine which (weighted) resist this number corresponds to */

		temp = artprobs[art_idx_high_resist[0]];
		i = 0;
		while (r > temp && i < ART_IDX_HIGH_RESIST_COUNT)
		{
			temp += artprobs[art_idx_high_resist[i]];
			i++;
		}

		/* Now i should give us the index of the correct high resist */
		if (i == 0) success = add_resist_poison(a_ptr);
		else if (i == 1) success = add_resist_fear(a_ptr);
		else if (i == 2) success = add_resist_light(a_ptr);
		else if (i == 3) success = add_resist_dark(a_ptr);
		else if (i == 4) success = add_resist_blindness(a_ptr);
		else if (i == 5) success = add_resist_confusion(a_ptr);
		else if (i == 6) success = add_resist_sound(a_ptr);
		else if (i == 7) success = add_resist_shards(a_ptr);
		else if (i == 8) success = add_resist_nexus(a_ptr);
		else if (i == 9) success = add_resist_nether(a_ptr);
		else if (i == 10) success = add_resist_chaos(a_ptr);
		else if (i == 11) success = add_resist_disenchantment(a_ptr);

		count++;
	}
}

static void add_slow_digestion(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_SLOW_DIGEST;
	LOG_PRINT("Adding ability: slow digestion\n");
}

static void add_feather_falling(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_FEATHER;
	LOG_PRINT("Adding ability: feather fall\n");
}

static void add_permanent_light(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_LITE;
	LOG_PRINT("Adding ability: permanent light\n");
}

static void add_regeneration(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_REGEN;
	LOG_PRINT("Adding ability: regeneration\n");
}

static void add_telepathy(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_TELEPATHY;
	LOG_PRINT("Adding ability: telepathy\n");
}

/* Start of ESP Add_functions ARD_ESP */
static bool add_sense_orc(artifact_type *a_ptr)
{
	if (a_ptr->flags3 & TR3_ESP_ORC) return FALSE;
	a_ptr->flags3 |= TR3_ESP_ORC;
	LOG_PRINT("Adding ability: sense orc\n");
	return (TRUE);
}

static bool add_sense_giant(artifact_type *a_ptr)
{
	if (a_ptr->flags3 & TR3_ESP_GIANT) return FALSE;
	a_ptr->flags3 |= TR3_ESP_GIANT;
	LOG_PRINT("Adding ability: sense giant\n");
	return (TRUE);
}

static bool add_sense_troll(artifact_type *a_ptr)
{
	if (a_ptr->flags3 & TR3_ESP_TROLL) return FALSE;
	a_ptr->flags3 |= TR3_ESP_TROLL;
	LOG_PRINT("Adding ability: sense troll\n");
	return (TRUE);
}

static bool add_sense_dragon(artifact_type *a_ptr)
{
	if (a_ptr->flags3 & TR3_ESP_DRAGON) return FALSE;
	a_ptr->flags3 |= TR3_ESP_DRAGON;
	LOG_PRINT("Adding ability: sense dragon\n");
	return (TRUE);
}

static bool add_sense_demon(artifact_type *a_ptr)
{
	if (a_ptr->flags3 & TR3_ESP_DEMON) return FALSE;
	a_ptr->flags3 |= TR3_ESP_DEMON;
	LOG_PRINT("Adding ability: sense demon\n");
	return (TRUE);
}		     

static bool add_sense_undead(artifact_type *a_ptr)
{
	if (a_ptr->flags3 & TR3_ESP_UNDEAD) return FALSE;
	a_ptr->flags3 |= TR3_ESP_UNDEAD;
	LOG_PRINT("Adding ability: sense undead\n");
	return (TRUE);
}

static bool add_sense_nature(artifact_type *a_ptr)
{
	if (a_ptr->flags3 & TR3_ESP_NATURE) return FALSE;
	a_ptr->flags3 |= TR3_ESP_NATURE;
	LOG_PRINT("Adding ability: sense nature\n");
	return (TRUE);
}


static void add_sense_slay(artifact_type *a_ptr)
{
	/* Pick a sense at random, as long as the weapon has the slay */
	/* Note we bias towards the high slays here */

	int r;
	int count = 0;
	bool success = FALSE;

	/* Hack -- not on telepathic items */
	if (a_ptr->flags3 & (TR3_TELEPATHY)) return;

	while ( (!success) & (count < MAX_TRIES) )
	{
		r = rand_int(10);
		if ((r == 0) && (a_ptr->flags1 & TR1_SLAY_ORC)) success = add_sense_orc(a_ptr);
		else if ((r == 1) && (a_ptr->flags1 & TR1_SLAY_GIANT)) success = add_sense_giant(a_ptr);
		else if ((r == 2) && (a_ptr->flags1 & TR1_SLAY_TROLL)) success = add_sense_troll(a_ptr);
		else if ((r == 3) && (a_ptr->flags1 & TR1_SLAY_DRAGON)) success = add_sense_dragon(a_ptr);
		else if ((r == 4) && (a_ptr->flags1 & TR1_SLAY_DEMON)) success = add_sense_demon(a_ptr);
		else if ((r == 5) && (a_ptr->flags1 & TR1_SLAY_UNDEAD)) success = add_sense_undead(a_ptr);
		else if ((r == 6) && (a_ptr->flags1 & TR1_KILL_DRAGON)) success = add_sense_dragon(a_ptr);
		else if ((r == 7) && (a_ptr->flags1 & TR1_KILL_DEMON)) success = add_sense_demon(a_ptr);
		else if ((r == 8) && (a_ptr->flags1 & TR1_KILL_UNDEAD)) success = add_sense_undead(a_ptr);
		    else if ((r == 9) && (a_ptr->flags1 & TR1_SLAY_NATURAL)) success = add_sense_nature(a_ptr);

		count++;
	}
}

static void add_sense_rand(artifact_type *a_ptr)
{
	/* Pick a sense at random */
	/* Note we bias towards the high slays here, even though per
	   * se, none of this equipment will have high-slays
	 */


	int r;
	int count = 0;
	bool success = FALSE;

	/* Hack -- not on telepathic items */
	if (a_ptr->flags3 & (TR3_TELEPATHY)) return;

	while ( (!success) & (count < MAX_TRIES) )
	{
		r = rand_int(10);
		if (r < 2) success = add_sense_dragon(a_ptr);
		else if (r < 4) success = add_sense_demon(a_ptr);
		else if (r < 6) success = add_sense_undead(a_ptr);
		else if (r == 6) success = add_sense_orc(a_ptr);
		else if (r == 7) success = add_sense_giant(a_ptr);
		else if (r == 8) success = add_sense_troll(a_ptr);
		else if (r == 9) success = add_sense_nature(a_ptr);

		count++;
	}
}
/* End of ESP add functions ARD_ESP */


static void add_see_invisible(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_SEE_INVIS;
	LOG_PRINT("Adding ability: see invisible\n");
}

static void add_free_action(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_FREE_ACT;
	LOG_PRINT("Adding ability: free action\n");
}

static void add_hold_life(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_HOLD_LIFE;
	LOG_PRINT("Adding ability: hold life\n");
}

static bool add_slay_natural(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_SLAY_NATURAL) return FALSE;
	a_ptr->flags1 |= TR1_SLAY_NATURAL;
	LOG_PRINT("Adding ability: slay animal\n");
	return TRUE;
}

static bool add_slay_evil(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_SLAY_EVIL) return FALSE;
	if (a_ptr->flags4 & (TR4_SLAY_MAN | TR4_SLAY_ELF | TR4_SLAY_DWARF)) return FALSE;
	a_ptr->flags1 |= TR1_SLAY_EVIL;
	LOG_PRINT("Adding ability: slay evil\n");
	return TRUE;
}

static bool add_slay_orc(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_SLAY_ORC) return FALSE;
	a_ptr->flags1 |= TR1_SLAY_ORC;
	LOG_PRINT("Adding ability: slay orc\n");
	return TRUE;
}

static bool add_slay_troll(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_SLAY_TROLL) return FALSE;
	a_ptr->flags1 |= TR1_SLAY_TROLL;
	LOG_PRINT("Adding ability: slay troll \n");
	return TRUE;
}

static bool add_slay_giant(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_SLAY_GIANT) return FALSE;
	a_ptr->flags1 |= TR1_SLAY_GIANT;
	LOG_PRINT("Adding ability: slay giant\n");
	return TRUE;
}

static bool add_slay_demon(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_SLAY_DEMON) return FALSE;
	a_ptr->flags1 |= TR1_SLAY_DEMON;
	LOG_PRINT("Adding ability: slay demon\n");
	return TRUE;
}

static bool add_slay_undead(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_SLAY_UNDEAD) return FALSE;
	a_ptr->flags1 |= TR1_SLAY_UNDEAD;
	LOG_PRINT("Adding ability: slay undead\n");
	return TRUE;
}

static bool add_slay_dragon(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_SLAY_DRAGON) return FALSE;
	a_ptr->flags1 |= TR1_SLAY_DRAGON;
	LOG_PRINT("Adding ability: slay dragon\n");
	return TRUE;
}

static bool add_kill_demon(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_KILL_DEMON) return FALSE;
	a_ptr->flags1 |= TR1_KILL_DEMON;
	LOG_PRINT("Adding ability: kill demon\n");
	return TRUE;
}

static bool add_kill_undead(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_KILL_UNDEAD) return FALSE;
	a_ptr->flags1 |= TR1_KILL_UNDEAD;
	LOG_PRINT("Adding ability: kill undead\n");
	return TRUE;
}

static bool add_kill_dragon(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_KILL_DRAGON) return FALSE;
	a_ptr->flags1 |= TR1_KILL_DRAGON;
	LOG_PRINT("Adding ability: kill dragon\n");
	return TRUE;
}

static bool add_acid_brand(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_BRAND_ACID) return FALSE;
	a_ptr->flags1 |= TR1_BRAND_ACID;
	LOG_PRINT("Adding ability: acid brand\n");
	return TRUE;
}

static bool add_lightning_brand(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_BRAND_ELEC) return FALSE;
	a_ptr->flags1 |= TR1_BRAND_ELEC;
	LOG_PRINT("Adding ability: lightning brand\n");
	return TRUE;
}

static bool add_fire_brand(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_BRAND_FIRE) return FALSE;
	a_ptr->flags1 |= TR1_BRAND_FIRE;
	LOG_PRINT("Adding ability: fire brand\n");
	return TRUE;
}

static bool add_frost_brand(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_BRAND_COLD) return FALSE;
	a_ptr->flags1 |= TR1_BRAND_COLD;
	LOG_PRINT("Adding ability: frost brand\n");
	return TRUE;
}

static bool add_poison_brand(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_BRAND_POIS) return FALSE;
	a_ptr->flags1 |= TR1_BRAND_POIS;
	LOG_PRINT("Adding ability: poison brand\n");
	return TRUE;
}

static bool add_lite_brand(artifact_type *a_ptr)
{
	if (a_ptr->flags4 & TR4_BRAND_LITE) return FALSE;
	a_ptr->flags4 |= TR4_BRAND_LITE;
	LOG_PRINT("Adding ability: light brand\n");
	return TRUE;
}

static bool add_dark_brand(artifact_type *a_ptr)
{
 	if (a_ptr->flags4 & TR4_BRAND_DARK) return FALSE;
	a_ptr->flags4 |= TR4_BRAND_DARK;
	LOG_PRINT("Adding ability: darkness brand\n");
	return TRUE;
}

static bool add_slay_man(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_SLAY_EVIL) return FALSE; 
	if (a_ptr->flags4 & TR4_SLAY_MAN) return FALSE;
	a_ptr->flags4 |= TR4_SLAY_MAN;
	if (rand_int(3) == 0) a_ptr->flags4 |= TR4_EVIL;
	LOG_PRINT("Adding ability: slay man\n");
	if (a_ptr->flags4 & TR4_EVIL) LOG_PRINT("Weapon is evil.");
	return TRUE;
}

static bool add_slay_elf(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_SLAY_EVIL) return FALSE; 
	if (a_ptr->flags4 & TR4_SLAY_ELF) return FALSE;
	a_ptr->flags4 |= TR4_SLAY_ELF;
	if (rand_int(3) == 0) a_ptr->flags4 |= TR4_EVIL;
	LOG_PRINT("Adding ability: slay elf\n");
	if (a_ptr->flags4 & TR4_EVIL) LOG_PRINT("Weapon is evil.");
	return TRUE;
}

static bool add_slay_dwarf(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_SLAY_EVIL) return FALSE; 
	if (a_ptr->flags4 & TR4_SLAY_DWARF) return FALSE;
	a_ptr->flags4 |= TR4_SLAY_DWARF;
	LOG_PRINT("Adding ability: slay dwarf\n");
	if (rand_int(3) == 0) a_ptr->flags4 |= TR4_EVIL;
	if (a_ptr->flags4 & TR4_EVIL) LOG_PRINT("Weapon is evil.");
	return TRUE;
}

static void add_brand_or_slay(artifact_type *a_ptr)
{
	/* Pick a brand or slay at random */

	int r;
	int count = 0;
	bool success = FALSE;

	while ( (!success) & (count < MAX_TRIES) )
	{
		r = rand_int(20);
		if (r == 0) success = add_slay_evil(a_ptr);
		else if (r == 1) success = add_kill_dragon(a_ptr);
		else if (r == 2) success = add_slay_natural(a_ptr);
		else if (r == 3) success = add_slay_undead(a_ptr);
		else if (r == 4) success = add_slay_dragon(a_ptr);
		else if (r == 5) success = add_slay_demon(a_ptr);
		else if (r == 6) success = add_slay_troll(a_ptr);
		else if (r == 7) success = add_slay_orc(a_ptr);
		else if (r == 8) success = add_slay_giant(a_ptr);
		else if (r == 9) success = add_acid_brand(a_ptr);
		else if (r == 10) success = add_lightning_brand(a_ptr);
		else if (r == 11) success = add_fire_brand(a_ptr);
		else if (r == 12) success = add_frost_brand(a_ptr);
		else if (r == 13) success = add_poison_brand(a_ptr);
		else if (r == 14) success = add_kill_demon(a_ptr);
		else if (r == 15) success = add_kill_undead(a_ptr);
		else if (r == 16) success = add_kill_dragon(a_ptr);
		else if (r == 17) success = add_lite_brand(a_ptr);
		else if (r == 18) success = add_dark_brand(a_ptr);

		/* Note: must keep this equal to slay_evil */
		else if (r == 19)
		{
			if (rand_int(3) == 0) success = add_slay_man(a_ptr);
			else if (rand_int(2) == 0) success = add_slay_elf(a_ptr);
			else success = add_slay_dwarf(a_ptr);
		}

		count++;
	}
}

static bool add_restrict_orc(artifact_type *a_ptr)
{
	if (a_ptr->flags4 & TR4_ORC) return FALSE;
	a_ptr->flags4 |= TR4_ORC;
	LOG_PRINT("Adding ability: mark as orc\n");
	return TRUE;
}

static bool add_restrict_giant(artifact_type *a_ptr)
{
	if (a_ptr->flags4 & TR4_GIANT) return FALSE;
	a_ptr->flags4 |= TR4_GIANT;
	LOG_PRINT("Adding ability: mark as giant\n");
	return TRUE;
}

static bool add_restrict_troll(artifact_type *a_ptr)
{
	if (a_ptr->flags4 & TR4_TROLL) return FALSE;
	a_ptr->flags4 |= TR4_TROLL;
	LOG_PRINT("Adding ability: mark as troll\n");
	return TRUE;
}

static bool add_restrict_dragon(artifact_type *a_ptr)
{
	if (a_ptr->flags4 & TR4_DRAGON) return FALSE;
	a_ptr->flags4 |= TR4_DRAGON;
	LOG_PRINT("Adding ability: mark as dragon\n");
	return TRUE;
}


static bool add_restrict_demon(artifact_type *a_ptr)
{
	if (a_ptr->flags4 & TR4_DEMON) return FALSE;
	a_ptr->flags4 |= TR4_DEMON;
	LOG_PRINT("Adding ability: mark as demon\n");
	return TRUE;
}

static bool add_restrict_undead(artifact_type *a_ptr)
{
	if (a_ptr->flags4 & TR4_UNDEAD) return FALSE;
	a_ptr->flags4 |= TR4_UNDEAD;
	LOG_PRINT("Adding ability: mark as undead\n");
	return TRUE;
}

static bool add_restrict_man(artifact_type *a_ptr)
{
	if (a_ptr->flags4 & TR4_MAN) return FALSE;
	a_ptr->flags4 |= TR4_MAN;
	LOG_PRINT("Adding ability: mark as man\n");
	return TRUE;
}

static bool add_restrict_elf(artifact_type *a_ptr)
{
	if (a_ptr->flags4 & TR4_ELF) return FALSE;
	a_ptr->flags4 |= TR4_ELF;
	LOG_PRINT("Adding ability: mark as elf\n");
	return TRUE;
}

static bool add_restrict_dwarf(artifact_type *a_ptr)
{
	if (a_ptr->flags4 & TR4_DWARF) return FALSE;
	a_ptr->flags4 |= TR4_DWARF;
	LOG_PRINT("Adding ability: mark as dwarf\n");
	return TRUE;
}

static bool add_restrict_animal(artifact_type *a_ptr)
{
	if (a_ptr->flags4 & TR4_ANIMAL) return FALSE;
	a_ptr->flags4 |= TR4_ANIMAL;
	LOG_PRINT("Adding ability: mark as animal\n");
	return TRUE;
}

static bool add_restrict_evil(artifact_type *a_ptr)
{
	if (a_ptr->flags4 & TR4_EVIL) return FALSE;
	a_ptr->flags4 |= TR4_EVIL;
	LOG_PRINT("Adding ability: mark as evil\n");
	return TRUE;
}

static void add_restrict_rand(artifact_type *a_ptr)
{
	/* Pick a restriction at random, as long as item has some related abilities or slays */

	int r;
	int count = 0;
	bool success = FALSE;

	while ( (!success) & (count < MAX_TRIES) )
	{
		r = rand_int(9);
		if ((r == 0) && ((a_ptr->flags2 & TR2_RES_DARK) || (a_ptr->flags4 & TR4_SLAY_ELF))) success = add_restrict_orc(a_ptr);
		else if ((r == 1) && ((a_ptr->flags1 & TR1_STR) || (a_ptr->flags4 & TR4_SLAY_DWARF))) success = add_restrict_giant(a_ptr);
		else if ((r == 2) && (a_ptr->flags3 & TR3_REGEN)) success = add_restrict_troll(a_ptr);
		else if ((r == 3) && (a_ptr->flags2 & TR2_RES_ACID) && (a_ptr->flags2 & TR2_RES_FIRE) && (a_ptr->flags2 & TR2_RES_ELEC) && (a_ptr->flags2 & TR2_RES_COLD)) success = add_restrict_dragon(a_ptr);
		else if ((r == 4) && (a_ptr->flags2 & (TR2_RES_FIRE | TR2_IM_FIRE))) success = add_restrict_demon(a_ptr);
		else if ((r == 5) && ((a_ptr->flags2 & TR2_RES_NETHR) || (a_ptr->flags3 & (TR3_SEE_INVIS | TR3_HOLD_LIFE)))) success = add_restrict_undead(a_ptr);
		else if ((r == 6) && ((a_ptr->flags1 & (TR1_SLAY_ORC | TR1_STEALTH)) || (a_ptr->flags2 & TR2_RES_LITE) || (a_ptr->flags3 & TR3_SEE_INVIS))) success = add_restrict_elf(a_ptr);
		else if ((r == 7) && ((a_ptr->flags2 & TR2_RES_BLIND) || (a_ptr->flags3 & (TR3_FREE_ACT | TR3_HOLD_LIFE)) || (a_ptr->flags4 & TR4_SLAY_DWARF))) success = add_restrict_dwarf(a_ptr);
		else if ((r == 8) && ((a_ptr->flags1 & TR1_STEALTH) || (a_ptr->flags3 & (TR3_SLOW_DIGEST | TR3_FEATHER)))) success = add_restrict_animal(a_ptr);

		count++;
	}

	if (!success)
	{
		if (rand_int(3)) (void)add_restrict_man(a_ptr);
		else (void)add_restrict_evil(a_ptr);
	}
}



static void add_bless_weapon(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_BLESSED;
	LOG_PRINT("Adding ability: blessed blade\n");
}

static void add_damage_dice(artifact_type *a_ptr)
{
	/* CR 2001-09-02: changed this to increments 1 or 2 only */
	a_ptr->dd += (byte)(1 + rand_int(2));
	if (a_ptr->dd > 9)
		a_ptr->dd = 9;
	LOG_PRINT1("Adding ability: extra damage dice (now %d dice)\n", a_ptr->dd);
}

static void add_to_hit(artifact_type *a_ptr, int fixed, int random)
{
	/* Inhibit above certain threshholds */
	if (a_ptr->to_h > 25)
	{
		/* Strongly inhibit */
		if (rand_int(INHIBIT_STRONG) > 0)
		{
			LOG_PRINT1("Failed to add to-hit, value of %d is too high\n", a_ptr->to_h);
			return;
		}
	}
	else if (a_ptr->to_h > 15)
	{
		/* Weakly inhibit */
		if (rand_int(INHIBIT_WEAK) > 0)
		{
			LOG_PRINT1("Failed to add to-hit, value of %d is too high\n", a_ptr->to_h);
			return;
		}
	}
	a_ptr->flags3 |= TR3_SHOW_MODS;
	a_ptr->to_h += (s16b)(fixed + rand_int(random));
	LOG_PRINT1("Adding ability: extra to_h (now %+d)\n", a_ptr->to_h);
}

static void add_to_dam(artifact_type *a_ptr, int fixed, int random)
{
	/* Inhibit above certain threshholds */
	if (a_ptr->to_d > 25)
	{
		/* Strongly inhibit */
		if (rand_int(INHIBIT_STRONG) > 0)
		{
			LOG_PRINT1("Failed to add to-dam, value of %d is too high\n", a_ptr->to_d);
			return;
		}
	}
	else if (a_ptr->to_d > 15)
	{
		/* Weakly inhibit */
		if (rand_int(INHIBIT_WEAK) > 0)
		{
			LOG_PRINT1("Failed to add to-dam, value of %d is too high\n", a_ptr->to_d);
			return;
		}
	}
	a_ptr->flags3 |= TR3_SHOW_MODS;
	a_ptr->to_d += (s16b)(fixed + rand_int(random));
	LOG_PRINT1("Adding ability: extra to_dam (now %+d)\n", a_ptr->to_d);
}

static void add_aggravation(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_AGGRAVATE;
	LOG_PRINT("Adding aggravation\n");
}

static void add_to_AC(artifact_type *a_ptr, int fixed, int random)
{
	/* Inhibit above certain threshholds */
	if (a_ptr->to_a > 35)
	{
		/* Strongly inhibit */
		if (rand_int(INHIBIT_STRONG) > 0)
		{
			return;
			LOG_PRINT1("Failed to add to-AC, value of %d is too high\n", a_ptr->to_a);
		}
	}
	else if (a_ptr->to_a > 25)
	{
		/* Weakly inhibit */
		if (rand_int(INHIBIT_WEAK) > 0)
		{
			LOG_PRINT1("Failed to add to-AC, value of %d is too high\n", a_ptr->to_a);
			return;
		}
	}
	a_ptr->to_a += (s16b)(fixed + rand_int(random));
	LOG_PRINT1("Adding ability: AC bonus (new bonus is %+d)\n", a_ptr->to_a);
}

static void add_weight_mod(artifact_type *a_ptr)
{
	a_ptr->weight = (a_ptr->weight * 9) / 10;
	LOG_PRINT1("Adding ability: lower weight (new weight is %d)\n", a_ptr->weight);
}

/*
 * Add a random immunity to this artifact
 * ASSUMPTION: All immunities are equally likely.
 */
static void add_immunity(artifact_type *a_ptr)
{
	int imm_type = rand_int(4);

	switch(imm_type)
	{
		case 0:
		{
			a_ptr->flags2 |= TR2_IM_ACID;
			LOG_PRINT("Adding ability: immunity to acid\n");
			break;
		}
		case 1:
		{
			a_ptr->flags2 |= TR2_IM_ELEC;
			LOG_PRINT("Adding ability: immunity to lightning\n");
			break;
		}
		case 2:
		{
			a_ptr->flags2 |= TR2_IM_FIRE;
			LOG_PRINT("Adding ability: immunity to fire\n");
			break;
		}
		case 3:
		{
			a_ptr->flags2 |= TR2_IM_COLD;
			LOG_PRINT("Adding ability: immunity to cold\n");
			break;
		}
	}
}

/*
 * Build a suitable frequency table for this item, based on the generated
 * frequencies.  The frequencies for any abilities that don't apply for
 * this item type will be set to zero.  First parameter is the artifact
 * for which to generate the frequency table.
 *
 * The second input parameter is a pointer to an array that the function
 * will use to store the frequency table.  The array must have size
 * ART_IDX_TOTAL.
 *
 * The resulting frequency table is cumulative for ease of use in the
 * weighted randomization algorithm.
 */

static void build_freq_table(artifact_type *a_ptr, s16b *freq)
{
	int i,j;
	s16b f_temp[ART_IDX_TOTAL];

	/* First, set everything to zero */
	for (i = 0; i < ART_IDX_TOTAL; i++)
	{
		f_temp[i] = 0;
		freq[i] = 0;
	}

	/* Now copy over appropriate frequencies for applicable abilities */
	/* Bow abilities */
	if (a_ptr->tval == TV_BOW)
	{
		for (j = 0; j < ART_IDX_BOW_COUNT; j++)
		{
			f_temp[art_idx_bow[j]] = artprobs[art_idx_bow[j]];
		}
	}
	/* General weapon abilities */
	if (a_ptr->tval == TV_BOW || a_ptr->tval == TV_DIGGING ||
		a_ptr->tval == TV_HAFTED || a_ptr->tval == TV_POLEARM ||
		a_ptr->tval == TV_SWORD)
	{
		for (j = 0; j < ART_IDX_WEAPON_COUNT; j++)
		{
			f_temp[art_idx_weapon[j]] = artprobs[art_idx_weapon[j]];
		}
	}
	/* General non-weapon abilities */
	else
	{
		for (j = 0; j < ART_IDX_NONWEAPON_COUNT; j++)
		{
			f_temp[art_idx_nonweapon[j]] = artprobs[art_idx_nonweapon[j]];
		}
	}
	/* General melee abilities */
	if (a_ptr->tval == TV_DIGGING || a_ptr->tval == TV_HAFTED ||
		a_ptr->tval == TV_POLEARM || a_ptr->tval == TV_SWORD)
	{
		for (j = 0; j < ART_IDX_MELEE_COUNT; j++)
		{
			f_temp[art_idx_melee[j]] = artprobs[art_idx_melee[j]];
		}
	}
	/* General armor abilities */
	if ( a_ptr->tval == TV_BOOTS || a_ptr->tval == TV_GLOVES ||
		a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN ||
		a_ptr->tval == TV_SHIELD || a_ptr->tval == TV_CLOAK ||
		a_ptr->tval == TV_SOFT_ARMOR || a_ptr->tval == TV_HARD_ARMOR ||
		a_ptr->tval == TV_DRAG_ARMOR)
		{
		for (j = 0; j < ART_IDX_ALLARMOR_COUNT; j++)
		{
			f_temp[art_idx_allarmor[j]] = artprobs[art_idx_allarmor[j]];
		}
	}
	/* Boot abilities */
	if (a_ptr->tval == TV_BOOTS)
	{
		for (j = 0; j < ART_IDX_BOOT_COUNT; j++)
		{
			f_temp[art_idx_boot[j]] = artprobs[art_idx_boot[j]];
		}
	}
	/* Glove abilities */
	if (a_ptr->tval == TV_GLOVES)
	{
		for (j = 0; j < ART_IDX_GLOVE_COUNT; j++)
		{
			f_temp[art_idx_glove[j]] = artprobs[art_idx_glove[j]];
		}
	}
	/* Headgear abilities */
	if (a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN)
	{
		for (j = 0; j < ART_IDX_HELM_COUNT; j++)
		{
			f_temp[art_idx_headgear[j]] = artprobs[art_idx_headgear[j]];
		}
	}
	/* Shield abilities */
	if (a_ptr->tval == TV_SHIELD)
	{
		for (j = 0; j < ART_IDX_SHIELD_COUNT; j++)
		{
			f_temp[art_idx_shield[j]] = artprobs[art_idx_shield[j]];
		}
	}
	/* Cloak abilities */
	if (a_ptr->tval == TV_CLOAK)
	{
		for (j = 0; j < ART_IDX_CLOAK_COUNT; j++)
		{
			f_temp[art_idx_cloak[j]] = artprobs[art_idx_cloak[j]];
		}
	}
	/* Armor abilities */
	if (a_ptr->tval == TV_SOFT_ARMOR || a_ptr->tval == TV_HARD_ARMOR ||
		a_ptr->tval == TV_DRAG_ARMOR)
	{
		for (j = 0; j < ART_IDX_ARMOR_COUNT; j++)
		{
			f_temp[art_idx_armor[j]] = artprobs[art_idx_armor[j]];
		}
	}

	/* General abilities - no constraint */
	for (j = 0; j < ART_IDX_GEN_COUNT; j++)
	{
		f_temp[art_idx_gen[j]] = artprobs[art_idx_gen[j]];
	}

	/*
	 * Now we have the correct individual frequencies, we build a cumulative
	 * frequency table for them.
	 */
	for (i = 0; i < ART_IDX_TOTAL; i++)
	{
		for (j = i; j < ART_IDX_TOTAL; j++)
		{
			freq[j] += f_temp[i];
		}
	}
	/* Done - the freq array holds the desired frequencies. */

	/* Print out the frequency table, for verification */
	for (i = 0; i < ART_IDX_TOTAL; i++)
		LOG_PRINT2("Cumulative frequency of ability %d is: %d\n", i, freq[i]);
}

/*
 * Choose a random ability using weights based on the given cumulative frequency
 * table.  A pointer to the frequency array (which must be of size ART_IDX_TOTAL)
 * is passed as a parameter.  The function returns a number representing the
 * index of the ability chosen.
 */

static int choose_ability (s16b *freq_table)
{
	int r, ability;

	/* Generate a random number between 1 and the last value in the table */
	r = randint (freq_table[ART_IDX_TOTAL-1]);

	/* Find the entry in the table that this number represents. */
	ability = 0;
	while (r > freq_table[ability])
		ability++;

	LOG_PRINT1("Ability chosen was number: %d\n", ability);
	/*
	 * The ability variable is now the index of the first value in the table
	 * greater than or equal to r, which is what we want.
	 */
	return ability;
}

/*
 * Add an ability given by the index r.  This is mostly just a long case
 * statement.
 *
 * Note that this method is totally general and imposes no restrictions on
 * appropriate item type for a given ability.  This is assumed to have
 * been done already.
 */

static void add_ability_aux(artifact_type *a_ptr, int r)
{
	switch(r)
	{
		case ART_IDX_BOW_SHOTS:
			add_shots(a_ptr);
			break;

		case ART_IDX_BOW_MIGHT:
			add_might(a_ptr);
			break;

		case ART_IDX_WEAPON_HIT:
		case ART_IDX_NONWEAPON_HIT:
			add_to_hit(a_ptr, 1, 2 * mean_hit_increment);
			break;

		case ART_IDX_WEAPON_DAM:
		case ART_IDX_NONWEAPON_DAM:
			add_to_dam(a_ptr, 1, 2 * mean_dam_increment);
			break;

		case ART_IDX_WEAPON_AGGR:
		case ART_IDX_NONWEAPON_AGGR:
			add_aggravation(a_ptr);
			break;

		case ART_IDX_MELEE_BLESS:
			add_bless_weapon(a_ptr);
			break;

		case ART_IDX_MELEE_BRAND_SLAY:
			add_brand_or_slay(a_ptr);
			break;

		case ART_IDX_MELEE_SINV:
		case ART_IDX_HELM_SINV:
		case ART_IDX_GEN_SINV:
			add_see_invisible(a_ptr);
			break;

		case ART_IDX_MELEE_BLOWS:
			add_blows(a_ptr);
			break;

		case ART_IDX_MELEE_AC:
		case ART_IDX_BOOT_AC:
		case ART_IDX_GLOVE_AC:
		case ART_IDX_HELM_AC:
		case ART_IDX_SHIELD_AC:
		case ART_IDX_CLOAK_AC:
		case ART_IDX_ARMOR_AC:
		case ART_IDX_GEN_AC:
			add_to_AC(a_ptr, 1, 2 * mean_ac_increment);
			break;

		case ART_IDX_MELEE_DICE:
			add_damage_dice(a_ptr);
			break;

		case ART_IDX_MELEE_WEIGHT:
		case ART_IDX_ALLARMOR_WEIGHT:
			add_weight_mod(a_ptr);
			break;

		case ART_IDX_MELEE_TUNN:
		case ART_IDX_GEN_TUNN:
			add_tunnelling(a_ptr);
			break;

		case ART_IDX_BOOT_FEATHER:
		case ART_IDX_GEN_FEATHER:
			add_feather_falling(a_ptr);
			break;

		case ART_IDX_GEN_SAVE:
			add_spell_resistance(a_ptr);
			break;

		case ART_IDX_GEN_DEVICE:
			add_magical_device(a_ptr);
			break;

		case ART_IDX_BOOT_STEALTH:
		case ART_IDX_CLOAK_STEALTH:
		case ART_IDX_ARMOR_STEALTH:
		case ART_IDX_GEN_STEALTH:
			add_stealth(a_ptr);
			break;

		case ART_IDX_BOOT_SPEED:
		case ART_IDX_GEN_SPEED:
			add_speed(a_ptr);
			break;

		case ART_IDX_GLOVE_FA:
		case ART_IDX_GEN_FA:
			add_free_action(a_ptr);
			break;

		case ART_IDX_GLOVE_DEX:
			add_dex(a_ptr);
			break;

		case ART_IDX_HELM_RBLIND:
		case ART_IDX_GEN_RBLIND:
			add_resist_blindness(a_ptr);
			break;

		case ART_IDX_HELM_ESP:
		case ART_IDX_GEN_ESP:
			add_telepathy(a_ptr);
			break;

		case ART_IDX_MELEE_SENSE:
			add_sense_slay(a_ptr);
			break;

		case ART_IDX_BOW_SENSE:
		case ART_IDX_NONWEAPON_SENSE:
			add_sense_rand(a_ptr);
			break;

		case ART_IDX_MELEE_RESTRICT:
		case ART_IDX_BOW_RESTRICT:
		case ART_IDX_NONWEAPON_RESTRICT:
			add_restrict_rand(a_ptr);
			break;

		case ART_IDX_HELM_WIS:
			add_wis(a_ptr);
			break;

		case ART_IDX_HELM_INT:
			add_int(a_ptr);
			break;

		case ART_IDX_SHIELD_LRES:
		case ART_IDX_ARMOR_LRES:
		case ART_IDX_GEN_LRES:
			add_low_resist(a_ptr);
			break;

		case ART_IDX_ARMOR_HLIFE:
		case ART_IDX_GEN_HLIFE:
			add_hold_life(a_ptr);
			break;

		case ART_IDX_ARMOR_CON:
			add_con(a_ptr);
			break;

		case ART_IDX_ARMOR_ALLRES:
			add_resist_acid(a_ptr);
			add_resist_lightning(a_ptr);
			add_resist_fire(a_ptr);
			add_resist_cold(a_ptr);
			break;

		case ART_IDX_ARMOR_HRES:
			add_high_resist(a_ptr);
			break;

		case ART_IDX_GEN_STAT:
			add_stat(a_ptr);
			break;

		case ART_IDX_GEN_SUST:
			add_sustain(a_ptr);
			break;

		case ART_IDX_GEN_SEARCH:
			add_search(a_ptr);
			break;

		case ART_IDX_GEN_INFRA:
			add_infravision(a_ptr);
			break;

		case ART_IDX_GEN_IMMUNE:
			add_immunity(a_ptr);
			break;

		case ART_IDX_GEN_LITE:
			add_permanent_light(a_ptr);
			break;

		case ART_IDX_GEN_SDIG:
			add_slow_digestion(a_ptr);
			break;

		case ART_IDX_GEN_REGEN:
			add_regeneration(a_ptr);
			break;

		case ART_IDX_GEN_RPOIS:
			add_resist_poison(a_ptr);
			break;

		case ART_IDX_GEN_RFEAR:
			add_resist_fear(a_ptr);
			break;

		case ART_IDX_GEN_RLITE:
			add_resist_light(a_ptr);
			break;

		case ART_IDX_GEN_RDARK:
			add_resist_dark(a_ptr);
			break;

		case ART_IDX_GEN_RCONF:
			add_resist_confusion(a_ptr);
			break;

		case ART_IDX_GEN_RSOUND:
			add_resist_sound(a_ptr);
			break;

		case ART_IDX_GEN_RSHARD:
			add_resist_shards(a_ptr);
			break;

		case ART_IDX_GEN_RNEXUS:
			add_resist_nexus(a_ptr);
			break;

		case ART_IDX_GEN_RNETHER:
			add_resist_nether(a_ptr);
			break;

		case ART_IDX_GEN_RCHAOS:
			add_resist_chaos(a_ptr);
			break;

		case ART_IDX_GEN_RDISEN:
			add_resist_disenchantment(a_ptr);
			break;
	}
}

/*
 * Randomly select an extra ability to be added to the artifact in question.
 * XXX - This function is way too large.
 */
static void add_ability(artifact_type *a_ptr)
{
	int r;

	/* Choose a random ability using the frequency table previously defined*/
	r = choose_ability(art_freq);

	/* Add the appropriate ability */
	add_ability_aux(a_ptr, r);

	/* Now remove contradictory or redundant powers. */
	remove_contradictory(a_ptr);

	/* Adding WIS to sharp weapons always blesses them */
	if ((a_ptr->flags1 & TR1_WIS) && (a_ptr->tval == TV_SWORD || a_ptr->tval == TV_POLEARM))
	{
		add_bless_weapon(a_ptr);
	}
}


/*
 * Try to supercharge this item by running through the list of the supercharge
 * abilities and attempting to add each in turn.  An artifact only gets one
 * chance at each of these up front (if applicable).
 */
static void try_supercharge(artifact_type *a_ptr)
{
	/* Huge damage dice - melee weapon only */
	if (a_ptr->tval == TV_DIGGING || a_ptr->tval == TV_HAFTED ||
		a_ptr->tval == TV_POLEARM || a_ptr->tval == TV_SWORD)
	{
		if (rand_int (z_info->a_max) < artprobs[ART_IDX_MELEE_DICE_SUPER])
		{
			a_ptr->dd += 3 + rand_int(4);
			if (a_ptr->dd > 9) a_ptr->dd = 9;
			LOG_PRINT1("Supercharging damage dice!  (Now %d dice)\n", a_ptr->dd);
		}
	}

	/* Bows - +3 might or +3 shots */
	if (a_ptr->tval == TV_BOW)
	{
		if (rand_int (z_info->a_max) < artprobs[ART_IDX_BOW_SHOTS_SUPER])
		{
			a_ptr->flags1 |= TR1_SHOTS;
			a_ptr->pval = 3;
			LOG_PRINT("Supercharging shots for bow!  (3 extra shots)\n");
		}
		else if (rand_int (z_info->a_max) < artprobs[ART_IDX_BOW_MIGHT_SUPER])
		{
			a_ptr->flags1 |= TR1_MIGHT;
			a_ptr->pval = 3;
			LOG_PRINT("Supercharging might for bow!  (3 extra might)\n");
		}
	}

	/* Big speed bonus - any item (potentially) */
	if (rand_int (z_info->a_max) < artprobs[ART_IDX_GEN_SPEED_SUPER])
	{
		a_ptr->flags1 |= TR1_SPEED;
		a_ptr->pval = 6 + rand_int(4);
		LOG_PRINT1("Supercharging speed for this item!  (New speed bonus is %d)\n", a_ptr->pval);
	}
	/* Aggravation */
	if (a_ptr->tval == TV_BOW || a_ptr->tval == TV_DIGGING ||
		a_ptr->tval == TV_HAFTED || a_ptr->tval == TV_POLEARM ||
		a_ptr->tval == TV_SWORD)
	{
		if (rand_int (z_info->a_max) < artprobs[ART_IDX_WEAPON_AGGR])
		{
			a_ptr->flags3 |= TR3_AGGRAVATE;
			LOG_PRINT("Adding aggravation\n");
		}
	}
	else
	{
		if (rand_int (z_info->a_max) < artprobs[ART_IDX_NONWEAPON_AGGR])
		{
			a_ptr->flags3 |= TR3_AGGRAVATE;
			LOG_PRINT("Adding aggravation\n");
		}
	}
}

/*
 * Make it bad, or if it's already bad, make it worse!
 */
static void do_curse(artifact_type *a_ptr)
{

	int r = rand_int(21);

	r = rand_int(21);
	if (r == 0) a_ptr->flags3 |= TR3_AGGRAVATE;
	else if (r == 1) a_ptr->flags3 |= TR3_DRAIN_EXP;
	else if (r == 2) a_ptr->flags3 |= TR3_DRAIN_HP;
	else if (r == 3) a_ptr->flags3 |= TR3_DRAIN_MANA;
	else if (r == 4) a_ptr->flags3 |= TR3_TELEPORT;
	else if (r == 5) a_ptr->flags4 |= TR4_HURT_LITE;
	else if (r == 6) a_ptr->flags4 |= TR4_HURT_WATER;
	else if (r == 7) a_ptr->flags4 |= TR4_HUNGER;
	else if (r == 8) a_ptr->flags4 |= TR4_ANCHOR;
	else if (r == 9) a_ptr->flags4 |= TR4_SILENT;
	else if (r == 10) a_ptr->flags4 |= TR4_STATIC;
	else if (r == 11) a_ptr->flags4 |= TR4_WINDY;
	else if (r == 12) a_ptr->flags4 |= TR4_EVIL;
	else if (r == 13) a_ptr->flags4 |= TR4_UNDEAD;
	else if (r == 14) a_ptr->flags4 |= TR4_DRAGON;
	else if (r == 15) a_ptr->flags4 |= TR4_DEMON;
	else if (r == 16) a_ptr->flags4 |= TR4_HURT_POIS;
	else if (r == 17) a_ptr->flags4 |= TR4_HURT_ACID;
	else if (r == 18) a_ptr->flags4 |= TR4_HURT_ELEC;
	else if (r == 19) a_ptr->flags4 |= TR4_HURT_FIRE;
	else if (r == 20) a_ptr->flags4 |= TR4_HURT_COLD;

	if ((a_ptr->pval > 0) && (rand_int(2) == 0))
		a_ptr->pval = -a_ptr->pval;
	if ((a_ptr->to_a > 0) && (rand_int(2) == 0))
		a_ptr->to_a = -a_ptr->to_a;
	if ((a_ptr->to_h > 0) && (rand_int(2) == 0))
		a_ptr->to_h = -a_ptr->to_h;
	if ((a_ptr->to_d > 0) && (rand_int(4) == 0))
		a_ptr->to_d = -a_ptr->to_d;

	if (a_ptr->flags3 & TR3_LIGHT_CURSE)
	{
		if (rand_int(2) == 0) a_ptr->flags3 |= TR3_HEAVY_CURSE;
		return;
	}

	a_ptr->flags3 |= TR3_LIGHT_CURSE;

	if (rand_int(4) == 0)
		a_ptr->flags3 |= TR3_HEAVY_CURSE;
}


/*
 * Note the three special cases (One Ring, Grond, Morgoth).
 */
static void scramble_artifact(int a_idx)
{
	artifact_type *a_ptr = &a_info[a_idx];
	artifact_type a_old;
	object_kind *k_ptr;
	u32b activates = a_ptr->flags3 & TR3_ACTIVATE;
	s32b power;
	int tries=0;
	s16b k_idx;
	byte rarity_old, base_rarity_old;
	s16b rarity_new;
	s32b ap;
	bool curse_me = FALSE;
	bool success = FALSE;

	/* Special cases -- don't randomize these! */
	if ((a_idx == ART_POWER) ||
	    (a_idx == ART_GROND) ||
	    (a_idx == ART_MORGOTH))
		return;

	/* Skip unused artifacts, too! */
	if (a_ptr->tval == 0) return;

	/* Evaluate the original artifact to determine the power level. */
	power = base_power[a_idx];

	/* If it has a restricted ability then don't randomize it. */
	if (power > 10000)
	{
		LOG_PRINT1("Skipping artifact number %d - too powerful to randomize!", a_idx);
		return;
	}

	if (power < 0) curse_me = TRUE;

	LOG_PRINT("+++++++++++++++++ CREATING NEW ARTIFACT ++++++++++++++++++\n");
	LOG_PRINT2("Artifact %d: power = %d\n", a_idx, power);

	/*
	 * Flip the sign on power if it's negative, since it's only used for base
	 * item choice
	 */
	if (power < 0) power = -power;

	if (a_idx >= ART_MIN_NORMAL)
	{
		/*
		 * Normal artifact - choose a random base item type.  Not too
		 * powerful, so we'll have to add something to it.  Not too
		 * weak, for the opposite reason.  We also require a new
		 * rarity rating of at least 2.
		 *
		 * CR 7/15/2001 - lowered the upper limit so that we get at
		 * least a few powers (from 8/10 to 6/10) but permit anything
		 * more than 20 below the target power
		 */
		int count = 0;
		s32b ap2;

		/* Capture the rarity of the original base item and artifact */
		base_rarity_old = base_item_rarity[a_idx];
		rarity_old = base_art_rarity[a_idx];
		do
		{
			k_idx = choose_item(a_idx);

			/*
			 * Hack: if power is positive but very low, and if we're not having
			 * any luck finding a base item, curse it once.  This helps ensure
			 * that we get a base item for borderline cases like Wormtongue.
			 */

			if (power > 0 && power < 10 && count > MAX_TRIES / 2)
			{
				LOG_PRINT("Cursing base item to help get a match.\n");
				do_curse(a_ptr);
				remove_contradictory(a_ptr);
			}

			else if (power >= 10 && power < 20 && count > MAX_TRIES / 2)
			{
				LOG_PRINT("Restricting base item to help get a match.\n");
				add_restrict_rand(a_ptr);
				remove_contradictory(a_ptr);
			}

			ap2 = artifact_power(a_idx);
			count++;
			/*
			 * Calculate the proper rarity based on the new type.  We attempt
			 * to preserve the 'effective rarity' which is equal to the
			 * artifact rarity multiplied by the base item rarity.
			 */
			k_ptr = &k_info[k_idx];
			rarity_new = ( (s16b) rarity_old * (s16b) base_rarity_old ) /
				(s16b) (k_ptr->chance[0] ? k_ptr->chance[0] : 1);

			if (rarity_new > 255) rarity_new = 255;
			if (rarity_new < 1) rarity_new = 1;

		} while ( (count < MAX_TRIES) &&
			(((ap2 > (power * 6) / 10 + 1) && (power-ap2 < 20)) ||
			(ap2 < (power / 10)) || rarity_new == 1) );

		/* Got an item - set the new rarity */
		a_ptr->rarity = (byte) rarity_new;
	}
	else
	{
		/* Special artifact (light source, ring, or
		   amulet).  Clear the following fields; leave
		   the rest alone. */
		a_ptr->pval = 0;
		a_ptr->to_h = a_ptr->to_d = a_ptr->to_a = 0;
		a_ptr->flags1 = a_ptr->flags2 = 0;

		/* Artifacts ignore everything */
		a_ptr->flags2 = (TR2_IGNORE_MASK);
	}

	/* Got a base item. */

	/* Generate the cumulative frequency table for this item type */
	build_freq_table(a_ptr, art_freq);

	/* Copy artifact info temporarily. */
	COPY(&a_old, a_ptr, artifact_type);

	/* Give this artifact a shot at being supercharged */
	try_supercharge(a_ptr);
	ap = artifact_power(a_idx);
	if (ap > (power * 23) / 20 + 1)
	{
		/* too powerful -- put it back */
		*a_ptr = a_old;
		LOG_PRINT("--- Supercharge is too powerful!  Rolling back.\n");
	}

	/* First draft: add two abilities, then curse it three times. */
	if (curse_me)
	{
		/* Copy artifact info temporarily. */
		COPY(&a_old, a_ptr, artifact_type);

		do
		{
			add_ability(a_ptr);
			add_ability(a_ptr);
			do_curse(a_ptr);
			do_curse(a_ptr);
			do_curse(a_ptr);
			remove_contradictory(a_ptr);
			ap = artifact_power(a_idx);
			/* Accept if it doesn't have any inhibited abilities */
			if (ap < 10000) success = TRUE;
			/* Otherwise go back and try again */
			else
			{
				LOG_PRINT("Inhibited ability added - rolling back.\n");
				COPY(a_ptr, &a_old, artifact_type);

			}
		} while (!success);
	}
	else
	{

		/*
		 * Select a random set of abilities which roughly matches the
		 * original's in terms of overall power/usefulness.
		 */
		for (tries = 0; tries < MAX_TRIES; tries++)
		{

			/* Copy artifact info temporarily. */
			COPY(&a_old, a_ptr, artifact_type);
			add_ability(a_ptr);
			ap = artifact_power(a_idx);


			/* CR 11/14/01 - pushed both limits up by about 5% */
			if (ap > (power * 23) / 20 + 1)
			{
				/* too powerful -- put it back */
				COPY(a_ptr, &a_old, artifact_type);
				continue;
			}

			else if (ap >= (power * 19) / 20)	/* just right */
			{

				/* Hack -- add a restriction on the most powerful artifacts */
				if (ap > 150)
				{
					add_restrict_rand(a_ptr);
					remove_contradictory(a_ptr);
				}

				break;
			}

			/* Stop if we're going negative, so we don't overload
			   the artifact with great powers to compensate. */
			/* Removed CR 11/10/01 */
			/*
			else if ((ap < 0) && (ap < (-(power * 1)) / 10))
			{
				break;
			}
			*/
		}		/* end of power selection */

		if (randart_verbose && tries >= MAX_TRIES)
		{
			/*
			 * We couldn't generate an artifact within the number of permitted
			 * iterations.  Show a warning message.
			 */
			msg_format("Warning!  Couldn't get appropriate power level.");
			LOG_PRINT("Warning!  Couldn't get appropriate power level.\n");
			msg_print(NULL);
		}

	}

	a_ptr->cost = ap * 1000L;

	if (a_ptr->cost < 0) a_ptr->cost = 0;

	/* Restore some flags */
	if (activates) a_ptr->flags3 |= TR3_ACTIVATE;
	if (a_idx < ART_MIN_NORMAL) a_ptr->flags3 |= TR3_INSTA_ART;

	/* Store artifact power */
	a_ptr->power = ap;

	/*
	 * Add TR3_HIDE_TYPE to all artifacts with nonzero pval because we're
	 * too lazy to find out which ones need it and which ones don't.
	 */
	if (a_ptr->pval)
		a_ptr->flags3 |= TR3_HIDE_TYPE;

	/* Success */
	LOG_PRINT(">>>>>>>>>>>>>>>>>>>>>>>>>> ARTIFACT COMPLETED <<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
	LOG_PRINT2("Number of tries for artifact %d was: %d\n", a_idx, tries);
	LOG_PRINT1("Final artifact power is %d\n",ap);
}

/*
 * Return nonzero if the whole set of random artifacts meets certain
 * criteria.  Return 0 if we fail to meet those criteria (which will
 * restart the whole process).
 */
static int artifacts_acceptable(void)
{
	int swords = 5, polearms = 5, blunts = 5, bows = 4;
	int bodies = 5, shields = 4, cloaks = 4, hats = 4;
	int gloves = 4, boots = 4;
	int i;

	for (i = ART_MIN_NORMAL; i < z_info->a_max; i++)
	{
		switch (a_info[i].tval)
		{
			case TV_SWORD:
				swords--; break;
			case TV_POLEARM:
				polearms--; break;
			case TV_HAFTED:
				blunts--; break;
			case TV_BOW:
				bows--; break;
			case TV_SOFT_ARMOR:
			case TV_HARD_ARMOR:
			case TV_DRAG_ARMOR:
				bodies--; break;
			case TV_SHIELD:
				shields--; break;
			case TV_CLOAK:
				cloaks--; break;
			case TV_HELM:
			case TV_CROWN:
				hats--; break;
			case TV_GLOVES:
				gloves--; break;
			case TV_BOOTS:
				boots--; break;
		}
	}

	LOG_PRINT1("Deficit amount for swords is %d\n", swords);
	LOG_PRINT1("Deficit amount for polearms is %d\n", polearms);
	LOG_PRINT1("Deficit amount for blunts is %d\n", blunts);
	LOG_PRINT1("Deficit amount for bows is %d\n", bows);
	LOG_PRINT1("Deficit amount for bodies is %d\n", bodies);
	LOG_PRINT1("Deficit amount for shields is %d\n", shields);
	LOG_PRINT1("Deficit amount for cloaks is %d\n", cloaks);
	LOG_PRINT1("Deficit amount for hats is %d\n", hats);
	LOG_PRINT1("Deficit amount for gloves is %d\n", gloves);
	LOG_PRINT1("Deficit amount for boots is %d\n", boots);

	if (swords > 0 || polearms > 0 || blunts > 0 || bows > 0 ||
	    bodies > 0 || shields > 0 || cloaks > 0 || hats > 0 ||
	    gloves > 0 || boots > 0)
	{
		if (randart_verbose)
		{
			char types[256];
			sprintf(types, "%s%s%s%s%s%s%s%s%s%s",
				swords > 0 ? " swords" : "",
				polearms > 0 ? " polearms" : "",
				blunts > 0 ? " blunts" : "",
				bows > 0 ? " bows" : "",
				bodies > 0 ? " body-armors" : "",
				shields > 0 ? " shields" : "",
				cloaks > 0 ? " cloaks" : "",
				hats > 0 ? " hats" : "",
				gloves > 0 ? " gloves" : "",
				boots > 0 ? " boots" : "");
			msg_format("Restarting generation process: not enough %s", types);
			LOG_PRINT(format("Restarting generation process: not enough %s", types));
		}
		return (0);
	}
	else
	{
		return (1);
	}
}


static errr scramble(void)
{
	/* If our artifact set fails to meet certain criteria, we start over. */
	do
	{
		int a_idx;
/* ARD_RAND - Note boundary condition. We only scramble the artifacts about the old
 * z_info->a_max if adult_rand_artifacts is not set.
 */
		/* Generate all the artifacts. */
		for (a_idx = (adult_rand_artifacts ? 1 : a_max) ; a_idx < z_info->a_max; a_idx++)
		{
			scramble_artifact(a_idx);
		}
	} while (!artifacts_acceptable());	/* end of all artifacts */

	/* Success */
	return (0);
}


static errr do_randart_aux(bool full)
{
	errr result;

	/* Generate random names */
	if ((result = init_names()) != 0) return (result);

	if (full)
	{
		/* Randomize the artifacts */
		if ((result = scramble()) != 0) return (result);
	}

	/* Success */
	return (0);
}

/*
 * Randomize the artifacts
 *
 * The full flag toggles between just randomizing the names and
 * complete randomization of the artifacts.
 */
errr do_randart(u32b randart_seed, bool full)
{
	errr err;

	int i;
	u32b j;

	/* Prepare to use the Angband "simple" RNG. */
	Rand_value = randart_seed;
	Rand_quick = TRUE;

	/* Allocate the "slay values" array - now used to compute magic item power */
	C_MAKE(slays, SLAY_MAX, s32b);

	/* Only do all the following if full randomization requested */
	if (full)
	{
		int i;

		artifact_type *a_info_new;

		object_lore *a_list_new;

		int art_high_slot = 255;

		/* Allocate the "monster power ratings" array */
		C_MAKE(mon_power, z_info->r_max, long);

/* ARD_RAND - Start of allocation routine to create extra artifact
 * slots. We store the old z_info->a_max for use elsewhere in randart.c
 */
		/* Allocate the new artifact range */
		C_MAKE(a_info_new, 256, artifact_type);

		/* Allocate the new artifact lore range */
		C_MAKE(a_list_new, 256, object_lore);

		for (i = ART_MIN_NORMAL; i < z_info->a_max;i++)
		{
			artifact_type *a_ptr = &a_info[i];
			artifact_type *a2_ptr = &a_info_new[art_high_slot];

			if (a_info[i].tval == 0) continue;
			if (i == ART_POWER) continue;
			if (i == ART_GROND) continue;
			if (i == ART_MORGOTH) continue;
			
			COPY(a2_ptr,a_ptr,artifact_type);
			art_high_slot--;
		}

		/* Copy existing a_info array to a_info_new */
		for (i = 0; i< z_info->a_max;i++)
		{
			artifact_type *a_ptr = &a_info[i];
			artifact_type *a2_ptr = &a_info_new[i];
	
			COPY(a2_ptr,a_ptr,artifact_type);
		}
	
		/* Free existing a_info array */
		KILL(a_info);

		/* Free existing a_list array */
		KILL(a_list);		
	
		/* Set new a_info array to existing */
		a_head.info_ptr = a_info = a_info_new;

		/* Set new a_info array to existing */
		a_list = a_list_new;

		/* Temporarily store old number of artifacts */
		a_max = z_info->a_max;

		/* Update number of artifacts */
		z_info->a_max = 256;

/* ARD_RAND - End of allocation process. */

		/* Allocate the "kinds" array */
		C_MAKE(kinds, z_info->a_max, s16b);

		/* Allocate the various "original powers" arrays */
		C_MAKE(base_power, z_info->a_max, s32b);
		C_MAKE(base_item_level, z_info->a_max, byte);
		C_MAKE(base_item_rarity, z_info->a_max, byte);
		C_MAKE(base_art_rarity, z_info->a_max, byte);


		/* Open the log file for writing */
		if (randart_verbose)
		{
			if ((randart_log = fopen("randart.log", "w")) == NULL)
			{
				msg_format("Error - can't open randart.log for writing.");
				exit(1);
			}
		}

		/* Initialize the monster power ratings */
		init_mon_power();

		/* Store the original power ratings */
		store_base_power();

		/* Determine the generation probabilities */
		parse_frequencies();
	}

	/* Generate the random artifact (names) */
	err = do_randart_aux(full);

	/* Precompute values for ego item slays for ego items */
	for (i = 0; i < z_info->e_max; i++);
	{
		ego_item_type *e_ptr = &e_info[i];

		e_ptr->slay_power = slay_power(slay_index(e_ptr->flags1, e_ptr->flags2, e_ptr->flags3, e_ptr->flags4));
	}

	/* Precompute values for single flag slays for magic items */
	for (i = 0, j = 0x00000001L; (i < 32) && (j < 0x00200000L); i++, j <<=1)
	{
		/* Compute slay power for single flags */
		magic_slay_power[i] = slay_power(j);
	}

	/* Only do all the following if full randomization requested */
	if (full)
	{
		/* Just for fun, look at the frequencies on the finished items */
		/* Remove this prior to release */
		store_base_power();
		parse_frequencies();

		/* Report artifact powers */
		for (i = 0; i < z_info->a_max; i++)
		{
			LOG_PRINT2("Artifact %d power is now %d\n",i,a_info[i].power);
		}

		/* Close the log file */
		if (randart_verbose)
		{
			if (fclose(randart_log) != 0)
			{
				msg_format("Error - can't close randart.log file.");
				exit(1);
			}
		}

		/* Free the "kinds" array */
		KILL(kinds);

		/* Free the "original powers" arrays */
		KILL(base_power);
		KILL(base_item_level);
		KILL(base_item_rarity);

	}

	/* Free the "slay values" array */
	KILL(slays);

	/* When done, resume use of the Angband "complex" RNG. */
	Rand_quick = FALSE;

	return (err);
}

#else /* GJW_RANDART */

#ifdef MACINTOSH
static int i = 0;
#endif /* MACINTOSH */

#endif /* GJW_RANDART */
