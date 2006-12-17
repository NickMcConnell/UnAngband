/* File: generate.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 *
 * UnAngband (c) 2001-6 Andrew Doull. Modifications to the Angband 2.9.1
 * source code are released under the Gnu Public License. See www.fsf.org
 * for current GPL license details. Addition permission granted to
 * incorporate modifications in all Angband variants as defined in the
 * Angband variants FAQ. See rec.games.roguelike.angband for FAQ.
 */

#include "angband.h"


/*
 * Note that Level generation is *not* an important bottleneck,
 * though it can be annoyingly slow on older machines...  Thus
 * we emphasize "simplicity" and "correctness" over "speed".
 *
 * This entire file is only needed for generating levels.
 * This may allow smart compilers to only load it when needed.
 *
 * Consider the "v_info.txt" file for vault generation.
 *
 * In this file, we use the "special" granite and perma-wall sub-types,
 * where "basic" is normal, "inner" is inside a room, "outer" is the
 * outer wall of a room, and "solid" is the outer wall of the dungeon
 * or any walls that may not be pierced by corridors.  Thus the only
 * wall type that may be pierced by a corridor is the "outer granite"
 * type.  The "basic granite" type yields the "actual" corridors.
 *
 * Note that we use the special "solid" granite wall type to prevent
 * multiple corridors from piercing a wall in two adjacent locations,
 * which would be messy, and we use the special "outer" granite wall
 * to indicate which walls "surround" rooms, and may thus be "pierced"
 * by corridors entering or leaving the room.
 *
 * Note that a tunnel which attempts to leave a room near the "edge"
 * of the dungeon in a direction toward that edge will cause "silly"
 * wall piercings, but will have no permanently incorrect effects,
 * as long as the tunnel can *eventually* exit from another side.
 * And note that the wall may not come back into the room by the
 * hole it left through, so it must bend to the left or right and
 * then optionally re-enter the room (at least 2 grids away).  This
 * is not a problem since every room that is large enough to block
 * the passage of tunnels is also large enough to allow the tunnel
 * to pierce the room itself several times.
 *
 * Note that no two corridors may enter a room through adjacent grids,
 * they must either share an entryway or else use entryways at least
 * two grids apart.  This prevents "large" (or "silly") doorways.
 *
 * To create rooms in the dungeon, we first divide the dungeon up
 * into "blocks" of 11x11 grids each, and require that all rooms
 * occupy a rectangular group of blocks.  As long as each room type
 * reserves a sufficient number of blocks, the room building routines
 * will not need to check bounds.  Note that most of the normal rooms
 * actually only use 23x11 grids, and so reserve 33x11 grids.
 *
 * Note that the use of 11x11 blocks (instead of the 33x11 panels)
 * allows more variability in the horizontal placement of rooms, and
 * at the same time has the disadvantage that some rooms (two thirds
 * of the normal rooms) may be "split" by panel boundaries.  This can
 * induce a situation where a player is in a room and part of the room
 * is off the screen.  This can be so annoying that the player must set
 * a special option to enable "non-aligned" room generation.
 *
 * Note that the dungeon generation routines are much different (2.7.5)
 * and perhaps "MAX_DUN_ROOMS" should be less than 50.
 *
 * XXX XXX XXX Note that it is possible to create a room which is only
 * connected to itself, because the "tunnel generation" code allows a
 * tunnel to leave a room, wander around, and then re-enter the room.
 *
 * XXX XXX XXX Note that it is possible to create a set of rooms which
 * are only connected to other rooms in that set, since there is nothing
 * explicit in the code to prevent this from happening.  But this is less
 * likely than the "isolated room" problem, because each room attempts to
 * connect to another room, in a giant cycle, thus requiring at least two
 * bizarre occurances to create an isolated section of the dungeon.
 *
 * Note that (2.7.9) monster pits have been split into monster "nests"
 * and monster "pits".  The "nests" have a collection of monsters of a
 * given type strewn randomly around the room (jelly, animal, or undead),
 * while the "pits" have a collection of monsters of a given type placed
 * around the room in an organized manner (orc, troll, giant, dragon, or
 * demon).  Note that both "nests" and "pits" are now "level dependant",
 * and both make 16 "expensive" calls to the "get_mon_num()" function.
 *
 * Note that the cave grid flags changed in a rather drastic manner
 * for Angband 2.8.0 (and 2.7.9+), in particular, dungeon terrain
 * features, such as doors and stairs and traps and rubble and walls,
 * are all handled as a set of 64 possible "terrain features", and
 * not as "fake" objects (440-479) as in pre-2.8.0 versions.
 *
 * The 64 new "dungeon features" will also be used for "visual display"
 * but we must be careful not to allow, for example, the user to display
 * hidden traps in a different way from floors, or secret doors in a way
 * different from granite walls, or even permanent granite in a different
 * way from granite.  XXX XXX XXX
 */


/*
 * Dungeon generation values
 */
/* DUN_ROOMS now defined in defines.h */
#define DUN_UNUSUAL     200     /* Level/chance of unusual room */
#define DUN_DEST	30      /* 1/chance of having a destroyed level */

/*
 * Dungeon tunnel generation values
 */
#define DUN_TUN_RND     10      /* Chance of random direction */
#define DUN_TUN_CAV     50      /* Chance of random direction / changing direction in caves */
#define DUN_TUN_CHG     10      /* Chance of changing direction */
#define DUN_TUN_STY     10      /* Chance of changing style */
#define DUN_TUN_CON     15       /* Chance of extra tunneling */
#define DUN_TUN_PEN     25      /* Chance of doors at room entrances */
#define DUN_TUN_JCT     90      /* Chance of doors at tunnel junctions */

/*
 * Dungeon streamer generation values
 */
#define DUN_STR_DEN     5       /* Density of streamers */
#define DUN_STR_RNG     2       /* Width of streamers */
#define DUN_STR_MAG     3       /* Number of magma streamers */
#define DUN_STR_MC      90      /* 1/chance of treasure per magma */
#define DUN_STR_QUA     2       /* Number of quartz streamers */
#define DUN_STR_QC      40      /* 1/chance of treasure per quartz */
#define DUN_STR_SAN     2       /* Number of sandstone streamers */
#define DUN_STR_SLV     40      /* Deepest level sandstone occurs instead of magma */
#define DUN_STR_GOL     20      /* 1/chance of rich mineral vein */
#define DUN_STR_GC      2       /* 1/chance of treasure per rich mineral vein */
#define DUN_STR_CRA     8       /* 1/chance of cracks through dungeon */
#define DUN_STR_CC      0       /* 1/chance of treasure per crack */

/*
 * Dungeon feature values
 */
#define DUN_FEAT_OILC   10      /* 1/chance of oil/coal feature level */
#define DUN_FEAT	40      /* Chance in 100 of having features */
#define DUN_MAX_LAKES   3       /* Maximum number of lakes/rivers */
#define DUN_FEAT_RNG    2       /* Width of lake */

/*
 * Dungeon treasure allocation values
 */
#define DUN_AMT_ROOM    9       /* Amount of objects for rooms */
#define DUN_AMT_ITEM    3       /* Amount of objects for rooms/corridors */
#define DUN_AMT_GOLD    3       /* Amount of treasure for rooms/corridors */

/*
 * Hack -- Dungeon allocation "places"
 */
#define ALLOC_SET_CORR	  1       /* Hallway */
#define ALLOC_SET_ROOM	  2       /* Room */
#define ALLOC_SET_BOTH	  3       /* Anywhere */

/*
 * Hack -- Dungeon allocation "types"
 */
#define ALLOC_TYP_RUBBLE	1       /* Rubble */
#define ALLOC_TYP_TRAP	  3       /* Trap */
#define ALLOC_TYP_GOLD	  4       /* Gold */
#define ALLOC_TYP_OBJECT	5       /* Object */
#define ALLOC_TYP_FEATURE	6	/* Feature eg fountain */

/*
 * Bounds on some arrays used in the "dun_data" structure.
 * These bounds are checked, though usually this is a formality.
 */
#define CENT_MAX	100
#define DOOR_MAX	200
#define WALL_MAX	500
#define TUNN_MAX	900


/*
 * Maximal number of room types
 */
#define ROOM_MAX	9



/*
 * Simple structure to hold a map location
 */

typedef struct coord coord;

struct coord
{
	byte y;
	byte x;
};


/*
 * Room type information
 */

typedef struct room_data room_data;

struct room_data
{
	/* Required size in blocks */
	s16b dy1, dy2, dx1, dx2;

	/* Hack -- minimum level */
	s16b level;
};


/*
 * Structure to hold all "dungeon generation" data
 */

typedef struct dun_data dun_data;

struct dun_data
{
	/* Array of centers of rooms */
	int cent_n;
	coord cent[CENT_MAX];

	/* Array of possible door locations */
	int door_n;
	coord door[DOOR_MAX];

	/* Array of solid wall locations and feature types */
	int next_n;
	coord next[DOOR_MAX];
	s16b next_feat[DOOR_MAX];

	/* Array of wall piercing locations */
	int wall_n;
	coord wall[WALL_MAX];

	/* Array of tunnel grids and feature types */
	int tunn_n;
	coord tunn[TUNN_MAX];
	s16b tunn_feat[TUNN_MAX];

	/* Array of partitions of rooms */
	int part_n;
	int part[CENT_MAX];

	/* Number of blocks along each axis */
	int row_rooms;
	int col_rooms;

	/* Array of which blocks are used */
	bool room_map[MAX_ROOMS_ROW][MAX_ROOMS_COL];

	/* Hack -- there is a pit/nest on this level */
	bool crowded;

	/* Hack -- number of entrances to dungeon */
	bool entrance;
};


/*
 * Dungeon generation data -- see "cave_gen()"
 */
static dun_data *dun;


/*
 * Array of room types (assumes 11x11 blocks)
 */
static room_data room[ROOM_MAX] =
{
	{ 0, 0, 0, 0, 0 },	      /* 0 = Nothing */
	{ 0, 0, -1, 1, 1 },	     /* 1 = Simple (33x11) */
	{ 0, 0, -1, 1, 1 },	     /* 2 = Overlapping (33x11) */
	{ 0, 0, -1, 1, 3 },	     /* 3 = Crossed (33x11) */
	{ 0, 0, -1, 1, 3 },	     /* 4 = Large (33x11) */
	{ 0, 0, -1, 1, 5 },	     /* 5 = Monster nest (33x11) */
	{ 0, 0, -1, 1, 5 },	     /* 6 = Monster pit (33x11) */
	{ 0, 1, -1, 1, 5 },	     /* 7 = Lesser vault (33x22) */
	{ -1, 2, -2, 3, 10 }    /* 8 = Greater vault (66x44) */
};



/*
 * Always picks a correct direction
 */
static void correct_dir(int *rdir, int *cdir, int y1, int x1, int y2, int x2)
{
	/* Extract vertical and horizontal directions */
	*rdir = (y1 == y2) ? 0 : (y1 < y2) ? 1 : -1;
	*cdir = (x1 == x2) ? 0 : (x1 < x2) ? 1 : -1;

	/* Never move diagonally */
	if (*rdir && *cdir)
	{
		if (rand_int(100) < 50)
		{
			*rdir = 0;
		}
		else
		{
			*cdir = 0;
		}
	}
}


/*
 * Pick a random direction
 */
static void rand_dir(int *rdir, int *cdir)
{
	/* Pick a random direction */
	int i = rand_int(4);

	/* Extract the dy/dx components */
	*rdir = ddy_ddd[i];
	*cdir = ddx_ddd[i];
}


/*
 * Pick a random direction
 */
static void rand_dir_cave(int *rdir, int *cdir)
{
	/* Pick a random direction */
	int i = rand_int(8);

	/* Extract the dy/dx components */
	*rdir = ddy_ddd[i];
	*cdir = ddx_ddd[i];
}


/*
 * Returns random co-ordinates for player/monster/object
 */
static void new_player_spot(void)
{
	int y, x;

	int count=0;

	/* Place the player */
	while (1)
	{

		/* Pick a legal spot */
		y = rand_range(1, DUNGEON_HGT - 2);
		x = rand_range(1, DUNGEON_WID - 2);

		if (cave_naked_bold(y,x) && (count > 2000)) break;

		/* Must be a "start" floor grid */
		if (!cave_start_bold(y, x)) continue;

		/* Refuse to start in anti-teleport rooms */
		if (room_has_flag(y, x, ROOM_ICKY)) continue;

		/* Done */
		break;
	}

	/* Place the player */
	player_place(y, x);
}



/*
 * Count the number of walls adjacent to the given grid.
 *
 * Note -- Assumes "in_bounds_fully(y, x)"
 *
 * We count only granite walls and permanent walls.
 */
static int next_to_walls(int y, int x)
{
	int k = 0;

	if (f_info[cave_feat[y+1][x]].flags1 & (FF1_WALL)) k++;
	if (f_info[cave_feat[y+1][x]].flags1 & (FF1_WALL)) k++;
	if (f_info[cave_feat[y+1][x]].flags1 & (FF1_WALL)) k++;
	if (f_info[cave_feat[y+1][x]].flags1 & (FF1_WALL)) k++;

	return (k);
}



/*
 * Convert existing terrain type to rubble
 */
static void place_rubble(int y, int x)
{
	/* Put item under rubble */
	if (rand_int(100) < 5) cave_set_feat(y, x, FEAT_RUBBLE_H);

	/* Create rubble */
	else cave_set_feat(y, x, FEAT_RUBBLE);
}


/*
 * Places some staircases near walls
 */
static void alloc_stairs(int feat, int num, int walls)
{
	int y, x, i, j, flag;

	/* Place "num" stairs */
	for (i = 0; i < num; i++)
	{
		/* Place some stairs */
		for (flag = FALSE; !flag; )
		{
			/* Try several times, then decrease "walls" */
			for (j = 0; !flag && j <= 3000; j++)
			{
				/* Pick a random grid */
				y = rand_int(DUNGEON_HGT);
				x = rand_int(DUNGEON_WID);

				/* Require actual floor grid */
				if (!(f_info[cave_feat[y][x]].flags1 & (FF1_FLOOR))) continue;

				/* Require a certain number of adjacent walls */
				if (next_to_walls(y, x) < walls) continue;

				/* Place fixed stairs */
				place_random_stairs(y, x, feat);

				/* All done */
				flag = TRUE;
			}

			/* Require fewer walls */
			if (walls) walls--;
		}
	}
}



/*
 * Allocates some objects (using "place" and "type")
 */
static void alloc_object(int set, int typ, int num)
{
	int y, x, k, c;

	/* Place some objects */
	for (k = 0; k < num; k++)
	{
		c = 0;

		/* Pick a "legal" spot */
		while (TRUE)
		{
			bool room;

			bool surface = (p_ptr->depth == min_depth(p_ptr->dungeon));

			/* Paranoia */
			if (c++ > 2000) return;

			/* Location */
			y = rand_int(DUNGEON_HGT);
			x = rand_int(DUNGEON_WID);

			/* Require actual floor grid */
			if ((f_info[cave_feat[y][x]].flags1 & (FF1_FLOOR)) == 0) continue;

			/* Check for "room" */
			room = (cave_info[y][x] & (CAVE_ROOM)) ? TRUE : FALSE;

			/* Require corridor? */
			if ((set == ALLOC_SET_CORR) && room && !surface) continue;

			/* Require room? */
			if ((set == ALLOC_SET_ROOM) && !room) continue;

			/* Accept it */
			break;
		}

		/* Place something */
		switch (typ)
		{
			case ALLOC_TYP_RUBBLE:
			{
				place_rubble(y, x);
				break;
			}

			case ALLOC_TYP_TRAP:
			{
				place_trap(y, x);
				break;
			}

			case ALLOC_TYP_GOLD:
			{
				place_gold(y, x);
				break;
			}

			case ALLOC_TYP_OBJECT:
			{
				place_object(y, x, FALSE, FALSE);
				break;
			}

			case ALLOC_TYP_FEATURE:
			{
				place_feature(y, x);
				break;
			}
		}
	}
}

/*
 * Note that the order we generate the dungeon is terrain features, then
 * rooms, then corridors, then streamers. This is important, because 
 * (currently) we ensure that deep or hostile terrain is bridged by safe
 * terrain, and rooms (and vaults) alway have their respective walls intact.
 *
 * Note that rooms can be generated inside 'big' lakes, but not on regular
 * lakes. We take a risk here that 'big' lakes are less likely to have
 * areas rendered inaccessible by having a room block them.
 *
 * XXX XXX XXX Currently both types of lakes can have areas that are completely
 * blocked because of the 20% chance of filling a lake centre location with
 * a lake edge location. We should always guarantee that all areas are connected.
 *
 * XXX XXX These huge case statements should be cut down by using WALL, FLOOR,
 * etc. flags to take out the common cases and ensuring we never overwrite a
 * dun square with an edge square. But the resulting code might be less
 * efficient.
 */ 

/*
 * Places a terrain on another terrain 
 */

