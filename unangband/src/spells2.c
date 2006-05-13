/* File: spells2.c */

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
 * Increase players hit points, notice effects
 */
bool hp_player(int num)
{
	/* Healing needed */
	if (p_ptr->chp < p_ptr->mhp)
	{
		/* Gain hitpoints */
		p_ptr->chp += num;

		/* Enforce maximum */
		if (p_ptr->chp >= p_ptr->mhp)
		{
			p_ptr->chp = p_ptr->mhp;
			p_ptr->chp_frac = 0;
		}

		/* Redraw */
		p_ptr->redraw |= (PR_HP);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1 | PW_PLAYER_2 | PW_PLAYER_3);

		/* Heal 0-4 */
		if (num < 5)
		{
			msg_print("You feel a little better.");
		}

		/* Heal 5-14 */
		else if (num < 15)
		{
			msg_print("You feel better.");
		}

		/* Heal 15-34 */
		else if (num < 35)
		{
			msg_print("You feel much better.");
		}

		/* Heal 35+ */
		else
		{
			msg_print("You feel very good.");
		}

		/* Notice */
		return (TRUE);
	}

	/* Ignore */
	return (FALSE);
}



/*
 * Leave a "glyph of warding" which prevents monster movement
 */
void warding_glyph(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	/* XXX XXX XXX */
	if (!cave_clean_bold(py, px))
	{
		msg_print("The object resists the spell.");
		return;
	}

	/* Create a glyph */
	cave_set_feat(py, px, FEAT_GLYPH);
}

/*
 * Leave a "trap" which affects those moving on it
 */
void warding_trap(int feat, int dir)
{
	int ty = p_ptr->py + ddy[dir];
	int tx = p_ptr->px + ddx[dir];

	/* XXX XXX XXX */
	if (!cave_clean_bold(ty, tx))
	{
		msg_print("The object resists the spell.");
		return;
	}

	/* Create a trap */
	cave_set_feat(ty, tx, feat);
}


/*
 * Array of stat "descriptions"
 */
static cptr desc_stat_pos[] =
{
	"strong",
	"smart",
	"wise",
	"dextrous",
	"healthy",
	"cute"
};


/*
 * Array of stat "descriptions"
 */
static cptr desc_stat_neg[] =
{
	"weak",
	"stupid",
	"naive",
	"clumsy",
	"sickly",
	"ugly"
};


/*
 * Lose a "point"
 */
bool do_dec_stat(int stat)
{
	/* Sustain */
	if ((p_ptr->cur_flags2 & (TR2_SUST_STR << stat)) != 0)
	{
		/* Message */
		msg_format("You feel very %s for a moment, but the feeling passes.",
			   desc_stat_neg[stat]);

		/* Always notice */
		equip_can_flags(0x0L,(TR2_SUST_STR<<stat),0x0L,0x0L);

		/* Notice effect */
		return (TRUE);
	}

	/* Attempt to reduce the stat */
	if (dec_stat(stat, 10, FALSE))
	{
		/* Message */
		msg_format("You feel very %s.", desc_stat_neg[stat]);

		/* Always notice */
		equip_not_flags(0x0L,(TR2_SUST_STR<<stat),0x0L,0x0L);

		/* Notice effect */
		return (TRUE);
	}

	/* Nothing obvious */
	return (FALSE);
}


/*
 * Restore lost "points" in a stat
 */
bool do_res_stat(int stat)
{
	/* Attempt to increase */
	if (res_stat(stat))
	{
		/* Message */
		msg_format("You feel less %s.", desc_stat_neg[stat]);

		/* Notice */
		return (TRUE);
	}

	/* Nothing obvious */
	return (FALSE);
}


/*
 * Gain a "point" in a stat
 */
bool do_inc_stat(int stat)
{
	bool res;

	/* Restore strength */
	res = res_stat(stat);

	/* Attempt to increase */
	if (inc_stat(stat))
	{
		/* Message */
		msg_format("You feel very %s!", desc_stat_pos[stat]);

		/* Notice */
		return (TRUE);
	}

	/* Restoration worked */
	if (res)
	{
		/* Message */
		msg_format("You feel less %s.", desc_stat_neg[stat]);

		/* Notice */
		return (TRUE);
	}

	/* Nothing obvious */
	return (FALSE);
}

/*
 * Identify everything being carried.
 */
void identify_pack(void)
{
	int i;

	/* Simply identify and know every item */
	for (i = 0; i < INVEN_TOTAL+1; i++)
	{
		object_type *o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Aware and Known */
		object_aware(o_ptr);
		object_known(o_ptr);
	}

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);
}






/*
 * Used by the "enchant" function (chance of failure)
 */
static int enchant_table[16] =
{
	0, 10,  50, 100, 200,
	300, 400, 500, 700, 950,
	990, 992, 995, 997, 999,
	1000
};


/*
 * Hack -- Removes curse from an object.
 */
static void uncurse_object(object_type *o_ptr)
{
	/* Uncurse it */
	o_ptr->ident &= ~(IDENT_CURSED);

	/* Take note */
	o_ptr->feeling = INSCRIP_UNCURSED;

	/* The object has been "sensed" */
	o_ptr->ident |= (IDENT_SENSE);
}


/*
 * Removes curses from items in inventory.
 *
 * Note that Items which are "Perma-Cursed" (The One Ring,
 * The Crown of Morgoth) can NEVER be uncursed.
 *
 * Note that if "all" is FALSE, then Items which are
 * "Heavy-Cursed" (Mormegil, Calris, and Weapons of Morgul)
 * will not be uncursed.
 */
static int remove_curse_aux(int all)
{
	int i, cnt = 0;

	/* Attempt to uncurse items being worn */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		u32b f1, f2, f3, f4;

		object_type *o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Uncursed already */
		if (!cursed_p(o_ptr)) continue;

		/* Extract the flags */
		object_flags(o_ptr, &f1, &f2, &f3, &f4);

		/* Heavily Cursed Items need a special spell */
		if (!all && (f3 & (TR3_HEAVY_CURSE)))
		{
			/* Learn about the object */
			object_can_flags(o_ptr,0x0L,0x0L,TR3_HEAVY_CURSE,0x0L);

			continue;
		}

		/* Perma-Cursed Items can NEVER be uncursed */
		if (f3 & (TR3_PERMA_CURSE))
		{
			/* Learn about the object */
			if (all) object_can_flags(o_ptr,0x0L,0x0L,TR3_PERMA_CURSE,0x0L);

			continue;
		}

		/* Learn about the object */
		if (!all) object_not_flags(o_ptr,0x0L,0x0L,TR3_HEAVY_CURSE,0x0L);

		/* Learn about the object */
		object_not_flags(o_ptr,0x0L,0x0L,TR3_PERMA_CURSE,0x0L);

		/* Uncurse the object */
		uncurse_object(o_ptr);

		/* Recalculate the bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Window stuff */
		p_ptr->window |= (PW_EQUIP);

		/* Count the uncursings */
		cnt++;
	}

	/* Return "something uncursed" */
	return (cnt);
}


/*
 * Remove most curses
 */
bool remove_curse(void)
{
	return (remove_curse_aux(FALSE));
}

/*
 * Remove all curses
 */
bool remove_all_curse(void)
{
	return (remove_curse_aux(TRUE));
}



/*
 * Restores any drained experience
 */
bool restore_level(void)
{
	/* Restore experience */
	if (p_ptr->exp < p_ptr->max_exp)
	{
		/* Message */
		msg_print("You feel your life energies returning.");

		/* Restore the experience */
		p_ptr->exp = p_ptr->max_exp;

		/* Check the experience */
		check_experience();

		/* Did something */
		return (TRUE);
	}

	/* No effect */
	return (FALSE);
}

/*
 *  Hack -- exclude the following flags from equipment self-knowledge except weapons.
 */
#define TR1_WEAPON_FLAGS (TR1_SLAY_ORC | TR1_SLAY_TROLL | TR1_SLAY_GIANT |\
			TR1_SLAY_DRAGON | TR1_SLAY_UNDEAD | TR1_SLAY_DEMON |\
			TR1_SLAY_NATURAL | TR1_BRAND_HOLY | TR1_KILL_DRAGON |\
			TR1_KILL_UNDEAD | TR1_KILL_DEMON | TR1_BRAND_ACID |\
			TR1_BRAND_FIRE | TR1_BRAND_POIS | TR1_BRAND_ELEC |\
			TR1_BRAND_COLD)

#define TR2_WEAPON_FLAGS 0x0L

#define TR3_WEAPON_FLAGS (TR3_IMPACT | TR3_BLESSED | TR3_THROWING)

#define TR4_WEAPON_FLAGS (TR4_VAMP_HP | TR4_VAMP_MANA |\
			  TR4_BRAND_LITE | TR4_BRAND_DARK | TR4_SLAY_MAN | TR4_SLAY_ELF |\
			  TR4_SLAY_DWARF)
/*
 *  Hack -- exclude the following flags from equipment self-knowledge as they never apply
 *  directly to the player.
 */
#define TR1_IGNORE_FLAGS (TR1_WEAPON_FLAGS)

#define TR2_IGNORE_FLAGS (TR2_WEAPON_FLAGS | TR2_IGNORE_MASK)

#define TR3_IGNORE_FLAGS (TR3_WEAPON_FLAGS | TR3_ACTIVATE | TR3_RANDOM | TR3_INSTA_ART |\
			  TR3_EASY_KNOW | TR3_HIDE_TYPE | TR3_SHOW_MODS)

#define TR4_IGNORE_FLAGS (TR4_WEAPON_FLAGS)



/*
 * Hack -- acquire self knowledge
 *
 * List various information about the player and/or his current equipment.
 */
void self_knowledge(bool spoil)
{
	int i, n;

	u32b f1 = 0L, f2 = 0L, f3 = 0L, f4 = 0L;

	u32b t1, t2, t3, t4;

	object_type *o_ptr;

	cptr vp[64];

	int vn;

	bool healthy = TRUE;

	/* Save screen */
	screen_save();

	/* Clear the screen */
	Term_clear();

	/* Set text_out hook */
	text_out_hook = text_out_to_screen;
	
	text_out_c(TERM_L_BLUE, "Self-knowledge\n");

	if (p_ptr->blind)
	{
		text_out("You cannot see.  ");
		healthy = FALSE;
	}
	if (p_ptr->confused)
	{
		text_out("You are confused.  ");
		healthy = FALSE;
	}
	if (p_ptr->afraid)
	{
		text_out("You are terrified.  ");
		healthy = FALSE;
	}
	if (p_ptr->cut)
	{
		text_out("You are bleeding.  ");
		healthy = FALSE;
	}
	if (p_ptr->stun)
	{
		text_out("You are stunned.  ");
		healthy = FALSE;
	}
	if (p_ptr->poisoned)
	{
		text_out("You are poisoned.  ");
		healthy = FALSE;
	}
	if (p_ptr->image)
	{
		text_out("You are hallucinating.  ");
		healthy = FALSE;
	}

	if (p_ptr->disease)
	{
		text_out("You are afflicted with ");
		healthy = FALSE;

		/* Collect symptoms */
		vn = 0;
		for (i = 1, n = 0; n < DISEASE_TYPES_HEAVY; i <<= 1, n++)
		{
			if ((p_ptr->disease & i) != 0) vp[vn++] = disease_name[n];
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

		/* Collect causes */
		vn = 0;
		for (i = (1 << DISEASE_TYPES_HEAVY), n = DISEASE_TYPES_HEAVY; n < 32; i <<= 1, n++)
		{
			if ((p_ptr->disease & i) != 0) vp[vn++] = disease_name[n];
		}

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) { if ((p_ptr->disease & ((1 << DISEASE_TYPES_HEAVY) -1)) != 0) text_out (" caused by "); }
			else if (n < vn-1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out(vp[n]);
		}

		/* Dump */
		text_out(".  ");

	}

	if (healthy)
	{
		text_out("You suffer from no afflictions.  ");
	}

	if (p_ptr->blessed)
	{
		text_out("You feel rightous.  ");
	}
	if (p_ptr->hero)
	{
		text_out("You feel heroic.  ");
	}
	if (p_ptr->shero)
	{
		text_out("You are in a battle rage.  ");
	}
	if ((p_ptr->protevil) || (p_ptr->shield) || (p_ptr->hero) || (p_ptr->shero))
	{

		text_out("You are protected ");

		/* Collect protections */
		vn = 0;

		if (p_ptr->protevil) vp[vn++]="from evil";
		if ((p_ptr->hero) || (p_ptr->shero)) vp[vn++]="from fear";
		if (p_ptr->shield) vp[vn++]="by a mystic sheild";

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

		if (n) text_out(".  ");

	}

	if (p_ptr->climbing)
	{
		text_out("You are climbing over an obstacle.  ");
	}
	if (p_ptr->searching)
	{
		text_out("You are looking around very carefully.  ");
	}
	if (p_ptr->new_spells)
	{
		text_out("You can learn some spells/prayers.  ");
	}

	if (p_ptr->word_recall)
	{
		text_out("You will soon be recalled.  ");
	}

	/* Hack -- timed abilities that may also be from equipment */
	if (p_ptr->tim_infra)
	{
		text_out("Your eyes are temporarily sensitive to infrared light.  ");
	}

	if (p_ptr->tim_invis)
	{
		text_out("You can temporarily see invisible monsters.  ");

	}

	/* Collect temporary effects */
	vn = 0;

	if (p_ptr->invuln) vp[vn++] = "invulnerable";
	for (n = 0; n < A_CHR; n++)
	{
		if (p_ptr->stat_inc_tim[n]) vp[vn++] = desc_stat_imp[n];
	}
	for (n = 0; n < A_CHR; n++)
	{
		if (p_ptr->stat_dec_tim[n]) vp[vn++] = desc_stat_dec[n];
	}

	if ((p_ptr->oppose_acid) || (p_ptr->oppose_elec) || (p_ptr->oppose_fire) || (p_ptr->oppose_cold)) vp[vn++]= "resistant to";

	/* Introduce */
	if (vn) text_out("You are temporarily ");

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

	if ((p_ptr->oppose_acid) || (p_ptr->oppose_elec) || (p_ptr->oppose_fire) || (p_ptr->oppose_cold))
	{
		/* Collect temporary resistances */
		vn = 0;

		if (p_ptr->oppose_acid) vp[vn++]= "acid";
		if (p_ptr->oppose_elec) vp[vn++]= "electricity";
		if (p_ptr->oppose_fire) vp[vn++]= "fire";
		if (p_ptr->oppose_cold) vp[vn++]= "cold";

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

		text_out(".  ");

	}
	else if (vn)
	{
		text_out(".  ");
	}

	text_out("\n");

	/* Hack -- racial effects */
	if (rp_ptr->flags1 || rp_ptr->flags2 || rp_ptr->flags3 || rp_ptr->flags4)
	{
		text_out("Your race affects you.  ");

		list_object_flags(rp_ptr->flags1,rp_ptr->flags1,rp_ptr->flags1,rp_ptr->flags4,1);

		text_out("\n");
	}

	/* Get player flags */
	player_flags(&t1,&t2,&t3,&t4);

	/* Eliminate race flags */
	t1 &= ~(rp_ptr->flags1);
	t2 &= ~(rp_ptr->flags2);
	t3 &= ~(rp_ptr->flags3);
	t4 &= ~(rp_ptr->flags4);

	/* Hack -- class effects */
	if (t1 || t2 || t3 || t4)
	{
		text_out("Your training affects you.  ");

		list_object_flags(t1,t2,t3,t4,1);

		text_out("\n");
	}

	/* Get item flags from equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		o_ptr = &inventory[i];

		/* Clear the flags */
		t1 = t2 = t3 = t4 = 0L;

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Extract the flags */
		if (spoil) object_flags(o_ptr, &t1, &t2, &t3, &t4);
		else object_flags_known(o_ptr, &t1, &t2, &t3, &t4);

		/* Extract flags */
		f1 |= t1 & ~(TR1_IGNORE_FLAGS);
		f2 |= t2 & ~(TR2_IGNORE_FLAGS);
		f3 |= t3 & ~(TR3_IGNORE_FLAGS);
		f4 |= t4 & ~(TR4_IGNORE_FLAGS);
	}

	/* Hack -- other equipment effects */
	if ((f1) || (f2) || (f3) || (f4))
	{
		text_out("Your equipment affects you.  ");

		list_object_flags(f1,f2,f3,f4,1);

		if (spoil)
		{
			equip_can_flags(f1,f2,f3,f4);

			equip_not_flags(~(f1 | TR1_IGNORE_FLAGS),~(f2 | TR2_IGNORE_FLAGS),~(f3 | TR3_IGNORE_FLAGS), ~(f4 | TR4_IGNORE_FLAGS));
		}

		text_out("\n");
	}

	o_ptr = &inventory[INVEN_WIELD];

	if (o_ptr->k_idx)
	{
		/* Clear the flags */
		t1 = t2 = t3 = t4 = 0L;

		/* Extract the flags */
		if (spoil) object_flags(o_ptr, &t1, &t2, &t3, &t4);
		else object_flags_known(o_ptr, &t1, &t2, &t3, &t4);

		/* Extract weapon flags */
		t1 = t1 & (TR1_WEAPON_FLAGS);
		t2 = t2 & (TR2_WEAPON_FLAGS);
		t3 = t3 & (TR3_WEAPON_FLAGS);
		t4 = t4 & (TR4_WEAPON_FLAGS);

		/* Hack -- weapon effects */
		if (t1 || t2 || t3 || t4)
		{
			text_out("Your weapon has special powers.  ");
	
			list_object_flags(t1,t2,t3,t4,1);

			if (spoil)
			{	
				object_can_flags(o_ptr,t1,t2,t3,t4);
	
				object_not_flags(o_ptr,TR1_WEAPON_FLAGS & ~(t1),TR2_WEAPON_FLAGS & ~(t2),TR3_WEAPON_FLAGS & ~(t3), TR4_WEAPON_FLAGS & ~(t4));
			}

			text_out("\n");
		}

	}

	(void)anykey();

	/* Load screen */
	screen_load();
}






/*
 * Forget everything. This really sucks now.
 */
bool lose_all_info(void)
{
	int i;

	/* Forget info about objects */
	for (i = 0; i < INVEN_TOTAL+1; i++)
	{
		object_type *o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Allow "protection" by the MENTAL flag */
		if (o_ptr->ident & (IDENT_MENTAL)) continue;

		/* Remove special inscription, if any */
		o_ptr->feeling = 0;

		/* Hack -- Clear the "felt" flag */
		o_ptr->ident &= ~(IDENT_SENSE);

		/* Hack -- Clear the "bonus" flag */
		o_ptr->ident &= ~(IDENT_BONUS);

		/* Hack -- Clear the "known" flag */
		o_ptr->ident &= ~(IDENT_KNOWN);
	}

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);

	/* Mega-Hack -- Forget the map */
	wiz_dark();

	/* It worked */
	return (TRUE);
}



/*
 *  Set word of recall as appropriate
 */
void set_recall(void)
{
	/* Ironman */
	if (adult_ironman && !p_ptr->total_winner)
	{
		msg_print("Nothing happens.");
		return;
	}

	/* Activate recall */
	if (!p_ptr->word_recall)
	{
		/* Reset recall depth */
		if ((p_ptr->depth > min_depth(p_ptr->dungeon)) && (p_ptr->depth != p_ptr->max_depth))
		{
			/*
			 * ToDo: Add a new player_type field "recall_depth"
			 * ToDo: Poll: Always reset recall depth?
			 */
			 if (get_check("Reset recall depth? "))
				p_ptr->max_depth = p_ptr->depth;
		}

		p_ptr->word_recall = rand_int(20) + 15;
		msg_print("The air about you becomes charged...");
	}

	/* Deactivate recall */
	else
	{
		p_ptr->word_recall = 0;
		msg_print("A tension leaves the air around you...");
	}
}


