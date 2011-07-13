/* File: info.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 *
 * UnAngband (c) 2001-2009 Andrew Doull. Modifications to the Angband 2.9.6
 * source code are released under the Gnu Public License. See www.fsf.org
 * for current GPL license details. Addition permission granted to
 * incorporate modifications in all Angband variants as defined in the
 * Angband variants FAQ. See rec.games.roguelike.angband for FAQ.
 */



#include "angband.h"

/*
 * Modes of object_flags_aux()
 */
#define OBJECT_FLAGS_FULL   1 /* Full info */
#define OBJECT_FLAGS_KNOWN  2 /* Only flags known to the player */
#define OBJECT_FLAGS_RANDOM 3 /* Only known random flags */
#define OBJECT_FLAGS_TRAP	4 /* Only effects of object in trap */


/*
 * Modes of spell_desc()
 */
#define SPELL_TARGET_NORMAL   1 /* Target selected normally */
#define SPELL_TARGET_SELF     2 /* Always targets self */
#define SPELL_TARGET_AIMED    3 /* Always targets aimed target */
#define SPELL_TARGET_COATED   4 /* Target applied from a weapon attack */
#define SPELL_TARGET_EXPLODE  5 /* Always targets radius 1 ball attack */
#define SPELL_TARGET_OTHER    6 /* Always affects a monster using it on itself */
#define SPELL_TARGET_VICTIM   7 /* Always affects the victime of a trap */
#define SPELL_TARGET_ATTACKER 8 /* Always affects the attacking monster */


/*
 * Modes of list_object_flags()
 */
#define LIST_FLAGS_CAN		1 /* Listing flags which object can do */
#define LIST_FLAGS_MAY		2 /* Listing flags which object may not do */
#define LIST_FLAGS_NOT		3 /* Listing flags which object can not do */
#define LIST_FLAGS_PREVENT  4 /* Listing flags which object prevents you from doing */


/*
 * Calculate the multiplier we'll get with a given bow type.
 */
int bow_multiplier(int sval)
{
	return (sval % 10);
}


/*
 * Obtain the "flags" for an item
 */
static void object_flags_aux(int mode, const object_type *o_ptr, u32b f0[ABILITY_ARRAY_SIZE], u32b *f1, u32b *f2, u32b *f3, u32b *f4)
{
	object_kind *k_ptr;
	int i;

	if (mode != OBJECT_FLAGS_FULL)
	{
		/* Clear */
		if (f0) for (i = 0; i < ABILITY_ARRAY_SIZE; i++) f0[i] = 0L; 
		(*f1) = (*f2) = (*f3) = (*f4) = 0L;

		if (mode >= OBJECT_FLAGS_TRAP)
		{
			if (f0) for (i = 0; i < ABILITY_MAX; i++) if (ability_bonus[i].type >= BONUS_WEAPON) f0[i] |= o_ptr->can_flags0[i/32] & (1L << (i % 32)); 
			*f1 |= o_ptr->can_flags1 & (TR1_WEAPON_FLAGS);
			*f2 |= o_ptr->can_flags2 & (TR2_WEAPON_FLAGS);
			*f3 |= o_ptr->can_flags3 & (TR3_WEAPON_FLAGS);
			*f4 |= o_ptr->can_flags4 & (TR4_WEAPON_FLAGS);

			return;
		}
		else if (mode != OBJECT_FLAGS_RANDOM)
		{
			/* Add flags object is known to have */
			if (f0) for (i = 0; i < ABILITY_ARRAY_SIZE; i++) f0[i] |= o_ptr->can_flags0[i];
			*f1 |= o_ptr->can_flags1;
			*f2 |= o_ptr->can_flags2;
			*f3 |= o_ptr->can_flags3;
			*f4 |= o_ptr->can_flags4;

			return;
		}

		/* Must be identified */
		if (!object_named_p(o_ptr)) return;
	}

	if (mode != OBJECT_FLAGS_RANDOM)
	{
		k_ptr = &k_info[o_ptr->k_idx];

		/* Base object */
		if ((f0) && (mode != OBJECT_FLAGS_FULL))
		{
			for (i = 0; i < k_ptr->ability_count; i++)
			{
				f0[k_ptr->ability[i]/32] |= (1L << (k_ptr->ability[i]%32));
			}
		}
		
		(*f1) = k_ptr->flags1;
		(*f2) = k_ptr->flags2;
		(*f3) = k_ptr->flags3;
		(*f4) = k_ptr->flags4;

		if (mode == OBJECT_FLAGS_FULL)
		{
			/* Artifact */
			if (o_ptr->name1)
			{
				artifact_type *a_ptr = &a_info[o_ptr->name1];

				if (f0) for (i = 0; i < a_ptr->ability_count; i++)
				{
					f0[a_ptr->ability[i]/32] |= (1L << (a_ptr->ability[i]%32));
				}
				
				(*f1) = a_ptr->flags1;
				(*f2) = a_ptr->flags2;
				(*f3) = a_ptr->flags3;
				(*f4) = a_ptr->flags4;
			}

			/* Ego-item */
			if (o_ptr->name2)
			{
				ego_item_type *e_ptr = &e_info[o_ptr->name2];

				if (f0) for (i = 0; i < e_ptr->ability_count; i++)
				{
					f0[e_ptr->ability[i]/32] |= (1L << (e_ptr->ability[i]%32));
				}

				(*f1) |= e_ptr->flags1;
				(*f2) |= e_ptr->flags2;
				(*f3) |= e_ptr->flags3;
				(*f4) |= e_ptr->flags4;
			}
			
			/* Item abilities */
			if (f0)
			{
				for (i = 0; i < ABILITY_MAX; i++)
				{
					if (object_aval(o_ptr, i)) f0[i/32] |= (1L << (i % 32));
				}
			}
		}

		if (mode == OBJECT_FLAGS_KNOWN)
		{
			/* Obvious artifact flags */
			if (o_ptr->name1)
			{
				artifact_type *a_ptr = &a_info[o_ptr->name1];

				/* Obvious flags (pval) */
				(*f2) |= (a_ptr->flags2 & (TR2_IGNORE_MASK));
			}
		}
	}

	if (mode != OBJECT_FLAGS_FULL)
	{
		bool spoil = FALSE;

#ifdef SPOIL_ARTIFACTS
		/* Full knowledge for some artifacts */
		if (artifact_p(o_ptr) || cheat_lore) spoil = TRUE;
#endif /* SPOIL_ARTIFACTS */

#ifdef SPOIL_EGO_ITEMS
		/* Full knowledge for some ego-items */
		if (ego_item_p(o_ptr) || cheat_lore) spoil = TRUE;
#endif /* SPOIL_ARTIFACTS */

		/* Need full knowledge or spoilers */
		if (!spoil && !(o_ptr->ident & IDENT_MENTAL)) return;

		/* Artifact */
		if (o_ptr->name1)
		{
			artifact_type *a_ptr = &a_info[o_ptr->name1];

			for (i = 0; i < a_ptr->ability_count; i++)
			{
				f0[a_ptr->ability[i]/32] |= (1L << (a_ptr->ability[i]%32));
			}

			(*f1) = a_ptr->flags1;
			(*f2) = a_ptr->flags2;
			(*f3) = a_ptr->flags3;
			(*f4) = a_ptr->flags4;

			if (mode == OBJECT_FLAGS_RANDOM)
			{
				/* Hack - remove 'ignore' flags */
				(*f2) &= ~(TR2_IGNORE_MASK);
			}
		}

		/* Ego Item */
		if ((o_ptr->name2) && (mode != OBJECT_FLAGS_RANDOM))
		{
			ego_item_type *e_ptr = &e_info[o_ptr->name2];

			for (i = 0; i < e_ptr->ability_count; i++)
			{
				f0[e_ptr->ability[i]/32] |= (1L << (e_ptr->ability[i]%32));
			}

			(*f1) = e_ptr->flags1;
			(*f2) = e_ptr->flags2;
			(*f3) = e_ptr->flags3;
			(*f4) = e_ptr->flags4;

		}

		/* Item abilities */
		if (f0)
		{
			for (i = 0; i < ABILITY_MAX; i++)
			{
				/* Clear 'learned' ability if the aval is zero */
				if (!object_aval(o_ptr, i)) f0[i/32] &= ~(1L << (i % 32));
			}
		}

		/* Full knowledge for *identified* objects */
		if (!(o_ptr->ident & IDENT_MENTAL)) return;
	}

	/* Coating */
	if (coated_p(o_ptr))
	{
		/* No extra powers */
	}
	
	/* Rune powers */
	if (runed_p(o_ptr))
	{
		int rune = o_ptr->xtra1 - OBJECT_XTRA_MIN_RUNES;
		int i;

		for (i = 0;i<MAX_RUNE_FLAGS;i++)
		{
			if ((y_info[rune].count[i]) && (y_info[rune].count[i]<= o_ptr->xtra2))
			{
				if (y_info[rune].flag[i] < 32) (*f1) |= (1L << y_info[rune].flag[i]);

				if ((y_info[rune].flag[i] >= 32)
				 && (y_info[rune].flag[i] < 64)) (*f2) |= (1L << (y_info[rune].flag[i]-32));

				if ((y_info[rune].flag[i] >= 64)
				 && (y_info[rune].flag[i] < 96)) (*f3) |= (1L << (y_info[rune].flag[i]-64));

				if ((y_info[rune].flag[i] >= 96)
				 && (y_info[rune].flag[i] < 128)) (*f4) |= (1L << (y_info[rune].flag[i]-96));
			}
		}
	}
	
	/* Hidden powers */
	if (hidden_p(o_ptr))
	{
		if (object_xtra_what[o_ptr->xtra1] == 0)
		{
			int ability = object_xtra_base[o_ptr->xtra1] + o_ptr->xtra2;

			/* Add the ability */
			if ((f0) && (object_aval(o_ptr, ability))) f0[ability/32] |= (1L << (ability %32));

			/* Guarantee sustains - consider removing this */
			if (ability_bonus[ability].type == BONUS_ADD_STAT)
			{
				if (object_aval(o_ptr, ability) > 0) (*f2) |= (TR1_SUST_STR) << ability_bonus[ability].flag_num;
			}
			
			/* Guarantee some other flags */
			switch (ability)
			{
				case ABILITY_RESIST_ACID:
				case ABILITY_BRAND_ACID:
					(*f2) |= (TR2_IGNORE_ACID);
					break;
				case ABILITY_RESIST_FIRE:
				case ABILITY_BRAND_FIRE:
					(*f2) |= (TR2_IGNORE_FIRE);
					break;
				case ABILITY_RESIST_ELEC:
				case ABILITY_BRAND_ELEC:
					(*f2) |= (TR2_IGNORE_ELEC);
					break;
				case ABILITY_RESIST_COLD:
				case ABILITY_BRAND_COLD:
					(*f2) |= (TR2_IGNORE_COLD);
					break;
				case ABILITY_BRAND_WATER:
				case ABILITY_RESIST_WATER:
					(*f2) |= (TR2_IGNORE_WATER);
					break;
				case ABILITY_BRAND_LAVA:
				case ABILITY_RESIST_LAVA:
					(*f2) |= (TR2_IGNORE_LAVA);
					break;
				case ABILITY_BRAND_LITE:
					if (f0) f0[ABILITY_LITE/32] |= (1L << (ABILITY_LITE % 32));
					break;
			}
		}
		
		if (object_xtra_what[o_ptr->xtra1] == 1)
		{
			(*f1) |= (object_xtra_base[o_ptr->xtra1] << o_ptr->xtra2);
			
			/* Guarantee some other flags */
			switch (object_xtra_base[o_ptr->xtra1] << o_ptr->xtra2)
			{
				case TR1_NO_PARALYZE:
					(*f1) |= (TR1_NO_SLOW);
					break;
				case TR1_NO_PSLEEP:
					(*f1) |= (TR1_NO_MSLEEP);
					break;
				case TR1_NO_TERROR:
					(*f1) |= (TR1_NO_FEAR);
					break;
			}
		}

		else if (object_xtra_what[o_ptr->xtra1] == 2)
		{
			(*f2) |= (object_xtra_base[o_ptr->xtra1] << o_ptr->xtra2);

			/* Guarantee some other flags */
			switch (object_xtra_base[o_ptr->xtra1] << o_ptr->xtra2)
			{
				case TR2_WALK_ACID:
				case TR2_IM_ACID:
					(*f2) |= (TR2_IGNORE_ACID);
					break;
				case TR2_WALK_FIRE:
				case TR2_IM_FIRE:
					(*f2) |= (TR2_IGNORE_FIRE);
					break;
				case TR2_WALK_WATER:
					(*f2) |= (TR2_IGNORE_WATER);
					break;
				case TR2_WALK_LAVA:
					(*f2) |= (TR2_IGNORE_LAVA);
					break;
				case TR2_IM_ELEC:
					(*f2) |= (TR2_IGNORE_ELEC);
					break;
				case TR2_IM_COLD:
					(*f2) |= (TR2_IGNORE_COLD);
					break;
			}
		}
		else if (object_xtra_what[o_ptr->xtra1] == 3)
		{
			(*f3) |= (object_xtra_base[o_ptr->xtra1] << o_ptr->xtra2);

			/* Guarantee some other flags */
			switch (object_xtra_base[o_ptr->xtra1] << o_ptr->xtra2)
			{
				case TR3_PERMA_CURSE:
					(*f3) |= (TR3_HEAVY_CURSE);
					/* Fall through */
				case TR3_STENCH:
				case TR3_HUNGER:
				case TR3_UNCONTROLLED:
				case TR3_DRAIN_MANA:
				case TR3_DRAIN_HP:
				case TR3_DRAIN_EXP:
				case TR3_AGGRAVATE:
				case TR3_HEAVY_CURSE:
					(*f3) |= (TR3_LIGHT_CURSE);
					break;
			}
		}
		else if (object_xtra_what[o_ptr->xtra1] == 4)
		{
			(*f4) |= (object_xtra_base[o_ptr->xtra1] << o_ptr->xtra2);

			/* Guarantee some other flags */
			switch (object_xtra_base[o_ptr->xtra1] << o_ptr->xtra2)
			{
				case TR4_VAMP_HP:
					(*f3) |= (TR3_DRAIN_HP | TR3_LIGHT_CURSE);
					break;
				case TR4_VAMP_MANA:
					(*f3) |= (TR3_DRAIN_MANA | TR3_LIGHT_CURSE);
					break;
				case TR4_HURT_WATER:
				case TR4_HURT_LITE:
				case TR4_LODESTONE:
				case TR4_SILENT:
				case TR4_STATIC:
				case TR4_WINDY:
				case TR4_HURT_POIS:
				case TR4_HURT_ACID:
				case TR4_HURT_ELEC:
				case TR4_HURT_FIRE:
				case TR4_HURT_COLD:
				case TR4_UNDEAD:
					(*f3) |= TR3_LIGHT_CURSE;
					break;
				case TR4_DEMON:
					(*f4) |= TR4_EVIL;
					break;
			}
		}
	}
}




/*
 * Obtain the "flags" for an item
 */
void object_flags(const object_type *o_ptr, u32b f0[ABILITY_ARRAY_SIZE], u32b *f1, u32b *f2, u32b *f3, u32b *f4)
{
	object_flags_aux(OBJECT_FLAGS_FULL, o_ptr, f0, f1, f2, f3, f4);
}

/*
 * Obtain the "aval" for a particular ability on an item kind
 */
int kind_aval(int k_idx, int ability)
{
	const object_kind *k_ptr = &k_info[k_idx];

	int i;
	
	for (i = 0; i < MAX_AVALS_KIND; i++)
	{
		if (k_ptr->ability[i]==ability) return(k_ptr->aval[i]);
	}
	
	return (0);
}



/*
 * Obtain the "aval" for a particular ability on an item
 */
int object_aval(const object_type *o_ptr, int ability)
{
	int i;

	for (i = 0; i < o_ptr->ability_count; i++)
	{
		if (o_ptr->ability[i]==ability) return(o_ptr->aval[i]);
	}
	
	return (0);
}


/*
 * Obtain the "aval" for all abilities on an item
 */
void object_eval(const object_type *o_ptr, s16b ability[ABILITY_MAX])
{
	int i;
	
	for (i = 0; i < ABILITY_MAX; i++)
	{
		ability[i] = 0;
	}

	for (i = 0; i < o_ptr->ability_count; i++)
	{
		ability[o_ptr->ability[i]] = o_ptr->aval[i];
	}
}

/*
 * Adds an ability to an existing item. Returns true iff we can add this ability.
 */
bool object_ability_add(object_type *o_ptr, int ability, int value)
{
	int i;
	
	for (i = 0; i < o_ptr->ability_count; i++)
	{
		if (o_ptr->ability[i] == ability)
		{
			o_ptr->aval[i] += value;
			return(TRUE);
		}
	}
	
	if (/*(i == o_ptr->ability_count) && */(o_ptr->ability_count < MAX_AVALS_OBJECT))
	{
		o_ptr->ability[i] = ability;
		o_ptr->aval[i] = value;
		o_ptr->ability_count++;
		
		return (TRUE);
	}
	
	return (FALSE);
}


/*
 * Sets an ability on an existing item. Returns true iff we can add this ability.
 */
bool object_ability_set(object_type *o_ptr, int ability, int value)
{
	int i;

	for (i = 0; i < o_ptr->ability_count; i++)
	{
		if (o_ptr->ability[i] == ability)
		{
			o_ptr->aval[i] = value;
			return(TRUE);
		}
	}

	if (/*(i == o_ptr->ability_count) && */(o_ptr->ability_count < MAX_AVALS_OBJECT))
	{
		o_ptr->ability[i] = ability;
		o_ptr->aval[i] = value;
		o_ptr->ability_count++;

		return (TRUE);
	}

	return (FALSE);
}


/*
 * Divides an ability on an existing item by a value. Returns true iff item has this ability.
 */
bool object_ability_div(object_type *o_ptr, int ability, int value)
{
	int i;
	
	/* Paranoia */
	if (!value) return (FALSE);
	
	for (i = 0; i < o_ptr->ability_count; i++)
	{
		if (o_ptr->ability[i] == ability)
		{
			o_ptr->aval[i] /= value;
			return(TRUE);
		}
	}
	
	return (FALSE);
}


/*
 * Removes an ability from an existing item.
 */
bool object_ability_clear(object_type *o_ptr, int ability)
{
	int i;
	bool removed = FALSE;
	
	for (i = 0; i < o_ptr->ability_count; i++)
	{
		if (o_ptr->ability[i] == ability)
		{
			o_ptr->aval[i] = o_ptr->aval[--o_ptr->ability_count];
			o_ptr->ability[i] = o_ptr->ability[o_ptr->ability_count];
			removed = TRUE;
		}
	}
	
	return (removed);
}



/*
 * The player learns whether an object has a particular ability
 */
bool object_learn_ability(object_type *o_ptr, int ability, bool floor)
{
	if (object_aval(o_ptr, ability) != 0)
	{
		object_can_ability(o_ptr, ability, floor);
		
		return (TRUE);
	}
	else
	{
		object_not_ability(o_ptr, ability, floor);
		
		return (FALSE);
	}
}


/*
 * Set obvious flags for items
 */
void object_obvious_flags(object_type *o_ptr, bool floor)
{
	u32b f0[ABILITY_ARRAY_SIZE];
	u32b not_f0[ABILITY_ARRAY_SIZE];
	u32b f1, f2, f3, f4;
	int i;

	object_kind *k_ptr = &k_info[o_ptr->k_idx];
	
	/* Spoil the object */
	object_flags(o_ptr, f0, &f1, &f2, &f3, &f4);

	/* Fully identified */
	if (o_ptr->ident & (IDENT_MENTAL))
	{
		object_can_flags(o_ptr, f0, f1, f2, f3, f4, floor);

		for (i = 0; i < ABILITY_ARRAY_SIZE; i++)
		{
			not_f0[i] = ~(f0[i]);
		}
		
		object_not_flags(o_ptr, not_f0, ~(f1), ~(f2), ~(f3), ~(f4), floor);

		return;
	}

	/* Abilities of base item are always known if aware */
	if (object_aware_p(o_ptr) || ((o_ptr->ident & (IDENT_STORE)) != 0))
	{
		for (i = 0; i < k_ptr->ability_count; i++)
		{
			o_ptr->can_flags0[k_ptr->ability[i]/32] |= (1L << (k_ptr->ability[i]%32));
		}

#if 0
		for (i = 0; i < ABILITY_ARRAY_SIZE; i++)
		{
			for (j = 0; j < MAX_AVALS_KIND; j++)
			{
				o_ptr->can_flags0[i] |= k_ptr->flags0[i][j];
			}
		}
#endif
		o_ptr->can_flags1 |= k_ptr->flags1;
		o_ptr->can_flags2 |= k_ptr->flags2;
		o_ptr->can_flags3 |= k_ptr->flags3;
		o_ptr->can_flags4 |= k_ptr->flags4;
	}
	/* Learnt abilities of flavored items are added if not aware */
	else if (k_ptr->flavor)
	{
		object_can_flags(o_ptr,x_list[k_ptr->flavor].can_flags0,
							  x_list[k_ptr->flavor].can_flags1,
							  x_list[k_ptr->flavor].can_flags2,
							  x_list[k_ptr->flavor].can_flags3,
							  x_list[k_ptr->flavor].can_flags4, floor);

		object_not_flags(o_ptr,x_list[k_ptr->flavor].not_flags0,
							  x_list[k_ptr->flavor].not_flags1,
							  x_list[k_ptr->flavor].not_flags2,
							  x_list[k_ptr->flavor].not_flags3,
							  x_list[k_ptr->flavor].not_flags4, floor);
	}

	/* Identified name */
	if ((object_named_p(o_ptr)) || (o_ptr->ident & (IDENT_STORE)))
	{
		/* Now we know what it is,
			update what we know about it from our artifact memory */
		if (o_ptr->name1)
		{
			object_can_flags(o_ptr,a_list[o_ptr->name1].can_flags0,
								  a_list[o_ptr->name1].can_flags1,
								  a_list[o_ptr->name1].can_flags2,
								  a_list[o_ptr->name1].can_flags3,
								  a_list[o_ptr->name1].can_flags4, floor);

			object_not_flags(o_ptr,a_list[o_ptr->name1].not_flags0,
								  a_list[o_ptr->name1].not_flags1,
								  a_list[o_ptr->name1].not_flags2,
								  a_list[o_ptr->name1].not_flags3,
								  a_list[o_ptr->name1].not_flags4, floor);
		}
		/* Now we know what it is,
			update what we know about it from our ego item memory */
		else if (o_ptr->name2)
		{
			/* Obvious flags */
			object_can_flags(o_ptr,e_info[o_ptr->name2].obv_flags0,
								  e_info[o_ptr->name2].obv_flags1,
								  e_info[o_ptr->name2].obv_flags2,
								  e_info[o_ptr->name2].obv_flags3,
								  e_info[o_ptr->name2].obv_flags4, floor);

			/* Known flags */
			object_can_flags(o_ptr,e_list[o_ptr->name2].can_flags0,
								  e_list[o_ptr->name2].can_flags1,
								  e_list[o_ptr->name2].can_flags2,
								  e_list[o_ptr->name2].can_flags3,
								  e_list[o_ptr->name2].can_flags4, floor);

			object_not_flags(o_ptr,e_list[o_ptr->name2].not_flags0,
								  e_list[o_ptr->name2].not_flags1,
								  e_list[o_ptr->name2].not_flags2,
								  e_list[o_ptr->name2].not_flags3,
								  e_list[o_ptr->name2].not_flags4, floor);
		}
		/* Hack -- Magic items have an 'obvious' ability for which they are named */
		else if ((o_ptr->xtra1) && (o_ptr->xtra1 < OBJECT_XTRA_MIN_RUNES) && (o_ptr->feeling < INSCRIP_MIN_HIDDEN))
		{
			if (object_xtra_what[o_ptr->xtra1] == 1)
				(o_ptr->can_flags1) |= (object_xtra_base[o_ptr->xtra1] << o_ptr->xtra2);
			else if (object_xtra_what[o_ptr->xtra1] == 2)
				(o_ptr->can_flags2) |= (object_xtra_base[o_ptr->xtra1] << o_ptr->xtra2);
			else if (object_xtra_what[o_ptr->xtra1] == 3)
				(o_ptr->can_flags3) |= (object_xtra_base[o_ptr->xtra1] << o_ptr->xtra2);
			else if (object_xtra_what[o_ptr->xtra1] == 4)
				(o_ptr->can_flags4) |= (object_xtra_base[o_ptr->xtra1] << o_ptr->xtra2);
		}
               	/* Non-ego, non-magical, non-runed average item have no more hidden abilities */
		else if (object_aware_p(o_ptr))
		{
			for (i = 0; i < ABILITY_ARRAY_SIZE; i++)
			{
				not_f0[i] = ~(f0[i]);
			}
			
			object_not_flags(o_ptr, not_f0,
								  ~(o_ptr->can_flags1),
								  ~(o_ptr->can_flags2),
								  ~(o_ptr->can_flags3),
								  ~(o_ptr->can_flags4), floor);
		}
	}
}


/*
 * Obtain the "flags" for an item which are known to the player
 */
void object_flags_known(const object_type *o_ptr, u32b f0[ABILITY_ARRAY_SIZE], u32b *f1, u32b *f2, u32b *f3, u32b *f4)
{
	object_flags_aux(OBJECT_FLAGS_KNOWN, o_ptr, f0, f1, f2, f3, f4);
}


/*
 * Describe an item's random attributes for "character dumps"
 */
void identify_random_gen(const object_type *o_ptr)
{
	/* Set the indent/wrap */
	text_out_indent = 1;
	text_out_wrap = 75;

	list_object(o_ptr, OBJECT_FLAGS_RANDOM);

	/* Reset indent/wrap */
	text_out_indent = 0;
	text_out_wrap = 0;
}

/* Hack -- must not collide with existing ident flags */
#define SF1_IDENT_PACK	0x00000001L

/*
 * Hack -- Get spell description for effects on you.
 */
static bool spell_desc_flags(const spell_type *s_ptr, const cptr intro, int level, bool detail, int target, bool introduced)
{
	int vn;

	int n,r;
	cptr vp[64];
	bool timed = FALSE;

	u32b id_flags = s_ptr->flags1;
	
	bool blood_debt = FALSE;
	bool summons = FALSE;
	bool aim_summons = FALSE;

	(void)level;

	/* Only apply effects to player */
	if ((target != SPELL_TARGET_NORMAL) && (target != SPELL_TARGET_SELF)) return (FALSE);

	/* Collect detects */
	vn = 0;
	if (s_ptr->flags1 & (SF1_DETECT_DOORS))	vp[vn++] = "doors";
	if (s_ptr->flags1 & (SF1_DETECT_TRAPS))	vp[vn++] = "traps";
	if (s_ptr->flags1 & (SF1_DETECT_STAIRS))	vp[vn++] = "stairs";
	if (s_ptr->flags1 & (SF1_DETECT_WATER))	vp[vn++] = "running water";
	if (s_ptr->flags1 & (SF1_DETECT_GOLD))	vp[vn++] = "gold, including hidden treasures";
	if (s_ptr->flags1 & (SF1_DETECT_OBJECT))	vp[vn++] = "objects, including hidden objects";
	if (s_ptr->flags1 & (SF1_DETECT_MAGIC))	vp[vn++] = "magic objects, and senses their power";
	if (s_ptr->flags1 & (SF1_DETECT_POWER))	vp[vn++] = "powerful objects, and senses their power";
	if (s_ptr->flags1 & (SF1_DETECT_CURSE))	vp[vn++] = "cursed objects, and senses their power";
	if (s_ptr->flags1 & (SF1_DETECT_MONSTER))	vp[vn++] = "visible monsters";
	if (s_ptr->flags1 & (SF1_DETECT_EVIL))	vp[vn++] = "evil monsters";
	if (s_ptr->flags1 & (SF1_DETECT_LIFE))	vp[vn++] = "living monsters";
	if (s_ptr->type == SPELL_DETECT_MIND)	vp[vn++] = "minds";
	if (s_ptr->type == SPELL_DETECT_FIRE)	vp[vn++] = "fire";
	if (s_ptr->type == SPELL_REVEAL_SECRETS)	vp[vn++] = "secrets, revealing them only if you are aware of the grid";

	/* Describe detection spells */
	if (vn)
	{
		/* Intro */
		text_out(intro);

		introduced = TRUE;

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out("detects all ");
			else if (n < vn-1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out(vp[n]);
		}

		/* End */
		text_out(format(" within %d grids", 2 * MAX_SIGHT));
	}

	/* Some identifies assume earlier IDs */
	if (s_ptr->flags1 & (SF1_IDENT_FULLY)) id_flags |= SF1_IDENT;
	if (s_ptr->type == SPELL_IDENT_PACK) id_flags |= (SF1_IDENT | SF1_IDENT_PACK);
	if (s_ptr->flags1 & (SF1_DETECT_CURSE)) id_flags |= SF1_IDENT_PACK;
	if (s_ptr->flags1 & (SF1_DETECT_MAGIC)) id_flags |= SF1_IDENT_PACK;
	if (s_ptr->flags1 & (SF1_DETECT_POWER)) id_flags |= SF1_IDENT_PACK;
	if (s_ptr->flags1 & (SF1_DETECT_MAGIC)) id_flags |= SF1_DETECT_CURSE;
	if (s_ptr->flags1 & (SF1_DETECT_MAGIC)) id_flags |= SF1_IDENT_SENSE;
	if (s_ptr->flags1 & (SF1_DETECT_POWER)) id_flags |= SF1_IDENT_SENSE;
	if (s_ptr->flags1 & (SF1_FORGET)) id_flags |= SF1_IDENT_PACK;

	/* Collect identifies */
	vn = 0;
	if (id_flags & (SF1_DETECT_CURSE)) vp[vn++]="curses";
	if (id_flags & (SF1_IDENT_SENSE)) vp[vn++]="the general power level";
	if (id_flags & (SF1_IDENT_MAGIC)) vp[vn++]="a magical attribute";
	if (id_flags & (SF1_IDENT_GAUGE)) vp[vn++]="the bonuses to hit, damage and armour class";
	if (id_flags & (SF1_IDENT_GAUGE)) vp[vn++]="the number of charges";
	if (id_flags & (SF1_IDENT_VALUE)) vp[vn++]="the value";
	if (id_flags & (SF1_IDENT_RUNES)) vp[vn++]="the types of runes";
	if ((id_flags & (SF1_IDENT)) || (s_ptr->type == SPELL_IDENT_TVAL) || (s_ptr->type == SPELL_IDENT_NAME)) vp[vn++]="the kind, ego-item and artifact names";
	if ((id_flags & (SF1_IDENT)) || (s_ptr->type == SPELL_IDENT_TVAL)) vp[vn++]="all bonuses";
	if (id_flags & (SF1_IDENT_RUMOR)) vp[vn++]="some hidden powers";
	if (id_flags & (SF1_IDENT_FULLY)) vp[vn++]="all hidden powers";
	if (id_flags & (SF1_FORGET)) vp[vn++]="all information";

	/* Describe identify spells */
	if (vn)
	{
		if (!introduced)
		{
			/* Intro */
			text_out(intro);

			introduced = TRUE;

		}
		else
		{
			text_out(" and ");
		}

		/* Intro */
		if (id_flags & (SF1_IDENT_PACK)) text_out("will ");
		else text_out ("can ");

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if ((n == 0) && (id_flags & (SF1_IDENT_SENSE))) text_out("sense ");
			else if ((n == 0) && (id_flags & (SF1_FORGET))) text_out("forget ");
			else if (n == 0) text_out("identify ");
			else if (n < vn-1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out(vp[n]);
		}

		/* Intro */
		if (id_flags & (SF1_IDENT_PACK)) text_out(" on all ");
		else text_out(" on one ");
	}

	/* Collect identifies */
	vn = 0;

	if (id_flags & (SF1_IDENT_GAUGE)) vp[vn++]="weapon";
	if (id_flags & (SF1_IDENT_GAUGE)) vp[vn++]="wearable item";
	if (id_flags & (SF1_IDENT_GAUGE)) vp[vn++]="wand";
	if (id_flags & (SF1_IDENT_GAUGE)) vp[vn++]="staff";
	if (id_flags & (SF1_IDENT | SF1_IDENT_SENSE | SF1_IDENT_MAGIC)) vp[vn++]="unknown item";
	if (id_flags & (SF1_IDENT_RUMOR | SF1_IDENT_FULLY | SF1_FORGET | SF1_IDENT_MAGIC)) vp[vn++]="known item";
	if (id_flags & (SF1_DETECT_CURSE)) vp[vn++]="cursed item";
	if (id_flags & (SF1_DETECT_MAGIC)) vp[vn++]="magic item";
	if (id_flags & (SF1_IDENT_VALUE | SF1_IDENT_RUNES)) vp[vn++]="item";

	if (s_ptr->type == SPELL_IDENT_TVAL)
	{
		int i=0;

		while ((object_group[i].tval) && (object_group[i].tval != s_ptr->param))
			i++;

		if (object_group[i].tval == s_ptr->param) vp[vn++]=object_group[i].text;
	}

	if (s_ptr->type == SPELL_IDENT_NAME)
		vp[vn++]="unknown item";

	/* Finish describing identify spells */
	if (vn)
	{
		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) { }
			else if (n < vn-1) text_out(", ");
			else text_out(" or ");

			/* Dump */
			text_out(vp[n]);

			/* Intro */
			if (id_flags & (SF1_IDENT_PACK)) text_out("s");
		}

		/* Intro */
		if (id_flags & (SF1_IDENT_PACK)) text_out(" in your pack");

		if (vn > 1) text_out(" as appropriate");

		if (id_flags & (SF1_IDENT_FULLY)) text_out(" and prevents you from magically forgetting this information");
		if (id_flags & (SF1_FORGET)) text_out(" except when the item is fully identified");
	}


	/* Collect enchantments */
	vn = 0;
	if ((s_ptr->flags1 & (SF1_ENCHANT_TOH | SF1_ENCHANT_TOD))
		|| (s_ptr->type == SPELL_BRAND_WEAPON)) vp[vn++]="weapon";
	if ((s_ptr->flags1 & (SF1_ENCHANT_TOH | SF1_ENCHANT_TOD))
		|| (s_ptr->type == SPELL_BRAND_AMMO)) vp[vn++]="missile";
	if ((s_ptr->flags1 & (SF1_ENCHANT_TOA))
		|| (s_ptr->type == SPELL_BRAND_ARMOR)) vp[vn++]="piece of armor";
	if (s_ptr->type == SPELL_BRAND_ITEM) vp[vn++]="item";

	if (s_ptr->type == SPELL_ENCHANT_TVAL)
	{
		int i=0;

		while ((object_group[i].tval) && (object_group[i].tval != s_ptr->param)) i++;

		if (object_group[i].tval == s_ptr->param) vp[vn++]=object_group[i].text;

	}

	/* Describe enchantments spells */
	if (vn)
	{
		if (!introduced)
		{
			/* Intro */
			text_out(intro);
			text_out("can enchant one ");

			introduced = TRUE;

		}
		else
		{
			text_out(" and can enchant one ");
		}
		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) { }
			else if (n < vn-1) text_out(", ");
			else text_out(" or ");

			/* Dump */
			text_out(vp[n]);

		}

	}

	/* Collect enchantments */
	vn = 0;
	if (s_ptr->flags1 & (SF1_ENCHANT_TOH)) vp[vn++]="improve accuracy to hit";
	if (s_ptr->flags1 & (SF1_ENCHANT_TOD)) vp[vn++]="increase damage";
	if (s_ptr->flags1 & (SF1_ENCHANT_TOA)) vp[vn++]="increase armor class";

	if ((s_ptr->type == SPELL_BRAND_WEAPON) ||
	    (s_ptr->type == SPELL_BRAND_ARMOR) ||
	    (s_ptr->type == SPELL_BRAND_AMMO) ||
	    (s_ptr->type == SPELL_BRAND_ITEM))
	{
		vp[vn++]=format("become %s",inscrip_text[INSCRIP_MIN_HIDDEN-INSCRIP_NULL+s_ptr->param-1]);
	}

	if (s_ptr->type == SPELL_ENCHANT_TVAL) vp[vn++]="change its kind";

	/* Describe enchantment spells */
	if (vn)
	{
		if (s_ptr->flags1 & (SF1_ENCHANT_HIGH)) text_out(" highly");

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out(" to ");
			else if (n < vn-1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out(vp[n]);
		}
	}

	/* Hack */
	r = 0;
#if 0

	/* Collect timed effects */
	vn = 0;
	if (s_ptr->flags2 & (SF2_OPP_FIRE)) vp[vn++]="fire";
	if (s_ptr->flags2 & (SF2_OPP_COLD)) vp[vn++]="cold";
	if (s_ptr->flags2 & (SF2_OPP_ACID)) vp[vn++]="acid";
	if (s_ptr->flags2 & (SF2_OPP_ELEC)) vp[vn++]="lightning";
	if (s_ptr->flags2 & (SF2_OPP_POIS)) vp[vn++]="poison";

	/* Describe timed effects */
	if (vn)
	{
		timed = TRUE;

		/* Hack -- continue sentence */
		r = 1;

		if (!introduced)
		{
			/* Intro */
			text_out(intro);

			introduced = TRUE;

		}
		else
		{
			text_out(" and ");
		}

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out("provides temporary ");
			else if (n < vn-1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out(vp[n]);
		}

		text_out(" resistance");
	}
#endif
	/* Collect timed effects */
	vn = 0;

	/* Scan */
	for (n = 0; n < TMD_MAX; n++)
	{
		/* Get description */
		if ((n < 32) && (s_ptr->timed_flags1 & (1L << n))) vp[vn++] = timed_effects[n].long_description;
		else if ((n >= 32) && (n < 64) && (s_ptr->timed_flags2 & (1L << (n-32)))) vp[vn++] = timed_effects[n].long_description;
	}

	

#if 0
	if (s_ptr->flags2 & (SF2_INFRA)) vp[vn++]="extends your infravision by 50 feet";
	if (s_ptr->flags2 & (SF2_HERO)) vp[vn++]="makes you heroic";
	if (s_ptr->flags2 & (SF2_SHERO)) vp[vn++]="makes you go berserk";
	if (s_ptr->flags2 & (SF2_BLESS)) vp[vn++]="blesses you";
	if (s_ptr->flags2 & (SF2_SHIELD)) vp[vn++]="shields you";
	if (s_ptr->flags2 & (SF2_INVIS)) vp[vn++]="makes you invisible";
	if (s_ptr->flags3 & (SF3_FREE_ACT)) vp[vn++] = "protects you from paralysis and magical slowness";
	if (s_ptr->flags2 & (SF2_SEE_INVIS)) vp[vn++]="allows you to see invisible monsters";
	if (s_ptr->flags2 & (SF2_PROT_EVIL)) vp[vn++]="protects you from evil monsters";
	if (s_ptr->flags2 & (SF2_HASTE)) vp[vn++]="makes you faster";
	if (s_ptr->flags2 & (SF2_SLOW)) vp[vn++]="slows you down";
	if (s_ptr->flags2 & (SF2_CUT)) vp[vn++]="makes you bleed";
	if (s_ptr->flags2 & (SF2_STUN)) vp[vn++]="stuns you";
	if (s_ptr->flags2 & (SF2_POISON)) vp[vn++]="poisons you";
	if (s_ptr->flags2 & (SF2_BLIND)) vp[vn++]="blinds you";
	if (s_ptr->flags2 & (SF2_FEAR)) vp[vn++]="makes you afraid";
	if (s_ptr->flags2 & (SF2_CONFUSE)) vp[vn++]="confuses you";
	if (s_ptr->flags2 & (SF2_HALLUC)) vp[vn++]="makes you hallucinate";
	if (s_ptr->flags2 & (SF2_PARALYZE)) vp[vn++]="paralyzes you";
#endif
	
	if (s_ptr->type ==SPELL_INVEN_WIELD) vp[vn++]="creates a magical weapon";
	if (s_ptr->type ==SPELL_INVEN_BOW) vp[vn++]="creates a magical bow";
	if (s_ptr->type ==SPELL_INVEN_LEFT) vp[vn++]="creates a magical ring";
	if (s_ptr->type ==SPELL_INVEN_RIGHT) vp[vn++]="creates a magical ring";
	if (s_ptr->type ==SPELL_INVEN_NECK) vp[vn++]="creates a magical amulet";
	if (s_ptr->type ==SPELL_INVEN_LITE) vp[vn++]="creates a magical light";
	if (s_ptr->type ==SPELL_INVEN_BODY) vp[vn++]="creates magical armor";
	if (s_ptr->type ==SPELL_INVEN_OUTER) vp[vn++]="creates a magical cloak";
	if (s_ptr->type ==SPELL_INVEN_ARM) vp[vn++]="creates a magical shield";
	if (s_ptr->type ==SPELL_INVEN_HEAD) vp[vn++]="creates magical headgear";
	if (s_ptr->type ==SPELL_INVEN_HANDS) vp[vn++]="creates magical gloves";
	if (s_ptr->type ==SPELL_INVEN_FEET) vp[vn++]="creates magical boots";

	/* Describe timed effects */
	if (vn)
	{
		timed = TRUE;

		if (!introduced)
		{
			/* Intro */
			text_out(intro);

			introduced = TRUE;

		}
		else if (r)
		{
			text_out(", ");
		}
		else
		{
			text_out(" and ");
		}

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) {}
			else if (n < vn-1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out(vp[n]);
		}
	}

	/* Collect stat gain effects */
	vn = 0;

#if 0
	if (s_ptr->flags3 & (SF3_INC_STR)) vp[vn++]="strength";
	if (s_ptr->flags3 & (SF3_INC_STR)) vp[vn++]="size";
	if (s_ptr->flags3 & (SF3_INC_INT)) vp[vn++]="intelligence";
	if (s_ptr->flags3 & (SF3_INC_WIS)) vp[vn++]="wisdom";
	if (s_ptr->flags3 & (SF3_INC_DEX)) vp[vn++]="dexterity";
	if (s_ptr->flags3 & (SF3_INC_DEX)) vp[vn++]="agility";
	if (s_ptr->flags3 & (SF3_INC_CON)) vp[vn++]="constitution";
	if (s_ptr->flags3 & (SF3_INC_CHR)) vp[vn++]="charisma";
#endif
	if (s_ptr->flags3 & (SF3_INC_EXP)) vp[vn++]="experience";

	/* Describe stat effects */
	if (vn)
	{
		timed = TRUE;

		if (!introduced)
		{
			/* Intro */
			text_out(intro);

			introduced = TRUE;

		}
		else
		{
			text_out(" and ");
		}

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out("increases your ");
			else if (n < vn-1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out(vp[n]);
		}
	}

	/* Roll out the duration */
	if (!detail || !timed)
	{
		/* Nothing */
	}
	else if ((s_ptr->lasts_dice) && (s_ptr->lasts_side) && (s_ptr->lasts_plus))
	{
		/* End */
		text_out(format(" for %dd%d+%d turns",s_ptr->lasts_dice,s_ptr->lasts_side,s_ptr->lasts_plus));
	}
	else if ((s_ptr->lasts_dice) && (s_ptr->lasts_side) && (s_ptr->lasts_side == 1))
	{
		/* End */
		text_out(format(" for %d turn%s",s_ptr->lasts_dice, s_ptr->lasts_dice != 1 ? "s" : ""));
	}
	else if ((s_ptr->lasts_dice) && (s_ptr->lasts_side))
	{
		/* End */
		text_out(format(" for %dd%d turns",s_ptr->lasts_dice,s_ptr->lasts_side));
	}
	else if (s_ptr->lasts_plus)
	{
		/* End */
		text_out(format(" for %d turn%s",s_ptr->lasts_plus, s_ptr->lasts_plus != 1 ? "s" : ""));
	}

	/* Collect cure effects */
	vn = 0;
#if 0
	if (s_ptr->flags3 & (SF3_CURE_CUTS)) vp[vn++]="cuts";
	if (s_ptr->flags3 & (SF3_CURE_STUN)) vp[vn++]="stun damage";
	if (s_ptr->flags3 & (SF3_CURE_POIS)) vp[vn++]="poison";
	if (s_ptr->flags3 & (SF3_CURE_FOOD)) vp[vn++]="hunger";
	if (s_ptr->flags3 & (SF3_CURE_BLIND)) vp[vn++]="blindness";
	if (s_ptr->flags3 & (SF3_CURE_IMAGE)) vp[vn++]="hallucinations";
	if (s_ptr->flags3 & (SF3_CURE_CONF)) vp[vn++]="confusion";
	if (s_ptr->flags3 & (SF3_CURE_FEAR)) vp[vn++]="fear";
	if (s_ptr->flags3 & (SF3_CURE_FEAR)) vp[vn++]="petrification";
	if (s_ptr->flags3 & (SF3_FREE_ACT)) vp[vn++]="slowness";
	if (s_ptr->flags3 & (SF3_CURE_MEM)) vp[vn++]="amnesia";
	if (s_ptr->flags3 & (SF3_CURE_CURSE)) vp[vn++]="curses";
#endif
	
	/* Scan */
	for (n = 0; n < 32; n++)
	{
		/* Get description */
		if (s_ptr->clear_timed_flags1 & (1L << n)) vp[vn++] = timed_effects[n].short_description;
	}
	
	if (s_ptr->type == SPELL_CURE_DISEASE) vp[vn++] = disease_name[s_ptr->param];

	/* Hack -- cure disease also cures minor diseases */
	if ((s_ptr->type == SPELL_CURE_DISEASE) && (s_ptr->param == 32)) vp[vn++] = disease_name[29];

	/* Describe cure effects */
	if (vn)
	{
		if (!introduced)
		{
			/* Intro */
			text_out(intro);

			introduced = TRUE;

		}
		else
		{
			text_out(" and ");
		}

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out("cures you of ");
			else if (n < vn-1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out(vp[n]);
		}
	}

	/* Collect partial fix effects */
	vn = 0;
#if 0
	if (s_ptr->type == SPELL_PFIX_POIS) vp[vn++]="poison";
	if (s_ptr->type == SPELL_PFIX_CONF) vp[vn++]="confusion";
	if (s_ptr->flags3 & (SF3_PFIX_CUTS)) vp[vn++]="cuts";
	if (s_ptr->flags3 & (SF3_PFIX_STUN)) vp[vn++]="stun damage";
	if (s_ptr->flags3 & (SF3_PFIX_CURSE)) vp[vn++]="curses";
#endif

	/* Scan */
	for (n = 0; n < 32; n++)
	{
		/* Get description */
		if (s_ptr->pfix_timed_flags1 & (1L << n)) vp[vn++] = timed_effects[n].short_description;
	}
	
	/* Describe cure effects */
	if (vn)
	{
		if (!introduced)
		{
			/* Intro */
			text_out(intro);

			introduced = TRUE;

		}
		else
		{
			text_out(" and ");
		}

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out("partially cures you of ");
			else if (n < vn-1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out(vp[n]);
		}
	}
#if 0
	/* Collect restore effects */
	vn = 0;
	if (s_ptr->flags3 & (SF3_CURE_STR)) vp[vn++]="strength";
	if (s_ptr->flags3 & (SF3_CURE_STR)) vp[vn++]="size";
	if (s_ptr->flags3 & (SF3_CURE_INT)) vp[vn++]="intelligence";
	if (s_ptr->flags3 & (SF3_CURE_WIS)) vp[vn++]="wisdom";
	if (s_ptr->flags3 & (SF3_CURE_DEX)) vp[vn++]="dexterity";
	if (s_ptr->flags3 & (SF3_CURE_DEX)) vp[vn++]="agility";
	if (s_ptr->flags3 & (SF3_CURE_CON)) vp[vn++]="constitution";
	if (s_ptr->flags3 & (SF3_CURE_CHR)) vp[vn++]="charisma";
	if (s_ptr->flags3 & (SF3_CURE_EXP)) vp[vn++]="experience";

	/* Describe restore stat effects */
	if (vn)
	{
		if (!introduced)
		{
			/* Intro */
			text_out(intro);

			introduced = TRUE;

		}
		else
		{
			text_out(" and ");
		}

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out("restores your ");
			else if (n < vn-1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out(vp[n]);
		}
	}
#endif

	/* Collect summoning */
	vn = 0;

	if ((s_ptr->type == SPELL_SUMMON) || (s_ptr->type == SPELL_AIM_SUMMON) || (s_ptr->type == SPELL_CREATE) ||
			(s_ptr->type == SPELL_AIM_CREATE))
	{
		switch(s_ptr->param)
		{
			case SUMMON_KIN: vp[vn++] = "summons kindred monsters"; break;
			case SUMMON_PLANT: vp[vn++] = "summons plants"; break;
			case SUMMON_INSECT: vp[vn++] = "summons insects"; break;
			case SUMMON_ANIMAL: vp[vn++] = "summons animals"; break;
			case SUMMON_HOUND: vp[vn++] = "summons hounds"; break;
			case SUMMON_SPIDER: vp[vn++] = "summons spiders"; break;
			case SUMMON_CLASS: vp[vn++] = "summons related classes"; break;
			case SUMMON_RACE: vp[vn++] = "summons related races"; break;
			case SUMMON_GROUP: vp[vn++] = "summons related monsters"; break;
			case SUMMON_FRIEND: vp[vn++] = "summons related monsters"; break;
			case SUMMON_UNIQUE_FRIEND : vp[vn++] = "summons related uniques"; break;
			case SUMMON_ORC: vp[vn++] = "summons orcs"; break;
			case SUMMON_TROLL: vp[vn++] = "summons trolls"; break;
			case SUMMON_GIANT: vp[vn++] = "summons giants"; break;
			case SUMMON_DRAGON: vp[vn++] = "summons dragons"; break;
			case SUMMON_HI_DRAGON: vp[vn++] = "summons high dragons"; break;
			case SUMMON_DEMON: vp[vn++] = "summons demons"; break;
			case SUMMON_HI_DEMON: vp[vn++] = "summons high demons"; break;
			case SUMMON_UNIQUE: vp[vn++] = "summons uniques"; break;
			case SUMMON_HI_UNIQUE: vp[vn++] = "summons high uniques"; break;
			case SUMMON_UNDEAD: vp[vn++] = "summons undead"; break;
			case SUMMON_HI_UNDEAD: vp[vn++] = "summons high undead"; break;
			case SUMMON_WRAITH: vp[vn++] = "summons wraiths"; break;
			default: vp[vn++] = "summons monsters"; break;
		}
		
		/* We check this this way to allow flags we might introduce later to define this instead */
		if ((s_ptr->type == SPELL_SUMMON) || (s_ptr->type == SPELL_AIM_SUMMON)) blood_debt = TRUE;
		if ((s_ptr->type == SPELL_AIM_SUMMON) || (s_ptr->type == SPELL_AIM_CREATE)) aim_summons = TRUE;
	}
	
	if ((s_ptr->type == SPELL_SUMMON_RACE) || (s_ptr->type == SPELL_AIM_SUMMON_RACE) || (s_ptr->type == SPELL_CREATE_RACE) ||
			(s_ptr->type == SPELL_AIM_CREATE_RACE))
	{
		char m_name[80];
		
		/* Get the name */
		race_desc(m_name, sizeof(m_name), s_ptr->param, 0x408, 1);

		vp[vn++] = format("summons %s",	m_name);
		
		/* We check this this way to allow flags we might introduce later to define this instead */
		if ((s_ptr->type == SPELL_SUMMON_RACE) || (s_ptr->type == SPELL_AIM_SUMMON_RACE)) blood_debt = TRUE;
		if ((s_ptr->type == SPELL_AIM_SUMMON_RACE) || (s_ptr->type == SPELL_AIM_CREATE_RACE)) aim_summons = TRUE;
	}
	
	if (s_ptr->type == SPELL_RAISE_RACE)
	{
		char m_name[80];
		
		/* Get the name */
		race_desc(m_name, sizeof(m_name), s_ptr->param, 0x408, 1);

		vp[vn++] = format("summons %s from beyond the grave",	m_name);
		blood_debt = TRUE;
	}

	if ((s_ptr->type == SPELL_SUMMON_GROUP_IDX) || (s_ptr->type == SPELL_AIM_SUMMON_GROUP_IDX) || (s_ptr->type == SPELL_CREATE_GROUP_IDX) ||
			(s_ptr->type == SPELL_AIM_CREATE_GROUP_IDX))
	{
		vp[vn++] = "summons related monsters";
		
		/* We check this this way to allow flags we might introduce later to define this instead */
		if ((s_ptr->type == SPELL_SUMMON_GROUP_IDX) || (s_ptr->type == SPELL_AIM_SUMMON_GROUP_IDX)) blood_debt = TRUE;
		if ((s_ptr->type == SPELL_AIM_SUMMON_GROUP_IDX) || (s_ptr->type == SPELL_AIM_CREATE_GROUP_IDX)) aim_summons = TRUE;
	}
	
	/* Describe summoning effects */
	if (vn)
	{
		if (!introduced)
		{
			/* Intro */
			text_out(intro);

			introduced = TRUE;

		}
		else
		{
			text_out(" and ");
		}

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) { }
			else if (n < vn-1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out(vp[n]);
		}
		
		/* For later */
		summons = TRUE;
	}

	/* Collect miscellaneous */
	vn = 0;

	if (s_ptr->flags3 & (SF3_DEC_FOOD)) vp[vn++] = "makes you weak from hunger";
#if 0
	if (s_ptr->flags2 & (SF2_SLOW_POIS)) vp[vn++] = "delays the onset of poison";
	if (s_ptr->flags2 & (SF2_SLOW_DIGEST)) vp[vn++] = "digests food more efficiently";
#endif
	if (s_ptr->flags2 & (SF2_AGGRAVATE)) vp[vn++] = "wakes up nearby monsters and hastes those in line of sight";
	if (s_ptr->type == SPELL_CREATE_KIND) vp[vn++] = "creates gold";
	if (s_ptr->flags2 & (SF2_CREATE_STAIR)) vp[vn++] = "creates a staircase under you";
	if (s_ptr->type == SPELL_WARD_GLYPH) vp[vn++] = "creates a glyph of warding under you";
	if (s_ptr->type == SPELL_WARD_TRAP) vp[vn++] = format("creates %s%s next to you",
		is_a_vowel((f_name+f_info[s_ptr->param].name)[0])?"an ":"a ",
		f_name+f_info[s_ptr->param].name);
	if (s_ptr->flags1 & (SF1_STAR_ACQUIREMENT)) vp[vn++] = "creates several excellent, superb or special items";
	else if (s_ptr->flags1 & (SF1_ACQUIREMENT)) vp[vn++] = "creates an excellent, superb or special item";
	if (s_ptr->flags2 & (SF2_TELE_LEVEL)) vp[vn++] = "pushes you through floor or ceiling";
#if 0
	if (s_ptr->flags2 & (SF2_RECALL)) vp[vn++]="returns you to the surface, or teleports you into the depths";
#endif
	if (s_ptr->flags2 & (SF2_ALTER_LEVEL)) vp[vn++] = "alters the level you are on";
	if (s_ptr->flags2 & (SF2_BANISHMENT)) vp[vn++] = "allows you to remove a monster type from a level (1d4 damage per monster)";
	if (s_ptr->flags2 & (SF2_MASS_BANISHMENT)) vp[vn++] = "removes all nearby monsters";
	if (s_ptr->flags3 & (SF3_PFIX_CURSE)) vp[vn++] = "removes a normal curse from an item";
	if (s_ptr->flags3 & (SF3_CURE_CURSE)) vp[vn++] = "removes all normal and some heavy curses from all items you are wearing or wielding";
	if (s_ptr->type == SPELL_RECHARGE) vp[vn++] = format("recharges one staff or wand for %d power", s_ptr->param);
	if (s_ptr->flags1 & (SF1_MAP_AREA)) vp[vn++] = "maps your surroundings";
	if (s_ptr->flags1 & (SF1_WIZ_LITE)) vp[vn++] = "lights up and maps the entire level";
	if (s_ptr->flags1 & (SF1_LITE_ROOM)) vp[vn++] = "lights up the room you are in";
	if (s_ptr->flags1 & (SF1_DARK_ROOM)) vp[vn++] = "plunges the room you are in into darkness";
	if (s_ptr->flags1 & (SF1_FORGET)) vp[vn++] = "erases the knowledge of the entire level from your mind";
	if (s_ptr->flags1 & (SF1_SELF_KNOW)) vp[vn++] = "reveals all knowledge of yourself";
	if (s_ptr->type == SPELL_CONCENTRATE_LITE) vp[vn++] = "concentrates light around you";
	if (s_ptr->type == SPELL_CONCENTRATE_LIFE) vp[vn++] = "concentrates life around you";
	if (s_ptr->type == SPELL_CONCENTRATE_WATER) vp[vn++] = "concentrates water around you";
	if (s_ptr->type == SPELL_RELEASE_CURSE) vp[vn++] = "releases a curse from an item";
	if (s_ptr->type == SPELL_SET_RETURN) vp[vn++] = "marks this grid as a destination for later return";
	if (s_ptr->type == SPELL_SET_OR_MAKE_RETURN) vp[vn++] = "marks this grid as a destination for later return or returns you to a marked grid";
	if (s_ptr->type == SPELL_BLOOD_BOND) vp[vn++] = "bonds you with a living creature to share damage and healing";
	if (s_ptr->type == SPELL_MINDS_EYE) vp[vn++] = "bonds you with a mind to allow you to see through its eyes";
	if (s_ptr->type == SPELL_LIGHT_CHAMBERS) vp[vn++] = "lights all rooms on the level, except vaults";
	if (s_ptr->type == SPELL_CHANGE_SHAPE) vp[vn++] = format("changes you into a %s",p_name + p_info[s_ptr->param].name);
	if (s_ptr->type == SPELL_REVERT_SHAPE) vp[vn++] = "returns you to your normal form";
	if (s_ptr->type == SPELL_REFUEL) vp[vn++] = "fuels a torch";
	if (s_ptr->type == SPELL_REST_UNTIL_DUSK) vp[vn++] = "lets you await the night undisturbed in peaceful room";
	if (s_ptr->type == SPELL_REST_UNTIL_DAWN) vp[vn++] = "lets you sleep at the inn until the day comes";
	if (s_ptr->type == SPELL_MAGIC_BLOW) vp[vn++] = "increases the effectiveness of a single round of blows";
	if (s_ptr->type == SPELL_MAGIC_SHOT) vp[vn++] = "increases the effectiveness of a single round of firing";
	if (s_ptr->type == SPELL_MAGIC_HURL) vp[vn++] = "increases the effectiveness of a single round of thrown weapons";
	if (s_ptr->type == SPELL_ACCURATE_BLOW) vp[vn++] = "increases the accuracy of a single round of blows";
	if (s_ptr->type == SPELL_ACCURATE_SHOT) vp[vn++] = "increases the accuracy of a single round of firing";
	if (s_ptr->type == SPELL_ACCURATE_HURL) vp[vn++] = "increases the accuracy of a single round of thrown weapons";
	if (s_ptr->type == SPELL_DAMAGING_BLOW) vp[vn++] = "increases the damage of a single round of blows";
	if (s_ptr->type == SPELL_DAMAGING_SHOT) vp[vn++] = "increases the damage of a single round of firing";
	if (s_ptr->type == SPELL_DAMAGING_HURL) vp[vn++] = "increases the damage of a single round of thrown weapons";
	if (s_ptr->type == SPELL_CURSE_WEAPON) vp[vn++] = "curses your weapon";
	if (s_ptr->type == SPELL_CURSE_ARMOR) vp[vn++] = "curses your armor";


	/* Describe miscellaneous effects */
	if (vn)
	{
		if (!introduced)
		{
			/* Intro */
			text_out(intro);

			introduced = TRUE;

		}
		else
		{
			text_out(" and ");
		}

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) { }
			else if (n < vn-1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out(vp[n]);
		}
	}

	/* Note for the player */
	if ((s_ptr->type >= SPELL_MAGIC_BLOW) && (s_ptr->type <= SPELL_DAMAGING_HURL))
	{
		text_out(".  ");
	}

	/* Hack -- describe the blow/shot/hurl effect */
	if ((s_ptr->type >= SPELL_MAGIC_BLOW) && (s_ptr->type <= SPELL_MAGIC_HURL))
	{
		s16b ability[ABILITY_MAX];
		u32b f0[ABILITY_ARRAY_SIZE];
		
		int i;
		int param = s_ptr->param;
		int pval = (p_ptr->lev + 19) / 20;

		/* Clear ability values */
		for (i = 0; i < ABILITY_MAX; i++) ability[i] = 0;
		
		/* Clear ability flags */
		for (i = 0; i < ABILITY_ARRAY_SIZE; i++) f0[i] = 0L;
		
		/* Set ability */
		ability[param] = pval;
		if (param < 32) f0[param/32] = (1L << param);
		else if (param < 64) f0[param/32] = (1L << (param - 32));
		else if (param < 96) f0[param/32] = (1L << (param - 64));
		else if (param < 128) f0[param/32] = (1L << (param - 96));
		
		/* List the flags */
		list_object_flags(ability, f0, 0L, 0L, 0L, 0L, pval, LIST_FLAGS_CAN);
	}

	/* Note for the player */
	if ((s_ptr->type >= SPELL_MAGIC_BLOW) && (s_ptr->type <= SPELL_DAMAGING_HURL))
	{
		text_out("This initiates the attack if successful");
	}

	/* Descrive advantages of shape change */
	if (s_ptr->type == SPELL_CHANGE_SHAPE)
	{
		text_out(format(".  Being a %s affects you.  ", p_name + p_info[s_ptr->param].name));
		
		describe_shape(s_ptr->param, FALSE);
		
		text_out(format("You will remain a %s until you change to another shape or end the shape change effect", p_name + p_info[s_ptr->param].name));
	}
	
	/* Provide more detail if summoning something */
	if (summons)
	{
		if (blood_debt) text_out(".  The summoning incurs a mana debt or blood debt which you must pay ");
		else text_out(".  Monsters summoned this way will not leave your service ");
		if (blood_debt) text_out("if the summons are killed before they leave your service");
		else text_out("nor incur a mana or blood debt if they are killed");
		
		if (aim_summons) text_out(".  You can target this summons anywhere in line of sight");
	}
	
	return (introduced);
}


/*
 * Writes the spell damage dice string into a buffer.
 *
 * Note the reckless use of a global for breath weapons.
 *
 */
bool spell_desc_damage(const spell_blow *blow_ptr, int target, int level, char *buf, int buf_size)
{
	int method = blow_ptr->method;
	method_type *method_ptr = &method_info[method];
	int d1 = blow_ptr->d_dice;
	int d2 = blow_ptr->d_side;
	int d3 = blow_ptr->d_plus;
	int d4 = 0;
	int d5 = 0;

	/* Initialise the buffer */
	buf[0] = '\0';

	/* Use player hit points */
	if (method_ptr->flags2 & (PR2_BREATH))
	{
		/* Damage uses current hit points */
		d3 = p_ptr->chp * d3 / 300;
		d1 = 0;
		d2 = 0;
	}
	/* Damage uses dice roll */
	else if (blow_ptr->levels)
	{
		/* Add in level dependent damage */
		if (blow_ptr->l_side == blow_ptr->d_side)
		{
			d1 += blow_ptr->l_dice * level / blow_ptr->levels;
		}
		else if (blow_ptr->l_side)
		{
			if (d1 || d2 || d3)
			{
				d4 = blow_ptr->l_dice * level / blow_ptr->levels;
				d5 = blow_ptr->l_side;
			}
			else
			{
				d1 = blow_ptr->l_dice * level / blow_ptr->levels;
				d2 = blow_ptr->l_side;
			}
		}

		/* Add level dependent damage */
		if (blow_ptr->l_plus)
		{
			/* Mega-hack - dispel evil/undead etc. */
			if (!level)
			{
				d3 += blow_ptr->l_plus * 25 / blow_ptr->levels;
			}
			else
			{
				d3 += blow_ptr->l_plus * level / blow_ptr->levels;
			}
		}
	}

	/* Boost with player spell power */
	d3 += p_ptr->boost_spell_power;

	/* Consolidation */
	if (d2 == 1)
	{
		d3 += d1;
		d1 = 0;
		d2 = 0;
	}

	/* Consolidation */
	if (d5 == 1)
	{
		d3 += d4;
		d4 = 0;
		d5 = 0;
	}

	/*
	 * Hack -- coatings do 20% damage, so have a completely different
	 * damage output routine.
	 */
	if ((target == SPELL_TARGET_COATED) || (target == SPELL_TARGET_ATTACKER))
	{
		int min = (d1 + d3 + d5) / 5;
		int max = (d1 * d2 + d3 + d4 * d5) / 5;

		if (max)
		{
			/* End */
			if (max != min) my_strcpy(buf, format("%d-%d", min, max), buf_size);
			else my_strcpy(buf, format("%d", max), buf_size);
		}

		/* We are done */
		return (TRUE);
	}

	/* Display the damage dice */
	if ((d1) && (d2))
	{
		/* End */
		my_strcpy(buf,format("%dd%d",d1,d2), buf_size);
	}
	else if (d3)
	{
		/* End */
		my_strcpy(buf,format("%d",d3), buf_size);

		/* We are done */
		return (TRUE);
	}

	/* Display the damage dice */
	if ((d4) && (d5))
	{
		/* End */
		my_strcat(buf,format("+%dd%d",d1,d2), buf_size);
	}

	/* Add the damage plus */
	if (d3)
	{
		/* End */
		my_strcat(buf,format("+%d",d3), buf_size);

		/* We are done */
		return (TRUE);
	}

	/* Anything output? */
	if (d1 || d2 || d4 || d5) return (TRUE);

	return (FALSE);
}


/*
 * Helper function for describe_blow.
 *
 * See attack_desc for a similar, but sufficiently different, function...
 */
static void text_out_blow(const char *s, int person, bool infinitive, int num)
{
	int state = 0;
	int match = (infinitive) != 0 ? 1 : 2;

	const char *u;
	char *t;

	char buf[128];
	int buf_size = 128;

	/* Initialise buffer */
	buf[0] = '\0';

	/* Reference buffer */
	t = buf;

	/* Copy the string */
	for (; (*s); s++)
	{
		/* Handle tense changes */
		if (*s == '|')
		{
			state++;
			if (state == 3) state = 0;
		}

		/* Handle input suppression */
		if ((state) && (state != match))
		{
			continue;
		}

		/* Handle tense changes */
		if (*s == '|')
		{
			continue;
		}

		/* Handle a number */
		if (*s == '@')
		{
			const char *v = s+1;

			switch (num)
			{
				case 1:
				{
					/* Skip white space */
					for ( ; (*v) && (*v == ' '); v++) ;

					/* Is a vowel */
					if (is_a_vowel(*v))
					{
						u = "an";
					}
					else
					{
						u = "a";
					}
					break;
				}
				case 0: u = "no"; break;
				case 2: u = "two"; break;
				case 3: u = "three"; break;
				case 4: u = "four"; break;
				case 5: u = "five"; break;
				default: u = ""; break;
			}

			while ((*u) && ((t - buf) < buf_size))
			{
				*t++ = *u++;
			}

			if (num > 5)
			{
				my_strcat(t, format("%d", num), buf_size);
				t += num / 10;
				t++;
			}
		}

		/* Handle the target*/
		else if (*s == '&')
		{
			switch(person)
			{
				case 2: u = "you"; break;
				case 3: u = "him"; break;
				case 4: u = "her"; break;
				case 5: u = "him or her"; break;
				default: u = "it"; break;
			}

			while ((*u) && ((t - buf) < buf_size))
			{
				*t++ = *u++;
			}
		}

		/* Handle possessive */
		else if (*s == '$')
		{
			switch(person)
			{
				case 2: u = "your"; break;
				case 3: u = "his"; break;
				case 4: u = "her"; break;
				case 5: u = "his or her"; break;
				default: u = "its"; break;
			}

			while ((*u) && ((t - buf) < buf_size))
			{
				*t++ = *u++;
			}
		}

		/* Handle reflexive */
		else if (*s == '%')
		{
			switch(person)
			{
				case 2: u = "yourself"; break;
				case 3: u = "himself"; break;
				case 4: u = "herself"; break;
				case 5: u = "him or herself"; break;
				default: u = "itself"; break;
			}

			while ((*u) && ((t - buf) < buf_size))
			{
				*t++ = *u++;
			}
		}

		/* Tensor */
		else if (*s == '~')
		{
			/* Add an 's' */
			if (!infinitive) *t++ = 's';
		}

		/* Plural */
		else if (*s == '#')
		{
			/* Add an 's' */
			if (num != 1)
			{
				if ((t > buf) && (*(t-1) == 's')) *t++ = 'e';
				*t++ = 's';
			}
		}

		/* Normal */
		else
		{
			/* Copy */
			*t++ = *s;
		}
	}

	/* Terminate */
	if ((t - buf) <  buf_size) *t = '\0';

	/* Truncate the string to buf_size chars */
	buf[buf_size - 1] = '\0';

	/* Output text */
	text_out(buf);
}

/*
 * Describe blow definitions
 */
#define DESC_SKIP_METHOD_INTRO	0x01
#define DESC_SKIP_METHOD		0x02
#define DESC_SKIP_METHOD_MORE	0x04
#define DESC_SKIP_EFFECT		0x08
#define DESC_SKIP_DAMAGE_DETAIL	0x10
#define DESC_MELEE_ATTACK		0x20
#define DESC_MONSTER_SELF		0x40
#define DESC_TRAP_VICTIM		0x80
#define DESC_MONSTER_ATTACKER	0x100
#define DESC_MONSTER_MALE		0x200
#define DESC_MONSTER_FEMALE		0x400

/*
 * Hack -- Get spell description for effects on target based on blow.
 */
void describe_blow(int method, int effect, int level, int feat, const char *intro, const char *damage, u16b details, int num)
{
	int i;

	char *p[7];

	feature_type *f_ptr;
	
	bool need_space = FALSE;

	/* Get method details */
	method_type *method_ptr = &method_info[method];
	effect_type *effect_ptr = &effect_info[effect];

	int rad = scale_method(method_ptr->radius, level);
	int arc = method_ptr->arc;
	int rng = scale_method(method_ptr->max_range, level);

	int person = 2;
	
	/* No method - skip method */
	if (!method) details |= (DESC_SKIP_METHOD);
	
	/* MegaHack -- rewrite 'teleports you to it' as 'teleports its enemies to itself' */
	if ((method == 160+7) || (method == 160+8))
	{
		details |= (DESC_MONSTER_SELF);
	}
	
	/* Hack -- quash range details for describing monster attacks */
	if (details & (DESC_MELEE_ATTACK))
	{
		rad = 0;
		rng = 0;
		arc = 0;
	}

	/* Initialise the output string */
	for (i = 0; i < 5; i++)
	{
		p[i] = NULL;
	}

	/* Initialise more output string */
	p[5] = "for";
	p[6] = "damage";

	/* Get method info text */
	p[0] = method_text + method_ptr->info[0];
	p[3] = method_text + method_ptr->info[1];

	/* Hack -- nothing */
	if (!strlen(p[0])) p[0] = NULL;
	if (!strlen(p[3])) p[3] = NULL;

	/* Hack -- attack affects you */
	if (details & (DESC_MELEE_ATTACK)) p[3] = "you";

	/* Hack -- attack affects a monster using it on itself */
	if ((details & (DESC_MONSTER_SELF | DESC_MONSTER_MALE | DESC_MONSTER_FEMALE)) ==(DESC_MONSTER_SELF | DESC_MONSTER_MALE | DESC_MONSTER_FEMALE))
	{
		if (p[0]) person = 5;
		else p[3] = "him or herself";
	}
	else if ((details & (DESC_MONSTER_SELF | DESC_MONSTER_MALE)) ==(DESC_MONSTER_SELF | DESC_MONSTER_MALE))
	{
		if (p[0]) person = 3;
		else p[3] = "himself";
	}
	else if ((details & (DESC_MONSTER_SELF | DESC_MONSTER_FEMALE)) ==(DESC_MONSTER_SELF | DESC_MONSTER_FEMALE))
	{
		if (p[0]) person = 4;
		else p[3] = "herself";
	}
	else if (details & (DESC_MONSTER_SELF))
	{
		if (p[0]) person = 6;
		else p[3] = "itself";
	}

	/* Hack -- attack affects a monster using it on itself*/
	if (details & (DESC_TRAP_VICTIM)) p[3] = "the victim";

	/* Hack -- attack affects a monster using it on itself*/
	if (details & (DESC_MONSTER_ATTACKER)) p[3] = "the attacking monster";

	/* Get effect info text */
	for (i = 0; i < 6; i++)
	{
		/* Something to write */
		if (strlen(effect_text + effect_ptr->info[i]))
		{
			/* Hack -- allow effect text to blank out method text */
			if ((effect_text + effect_ptr->info[i])[0] == '/')
			{
				p[i+1] = NULL;
			}
			else
			{
				p[i+1] = effect_text + effect_ptr->info[i];
			}
		}
	}

	/* Hack -- handle features */
	if (effect == GF_FEATURE)
	{
		char buf[80];
		const char *name;

		f_ptr = &f_info[f_info[feat].mimic];

		name = f_name + f_ptr->name;

		p[2] = buf;
		sprintf(buf,"%ss %s",name,f_ptr->flags1 & (FF1_MOVE) ? "under" : "around" );
	}

	/* Introduce the attack description */
	if (intro && strlen(intro))
	{
		/* Intro */
		text_out(intro);
	}

	/* Describe the method */
	if ((p[0]) && ((details & (DESC_SKIP_METHOD_MORE)) == 0))
	{
		/* Display all method details */
		if ((details & (DESC_SKIP_METHOD)) == 0)
		{
			if ((details & (DESC_SKIP_METHOD_INTRO)) == 0)
			{
				text_out_blow(p[0], person, (details & (DESC_MELEE_ATTACK)) != 0, num);
				need_space = TRUE;
			}
			
			if ((details & (DESC_MONSTER_SELF | DESC_TRAP_VICTIM)) == 0)
			{
				if (rng || arc || rad) text_out(" of ");
				if (rng) text_out (format( "range %d",rng));
				if (rng && arc) text_out(" and ");
				if (arc) text_out (format( "%d degrees",arc));
				if ((rng || arc) && rad) text_out(" and ");
				if (rad) text_out (format( "%s %d",(method_ptr->flags1 & (PROJECT_BEAM)) ? "width" : "radius", rad));
				
				if (rng || arc || rad) need_space = TRUE;
			}
		}

		/* Display all effect details */
		if ((details & (DESC_SKIP_EFFECT)) == 0)
		{
			if (need_space)
			{
				text_out(" ");
				
				need_space = FALSE;
			}
			
			if (p[1] || p[2] || p[3])
			{
				text_out("to");
				
				need_space = TRUE;
			}
	
			if (p[1])
			{
				text_out(" ");
				text_out_blow(p[1], person, TRUE, num);
			}
			if (p[2])
			{
				text_out(" ");
				text_out_blow(p[2], person, TRUE, num);
			}
			if (p[3])
			{
				text_out(" ");
				text_out_blow(p[3], person, TRUE, num);
			}
		}
	}
	else if ((details & (DESC_SKIP_METHOD_MORE)) == 0)
	{
		if (p[1])
		{
			text_out_blow(p[1], person, FALSE, num);
		}
		else text_out("affects");

		/* Display all effect details */
		if ((details & (DESC_SKIP_EFFECT)) == 0)
		{
			if (p[2])
			{
				text_out(" ");
				text_out_blow(p[2], person, FALSE, num);
			}
			if (p[3])
			{
				text_out(" ");
				text_out_blow(p[3], person, FALSE, num);
			}
		}
		if (rng) text_out (format( " of range %d",rng));

		if (rad) text_out (format( " %s radius %d",rng ? "and" : "of", rad));
		
		need_space = TRUE;
	}

	/* Display all effect details */
	if (((details & (DESC_SKIP_EFFECT)) == 0) && p[4])
	{
		if (need_space) text_out(" ");
		text_out_blow(p[4], person, FALSE, num);
		need_space = TRUE;
	}

	/* Display the damage */
	/* Roll out the damage */
	if ((details & (DESC_SKIP_DAMAGE_DETAIL)) == 0)
	{
		/* Hack -- feature */
		if (effect == GF_FEATURE)
		{
			f_ptr = &f_info[f_info[feat].mimic];

			if (!(f_ptr->flags1 & (FF1_MOVE)))
			{
				if (need_space) text_out(" ");
				text_out_blow(p[5], person, FALSE, num);
				text_out(" 4d8 ");
				text_out_blow(p[6], person, FALSE, num);
			}
		}
		/* Get the description */
		else if (damage && strlen(damage))
		{
			if (need_space) text_out(" ");
			text_out_blow(p[5], person, FALSE, num);
			text_out(format(" %s ", damage));
			text_out_blow(p[6], person, FALSE, num);
		}
	}
}


/*
 * Hack -- Get spell description for effects on target based on blow.
 */
static bool spell_desc_blows(const spell_type *s_ptr, const char *intro, int level, bool detail, int target, bool introduced)
{
	int m,n,r;

	bool anything = FALSE;
	bool initial_delay = FALSE;

	int last_method = 0;
	int last_effect = 0;

	int last_num = 0;
	
	int delay = 0;
	int region_id = 0;
	
	/* Get the region identity */
	if ((p_ptr->spell_trap) || (p_ptr->delay_spell))
	{
		/* Override region */
		region_id = p_ptr->spell_trap;

		/* Delay the effect */
		delay = p_ptr->delay_spell;

		/* Start with a delay */
		initial_delay = TRUE;
	}

	/* Hack -- create spell region */
	else
	{
		region_id = s_ptr->param;
	}	

	/* Count the number of "known" attacks */
	for (n = 0, m = 0; m < 4; m++)
	{
		/* Skip non-attacks */
		if (!s_ptr->blow[m].method) continue;

		/* Count known attacks */
		n++;
	}
	
	/* Hack - for introduction */
	if (introduced) n++;

	/* Examine (and count) the actual attacks */
	for (r = introduced ? 1 : 0, m = 0; m < 4; m++)
	{
		const spell_blow *blow_ptr = &s_ptr->blow[m];

		int num = 0;

		int method  = blow_ptr->method;
		int effect  = blow_ptr->effect;
		
		char buf[40];

		const char *current_intro;
		
		u16b details = 0;
		
		/* Add detail */
		if (!detail) details |= (DESC_SKIP_DAMAGE_DETAIL); 

		/* Skip non-attacks */
		if (!method) continue;

		/* Hack -- potions and food */
		switch(target)
		{
			case SPELL_TARGET_SELF:
				if ((method!= RBM_SPIT) && (method != RBM_BREATH) && (method!= RBM_VOMIT)) method = RBM_SELF;
				break;

			case SPELL_TARGET_AIMED:
				method = RBM_AIM;
				break;

			case SPELL_TARGET_COATED:
				method = RBM_TRAP;
				break;

			case SPELL_TARGET_EXPLODE:
				method = RBM_EXPLODE;
				break;
				
			case SPELL_TARGET_OTHER:
				details |= (DESC_MONSTER_SELF);
				break;
				
			case SPELL_TARGET_VICTIM:
				details |= (DESC_TRAP_VICTIM);
				break;

			case SPELL_TARGET_ATTACKER:
				details |= (DESC_MONSTER_ATTACKER);
				break;

		}

		/* Get number of attacks */
		num = scale_method(method_info[method].number, level);

		/* Hack -- fix number */
		if (!num) num = 1;

		/* Hack -- we assume identical method and effect is enough to display identical blows */
		if ((m < 4) && (s_ptr->blow[m+1].method == blow_ptr->method) && (s_ptr->blow[m+1].effect == blow_ptr->effect))
		{
			last_num += num;
			continue;
		}
		/* Reset number */
		else
		{
			num += last_num;
			last_num = 0;
		}

		/* Introduce the attack description */
		if (!introduced)
		{
			/* Intro */
			current_intro = intro;

			introduced = TRUE;
		}
		else if (!r)
		{
			current_intro = " and ";
		}
		else if (r < n-1)
		{
			current_intro = ", ";
		}
		else
		{
			current_intro = " and ";
		}

		/* Prepare damage string */
		spell_desc_damage(blow_ptr, target, level, buf, 40);

		/* Hack -- determine if we can skip lots of text */
		if (method == last_method)
		{
			effect_type *effect1_ptr = &effect_info[effect];
			effect_type *effect2_ptr = &effect_info[last_effect];

			int i;

			/* Assume we are the same */
			details |= (DESC_SKIP_METHOD_MORE);

			/* Check string comparison */
			for (i = 0; i < 3; i++)
			{
				if (strcmp(effect_text + effect1_ptr->info[i], effect_text + effect2_ptr->info[i])) details &= ~(DESC_SKIP_METHOD_MORE);
			}
		}

		/* We are describing a region */
		if ((s_ptr->type == SPELL_REGION) || (s_ptr->type == SPELL_SET_TRAP) || (p_ptr->spell_trap) || (p_ptr->delay_spell))
		{
			region_type region_type_body;			
			region_type *r_ptr = &region_type_body;
			
			region_info_type *ri_ptr = &region_info[region_id];
			
			/* Build a description of how long we last */
			char buf2[20];

			/* Wipe the structure */
			(void)WIPE(r_ptr, region_type);

			/* Start with how long we last */
			if ((s_ptr->lasts_dice) && (s_ptr->lasts_side) && (s_ptr->lasts_plus))
			{
				/* End */
				my_strcpy(buf2, format("%dd%d+%d times",s_ptr->lasts_dice,s_ptr->lasts_side,s_ptr->lasts_plus), sizeof(buf2));
			}
			else if ((s_ptr->lasts_dice) && (s_ptr->lasts_side) && (s_ptr->lasts_side == 1))
			{
				/* End */
				my_strcpy(buf2, format("%d time%s",s_ptr->lasts_dice, s_ptr->lasts_dice != 1 ? "s" : ""), sizeof(buf2));
			}
			else if ((s_ptr->lasts_dice) && (s_ptr->lasts_side))
			{
				/* End */
				my_strcpy(buf2, format("%dd%d time%s",s_ptr->lasts_dice,s_ptr->lasts_side), sizeof(buf2));
			}
			else if (s_ptr->lasts_plus)
			{
				/* End */
				my_strcpy(buf2, format("%d time%s",s_ptr->lasts_plus, s_ptr->lasts_plus != 1 ? "s" : ""), sizeof(buf2));
			}
			
			/* Initialise region values from parameters passed to this routine */
			r_ptr->type = region_id;
			r_ptr->level = level;
			r_ptr->effect = effect;
			r_ptr->lifespan = 10000;
			r_ptr->method = method;
			
			/* Hack - only used for features */
			r_ptr->damage = s_ptr->blow[m].d_plus;

			/* Initialise region values from region info type */
			r_ptr->flags1 = ri_ptr->flags1;
			r_ptr->delay_reset = ri_ptr->delay_reset;
			r_ptr->child_region = ri_ptr->child_region;

			/* Hack -- we delay subsequent regions from the same spell casting until the first has expired */
			r_ptr->delay = delay;

			/* Delaying casting of a spell */
			if (initial_delay)
			{
				/* Lasts one turn only */
				r_ptr->lifespan = 1;
			}
			/* Set the life span according to the duration */
			else if ((s_ptr->lasts_dice) && (s_ptr->lasts_side))
			{
				r_ptr->lifespan = s_ptr->lasts_dice * (s_ptr->lasts_side + (s_ptr->lasts_side > 1 ? 1 : 0)) / (s_ptr->lasts_side > 1 ? 2 : 1) + s_ptr->lasts_plus;
			}
			else if (s_ptr->lasts_plus)
			{
				r_ptr->lifespan = s_ptr->lasts_plus;
			}
			
			/* Increase delay - we predict effects of acceleration/deceleration */
			if (r_ptr->flags1 & (RE1_ACCELERATE | RE1_DECELERATE))
			{
				int i;
				int delay_current = r_ptr->delay_reset;

				for (i = 0; i < r_ptr->lifespan; i++)
				{
					delay += delay_current;

					if ((r_ptr->flags1 & (RE1_ACCELERATE)) && (!(r_ptr->flags1 & (RE1_DECELERATE)) || (i < r_ptr->lifespan / 2)))
					{
						if (delay_current < 3) delay_current -= 1;
						delay_current -= delay_current / 3;
						if (delay_current < 1) delay_current = 1;
					}

					/* Decelerating */
					if ((r_ptr->flags1 & (RE1_DECELERATE)) && (!(r_ptr->flags1 & (RE1_ACCELERATE)) || (i > r_ptr->lifespan / 2)))
					{
						delay_current += delay_current / 3;
						if (delay_current < 3) delay_current += 1;
					}
				}
			}
			/* Normal delay - this is the total lifespan of the effect for subsequent effects */
			else
			{
				delay += r_ptr->lifespan * r_ptr->delay_reset;
			}

			/* Describe region creation */
			if (method != last_method)
			{
				/* Describe region basic */
				describe_region_basic(r_ptr, r ? ".  This also creates" : format("%screates", intro));
			}
			
			/* Describe region attacks */
			describe_region_attacks(r_ptr, method != last_method ? ".  It" : ".  It also", buf, buf2);

		}
		/* We describe the blow */
		else
		{
			/* Describe blow */
			describe_blow(method, effect, level, blow_ptr->d_plus, current_intro, buf, details | (method == last_method ? (DESC_SKIP_METHOD) : 0), num);
		}
	
		/* Count blows */
		r++;

		/* Something shown */
		anything = TRUE;

		/* Record last method */
		last_method = method;

		/* Record last effect */
		last_effect = effect;
	}

	return (anything);
}



/*
 * Hack -- Get spell description.
 */
bool spell_desc(spell_type *s_ptr, const cptr intro, int level, bool detail, int target)
{
	bool anything = FALSE;

	anything |= spell_desc_flags(s_ptr, intro, level, detail, target, anything);
	anything |= spell_desc_blows(s_ptr, intro, level, detail, target, anything);

	return (anything);
}


/*
 * Extra information on a spell		-DRS-
 *
 * We can use up to 14 characters of the buffer 'p'
 *
 * The strings in this function were extracted from the code in the
 * functions "do_cmd_cast()" and "do_cmd_pray()" and may be dated.
 *
 * Note they do not take account of modifiers to player level.
 */
void spell_info(char *p, int p_s, int spell, int level)
{
	spell_type *s_ptr = &s_info[spell];

	int m;

	/* Default */
	my_strcpy(p, "", p_s);

	/* Roll out the duration */
	if ((s_ptr->lasts_dice) && (s_ptr->lasts_side) && (s_ptr->lasts_plus))
	{
		/* End */
		my_strcpy(p,format(" dur %dd%d+%d",s_ptr->lasts_dice,s_ptr->lasts_side,s_ptr->lasts_plus), p_s);
	}
	else if ((s_ptr->lasts_dice) && (s_ptr->lasts_side) && (s_ptr->lasts_side == 1))
	{
		/* End */
		my_strcpy(p,format(" dur %d",s_ptr->lasts_dice), p_s);
	}
	else if ((s_ptr->lasts_dice) && (s_ptr->lasts_side))
	{
		/* End */
		my_strcpy(p,format(" dur %dd%d",s_ptr->lasts_dice,s_ptr->lasts_side), p_s);
	}
	else if (s_ptr->lasts_plus)
	{
		/* End */
		my_strcpy(p,format(" dur %d",s_ptr->lasts_plus), p_s);
	}

	/* Examine (and count) the actual attacks */
	for (m = 0; m < 4; m++)
	{
		spell_blow *blow_ptr = &s_ptr->blow[m];
		int method = blow_ptr->method;
		int effect = blow_ptr->effect;
		char buf[40];
		const char *q = effect_text + effect_info[effect].info[6];

		/* Skip non-attacks */
		if (!method) continue;

		/* Hack -- heroism/berserk strength */
		if (((s_ptr->lasts_dice) || (s_ptr->lasts_side) || (s_ptr->lasts_plus)) && (effect == GF_HEAL_PERC)) continue;

		/* Default */
		if (!strlen(q)) q = "dam";

		/* Hack -- feature */
		if (effect == GF_FEATURE)
		{
			feature_type *f_ptr = &f_info[f_info[blow_ptr->d_plus].mimic];

			if (!(f_ptr->flags1 & (FF1_MOVE)))
			{
				my_strcpy(p,format(" 4d8 %s", q), p_s);
			}
		}
		/* Get the description */
		else if (spell_desc_damage(blow_ptr, SPELL_TARGET_NORMAL, level, buf, 40))
		{
			my_strcpy(p, format(" %s %s", buf, q), p_s);
		}
	}

}



/*
 * Pair together a constant flag with a textual description.
 *
 * Note that it sometimes more efficient to actually make an array
 * of textual names, where entry 'N' is assumed to be paired with
 * the flag whose value is "1L << N", but that requires hard-coding.
 */
typedef struct o_flag_desc
{
	u32b flag;
	cptr desc;
} o_flag_desc;


/*
 * This function does most of the actual "analysis". Given a set of bit flags
 * (which will be from one of the flags fields from the object in question),
 * a "flag description structure", a "description list", and the number of
 * elements in the "flag description structure", this function sets the
 * "description list" members to the appropriate descriptions contained in
 * the "flag description structure".
 *
 * The possibly updated description pointer is returned.
 */

static cptr *spoiler_flag_aux(const u32b art_flags, const o_flag_desc *flag_x_ptr,
			      cptr *desc_x_ptr, const int n_elmnts)
{
	int i;

	for (i = 0; i < n_elmnts; ++i)
	{
		if (art_flags & flag_x_ptr[i].flag)
		{
			*desc_x_ptr++ = flag_x_ptr[i].desc;
		}
	}

	return desc_x_ptr;
}



static cptr *spoiler_flag(u32b f1, u32b f2, u32b f3, u32b f4, int bonus, cptr *desc_x_ptr)
{
	int i, j;
	
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 32; j++)
		{
			if (item_flag_bonus[i][j].type == bonus)
			{
				if ((i == 0) && ((f1 & (1L << j)) != 0)) *desc_x_ptr++ = item_flag_bonus[i][j].desc;
				else if ((i == 1) && ((f1 & (1L << j)) != 0)) *desc_x_ptr++ = item_flag_bonus[i][j].desc;
				else if ((i == 2) && ((f1 & (1L << j)) != 0)) *desc_x_ptr++ = item_flag_bonus[i][j].desc;
				else if ((i == 3) && ((f1 & (1L << j)) != 0)) *desc_x_ptr++ = item_flag_bonus[i][j].desc;
			}
		}
	}
	
	return desc_x_ptr;
}



/*
 * Hack -- Display the "name" and "attr/chars" of a monster race
 */
static void obj_top(const object_type *o_ptr)
{
	char o_name[80];

	object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 1);

	/* Clear the top line */
	Term_erase(0, 0, 255);

	/* Reset the cursor */
	Term_gotoxy(0, 0);

	/* Dump the name */
	Term_addstr(-1, TERM_L_BLUE, o_name);
}

/*
 * Display an object at the top of the screen
 */
void screen_object(object_type *o_ptr)
{
	/* Flush messages */
	message_flush();

	/* Set text_out hook */
	text_out_hook = text_out_to_screen;

	/* Begin recall */
	Term_gotoxy(0, 1);

	/* Actually display the item */
	if (o_ptr->ident & (IDENT_MENTAL)) list_object(o_ptr, OBJECT_FLAGS_FULL);
	else list_object(o_ptr, OBJECT_FLAGS_KNOWN);

	/* Display monster attributes */
	if ((o_ptr->name3) && ((o_ptr->tval != TV_HOLD) || (object_named_p(o_ptr)))) screen_roff(o_ptr->name3,&l_list[o_ptr->name3]);

	/* Display item name */
	obj_top(o_ptr);
}


/*
 * Describe an object which is part of the character. Deprecated.
 */
void describe_self_object(object_type *o_ptr, int slot)
{
	/* Display item */
	text_out("This is a part of your current shape.  ");
	text_out("You cannot take it off, but it will be removed if you change shape.  ");

	/* Display spell item full flags */
        if (o_ptr->tval == TV_SPELL)
	{
		list_object(o_ptr, OBJECT_FLAGS_FULL);
	}
	/* Hack -- body parts and other objects */
	else
	{
		bool attack = FALSE;
		bool unarmed = FALSE;
		bool charging = FALSE;

		switch (slot)
		{
			case INVEN_WIELD:
			{
				if (o_ptr->tval == TV_SWORD || o_ptr->tval == TV_POLEARM || o_ptr->tval == TV_HAFTED ||
					o_ptr->tval == TV_DIGGING) attack = TRUE;
				else
					text_out("You must fight using unarmed combat in this shape.  ");
				break;
			}
			case INVEN_HANDS:
			{
				attack = TRUE;
				unarmed = TRUE;
				break;
			}
			case INVEN_FEET:
			{
				charging = TRUE;
				unarmed = TRUE;
				break;
			}
		}

		if ((attack) || (charging))
		{
			if (attack) text_out("When attacking");
			else if (charging) text_out("When charging");
			if (attack && charging) text_out(" or charging");
			else if (attack) text_out(" unless charging");
			if (unarmed) text_out(" unarmed");
			text_out(format(", it does %dd%d", object_aval(o_ptr, ABILITY_DAMAGE_DICE), object_aval(o_ptr, ABILITY_DAMAGE_SIDES)));
			if (object_aval(o_ptr, ABILITY_TO_DAM) > 0) text_out(format("+%d", object_aval(o_ptr, ABILITY_TO_DAM)));
			else if (object_aval(o_ptr, ABILITY_TO_DAM) < 0) text_out(format("%d", object_aval(o_ptr, ABILITY_TO_DAM)));
			text_out(" ");
			text_out((o_ptr->tval == TV_SPELL) ? "magical" : ((o_ptr->tval == TV_SWORD || o_ptr->tval == TV_POLEARM ||
				o_ptr->tval == TV_ARROW || o_ptr->tval == TV_BOLT)
				? "edged" : "blunt"));
			text_out(" damage.  ");
		}
	}

	/* Display item name */
	obj_top(o_ptr);
}


/*
 * Display an object at the top of the screen that is part of the players shape. Deprecated.
 */
void screen_self_object(object_type *o_ptr, int slot)
{
	/* Flush messages */
	message_flush();

	/* Set text_out hook */
	text_out_hook = text_out_to_screen;

	/* Begin recall */
	Term_gotoxy(0, 1);

	/* Describe the object */
	describe_self_object(o_ptr, slot);
}


/*
 * Describes a player race / shape
 */
void describe_shape(int shape, bool random)
{
	u32b f0[ABILITY_ARRAY_SIZE];
	u32b f1 = 0L;
	u32b f2 = 0L;
	u32b f3 = 0L;
	u32b f4 = 0L;
	
	int i;
	
	object_type object_type_body;
	object_type *o_ptr = &object_type_body;
	
	player_race *shape_ptr = &p_info[shape];
	
	object_prep(o_ptr, lookup_kind(TV_RACE, shape));
	
	object_flags(o_ptr, f0, &f1, &f2, &f3, &f4);

	/* TODO: Describe stat modifiers */
	
	/* Hack -- shape flags */
	if ((!random) && (f1 || f2 || f3 || f4))
	{
		list_object_flags(NULL, f0, f1, f2, f3, f4, o_ptr->pval, LIST_FLAGS_CAN);
	}

	/* Hack -- shape flags which prevent actions */
	if ((!random) && (shape_ptr->cancel_flags1 || shape_ptr->cancel_flags2 || shape_ptr->cancel_flags3 || shape_ptr->cancel_flags4))
	{
		list_object_flags(NULL, shape_ptr->cancel_flags0, shape_ptr->cancel_flags1, shape_ptr->cancel_flags2, shape_ptr->cancel_flags3, shape_ptr->cancel_flags4, o_ptr->pval, LIST_FLAGS_PREVENT);
	}

	/* Show powers */
	if (f3 & (TR3_ACTIVATE))
	{
		s16b book[26];
		int num = 0;
		
		/* Fill the book with spells */
		fill_book(o_ptr,book,&num);

		/* Header */
		text_out("\nYou may activate yourself for the following powers:\n");
		
		/* Dump the spells */
		for (i = 0; i < num; i++)
		{
			bool dummy = TRUE;
			char info[80];

			/* Get the spell info */
			spell_type* s_ptr = &s_info[book[i]];

			/* Prepare the spell */
			process_spell_prepare(book[i], 25, &dummy, FALSE, FALSE);

			/* Get extra info */
			spell_info(info, sizeof(info), book[i], p_ptr->lev);

			/* Dump the spell --(-- */
			text_out(format("  %c) %-30s %s\n",
				I2A(i), s_name + s_ptr->name,
				info));

			/* Paranoia - clear boost */
			p_ptr->boost_spell_power = 0;
		}
	}
}



/*
 * This function displays lists of properties
 */
static bool outlist_pval(cptr header, const cptr *list, byte attr, int pval)
{
	/* Ignore an empty list */
	if (*list == NULL) return (FALSE);

	/* Create header (if one was given) */
	if (header && (header[0]))
	{
		text_out_c(attr, header);
		text_out_c(attr, " ");
	}

	/* Now begin the tedious task */
	while (1)
	{
		/* Print the current item */
		text_out_c(attr, *list);

		/*
		 * If there is an item following this one, pad with separator and a space
		 */
		if (list[1])
		{
			/* If there are two items, use a comma. */
			if (list[2]) text_out_c(attr, ", ");
			/* Otherwise, use "and" */
			else text_out_c(attr, " and ");
		}

		/* Advance, with break */
		if (!*++list) break;
	}

	if (pval)
	{
		text_out_c(attr, format(" by %d", ABS(pval)));
	}

	/* End the current list */
	text_out_c(attr, ".  ");

	/* Something was printed */
	return (TRUE);
}


/*
 * This function displays a list of non-pval dependent properties
 */
static bool outlist(cptr header, const cptr *list, byte attr)
{
	return (outlist_pval(header, list, attr, 0));
}


/*
 * Create a spoiler file entry for an artifact.
 * We use this to list the flags.
 */
bool list_object_flags(const s16b ability[ABILITY_MAX], const u32b f0[ABILITY_ARRAY_SIZE], u32b f1, u32b f2, u32b f3, u32b f4, int pval, int mode)
{
#if 0
	const u32b all_stats = (0x0300003FL);
#endif
	
	const u32b all_sustains = (TR1_SUST_STR | TR1_SUST_INT | TR1_SUST_WIS |
							   TR1_SUST_DEX | TR1_SUST_CON | TR1_SUST_CHR |
								TR1_SUST_SIZ | TR1_SUST_AGI);

	bool anything = FALSE; /* Printed anything at all */

	const char *list[ABILITY_MAX];
	const char **list_ptr;
	
	int max_aval;
	int min_aval;
	int i, j;

	/* Abilities - multiplier or divisor */
	/* Always do one pass. The first pass is just to find the maximum aval */
	if (ability) for (i = 0, max_aval = 0, min_aval = 0; ((i == 0) && (max_aval == 0)) || (i >= min_aval); i--)
	{
		/* Restart the list */
		list_ptr = list;

		/* Check each ability */
		for (j = 0; j < ABILITY_MAX; j++)
		{
			/* Skip if not known */
			if ((f0) && (((f0[j/32]) & (1L << (j % 32))) == 0)) continue;
			
			/* Skip unless a multiplier/divisor */
			if ((ability_bonus[j].type != BONUS_MULTIPLIER)
				&& (ability_bonus[j].type >= BONUS_WEAPON_MULTIPLIER)) continue;
			
			/* Skip weapons which do x1 multiplier or divisor */
			if ((i == 1) || (i == -1)) continue;
			
			/* We have done the first pass */
			if (i)
			{
				if ((ability) && (i != ability[j])) continue;
			}
			/* We are still finding the range of abilities */
			else
			{
				if ((ability) && (ability[j] > 0) && (max_aval < ability[j])) max_aval = ability[j];

				if ((ability) && (ability[j] < 0) && (min_aval > ability[j])) min_aval = ability[j];
			}
			
			/* Get the maximum aval for subsequent passes */
			if ((ability) && (!i)&& (max_aval < ability[j])) max_aval = ability[j];				 

			/* Skip if we are looking at particular values */
			if ((ability) && (i) && (i != ability[j])) continue;
			
			/* Get the list */
			*list_ptr++ = ability_bonus[j].desc;
		}
		
		/* Terminate the description list */
		*list_ptr = NULL;
		
		/* Collect */
		switch (mode)
		{
			case LIST_FLAGS_CAN:
				if (i) anything |= outlist(format("It does x%f damage", i > 0 ? (float)i : (float)1 / (float)i), list, TERM_WHITE);
				break;
			case LIST_FLAGS_MAY:
				if (!i) anything |= outlist("It may do extra damage", list, TERM_L_WHITE);
				break;
			case LIST_FLAGS_NOT:
				if (!i) anything |= outlist("It does not do extra damage", list, TERM_SLATE);
				break;
			case LIST_FLAGS_PREVENT:
				if (!i) anything |= outlist("It prevents you wielding weapons which do extra damage", list, TERM_L_PURPLE);
				break;
		}
		
		/* We are done if not listing avals */
		if (mode != LIST_FLAGS_CAN) break;
		
		/* We now have an maximum aval */
		if (i == 0) i = max_aval + 1;
	}	

	/* Abilities - addition or subtraction */
	/* Always do one pass. The first pass is just to find the maximum aval */
	if (ability) for (i = 0, max_aval = 0, min_aval = 0; ((i == 0) && (max_aval == 0)) || (i >= min_aval); i--)
	{
		/* Restart the list */
		list_ptr = list;

		/* Check each ability */
		for (j = 0; j < ABILITY_MAX; j++)
		{
			/* Skip if not known */
			if ((f0) && (((f0[j/32]) & (1L << (j % 32))) == 0)) continue;
			
			/* Skip if a multiplier/divisor */
			if ((ability_bonus[j].type == BONUS_MULTIPLIER)
				|| (ability_bonus[j].type >= BONUS_WEAPON_MULTIPLIER)) continue;
			
			/* We have done the first pass */
			if (i)
			{
				if ((ability) && (i != ability[j])) continue;
			}
			/* We are still finding the range of abilities */
			else
			{
				if ((ability) && (ability[j] > 0) && (max_aval < ability[j])) max_aval = ability[j];

				if ((ability) && (ability[j] < 0) && (min_aval > ability[j])) min_aval = ability[j];
			}
			
			/* Get the maximum aval for subsequent passes */
			if ((ability) && (!i)&& (max_aval < ability[j])) max_aval = ability[j];				 

			/* Skip if we are looking at particular values */
			if ((ability) && (i) && (i != ability[j])) continue;
			
			/* Get the list */
			*list_ptr++ = ability_bonus[j].desc;
		}
		
		/* Terminate the description list */
		*list_ptr = NULL;
		
		/* Collect */
		switch (mode)
		{
			case LIST_FLAGS_CAN:
				if (i) anything |= outlist(i > 0 ? format("It increases (+%d) your", i) :
					(i <  0 ? format("It decreases (%d) your", i) : "It modifies your"), list, TERM_WHITE);
				break;
			case LIST_FLAGS_MAY:
				if (!i) anything |= outlist("It may modify your", list, TERM_L_WHITE);
				break;
			case LIST_FLAGS_NOT:
				if (!i) anything |= outlist("It does not modify your", list, TERM_SLATE);
				break;
			case LIST_FLAGS_PREVENT:
				if (!i) anything |= outlist("It prevents you modifying your", list, TERM_L_PURPLE);
				break;
		}

		/* Restart the list */
		list_ptr = list;

		/* We are done if not listing avals */
		if (mode != LIST_FLAGS_CAN) break;
		
		/* We now have an maximum aval */
		if (i == 0) i = max_aval + 1;
	}	

	/* Slays/brands prevent wielding */
	if (mode == LIST_FLAGS_CAN)
	{
		/* Restart the list */
		list_ptr = list;

		/* Check each ability */
		if (ability) for (j = 0; j < ABILITY_MAX; j++)
		{
			/* Skip if not known */
			if ((f0) && (((f0[j/32]) & (1L << (j % 32))) == 0)) continue;
			
			/* Skip unless a slay */
			if (ability_bonus[j].type != BONUS_SLAY) continue;
			
			/* Skip unless it starts with "against" */
			if (!prefix(ability_bonus[j].desc, "against ")) continue;
			
			/* Get the list - skip 'against' */
			*list_ptr++ = ability_bonus[j].desc + 8;
		}
		
		/* Terminate the description list */
		*list_ptr = NULL;
		
		/* Collect */
		anything |= outlist("It cannot be wielded by", list, TERM_WHITE);
		
		/* Restart the list */
		list_ptr = list;
		
		/* Check each ability */
		if (ability) for (j = 0; j < ABILITY_MAX; j++)
		{
			/* Skip if not known */
			if ((f0) && (((f0[j/32]) & (1L << (j % 32))) == 0)) continue;
			
			/* Skip unless a slay/brand - we have to do this because brand light acts like a slay */
			if (ability_bonus[j].type < BONUS_SLAY) continue;
			
			/* Skip unless it starts with "from" */
			if (!prefix(ability_bonus[j].desc, "from ")) continue;
			
			/* Get the list - skip 'from' */
			*list_ptr++ = ability_bonus[j].desc + 5;
		}
		
		/* Terminate the description list */
		*list_ptr = NULL;
		
		/* Collect */
		anything |= outlist("It cannot be wielded if you are vulnerable to", list, TERM_WHITE);
	}	
	
	/* Vampirism */
	if (TRUE)
	{
		/* Restart the list */
		list_ptr = list;

		/* Get the vampiric abilities */
		list_ptr = spoiler_flag(f1, f2, f3, f4, BONUS_VAMP, list_ptr);

		/* Terminate the description list */
		*list_ptr = NULL;

		switch (mode)
		{
			case LIST_FLAGS_CAN:
				anything |= outlist("It feeds you stolen", list, TERM_WHITE);
				break;
			case LIST_FLAGS_MAY:
				anything |= outlist("It may feed you stolen", list, TERM_L_WHITE);
				break;
			case LIST_FLAGS_NOT:
				anything |= outlist("It does not feed you stolen", list, TERM_SLATE);
				break;
			case LIST_FLAGS_PREVENT:
				anything |= outlist("It prevents you feeding on stolen", list, TERM_L_PURPLE);
				break;
		}
		
		/* Restart the list */
		list_ptr = list;
	}

	/* Sustains */
	if (TRUE)
	{
		/* Restart the list */
		list_ptr = list;

		/* Simplify things if an item sustains all stats */
		if ((f1 & all_sustains) == all_sustains)
		{
			switch (mode)
			{
				case LIST_FLAGS_CAN:
				case LIST_FLAGS_MAY:
					*list_ptr++ = "all stats";
					break;
				case LIST_FLAGS_NOT:
					*list_ptr++ = "any stats";
					break;
			}
		}

		/* Should we bother? */
		else if (f1 & all_sustains)
		{
			list_ptr = spoiler_flag(f1, f2, f3, f4, BONUS_SUSTAIN, list_ptr);
		}

		/* Terminate the description list */
		*list_ptr = NULL;

		switch (mode)
		{
			case LIST_FLAGS_CAN:
				anything |= outlist("It sustains", list, TERM_WHITE);
				break;
			case LIST_FLAGS_MAY:
				anything |= outlist("It may sustain", list, TERM_L_WHITE);
				break;
			case LIST_FLAGS_NOT:
				anything |= outlist("It does not sustain", list, TERM_SLATE);
				break;
			case LIST_FLAGS_PREVENT:
				anything |= outlist("It prevents you sustaining", list, TERM_L_PURPLE);
				break;
		}
	}

	/* Immunity flags */
	if (TRUE)
	{
		list_ptr = list;

		/* Get the immunities */
		list_ptr = spoiler_flag(f1, f2, f3, f4, BONUS_IMMUNE, list_ptr);

		/* Terminate the description list */
		*list_ptr = NULL;

		switch (mode)
		{
			case LIST_FLAGS_CAN:
				anything |= outlist("It provides immunity to", list, TERM_WHITE);
				break;
			case LIST_FLAGS_MAY:
				anything |= outlist("It may provide immunity to", list, TERM_L_WHITE);
				break;
			case LIST_FLAGS_NOT:
				anything |= outlist("It does not provide immunity to", list, TERM_SLATE);
				break;
			case LIST_FLAGS_PREVENT:
				anything |= outlist("It prevents you becoming immune to", list, TERM_L_PURPLE);
				break;
		}
	}

	/* Protects flags */
	if (TRUE)
	{
		list_ptr = list;

		/* Note we skip the sustains as we have described these previously */
		for (i = 8; i < 32; i++)
		{
			/* Doesn't provide protection */
			if ((f1 & (1L << i)) == 0) continue;
			
			*list_ptr++ = timed_effects[i].short_description;
		}

		/* Get the protections */
		list_ptr = spoiler_flag(f1, f2, f3, f4, BONUS_PROTECT, list_ptr);

		/* Terminate the description list */
		*list_ptr = NULL;

		switch (mode)
		{
			case LIST_FLAGS_CAN:
				anything |= outlist("It protects you from", list, TERM_WHITE);
				break;
			case LIST_FLAGS_MAY:
				anything |= outlist("It may protect you from", list, TERM_L_WHITE);
				break;
			case LIST_FLAGS_NOT:
				anything |= outlist("It does not protect you from", list, TERM_SLATE);
				break;
			case LIST_FLAGS_PREVENT:
				anything |= outlist("It prevents you being protected from", list, TERM_L_PURPLE);
				break;
		}
	}

	/* Vulnerability flags */
	if (TRUE)
	{
		list_ptr = list;

		/* Get the vulnerabilities */
		list_ptr = spoiler_flag(f1, f2, f3, f4, BONUS_VULNERABLE, list_ptr);

		/* Terminate the description list */
		*list_ptr = NULL;

		switch (mode)
		{
			case LIST_FLAGS_CAN:
				anything |= outlist("It makes you vulnerable to", list, TERM_WHITE);
				break;
			case LIST_FLAGS_MAY:
				anything |= outlist("It may make you vulnerable to", list, TERM_L_WHITE);
				break;
			case LIST_FLAGS_NOT:
				anything |= outlist("It does not make you vulnerable to", list, TERM_SLATE);
				break;
			case LIST_FLAGS_PREVENT:
				anything |= outlist("It prevents you becoming vulnerable to", list, TERM_L_PURPLE);
				break;
		}
		
		/* Hack - note prevention */
		if (mode == LIST_FLAGS_CAN)
		{
			u32b pf0[4];
			
			for (i = 0; i < 4; i++) pf0[i] = 0L;
			
			/* Note opposites */
			if (f4 & (TR4_HURT_LITE)) pf0[3] |= (1L << (ABILITY_BRAND_LITE % 32));
			if (f4 & (TR4_HURT_WATER)) pf0[3] |= (1L << (ABILITY_BRAND_LITE % 32));
			if (f4 & (TR4_HURT_POIS)) pf0[0] |= (1L << (ABILITY_BRAND_POIS % 32));
			if (f4 & (TR4_HURT_ACID)) pf0[0] |= (1L << (ABILITY_BRAND_ACID % 32));
			if (f4 & (TR4_HURT_ELEC)) pf0[0] |= (1L << (ABILITY_BRAND_ELEC % 32));
			if (f4 & (TR4_HURT_FIRE)) pf0[0] |= (1L << (ABILITY_BRAND_FIRE % 32));
			if (f4 & (TR4_HURT_COLD)) pf0[0] |= (1L << (ABILITY_BRAND_COLD % 32));
			
			list_object_flags(NULL, pf0, 0L, 0L, 0L, 0L, 0, LIST_FLAGS_PREVENT);
		}
	}

	/* Miscellenious Abilities */
	if (TRUE)
	{
		list_ptr = list;

		/* Get the miscellaneous abilities */
		list_ptr = spoiler_flag(f1, f2, f3, f4, BONUS_MISC, list_ptr);
		list_ptr = spoiler_flag(f1, f2, f3, f4, BONUS_SENSE, list_ptr);

		/* Terminate the description list */
		*list_ptr = NULL;

		switch (mode)
		{
			case LIST_FLAGS_CAN:
				anything |= outlist("It allows you to", list, TERM_WHITE);
				break;
			case LIST_FLAGS_MAY:
				anything |= outlist("It may allow you to", list, TERM_L_WHITE);
				break;
			case LIST_FLAGS_NOT:
				anything |= outlist("It doesn't allow you to", list, TERM_SLATE);
				break;
			case LIST_FLAGS_PREVENT:
				anything |= outlist("It prevents you", list, TERM_L_PURPLE);
				break;
		}
	}

	/* Equipment only abilities */
	if (TRUE)
	{
		list_ptr = list;

		/* Get the miscellaneous abilities */
		list_ptr = spoiler_flag(f1, f2, f3, f4, BONUS_EQUIP, list_ptr);

		/* Terminate the description list */
		*list_ptr = NULL;

		switch (mode)
		{
			case LIST_FLAGS_CAN:
				anything |= outlist("It is", list, TERM_WHITE);
				break;
			case LIST_FLAGS_MAY:
				anything |= outlist("It might be", list, TERM_L_WHITE);
				break;
			case LIST_FLAGS_NOT:
				anything |= outlist("It is not", list, TERM_SLATE);
				break;
			case LIST_FLAGS_PREVENT:
				anything |= outlist("It prevents being", list, TERM_L_PURPLE);
				break;
		}
	}

	/* Extra sensory abilities */
	if (TRUE)
	{
		list_ptr = list;

		/* Extra sensory abilities */
		list_ptr = spoiler_flag(f1, f2, f3, f4, BONUS_ESP, list_ptr);

		/* Terminate the description list */
		*list_ptr = NULL;

		switch (mode)
		{
			case LIST_FLAGS_CAN:
				anything |= outlist("It senses", list, TERM_WHITE);
				break;
			case LIST_FLAGS_MAY:
				anything |= outlist("It may sense", list, TERM_L_WHITE);
				break;
			case LIST_FLAGS_NOT:
				anything |= outlist("It does not sense", list, TERM_SLATE);
				break;
			case LIST_FLAGS_PREVENT:
				anything |= outlist("It prevents you sensing", list, TERM_L_PURPLE);
				break;
		}
	}

	/* Language Abilities */
	if (TRUE)
	{
		list_ptr = list;

		/* Language abilities */
		list_ptr = spoiler_flag(f1, f2, f3, f4, BONUS_SPEAK, list_ptr);

		/* Terminate the description list */
		*list_ptr = NULL;

		switch (mode)
		{
			case LIST_FLAGS_CAN:
				anything |= outlist("It lets you speak with", list, TERM_WHITE);
				break;
			case LIST_FLAGS_MAY:
				anything |= outlist("It may let you speak with", list, TERM_L_WHITE);
				break;
			case LIST_FLAGS_NOT:
				anything |= outlist("It does not let you speak with", list, TERM_SLATE);
				break;
			case LIST_FLAGS_PREVENT:
				anything |= outlist("It prevents you speaking with", list, TERM_L_PURPLE);
				break;
		}
	}


	/* Proof Abilities */
	if (TRUE)
	{
		list_ptr = list;

		/* Proof abilities */
		list_ptr = spoiler_flag(f1, f2, f3, f4, BONUS_PROOF, list_ptr);

		/* Terminate the description list */
		*list_ptr = NULL;

		switch (mode)
		{
			case LIST_FLAGS_CAN:
				anything |= outlist("It is impervious to", list, TERM_WHITE);
				break;
			case LIST_FLAGS_MAY:
				anything |= outlist("It might be impervious to", list, TERM_L_WHITE);
				break;
			case LIST_FLAGS_NOT:
				anything |= outlist("It can be lost through", list, TERM_SLATE);
				break;
			case LIST_FLAGS_PREVENT:
				anything |= outlist("It prevents you makint it impervious to", list, TERM_L_PURPLE);
				break;
		}
	}

	/* Racial marking */
	if (TRUE)
	{
		list_ptr = list;

		/* Racial marking */
		list_ptr = spoiler_flag(f1, f2, f3, f4, BONUS_MARK, list_ptr);

		/* Terminate the description list */
		*list_ptr = NULL;

		switch (mode)
		{
			case LIST_FLAGS_CAN:
				anything |= outlist("It marks you as", list, TERM_WHITE);
				break;
			case LIST_FLAGS_MAY:
				anything |= outlist("It may mark you as", list, TERM_L_WHITE);
				break;
			case LIST_FLAGS_NOT:
				anything |= outlist("It does not mark you as", list, TERM_SLATE);
				break;
			case LIST_FLAGS_PREVENT:
				anything |= outlist("It prevents you being marked as", list, TERM_L_PURPLE);
				break;
		}
		
		/* Hack - note prevention */
		if (mode == LIST_FLAGS_CAN)
		{
			u32b pf0[4];
			
			for (i = 0; i < 4; i++) pf0[i] = 0L;
			
			/* Note opposites */
			if (f4 & (TR4_ANIMAL)) pf0[0] |= (1L << (ABILITY_SLAY_ANIMAL % 32));
			if (f4 & (TR4_EVIL)) pf0[0] |= (1L << (ABILITY_BRAND_HOLY % 32));
			if (f4 & (TR4_UNDEAD)) pf0[0] |= (1L << (ABILITY_SLAY_UNDEAD % 32));
			if (f4 & (TR4_DEMON)) pf0[0] |= (1L << (ABILITY_SLAY_DEMON % 32));
			if (f4 & (TR4_ORC)) pf0[0] |= (1L << (ABILITY_SLAY_ORC % 32));
			if (f4 & (TR4_TROLL)) pf0[0] |= (1L << (ABILITY_SLAY_TROLL % 32));
			if (f4 & (TR4_GIANT)) pf0[0] |= (1L << (ABILITY_SLAY_GIANT % 32));
			if (f4 & (TR4_DRAGON)) pf0[0] |= (1L << (ABILITY_SLAY_DRAGON % 32));
			if (f4 & (TR4_ELF)) pf0[3] |= (1L << (ABILITY_SLAY_ELF % 32));
			if (f4 & (TR4_MAN)) pf0[3] |= (1L << (ABILITY_SLAY_MAN % 32));
			if (f4 & (TR4_DWARF)) pf0[3] |= (1L << (ABILITY_SLAY_DWARF % 32));
			
			list_object_flags(NULL, pf0, 0L, 0L, 0L, 0L, 0, LIST_FLAGS_PREVENT);
		}

	}

	/* Negative effects */
	if (TRUE)
	{
		list_ptr = list;

		/* Negative effects */
		list_ptr = spoiler_flag(f1, f2, f3, f4, BONUS_BURDEN, list_ptr);
		list_ptr = spoiler_flag(f1, f2, f3, f4, BONUS_PASSIVE, list_ptr);
		list_ptr = spoiler_flag(f1, f2, f3, f4, BONUS_EQUIP, list_ptr);

		/* Terminate the description list */
		*list_ptr = NULL;

		switch (mode)
		{
			case LIST_FLAGS_CAN:
				anything |= outlist("It burdens you with", list, TERM_ORANGE);
				break;
			case LIST_FLAGS_MAY:
				anything |= outlist("It may burden you with", list, TERM_YELLOW);
				break;
			case LIST_FLAGS_NOT:
				anything |= outlist("It does not burden you with", list, TERM_SLATE);
				break;
			case LIST_FLAGS_PREVENT:
				anything |= outlist("It prevents you being burdened with", list, TERM_L_PURPLE);
				break;
		}
	}

	return (anything);

}



/*
 * Ways item can be damaged.
 */
static const o_flag_desc hurt_flags5_desc[] =
{
		{ TR5_KILL_ACID,  "acid" },
		{ TR5_KILL_ELEC,  "electricity" },
		{ TR5_KILL_FIRE,  "fire" },
		{ TR5_KILL_COLD,  "cold" },
		{ TR5_KILL_WATER,	"water"}
};


/*
 * Ways item can be damaged.
 */
static const o_flag_desc wield_flags6_desc[] =
{
		{ TR6_1_HANDED,  "one-handed" },
		{ TR6_2_HANDED,  "two-handed" },
		{ TR6_OFF_HAND,  "in your off-hand" },
		{ TR6_W_SHIELD,  "with a shield" }
};



/*
 * Create a spoiler file entry for an artifact.
 * We use this to list the flags.
 */
bool list_kind_flags(u32b f5, u32b f6, int mode)
{
	bool anything = FALSE; /* Printed anything at all */

	cptr list[40];
	cptr *list_ptr;

	/* Miscellenious Abilities */
	if (f5)
	{
		list_ptr = list;

		/*
		 * Special flags
		 */
		list_ptr = spoiler_flag_aux(f5, hurt_flags5_desc, list_ptr, N_ELEMENTS(hurt_flags5_desc));

		/* Terminate the description list */
		*list_ptr = NULL;

		switch (mode)
		{
			case LIST_FLAGS_CAN:
				anything |= outlist("It can normally be damaged by", list, TERM_WHITE);
				break;
			case LIST_FLAGS_MAY:
				anything |= outlist("It might be damaged by", list, TERM_L_WHITE);
				break;
			case LIST_FLAGS_NOT:
				anything |= outlist("It is not damaged by", list, TERM_SLATE);
				break;
		}
	}

	/* Miscellenious Abilities */
	if (f6)
	{
		list_ptr = list;

		/*
		 * Special flags
		 */
		list_ptr = spoiler_flag_aux(f6, wield_flags6_desc, list_ptr, N_ELEMENTS(wield_flags6_desc));

		/* Terminate the description list */
		*list_ptr = NULL;

		switch (mode)
		{
			case LIST_FLAGS_CAN:
				anything |= outlist("It can be wielded", list, TERM_WHITE);
				break;
			case LIST_FLAGS_MAY:
				anything |= outlist("It might be wielded", list, TERM_L_WHITE);
				break;
			case LIST_FLAGS_NOT:
				anything |= outlist("It is not able to be wielded", list, TERM_SLATE);
				break;
		}
	}

	return (anything);
}


/*
 * List the rune combinations an ego item type.
 *
 * Note this is used in cmd4.c for 'generic' ego items. Note we don't show
 * the upgrades here for several reasons - mostly to keep this simple.
 */
void list_ego_item_runes(int ego, bool spoil)
{
	bool output = TRUE;

	int i, k, l, n;

	char o_name[80];

	/* 'Basic' object */
	text_out(format("You can apply %d %s rune%s to any ",
			e_info[ego].runesc,
			y_name + y_info[e_info[ego].runest].name,
			e_info[ego].runesc != 0 ? "s" : ""));

	/* Check ego item slots */
	for (n = 2; n >= 0; n--)
	{
		/* Skip blank entries */
		if (!e_info[ego].tval[n]) continue;

		/* Hack -- matches exact item */
		if (e_info[ego].min_sval[n] == e_info[ego].max_sval[n])
		{
			strip_name(o_name, sizeof(o_name), lookup_kind(e_info[ego].tval[n], e_info[ego].min_sval[n]));

			text_out(o_name);
		}
		/* Hack -- matches 'any' tval */
		else for (i = 0; object_group[i].tval; i++)
		{
			if (e_info[ego].tval[n] == object_group[i].tval)
			{
				output = TRUE;

				text_out(object_group[i].text);
			}
		}
		if (n > 1) text_out(", ");
		else if (n == 1) text_out(" or ");
	}

	/* No match */
	if (!output) text_out("ego item");

	text_out(format(" to make an ego item %s.  ", e_name + e_info[ego].name));

	/* Check for other recipes */
	for (k = 0; k < z_info->e_max; k++)
	{
		if (((spoil) || (e_info[k].aware & (AWARE_RUNES))) &&
				(e_info[k].runest == e_info[ego].runest) &&
				(e_info[k].runesc < e_info[ego].runesc))
		{
			/* Need to do this trick again. */
			output = FALSE;

			/* Check ego item slots */
			for (n = 2; n >= 0; n--)
			{
				/* Skip blank entries */
				if (!e_info[ego].tval[n]) continue;

				/* Check if slots match */
				for (l = 0; l < 3; l++)
				{
					/* Must match tvals */
					if (e_info[k].tval[l] != e_info[ego].tval[n]) continue;

					/* Must match svals */
					if (e_info[k].min_sval[l] > e_info[ego].max_sval[n]) continue;
					if (e_info[k].max_sval[l] < e_info[ego].min_sval[n]) continue;

					/* 'Basic' object */
					if (!output) text_out(format("You can apply %d %s rune%s to any ",
							e_info[ego].runesc - e_info[k].runesc,
							y_name + y_info[e_info[ego].runest].name,
							e_info[ego].runesc - e_info[k].runesc != 1 ? "s" : ""));

					/* Hack -- exact match */
					if (e_info[ego].min_sval[n] == e_info[ego].max_sval[n])
					{
						strip_name(o_name, sizeof(o_name), lookup_kind(e_info[ego].tval[n], e_info[ego].min_sval[n]));

						text_out(o_name);

						output = TRUE;
					}
					/* Hack -- match 'any' tval */
					else for (i = 0; object_group[i].tval; i++)
					{
						if (e_info[ego].tval[l] == object_group[i].tval)
						{
							output = TRUE;

							text_out(object_group[i].text);
						}
					}
					/* TODO: This is wrong, but only breaks in a couple of instances. Hopefull no one will notice. */
					if (n > 1) text_out(", ");
					else if (n == 1) text_out(" or ");
				}
			}

			if (output) text_out(format(" %s to make an ego item %s.  ", e_name + e_info[k].name, e_name + e_info[ego].name));
		}
	}
}


/*
 * Create a spoiler file entry for an artifact
 */
void list_object(const object_type *o_ptr, int mode)
{
	int i, n;

	u32b f0[ABILITY_ARRAY_SIZE];
	u32b f1, f2, f3, f4, f5, f6;

	bool anything = FALSE;
	bool charge = FALSE;
	bool detail = FALSE;
	bool powers = FALSE;
	bool learn = FALSE;

	s16b book[26];

	int num = 1;

	int time = 0;
	int randtime = 0;

	bool random = (mode == OBJECT_FLAGS_RANDOM) ? TRUE : FALSE;
	bool spoil = (mode == OBJECT_FLAGS_FULL) ? TRUE : FALSE;
	bool trap = (mode >= OBJECT_FLAGS_TRAP) ? TRUE : FALSE;

	/* Hack for traps */
	if (trap) random = TRUE;

	/* Basic stats */
	if ((!random) && (o_ptr->tval < TV_SERVICE)) for (i = 0; object_group[i].tval; i++)
	{
		if (o_ptr->tval == object_group[i].tval)
		{
			anything = TRUE;

			if (o_ptr->number == 1)
			{
				text_out("This ");

				/* Hack -- sets of plural items */
				if (object_group[i].text[strlen(object_group[i].text)-1] == 's')
					text_out("set of ");

				/* 'Basic' object */
				text_out(object_group[i].text);
			}
			else
			{
				text_out("These ");

				/* Hack -- sets of plural items */
				if (object_group[i].text[strlen(object_group[i].text)-1] == 's')
				{
					text_out("sets of ");

					/* 'Basic' object */
					text_out(object_group[i].text);
				}
				else
				{
					/* 'Basic' object */
					text_out(object_group[i].text);

					text_out("s");
				}
			}

			if (o_ptr->number == 1) text_out(" weighs ");
			else text_out(" weigh ");

			/* Weight */
			text_out(format("%d.%d lbs",object_aval(o_ptr, ABILITY_WEIGHT)/10,object_aval(o_ptr, ABILITY_WEIGHT)%10));

			if (o_ptr->number == 1) text_out(".  ");
			else text_out(" each.  ");

		}

	}

	/* Basic abilities -- tool use */
	if (!random)
	{
		cptr build_bridge = "You can build a bridge across a chasm with this ";

		switch(o_ptr->tval)
		{
			case TV_DIGGING:
				text_out("You can dig pits with this and spikes to trap monsters.  ");
				anything = TRUE;
				/* Fall through */
			case TV_SWORD:
			case TV_HAFTED:
			case TV_POLEARM:
				if (cp_ptr)
				{
					if (object_aval(o_ptr, ABILITY_WEIGHT) >= 2 * cp_ptr->chg_weight)
					{
						text_out(format("It does x%d damage when charging.  ", object_aval(o_ptr, ABILITY_WEIGHT) / cp_ptr->chg_weight));
						anything = TRUE;
					}
		        }
		        else
				{
					if (object_aval(o_ptr, ABILITY_WEIGHT) >= 66)
					{
						text_out(format("It does x%d damage when charging.  ", object_aval(o_ptr, ABILITY_WEIGHT) / 33));
						anything = TRUE;
					}
				}
				break;
			case TV_SPIKE:
				text_out("You can spike doors shut with this.  ");
				text_out(format("%sand rope or chain.  ", build_bridge));
				anything = TRUE;
				break;
			case TV_ROPE:
				text_out(format("%sand spikes or grapples.  ", build_bridge));
				anything = TRUE;
				break;
			case TV_SHOT:
			case TV_ARROW:
			case TV_BOLT:
				if (o_ptr->sval == SV_AMMO_GRAPPLE)
				{
					text_out(format("%sand rope", build_bridge));
					if (o_ptr->tval == TV_BOLT) text_out(" or chain");
					text_out(".  ");
					anything = TRUE;
				}
				break;
			case TV_RUNESTONE:
				text_out("You can apply it to different objects to change them or enchant them with additional powers.  ");
				break;
			case TV_MAGIC_BOOK:
			case TV_PRAYER_BOOK:
			case TV_SONG_BOOK:
			{
				int vn = 0;
				cptr vp[128];
				
				/* Can we study spells from this book? */
				if (inven_study_okay(o_ptr))
				{					
					text_out("You can study this to learn new spells.  ");
					if (o_ptr->tval == TV_PRAYER_BOOK) text_out("The luck of the gods decides the order you learn these spells.  ");
					else if (o_ptr->tval == TV_SONG_BOOK) text_out("You must learn these spells in the listed order.  ");
					
					anything = TRUE;
				}

				/* Can we cast spells from this book? */
				if (inven_cast_okay(o_ptr))
				{
					text_out("You can cast spells from this that you have learnt.  ");
					
					anything = TRUE;
				}
				
				/* Check for services */
				for (i = 0; i < z_info->s_max; i++)
				{
					bool in_item = FALSE;
					int service_sval = 0;
					int ii;

					for (ii = 0; ii < MAX_SPELL_APPEARS; ii++)
					{
						spell_appears *s_object = &s_info[i].appears[ii];

						if ((s_object->tval == o_ptr->tval) && (s_object->sval == o_ptr->sval)) in_item = TRUE;

						if (s_object->tval == TV_SERVICE)
						{
							service_sval = s_object->sval;
						}
					}

					/* Create service object */
					if ((in_item == TRUE) && (service_sval > 0))
					{
						int k_idx = lookup_kind(TV_SERVICE,service_sval);

						vp[vn++] = k_name + k_info[k_idx].name;
					}
				}
				
				/* Scan */
				for (n = 0; n < vn; n++)
				{
					if (n == 0) text_out(format("You can sell this to a shopkeeper for them to offer the service%s of ", vn > 1 ? "s" : ""));
					text_out(vp[n]);
					if (n == vn - 1) text_out(".  ");
					else if (n == vn - 2) text_out(" and ");
					else text_out(", ");
				}

				break;

			}
			case TV_STUDY:
				text_out("You can study this to research a new field of magic.  ");
				anything = TRUE;
				break;
			case TV_LITE:
				text_out("You can wield this as a source of light.  ");
				anything = TRUE;
				break;
			case TV_STATUE:
				text_out("You're no art critic, but this might be worth something to sell.  ");
				anything = TRUE;
				break;
			case TV_ASSEMBLY:
				if (o_ptr->sval == SV_ASSEMBLY_FULL)
				{
					if (o_ptr->name3)
					{
						text_out("You have assembled something.  ");
						
						if (o_ptr->timeout) text_out("It is alive...  ");
					}
					else
					{
						text_out(format("When set in a trap with other items, this causes the trap to trigger automatically every %s turns.", o_ptr->charges));
					}
				}
				else
				{
					text_out("You can assemble this together to make something.  ");
				}
				anything = TRUE;
				break;
			case TV_MAP:
			  {
			    char str[46];

			    long_level_name(str, o_ptr->sval, 0);

			    text_out("You can travel to ");
			    text_out(str);
			    text_out(format(" (%d", min_depth(o_ptr->sval)));
			    if (max_depth(o_ptr->sval) > min_depth(o_ptr->sval))
					 text_out(format("-%d)", max_depth(o_ptr->sval)));
				 else
					 text_out(")");
			    text_out(" with this.  ");
			    anything = TRUE;
			    break;
			  }
			case TV_BAG:
				text_out("You can carry numerous objects inside it.  ");
				anything = TRUE;
				break;
		}
	}

	/* Extract the flags */
	object_flags_aux(mode, o_ptr, f0, &f1, &f2, &f3, &f4);

	/* Extract kind flags */
	f5 = k_info[o_ptr->k_idx].flags5;
	f6 = k_info[o_ptr->k_idx].flags6;

	/* Basic abilities -- damage/ damage multiplier */
	if (!random && object_ability(o_ptr, ABILITY_DAMAGE_DICE) && object_ability(o_ptr, ABILITY_DAMAGE_SIDES))
	{
		bool throw_it = TRUE;
		bool throw_it_good = ((f0[2] & ((1L << (ABILITY_HURL_DAM % 32)) | (1L << (ABILITY_HURL_NUM % 32)))) != 0) || ((f3 & TR3_THROWING) != 0);

		/* Handle melee & throwing weapon damage. Ammunition handled later. */
		switch (o_ptr->tval)
		{
			case TV_SWORD:
			case TV_STAFF:
			case TV_HAFTED:
			case TV_POLEARM:
			case TV_DIGGING:
				text_out(format("When attacking%s, it ", throw_it_good ? " or easily thrown" : ""));
				break;
			case TV_LITE:
			case TV_POTION:
			case TV_FLASK:
				/* Hack -- display throwing damage later */
				throw_it = FALSE;
				break;
			case TV_SPELL:
				/* Never thrown */
				throw_it = FALSE;
				break;
			case TV_BOW:
			case TV_ARROW:
			case TV_BOLT:
			case TV_SHOT:
				/* Hack -- display throwing damage later */
				throw_it = FALSE;
				break;
			default:
				if (throw_it) text_out(format("When %sthrown, it ", throw_it_good ? "easily " : ""));
				break;
		}

		if (throw_it)
		{
			text_out(format("does %dd%d", object_ability(o_ptr, ABILITY_DAMAGE_DICE), object_ability(o_ptr, ABILITY_DAMAGE_SIDES)));
			if (object_bonus_p(o_ptr) || spoil)
			{
				if (object_aval(o_ptr, ABILITY_TO_DAM) > 0) text_out(format("+%d", object_aval(o_ptr, ABILITY_TO_DAM)));
				else if (object_aval(o_ptr, ABILITY_TO_DAM) < 0) text_out(format("%d", object_aval(o_ptr, ABILITY_TO_DAM)));
			}
			text_out(" ");
			text_out((o_ptr->tval == TV_SPELL) ? "magical" : ((o_ptr->tval == TV_SWORD || o_ptr->tval == TV_POLEARM ||
						o_ptr->tval == TV_ARROW || o_ptr->tval == TV_BOLT)
						? "edged" : "blunt"));
			text_out(" damage.  ");
			anything = TRUE;
#if 0
			if (f3 & (TR3_HURL_DAM))
			{
				text_out(format(". It does x%d damage when thrown", o_ptr->pval));
			}
#endif
		}
	}

	/* Basic abilities */
	if (!random)
	{
		/* Display the flags */
		anything |= list_kind_flags(f5, f6, LIST_FLAGS_CAN);
	}

	/* Bows */
	if (!random)
	{
		int mult = 0;

		/* Bows get innate might */
		if (o_ptr->tval == TV_BOW) mult += bow_multiplier (o_ptr->sval);
#if 0
		/* Affect Might */
		if (f1 & (TR1_MIGHT)) mult += o_ptr->pval;
#endif
		/* Any benefit? */
		if (mult)
		{
			text_out(format("When %sset in a trap, it multiplies the base damage %s",
					o_ptr->tval == TV_BOW ? "shooting or " : "",
							o_ptr->tval == TV_BOW ? "of " : ""));

			/* Analyze the launcher */
			if (o_ptr->tval == TV_BOW)
			{
				switch (o_ptr->sval / 10)
				{
					/* Short Bow and Arrow */
					case 1:
					{
						text_out("arrows");
						break;
					}

					/* Light Crossbow and Bolt */
					case 2:
					{
						text_out("bolts");
						break;
					}

					/* Firearms */
					case 3:
					{
						text_out("shots");
						break;
					}

					/* Sling and ammo */
					default:
					{
						/* Hack -- slings now act like 'throwers' */
						text_out("shots or thrown items");
						break;
					}
				}
			}
			text_out(format(" by %d and has a range of %d grids.  ", mult, 6 + mult * 3));

			/* Firearms */
			if ((o_ptr->tval == TV_BOW) && (o_ptr->sval / 10 == 3))
			{
				int max_charge = k_info[o_ptr->k_idx].charges;

				text_out(format("It requires gunpowder to reload and can fit %d charge%s.  ",max_charge, max_charge > 1 ? "s" : ""));
			}
		}
	}

	/* Extra powers */
	if (((!random) || (trap)) && ((object_aware_p(o_ptr)) || (spoil) || (o_ptr->ident & (IDENT_STORE)))
		&& (o_ptr->tval !=TV_MAGIC_BOOK) && (o_ptr->tval != TV_PRAYER_BOOK)
		&& (o_ptr->tval !=TV_SONG_BOOK) && (o_ptr->tval != TV_STUDY))
	{
		int vn = 0;
		cptr vp[128];
		int vt[128];
		bool vd[128];
		bool fired = FALSE;

		cptr vp_set_trap = "When set in a trap, it ";
		cptr vp_throw = "When thrown, it ";
		cptr vp_coating = "When applied to coat an arrow, bolt, sword or polearm, it ";
		cptr vp_coat_self = "When applied to your skin or armour, it ";
		cptr vp_activate = "When activated, it ";
		cptr vp_activate_throw = "When inscribed with {=A} and attacking or thrown against an opponent, it ";
		cptr vp_activate_attack = "When inscribed with {=A} and attacking an opponent, it ";
		cptr vp_player_eat = "When eaten, it ";
		cptr vp_monster_eat = "When eaten by monsters, it ";

		bool throw_it_good = ((f0[2] & ((1L << (ABILITY_HURL_DAM % 32)) | (1L << (ABILITY_HURL_NUM % 32)))) != 0) || ((f3 & TR3_THROWING) != 0);
		
		vn = 0;

		/* Detailed explanation */
		detail = (k_info[o_ptr->k_idx].ever_used > 4 * k_info[o_ptr->k_idx].level * num) || (spoil) || (o_ptr->ident & (IDENT_MENTAL));

		/* Activates */
		if (f3 & (TR3_ACTIVATE))
		{
			vp[vn] = vp_activate; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_NORMAL;
			charge = (k_info[o_ptr->k_idx].ever_used > o_ptr->charges) || (o_ptr->ident & (IDENT_MENTAL)) || (spoil);
			time = o_ptr->charges;
			randtime = o_ptr->charges;

			switch (o_ptr->tval)
			{
				case TV_STAFF:
				case TV_SWORD:
				case TV_POLEARM:
				case TV_HAFTED:
				case TV_DIGGING:
				{
					if (throw_it_good) vp[vn] = vp_activate_throw;
					else vp[vn] = vp_activate_attack;
					vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_AIMED; break;
				}
			}
		}

		/* Other attacks based on type */
		switch(o_ptr->tval)
		{
			case TV_BOW:
				vp[vn] = vp_throw; vd[vn] = TRUE; vt[vn++] = SPELL_TARGET_AIMED;
				break;
			case TV_DIGGING:
				if (throw_it_good)
				{
					vp[vn] = vp_throw; vd[vn] = TRUE; vt[vn++] = SPELL_TARGET_AIMED;
				}
				break;

			case TV_SCROLL:
				vp[vn] = "When read, it "; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_NORMAL;
				vp[vn] = vp_set_trap; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_VICTIM;
				break;

			case TV_BODY:
			case TV_BONE:
				vp[vn] = vp_player_eat; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_SELF;
				vp[vn] = vp_monster_eat; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_OTHER;
				break;

			case TV_MUSHROOM:
				vp[vn] = vp_coating; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_COATED;
				vp[vn] = vp_coat_self; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_ATTACKER;
				vp[vn] = vp_set_trap; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_VICTIM;

				/* Fall through */
			case TV_FOOD:
				vp[vn] = vp_player_eat; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_SELF;
				vp[vn] = vp_monster_eat; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_OTHER;
				break;

			case TV_ROD:
				vp[vn] = "When zapped, it "; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_NORMAL;
				vp[vn] = vp_set_trap; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_VICTIM;
				charge = (k_info[o_ptr->k_idx].ever_used > o_ptr->charges) || (o_ptr->ident & (IDENT_MENTAL)) || (spoil);
				time = o_ptr->charges;
				randtime = o_ptr->charges;
				break;

			case TV_STAFF:
				vp[vn] = "When used, it "; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_NORMAL;
				if (throw_it_good)
				{
					vp[vn] = vp_throw; vd[vn] = TRUE; vt[vn++] = SPELL_TARGET_AIMED;
				}
				vp[vn] = vp_set_trap; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_VICTIM;
				break;

			case TV_WAND:
				vp[vn] = "When aimed, it "; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_NORMAL;
				vp[vn] = vp_set_trap; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_VICTIM;
				break;

			case TV_BOLT:
				vp[vn] = "When fired from a crossbow, it "; vd[vn] = TRUE; vt[vn++] = SPELL_TARGET_AIMED;
				vp[vn] = vp_set_trap; vd[vn] = TRUE; vt[vn++] = SPELL_TARGET_VICTIM;
				vp[vn] = vp_throw; vd[vn] = TRUE; vt[vn++] = SPELL_TARGET_AIMED;
				fired = TRUE;
				break;

			case TV_ARROW:
				vp[vn] = "When fired from a bow, it "; vd[vn] = TRUE; vt[vn++] = SPELL_TARGET_AIMED;
				vp[vn] = vp_set_trap; vd[vn] = TRUE; vt[vn++] = SPELL_TARGET_VICTIM;
				vp[vn] = vp_throw; vd[vn] = TRUE; vt[vn++] = SPELL_TARGET_AIMED;
				fired = TRUE;
				break;

			case TV_SHOT:
				vp[vn] = "When fired from a sling, it "; vd[vn] = TRUE; vt[vn++] = SPELL_TARGET_AIMED;
				vp[vn] = vp_set_trap; vd[vn] = TRUE; vt[vn++] = SPELL_TARGET_VICTIM;
				vp[vn] = vp_throw; vd[vn] = TRUE; vt[vn++] = SPELL_TARGET_AIMED;
				fired = TRUE;
				break;

			case TV_SERVICE:
				vp[vn] = "When purchased, it "; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_NORMAL;
				break;

			case TV_POTION:
				vp[vn] = "When quaffed, it "; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_SELF;
				/* Fall through */
			case TV_FLASK:
				if ((o_ptr->tval == TV_FLASK) && (o_ptr->sval == SV_FLASK_BLOOD))
				{
					vp[vn] = vp_player_eat; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_SELF;
				}
				vp[vn] = vp_coating; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_COATED;
				vp[vn] = vp_coat_self; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_ATTACKER;
				vp[vn] = vp_throw; vd[vn] = TRUE; vt[vn++] = SPELL_TARGET_EXPLODE;
				vp[vn] = vp_set_trap; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_VICTIM;
				break;

			case TV_LITE:
				vp[vn] = vp_throw; vd[vn] = TRUE; vt[vn++] = SPELL_TARGET_AIMED;
				vp[vn] = vp_set_trap; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_VICTIM;
				break;

			case TV_SWORD:
			case TV_POLEARM:
			case TV_HAFTED:
				if (throw_it_good)
				{
					vp[vn] = vp_throw; vd[vn] = TRUE; vt[vn++] = SPELL_TARGET_AIMED;
				}
				vp[vn] = vp_set_trap; vd[vn] = TRUE; vt[vn++] = SPELL_TARGET_VICTIM;
				break;

			case TV_DRAG_ARMOR:
				vp[vn] = vp_set_trap; vd[vn] = FALSE; vt[vn++] = SPELL_TARGET_VICTIM;
				break;

			case TV_RUNESTONE:
				vp[vn] = vp_set_trap; vd[vn] = TRUE; vt[vn++] = SPELL_TARGET_VICTIM;
				break;
		}

		/* Artifacts */
		if ((o_ptr->name1) && ((object_known_p(o_ptr)) || (spoil)) && (f3 & TR3_ACTIVATE))
		{
			artifact_type *a_ptr = &a_info[o_ptr->name1];

			charge = (a_ptr->activated > a_ptr->randtime) || (spoil) || (o_ptr->ident & (IDENT_MENTAL));
			detail = (a_ptr->activated > 8 * a_ptr->level) || (spoil) || (o_ptr->ident & (IDENT_MENTAL));
			time = a_ptr->time;
			randtime = a_ptr->randtime;
		}

		/* Hack -- for trap information */
		if (trap)
		{
			vn = 0;
			vp[vn] = "It "; vd[vn] = TRUE; vt[vn++] = SPELL_TARGET_VICTIM;
		}

		if (vn)
		{
			/* Scan */
			for (n = 0; n < vn; n++)
			{
				/* Reset powers count */
				powers = FALSE;

				/* Display damage if required */
				if (vd[n])
				{
					int to_d = object_aval(o_ptr, ABILITY_TO_DAM);

					text_out(vp[n]);
					/* Badly balanced weapons do minimum damage */
					if ((vp[n] == vp_throw) && (!throw_it_good) && ((f6 & (TR6_BAD_THROW)) != 0))
					{
						text_out(format("does %d", object_ability(o_ptr, ABILITY_DAMAGE_DICE)));
					}
					else
					{
						text_out(format("does %dd%d", object_ability(o_ptr, ABILITY_DAMAGE_DICE), object_ability(o_ptr, ABILITY_DAMAGE_SIDES)));

						if (vp[n] == vp_throw) to_d *= 2;
					}
					if (object_bonus_p(o_ptr) || spoil)
					{
						if (to_d > 0) text_out(format("+%d", to_d));
						else if (to_d < 0) text_out(format("%d", to_d));
					}
					text_out(" ");
					text_out((o_ptr->tval == TV_SPELL) ? "magical" : ((o_ptr->tval == TV_SWORD || o_ptr->tval == TV_POLEARM ||
							o_ptr->tval == TV_ARROW || o_ptr->tval == TV_BOLT)
							? "edged" : "blunt"));
					text_out(" damage");
					if (vp[n] != vp_throw)
					{
						switch(o_ptr->tval)
						{
							case TV_BOLT:
							case TV_ARROW:
							case TV_SHOT:
								text_out(format(" times the %s multiplier", o_ptr->tval == TV_BOLT ? "crossbow" :
									(o_ptr->tval == TV_ARROW ? "bow" : "sling or firearm")));
								if ((trap) || (vp[n] == vp_set_trap))
								{
									text_out(" of the trap it is loaded in");
								}

								break;
						}

					}
					else
					{

#if 0
						if (f3 & (TR3_HURL_DAM))
						{
							text_out(format(". It does x%d damage when thrown", o_ptr->pval));
						}
#endif
					}
					anything = TRUE;
					powers = TRUE;
				}

				/* Hack -- food feeds player */
				if ((vp[n] == vp_player_eat) && (o_ptr->charges))
				{
					text_out(vp[n]);
					text_out(detail ? format("provides %d units of nourishment", o_ptr->charges * object_aval(o_ptr, ABILITY_WEIGHT)) : "provides nourishment");
					switch (o_ptr->tval)
					{
						case TV_MUSHROOM:
						case TV_FOOD:
							break;
						case TV_FLASK:
							text_out(" if you are undead");
							break;
						case TV_EGG:
							text_out(" to hungry players");
							break;
						default:
							text_out(" to starving players");
							break;
					}
					vd[n] = TRUE;
					anything = TRUE;
					powers = TRUE;
				}

				/* Hack -- monster food recovers monsters */
				if (vp[n] == vp_monster_eat)
				{
					text_out(vp[n]);
					text_out("helps them recover");
					vd[n] = TRUE;
					anything = TRUE;
					powers = TRUE;
				}

				/* Fill the book with spells */
				fill_book(o_ptr,book,&num);

				/* Hack -- fill book with artifact activation */
				if ((o_ptr->name1) && ((object_known_p(o_ptr)) || (spoil)) && (f3 & TR3_ACTIVATE))
				{
					book[0] = a_info[o_ptr->name1].activation;
					num = 1;
				}

				/* No listing of numerous powers, as in wands of wonder,
					since it overflows the standard screen */
				if (num > 10)
				{
					text_out(vd[n] ? " and ": vp[n]);
					text_out("produces a random effect");
					powers = TRUE;
				}
				else
				{
					/* Display powers */
					for (i = 0; i < num; i++)
					{
						bool tmp;

						/* List powers */
						tmp = spell_desc(&s_info[book[i]],(i==0) ? (vd[n] ? " and ": vp[n]) : " or ",
								o_ptr->name1 ? a_info[o_ptr->name1].level : (o_ptr->name2 ?
									e_info[o_ptr->name2].level : k_info[o_ptr->k_idx].level),detail, vt[n]);

						/* Any output */
						powers |= tmp;
						if (!detail) learn |= tmp;
					}
				}
				
				if ((powers) && (vp[n] == vp_coat_self))
				{
					text_out(", if they hit the applied region");
				}
				
				if ((charge) && (powers))
				{
					if ((time) && (randtime)) text_out(format(", recharging in d%d+%d turns.  ",randtime, time));
					else if (randtime) text_out(format(", recharging in d%d turns.  ",randtime));
					else if (time) text_out(format(", recharging in %d turns.  ",time));
					else assert(FALSE); /* activable item with no recharge time */
				}
				else if (powers) text_out(".  ");

				anything |= powers;
			}
		}
	}

	/* Basic abilities -- armor class */
	if (!random)
	{
		int armor = object_aval(o_ptr, ABILITY_AC);

		if (object_bonus_p(o_ptr) || spoil) armor += object_aval(o_ptr, ABILITY_TO_AC);

		if (armor)
		{
			text_out(format("It %s your armor class by %d.  ", armor > 0 ? "increases" : "decreases" , armor > 0 ? armor : -armor));

			anything = TRUE;
		}
	}

	/* Display the flags */
	anything |= list_object_flags(NULL, f0, f1, f2, f3, f4, spoil || (o_ptr->ident & (IDENT_PVAL | IDENT_MENTAL | IDENT_KNOWN | IDENT_STORE)) ? o_ptr->pval : 0, LIST_FLAGS_CAN);

	/* Hack -- Caveat some flags */
	if (!random)
	{
		switch (o_ptr->tval)
		{
			case TV_BOOTS:
			case TV_GLOVES:
				text_out(format("Slays and brands are %s effective on %s when you attack using unarmed combat.  ",
						o_ptr->tval == TV_BOOTS ? "only" : "most",
								o_ptr->tval == TV_BOOTS ? "boots" : "gloves"));

				if (o_ptr->tval == TV_BOOTS) break;
				text_out("Slays and brands are x1 multiplier less effective on gloves when wielding a weapon.  ");
				break;

			case TV_RING:
				text_out("Slays and brands are x1 multiplier less effective on rings and only apply to the hand the ring is worn on.  ");
				text_out("Wear it on the right hand to affect thrown weapons and the main weapon you are wielding.  ");
				text_out("Wear it on the left hand to affect missile weapons and off-hand weapons.  ");
				text_out("Two handed weapons are affected by rings on both hands.  ");
				break;

			case TV_BOW:
				text_out("Slays and brands are x1 multiplier less effective on missile weapons then their ammunition.  ");
				break;
		}
	}

	/*
	 * Handle cursed objects here to avoid redundancies such as noting
	 * that a permanently cursed object is heavily cursed as well as
	 * being "lightly cursed".
	 */
	if (!random && cursed_p(o_ptr))
	{
		if (f3 & (TR3_PERMA_CURSE))
		{
			text_out_c(TERM_L_PURPLE, "It is permanently cursed.  ");
			anything = TRUE;
		}
		else if (f3 & (TR3_HEAVY_CURSE))
		{
			text_out_c(TERM_L_PURPLE, "It is heavily cursed.  ");
			anything = TRUE;
		}
		else if (object_known_p(o_ptr) || (o_ptr->feeling == INSCRIP_CURSED))
		{
			text_out_c(TERM_L_PURPLE, "It is cursed.  ");
			anything = TRUE;
		}
	}

	/* If trap, we are done */
	if (trap) return;

	/* Have sensed something about this item */
	if ((o_ptr->feeling) || (object_known_p(o_ptr)))
	{
		int feeling = o_ptr->feeling;
		if (!feeling)
		{
			feeling = value_check_aux1(o_ptr);

			/* Hack -- exclude 'average' feelings for known items */
			if ((feeling == INSCRIP_AVERAGE) && (object_known_p(o_ptr))) feeling = 0;
		}

		if (feeling)
		{
			if (feeling <= INSCRIP_COATED)
			{
				text_out(inscrip_info[feeling]);
				anything = TRUE;
			}
			else if (feeling >= MAX_INSCRIP)
			{
				int bag = lookup_kind(TV_BAG, feeling - MAX_INSCRIP);

				if (bag)
				{
					text_out(format("You sense it belongs with %s.  ",k_name + k_info[bag].name));
					anything = TRUE;
				}
			}
			else
			{
				text_out(inscrip_info[INSCRIP_COATED + 1]);
			}
		}
	}

	/* Know how to make this item */
	if ((!random) && (k_info[o_ptr->k_idx].runesc))
	{
		/* Show runes */
		if (spoil || (object_aware_p(o_ptr) && (k_info[o_ptr->k_idx].aware & (AWARE_RUNES))) ||
				(k_info[o_ptr->k_idx].aware & (AWARE_RUNEX)))
		{
			for (i = 0; object_group[i].tval; i++)
			{
				if (o_ptr->tval == object_group[i].tval)
				{
					anything = TRUE;

					/* 'Basic' object */
					text_out(format("You can apply %d %s rune%s to any %s to make this.  ",
							k_info[o_ptr->k_idx].runesc,
							y_name + y_info[k_info[o_ptr->k_idx].runest].name,
							k_info[o_ptr->k_idx].runesc != 1 ? "s" : "",
							object_group[i].text));
				}
			}

			/* Check for other recipes */
			for (i = 0; i < z_info->k_max; i++)
			{
				if (((spoil) || (k_info[i].aware & (AWARE_RUNES | AWARE_RUNEX))) &&
						(k_info[i].runest == k_info[o_ptr->k_idx].runest) &&
						(k_info[i].runesc < k_info[o_ptr->k_idx].runesc) &&
						(k_info[i].tval == k_info[o_ptr->k_idx].tval))
				{
					object_type object_type_body;
					object_type *i_ptr = &object_type_body;
					char o_name[80];

					object_prep(i_ptr, i);
					if (spoil) i_ptr->ident |= (IDENT_STORE);
					object_desc(o_name, sizeof(o_name), i_ptr, TRUE, 1);

					text_out(format("You can apply %d %s rune%s to %s to make this.  ",
							k_info[o_ptr->k_idx].runesc - k_info[i].runesc,
							y_name + y_info[k_info[o_ptr->k_idx].runest].name,
							(k_info[o_ptr->k_idx].runesc  - k_info[i].runesc) != 1 ? "s" : "",
							o_name));
				}
			}

			/* Check for rune upgrades */
			for (i = 0; i < z_info->k_max; i++)
			{
				if (((spoil) || (k_info[i].aware & (AWARE_RUNES | AWARE_RUNEX))) &&
						(k_info[i].runest == k_info[o_ptr->k_idx].runest) &&
						(k_info[i].runesc > k_info[o_ptr->k_idx].runesc) &&
						(k_info[i].tval == k_info[o_ptr->k_idx].tval))
				{
					object_type object_type_body;
					object_type *i_ptr = &object_type_body;
					char o_name[80];

					object_prep(i_ptr, i);
					if (spoil) i_ptr->ident |= (IDENT_STORE);
					object_desc(o_name, sizeof(o_name), i_ptr, TRUE, 1);

					text_out(format("You can apply %d %s rune%s to this to make %s.  ",
							k_info[i].runesc - k_info[o_ptr->k_idx].runesc,
							y_name + y_info[k_info[o_ptr->k_idx].runest].name,
							(k_info[i].runesc - k_info[o_ptr->k_idx].runesc) != 1 ? "s" : "",
							o_name));
				}
			}
		}
	}

	/* Know how to make this item */
	if ((!random) && (o_ptr->name2) && (spoil || object_named_p(o_ptr)) && (e_info[o_ptr->name2].runesc))
	{
		/* Show runes */
		if (spoil || (e_info[o_ptr->name2].aware & (AWARE_RUNES)))
		{
			int k, l;

			anything = TRUE;

			/* Check for ego item runes */
			list_ego_item_runes(o_ptr->name2, spoil);

			/* Check for upgrades */
			for (k = 0; k < z_info->e_max; k++)
			{
				if (((spoil) || (e_info[k].aware & (AWARE_RUNES))) &&
						(e_info[k].runest == e_info[o_ptr->name2].runest) &&
						(e_info[k].runesc > e_info[o_ptr->name2].runesc))
				{
					for (l = 0; l < 3; l++)
					{
						/* Must match tvals */
						if (e_info[k].tval[l] != o_ptr->tval) continue;

						/* Must match svals */
						if (e_info[k].min_sval[l] > o_ptr->sval) continue;
						if (e_info[k].max_sval[l] < o_ptr->sval) continue;

						/* 'Basic' object */
						text_out(format("You can apply %d %s rune%s to this to make it %s.  ",
								e_info[k].runesc - e_info[o_ptr->name2].runesc,
								y_name + y_info[e_info[o_ptr->name2].runest].name,
								e_info[k].runesc - e_info[o_ptr->name2].runesc != 1 ? "s" : "",
								e_name + e_info[k].name));
					}
				}
			}
		}
	}

	/* Can learn more about this item through use */
	if (learn)
	{
		text_out_c(TERM_L_VIOLET, "You will learn more about this item through further use.  ");
		anything = TRUE;
	}

	/* Can enchant this further? */
	if (!spoil && !random && object_named_p(o_ptr) && !(o_ptr->xtra1) && !artifact_p(o_ptr) && (o_ptr->tval != TV_SERVICE) )
	{
		text_out_c(TERM_VIOLET,"\nYou can apply runes to it or enchant it with additional powers.  ");
		anything = TRUE;
	}

	/* Unknown extra powers (ego-item with random extras or artifact) */
	if (!spoil && !random && object_named_p(o_ptr) && (((o_ptr->xtra1) && (o_ptr->name2)) || artifact_p(o_ptr)) )
	{
		bool hidden = TRUE;

		if (o_ptr->ident & (IDENT_MENTAL)) hidden = FALSE;

		if (hidden)
		{
			text_out_c(TERM_VIOLET,"\nIt may have undiscovered powers.  ");
			anything = TRUE;
		}
	}

	if (!random && !spoil)
	{
		/* Display the flags */
		anything |= list_object_flags(NULL, o_ptr->may_flags0, o_ptr->may_flags1, o_ptr->may_flags2, o_ptr->may_flags3, o_ptr->may_flags4, o_ptr->ident & (IDENT_PVAL | IDENT_MENTAL | IDENT_KNOWN) ? o_ptr->pval : 0, LIST_FLAGS_MAY);

#if 0
                /* Equipment only */
                if (wield_slot(o_ptr) >= INVEN_WIELD)
                        /* Display the flags */
                        anything |= list_object_flags(o_ptr->not_flags1, o_ptr->not_flags2, o_ptr->not_flags3, o_ptr->not_flags4, o_ptr->ident & (IDENT_PVAL | IDENT_MENTAL | IDENT_KNOWN) ? o_ptr->pval : 0, LIST_FLAGS_NOT);
#endif
	}

	/* Abort now if undisplayable origin */
	if (!random && !(o_ptr->origin == ORIGIN_NONE || o_ptr->origin == ORIGIN_MIXED))
	{

		if (o_ptr->number > 1)
			text_out("\nThey were ");
		else
			text_out("\nIt was ");

		/* Display the right thing */
		switch (o_ptr->origin)
		{
			case ORIGIN_BIRTH:
				text_out("an inheritance from your family");
				break;

			case ORIGIN_STORE:
				text_out("bought in a store");
				break;

			case ORIGIN_FLOOR:
				text_out("lying on the floor");
				break;

			case ORIGIN_DROP:
			{
				char m_name[80];

				text_out("dropped by ");

				/* Get the name */
				race_desc(m_name, sizeof(m_name), o_ptr->origin_xtra, 0x408, 1);

				text_out(m_name);
				
				break;
			}

			case ORIGIN_DROP_UNKNOWN:
				text_out("dropped by an unknown monster");
				break;

			case ORIGIN_FEAT:
				text_out("in a dungeon feature");
				break;

			case ORIGIN_ACQUIRE:
				text_out("conjured forth by magic");
				break;

			case ORIGIN_CHEAT:
				text_out("created by a debug option");
				break;

			case ORIGIN_CHEST:
				text_out("found in a chest");
				break;

			case ORIGIN_STORE_REWARD:
				text_out("given as a reward");
				break;

			case ORIGIN_STORE_STORAGE:
				text_out("found in storage");
				break;


		}

		if (o_ptr->origin_depth)
		{
			if (depth_in_feet)
				text_out(format(" at a depth of %d feet", o_ptr->origin_depth * 50));
			else
				text_out(format(" on dungeon level %d", o_ptr->origin_depth));
		}

		text_out(".  ");
		anything = TRUE;
	}

	/* Display bag contents */
	if (o_ptr->tval == TV_BAG)
	{
		object_type object_type_body;
		object_type *i_ptr = &object_type_body;

		bool intro = FALSE;

		char o_name[120];

		/* Display fake objects */
		for (i = 0; i < INVEN_BAG_TOTAL; i++)
		{
			/* Empty slot */
			if (!(bag_holds[o_ptr->sval][i][0])) continue;

			/* Nothing in slot */
			if (!(bag_contents[o_ptr->sval][i])) continue;

			/* Fake the item */
			fake_bag_item(i_ptr, o_ptr->sval, i);

			/* Intro */
			if (!intro && !random)
			{
				text_out("\nIt contains:\n");
			}

			/* Describe */
			object_desc(o_name, sizeof(o_name), i_ptr, TRUE, 1);

			text_out(format("%s %c) %s\n",random ? (!intro ? "with": "    ") : " ", 'a'+i, o_name));

			intro = TRUE;
		}
	}

	/* *Identified* object */
	if (spoil && anything) text_out("You know everything about this item.  ");

	/* Nothing was printed */
	if (!random && !anything) text_out("You know nothing worth noting about this item.  ");

	/* End */
	if (!random) text_out("\n");
	else if (anything) text_out("\n");
}


/*
 * Print a list of powers (for selection).
 */
void print_powers(const s16b *book, int num, int y, int x, int level)
{
	int i, spell;

	cptr comment;

	char info[80];

	char out_val[160];

	byte line_attr;

	spell_type *s_ptr;

	/* Title the list */
	prt("", y, x);
	put_str("Name", y, x + 5);

	/* Dump the spells */
	for (i = 0; i < num; i++)
	{
		bool dummy = TRUE;

		/* Get the spell index */
		spell = book[i];

		/* Get the spell info */
		s_ptr = &s_info[spell];

		/* Prepare the spell */
		process_spell_prepare(spell, 25, &dummy, FALSE, FALSE);

		/* Get extra info */
		spell_info(info, sizeof(info), spell, level);

		/* Use that info */
		comment = info;

		/* Assume spell is known and tried */
		line_attr = TERM_WHITE;

		/* Dump the spell --(-- */
		sprintf(out_val, "  %c) %-30s %s",
			I2A(i), s_name + s_ptr->name,
			comment);
		c_prt(line_attr, out_val, y + i + 1, x);

		/* Paranoia - clear boost */
		p_ptr->boost_spell_power = 0;
	}

	/* Clear the bottom line */
	prt("", y + i + 1, x);
}


/*
 * Print a list of spells (for browsing or casting or viewing).
 */
void print_spells(const s16b *sn, int num, int y, int x)
{
	int i, ii, spell, level;

	cptr comment;

	char info[80];

	char out_val[160];

	byte line_attr;

	spell_type *s_ptr;

	spell_cast *sc_ptr = NULL;

	/* Title the list */
	prt("", y, x);
	put_str("Name", y, x + 5);
	put_str("Lv Mana Fail Info", y, x + 35);

	/* Dump the spells */
	for (i = 0; i < num; i++)
	{
		bool dummy = TRUE;

		/* Get the spell index */
		spell = sn[i];

		/* Skip missing spells */
		if (!spell)
		{
			sprintf(out_val, "  %c) %-30s", I2A(i), "(missing)");
			c_prt(TERM_L_DARK, out_val, y + i + 1, x);
			continue;
		}

		/* Get the spell info */
		s_ptr = &s_info[spell];

		/* Get the spell details; warriors (class 0) have no spells */
		sc_ptr = spell_cast_details(spell);

		/* Skip illegible spells */
		if ((!spell_legible(spell)) || !(sc_ptr))
		{
			sprintf(out_val, "  %c) %-30s", I2A(i), s_name+s_info[0].name);
			c_prt(TERM_L_DARK, out_val, y + i + 1, x);
			continue;
		}

		/* Prepare the spell */
		process_spell_prepare(spell, spell_power(spell), &dummy, FALSE, FALSE);

		/* Get extra info */
		spell_info(info, sizeof(info), spell, spell_power(spell));

		/* Get level */
		level = spell_level(spell);

		/* Use that info */
		comment = info;

		/* Assume spell is known and tried */
		line_attr = TERM_WHITE;

		/* Get the spell knowledge*/
		for (ii=0;ii<PY_MAX_SPELLS;ii++)
		{

			if (p_ptr->spell_order[ii] == spell) break;

		}

		/* Analyze the spell */
		if (ii==PY_MAX_SPELLS)
		{
			if (level <= p_ptr->lev)
			{
				if (spell_okay(spell, FALSE))
				{
					comment = " unknown";
					line_attr = TERM_L_BLUE;
				}
				else
				{
					comment = " prerequisite";
					line_attr = TERM_VIOLET;
				}
			}
			else
			{
				comment = " difficult";
				line_attr = TERM_RED;
			}
		}
		else if ((ii < 32) ? (p_ptr->spell_forgotten1 & (1L << ii)) :
		      ((ii < 64) ? (p_ptr->spell_forgotten2 & (1L << (ii - 32))) :
		      ((ii < 96) ? (p_ptr->spell_forgotten3 & (1L << (ii - 64))) :
		      (p_ptr->spell_forgotten4 & (1L << (ii - 96))))))
		{
			comment = " forgotten";
			line_attr = TERM_YELLOW;
		}
		else if (!((ii < 32) ? (p_ptr->spell_learned1 & (1L << ii)) :
		      ((ii < 64) ? (p_ptr->spell_learned2 & (1L << (ii - 32))) :
		      ((ii < 96) ? (p_ptr->spell_learned3 & (1L << (ii - 64))) :
		      (p_ptr->spell_learned4 & (1L << (ii - 96)))))))
		{
			if (level <= p_ptr->lev)
			{
				comment = " unknown";
				line_attr = TERM_L_BLUE;
			}
			else
			{
				comment = " difficult";
				line_attr = TERM_RED;
			}
		}
		else if (!((ii < 32) ? (p_ptr->spell_worked1 & (1L << ii)) :
		      ((ii < 64) ? (p_ptr->spell_worked2 & (1L << (ii - 32))) :
		      ((ii < 96) ? (p_ptr->spell_worked3 & (1L << (ii - 64))) :
		      (p_ptr->spell_worked4 & (1L << (ii - 96)))))))
		{
			comment = " untried";
			line_attr = TERM_L_GREEN;
		}

		/* Dump the spell --(-- */
		sprintf(out_val, "  %c) %-30s%2d %4d %3d%%%s",
			I2A(i), s_name + s_ptr->name,
			level, sc_ptr->mana, spell_chance(spell), comment);
		c_prt(line_attr, out_val, y + i + 1, x);
	}


	/* Clear the bottom line */
	prt("", y + i + 1, x);
}

/*
 * Hack -- Create a "forged" artifact
 */
bool make_fake_artifact(object_type *o_ptr, byte name1)
{
	int i;

	artifact_type *a_ptr = &a_info[name1];

	/* Ignore "empty" artifacts */
	if (!a_ptr->name) return FALSE;

	/* Get the "kind" index */
	i = lookup_kind(a_ptr->tval, a_ptr->sval);

	/* Oops */
	if (!i) return (FALSE);

	/* Create the artifact */
	object_prep(o_ptr, i);

	/* Save the name */
	o_ptr->name1 = name1;

	/* Ensure curse is known */
	if (a_ptr->flags3 & (TR3_LIGHT_CURSE)) o_ptr->ident |= (IDENT_CURSED);

	/* Success */
	return (TRUE);
}


/*
 * Hack -- display an object kind in the current window
 *
 * Include list of usable spells for readible books
 */
void display_koff(const object_type *o_ptr)
{
	int y;

	int i,ii,iii;

	char o_name[80];

	bool browse;

	spell_type *s_ptr;

	/* Erase the window */
	for (y = 0; y < Term->hgt; y++)
	{
		/* Erase the line */
		Term_erase(0, y, 255);
	}


	/* No info */
	if (!o_ptr->k_idx) return;

	/* Fully describe the object */
	object_desc(o_name, sizeof(o_name), o_ptr, TRUE,1);

	/* Set text_out hook */
	text_out_hook = text_out_to_screen;

	text_out("\n");

	/* Actually display the item */
	list_object(o_ptr, OBJECT_FLAGS_KNOWN);

	/* Display monster attributes */
	if ((o_ptr->name3) && ((o_ptr->tval != TV_HOLD) || (object_named_p(o_ptr)))) screen_roff(o_ptr->name3, &l_list[o_ptr->name3]);

	/* Display item name */
	obj_top(o_ptr);

	/* Warriors are illiterate */
	if ((c_info[p_ptr->pclass].spell_first > PY_MAX_LEVEL)
		&& (p_ptr->pstyle != WS_MAGIC_BOOK) && (p_ptr->pstyle != WS_PRAYER_BOOK) && (p_ptr->pstyle != WS_SONG_BOOK)) return;

	browse = FALSE;

	for (i=0;i<z_info->s_max;i++)
	{
		s_ptr=&s_info[i];

		for (ii=0;ii<MAX_SPELL_APPEARS;ii++)
		{
			if ((s_ptr->appears[ii].tval == o_ptr->tval) &&
			    (s_ptr->appears[ii].sval == o_ptr->sval))
			{
				for (iii = 0; iii < MAX_SPELL_CASTERS; iii++)
				{
					if (s_ptr->cast[iii].class == p_ptr->pclass) browse=TRUE;
				}
			}
		}

	}

	if (browse)
	{
		s16b book[26];

		int num;

		/* Fill the book with spells */
		fill_book(o_ptr, book,&num);

		/* Print spells */
		print_spells(book, num, 2, 0);

	}

}


/*
 * Attempt to guess an item name or sval
 */
void object_guess_name(object_type *o_ptr)
{
	int i,ii,iii;

	int score;
	int high = 0;

	byte guess1=0;
	byte guess2=0;
	byte guess3=0;

	u32b f0[ABILITY_ARRAY_SIZE];
	u32b f1 = o_ptr->can_flags1;
	u32b f2 = o_ptr->can_flags2;
	u32b f3 = o_ptr->can_flags3;
	u32b f4 = o_ptr->can_flags4;
	
	/* Get f0 array */
	for (i = 0; i < ABILITY_ARRAY_SIZE; i++) f0[i] = o_ptr->can_flags0[i];
	
	/* Do not guess identified items */
	if (object_named_p(o_ptr)) return;

	/* Remove flags on aware objects */
	if (object_aware_p(o_ptr))
	{
		for (i = 0; i < k_info[o_ptr->k_idx].ability_count; i++) f0[k_info[o_ptr->k_idx].ability[i]/32] &= ~(1L << (k_info[o_ptr->k_idx].ability[i]%32));
		f1 &= ~(k_info[o_ptr->k_idx].flags1);
		f2 &= ~(k_info[o_ptr->k_idx].flags2);
		f3 &= ~(k_info[o_ptr->k_idx].flags3);
		f4 &= ~(k_info[o_ptr->k_idx].flags4);
	}

	/* Nothing unknown */
	if (!f1 && !f2 && !f3 && !f4)
	{
		for (i = 0; i < ABILITY_ARRAY_SIZE; i++)
		{
			if (f0[i]) break;
		}
		
		if (i == ABILITY_ARRAY_SIZE) return;
	}

	/* Check the ego item list */
	/* Hack -- exclude artifacts and potions */
	if ((o_ptr->feeling != INSCRIP_SPECIAL) &&
	       (o_ptr->feeling != INSCRIP_ARTIFACT) &&
	       (o_ptr->feeling != INSCRIP_TERRIBLE) &&
	       (o_ptr->feeling != INSCRIP_UNBREAKABLE))
	for (i = 1; i < z_info->e_max; i++)
	{
		ego_item_type *e_ptr = &e_info[i];
		object_lore *n_ptr = &e_list[i];

		bool legal;

		legal = FALSE;

		/* Skip "empty" items */
		if (!e_ptr->name) continue;

		/* Test if this is a legal ego-item type for this object */
		for (ii = 0; ii < 3; ii++)
		{
			/* Require identical base type */
			if ((o_ptr->tval == e_ptr->tval[ii])
				&& (o_ptr->sval > e_ptr->min_sval[ii])
				&& (o_ptr->sval < e_ptr->max_sval[ii]))
			{
				legal = TRUE;
			}
		}

		/* Legal ego item */
		if (!legal) continue;

		/* Must possess powers */
		for (ii = 0; ii < ABILITY_ARRAY_SIZE; ii++)
		{
			if (o_ptr->not_flags0[ii] & n_ptr->can_flags0[ii]) legal = FALSE;
		}
		if (!legal) continue;
		if (o_ptr->not_flags1 & n_ptr->can_flags1) continue;
		if (o_ptr->not_flags2 & n_ptr->can_flags2) continue;
		if (o_ptr->not_flags3 & n_ptr->can_flags3) continue;
		if (o_ptr->not_flags4 & n_ptr->can_flags4) continue;

		/* Must not have excepted powers */
		for (ii = 0; ii < ABILITY_ARRAY_SIZE; ii++)
		{
			if (f0[ii] & n_ptr->not_flags0[ii]) legal = FALSE;
		}
		if (!legal) continue;
		if (f1 & n_ptr->not_flags1) continue;
		if (f2 & n_ptr->not_flags2) continue;
		if (f3 & n_ptr->not_flags3) continue;
		if (f4 & n_ptr->not_flags4) continue;

		/* Reset score */
		score = 0;

		/* Aware points on matching abilities: 3 for have, 1 for may */
		for (ii = 0; ii < ABILITY_MAX; ii++)
		{
			if ((adult_lore) || !(e_ptr->xtra))
			{
				for (iii = 0; iii < MAX_AVALS_EGO_ITEM; iii++) if ((f0[ii/32] & (1L<<(ii % 32))) && (e_ptr->ability[iii] == ii)) score +=3;
				for (iii = 0; iii < MAX_AVALS_EGO_ITEM; iii++) if ((o_ptr->may_flags0[ii/32] & (1L<<(ii%32))) && (e_ptr->ability[iii] == ii)) score +=1;

			}
			else
			{
				if ((f0[ii/32] & (1L<<(ii%32))) && (n_ptr->can_flags0[ii/32] & (1L<<(ii%32)))) score +=3;
				if ((o_ptr->may_flags0[ii/32] & (1L<<(ii%32))) && (n_ptr->can_flags0[ii/32] & (1L<<(ii%32)))) score +=1;
			}
		}
		
		/* Award points on matching powers: 3 for have, 1 for may */
		for (ii=0;ii<32;ii++)
		{
			if ((adult_lore) || !(e_ptr->xtra))
			{
				if ((f1 & (1L<<ii)) && (e_ptr->flags1 & (1L<<ii))) score +=3;
				if ((o_ptr->may_flags1 & (1L<<ii)) && (e_ptr->flags1 & (1L<<ii))) score +=1;
			}
			else
			{
				if ((f1 & (1L<<ii)) && (n_ptr->can_flags1 & (1L<<ii))) score +=3;
				if ((o_ptr->may_flags1 & (1L<<ii)) && (n_ptr->can_flags1 & (1L<<ii))) score +=1;
			}
		}

		/* Award points on matching powers: 3 for have, 1 for may */
		for (ii=0;ii<32;ii++)
		{
			if ((adult_lore) || !(e_ptr->xtra))
			{
				if ((f2 & (1L<<ii)) && (e_ptr->flags2 & (1L<<ii))) score +=3;
				if ((o_ptr->may_flags2 & (1L<<ii)) && (e_ptr->flags2 & (1L<<ii))) score +=1;
			}
			else
			{
				if ((f2 & (1L<<ii)) && (n_ptr->can_flags2 & (1L<<ii))) score +=3;
				if ((o_ptr->may_flags2 & (1L<<ii)) && (n_ptr->can_flags2 & (1L<<ii))) score +=1;
			}
		}

		/* Award points on matching powers: 3 for have, 1 for may */
		for (ii=0;ii<32;ii++)
		{
			/* Hack -- don't match on curse flags */
			if ((1L << ii) >= TR3_LIGHT_CURSE) continue;

			if ((adult_lore) || !(e_ptr->xtra))
			{
				if ((f3 & (1L<<ii)) && (e_ptr->flags3 & (1L<<ii))) score +=3;
				if ((o_ptr->may_flags3 & (1L<<ii)) && (e_ptr->flags3 & (1L<<ii))) score +=1;
			}
			else
			{
				if ((f3 & (1L<<ii)) && (n_ptr->can_flags3 & (1L<<ii))) score +=3;
				if ((o_ptr->may_flags3 & (1L<<ii)) && (n_ptr->can_flags3 & (1L<<ii))) score +=1;
			}
		}

		/* Award points on matching powers: 3 for have, 1 for may */
		for (ii=0;ii<32;ii++)
		{
			if ((adult_lore) || !(e_ptr->xtra))
			{
				if ((f4 & (1L<<ii)) && (e_ptr->flags4 & (1L<<ii))) score +=3;
				if ((o_ptr->may_flags4 & (1L<<ii)) && (e_ptr->flags4 & (1L<<ii))) score +=1;
			}
			else
			{
				if ((f4 & (1L<<ii)) && (n_ptr->can_flags4 & (1L<<ii))) score +=3;
				if ((o_ptr->may_flags4 & (1L<<ii)) && (n_ptr->can_flags4 & (1L<<ii))) score +=1;
			}
		}

		/* Do we have a match? */
		if (score > high)
		{
			high = score;
			guess2 = i;
		}
		/* Hack -- force lowest depth items */
		else if ((score) && (score == high) && (guess2))
		{
			ego_item_type *e2_ptr = &e_info[guess2];

			if (e_ptr->level < e2_ptr->level) guess2 = i;
		}
	}

	/* This should be here to guess for rings/amulets etc. */

	/* Check the normal item list */
	/* Hack -- exclude artifacts and potions */
	if (o_ptr->tval != TV_POTION
	    && o_ptr->feeling != INSCRIP_SPECIAL
	    && o_ptr->feeling != INSCRIP_ARTIFACT
	    && o_ptr->feeling != INSCRIP_TERRIBLE
	    && o_ptr->feeling != INSCRIP_UNBREAKABLE)
	for (i = 1; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];
		u32b kf0[ABILITY_ARRAY_SIZE];

		bool legal = TRUE;
		
		/* Skip "empty" items */
		if (!k_ptr->name) continue;

		/* Must not already be aware? */
		if (k_ptr->aware & (AWARE_FLAVOR)) continue;

		/* Must be flavored */
		if (!k_ptr->flavor) continue;

		/* Must be the same tval */
		if (k_ptr->tval != o_ptr->tval) continue;

		/* Must possess powers */
		for (ii = 0; ii < k_ptr->ability_count; ii++)
		{
			if ((o_ptr->not_flags0[k_ptr->ability[ii]/32] & (1L << (k_ptr->ability[ii]%32))) != 0) legal = FALSE;
		}
		if (!legal) continue;
		if (o_ptr->not_flags1 & k_ptr->flags1) continue;
		if (o_ptr->not_flags2 & k_ptr->flags2) continue;
		if (o_ptr->not_flags3 & k_ptr->flags3) continue;
		if (o_ptr->not_flags4 & k_ptr->flags4) continue;

		/* Initialize test */
		for (ii = 0; ii < ABILITY_ARRAY_SIZE; ii++) kf0[ii] = 0L;
		
		/* Initialize excepted powers */
		for (ii = 0; ii < k_ptr->ability_count; ii++)
		{
			kf0[k_ptr->ability[ii]/32] |= (1L << (k_ptr->ability[ii]%32));
		}
		
		/* Must not have excepted powers */
		for (ii = 0; ii < ABILITY_ARRAY_SIZE; ii++)
		{
			if (f0[ii] & ~(kf0[ii])) legal = FALSE;
		}
		if (!legal) continue;
		if (f1 & ~(k_ptr->flags1)) continue;
		if (f2 & ~(k_ptr->flags2)) continue;
		if (f3 & ~(k_ptr->flags3)) continue;
		if (f4 & ~(k_ptr->flags4)) continue;

		/* Reset score */
		score = 0;

		/* Award points on matching abilities: 3 for have, 1 for may */
		for (ii = 0; ii < k_ptr->ability_count; ii++)
		{
			if (f0[k_ptr->ability[ii]/32] & (1L<<k_ptr->ability[ii]%32)) score +=3;
			if (o_ptr->may_flags0[k_ptr->ability[ii]/32] & (1L<<k_ptr->ability[ii]%32)) score +=1;
		}
		
		/* Award points on matching powers: 3 for have, 1 for may */
		for (ii=0;ii<32;ii++)
		{
			if ((f1 & (1L<<ii)) && (k_ptr->flags1 & (1L<<ii))) score +=3;
			if ((o_ptr->may_flags1 & (1L<<ii)) && (k_ptr->flags1 & (1L<<ii))) score +=1;
		}

		/* Award points on matching powers: 3 for have, 1 for may */
		for (ii=0;ii<32;ii++)
		{
			if ((f2 & (1L<<ii)) && (k_ptr->flags2 & (1L<<ii))) score +=3;
			if ((o_ptr->may_flags2 & (1L<<ii)) && (k_ptr->flags2 & (1L<<ii))) score +=1;
		}

		/* Award points on matching powers: 3 for have, 1 for may */
		for (ii=0;ii<32;ii++)
		{
			/* Hack -- don't match on curse flags */
			if ((1L << ii) >= TR3_LIGHT_CURSE) continue;

			if ((f3 & (1L<<ii)) && (k_ptr->flags3 & (1L<<ii))) score +=3;
			if ((o_ptr->may_flags3 & (1L<<ii)) && (k_ptr->flags3 & (1L<<ii))) score +=1;
		}

		/* Award points on matching powers: 3 for have, 1 for may */
		for (ii=0;ii<32;ii++)
		{
			if ((f4 & (1L<<ii)) && (k_ptr->flags4 & (1L<<ii))) score +=3;
			if ((o_ptr->may_flags4 & (1L<<ii)) && (k_ptr->flags4 & (1L<<ii))) score +=1;
		}


		/* Do we have a match? */
		if (score > high)
		{
			high = score;
			guess3 = k_ptr->sval+1;
		}
		/* Hack -- force lowest depth items */
		else if ((score) && (score == high) && (guess3))
		{
			object_kind *k2_ptr = &k_info[lookup_kind(o_ptr->tval,guess3-1)];

			if (k_ptr->level < k2_ptr->level) guess3 = k_ptr->sval+1;
		}

	}

	/* Check the artifact list */
	/* Hack -- exclude ego items */
	if ((o_ptr->feeling != INSCRIP_EXCELLENT) &&
	       (o_ptr->feeling != INSCRIP_SUPERB) &&
	       (o_ptr->feeling != INSCRIP_HIGH_EGO_ITEM) &&
	       (o_ptr->feeling != INSCRIP_EGO_ITEM) &&
	       (o_ptr->feeling != INSCRIP_WORTHLESS))
		for (i = 1; i < z_info->a_max; i++)
	{
		artifact_type *a_ptr = &a_info[i];
		object_info *n_ptr = &a_list[i];
		
		bool legal = TRUE;

		/* Skip "empty" items */
		if (!a_ptr->name) continue;

		/* Cannot make an artifact twice */
		if (a_ptr->cur_num) continue;

		/* Must have the correct fields */
		if (a_ptr->tval != o_ptr->tval) continue;

		/* Must have the correct sval or be a special artifact */
		if ((i<ART_MIN_NORMAL) || (a_ptr->sval != o_ptr->sval)) continue;

		/* Must possess powers */
		for (ii = 0; ii < ABILITY_ARRAY_SIZE; ii++)
		{
			if (o_ptr->not_flags0[ii] & n_ptr->can_flags0[ii]) legal = FALSE;
		}
		if (!legal) continue;
		if (o_ptr->not_flags1 & n_ptr->can_flags1) continue;
		if (o_ptr->not_flags2 & n_ptr->can_flags2) continue;
		if (o_ptr->not_flags3 & n_ptr->can_flags3) continue;
		if (o_ptr->not_flags4 & n_ptr->can_flags4) continue;

		/* Must not have excepted powers */
		for (ii = 0; ii < ABILITY_ARRAY_SIZE; ii++)
		{
			if (f0[ii] & (n_ptr->not_flags0[ii])) legal = FALSE;
		}
		if (!legal) continue;
		if (f1 & n_ptr->not_flags1) continue;
		if (f2 & n_ptr->not_flags2) continue;
		if (f3 & n_ptr->not_flags3) continue;
		if (f4 & n_ptr->not_flags4) continue;

		/* Reset score */
		score = 0;

		/* Aware points on matching abilities: 3 for have, 1 for may */
		for (ii = 0; ii < ABILITY_MAX; ii++)
		{
			if (adult_lore)
			{
				for (iii = 0; iii < a_ptr->ability_count; iii++) if ((f0[ii/32] & (1L<<(ii % 32))) && (a_ptr->ability[iii] == ii)) score +=3;
				for (iii = 0; iii < a_ptr->ability_count; iii++) if ((o_ptr->may_flags0[ii/32] & (1L<<(ii%32))) && (a_ptr->ability[iii] == ii)) score +=1;
				
#if 0
				for (iii = 0; iii < a_ptr->ability_count; iii++) if ((f0[ii/32] & (1L<<(ii % 32))) && (a_ptr->flags0[ii/32][iii] & (1L<<(ii%32)))) score +=3;
				for (iii = 0; iii < a_ptr->ability_count; iii++) if ((o_ptr->may_flags0[ii/32] & (1L<<(ii%32))) && (a_ptr->flags0[ii/32][iii] & (1L<<(ii%32)))) score +=1;
#endif
			}
			else
			{
				if ((f0[ii/32] & (1L<<(ii%32))) && (n_ptr->can_flags0[ii/32] & (1L<<(ii%32)))) score +=3;
				if ((o_ptr->may_flags0[ii/32] & (1L<<(ii%32))) && (n_ptr->can_flags0[ii/32] & (1L<<(ii%32)))) score +=1;
			}
		}

		/* Award points on matching powers: 3 for have, 1 for may */
		for (ii=0;ii<32;ii++)
		{
			if (adult_lore)
			{
				if ((f1 & (1L<<ii)) && (a_ptr->flags1 & (1L<<ii))) score +=3;
				if ((o_ptr->may_flags1 & (1L<<ii)) && (a_ptr->flags1 & (1L<<ii))) score +=1;
			}
			else
			{
				if ((f1 & (1L<<ii)) && (n_ptr->can_flags1 & (1L<<ii))) score +=3;
				if ((o_ptr->may_flags1 & (1L<<ii)) && (n_ptr->can_flags1 & (1L<<ii))) score +=1;
			}
		}

		/* Award points on matching powers: 3 for have, 1 for may */
		for (ii=0;ii<32;ii++)
		{
			if (adult_lore)
			{
				if ((f2 & (1L<<ii)) && (a_ptr->flags2 & (1L<<ii))) score +=3;
				if ((o_ptr->may_flags2 & (1L<<ii)) && (a_ptr->flags2 & (1L<<ii))) score +=1;
			}
			else
			{
				if ((f2 & (1L<<ii)) && (n_ptr->can_flags2 & (1L<<ii))) score +=3;
				if ((o_ptr->may_flags2 & (1L<<ii)) && (n_ptr->can_flags2 & (1L<<ii))) score +=1;
			}
		}

		/* Award points on matching powers: 3 for have, 1 for may */
		for (ii=0;ii<32;ii++)
		{
			if (adult_lore)
			{
				if ((f3 & (1L<<ii)) && (a_ptr->flags3 & (1L<<ii))) score +=3;
				if ((o_ptr->may_flags3 & (1L<<ii)) && (a_ptr->flags3 & (1L<<ii))) score +=1;
			}
			else
			{
				if ((f3 & (1L<<ii)) && (n_ptr->can_flags3 & (1L<<ii))) score +=3;
				if ((o_ptr->may_flags3 & (1L<<ii)) && (n_ptr->can_flags3 & (1L<<ii))) score +=1;
			}
		}

		/* Award points on matching powers: 3 for have, 1 for may */
		for (ii=0;ii<32;ii++)
		{
			if (adult_lore)
			{
				if ((f4 & (1L<<ii)) && (a_ptr->flags4 & (1L<<ii))) score +=3;
				if ((o_ptr->may_flags4 & (1L<<ii)) && (a_ptr->flags4 & (1L<<ii))) score +=1;
			}
			else
			{
				if ((f4 & (1L<<ii)) && (n_ptr->can_flags4 & (1L<<ii))) score +=3;
				if ((o_ptr->may_flags4 & (1L<<ii)) && (n_ptr->can_flags4 & (1L<<ii))) score +=1;
			}
		}

		/* Artifacts have a minimum match threshold */
		if ((score > 6) && (score > high))
		{
			high = score;
			guess1 = i;
			guess2 = 0;
		}
		/* Hack -- force lowest depth items */
		else if ((score > 6) && (score == high) && (guess1))
		{
			artifact_type *a2_ptr = &a_info[guess1];

			if (a_ptr->level < a2_ptr->level) guess1 = i;
		}

	}

	/* Guessed an artifact? */
	if ((guess1) && (guess1 != o_ptr->guess1))
	{
		char o_name[80];

		/* Guess an artifact */
		o_ptr->guess1 = guess1;

		/* Describe the object */
		object_desc(o_name, sizeof(o_name), o_ptr, FALSE, 0);

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);

	}

	/* Guessed an ego item */
	else if ((guess2) && (guess2 != o_ptr->guess2))
	{
		char o_name[80];

		/* Guess an artifact */
		o_ptr->guess2 = guess2;

		/* Describe the object */
		object_desc(o_name, sizeof(o_name), o_ptr, FALSE, 0);

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);
	}

	/* Guessed a kind */
	else if ((guess3) && (guess3 != k_info[o_ptr->k_idx].guess))
	{
		char o_name[80];

		/* Guess an artifact */
		k_info[o_ptr->k_idx].guess = guess3;

		/* Guess is tried */
		k_info[o_ptr->k_idx].aware |= (AWARE_TRIED);

		/* Describe the object */
		object_desc(o_name, sizeof(o_name), o_ptr, FALSE, 0);

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);
	}
}


/*
 * Object does have flags.
 *
 * XXX The function object_can_flags assumes that the object we notice has a particular
 * set of flags is either wielded or in the inventory, and therefore we clear the
 * 'may_flags' that we use to track which objects have information with which we may
 * be able to deduce what flags that object has, and which do not.
 *
 * However, for instance, objects that are thrown from the floor, or are damaged by fire
 * but ignore it, or are damaging to a particular type of monster which fails to pick it
 * up, are some of the instances where we call this routine for objects other than those
 * held in the inventory.
 *
 * Currently this is fine as the flags are not those kind we attempt to track anyway eg
 * weapon flags, ignore flags and so on.
 *
 * XXX We might want to return TRUE from this routine if the flags we are adding are not
 * already known.
 *
 */
void object_can_flags(object_type *o_ptr, u32b f0[ABILITY_ARRAY_SIZE], u32b f1, u32b f2, u32b f3, u32b f4, bool floor)
{
	u32b xf0[ABILITY_ARRAY_SIZE];
	u32b xf1 = 0L, xf2 = 0L, xf3 = 0L, xf4 = 0L;
	int i,j;

	u32b tmpf0[ABILITY_ARRAY_SIZE];
	
	u32b if0[ABILITY_ARRAY_SIZE];
	u32b if1 = o_ptr->may_flags1 & (f1);
	u32b if2 = o_ptr->may_flags2 & (f2);
	u32b if3 = o_ptr->may_flags3 & (f3);
	u32b if4 = o_ptr->may_flags4 & (f4);

	object_kind *k_ptr = &k_info[o_ptr->k_idx];
	
	/* Prevent tainting array */
	for (i = 0; i < ABILITY_ARRAY_SIZE; i++) tmpf0[i] = (f0) ? f0[i] : 0L;

	/* Get f0 array */
	for (i = 0; i < ABILITY_ARRAY_SIZE; i++) if0[i] = (f0) ? o_ptr->may_flags0[i] & (f0[i]) : 0L;
	for (i = 0; i < ABILITY_ARRAY_SIZE; i++) xf0[i] = 0L;
	
	/* Clear not flags */
	if (f0) for (i = 0; i < ABILITY_ARRAY_SIZE; i++) o_ptr->not_flags0[i] &= ~(f0[i]);
	o_ptr->not_flags1 &= ~(f1);
	o_ptr->not_flags2 &= ~(f2);
	o_ptr->not_flags3 &= ~(f3);
	o_ptr->not_flags4 &= ~(f4);

	/* Clear may flags on all kit - include inventory */
	if (!floor) for (i = 0; i < INVEN_TOTAL; i++)
	{
		object_type *i_ptr = &inventory[i];

		/* Skip non-objects */
		if (!i_ptr->k_idx) continue;

		/* Clear may flags */
		for (j = 0; j < ABILITY_ARRAY_SIZE; j++) i_ptr->may_flags0[j] &= ~(if0[j]);
		i_ptr->may_flags1 &= ~(if1);
		i_ptr->may_flags2 &= ~(if2);
		i_ptr->may_flags3 &= ~(if3);
		i_ptr->may_flags4 &= ~(if4);
	}

	/* Mark can flags */
	if (f0) for (i = 0; i < ABILITY_ARRAY_SIZE; i++) o_ptr->can_flags0[i] |= (f0[i]); 
	o_ptr->can_flags1 |= (f1);
	o_ptr->can_flags2 |= (f2);
	o_ptr->can_flags3 |= (f3);
	o_ptr->can_flags4 |= (f4);

	/* If object flavored, learn flags about that flavor */
	if (!object_aware_p(o_ptr) && !(o_ptr->ident & IDENT_STORE) && (k_ptr->flavor))
	{
		/* Learn for base flavor */
		if (f0) for (i = 0; i < ABILITY_ARRAY_SIZE; i++) x_list[k_ptr->flavor].can_flags0[i] |= (f0[i]);
		x_list[k_ptr->flavor].can_flags1 |= (f1);
		x_list[k_ptr->flavor].can_flags2 |= (f2);
		x_list[k_ptr->flavor].can_flags3 |= (f3);
		x_list[k_ptr->flavor].can_flags4 |= (f4);

		/* Process inventory */
		for (i = 0; i < INVEN_TOTAL; i++)
		{
			object_type *i_ptr = &inventory[i];

			/* Skip non-objects */
			if (!i_ptr->k_idx) continue;

			/* Not matching kind */
			if (i_ptr->k_idx != o_ptr->k_idx) continue;

			if (f0) for (j = 0; j < ABILITY_ARRAY_SIZE; j++) i_ptr->can_flags0[j] |= (f0[j]);
			i_ptr->can_flags1 |= (f1);
			i_ptr->can_flags2 |= (f2);
			i_ptr->can_flags3 |= (f3);
			i_ptr->can_flags4 |= (f4);

			for (j = 0; j < ABILITY_ARRAY_SIZE; j++) o_ptr->can_flags0[j] |= i_ptr->can_flags0[j];
			o_ptr->can_flags1 |= i_ptr->can_flags1;
			o_ptr->can_flags2 |= i_ptr->can_flags2;
			o_ptr->can_flags3 |= i_ptr->can_flags3;
			o_ptr->can_flags4 |= i_ptr->can_flags4;

			/* Guess name */
			object_guess_name(i_ptr);
		}

		/* Process objects */
		for (i = 1; i < o_max; i++)
		{
			/* Get the object */
			object_type *i_ptr = &o_list[i];

			/* Skip dead objects */
			if (!i_ptr->k_idx) continue;

			/* Not matching kind */
			if (i_ptr->k_idx != o_ptr->k_idx) continue;

			if (f0) for (j = 0; j < ABILITY_ARRAY_SIZE; j++) i_ptr->can_flags0[j] |= (f0[j]);
			i_ptr->can_flags1 |= (f1);
			i_ptr->can_flags2 |= (f2);
			i_ptr->can_flags3 |= (f3);
			i_ptr->can_flags4 |= (f4);

			for (j = 0; j < ABILITY_ARRAY_SIZE; j++) o_ptr->can_flags0[j] |= i_ptr->can_flags0[j];
			o_ptr->can_flags1 |= i_ptr->can_flags1;
			o_ptr->can_flags2 |= i_ptr->can_flags2;
			o_ptr->can_flags3 |= i_ptr->can_flags3;
			o_ptr->can_flags4 |= i_ptr->can_flags4;

			/* Guess name */
			object_guess_name(i_ptr);
		}

		/* Guess name */
		object_guess_name(o_ptr);
	}

	/* If in store, stop here */
	if (o_ptr->ident & (IDENT_STORE)) return;

	/* Must be identified to continue */
	if (!object_named_p(o_ptr))
	{
		object_guess_name(o_ptr);

		return;
	}

	/* Hack -- Remove kind flags */
	/* This prevents Blades of Chaos 'tainting' ego items etc. */
	for (i = 0; i < k_ptr->ability_count; i++) tmpf0[k_ptr->ability[i]/32] &= ~(k_ptr->ability[i]%32);
	f1 &= ~(k_ptr->flags1);
	f2 &= ~(k_ptr->flags2);
	f3 &= ~(k_ptr->flags3);
	f4 &= ~(k_ptr->flags4);

	/* Hack -- Remove 'user' enchanted hidden flags */
	/* This prevents runes and enchantment spells 'tainting' ego items */

	/* Ignore coatings */
	if (coated_p(o_ptr))
	{
		/* No extra powers */
	}
	
	/* Ignore runes */
	if (runed_p(o_ptr))
	{
		int rune = o_ptr->xtra1 - OBJECT_XTRA_MIN_RUNES;
		int i;

		for (i = 0;i<MAX_RUNE_FLAGS;i++)
		{
			if ((y_info[rune].count[i]) && (y_info[rune].count[i]<= o_ptr->xtra2))
			{
				if (y_info[rune].flag[i] < 32) (f1) &= ~(1L << y_info[rune].flag[i]);

				if ((y_info[rune].flag[i] >= 32)
				 && (y_info[rune].flag[i] < 64)) (f2) &= ~(1L << (y_info[rune].flag[i]-32));

				if ((y_info[rune].flag[i] >= 64)
				 && (y_info[rune].flag[i] < 96)) (f3) &= ~(1L << (y_info[rune].flag[i]-64));

				if ((y_info[rune].flag[i] >= 96)
				 && (y_info[rune].flag[i] < 128)) (f4) &= ~(1L << (y_info[rune].flag[i]-96));
			}
		}
	}
	
	/* Extra powers */
	if (hidden_p(o_ptr))
	{
		if (object_xtra_what[o_ptr->xtra1] == 0)
		{
			i = (object_xtra_base[o_ptr->xtra1] << o_ptr->xtra2);
			xf0[i/32] = 1L << (i % 32);
			f0[i/32] &= ~xf0[i/32];
		}
		else if (object_xtra_what[o_ptr->xtra1] == 1)
		{
			xf1 = (object_xtra_base[o_ptr->xtra1] << o_ptr->xtra2);
			f1 &= ~xf1;
		}
		else if (object_xtra_what[o_ptr->xtra1] == 2)
		{
			xf2 = (object_xtra_base[o_ptr->xtra1] << o_ptr->xtra2);
			f2 &= ~xf2;
		}
		else if (object_xtra_what[o_ptr->xtra1] == 3)
		{
			xf3 = (object_xtra_base[o_ptr->xtra1] << o_ptr->xtra2);
			f3 &= ~xf3;
		}
		else if (object_xtra_what[o_ptr->xtra1] == 4)
		{
			xf4 = (object_xtra_base[o_ptr->xtra1] << o_ptr->xtra2);
			f4 &= ~xf4;
		}
	}

	/* Artifact */
	if (o_ptr->name1)
	{
		object_info *n_ptr = &a_list[o_ptr->name1];

		for (i = 0; i < ABILITY_ARRAY_SIZE; i++) n_ptr->not_flags0[i] &= ~(tmpf0[i]);		
		n_ptr->not_flags1 &= ~(f1);
		n_ptr->not_flags2 &= ~(f2);
		n_ptr->not_flags3 &= ~(f3);
		n_ptr->not_flags4 &= ~(f4);

		/* Fixed flags */
		for (i = 0; i < ABILITY_ARRAY_SIZE; i++) n_ptr->can_flags0[i] |= (tmpf0[i]);		
		n_ptr->can_flags1 |= (f1);
		n_ptr->can_flags2 |= (f2);
		n_ptr->can_flags3 |= (f3);
		n_ptr->can_flags4 |= (f4);
	}

	/* Ego item */
	else if (o_ptr->name2)
	{
		object_lore *n_ptr = &e_list[o_ptr->name2];

		for (i = 0; i < ABILITY_ARRAY_SIZE; i++) n_ptr->not_flags0[i] &= ~(tmpf0[i]);		
		n_ptr->not_flags1 &= ~(f1);
		n_ptr->not_flags2 &= ~(f2);
		n_ptr->not_flags3 &= ~(f3);
		n_ptr->not_flags4 &= ~(f4);

		/* Fixed flags */
		for (i = 0; i < ABILITY_ARRAY_SIZE; i++) n_ptr->can_flags0[i] |= (tmpf0[i]);		
		n_ptr->can_flags1 |= (f1);
		n_ptr->can_flags2 |= (f2);
		n_ptr->can_flags3 |= (f3);
		n_ptr->can_flags4 |= (f4);

		/* Extra flags */
		for (i = 0; i < ABILITY_ARRAY_SIZE; i++) n_ptr->may_flags0[i] |= xf0[i];		
		n_ptr->may_flags1 |= xf1;
		n_ptr->may_flags2 |= xf2;
		n_ptr->may_flags3 |= xf3;
		n_ptr->may_flags4 |= xf4;

		/* Exclude fixed flags */
		for (i = 0; i < ABILITY_ARRAY_SIZE; i++) n_ptr->may_flags0[i] &= ~(n_ptr->can_flags0[i]);
		n_ptr->may_flags1 &= ~(n_ptr->can_flags1);
		n_ptr->may_flags2 &= ~(n_ptr->can_flags2);
		n_ptr->may_flags3 &= ~(n_ptr->can_flags3);
		n_ptr->may_flags4 &= ~(n_ptr->can_flags4);
	}
}


/*
 * Check if inventory (including equipment) has only 1 item that may flags
 */
static void inven_may_flags()
{
	int i, j;

	u32b f0[ABILITY_ARRAY_SIZE];
	u32b f1 = 0x0L;
	u32b f2 = 0x0L;
	u32b f3 = 0x0L;
	u32b f4 = 0x0L;

	u32b nf0[ABILITY_ARRAY_SIZE];
	u32b nf1 = 0x0L;
	u32b nf2 = 0x0L;
	u32b nf3 = 0x0L;
	u32b nf4 = 0x0L;

	object_type *i_ptr;

	/* Get f0 array */
	for (i = 0; i < ABILITY_ARRAY_SIZE; i++) f0[i] = 0L;
	for (i = 0; i < ABILITY_ARRAY_SIZE; i++) nf0[i] = 0L;
	
	/* Check inventory may flags*/
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		i_ptr = &inventory[i];

		/* Skip non-objects */
		if (!i_ptr->k_idx) continue;

		for (j = 0; j< ABILITY_MAX; j++)
		{
			if (i_ptr->may_flags0[j/32] & (1L<<(j%32)))
			{
				if (!(nf0[j/32] & (1L<<(j%32)))) { nf0[j/32] |= (1L<<(j%32)); f0[j/32] |= (1L<<(j%32)); }
				else f0[j/32] &= ~(1L<<(j%32));
			}
		}

		for (j = 0; j< 32; j++)
		{
			if (i_ptr->may_flags1 & (1L<<j))
			{
				if (!(nf1 & (1L<<j))) { nf1 |= (1L<<j); f1 |= (1L<<j); }
				else f1 &= ~(1L<<j);
			}
		}

		for (j = 0; j< 32; j++)
		{
			if (i_ptr->may_flags2 & (1L<<j))
			{
				if (!(nf2 & (1L<<j))) { nf2 |= (1L<<j); f2 |= (1L<<j); }
				else f2 &= ~(1L<<j);
			}
		}

		for (j = 0; j< 32; j++)
		{
			if (i_ptr->may_flags3 & (1L<<j))
			{
				if (!(nf3 & (1L<<j))) { nf3 |= (1L<<j); f3 |= (1L<<j); }
				else f3 &= ~(1L<<j);
			}
		}

		for (j = 0; j< 32; j++)
		{
			if (i_ptr->may_flags4 & (1L<<j))
			{
				if (!(nf4 & (1L<<j))) { nf4 |= (1L<<j); f4 |= (1L<<j); }
				else f4 &= ~(1L<<j);
			}
		}
	}

	/* Check inventory may flags */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		bool may = FALSE;
		
		i_ptr = &inventory[i];

		/* Skip non-objects */
		if (!i_ptr->k_idx) continue;
		
		for (j = 0; j < ABILITY_ARRAY_SIZE; j++) may |= (i_ptr->may_flags0[i]) != 0;

		if ((may) || ((f1 & (i_ptr->may_flags1)) != 0) || ((f2 & (i_ptr->may_flags2)) != 0)
			|| ((f3 & (i_ptr->may_flags3)) != 0) || ((f4 & (i_ptr->may_flags4)) != 0))
		{
			u32b tmpf0[ABILITY_ARRAY_SIZE];
			
			for (j = 0; j < ABILITY_ARRAY_SIZE;j++) tmpf0[j] = f0[j] & (i_ptr->may_flags0[j]);
			
			update_slot_flags(i,tmpf0, f1 & (i_ptr->may_flags1),f2 & (i_ptr->may_flags2),f3 & (i_ptr->may_flags3),f4 & (i_ptr->may_flags4));
		}
	}
}

/*
 * Check if equipment has only 1 item that may flags
 */
static void equip_may_flags(u32b f0[ABILITY_ARRAY_SIZE], u32b f1, u32b f2, u32b f3, u32b f4)
{
	int i, j;

	u32b if0[ABILITY_ARRAY_SIZE];
	u32b if1 = 0x0L;
	u32b if2 = 0x0L;
	u32b if3 = 0x0L;
	u32b if4 = 0x0L;

	u32b nf0[ABILITY_ARRAY_SIZE];
	u32b nf1 = 0x0L;
	u32b nf2 = 0x0L;
	u32b nf3 = 0x0L;
	u32b nf4 = 0x0L;

	object_type *i_ptr;

	/* Get f0 array */
	for (i = 0; i < ABILITY_ARRAY_SIZE; i++) if0[i] = 0L;
	for (i = 0; i < ABILITY_ARRAY_SIZE; i++) nf0[i] = 0L;

	/* Check inventory may flags*/
	for (i = INVEN_WIELD; i < END_EQUIPMENT; i++)
	{
		i_ptr = &inventory[i];

		/* Skip non-objects */
		if (!i_ptr->k_idx) continue;

		for (j = 0; j< ABILITY_MAX; j++)
		{
			if (i_ptr->may_flags0[j/32] & (1L<<(j%32)))
			{
				if (!(nf0[j/32] & (1L<<(j%32)))) { nf0[j/32] |= (1L<<(j%32)); if0[j/32] |= (1L<<(j%32)); }
				else if0[j/32] &= ~(1L<<(j%32));
			}
		}

		for (j = 0; j< 32; j++)
		{
			if (i_ptr->may_flags1 & (1L<<j))
			{
				if (!(nf1 & (1L<<j))) { nf1 |= (1L<<j); if1 |= (1L<<j); }
				else if1 &= ~(1L<<j);
			}
		}

		for (j = 0; j< 32; j++)
		{
			if (i_ptr->may_flags2 & (1L<<j))
			{
				if (!(nf2 & (1L<<j))) { nf2 |= (1L<<j); if2 |= (1L<<j); }
				else if2 &= ~(1L<<j);
			}
		}

		for (j = 0; j< 32; j++)
		{
			if (i_ptr->may_flags3 & (1L<<j))
			{
				if (!(nf3 & (1L<<j))) { nf3 |= (1L<<j); if3 |= (1L<<j); }
				else if3 &= ~(1L<<j);
			}
		}

		for (j = 0; j< 32; j++)
		{
			if (i_ptr->may_flags4 & (1L<<j))
			{
				if (!(nf4 & (1L<<j))) { nf4 |= (1L<<j); if4 |= (1L<<j); }
				else if4 &= ~(1L<<j);
			}
		}
	}

	/* Only check passed flags */
	if (f0) for (i = 0; i < ABILITY_ARRAY_SIZE; i++) if0[i] &= (f0[i]);
	else for (i = 0; i < ABILITY_ARRAY_SIZE; i++) if0[i] = 0L;
	if1 &= (f1);
	if2 &= (f2);
	if3 &= (f3);
	if4 &= (f4);

	/* Nothing unknown */
	if (!if1 && !if2 && !if3 && !if4)
	{
		for (i = 0; i < ABILITY_ARRAY_SIZE; i++)
		{
			if (if0[i]) break;
		}
		
		if (i == ABILITY_ARRAY_SIZE) return;
	}

	/* Check equipment may flags*/
	for (i = INVEN_WIELD; i < END_EQUIPMENT; i++)
	{
		bool may = FALSE;
		
		i_ptr = &inventory[i];

		/* Skip non-objects */
		if (!i_ptr->k_idx) continue;
		
		for (j = 0; j < ABILITY_ARRAY_SIZE; j++) may |= (i_ptr->may_flags0[i]) != 0;

		if ((may) || ((f1 & (i_ptr->may_flags1)) != 0) || ((f2 & (i_ptr->may_flags2)) != 0)
			|| ((f3 & (i_ptr->may_flags3)) != 0) || ((f4 & (i_ptr->may_flags4)) != 0))
		{
			u32b tmpf0[ABILITY_ARRAY_SIZE];
			
			for (j = 0; j < ABILITY_ARRAY_SIZE;j++) tmpf0[j] = f0[j] & (i_ptr->may_flags0[j]);
			
			update_slot_flags(i,tmpf0, f1 & (i_ptr->may_flags1),f2 & (i_ptr->may_flags2),f3 & (i_ptr->may_flags3),f4 & (i_ptr->may_flags4));
		}
	}
}


/*
 * Object does not have flags.
 */
void object_not_flags(object_type *o_ptr, u32b f0[ABILITY_ARRAY_SIZE], u32b f1, u32b f2, u32b f3, u32b f4, bool floor)
{
	int i, j;
	bool oops = FALSE;
	u32b tmpf0[ABILITY_ARRAY_SIZE];

	object_kind *k_ptr = &k_info[o_ptr->k_idx];
	
	for (i = 0; i < ABILITY_ARRAY_SIZE; i++) tmpf0[i] = 0L;
	
	/* No change */
	if (((f1 & ~(o_ptr->not_flags1)) != 0) && ((f2 & ~(o_ptr->not_flags2)) != 0) && ((f3 & ~(o_ptr->not_flags3)) != 0) && ((f4 & ~(o_ptr->not_flags4)) != 0))
	{
		for (i = 0; i < ABILITY_ARRAY_SIZE; i++)
		{
				if ((f0[i] & ~(o_ptr->not_flags0[i])) != 0) break;
		}
			
		if (i == ABILITY_ARRAY_SIZE) return;
	}

	/* Mark not flags */
	if (f0) for (i = 0; i < ABILITY_ARRAY_SIZE; i++) o_ptr->not_flags0[i] |= (f0[i]);
	o_ptr->not_flags1 |= (f1);
	o_ptr->not_flags2 |= (f2);
	o_ptr->not_flags3 |= (f3);
	o_ptr->not_flags4 |= (f4);

	/* Clear may flags */
	if (f0) for (i = 0; i < ABILITY_ARRAY_SIZE; i++) o_ptr->may_flags0[i] &= ~(f0[i]);
	o_ptr->may_flags1 &= ~(f1);
	o_ptr->may_flags2 &= ~(f2);
	o_ptr->may_flags3 &= ~(f3);
	o_ptr->may_flags4 &= ~(f4);

	/* Oops */
	if (f0) for (i = 0; i < ABILITY_ARRAY_SIZE; i++)
	{
		tmpf0[i] = f0[i] & (o_ptr->can_flags0[i]);
		
		if (tmpf0[i]) oops = TRUE;
	}
	
	/* Oops */
	if ((oops) || (f1 & (o_ptr->can_flags1)) || (f2 & (o_ptr->can_flags2)) || (f3 & (o_ptr->can_flags3)) || (f4 & (o_ptr->can_flags4)))
	{
		s16b ability[ABILITY_MAX];
		
		msg_print("BUG: Forgetting something on an object we shouldn't forget. Please report.");

		/* Set text_out hook */
		text_out_hook = text_out_to_screen;

		/* Load screen */
		screen_save();

		/* Begin recall */
		Term_gotoxy(0, 1);
		
		/* Get abilities */
		object_eval(o_ptr, ability);

		/* Actually display the item */
		list_object_flags(ability, tmpf0, f1 & (o_ptr->can_flags1), f2 & (o_ptr->can_flags2), f3 & (o_ptr->can_flags3), f4 & (o_ptr->can_flags4), o_ptr->pval, LIST_FLAGS_CAN);

		(void)anykey();

		/* Load screen */
		screen_load();

		msg_format("%ld:%ld:%ld:%ld", f1 & (o_ptr->can_flags1), f2 & (o_ptr->can_flags2), f3 & (o_ptr->can_flags3), f4 & (o_ptr->can_flags4));
	}

	/* Clear can flags */
	if (f0) for (i = 0; i < ABILITY_ARRAY_SIZE; i++) o_ptr->can_flags0[i] &= ~(f0[i]);
	o_ptr->can_flags1 &= ~(f1);
	o_ptr->can_flags2 &= ~(f2);
	o_ptr->can_flags3 &= ~(f3);
	o_ptr->can_flags4 &= ~(f4);

	/* If object flavored, learn flags about that flavor */
	if (!object_aware_p(o_ptr) && !(o_ptr->ident & (IDENT_STORE)) && (k_ptr->flavor))
	{
		if (f0) for (i = 0; i < ABILITY_ARRAY_SIZE; i++) x_list[k_ptr->flavor].not_flags0[i] |= (f0[i]);		
		x_list[k_ptr->flavor].not_flags1 |= (f1);
		x_list[k_ptr->flavor].not_flags2 |= (f2);
		x_list[k_ptr->flavor].not_flags3 |= (f3);
		x_list[k_ptr->flavor].not_flags4 |= (f4);

		/* Process inventory */
		for (i = 0; i < INVEN_TOTAL; i++)
		{
			object_type *i_ptr = &inventory[i];

			/* Skip non-objects */
			if (!i_ptr->k_idx) continue;

			/* Not matching kind */
			if (i_ptr->k_idx != o_ptr->k_idx) continue;

			if (f0) for (j = 0; j < ABILITY_ARRAY_SIZE; j++) i_ptr->not_flags0[j] |= (f0[j]);					
			i_ptr->not_flags1 |= (f1);
			i_ptr->not_flags2 |= (f2);
			i_ptr->not_flags3 |= (f3);
			i_ptr->not_flags4 |= (f4);

			/* Important -- have to clear may flags on inventory */
			if (f0) for (j = 0; j < ABILITY_ARRAY_SIZE; j++) i_ptr->may_flags0[j] &= ~(f0[j]);					
			i_ptr->may_flags1 &= ~(f1);
			i_ptr->may_flags2 &= ~(f2);
			i_ptr->may_flags3 &= ~(f3);
			i_ptr->may_flags4 &= ~(f4);

			/* Guess name */
			object_guess_name(i_ptr);
		}

		/* Process objects */
		for (i = 1; i < o_max; i++)
		{
			/* Get the object */
			object_type *i_ptr = &o_list[i];

			/* Skip dead objects */
			if (!i_ptr->k_idx) continue;

			/* Not matching kind */
			if (i_ptr->k_idx != o_ptr->k_idx) continue;

			if (f0) for (j = 0; j < ABILITY_ARRAY_SIZE; j++) i_ptr->not_flags0[j] |= (f0[j]);
			i_ptr->not_flags1 |= (f1);
			i_ptr->not_flags2 |= (f2);
			i_ptr->not_flags3 |= (f3);
			i_ptr->not_flags4 |= (f4);

			/* Guess name */
			object_guess_name(i_ptr);
		}
	}

	/* Stop here if item in a store */
	if (o_ptr->ident & (IDENT_STORE)) return;

	/* Check inventory */
	if (!floor) inven_may_flags();

	/* Must be identified to continue */
	if (!object_named_p(o_ptr))
	{
		object_guess_name(o_ptr);

		return;
	}

	/* Artifact */
	if (o_ptr->name1)
	{
		object_info *n_ptr = &a_list[o_ptr->name1];

		if (f0) for (i = 0; i < ABILITY_ARRAY_SIZE; i++) n_ptr->not_flags0[i] |= (f0[i]);
		n_ptr->not_flags1 |= f1;
		n_ptr->not_flags2 |= f2;
		n_ptr->not_flags3 |= f3;
		n_ptr->not_flags4 |= f4;

		if (f0) for (i = 0; i < ABILITY_ARRAY_SIZE; i++) n_ptr->can_flags0[i] &= ~(f0[i]);
		n_ptr->can_flags1 &= ~(f1);
		n_ptr->can_flags2 &= ~(f2);
		n_ptr->can_flags3 &= ~(f3);
		n_ptr->can_flags4 &= ~(f4);
	}

	/* Ego item */
	else if (o_ptr->name2)
	{
		object_lore *n_ptr = &e_list[o_ptr->name2];

		if (f0) for (i = 0; i < ABILITY_ARRAY_SIZE; i++) n_ptr->not_flags0[i] |= (f0[i]);
		n_ptr->not_flags1 |= f1;
		n_ptr->not_flags2 |= f2;
		n_ptr->not_flags3 |= f3;
		n_ptr->not_flags4 |= f4;

		if (f0) for (i = 0; i < ABILITY_ARRAY_SIZE; i++) n_ptr->can_flags0[i] &= ~(f0[i]);
		n_ptr->can_flags1 &= ~(f1);
		n_ptr->can_flags2 &= ~(f2);
		n_ptr->can_flags3 &= ~(f3);
		n_ptr->can_flags4 &= ~(f4);
	}
}


/*
 * Object may have these flags. If only object in equipment
 * to do so, will have these flags. Use for object absorbtion.s
 */
void object_may_flags(object_type *o_ptr, u32b f0[ABILITY_ARRAY_SIZE], u32b f1,u32b f2,u32b f3, u32b f4, bool floor)
{
	int i;
	u32b tmpf0[ABILITY_ARRAY_SIZE];
	
	/* Important: Object must be in inventory to track may flags */
	if (floor) return;
	
	for (i = 0; i < ABILITY_ARRAY_SIZE; i++) tmpf0[i] = (f0 != 0) ? f0[i] : 0L;

	/* Clear bits with not flags */
	if (f0) for (i = 0; i < ABILITY_ARRAY_SIZE; i++) tmpf0[i] &= ~(o_ptr->not_flags0[i]);
	f1 &= ~(o_ptr->not_flags1);
	f2 &= ~(o_ptr->not_flags2);
	f3 &= ~(o_ptr->not_flags3);
	f4 &= ~(o_ptr->not_flags4);

	/* Clear bits with can flags */
	if (f0) for (i = 0; i < ABILITY_ARRAY_SIZE; i++) tmpf0[i] &= ~(o_ptr->can_flags0[i]);
	else tmpf0[i] = 0L;
	f1 &= ~(o_ptr->can_flags1);
	f2 &= ~(o_ptr->can_flags2);
	f3 &= ~(o_ptr->can_flags3);
	f4 &= ~(o_ptr->can_flags4);

	/* Mark may flags */
	if (f0) for (i = 0; i < ABILITY_ARRAY_SIZE; i++) o_ptr->may_flags0[i] |= (tmpf0[i]);
	o_ptr->may_flags1 |= (f1);
	o_ptr->may_flags2 |= (f2);
	o_ptr->may_flags3 |= (f3);
	o_ptr->may_flags4 |= (f4);

	/* Check the inventory */
	inven_may_flags();

	/* Must be identified to continue */
	if (!object_named_p(o_ptr))
	{
		object_guess_name(o_ptr);
	}
}


/*
 * Object forgets all may flags
 */
void drop_may_flags(object_type *o_ptr)
{
	int i;
	
	/* Clear may flags */
	for (i = 0; i < ABILITY_ARRAY_SIZE; i++) o_ptr->may_flags0[i] = 0L;
	o_ptr->may_flags1 = 0L;
	o_ptr->may_flags2 = 0L;
	o_ptr->may_flags3 = 0L;
	o_ptr->may_flags4 = 0L;

	return;
}

/*
 * Object forgets all flags
 */
void drop_all_flags(object_type *o_ptr)
{
	int i;
	
	/* Clear may flags */
	for (i = 0; i < ABILITY_ARRAY_SIZE; i++) o_ptr->can_flags0[i] = 0L;
	o_ptr->can_flags1 = 0L;
	o_ptr->can_flags2 = 0L;
	o_ptr->can_flags3 = 0L;
	o_ptr->can_flags4 = 0L;

	/* Clear may flags */
	for (i = 0; i < ABILITY_ARRAY_SIZE; i++) o_ptr->may_flags0[i] = 0L;
	o_ptr->may_flags1 = 0L;
	o_ptr->may_flags2 = 0L;
	o_ptr->may_flags3 = 0L;
	o_ptr->may_flags4 = 0L;

	/* Clear may flags */
	for (i = 0; i < ABILITY_ARRAY_SIZE; i++) o_ptr->not_flags0[i] = 0L;
	o_ptr->not_flags1 = 0L;
	o_ptr->not_flags2 = 0L;
	o_ptr->not_flags3 = 0L;
	o_ptr->not_flags4 = 0L;

	return;
}

/*
 * Usage count for an object
 */
void object_usage(int slot)
{
	object_type *o_ptr;

	char o_name[80];

	bool sense = FALSE;
	bool bonus = FALSE;
	bool heavy = FALSE;

	if (slot >=0) o_ptr =&inventory[slot];
	else o_ptr=&o_list[0-slot];

	if (!o_ptr->k_idx) return;

	/* No sensing when confused */
	if (p_ptr->timed[TMD_CONFUSED]) return;

	/* No sensing when hallucinating */
	if (p_ptr->timed[TMD_IMAGE]) return;

	if ((o_ptr->usage)<MAX_SHORT) o_ptr->usage++;

	/* Don't identify if fully known */
	if (o_ptr->ident & (IDENT_MENTAL)) return;

	/* Don't identify if identified */
	if (object_known_p(o_ptr)) return;

	/* Describe the object */
	object_desc(o_name, sizeof(o_name), o_ptr, FALSE, 0);

	/* Valid "tval" codes */
	switch (o_ptr->tval)
	{
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		case TV_DIGGING:
		case TV_STAFF:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		{
			/* Suggested calculation for weapons is 1 + (dd * ds ^ 1.5) / 4 from
			 * http://angband.oook.cz/forum/showthread.php?t=1135&highlight=weapon+calculation&page=2 */
			if (!object_bonus_p(o_ptr) && (o_ptr->usage >= (1 + (object_ability(o_ptr, ABILITY_DAMAGE_DICE) * object_ability(o_ptr, ABILITY_DAMAGE_SIDES) * (object_ability(o_ptr, ABILITY_DAMAGE_SIDES) / 2)) / 4))) bonus = TRUE;

			/* Heavy sense if half-way there */
			else if (o_ptr->usage > (1 + (object_ability(o_ptr, ABILITY_DAMAGE_DICE) * object_ability(o_ptr, ABILITY_DAMAGE_SIDES) * (object_ability(o_ptr, ABILITY_DAMAGE_SIDES) / 2)) / 4) / 2) heavy = TRUE;

			/* Sense if one third of way there */
			else if (o_ptr->usage == (1 + (object_ability(o_ptr, ABILITY_DAMAGE_DICE) * object_ability(o_ptr, ABILITY_DAMAGE_SIDES) * (object_ability(o_ptr, ABILITY_DAMAGE_SIDES) / 2)) / 4) / 3) sense = TRUE;

			break;
		}

		case TV_BOW:
		{
			/* Calculation is 'simple' */
			if (!object_bonus_p(o_ptr) && (o_ptr->usage > 6)) bonus = TRUE;

			/* Heavy sense if half-way there */
			else if (o_ptr->usage > 3) heavy = TRUE;

			/* Sense if one-third of way there */
			else if (o_ptr->usage == 2) sense = TRUE;

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
			/* Calculation is based on the fact that base armour outweighs bonus */
			if (!object_bonus_p(o_ptr) && (o_ptr->usage >= object_aval(o_ptr, ABILITY_AC) /2)) bonus = TRUE;

			/* Heavy sense if half-way there */
			else if (o_ptr->usage > object_aval(o_ptr, ABILITY_AC) / 4) heavy = TRUE;

			/* Sense if one-third of way there */
			else if (o_ptr->usage == 1 + object_aval(o_ptr, ABILITY_AC) / 6) sense = TRUE;

			break;
		}
	}

	/* Fully sense bonus */
	if (bonus)
	{
		/* Describe what we know */
		msg_format("You feel you know more about the %s you are %s.",o_name,describe_use(slot));

		/* Mark the item as partially known */
		object_bonus(o_ptr, slot < 0);

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);
	}

	/* Sense the item if we don't get an exact match */
	if (((sense) || (heavy)) && !(o_ptr->ident & (IDENT_SENSE)))
	{
		/* Check for a feeling */
		if (sense_magic(o_ptr, 1, heavy, slot < 0))
		{
			/* Describe what we know */
			msg_format("You feel you know more about the %s you are %s.",o_name,describe_use(slot));

			/* Describe the feeling */
			msg_format("%s", inscrip_info[o_ptr->feeling]);

			/* Auto-id average items */
			if (o_ptr->feeling == INSCRIP_AVERAGE) object_bonus(o_ptr, slot < 0);

			/* Combine / Reorder the pack (later) */
			p_ptr->notice |= (PN_COMBINE | PN_REORDER);

			/* Window stuff */
			p_ptr->window |= (PW_INVEN | PW_EQUIP);
		}
	}
}


/*
 * Slot holds an object with these flags. Inform the player.
 */
void update_slot_flags(int slot, u32b f0[ABILITY_ARRAY_SIZE], u32b f1, u32b f2, u32b f3, u32b f4)
{
	char o_name[80];
	s16b ability[ABILITY_MAX];

	object_type *i_ptr;

	/* Get the item */
	if (slot >=0) i_ptr = &inventory[slot];
	else i_ptr= &o_list[0-slot];

	/* Update the object */
	object_can_flags(i_ptr,f0,f1,f2,f3,f4, slot < 0);

	/* Describe the object */
	object_desc(o_name, sizeof(o_name), i_ptr, FALSE, 0);

	/* Describe what we now know */
	msg_format("You feel you know the %s you are %s better...",o_name, describe_use(slot));

	/* Set text_out hook */
	text_out_hook = text_out_to_screen;

	/* Load screen */
	screen_save();

	/* Begin recall */
	Term_gotoxy(0, 1);
	
	/* Get all values */
	object_eval(i_ptr, ability);

	/* Actually display the item */
	list_object_flags(ability, f0, f1, f2, f3, f4, i_ptr->ident & (IDENT_PVAL | IDENT_MENTAL | IDENT_KNOWN) ? i_ptr->pval : 0, LIST_FLAGS_CAN);

	(void)anykey();

	/* Load screen */
	screen_load();

}

/*
 * Equipment must have these flags.
 * Note with ALLOW_OBJECT_INFO_MORE, this is
 * an efficiency bottleneck.
 */
void equip_can_flags(u32b f0[ABILITY_ARRAY_SIZE], u32b f1,u32b f2,u32b f3, u32b f4)
{
	u32b nf0[ABILITY_ARRAY_SIZE];
	u32b nf1;
	u32b nf2;
	u32b nf3;
	u32b nf4;
	
	u32b tmpf0[ABILITY_ARRAY_SIZE];

	int i, j;

	object_type *i_ptr;

	/* Don't overwrite original array */
	for (i = 0; i < ABILITY_ARRAY_SIZE; i++) tmpf0[i] = f0[i];
	
	/* Hack --- exclude player flags */
	player_flags(nf0, &nf1,&nf2,&nf3,&nf4);

	for (i = 0; i < ABILITY_ARRAY_SIZE; i++) tmpf0[i] &= ~(nf0[i]);
	f1 &= ~(nf1);
	f2 &= ~(nf2);
	f3 &= ~(nf3);
	f4 &= ~(nf4);
#if 0
	/* Hack --- exclude temporary effect flags */
	if (p_ptr->timed[TMD_INFRA]) f1 &= ~(TR1_INFRA);
	if ((p_ptr->timed[TMD_HERO]) || (p_ptr->timed[TMD_BERSERK])) f2 &= ~(TR2_RES_FEAR);
	if (p_ptr->timed[TMD_SEE_INVIS]) f3 &= ~(TR3_SEE_INVIS);
	if (p_ptr->timed[TMD_BLESSED]) f3 &= ~(TR3_HOLD_LIFE);
#endif
	/* Exclude known flags */
	for (i = INVEN_WIELD; i < END_EQUIPMENT; i++)
	{
		i_ptr = &inventory[i];

		/* Skip non-objects */
		if (!i_ptr->k_idx) continue;

		/* Clear bits with can flags */
		for (j = 0; j < ABILITY_ARRAY_SIZE; j++) tmpf0[i] &= ~(i_ptr->can_flags0[0]);
		f1 &= ~(i_ptr->can_flags1);
		f2 &= ~(i_ptr->can_flags2);
		f3 &= ~(i_ptr->can_flags3);
		f4 &= ~(i_ptr->can_flags4);
	}

	/* Nothing unknown */
	if (!f1 && !f2 && !f3 && !f4)
	{
		for (i = 0; i < ABILITY_ARRAY_SIZE; i++)
		{
			if (tmpf0[i]) break;
		}
		
		if (i == ABILITY_ARRAY_SIZE) return;
	}

	/* Check for flags */
	for (i = INVEN_WIELD; i < END_EQUIPMENT; i++)
	{
		u32b if0[ABILITY_ARRAY_SIZE];
		u32b if1 = f1;
		u32b if2 = f2;
		u32b if3 = f3;
		u32b if4 = f4;

		bool guess = FALSE;

		i_ptr = &inventory[i];

		/* Skip non-objects */
		if (!i_ptr->k_idx) continue;

		/* Set up f0 array */
		for (j = 0; j < ABILITY_ARRAY_SIZE; j++) if0[j] = tmpf0[j];

		/* Clear bits with not flags */
		for (j = 0; j < ABILITY_ARRAY_SIZE; j++) if0[j] &= ~(i_ptr->not_flags0[j]);
		if1 &= ~(i_ptr->not_flags1);
		if2 &= ~(i_ptr->not_flags2);
		if3 &= ~(i_ptr->not_flags3);
		if4 &= ~(i_ptr->not_flags4);

		/* Do we guess again ? */
		for (j = 0; j < ABILITY_ARRAY_SIZE; j++) guess |= if0[j] &=  (if0[j] & ~(i_ptr->may_flags0[j])) != 0;		
		guess |= ((if1 & ~(i_ptr->may_flags1)) != 0) || ((if2 & ~(i_ptr->may_flags2)) != 0)
			|| ((if3 & ~(i_ptr->may_flags3)) != 0) || ((if4 & ~(i_ptr->may_flags4)) != 0);

		/* Mark may flags */
		for (j = 0; j < ABILITY_ARRAY_SIZE; j++) i_ptr->not_flags0[j] |= (if0[j]);
		i_ptr->may_flags1 |= (if1);
		i_ptr->may_flags2 |= (if2);
		i_ptr->may_flags3 |= (if3);
		i_ptr->may_flags4 |= (if4);

		/* Must be identified to continue */
		if ((guess) && (!object_named_p(i_ptr)))
		{
			object_guess_name(i_ptr);
		}
	}

	/* Check inventory */
	equip_may_flags(tmpf0, f1, f2, f3, f4);
}


/*
 * Equipment does not have these flags
 */
void equip_not_flags(u32b f0[ABILITY_ARRAY_SIZE], u32b f1,u32b f2,u32b f3, u32b f4)
{
	int i;

	object_type *i_ptr;

	player_race *shape_ptr = &p_info[p_ptr->pshape];

	u32b tmpf0[ABILITY_ARRAY_SIZE];

	/* Don't overwrite original array */
	for (i = 0; i < ABILITY_ARRAY_SIZE; i++) tmpf0[i] = f0[i];

	/*** Handle shape prohibitions ***/
	for (i = 0; i < ABILITY_ARRAY_SIZE; i++) tmpf0[i] &= ~(shape_ptr->cancel_flags0[i]);
	f1 &= ~(shape_ptr->cancel_flags1);
	f2 &= ~(shape_ptr->cancel_flags2);
	f3 &= ~(shape_ptr->cancel_flags3);
	f4 &= ~(shape_ptr->cancel_flags4);

	/* Mark equipment with not flags*/
	for (i = INVEN_WIELD; i < END_EQUIPMENT; i++)
	{
		i_ptr = &inventory[i];

		/* Skip non-objects */
		if (!i_ptr->k_idx) continue;

		object_not_flags(i_ptr,tmpf0,f1,f2,f3,f4, TRUE);
	}
}


/*
 * Equipment dropped off (forget all equipped/inventory may flags
 * on objects still held)
 */
void inven_drop_flags(object_type *o_ptr)
{
	int i, j;
	u32b f0[ABILITY_ARRAY_SIZE];
	u32b f1 = o_ptr->may_flags1;
	u32b f2 = o_ptr->may_flags2;
	u32b f3 = o_ptr->may_flags3;
	u32b f4 = o_ptr->may_flags4;

	object_type *i_ptr;

	/* Get may flags */
	for (i = 0; i < ABILITY_ARRAY_SIZE; i++) f0[i] = o_ptr->may_flags0[i];
	
	/* Nothing unknown */
	if (!f1 && !f2 && !f3 && !f4)
	{
		for (i = 0; i < ABILITY_ARRAY_SIZE; i++)
		{
			if (f0[i]) break;
		}
		
		if (i == ABILITY_ARRAY_SIZE) return;
	}

	/* Clear equipment may flags*/
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		i_ptr = &inventory[i];

		/* Skip non-objects */
		if (!i_ptr->k_idx) continue;

		for (j = 0; j < ABILITY_ARRAY_SIZE; j++) i_ptr->may_flags0[j] &= ~(f0[j]);		
		i_ptr->may_flags1 &= ~(f1);
		i_ptr->may_flags2 &= ~(f2);
		i_ptr->may_flags3 &= ~(f3);
		i_ptr->may_flags4 &= ~(f4);
	}
}


#define sign(x)	((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))


/*
 * Calculate the rating for a given slay combination.
 *
 * The returned value needs to be divided by tot_mon_power to
 * give a normalised value.
 *
 * XXX Note we should be careful using this function, as it
 * is expensive. When we look it up frequently, such as during
 * initialisation and generation of random artifacts we
 * maintain a cache of values, so that we minimise the number
 * of calculations required.
 *
 * After this, we should never have to call it again, as we
 * cache the relevant values for magic items and ego items,
 * and store the computed artifact power with the artifact.
 */
s32b slay_power(u32b s_index)
{
	s32b sv;
	int i;
	u32b j;
	int mult;
	monster_race *r_ptr;

	u32b rf2_slay = 0x00000000L;
	u32b rf3_slay = 0x00000000L;
	u32b rf9_slay = 0x00000000L;
	u32b rf2_brand = 0x00000000L;
	u32b rf3_brand = 0x00000000L;
	u32b rf9_brand = 0x00000000L;

	/* s_index combines the slay bytes into an index value. It is determined
	 * using the slay_index function below.
	 */

	/* Check the cache */
	if (slays)
	{
		/* Look in the cache to see if we know this one yet */
		sv = slays[s_index];

		/* If it's cached, return its value */
		if (sv) return slays[s_index];
	}

	/* Precompute matches */
	for (i = 0, j = 0x00000000L; i < 32; i++, j <<= 1L)
	{
		if ((s_index & j) == 0) continue;

		if (ability_bonus[slay_index_to_ability[i]].type == BONUS_SLAY)
		{
			switch(ability_bonus[slay_index_to_ability[i]].flag_num)
			{
				case 2:
					rf2_slay |= ability_bonus[slay_index_to_ability[i]].flag_match;
					break;
				case 3:
					rf3_slay |= ability_bonus[slay_index_to_ability[i]].flag_match;
					break;
				case 9:
					rf9_slay |= ability_bonus[slay_index_to_ability[i]].flag_match;
					break;
			}
		}

		else if (ability_bonus[slay_index_to_ability[i]].type >= BONUS_BRAND)
		{
			switch(ability_bonus[slay_index_to_ability[i]].flag_num)
			{
				case 2:
					rf2_brand |= ability_bonus[slay_index_to_ability[i]].flag_match;
					break;
				case 3:
					rf3_brand |= ability_bonus[slay_index_to_ability[i]].flag_match;
					break;
				case 9:
					rf9_brand |= ability_bonus[slay_index_to_ability[i]].flag_match;
					break;
			}
		}
	}


	/* Otherwise we need to calculate the expected average multiplier
	 * for this combination (multiplied by the total number of
	 * monsters, which we'll divide out later). Note that we only ever
	 * have to compute a multiplier of 2 as we can determine higher multipliers
	 * through clever use of algebra.
	 *
	 * XXX We could equally compute 'partial' resistances by bumping the multiple
	 * up to 3, and using partial resistance as a multiple of 2. Examples include
	 * monsters with oppose element, and monsters who are likely to be encountered
	 * in water. This is unlikely to affect things much in the big scheme of things,
	 * and will increase the number of comparisons required in what is probably
	 * an 'expensive' function (there's been a considerable amount of effort to
	 * cache the results of this function, so I'm guessing it is expensive).
	 */

	sv = 0;

	for (i = 0; i < z_info->r_max; i++) {

		mult = 1;

		r_ptr = &r_info[i];

		/* Any matching slays */
		if ( (r_ptr->flags2 & (rf2_slay)) != 0) mult = 2;
		if ( (r_ptr->flags3 & (rf3_slay)) != 0) mult = 2;
		if ( (r_ptr->flags9 & (rf9_slay)) != 0) mult = 2;

		/* Any missing brand resistances */
		if ( (r_ptr->flags2 & (rf2_brand)) != rf2_brand) mult = 2;
		if ( (r_ptr->flags3 & (rf3_brand)) != rf3_brand) mult = 2;
		if ( (r_ptr->flags9 & (rf9_brand)) != rf9_brand) mult = 2;

		/* Add the multiple to sv */
		sv += mult * r_info[i].power;

		/* End loop */
	}

	/* Caching the values? */
	if (slays)
	{
		/* Add to the cache */
		slays[s_index] = sv;
	}

	return sv;

	/* End method */
}

/*
 * Convert all slay and brand flags into a single index value. This is used in slay_power, above.
 *
 * The order is dependent on the ability_bonus table, and at initialisation is:
 * 	ABILITY_SLAY_DEMON	0x00000001L
 *	ABILITY_SLAY_ORC	0x00000002L
 *  ABILITY_SLAY_TROLL	0x00000004L
 *	ABILITY_SLAY_GIANT	0x00000008L
 * 	ABILITY_SLAY_DRAGON 0x00000010L
 *	ABILITY_SLAY_ANIMAL	0x00000020L
 *  ABILITY_SLAY_UNDEAD	0x00000040L
 *	ABILITY_BRAND_HOLY	0x00000080L
 *	ABILITY_BRAND_POIS	0x00000100L
 *	ABILITY_BRAND_ACID	0x00000200L
 *	ABILITY_BRAND_ELEC	0x00000400L
 *	ABILITY_BRAND_FIRE	0x00000800L
 *	ABILITY_BRAND_COLD	0x00001000L
 *	ABILITY_SLAY_PLANT	0x00002000L
 *	ABILITY_SLAY_INSECT	0x00004000L
 *	ABILITY_BRAND_DARK	0x00008000L
 *	ABILITY_BRAND_LITE	0x00010000L
 *	ABILITY_BRAND_NETHER0x00020000L
 *	ABILITY_BRAND_NEXUS	0x00040000L
 *	ABILITY_BRAND_MAGIC	0x00080000L
 *	ABILITY_BRAND_CHAOS	0x00100000L
 *	ABILITY_SLAY_MAN	0x00200000L
 *	ABILITY_SLAY_ELF	0x00400000L
 *	ABILITY_SLAY_DWARF	0x00800000L
 *	ABILITY_SLAY_ARCHER	0x01000000L
 *	ABILITY_SLAY_WARRIOR0x02000000L
 *	ABILITY_SLAY_MAGE	0x04000000L
 *	ABILITY_SLAY_PRIEST	0x08000000L
 *	ABILITY_SLAY_THIEF	0x10000000L
 *	ABILITY_BRAND_PSYCHIC 0x20000000L
 *	ABILITY_BRAND_WATER	0x40000000L
 *	ABILITY_BRAND_LAVA	0x80000000L
 *
 *	However, following initialisation, we cut down the table so that only brands/slays which
 *	can occur multiple times on a single ego item are counted.
 */
u32b slay_index(const s16b *ability, int ability_count)
{
	u32b s_index = 0x00000000L;
	int i;

	/* Efficiency */
	for (i = 0; i < ability_count; i++)
	{
		if (ability_bonus[i].type >= BONUS_SLAY)
		{
			/* Look up value from pre-computed table */
			s_index |= (ability_to_slay_index[i]);
		}
	}

	return s_index;
}


enum
{
	POWER_MELEE,
	POWER_CHARGE,
	POWER_SLING,
	POWER_BOW,
	POWER_XBOW,
	POWER_GUN,
	POWER_THROW,
	POWER_TRAP,
	POWER_UNARMED,
	POWER_AMMO,
	POWER_MAX
};


enum
{
	POWER2_DD,
	POWER2_DS,
	POWER2_ATTACKS,
	POWER2_MAX_ATTACKS,
	POWER2_TO_HIT,
	POWER2_MAX_TO_HIT,
	POWER2_TO_DAM,
	POWER2_MAX_TO_DAM,
	POWER2_MIGHT,
	POWER2_MAX_MIGHT,
	POWER2_CRIT,
	POWER2_RANGE,
	POWER2_MIN_RANGE,
	POWER2_MAX
};


/*
 * This table lists the individual collection of abilities for weapons and is used to calculate their effectiveness.
 * Trap abilities only apply when the weapon is used in a trap, hurling is only for the weapon when hurled, and so
 * on. We don't add power for e.g. unarmed combat from a bow, which means that bows with an unarmed combat bonus
 * "shouldn't" be generated.
 */
static const s16b power_table[POWER_MAX][POWER2_MAX] =
{
		{ 0, 0, ABILITY_BLOWS, 3, ABILITY_TO_HIT_MELEE, 9, ABILITY_TO_DAM_MELEE, 0, ABILITY_MELEE_MIGHT, 3, ABILITY_MELEE_CRIT, 0, 0},
		{ 0, 0, ABILITY_CHARGE, 3, ABILITY_TO_HIT_MELEE, 9, ABILITY_TO_DAM_MELEE, 0, 0, 0, ABILITY_MELEE_CRIT, 0, 0},
		{ 3, 5, ABILITY_SHOTS, 3, ABILITY_TO_HIT_BOW, 9, ABILITY_TO_DAM_BOW, 9, ABILITY_MIGHT, 3, ABILITY_BOW_CRIT, ABILITY_BOW_RANGE, 1},
		{ 4, 6, ABILITY_SHOTS, 3, ABILITY_TO_HIT_BOW, 9, ABILITY_TO_DAM_BOW, 9, ABILITY_MIGHT, 3, ABILITY_BOW_CRIT, ABILITY_BOW_RANGE, 1},
		{ 4, 7, ABILITY_SHOTS, 3, ABILITY_TO_HIT_BOW, 9, ABILITY_TO_DAM_BOW, 9, ABILITY_MIGHT, 3, ABILITY_BOW_CRIT, ABILITY_BOW_RANGE, 1},
		{ 3, 5, ABILITY_SHOTS, 3, ABILITY_TO_HIT_BOW, 9, ABILITY_TO_DAM_BOW, 9, ABILITY_MIGHT, 3, ABILITY_BOW_CRIT, ABILITY_BOW_RANGE, 1},
		{ 0, 0, ABILITY_HURL_NUM, 4, ABILITY_TO_HIT_THROW, 9, ABILITY_TO_DAM_THROW, 0, ABILITY_HURL_DAM, 4, ABILITY_HURL_CRIT, ABILITY_HURL_RANGE, 1},
		{ 0, 0, ABILITY_TRAPS, 4, ABILITY_TO_HIT_TRAP, 9, ABILITY_TO_DAM_TRAP, 0, ABILITY_TRAP_MIGHT, 4, ABILITY_TRAP_CRIT, ABILITY_TRAP_RANGE, 1},
		{ 0, 0, ABILITY_STRIKES, 4, ABILITY_TO_HIT_UNARM, 9, ABILITY_TO_DAM_UNARM, 0, ABILITY_UNARM_MIGHT, 4, ABILITY_UNARM_CRIT, 0, 0},
		{ 0, 0, ABILITY_SHOTS, 3, ABILITY_TO_HIT_BOW, 9, ABILITY_TO_DAM_BOW, 0, ABILITY_MIGHT, 4, ABILITY_BOW_CRIT, ABILITY_BOW_RANGE, 1}
};

#define AVG_DAM_UNCORRECTED(x,y) ((x) * (((y) > 1) ? ((y) + 1) : (y)) * (((y) > 1) ? 1 : 2))

#define AVG_DAM(x,y) ((x) * (((y) > 1) ? ((y) + 1) : (y)) / (((y) > 1) ? 2 : 1))


/*
 * Evaluate the objects's overall power level.
 *
 * Adopted from the randart.c patch by Chris Carr / Chris Robertson.
 */
s32b object_power(const object_type *o_ptr)
{
	s32b p = 0;
	s16b k_idx;
	object_kind *k_ptr;
	int immunities = 0;
	int sustains = 0;
	int low_resists = 0;
	int high_resists = 0;
	int i, j;

	u32b kf0[ABILITY_ARRAY_SIZE];	
	u32b kf1, kf2, kf3, kf4;
	
	u32b f0[ABILITY_ARRAY_SIZE];
	u32b f1, f2, f3, f4, f5, f6;

	s16b ability[ABILITY_MAX];
	s16b ability_base[ABILITY_MAX];
	
	/* If artifact, already computed */
	if (o_ptr->name1) return (a_info[o_ptr->name1].power);

	/* Get the flags */
	object_flags(o_ptr,f0,&f1,&f2,&f3,&f4);

	/* Extract kind flags */
	f5 = k_info[o_ptr->k_idx].flags5;
	f6 = k_info[o_ptr->k_idx].flags6;

	/* Evaluate the item */
	object_eval(o_ptr, ability);
	
	/* Get difference between item and base kind */
	for (i = 0; i < ABILITY_MAX; i++) ability_base[i] = 0;
	
	/* Lookup the item if not yet cached */
	k_idx = o_ptr->k_idx;

	if(o_ptr->k_idx<=0)
		k_idx = lookup_kind(o_ptr->tval, o_ptr->sval);
	
	/* Get object kind */
	k_ptr = &k_info[k_idx];
	
	/* Get kind flags */
	kf1 = k_ptr->flags1;
	kf2 = k_ptr->flags2;
	kf3 = k_ptr->flags3;
	kf4 = k_ptr->flags4;
	
	/* Set base ability */
	for (i = 0; i < ABILITY_MAX; i++)
	{
		ability_base[i] = 0;
	}
	
	/* Subtract the kind bonuses from the object */
	for (i = 0; i < k_ptr->ability_count; i++)
	{
		ability_base[k_ptr->ability[i]] -= k_ptr->aval[i];
	}
	
	/*
	 * Evaluate abilities on weapons based on the damage that
	 * weapon causes; skip evaluating some abilities based
	 * on the type of weapon.
	 * 
	 * We determine q which is twice the average damage inflicted
	 * per round of combat by the item, and subtract r which is
	 * twice the average damage per round of combat for the base
	 * item type (object kind). Because objects can be used in
	 * multiple ways, the power p is the maximum difference between
	 * the q & r, checked for all possible ways the item can be
	 * used to cause damage.
	 */
	for (i = 0; i < POWER_MAX; i++)
	{
		s32b q = 0;
		s32b r = 0;
		int mult = 1;
		
		/* Ignore abilities on some items */
		switch (i)
		{
			case POWER_MELEE:
			case POWER_CHARGE:
				if ((f6 & (TR6_WEAPON)) == 0) continue;
				if (o_ptr->tval == TV_BOW) continue;
				break;
			case POWER_SLING:
			case POWER_BOW:
			case POWER_XBOW:
			case POWER_GUN:
				if (o_ptr->tval != TV_BOW) continue;
				if ((o_ptr->sval / 10) != (i - POWER_SLING)) continue;
				break;
			case POWER_THROW:
				if ((f3 & (TR3_THROWING)) == 0) continue;
				break;
			case POWER_UNARMED:
				if ((f6 & (TR6_WEARABLE)) == 0) continue;
				if (o_ptr->tval == TV_AMULET) continue;
				if (o_ptr->tval == TV_BODY) continue;
				if (o_ptr->tval == TV_CLOAK) continue;
				break;
			case POWER_AMMO:
				if ((f5 & (TR5_AMMO)) == 0) continue;
				break;
				
			/* All items count trap abilities */
		}
		
		/* Get average damage - missile weapons*/
		if ((power_table[i][POWER2_DD]) && (power_table[i][POWER2_DS]))
		{
			q = AVG_DAM_UNCORRECTED(power_table[i][POWER2_DD], power_table[i][POWER2_DS]);
			r = q;
		}
		/* Get average damage - other weapons*/
		else
		{
			q = AVG_DAM_UNCORRECTED(object_aval(o_ptr, ABILITY_DAMAGE_DICE), object_aval(o_ptr, ABILITY_DAMAGE_SIDES));
			r = q;
		}

		/* Increased to hit chance? */
		if (power_table[i][POWER2_TO_HIT])
		{
			/* Increase power for to-dam - power specific - if we exceed the maximum allowed */
			if (object_aval(o_ptr, power_table[i][POWER2_TO_HIT]) > power_table[i][POWER2_TO_HIT])
			{
				q += 4 * object_aval(o_ptr, power_table[i][POWER2_TO_HIT]) / 3;
			}
			/* Assume the maximum allowed for any lessor values- these can be enchanted up */
			else if ((f0[power_table[i][POWER2_TO_HIT]/32] & (1L << (power_table[i][POWER2_TO_HIT]%32))) != 0)
			{
				q += 4 * power_table[i][POWER2_MAX_TO_HIT] / 3;
			}
			
			/*** Base item ***/
			/* Increase power for to-dam - power specific - if we exceed the maximum allowed */
			if ((ability_base[power_table[i][POWER2_TO_HIT]]) > power_table[i][POWER2_TO_HIT])
			{
				r += 4 * ability_base[power_table[i][POWER2_TO_HIT]] / 3;
			}
			/* Assume the maximum allowed for any lessor values- these can be enchanted up */
			else if ((kf0[power_table[i][POWER2_TO_HIT]/32] & (1L << (power_table[i][POWER2_TO_HIT]%32))) != 0)
			{
				r += 4 * power_table[i][POWER2_MAX_TO_HIT] / 3;
			}
		}
		
		/* Increase power for to-hit - general to-hit bonus - if we exceed the maximum allowed */
		if (object_aval(o_ptr, ABILITY_TO_HIT) > power_table[i][POWER2_MAX_TO_HIT])
		{
			q += 4 * object_aval(o_ptr, ABILITY_TO_HIT) / 3;
		}
		/* Assume the maximum allowed for any lessor values- these can be enchanted up */
		else if ((f0[ABILITY_TO_HIT/32] & (1L << (ABILITY_TO_HIT%32))) != 0)
		{
			q += 4 * (power_table[i][POWER2_MAX_TO_HIT]) / 3;
		}
		
		/*** Base item ***/
		/* Increase power for to-hit - general to-hit bonus - if we exceed the maximum allowed */
		if (ability_base[ABILITY_TO_HIT] > power_table[i][POWER2_MAX_TO_HIT])
		{
			q += 4 * ability_base[ABILITY_TO_HIT] / 3;
		}
		/* Assume the maximum allowed for any lessor values- these can be enchanted up */
		else if ((kf0[ABILITY_TO_HIT/32] & (1L << (ABILITY_TO_HIT%32))) != 0)
		{
			q += 4 * (power_table[i][POWER2_MAX_TO_HIT]) / 3;
		}
		
		
		/* Multiply average damage? */
		if (power_table[i][POWER2_MIGHT])
		{
			/* Hack - for the moment include bow multiplier */
			if (o_ptr->tval == TV_BOW) mult = bow_multiplier(o_ptr->sval);
	
			/* Get damage multiplier */
			if ((object_aval(o_ptr, power_table[i][POWER2_MIGHT]) > power_table[i][POWER2_MAX_MIGHT]) || (object_aval(o_ptr, power_table[i][POWER2_MIGHT]) < 0))
			{
				p += 20000;	/* inhibit */
				break;	/* don't overflow */
			}
			else
			{
				mult += object_aval(o_ptr, power_table[i][POWER2_MIGHT]);
			}
			
			q *= mult;

			/*** Base item ***/
			mult = 1;
			
			/* Get damage multiplier */
			if ((ability_base[power_table[i][POWER2_MIGHT]] > power_table[i][POWER2_MAX_MIGHT]) || (ability_base[power_table[i][POWER2_MIGHT]] < 0))
			{
				p += 20000;	/* inhibit */
				break;	/* don't overflow */
			}
			else
			{
				mult += ability_base[power_table[i][POWER2_MIGHT]];
			}
			
			r *= mult;
		}
		
		/* Apply the correct artifact/ego slay multiplier */
		if ((o_ptr->name1) || (o_ptr->name2))
		{
			int lowest_aval = 9999;
			int highest_aval = -9999;

			int j, k;

			/* Find highest aval which is a brand/slay */
			for (j = 0; j < o_ptr->ability_count; j++)
			{
				/* Note we use the 'restricted' ego list once random artifacts have been created */
				if (ability_to_slay_index[o_ptr->ability[j]])
				{
					if ((o_ptr->aval[j] > 0) && (lowest_aval > o_ptr->aval[j])) lowest_aval = o_ptr->aval[j];
					if (highest_aval < o_ptr->aval[j]) highest_aval = o_ptr->aval[j];
				}
			}

			/* Iterate from lowest aval to highest */
			if (highest_aval > 0)
			{
				u32b s_index = 0L;
				int s = 0;
				int last_match = 0;

				/* Iterate through valid avals */
				for (j = lowest_aval; j <= highest_aval; j++)
				{
					bool match = FALSE;

					for (k = 0; k < o_ptr->ability_count; k++)
					{
						/* Collect brands/slays with this or better aval */
						if (ability_to_slay_index[o_ptr->ability[k]])
						{
							if (o_ptr->aval[k] >= j) s_index |= ability_to_slay_index[o_ptr->ability[k]];
							if (o_ptr->aval[k] == j) match = TRUE;
						}
					}

					/* No slays/brands with this aval */
					if (!match) continue;

					/* Get the power */
					s += (slay_power(s_index) - tot_mon_power) * (j - last_match);

					/* Looking for difference */
					last_match = j;
				}

				/* Add total power contributed by slays/brands */
				q = q * (tot_mon_power + s) / tot_mon_power;
			}
		}

		/* Hack -- For efficiency, compute for first slay or brand flag only */
		else
		{
			int j;

			/* Increase power based on effectiveness against this monster class */
			for (j = 0; j < o_ptr->ability_count; j++)
			{
				if ((ability_to_magic_slay_power[o_ptr->ability[j]]) && (o_ptr->aval[j] > 0))
				{
					q = (q * (tot_mon_power + (magic_slay_power[ability_to_magic_slay_power[o_ptr->ability[j]]-1] - tot_mon_power) * o_ptr->aval[j])) / tot_mon_power;
					break;
				}
			}
		}

		/*** Base item - only count first slay or brand ***/
		for (j = 0; j < k_ptr->ability_count; j++)
		{
			if ((ability_to_magic_slay_power[k_ptr->ability[j]]) && (k_ptr->aval[j] > 0))
			{
				r = (r * (tot_mon_power + (magic_slay_power[ability_to_magic_slay_power[k_ptr->ability[j]]-1] - tot_mon_power) * k_ptr->aval[j])) / tot_mon_power;
				break;
			}
		}

		/* Increase average damage? */
		if (power_table[i][POWER2_TO_DAM])
		{
			/* Increase power for to-dam - power specific - if we exceed the maximum allowed */
			if (object_aval(o_ptr, power_table[i][POWER2_TO_DAM]) > (power_table[i][POWER2_MAX_TO_DAM] ? power_table[i][POWER2_MAX_TO_DAM] :
				AVG_DAM_UNCORRECTED(object_aval(o_ptr, ABILITY_DAMAGE_DICE), object_aval(o_ptr, ABILITY_DAMAGE_SIDES))))
			{
				q += 2 * object_aval(o_ptr, power_table[i][POWER2_TO_DAM]);
			}
			/* Assume the maximum allowed for any lessor values- these can be enchanted up */
			else if ((f0[power_table[i][POWER2_TO_DAM]/32] & (1L << (power_table[i][POWER2_TO_DAM]%32))) != 0)
			{
				q += 2 * (power_table[i][POWER2_MAX_TO_DAM] ? power_table[i][POWER2_MAX_TO_DAM] :
					AVG_DAM_UNCORRECTED(object_aval(o_ptr, ABILITY_DAMAGE_DICE), object_aval(o_ptr, ABILITY_DAMAGE_SIDES)));
			}

			/*** Base item ***/
			/* Increase power for to-dam - power specific - if we exceed the maximum allowed */
			if (ability_base[power_table[i][POWER2_TO_DAM]] > (power_table[i][POWER2_MAX_TO_DAM] ? power_table[i][POWER2_MAX_TO_DAM] :
				AVG_DAM_UNCORRECTED(ability_base[ABILITY_DAMAGE_DICE], ability_base[ABILITY_DAMAGE_SIDES])))
			{
				r += 2 * ability_base[power_table[i][POWER2_TO_DAM]];
			}
			/* Assume the maximum allowed for any lessor values- these can be enchanted up */
			else if ((kf0[power_table[i][POWER2_TO_DAM]/32] & (1L << (power_table[i][POWER2_TO_DAM]%32))) != 0)
			{
				r += 2 * (power_table[i][POWER2_MAX_TO_DAM] ? power_table[i][POWER2_MAX_TO_DAM] :
					AVG_DAM_UNCORRECTED(ability_base[ABILITY_DAMAGE_DICE], ability_base[ABILITY_DAMAGE_SIDES]));
			}
		}
		
		/* Increase power for to-dam - general to-dam bonus - if we exceed the maximum allowed */
		if (object_aval(o_ptr, ABILITY_TO_DAM) > (power_table[i][POWER2_MAX_TO_DAM] ? power_table[i][POWER2_MAX_TO_DAM] :
			AVG_DAM(object_aval(o_ptr, ABILITY_DAMAGE_DICE), object_aval(o_ptr, ABILITY_DAMAGE_SIDES))))
		{
			q += 2 * object_aval(o_ptr, ABILITY_TO_DAM);
		}
		/* Assume the maximum allowed for any lessor values- these can be enchanted up */
		else if ((f0[ABILITY_TO_DAM/32] & (1L << (ABILITY_TO_DAM%32))) != 0)
		{
			q += 2 * (power_table[i][POWER2_TO_DAM] ? power_table[i][POWER2_TO_DAM] :
				AVG_DAM(object_aval(o_ptr, ABILITY_DAMAGE_DICE), object_aval(o_ptr, ABILITY_DAMAGE_SIDES)));
		}
		
		/*** Base item ***/
		/* Increase power for to-dam - general to-dam bonus - if we exceed the maximum allowed */
		if (ability_base[ABILITY_TO_DAM] > (power_table[i][POWER2_MAX_TO_DAM] ? power_table[i][POWER2_MAX_TO_DAM] :
			AVG_DAM(ability_base[ABILITY_DAMAGE_DICE], ability_base[ABILITY_DAMAGE_SIDES])))
		{
			r += 2 * object_aval(o_ptr, ABILITY_TO_DAM);
		}
		/* Assume the maximum allowed for any lessor values- these can be enchanted up */
		else if ((kf0[ABILITY_TO_DAM/32] & (1L << (ABILITY_TO_DAM%32))) != 0)
		{
			r += 2 * (power_table[i][POWER2_TO_DAM] ? power_table[i][POWER2_TO_DAM] : AVG_DAM(ability_base[ABILITY_DAMAGE_DICE], ability_base[ABILITY_DAMAGE_SIDES]));
		}

		/* Increase average crits? */
		if (power_table[i][POWER2_CRIT])
		{
			/* TODO: Need to check the relative 'value' of crits */
			q += object_aval(o_ptr, power_table[i][POWER2_CRIT]);
			
			r += ability_base[power_table[i][POWER2_CRIT]];
		}
		
		/* Increase average damage? */
		if (power_table[i][POWER2_RANGE])
		{
			int range = 6, div;
			
			/* Calculate throwing range instead */
			if (i == POWER_THROW)
			{
				/* Extract a "distance multiplier" */
				mult = is_throwing_item(o_ptr) ? 10 : 3;

				/* Enforce a minimum "weight" of one pound */
				div = object_aval(o_ptr, ABILITY_WEIGHT) > 10 ? object_aval(o_ptr, ABILITY_WEIGHT) : 10;

				/* Hack -- Distance -- Reward strength, penalize weight */
				range = 120 * mult / div;

				/* Max distance of 10 */
				if (range > 10) range = 10;
			}
			
			/* TODO: Need to check the relative 'value' of range */
			q += (range + object_aval(o_ptr, power_table[i][POWER2_RANGE])) * q / 10;
			
			/*** Base item ***/
			/* Calculate throwing range instead */
			if (i == POWER_THROW)
			{
				/* Extract a "distance multiplier" */
				mult = is_throwing_item(o_ptr) ? 10 : 3;

				/* Enforce a minimum "weight" of one pound */
				div = ability_base[ABILITY_WEIGHT] > 10 ? ability_base[ABILITY_WEIGHT] : 10;

				/* Hack -- Distance -- Reward strength, penalize weight */
				range = 120 * mult / div;

				/* Max distance of 10 */
				if (range > 10) range = 10;
			}			

			/*** Base item ***/
			r += (range + ability_base[power_table[i][POWER2_RANGE]]) * r / ((kf3 & TR3_THROWING) ? 10 : 3);
		}

		/* Multiply total damage? */
		if ((power_table[i][POWER2_ATTACKS]) || (i == POWER_MELEE) || (i == POWER_THROW))
		{
			if (i == POWER_MELEE)
			{
				/* These values are calculated from the maximum possible blows from
				 * a warrior for the weapon weight. It's much easier to hard code this. */
				if (object_aval(o_ptr, ABILITY_WEIGHT) > 500) mult = 3;
				else if (object_aval(o_ptr, ABILITY_WEIGHT) > 250) mult = 4;
				else mult = 5;
			}
			else if (i == POWER_THROW) mult = 5;
			else mult = 1;
			
			/* Get damage multiplier */
			if ((object_aval(o_ptr, power_table[i][POWER2_ATTACKS]) > power_table[i][POWER2_MAX_ATTACKS]) || object_aval(o_ptr, power_table[i][POWER2_ATTACKS]) < 0)
			{
				p += 20000;	/* inhibit */
				break;
			}
			else
			{
				mult += object_aval(o_ptr, power_table[i][POWER2_ATTACKS]);
			}
			
			q *= mult;
			
			if (i == POWER_MELEE)
			{
				/* These values are calculated from the maximum possible blows from
				 * a warrior for the weapon weight. It's much easier to hard code this. */
				if (ability_base[ABILITY_WEIGHT] > 500) mult = 3;
				else if (ability_base[ABILITY_WEIGHT] > 250) mult = 4;
				else mult = 5;
			}
			else if (i == POWER_THROW) mult = 5;
			else mult = 1;
			
			/* Get damage multiplier */
			mult += ability[power_table[i][POWER2_ATTACKS]];
			
			r *= mult;
		}
		
		/*** Figure out the difference between the 'base object' and the object ***/
		if (q > r)
		{
			/* Use the highest power level for combat abilities */
			if (p < (q - r)) p = (q - r);
		}
		else if (q < r)
		{
			/* Only use this if we don't have a useful ability - pick the 'least bad' ability */
			if ((p <= 0) && (p < (q - r))) p = (q - r);
		}
	}
	
#if 0
	/* Evaluate certain abilities based on type of object. */
	switch (o_ptr->tval)
	{
		case TV_BOW:
		{
			/* Slight bonus as we may choose to use a swap bow */
			/* Hack -- only if it has other flags though */
			if (((f2 & (TR2_IGNORE_ACID)) != 0) && ((kf2 & (TR2_IGNORE_ACID)) == 0)
				&& ( (f1 & ~(kf1)) || (f2 & ~(TR2_IGNORE_MASK) & ~(kf2)) || (f3 & ~(kf3)) || (f4 & ~(kf4)) ) ) p++;

			if (((f2 & (TR2_IGNORE_FIRE)) != 0) && ((kf2 & (TR2_IGNORE_FIRE)) == 0)
				&& ( (f1 & ~(kf1)) || (f2 & ~(TR2_IGNORE_MASK) & ~(kf2)) || (f3 & ~(kf3)) || (f4 & ~(kf4)) ) ) p++;

			if (((f2 & (TR2_IGNORE_THEFT)) != 0) && ((kf2 & (TR2_IGNORE_THEFT)) == 0)
				&& ( (f1 & ~(kf1)) || (f2 & ~(TR2_IGNORE_MASK) & ~(kf2)) || (f3 & ~(kf3)) || (f4 & ~(kf4)) ) ) p++;
			break;
		}

		case TV_HAFTED:
		case TV_STAFF:
		case TV_DIGGING:
		case TV_POLEARM:
		case TV_SWORD:
		{
			/* Hack -- secondary weapons are more useful */
			if (p > 0 && (f6 & (TR6_OFF_HAND)))
				p++;

			if (object_aval(o_ptr, ABILITY_AC) != k_ptr->ac)
			{
				p += object_aval(o_ptr, ABILITY_AC) - k_ptr->ac;
			}

			/* Remember, weight is in 0.1 lb. units. */
			if (object_aval(o_ptr, ABILITY_WEIGHT) != k_ptr->weight)
			{
				p += (k_ptr->weight - object_aval(o_ptr, ABILITY_WEIGHT)) / 20;
			}

			/* Bonus as we may use a staff or hafted weapon as swap weapon */
			if ((o_ptr->tval == TV_STAFF) && (f2 & (TR2_IGNORE_FIRE))) p+= 2;
			if ((o_ptr->tval == TV_HAFTED) && (f2 & (TR2_IGNORE_FIRE))) p++;

			/* Bonuses as we may choose to use a swap weapon */
			if (((f2 & (TR2_IGNORE_ACID)) != 0) && ((kf2 & (TR2_IGNORE_ACID)) == 0)) p++;
			if (((f2 & (TR2_IGNORE_THEFT)) != 0) && ((kf2 & (TR2_IGNORE_THEFT)) == 0)) p++;

			/* Add some specific powers here only */
			ADD_POWER("blessed",		 1, TR3_BLESSED, 3,);
			ADD_POWER("blood vampire",	 25, TR4_VAMP_HP, 4,);
			ADD_POWER("mana vampire",	 14, TR4_VAMP_MANA, 4,);
			break;
		}

		case TV_ARROW:
		case TV_SHOT:
		case TV_BOLT:
		{
			if (object_aval(o_ptr, ABILITY_WEIGHT) < k_ptr->weight)
			{
				p++;
			}

			/* Bonus as we carry arrows in inventory and fire them */
			if ((o_ptr->tval == TV_ARROW) && ((f2 & (TR2_IGNORE_FIRE)) != 0) && ((kf2 & (TR2_IGNORE_FIRE)) == 0)) p += 2;

			/* Bonus as we carry ammo in inventory and fire them */
			if (((f2 & (TR2_IGNORE_ACID)) != 0) && ((kf2 & (TR2_IGNORE_ACID)) == 0)) p += 2;
			if (((f2 & (TR2_IGNORE_THEFT)) != 0) && ((kf2 & (TR2_IGNORE_THEFT)) == 0)) p++;

			break;
		}

		case TV_BOOTS:
		case TV_GLOVES:
		{
			/* Note this is 'uncorrected' */
			p += object_ability(o_ptr, ABILITY_DAMAGE_DICE) * (object_ability(o_ptr, ABILITY_DAMAGE_SIDES) + 1);

			/* Apply the correct ego slay multiplier */
			if (o_ptr->name2)
			{
				p = (p * e_info[o_ptr->name2].slay_power) / tot_mon_power;
			}

			/* Hack -- For efficiency, compute for first slay or brand flag only */
			else
			{
				int i;
				u32b j, s_index;

				s_index = slay_index(f0, f1, f2, f3, f4);

				for (i = 0, j = 0x00000001L;(i < 32) && (j != s_index); i++, j<<=1);

				if (i < 32) p = (p * magic_slay_power[i]) / tot_mon_power;
			}

			/* Correction factor for damage */
			p /= 2;

			/* Normalise power back */
			/* We remove the weapon base damage to get 'true' power */
			/* This makes e.g. a sword that provides fire immunity the same value as
			   a ring that provides fire immunity */
			if (ABS(p) > k_ptr->dd * (k_ptr->ds + 1) / 2)
				p -= sign(p) * (k_ptr->dd * (k_ptr->ds + 1) / 2);
			else
				p = 0;

			/* Fall through */

		}

		case TV_CLOAK:
		{
			/* Bonus as we may choose to use a swap armour */
			/* Hack -- only if it has other flags though */
			if (((f2 & (TR2_IGNORE_FIRE)) != 0) && ((kf2 & (TR2_IGNORE_FIRE)) == 0)
				&& ( (f1 & ~(kf1)) || (f2 & ~(TR2_IGNORE_MASK) & ~(kf2)) || (f3 & ~(kf3)) || (f4 & ~(kf4)) ) ) p++;

			/* Fall through */
		}

		case TV_SOFT_ARMOR:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{
			if (object_aval(o_ptr, ABILITY_AC) != k_ptr->ac)
			{
				p += object_aval(o_ptr, ABILITY_AC) - k_ptr->ac;
			}

			p += sign(ability_difference[ABILITY_TO_HIT]) * ((ABS(ability_difference[ABILITY_TO_HIT]) * 2) / 3);

			p += object_aval(o_ptr, ABILITY_TO_DAM) * 2;

			/* We assume rings of damage +5 */
			if (object_aval(o_ptr, ABILITY_TO_DAM) > 5)
			{
				p += (object_aval(o_ptr, ABILITY_TO_DAM) - 5) * 2;
			}

			if (object_aval(o_ptr, ABILITY_WEIGHT) < k_ptr->weight)
			{
				p += (k_ptr->weight - object_aval(o_ptr, ABILITY_WEIGHT)) / 10;
			}

			/* Big bonus as it protects against acid damage */
			if (((f2 & (TR2_IGNORE_ACID)) != 0) && ((kf2 & (TR2_IGNORE_ACID)) == 0)) p += 3;

			/* Bonus as we may choose to use a swap armour */
			/* Hack -- only if it has other flags though */
			if (((f2 & (TR2_IGNORE_THEFT)) != 0) && ((kf2 & (TR2_IGNORE_THEFT)) == 0)
				&& ( (f1 & ~(kf1)) || (f2 & ~(TR2_IGNORE_MASK) & ~(kf2)) || (f3 & ~(kf3)) || (f4 & ~(kf4)) ) ) p++;

			break;
		}

		case TV_LITE:
		{
			p += sign(object_aval(o_ptr, ABILITY_TO_HIT)) * ((ABS(object_aval(o_ptr, ABILITY_TO_HIT)) * 2) / 3);

			p += object_aval(o_ptr, ABILITY_TO_DAM) * 2;

			/* We assume rings of damage +5 */
			if (object_aval(o_ptr, ABILITY_TO_DAM) > 5)
			{
				p += (object_aval(o_ptr, ABILITY_TO_DAM) - 5) * 2;
			}

			/* Bonuses as we may choose to use a swap light */
			/* Hack -- only if it has other flags though */
			if (((f2 & (TR2_IGNORE_FIRE)) != 0) && ((kf2 & (TR2_IGNORE_FIRE)) == 0)) p++;
			if (f2 & (TR2_IGNORE_THEFT)) p++;

			/* Bonus as light will not go out in water */
			if (((f2 & (TR2_IGNORE_WATER)) != 0) && ((kf2 & (TR2_IGNORE_WATER)) == 0)) p += 3;
			break;
		}

		case TV_RING:
		{
			/* Bonus as we may choose to use a swap armour */
			/* Hack -- only if it has other flags though */
			if (((f2 & (TR2_IGNORE_ELEC)) != 0) && ((kf2 & (TR2_IGNORE_ELEC)) == 0)
				&& ( (f1 & ~(kf1)) || (f2 & ~(TR2_IGNORE_MASK) & ~(kf2)) || (f3 & ~(kf3)) || (f4 & ~(kf4)) ) ) p++;

			/* Apply the correct ego slay multiplier */
			if (o_ptr->name2)
			{
				p = (p * e_info[o_ptr->name2].slay_power) / tot_mon_power;
			}

			/* Hack -- For efficiency, compute for first slay or brand flag only */
			else
			{
				int i;
				u32b j, s_index;

				s_index = slay_index(f0, f1, f2, f3, f4);

				for (i = 0, j = 0x00000001L;(i < 32) && (j != s_index); i++, j<<=1);

				if (i < 32) p = (p * magic_slay_power[i]) / tot_mon_power;
			}

			/* Fall through */
		}
		case TV_AMULET:
		{

			p += sign(object_aval(o_ptr, ABILITY_TO_HIT)) * ((ABS(object_aval(o_ptr, ABILITY_TO_HIT)) * 2) / 3);

			p += object_aval(o_ptr, ABILITY_TO_DAM) * 2;

			/* We assume rings of damage +5 */
			if (object_aval(o_ptr, ABILITY_TO_DAM) > 5)
			{
				p += (object_aval(o_ptr, ABILITY_TO_DAM) - 5) * 2;
			}

			/* Bonus as we may choose to use a swap armour */
			/* Hack -- only if it has other flags though */
			if (((f2 & (TR2_IGNORE_THEFT)) != 0) && ((kf2 & (TR2_IGNORE_THEFT)) == 0)
				&& ( (f1 & ~(kf1)) || (f2 & ~(TR2_IGNORE_MASK) & ~(kf2)) || (f3 & ~(kf3)) || (f4 & ~(kf4)) ) ) p++;
			break;
		}

		case TV_WAND:
		{
			if (f2 & (TR2_IGNORE_ELEC)) p+= 2;
			if (f2 & (TR2_IGNORE_THEFT)) p++;

			break;
		}


		case TV_MAGIC_BOOK:
		case TV_PRAYER_BOOK:
		case TV_SONG_BOOK:
		case TV_INSTRUMENT:
		case TV_SCROLL:
		case TV_MAP:
		{
			/* Bonus as we carry items in inventory */
			if (((f2 & (TR2_IGNORE_FIRE)) != 0) && ((kf2 & (TR2_IGNORE_FIRE)) == 0)) p += 2;

			/* Fall through */
		}
		case TV_MUSHROOM:
		case TV_FOOD:
		{
			if (((f2 & (TR2_IGNORE_WATER)) != 0) && ((kf2 & (TR2_IGNORE_WATER)) == 0)) p += 2;
			if (f2 & (TR2_IGNORE_THEFT)) p++;

			break;
		}

		case TV_FLASK:
		case TV_POTION:
		{
			if (f2 & (TR2_IGNORE_COLD)) p += 2;
			if (f2 & (TR2_IGNORE_THEFT)) p++;

			break;
		}

	}

	/* Compute ac bonuses */

	/* Evaluate ac bonus differently for armour and non-armour. */
	switch (o_ptr->tval)
	{
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{
			if (object_aval(o_ptr, ABILITY_TO_AC) > object_aval(o_ptr, ABILITY_AC))
			{
				p+= (object_aval(o_ptr, ABILITY_TO_AC) - object_aval(o_ptr, ABILITY_AC));
			}

			if (object_aval(o_ptr, ABILITY_TO_AC) > object_aval(o_ptr, ABILITY_AC) + 10)
			{
				p += (object_aval(o_ptr, ABILITY_TO_AC) - object_aval(o_ptr, ABILITY_AC) - 10);
			}

			if (object_aval(o_ptr, ABILITY_TO_AC) > object_aval(o_ptr, ABILITY_AC) + 20)
			{
				p += (object_aval(o_ptr, ABILITY_TO_AC) - object_aval(o_ptr, ABILITY_AC) - 20);
			}

			if (object_aval(o_ptr, ABILITY_TO_AC) > 39)
			{
				p += 20000;	/* inhibit */
			}
			break;
		}
		case TV_CROWN:
		case TV_SWORD:
		case TV_DIGGING:
		case TV_STAFF:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_BOW:
		case TV_INSTRUMENT:
		case TV_LITE:
		case TV_RING:
		case TV_AMULET:
		{
			p += sign(object_aval(o_ptr, ABILITY_TO_AC)) * (ABS(object_aval(o_ptr, ABILITY_TO_AC)) / 2);

			if (object_aval(o_ptr, ABILITY_TO_AC) > 9)
			{
				p+= (object_aval(o_ptr, ABILITY_TO_AC) - 9);
			}

			if (object_aval(o_ptr, ABILITY_TO_AC) > 19)
			{
				p += (object_aval(o_ptr, ABILITY_TO_AC) - 19);
			}

			if (object_aval(o_ptr, ABILITY_TO_AC) > 29)
			{
				p += 20000;	/* inhibit */
			}
			break;
		}
		default:
			return(p);
	}

#endif
	
	/* The general power function for abilities */
	for (i = 0; i < ABILITY_MAX; i++)
	{
		/* Skip calculation of abilities which only apply when the item is being used. We calculate these later. */
		if ((ability_bonus[i].type >= BONUS_WEAPON) && (((f5 & (TR5_AMMO)) != 0) || ((f6 & (TR6_WEAPON)) != 0))) continue;
		
		/* Skip everything except weapon abilities for items we cannot wield/equip */
		if ((ability_bonus[i].type < BONUS_WEAPON) && (ability_bonus[i].type != BONUS_PROOF) &&
				((f5 & (TR5_PACK)) != 0) && ((f6 & (TR6_WEAPON | TR6_WEARABLE)) == 0)) continue;
		
		/* Good abilities increase item power rating */
		if (ability[i] > 0)
		{
			/* Item has the ability at a level greater than its base kind allows for */
			/* Note that there is no need to subtract k_ptr->aval from o_ptr->aval,
				because we want to penalize non-standard aval, even just 1 higher,
				especially if it's atop already high standard aval */
			if (((f0[i/32] & (1L << (i % 32))) != 0) && (((kf0[i/32] & (1L << (i % 32))) == 0) || (ability[i] > ability_base[i])))
			{
				/* a * x^2 / b */
				if ((ability_bonus[i].a) && (ability_bonus[i].b))
				{
					p += ability_bonus[i].a * ability[i] * ability[i] / ability_bonus[i].b;
				}
				else if (ability_bonus[i].a)
				{
					p += ability_bonus[i].a * ability[i] * ability[i];
				}

				/* + c * x / d */
				if ((ability_bonus[i].c) && (ability_bonus[i].d))
				{
					p += ability_bonus[i].c * ability[i] / ability_bonus[i].d;
				}
				else if (ability_bonus[i].c)
				{
					p += ability_bonus[i].c * ability[i];
				}
				
				/* + e */
				p+= ability_bonus[i].e;
				
				/* Various accumulators */
				if (ability_bonus[i].type == BONUS_LOW_RESIST) low_resists++;
				if (ability_bonus[i].type == BONUS_RESIST) high_resists++;
			}
		}
		/* Bad abilities decrease the power rating */
		/* Hack: don't give large negatives.
		   Also, don't penalize for aval less than k_ptr->aval,
		   so that default avals are no lowered to decrease power
		   and we may end with the boring 0 aval */
		else if (ability[i] < 0)
		{
			if ((ability_bonus[i].f) && (ability_bonus[i].g))
			{
				p += ability_bonus[i].f * ability[i] / ability_bonus[i].g;
			}
			else if (ability_bonus[i].g)
			{
				p += ability_bonus[i].g * ability[i];
			}
			
			/* - h */
			p -= ability_bonus[i].h;
		}
	}
	
	/* Add bonus for low resists getting 'low_resists-lock' */
	if (low_resists > 1) p += low_resists * low_resists;

	/* Add bonus for high resists getting 'high_resists-lock' */
	if (high_resists > 1) p += high_resists * high_resists / 2;

	/* The general power function for flags */
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 32; j++)
		{
			/* Skip calculation of abilities which only apply when the item is being used. We calculate these later. */
			if ((item_flag_bonus[i][j].type >= BONUS_WEAPON) && (((f5 & (TR5_AMMO)) != 0) || ((f6 & (TR6_WEAPON)) != 0))) continue;
			
			/* Skip everything except weapon abilities for items we cannot wield/equip */
			if ((item_flag_bonus[i][j].type < BONUS_WEAPON) && (ability_bonus[i].type != BONUS_PROOF) &&
					((f5 & (TR5_PACK)) != 0) && ((f6 & (TR6_WEAPON | TR6_WEARABLE)) == 0)) continue;

			/* Add power */
			if ((i == 0) && ((f1 & (1L << j)) == 0)) continue;
			else if ((i == 1) && ((f2 & (1L << j)) == 0)) continue;
			else if ((i == 2) && ((f3 & (1L << j)) == 0)) continue;
			else if ((i == 3) && ((f4 & (1L << j)) == 0)) continue;
			
			/* Don't give bonus for proofing if item cannot be damaged/stolen */
			if ((item_flag_bonus[i][j].power > 0) && (item_flag_bonus[i][j].type == BONUS_PROOF) && 
					(item_flag_bonus[i][j].flag_match == 5) && ((f5 & (item_flag_bonus[i][j].flag_match)) == 0)) continue;

			/* Increase power */
			p+= item_flag_bonus[i][j].power;

			/* Various accumulators */
			if (ability_bonus[i].type == BONUS_SUSTAIN) sustains++;
			if (ability_bonus[i].type == BONUS_IMMUNE) immunities++;
		}
	}

	/* Add bonus for sustains getting 'sustain-lock' */
	if (sustains > 2) p += sustains * sustains / 3;

	/* Apply bonus for multiple immunities */
	if (immunities > 1)
	{
		p += 15;
	}
	if (immunities > 2)
	{
		p += 45;
	}
	if (immunities > 3)
	{
		p += 20000;		/* inhibit */
	}

	

	/* Evaluate weight discount. */
	switch (o_ptr->tval)
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
	/*
	 * If a worn item weighs more than 5 pounds, we discount its power by up to 50%.
	 *
	 * This figure is from 30 lb weight limit for mages divided by 6 slots.
	 */
			if (object_aval(o_ptr, ABILITY_WEIGHT) >= 50)
			{
				if (p > object_aval(o_ptr, ABILITY_WEIGHT) / 25)
					p -= object_aval(o_ptr, ABILITY_WEIGHT) / 50;
				else if (p > 0)
					p = (p + 1) / 2;
			}

			break;
		}
		case TV_SWORD:
		case TV_DIGGING:
		case TV_STAFF:
		case TV_HAFTED:
		case TV_POLEARM:
		{
			/* These are breaks for 1-handed, 2-handed, max 4 blows for warriors,
				and max 3 blows for others */
			if (p > 0)
			{
				if ((object_aval(o_ptr, ABILITY_WEIGHT) > 960) && (p > 7))
				{
					p -= 1;
				}

				if ((object_aval(o_ptr, ABILITY_WEIGHT) > 240) && (p > 5))
				{
					p -= 1;
				}

				if ((object_aval(o_ptr, ABILITY_WEIGHT) >= 200) && (p > 3))
				{
					p -= 1;
				}

				if ((object_aval(o_ptr, ABILITY_WEIGHT) >= 150) && (p > 1))
				{
					p -= 1;
				}
			}
			break;
		}
		case TV_BOW:
		case TV_INSTRUMENT:
		case TV_LITE:
		case TV_RING:
		case TV_AMULET:
		{
	/*
	 * If a worn item weighs more than 15 pounds, we discount its power by up to 50%.
	 *
	 * This if for acting as a swap item. Note that super heavy weapons get less of a
	 * discount than other super heavy swap items, because of the increased criticals
	 * and charge bonus.
	 */
			if (object_aval(o_ptr, ABILITY_WEIGHT) >= 150)
			{
				if (p > object_aval(o_ptr, ABILITY_WEIGHT) / 75)
					p -= object_aval(o_ptr, ABILITY_WEIGHT) / 150;
				else if (p > 0)
					p = (p + 1) / 2;
			}
			break;
		}
	}


	return (p);
}


/* Describe the type of feature */
static void describe_feature_type(const feature_type *f_ptr)
{
	if (f_ptr->flags1 & FF1_ENTER)			text_out(" shop entrance");
	else if (f_ptr->flags1 & FF1_DOOR)	text_out(" door");
	else if (f_ptr->flags3 & FF3_CHEST)	text_out(" chest");
	else if (f_ptr->flags1 & FF1_TRAP)	text_out(" trap");
	else if (f_ptr->flags1 & FF1_STAIRS)	text_out(" staircase");
	else if (f_ptr->flags1 & FF1_GLYPH)	text_out(" glyph");
	else if (f_ptr->flags1 & FF1_FLOOR)	text_out(" floor");
	else if (f_ptr->flags1 & FF1_WALL)	text_out(" wall");
	else if (f_ptr->flags3 & FF3_TREE)	text_out(" tree");
	else if (f_ptr->flags1 & FF3_GROUND)	text_out(" ground");

	/*Default*/
	else text_out(" feature");
}



static void describe_feature_basic(int f_idx)
{
	const feature_type *f_ptr = &f_info[f_idx];

	int n, vn;
	cptr vp[128];

	text_out("This is a");

	if (f_ptr->flags2 & FF2_SHALLOW) 	text_out(" shallow");
	if (f_ptr->flags2 & FF2_DEEP) 	text_out(" deep");

	if (f_ptr->flags2 & FF2_GLOW)
	{
	     if (f_ptr->flags1 & FF1_ENTER)	text_out(" well-lit");
		else	text_out(" glowing");
	}

	if (f_ptr->flags2 & FF2_LAVA)
	{
		if (f_ptr->flags2 & FF2_WATER) text_out(" boiling");
		else text_out(" lava");
	}
	if (f_ptr->flags2 & FF2_ICE) 	text_out(" icy");
	if (f_ptr->flags2 & FF2_ACID) 	text_out(" acid");
	if (f_ptr->flags2 & FF2_OIL) 	text_out(" fuel");

	if (f_ptr->flags2 & FF2_WATER)
	{
		if (f_ptr->flags2 & FF2_CAN_DIG) text_out(" mud");
	 	text_out(" water");
	}

	/*Describe the feature type*/
	describe_feature_type(f_ptr);

	/* Describe location */
	if (f_ptr->flags1 & FF1_ENTER)
	{
		text_out(" that is found in the town");
	}
	else if (f_ptr->flags1 & FF1_GLYPH)
	{
		text_out(" that is set by the player");
	}
	else
	{
		text_out(" that");

		if (f_ptr->rarity >= 4) text_out(" rarely");
		else if (f_ptr->rarity >= 2) text_out(" occasionally");
		else text_out(" commonly");

		if (f_ptr->level == 0)
		{
			text_out(" appears in both the town and dungeon");
		}
		else if (f_ptr->level == 1)
		{
			text_out(" appears throughout the dungeon");
		}
		else
		{
			text_out(" appears");

			if (depth_in_feet)
			{
				text_out(format(" at depths of %d feet and below",
			                            f_ptr->level * 50));
			}
			else
			{
				text_out(format(" on dungeon level %d and below",
			                            f_ptr->level));
			}
		}
	}

	/* Allocation */
	vn = 0;

	if (f_ptr->flags3 & (FF3_ALLOC)) vp[vn++] = "as a specially placed item";
	if (f_ptr->flags3 & (FF3_CHEST)) vp[vn++] = "as a item carried by monsters";
	if (f_ptr->flags2 & (FF2_LAKE)) vp[vn++] = "as a 'lake' of terrain";
	if (f_ptr->flags2 & (FF2_RIVER)) vp[vn++] = "as a 'river' of terrain";
	if (f_ptr->flags1 & (FF1_STREAMER)) vp[vn++] = "as a 'streams' of terrain through the dungeon walls";
	if (f_ptr->flags1 & (FF1_DOOR)) vp[vn++] = "placed in a doorway";
	if (f_ptr->flags1 & (FF1_INNER)) vp[vn++] = "as part of the inner wall of a room";
	if (f_ptr->flags1 & (FF1_OUTER)) vp[vn++] = "as part of the outer wall of a room";
	if (f_ptr->flags1 & (FF1_SOLID)) vp[vn++] = "next to the entrance of a corridor in a room";

	/* Describe innate attacks */
	if (vn)
	{
		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (!n) text_out(" ");
			else if ((n) && (n < vn-1)) text_out(", ");
			else text_out(" or ");

			/* Dump */
			text_out(vp[n]);
		}
	}

	if (f_ptr->flags1 & (FF1_TRAP)) text_out(" with a hidden trap");

	/* End this sentence */
	text_out(".  ");

}




/*
 * Describe the player ability to move, see and cast on or through the feature.
 */
static void describe_feature_player_moves(int f_idx)
{
	const feature_type *f_ptr = &f_info[f_idx];

	int n, vn;
	cptr vp[128];

	bool intro = FALSE;
	bool effect = FALSE;
	bool impede = TRUE;

	/* Collect sight and movement */
	vn = 0;

	if (!(f_ptr->flags1 & (FF1_FLOOR)) && !(f_ptr->flags3 & (FF3_GROUND)))
	{
		if (f_ptr->flags1 & (FF1_LOS)) vp[vn++] = "see";
		if (f_ptr->flags1 & (FF1_PROJECT)) vp[vn++] = "cast spells";
		if (f_ptr->flags1 & (FF1_PROJECT)) vp[vn++] = "fire missiles";

		if (vn) impede = FALSE;
	}

	if (f_ptr->flags1 & (FF1_MOVE)) vp[vn++] = "walk";
	if (f_ptr->flags1 & (FF1_RUN)) vp[vn++] = "run";
	if (f_ptr->flags3 & (FF3_EASY_CLIMB)) vp[vn++] = "climb";

	/* Describe sight and movement */
	if (vn)
	{
		/* Intro */
		text_out("You");

		intro = TRUE;

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out(" can ");
			else if (n < vn-1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out(vp[n]);
		}

		if ((f_ptr->flags1 & (FF1_FLOOR)) || (f_ptr->flags3 & (FF3_GROUND))) text_out(" on");
		else text_out(" through");

		text_out(" this");

		describe_feature_type(f_ptr);
	}

	/* Have to climb in or out of the grid */
	if ((intro) && (f_ptr->flags3 & (FF3_EASY_CLIMB | FF3_MUST_CLIMB)))
	{
		text_out(" taking an extra turn to ");
		if (f_ptr->flags3 & (FF3_EASY_CLIMB))
		{
			text_out("enter ");

			if (f_ptr->flags3 & (FF3_MUST_CLIMB)) text_out("and ");
		}

		if (f_ptr->flags3 & (FF3_MUST_CLIMB))
		{
			text_out("leave ");
		}

		text_out("the grid");
		effect = TRUE;
		impede = TRUE;
	}

	if ((intro) && (f_ptr->flags2 & (FF2_SHALLOW | FF2_DEEP | FF2_FILLED)))
	{
		if (effect) text_out(" and");
		text_out(" making your equipment");

		if (f_ptr->flags2 & (FF2_FILLED)) text_out(" significantly");
		else if (!(f_ptr->flags2 & (FF2_DEEP))) text_out(" slightly");
		text_out(" heavier");
		impede = TRUE;
	}

	if ((f_ptr->flags1 & (FF1_FLOOR)) || (f_ptr->flags3 & (FF3_GROUND)))
	{
		vn = 0;

		if (f_ptr->flags1 & (FF1_LOS)) vp[vn++] = "see";
		if (f_ptr->flags1 & (FF1_PROJECT)) vp[vn++] = "cast spells";
		if (f_ptr->flags1 & (FF1_PROJECT)) vp[vn++] = "fire missiles";

		/* Describe sight and movement */
		if (vn)
		{
			/* Intro */
			if (!intro) text_out("You");
			else text_out(" and");

			/* Scan */
			for (n = 0; n < vn; n++)
			{
				/* Intro */
				if (n == 0) text_out(" can ");
				else if (n < vn-1) text_out(", ");
				else text_out(" and ");

				/* Dump */
				text_out(vp[n]);
			}

			if (!intro)
			{
				text_out(" through this");

				describe_feature_type(f_ptr);

			}
			else
			{
				text_out(" through it");
			}

			intro = TRUE;
			impede = FALSE;
		}
	}

	if (!impede) text_out(" without impediment");


	/* Collect innate attacks */
	vn = 0;
	effect = FALSE;
	impede = FALSE;

	if (!(f_ptr->flags1 & (FF1_MOVE)))
	{
		if (!(f_ptr->flags3 & (FF3_EASY_CLIMB)))
		{
			vp[vn++] = "you from moving through it";
			impede = TRUE;
			if (!(f_ptr->flags2 & (FF2_CAN_PASS))) { vp[vn++] = "pass through walls"; effect = TRUE; }
			if (f_ptr->flags1 & (FF1_PERMANENT)) { vp[vn++] = "bore through walls"; effect = TRUE; }
			if (!(f_ptr->flags2 & (FF2_CAN_CLIMB))) { vp[vn++] = "climb"; effect = TRUE; }
		}
	}
	else
	{
		if ((f_ptr->flags2 & (FF2_COVERED)) && !(f_ptr->flags1 & (FF1_BASH))) { vp[vn++] = "lie underneath from surfacing"; effect = TRUE; }
		else if (f_ptr->flags2 & (FF2_COVERED)) { vp[vn++] = "lie underneath from surfacing without bashing through"; effect = TRUE; }
		if (!(f_ptr->flags2 & (FF2_CAN_FLY))) { vp[vn++] = "must fly"; effect = TRUE; }
		if (!(f_ptr->flags2 & (FF2_CAN_SWIM))) { vp[vn++] = "must swim"; effect = TRUE; }
	}
	if (!(f_ptr->flags1 & (FF1_LOS))) vp[vn++] = "line of sight";
	if (!(f_ptr->flags1 & (FF1_PROJECT))) vp[vn++] = "casting spells";
	if (!(f_ptr->flags1 & (FF1_PROJECT))) vp[vn++] = "firing missiles";

	/* Describe innate attacks */
	if (vn)
	{
		/* Intro */
		if (!intro)
		{
			text_out("This ");
			describe_feature_type(f_ptr);
		}
		else text_out(" but it");

		intro = TRUE;

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (!n) text_out(" blocks ");
			else if ((n == 1) && (impede)) text_out(" and ");
			else if (n < vn-1) text_out(", ");
			else text_out(" and ");

			if ((effect) && ((n == 1) || (!impede)))
			{
				effect = FALSE;
				text_out("monsters that ");
			}


			/* Dump */
			text_out(vp[n]);
		}
	}

	/* End sentence */
	if (intro) text_out(".  ");

	/* Collect innate attacks */
	vn = 0;
	intro = FALSE;
}


/*
 * Describe the monster ability to move and hide on or through the feature.
 */
static void describe_feature_monster_moves(int f_idx)
{
	const feature_type *f_ptr = &f_info[f_idx];

	int n, vn;
	cptr vp[128];

	bool intro = FALSE;

	vn = 0;

	if (!(f_ptr->flags1 & (FF1_MOVE)) || (f_ptr->blow.method) || (f_ptr->spell))
	{
		if ((f_ptr->flags2 & (FF2_CAN_FLY))) vp[vn++] = "fly";
		if ((f_ptr->flags2 & (FF2_CAN_SWIM))) vp[vn++] = "swim";
		if ((f_ptr->flags2 & (FF2_CAN_CLIMB))) vp[vn++] = "climb";
		if ((f_ptr->flags2 & (FF2_CAN_DIG))) vp[vn++] = "dig";
		if ((f_ptr->flags2 & (FF2_CAN_OOZE))) vp[vn++] = "ooze";
		if ((f_ptr->flags2 & (FF2_CAN_PASS))) vp[vn++] = "pass through walls";
		if (!(f_ptr->flags1 & (FF1_PERMANENT))) vp[vn++] = "bore through walls";
	}

	/* Describe monster moves */
	if (vn)
	{
		/* Intro */
		if (!intro)
		{
			text_out("This");
			describe_feature_type(f_ptr);
		}

		intro = TRUE;

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (!n) text_out(" lets monsters that can ");
			else if (n < vn-1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out(vp[n]);
		}

		text_out(" pass ");
		if ((f_ptr->flags1 & (FF1_FLOOR)) || (f_ptr->flags3 & (FF3_GROUND))) text_out("over");
		else text_out("through");

		text_out(" without impediment");
	}

	if ((vn) && (f_ptr->blow.method))
	{
		text_out(" provided they resist");
	}

	/* Collect innate attacks */
	vn = 0;

	if ((f_ptr->flags2 & (FF2_HIDE_SNEAK))) vp[vn++] = "sneak";
	if ((f_ptr->flags2 & (FF2_HIDE_SWIM))) vp[vn++] = "swim";
	if ((f_ptr->flags2 & (FF2_HIDE_DIG))) vp[vn++] = "dig";
	if ((f_ptr->flags3 & (FF3_EASY_HIDE))) vp[vn++] = "stay still";
	if ((f_ptr->flags2 & (FF2_HIDE_SWIM))) vp[vn++] = "survive without breathing";
	if ((f_ptr->flags2 & (FF2_CAN_PASS))) vp[vn++] = "pass through walls";

	/* Describe innate attacks */
	if (vn)
	{
		/* Intro */
		if (!intro)
		{
			text_out("This");
			describe_feature_type(f_ptr);
		}
		else text_out(" and");

		intro = TRUE;

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (!n) text_out(" hides monsters that can ");
			else if (n < vn-1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out(vp[n]);
		}
	}
	/* End sentence */
	if (intro) text_out(".  ");
}


/*
 * Writes the feature damage dice string into a buffer.
 *
 * Note the reckless use of a global for breath weapons.
 *
 */
bool feat_desc_damage(const feature_blow *blow_ptr, int target, int level, char *buf, int buf_size)
{
	int method = blow_ptr->method;
	method_type *method_ptr = &method_info[method];
	int d1 = blow_ptr->d_dice;
	int d2 = blow_ptr->d_side;
	int d3 = 0;

	/* Target & level unused in this implementation */
	(void)target;
	(void)level;

	/* Initialise the buffer */
	buf[0] = '\0';

	/* Use player hit points */
	if (method_ptr->flags2 & (PR2_BREATH))
	{
		/* Damage uses current hit points */
		d3 = p_ptr->depth * 6;
		d1 = 0;
		d2 = 0;
	}

	/* Consolidation */
	if (d2 == 1)
	{
		d3 += d1;
		d1 = 0;
		d2 = 0;
	}

	/* Display the damage dice */
	if ((d1) && (d2))
	{
		/* End */
		my_strcpy(buf,format("%dd%d",d1,d2), buf_size);
	}
	else if (d3)
	{
		/* End */
		my_strcpy(buf,format("%d",d3), buf_size);

		/* We are done */
		return (TRUE);
	}

	/* Anything output? */
	if (d1 || d2) return (TRUE);

	return (FALSE);
}


/*
 * Describe the feature blow
 */
static void describe_feature_blow(int f_idx)
{
	const feature_type *f_ptr = &f_info[f_idx];

	const feature_blow *blow_ptr = &f_ptr->blow;

	int method = blow_ptr->method;
	int effect = blow_ptr->effect;

	int level = f_ptr->level;

	char buf[40];

	/* Paranoia */
	if (!blow_ptr->method) return;

	/* Prepare damage string */
	feat_desc_damage(blow_ptr, 0, level, buf, 40);

	/* Describe the blow */
	describe_blow(method, effect, level, blow_ptr->d_dice * blow_ptr->d_side, "", buf, 0, 1);
}


/*
 * Describe feature spell
 */
static void describe_feature_spell(int f_idx)
{
	const feature_type *f_ptr = &f_info[f_idx];

	/* Paranoia */
	if (!f_ptr->spell) return;
}





/*
 * Describe the player miscellaneous notes on the feature.
 */
static void describe_feature_misc(int f_idx)
{
	const feature_type *f_ptr = &f_info[f_idx];

	/* Other player actions */
	if (f_ptr->flags1 & (FF1_DROP))
	{
		text_out("You can drop objects here");
		if (f_ptr->flags2 & (FF2_HIDE_ITEM))
		{
			text_out(" but they will disappear from view");
		}
		text_out(".  ");
	}

	/* Other player actions */
	if (f_ptr->flags1 & (FF1_REMEMBER))
	{
		text_out("You will remember this");
		describe_feature_type(f_ptr);
		text_out(" on your overhead map.  ");
	}

	/* Other player actions */
	if (f_ptr->flags1 & (FF1_NOTICE))
	{
		text_out("You stop running next to this");
		describe_feature_type(f_ptr);

		if (f_ptr->flags1 & (FF1_STAIRS))
		{
			text_out(" unless you have the ignore_stairs option on");
		}
		text_out(".  ");
	}

	/* Other player actions */
	if (!(f_ptr->flags1 & (FF1_MOVE)))
	{
		text_out("You can get stuck inside this");
		describe_feature_type(f_ptr);
		text_out(" should it appear around you.  ");
	}

	/* Other player actions */
	if (f_ptr->flags2 & (FF2_FILLED))
	{
		text_out("You can't breath in this.  ");
	}

	/* Take damage if trap */
	if (!(f_ptr->flags1 & (FF1_HIT_TRAP)))
	{
		bool effect = FALSE;

		if (f_ptr->blow.method)
		{
			effect = TRUE;

			text_out("Whilst in this");
			describe_feature_type(f_ptr);

			/* Hack -- should really describe blow */
			text_out(format(" you take %dd%d damage", f_ptr->blow.d_dice, f_ptr->blow.d_side));
		}

		if (f_ptr->spell)
		{
			if (effect) text_out(" and");
			else
			{
				text_out("Whilst in this");
				describe_feature_type(f_ptr);
			}

			effect = TRUE;

			/* Hack -- should really describe spell */
			text_out(" suffer its effects");
		}

		if (effect) text_out(" continuously.  ");
	}

	if (f_ptr->flags3 & (FF3_EASY_CLIMB))
	{
		text_out("You can melee huge monsters more easily from here.  ");
	}
}



/*
 * Return true if player can do this action to the feature.
 */
static bool is_player_action_valid(int f_idx, int action)
{
	const feature_type *f_ptr = &f_info[f_idx];

	/* Feature flag 1 actions */
	if (action < 32)
	{
		if ((f_ptr->flags1 & (1L << action)) &&
			((1L << action) & (FF1_SECRET | FF1_OPEN | FF1_CLOSE | FF1_BASH |
				FF1_DISARM | FF1_SPIKE | FF1_ENTER | FF1_TUNNEL | FF1_FLOOR | FF1_HIT_TRAP)))
				return (TRUE);
		/* Hack -- bashable features are opened 50% of the time */
		else if ((f_ptr->flags1 & (FF1_BASH)) &&
			((1L << action) & (FF1_OPEN)) &&
			(feat_state(f_idx, FS_BASH) != feat_state(f_idx, FS_OPEN)))
				return (TRUE);

		/* Hack -- glyphs can be set on a floor */
		else if ((f_ptr->flags1 & (FF1_FLOOR)) &&
			((1L << action) & (FF1_GLYPH)))
				return (TRUE);

		return (FALSE);
	}

	/* Feature flag 2 actions */
	else if (action < 64)
	{
		if ((f_ptr->flags2 & (1L << (action - 32))) &&
			((1L << (action - 32)) & (FF2_HURT_ROCK | FF2_HURT_FIRE | FF2_HURT_COLD | FF2_HURT_ACID |
				FF2_KILL_HUGE | FF2_KILL_MOVE)))
				return (TRUE);

		return (FALSE);
	}

	/* Feature flag 3 actions */
	else if (action < 96)
	{
		if((f_ptr->flags3 & (1L << (action - 64))) &&
			((1L << (action - 64)) & (FF3_HURT_POIS | FF3_HURT_ELEC | FF3_HURT_WATER |
				FF3_HURT_BWATER | FF3_USE_FEAT | FF3_GET_FEAT | FF3_NEED_TREE)))
				return (TRUE);

		return (FALSE);
	}

	/* Player action not valid */
	return (FALSE);
}

/*
 * Describe the player initiated transitions for this feature, and the resulting features.
 */
static void describe_feature_actions(int f_idx)
{
	const feature_type *f_ptr = &f_info[f_idx];

	int n, vn;
	cptr vp[128];

	int i;

	bool intro = FALSE;

	/* Permanent stuff never gets changed */
	if (f_ptr->flags1 & FF1_PERMANENT) return;

	/* Check all the actions */
	for (i=0; i<96;i++)
	{
		/* Do we have a known transition */
		if (is_player_action_valid(f_idx, i))
		{
			int newfeat = feat_state(f_idx, i);

			vn = 0;

			switch (i)
			{
				case FS_SECRET: vp[vn++] = "search"; break;
				case FS_OPEN:
					if (f_ptr->flags1 & (FF1_OPEN)) vp[vn++] = "open";
					if (f_ptr->flags1 & (FF1_BASH)) vp[vn++] = "bash";
					break;
				case FS_CLOSE: vp[vn++] = "close"; break;
				case FS_BASH: vp[vn++] = "bash"; break;
				case FS_DISARM: vp[vn++] ="disarm"; break;
				case FS_SPIKE: vp[vn++] ="spike"; break;
				case FS_ENTER: vp[vn++] ="enter"; break;
				case FS_TUNNEL:
					if (f_ptr->flags2 & (FF2_CAN_DIG)) vp[vn++] ="dig";
					else vp[vn++] ="tunnel";
					break;
				case FS_FLOOR: vp[vn++] ="set traps on"; break;
				case FS_GLYPH: vp[vn++] ="create glyphs on"; break;
				case FS_HIT_TRAP: vp[vn++] = "stumble on"; break;
				case FS_HURT_ROCK: vp[vn++] = "magically remove rock from"; break;
				case FS_HURT_FIRE: vp[vn++] = "burn"; break;
				case FS_HURT_COLD: vp[vn++] = "freeze"; break;
				case FS_HURT_ACID: vp[vn++] = "melt"; break;
				case FS_KILL_HUGE: vp[vn++] = "magically destroy"; break;
				case FS_KILL_MOVE: vp[vn++] ="disturb"; break;
				case FS_HURT_POIS: vp[vn++] = "poison"; break;
				case FS_HURT_ELEC: vp[vn++] = "electrify"; break;
				case FS_HURT_WATER: vp[vn++] = "flood"; break;
				case FS_HURT_BWATER: vp[vn++] = "boil"; vp[vn++] = "steam"; break;
				case FS_USE_FEAT: vp[vn++] = "use"; break;
				case FS_GET_FEAT: vp[vn++] = "gather"; break;
				case FS_NEED_TREE: vp[vn++] = "cut down"; break;
				default: break;
			}

			/* Hack -- handle some transitions */
			if (i == FS_GLYPH) newfeat = FEAT_GLYPH;
			else if (i == FS_FLOOR) newfeat = FEAT_INVIS;
			else if (i == FS_ENTER) newfeat = f_idx;

			/* Describe transitions */
			if (vn)
			{
				bool effect = FALSE;

				/* Intro */
				if (!intro)
				{
					text_out("You");
				}

				/* Note */
				intro = TRUE;

				/* Scan */
				for (n = 0; n < vn; n++)
				{
					/* Intro */
					if (n == 0) text_out(" can ");
					else if (n < vn-1) text_out(", ");
					else text_out(" or ");

					/* Dump */
					text_out(vp[n]);
				}

				text_out(" it to");

				/* Take damage if trap */
				if (((f_ptr->flags1 & (FF1_HIT_TRAP)) != 0) && ((i == FS_OPEN) || (i == FS_CLOSE) || (i == FS_BASH) || (i == FS_TUNNEL) || (i == FS_HIT_TRAP)))
				{
					if (f_ptr->blow.method)
					{
						if (effect) text_out(" and");
						effect = TRUE;

						/* Hack -- should really describe blow */
						text_out(format(" take %dd%d damage", f_ptr->blow.d_dice, f_ptr->blow.d_side));
					}

					if (f_ptr->spell)
					{
						if (effect) text_out(" and");
						effect = TRUE;

						/* Hack -- should really describe spell */
						text_out(" suffer its effects");
					}

					if ((f_ptr->flags3 & (FF3_PICK_TRAP | FF3_PICK_DOOR)) != 0)
					{
						newfeat = f_idx;
					}
				}

				/* Describe new feature */
				if ((newfeat != f_idx) || ((f_info[newfeat].flags3 & (FF3_PICK_TRAP | FF3_PICK_DOOR)) != 0))
				{
					if (effect) text_out(" and");
					effect = TRUE;

					if (i == FS_SECRET) text_out(" find ");
					else text_out(" make it ");

					if (f_info[newfeat].flags3 & (FF3_PICK_TRAP)) text_out("a random trap");
					else if (f_info[newfeat].flags3 & (FF3_PICK_DOOR)) text_out("a random door");
					else text_out(f_name + f_info[newfeat].name);

					if (cheat_xtra) text_out(format(" (%d)", newfeat));

					/* Side effects -- stop glow */
					if (((f_ptr->flags2 & (FF2_GLOW)) != 0)
						&& ((f_info[newfeat].flags2 & (FF2_GLOW)) == 0))
					{
						text_out(" and darken the surrounding grids");
					}

					/* Side effects -- start glow */
					if (((f_ptr->flags2 & (FF2_GLOW)) == 0)
						&& ((f_info[newfeat].flags2 & (FF2_GLOW)) != 0))
					{
						text_out(" and light up the surrounding grids");
					}

					/* Side effects -- remove branches */
					if (((f_ptr->flags3 & (FF3_TREE)) == 0)
						&& ((f_info[newfeat].flags3 & (FF3_TREE)) != 0))
					{
						text_out(" and remove the surrounding branches");
					}

					/* Side effects -- remove branches */
					if (((f_ptr->flags3 & (FF3_TREE)) != 0)
						&& ((f_info[newfeat].flags3 & (FF3_TREE)) == 0))
					{
						text_out(" and cover the surrounding grids with branches");
					}

					/* Side effects -- remove outside */
					if (((f_ptr->flags3 & (FF3_OUTSIDE)) != 0)
						&& ((f_info[newfeat].flags3 & (FF3_OUTSIDE)) == 0))
					{
						text_out(" and hide the surrounding grids from the sun");
					}

					/* Side effects -- remove outside */
					if (((f_ptr->flags3 & (FF3_OUTSIDE)) == 0)
						&& ((f_info[newfeat].flags3 & (FF3_OUTSIDE)) != 0))
					{
						text_out(" and expose the surrounding grids to daylight");
					}
				}

				/* Side effects -- drop / use / get object */
				if ((((f_ptr->flags1 & (FF1_HAS_GOLD | FF1_HAS_ITEM)) != 0)
					&& ((f_info[newfeat].flags1 & (FF1_HAS_GOLD | FF1_HAS_ITEM)) == 0))
					|| (f_ptr->k_idx && ((i == FS_USE_FEAT) || (i == FS_GET_FEAT) || (i == FS_DISARM))))
				{
					int count = 0;

					if (effect) text_out(" and ");
					else text_out(" ");

					effect = TRUE;

					if (f_ptr->flags3 & (FF3_DROP_1D2)) count += 2;
					if (f_ptr->flags3 & (FF3_DROP_1D3)) count += 3;

					if (i == FS_USE_FEAT)
					{
						switch(k_info[f_ptr->k_idx].tval)
						{
							case TV_RUNESTONE:
								text_out("apply, "); /* Fall through */
							case TV_SONG_BOOK:
							case TV_MAGIC_BOOK:
							case TV_PRAYER_BOOK:
								text_out("study or cast from "); break;
							case TV_INSTRUMENT:
								text_out("play "); break;
							case TV_FLASK:
							case TV_POTION:
								text_out("fill a bottle or flask to get "); break;
							default: text_out("use "); break;
						}
					}

					if (((f_ptr->flags1 & (FF1_HAS_GOLD | FF1_HAS_ITEM)) != 0) || (i == FS_DISARM) || (i == FS_GET_FEAT))
					{
						if (i == FS_USE_FEAT) text_out("or ");
						text_out("find ");
					}

					if ((f_ptr->k_idx) && ((i == FS_USE_FEAT) || (i == FS_GET_FEAT) || (i == FS_DISARM)))
					{
						object_type object_type_body;
						char o_name[80];

						object_type *o_ptr = &object_type_body;

						object_prep(o_ptr, f_ptr->k_idx);

						/* Set it to stored to prevent revealing flavours */
						o_ptr->ident |= (IDENT_STORE);

						if (count)
						{
							text_out("up to ");
							o_ptr->number = count;
						}
						else o_ptr->number = 1;

						object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 0);

						text_out(o_name);
					}
					else if (count)
					{
						text_out(format("up to %d ", count));
					}

					if (f_ptr->flags1 & (FF1_HAS_ITEM))
					{
						if (!count) text_out("an ");
						text_out(format("object%s", count > 1 ? "s" : ""));
					}

					if (f_ptr->flags1 & (FF1_HAS_GOLD))
					{
						if (f_ptr->flags1 & (FF1_HAS_ITEM)) text_out(" or ");
						else if (!count) text_out("a ");
						text_out(format("treasure%s", count > 1 ? "s" : ""));
					}
				}

				/* Side effects -- set traps or make feature dangerous */
				if ((i == FS_FLOOR) || (!(f_ptr->blow.method) && (f_info[newfeat].blow.method)))
				{
					if (effect) text_out(" and");
					effect = TRUE;

					text_out(" potentially harm monsters");
				}

				/* Side effects -- enter shop */
				if (i == FS_ENTER)
				{
					if (effect) text_out(" and");
					effect = TRUE;
					text_out(" find useful items");
				}

				/* No effect */
				if (!effect)
				{
					text_out(" no effect");
				}

				/* End sentence */
				text_out(".  ");

				/* Need intro */
				intro = FALSE;
			}
		}
	}
}


/*
 * Return true if feature does this action to itself when placed or dynamically
 */
static bool is_feature_action_valid(int f_idx, int action)
{
	const feature_type *f_ptr = &f_info[f_idx];

	/* Feature flag 1 actions that always occur */
	if (action < 32)
	{
		if (((1L << action) & (FF1_TUNNEL)) != 0)
				return (TRUE);

		return (FALSE);
	}

	/* Feature flag 2 actions */
	else if (action < 64)
	{
		if (((f_ptr->flags2 & (1L << (action - 32))) != 0) &&
			(((1L << action) & (FF2_HURT_FIRE)) != 0))
				return (TRUE);

		/* Feature flag 2 actions that always occur */
		else if (((1L << (action - 32)) & (FF2_CHASM)) != 0)
				return (TRUE);

		return (FALSE);
	}

	/* Feature flag 3 actions */
	else if (action < 96)
	{
		if (((f_ptr->flags3 & (1L << (action - 64))) != 0) &&
			(((1L << (action - 64)) & (FF3_PICK_TRAP | FF3_PICK_DOOR | FF3_NEED_TREE | FF3_INSTANT |
				FF3_ADJACENT | FF3_TIMED | FF3_ERUPT | FF3_STRIKE | FF3_SPREAD)) != 0))
				return (TRUE);

		/* Feature flag 3 actions that always occur*/
		else if (((1L << (action - 64)) & (FF3_TREE)) != 0)
				return (TRUE);

		return (FALSE);
	}

	/* Feature action not valid */
	return (FALSE);
}

/*
 * Describe the feature initiated transitions and resulting features.
 */
static void describe_feature_transitions(int f_idx)
{
	int n, vn;
	cptr vp[128];

	int i;

	bool intro = FALSE;

	/* Permanent stuff never gets changed */
	if (f_info[f_idx].flags1 & FF1_PERMANENT) return;

	/* Get the new feature */
	for (i=0;i<MAX_FEAT_STATES;i++)
	{
		/* Do we have a known transition */
		if (is_feature_action_valid(f_idx, i))
		{
			int newfeat = feat_state(f_idx, i);

			vn = 0;

			switch (i)
			{
				case FS_TUNNEL: vp[vn++] ="tunnelled through"; break;
				case FS_BRIDGE: vp[vn++] = "on the safe path"; break;
				case FS_HURT_FIRE: vp[vn++] = "on destroyed levels"; break;
				case FS_TREE: vp[vn++] = "near a tree"; break;
				case FS_NEED_TREE: vp[vn++] = "not near a tree"; break;
				case FS_INSTANT: case FS_ADJACENT: vp[vn++] = "a moment passes"; break;
				case FS_TIMED: vp[vn++] = "time passes"; break;
				case FS_SPREAD: vp[vn++] = "it spreads"; break;
				case FS_STRIKE: case FS_ERUPT: vp[vn++] = "chance dictates"; break;
				default: break;
			}

			/* Describe transitions */
			if (vn)
			{
				bool effect = FALSE;

				/* Intro */
				if (!intro)
				{
					text_out("When ");
				}

				/* Note */
				intro = TRUE;

				/* Scan */
				for (n = 0; n < vn; n++)
				{
					/* Intro */
					if ((n) && (n < vn-1)) text_out(", ");
					else text_out(" or ");

					/* Dump */
					text_out(vp[n]);
				}

				/* Describe new feature */
				if ((newfeat != f_idx) || (f_info[newfeat].flags3 & (FF3_PICK_TRAP | FF3_PICK_DOOR)))
				{
					text_out(" it becomes ");

					if (f_info[newfeat].flags3 & (FF3_PICK_TRAP)) text_out("a random trap");
					else if (f_info[newfeat].flags3 & (FF3_PICK_DOOR)) text_out("a random door");
					else text_out(f_name + f_info[newfeat].name);

					if (cheat_xtra) text_out(format(" (%d)", newfeat));
				}

				/* Side effects -- dynamic */
				if (i == FS_ADJACENT)
				{
					if (effect) text_out(" and");
					effect = TRUE;

					text_out(" it affects all adjacent grids");
				}

				/* Side effects -- erupts */
				if (i == FS_ERUPT)
				{
					if (effect) text_out(" and");
					effect = TRUE;

					text_out(" it erupts in a radius 2 ball");
				}

				/* Side effects -- erupts */
				if (i == FS_STRIKE)
				{
					if (effect) text_out(" and");
					effect = TRUE;

					text_out(" strikes a random grid nearby");
				}

				/* No effect */
				if (!effect)
				{
					text_out(" it remains unchanged");
				}

				/* End sentence */
				text_out(".  ");

				/* Need intro */
				intro = FALSE;
			}
		}
	}
}




/*
 * Hack -- display feature information using "roff()"
 *
 *
 * This function should only be called with the cursor placed at the
 * left edge of the screen or line, on a cleared line, in which the output is
 * to take place.  One extra blank line is left after the recall.
 */
void describe_feature(int f_idx)
{
	/* Describe the movement and level of the monster */
	describe_feature_basic(f_idx);

	/* Describe the movement, LOS, and projection for player and monsters */
	describe_feature_player_moves(f_idx);

	/* Describe the movement, LOS, and projection for player and monsters */
	describe_feature_monster_moves(f_idx);

	/* Describe the movement, LOS, and projection for player and monsters */
	describe_feature_misc(f_idx);

	/* Describe feature actions */
	describe_feature_actions(f_idx);

	/* Describe feature transitions */
	describe_feature_transitions(f_idx);

	/* All done */
	text_out("\n");
}





/*
 * Hack -- Display the "name" and "attr/chars" of a feature
 */
void feature_roff_top(int f_idx)
{
	feature_type *f_ptr = &f_info[f_idx];

	byte a1, a2;
	char c1, c2;

	/* Get the chars */
	c1 = f_ptr->d_char;
	c2 = f_ptr->x_char;

	/* Get the attrs */
	a1 = f_ptr->d_attr;
	a2 = f_ptr->x_attr;

	/* Clear the top line */
	Term_erase(0, 0, 255);

	/* Reset the cursor */
	Term_gotoxy(0, 0);

	/* Dump the name */
	Term_addstr(-1, TERM_WHITE, format("%^s",f_name + f_ptr->name));


	/* Append the "standard" attr/char info */
	Term_addstr(-1, TERM_WHITE, " ('");
	Term_addch(a1, c1);
	Term_addstr(-1, TERM_WHITE, "')");

	if (!(use_trptile) && !(use_dbltile))
	{
		/* Append the "optional" attr/char info */
		Term_addstr(-1, TERM_WHITE, "/('");
		Term_addch(a2, c2);
		if (use_bigtile && (a2 & 0x80)) Term_addch(255, -1);
		Term_addstr(-1, TERM_WHITE, "'):");
	}
}



/*
 * Hack -- describe the given feature at the top of the screen
 */
void screen_feature_roff(int f_idx)
{
	/* Flush messages */
	message_flush();

	/* Begin recall */
	Term_erase(0, 1, 255);

	/* Output to the screen */
	text_out_hook = text_out_to_screen;

	/* Recall feature */
	describe_feature(f_idx);

	/* Describe feature */
	feature_roff_top(f_idx);

}


/*
 * Hack -- describe the given feature in the current "term" window
 */
void display_feature_roff(int f_idx)
{
	int y;

	/* Erase the window */
	for (y = 0; y < Term->hgt; y++)
	{
		/* Erase the line */
		Term_erase(0, y, 255);
	}

	/* Begin recall */
	Term_gotoxy(0, 1);

	/* Output to the screen */
	text_out_hook = text_out_to_screen;

	/* Recall feature */
	describe_feature(f_idx);

	/* Describe feature  */
	feature_roff_top(f_idx);
}


/*
 * Hack -- display region information.
 */
void describe_region_basic(region_type *r_ptr, const char *intro)
{
	int n, vn;
	cptr vp[128];

	bool intro_hack = FALSE;
	bool nomove_hack = FALSE;
	
	text_out(format("%s a", intro));

	/* Collect sight and movement */
	vn = 0;

	/* Collect basic behaviour */
	if (r_ptr->flags1 & (RE1_CLOCKWISE | RE1_COUNTER_CLOCKWISE)) vp[vn++] = "spinning";
	if (r_ptr->flags1 & (RE1_SPREAD)) vp[vn++] = "spreading";
	if (r_ptr->flags1 & (RE1_CHAIN)) vp[vn++] = "jumping";
	if (r_ptr->flags1 & (RE1_SEEKER | RE1_WALL)) vp[vn++] = "moving";

	/* Describe innate attacks */
	if (vn)
	{
		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (!n)
			{
				if (r_ptr->flags1 & (RE1_RANDOM))
				{
					text_out(" randomly ");
				}
				else
				{
					/* Vowel? */
					if (is_a_vowel(vp[0][0])) text_out("n ");
					else text_out(" ");
				}
			}
			else if (n < vn-1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out(vp[n]);
			
			intro_hack = TRUE;
		}
	}

	/* Describe shape */
	text_out(" ");
	text_out(region_name + region_info[r_ptr->type].name);
	
	/* Describe the shape */
	describe_blow(r_ptr->method, 0, r_ptr->level, r_ptr->damage, "", 0, (DESC_SKIP_METHOD_INTRO) | (DESC_SKIP_EFFECT), 1);
	
	/* Collect behaviour */
	vn = 0;

	/* Collect behaviour */
	if (r_ptr->flags1 & (RE1_SHINING)) vp[vn++] = "shines with its own light";
	if (r_ptr->flags1 & (RE1_SEEKER)) vp[vn++] = "seeks out nearby enemies";
	if (r_ptr->flags1 & (RE1_WALL)) vp[vn++] = "advances in a straight line";
	if (r_ptr->flags1 & (RE1_SCALAR_VECTOR)) vp[vn++] = "explodes outwards from a single point";
	if (r_ptr->flags1 & (RE1_LINGER)) vp[vn++] = "lingers, damaging all who remain inside or move through it";

	/* Describe innate attacks */
	if (vn)
	{
		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (!n) text_out(" which ");
			else if (n < vn-1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out(vp[n]);
			
			intro_hack = TRUE;
		}
	}

	/* Collect more behaviour */
	vn = 0;

	/* Collect future behaviour */
	if ((r_ptr->flags1 & (RE1_CLOCKWISE)) && (r_ptr->flags1 & (RE1_COUNTER_CLOCKWISE))) vp[vn++] = "change direction";
	if (((r_ptr->flags1 & (RE1_ACCELERATE)) != 0) &&
			(((r_ptr->flags1 & (RE1_DECELERATE)) == 0) || (r_ptr->age < r_ptr->lifespan / 2))) vp[vn++] = "go faster and faster";
	if (((r_ptr->flags1 & (RE1_DECELERATE)) != 0) &&
			((r_ptr->flags1 & (RE1_ACCELERATE)) != 0) && (r_ptr->age < r_ptr->lifespan / 2)) vp[vn++] = "eventually begin to decelerate";
	else if ((r_ptr->flags1 & (RE1_DECELERATE)) != 0) vp[vn++] = "go slower and slower";
	if (((r_ptr->flags1 & (RE1_MOVE_SOURCE)) == 0) &&
			((r_ptr->flags1 & (RE1_CHAIN | RE1_SEEKER | RE1_SCALAR_VECTOR | RE1_SPREAD | RE1_WALL)) == 0)) { vp[vn++] = "remain stationary"; nomove_hack = TRUE; }

	/* Describe innate attacks */
	if (vn)
	{
		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (!n)
			{
				text_out(format(" %s will ", intro_hack ? "and" : "which"));
			}
			else if (n < vn-1) text_out(", ");
			else if ((n > 1) && (nomove_hack)) text_out(" but otherwise ");
			else text_out(" and ");

			/* Dump */
			text_out(vp[n]);
			
			intro_hack = TRUE;
		}
	}
}


/*
 * Hack -- display region attacks.
 */
void describe_region_attacks(region_type *r_ptr, const char *intro, const char *damage, const char *lasts)
{
	int n, vn;
	cptr vp[128];

	bool intro_hack = FALSE;
	
	region_info_type *ri_ptr = &region_info[r_ptr->type];
		
	vn = 0;

	/* Begin this sentence */
	text_out(format("%s ", intro));
	
	intro_hack = FALSE;

	if (r_ptr->flags1 & (RE1_AUTOMATIC)) text_out("automatically ");

	/* Hitting a trap - note check to ensure we are really in the dungeon */
	if ((r_ptr->flags1 & (RE1_HIT_TRAP)) && (r_ptr->y0) && (r_ptr->x0))
	{
		int feat = cave_feat[r_ptr->y0][r_ptr->x0];

		/* Describe the object */
		if (cave_o_idx[r_ptr->y0][r_ptr->x0])
		{

		}
		/* Describe the feature blow */
		else if (f_info[feat].blow.method)
		{
			/* Describe feature blow */
			describe_feature_blow(feat);
		}
		else if (f_info[feat].spell)
		{
			/* Describe feature spell */
			describe_feature_spell(feat);
		}
	}
	else
	{
		/* Hack -- basic attack */
		if (!ri_ptr->method) text_out("attacks ");
		
		/* Describe the blow */
		describe_blow(ri_ptr->method ? ri_ptr->method : 0, r_ptr->effect, r_ptr->level, r_ptr->damage, "", damage, 0, 1);
	}

	/* Collect methods of triggering */
	if (r_ptr->flags1 & (RE1_TRIGGER_MOVE)) vp[vn++] = "movement";
	if (r_ptr->flags1 & (RE1_TRIGGER_DROP)) vp[vn++] = "throwing an item into the area of effect";
	if ((r_ptr->flags1 & (RE1_TRIGGER_OPEN)) && !(r_ptr->age)) vp[vn++] = "opening a lever";
	if ((r_ptr->flags1 & (RE1_TRIGGER_OPEN)) && !(r_ptr->age)) vp[vn++] = "unbarring magic";

	/* Describe innate attacks */
	if (vn)
	{
		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (!n) text_out(" and is triggered by ");
			else if ((n) && (n < vn-1)) text_out(", ");
			else text_out(" ");

			/* Dump */
			text_out(vp[n]);
		}
		
		intro_hack = TRUE;
	}

	vn = 0;

	/* Collect methods of stopping trap */
	if (r_ptr->flags1 & (RE1_TRIGGER_CLOSE)) vp[vn++] = "closing a lever";
	if (r_ptr->flags1 & (RE1_TRIGGER_CLOSE)) vp[vn++] = "wizard lock magic";

	/* Describe innate attacks */
	if (vn)
	{
		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (!n) text_out(format(" %sis stopped by ", intro_hack ? "and" : ""));
			else if ((n) && (n < vn-1)) text_out(", ");
			else text_out(" ");

			/* Dump */
			text_out(vp[n]);
			
			intro_hack = FALSE;
		}

		if (!r_ptr->age) text_out("when triggered");
	}

	/* Trigger recharges */
	if (((r_ptr->flags1 & (RE1_AUTOMATIC)) == 0) && (r_ptr->delay || r_ptr->delay_reset)) text_out(" recharging");

	/* Clear intro again */
	intro_hack = FALSE;
	
	/* Time to take effect */
	if (r_ptr->delay)
	{
		int energy = r_ptr->delay * extract_energy[p_ptr->pspeed];
		
		text_out(format(" %s %d game ticks", r_ptr->delay != r_ptr->delay_reset ? "in" : "every", r_ptr->delay * 10));
		text_out(format(", which is %d.%d turns at your current speed", energy / 100, (energy / 10) % 10));

		intro_hack = TRUE;
	}

	/* Time to take effect */
	if ((r_ptr->delay_reset) && ((r_ptr->delay != r_ptr->delay_reset) || ((r_ptr->flags1 & (RE1_ACCELERATE | RE1_DECELERATE)) != 0)))
	{
		int delay_reset = r_ptr->delay_reset;
		int energy;

		if (intro_hack) text_out(" and then");

		/* Accelerating */
		if ((r_ptr->flags1 & (RE1_ACCELERATE)) && (!(r_ptr->flags1 & (RE1_DECELERATE)) || (r_ptr->age < r_ptr->lifespan / 2)))
		{
			if (delay_reset < 3) delay_reset -= 1;
			delay_reset -= delay_reset / 3;
			if (delay_reset < 1) delay_reset = 1;
		}

		/* Decelerating */
		if ((r_ptr->flags1 & (RE1_DECELERATE)) && (!(r_ptr->flags1 & (RE1_ACCELERATE)) || (r_ptr->age > r_ptr->lifespan / 2)))
		{
			delay_reset += delay_reset / 3;
			if (delay_reset < 3) delay_reset += 1;
		}
		
		/* Now we've computed the modified delay, compute the player turns */
		energy = delay_reset * extract_energy[p_ptr->pspeed];
		
		text_out(format(" %s %d game ticks", r_ptr->delay_reset != delay_reset ? "in" : "every", delay_reset * 10));
		text_out(format(", which is %d.%d turns at your current speed", energy / 100, (energy / 10) % 10));
		
		intro_hack = TRUE;
	}
	
	/* Measure lifespan */
	if ((r_ptr->lifespan) && (r_ptr->lifespan < 10000))
	{
		int energy;
		int i;
		int delay = 0;
		int delay_current = r_ptr->delay_reset;

		for (i = r_ptr->age; i < r_ptr->lifespan; i++)
		{
			delay += delay_current;

			if ((r_ptr->flags1 & (RE1_ACCELERATE)) && (!(r_ptr->flags1 & (RE1_DECELERATE)) || (i < r_ptr->lifespan / 2)))
			{
				if (delay_current < 3) delay_current -= 1;
				delay_current -= delay_current / 3;
				if (delay_current < 1) delay_current = 1;
			}

			/* Decelerating */
			if ((r_ptr->flags1 & (RE1_DECELERATE)) && (!(r_ptr->flags1 & (RE1_ACCELERATE)) || (i > r_ptr->lifespan / 2)))
			{
				delay_current += delay_current / 3;
				if (delay_current < 3) delay_current += 1;
			}
		}

		energy = delay * extract_energy[p_ptr->pspeed];
		
		text_out(format(" %stakes effect %s%s", intro_hack ? "and " : "", r_ptr->age ? "another " : "", lasts));
		text_out(format(", or %s%d.%d turns at your current speed", strchr(lasts, 'd') ? "around " : "", energy / 100, (energy / 10) % 10));
	}
}


/*
 * Hack -- display region information
 *
 *
 * This function should only be called with the cursor placed at the
 * left edge of the screen or line, on a cleared line, in which the output is
 * to take place.  One extra blank line is left after the recall.
 */
void describe_region(region_type *r_ptr)
{
	char damage[6];
	char lasts[8];
	
	/* Initialise the strings */
	my_strcpy(damage, format("%d", r_ptr->damage), sizeof(damage));
	my_strcpy(lasts, format("%d time%s", r_ptr->lifespan - r_ptr->age, r_ptr->lifespan - r_ptr->age != 1 ? "s" : ""), sizeof(lasts));
	
	/* Describe the movement and shape of the region */
	describe_region_basic(r_ptr, "This is");

	/* Describe the region attacks */
	describe_region_attacks(r_ptr, ".  It", damage, lasts);
	
	/* End sentence */
	text_out(".  ");

	/* All done */
	text_out("\n");
}




/*
 * Hack -- Display the "name" and "attr/chars" of a region
 */
void region_top(const region_type *r_ptr)
{
	/* Clear the top line */
	Term_erase(0, 0, 255);

	/* Reset the cursor */
	Term_gotoxy(0, 0);

	/* Dump the name */
	Term_addstr(-1, TERM_L_BLUE, region_name + region_info[r_ptr->type].name);
}


/*
 * Display an object at the top of the screen
 */
void screen_region(region_type *r_ptr)
{
	/* Flush messages */
	message_flush();

	/* Set text_out hook */
	text_out_hook = text_out_to_screen;

	/* Begin recall */
	Term_gotoxy(0, 1);

	/* Actually display the region */
	describe_region(r_ptr);

	/* Display item name */
	region_top(r_ptr);
}


/*
 * Hack -- describe the given region in the current "term" window
 */
void display_region(region_type *r_ptr)
{
	int y;

	/* Erase the window */
	for (y = 0; y < Term->hgt; y++)
	{
		/* Erase the line */
		Term_erase(0, y, 255);
	}

	/* Begin recall */
	Term_gotoxy(0, 1);

	/* Output to the screen */
	text_out_hook = text_out_to_screen;

	/* Recall feature */
	describe_region(r_ptr);

	/* Describe feature  */
	region_top(r_ptr);
}