static void build_terrain(int y, int x, int feat)
{
	int oldfeat, newfeat;
	int k;

	feature_type *f_ptr;
	feature_type *f2_ptr;

	/* Get the feature */
	oldfeat = cave_feat[y][x];
	f_ptr = &f_info[oldfeat];

	/* Set the new feature */
	newfeat = oldfeat;
	f2_ptr = &f_info[feat];

	/* Special cases first */
	switch (feat)
	{
		case FEAT_QSAND_H:
		case FEAT_SAND_H:
		{
			if (oldfeat == FEAT_WATER) newfeat = FEAT_QSAND_H;
			if (oldfeat == FEAT_WATER_H) newfeat = FEAT_QSAND_H;
			break;
		}
		case FEAT_ICE_WATER_K:
		case FEAT_ICE_WATER_H:
		{
			if (f_ptr->flags2 & (FF2_LAVA)) newfeat = FEAT_BWATER;
			else if (f_ptr->flags2 & (FF2_ICE | FF2_WATER)) newfeat = feat;
			break;
		}	
		case FEAT_BMUD:
		case FEAT_BWATER:
		{
			if (f_ptr->flags2 & (FF2_WATER)) newfeat = feat;
			if (f_ptr->flags2 & (FF2_HIDE_DIG)) newfeat = feat;
			if (oldfeat == FEAT_ICE) newfeat = FEAT_ICE_GEOTH;
			if (oldfeat == FEAT_ICE_C) newfeat = FEAT_ICE_GEOTH;
			if (oldfeat == FEAT_FLOOR_ICE) newfeat = FEAT_GEOTH;		    
			if (oldfeat == FEAT_LAVA) newfeat = FEAT_GEOTH;
			if (oldfeat == FEAT_LAVA_H) newfeat = feat;
			if (oldfeat == FEAT_LAVA_K) newfeat = feat;
			if (oldfeat == FEAT_FLOOR_EARTH) newfeat = FEAT_BMUD;
			if (oldfeat == FEAT_FLOOR_EARTH_T) newfeat = FEAT_BMUD;
			if (oldfeat == FEAT_SAND) newfeat = feat;
			if (oldfeat == FEAT_QSAND_H) newfeat = feat;
			if (oldfeat == FEAT_MUD_H) newfeat = FEAT_BMUD;
			if (oldfeat == FEAT_MUD_HT) newfeat = FEAT_BMUD;
			if (oldfeat == FEAT_MUD_K) newfeat = FEAT_BMUD;
			if (oldfeat == FEAT_EARTH) newfeat = FEAT_BMUD;
			break;
		}
		case FEAT_GEOTH:
		{
			if (oldfeat == FEAT_ICE) newfeat = FEAT_ICE_GEOTH;
			if (oldfeat == FEAT_ICE) newfeat = FEAT_ICE_GEOTH;
			if (oldfeat == FEAT_MUD_H) newfeat = FEAT_BMUD;
			if (oldfeat == FEAT_MUD_HT) newfeat = FEAT_BMUD;
			if (oldfeat == FEAT_MUD_K) newfeat = FEAT_BMUD;
			break;
		}
	}

	/* Have we handled a special case? */
	if (newfeat != oldfeat)
	{
		/* Nothing */
	}
	else if (!oldfeat)
	{
		newfeat = feat;
	}
	else if ((f_ptr->flags1 & (FF1_WALL))
		&& !(f_ptr->flags2 & (FF2_WATER | FF2_LAVA | FF2_ACID | FF2_OIL | FF2_ICE | FF2_CHASM))
		&& !(f_ptr->flags3 & (FF3_TREE)))
	{
		newfeat = feat;
	}
	else if (f2_ptr->flags2 & (FF2_BRIDGED))
	{
		newfeat = feat_state(oldfeat, FS_BRIDGE);
	}
	else if ((f_ptr->flags2 & (FF2_CHASM)) || (f2_ptr->flags2 & (FF2_CHASM)))
	{
		newfeat = feat_state(oldfeat,FS_CHASM);
	}
	else if (f_ptr->flags1 & (FF1_FLOOR))
	{
		newfeat = feat_state(feat,FS_TUNNEL);
	}
	else if (f2_ptr->flags1 & (FF1_FLOOR))
	{
		newfeat = feat_state(oldfeat,FS_TUNNEL);
	}
	else if (f_ptr->flags3 & (FF3_GROUND))
	{
		newfeat = feat;
	}
	else if (f2_ptr->flags3 & (FF3_GROUND))
	{
		newfeat = feat;
	}
	else if (f_ptr->flags2 & (FF2_LAVA))
	{
		if ((f2_ptr->flags2 & (FF2_ICE)) && (f2_ptr->flags1 & (FF1_WALL)) &&
                        (f2_ptr->flags2 & (FF2_CAN_OOZE)))
		{
			newfeat = FEAT_ICE_GEOTH_HC;
		}
		else if ((f2_ptr->flags2 & (FF2_ICE)) && (f2_ptr->flags1 & (FF1_WALL)))
		{
			newfeat = FEAT_ICE_GEOTH;
		}
		else if ((f2_ptr->flags2 & (FF2_HIDE_DIG)) && (f2_ptr->flags2 & (FF2_DEEP | FF2_FILLED)))
		{
			newfeat = FEAT_BMUD;
		}
		else if ((f2_ptr->flags2 & (FF2_WATER)) && (f2_ptr->flags2 & (FF2_DEEP | FF2_FILLED)))
		{
			newfeat = FEAT_BWATER;
		}
		else if (f2_ptr->flags2 & (FF2_WATER | FF2_ACID | FF2_OIL | FF2_ICE | FF2_CHASM))
		{
			newfeat = FEAT_GEOTH_LAVA;
		}
	}
	else if (f2_ptr->flags2 & (FF2_LAVA))
	{
		if ((f_ptr->flags2 & (FF2_ICE)) && (f_ptr->flags1 & (FF1_WALL)) &&
                        (f_ptr->flags2 & (FF2_CAN_OOZE)))
		{
			newfeat = FEAT_ICE_GEOTH_HC;
		}
		else if ((f_ptr->flags2 & (FF2_ICE)) && (f_ptr->flags1 & (FF1_WALL)))
		{
			newfeat = FEAT_ICE_GEOTH;
		}
		else if ((f_ptr->flags2 & (FF2_HIDE_DIG)) && (f_ptr->flags2 & (FF2_DEEP | FF2_FILLED)))
		{
			newfeat = FEAT_BMUD;
		}
		else if ((f_ptr->flags2 & (FF2_WATER)) && (f_ptr->flags2 & (FF2_DEEP | FF2_FILLED)))
		{
			newfeat = FEAT_BWATER;
		}
		else if (f_ptr->flags2 & (FF2_WATER | FF2_ACID | FF2_OIL | FF2_ICE | FF2_CHASM))
		{
			newfeat = FEAT_GEOTH_LAVA;
		}
		else
		{
			newfeat = feat;
		}
	}
	else if (f_ptr->flags2 & (FF2_ICE))
	{
		/* Handle case of ice wall over underwater */
                if ((f_ptr->flags1 & (FF1_WALL)) && (f_ptr->flags2 & (FF2_CAN_OOZE)))
		{
			if ((f2_ptr->flags2 & (FF2_WATER)) && (f2_ptr->flags2 & (FF2_FILLED))
			 && (f2_ptr->flags1 & (FF1_SECRET)))
			{
				newfeat = FEAT_UNDER_ICE_HC;
			}
			else if ((f2_ptr->flags2 & (FF2_WATER)) && (f2_ptr->flags2 & (FF2_FILLED)))
			{
				newfeat = FEAT_UNDER_ICE_KC;
			}
		}
		else if (f_ptr->flags1 & (FF1_WALL))
		{
			if ((f2_ptr->flags2 & (FF2_WATER)) && (f2_ptr->flags2 & (FF2_FILLED)))
			{
				newfeat = FEAT_UNDER_ICE;
			}
		}
		else if (f2_ptr->flags2 & (FF2_HURT_COLD)) newfeat = feat_state(feat,FS_HURT_COLD);
	}
	else if (f2_ptr->flags2 & (FF2_ICE))
	{
		/* Handle case of ice wall over underwater */
                if ((f2_ptr->flags1 & (FF1_WALL)) && (f2_ptr->flags2 & (FF2_CAN_OOZE)))
		{
			if ((f_ptr->flags2 & (FF2_WATER)) && (f_ptr->flags2 & (FF2_FILLED))
			 && (f_ptr->flags1 & (FF1_SECRET)))
			{
				newfeat = FEAT_UNDER_ICE_HC;
			}
			else if ((f_ptr->flags2 & (FF2_WATER)) && (f_ptr->flags2 & (FF2_FILLED)))
			{
				newfeat = FEAT_UNDER_ICE_KC;
			}
		}
		else if (f2_ptr->flags1 & (FF1_WALL))
		{
			if ((f_ptr->flags2 & (FF2_WATER)) && (f_ptr->flags2 & (FF2_FILLED)))
			{
				newfeat = FEAT_UNDER_ICE;
			}
		}
		else if (f_ptr->flags2 & (FF2_HURT_COLD)) newfeat = feat_state(oldfeat,FS_HURT_COLD);
		else if (f_ptr->flags2 & (FF2_HIDE_DIG)) newfeat = feat;
	}
	else if ((f_ptr->flags2 & (FF2_WATER)) || (f2_ptr->flags2 & (FF2_WATER)))
	{
		/* Hack -- we try and match water properties */
		u32b mask1 = (FF1_SECRET | FF1_LESS);
		u32b mask2 = (FF2_WATER | FF2_SHALLOW | FF2_FILLED | FF2_DEEP | FF2_ICE | FF2_LAVA | FF2_CHASM | FF2_HIDE_SWIM);
		u32b match1 = 0x0L;
		u32b match2 = FF2_WATER;

		int k_idx = f_info[oldfeat].k_idx;

		int i;

		/* Hack -- get most poisonous object */
		if (f_info[feat].k_idx > k_idx) k_idx = f_info[feat].k_idx;

		/* Hack -- get flags */
		if ((f_ptr->flags2 & (FF2_SHALLOW)) || (f2_ptr->flags2 & (FF2_SHALLOW)))
		{
			match2 |= FF2_SHALLOW;
		}
		else if ((f_ptr->flags2 & (FF2_FILLED)) || (f2_ptr->flags2 & (FF2_FILLED)))
		{
			match2 |= FF2_FILLED;
			match1 |= ((f_ptr->flags1 & (FF1_SECRET)) || (f2_ptr->flags1 & (FF1_SECRET)));
		}
		else
		{
			match2 |= FF2_DEEP;
			match2 |= ((f_ptr->flags2 & (FF2_HIDE_SWIM)) || (f2_ptr->flags2 & (FF2_HIDE_SWIM)));
			match1 |= ((f_ptr->flags1 & (FF1_SECRET)) || (f2_ptr->flags1 & (FF1_SECRET)));
			match1 |= ((f_ptr->flags1 & (FF1_LESS)) || (f2_ptr->flags1 & (FF1_LESS)));
		}

		for (i = 0;i < z_info->f_max;i++)
		{
			/* Hack -- force match */
			if ((f_info[i].flags1 & (mask1)) != match1) continue;
			if ((f_info[i].flags2 & (mask2)) != match2) continue;

			if (f_info[i].k_idx != k_idx) continue;

			newfeat = i;
		}
	}
	else if ((f_ptr->flags2 & (FF2_CAN_DIG)) || (f2_ptr->flags2 & (FF2_CAN_DIG)))
	{
		newfeat = feat;
	}


	/* Hack -- no change */
	if (newfeat == oldfeat) return;

	k = randint(100);

	if (f_info[newfeat].flags3 & (FF3_TREE))
	{
		if (k<=85) newfeat = oldfeat;
	}

	switch (newfeat)
	{
		case FEAT_BUSH:
		case FEAT_BUSH_HURT:
		case FEAT_BUSH_FOOD:
		case FEAT_BUSH_HURT_P:
		if (k<=30) newfeat = oldfeat;
		if (k<=90) newfeat = FEAT_GRASS;
		break;

		case FEAT_RUBBLE:
		if (k<90) newfeat = oldfeat;
		break;

		case FEAT_LIMESTONE:
		if (k<40) newfeat = FEAT_FLOOR;
		if ((k > 40) && (k <= 60)) newfeat = FEAT_WATER;
		break;

		case FEAT_ICE:
		if (k <= 10) newfeat = FEAT_ICE_C;
		break;

		case FEAT_ICE_GEOTH:
		if (k <= 10) newfeat = FEAT_ICE_GEOTH_HC;
		break;

		case FEAT_ICE_WATER_K:
		if (k<= 15) newfeat = FEAT_WATER_K;
		break;

		case FEAT_ICE_CHASM:
		if (k <= 80) newfeat = FEAT_FLOOR_ICE;
		if ((k > 80) && (k<90)) newfeat = FEAT_CHASM_E;
		break;

		case FEAT_ICE_FALLS:
		if (k <= 40) newfeat = FEAT_FLOOR_ICE;
		if (k <= 60) newfeat = FEAT_ICE_CHASM;
		if ((k > 60) && (k<80)) newfeat = FEAT_ICE_FALL;
		break;

		case FEAT_WATER_FALLS:
		if (k <= 60) newfeat = FEAT_WATER_H;
		if ((k > 60) && (k<80)) newfeat = FEAT_WATER;
		break;

		case FEAT_ACID_FALLS:
		if (k <= 60) newfeat = FEAT_ACID_H;
		if ((k > 60) && (k<80)) newfeat = FEAT_ACID;
		break;

		case FEAT_MUD:
		if (k <= 10) newfeat = FEAT_FLOOR_EARTH;
		if ((k> 10) && (k <= 13)) newfeat = FEAT_WATER;
		break;

		case FEAT_MUD_H:
		if (k <= 10) newfeat = FEAT_FLOOR_EARTH;
		if ((k> 10) && (k <= 23)) newfeat = FEAT_WATER_H;
		break;

		case FEAT_MUD_K:
		if (k <= 5) newfeat = FEAT_WATER_K;
		break;

		case FEAT_QSAND_H:
		if (k <= 25) newfeat = FEAT_SAND_H;
		if ((k> 25) && (k <= 28)) newfeat = FEAT_WATER;
		break;

		case FEAT_BWATER_FALLS:
		if (k <= 60) newfeat = FEAT_BWATER;
		if ((k > 60) && (k<80)) newfeat = FEAT_FLOOR_RUBBLE;
		break;

		case FEAT_BMUD:
		if (k <= 10) newfeat = FEAT_BWATER;
		if ((k> 10) && (k <= 13)) newfeat = FEAT_VENT_BWATER;
		break;

		case FEAT_GEOTH:
		if (k <= 5) newfeat = FEAT_VENT_STEAM;
		if ((k> 5) && (k <= 10)) newfeat = FEAT_VENT_GAS;
		break;

		case FEAT_GEOTH_LAVA:
		if (k <= 5) newfeat = FEAT_LAVA_H;
		if ((k> 5) && (k <= 10)) newfeat = FEAT_LAVA;
		if ((k> 10) && (k <= 13)) newfeat = FEAT_VENT_LAVA;
		break;

		case FEAT_LAVA_FALLS:
		if (k <= 60) newfeat = FEAT_LAVA_H;
		if ((k > 60) && (k<80)) newfeat = FEAT_FLOOR_RUBBLE;
		break;

	}

	/* Set the feature if we have a change */
	if (newfeat != oldfeat) cave_set_feat(y,x,newfeat);

	/* Change reference */
        f2_ptr = &f_info[newfeat];

	/*
	 * Handle creation of big trees.
         *
         * Note hack to minimise number of calls to rand_int.
	 */
	if (f_info[newfeat].flags3 & (FF3_TREE))
	{
            int k = 0;
		int i;

            k = rand_int(2<<26);

		/* Place branches over trunk */
            if (k & (0xFF000000)) cave_alter_feat(y,x,FS_TREE);

		for (i = 0; i < 8; i++)
		{
			int yy,xx;

			yy = y + ddy_ddd[i];
			xx = x + ddx_ddd[i];
	
			/* Ignore annoying locations */
			if (!in_bounds_fully(yy, xx)) continue;

			/* Ignore if not placing a tree */
			/* Hack -- we make it 150% as likely to place branches on non-diagonal locations */
                  if (!(k & (2 << i)) && !(k & (2 << (i+8) )) && !((i<4) && (k & (2 << (i+16)))) ) continue;

			/* Place branches */
			cave_alter_feat(yy,xx,FS_TREE);
		}
	}
}

/*
 * Places "lakes" of a feature through dungeon
 *
 */
static void build_feature(int y, int x, int feat, bool do_big_lake)
{
	int i, dir;
	int ty, tx, yi,xi;
	int by,bx;
	int feat1 = feat;
	int feat2 = f_info[feat].edge;

	int lake_width,lake_length;

	bool surface = (p_ptr->depth == min_depth(p_ptr->dungeon));

	/* Hack -- increase the 'big'ness */
	if (f_info[feat1].flags1 & (FF1_WALL))
	{
		/* Make small go big */
		do_big_lake = TRUE;
	}

	/* Hack -- minimise holes in terrain */
	if (surface && !feat2) feat2 = feat1;

	/* Hack -- Save the room location */
	if (!(f_info[feat1].flags2 & (FF2_RIVER))
	       && (dun->cent_n < CENT_MAX))
	{
		dun->cent[dun->cent_n].y = y;
		dun->cent[dun->cent_n].x = x;
		dun->cent_n++;
	}

	if ((f_info[feat1].flags2 & (FF2_LAKE)) || !(f_info[feat1].flags2 & (FF2_RIVER)))
	{
		lake_width = DUN_FEAT_RNG;
		lake_length = 15+randint(p_ptr->depth/2);

		if (do_big_lake)
		{
			lake_width = DUN_FEAT_RNG+1;
			lake_length = 200;
		}

		/* Place lake into dungeon */
		for (i = 0; i<lake_length; i++)
		{
			/* Paranoia */
			if (!in_bounds_fully(y,x)) break;

			/* Pick a nearby grids */
			ty = y + rand_int(2* lake_width+1)- lake_width;
			tx = x + rand_int(2* lake_width+1)- lake_width;
			if (!in_bounds_fully(ty, tx)) continue;
			if (f_info[cave_feat[ty][tx]].flags1 & (FF1_PERMANENT)) continue;
		 
			y = ty;
			x = tx;

			/* Don't want to write over own feat */
			if (cave_feat[y][x] == feat1)
			{
				/* Choose a random compass direction */
				dir = ddd[rand_int(4)];
			
				/* Walk to edge of feature */
				while (cave_feat[y][x] == feat1)
				{
					y = y+ddy[dir];
					x = x+ddx[dir];

					/* Stop at dungeon edge */
					if (!in_bounds_fully(y, x)) break;
					if (f_info[cave_feat[y][x]].flags1 & (FF1_PERMANENT)) break;
				}

			}

			if (!in_bounds_fully(y, x)) break;
			if (f_info[cave_feat[y][x]].flags1 & (FF1_PERMANENT)) break;

			if ((!do_big_lake) && (!surface))
			{
				/* Don't allow rooms here */
				by = y/BLOCK_HGT;
				bx = x/BLOCK_WID;
	
				dun->room_map[by][bx] = TRUE;
			}

			for (yi=y-lake_width;yi<=y+lake_width;yi++)
			{
				for (xi=x-lake_width;xi<=x+lake_width;xi++)
				{
	
					if ((yi==y-lake_width)||
					    (yi==y+lake_width)||
					    (xi==x-lake_width)||
					    (xi==x+lake_width))
					{
						if ((in_bounds_fully(yi,xi)) && (feat2)) build_terrain(yi,xi,feat2);
	
					}
					else
					{
						if ((in_bounds_fully(yi,xi))
							&& (randint(100)<(do_big_lake? 40:20)))
						{
							if (feat2) build_terrain(yi,xi,feat2);
						}
						else if (in_bounds_fully(yi,xi))
						{
							build_terrain(yi,xi,feat1);
						}
					}
				}
			}
		}

	}

	if (f_info[feat1].flags2 & (FF2_RIVER))
	{

		/* Choose a random compass direction */
		dir = ddd[rand_int(4)];
	
		/* Place river into dungeon */
		while (TRUE)
		{
			/* Stop at dungeon edge */
			if (!in_bounds_fully(y, x)) break;
			if (f_info[cave_feat[y][x]].flags1 & (FF1_PERMANENT)) break;
	
			if (!(f_info[feat1].flags1 & (FF1_WALL)))
			{
				/* Don't allow rooms here */
				by = y/BLOCK_HGT;
				bx = x/BLOCK_WID;

				dun->room_map[by][bx] = TRUE;
			}
	
			/*Add terrain*/
			build_terrain(y,x,feat1);
	
			if (feat2)
			{
				for (i=0;i<8;i++)
				{
					int di = ddd[i];
					int yi = y+ddy[di];
					int xi = x+ddx[di];
	
					if (!in_bounds_fully(yi, xi)) continue;
					if (f_info[cave_feat[yi][xi]].flags1 & (FF1_PERMANENT)) continue;
	
					build_terrain(yi,xi,feat2);
				}
			}
	
			/*Stagger the river*/
			if (rand_int(100)<50)
			{
				int dir2 = ddd[rand_int(4)];
	
				y += ddy[dir2];
				x += ddx[dir2];
			}
			/* Advance the streamer */
			else
			{
				y += ddy[dir];
				x += ddx[dir];
			}
		}
	}

}
				


/*
 * Places "streamers" of rock through dungeon
 *
 * Note that their are actually six different terrain features used
 * to represent streamers.  Three each of magma and quartz, one for
 * basic vein, one with hidden gold, and one with known gold.  The
 * hidden gold types are currently unused.
 */
static void build_streamer(int feat, int chance)
{
	int i, tx, ty;
	int y, x, dir;


	/* Hack -- Choose starting point */
	y = rand_spread(DUNGEON_HGT / 2, 10);
	x = rand_spread(DUNGEON_WID / 2, 15);

	/* Choose a random compass direction */
	dir = ddd[rand_int(8)];

	/* Place streamer into dungeon */
	while (TRUE)
	{
		/* One grid per density */
		for (i = 0; i < DUN_STR_DEN; i++)
		{
			int d = DUN_STR_RNG;

			/* Pick a nearby grid */
			while (1)
			{
				ty = rand_spread(y, d);
				tx = rand_spread(x, d);
				if (!in_bounds_fully(ty, tx)) continue;
				break;
			}

			/* Only convert "granite" walls */
			if (cave_feat[ty][tx] < FEAT_WALL_EXTRA) continue;
			if (cave_feat[ty][tx] > FEAT_WALL_SOLID) continue;

			/* Clear previous contents, add proper vein type */
			cave_set_feat(ty, tx, feat);

			/* Hack -- Add some (known) treasure */
			if ((chance) &&(rand_int(chance) == 0)) cave_feat[ty][tx] += 0x04;
		}

		/* Advance the streamer */
		y += ddy[dir];
		x += ddx[dir];

		/* Stop at dungeon edge */
		if (!in_bounds_fully(y, x)) break;
	}
}


/*
 * Build a destroyed level
 */
static void destroy_level(void)
{
	int y1, x1, y, x, k, t, n;


	/* Note destroyed levels */
	if (cheat_room) msg_print("Destroyed Level");

	/* Drop a few epi-centers (usually about two) */
	for (n = 0; n < randint(5); n++)
	{
		/* Pick an epi-center */
		x1 = rand_range(5, DUNGEON_WID-1 - 5);
		y1 = rand_range(5, DUNGEON_HGT-1 - 5);

		/* Big area of affect */
		for (y = (y1 - 15); y <= (y1 + 15); y++)
		{
			for (x = (x1 - 15); x <= (x1 + 15); x++)
			{
				/* Skip illegal grids */
				if (!in_bounds_fully(y, x)) continue;

				/* Extract the distance */
				k = distance(y1, x1, y, x);

				/* Stay in the circle of death */
				if (k >= 16) continue;

				/* Delete the monster (if any) */
				delete_monster(y, x);

				/* Destroy "outside" grids */
				if ((cave_valid_bold(y,x)) && (f_info[cave_feat[y][x]].flags3 & (FF3_OUTSIDE)))
				{
					/* Delete objects */
					delete_object(y, x);
	
					/* Burn stuff */
					if (f_info[cave_feat[y][x]].flags2 & (FF2_HURT_FIRE))
					{
						cave_alter_feat(y,x,FS_HURT_FIRE);
					}
					/* Don't touch chasms */
					else if (f_info[cave_feat[y][x]].flags2 & (FF2_CHASM))
					{
						/* Nothing */
					}
							/* Magma */
					else if (rand_int(100)< 15)
					{
						/* Create magma vein */
						cave_set_feat(y, x, FEAT_RUBBLE);
					}


				}
				/* Destroy valid grids */
				else if (cave_valid_bold(y, x))
				{
					/* Delete objects */
					delete_object(y, x);

					/* Wall (or floor) type */
					t = rand_int(200);

					/* Burn stuff */
					if (f_info[cave_feat[y][x]].flags2 & (FF2_HURT_FIRE))
					{
						cave_alter_feat(y,x,FS_HURT_FIRE);
					}

					/* Granite */
					else if (t < 20)
					{
						/* Create granite wall */
						cave_set_feat(y, x, FEAT_WALL_EXTRA);
					}

					/* Quartz */
					else if (t < 70)
					{
						/* Create quartz vein */
						cave_set_feat(y, x, FEAT_QUARTZ);
					}

					/* Magma */
					else if (t < 100)
					{
						/* Create magma vein */
						cave_set_feat(y, x, FEAT_MAGMA);
					}

					/* Rubble */
					else if (t < 130)
					{
						/* Create rubble */
						cave_set_feat(y, x, FEAT_RUBBLE);
					}

					/* Floor */
					else
					{
						/* Create floor */
						cave_set_feat(y, x, FEAT_FLOOR);
					}

					/* No longer part of a room or vault */
					cave_info[y][x] &= ~(CAVE_ROOM);

					/* No longer illuminated */
					cave_info[y][x] &= ~(CAVE_GLOW);
				}
			}
		}
	}
}


/*
 * Create up to "num" gold near the given coordinates
 * Only really called by the room_info routines
 */
static void vault_treasure(int y, int x, int num)
{
	int i, j, k;

	/* Attempt to place 'num' objects */
	for (; num > 0; --num)
	{
		/* Try up to 11 spots looking for empty space */
		for (i = 0; i < 11; ++i)
		{
			/* Pick a random location */
			while (1)
			{
				j = rand_spread(y, 2);
				k = rand_spread(x, 3);
				if (!in_bounds(j, k)) continue;
				break;
			}

			/* Require "clean" floor space */
			if (!cave_clean_bold(j, k)) continue;

			/* Place gold */
			place_gold(j, k);

			/* Placement accomplished */
			break;
		}
	}
}


/*
 * Create up to "num" objects excluding gold near the given coordinates
 * Only really called by the room_info routines
 */
static void vault_items(int y, int x, int num)
{
	int i, j, k;

	/* Attempt to place 'num' objects */
	for (; num > 0; --num)
	{
		/* Try up to 11 spots looking for empty space */
		for (i = 0; i < 11; ++i)
		{
			/* Pick a random location */
			while (1)
			{
				j = rand_spread(y, 2);
				k = rand_spread(x, 3);
				if (!in_bounds(j, k)) continue;
				break;
			}

			/* Require "clean" floor space */
			if (!cave_clean_bold(j, k)) continue;

			/* Place gold */
			place_object(j, k, FALSE,FALSE);

			/* Placement accomplished */
			break;
		}
	}
}



/*
 * Create up to "num" objects near the given coordinates
 * Only really called by some of the "vault" routines.
 */
static void vault_objects(int y, int x, int num)
{
	int i, j, k;

	/* Attempt to place 'num' objects */
	for (; num > 0; --num)
	{
		/* Try up to 11 spots looking for empty space */
		for (i = 0; i < 11; ++i)
		{
			/* Pick a random location */
			while (1)
			{
				j = rand_spread(y, 2);
				k = rand_spread(x, 3);
				if (!in_bounds(j, k)) continue;
				break;
			}

			/* Require "clean" floor space */
			if (!cave_clean_bold(j, k)) continue;

			/* Place an item */
			if (rand_int(100) < 75)
			{
				place_object(j, k, FALSE, FALSE);
			}

			/* Place gold */
			else
			{
				place_gold(j, k);
			}

			/* Placement accomplished */
			break;
		}
	}
}


/*
 * Place a trap with a given displacement of point
 */
static void vault_trap_aux(int y, int x, int yd, int xd)
{
	int count, y1, x1;

	/* Place traps */
	for (count = 0; count <= 5; count++)
	{
		/* Get a location */
		while (1)
		{
			y1 = rand_spread(y, yd);
			x1 = rand_spread(x, xd);
			if (!in_bounds(y1, x1)) continue;
			break;
		}

		/* Require "naked" floor grids */
		if (!cave_naked_bold(y1, x1)) continue;

		/* Place the trap */
		place_trap(y1, x1);

		/* Done */
		break;
	}
}


/*
 * Place some traps with a given displacement of given location
 */
static void vault_traps(int y, int x, int yd, int xd, int num)
{
	int i;

	for (i = 0; i < num; i++)
	{
		vault_trap_aux(y, x, yd, xd);
	}
}


/*
 * Hack -- Place some sleeping monsters near the given location
 */
static void vault_monsters(int y1, int x1, int num)
{
	int k, i, y, x;

	/* Try to summon "num" monsters "near" the given location */
	for (k = 0; k < num; k++)
	{
		/* Try nine locations */
		for (i = 0; i < 9; i++)
		{
			int d = 1;

			/* Pick a nearby location */
			scatter(&y, &x, y1, x1, d, 0);

			/* Require "empty" floor grids */
			if (!cave_empty_bold(y, x)) continue;

			/* Place the monster (allow groups) */
			monster_level = p_ptr->depth + 2;
			(void)place_monster(y, x, TRUE, TRUE);
			monster_level = p_ptr->depth;

			/* Check we have number */
			if (++k>=num) break;
		}
	}
}