/*
 * Detect all features on current panel
 */
bool detect_feat_flags(u32b flags1, u32b flags2, int r)
{
	int y, x;

	bool detect = FALSE;

	/* Scan the grids out to radius */
	for (y = MAX(p_ptr->py - r, 0); y < MIN(p_ptr->py + r, DUNGEON_HGT); y++)
	{
		for (x = MAX(p_ptr->px - r, 0); x < MIN(p_ptr->px+r, DUNGEON_WID); x++)
		{
			/* Check distance */
			if (distance(p_ptr->py, p_ptr->px, y, x) > r) continue;

			/* Safe from traps */
			if (view_unsafe_grids && (flags1 & (FF1_TRAP)))
			{
				play_info[y][x] |= (PLAY_SAFE);
				lite_spot(y,x);

				detect = TRUE;
			}

			/* Safe from monsters */
			if (view_detect_grids && !(flags1) && !(flags2))
			{
				play_info[y][x] |= (PLAY_SAFE);
				lite_spot(y,x);

				detect = TRUE;
			}

			/* Detect flags */
			if ((f_info[cave_feat[y][x]].flags1 & (flags1)) ||
				(f_info[cave_feat[y][x]].flags2 & (flags2)))
			{
				/* Detect secrets */
				if (f_info[cave_feat[y][x]].flags1 & (FF1_SECRET))
				{
					/*Find secrets*/
					cave_alter_feat(y,x,FS_SECRET);
				}

				/* Hack -- Memorize */
				play_info[y][x] |= (PLAY_MARK);

				/* Redraw */
				lite_spot(y, x);

				/* Obvious */
				detect = TRUE;
			}

		}
	}

	/* Result */
	return (detect);
}


/*
 * Detect all traps on current panel
 */
bool detect_traps(void)
{
	bool detect = FALSE;

	/* Describe */
	if (detect_feat_flags(FF1_TRAP,0x00, 2 * MAX_SIGHT))
	{
		msg_print("You sense the presence of traps!");

		detect = TRUE;
	}

	/* Result */
	return (detect);
}



/*
 * Detect all doors on current panel
 */
bool detect_doors(void)
{

	bool detect = FALSE;


	/* Describe */
	if (detect_feat_flags(FF1_DOOR,0x00, 2 * MAX_SIGHT))
	{
		msg_print("You sense the presence of doors!");

		detect = TRUE;
	}

	/* Result */
	return (detect);
}


/*
 * Detect all stairs on current panel
 */
bool detect_stairs(void)
{

	bool detect = FALSE;

	/* Describe */
	if (detect_feat_flags(FF1_STAIRS,0x00, 2 * MAX_SIGHT))
	{
		msg_print("You sense the presence of stairs!");

		detect = TRUE;
	}

	/* Result */
	return (detect);
}

/*
 * Detect all (running) water on current panel
 */
bool detect_water(void)
{

	bool detect = FALSE;

	/* Describe */
	if (detect_feat_flags(0x00,FF2_WATER, 2 * MAX_SIGHT))
	{
		msg_print("You sense the presence of running water!");

		detect = TRUE;
	}

	/* Result */
	return (detect);
}



/*
 * Detect any treasure on the current panel
 */
bool detect_treasure(void)
{

	bool detect = FALSE;


	/* Describe */
	if (detect_feat_flags(FF1_HAS_GOLD,0x00, 2 * MAX_SIGHT))
	{
		msg_print("You sense the presence of buried treasure!");

		detect = TRUE;

	}

	/* Result */
	return (detect);
}

/*
 * Detect any treasure on the current panel
 */
bool detect_objects_buried(void)
{

	bool detect = FALSE;

	/* Describe */
	if (detect_feat_flags(FF1_HAS_ITEM,0x00, 2 * MAX_SIGHT))
	{
		msg_print("You sense the presence of buried objects!");

		detect = TRUE;

	}

	/* Result */
	return (detect);
}


/*
 * Detect all "gold" objects on the current panel
 */
bool detect_objects_gold(void)
{
	int i, y, x;

	bool detect = FALSE;

	/* Scan objects */
	for (i = 1; i < o_max; i++)
	{
		object_type *o_ptr = &o_list[i];

		/* Skip dead objects */
		if (!o_ptr->k_idx) continue;

		/* Skip held objects */
		if (o_ptr->held_m_idx) continue;

		/* Location */
		y = o_ptr->iy;
		x = o_ptr->ix;

		/* Only detect nearby objects */
		if (distance(p_ptr->py, p_ptr->px, y, x) > 2 * MAX_SIGHT) continue;

		/* Detect "gold" objects */
		if (o_ptr->tval >= TV_GOLD)
		{
			/* Hack -- memorize it */
			o_ptr->ident |= (IDENT_MARKED);

			/* Redraw */
			lite_spot(y, x);

			/* Detect */
			detect = TRUE;
		}
	}

	/* Describe */
	if (detect)
	{
		msg_print("You sense the presence of treasure!");
	}

	/* Result */
	return (detect);
}



/*
 * Detect all "normal" objects on the current panel
 */
bool detect_objects_normal(void)
{
	int i, y, x;

	bool detect = FALSE;

	/* Scan objects */
	for (i = 1; i < o_max; i++)
	{
		object_type *o_ptr = &o_list[i];

		/* Skip dead objects */
		if (!o_ptr->k_idx) continue;

		/* Skip held objects */
		if (o_ptr->held_m_idx) continue;

		/* Location */
		y = o_ptr->iy;
		x = o_ptr->ix;

		/* Only detect nearby objects */
		if (distance(p_ptr->py, p_ptr->px, y, x) > 2 * MAX_SIGHT) continue;

		/* Detect "real" objects */
		if (o_ptr->tval < TV_GOLD)
		{
			/* Hack -- memorize it */
			if (!auto_pickup_ignore(o_ptr)) o_ptr->ident |= (IDENT_MARKED);

			/* Hack -- have seen object */
			if (!(k_info[o_ptr->k_idx].flavor)) k_info[o_ptr->k_idx].aware = TRUE;

			/* XXX XXX - Mark monster objects as "seen" */
			if ((o_ptr->name3 > 0) && !(l_list[o_ptr->name3].sights)) l_list[o_ptr->name3].sights++;

			/* Redraw */
			lite_spot(y, x);

			/* Detect */
			detect = TRUE;
		}
	}

	/* Describe */
	if (detect)
	{
		msg_print("You sense the presence of objects!");
	}

	/* Result */
	return (detect);
}


/*
 * Return a "feeling" (or NULL) about an item.  Method 3 (Magic).
 */
int value_check_aux3(const object_type *o_ptr)
{
	/* If sensed cursed, have no more value to add */
	if ((o_ptr->feeling == INSCRIP_CURSED) || (o_ptr->feeling == INSCRIP_TERRIBLE)
		|| (o_ptr->feeling == INSCRIP_WORTHLESS)) return (0);

	/* Artifacts */
	if (artifact_p(o_ptr))
	{
		/* Cursed/Broken */
		if (cursed_p(o_ptr) || broken_p(o_ptr))
		{
			if ((o_ptr->feeling == INSCRIP_UNBREAKABLE)
				|| (o_ptr->feeling == INSCRIP_ARTIFACT)) return (INSCRIP_TERRIBLE);

			return (INSCRIP_NONMAGICAL);
		}

		/* Normal */
		return (INSCRIP_SPECIAL);
	}

	/* Ego-Items */
	if (ego_item_p(o_ptr))
	{
		/* Cursed/Broken */
		if (cursed_p(o_ptr) || broken_p(o_ptr))	return (INSCRIP_NONMAGICAL);

		/* Superb */
		if (o_ptr->xtra1) return (INSCRIP_SUPERB);

		/* Normal */
		return (INSCRIP_EXCELLENT);
	}

	/* Cursed or broken items */
	if ((cursed_p(o_ptr)) || (broken_p(o_ptr)))
	{
		if (o_ptr->feeling == INSCRIP_POWERFUL) return (INSCRIP_CURSED);

		return (INSCRIP_NONMAGICAL);
	}

	/* Coated item */
	if (coated_p(o_ptr)) return (INSCRIP_COATED);

	/* Magic item */
	if ((o_ptr->xtra1) && (object_power(o_ptr) > 0)) return (INSCRIP_EXCELLENT);

	/* Great "armor" bonus */
	if (o_ptr->to_a > 8) return (INSCRIP_GREAT);

	/* Great "weapon" bonus */
	if (o_ptr->to_h + o_ptr->to_d > 14) return (INSCRIP_GREAT);

	/* Great "weapon" dice */
	if (o_ptr->dd > k_info[o_ptr->k_idx].dd) return (INSCRIP_GREAT);

	/* Great "weapon" sides */
	if (o_ptr->ds > k_info[o_ptr->k_idx].ds) return (INSCRIP_GREAT);

	/* Very good "armor" bonus */
	if (o_ptr->to_a > 4) return (INSCRIP_VERY_GOOD);

	/* Good "weapon" bonus */
	if (o_ptr->to_h + o_ptr->to_d > 7) return (INSCRIP_VERY_GOOD);

	/* Good "armor" bonus */
	if (o_ptr->to_a > 0) return (INSCRIP_GOOD);

	/* Good "weapon" bonus */
	if (o_ptr->to_h + o_ptr->to_d > 0) return (INSCRIP_GOOD);

	/* Have already got nonmagical sensed */
	if (o_ptr->feeling == INSCRIP_UNCURSED) return(INSCRIP_AVERAGE);

	/* Default to nothing */
	return (INSCRIP_NONMAGICAL);
}


/*
 * Return a "feeling" (or NULL) about an item.  Method 4 (Cursed).
 *
 * Note we return INSCRIP_UNCURSED for items that we do not mark
 * this way, but allow such items to be sensed again, elsewhere.
 * Hack -- we 'overload' this semantically with removed curses.
 */
int value_check_aux4(const object_type *o_ptr)
{
	/* If sensed magical, have no more value to add */
	if ((o_ptr->feeling == INSCRIP_GOOD) || (o_ptr->feeling == INSCRIP_VERY_GOOD)
		|| (o_ptr->feeling == INSCRIP_GREAT) || (o_ptr->feeling == INSCRIP_EXCELLENT)
		|| (o_ptr->feeling == INSCRIP_SUPERB) || (o_ptr->feeling == INSCRIP_SPECIAL)
		|| (o_ptr->feeling == INSCRIP_MAGICAL)) return (0);

	/* Artifacts */
	if ((artifact_p(o_ptr)) || (o_ptr->feeling == INSCRIP_ARTIFACT))
	{
		/* Cursed/Broken */
		if (cursed_p(o_ptr) || broken_p(o_ptr)) return (INSCRIP_TERRIBLE);

		/* Known to be artifact strength */
		if ((o_ptr->feeling == INSCRIP_UNBREAKABLE)
			|| (o_ptr->feeling == INSCRIP_ARTIFACT)) return (INSCRIP_SPECIAL);

		/* Normal */
		return (INSCRIP_UNCURSED);
	}

	/* Ego-Items */
	if ((ego_item_p(o_ptr)) || (o_ptr->feeling == INSCRIP_HIGH_EGO_ITEM) || (o_ptr->feeling == INSCRIP_EGO_ITEM))
	{
		/* Cursed/Broken */
		if (cursed_p(o_ptr) || broken_p(o_ptr)) return (INSCRIP_WORTHLESS);

		/* Known to be high ego-item strength */
		if ((o_ptr->feeling == INSCRIP_HIGH_EGO_ITEM)) return (INSCRIP_SUPERB);

		/* Known to be high ego-item strength */
		if ((o_ptr->feeling == INSCRIP_EGO_ITEM)) return (INSCRIP_EXCELLENT);

		/* Normal */
		return (INSCRIP_UNCURSED);
	}

	/* Cursed items */
	if (cursed_p(o_ptr)) return (INSCRIP_CURSED);

	/* Broken items */
	/* if (broken_p(o_ptr)) return (INSCRIP_BROKEN); */

	/* Known to be powerful */
	if ((o_ptr->feeling == INSCRIP_POWERFUL)) return (INSCRIP_MAGICAL);

	/* Default to uncursed */
	return (INSCRIP_UNCURSED);
}


/*
 * Return a "feeling" (or NULL) about an item.  Method 5 (Power).
 */
int value_check_aux5(const object_type *o_ptr)
{
	/* Artifacts */
	if (artifact_p(o_ptr))
	{
		if (o_ptr->feeling == INSCRIP_UNCURSED) return (INSCRIP_SPECIAL);
		else if (o_ptr->feeling == INSCRIP_NONMAGICAL) return (INSCRIP_TERRIBLE);

		/* Normal */
		return (INSCRIP_ARTIFACT);
	}

	/* Ego-Items */
	if (ego_item_p(o_ptr))
	{
		if (o_ptr->feeling == INSCRIP_NONMAGICAL) return (INSCRIP_WORTHLESS);
		else if ((o_ptr->feeling == INSCRIP_UNCURSED) && (o_ptr->xtra1)) return (INSCRIP_EXCELLENT);
		else if (o_ptr->feeling == INSCRIP_UNCURSED) return (INSCRIP_EXCELLENT);

		/* Superb */
		if (o_ptr->xtra1) return (INSCRIP_HIGH_EGO_ITEM);

		/* Normal */
		return (INSCRIP_EGO_ITEM);
	}

	/* Cursed items */
	if (cursed_p(o_ptr))
	{
		if (o_ptr->feeling == INSCRIP_NONMAGICAL) return (INSCRIP_CURSED);

		return (INSCRIP_POWERFUL);
	}

	/* Broken items */
	/*if (broken_p(o_ptr)) return (INSCRIP_NONMAGICAL);*/

	/* Coated item */
	if (coated_p(o_ptr)) return (INSCRIP_COATED);

	/* Magical item */
	if ((o_ptr->xtra1) && (object_power(o_ptr) > 0)) return (INSCRIP_EGO_ITEM);

	/* Good "armor" bonus */
	if (o_ptr->to_a > 0)
	{
		return (INSCRIP_POWERFUL);
	}

	/* Good "weapon" bonus */
	if (o_ptr->to_h + o_ptr->to_d > 0)
	{
		return (INSCRIP_POWERFUL);
	}

	/* Default to nothing */
	return (INSCRIP_AVERAGE);
}


/*
 * Detect all "magic" objects on the current panel.
 *
 * This will light up all spaces with "magic" items, including artifacts,
 * ego-items, potions, scrolls, books, rods, wands, staves, amulets, rings,
 * and "enchanted" items of the "good" variety.
 *
 * It can probably be argued that this function is now too powerful.
 *
 * Now also senses all magical objects in the inventory.
 */
bool detect_objects_magic(void)
{
	int i, y, x, tv;

	bool detect = FALSE;

	/* Scan all objects */
	for (i = 1; i < o_max; i++)
	{
		int feel = 0;

		object_type *o_ptr = &o_list[i];

		/* Skip dead objects */
		if (!o_ptr->k_idx) continue;

		/* Skip held objects */
		if (o_ptr->held_m_idx) continue;

		/* Location */
		y = o_ptr->iy;
		x = o_ptr->ix;

		/* Only detect nearby objects */
		if (distance(p_ptr->py, p_ptr->px, y, x) > 2 * MAX_SIGHT) continue;

		/* Examine the tval */
		tv = o_ptr->tval;

		/* Artifacts, misc magic items, or enchanted wearables */
		if (!(cursed_p(o_ptr)) && !(broken_p(o_ptr)) && (artifact_p(o_ptr) || ego_item_p(o_ptr) ||
		    (o_ptr->xtra1 < OBJECT_XTRA_MIN_COATS) ||
		    (tv == TV_AMULET) || (tv == TV_RING) ||
		    (tv == TV_STAFF) || (tv == TV_WAND) || (tv == TV_ROD) ||
		    (tv == TV_SCROLL) || (tv == TV_POTION) ||
		    (tv == TV_MAGIC_BOOK) || (tv == TV_PRAYER_BOOK) ||
		    (o_ptr->to_a > 0) || (o_ptr->to_h + o_ptr->to_d > 0)))
		{
			/* Memorize the item */
			if (!auto_pickup_ignore(o_ptr)) o_ptr->ident |= (IDENT_MARKED);

			/* Hack -- have seen object */
			if (!(k_info[o_ptr->k_idx].flavor)) k_info[o_ptr->k_idx].aware = TRUE;

			/* XXX XXX - Mark monster objects as "seen" */
			if ((o_ptr->name3 > 0) && !(l_list[o_ptr->name3].sights)) l_list[o_ptr->name3].sights++;

			/* Redraw */
			lite_spot(y, x);

			/* Detect */
			detect = TRUE;
		}

		/* Get the inscription */
		feel = sense_magic(o_ptr, 3, TRUE);

		/* Sense something */
		if (!feel) continue;

		/* Sense the object */
		o_ptr->feeling = feel;

		/* The object has been "sensed" */
		o_ptr->ident |= (IDENT_SENSE);
	}

	/* Sense inventory */
	for (i = 0; i < INVEN_TOTAL+1; i++)
	{
		int feel = 0;

		object_type *o_ptr = &inventory[i];

		/* Get the inscription */
		feel = sense_magic(o_ptr, 3, TRUE);

		/* Sense something */
		if (!feel) continue;

		/* Any different */
		if (o_ptr->feeling == feel) continue;

		/* Detect */
		if (feel != INSCRIP_NONMAGICAL) detect = TRUE;

		/* Sense the object */
		o_ptr->feeling = feel;

		/* The object has been "sensed" */
		o_ptr->ident |= (IDENT_SENSE);

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);
	}

	/* Describe */
	if (detect)
	{
		msg_print("You sense the presence of magic objects!");
	}

	/* Return result */
	return (detect);
}



/*
 * Detect all "cursed" objects on the current panel.
 *
 * This will light up all spaces with "cursed" items, including artifacts,
 * ego-items, and "enchanted" items of the cursed or broken variety.
 *
 * Should probably marked objects detected as cursed when appropriate.
 *
 * Now also senses all cursed objects in the inventory.
 */