/*
 * Generate helper -- create a new room with optional light
 */
static void generate_room(int y1, int x1, int y2, int x2, int light)
{
	int y, x;

	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			cave_info[y][x] |= (CAVE_ROOM);
			if (light) cave_info[y][x] |= (CAVE_GLOW);
		}
	}
}


/*
 * Generate helper -- fill a rectangle with a feature
 */
static void generate_fill(int y1, int x1, int y2, int x2, int feat)
{
	int y, x;

	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			cave_set_feat(y, x, feat);
		}
	}
}


/*
 * Generate helper -- draw a rectangle with a feature
 */
static void generate_rect(int y1, int x1, int y2, int x2, int feat)
{
	int y, x;

	for (y = y1; y <= y2; y++)
	{
		cave_set_feat(y, x1, feat);
		cave_set_feat(y, x2, feat);
	}

	for (x = x1; x <= x2; x++)
	{
		cave_set_feat(y1, x, feat);
		cave_set_feat(y2, x, feat);
	}
}



/*
 * Hack -- mimic'ed feature for "room_info_feat()"
 */
static s16b room_info_feat_mimic;

/*
 *
 */
static bool room_info_feat(int f_idx)
{
	feature_type *f_ptr = &f_info[f_idx];

	if (f_ptr->mimic == room_info_feat_mimic) return(TRUE);

	return(FALSE);
}


/*
 * Number to place for scattering
 */
#define NUM_SCATTER   7


/*
 * Generate helper -- draw a rectangle with a feature using a series of 'pattern' flags.
 */
static void generate_patt(int y1, int x1, int y2, int x2, int feat, u32b flag, int dy, int dx)
{
	int y, x, i, k;

	int y_alloc = 0, x_alloc = 0, choice;

	int offset = rand_int(100) < 50 ? 1 : 0;
	int max_offset = offset + 1;

	/* Paranoia */
	if (!dy || !dx) return;

	/* Pick features if needed */
	if ((feat) && (f_info[feat].mimic == feat))
	{
		/* Set feature hook */
		room_info_feat_mimic = feat;

		get_feat_num_hook = room_info_feat;

		/* Prepare allocation table */
		get_feat_num_prep();
	}

	/* Scatter several about if requested */
	for (k = 0; k < ((flag & (RG1_SCATTER | RG1_TRAIL)) != 0 ? NUM_SCATTER : 1); k++)
	{
		/* Pick location */
		choice = 0;

		/* Scan the whole room */
		for (y = y1; (dy > 0) ? y <= y2 : y >= y2; y += dy)
		{
			for (x = x1; (dx > 0) ? x <= x2 : x >= x2; x += dx)
			{
				/* Checkered room */
				if (((flag & (RG1_CHECKER)) != 0) && ((x + y + offset) % 2)) continue;

				/* Only place on outer wall */
				if (((flag & (RG1_OUTER)) != 0) && (cave_feat[y][x] != FEAT_WALL_OUTER)) continue;

				/* Only place on floor otherwise */
				if (((flag & (RG1_OUTER)) == 0) && (cave_feat[y][x] != FEAT_FLOOR))
				{
					if (((flag & (RG1_CHECKER)) != 0) && (offset < max_offset)) offset++;

					continue;
				}

				/* Clear max_offset */
				max_offset = 0;

				/* Only place on edge of room */
				if (((flag & (RG1_EDGE)) != 0) && (cave_feat[y][x] != FEAT_WALL_OUTER))
				{
					bool accept = FALSE;

					for (i = 0; i < 8; i++)
					{
						if (cave_feat[y + ddy_ddd[i]][x + ddx_ddd[i]] == FEAT_WALL_OUTER) accept = TRUE;
					}

					if (!accept) continue;
				}
				/* Don't place on edge of room */
				else if (((flag & (RG1_CENTRE)) != 0) && (cave_feat[y][x] != FEAT_WALL_OUTER))
				{
					bool accept = TRUE;

					for (i = 0; i < 4; i++)
					{
						if (cave_feat[y + ddy_ddd[i]][x + ddx_ddd[i]] == FEAT_WALL_OUTER) accept = FALSE;
					}

					if (!accept) continue;
				}

				/* Leave inner area open */
				if ((flag & (RG1_INNER)) != 0)
				{
					if (((dy > 0) ? (y > y1) && (y + dy <= y2) : (y + dy >= y2) && (y < y1)) &&
						((dx > 0) ? (x > x1) && (x + dx <= x2) : (x + dx >= x2) && (x < x1))) continue; 
				}

				/* Random */
				if (((flag & (RG1_RANDOM)) != 0) && (rand_int(100) < 40)) continue;

				/* Only place next to last choice */
				if ((flag & (RG1_TRAIL)) != 0)
				{
					/* Place next to previous position */
					if ((k == 0) || (distance(y, y_alloc, x, x_alloc) < ABS(dy) + ABS(dx)))
					{
						if (rand_int(++choice) == 0)
						{
							y_alloc = y;
							x_alloc = x;
						}
					}
				}
				/* Maybe pick if placing one */
				else if ((flag & (RG1_ALLOC | RG1_SCATTER | RG1_8WAY)) != 0)
				{
					if (rand_int(++choice) == 0)
					{
						y_alloc = y;
						x_alloc = x;
					}
				}

				/* Set feature */
				else
				{
					int place_feat = feat;

					/* Hack -- in case we don't place enough */
					if ((flag & (RG1_RANDOM)) != 0)
					{
						if (rand_int(++choice) == 0)
						{
							y_alloc = y;
							x_alloc = x;
						}
					}

					/* Pick a random feature? */
					if ((feat) && (f_info[feat].mimic == feat))
					{
						place_feat = get_feat_num(object_level);

						if (!place_feat) place_feat = feat;
					}

					/* Assign feature */
					if (place_feat) cave_set_feat(y, x, place_feat);

					/* Require "clean" floor space */
					if ((flag & (RG1_HAS_GOLD | RG1_HAS_ITEM)) != 0)
					{
						/* Either place or overwrite outer wall if required */
						if ((cave_clean_bold(y, x)) || (((flag & (RG1_OUTER)) != 0) && (cave_feat[y][x] == FEAT_WALL_OUTER)))
						{
							/* Hack -- erase outer wall */
							if (cave_feat[y][x] == FEAT_WALL_OUTER) cave_set_feat(y, x, FEAT_FLOOR);

							/* Drop gold 50% of the time if both defined */
							if (((flag & (RG1_HAS_GOLD)) != 0) && (((flag & (RG1_HAS_ITEM)) == 0) || (rand_int(100) < 50))) place_gold(y, x);
							else place_object(y, x, FALSE, FALSE);
						}
					}
				}
			}
		}

		/* Hack -- if we don't have enough the first time, scatter instead */
		if (((flag & (RG1_RANDOM)) != 0) && (choice < NUM_SCATTER))
		{
			flag &= ~(RG1_RANDOM);
			flag |= (RG1_SCATTER);

			/* Paranoia */
			if (!choice) continue;
		}

		/* Finally place in 8 directions */
		if (((flag & (RG1_8WAY)) != 0) && choice)
		{
			int place_feat = feat;

			/* Pick a random feature? */
			if ((feat) && (f_info[feat].mimic == feat))
			{
				place_feat = get_feat_num(object_level);

				if (!place_feat) place_feat = feat;
			}

			/* Loop through features */
			for (k = 0; k < MAX_SIGHT; k++)
			{
				for (i = 0; i < 8; i++)
				{
					/* Get position */
					y = y_alloc + k * ddy_ddd[i];
					x = x_alloc + k * ddx_ddd[i];

					/* Limit spread */
					if ((y < y1) || (y > y2) || (x < x1) || (x > x2)) continue;

					/* Assign feature */
					if (place_feat) cave_set_feat(y, x, place_feat);

					/* Require "clean" floor space */
					if ((flag & (RG1_HAS_GOLD | RG1_HAS_ITEM)) != 0)
					{
						/* Either place or overwrite outer wall if required */
						if ((cave_clean_bold(y, x)) || (((flag & (RG1_OUTER)) != 0) && (cave_feat[y][x] == FEAT_WALL_OUTER)))
						{
							/* Hack -- erase outer wall */
							if (cave_feat[y][x] == FEAT_WALL_OUTER) cave_set_feat(y, x, FEAT_FLOOR);

							/* Drop gold 50% of the time if both defined */
							if (((flag & (RG1_HAS_GOLD)) != 0) && (((flag & (RG1_HAS_ITEM)) == 0) || (rand_int(100) < 50))) place_gold(y, x);
							else place_object(y, x, FALSE, FALSE);
						}
					}
				}
			}
		}

		/* Finally place if allocating a single feature */
		else if (((flag & (RG1_ALLOC | RG1_SCATTER | RG1_TRAIL)) != 0) && choice)
		{
			int place_feat = feat;

			/* Get location */
			y = y_alloc;
			x = x_alloc;

			/* Pick a random feature? */
			if ((feat) && (f_info[feat].mimic == feat))
			{
				place_feat = get_feat_num(object_level);

				if (!place_feat) place_feat = feat;
			}

			/* Assign feature */
			if (place_feat) cave_set_feat(y, x, place_feat);

			/* Require "clean" floor space */
			if ((flag & (RG1_HAS_GOLD | RG1_HAS_ITEM)) != 0)
			{
				/* Either place or overwrite outer wall if required */
				if ((cave_clean_bold(y, x)) || (((flag & (RG1_OUTER)) != 0) && (cave_feat[y][x] == FEAT_WALL_OUTER)))
				{
					/* Hack -- erase outer wall */
					if (cave_feat[y][x] == FEAT_WALL_OUTER) cave_set_feat(y, x, FEAT_FLOOR);

					/* Drop gold 50% of the time if both defined */
					if (((flag & (RG1_HAS_GOLD)) != 0) && (((flag & (RG1_HAS_ITEM)) == 0) || (rand_int(100) < 50))) place_gold(y, x);
					else place_object(y, x, FALSE, FALSE);
				}
			}
		}
	}

	/* Clear feature hook */
	if ((feat) && (f_info[feat].mimic == feat))
	{
		/* Clear the hook */
		get_feat_num_hook = NULL;

		get_feat_num_prep();				
	}
}


/*
 * Generate helper -- split a rectangle with a feature
 */
static void generate_plus(int y1, int x1, int y2, int x2, int feat)
{
	int y, x;
	int y0, x0;

	/* Center */
	y0 = (y1 + y2) / 2;
	x0 = (x1 + x2) / 2;

	for (y = y1; y <= y2; y++)
	{
		cave_set_feat(y, x0, feat);
	}

	for (x = x1; x <= x2; x++)
	{
		cave_set_feat(y0, x, feat);
	}
}


/*
 * Generate helper -- open all sides of a rectangle with a feature
 */
static void generate_open(int y1, int x1, int y2, int x2, int feat)
{
	int y0, x0;

	/* Center */
	y0 = (y1 + y2) / 2;
	x0 = (x1 + x2) / 2;

	/* Open all sides */
	cave_set_feat(y1, x0, feat);
	cave_set_feat(y0, x1, feat);
	cave_set_feat(y2, x0, feat);
	cave_set_feat(y0, x2, feat);
}


/*
 * Generate helper -- open one side of a rectangle with a feature
 */
static void generate_hole(int y1, int x1, int y2, int x2, int feat)
{
	int y0, x0;

	/* Center */
	y0 = (y1 + y2) / 2;
	x0 = (x1 + x2) / 2;

	/* Open random side */
	switch (rand_int(4))
	{
		case 0:
		{
			cave_set_feat(y1, x0, feat);
			break;
		}
		case 1:
		{
			cave_set_feat(y0, x1, feat);
			break;
		}
		case 2:
		{
			cave_set_feat(y2, x0, feat);
			break;
		}
		case 3:
		{
			cave_set_feat(y0, x2, feat);
			break;
		}
	}
}


/*
 * Hack -- tval and sval range for "room_info_kind()"
 */
static byte room_info_kind_tval;
static byte room_info_kind_min_sval;
static byte room_info_kind_max_sval;

/*
 *
 */
static bool room_info_kind(int k_idx)
{
	object_kind *k_ptr = &k_info[k_idx];

	if (k_ptr->tval != room_info_kind_tval) return (FALSE);
	if (k_ptr->sval < room_info_kind_min_sval) return (FALSE);
	if (k_ptr->sval > room_info_kind_max_sval) return (FALSE);

	return(TRUE);
}


/*
 * Get the room description, and place stuff accordingly.
 */
static void get_room_info(int y0, int x0)
{
	int y1a, x1a, y2a, x2a;
	int y1b, x1b, y2b, x2b;

	int light = FALSE;

	int i, j, chart, pick, chance, count, counter;

	int room = dun->cent_n + 1;

	u32b place_flag = 0;

	byte place_tval, place_min_sval, place_max_sval;
	int place_feat;

	int branch;
	int branch_on;

	/* Occasional light */
	if (p_ptr->depth <= randint(25)) light = TRUE;

	/* Determine extents of room (a) */
	y1a = y0 - randint(4) - 1;
	x1a = x0 - randint(11) - 1;
	y2a = y0 + randint(3) + 1;
	x2a = x0 + randint(10) + 1;

	/* Determine extents of room (b) */
	y1b = y0 - randint(3) - 1;
	x1b = x0 - randint(10) - 1;
	y2b = y0 + randint(4) + 1;
	x2b = x0 + randint(11) + 1;

	/* Generate new room (a) */
	generate_room(y1a, x1a, y2a, x2a, light);

	/* Generate new room (b) */
	generate_room(y1b, x1b, y2b, x2b, light);

	/* Generate outer walls (a) */
	generate_rect(y1a, x1a, y2a, x2a, FEAT_WALL_OUTER);

	/* Generate outer walls (b) */
	generate_rect(y1b, x1b, y2b, x2b, FEAT_WALL_OUTER);

	/* Make corners solid */
	cave_set_feat(y1a, x1a, FEAT_WALL_SOLID);
	cave_set_feat(y2a, x2a, FEAT_WALL_SOLID);
	cave_set_feat(y1a, x2a, FEAT_WALL_SOLID);
	cave_set_feat(y2a, x1a, FEAT_WALL_SOLID);
	cave_set_feat(y1b, x1b, FEAT_WALL_SOLID);
	cave_set_feat(y2b, x2b, FEAT_WALL_SOLID);
	cave_set_feat(y1b, x2b, FEAT_WALL_SOLID);
	cave_set_feat(y2b, x1b, FEAT_WALL_SOLID);

	/* Generate inner floors (a) */
	generate_fill(y1a+1, x1a+1, y2a-1, x2a-1, FEAT_FLOOR);

	/* Generate inner floors (b) */
	generate_fill(y1b+1, x1b+1, y2b-1, x2b-1, FEAT_FLOOR);

	/* Start sections */
	chart = 1;
	j = 0;
	counter = 0;
	place_tval = 0;
	place_min_sval = 0;
	place_max_sval = 0;
	place_feat = 0;
	place_flag = 0;

	branch = 0;
	branch_on = 0;

	/* Room flags */
	room_info[room].flags = 0;

	/* Process the description */
	while (chart && (j < ROOM_DESC_SECTIONS - 1))
	{
		/* Start over */
		i = 0;
		count = 0;
		pick = -1;

		/* Get the start of entries in the table for this index */
		while ((chart != d_info[i].chart) && (counter < 5000)) { i++; counter++;}

		/* Cycle through valid entries */
		while (chart == d_info[i].chart)
		{
			counter++;
			if (counter > 10000)
			{
				msg_format("Error: loop in chart position %d", chart);
				break;
			}

			/* If not allowed at this depth, skip completely */
			while (p_ptr->depth < d_info[i].level_min) i++;
			while (p_ptr->depth > d_info[i].level_max) i++;

			/* Get chance */
			chance = d_info[i].chance;

			/* If requires this level type, reduce chance of occurring */
			if ((d_info[i].l_flag) && ((level_flag & d_info[i].l_flag) == 0)) chance = d_info[i].not_chance;

			/* If not allowed because doesn't match level monster, reduce chance of occuring */
			else if ((cave_ecology.ready) && (cave_ecology.num_races))
			{
				/* Match main monster */
				monster_race *r_ptr = &r_info[cave_ecology.race[0]];

				/* Check for char match */
				if ((d_info[i].r_char) && (d_info[i].r_char != r_ptr->d_char)) chance = d_info[i].not_chance;

				/* Check for flag match */
				if (d_info[i].r_flag)
				{
					if ((d_info[i].r_flag < 33) && 
						((r_ptr->flags1 & (1L << (d_info[i].r_flag - 1))) == 0)) chance = d_info[i].not_chance;

					if ((d_info[i].r_flag >= 33) && 
						(d_info[i].r_flag < 65) && 
						((r_ptr->flags2 & (1L << (d_info[i].r_flag - 33))) == 0)) chance = d_info[i].not_chance;

					if ((d_info[i].r_flag >= 65) && 
						(d_info[i].r_flag < 97) && 
						((r_ptr->flags3 & (1L << (d_info[i].r_flag - 65))) == 0)) chance = d_info[i].not_chance;

					if ((d_info[i].r_flag >= 97) && 
						(d_info[i].r_flag < 129) && 
						((r_ptr->flags4 & (1L << (d_info[i].r_flag - 97))) == 0)) chance = d_info[i].not_chance;

					if ((d_info[i].r_flag >= 129) && 
						(d_info[i].r_flag < 161) && 
						((r_ptr->flags5 & (1L << (d_info[i].r_flag - 129))) == 0)) chance = d_info[i].not_chance;

					if ((d_info[i].r_flag >= 161) && 
						(d_info[i].r_flag < 193) && 
						((r_ptr->flags6 & (1L << (d_info[i].r_flag - 161))) == 0)) chance = d_info[i].not_chance;

					if ((d_info[i].r_flag >= 193) && 
						(d_info[i].r_flag < 225) && 
						((r_ptr->flags7 & (1L << (d_info[i].r_flag - 193))) == 0)) chance = d_info[i].not_chance;

					if ((d_info[i].r_flag >= 225) && 
						(d_info[i].r_flag < 257) && 
						((r_ptr->flags8 & (1L << (d_info[i].r_flag - 225))) == 0)) chance = d_info[i].not_chance;

					if ((d_info[i].r_flag >= 257) && 
						(d_info[i].r_flag < 289) && 
						((r_ptr->flags9 & (1L << (d_info[i].r_flag - 257))) == 0)) chance = d_info[i].not_chance;
				}
			}

			/* Chance of room entry */
			if (chance)
			{
				/* Add to chances */
				count += chance;

				/* Check chance */
				if (rand_int(count) < chance) pick = i;
			}

			/* Increase index */
			i++;
		}

		/* Paranoia -- Have picked any entry? */
		if (pick >= 0)
		{
			/* Set index to choice */
			i = pick;

			/* Save index */
			room_info[room].section[j++] = i;

			/* Enter the next chart */
			chart = d_info[i].next;

			/* Branch if required */
			if (chart == branch_on)
			{
				/* Set alternate chart */
				chart = branch;

				/* Clear branch conditions */
				branch = 0;
				branch_on = 0;
			}

			/* Place flags except SEEN */
			room_info[room].flags |= (d_info[i].flags & ~(ROOM_SEEN));
		
			/* Get tval */
			if (d_info[i].tval)
			{
				place_tval = d_info[i].tval;
				place_min_sval = d_info[i].min_sval;
				place_max_sval = d_info[i].max_sval;
			}

			/* Get feature */
			if (d_info[i].feat) place_feat = d_info[i].feat;

			/* Get branch */
			if (d_info[i].branch) branch = d_info[i].branch;

			/* Get branch condition */
			if (d_info[i].branch_on) branch_on = d_info[i].branch_on;

			/* Clear old position information */
			if (d_info[i].p_flag & (RG1_PLACE_FLAGS)) place_flag &= ~(RG1_PLACE_FLAGS);

			/* Add flags */
			place_flag |= d_info[i].p_flag;

			/* Add level flags */
			level_flag |= (d_info[i].p_flag & (RG1_LEVEL_FLAGS));

			/* Don't place yet */
			if ((place_flag & (RG1_PLACE)) == 0) continue;

			/* Pick objects if needed */
			if (place_tval)
			{
				/* Set object hooks if required */
				if (place_tval < TV_GOLD)
				{
					room_info_kind_tval = place_tval;
					room_info_kind_min_sval = place_min_sval;
					room_info_kind_max_sval = place_max_sval;

					get_obj_num_hook = room_info_kind;

					/* Prepare allocation table */
					get_obj_num_prep();

					/* Drop gold */
					place_flag |= (RG1_HAS_ITEM);
				}
				else
				{
					/* Drop gold */
					place_flag |= (RG1_HAS_GOLD);
				}
			}

			/* Place features or items if needed */
			if ((place_feat) || (place_tval))
			{
				int dy = ((place_flag & (RG1_ROWS)) != 0) ? 2 : 1;
				int dx = ((place_flag & (RG1_COLS)) != 0) ? 2 : 1;
				int outer = ((place_flag & (RG1_OUTER)) == 0) ? 1 : 0;

				/* Place in centre of room */
				if ((place_flag & (RG1_CENTRE)) != 0)
				{
					int y1c = MAX(y1a, y1b) + 1;
					int y2c = MIN(y2a, y2b) - 1;
					int x1c = MAX(x1a, x1b) + 1;
					int x2c = MIN(x2a, x2b) - 1;
					u32b place_flag_temp = place_flag;

					/* Ensure some space */
					if (y1c >= y2c) { y1c = y2c - 1; y2c = y1c + 3;}
					if (x1c >= x2c) { x1c = x2c - 1; x2c = x1c + 3;}

					/* Hack -- 'outer' and 'edge' walls do not exist in centre of room */
					if ((place_flag_temp & (RG1_OUTER | RG1_EDGE)) != 0)
					{
						place_flag_temp &= ~(RG1_OUTER | RG1_EDGE);
						place_flag_temp |= (RG1_INNER);
					}

					generate_patt(y1c, x1c, y2c, x2c, place_feat, place_flag_temp, dy, dx);
				}

				/* Place in west of room */
				if ((place_flag & (RG1_WEST)) != 0)
				{
					int y1w = (x1a < x1b ? y1a : (x1a == x1b ? MIN(y1a, y1b) : y1b));
					int y2w = (x1a < x1b ? y2a : (x1a == x1b ? MAX(y2a, y2b) : y2b));
					int x1w = MIN(x1a, x1b) + outer;
					int x2w = (x1a == x1b ? x1a + 1 : MAX(x1a, x1b) - 1);

					/* Ensure some space */
					if (x2w <= x2w) x2w = x1w + 1;

					generate_patt(y1w, x1w, y2w, x2w, place_feat, place_flag, dy, dx);
				}

				/* Place in east of room */
				if ((place_flag & (RG1_EAST)) != 0)
				{
					int y1e = (x2a > x2b ? y1a : (x1a == x1b ? MIN(y1a, y1b): y1b));
					int y2e = (x2a > x2b ? y2a : (x1a == x1b ? MAX(y2a, y2b): y2b));
					int x1e = (x2a == x2b ? x2a - 1 : MIN(x2a, x2b) + 1);
					int x2e = MAX(x2a, x2b) - outer;

					/* Ensure some space */
					if (x1e >= x2e) x1e = x2e - 1;

					/* Draw from east to west */
					generate_patt(y1e, x2e, y2e, x1e, place_feat, place_flag, dy, -dx);
				}

				/* Place in north of room */
				if ((place_flag & (RG1_NORTH)) != 0)
				{
					int y1n = MIN(y1a, y1b) + outer;
					int y2n = (y1a == y1b ? y1a + 1 : MAX(y1a, y1b) - 1);
					int x1n = (y1a < y1b ? x1a : (y1a == y1b ? MIN(x1a, x1b): y1b));
					int x2n = (y1a < y1b ? x2a : (y1a == y1b ? MAX(x2a, x2b): x2b));

					/* Ensure some space */
					if (y2n <= y1n) y2n = y1n + 1;

					generate_patt(y1n, x1n, y2n, x2n, place_feat, place_flag, dy, dx);
				}

				/* Place in south of room */
				if ((place_flag & (RG1_SOUTH)) != 0)
				{
					int y1s = (y2a == y2b ? y2a - 1 : MIN(y2a, y2b) + 1);
					int y2s = MAX(y2a, y2b) - outer;
					int x1s = (y2a > y2b ? x1a : (y2a == y2b ? MIN(x1a, x1b): x1b));
					int x2s = (y2a > y2b ? x2a : (y2a == y2b ? MAX(x2a, x2b): x2b));

					/* Ensure some space */
					if (y1s >= y2s) y1s = y2s - 1;

					/* Draw from south to north */
					generate_patt(y2s, x1s, y1s, x2s, place_feat, place_flag, -dy, dx);
				}
			}

			/* Clear object hook */
			if (place_tval < 100)
			{
				get_obj_num_hook = NULL;

				/* Prepare allocation table */
				get_obj_num_prep();

				place_tval = 0;
			}

			/* Clear placement details */
			place_flag &= (RG1_PLACE_FLAGS);
			place_feat = 0;
		}

		/* Report errors */
		if (pick < 0)
		{
			msg_format("Error: unable to pick from chart entry %d with %d valid choices", chart, count);
			chart = 0;
		}
	}

	/* Type */
	room_info[room].type = ROOM_NORMAL;

	/* Terminate index list */
	room_info[room].section[j] = -1;

}