bool detect_objects_cursed(void)
{
	int i, y, x;

	bool detect = FALSE;

	/* Scan all objects */
	for (i = 1; i < o_max; i++)
	{
		int feel;

		object_type *o_ptr = &o_list[i];

		/* Skip dead objects */
		if (!o_ptr->k_idx) continue;

		/* Skip held objects */
		if (o_ptr->held_m_idx) continue;

		/* Location */
		y = o_ptr->iy;
		x = o_ptr->ix;

		/* Only detect nearby objects */
		if (distance(p_ptr->py, p_ptr->px, y, x) > 2 * MAX_SIGHT) continue;

		/* Cursed items */
		if (cursed_p(o_ptr) || broken_p(o_ptr))
		{
			/* Memorize the item */
			if (!auto_pickup_ignore(o_ptr)) o_ptr->ident |= (IDENT_MARKED);

			/* Hack -- have seen object */
			if (!(k_info[o_ptr->k_idx].flavor)) k_info[o_ptr->k_idx].aware = TRUE;

			/* XXX XXX - Mark monster objects as "seen" */
			if ((o_ptr->name3 > 0) && !(l_list[o_ptr->name3].sights)) l_list[o_ptr->name3].sights++;

			/* Redraw */
			lite_spot(y, x);

			/* Detect */
			detect = TRUE;
		}

		/* Get the inscription */
		feel = sense_magic(o_ptr, 4, TRUE);

		/* Sense something */
		if (!feel) continue;

		/* Sense the object */
		o_ptr->feeling = feel;

		/* The object has been "sensed" */
		o_ptr->ident |= (IDENT_SENSE);
	}

	/* Sense inventory */
	for (i = 0; i < INVEN_TOTAL+1; i++)
	{
		int feel;

		object_type *o_ptr = &inventory[i];

		/* Get the inscription */
		feel = sense_magic(o_ptr, 4, TRUE);

		/* Sense something? */
		if (!feel) continue;

		/* Any different */
		if (o_ptr->feeling == feel) continue;

		/* Detect */
		if (feel != INSCRIP_UNCURSED) detect = TRUE;

		/* Sense the object */
		o_ptr->feeling = feel;

		/* The object has been "sensed" */
		o_ptr->ident |= (IDENT_SENSE);

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);
	}

	/* Describe */
	if (detect)
	{
		msg_print("You sense the presence of cursed objects!");
	}

	/* Return result */
	return (detect);
}



/*
 * Detect all "powerful" objects on the current panel.
 *
 * This will light up all spaces with "magic" items, including artifacts,
 * ego-items, potions, scrolls, books, rods, wands, staves, amulets, rings,
 * and "enchanted" items of any time.
 *
 * Now also senses all powerful objects in the inventory.
 */
bool detect_objects_power(void)
{
	int i, y, x, tv;

	bool detect = FALSE;

	/* Scan all objects */
	for (i = 1; i < o_max; i++)
	{
		int feel = 0;

		object_type *o_ptr = &o_list[i];

		/* Skip dead objects */
		if (!o_ptr->k_idx) continue;

		/* Skip held objects */
		if (o_ptr->held_m_idx) continue;

		/* Location */
		y = o_ptr->iy;
		x = o_ptr->ix;

		/* Only detect nearby objects */
		if (distance(p_ptr->py, p_ptr->px, y, x) > 2 * MAX_SIGHT) continue;

		/* Examine the tval */
		tv = o_ptr->tval;

		/* Artifacts, misc magic items, or enchanted wearables */
		if (artifact_p(o_ptr) || ego_item_p(o_ptr) ||
		    (o_ptr->xtra1 < OBJECT_XTRA_MIN_COATS) ||
		    (tv == TV_AMULET) || (tv == TV_RING) ||
		    (tv == TV_STAFF) || (tv == TV_WAND) || (tv == TV_ROD) ||
		    (tv == TV_SCROLL) || (tv == TV_POTION) ||
		    (tv == TV_MAGIC_BOOK) || (tv == TV_PRAYER_BOOK) ||
		    (o_ptr->to_a > 0) || (o_ptr->to_h + o_ptr->to_d > 0))
		{
			/* Memorize the item */
			if (!auto_pickup_ignore(o_ptr)) o_ptr->ident |= (IDENT_MARKED);

			/* Hack -- have seen object */
			if (!(k_info[o_ptr->k_idx].flavor)) k_info[o_ptr->k_idx].aware = TRUE;

			/* XXX XXX - Mark monster objects as "seen" */
			if ((o_ptr->name3 > 0) && !(l_list[o_ptr->name3].sights)) l_list[o_ptr->name3].sights++;

			/* Redraw */
			lite_spot(y, x);

			/* Detect */
			detect = TRUE;
		}

		/* Get the inscription */
		feel = sense_magic(o_ptr, 5, TRUE);

		/* Sense something */
		if (!feel) continue;

		/* Sense the object */
		o_ptr->feeling = feel;

		/* The object has been "sensed" */
		o_ptr->ident |= (IDENT_SENSE);
	}

	/* Sense inventory */
	for (i = 0; i < INVEN_TOTAL+1; i++)
	{
		int feel = 0;

		object_type *o_ptr = &inventory[i];

		/* Get the inscription */
		feel = sense_magic(o_ptr, 5, TRUE);

		/* Sense something */
		if (!feel) continue;

		/* Any different */
		if (o_ptr->feeling == feel) continue;

		/* Detect */
		if (feel != INSCRIP_AVERAGE) detect = TRUE;

		/* Sense the object */
		o_ptr->feeling = feel;

		/* The object has been "sensed" */
		o_ptr->ident |= (IDENT_SENSE);

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);
	}

	/* Describe */
	if (detect)
	{
		msg_print("You sense the presence of powerful objects!");
	}

	/* Return result */
	return (detect);
}



/*
 * Detect all "normal" monsters on the current panel
 */
bool detect_monsters_normal(void)
{
	int i, y, x;

	bool flag = FALSE;


	/* Scan monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Location */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Only detect nearby monsters */
		if (distance(p_ptr->py, p_ptr->px, y, x) > 2 * MAX_SIGHT) continue;

		/* Detect all non-invisible monsters */
		if (!(r_ptr->flags2 & (RF2_INVISIBLE)) && !(m_ptr->tim_invis))
		{
			/* Optimize -- Repair flags */
			repair_mflag_mark = repair_mflag_show = TRUE;

			/* Hack -- Detect the monster */
			m_ptr->mflag |= (MFLAG_MARK | MFLAG_SHOW);

			/* Update the monster */
			update_mon(i, FALSE);

			/* Detect */
			flag = TRUE;
		}
	}

	/* Hack -- mark as safe */
	if (view_detect_grids) detect_feat_flags(0L, 0L, 2 * MAX_SIGHT);

	/* Describe */
	if (flag)
	{
		/* Describe result */
		msg_print("You sense the presence of monsters!");
	}

	/* Result */
	return (flag);
}


/*
 * Detect all "evil" monsters on current panel
 */
bool detect_monsters_evil(void)
{
	int i, y, x;

	bool flag = FALSE;


	/* Scan monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];
		monster_lore *l_ptr = &l_list[m_ptr->r_idx];

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Location */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Only detect nearby monsters */
		if (distance(p_ptr->py, p_ptr->px, y, x) > 2 * MAX_SIGHT) continue;

		/* Detect evil monsters */
		if (r_ptr->flags3 & (RF3_EVIL))
		{
			/* Take note that they are evil */
			l_ptr->flags3 |= (RF3_EVIL);

			/* Update monster recall window */
			if (p_ptr->monster_race_idx == m_ptr->r_idx)
			{
				/* Window stuff */
				p_ptr->window |= (PW_MONSTER);
			}

			/* Optimize -- Repair flags */
			repair_mflag_mark = repair_mflag_show = TRUE;

			/* Detect the monster */
			m_ptr->mflag |= (MFLAG_MARK | MFLAG_SHOW);

			/* Update the monster */
			update_mon(i, FALSE);

			/* Detect */
			flag = TRUE;
		}
	}

	/* Hack -- mark as safe */
	if (view_detect_grids) detect_feat_flags(0L, 0L, 2 * MAX_SIGHT);

	/* Describe */
	if (flag)
	{
		/* Describe result */
		msg_print("You sense the presence of evil creatures!");
	}

	/* Result */
	return (flag);
}


/*
 * Detect all "living" monsters on current panel
 */
bool detect_monsters_living(void)
{
	int i, y, x;

	bool flag = FALSE;


	/* Scan monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Location */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Only detect nearby monsters */
		if (distance(p_ptr->py, p_ptr->px, y, x) > 2 * MAX_SIGHT) continue;

		/* Detect evil monsters */
		if (!(r_ptr->flags3 & (RF3_NONLIVING)))
		{
			/* Update monster recall window */
			if (p_ptr->monster_race_idx == m_ptr->r_idx)
			{
				/* Window stuff */
				p_ptr->window |= (PW_MONSTER);
			}

			/* Optimize -- Repair flags */
			repair_mflag_mark = repair_mflag_show = TRUE;

			/* Detect the monster */
			m_ptr->mflag |= (MFLAG_MARK | MFLAG_SHOW);

			/* Update the monster */
			update_mon(i, FALSE);

			/* Detect */
			flag = TRUE;
		}
	}

	/* Hack -- mark as safe */
	if (view_detect_grids) detect_feat_flags(0L, 0L, 2 * MAX_SIGHT);

	/* Describe */
	if (flag)
	{
		/* Describe result */
		msg_print("You sense the presence of evil creatures!");
	}

	/* Result */
	return (flag);
}


/*
 * Detect everything
 */
bool detect_all(void)
{
	bool detect = FALSE;

	/* Detect everything */
	if (detect_doors()) detect = TRUE;
	if (detect_traps()) detect = TRUE;
	if (detect_stairs()) detect = TRUE;
	if (detect_treasure()) detect = TRUE;
	if (detect_objects_buried()) detect = TRUE;
	if (detect_objects_gold()) detect = TRUE;
	if (detect_objects_normal()) detect = TRUE;
	if (detect_monsters_evil()) detect = TRUE;
	if (detect_monsters_living()) detect = TRUE;
	if (detect_monsters_normal()) detect = TRUE;

	/* Result */
	return (detect);
}



/*
 * Create stairs at the player location
 */
void stair_creation(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	/* XXX XXX XXX */
	if (!cave_valid_bold(py, px))
	{
		msg_print("The object resists the spell.");
		return;
	}

	/* XXX XXX XXX */
	delete_object(py, px);

	/* Create a staircase */
	if (p_ptr->depth == min_depth(p_ptr->dungeon))
	{
		cave_set_feat(py, px, FEAT_MORE);
	}
	else if (is_quest(p_ptr->depth) || (p_ptr->depth >= max_depth(p_ptr->dungeon)))
	{
		cave_set_feat(py, px, FEAT_LESS);
	}
	else if (rand_int(100) < 50)
	{
		cave_set_feat(py, px, FEAT_MORE);
	}
	else
	{
		cave_set_feat(py, px, FEAT_LESS);
	}
}


/*
 * Hook to specify "weapon"
 */
static bool item_tester_hook_weapon(const object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_DIGGING:
		case TV_BOW:
		case TV_BOLT:
		case TV_ARROW:
		case TV_SHOT:
		case TV_STAFF:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}

/*
 * Hook to specify a strict "weapon"
 */
static bool item_tester_hook_weapon_strict(const object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Hook to specify "armour"
 */
static bool item_tester_hook_armour(const object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_DRAG_ARMOR:
		case TV_HARD_ARMOR:
		case TV_SOFT_ARMOR:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_CROWN:
		case TV_HELM:
		case TV_BOOTS:
		case TV_GLOVES:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}

/*
 * Hook to specify "bolts"
 */
static bool item_tester_hook_ammo(const object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_ARROW:
		case TV_BOLT:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}



static bool item_tester_unknown(const object_type *o_ptr)
{
	if (object_known_p(o_ptr))
		return FALSE;
	else
		return TRUE;
}

static int unknown_tval;

static bool item_tester_unknown_tval(const object_type *o_ptr)
{
	if(o_ptr->tval != unknown_tval) return FALSE;

	if (object_known_p(o_ptr))
		return FALSE;
	else
		return TRUE;
}


static bool item_tester_unknown_bonus(const object_type *o_ptr)
{
	if (object_known_p(o_ptr))
		return FALSE;
	else
	{
		switch (o_ptr->tval)
		{
			case TV_SHOT:
			case TV_ARROW:
			case TV_BOLT:
			case TV_BOW:
			case TV_DIGGING:
			case TV_HAFTED:
			case TV_POLEARM:
			case TV_SWORD:
			case TV_BOOTS:
			case TV_GLOVES:
			case TV_HELM:
			case TV_CROWN:
			case TV_SHIELD:
			case TV_CLOAK:
			case TV_SOFT_ARMOR:
			case TV_HARD_ARMOR:
			case TV_DRAG_ARMOR:
			case TV_RING:
			case TV_AMULET:
			{
				if (!(o_ptr->ident & (IDENT_BONUS))) return TRUE;
				if (!(o_ptr->ident & (IDENT_PVAL))) return TRUE;
				break;
			}
			case TV_STAFF:
			{
				if (!(o_ptr->ident & (IDENT_BONUS))) return TRUE;
			}
			case TV_WAND:
			{
				if (!(o_ptr->ident & (IDENT_CHARGES))) return TRUE;
				break;
			}
		}
	}

	return FALSE;
}


static bool item_tester_unknown_star(const object_type *o_ptr)
{
	if (o_ptr->ident & IDENT_MENTAL)
		return FALSE;
	else
		return TRUE;
}


static bool item_tester_unknown_sense(const object_type *o_ptr)
{
	if (object_known_p(o_ptr))
		return FALSE;
	else if (o_ptr->ident & IDENT_SENSE)
		return FALSE;
	else
		return TRUE;
}


static bool item_tester_unknown_value(const object_type *o_ptr)
{
	if (o_ptr->ident & IDENT_VALUE)
		return FALSE;
	else
		return TRUE;
}


static bool item_tester_unknown_runes(const object_type *o_ptr)
{
	if (o_ptr->xtra1 >= OBJECT_XTRA_MIN_RUNES)
		return FALSE;
	else if (o_ptr->ident & IDENT_RUNES)
		return FALSE;
	else
		return TRUE;
}


/*
 * Note for the following function, the player must know it is an artifact
 * or ego item, either by sensing or identifying it.
 */
static bool item_tester_known_rumor(const object_type *o_ptr)
{
	if (o_ptr->ident & IDENT_MENTAL)
		return FALSE;
	else if ((!object_known_p(o_ptr))
		&& !(o_ptr->feeling == INSCRIP_SPECIAL)
		&& !(o_ptr->feeling == INSCRIP_EXCELLENT)
		&& !(o_ptr->feeling == INSCRIP_SUPERB)
		&& !(o_ptr->feeling == INSCRIP_WORTHLESS)
		&& !(o_ptr->feeling == INSCRIP_TERRIBLE))
		return FALSE;
	else if (!(o_ptr->name1) && !(o_ptr->name2))
		return FALSE;
	else
		return TRUE;
}



/*
 * Enchant an item
 *
 * Revamped!  Now takes item pointer, number of times to try enchanting,
 * and a flag of what to try enchanting.  Artifacts resist enchantment
 * some of the time, and successful enchantment to at least +0 might
 * break a curse on the item.  -CFT
 *
 * Note that an item can technically be enchanted all the way to +15 if
 * you wait a very, very, long time.  Going from +9 to +10 only works
 * about 5% of the time, and from +10 to +11 only about 1% of the time.
 *
 * Note that this function can now be used on "piles" of items, and
 * the larger the pile, the lower the chance of success.
 */
bool enchant(object_type *o_ptr, int n, int eflag)
{
	int i, chance, prob;

	bool res = FALSE;

	bool a = artifact_p(o_ptr);

	u32b f1, f2, f3, f4;

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4);

	/* Large piles resist enchantment */
	prob = o_ptr->number * 100;

	/* Missiles are easy to enchant */
	if ((o_ptr->tval == TV_BOLT) ||
	    (o_ptr->tval == TV_ARROW) ||
	    (o_ptr->tval == TV_SHOT))
	{
		prob = prob / 20;
	}

	/* Try "n" times */
	for (i=0; i<n; i++)
	{
		/* Hack -- Roll for pile resistance */
		if ((prob > 100) && (rand_int(prob) >= 100)) continue;

		/* Enchant to hit */
		if (eflag & (ENCH_TOHIT))
		{
			if (o_ptr->to_h < 0) chance = 0;
			else if (o_ptr->to_h > 15) chance = 1000;
			else chance = enchant_table[o_ptr->to_h];

			/* Attempt to enchant */
			if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
			{
				res = TRUE;

				/* Enchant */
				o_ptr->to_h++;

				/* Break curse */
				if (cursed_p(o_ptr) &&
				    (!(f3 & (TR3_PERMA_CURSE))) &&
				    (o_ptr->to_h >= 0) && (rand_int(100) < 25))
				{
					msg_print("The curse is broken!");

					/* Uncurse the object */
					uncurse_object(o_ptr);
				}
			}
		}

		/* Enchant to damage */
		if (eflag & (ENCH_TODAM))
		{
			if (o_ptr->to_d < 0) chance = 0;
			else if (o_ptr->to_d > 15) chance = 1000;
			else chance = enchant_table[o_ptr->to_d];

			/* Attempt to enchant */
			if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
			{
				res = TRUE;

				/* Enchant */
				o_ptr->to_d++;

				/* Break curse */
				if (cursed_p(o_ptr) &&
				    (!(f3 & (TR3_PERMA_CURSE))) &&
				    (o_ptr->to_d >= 0) && (rand_int(100) < 25))
				{
					msg_print("The curse is broken!");

					/* Uncurse the object */
					uncurse_object(o_ptr);
				}
			}
		}

		/* Enchant to armor class */
		if (eflag & (ENCH_TOAC))
		{
			if (o_ptr->to_a < 0) chance = 0;
			else if (o_ptr->to_a > 15) chance = 1000;
			else chance = enchant_table[o_ptr->to_a];

			/* Attempt to enchant */
			if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
			{
				res = TRUE;

				/* Enchant */
				o_ptr->to_a++;

				/* Break curse */
				if (cursed_p(o_ptr) &&
				    (!(f3 & (TR3_PERMA_CURSE))) &&
				    (o_ptr->to_a >= 0) && (rand_int(100) < 25))
				{
					msg_print("The curse is broken!");

					/* Uncurse the object */
					uncurse_object(o_ptr);
				}
			}
		}
	}

	/* Failure */
	if (!res) return (FALSE);

	/* Hack --- unsense the item */
	o_ptr->ident &= ~(IDENT_SENSE);	

	/* Remove special inscription, if any */
	o_ptr->feeling = 0;

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);

	/* Success */
	return (TRUE);
}



/*
 * Enchant an item (in the inventory or on the floor)
 * Note that "num_ac" requires armour, else weapon
 * Returns TRUE if attempted, FALSE if cancelled
 */
bool enchant_spell(int num_hit, int num_dam, int num_ac)
{
	int item;
	bool okay = FALSE;

	object_type *o_ptr;

	char o_name[80];

	cptr q, s;

	/* Assume enchant weapon */
	item_tester_hook = item_tester_hook_weapon;

	/* Enchant armor if requested */
	if (num_ac) item_tester_hook = item_tester_hook_armour;

	/* Get an item */
	q = "Enchant which item? ";
	s = "You have nothing to enchant.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Description */
	object_desc(o_name, sizeof(o_name), o_ptr, FALSE, 0);

	/* Describe */
	msg_format("%s %s glow%s brightly!",
		   ((item >= 0) ? "Your" : "The"), o_name,
		   ((o_ptr->number > 1) ? "" : "s"));

	/* Enchant */
	if (enchant(o_ptr, num_hit, ENCH_TOHIT)) okay = TRUE;
	if (enchant(o_ptr, num_dam, ENCH_TODAM)) okay = TRUE;
	if (enchant(o_ptr, num_ac, ENCH_TOAC)) okay = TRUE;

	/* Failure */
	if (!okay)
	{
		/* Flush */
		if (flush_failure) flush();

		/* Message */
		msg_print("The enchantment failed.");
	}
	else
	{

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);
	
		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	}

	/* Something happened */
	return (TRUE);
}

/*
 * Brand any item
 */
bool brand_item(int brand, cptr act)
{
	int item;

	object_type *o_ptr;

	char o_name[80];

	cptr q, s;

	bool brand_ammo = FALSE;

	/* Get an item */
	q = "Enchant which item? ";
	s = "You have nothing to enchant.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Hack -- check ammo */
	switch (o_ptr->tval)
	{
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
			brand_ammo = TRUE;
		break;
	}

	/* Description */
	object_desc(o_name, sizeof(o_name), o_ptr, FALSE, 0);

	/* Describe */
	msg_format("%s %s %s!",
		   ((item >= 0) ? "Your" : "The"), o_name, act);

	/* you can never modify artifacts / ego-items with hidden powers*/
	if ((!artifact_p(o_ptr)) && !(o_ptr->xtra1))
	{
		bool split = FALSE;

		object_type *i_ptr;
		object_type object_type_body;

		/* Hack -- split stack only if required. This is dangerous otherwise as we may
		   be calling from a routine where we delete items later. XXX XXX */
		/* Mega-hack -- we allow 5 arrows/shots/bolts to be enchanted per application */
		if ((o_ptr->number > 1) && ((!brand_ammo) || (o_ptr->number > 5)))
		{
			int qty = (brand_ammo) ? 5 : 1;
			split = TRUE;

			/* Get local object */
			i_ptr = &object_type_body;

			/* Obtain a local object */
			object_copy(i_ptr, o_ptr);

			/* Modify quantity */
			i_ptr->number = qty;

			/* Reset stack counter */
			i_ptr->stackc = 0;

			/* Decrease the item (in the pack) */
			if (item >= 0)
			{
				/* Forget about item */
				if (o_ptr->number == qty) inven_drop_flags(o_ptr);

				inven_item_increase(item, -qty);
				inven_item_describe(item);
				inven_item_optimize(item);
			}
			/* Decrease the item (from the floor) */
			else
			{
				floor_item_increase(0 - item, -qty);
				floor_item_describe(0 - item);
				floor_item_optimize(0 - item);
			}

			/* Hack -- use new temporary item */
			o_ptr = i_ptr;
		}

		o_ptr->xtra1 = brand;
		o_ptr->xtra2 = (byte)rand_int(object_xtra_size[brand]);

		if (object_xtra_what[brand] == 1)
		{
    			object_can_flags(o_ptr,object_xtra_base[brand] << o_ptr->xtra2,0x0L,0x0L,0x0L);
		}
		else if (object_xtra_what[brand] == 2)
		{
	    		object_can_flags(o_ptr,0x0L,object_xtra_base[brand] << o_ptr->xtra2,0x0L,0x0L);
		}
		else if (object_xtra_what[brand] == 3)
		{
    			object_can_flags(o_ptr,0x0L,0x0L,object_xtra_base[brand] << o_ptr->xtra2,0x0L);
		}
		else if (object_xtra_what[brand] == 4)
		{
    			object_can_flags(o_ptr,0x0L,0x0L,0x0L,object_xtra_base[brand] << o_ptr->xtra2);
		}

		/* Remove special inscription, if any */
		o_ptr->feeling = INSCRIP_MIN_HIDDEN + brand - 1;

		/* The object has been "sensed" */
		if (!object_known_p(o_ptr)) o_ptr->ident |= (IDENT_SENSE);

		/* Carry item again if split */
		if (split)
		{
			/* Carry the item */
			if (inven_carry_okay(o_ptr)) inven_carry(o_ptr);
			else drop_near(o_ptr,0,p_ptr->py,p_ptr->px);
		}
	
		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);
	
		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	}
	else
	{
		if (flush_failure) flush();
		msg_print("The branding failed.");

		/* Sense it? */
	}

	/* Something happened */
	return (TRUE);
}



/*
 * Identify an object in the inventory (or on the floor)
 * This routine does *not* automatically combine objects.
 * Returns TRUE if something was identified, else FALSE.
 */
bool ident_spell(void)
{
	int item;

	object_type *o_ptr;

	char o_name[80];

	cptr q, s;


	/* Only un-id'ed items */
	item_tester_hook = item_tester_unknown;

	/* Get an item */
	q = "Identify which item? ";
	s = "You have nothing to identify.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Identify it fully */
	object_aware(o_ptr);
	object_known(o_ptr);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);

	/* Description */
	object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

	/* Describe */
	if (item >= INVEN_WIELD)
	{
		msg_format("%^s: %s (%c).",
			   describe_use(item), o_name, index_to_label(item));
	}
	else if (item >= 0)
	{
		msg_format("In your pack: %s (%c).",
			   o_name, index_to_label(item));
	}
	else
	{
		msg_format("On the ground: %s.",
			   o_name);
	}
	/* Something happened */
	return (TRUE);
}

/*
 * Identify the bonus/charges of an object in the inventory (or on the floor)
 * This routine does *not* automatically combine objects.
 * Returns TRUE if something was identified, else FALSE.
 */
bool ident_spell_bonus(void)
{
	int item;

	object_type *o_ptr;

	char o_name[80];

	cptr q, s;


	/* Only un-id'ed items */
	item_tester_hook = item_tester_unknown_bonus;

	/* Get an item */
	q = "Gauge which item? ";
	s = "You have nothing to gauge.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Identify it's bonuses */
	object_bonus(o_ptr);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);

	/* Description */
	object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

	/* Describe */
	if (item >= INVEN_WIELD)
	{
		msg_format("%^s: %s (%c).",
			   describe_use(item), o_name, index_to_label(item));
	}
	else if (item >= 0)
	{
		msg_format("In your pack: %s (%c).",
			   o_name, index_to_label(item));
	}
	else
	{
		msg_format("On the ground: %s.",
			   o_name);
	}

	/* Something happened */
	return (TRUE);
}


/*
 * Heavily senses an item in the inventory or on the floor.
 * Returns TRUE if something was sensed, else FALSE.
 */
bool ident_spell_sense(void)
{
	int item;

	object_type *o_ptr;

	char o_name[80];

	cptr q, s;


	/* Only un-id'ed items */
	item_tester_hook = item_tester_unknown_sense;

	/* Get an item */
	q = "Sense which item? ";
	s = "You have nothing to sense.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Identify it's bonuses */
	sense_magic(o_ptr, cp_ptr->sense_type, TRUE);

	/* Description */
	object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

	/* Describe */
	if (item >= INVEN_WIELD)
	{
		msg_format("%^s: %s (%c).",
			   describe_use(item), o_name, index_to_label(item));
	}
	else if (item >= 0)
	{
		msg_format("In your pack: %s (%c).",
			   o_name, index_to_label(item));
	}
	else
	{
		msg_format("On the ground: %s.",
			   o_name);
	}

	/* Something happened */
	return (TRUE);
}




/*
 * Identify the value of an object in the inventory (or on the floor)
 * Returns TRUE if something was identified, else FALSE.
 */
bool ident_spell_value(void)
{
	int item;

	object_type *o_ptr;

	char o_name[80];

	cptr q, s;


	/* Only un-id'ed items */
	item_tester_hook = item_tester_unknown_value;

	/* Get an item */
	q = "Value which item? ";
	s = "You have nothing to value.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Value the item */
	o_ptr->ident |= (IDENT_VALUE);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Description */
	object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

	/* Describe */
	if (item >= INVEN_WIELD)
	{
		msg_format("%^s: %s (%c).",
			   describe_use(item), o_name, index_to_label(item));
	}
	else if (item >= 0)
	{
		msg_format("In your pack: %s (%c).",
			   o_name, index_to_label(item));
	}
	else
	{
		msg_format("On the ground: %s.",
			   o_name);
	}

	/* Something happened */
	return (TRUE);
}


/*
 * Determine the runes on an object in the inventory (or on the floor)
 * Returns TRUE if something was identified, else FALSE.
 */
bool ident_spell_runes(void)
{
	int item;

	object_type *o_ptr;

	char o_name[80];

	cptr q, s;


	/* Only un-id'ed items */
	item_tester_hook = item_tester_unknown_runes;

	/* Get an item */
	q = "Read runes on which item? ";
	s = "You have nothing to read runes on.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Identify it's bonuses */
	o_ptr->ident |= (IDENT_RUNES);

	/* Description */
	object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

	/* Describe */
	if (item >= INVEN_WIELD)
	{
		msg_format("%^s: %s (%c).",
			   describe_use(item), o_name, index_to_label(item));
	}
	else if (item >= 0)
	{
		msg_format("In your pack: %s (%c).",
			   o_name, index_to_label(item));
	}
	else
	{
		msg_format("On the ground: %s.",
			   o_name);
	}

	/* Something happened */
	return (TRUE);
}


/*
 * Reveal some powers of a known artifact or ego item.
 * Returns TRUE if something was attempted, else FALSE.
 */
bool ident_spell_rumor(void)
{
	int item;

	object_type *o_ptr;

	cptr p, q, r, s;

	int i;

	bool done;

	u32b f1,f2,f3,f4;

	/* Only un-id'ed items */
	item_tester_hook = item_tester_known_rumor;

	/* Get an item */
	q = "Learn legends about which item? ";
	s = "You have nothing legendary to examine.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Pick an interesting phrase */
	switch (randint(6))
	{
		case 1:
			p="Hmm... these runes look interesting..";
			r=". but they tell you nothing more.";
			break;
		case 2:
			p="You recall tales fitting an item of this description";
			r=", and they are bawdy and dull.";
			break;
		case 3:
			p="Ancient visions fill your mind..";
			r=". and give you a headache.";
			break;
		case 4:
			p="The maker's mark is strangely familiar";
			r="... oh, it's just a smudge of dirt.";
			break;
		case 5:
			p="The item glows with faint enchantments";
			r=", snaps, crackles and pops.";
			break;
		default:
			p="Ah... the memories..";
			r=". things were always worse than you remember them.";
			break;
	}

	/* Examine the item */
	object_flags(o_ptr, &f1, &f2, &f3, &f4);

	/* Remove known flags */
	f1 &= ~(o_ptr->can_flags1);
	f2 &= ~(o_ptr->can_flags2);
	f3 &= ~(o_ptr->can_flags3); 
	f4 &= ~(o_ptr->can_flags4); 

	/* We know everything? */
	done = ((f1 | f2 | f3 | f4) ? FALSE : TRUE);

	/* Clear some flags1 */
	if (f1)	for (i = 0;i<32;i++)
	{
		if ((f1 & (1L<<i)) && (rand_int(100)<25)) f1 &= ~(1L<<i);
	}

	/* Clear some flags2 */
	if (f2)	for (i = 0;i<32;i++)
	{
		if ((f2 & (1L<<i)) && (rand_int(100)<25)) f2 &= ~(1L<<i);
	}

	/* Clear some flags3 */
	if (f3)	for (i = 0;i<32;i++)
	{
		if ((f3 & (1L<<i)) && (rand_int(100)<25)) f3 &= ~(1L<<i);
	}

	/* Clear some flags3 */
	if (f4)	for (i = 0;i<32;i++)
	{
		if ((f4 & (1L<<i)) && (rand_int(100)<25)) f4 &= ~(1L<<i);
	}

	if (done || (f1 | f2 | f3 | f4))
	{
		char o_name[80];

		/* Tell the player the good news */
		msg_format("%s.",p);

		if (done)
		{
			/* Do we know absolutely everything? */
			if (object_known_p(o_ptr)) object_mental(o_ptr);
			else
			{
				object_known(o_ptr);
				object_aware(o_ptr);
			}

		}

		/* Learn more about the item */
		object_can_flags(o_ptr,f1,f2,f3,f4);

		/* Description */
		object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

		/* Describe */
		if (item >= INVEN_WIELD)
		{
			msg_format("%^s: %s (%c).",
				   describe_use(item), o_name, index_to_label(item));
		}
		else if (item >= 0)
		{
			msg_format("In your pack: %s (%c).",
				   o_name, index_to_label(item));
		}
		else
		{
			msg_format("On the ground: %s.",
				   o_name);
		}
	}
	else
	{
		/* Tell the player the bad news */
		msg_format("%s%s.",p,r);
	}

	if (done)
	{
		/* Tell the player to stop trying */
		msg_format("You feel you know all %s secrets.",(o_ptr->number>0?"their":"its"));

	}
	else if (f1 | f2 | f3 | f4)
	{
		/* Set text_out hook */
		text_out_hook = text_out_to_screen;

		/* Load screen */
		screen_save();

		/* Begin recall */
		Term_gotoxy(0, 1);

		/* Actually display the item */
		list_object_flags(f1, f2, f3, f4, 1);

		(void)anykey();
	
		/* Load screen */
		screen_load();
	}

	/* Something happened */
	return (TRUE);
}



/*
 * Identify an object in the inventory (or on the floor)
 * of a specified tval only.
 * This routine does *not* automatically combine objects.
 * Returns TRUE if something was identified, else FALSE.
 */
bool ident_spell_tval(int tval)
{
	int item;

	object_type *o_ptr;

	char o_name[80];

	cptr q, s;


	/* Restrict items */
	unknown_tval = tval;

	/* Only un-id'ed items */
	item_tester_hook = item_tester_unknown_tval;

	/* Get an item */
	q = "Identify which item? ";
	s = "You have nothing to identify.";

	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Identify it fully */
	object_aware(o_ptr);
	object_known(o_ptr);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);

	/* Description */
	object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

	/* Describe */
	if (item >= INVEN_WIELD)
	{
		msg_format("%^s: %s (%c).",
			   describe_use(item), o_name, index_to_label(item));
	}
	else if (item >= 0)
	{
		msg_format("In your pack: %s (%c).",
			   o_name, index_to_label(item));
	}
	else
	{
		msg_format("On the ground: %s.",
			   o_name);
	}

	/* Something happened */
	return (TRUE);
}



/*
 * Fully "identify" an object in the inventory
 *
 * This routine returns TRUE if an item was identified.
 */
bool identify_fully(void)
{
	int item;

	object_type *o_ptr;

	char o_name[80];

	cptr q, s;


	/* Only un-*id*'ed items */
	item_tester_hook = item_tester_unknown_star;

	/* Get an item */
	q = "Identify which item? ";
	s = "You have nothing to identify.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Identify it fully */
	object_aware(o_ptr);
	object_known(o_ptr);
	object_mental(o_ptr);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);

	/* Handle stuff */
	handle_stuff();

	/* Description */
	object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

	/* Describe */
	if (item >= INVEN_WIELD)
	{
		msg_format("%^s: %s (%c).",
			   describe_use(item), o_name, index_to_label(item));
	}
	else if (item >= 0)
	{
		msg_format("In your pack: %s (%c).",
			   o_name, index_to_label(item));
	}
	else
	{
		msg_format("On the ground: %s.",
			   o_name);
	}


	msg_print("");

	/* Save the screen */
	screen_save();

	/* Describe */
	screen_object(o_ptr);

	(void)anykey();

	/* Load the screen */
	screen_load();

	/* Success */
	return (TRUE);
}

/*
 * Hook for "get_item()".  Determine if something is rechargable.
 */
static bool item_tester_hook_recharge(const object_type *o_ptr)
{
	/* Recharge staffs */
	if (o_ptr->tval == TV_STAFF) return (TRUE);

	/* Recharge wands */
	if (o_ptr->tval == TV_WAND) return (TRUE);

	/* Hack -- Recharge rods */
	if (o_ptr->tval == TV_ROD) return (TRUE);

	/* Nope */
	return (FALSE);
}


/*
 * Recharge a wand/staff/rod from the pack or on the floor.
 *
 * Mage -- Recharge I --> recharge(5)
 * Mage -- Recharge II --> recharge(40)
 * Mage -- Recharge III --> recharge(100)
 *
 * Priest -- Recharge --> recharge(15)
 *
 * Scroll of recharging --> recharge(60)
 *
 * recharge(20) = 1/6 failure for empty 10th level wand
 * recharge(60) = 1/10 failure for empty 10th level wand
 *
 * It is harder to recharge high level, and highly charged wands.
 *
 * XXX XXX XXX Beware of "sliding index errors".
 *
 * Should probably not "destroy" over-charged items, unless we
 * "replace" them by, say, a broken stick or some such.  The only
 * reason this is okay is because "scrolls of recharging" appear
 * BEFORE all staffs/wands/rods in the inventory.  Note that the
 * new "auto_sort_pack" option would correctly handle replacing
 * the "broken" wand with any other item (i.e. a broken stick).
 *
 * XXX XXX XXX Perhaps we should auto-unstack recharging stacks.
 */
bool recharge(int num)
{
	int i, t, item, lev;

	object_type *o_ptr;

	cptr q, s;


	/* Only accept legal items */
	item_tester_hook = item_tester_hook_recharge;

	/* Get an item */
	q = "Recharge which item? ";
	s = "You have nothing to recharge.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return (FALSE);

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Extract the object "level" */
	lev = k_info[o_ptr->k_idx].level;

	/* Recharge a rod */
	if (o_ptr->tval == TV_ROD)
	{
		/* Extract a recharge power */
		i = (100 - lev + num) / 5;

		/* Back-fire */
		if ((i <= 1) || (rand_int(i) == 0))
		{
			/* Hack -- backfire */
			msg_print("The recharge backfires, draining the rod further!");

			/* Hack -- decharge the rod */
			if (o_ptr->charges < 10000) o_ptr->charges = (o_ptr->charges + 100) * 2;
		}

		/* Recharge */
		else
		{
			/* Rechange amount */
			t = (num * damroll(2, 4));

			/* Recharge by that amount */
			if (o_ptr->timeout > t)
			{
				o_ptr->timeout -= t;
			}

			/* Fully recharged */
			else
			{
				o_ptr->timeout = 0;
			}
		}

		/* Hack -- round up */
		o_ptr->stackc = 0;

	}

	/* Recharge wand/staff */
	else
	{
		/* Recharge power */
		i = (num + 100 - lev - (10 * o_ptr->charges)) / 15;

		/* Back-fire XXX XXX XXX */
		if ((i <= 1) || (rand_int(i) == 0))
		{
			/* Dangerous Hack -- Destroy the item */
			msg_print("There is a bright flash of light.");

			/* Reduce and describe inventory */
			if (item >= 0)
			{
				/* Forget about item */
				inven_drop_flags(o_ptr);

				inven_item_increase(item, -999);
				inven_item_describe(item);
				inven_item_optimize(item);
			}

			/* Reduce and describe floor item */
			else
			{
				floor_item_increase(0 - item, -999);
				floor_item_describe(0 - item);
				floor_item_optimize(0 - item);
			}
		}

		/* Recharge */
		else
		{
			/* Extract a "power" */
			t = (num / (lev + 2)) + 1;

			/* Recharge based on the power */
			if (t > 0) o_ptr->charges += 2 + randint(t);

			/* Hack -- we no longer "know" the item */
			o_ptr->ident &= ~(IDENT_KNOWN);

			/* Hack -- we no longer "sense" the item */
			o_ptr->ident &= ~(IDENT_SENSE);

			/* Hack -- the item is no longer empty */
			if (o_ptr->feeling == INSCRIP_EMPTY) o_ptr->feeling = 0;

			/* Hack -- round up */
			o_ptr->stackc = 0;

		}
	}

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN);

	/* Something was done */
	return (TRUE);
}








/*
 * Apply a "project()" directly to all viewable monsters
 *
 * Note that affected monsters are NOT auto-tracked by this usage.
 */