/*
 * Room building routines.
 *
 * Six basic room types:
 *   1 -- normal
 *   2 -- overlapping
 *   3 -- cross shaped
 *   4 -- large room with features
 *   5 -- monster nests
 *   6 -- monster pits
 *   7 -- simple vaults
 *   8 -- greater vaults
 */

#if 0

/*
 * Type 1 -- normal rectangular rooms
 */
static void build_type1(int y0, int x0)
{
	int y, x;

	int y1, x1, y2, x2;

	int light = FALSE;


	/* Occasional light */
	if (p_ptr->depth <= randint(25)) light = TRUE;


	/* Pick a room size */
	y1 = y0 - randint(4);
	x1 = x0 - randint(11);
	y2 = y0 + randint(3);
	x2 = x0 + randint(11);

	/* Generate new room */
	generate_room(y1-1, x1-1, y2+1, x2+1, light);

	/* Generate outer walls */
	generate_rect(y1-1, x1-1, y2+1, x2+1, FEAT_WALL_OUTER);

	/* Generate inner floors */
	generate_fill(y1, x1, y2, x2, FEAT_FLOOR);

	/* Hack -- Occasional pillar room */
	if ((level_flag & (LF1_CRYPT)) != 0)
	{
		switch (rand_int(2))
		{
			case 0:
				/* Pillared room */
				for (y = y1; y <= y2; y += 2)
				{
					for (x = x1; x <= x2; x += 2)
					{
						cave_set_feat(y, x, FEAT_WALL_INNER);
					}
				}
				break;

			case 1:
				/* Ragged edged room */
				for (y = y1 + 2; y <= y2 - 2; y += 2)
				{
					cave_set_feat(y, x1, FEAT_WALL_INNER);
					cave_set_feat(y, x2, FEAT_WALL_INNER);
				}

				for (x = x1 + 2; x <= x2 - 2; x += 2)
				{
					cave_set_feat(y1, x, FEAT_WALL_INNER);
					cave_set_feat(y2, x, FEAT_WALL_INNER);
				}
				break;
		}
	}

	/* Hack -- mark light rooms */
	if (light) level_flag &= ~(LF1_DARK);
	else level_flag |= (LF1_DARK) ;

	/* Pretty description and maybe more monsters/objects/traps*/
	get_room_info(y0,x0);
}


/*
 * Type 2 -- Overlapping rectangular rooms
 */
static void build_type2(int y0, int x0)
{
	int y1a, x1a, y2a, x2a;
	int y1b, x1b, y2b, x2b;

	int light = FALSE;


	/* Occasional light */
	if (p_ptr->depth <= randint(25)) light = TRUE;


	/* Determine extents of room (a) */
	y1a = y0 - randint(4);
	x1a = x0 - randint(11);
	y2a = y0 + randint(3);
	x2a = x0 + randint(10);


	/* Determine extents of room (b) */
	y1b = y0 - randint(3);
	x1b = x0 - randint(10);
	y2b = y0 + randint(4);
	x2b = x0 + randint(11);


	/* Generate new room (a) */
	generate_room(y1a-1, x1a-1, y2a+1, x2a+1, light);

	/* Generate new room (b) */
	generate_room(y1b-1, x1b-1, y2b+1, x2b+1, light);

	/* Generate outer walls (a) */
	generate_rect(y1a-1, x1a-1, y2a+1, x2a+1, FEAT_WALL_OUTER);

	/* Generate outer walls (b) */
	generate_rect(y1b-1, x1b-1, y2b+1, x2b+1, FEAT_WALL_OUTER);

	/* Generate inner floors (a) */
	generate_fill(y1a, x1a, y2a, x2a, FEAT_FLOOR);

	/* Generate inner floors (b) */
	generate_fill(y1b, x1b, y2b, x2b, FEAT_FLOOR);


	/* Hack -- mark light rooms */
	if (light) level_flag &= ~(LF1_DARK);
	else level_flag |= (LF1_DARK);

	/* Pretty description and maybe more monsters/objects/traps*/
	get_room_info(y0,x0);
}


/*
 * Type 3 -- Cross shaped rooms
 *
 * Room "a" runs north/south, and Room "b" runs east/east
 * So a "central pillar" would run from x1a,y1b to x2a,y2b.
 *
 * Note that currently, the "center" is always 3x3, but I think that
 * the code below will work for 5x5 (and perhaps even for unsymetric
 * values like 4x3 or 5x3 or 3x4 or 3x5).
 */
static void build_type3(int y0, int x0)
{
	int y, x;

	int y1a, x1a, y2a, x2a;
	int y1b, x1b, y2b, x2b;

	int dy, dx, wy, wx;

	int light = FALSE;


	/* Occasional light */
	if (p_ptr->depth <= randint(25)) light = TRUE;


	/* Pick inner dimension */
	wy = 1;
	wx = 1;

	/* Pick outer dimension */
	dy = rand_range(3, 4);
	dx = rand_range(3, 11);


	/* Determine extents of room (a) */
	y1a = y0 - dy;
	x1a = x0 - wx;
	y2a = y0 + dy;
	x2a = x0 + wx;

	/* Determine extents of room (b) */
	y1b = y0 - wy;
	x1b = x0 - dx;
	y2b = y0 + wy;
	x2b = x0 + dx;


	/* Generate new room (a) */
	generate_room(y1a-1, x1a-1, y2a+1, x2a+1, light);

	/* Generate new room (b) */
	generate_room(y1b-1, x1b-1, y2b+1, x2b+1, light);

	/* Generate outer walls (a) */
	generate_rect(y1a-1, x1a-1, y2a+1, x2a+1, FEAT_WALL_OUTER);

	/* Generate outer walls (b) */
	generate_rect(y1b-1, x1b-1, y2b+1, x2b+1, FEAT_WALL_OUTER);

	/* Generate inner floors (a) */
	generate_fill(y1a, x1a, y2a, x2a, FEAT_FLOOR);

	/* Generate inner floors (b) */
	generate_fill(y1b, x1b, y2b, x2b, FEAT_FLOOR);


	/* Special features */
	switch (randint(4))
	{
		/* Nothing */
		case 1:
		{
			break;
		}

		/* Large solid middle pillar */
		case 2:
		{
			/* Generate a small inner solid pillar */
			generate_fill(y1b, x1a, y2b, x2a, FEAT_WALL_INNER);

			break;
		}

		/* Inner treasure vault */
		case 3:
		{
			/* Generate a small inner vault */
			generate_rect(y1b, x1a, y2b, x2a, FEAT_WALL_INNER);

			/* Open the inner vault with a secret door */
			generate_hole(y1b, x1a, y2b, x2a, FEAT_SECRET);

			/* Place a treasure in the vault */
			place_object(y0, x0, FALSE, FALSE);

			/* Let's guard the treasure well */
			vault_monsters(y0, x0, rand_int(2) + 3);

			/* Traps naturally */
			vault_traps(y0, x0, 4, 4, rand_int(3) + 2);

			break;
		}

		/* Something else */
		case 4:
		{
			/* Occasionally pinch the center shut */
			if (rand_int(3) == 0)
			{
				/* Pinch the east/west sides */
				for (y = y1b; y <= y2b; y++)
				{
					if (y == y0) continue;
					cave_set_feat(y, x1a - 1, FEAT_WALL_INNER);
					cave_set_feat(y, x2a + 1, FEAT_WALL_INNER);
				}

				/* Pinch the north/south sides */
				for (x = x1a; x <= x2a; x++)
				{
					if (x == x0) continue;
					cave_set_feat(y1b - 1, x, FEAT_WALL_INNER);
					cave_set_feat(y2b + 1, x, FEAT_WALL_INNER);
				}

				/* Open sides with secret doors */
				if (rand_int(3) == 0)
				{
					generate_open(y1b-1, x1a-1, y2b+1, x2a+1, FEAT_SECRET);
				}
			}

			/* Occasionally put a "plus" in the center */
			else if (rand_int(3) == 0)
			{
				generate_plus(y1b, x1a, y2b, x2a, FEAT_WALL_INNER);
			}

			/* Occasionally put a "pillar" in the center */
			else if (rand_int(3) == 0)
			{
				cave_set_feat(y0, x0, FEAT_WALL_INNER);
			}

			break;
		}
	}


	/* Hack -- mark light rooms */
	if (light) level_flag &= ~(LF1_DARK);
	else level_flag |= (LF1_DARK);


	/* Pretty description and maybe more monsters/objects/traps*/
	get_room_info(y0,x0);
}


/*
 * Type 4 -- Large room with an inner room
 *
 * Possible sub-types:
 *      1 - An inner room
 *      2 - An inner room with a small inner room
 *      3 - An inner room with a pillar or pillars
 *      4 - An inner room with a checkerboard
 *      5 - An inner room with four compartments
 */