static bool project_hack(int typ, int dam)
{
	int i, x, y;

	int flg = PROJECT_JUMP | PROJECT_KILL | PROJECT_HIDE | PROJECT_PLAY | PROJECT_ITEM | PROJECT_GRID;

	bool obvious = FALSE;


	/* Affect all (nearby) monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Location */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Require line of sight */
		if (!player_has_los_bold(y, x)) continue;

		/* Jump directly to the target monster */
		if (project(-1, 0, y, x, y, x, dam, typ, flg, 0, 0)) obvious = TRUE;
	}

	/* Result */
	return (obvious);
}



/*
 * Wake up all monsters, and speed up "los" monsters.
 */
void aggravate_monsters(int who)
{
	int i;

	bool sleep = FALSE;
	bool speed = FALSE;

	/* Aggravate everyone nearby */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip aggravating monster (or player) */
		if (i == who) continue;

		/* Wake up nearby sleeping monsters */
		if (m_ptr->cdis < MAX_SIGHT * 2)
		{
			/* Wake up */
			if (m_ptr->csleep)
			{
				/* Wake up */
				m_ptr->csleep = 0;
				sleep = TRUE;
			}
		}

		/* Speed up monsters in line of sight */
		if (player_has_los_bold(m_ptr->fy, m_ptr->fx))
		{
			/* Speed up (instantly) to racial base + 10 */
			if (m_ptr->mspeed < r_ptr->speed + 10)
			{
				/* Speed up */
				m_ptr->mspeed = r_ptr->speed + 10;
				speed = TRUE;
			}
		}
	}

	/* Messages */
	if (speed) msg_print("You feel a sudden stirring nearby!");
	else if (sleep) msg_print("You hear a sudden stirring in the distance!");
}



/*
 * Delete all non-unique monsters of a given "type" from the level
 */
bool banishment(void)
{
	int i;

	char typ;

	bool result = FALSE;


	/* Mega-Hack -- Get a monster symbol */
	(void)(get_com("Choose a monster race (by symbol) to banishment: ", &typ));

	/* Delete the monsters of that "type" */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Hack -- Skip Unique Monsters */
		if (r_ptr->flags1 & (RF1_UNIQUE)) continue;

		/* Skip "wrong" monsters */
		if (r_ptr->d_char != typ) continue;

		/* Delete the monster */
		delete_monster_idx(i);

		/* Take some damage */
		take_hit(randint(4), "the strain of casting Banishment");

		/* Take note */
		result = TRUE;
	}

	/* Update monster list window */
	p_ptr->window |= PW_MONLIST;

	return (result);
}


/*
 * Delete all nearby (non-unique) monsters
 */
bool mass_banishment(void)
{
	int i;

	bool result = FALSE;


	/* Delete the (nearby) monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Hack -- Skip unique monsters */
		if (r_ptr->flags1 & (RF1_UNIQUE)) continue;

		/* Skip distant monsters */
		if (m_ptr->cdis > MAX_SIGHT) continue;

		/* Delete the monster */
		delete_monster_idx(i);

		/* Take some damage */
		take_hit(randint(3), "the strain of casting Mass Banishment");

		/* Note effect */
		result = TRUE;
	}

	/* Update monster list window */
	p_ptr->window |= PW_MONLIST;

	return (result);
}



/*
 * The spell of destruction
 *
 * This spell "deletes" monsters (instead of "killing" them).
 *
 * Later we may use one function for both "destruction" and
 * "earthquake" by using the "full" to select "destruction".
 */
void destroy_area(int y1, int x1, int r, bool full)
{
	int y, x, k, t;

	bool flag = FALSE;

	/* Prevent compiler warning */
	(void)full;

	/* Big area of affect */
	for (y = (y1 - r); y <= (y1 + r); y++)
	{
		for (x = (x1 - r); x <= (x1 + r); x++)
		{
			/* Skip illegal grids */
			if (!in_bounds_fully(y, x)) continue;

			/* Extract the distance */
			k = distance(y1, x1, y, x);

			/* Stay in the circle of death */
			if (k > r) continue;

			/* Lose room and vault */
			cave_info[y][x] &= ~(CAVE_ROOM);

			/* Lose light */
			cave_info[y][x] &= ~(CAVE_GLOW);

			/* Lose knowledge */
			play_info[y][x] &= ~(PLAY_MARK);

			/* Hack -- Notice player affect */
			if (cave_m_idx[y][x] < 0)
			{
				/* Hurt the player later */
				flag = TRUE;

				/* Do not hurt this grid */
				continue;
			}

			/* Hack -- Skip the epicenter */
			if ((y == y1) && (x == x1)) continue;

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
				/* Chasm */
				else if (f_info[cave_feat[y][x]].flags2 & (FF2_CHASM))
				{
					/* Nothing */
				}
				/* Rubble */
				else if (rand_int(100) < 15)
				{
					/* Create magma vein */
					cave_set_feat(y,x,FEAT_RUBBLE);
				}

			}
			/* Destroy "valid" grids */
			else if (cave_valid_bold(y, x))
			{
				int feat = FEAT_FLOOR;

				/* Delete objects */
				delete_object(y, x);

				/* Wall (or floor) type */
				t = rand_int(200);

				/* Burn stuff */
				if (f_info[cave_feat[y][x]].flags2 & (FF2_HURT_FIRE))
				{
					feat = feat_state(cave_feat[y][x], FS_HURT_FIRE);
				}

				/* Granite */
				else if (t < 20)
				{
					/* Create granite wall */
					feat = FEAT_WALL_EXTRA;
				}

				/* Quartz */
				else if (t < 70)
				{
					/* Create quartz vein */
					feat = FEAT_QUARTZ;
				}

				/* Magma */
				else if (t < 100)
				{
					/* Create magma vein */
					feat = FEAT_MAGMA;
				}

				/* Rubble */
				else if (t < 130)
				{
					/* Create magma vein */
					feat = FEAT_RUBBLE;
				}

				/* Change the feature */
				cave_set_feat(y, x, feat);
			}
		}
	}


	/* Hack -- Affect player */
	if (flag)
	{
		/* Message */
		msg_print("There is a searing blast of light!");

		/* Blind the player */
		if ((p_ptr->cur_flags2 & (TR2_RES_BLIND | TR2_RES_LITE)) == 0)
		{
			/* Become blind */
			(void)set_blind(p_ptr->blind + 10 + randint(10));

			/* Always notice */
			equip_not_flags(0x0L,TR2_RES_BLIND,0x0L,0x0L);

			/* Always notice */
			equip_not_flags(0x0L,TR2_RES_LITE,0x0L,0x0L);
		}
		else
		{
			/* Sometimes notice */
			if (((p_ptr->cur_flags2 & (TR2_RES_BLIND)) != 0) && (rand_int(100)<50)) equip_can_flags(0x0L,TR2_RES_BLIND,0x0L,0x0L);

			/* Sometimes notice */
			if (((p_ptr->cur_flags2 & (TR2_RES_LITE)) != 0) && (rand_int(100)<50)) equip_can_flags(0x0L,TR2_RES_LITE,0x0L,0x0L);
		}
	}


	/* Fully update the visuals */
	p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD | PW_MONLIST | PW_MAP);
}


/*
 * Entomb a player or monster in a location known to be impassable.
 *
 * Players take a lot of damage.
 * 
 * Monsters will take damage, and "jump" into a safe grid if possible,
 * otherwise they will be "buried" in the rubble, disappearing from
 * the level in the same way that they do when banished.
 *
 * Note that players and monsters (except eaters of walls and passers
 * through walls) will never occupy the same grid as a wall (or door).
 *
 * This is not 'strictly' true, as it is now possible to get stuck in
 * cages, blocks of ice and so on, and locations such as rubble that
 * are not strictly empty. So we should technically check for FF1_MOVE
 * and FF3_EASY_CLIMB both not existing before calling this function.
 *
 * Note we encode a boolean value of invalid directions around the
 * grid to prevent monsters getting hit twice by earthquakes.
 *
 * XXX Consider passing a damage value and/or flavor so that getting
 * stuck in ice is different from granite wall is different from 
 * (impassable) rubble. Of course, we could just pull this from the
 * feature at this location.
 */
void entomb(int cy, int cx, byte invalid)
{
	int i;
	int y, x;
	int sy = 0, sx = 0, sn = 0;
	int damage = 0;

	/* Entomb the player */
	if (cave_m_idx[cy][cx] < 0)
	{

		/* Check around the player */
		for (i = 0; i < 8; i++)
		{
			/* Get the location */
			y = cy + ddy_ddd[i];
			x = cx + ddx_ddd[i];

			/* Skip non-empty grids */
			if (!cave_empty_bold(y, x)) continue;

			/* Important -- Skip "quake" grids */
			if ((invalid & (1 << i)) != 0) continue;

			/* Count "safe" grids, apply the randomizer */
			if ((++sn > 1) && (rand_int(sn) != 0)) continue;

			/* Save the safe location */
			sy = y; sx = x;
		}

		/* Random message */
		switch (randint(3))
		{
			case 1:
			{
				msg_print("The cave ceiling collapses!");
				break;
			}
			case 2:
			{
				msg_print("The cave floor twists in an unnatural way!");
				break;
			}
			default:
			{
				msg_print("The cave quakes!");
				msg_print("You are pummeled with debris!");
				break;
			}
		}

		/* Hurt the player a lot */
		if (!sn)
		{
			/* Message and damage */
			msg_print("You are severely crushed!");
			damage = 300;
		}

		/* Destroy the grid, and push the player to safety */
		else
		{
			/* Calculate results */
			switch (randint(3))
			{
				case 1:
				{
					msg_print("You nimbly dodge the blast!");
					damage = 0;
					break;
				}
				case 2:
				{
					msg_print("You are bashed by rubble!");
					damage = damroll(10, 4);
					(void)set_stun(p_ptr->stun + randint(50));
					break;
				}
				case 3:
				{
					msg_print("You are crushed between the floor and ceiling!");
					damage = damroll(10, 4);
					(void)set_stun(p_ptr->stun + randint(50));
					break;
				}
			}

			/* Move player */
			monster_swap(cy, cx, sy, sx);
		}

		/* Take some damage */
		if (damage) take_hit(damage, "an earthquake");
	}
	/* Entomb a monster */
	else if (cave_m_idx[cy][cx] > 0)
	{
		monster_type *m_ptr = &m_list[cave_m_idx[cy][cx]];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		/* Most monsters cannot co-exist with rock */
		if (!(r_ptr->flags2 & (RF2_KILL_WALL)) && !(m_ptr->tim_passw) &&
		    !(r_ptr->flags2 & (RF2_PASS_WALL)) &&
			!(f_info[cave_feat[cy][cx]].flags3 & (FF3_OUTSIDE)))
		{
			char m_name[80];

			/* Assume not safe */
			sn = 0;

			/* Monster can move to escape the wall */
			if (!(r_ptr->flags1 & (RF1_NEVER_MOVE)))
			{
				/* Look for safety */
				for (i = 0; i < 8; i++)
				{
					/* Get the grid */
					y = cy + ddy_ddd[i];
					x = cx + ddx_ddd[i];

					/* Skip non-empty grids */
					if (!cave_empty_bold(y, x)) continue;

					/* Hack -- no safety on glyph of warding */
					if (f_info[cave_feat[y][x]].flags1 & (FF1_GLYPH)) continue;

					/* Important -- Skip "quake" grids */
					if ((invalid & (1 << i)) != 0) continue;

					/* Count "safe" grids, apply the randomizer */
					if ((++sn > 1) && (rand_int(sn) != 0)) continue;

					/* Save the safe grid */
					sy = y;
					sx = x;
				}
			}

			/* Describe the monster */
			monster_desc(m_name, m_ptr, 0);

			/* Scream in pain */
			msg_format("%^s wails out in pain!", m_name);

			/* Take damage from the quake */
			damage = (sn ? (int)damroll(4, 8) : (m_ptr->hp + 1));

			/* Monster is certainly awake */
			m_ptr->csleep = 0;

			/* Apply damage directly */
			m_ptr->hp -= damage;

			/* Delete (not kill) "dead" monsters */
			if (m_ptr->hp < 0)
			{
				/* Message */
				msg_format("%^s is embedded in the rock!", m_name);

				/* Delete the monster */
				delete_monster(cy, cx);

				/* No longer safe */
				sn = 0;
			}

			/* Hack -- Escape from the rock */
			if (sn)
			{
				/* Move the monster */
				monster_swap(cy, cx, sy, sx);
			}
		}
	}
}



/*
 * Induce an "earthquake" of the given radius at the given location.
 *
 * This will turn some walls into floors and some floors into walls.
 *
 * The player will take damage and "jump" into a safe grid if possible,
 * otherwise, he will "tunnel" through the rubble instantaneously.
 *
 * Monsters will take damage, and "jump" into a safe grid if possible,
 * otherwise they will be "buried" in the rubble, disappearing from
 * the level in the same way that they do when banished.
 *
 * Note that players and monsters (except eaters of walls and passers
 * through walls) will never occupy the same grid as a wall (or door).
 */
void earthquake(int cy, int cx, int r)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int t, y, x, yy, xx, dy, dx;

	bool hurt = FALSE;

	bool map[32][32];


	/* Paranoia -- Enforce maximum range */
	if (r > 12) r = 12;

	/* Clear the "maximal blast" area */
	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 32; x++)
		{
			map[y][x] = FALSE;
		}
	}

	/* Check around the epicenter */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip illegal grids */
			if (!in_bounds_fully(yy, xx)) continue;

			/* Skip distant grids */
			if (distance(cy, cx, yy, xx) > r) continue;

			/* Lose room and vault */
			cave_info[yy][xx] &= ~(CAVE_ROOM);

			/* Lose light */
			cave_info[y][x] &= ~(CAVE_GLOW);

			/* Lose light */
			play_info[y][x] &= ~(PLAY_MARK);

			/* Skip the epicenter */
			if (!dx && !dy) continue;

			/* Skip most grids */
			if (rand_int(100) < 85) continue;

			/* Damage this grid */
			map[16+yy-cy][16+xx-cx] = TRUE;

			/* Hack -- Take note of player damage */
			if ((yy == py) && (xx == px)  && !(f_info[cave_feat[yy][xx]].flags3 & (FF3_OUTSIDE))) hurt = TRUE;
		}
	}

	if (hurt)
	{
		/* Entomb the player */
		entomb(py,px, 0x00);
	}

	/* Examine the quaked region */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip unaffected grids */
			if (!map[16+yy-cy][16+xx-cx]) continue;

			/* Entomb monster */
			if (cave_m_idx[yy][xx] > 0)
			{
				byte invalid = 0x00;

				int i;

				/* Determine invalid directions */
				for (i = 0; i < 8; i++)
					if (map[16+yy-cy+ddy_ddd[i]][16+xx-cx+ddx_ddd[i]]) invalid |= 1 << i;

				/* Entomb the monster */
				entomb(yy, xx, invalid);
			}
		}
	}

	/* XXX XXX XXX */

	/* New location */
	py = p_ptr->py;
	px = p_ptr->px;

	/* Important -- no wall on player */
	map[16+py-cy][16+px-cx] = FALSE;

	/* Examine the quaked region */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip unaffected grids */
			if (!map[16+yy-cy][16+xx-cx]) continue;

			/* Paranoia -- never affect player */
			if ((yy == py) && (xx == px)) continue;

			/* Destroy "outside" grids */
			if ((cave_valid_bold(yy,xx)) && (f_info[cave_feat[yy][xx]].flags3 & (FF3_OUTSIDE)))
			{
				/* Delete objects */
				delete_object(yy, xx);

				/* Bash stuff */
				if (f_info[cave_feat[yy][xx]].flags1 & (FF1_BASH))
				{
					cave_alter_feat(yy,xx,FS_BASH);
				}
			}
			/* Destroy location (if valid) */
			else if (cave_valid_bold(yy, xx))
			{
				int feat = FEAT_FLOOR;

				bool floor = cave_floor_bold(yy, xx);

				/* Delete objects */
				delete_object(yy, xx);

				/* Wall (or floor) type */
				t = (floor ? rand_int(100) : 200);

				/* Granite */
				if (t < 20)
				{
					/* Create granite wall */
					feat = FEAT_WALL_EXTRA;
				}

				/* Quartz */
				else if (t < 70)
				{
					/* Create quartz vein */
					feat = FEAT_QUARTZ;
				}

				/* Magma */
				else if (t < 100)
				{
					/* Create magma vein */
					feat = FEAT_MAGMA;
				}

				/* Change the feature */
				cave_set_feat(yy, xx, feat);
			}
		}
	}


	/* Fully update the visuals */
	p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Update the health bar */
	p_ptr->redraw |= (PR_HEALTH);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD | PW_MONLIST | PW_MAP);
}




/*
 * This routine clears the entire "temp" set.
 */
void clear_temp_array(void)
{
	int i;

	/* Apply flag changes */
	for (i = 0; i < temp_n; i++)
	{
		int y = temp_y[i];
		int x = temp_x[i];

		/* No longer in the array */
		play_info[y][x] &= ~(PLAY_TEMP);
	}

	/* None left */
	temp_n = 0;
}


/*
 * Aux function -- see below
 */
void cave_temp_mark(int y, int x, bool room)
{
	/* Avoid infinite recursion */
	if (play_info[y][x] & (PLAY_TEMP)) return;

	/* Option -- do not leave the current room */
	if ((room) && (!(cave_info[y][x] & (CAVE_ROOM)))) return;

	/* Verify space */
	if (temp_n == TEMP_MAX) return;

	/* Mark the grid */
	play_info[y][x] |= (PLAY_TEMP);

	/* Add it to the marked set */
	temp_y[temp_n] = y;
	temp_x[temp_n] = x;
	temp_n++;
}

/*
 * Mark the nearby area with CAVE_TEMP flags.  Allow limited range.
 */
void spread_cave_temp(int y1, int x1, int range, bool room)
{
	int i, y, x;

	/* Add the initial grid */
	cave_temp_mark(y1, x1, room);

	/* While grids are in the queue, add their neighbors */
	for (i = 0; i < temp_n; i++)
	{
		x = temp_x[i], y = temp_y[i];

		/* Walls get marked, but stop further spread */
		if (!cave_project_bold(y, x)) continue;

		/* Note limited range (note:  we spread out one grid further) */
		if ((range) && (distance(y1, x1, y, x) >= range)) continue;

		/* Spread adjacent */
		cave_temp_mark(y + 1, x, room);
		cave_temp_mark(y - 1, x, room);
		cave_temp_mark(y, x + 1, room);
		cave_temp_mark(y, x - 1, room);

		/* Spread diagonal */
		cave_temp_mark(y + 1, x + 1, room);
		cave_temp_mark(y - 1, x - 1, room);
		cave_temp_mark(y - 1, x + 1, room);
		cave_temp_mark(y + 1, x - 1, room);
	}
}




/*
 * This routine clears the entire "temp" set.
 *
 * This routine will Perma-Lite all "temp" grids.
 *
 * This routine is used (only) by "lite_room()"
 *
 * Dark grids are illuminated.
 *
 * Also, process all affected monsters.
 *
 * SMART monsters always wake up when illuminated
 * NORMAL monsters wake up 3/4 the time when illuminated
 * STUPID monsters wake up 3/10 the time when illuminated
 *
 * Percentages were adjusted up, to make a greater change
 * of waking up monsters.
 */
static void cave_temp_room_lite(void)
{
	int i;

	/* Apply flag changes */
	for (i = 0; i < temp_n; i++)
	{
		int y = temp_y[i];
		int x = temp_x[i];

		/* No longer in the array */
		play_info[y][x] &= ~(PLAY_TEMP);

		/* Perma-Lite */
		cave_info[y][x] |= (CAVE_GLOW);
	}

	/* Fully update the visuals */
	p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

	/* Update stuff */
	update_stuff();

	/* Process the grids */
	for (i = 0; i < temp_n; i++)
	{
		int y = temp_y[i];
		int x = temp_x[i];

		/* Redraw the grid */
		lite_spot(y, x);

		/* Process affected monsters */
		if (cave_m_idx[y][x] > 0)
		{
			int chance = 75;

			monster_type *m_ptr = &m_list[cave_m_idx[y][x]];
			monster_race *r_ptr = &r_info[m_ptr->r_idx];

			/* Stupid monsters rarely wake up */
			if (r_ptr->flags2 & (RF2_STUPID)) chance = 30;

			/* Smart monsters always wake up */
			if (r_ptr->flags2 & (RF2_SMART)) chance = 100;

			/* Sometimes monsters wake up */
			if (m_ptr->csleep && (rand_int(100) < chance))
			{
				/* Wake up! */
				m_ptr->csleep = 0;

				/* Notice the "waking up" */
				if (m_ptr->ml)
				{
					char m_name[80];

					/* Get the monster name */
					monster_desc(m_name, m_ptr, 0);

					/* Dump a message */
					msg_format("%^s wakes up.", m_name);
				}
			}
		}
	}

	/* None left */
	temp_n = 0;
}



/*
 * This routine clears the entire "temp" set.
 *
 * This routine will "darken" all "temp" grids.
 *
 * In addition, some of these grids will be "unmarked".
 *
 * This routine is used (only) by "unlite_room()"
 */
static void cave_temp_room_unlite(void)
{
	int i,ii;

	/* Apply flag changes */
	for (i = 0; i < temp_n; i++)
	{
		int y = temp_y[i];
		int x = temp_x[i];

		/* No longer in the array */
		play_info[y][x] &= ~(PLAY_TEMP);

		/* Darken the grid */
		if (!(f_info[cave_feat[y][x]].flags2 & (FF2_GLOW)))
		{
			cave_info[y][x] &= ~(CAVE_GLOW);
		}

		/* Check to see if not illuminated by innately glowing grids */
		for (ii = 0; ii < 8; ii++)
		{
			int yy = y + ddy_ddd[ii];
			int xx = x + ddx_ddd[ii];

			/* Ignore annoying locations */
			if (!in_bounds_fully(yy, xx)) continue;

			if (f_info[cave_feat[yy][xx]].flags2 & (FF2_GLOW))
			{
				/* Illuminate the grid */
				cave_info[y][x] |= (CAVE_GLOW);

			}

		}

		/* Hack -- Forget "boring" grids */
		if (!(cave_info[y][x] & (CAVE_GLOW)) && 
			!(f_info[cave_feat[y][x]].flags1 & (FF1_REMEMBER)))
		{
			/* Forget the grid */
			play_info[y][x] &= ~(PLAY_MARK);
		}
	}

	/* Fully update the visuals */
	p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

	/* Update stuff */
	update_stuff();

	/* Process the grids */
	for (i = 0; i < temp_n; i++)
	{
		int y = temp_y[i];
		int x = temp_x[i];

		/* Redraw the grid */
		lite_spot(y, x);
	}

	/* None left */
	temp_n = 0;
}


/*
 * Illuminate any room containing the given location.
 */
void lite_room(int y, int x)
{
	/* Check the room */
	if (cave_info[y][x] & (CAVE_ROOM))
	{
		/* Hack --- Have we seen this room before? */
		if (!(room_has_flag(y, x, ROOM_SEEN)))
		{
			p_ptr->update |= (PU_ROOM_INFO);
			p_ptr->window |= (PW_ROOM_INFO);
		}

		/* Some rooms cannot be completely lit */
		if (room_has_flag(y, x, ROOM_GLOOMY))
		{

			/* Warn the player */
			msg_print("The light fails to penetrate the gloom.");

			return;
		}
	}

	/* Add the initial grid */
	spread_cave_temp(y, x, MAX_SIGHT, TRUE);

	/* Lite the (part of) room */
	cave_temp_room_lite();
}


/*
 * Darken all rooms containing the given location
 */
void unlite_room(int y, int x)
{
	/* Add the initial grid */
	spread_cave_temp(y, x, MAX_SIGHT, TRUE);

	/* Lite the (part of) room */
	cave_temp_room_unlite();
}


/*
 * Cast a ball spell
 * Stop if we hit a monster, act as a "ball"
 * Allow "target" mode to pass over monsters
 * Affect grids, objects, and monsters
 */
bool fire_ball(int typ, int dir, int dam, int rad)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int ty, tx;

	int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_PLAY | PROJECT_BOOM;

	/* Use the given direction */
	ty = py + 99 * ddy[dir];
	tx = px + 99 * ddx[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay())
	{
		flg &= ~(PROJECT_STOP);

		ty = p_ptr->target_row;
		tx = p_ptr->target_col;
	}

	/* Analyze the "dir" and the "target".  Hurt items on floor. */
	return (project(-1, rad, py, px, ty, tx, dam, typ, flg, 0, 0));
}


/*
 * Cast a cloud spell
 * Stop if we hit a monster, act as a "ball"
 * Allow "target" mode to pass over monsters
 * Affect monsters only
 */
bool fire_cloud(int typ, int dir, int dam, int rad)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int ty, tx;

	int flg = PROJECT_STOP | PROJECT_KILL | PROJECT_BOOM | PROJECT_PLAY | PROJECT_BOOM;

	/* Use the given direction */
	ty = py + 99 * ddy[dir];
	tx = px + 99 * ddx[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay())
	{
		flg &= ~(PROJECT_STOP);

		ty = p_ptr->target_row;
		tx = p_ptr->target_col;
	}

	/* Analyze the "dir" and the "target".  Hurt items on floor. */
	return (project(-1, rad, py, px, ty, tx, dam, typ, flg, 0, 0));
}


/*
 * Hack -- apply a "projection()" in a direction (or at the target)
 */
static bool project_hook(int typ, int dir, int dam, int flg)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int ty, tx;

	/* Pass through the target if needed */
	flg |= (PROJECT_THRU);

	/* Use the given direction */
	ty = py + ddy[dir];
	tx = px + ddx[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay())
	{
		ty = p_ptr->target_row;
		tx = p_ptr->target_col;
	}

	/* Analyze the "dir" and the "target", do NOT explode */
	return (project(-1, 0, py, px, ty, tx, dam, typ, flg, 0, 0));
}

/*
 * Apply an arc in a direction
 */
static bool fire_arc(int typ, int dir, int dam, int rad, int degrees_of_arc)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int ty, tx;

	int flg = PROJECT_ARC | PROJECT_BOOM | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_PLAY | PROJECT_WALL;

	/* Diameter of source of energy is at least 20. */
	int diameter_of_source = 20;

	int degree_factor = 60;

	/* Use the given direction */
	ty = py + 99 * ddy[dir];
	tx = px + 99 * ddx[dir];

	/* Narrow arcs lose relatively little energy over distance. */
	if (degrees_of_arc < degree_factor)
	{
		if (degrees_of_arc <= 6) diameter_of_source = rad * 10;
		else diameter_of_source = diameter_of_source * degree_factor /
			degrees_of_arc;
	}

	/* Radius of zero means no fixed limit. */
	if (rad == 0) rad = MAX_SIGHT;

	/* Analyze the "dir" and the "target" */
	return (project(-1, rad, py, px, ty, tx, dam, typ, flg, degrees_of_arc,
			(byte)diameter_of_source));
}



/*
 * Cast a bolt spell
 * Stop if we hit a monster, as a "bolt"
 * Affect monsters (not grids or objects)
 */
bool fire_bolt(int typ, int dir, int dam)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int ty, tx;

	int flg = PROJECT_STOP | PROJECT_KILL | PROJECT_GRID | PROJECT_PLAY;

	/* Use the given direction */
	ty = py + 99 * ddy[dir];
	tx = px + 99 * ddx[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay())
	{
		flg &= ~(PROJECT_STOP);

		ty = p_ptr->target_row;
		tx = p_ptr->target_col;
	}

	/* Analyze the "dir" and the "target".  Hurt items on floor. */
	return (project(-1, 0, py, px, ty, tx, dam, typ, flg, 0, 0));
}

/*
 * Cast a beam spell
 * Pass through monsters, as a "beam"
 * Affect monsters (not grids or objects)
 * Now only range 10.
 */
bool fire_beam(int typ, int dir, int dam)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int ty, tx;

	int flg = PROJECT_BEAM | PROJECT_KILL | PROJECT_GRID | PROJECT_THRU;

	int range = 10;

	/* Use the given direction */
	ty = py + 99 * ddy[dir];
	tx = px + 99 * ddx[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay())
	{
		flg &= ~(PROJECT_STOP);

		ty = p_ptr->target_row;
		tx = p_ptr->target_col;
	}

	/* Analyze the "dir" and the "target".  Hurt items on floor. */
	return (project(-1, range, py, px, ty, tx, dam, typ, flg, 0, 0));
}

/*
 * Cast a bolt spell, or rarely, a beam spell
 */
bool fire_bolt_or_beam(int prob, int typ, int dir, int dam)
{
	if (rand_int(100) < prob)
	{
		return (fire_beam(typ, dir, dam));
	}
	else
	{
		return (fire_bolt(typ, dir, dam));
	}
}


/*
 * Cast a blast spell
 * A blast spell is a radius 1 ball spell that only fires to adjacent
 * squares. Used for a couple of alchemy spells.
 */
bool fire_blast(int typ, int dir, int dam)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int ty = p_ptr->py+ddy[dir];
	int tx = p_ptr->px+ddx[dir];

	int flg = PROJECT_KILL | PROJECT_GRID | PROJECT_ITEM | PROJECT_BOOM | PROJECT_PLAY;
	return (project(-1, 1, py, px, ty, tx, dam, typ, flg, 0, 0));

}

/*
 * Hands is now a range 3 beam, similar to lightening spark from Sangband.
 * It does not affect the grid however.
 */
bool fire_hands(int typ, int dir, int dam)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int ty, tx;

	int flg = PROJECT_BEAM | PROJECT_KILL | PROJECT_THRU;

	int range = 3;

	/* Use the given direction */
	ty = py + 99 * ddy[dir];
	tx = px + 99 * ddx[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay())
	{
		flg &= ~(PROJECT_STOP);

		ty = p_ptr->target_row;
		tx = p_ptr->target_col;
	}

	/* Analyze the "dir" and the "target".  Hurt items on floor. */
	return (project(-1, range, py, px, ty, tx, dam, typ, flg, 0, 0));
}

/*
 * Create a (wielded) spell item
 */
static void wield_spell(int item, int k_idx, int time)
{
	object_type *i_ptr;
	object_type object_type_body;

	cptr act;

	char o_name[80];

	/* Get the wield slot */
	object_type *o_ptr = &inventory[item];

	/* Get local object */
	i_ptr = &object_type_body;

	/* Create the spell */
	object_prep(i_ptr, k_idx);
	i_ptr->timeout = time;
	object_aware(i_ptr);
	object_known(i_ptr);

	/* Take off existing item */
	if (o_ptr->k_idx)
	{
		/* Check if same spell */
		if ((o_ptr->tval == TV_SPELL) && (o_ptr->sval))
		{
			/* Reset duration */
			if (o_ptr->timeout < time) o_ptr->timeout = time;

			/* And done */
			return;
		}

		/* Take off existing item */
		(void)inven_takeoff(item, 255);

		/* Oops. Cursed... */
		if (inventory[item].k_idx) return;
	}

	/* 'Wear' the spell */
	object_copy(o_ptr,i_ptr);

	/* Increment the equip counter by hand */
	p_ptr->equip_cnt++;

	/* Where is the item now */
	if ((o_ptr->tval == TV_SWORD) || (o_ptr->tval == TV_POLEARM)
		|| (o_ptr->tval == TV_HAFTED) || (o_ptr->tval == TV_STAFF) ||
		(o_ptr->tval == TV_DIGGING))
	{
		act = "You are wielding";
		if (item == INVEN_ARM) act = "You are wielding off-handed";
	}
	else if (item == INVEN_WIELD)
	{
		act = "You are using";
	}
	else if (o_ptr->tval == TV_INSTRUMENT)
	{
		act = "You are playing music with";
	}
	else if (item == INVEN_BOW)
	{
		act = "You are shooting with";
	}
	else if (item == INVEN_LITE)
	{
		act = "Your light source is";
	}
	else
	{
		act = "You are wearing";
	}



	/* Describe the result */
	object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

	/* Message */
	msg_format("%s %s (%c).", act, o_name, index_to_label(item));

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Recalculate torch */
	p_ptr->update |= (PU_TORCH);

	/* Recalculate mana */
	p_ptr->update |= (PU_MANA);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);

}


/*
 * Enchant item --- a big pile of (fun) hack
 *
 * Basically we select an adjacent item of the same tval,
 * compare the difference in levels between the two, and 
 * if randint(plev) > difference, polymorph the item.
 * Otherwise scan until we encounter a 0 value item
 * of the same tval and polymorph the item into that, or
 * generate some junk (50% chance of each).
 * We need to generate junk or caster could infinitely
 * polymorph their items until getting exceedingly good
 * stuff.
 * Note it is dangerous to allow enchant scroll...
 */
static void enchant_item(byte tval, int plev)
{

	object_type *o_ptr;
	object_type *i_ptr;
	object_type object_type_body;

	cptr q,s;
	int dirs,kind,i;

	int item;

	/* Get the local item */
	i_ptr = &object_type_body;

	q = "Enchant which item?";
	s = "You have no items to enchant.";

	/* Restrict choices */
	item_tester_tval = tval;

	/* Get an item */
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Start with same kind */
	i = o_ptr->k_idx;

	/* Randomly increase or decrease */
	if (rand_int(100)<50)
	{
		if (k_info[i-1].tval == tval) dirs = -1;
		else dirs = 1;
	}
	else
	{
		if (k_info[i-1].tval == tval) dirs = 1;
		else dirs = -1;
	}

	/* Check for improvement */
	if (randint(plev) > (k_info[i+dirs].level - k_info[i].level))
	{
		/*Success*/
		kind = i+dirs;
	}
	else if (randint(100) < 50)
	{
		/* Pick bad failure */
		while ((k_info[i].cost) && (k_info[i].tval == tval))
		{
			/* Keep trying */
			i+=dirs;
		}

		/* No bad items downwards */
		if (k_info[i].tval != tval)
		{
			/* Try other direction */
			i = o_ptr->k_idx;
			dirs = -dirs;

			/* Pick bad failure */
			while ((k_info[i].cost) && (k_info[i].tval == tval))
			{
				/* Keep trying */
				i+=dirs;
			}
		}
					
		/* No bad items at all */
		if (k_info[i].tval !=tval)
		{
			i = o_ptr->k_idx;
		}

		/* No change */
		kind = i;
	}
	else
	{

		/* Pick some junk*/
		switch (tval)
		{
			case TV_POTION:
			{
				kind = lookup_kind(TV_HOLD,1);
				break;
			}
		/* XXX Really need some junk for each of these */
		case TV_WAND:
		case TV_STAFF:
		case TV_ROD:
			{
				kind = lookup_kind(TV_JUNK,6);
				break;
			}
		/* XXX Odd junk, but can't be food */
		case TV_FOOD:
			{
				kind = lookup_kind(TV_JUNK,3);
				break;
			}
		/* XXX Pick meaningless junk */
		default:
			{
				kind = lookup_kind(TV_JUNK,3);
				break;
			}
		}
	}
	

	/* Ta da! */
	msg_print("There is a blinding flash!");

	/* Destroy an item in the pack */
	if (item >= 0)
	{
		/* Forget about item */
		if (o_ptr->number == 1) inven_drop_flags(o_ptr);

		inven_item_increase(item, -1);
		inven_item_describe(item);
		inven_item_optimize(item);
	}

	/* Destroy a potion on the floor */
	else
	{
		floor_item_increase(0 - item, -1);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
	}

	/* Create the new object */
	object_prep(i_ptr, kind);

	/* Carry the object */
	item = inven_carry(i_ptr);

	/* Describe the new object */
	inven_item_describe(item);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP);

}

/*
 * Create gold
 */
static void create_gold(void)
{
	object_type *i_ptr;
	object_type object_type_body;

	int base;

	/* Get local object */
	i_ptr = &object_type_body;

	/* Create the gold */
	object_prep(i_ptr, lookup_kind(TV_GOLD, 10));

	/* Hack -- Base coin cost */
	base = k_info[i_ptr->k_idx].cost;

	/* Determine how much the treasure is "worth" */
	i_ptr->charges = (base + (8L * randint(base)) + randint(8));

	/* Floor carries the item */
	drop_near(i_ptr, 0, p_ptr->py, p_ptr->px);

	/* XXX To do thesis on inflation in Angband economies. */
}


/*
 * Curse the players armor
 */
static bool curse_armor(void)
{
	object_type *o_ptr;

	char o_name[80];


	/* Curse the body armor */
	o_ptr = &inventory[INVEN_BODY];

	/* Nothing to curse */
	if (!o_ptr->k_idx) return (FALSE);


	/* Describe */
	object_desc(o_name, sizeof(o_name), o_ptr, FALSE, 3);

	/* Attempt a saving throw for artifacts */
	if (artifact_p(o_ptr) && (rand_int(100) < 50))
	{
		/* Cool */
		msg_format("A %s tries to %s, but your %s resists the effects!",
			   "terrible black aura", "surround your armor", o_name);
	}

	/* not artifact or failed save... */
	else
	{
		/* Oops */
		msg_format("A terrible black aura blasts your %s!", o_name);

		/* Blast the armor */
		o_ptr->name1 = 0;
		o_ptr->name2 = EGO_BLASTED;
		o_ptr->to_a = 0 - randint(5) - randint(5);
		o_ptr->to_h = 0;
		o_ptr->to_d = 0;
		o_ptr->ac = 0;
		o_ptr->dd = 0;
		o_ptr->ds = 0;

		/* Forget about it */
		inven_drop_flags(o_ptr);
		drop_all_flags(o_ptr);

		/* Curse it */
		o_ptr->ident |= (IDENT_CURSED);

		/* Break it */
		o_ptr->ident |= (IDENT_BROKEN);

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Recalculate mana */
		p_ptr->update |= (PU_MANA);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);
	}

	return (TRUE);
}


/*
 * Curse the players weapon
 */
static bool curse_weapon(void)
{
	object_type *o_ptr;

	char o_name[80];


	/* Curse the weapon */
	o_ptr = &inventory[INVEN_WIELD];

	/* Nothing to curse */
	if (!o_ptr->k_idx) return (FALSE);


	/* Describe */
	object_desc(o_name, sizeof(o_name), o_ptr, FALSE, 3);

	/* Attempt a saving throw */
	if (artifact_p(o_ptr) && (rand_int(100) < 50))
	{
		/* Cool */
		msg_format("A %s tries to %s, but your %s resists the effects!",
			   "terrible black aura", "surround your weapon", o_name);
	}

	/* not artifact or failed save... */
	else
	{
		/* Oops */
		msg_format("A terrible black aura blasts your %s!", o_name);

		/* Shatter the weapon */
		o_ptr->name1 = 0;
		o_ptr->name2 = EGO_SHATTERED;
		o_ptr->to_h = 0 - randint(5) - randint(5);
		o_ptr->to_d = 0 - randint(5) - randint(5);
		o_ptr->to_a = 0;
		o_ptr->ac = 0;
		o_ptr->dd = 0;
		o_ptr->ds = 0;

		/* Drop all flags */
		inven_drop_flags(o_ptr);
		drop_all_flags(o_ptr);

		/* Curse it */
		o_ptr->ident |= (IDENT_CURSED);

		/* Break it */
		o_ptr->ident |= (IDENT_BROKEN);

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Recalculate mana */
		p_ptr->update |= (PU_MANA);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);
	}

	/* Notice */
	return (TRUE);
}