static void build_type4(int y0, int x0)
{
	int y, x, y1, x1, y2, x2;

	int light = FALSE;


	/* Occasional light */
	if (p_ptr->depth <= randint(25)) light = TRUE;


	/* Large room */
	y1 = y0 - 4;
	y2 = y0 + 4;
	x1 = x0 - 11;
	x2 = x0 + 11;


	/* Generate new room */
	generate_room(y1-1, x1-1, y2+1, x2+1, light);

	/* Generate outer walls */
	generate_rect(y1-1, x1-1, y2+1, x2+1, FEAT_WALL_OUTER);

	/* Generate inner floors */
	generate_fill(y1, x1, y2, x2, FEAT_FLOOR);


	/* The inner room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* Generate inner walls */
	generate_rect(y1-1, x1-1, y2+1, x2+1, FEAT_WALL_INNER);


	/* Inner room variations */
	switch (randint(5))
	{
		/* An inner room */
		case 1:
		{
			/* Open the inner room with a secret door */
			generate_hole(y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

			/* Place a monster in the room */
			vault_monsters(y0, x0, 1);

			break;
		}


		/* An inner room with a small inner room */
		case 2:
		{
			/* Open the inner room with a secret door */
			generate_hole(y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

			/* Place another inner room */
			generate_rect(y0-1, x0-1, y0+1, x0+1, FEAT_WALL_INNER);

			/* Open the inner room with a locked door */
			generate_hole(y0-1, x0-1, y0+1, x0+1, FEAT_DOOR_HEAD + randint(7));

			/* Monsters to guard the treasure */
			vault_monsters(y0, x0, randint(3) + 2);

			/* Object (80%) */
			if (rand_int(100) < 80)
			{
				place_object(y0, x0, FALSE, FALSE);
			}

			/* Stairs (20%) */
			else
			{
				place_random_stairs(y0, x0, 0);
			}

			/* Traps to protect the treasure */
			vault_traps(y0, x0, 4, 10, 2 + randint(3));

			break;
		}


		/* An inner room with an inner pillar or pillars */
		case 3:
		{
			/* Open the inner room with a secret door */
			generate_hole(y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

			/* Inner pillar */
			generate_fill(y0-1, x0-1, y0+1, x0+1, FEAT_WALL_INNER);

			/* Occasionally, two more Large Inner Pillars */
			if (rand_int(2) == 0)
			{
				/* Three spaces */
				if (rand_int(100) < 50)
				{
					/* Inner pillar */
					generate_fill(y0-1, x0-7, y0+1, x0-5, FEAT_WALL_INNER);

					/* Inner pillar */
					generate_fill(y0-1, x0+5, y0+1, x0+7, FEAT_WALL_INNER);
				}

				/* Two spaces */
				else
				{
					/* Inner pillar */
					generate_fill(y0-1, x0-6, y0+1, x0-4, FEAT_WALL_INNER);

					/* Inner pillar */
					generate_fill(y0-1, x0+4, y0+1, x0+6, FEAT_WALL_INNER);
				}
			}

			/* Occasionally, some Inner rooms */
			if (rand_int(3) == 0)
			{
				/* Inner rectangle */
				generate_rect(y0-1, x0-5, y0+1, x0+5, FEAT_WALL_INNER);

				/* Secret doors (random top/bottom) */
				place_secret_door(y0 - 3 + (randint(2) * 2), x0 - 3);
				place_secret_door(y0 - 3 + (randint(2) * 2), x0 + 3);

				/* Monsters */
				vault_monsters(y0, x0 - 2, randint(2));
				vault_monsters(y0, x0 + 2, randint(2));

				/* Objects */
				if (rand_int(3) == 0) place_object(y0, x0 - 2, FALSE, FALSE);
				if (rand_int(3) == 0) place_object(y0, x0 + 2, FALSE, FALSE);
			}

			break;
		}


		/* An inner room with a checkerboard */
		case 4:
		{
			/* Open the inner room with a secret door */
			generate_hole(y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

			/* Checkerboard */
			for (y = y1; y <= y2; y++)
			{
				for (x = x1; x <= x2; x++)
				{
					if ((x + y) & 0x01)
					{
						cave_set_feat(y, x, FEAT_WALL_INNER);
					}
				}
			}

			/* Monsters just love mazes. */
			vault_monsters(y0, x0 - 5, randint(3));
			vault_monsters(y0, x0 + 5, randint(3));

			/* Traps make them entertaining. */
			vault_traps(y0, x0 - 3, 2, 8, randint(3));
			vault_traps(y0, x0 + 3, 2, 8, randint(3));

			/* Mazes should have some treasure too. */
			vault_objects(y0, x0, 3);

			break;
		}


		/* Four small rooms. */
		case 5:
		{
			/* Inner "cross" */
			generate_plus(y1, x1, y2, x2, FEAT_WALL_INNER);

			/* Doors into the rooms */
			if (rand_int(100) < 50)
			{
				int i = randint(10);
				place_secret_door(y1 - 1, x0 - i);
				place_secret_door(y1 - 1, x0 + i);
				place_secret_door(y2 + 1, x0 - i);
				place_secret_door(y2 + 1, x0 + i);
			}
			else
			{
				int i = randint(3);
				place_secret_door(y0 + i, x1 - 1);
				place_secret_door(y0 - i, x1 - 1);
				place_secret_door(y0 + i, x2 + 1);
				place_secret_door(y0 - i, x2 + 1);
			}

			/* Treasure, centered at the center of the cross */
			vault_objects(y0, x0, 2 + randint(2));

			/* Gotta have some monsters */
			vault_monsters(y0 + 1, x0 - 4, randint(4));
			vault_monsters(y0 + 1, x0 + 4, randint(4));
			vault_monsters(y0 - 1, x0 - 4, randint(4));
			vault_monsters(y0 - 1, x0 + 4, randint(4));

			break;
		}
	}

	/* Initialise room description */
	room_info[dun->cent_n+1].type = ROOM_LARGE;
	room_info[dun->cent_n+1].flags = 0;
}


/*
 * The following functions are used to determine if the given monster
 * is appropriate for inclusion in a monster nest or monster pit or
 * the given type.
 *
 * None of the pits/nests are allowed to include "unique" monsters,
 * or monsters which can "multiply".
 *
 * Some of the pits/nests are asked to avoid monsters which can blink
 * away or which are invisible.  This is probably a hack.
 *
 * The old method used monster "names", which was bad, but the new
 * method uses monster race characters, which is also bad.  XXX XXX XXX
 */


/*
 * Helper function for "monster nest (jelly)"
 */
static bool vault_aux_jelly(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Require icky thing, jelly, mold, or mushroom */
	if (!strchr("ijm,", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster nest (animal)"
 */
static bool vault_aux_animal(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Require "animal" flag */
	if (!(r_ptr->flags3 & (RF3_ANIMAL))) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster nest (undead)"
 */
static bool vault_aux_undead(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Require Undead */
	if (!(r_ptr->flags3 & (RF3_UNDEAD))) return (FALSE);

	/* Okay */
	return (TRUE);
}

/*
 * Helper function for "monster nest (theme)"
 */
static bool vault_aux_theme_nest(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Hack -- Accept monsters with flag */
	if (t_info[p_ptr->dungeon].r_flag)
	{
		int mon_flag = t_info[p_ptr->dungeon].r_flag-1;

		if ((mon_flag < 32) && 
			(r_ptr->flags1 & (1L << mon_flag))) return (TRUE);

		if ((mon_flag >= 32) && 
			(mon_flag < 64) && 
			(r_ptr->flags2 & (1L << (mon_flag -32)))) return (TRUE);

		if ((mon_flag >= 64) && 
			(mon_flag < 96) && 
			(r_ptr->flags3 & (1L << (mon_flag -64)))) return (TRUE);

		if ((mon_flag >= 96) && 
			(mon_flag < 128) && 
			(r_ptr->flags4 & (1L << (mon_flag -96)))) return (TRUE);

		return (FALSE);
	}

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster pit (theme)"
 */
static bool vault_aux_theme_pit(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Hack -- Accept monsters with graphic */
	if (t_info[p_ptr->dungeon].r_char)
	{
		if (r_ptr->d_char == t_info[p_ptr->dungeon].r_char) return (TRUE);

		return (FALSE);
	}

	/* Okay */
	return (TRUE);
}



/*
 * Helper function for "monster pit (orc)"
 */
static bool vault_aux_orc(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Hack -- Require orcs */
	if (!strchr("o", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster pit (troll)"
 */
static bool vault_aux_troll(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Hack -- Require trolls monster */
	if (!strchr("T", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster pit (giant)"
 */
static bool vault_aux_giant(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Hack -- Require giants */
	if (!strchr("P", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Hack -- breath type for "vault_aux_dragon()"
 */
static u32b vault_aux_dragon_mask4;


/*
 * Helper function for "monster pit (dragon)"
 */
static bool vault_aux_dragon(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Hack -- Require dragons (note that this includes "A" now)  */
	if (!strchr("ADd", r_ptr->d_char)) return (FALSE);

	/* Hack -- Require correct "breath attack" */
	if (r_ptr->flags4 != vault_aux_dragon_mask4) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster pit (demon)"
 */
static bool vault_aux_demon(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Hack -- Require "U" monsters */
	if (!strchr("U", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}



/*
 * Type 5 -- Monster nests
 *
 * A monster nest is a "big" room, with an "inner" room, containing
 * a "collection" of monsters of a given type strewn about the room.
 *
 * The monsters are chosen from a set of 64 randomly selected monster
 * races, to allow the nest creation to fail instead of having "holes".
 *
 * Note the use of the "get_mon_num_prep()" function, and the special
 * "get_mon_num_hook()" restriction function, to prepare the "monster
 * allocation table" in such a way as to optimize the selection of
 * "appropriate" non-unique monsters for the nest.
 *
 * Currently, a monster nest is one of
 *   a nest of "jelly" monsters   (Dungeon level 5 and deeper)
 *   a nest of "animal" monsters  (Dungeon level 30 and deeper)
 *   a nest of "undead" monsters  (Dungeon level 50 and deeper)
 *
 * Note that the "get_mon_num()" function may (rarely) fail, in which
 * case the nest will be empty, and will not affect the level rating.
 *
 * Note that "monster nests" will never contain "unique" monsters.
 */
static void build_type5(int y0, int x0)
{
	int y, x, y1, x1, y2, x2;

	int tmp, i;

	s16b what[64];

	cptr name;

	bool empty = FALSE;

	int light = FALSE;

	int rating_bonus;

	/* Large room */
	y1 = y0 - 4;
	y2 = y0 + 4;
	x1 = x0 - 11;
	x2 = x0 + 11;


	/* Generate new room */
	generate_room(y1-1, x1-1, y2+1, x2+1, light);

	/* Generate outer walls */
	generate_rect(y1-1, x1-1, y2+1, x2+1, FEAT_WALL_OUTER);

	/* Generate inner floors */
	generate_fill(y1, x1, y2, x2, FEAT_FLOOR);


	/* Advance to the center room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* Generate inner walls */
	generate_rect(y1-1, x1-1, y2+1, x2+1, FEAT_WALL_INNER);

	/* Open the inner room with a secret door */
	generate_hole(y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);


	/* Hack -- Choose a nest type */
	tmp = randint(p_ptr->depth*2);

	/* Dungeon pit */
	if ((adult_campaign) && (tmp < 25))
	{
		/* Describe */
		name = "themed";

		get_mon_num_hook = vault_aux_theme_nest;

		/* Appropriate rating bonus */
		rating_bonus = 10;

		/* Initialise room description */
		room_info[dun->cent_n+1].type = ROOM_NEST_THEME;

	}
	/* Monster nest (jelly) */
	else if (tmp < 30)
	{
		/* Describe */
		name = "jelly";

		/* Restrict to jelly */
		get_mon_num_hook = vault_aux_jelly;

		/* Appropriate rating bonus */
		rating_bonus = 25 - p_ptr->depth;

		/* Initialise room description */
		room_info[dun->cent_n+1].type = ROOM_NEST_JELLY;
	}

	/* Monster nest (animal) */
	else if (tmp < 50)
	{
		/* Describe */
		name = "animal";

		/* Restrict to animal */
		get_mon_num_hook = vault_aux_animal;

		/* Appropriate rating bonus */
		rating_bonus = 45 - p_ptr->depth;


		/* Initialise room description */
		room_info[dun->cent_n+1].type = ROOM_NEST_ANIMAL;
	}

	/* Monster nest (undead) */
	else
	{
		/* Describe */
		name = "undead";

		/* Restrict to undead */
		get_mon_num_hook = vault_aux_undead;

		/* Appropriate rating bonus */
		rating_bonus = 85 - p_ptr->depth;

		/* Initialise room description */
		room_info[dun->cent_n+1].type = ROOM_NEST_UNDEAD;
	}

	/* Prepare allocation table */
	get_mon_num_prep();


	/* Pick some monster types */
	for (i = 0; i < 64; i++)
	{
		/* Get a (hard) monster type */
		what[i] = get_mon_num(p_ptr->depth + 10);

		/* Notice failure */
		if (!what[i]) empty = TRUE;
	}


	/* Remove restriction */
	get_mon_num_hook = NULL;

	/* Prepare allocation table */
	get_mon_num_prep();


	/* Oops */
	if (empty) return;


	/* Describe */
	if (cheat_room)
	{
		/* Room type */
		msg_format("Monster nest (%s)", name);
	}

	/* Initialize room description */
	room_info[dun->cent_n+1].flags = 0;

	/* Increase the level rating */
	if (rating_bonus > 0) rating += rating_bonus;

	/* (Sometimes) Cause a "special feeling" (for "Monster Nests") */
	if ((p_ptr->depth <= 40) &&
	    (randint(p_ptr->depth * p_ptr->depth + 1) < 300))
	{
		good_item_flag = TRUE;
	}


	/* Place some monsters */
	for (y = y0 - 2; y <= y0 + 2; y++)
	{
		for (x = x0 - 9; x <= x0 + 9; x++)
		{
			int r_idx = what[rand_int(64)];

			/* Place that "random" monster (no groups) */
			(void)place_monster_aux(y, x, r_idx, FALSE, FALSE);
		}
	}
}



/*
 * Type 6 -- Monster pits
 *
 * A monster pit is a "big" room, with an "inner" room, containing
 * a "collection" of monsters of a given type organized in the room.
 *
 * Monster types in the pit
 *   orc pit    (Dungeon Level 5 and deeper)
 *   troll pit  (Dungeon Level 20 and deeper)
 *   giant pit  (Dungeon Level 40 and deeper)
 *   dragon pit (Dungeon Level 60 and deeper)
 *   demon pit  (Dungeon Level 80 and deeper)
 *
 * The inside room in a monster pit appears as shown below, where the
 * actual monsters in each location depend on the type of the pit
 *
 *   #####################
 *   #0000000000000000000#
 *   #0112233455543322110#
 *   #0112233467643322110#
 *   #0112233455543322110#
 *   #0000000000000000000#
 *   #####################
 *
 * Note that the monsters in the pit are now chosen by using "get_mon_num()"
 * to request 16 "appropriate" monsters, sorting them by level, and using
 * the "even" entries in this sorted list for the contents of the pit.
 *
 * Hack -- all of the "dragons" in a "dragon" pit must be the same "color",
 * which is handled by requiring a specific "breath" attack for all of the
 * dragons.  This may include "multi-hued" breath.  Note that "wyrms" may
 * be present in many of the dragon pits, if they have the proper breath.
 *
 * Note the use of the "get_mon_num_prep()" function, and the special
 * "get_mon_num_hook()" restriction function, to prepare the "monster
 * allocation table" in such a way as to optimize the selection of
 * "appropriate" non-unique monsters for the pit.
 *
 * Note that the "get_mon_num()" function may (rarely) fail, in which case
 * the pit will be empty, and will not effect the level rating.
 *
 * Note that "monster pits" will never contain "unique" monsters.
 */
static void build_type6(int y0, int x0)
{
	int tmp, what[16];

	int i, j, y, x, y1, x1, y2, x2;

	int rating_bonus=0;

	bool empty = FALSE;

	int light = FALSE;

	cptr name ="monster";


	/* Large room */
	y1 = y0 - 4;
	y2 = y0 + 4;
	x1 = x0 - 11;
	x2 = x0 + 11;


	/* Generate new room */
	generate_room(y1-1, x1-1, y2+1, x2+1, light);

	/* Generate outer walls */
	generate_rect(y1-1, x1-1, y2+1, x2+1, FEAT_WALL_OUTER);

	/* Generate inner floors */
	generate_fill(y1, x1, y2, x2, FEAT_FLOOR);


	/* Advance to the center room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* Generate inner walls */
	generate_rect(y1-1, x1-1, y2+1, x2+1, FEAT_WALL_INNER);

	/* Open the inner room with a secret door */
	generate_hole(y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);


	/* Choose a pit type */
	tmp = randint(p_ptr->depth*2);

	/* Dungeon pit */
	if ((adult_campaign) && (tmp < 10))
	{
		/* Describe */
		name = "themed";

		get_mon_num_hook = vault_aux_theme_pit;

		/* Appropriate rating bonus */
		rating_bonus = 10;

		/* Initialise room description */
		room_info[dun->cent_n+1].type = ROOM_PIT_THEME;

	}
	else if (tmp < 20)
	{
		/* Message */
		name = "orc";

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_orc;

		/* Appropriate rating bonus */
		rating_bonus = 30 - p_ptr->depth;

		room_info[dun->cent_n+1].type = ROOM_PIT_ORC;

	}

	/* Troll pit */
	else if (tmp < 40)
	{
		/* Message */
		name = "troll";

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_troll;

		/* Appropriate rating bonus */
		rating_bonus = 35 - p_ptr->depth;

		room_info[dun->cent_n+1].type = ROOM_PIT_TROLL;

	}

	/* Giant pit */
	else if (tmp < 60)
	{
		/* Message */
		name = "giant";

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_giant;

		/* Appropriate rating bonus */
		rating_bonus = 65 - p_ptr->depth;

		room_info[dun->cent_n+1].type = ROOM_PIT_GIANT;
	}

	/* Dragon pit */
	else if (tmp < 80)
	{
		/* Pick dragon type */
		/* Now include metallic dragons */
		switch (rand_int(11))
		{
			/* Black */
			case 0:
			{
				/* Message */
				name = "acid dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = RF4_BRTH_ACID;

				/* Done */
				break;
			}

			/* Blue */
			case 1:
			{
				/* Message */
				name = "electric dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = RF4_BRTH_ELEC;

				/* Done */
				break;
			}

			/* Red */
			case 2:
			{
				/* Message */
				name = "fire dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = RF4_BRTH_FIRE;

				/* Done */
				break;
			}

			/* White */
			case 3:
			{
				/* Message */
				name = "cold dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = RF4_BRTH_COLD;

				/* Done */
				break;
			}

			/* Green */
			case 4:
			{
				/* Message */
				name = "poison dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = RF4_BRTH_POIS;

				/* Done */
				break;
			}

			/* Multi-hued */
			case 5:
			{
				/* Message */
				name = "multi-hued dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = (RF4_BRTH_ACID | RF4_BRTH_ELEC |
							  RF4_BRTH_FIRE | RF4_BRTH_COLD |
							  RF4_BRTH_POIS);

				/* Done */
				break;
			}

			/* Brass */
			case 6:
			{
				/* Message */
				name = "brass dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = (RF4_BRTH_ACID | RF4_BRTH_DISEN);

				/* Done */
				break;
			}


			/* Copper */
			case 7:
			{
				/* Message */
				name = "copper dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = (RF4_BRTH_ELEC | RF4_BRTH_NEXUS);

				/* Done */
				break;
			}

			/* Bronze */
			case 8:
			{
				/* Message */
				name = "bronze dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = (RF4_BRTH_FIRE | RF4_BRTH_CONFU);

				/* Done */
				break;
			}

			/* Silver */
			case 9:
			{
				/* Message */
				name = "silver dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = (RF4_BRTH_COLD | RF4_BRTH_NEXUS);

				/* Done */
				break;
			}

			/* Gold */
			case 10:
			{
				/* Message */
				name = "gold dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = (RF4_BRTH_COLD | RF4_BRTH_SOUND);

				/* Done */
				break;
			}


		}

		/* Appropriate rating bonus */
		rating_bonus = 75 - p_ptr->depth;

		room_info[dun->cent_n+1].type = ROOM_PIT_DRAGON;

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_dragon;
	}

	/* Demon pit */
	else
	{
		/* Message */
		name = "demon";

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_demon;
	}

	/* Prepare allocation table */
	get_mon_num_prep();


	/* Pick some monster types */
	for (i = 0; i < 16; i++)
	{
		/* Get a (hard) monster type */
		what[i] = get_mon_num(p_ptr->depth + 10);

		/* Notice failure */
		if (!what[i]) empty = TRUE;
	}


	/* Remove restriction */
	get_mon_num_hook = NULL;

	/* Prepare allocation table */
	get_mon_num_prep();


	/* Oops */
	if (empty) return;


	/* Sort the entries XXX XXX XXX */
	for (i = 0; i < 16 - 1; i++)
	{
		/* Sort the entries */
		for (j = 0; j < 16 - 1; j++)
		{
			int i1 = j;
			int i2 = j + 1;

			int p1 = r_info[what[i1]].level;
			int p2 = r_info[what[i2]].level;

			/* Bubble */
			if (p1 > p2)
			{
				int tmp = what[i1];
				what[i1] = what[i2];
				what[i2] = tmp;
			}
		}
	}

	/* Select the entries */
	for (i = 0; i < 8; i++)
	{
		/* Every other entry */
		what[i] = what[i * 2];
	}


	/* Message */
	if (cheat_room)
	{
		/* Room type */
		msg_format("Monster pit (%s)", name);
	}

	/* Increase the level rating */
	if (rating_bonus>0) rating += rating_bonus;

	/* (Sometimes) Cause a "special feeling" (for "Monster Pits") */
	if ((p_ptr->depth <= 40) &&
	    (randint(p_ptr->depth * p_ptr->depth + 1) < 300))
	{
		good_item_flag = TRUE;
	}


	/* Top and bottom rows */
	for (x = x0 - 9; x <= x0 + 9; x++)
	{
		place_monster_aux(y0 - 2, x, what[0], FALSE, FALSE);
		place_monster_aux(y0 + 2, x, what[0], FALSE, FALSE);
	}

	/* Middle columns */
	for (y = y0 - 1; y <= y0 + 1; y++)
	{
		place_monster_aux(y, x0 - 9, what[0], FALSE, FALSE);
		place_monster_aux(y, x0 + 9, what[0], FALSE, FALSE);

		place_monster_aux(y, x0 - 8, what[1], FALSE, FALSE);
		place_monster_aux(y, x0 + 8, what[1], FALSE, FALSE);

		place_monster_aux(y, x0 - 7, what[1], FALSE, FALSE);
		place_monster_aux(y, x0 + 7, what[1], FALSE, FALSE);

		place_monster_aux(y, x0 - 6, what[2], FALSE, FALSE);
		place_monster_aux(y, x0 + 6, what[2], FALSE, FALSE);

		place_monster_aux(y, x0 - 5, what[2], FALSE, FALSE);
		place_monster_aux(y, x0 + 5, what[2], FALSE, FALSE);

		place_monster_aux(y, x0 - 4, what[3], FALSE, FALSE);
		place_monster_aux(y, x0 + 4, what[3], FALSE, FALSE);

		place_monster_aux(y, x0 - 3, what[3], FALSE, FALSE);
		place_monster_aux(y, x0 + 3, what[3], FALSE, FALSE);

		place_monster_aux(y, x0 - 2, what[4], FALSE, FALSE);
		place_monster_aux(y, x0 + 2, what[4], FALSE, FALSE);
	}

	/* Above/Below the center monster */
	for (x = x0 - 1; x <= x0 + 1; x++)
	{
		place_monster_aux(y0 + 1, x, what[5], FALSE, FALSE);
		place_monster_aux(y0 - 1, x, what[5], FALSE, FALSE);
	}

	/* Next to the center monster */
	place_monster_aux(y0, x0 + 1, what[6], FALSE, FALSE);
	place_monster_aux(y0, x0 - 1, what[6], FALSE, FALSE);

	/* Center monster */
	place_monster_aux(y0, x0, what[7], FALSE, FALSE);
}


#endif
/*
 * Hack -- fill in "vault" rooms
 */
static void build_vault(int y0, int x0, int ymax, int xmax, cptr data)
{
	int dx, dy, x, y;

	cptr t;


	/* Place dungeon features and objects */
	for (t = data, dy = 0; dy < ymax; dy++)
	{
		for (dx = 0; dx < xmax; dx++, t++)
		{
			/* Extract the location */
			x = x0 - (xmax / 2) + dx;
			y = y0 - (ymax / 2) + dy;

			/* Hack -- skip "non-grids" */
			if (*t == ' ') continue;

			/* Lay down a floor */
			cave_set_feat(y, x, FEAT_FLOOR);

			/* Part of a vault */
			cave_info[y][x] |= (CAVE_ROOM);

			/* Analyze the grid */
			switch (*t)
			{
				/* Granite wall (outer) */
				case '%':
				{
					cave_set_feat(y, x, FEAT_WALL_OUTER);
					break;
				}

				/* Granite wall (inner) */
				case '#':
				{
					cave_set_feat(y, x, FEAT_WALL_INNER);
					break;
				}

				/* Permanent wall (inner) */
				case 'X':
				{
					cave_set_feat(y, x, FEAT_PERM_INNER);
					break;
				}

				/* Treasure/trap */
				case '*':
				{
					if (rand_int(100) < 75)
					{
						place_object(y, x, FALSE, FALSE);
					}
					else
					{
						place_trap(y, x);
					}
					break;
				}

				/* Now trapped/locked doors */
				case '+':
				{
					place_locked_door(y, x);
					break;
				}

				/* Trap */
				case '^':
				{
					place_trap(y, x);
					break;
				}
			}
		}
	}


	/* Place dungeon monsters and objects */
	for (t = data, dy = 0; dy < ymax; dy++)
	{
		for (dx = 0; dx < xmax; dx++, t++)
		{
			/* Extract the grid */
			x = x0 - (xmax/2) + dx;
			y = y0 - (ymax/2) + dy;

			/* Hack -- skip "non-grids" */
			if (*t == ' ') continue;

			/* Analyze the symbol */
			switch (*t)
			{
				/* Monster */
				case '&':
				{
					monster_level = p_ptr->depth + 5;
					place_monster(y, x, TRUE, TRUE);
					monster_level = p_ptr->depth;
					break;
				}

				/* Meaner monster */
				case '@':
				{
					monster_level = p_ptr->depth + 11;
					place_monster(y, x, TRUE, TRUE);
					monster_level = p_ptr->depth;
					break;
				}

				/* Meaner monster, plus treasure */
				case '9':
				{
					monster_level = p_ptr->depth + 9;
					place_monster(y, x, TRUE, TRUE);
					monster_level = p_ptr->depth;
					object_level = p_ptr->depth + 7;
					place_object(y, x, TRUE, FALSE);
					object_level = p_ptr->depth;
					break;
				}

				/* Nasty monster and treasure */
				case '8':
				{
					monster_level = p_ptr->depth + 40;
					place_monster(y, x, TRUE, TRUE);
					monster_level = p_ptr->depth;
					object_level = p_ptr->depth + 20;
					place_object(y, x, TRUE, TRUE);
					object_level = p_ptr->depth;
					break;
				}

				/* Monster and/or object */
				case ',':
				{
					if (rand_int(100) < 50)
					{
						monster_level = p_ptr->depth + 3;
						place_monster(y, x, TRUE, TRUE);
						monster_level = p_ptr->depth;
					}
					if (rand_int(100) < 50)
					{
						object_level = p_ptr->depth + 7;
						place_object(y, x, FALSE, FALSE);
						object_level = p_ptr->depth;
					}
					break;
				}
			}
		}
	}
}



/*
 * Type 7 -- simple vaults (see "v_info.txt")
 */
static void build_type7(int y0, int x0)
{
	vault_type *v_ptr;

	/* Pick a lesser vault */
	while (TRUE)
	{
		/* Get a random vault record */
		v_ptr = &v_info[rand_int(z_info->v_max)];

		/* Accept the first lesser vault */
		if (v_ptr->typ == 7) break;
	}

	/* Message */
	if (cheat_room) msg_print("Lesser Vault");

	/* Initialize room description */
	room_info[dun->cent_n+1].type = ROOM_LESSER_VAULT;
	room_info[dun->cent_n+1].flags |= (ROOM_ICKY);

	/* Boost the rating */
	rating += v_ptr->rat;

	/* (Sometimes) Cause a special feeling */
	if ((p_ptr->depth <= 50) ||
	    (randint((p_ptr->depth-40) * (p_ptr->depth-40) + 1) < 400))
	{
		good_item_flag = TRUE;
	}

	/* Hack -- Build the vault */
	build_vault(y0, x0, v_ptr->hgt, v_ptr->wid, v_text + v_ptr->text);
}

/*
 * Type 8 -- greater vaults (see "v_info.txt")
 */
static void build_type8(int y0, int x0)
{
	vault_type *v_ptr;

	/* Pick a lesser vault */
	while (TRUE)
	{
		/* Get a random vault record */
		v_ptr = &v_info[rand_int(z_info->v_max)];

		/* Accept the first greater vault */
		if (v_ptr->typ == 8) break;
	}

	/* Message */
	if (cheat_room) msg_print("Greater Vault");

	/* Initialize room description */
	room_info[dun->cent_n+1].type = ROOM_GREATER_VAULT;
	room_info[dun->cent_n+1].flags |= ROOM_ICKY;

	/* Boost the rating */
	rating += v_ptr->rat;

	/* (Sometimes) Cause a special feeling */
	if ((p_ptr->depth <= 50) ||
	    (randint((p_ptr->depth-40) * (p_ptr->depth-40) + 1) < 400))
	{
		good_item_flag = TRUE;
	}

	/* Hack -- Build the vault */
	build_vault(y0, x0, v_ptr->hgt, v_ptr->wid, v_text + v_ptr->text);
}

/*
 * Pick appropriate feature for lake.
 */
bool cave_feat_lake(int f_idx)
{
	feature_type *f_ptr = &f_info[f_idx];

	/* Require lake or river */
	if (!(f_ptr->flags2 & (FF2_RIVER)))
	{
		if (!(f_ptr->flags2 & (FF2_LAKE)))
		{
			return (FALSE);
		}
	}

	/* Exclude terrain of various types */
	if (level_flag & (LF1_WATER | LF1_LAVA | LF1_ICE | LF1_ACID | LF1_OIL | LF1_LIVING))
	{
		if (!(level_flag & (LF1_LIVING)) && (f_ptr->flags3 & (FF3_LIVING)))
		{
			return (FALSE);
		}

		if (!(level_flag & (LF1_WATER)) && (f_ptr->flags2 & (FF2_WATER)))
		{
			return (FALSE);
		}

		if (!(level_flag & (LF1_LAVA)) && (f_ptr->flags2 & (FF2_LAVA)))
		{
			return (FALSE);
		}

		if (!(level_flag & (LF1_ICE)) && (f_ptr->flags2 & (FF2_ICE)))
		{
			return (FALSE);
		}

		if (!(level_flag & (LF1_ACID)) && (f_ptr->flags2 & (FF2_ACID)))
		{
			return (FALSE);
		}

		if (!(level_flag & (LF1_OIL)) && (f_ptr->flags2 & (FF2_OIL)))
		{
			return (FALSE);
		}
	}

	/* Okay */
	return (TRUE);
}


/*
 * Hack -- fill in "tower" rooms
 *
 * Similar to vaults, but we never place monsters/traps/treasure.
 */
static void build_tower(int y0, int x0, int ymax, int xmax, cptr data)
{
	int dx, dy, x, y;

	cptr t;

	/* Place dungeon features and objects */
	for (t = data, dy = 0; dy < ymax; dy++)
	{
		for (dx = 0; dx < xmax; dx++, t++)
		{
			/* Extract the location */
			x = x0 - (xmax / 2) + dx;
			y = y0 - (ymax / 2) + dy;

			/* Hack -- skip "non-grids" */
			if (*t == ' ') continue;

			/* Lay down a floor */
			cave_set_feat(y, x, FEAT_FLOOR);

			/* Part of a vault */
			cave_info[y][x] |= (CAVE_ROOM);

			/* Hack -- always lite towers */
			cave_info[y][x] |= (CAVE_GLOW);

			/* Analyze the grid */
			switch (*t)
			{
				/* Granite wall (outer) */
				case '%':
				{
					cave_set_feat(y, x, FEAT_WALL_OUTER);
					break;
				}

				/* Granite wall (inner) */
				case '#':
				{
					cave_set_feat(y, x, FEAT_WALL_INNER);
					break;
				}

				/* Permanent wall (inner) */
				case 'X':
				{
					cave_set_feat(y, x, FEAT_PERM_INNER);
					break;
				}

				/* Treasure/trap */
				case '*':
				{
					break;
				}

				/* Now locked doors */
				case '+':
				{
					place_locked_door(y, x);
					break;
				}

				/* Trap */
				case '^':
				{
					break;
				}
			}
		}
	}

}

/*
 * Hack -- fill in "roof"
 *
 * This allows us to stack multiple towers on top of each other, so that
 * a player ascending from one tower to another will not fall.
 */
static void build_roof(int y0, int x0, int ymax, int xmax, cptr data)
{
	int dx, dy, x, y;

	cptr t;

	/* Place dungeon features and objects */
	for (t = data, dy = 0; dy < ymax; dy++)
	{
		for (dx = 0; dx < xmax; dx++, t++)
		{
			/* Extract the location */
			x = x0 - (xmax / 2) + dx;
			y = y0 - (ymax / 2) + dy;

			/* Hack -- skip "non-grids" */
			if (*t == ' ') continue;

			/* Lay down a floor */
			cave_set_feat(y, x, FEAT_FLOOR);
		}
	}
}

/*
 * Leave 1 & 2 empty as these are used to vary the crenallations in crypt
 * corridors
 */

#define TUNNEL_STYLE	4	/* First 'real' style */
#define TUNNEL_CRYPT_L	4
#define TUNNEL_CRYPT_R	8
#define TUNNEL_LARGE_L	16
#define TUNNEL_LARGE_R	32
#define TUNNEL_CAVE	64

static int get_tunnel_style(void)
{
	int style = 0;

	/* Change tunnel type */
	if (level_flag & (LF1_VAULT))
	{
		style |= (TUNNEL_LARGE_L | TUNNEL_LARGE_R);
	}
	else if (level_flag & (LF1_STRONGHOLD))
	{
		if (p_ptr->depth % 2) style |= (TUNNEL_LARGE_L);
		else style |= (TUNNEL_LARGE_R);
	}
	else if (level_flag & (LF1_CRYPT))
	{
		if (rand_int(100) < 50) style |= (TUNNEL_CRYPT_L);
		if (rand_int(100) < 50) style |= (TUNNEL_CRYPT_R);
	}
	else if (level_flag & (LF1_CAVE)) style |= (TUNNEL_CAVE);

	style |= rand_int(TUNNEL_STYLE);

	return (style);
}


/*
 * Constructs a tunnel between two points
 *
 * This function must be called BEFORE any streamers are created,
 * since we use the special "granite wall" sub-types to keep track
 * of legal places for corridors to pierce rooms.
 *
 * We use "door_flag" to prevent excessive construction of doors
 * along overlapping corridors.
 *
 * We queue the tunnel grids to prevent door creation along a corridor
 * which intersects itself.
 *
 * We queue the wall piercing grids to prevent a corridor from leaving
 * a room and then coming back in through the same entrance.
 *
 * We "pierce" grids which are "outer" walls of rooms, and when we
 * do so, we change all adjacent "outer" walls of rooms into "solid"
 * walls so that no two corridors may use adjacent grids for exits.
 *
 * The "solid" wall check prevents corridors from "chopping" the
 * corners of rooms off, as well as "silly" door placement, and
 * "excessively wide" room entrances.
 *
 * Useful "feat" values:
 *   FEAT_WALL_EXTRA -- granite walls
 *   FEAT_WALL_INNER -- inner room walls
 *   FEAT_WALL_OUTER -- outer room walls
 *   FEAT_WALL_SOLID -- solid room walls
 *   FEAT_PERM_EXTRA -- shop walls (perma)
 *   FEAT_PERM_INNER -- inner room walls (perma)
 *   FEAT_PERM_OUTER -- outer room walls (perma)
 *   FEAT_PERM_SOLID -- dungeon border (perma)
 *
 *
 * We now style the tunnels. The following tunnel styles are supported:
 *
 * -- standard Angband tunnel
 * -- tunnel with pillared edges (on LF1_CRYPT levels)
 * -- width 2 or width 3 tunnel (on LF1_STRONGHOLD levels)
 * -- tunnels with lateral and diagonal interruptions (on LF1_CAVE levels)
 *
 * We also can fill tunnels now. Filled tunnels occur iff the last floor
 * space leaving the start room is not a FEAT_FLOOR or the first floor
 * space entering the finishing room is not a FEAT_FLOOR.
 *
 * We also put 'decorations' next to tunnel entrances. These are various
 * solid wall types relating to the room the tunnel goes from or to.
 * However, we use the decorations of the room at the other end of the tunnel
 * unless that room has no decorations, in which case we use our own.
 */

static void build_tunnel(int row1, int col1, int row2, int col2)
{
	int i, y, x;
	int tmp_row, tmp_col;
	int row_dir, col_dir;
	int start_row, start_col;
	int main_loop_count = 0;
	int last_turn = 0, first_door, last_door, first_tunn, first_next;
	int start_tunnel = 0;

	bool door_flag = FALSE;
	bool overrun_flag = FALSE;

	/* Force style change */
	int style = get_tunnel_style();

	int by1 = row1/BLOCK_HGT;
	int bx1 = col1/BLOCK_WID;

	/* Reset the arrays */
	dun->tunn_n = 0;
	dun->wall_n = 0;

	/* Save the starting location */
	start_row = row1;
	start_col = col1;

	/* Record number of doorways */
	first_door = dun->door_n;
	last_door = dun->door_n;

	/* Record first tunnel location */
	first_tunn = dun->tunn_n;

	/* Record number of adjacent solid terrain to doorways */
	first_next = dun->next_n;

	/* Start out in the correct direction */
	correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

	/* Keep going until done (or bored) */
	while ((row1 != row2) || (col1 != col2)) 
	{
		/* Mega-Hack -- Paranoia -- prevent infinite loops */
		if (main_loop_count++ > 2000) return;

		/* Hack -- Prevent tunnel weirdness */
		if (dun->tunn_n >= TUNN_MAX) return;

		/* Allow changes in the tunnel style */
		if (rand_int(100) < DUN_TUN_STY)
		{
			style = get_tunnel_style();
		}

		/* Allow bends in the tunnel */
		if (rand_int(100) < ((style & TUNNEL_CAVE ? DUN_TUN_CAV : DUN_TUN_CHG) /
				(style & (TUNNEL_LARGE_L | TUNNEL_LARGE_R) ? 2 : 1)))
		{
			/* Get the correct direction */
			correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

			/* Random direction */
			if (rand_int(100) < (style & TUNNEL_CAVE ? DUN_TUN_CAV : DUN_TUN_RND))
			{
				if ((style & TUNNEL_CAVE) != 0)
					rand_dir_cave(&row_dir, &col_dir);
				else
					rand_dir(&row_dir, &col_dir);
			}

			/* Record this */
			last_turn = dun->tunn_n;
			last_door = dun->door_n;
		}

		/* Get the next location */
		tmp_row = row1 + row_dir;
		tmp_col = col1 + col_dir;

		/* Do not leave the dungeon!!! XXX XXX */
		while (!in_bounds_fully_tunnel(tmp_row, tmp_col))
		{
			/* Fall back to last turn coords */
			if (!last_turn)
			{
				row1 = start_row;
				col1 = start_col;
			}
			else
			{
				row1 = dun->tunn[last_turn - 1].y;
				col1 = dun->tunn[last_turn - 1].x;
			}

			/* Fall back to last turn */
			dun->tunn_n = last_turn;
			dun->door_n = last_door;

			/* Back up some more */
			last_turn /= 2;
			last_door = first_door;

			/* Get the correct direction */
			correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

			/* Random direction */
			if (rand_int(100) < (style & TUNNEL_CAVE ? DUN_TUN_CAV : DUN_TUN_RND))
			{
				if ((style & TUNNEL_CAVE) != 0)
					rand_dir_cave(&row_dir, &col_dir);
				else
					rand_dir(&row_dir, &col_dir);
			}

			/* Get the next location */
			tmp_row = row1 + row_dir;
			tmp_col = col1 + col_dir;
		}

		/* Avoid "solid" granite walls */
		if (f_info[cave_feat[tmp_row][tmp_col]].flags1 & (FF1_SOLID)) continue;

		/* Pierce "outer" walls of rooms */
		if (f_info[cave_feat[tmp_row][tmp_col]].flags1 & (FF1_OUTER))
		{
			int wall1 = dun->wall_n;
			bool door = TRUE;
			bool pillar = FALSE;

			/* Get the "next" location */
			y = tmp_row + row_dir;
			x = tmp_col + col_dir;

			/* Hack -- Avoid outer/solid walls */
			if (f_info[cave_feat[y][x]].flags1 & (FF1_OUTER)) continue;
			if (f_info[cave_feat[y][x]].flags1 & (FF1_SOLID)) continue;

			/* Accept this location */
			row1 = tmp_row;
			col1 = tmp_col;

			/* Save the wall location */
			if (dun->wall_n < WALL_MAX)
			{
				dun->wall[dun->wall_n].y = row1;
				dun->wall[dun->wall_n].x = col1;
				dun->wall_n++;
			}

			/* XXX Note that no bounds checking is required because of in_bounds_fully_tunnel above */
			if (style & (TUNNEL_LARGE_L))
			{
				if (f_info[cave_feat[row1 + col_dir][col1 - row_dir]].flags1 & (FF1_OUTER))
				{
					/* Save the wall location */
					if (dun->wall_n < WALL_MAX)
					{
						dun->wall[dun->wall_n].y = row1 + col_dir;
						dun->wall[dun->wall_n].x = col1 - row_dir;
						dun->wall_n++;

						/* Hack -- add regular pillars to some width 3 corridors */
						if ((((row1 + col1) % ((style % 4) + 2)) == 0)
							&& ((style & (TUNNEL_CRYPT_L | TUNNEL_CRYPT_R))== 0)) pillar = TRUE;
					}
				}
				else
				{
					door = FALSE;
				}
			}

			/* XXX Note that no bounds checking is required because of in_bounds_fully_tunnel above */
			if (style & (TUNNEL_LARGE_R))
			{
				if (f_info[cave_feat[row1 - col_dir][col1 + row_dir]].flags1 & (FF1_OUTER))
				{
					/* Save the wall location */
					if (dun->wall_n < WALL_MAX)
					{
						if (pillar) dun->wall_n -= 2;

						dun->wall[dun->wall_n].y = row1 - col_dir;
						dun->wall[dun->wall_n].x = col1 + row_dir;
						dun->wall_n++;

						if (pillar) dun->wall_n++;
					}
				}
				else
				{
					door = FALSE;
				}
			}

			/* Cancel if can't make all the doors */
			if (!door)
			{
				dun->wall_n = wall1;
				continue;
			}

			/* Forbid re-entry near these piercings */
			for (i = wall1; i < dun->wall_n; i++)
			{
				for (y = dun->wall[i].y - 1; y <= dun->wall[i].y + 1; y++)
				{
					for (x = dun->wall[i].x - 1; x <= dun->wall[i].x + 1; x++)
					{
						/* Convert adjacent "outer" walls as "solid" walls */
						if (f_info[cave_feat[y][x]].flags1 & (FF1_OUTER))
						{
							/* Room */
							int by2 = tmp_row/BLOCK_HGT;
							int bx2 = tmp_col/BLOCK_WID;

							/* Change the wall to a "solid" wall */
							cave_alter_feat(y, x, FS_SOLID);

							/* Starting room overwrites ending room decorations unless has none */
							if ((room_info[dun_room[by1][bx1]].solid) && (dun->next_n < WALL_MAX))
							{
								/* Overwrite with alternate terrain from starting room later */
								dun->next[dun->next_n].y = y;
								dun->next[dun->next_n].x = x;
								dun->next_feat[dun->next_n] = room_info[dun_room[by1][bx1]].solid;
								dun->next_n++;
							}

							/* Ending room overwrites starting room decorations unless has none */
							if (room_info[dun_room[by2][bx2]].solid)
							{
								int j;

								for (j = first_next; (j < dun->next_n) && (dun_room[by1][by2] == dun_room[dun->next[j].y][dun->next[j].x]); j++)
								{
									/* Overwrite with alternate terrain from ending room later */
									dun->next_feat[j] = room_info[dun_room[by2][bx2]].solid;
								}
							}
						}
					}
				}
			}
		}

		/* Travel quickly through rooms */
		else if (cave_info[tmp_row][tmp_col] & (CAVE_ROOM))
		{
			/* Room */
			int by2 = tmp_row/BLOCK_HGT;
			int bx2 = tmp_col/BLOCK_WID;

			/* Different room */
			if (dun_room[by1][bx1] != dun_room[by2][bx2])
			{
				/* Different room in same partition */
				if (dun->part[dun_room[by1][bx1]-1] == dun->part[dun_room[by2][bx2]-1])
				{
					/* Clear intersections */
					dun->door_n = first_door;

					/* Abort */
					return;
				}
				else
				{
					int part1 = dun->part[dun_room[by1][bx1]-1];
					int part2 = dun->part[dun_room[by2][bx2]-1];

					/* Merge partitions */
					for (i = 0; i < dun->cent_n; i++)
					{
						if (dun->part[i] == part2) dun->part[i] = part1;
					}

					/* Merge partitions */
					dun->part_n--;

					/* Rewrite tunnel to room if we end up on a non-floor space */
					if ((cave_feat[tmp_row][tmp_col] != FEAT_FLOOR) && (room_info[dun_room[by2][bx2]].tunnel))
					{
						/* Hack -- overwrite half of tunnel */
						if (start_tunnel)
						{
							/* Round up some times */
							if (((dun->tunn_n - first_tunn) % 2) && (rand_int(100) < 50)) first_tunn++;

							/* Adjust from half-way */
							first_tunn = first_tunn + (dun->tunn_n - first_tunn) / 2;
						}

						/* Overwrite starting tunnel terrain with end tunnel terrain */
						for (i = first_tunn; i < dun->tunn_n; i++)
						{
							if (dun->tunn_feat[i]) dun->tunn_feat[i] = room_info[dun_room[by2][bx2]].tunnel;
						}
					}

					/* Accept tunnel */
					break;
				}
			}
			else
			{
				/* Accept the location */
				row1 = tmp_row;
				col1 = tmp_col;

				/* Clear start tunnel if feature is floor */
				if (cave_feat[row1][col1] == FEAT_FLOOR)
				{
					start_tunnel = 0;
				}
				/* Set start tunnel if feature is not floor */
				else
				{
					start_tunnel = room_info[dun_room[by1][bx1]].tunnel;
				}
			}
		}

		/* Bridge features */
		else if (f_info[cave_feat[tmp_row][tmp_col]].flags2 & (FF2_BRIDGE))
		{
			/* Accept this location */
			row1 = tmp_row;
			col1 = tmp_col;

			/* Save the tunnel location */
			if (dun->tunn_n < TUNN_MAX)
			{
				dun->tunn[dun->tunn_n].y = row1;
				dun->tunn[dun->tunn_n].x = col1;
				dun->tunn_n++;
			}

			/* XXX Note that no bounds checking is required because of in_bounds_fully_tunnel above */
			if ((style & (TUNNEL_LARGE_L)) && (!(style & (TUNNEL_LARGE_R)) || (style % 2)))
			{
				if (f_info[cave_feat[row1+col_dir][col1-row_dir]].flags2 & (FF2_BRIDGE))
				{
					/* Save the tunnel location */
					if (dun->tunn_n < TUNN_MAX)
					{
						dun->tunn[dun->tunn_n].y = row1 + col_dir;
						dun->tunn[dun->tunn_n].x = col1 - row_dir;
						dun->tunn_n++;
					}
				}
			}

			/* Hack -- note use of else to ensure bridges are never more than 2 units wide */
			else if (style & (TUNNEL_LARGE_L | TUNNEL_LARGE_R))
			{
				if (f_info[cave_feat[row1-col_dir][col1+row_dir]].flags2 & (FF2_BRIDGE))
				{
					/* Save the tunnel location */
					if (dun->tunn_n < TUNN_MAX)
					{
						dun->tunn[dun->tunn_n].y = row1 - col_dir;
						dun->tunn[dun->tunn_n].x = col1 + row_dir;
						dun->tunn_n++;
					}
				}
			}

			/* Prevent door in next grid */
			door_flag = TRUE;
		}

		/* Tunnel through all other walls and bridge features */
		else if (f_info[cave_feat[tmp_row][tmp_col]].flags1 & (FF1_TUNNEL))
		{
			bool pillar = FALSE;

			/* Accept this location */
			row1 = tmp_row;
			col1 = tmp_col;

			/* Save the tunnel location */
			if (dun->tunn_n < TUNN_MAX)
			{
				dun->tunn[dun->tunn_n].y = row1;
				dun->tunn[dun->tunn_n].x = col1;

				/* Mark centre of tunnel with terrain type */
				if (start_tunnel)
				{
					dun->tunn_feat[dun->tunn_n] = start_tunnel;
				}
				/* Hack -- mark centre in case we need to overwrite later */
				{
					dun->tunn_feat[dun->tunn_n] = 1;
				}

				dun->tunn_n++;
			}

			/* XXX Note that no bounds checking is required because of in_bounds_fully_tunnel above */
			if (style & (TUNNEL_CRYPT_L | TUNNEL_LARGE_L))
			{
				if ((f_info[cave_feat[row1+col_dir][col1-row_dir]].flags1 & (FF1_TUNNEL))
					&& ((style & (TUNNEL_LARGE_L)) || !((row1 + col1 + style) % 2)))
				{
					/* Save the tunnel location */
					if (dun->tunn_n < TUNN_MAX)
					{
						dun->tunn[dun->tunn_n].y = row1 + col_dir;
						dun->tunn[dun->tunn_n].x = col1 - row_dir;
						dun->tunn_n++;

						/* Hack -- add regular pillars to some width 3 corridors */
						if ((((row1 + col1) % ((style % 4) + 2)) == 0)
							&& ((style & (TUNNEL_CRYPT_L | TUNNEL_CRYPT_R))== 0)) pillar = TRUE;
					}
				}
			}

			/* XXX Note that no bounds checking is required because of in_bounds_fully_tunnel above */
			if (style & (TUNNEL_CRYPT_R | TUNNEL_LARGE_R))
			{
				if ((f_info[cave_feat[row1-col_dir][col1+row_dir]].flags1 & (FF1_TUNNEL))
					&& ((style & (TUNNEL_LARGE_R)) || !((row1 + col1 + style / 2) % 2)))
				{
					/* Save the tunnel location */
					if (dun->tunn_n < TUNN_MAX)
					{
						if (pillar) dun->tunn_n -= 2;

						dun->tunn[dun->tunn_n].y = row1 - col_dir;
						dun->tunn[dun->tunn_n].x = col1 + row_dir;
						dun->tunn_n++;

						if (pillar) dun->tunn_n++;
					}
				}
			}

			/* Allow door in next grid */
			door_flag = FALSE;
		}

		/* Handle corridor intersections or overlaps */
		else
		{
			/* Accept the location */
			row1 = tmp_row;
			col1 = tmp_col;

			/* Prevent us following corridor length */
			if (door_flag)
			{
				if ((overrun_flag) || (dun->door_n > first_door + 6))
				{
					/* Clear intersections */
					dun->door_n = first_door;

					/* Abort */
					return;
				}

				overrun_flag = TRUE;
			}

			/* Collect legal door locations */
			if (!door_flag)
			{
				/* Save the door location */
				if (dun->door_n < DOOR_MAX)
				{
					dun->door[dun->door_n].y = row1;
					dun->door[dun->door_n].x = col1;
					dun->door_n++;
				}

				/* No door in next grid */
				door_flag = TRUE;

				overrun_flag = FALSE;
			}

			/* Hack -- allow pre-emptive tunnel termination */
			if ((rand_int(100) >= DUN_TUN_CON) && (dun->door_n < 3))
			{
				/* Distance between row1 and start_row */
				tmp_row = row1 - start_row;
				if (tmp_row < 0) tmp_row = (-tmp_row);

				/* Distance between col1 and start_col */
				tmp_col = col1 - start_col;
				if (tmp_col < 0) tmp_col = (-tmp_col);

				/* Terminate the tunnel */
				if ((tmp_row > 10) || (tmp_col > 10)) break;
			}
		}

		/* Fix up diagonals from cave tunnels after 1 move */
		/* Never move diagonally */
		if (row_dir && col_dir)
		{
			if (rand_int(100) < 50)
			{
				row_dir = 0;
			}
			else
			{
				col_dir = 0;
			}
		}

		/* End found */
		if ((row1 == row2) && (col1 == col2)) 
		{
			/* Room */
			int by2 = row1/BLOCK_HGT;
			int bx2 = col1/BLOCK_WID;

			/* Different room in same partition */
			if (dun->part[dun_room[by1][bx1]-1] == dun->part[dun_room[by2][bx2]-1])
			{
				/* Clear intersections */
				dun->door_n = first_door;

				/* Abort */
				return;
			}
			else
			{
				int part1 = dun->part[dun_room[by1][bx1]-1];
				int part2 = dun->part[dun_room[by2][bx2]-1];

				/* Merge partitions */
				for (i = 0; i < dun->cent_n; i++)
				{
					if (dun->part[i] == part2) dun->part[i] = part1;
				}

				/* Merge partitions */
				dun->part_n--;

				/* Accept tunnel */
				break;
			}
		}
	}

	/* Turn the tunnel into corridor */
	for (i = 0; i < dun->tunn_n; i++)
	{
		/* Get the grid */
		y = dun->tunn[i].y;
		x = dun->tunn[i].x;

		/* Apply feature - note hack */
		if (dun->tunn_feat[i] > 1)
		{
			/* Clear previous contents, write terrain */
			cave_set_feat(y, x, dun->tunn_feat[i]);
		}
		/* Apply bridge */
		else if (f_info[cave_feat[y][x]].flags2 & (FF2_BRIDGE))
		{
			/* Bridge previous contents */
			cave_alter_feat(y, x, FS_BRIDGE);
		}
		/* Apply tunnel */
		else
		{
			/* Tunnel previous contents */
			cave_alter_feat(y, x, FS_TUNNEL);
		}
	}

	/* Apply the piercings that we found */
	for (i = 0; i < dun->wall_n; i++)
	{
		/* Get the grid */
		y = dun->wall[i].y;
		x = dun->wall[i].x;

		/* Convert to floor grid */
		cave_set_feat(y, x, FEAT_FLOOR);

		/* Occasional doorway */
		if (rand_int(100) < DUN_TUN_PEN)
		{

			/* Place a random door */
			place_random_door(y, x);
		}

		/* Place identical doors if next set of doors is adjacent */
		while ((i < dun->wall_n - 1) && (dun->wall[i+1].y - y < 1) && (dun->wall[i+1].x - x < 1))
		{
			cave_set_feat(dun->wall[i+1].y, dun->wall[i+1].x, cave_feat[y][x]);
			i++;
		}
	}
}




/*
 * Count the number of "corridor" grids adjacent to the given grid.
 *
 * Note -- Assumes "in_bounds_fully(y1, x1)"
 *
 * This routine currently only counts actual "empty floor" grids
 * which are not in rooms.  We might want to also count stairs,
 * open doors, closed doors, etc.  XXX XXX
 */
static int next_to_corr(int y1, int x1)
{
	int i, y, x, k = 0;


	/* Scan adjacent grids */
	for (i = 0; i < 4; i++)
	{
		/* Extract the location */
		y = y1 + ddy_ddd[i];
		x = x1 + ddx_ddd[i];

		/* Skip non floors */
		if (!cave_floor_bold(y, x)) continue;

		/* Skip non "empty floor" grids */
		if (cave_feat[y][x] != FEAT_FLOOR) continue;

		/* Skip grids inside rooms */
		if (cave_info[y][x] & (CAVE_ROOM)) continue;

		/* Count these grids */
		k++;
	}

	/* Return the number of corridors */
	return (k);
}


/*
 * Determine if the given location is "between" two walls,
 * and "next to" two corridor spaces.  XXX XXX XXX
 *
 * Assumes "in_bounds_fully(y,x)"
 */
static bool possible_doorway(int y, int x)
{
	/* Count the adjacent corridors */
	if (next_to_corr(y, x) >= 2)
	{
		/* Check Vertical */
		if ((cave_feat[y-1][x] >= FEAT_MAGMA) &&
		    (cave_feat[y+1][x] >= FEAT_MAGMA))
		{
			return (TRUE);
		}

		/* Check Horizontal */
		if ((cave_feat[y][x-1] >= FEAT_MAGMA) &&
		    (cave_feat[y][x+1] >= FEAT_MAGMA))
		{
			return (TRUE);
		}
	}

	/* No doorway */
	return (FALSE);
}


/*
 * Places door at y, x position if at least 2 walls found
 */
static bool try_door(int y, int x)
{
	/* Paranoia */
	if (!in_bounds(y, x)) return (FALSE);

	/* Ignore walls */
	if (!(f_info[cave_feat[y][x]].flags1 & (FF1_WALL))) return (FALSE);

	/* Ignore room grids */
	if (cave_info[y][x] & (CAVE_ROOM)) return (FALSE);

	/* Occasional door (if allowed) */
	if ((rand_int(100) < DUN_TUN_JCT) && possible_doorway(y, x))
	{
		/* Place a door */
		place_random_door(y, x);

		return (TRUE);
	}

	return (FALSE);
}

/*
 * Attempt to build a room of the given type at the given block
 *
 * Note that we restrict the number of "crowded" rooms to reduce
 * the chance of overflowing the monster list during level creation.
 */
static bool room_build(int by0, int bx0, int typ)
{
	int y, x;
	int by, bx;
	int by1, bx1, by2, bx2;

	/* Restrict level */
	if (p_ptr->depth < room[typ].level) return (FALSE);

	/* Restrict "crowded" rooms */
	if ((level_flag & (LF1_CROWDED)) && ((typ == 5) || (typ == 6))) return (FALSE);

	/* Extract blocks */
	by1 = by0 + room[typ].dy1;
	bx1 = bx0 + room[typ].dx1;
	by2 = by0 + room[typ].dy2;
	bx2 = bx0 + room[typ].dx2;

	/* Never run off the screen */
	if ((by1 < 0) || (by2 >= dun->row_rooms)) return (FALSE);
	if ((bx1 < 0) || (bx2 >= dun->col_rooms)) return (FALSE);

	/* Verify open space */
	for (by = by1; by <= by2; by++)
	{
		for (bx = bx1; bx <= bx2; bx++)
		{
			if (dun->room_map[by][bx]) return (FALSE);
		}
	}

	/* It is *extremely* important that the following calculation */
	/* be *exactly* correct to prevent memory errors XXX XXX XXX */

	/* Get the location of the room */
	y = ((by1 + by2 + 1) * BLOCK_HGT) / 2;
	x = ((bx1 + bx2 + 1) * BLOCK_WID) / 2;

	/* Build a room */
	switch (typ)
	{
		/* Build an appropriate room */
		case 8: build_type8(y, x); break;
		case 7: build_type7(y, x); break;
#if 0
		case 6: build_type6(y, x); break;
		case 5: build_type5(y, x); break;
		case 4: build_type4(y, x); break;
		case 3: build_type3(y, x); break;
		case 2: build_type2(y, x); break;
		case 1: build_type1(y, x); break;
#endif

		case 6: case 5: case 4: case 3: case 2: case 1: get_room_info(y, x); break;

		default: if ((!dun->entrance) && (p_ptr->depth < max_depth(p_ptr->dungeon)))
		{
			cave_set_feat(y,x,FEAT_ENTRANCE);
			dun->entrance = TRUE;
			break;
		}
	}


	/* Reserve some blocks */
	for (by = by1; by <= by2; by++)
	{
		for (bx = bx1; bx <= bx2; bx++)
		{
			dun->room_map[by][bx] = TRUE;

			dun_room[by][bx] = dun->cent_n+1;

		}
	}

	/* Save the room location */
	if (dun->cent_n < CENT_MAX)
	{
		dun->cent[dun->cent_n].y = y;
		dun->cent[dun->cent_n].x = x;
		dun->cent_n++;
	}

	/* Count "crowded" rooms */
	if ((typ == 5) || (typ == 6)) level_flag |= LF1_CROWDED;

	/* Success */
	return (TRUE);
}


/*
 *  Sets various level flags at initialisation
 */
void init_level_flags(void)
{
	dungeon_zone *zone=&t_info[0].zone[0];

	/* Get the zone */
	get_zone(&zone,p_ptr->dungeon,p_ptr->depth);

	/* Set night and day level flag */
	level_flag =  (p_ptr->depth == min_depth(p_ptr->dungeon)) ?
			((((turn % (10L * TOWN_DAWN)) < ((10L * TOWN_DAWN) / 2))) ?
				LF1_SURFACE | LF1_DAYLIGHT : LF1_SURFACE) : 0;

	/* Add 'common' level flags */
	if (zone->tower) level_flag |= (LF1_TOWER);
	if ((zone->guard) && (r_info[zone->guard].cur_num <= 0)) level_flag |= (LF1_GUARDIAN);
	if (is_quest(p_ptr->depth)) level_flag |= (LF1_QUEST);

	/* Define town */
	if (!zone->fill) level_flag |= LF1_TOWN;

	/* Define wilderness */
	if ((zone->fill) && ((f_info[zone->fill].flags1 & (FF1_WALL)) != 0)) level_flag |= LF1_WILD;
	if ((zone->big) && ((f_info[zone->big].flags1 & (FF1_WALL)) != 0)) level_flag |= LF1_WILD;
	if ((zone->small) && ((f_info[zone->small].flags1 & (FF1_WALL)) != 0)) level_flag |= LF1_WILD;

	/* No dungeon, no stairs */
	if (min_depth(p_ptr->dungeon) == max_depth(p_ptr->dungeon))
	{
		/* Do nothing */;
	}

	/* Towers */
	else if (level_flag & (LF1_TOWER))
	{
		/* Base of tower */
		if (p_ptr->depth == min_depth(p_ptr->dungeon))
		{
			/* Do nothing -- We place upstairs as a hack */;
		}

		/* Top of tower */
		else if (p_ptr->depth == max_depth(p_ptr->dungeon))
		{
			level_flag |= (LF1_MORE);
		}

		/* In tower */
		else
		{
			level_flag |= (LF1_LESS | LF1_MORE);
		}
	}
	/* Others */
	else
	{
		/* Surface -- must go down */
		if (p_ptr->depth == min_depth(p_ptr->dungeon))
		{
			level_flag |= (LF1_MORE);
		}

		/* Bottom of dungeon -- must go up */
		else if (p_ptr->depth == max_depth(p_ptr->dungeon))
		{
			level_flag |= (LF1_LESS);
		}

		/* Middle of dungeon */
		else
		{
			level_flag |= (LF1_LESS | LF1_MORE);
		}
	}

	/* At the moment, all levels have rooms and corridors */
	level_flag |= (LF1_ROOMS | LF1_TUNNELS);
}


/*
 * Generate a new dungeon level
 *
 * Note that "dun_body" adds about 4000 bytes of memory to the stack.
 */
static void cave_gen(void)
{
	int i, j, k, y, x, y1, x1;

	int by, bx;

	int base;

	char *name;

	dungeon_zone *zone=&t_info[0].zone[0];

	dun_data dun_body;

	/* Global data */
	dun = &dun_body;

	/* Get the zone */
	get_zone(&zone,p_ptr->dungeon,p_ptr->depth);

	/* Create air */
	if (((level_flag & (LF1_TOWER)) != 0) && ((level_flag & (LF1_SURFACE)) == 0))
	{
		base = FEAT_CHASM;
	}
	/* Create ground */
	else if ((level_flag & (LF1_SURFACE)) != 0)
	{
		if (f_info[zone->fill].flags1 & (FF1_FLOOR)) base = zone->fill;
		else base = FEAT_GROUND;
	}
	/* Create granite wall */
	else
	{
		base = FEAT_WALL_EXTRA;
	}

	/* Hack -- Start with base */
	for (y = 0; y < DUNGEON_HGT; y++)
	{
		for (x = 0; x < DUNGEON_WID; x++)
		{
			cave_set_feat(y,x,base);
		}
	}

	/* Hack -- Build terrain */
	if (zone->fill) for (y = 0; y < DUNGEON_HGT; y++)
	{
		for (x = 0; x < DUNGEON_WID; x++)
		{
			build_terrain(y,x,zone->fill);
		}
	}

	/* Initialise the dungeon ecology */
	cave_ecology.num_races = 0;
	cave_ecology.ready = FALSE;

	/* Place guardian if permitted */
	if ((level_flag & (LF1_GUARDIAN)) != 0)
	{
		get_monster_ecology(zone->guard);
	}

	/* Get a seed monster for the ecology */
	else
	{
		/* Set monster hook */
		get_mon_num_hook = dun_level_mon;

		/* Prepare allocation table */
		get_mon_num_prep();

		/* Get seed monster for ecology */
		get_monster_ecology(0);
	}

	/* Clear monster hook */
	get_mon_num_hook = NULL;

	/* Prepare allocation table */
	get_mon_num_prep();

	/* Get additional monsters for the ecology */
	while (cave_ecology.num_races < 4)
	{
		get_monster_ecology(0);
	}

	/* Start the ecology */
	cave_ecology.ready = TRUE;

	/* Actual maximum number of rooms on this level */
	dun->row_rooms = DUNGEON_HGT / BLOCK_HGT;
	dun->col_rooms = DUNGEON_WID / BLOCK_WID;

	/* Initialise 'zeroeth' room description */
	room_info[0].flags = 0;

	/* Initialize the room table */
	for (by = 0; by < dun->row_rooms; by++)
	{
		for (bx = 0; bx < dun->col_rooms; bx++)
		{
			dun->room_map[by][bx] = FALSE;
			dun_room[by][bx] = 0;
		}
	}

	/* No "entrance" yet */
	dun->entrance = FALSE;

	/* No rooms yet */
	dun->cent_n = 0;

	/* Hack -- chance of destroyed level */
	if ((p_ptr->depth > 10) && (rand_int(DUN_DEST) == 0)) level_flag |= LF1_DESTROYED;

	/* Hack -- No destroyed "quest", "wild" or "guardian" levels */
	if (level_flag & (LF1_QUEST | LF1_WILD | LF1_GUARDIAN)) level_flag &= ~(LF1_DESTROYED);

	/* No features on destroyed level or in a tower above the surface */
	if (((level_flag & (LF1_DESTROYED)) != 0) && (((level_flag & (LF1_TOWER)) == 0) || ((level_flag & (LF1_SURFACE)) != 0) ))
	{
		bool big = FALSE;
		bool done_big = FALSE;

		int feat;
		int count=0;

		/* Allocate some lakes and rivers*/
		while ((randint(100)<DUN_FEAT) || (zone->big) || (zone->small))
		{
			/* Increase count */
			count++;

			/* No more than 3 features */
			if (count > DUN_MAX_LAKES) break;

			get_feat_num_hook = cave_feat_lake;

			get_feat_num_prep();

			feat = get_feat_num(p_ptr->depth);

			get_feat_num_hook = NULL;

			get_feat_num_prep();

			if ((zone->big) && (count == 1)) feat = zone->big;
			else if (zone->small) feat = zone->small;

			if (feat)
			{			     
				if (!done_big)
				{
					big = randint(150) < p_ptr->depth;

					if (zone->big) big = TRUE;

					done_big = big;
				}
				else
				{
					big = FALSE;
				}

				/* Room type */
				if (cheat_room)
				{

					name = (f_name + f_info[feat].name);

					if (f_info[feat].edge)
					{
						cptr edge;

						edge = (f_name + f_info[f_info[feat].edge].name);

						msg_format("Building %s%s surrounded by %s.", (big?"big ":""),name,edge);
					}
					else
					{
					msg_format ("Building %s%s.", (big?"big ":""),name);
					}
				}

				/* Hack -- Choose starting point */
				y = rand_spread(DUNGEON_HGT / 2, 10);
				x = rand_spread(DUNGEON_WID / 2, 15);

				build_feature(y, x, feat, big);

				if (f_info[feat].flags2 & (FF2_WATER)) level_flag |= (LF1_WATER);
				if (f_info[feat].flags2 & (FF2_LAVA)) level_flag |= (LF1_LAVA);
				if (f_info[feat].flags2 & (FF2_ICE)) level_flag |= (LF1_ICE);
				if (f_info[feat].flags2 & (FF2_ACID)) level_flag |= (LF1_ACID);
				if (f_info[feat].flags2 & (FF2_OIL)) level_flag |= (LF1_OIL);
				if (f_info[feat].flags2 & (FF2_CHASM)) level_flag |= (LF1_CHASM);

			}

			get_feat_num_hook = NULL;

		}

	}

	/* Hack -- build a tower in the centre of the level */
	if ((zone->tower) && (p_ptr->depth >= min_depth(p_ptr->dungeon)))
	{
		int typ = v_info[zone->tower].typ;

		/* Hack - set to center of level */
		int by0 = (dun->row_rooms - (room[typ].dy1 + room[typ].dy2)) /2;
		int bx0 = (dun->col_rooms - (room[typ].dx1 + room[typ].dx2)) /2;

		/* Extract blocks */
		int by1 = by0 + room[typ].dy1;
		int bx1 = bx0 + room[typ].dx1;
		int by2 = by0 + room[typ].dy2;
		int bx2 = bx0 + room[typ].dx2;

		vault_type *v_ptr;

		/* It is *extremely* important that the following calculation */
		/* be *exactly* correct to prevent memory errors XXX XXX XXX */
	
		/* Get the location of the tower */
		y = ((by1 + by2 + 1) * BLOCK_HGT) / 2;
		x = ((bx1 + bx2 + 1) * BLOCK_WID) / 2;

		/* Hack -- are we directly above another tower? */
		if ((p_ptr->depth == zone->level) && (p_ptr->depth > min_depth(p_ptr->dungeon)))
		{
			dungeon_zone *roof;
	
			get_zone(&roof,p_ptr->dungeon,p_ptr->depth-1);

			v_ptr = &v_info[roof->tower];

			build_roof(y, x, v_ptr->hgt, v_ptr->wid, v_text + v_ptr->text);
		}

		v_ptr = &v_info[zone->tower];

		/* Hack -- Build the tower */
		build_tower(y, x, v_ptr->hgt, v_ptr->wid, v_text + v_ptr->text);

		/* Reserve some blocks */
		for (by = by1; by <= by2; by++)
		{
			for (bx = bx1; bx <= bx2; bx++)
			{
				dun->room_map[by][bx] = TRUE;
	
				dun_room[by][bx] = dun->cent_n+1;
	
			}
		}
	
		/* Initialise room description */
		room_info[dun->cent_n+1].type = ROOM_TOWER;
		room_info[dun->cent_n+1].flags = 0; /* Will be set to ROOM_ICKY at end of generation */
		dun->cent[dun->cent_n].y = y;
		dun->cent[dun->cent_n].x = x;
		dun->cent_n++;

		/* Hack -- descending player always in tower */
		if ((level_flag & LF1_SURFACE) && (p_ptr->create_up_stair))
		{
			player_place(y, x);
		}

		/* Hack -- always have upstairs */
		else if (level_flag & LF1_SURFACE)
		{
			feat_near(FEAT_LESS, y, x);
		}

	}

	/* Hack -- All levels deeper than 20 on surface are 'destroyed' */
	if ((p_ptr->depth > 20) && (level_flag & (LF1_SURFACE))) level_flag |= (LF1_DESTROYED);

	/* Hack -- All levels with escorts are 'battlefields' */
	if (RF1_ESCORT & (1L << (t_info[p_ptr->dungeon].r_flag-1))) level_flag |= (LF1_BATTLE);
	if (RF1_ESCORTS & (1L << (t_info[p_ptr->dungeon].r_flag-1))) level_flag |= (LF1_BATTLE);

	/* Battlefields don't have rooms, but do have paths across the level */
	if (level_flag & (LF1_BATTLE)) level_flag &= ~(LF1_ROOMS);

	/* Non-destroyed surface locations don't have rooms, but do have paths across the level */
	if (((level_flag & (LF1_SURFACE)) != 0) && ((level_flag & (LF1_DESTROYED)) == 0)) level_flag &= ~(LF1_ROOMS);

	/* Towers don't have rooms or tunnels */
	if (((level_flag & (LF1_TOWER)) != 0) && ((level_flag & (LF1_SURFACE)) == 0)) level_flag &= ~(LF1_ROOMS | LF1_TUNNELS);

	/* Build some rooms or points to connect tunnels */
	if ((level_flag & (LF1_ROOMS | LF1_TUNNELS)) != 0)
		for (i = 0; i < DUN_ROOMS; i++)
	{
		/* Pick a block for the room */
		by = rand_int(dun->row_rooms);
		bx = rand_int(dun->col_rooms);

		/* Align dungeon rooms */
		if (dungeon_align)
		{
			/* Slide some rooms right */
			if ((bx % 3) == 0) bx++;

			/* Slide some rooms left */
			if ((bx % 3) == 2) bx--;
		}

		/* Don't have rooms or sometimes has rooms */
		if (((level_flag & (LF1_ROOMS)) == 0) || (((level_flag & (LF1_MINE)) != 0) && (dun->cent_n % 2)))
		{
			/* Attempt a "non-existent" room */
			if (room_build(by, bx, 0)) continue;

			/* Never mind */
			continue;
		}

		/* Attempt to build a vault */
		if (level_flag & (LF1_VAULT))
		{
			room_build(by, bx, !i ? 8 : 1);

			continue;
		}

		/* Mines always have irregular rooms */
		else if (level_flag & (LF1_MINE))
		{
			room_build(by, bx, 2);

			continue;
		}

		/* Other levels have regular shaped rooms */
		else
		{
			room_build(by, bx, 1);

			continue;
		}
#if 0
		/* Attempt an "unusual" room */
		if (rand_int(DUN_UNUSUAL) < p_ptr->depth)
		{
			/* Roll for room type */
			k = rand_int(100);

			/* Attempt a very unusual room */
			if (rand_int(DUN_UNUSUAL) < p_ptr->depth)
			{
				/* Type 8 -- Greater vault (10%) */
				if ((k < 10) && room_build(by, bx, 8)) continue;

				/* Type 7 -- Lesser vault (15%) */
				if ((k < 25) && room_build(by, bx, 7)) continue;

				/* Type 6 -- Monster pit (15%) */
				if ((k < 40) && room_build(by, bx, 6)) continue;

				/* Type 5 -- Monster nest (10%) */
				if ((k < 50) && room_build(by, bx, 5)) continue;
			}

			/* Type 4 -- Large room (25%) */
			if ((k < 25) && room_build(by, bx, 4)) continue;

			/* Type 3 -- Cross room (25%) */
			if ((k < 50) && room_build(by, bx, 3)) continue;

			/* Type 2 -- Overlapping (50%) */
			if ((k < 100) && room_build(by, bx, 2)) continue;
		}

		/* Attempt a trivial room */
		if (room_build(by, bx, 1)) continue;
#endif
	}

	/* Special boundary walls -- Top */
	for (x = 0; x < DUNGEON_WID; x++)
	{
		y = 0;

		cave_feat[y][x] = FEAT_PERM_SOLID;

		cave_info[y][x] |= (CAVE_XLOS);
		cave_info[y][x] |= (CAVE_XLOF);
	}

	/* Special boundary walls -- Bottom */
	for (x = 0; x < DUNGEON_WID; x++)
	{
		y = DUNGEON_HGT-1;

		cave_feat[y][x] = FEAT_PERM_SOLID;

		cave_info[y][x] |= (CAVE_XLOS);
		cave_info[y][x] |= (CAVE_XLOF);
	}

	/* Special boundary walls -- Left */
	for (y = 0; y < DUNGEON_HGT; y++)
	{
		x = 0;

		cave_feat[y][x] = FEAT_PERM_SOLID;

		cave_info[y][x] |= (CAVE_XLOS);
		cave_info[y][x] |= (CAVE_XLOF);
	}

	/* Special boundary walls -- Right */
	for (y = 0; y < DUNGEON_HGT; y++)
	{
		x = DUNGEON_WID-1;

		cave_feat[y][x] = FEAT_PERM_SOLID;

		cave_info[y][x] |= (CAVE_XLOS);
		cave_info[y][x] |= (CAVE_XLOF);
	}

	/* Start with no tunnel doors */
	dun->door_n = 0;

	/* Start with no tunnel doorways */
	dun->next_n = 0;

	/* Hack -- Scramble the room order */
	for (i = 0; i < dun->cent_n; i++)
	{
		int pick1 = rand_int(dun->cent_n);
		int pick2 = rand_int(dun->cent_n);
		y1 = dun->cent[pick1].y;
		x1 = dun->cent[pick1].x;
		dun->cent[pick1].y = dun->cent[pick2].y;
		dun->cent[pick1].x = dun->cent[pick2].x;
		dun->cent[pick2].y = y1;
		dun->cent[pick2].x = x1;
	}

	/* Set number of partitions */
	dun->part_n = dun->cent_n;

	/* Partition rooms */
	for (i = 0; i < dun->cent_n; i++)
	{
		dun->part[i] = i;		
	}

	/*
	 * New tunnel generation routine.
	 *
	 * We partition the rooms into distinct partition numbers. We then find the room in
	 * each partition with the closest neighbour in an adjacent partition and attempt
	 * to connect the two rooms.
	 *
	 * When two rooms are connected by build_tunnel, the partitions are merged.
	 *
	 * We repeat, until there is only one partition.
	 */

	while (dun->part_n > 1)
	{
		for (i = 0; i < dun->cent_n; i++)
		{
			int dist = 30000;
			int choice = -1;

			for (j = 0; j < dun->cent_n; j++)
			{
				if (dun->part[j] != i) continue;

				for (k = 0; k < dun->cent_n; k++)
				{
					int dist1 = distance(dun->cent[j].y, dun->cent[j].x, dun->cent[k].y, dun->cent[k].x);

					/* Better choice? */
					if ((dun->part[j] != dun->part[k]) && (dist > dist1))
					{
						dist = dist1;
						choice = k;
					}
				}

				/* Connect the room to the nearest neighbour */
				build_tunnel(dun->cent[j].y, dun->cent[j].x, dun->cent[k].y, dun->cent[k].x);

				/* Finish partition */
				if (dun->part_n == 1) break;
			}

			/* Finish partition */
			if (dun->part_n == 1) break;
		}
	}


/* Old connection routine below */
#if 0
	/* Paranoia */
	if (dun->cent_n)
	{
		/* Hack -- connect the first room to the last room */
		y = dun->cent[dun->cent_n-1].y;
		x = dun->cent[dun->cent_n-1].x;
	}

	/* Connect all the rooms together */
	for (i = 0; i < dun->cent_n; i++)
	{
		/* Connect the room to the previous room */
		build_tunnel(dun->cent[i].y, dun->cent[i].x, y, x);

		/* Remember the "previous" room */
		y = dun->cent[i].y;
		x = dun->cent[i].x;
	}
#endif

	/* Place intersection doors */
	for (i = 0; i < dun->door_n; i++)
	{
		int dk = rand_int(100) < 50 ? 1 : -1;
		int count = 0;

		/* Extract junction location */
		y = dun->door[i].y;
		x = dun->door[i].x;

		/* Try a door in one direction */
		/* If the first created door is secret, stop */
		for (j = 0, k = rand_int(4); j < 4; j++)
		{
			if (try_door(y + ddy_ddd[k], x + ddx_ddd[k])) count++;

			if ((!count) && (f_info[cave_feat[y + ddy_ddd[k]][x + ddx_ddd[k]]].flags1 & (FF1_SECRET))) break;

			k = (k + dk) % 4;
		}
	}

	/* Place room decorations */
	for (i = 0; i < dun->next_n; i++)
	{
		/* Extract doorway location */
		y = dun->next[i].y;
		x = dun->next[i].x;

		/* Place feature if required */
		if (dun->next_feat[i]) cave_set_feat(y, x, dun->next_feat[i]);
	}

	/* Hack -- Sandstone streamers are shallow */
	if (rand_int(DUN_STR_SLV) > p_ptr->depth)
	{

		/* Hack -- Add some sandstone streamers */
		for (i = 0; i < DUN_STR_SAN; i++)
		{
			build_streamer(FEAT_SANDSTONE, 0);
		}
	}

	else
	{

		/* Hack -- Add some magma streamers */
		for (i = 0; i < DUN_STR_MAG; i++)
		{
			build_streamer(FEAT_MAGMA, DUN_STR_MC);
		}

	}

	/* Hack -- Add some quartz streamers */
	for (i = 0; i < ((level_flag & LF1_MINE) != 0 ? DUN_STR_QUA * 2 : DUN_STR_QUA); i++)
	{		
		build_streamer(FEAT_QUARTZ, DUN_STR_QC);
	}

	/* Hack -- Add a rich mineral vein very rarely */
	if (!rand_int(DUN_STR_GOL))
	{
		build_streamer(FEAT_QUARTZ, DUN_STR_GC);
	}

	/* Hack -- Add cracks through the dungeon occasionally */
	if (!(rand_int(DUN_STR_CRA)))
	{
		build_streamer(FEAT_WALL_C, DUN_STR_CC);
	}

	/* Destroy the level if necessary */
	if ((level_flag & (LF1_DESTROYED)) != 0) destroy_level();

	/* Hack -- have less monsters during day light */
	if ((level_flag & (LF1_DAYLIGHT)) != 0) k = (p_ptr->depth / 6);
	else k = (p_ptr->depth / 3);

	if (k > 10) k = 10;
	if (k < 2) k = 2;

	/* Hack -- make sure we have rooms/corridors to place stuff */
	if ((level_flag & (LF1_ROOMS | LF1_TOWER)) != 0)
	{	
		/* Place 1 or 2 down stairs near some walls */
		alloc_stairs(FEAT_MORE, rand_range(1, 2), 3);

		/* Place 1 or 2 up stairs near some walls */
		alloc_stairs(FEAT_LESS, rand_range(1, 2), 3);

		/* Place 2 random stairs near some walls */
		alloc_stairs(0, 2, 3);

		/* Put some rubble in corridors -- we want to exclude towers unless other rooms on level */
		if ((level_flag & (LF1_ROOMS)) != 0) alloc_object(ALLOC_SET_CORR, ALLOC_TYP_RUBBLE, randint(k));
	
		/* Place some traps in the dungeon */
		alloc_object(ALLOC_SET_BOTH, ALLOC_TYP_TRAP, randint(k));
	
		/* Place some features in rooms */
		alloc_object(ALLOC_SET_ROOM, ALLOC_TYP_FEATURE, 1);
	}
	else if (!dun->entrance)
	{
		/* Place the dungeon entrance */
		while (TRUE)
		{
			/* Pick a location at least "three" from the outer walls */
			y = rand_range(3, DUNGEON_HGT - 4);
			x = rand_range(3, DUNGEON_WID - 4);

			/* Require a "naked" floor grid */
			if (cave_naked_bold(y, x)) break;
		}

		place_random_stairs(y, x, FEAT_ENTRANCE);
	}

	/* Determine the character location */
	if ((p_ptr->py == 0) || (p_ptr->px == 0)) new_player_spot();

	/* Pick a base number of monsters */
	i = MIN_M_ALLOC_LEVEL + randint(8);

	/* Put some monsters in the dungeon */
	for (i = i + k; i > 0; i--)
	{
		(void)alloc_monster(0, TRUE);
	}

	/* Hack -- make sure we have rooms to place stuff */
	if ((level_flag & (LF1_ROOMS | LF1_TOWER)) != 0)
	{
		/* Put some objects in rooms */
		alloc_object(ALLOC_SET_ROOM, ALLOC_TYP_OBJECT, Rand_normal(DUN_AMT_ROOM, 3));
	
		/* Put some objects/gold in the dungeon */
		alloc_object(ALLOC_SET_BOTH, ALLOC_TYP_OBJECT, Rand_normal(DUN_AMT_ITEM, 3));
		alloc_object(ALLOC_SET_BOTH, ALLOC_TYP_GOLD, Rand_normal(DUN_AMT_GOLD, 3));
	}

	/* Apply illumination */
	if ((level_flag & (LF1_SURFACE)) != 0) town_illuminate((level_flag & (LF1_DAYLIGHT)) != 0);

	/* Ensure quest monsters */
	if (is_quest(p_ptr->depth))
	{
		/* Ensure quest monsters */
		for (i = 1; i < z_info->r_max; i++)
		{
			monster_race *r_ptr = &r_info[i];

			/* Ensure quest monsters */
			if ((r_ptr->flags1 & (RF1_QUESTOR)) &&
			    (r_ptr->level == p_ptr->depth) &&
			    (r_ptr->cur_num <= 0))
			{
				int y, x;

				/* Pick a location */
				while (1)
				{
					y = rand_int(DUNGEON_HGT);
					x = rand_int(DUNGEON_WID);

					if (cave_naked_bold(y, x)) break;
				}

				/* Place the questor */
				place_monster_aux(y, x, i, FALSE, TRUE);
			}
		}
	}

	/* Ensure guardian monsters */
	if ((level_flag & (LF1_GUARDIAN)) != 0)
	{
		int y, x;

		/* Pick a location */
		while (1)
		{
			y = rand_int(DUNGEON_HGT);
			x = rand_int(DUNGEON_WID);

			if (place_monster_here(y, x, zone->guard) > 0) break;
		}

		/* Place the questor */
		place_monster_aux(y, x, zone->guard, FALSE, TRUE);
	}

	/* Hack -- restrict teleporation in towers */
	/* XXX Important that this occurs after placing the player */
	if ((level_flag & (LF1_TOWER)) != 0)
	{
		room_info[1].flags = (ROOM_ICKY);
	}
}



/*
 * Builds a store at a given pseudo-location
 *
 * As of 2.7.4 (?) the stores are placed in a more "user friendly"
 * configuration, such that the four "center" buildings always
 * have at least four grids between them, to allow easy running,
 * and the store doors tend to face the middle of town.
 *
 * The stores now lie inside boxes from 3-9 and 12-18 vertically,
 * and from 7-17, 21-31, 35-45, 49-59.  Note that there are thus
 * always at least 2 open grids between any disconnected walls.
 *
 * Note the use of "town_illuminate()" to handle all "illumination"
 * and "memorization" issues.
 */
static void build_store(int feat, int yy, int xx)
{
	int y, x, y0, x0, y1, x1, y2, x2, tmp;

	/* Hack -- extract char value */
	byte d_char = f_info[feat].d_char;

	/* Hack -- don't build building for some 'special locations' */
	bool building = (((d_char > '0') && (d_char <= '8')) || (d_char == '+'));

	town_type *t_ptr = &t_info[p_ptr->dungeon];
	dungeon_zone *zone=&t_ptr->zone[0];;

	/* Get the zone */
	get_zone(&zone,p_ptr->dungeon,p_ptr->depth);

	/* Find the "center" of the store */
	y0 = yy * 9 + 6;
	x0 = xx * 14 + 12;

	/* Determine the store boundaries */
	y1 = y0 - randint((yy == 0) ? 3 : 2);
	y2 = y0 + randint((yy == 1) ? 3 : 2);
	x1 = x0 - randint(5);
	x2 = x0 + randint(5);

	/* Hack -- decrease building size to create space for small terrain */
	if (zone->small)
	{
		if (x2 == 31) x2--;
		if (x1 == 35) x1++;
		if (x1 == 36) x1++;
		if (y2 == 9) y2--;
		if (y1 == 12) y1++;
	}

	/* Create a building? */
	if (building)
	{
		/* Build an invulnerable rectangular building */
		for (y = y1; y <= y2; y++)
		{
			for (x = x1; x <= x2; x++)
			{
				/* Create the building */
				cave_set_feat(y, x, FEAT_PERM_EXTRA);
			}
		}
	}

	/* Pick a door direction (S,N,E,W) */
	tmp = rand_int(4);

	/* Re-roll "annoying" doors */
	if (((tmp == 0) && (yy == 1)) ||
	    ((tmp == 1) && (yy == 0)) ||
	    ((tmp == 2) && (xx == 3)) ||
	    ((tmp == 3) && (xx == 0)))
	{
		/* Pick a new direction */
		tmp = rand_int(4);
	}

	/* Extract a "door location" */
	switch (tmp)
	{
		/* Bottom side */
		case 0:
		{
			y = y2;
			x = rand_range(x1, x2);
			break;
		}

		/* Top side */
		case 1:
		{
			y = y1;
			x = rand_range(x1, x2);
			break;
		}

		/* Right side */
		case 2:
		{
			y = rand_range(y1, y2);
			x = x2;
			break;
		}

		/* Left side */
		default:
		{
			y = rand_range(y1, y2);
			x = x1;
			break;
		}
	}

	/* Clear previous contents, add a store door */
	cave_set_feat(y, x, feat);
}




/*
 * Generate the "consistent" town features, and place the player
 *
 * Hack -- play with the R.N.G. to always yield the same town
 * layout, including the size and shape of the buildings, the
 * locations of the doorways, and the location of the stairs.
 */
static void town_gen_hack(void)
{
	int y, x, k, n;

	int rooms[MAX_STORES];

	town_type *t_ptr = &t_info[p_ptr->dungeon];
	dungeon_zone *zone=&t_ptr->zone[0];;

	/* Get the zone */
	get_zone(&zone,p_ptr->dungeon,p_ptr->depth);

	/* Hack -- Use the "simple" RNG */
	Rand_quick = TRUE;

	/* Hack -- Induce consistant town layout */
	Rand_value = seed_town;

	/* Then place some floors */
	for (y = 1; y < TOWN_HGT-1; y++)
	{
		for (x = 1; x < TOWN_WID-1; x++)
		{
			/* Create terrain on top */
			build_terrain(y, x, zone->big);
		}
	}

	/* MegaHack -- place small terrain north to south & bridge east to west */
	if (zone->small)
	{
		for (y = 1; y < TOWN_HGT-1; y++)
		{
			for (x = 32; x < 36; x++)
			{
				/* Create terrain on top */
				build_terrain(y, x, zone->small);
			}
		}

		for (y = 10; y < 12; y++)
		{
			for (x = 1; x < TOWN_WID-1; x++)
			{
				/* Create terrain on top */
				cave_alter_feat(y, x, FS_BRIDGE);
			}
		}

		/* Hack -- town square */
		if (feat_state(zone->big, FS_BRIDGE) == zone->small)
		{
			for (y = 2; y < 20; y++)
			{
				for (x = 6; x < 61; x++)
				{
					/* Exclude already built terrain */
					if ((y < 10) || (y >= 12) || (x< 32) || (x >= 36))
					{
						/* Create terrain on top */
						build_terrain(y, x, zone->small);
					}
				}
			}	
		}
	}

	/* Prepare an Array of "remaining stores", and count them */
	for (n = 0; n < MAX_STORES; n++) rooms[n] = n;

	/* Place two rows of stores */
	for (y = 0; y < 2; y++)
	{
		/* Place four stores per row */
		for (x = 0; x < 4; x++)
		{
			/* Pick a random unplaced store */
			k = ((n <= 1) ? 0 : rand_int(n));

			/* Build that store at the proper location */
			if (t_ptr->store[rooms[k]]) build_store(t_ptr->store[rooms[k]], y, x);
				
			/* Shift the stores down, remove one store */
			rooms[k] = rooms[--n];
		}
	}

	/* Place the dungeon entrance */
	while (TRUE)
	{
		/* Pick a location at least "three" from the outer walls */
		y = rand_range(3, TOWN_HGT - 4);
		x = rand_range(3, TOWN_WID - 4);

		/* Require a "naked" floor grid */
		if (cave_naked_bold(y, x)) break;
	}

	/* Clear previous contents, add dungeon entrance */
	place_random_stairs(y, x, FEAT_ENTRANCE);

	/* Place the player */
	player_place(y, x);

	/* Sometimes we have to place upstairs as well */
	if (((t_info[p_ptr->dungeon].zone[0].tower) &&
		(p_ptr->depth < max_depth(p_ptr->dungeon)) && (p_ptr->depth > min_depth(p_ptr->dungeon)))
		|| (p_ptr->depth > min_depth(p_ptr->dungeon)))
	{
		/* Place the up stairs */
		while (TRUE)
		{
			/* Pick a location at least "three" from the outer walls */
			y = rand_range(3, TOWN_HGT - 4);
			x = rand_range(3, TOWN_WID - 4);

			/* Require a "naked" floor grid */
			if (cave_naked_bold(y, x)) break;
		}

		/* Clear previous contents, add up stairs */
		cave_set_feat(y, x, FEAT_LESS);
	}

	/* Hack -- use the "complex" RNG */
	Rand_quick = FALSE;

}




/*
 * Town logic flow for generation of new town
 *
 * We start with a fully wiped cave of normal floors.
 *
 * Note that town_gen_hack() plays games with the R.N.G.
 *
 * This function does NOT do anything about the owners of the stores,
 * nor the contents thereof.  It only handles the physical layout.
 *
 * We place the player on the stairs at the same time we make them.
 *
 * Hack -- since the player always leaves the dungeon by the stairs,
 * he is always placed on the stairs, even if he left the dungeon via
 * word of recall or teleport level.
 */
static void town_gen(void)
{
	int i, y, x;

	int residents;

	int by,bx;

	town_type *t_ptr = &t_info[p_ptr->dungeon];
	dungeon_zone *zone=&t_ptr->zone[0];;

	/* Get the zone */
	get_zone(&zone,p_ptr->dungeon,p_ptr->depth);

	/* Initialize the room table */
	for (by = 0; by < MAX_ROOMS_ROW; by++)
	{
		for (bx = 0; bx < MAX_ROOMS_COL; bx++)
		{
			dun_room[by][bx] = 0;
		}
	}

	/* Initialise 'zeroeth' room description */
	room_info[0].flags = 0;

	/* Town does not have an ecology */
	cave_ecology.num_races = 0;
	cave_ecology.ready = FALSE;

	/* Day time */
	if ((level_flag & (LF1_DAYLIGHT)) != 0)
	{
		/* Number of residents */
		residents = MIN_M_ALLOC_TD;
	}

	/* Night time / underground */
	else
	{
		/* Number of residents */
		residents = MIN_M_ALLOC_TN;

	}

	/* Start with solid walls */
	for (y = 0; y < DUNGEON_HGT; y++)
	{
		for (x = 0; x < DUNGEON_WID; x++)
		{
			/* Create "solid" perma-wall */
			cave_set_feat(y, x, FEAT_PERM_SOLID);
		}
	}

	/* Then place some floors */
	for (y = 1; y < TOWN_HGT-1; y++)
	{
		for (x = 1; x < TOWN_WID-1; x++)
		{
			/* Create empty ground */
			cave_set_feat(y, x, f_info[zone->big].flags1 & (FF1_FLOOR) ? zone->big : FEAT_GROUND);
		}
	}

	/* Build stuff */
	town_gen_hack();

	/* Apply illumination */
	if ((level_flag & (LF1_SURFACE)) != 0) town_illuminate((level_flag & (LF1_DAYLIGHT)) != 0);

	/* Ensure guardian monsters */
	if (((level_flag & (LF1_GUARDIAN)) != 0) && ((level_flag & (LF1_DAYLIGHT)) == 0))
	{
		/* Pick a location */
		while (1)
		{
			y = rand_range(3, TOWN_HGT - 4);
			x = rand_range(3, TOWN_WID - 4);

			/* Require a "naked" floor grid */
			if (cave_naked_bold(y, x)) break;
		}

		/* Place the questor */
		place_monster_aux(y, x, zone->guard, FALSE, TRUE);
	}
	else
	{
		/* Ensure wandering monsters suit the dungeon level */
		get_mon_num_hook = dun_level_mon;
	
		/* Prepare allocation table */
		get_mon_num_prep();

		/* Make some residents */
		for (i = 0; i < residents; i++)
		{
			/* Make a resident */
			(void)alloc_monster(3, TRUE);
		}

		get_mon_num_hook = NULL;

		/* Prepare allocation table */
		get_mon_num_prep();
	}
}


/*
 * Generate a random dungeon level
 *
 * Hack -- regenerate any "overflow" levels
 *
 * Hack -- allow auto-scumming via a gameplay option.
 *
 * Note that this function resets "cave_feat" and "cave_info" directly.
 */
void generate_cave(void)
{
	int i, y, x, num;

	/* The dungeon is not ready */
	character_dungeon = FALSE;

        /* There is no dynamic terrain */
        dyna_full = FALSE;
	dyna_n = 0;

	/* Generate */
	for (num = 0; TRUE; num++)
	{
		bool okay = TRUE;

		cptr why = NULL;


		/* Reset */
		o_max = 1;
		m_max = 1;

		/* Start with a blank cave */
		for (y = 0; y < DUNGEON_HGT; y++)
		{
			for (x = 0; x < DUNGEON_WID; x++)
			{
				/* No flags */
				cave_info[y][x] = 0;

				/* No flags */
				play_info[y][x] = 0;

				/* No features */
				cave_feat[y][x] = 0;

				/* No objects */
				cave_o_idx[y][x] = 0;

				/* No monsters */
				cave_m_idx[y][x] = 0;

				/* No flow */
				cave_cost[y][x] = 0;
				cave_when[y][x] = 0;
			}
		}


		/* Mega-Hack -- no player yet */
		p_ptr->px = p_ptr->py = 0;


		/* Hack -- illegal panel */
		p_ptr->wy = DUNGEON_HGT;
		p_ptr->wx = DUNGEON_WID;


		/* Reset the monster generation level */
		monster_level = p_ptr->depth;

		/* Reset the object generation level */
		object_level = p_ptr->depth;

		/* Nothing special here yet */
		good_item_flag = FALSE;

		/* Nothing good here yet */
		rating = 0;

		/* Initialise level flags */
		init_level_flags();

		/* Build the town */
		if (level_flag & (LF1_TOWN))
		{
			/* Make a town */
			town_gen();
		}

		/* Build a real level */
		else
		{
			/* Make a dungeon */
			cave_gen();
		}

		/* Hack -- ensure quest components */
		for (i = 0; i < MAX_Q_IDX; i++)
		{
			quest_type *q_ptr = &q_list[i];
			quest_event *qe_ptr = &(q_ptr->event[q_ptr->stage]);

			/* Hack -- player's actions don't change level */
			if (q_ptr->stage == QUEST_ACTION) qe_ptr = &(q_ptr->event[QUEST_ACTIVE]);

			/* Quest occurs on this level */
			if ((qe_ptr->dungeon == p_ptr->dungeon) && (qe_ptr->level == (p_ptr->depth - min_depth(p_ptr->dungeon))))
			{
				int n, j;

				n = 0;

				/* Hack -- quest partially completed */
				if (q_ptr->stage == QUEST_ACTION) n = q_ptr->event[QUEST_ACTION].number;

				/* Require features */
				if (qe_ptr->feat)
				{
					/* Check for feature type */
					while (n < qe_ptr->number)
					{
						/* Count quest features */
						for (y = 0; y < DUNGEON_HGT; y++)
						{
							for (x = 0; x < DUNGEON_WID; x++)
							{
								/* Check if feat okay */
								if (cave_feat[y][x] == qe_ptr->feat) n++;
							}
						}

						/* Try placing remaining features */
						for ( ; n < qe_ptr->number; n++)
						{
							/* Pick a "legal" spot */
							while (TRUE)
							{
								/* Location */
								y = rand_int(DUNGEON_HGT);
								x = rand_int(DUNGEON_WID);

								/* Require empty, clean, floor grid */
								if (!cave_naked_bold(y, x)) continue;

								/* Accept it */
								break;
							}

							/* Create the feature */
							cave_set_feat(y, x, qe_ptr->feat);

							/* Guard the feature */
							if (qe_ptr->race) race_near(qe_ptr->race, y, x);

							/* XXX Hide item in the feature */
						}
					}

					/* Amend quest numbers */
					if (n > qe_ptr->number) qe_ptr->number = n;
				}

				/* Require race */
				else if (qe_ptr->race)
				{
					n = 0;

					/* Check for monster race */
					while (n < qe_ptr->number)
					{
						/* Count quest races */
						for (j = 0; j < z_info->m_max; j++)
						{
							/* Check if feat okay */
							if (m_list[j].r_idx == qe_ptr->race) n++;
						}

						/* Try placing remaining monsters */
						for ( ; n < qe_ptr->number; n++)
						{
							/* Pick a "legal" spot */
							while (TRUE)
							{
								/* Location */
								y = rand_int(DUNGEON_HGT);
								x = rand_int(DUNGEON_WID);

								/* Require empty grid */
								if (!cave_empty_bold(y, x)) continue;

								/* Require monster can survive on terrain */
								if (!place_monster_here(y, x, qe_ptr->race)) continue;

								/* Accept it */
								break;
							}

							/* Create a new monster (awake, no groups) */
							(void)place_monster_aux(y, x, qe_ptr->race, FALSE, FALSE);

							/* XXX Monster should carry item */
							/* This is done as a part of death / fear etc. routine */
						}
					}

					/* Amend quest numbers */
					if (n > qe_ptr->number) qe_ptr->number = n;
				}

				/* Require object */
				else if ((qe_ptr->artifact) || (qe_ptr->ego_item_type) || (qe_ptr->kind))
				{
					n = 0;

					/* Check for object kind */
					while (n < qe_ptr->number)
					{
						/* Count quest objects */
						for (j = 0; j < z_info->m_max; j++)
						{
							/* Check if feat okay */
							if (o_list[j].k_idx)
							{
								if ((qe_ptr->artifact) && (o_list[j].name1 != qe_ptr->artifact)) continue;
								if ((qe_ptr->ego_item_type) && (o_list[j].name2 != qe_ptr->ego_item_type)) continue;
								if ((qe_ptr->kind) && (o_list[j].k_idx != qe_ptr->kind)) continue;

								n++;
							}
						}

						/* Try placing remaining objects */
						for ( ; n < qe_ptr->number; n++)
						{
							object_type object_type_body;
							object_type *o_ptr = &object_type_body;

							/* Pick a "legal" spot */
							while (TRUE)
							{
								/* Location */
								y = rand_int(DUNGEON_HGT);
								x = rand_int(DUNGEON_WID);

								/* Require empty grid */
								if (!cave_naked_bold(y, x)) continue;

								/* Prepare artifact */
								if (qe_ptr->artifact) qe_ptr->kind = lookup_kind(a_info[qe_ptr->artifact].tval, a_info[qe_ptr->artifact].sval);

								/* Prepare ego item */
								if ((qe_ptr->ego_item_type) && !(qe_ptr->kind)) qe_ptr->kind =
									lookup_kind(e_info[qe_ptr->ego_item_type].tval[0],
										e_info[qe_ptr->ego_item_type].min_sval[0]);

								/* Prepare object */
								object_prep(o_ptr, qe_ptr->kind);

								/* Prepare artifact */
								o_ptr->name1 = qe_ptr->artifact;

								/* Prepare ego item */
								o_ptr->name2 = qe_ptr->ego_item_type;

								/* Apply magic -- hack: use player level as reward level */
								apply_magic(o_ptr, p_ptr->max_lev * 2, FALSE, FALSE, FALSE);

								/* Several objects */
								if (o_ptr->number > 1) n += o_ptr->number -1;

								/* Accept it */
								break;
							}
						}
					}

					/* Amend quest numbers */
					if (n > qe_ptr->number) qe_ptr->number = n;
				}
			}
		}

		/* Extract the feeling */
		if (rating > 100) feeling = 2;
		else if (rating > 80) feeling = 3;
		else if (rating > 60) feeling = 4;
		else if (rating > 40) feeling = 5;
		else if (rating > 30) feeling = 6;
		else if (rating > 20) feeling = 7;
		else if (rating > 10) feeling = 8;
		else if (rating > 0) feeling = 9;
		else feeling = 10;

		/* Hack -- Have a special feeling sometimes */
		if (good_item_flag && !adult_preserve) feeling = 1;

		/* It takes 1000 game turns for "feelings" to recharge */
		if ((old_turn) && ((turn - old_turn) < 1000)) feeling = 0;

		/* Hack -- no feeling in the town */
		if (level_flag & (LF1_TOWN)) feeling = 0;

		/* Prevent object over-flow */
		if (o_max >= z_info->o_max)
		{
			/* Message */
			why = "too many objects";

			/* Message */
			okay = FALSE;
		}

		/* Prevent monster over-flow */
		if (m_max >= z_info->m_max)
		{
			/* Message */
			why = "too many monsters";

			/* Message */
			okay = FALSE;

		}

		/* Accept */
		if (okay) break;

		/* Message */
		if (why) msg_format("Generation restarted (%s)", why);

		/* Wipe the objects */
		wipe_o_list();

		/* Wipe the monsters */
		wipe_m_list();
	}


	/* The dungeon is ready */
	character_dungeon = TRUE;

	/* Remember when this level was "created", except in town or surface locations */
	if (!(level_flag & (LF1_TOWN | LF1_SURFACE))) old_turn = turn;

	/* Hack -- always get a feeling leaving town or surface */
	else old_turn = 0;

	/* Set dodging - 'just appeared' */
	p_ptr->dodging = 9;

	/* Redraw state */
	p_ptr->redraw |= (PR_STATE);

	/* Set maximum depth for this dungeon */
	if (t_info[p_ptr->dungeon].max_depth < p_ptr->depth - min_depth(p_ptr->dungeon))
	{
		t_info[p_ptr->dungeon].max_depth = p_ptr->depth - min_depth(p_ptr->dungeon);
	}
}