/*
 *      Process a spell.
 *
 *      Returns -1 iff the spell did something obvious.
 *
 *      Level is set to 0 for most objects, and player level for spells.
 *
 *      *abort is set to TRUE if the user aborts before any spell effects
 *      occur. The return value reflects whether or not they were able to
 *      determine what effect the item has (Currently enchantment and
 *      identify items).
 *
 *      We should choose any item for enchant/curse/identify spells before
 *      calling this routine, when we are using an unknown object, and
 *      only apply the spell, and make the effect obvious, if the object
 *      is correctly chosen. XXX
 *
 *      We should allow useful spells to be cast on adjacent monsters. XXX
 *
 *      We should make summoned monsters friendly if plev is > 0. XXX
 *
 */

bool process_spell_blows(int spell, int level, bool *cancel)
{

	spell_type *s_ptr = &s_info[spell];

	bool obvious = FALSE;
	int ap_cnt;
	int dir;

	/* Scan through all four blows */
	for (ap_cnt = 0; ap_cnt < 4; ap_cnt++)
	{
		int damage = 0;

		/* Extract the attack infomation */
		int effect = s_ptr->blow[ap_cnt].effect;
		int method = s_ptr->blow[ap_cnt].method;
		int d_dice = s_ptr->blow[ap_cnt].d_dice;
		int d_side = s_ptr->blow[ap_cnt].d_side;
		int d_plus = s_ptr->blow[ap_cnt].d_plus;

		/* Hack -- no more attacks */
		if (!method) break;

		/* Mega hack -- dispel evil/undead objects */
		if ((!d_side) && (!level))
		{
			d_plus += 25 * d_dice;
		}
		/* Hack -- use level as modifier */
		else if (!d_side)
		{
			d_plus += level * d_dice;
		}

		/* Roll out the damage */
		if ((d_dice) && (d_side))
		{
			damage = damroll(d_dice, d_side) + d_plus;
		}
		else
		{
			damage = d_plus;
		}

		/* Apply spell method */
		switch (method)
		{
			/* Affect self directly */
			case RBM_SELF:
			{
				int py = p_ptr->py;
				int px = p_ptr->px;

				if (project_p(-2, 0, py, px, damage, effect)) obvious = TRUE;
				break;
			}

			/* Radius 1, centred on self */
			case RBM_ADJACENT:
			{
				int py = p_ptr->py;
				int px = p_ptr->px;
			
				int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE | PROJECT_KILL;
				if (project(-1, 1, py, px, py, px, damage, effect, flg, 0, 0)) obvious = TRUE;
				break;
			}
			case RBM_HANDS:
			{
				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				/* Hack - scale damage */
				if ((level > 5) && (d_side)) damage+= damroll((level-1)/5, d_side);

				if (fire_hands(effect, dir, damage)) obvious = TRUE;
				break;
			}
			case RBM_MISSILE:
			{
				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				/* Hack - scale damage */
				if ((level > 5) && (d_side)) damage += damroll((level-1)/5, d_side);

				if (fire_bolt(effect, dir, damage)) obvious = TRUE;
				break;
			}
			case RBM_BOLT_10:
			{
				int beam;

				if (c_info[p_ptr->pclass].spell_power) beam = level;
				else beam = level/2;

				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				/* Hack - scale damage */
				if ((level > 8) && (d_side)) damage += damroll((level-5)/4, d_side);
				
				if (fire_bolt_or_beam(beam - 10, effect, dir, damage)) obvious = TRUE;
				break;
			}
			case RBM_BOLT:
			{
				int beam;

				if (c_info[p_ptr->pclass].spell_power) beam = level;
				else beam = level/2;

				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);
				/* Hack - scale damage */
				if ((level > 8) && (d_side)) damage += damroll((level-5)/4, d_side);
				
				if (fire_bolt_or_beam(beam, effect, dir, damage)) obvious = TRUE;

				break;
			}
			case RBM_BEAM:
			{
				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				/* Hack - scale damage */
				if ((level > 8) && (d_side)) damage += damroll((level-5)/4, d_side);

				if (fire_beam(effect, dir, damage)) obvious = TRUE;

				break;
			}
			case RBM_BLAST:
			{
				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				/* Hack - scale damage */
				damage += level;

				if (fire_blast(effect, dir, damage)) obvious = TRUE;

				break;
			}
			case RBM_WALL:
			{
				int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL | PROJECT_ITEM;

				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				/* Hack - scale damage */
				if ((level > 8) && (d_side)) damage += damroll((level-5)/4, d_side);

				if (project_hook(effect, dir, damage, flg)) obvious = TRUE;
				break;
			}
			case RBM_BALL:
			{
				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				if (fire_ball(effect, dir, damage, 2)) obvious = TRUE;

				break;
			}
			case RBM_CLOUD:
			{
				int rad = 2;

				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				/* Hack - scale damage */
				damage += level / 2;
				
				if (fire_cloud(effect, dir, damage, rad)) obvious = TRUE;

				break;
			}
			case RBM_STORM:
			{
				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				if (fire_ball(effect, dir, damage, 3)) obvious = TRUE;

				break;
			}
			case RBM_BREATH:
			{
				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				if (fire_ball(effect, dir, MIN(p_ptr->chp,damage), 2)) obvious = TRUE;

				break;
			}
			case RBM_AREA:
			{
				int py = p_ptr->py;
				int px = p_ptr->px;
			
				int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE | PROJECT_KILL | PROJECT_BOOM;
				if (project(-1, (level / 10)+1, py, px, py, px, damage, effect, flg, 0, 0)) obvious = TRUE;
				break;
			}
			case RBM_LOS:
			{
				if (project_hack(effect, damage)) obvious = TRUE;

				break;
			}
			case RBM_LINE:
			{

				int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL;

				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);
				if (project_hook(effect, dir, damage, flg)) obvious = TRUE;
				break;
			}
			case RBM_AIM:
			{
				int flg = PROJECT_STOP | PROJECT_KILL;
				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				if (project_hook(effect, dir, damage, flg)) obvious = TRUE;

				break;
			}
			case RBM_ORB:
			{
				int rad = (level < 30 ? 2 : 3);

				if (c_info[p_ptr->pclass].spell_power) damage += level + level/2;
				else damage += level + level/4;

				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				if (fire_ball(effect, dir, damage, rad)) obvious = TRUE;

				break;
			}
			case RBM_CROSS:
			{
				int k;

				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				for (k = 0; k < 4; k++) if (fire_beam(effect, ddd[k], damage)) obvious = TRUE;

				break;
			}
			case RBM_STAR:
			{
				int k;

									
				for (k = 0; k < 8; k++) if (fire_beam(effect, ddd[k], damage)) obvious = TRUE;

				break;
			}
			case RBM_AURA:
			/* Radius 2, centred on self */
			{
				int py = p_ptr->py;
				int px = p_ptr->px;
			
				int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE | PROJECT_KILL | PROJECT_BOOM;
				if (project(-1, 2, py, px, py, px, damage, effect, flg, 0, 0)) obvious = TRUE;
				break;
			}
			case RBM_EXPLODE:
			/* Radius 1, centred on self -- affects self */
			{
				int py = p_ptr->py;
				int px = p_ptr->px;
			
				int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_BOOM | PROJECT_PLAY;
				if (project(-2, 1, py, px, py, px, damage, effect, flg, 0, 0)) obvious = TRUE;
				break;
			}
			case RBM_SPHERE:
			/* Variable radius */
			{
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				if (fire_ball(effect, dir, damage, (level/10)+1)) obvious = TRUE;

				break;
			}
			case RBM_PANEL:
			{
				if (project_hack(effect, damage)) obvious = TRUE;

				break;
			}
			case RBM_LEVEL:
			{
				if (project_hack(effect, damage)) obvious = TRUE;

				break;
			}

			case RBM_STRIKE:
			{
				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				/* Hack - scale damage */
				if ((level > 5) && (d_side)) damage += damroll((level-1)/5, d_side);

				if (fire_ball(effect, dir, damage, 0)) obvious = TRUE;

				break;
			}

			case RBM_ARC_20:
			{
				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				/* Hack - scale damage */
				if ((level > 5) && (d_side)) damage += damroll((level-1)/5, d_side);

				if (fire_arc(effect, dir, damage, 0, 20)) obvious = TRUE;

				break;
			}

			case RBM_ARC_30:
			{
				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				/* Hack - scale damage */
				if ((level > 5) && (d_side)) damage += damroll((level-1)/5, d_side);

				if (fire_arc(effect, dir, damage, 0, 30)) obvious = TRUE;

				break;
			}

			case RBM_ARC_40:
			{
				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				/* Hack - scale damage */
				if ((level > 5) && (d_side)) damage += damroll((level-1)/5, d_side);

				if (fire_arc(effect, dir, damage, 0, 40)) obvious = TRUE;

				break;
			}

			case RBM_ARC_50:
			{
				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				/* Hack - scale damage */
				if ((level > 5) && (d_side)) damage += damroll((level-1)/5, d_side);

				if (fire_arc(effect, dir, damage, 0, 50)) obvious = TRUE;

				break;
			}

			case RBM_ARC_60:
			{
				/* Allow direction to be cancelled for free */
				if ((!get_aim_dir(&dir)) && (*cancel)) return (FALSE);

				/* Hack - scale damage */
				if ((level > 5) && (d_side)) damage += damroll((level-1)/5, d_side);

				if (fire_arc(effect, dir, damage, 0, 60)) obvious = TRUE;

				break;
			}

			default:
			{
				/* Allow direction to be cancelled for free */
				if ((!get_rep_dir(&dir)) && (*cancel)) return (FALSE);

				if (fire_hands(effect, dir, damage)) obvious = TRUE;

				break;
			}
		}

		    /* Hack -- haven't cancelled */
		    *cancel = FALSE;
	}

	/* Return result */
	return (obvious);

}





bool process_spell_flags(int spell, int level, bool *cancel)
{
	spell_type *s_ptr = &s_info[spell];

	bool obvious = FALSE;

	int lasts;

	int ench_toh = 0;
	int ench_tod = 0;
	int ench_toa = 0;

	/* Roll out the duration */
	if ((s_ptr->l_dice) && (s_ptr->l_side))
	{
		lasts = damroll(s_ptr->l_dice, s_ptr->l_side) + s_ptr->l_plus;
	}
	else
	{
		lasts = s_ptr->l_plus;
	}

	/* Process the flags that apply to equipment */
	if (s_ptr->flags1 & (SF1_IDENT_BONUS))
	{
		if (!ident_spell_bonus() && (*cancel)) return (TRUE);
		*cancel = FALSE;
		obvious = TRUE;
	}

	if (s_ptr->flags1 & (SF1_IDENT_SENSE))
	{
		if (!ident_spell_sense() && (*cancel)) return (TRUE);
		*cancel = FALSE;
		obvious = TRUE;
	}

	if (s_ptr->flags1 & (SF1_IDENT_RUNES))
	{
		if (!ident_spell_runes() && (*cancel)) return (TRUE);
		*cancel = FALSE;
		obvious = TRUE;
	}

	if (s_ptr->flags1 & (SF1_IDENT_VALUE))
	{
		if (!ident_spell_value() && (*cancel)) return (TRUE);
		*cancel = FALSE;
		obvious = TRUE;
	}

	if (s_ptr->flags1 & (SF1_IDENT))
	{
		if (!ident_spell() && (*cancel)) return (TRUE);
		*cancel = FALSE;
		obvious = TRUE;
	}

	if (s_ptr->flags1 & (SF1_IDENT_RUMOR))
	{
		if (!ident_spell_rumor() && (*cancel)) return (TRUE);
		*cancel = FALSE;
		obvious = TRUE;
	}

	if (s_ptr->flags1 & (SF1_IDENT_FULLY))
	{
		if (!identify_fully() && (*cancel)) return (TRUE);
		*cancel = FALSE;
		obvious = TRUE;
	}

	/* Process enchantment */
	if (s_ptr->flags1 & (SF1_ENCHANT_TOH))
	{
		if (s_ptr->flags1 & (SF1_ENCHANT_HIGH)) ench_toh = 3+rand_int(4);
		else ench_toh = 1;
	}
	if (s_ptr->flags1 & (SF1_ENCHANT_TOD))
	{
		if (s_ptr->flags1 & (SF1_ENCHANT_HIGH)) ench_tod = 3+rand_int(4);
		else ench_tod = 1;
	}
	if (s_ptr->flags1 & (SF1_ENCHANT_TOA))
	{
		if (s_ptr->flags1 & (SF1_ENCHANT_HIGH)) ench_toa = 3+rand_int(4);
		else ench_toa = 1;
	}

	/* Apply enchantment */
	if (ench_toh || ench_tod || ench_toa)
	{
		if (!(enchant_spell(ench_toh, ench_tod, ench_toa)) && (*cancel))
		{
			return (TRUE);
		}
		
		*cancel = FALSE;
		obvious = TRUE;
	}

	/* Process the flags */
	if ((s_ptr->flags1 & (SF1_DETECT_DOORS)) && (detect_doors())) obvious = TRUE;
	if ((s_ptr->flags1 & (SF1_DETECT_TRAPS)) && (detect_traps())) obvious = TRUE;
	if ((s_ptr->flags1 & (SF1_DETECT_STAIRS)) && (detect_stairs())) obvious = TRUE;
	if ((s_ptr->flags1 & (SF1_DETECT_WATER)) && (detect_water())) obvious = TRUE;
	if (s_ptr->flags1 & (SF1_DETECT_GOLD))
        {
                if (detect_treasure()) obvious = TRUE;
                if (detect_objects_gold()) obvious = TRUE;
        }
	if (s_ptr->flags1 & (SF1_DETECT_OBJECT))
        {
                if (detect_objects_buried()) obvious = TRUE;
                if (detect_objects_normal()) obvious = TRUE;
        }
	if ((s_ptr->flags1 & (SF1_DETECT_MAGIC)) && (detect_objects_magic())) obvious = TRUE;
	if ((s_ptr->flags1 & (SF1_DETECT_CURSE)) && (detect_objects_cursed())) obvious = TRUE;
	if ((s_ptr->flags1 & (SF1_DETECT_MONSTER)) && (detect_monsters_normal())) obvious = TRUE;
	if ((s_ptr->flags1 & (SF1_DETECT_EVIL)) && (detect_monsters_evil())) obvious = TRUE;
	if ((s_ptr->flags1 & (SF1_DETECT_LIFE)) && (detect_monsters_living())) obvious = TRUE;

	if (s_ptr->flags1 & (SF1_MAP_AREA))
	{
		map_area();
		obvious = TRUE;
	}

	if (s_ptr->flags1 & (SF1_WIZ_LITE))
	{
		wiz_lite();
		obvious = TRUE;
	}

	if (s_ptr->flags1 & (SF1_LITE_ROOM))
	{
		lite_room(p_ptr->py,p_ptr->px);
		obvious = TRUE;
	}

	if (s_ptr->flags1 & (SF1_DARK_ROOM))
	{
		unlite_room(p_ptr->py,p_ptr->px);
		obvious = TRUE;
	}

	if (s_ptr->flags1 & (SF1_FORGET))
	{
		lose_all_info();
		obvious = TRUE;
	}

	if (s_ptr->flags1 & (SF1_SELF_KNOW))
	{
		self_knowledge(TRUE);
		obvious = TRUE;
	}

	if (s_ptr->flags1 & (SF1_IDENT_PACK))
	{
		identify_pack();
		obvious = TRUE;
	}

	if (s_ptr->flags1 & (SF1_ACQUIREMENT))
	{
		acquirement(p_ptr->py,p_ptr->px, 1, TRUE);
		obvious = TRUE;
	}

	if (s_ptr->flags1 & (SF1_STAR_ACQUIREMENT))
	{
		acquirement(p_ptr->py,p_ptr->px, rand_int(2)+1, TRUE);
		obvious = TRUE;
	}


	/* SF2 - timed abilities and modifying level */
	if ((s_ptr->flags2 & (SF2_CURSE_WEAPON)) && (curse_weapon())) obvious = TRUE;
	if ((s_ptr->flags2 & (SF2_CURSE_ARMOR)) && (curse_armor())) obvious = TRUE;
	if ((s_ptr->flags2 & (SF2_INFRA)) && (set_tim_infra(p_ptr->tim_infra + lasts))) obvious = TRUE;
	if ((s_ptr->flags2 & (SF2_HERO)) && (set_hero(p_ptr->hero + lasts))) obvious = TRUE;
	if ((s_ptr->flags2 & (SF2_SHERO)) && (set_shero(p_ptr->shero + lasts))) obvious = TRUE;
	if ((s_ptr->flags2 & (SF2_BLESS)) && (set_blessed(p_ptr->blessed + lasts))) obvious = TRUE;
	if ((s_ptr->flags2 & (SF2_SHIELD)) && (set_shield(p_ptr->shield + lasts))) obvious = TRUE;
	if ((s_ptr->flags2 & (SF2_INVULN)) && (set_invuln(p_ptr->invuln + lasts))) obvious = TRUE;
	if ((s_ptr->flags2 & (SF2_SEE_INVIS)) && (set_tim_invis(p_ptr->tim_invis + lasts))) obvious = TRUE;
	if ((s_ptr->flags2 & (SF2_PROT_EVIL)) && (set_protevil(p_ptr->protevil + lasts + (level== 0 ? 3 * p_ptr->lev : 0)))) obvious = TRUE;
	if ((s_ptr->flags2 & (SF2_OPP_FIRE)) && (set_oppose_fire(p_ptr->oppose_fire + lasts))) obvious = TRUE;
	if ((s_ptr->flags2 & (SF2_OPP_COLD)) && (set_oppose_cold(p_ptr->oppose_cold + lasts))) obvious = TRUE;
	if ((s_ptr->flags2 & (SF2_OPP_ACID)) && (set_oppose_acid(p_ptr->oppose_acid + lasts))) obvious = TRUE;
	if ((s_ptr->flags2 & (SF2_OPP_ELEC)) && (set_oppose_elec(p_ptr->oppose_elec + lasts))) obvious = TRUE;
	if ((s_ptr->flags2 & (SF2_OPP_POIS)) && (set_oppose_pois(p_ptr->oppose_pois + lasts))) obvious = TRUE;

	if (s_ptr->flags2 & (SF2_AGGRAVATE))
	{
		aggravate_monsters(0);
		obvious = TRUE;
	}

	if (s_ptr->flags2 & (SF2_CREATE_STAIR))
	{
		stair_creation();
		obvious = TRUE;
	}

	if (s_ptr->flags2 & (SF2_TELE_LEVEL))
	{
		(void)teleport_player_level();
		obvious = TRUE;
	}

	if (s_ptr->flags2 & (SF2_ALTER_LEVEL))
	{
		p_ptr->leaving = TRUE;
		obvious = TRUE;
	}

	if (s_ptr->flags2 & (SF2_BANISHMENT))
	{
		(void)banishment();
		obvious = TRUE;
	}

	if (s_ptr->flags2 & (SF2_MASS_BANISHMENT))
	{
		mass_banishment();
		obvious = TRUE;
	}

	if (s_ptr->flags2 & (SF2_CUT))
	{
		if ((p_ptr->cur_flags2 & (TR2_RES_SHARD)) == 0)
		{
			if (set_cut(p_ptr->cut + lasts))
			{
				obvious = TRUE;

				/* Always notice */
				equip_not_flags(0x0L,TR2_RES_SHARD,0x0L,0x0L);
			}
		}
		else /* if (obvious) */
		{
			/* Always notice */
			equip_can_flags(0x0L,TR2_RES_SHARD,0x0L,0x0L);
		}
	}

	if (s_ptr->flags2 & (SF2_STUN))
	{
		if ((p_ptr->cur_flags2 & (TR2_RES_SOUND)) == 0)
		{
			if (set_stun(p_ptr->stun + lasts))
			{
				obvious = TRUE;

				/* Always notice */
				equip_not_flags(0x0L,TR2_RES_SOUND,0x0L,0x0L);
			}
		}
		else /* if (obvious)*/
		{
			/* Always notice */
			equip_can_flags(0x0L,TR2_RES_SHARD,0x0L,0x0L);
		}
	}

	if (s_ptr->flags2 & (SF2_POISON))
	{
		if (((p_ptr->cur_flags2 & (TR2_RES_POIS)) == 0) && !(p_ptr->oppose_pois) &&
			(p_ptr->cur_flags4 & (TR4_IM_POIS)) == 0)
		{
			if (set_poisoned(p_ptr->poisoned + lasts))
			{
				obvious = TRUE;

				/* Always notice */
				equip_not_flags(0x0L,TR2_RES_POIS,0x0L,TR4_IM_POIS);
			}
		}
		else if (!p_ptr->oppose_pois) /* && (obvious) */
		{
			/* Always notice */
			if (p_ptr->cur_flags4 & (TR4_IM_POIS)) equip_can_flags(0x0L,TR2_RES_POIS,0x0L,0x0L);

			/* Always notice */
			else equip_can_flags(0x0L,TR2_RES_POIS,0x0L,0x0L);
		}
	}

	if (s_ptr->flags2 & (SF2_BLIND))
	{
		if ((p_ptr->cur_flags2 & (TR2_RES_BLIND)) == 0)
		{
			if (set_blind(p_ptr->blind + lasts))
			{
				obvious = TRUE;

				/* Always notice */
				equip_not_flags(0x0L,TR2_RES_BLIND,0x0L,0x0L);
			}
		}
		else /* if (obvious)*/
		{
			/* Always notice */
			equip_can_flags(0x0L,TR2_RES_BLIND,0x0L,0x0L);
		}
	}

	if (s_ptr->flags2 & (SF2_FEAR))
	{
		if (((p_ptr->cur_flags2 & (TR2_RES_FEAR)) == 0) && (!p_ptr->hero) && (!p_ptr->shero))
		{
			if (set_afraid(p_ptr->afraid + lasts))
			{
				obvious = TRUE;

				/* Always notice */
				equip_not_flags(0x0L,TR2_RES_FEAR,0x0L,0x0L);
			}
		}
		else if ((!p_ptr->hero)&&(!p_ptr->shero)) /* && (obvious) */
		{
			/* Always notice */
			equip_can_flags(0x0L,TR2_RES_FEAR,0x0L,0x0L);
		}
	}

	if (s_ptr->flags2 & (SF2_CONFUSE))
	{
		if ((p_ptr->cur_flags2 & (TR2_RES_CONFU)) == 0)
		{
			if (set_confused(p_ptr->confused + lasts))
			{
				obvious = TRUE;

				/* Always notice */
				equip_not_flags(0x0L,TR2_RES_CONFU,0x0L,0x0L);
			}
		}
		else /* if (obvious) */
		{
			/* Always notice */
			equip_can_flags(0x0L,TR2_RES_CONFU,0x0L,0x0L);
		}
	}

	if (s_ptr->flags2 & (SF2_HALLUC))
	{
		if ((p_ptr->cur_flags2 & (TR2_RES_CHAOS)) == 0)
		{
			if (set_image(p_ptr->image + lasts))
			{
				obvious = TRUE;

				/* Always notice */
				equip_not_flags(0x0L,TR2_RES_CHAOS,0x0L,0x0L);
			}
		}
		else /* if (obvious) */
		{
			/* Always notice */
			equip_can_flags(0x0L,TR2_RES_CHAOS,0x0L,0x0L);
		}
	}

	if (s_ptr->flags2 & (SF2_PARALYZE))
	{
		if ((p_ptr->cur_flags3 & (TR3_FREE_ACT)) == 0)
		{
			if (set_paralyzed(p_ptr->paralyzed + lasts))
			{
				obvious = TRUE;

				/* Always notice */
				equip_not_flags(0x0L,0x0L,TR3_FREE_ACT,0x0L);
			}
		}
		else /* if (obvious) */
		{
			/* Always notice */
			equip_can_flags(0x0L,0x0L,TR3_FREE_ACT,0x0L);
		}
	}

	if (s_ptr->flags2 & (SF2_SLOW))
	{
		if ((p_ptr->cur_flags3 & (TR3_FREE_ACT)) == 0)
		{
			if (set_slow(p_ptr->slow + lasts))
			{
				obvious = TRUE;

				/* Always notice */
				equip_not_flags(0x0L,0x0L,TR3_FREE_ACT,0x0L);
			}
		}
		else /* if (obvious) */
		{
			/* Always notice */
			equip_can_flags(0x0L,0x0L,TR3_FREE_ACT,0x0L);
		}
	}

	if (s_ptr->flags2 & (SF2_HASTE))
	{
		if ((p_ptr->fast) && (set_fast(p_ptr->fast + rand_int(s_ptr->l_side/3)))) obvious = TRUE;
		else if (set_fast(p_ptr->fast + lasts + level)) obvious = TRUE;
	}

	if (s_ptr->flags2 & (SF2_RECALL))
	{
		set_recall();
		obvious = TRUE;
	}

	/* SF3 - duration specified, so temporary stat increase */
	if (lasts)
	{
		if ((s_ptr->flags3 & (SF3_INC_STR)) && (set_stat_inc_tim(lasts, A_STR))) obvious = TRUE;
		if ((s_ptr->flags3 & (SF3_INC_INT)) && (set_stat_inc_tim(lasts, A_INT))) obvious = TRUE;
		if ((s_ptr->flags3 & (SF3_INC_WIS)) && (set_stat_inc_tim(lasts, A_WIS))) obvious = TRUE;
		if ((s_ptr->flags3 & (SF3_INC_DEX)) && (set_stat_inc_tim(lasts, A_DEX))) obvious = TRUE;
		if ((s_ptr->flags3 & (SF3_INC_CON)) && (set_stat_inc_tim(lasts, A_CON))) obvious = TRUE;
		if ((s_ptr->flags3 & (SF3_INC_CHR)) && (set_stat_inc_tim(lasts, A_CHR))) obvious = TRUE;
	}
	/* SF3 - no duration specified, so permanent stat increase */
	else
	{
		if ((s_ptr->flags3 & (SF3_INC_STR)) && (do_inc_stat(A_STR))) obvious = TRUE;
		if ((s_ptr->flags3 & (SF3_INC_INT)) && (do_inc_stat(A_INT))) obvious = TRUE;
		if ((s_ptr->flags3 & (SF3_INC_WIS)) && (do_inc_stat(A_WIS))) obvious = TRUE;
		if ((s_ptr->flags3 & (SF3_INC_DEX)) && (do_inc_stat(A_DEX))) obvious = TRUE;
		if ((s_ptr->flags3 & (SF3_INC_CON)) && (do_inc_stat(A_CON))) obvious = TRUE;
		if ((s_ptr->flags3 & (SF3_INC_CHR)) && (do_inc_stat(A_CHR))) obvious = TRUE;
	}

	/* SF3 - healing self, and untimed improvements */
	if ((s_ptr->flags3 & (SF3_CURE_STR)) && (do_res_stat(A_STR))) obvious = TRUE;
	if ((s_ptr->flags3 & (SF3_CURE_INT)) && (do_res_stat(A_INT))) obvious = TRUE;
	if ((s_ptr->flags3 & (SF3_CURE_WIS)) && (do_res_stat(A_WIS))) obvious = TRUE;
	if ((s_ptr->flags3 & (SF3_CURE_DEX))  && (do_res_stat(A_DEX))) obvious = TRUE;
	if ((s_ptr->flags3 & (SF3_CURE_CON))  && (do_res_stat(A_CON))) obvious = TRUE;
	if ((s_ptr->flags3 & (SF3_CURE_CHR))  && (do_res_stat(A_CHR))) obvious = TRUE;
	if ((s_ptr->flags3 & (SF3_CURE_EXP)) && (restore_level())) obvious = TRUE;
	if ((s_ptr->flags3 & (SF3_SLOW_CURSE)) && (remove_curse())) obvious = TRUE;
	if ((s_ptr->flags3 & (SF3_CURE_CURSE)) && (remove_all_curse())) obvious = TRUE;
	if ((s_ptr->flags3 & (SF3_SLOW_POIS)) && (set_poisoned(p_ptr->poisoned / 2))) obvious = TRUE;
	if ((s_ptr->flags3 & (SF3_CURE_POIS)) && (set_poisoned(0))) obvious = TRUE;
	if ((s_ptr->flags3 & (SF3_SLOW_CUTS)) && (set_cut(p_ptr->cut / 2))) obvious = TRUE;
	if ((s_ptr->flags3 & (SF3_CURE_CUTS)) && (set_cut(0))) obvious = TRUE;
	if ((s_ptr->flags3 & (SF3_CURE_STUN)) && (set_stun(0))) obvious = TRUE;
	if ((s_ptr->flags3 & (SF3_CURE_CONF)) && (set_confused(0))) obvious = TRUE;
	if ((s_ptr->flags3 & (SF3_CURE_FOOD)) && (set_food(PY_FOOD_MAX -1))) obvious = TRUE;
	if ((s_ptr->flags3 & (SF3_CURE_FEAR)) && (set_afraid(0))) obvious = TRUE;
	if ((s_ptr->flags3 & (SF3_CURE_BLIND)) && (set_blind(0))) obvious = TRUE;
	if ((s_ptr->flags3 & (SF3_CURE_IMAGE)) && (set_image(0))) obvious = TRUE;


	if (s_ptr->flags3 & (SF3_DEC_EXP))
	{
		if (((p_ptr->cur_flags3 & (TR3_HOLD_LIFE)) == 0) && !p_ptr->blessed && (p_ptr->exp > 0))
		{
			lose_exp(p_ptr->exp / 4);
			obvious = TRUE;

			/* Always notice */
			equip_not_flags(0x0L,0x0L,TR3_HOLD_LIFE,0x0L);
		}
		else if ((p_ptr->cur_flags3 & (TR3_HOLD_LIFE)) != 0)
		{
			/* Always notice */
			equip_can_flags(0x0L,0x0L,TR3_HOLD_LIFE,0x0L);
		}
	}

	

	if (s_ptr->flags3 & (SF3_DEC_FOOD))
	{
		(void)set_food(PY_FOOD_STARVE - 1);
		obvious = TRUE;
	}

	if (s_ptr->flags3 & (SF3_INC_EXP))
	{
		s32b ee = (p_ptr->exp / 2) + 10;
		if (ee > 100000L) ee = 100000L;
		gain_exp(ee);
		obvious = TRUE;
	}

	if (s_ptr->flags1 || s_ptr->flags2 || s_ptr->flags3) *cancel = FALSE;

	return (obvious);

}




bool process_spell_types(int spell, int level, bool *cancel)
{

	spell_type *s_ptr = &s_info[spell];

	bool obvious = FALSE;

	int lasts;

	int dir;

	/* Roll out the duration */
	if ((s_ptr->l_dice) && (s_ptr->l_side))
	{
		lasts = damroll(s_ptr->l_dice, s_ptr->l_side) + s_ptr->l_plus;
	}
	else
	{
		lasts = s_ptr->l_plus;
	}

	/* Has another effect? */
	if (s_ptr->type)
	{
		/* Process the effects */
		switch(s_ptr->type)
		{
			case SPELL_RECHARGE:
			{
				if ((!recharge(s_ptr->param)) && (*cancel)) return (TRUE);
				*cancel = FALSE;
				obvious = TRUE;
				break;
			}
			case SPELL_IDENT_TVAL:
			{
				if ((!ident_spell_tval(s_ptr->param)) && (*cancel)) return (TRUE);
				*cancel = FALSE;
				obvious = TRUE;
				break;
			}
			case SPELL_ENCHANT_TVAL:
			{
				enchant_item(s_ptr->param,level);
				*cancel = FALSE;
				obvious = TRUE;
				break;
			}
			case SPELL_BRAND_WEAPON:
			{
				/* Only brand weapons */
				item_tester_hook = item_tester_hook_weapon_strict;

				if (!brand_item(s_ptr->param, "glows brightly.") && (*cancel)) return (TRUE);
				*cancel = FALSE;
				obvious = TRUE;
				break;
			}
			case SPELL_BRAND_ARMOR:
			{
				/* Only brand weapons */
				item_tester_hook = item_tester_hook_armour;

				if (!brand_item(s_ptr->param, "glows brightly.") && (*cancel)) return (TRUE);
				*cancel = FALSE;
				obvious = TRUE;
				break;
			}
			case SPELL_BRAND_ITEM:
			{
				if (!brand_item(s_ptr->param, "glows brightly.") && (*cancel)) return (TRUE);
				*cancel = FALSE;
				obvious = TRUE;
				break;
			}
			case SPELL_BRAND_AMMO:
			{
				/* Only brand ammo */
				item_tester_hook = item_tester_hook_ammo;

				if (!brand_item(s_ptr->param, "glows brightly.") && (*cancel)) return (TRUE);
				*cancel = FALSE;
				obvious = TRUE;
				break;
			}
			case SPELL_WARD_GLYPH:
			{
				warding_glyph();
				*cancel = FALSE;
				obvious = TRUE;
				break;
			}
			case SPELL_WARD_TRAP:
			{
				if ((!get_rep_dir(&dir)) && (*cancel)) return (FALSE);
				else warding_trap(s_ptr->param,dir);
				*cancel = FALSE;
				obvious = TRUE;
				break;
			}
			case SPELL_SUMMON:
			{
				if (summon_specific(p_ptr->py, p_ptr->px, p_ptr->depth+5, s_ptr->param)) obvious = TRUE;
				*cancel = FALSE;
				break;
			}
			case SPELL_SUMMON_RACE:
			{
				if (summon_specific_one(p_ptr->py, p_ptr->px, s_ptr->param, FALSE)) obvious = TRUE;
				*cancel = FALSE;
				break;
			}
			case SPELL_CREATE_RACE:
			{
				if (summon_specific_one(p_ptr->py, p_ptr->px, s_ptr->param, TRUE)) obvious = TRUE;
				*cancel = FALSE;
				break;
			}
			case SPELL_CREATE_KIND:
			{
				create_gold();
				*cancel = FALSE;
				obvious = TRUE;
				break;
			}
			case SPELL_EARTHQUAKE:
			{
				earthquake(p_ptr->py,p_ptr->px, s_ptr->param);
				*cancel = FALSE;
				obvious = TRUE;
				break;
			}
			case SPELL_DESTRUCTION:
			{
				destroy_area(p_ptr->py,p_ptr->px, s_ptr->param, TRUE);
				*cancel = FALSE;
				obvious = TRUE;
				break;
			}
			case SPELL_CURE_DISEASE:
			{
				int v = (1 << s_ptr->param);

				if ((p_ptr->disease & v) != 0)
				{
					obvious = TRUE;
					*cancel = FALSE;
					p_ptr->disease &= ~(v);

					/* Hack -- cured all symptoms or cured all origins of disease */
					if ( ((s_ptr->param >= DISEASE_TYPES_HEAVY) && (p_ptr->disease < (1 << DISEASE_TYPES_HEAVY)))
						|| ((p_ptr->disease & ((1 << DISEASE_TYPES_HEAVY) -1)) == 0) )
					{
						p_ptr->disease = 0;
					}
				}

				break;
			}
			default:
			{
				wield_spell(s_ptr->type,s_ptr->param,lasts);
				*cancel = FALSE;
				obvious = TRUE;
				break;
			}
	
		}

	}

	/* Return result */
	return (obvious);
}

/*
 * Hack -- we process swallowed objects a little differently.
 */
bool process_spell_eaten(int spell, int level, bool *cancel)
{
	spell_type *s_ptr = &s_info[spell];

	bool obvious = FALSE;

	int ap_cnt;

	/* Inform the player */
	if (strlen(s_text + s_info[spell].text))
	{
		msg_format("%s",s_text + s_info[spell].text);
		obvious = TRUE;
	}

	/* Apply flags */
	if (process_spell_flags(spell,level,cancel)) obvious = TRUE;

	/* Scan through all four blows */
	for (ap_cnt = 0; ap_cnt < 4; ap_cnt++)
	{
		int damage = 0;

		int dir;

		/* Extract the attack infomation */
		int effect = s_ptr->blow[ap_cnt].effect;
		int method = s_ptr->blow[ap_cnt].method;
		int d_dice = s_ptr->blow[ap_cnt].d_dice;
		int d_side = s_ptr->blow[ap_cnt].d_side;
		int d_plus = s_ptr->blow[ap_cnt].d_plus;

		/* Hack -- no more attacks */
		if (!method) break;

		/* Mega hack -- dispel evil/undead objects */
		if ((!d_side) && (!level))
		{
			d_plus += 25 * d_dice;
		}
		/* Hack -- use level as modifier */
		else if (!d_side)
		{
			d_plus += level * d_dice;
		}

		/* Roll out the damage */
		if ((d_dice) && (d_side))
		{
			damage = damroll(d_dice, d_side) + d_plus;
		}
		else
		{
			damage = d_plus;
		}

		/* Hack -- breath weapons act like a normal spell */
		if ((method == RBM_BREATH) && (get_aim_dir(&dir)))
		{
			if (fire_ball(effect, dir, MIN(p_ptr->chp,damage), 2)) obvious = TRUE;
		}
		/* Hack -- spitting acts like a normal spell */
		else if ((method == RBM_SPIT) && (get_rep_dir(&dir)))
		{
			if (fire_hands(effect, dir, damage)) obvious = TRUE;
		}
		/* Hack -- vomit in a random direction */
		else if (method == RBM_VOMIT)
		{
			/* Random direction */
			dir = ddd[rand_int(8)];

			if (fire_hands(effect, dir, damage)) obvious = TRUE;
		}
		else
		{
			if (project_p(-2,0,p_ptr->py,p_ptr->px,damage, effect)) obvious = TRUE;
		}
	}

	return (obvious);

}




bool process_spell(int spell, int level, bool *cancel)
{
	bool obvious = FALSE;

	/* Inform the player */
	if (strlen(s_text + s_info[spell].text))
	{
		msg_format("%s",s_text + s_info[spell].text);
		obvious = TRUE;
	}

	/* Note the order is important -- because of the impact of blinding a character on their subsequent
		ability to see spell blows that affect themselves */
	if (process_spell_blows(spell,level,cancel)) obvious = TRUE;
	if (process_spell_flags(spell,level,cancel)) obvious = TRUE;
	if (process_spell_types(spell,level,cancel)) obvious = TRUE;

	/* Return result */
	return (obvious);
}

