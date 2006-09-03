/* File: cmd1.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 *
 * UnAngband (c) 2001-6 Andrew Doull. Modifications to the Angband 2.9.6
 * source code are released under the Gnu Public License. See www.fsf.org
 * for current GPL license details. Addition permission granted to
 * incorporate modifications in all Angband variants as defined in the
 * Angband variants FAQ. See rec.games.roguelike.angband for FAQ.
 */

#include "angband.h"



/*
 * Determine if the player "hits" a monster (normal combat).
 *
 * Note -- Always miss 5%, always hit 5%, otherwise random.
 */
bool test_hit_fire(int chance, int ac, int vis)
{
	int k;

	/* Percentile dice */
	k = rand_int(100);

	/* Hack -- Instant miss or hit */
	if (k < 10) return (k < 5);

	/* Invisible monsters are harder to hit */
	if (!vis) chance = chance / 2;

	/* Power competes against armor */
	if ((chance > 0) && (rand_int(chance) >= (ac * 3 / 4))) return (TRUE);

	/* Assume miss */
	return (FALSE);
}



/*
 * Determine if the player "hits" a monster (normal combat).
 *
 * Note -- Always miss 5%, always hit 5%, otherwise random.
 */
bool test_hit_norm(int chance, int ac, int vis)
{
	int k;

	/* Percentile dice */
	k = rand_int(100);

	/* Hack -- Instant miss or hit */
	if (k < 10) return (k < 5);

	/* Penalize invisible targets */
	if (!vis) chance = chance / 2;

	/* Power competes against armor */
	if ((chance > 0) && (rand_int(chance) >= (ac * 3 / 4))) return (TRUE);

	/* Assume miss */
	return (FALSE);
}



/*
 * Critical hits (from objects thrown by player)
 * Factor in item weight, total plusses, and player level.
 */
sint critical_shot(int weight, int plus, int dam)
{
	int i, k, crit = 0;

	/* Extract "shot" power */
	i = (weight + ((p_ptr->to_h + plus) * 4) + (p_ptr->lev * 2));

	/* Critical hit */
	if (randint(5000) <= i)
	{
		k = weight + randint(500) + plus * 5;

		if (k < 500)
		{
			crit = dam + 5;
		}
		else if (k < 1000)
		{
			crit = ((dam * 3) / 2) + 10;
		}
		else
		{
			crit = dam * 2 + 15;
		}
	}

	return (crit);
}



/*
 * Critical hits (by player)
 *
 * Factor in weapon weight, total plusses, player level.
 */
sint critical_norm(int weight, int plus, int dam)
{
	int i, k, crit = 0;

	/* Extract "blow" power */
	i = (weight + ((p_ptr->to_h + plus) * 5) + (p_ptr->lev * 3));

	/* Chance */
	if (randint(5000) <= i)
	{
		k = weight + randint(650) + plus * 5;

		if (k < 400)
		{
			crit = dam + 5;
		}
		else if (k < 700)
		{
			crit = ((dam * 3) / 2) + 10;
		}
		else if (k < 900)
		{
			crit = dam * 2 + 15;
		}
		else if (k < 1300)
		{
			crit = ((dam * 5) / 2) + 20;
		}
		else
		{
			crit = dam * 3 + 25;
		}
	}

	return (crit);
}



/*
 * Extract the "total damage" from a given object hitting a given monster.
 *
 * Note that most brands and slays are x3, except Slay Animal (x2),
 * Slay Evil (x2), and Kill dragon (x5).
 * Brand lite and brand dark are also (x2).
 *
 * Acid damage is only (x2) against armoured opponents.
 */
sint tot_dam_aux(object_type *o_ptr, int tdam, const monster_type *m_ptr)
{
	int mult = 1;

	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	monster_lore *l_ptr = &l_list[m_ptr->r_idx];

	u32b f1, f2, f3, f4;

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4);

	/* Some "weapons" and "ammo" do extra damage */
	switch (o_ptr->tval)
	{
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_DIGGING:
		case TV_STAFF:
		case TV_GLOVES:
		{
			/* Slay Animal */
			if ((f1 & (TR1_SLAY_NATURAL)) &&
			    (r_ptr->flags3 & (RF3_ANIMAL | RF3_PLANT | RF3_INSECT)))
			{
				if (m_ptr->ml)
				{
					object_can_flags(o_ptr,TR1_SLAY_NATURAL,0x0L,0x0L,0x0L);
					if (r_ptr->flags3 & RF3_ANIMAL) l_ptr->flags3 |= (RF3_ANIMAL);
					if (r_ptr->flags3 & RF3_PLANT) l_ptr->flags3 |= (RF3_PLANT);
					if (r_ptr->flags3 & RF3_INSECT) l_ptr->flags3 |= (RF3_INSECT);
				}

				if (mult < 4) mult = 4;
			}
			else if ((l_ptr->flags3 & (RF3_ANIMAL | RF3_PLANT | RF3_INSECT)) && (m_ptr->ml))
			{
				object_not_flags(o_ptr,TR1_SLAY_NATURAL,0x0L,0x0L,0x0L);
			}

			/* Slay Undead */
			if ((f1 & (TR1_SLAY_UNDEAD)) &&
			    (r_ptr->flags3 & (RF3_UNDEAD)))
			{
				if (m_ptr->ml)
				{
					object_can_flags(o_ptr,TR1_SLAY_UNDEAD,0x0L,0x0L,0x0L);
					l_ptr->flags3 |= (RF3_UNDEAD);
				}

				if (mult < 3) mult = 3;
			}
			else if ((r_ptr->flags3 & (RF3_UNDEAD)) && (m_ptr->ml))
			{
				object_not_flags(o_ptr,TR1_SLAY_UNDEAD,0x0L,0x0L,0x0L);
			}

			/* Slay Demon */
			if ((f1 & (TR1_SLAY_DEMON)) &&
			    (r_ptr->flags3 & (RF3_DEMON)))
			{
				if (m_ptr->ml)
				{
					object_can_flags(o_ptr,TR1_SLAY_DEMON,0x0L,0x0L,0x0L);
					l_ptr->flags3 |= (RF3_DEMON);
				}
				if (mult < 3) mult = 3;
			}
			else if ((l_ptr->flags3 & (RF3_DEMON)) && (m_ptr->ml))
			{
				object_not_flags(o_ptr,TR1_SLAY_DEMON,0x0L,0x0L,0x0L);
			}

			/* Slay Dragon */
			if ((f1 & (TR1_SLAY_DRAGON)) &&
			    (r_ptr->flags3 & (RF3_DRAGON)))
			{
				if (m_ptr->ml)
				{
					object_can_flags(o_ptr,TR1_SLAY_DRAGON,0x0L,0x0L,0x0L);
					l_ptr->flags3 |= (RF3_DRAGON);
				}

				if (mult < 3) mult = 3;
			}
			else if ((l_ptr->flags3 & (RF3_DRAGON)) && (m_ptr->ml))
			{
				object_not_flags(o_ptr,TR1_SLAY_DRAGON,0x0L,0x0L,0x0L);
			}

			/* Slay Orc */
			if ((f1 & (TR1_SLAY_ORC)) &&
			    (r_ptr->flags3 & (RF3_ORC)))
			{
				if (m_ptr->ml)
				{
					object_can_flags(o_ptr,TR1_SLAY_ORC,0x0L,0x0L,0x0L);
					l_ptr->flags3 |= (RF3_ORC);
				}

				if (mult < 4) mult = 4;
			}
			else if ((l_ptr->flags3 & (RF3_ORC)) && (m_ptr->ml))
			{
				object_not_flags(o_ptr,TR1_SLAY_ORC,0x0L,0x0L,0x0L);
			}

			/* Slay Troll */
			if ((f1 & (TR1_SLAY_TROLL)) &&
			    (r_ptr->flags3 & (RF3_TROLL)))
			{
				if (m_ptr->ml)
				{
					object_can_flags(o_ptr,TR1_SLAY_TROLL,0x0L,0x0L,0x0L);
					l_ptr->flags3 |= (RF3_TROLL);
				}

				if (mult < 4) mult = 4;
			}
			else if ((l_ptr->flags3 & (RF3_TROLL)) && (m_ptr->ml))
			{
				object_not_flags(o_ptr,TR1_SLAY_TROLL,0x0L,0x0L,0x0L);
			}

			/* Slay Giant */
			if ((f1 & (TR1_SLAY_GIANT)) &&
			    (r_ptr->flags3 & (RF3_GIANT)))
			{
				if (m_ptr->ml)
				{
					object_can_flags(o_ptr,TR1_SLAY_GIANT,0x0L,0x0L,0x0L);
					l_ptr->flags3 |= (RF3_GIANT);
				}

				if (mult < 4) mult = 4;
			}
			else if ((l_ptr->flags3 & (RF3_GIANT)) && (m_ptr->ml))
			{
				object_not_flags(o_ptr,TR1_SLAY_GIANT,0x0L,0x0L,0x0L);
			}

			/* Execute Dragon */
			if ((f1 & (TR1_KILL_DRAGON)) &&
			    (r_ptr->flags3 & (RF3_DRAGON)))
			{
				if (m_ptr->ml)
				{
					if (rand_int(100)<tdam)
					{
						object_can_flags(o_ptr,TR1_KILL_DRAGON,0x0L,0x0L,0x0L);
					}
					l_ptr->flags3 |= (RF3_DRAGON);
				}

				if (mult < 5) mult = 5;
			}
			else if ((l_ptr->flags3 & (RF3_DRAGON)) && (m_ptr->ml))
			{
				if (rand_int(100)<tdam*3) object_not_flags(o_ptr,TR1_KILL_DRAGON,0x0L,0x0L,0x0L);
			}

			/* Execute demon */
			if ((f1 & (TR1_KILL_DEMON)) &&
			    (r_ptr->flags3 & (RF3_DEMON)))
			{
				if (m_ptr->ml)
				{
					if (rand_int(100)<tdam)
					{
						object_can_flags(o_ptr,TR1_KILL_DEMON,0x0L,0x0L,0x0L);
					}
					l_ptr->flags3 |= (RF3_DEMON);
				}

				if (mult < 5) mult = 5;
			}
			else if ((l_ptr->flags3 & (RF3_DEMON)) && (m_ptr->ml))
			{
				if (rand_int(100)<tdam*3) object_not_flags(o_ptr,TR1_KILL_DEMON,0x0L,0x0L,0x0L);
			}

			/* Execute undead */
			if ((f1 & (TR1_KILL_UNDEAD)) &&
			    (r_ptr->flags3 & (RF3_UNDEAD)))
			{
				if (m_ptr->ml)
				{
					if (rand_int(100)<tdam)
					{
						object_can_flags(o_ptr,TR1_KILL_UNDEAD,0x0L,0x0L,0x0L);
					}
					l_ptr->flags3 |= (RF3_UNDEAD);
				}

				if (mult < 5) mult = 5;
			}
			else if ((l_ptr->flags3 & (RF3_UNDEAD)) && (m_ptr->ml))
			{
				if (rand_int(100)<tdam*3) object_not_flags(o_ptr,TR1_KILL_UNDEAD,0x0L,0x0L,0x0L);
			}

			/* Brand (Acid) */
			if (f1 & (TR1_BRAND_ACID))
			{
				/* Notice immunity */
				if (r_ptr->flags3 & (RF3_IM_ACID))
				{
					if (m_ptr->ml)
					{
						object_can_flags(o_ptr,TR1_BRAND_ACID,0x0L,0x0L,0x0L);
						l_ptr->flags3 |= (RF3_IM_ACID);
					}
				}

				/* Otherwise, take the damage */
				else
				{
					if ((rand_int(100)<tdam*3) && (m_ptr->ml)) object_can_flags(o_ptr,TR1_BRAND_ACID,0x0L,0x0L,0x0L);

					/* Armour partially protects the monster */
					if (r_ptr->flags2 & (RF2_ARMOR))
					{
						if (m_ptr->ml)
						{
							object_can_flags(o_ptr,TR1_BRAND_ACID,0x0L,0x0L,0x0L);
							l_ptr->flags2 |= (RF2_ARMOR);
						}

						if (m_ptr->oppose_elem)/* Do nothing */;
						else if (mult < 2) mult = 2;
					}

					/* Water decreases damage */
					if (f_info[cave_feat[m_ptr->fy][m_ptr->fx]].flags2 & (FF2_WATER))
					{
						if (m_ptr->oppose_elem)/* Do nothing */;
						else if (mult < 2) mult = 2;
					}
					else if ((m_ptr->oppose_elem) && (mult < 2)) mult = 2;
					else if (mult < 3) mult = 3;
				}
			}
			else if ((m_ptr->ml) && ((l_ptr->flags3 & (RF3_IM_ACID)) || (rand_int(100)<tdam*3)))
			{
				object_not_flags(o_ptr,TR1_BRAND_ACID,0x0L,0x0L,0x0L);
			}

			/* Brand (Elec) */
			if (f1 & (TR1_BRAND_ELEC))
			{
				/* Notice immunity */
				if (r_ptr->flags3 & (RF3_IM_ELEC))
				{
					if (m_ptr->ml)
					{
						object_can_flags(o_ptr,TR1_BRAND_ELEC,0x0L,0x0L,0x0L);
						l_ptr->flags3 |= (RF3_IM_ELEC);
					}
				}

				/* Otherwise, take the damage */
				else
				{
					if ((rand_int(100)<tdam*3) && (m_ptr->ml)) object_can_flags(o_ptr,TR1_BRAND_ELEC,0x0L,0x0L,0x0L);

					/* Water increases damage */
					if (f_info[cave_feat[m_ptr->fy][m_ptr->fx]].flags2 & (FF2_WATER))
					{
						if ((m_ptr->oppose_elem) && (mult < 3)) mult = 3;
						else if (mult < 4) mult = 4;
					}
					else if ((m_ptr->oppose_elem) && (mult < 2)) mult = 2;
					else if (mult < 3) mult = 3;
				}
			}
			else if ((m_ptr->ml) && ((l_ptr->flags3 & (RF3_IM_ELEC)) || (rand_int(100)<tdam*3)))
			{
				object_not_flags(o_ptr,TR1_BRAND_ELEC,0x0L,0x0L,0x0L);
			}

			/* Brand (Fire) */
			if (f1 & (TR1_BRAND_FIRE))
			{
				/* Notice immunity */
				if (r_ptr->flags3 & (RF3_IM_FIRE))
				{
					if (m_ptr->ml)
					{
						object_can_flags(o_ptr,TR1_BRAND_FIRE,0x0L,0x0L,0x0L);
						l_ptr->flags3 |= (RF3_IM_FIRE);
					}
				}

				/* Otherwise, take the damage */
				else
				{
					if ((rand_int(100)<tdam*3) && (m_ptr->ml)) object_can_flags(o_ptr,TR1_BRAND_FIRE,0x0L,0x0L,0x0L);

					/* Water decreases damage */
					if (f_info[cave_feat[m_ptr->fy][m_ptr->fx]].flags2 & (FF2_WATER))
					{
						if (m_ptr->oppose_elem) ;
						else if (mult < 2) mult = 2;
					}
					else if ((m_ptr->oppose_elem) && (mult < 2)) mult = 2;
					else if (mult < 3) mult = 3;
				}
			}
			else if ((m_ptr->ml) && ((l_ptr->flags3 & (RF3_IM_FIRE)) || (rand_int(100)<tdam*3)))
			{
				object_not_flags(o_ptr,TR1_BRAND_FIRE,0x0L,0x0L,0x0L);
			}

			/* Brand (Cold) */
			if (f1 & (TR1_BRAND_COLD))
			{
				/* Notice immunity */
				if (r_ptr->flags3 & (RF3_IM_COLD))
				{
					if (m_ptr->ml)
					{
						object_can_flags(o_ptr,TR1_BRAND_COLD,0x0L,0x0L,0x0L);
						l_ptr->flags3 |= (RF3_IM_COLD);
					}
				}

				/* Otherwise, take the damage */
				else
				{
					if ((rand_int(100)<tdam*3) && (m_ptr->ml)) object_can_flags(o_ptr,TR1_BRAND_COLD,0x0L,0x0L,0x0L);

					/* Water increases damage */
					if (f_info[cave_feat[m_ptr->fy][m_ptr->fx]].flags2 & (FF2_WATER))
					{
						if ((m_ptr->oppose_elem) && (mult < 3)) mult = 3;
						else if (mult < 4) mult = 4;
					}
					else if ((m_ptr->oppose_elem) && (mult < 2)) mult = 2;
					else if (mult < 3) mult = 3;
				}
			}
			else if ((m_ptr->ml) && ((l_ptr->flags3 & (RF3_IM_COLD)) || (rand_int(100)<tdam*3)))
			{
				object_not_flags(o_ptr,TR1_BRAND_COLD,0x0L,0x0L,0x0L);
			}

			/* Brand (Poison) */
			if (f1 & (TR1_BRAND_POIS))
			{
				/* Notice immunity */
				if (r_ptr->flags3 & (RF3_IM_POIS))
				{
					if (m_ptr->ml)
					{
						object_can_flags(o_ptr,TR1_BRAND_POIS,0x0L,0x0L,0x0L);
						l_ptr->flags3 |= (RF3_IM_POIS);
					}
				}

				/* Otherwise, take the damage */
				else
				{
					if ((rand_int(100)<tdam*3) && (m_ptr->ml)) object_can_flags(o_ptr,TR1_BRAND_POIS,0x0L,0x0L,0x0L);

					if (mult < 3) mult = 3;
				}
			}
			else if ((m_ptr->ml) && ((l_ptr->flags3 & (RF3_IM_POIS)) || (rand_int(100)<tdam*3)))
			{
				object_not_flags(o_ptr,TR1_BRAND_POIS,0x0L,0x0L,0x0L);
			}

			/* Brand Holy */
			if ((f1 & (TR1_BRAND_HOLY)) &&
			    (r_ptr->flags3 & (RF3_EVIL)))
			{
				if (m_ptr->ml)
				{
					object_can_flags(o_ptr,TR1_BRAND_HOLY,0x0L,0x0L,0x0L);
					l_ptr->flags3 |= (RF3_EVIL);
				}

				if (mult < 3) mult = 3;
			}
			else if ((l_ptr->flags3 & (RF3_EVIL)) && (m_ptr->ml))
			{
				object_not_flags(o_ptr,TR1_BRAND_HOLY,0x0L,0x0L,0x0L);
			}

			/* Brand lite */
			if ((f4 & (TR4_BRAND_LITE)) &&
			    (r_ptr->flags3 & (RF3_HURT_LITE)))
			{
				if (m_ptr->ml)
				{
					object_can_flags(o_ptr,0x0L,0x0L,0x0L,TR4_HURT_LITE);
					l_ptr->flags3 |= (RF3_HURT_LITE);
				}

				if (mult < 3) mult = 3;
			}
			else if ((l_ptr->flags3 & (RF3_HURT_LITE)) && (m_ptr->ml))
			{
				object_not_flags(o_ptr,0x0L,0x0L,0x0L,TR4_HURT_LITE);
			}

			/* Brand (Dark) */
			if (f4 & (TR4_BRAND_DARK))
			{
				/* Notice immunity */
				if (r_ptr->flags9 & (RF9_RES_DARK))
				{
					if (m_ptr->ml)
					{
						object_can_flags(o_ptr,0x0L,0x0L,0x0L,TR4_BRAND_DARK);
						l_ptr->flags9 |= (RF9_RES_DARK);
					}
				}

				/* Otherwise, take the damage */
				else
				{
					if ((rand_int(100)<tdam*3) && (m_ptr->ml)) object_can_flags(o_ptr,0x0L,0x0L,0x0L,TR4_BRAND_DARK);

					if (mult < 3) mult = 3;
				}
			}
			else if ((m_ptr->ml) && ((l_ptr->flags9 & (RF9_RES_DARK)) || (rand_int(100)<tdam*3)))
			{
				object_not_flags(o_ptr,0x0L,0x0L,0x0L,TR4_BRAND_DARK);
			}

			/* Slay man */
			if ((f4 & (TR4_SLAY_MAN)) &&
			    (r_ptr->flags9 & (RF9_MAN)))
			{
				if (m_ptr->ml)
				{
					object_can_flags(o_ptr,0x0L,0x0L,0x0L,TR4_SLAY_MAN);
					l_ptr->flags9 |= (RF9_MAN);
				}

				if (mult < 4) mult = 4;
			}
			else if ((l_ptr->flags9 & (RF9_MAN)) && (m_ptr->ml))
			{
				object_not_flags(o_ptr,0x0L,0x0L,0x0L,TR4_SLAY_MAN);
			}

			/* Slay elf - includes Maia */
			if ((f4 & (TR4_SLAY_ELF)) &&
			    (r_ptr->flags9 & (RF9_ELF)))
			{
				if (m_ptr->ml)
				{
					object_can_flags(o_ptr,0x0L,0x0L,0x0L,TR4_SLAY_ELF);
					l_ptr->flags9 |= (RF9_ELF);
				}

				if (mult < 4) mult = 4;
			}
			else if ((l_ptr->flags9 & (RF9_ELF)) && (m_ptr->ml))
			{
				object_not_flags(o_ptr,0x0L,0x0L,0x0L,TR4_SLAY_ELF);
			}

			/* Slay dwarf */
			if ((f4 & (TR4_SLAY_DWARF)) &&
			    (r_ptr->flags9 & (RF9_DWARF)))
			{
				if (m_ptr->ml)
				{
					object_can_flags(o_ptr,0x0L,0x0L,0x0L,TR4_SLAY_DWARF);
					l_ptr->flags9 |= (RF9_DWARF);
				}

				if (mult < 4) mult = 4;
			}
			else if ((l_ptr->flags9 & (RF9_ELF)) && (m_ptr->ml))
			{
				object_not_flags(o_ptr,0x0L,0x0L,0x0L,TR4_SLAY_DWARF);
			}

			break;

		}
	}

	/* Hack -- if dice roll less than three, treat as three for adding multiplier only */
	if (tdam < 3)
	{
		return (tdam + 3 * (mult - 1));
	}

	/* Return the total damage */
	return (tdam * mult);
}


/*
 * Find a secret at the specified location and change it according to
 * the state.
 */
void find_secret(int y, int x)
{
	feature_type *f_ptr;

	cptr text;

	/* Get feature */
	f_ptr = &f_info[cave_feat[y][x]];

	/* Get the feature description */
	text = (f_text + f_ptr->text);

	if (strlen(text))
	{
		/* You have found something */
		msg_format("%s",text);
	}

	/* Change the location */
	cave_alter_feat(y,x,FS_SECRET);

	/* Update the visuals */
	p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Disturb */
	disturb(0, 0);
}


/*
 * Search for hidden things
 */
void search(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y, x, chance;

	int range = 1;

	/* Start with base search ability, modified by depth */
	chance = 10 + (2 * p_ptr->skill_srh) - p_ptr->depth;

	/* Penalize various conditions */
	if (p_ptr->blind || no_lite()) chance = chance / 10;
	if (p_ptr->confused || p_ptr->image) chance = chance / 10;

	/* Increase searching range sometimes */
	if (chance >= rand_range(40,  70)) range++;
	if (chance >= rand_range(80, 140)) range++;

	/* Search all grids in range and in los */
	for (y = py - range; y <= py + range; y++)
	{
		for (x = px - range; x <= px + range; x++)
		{
			/* Get adjusted chance */
			int chance2 = chance - ((range-1) * 40);

			/* Require that grid be fully in bounds and in LOS */
			if (!in_bounds_fully(y, x)) continue;
			if (!generic_los(py, px, y, x, CAVE_XLOS)) continue;

			/* Sometimes, notice things */
			if (rand_int(100) < chance2)
			{
				if (f_info[cave_feat[y][x]].flags1 & (FF1_SECRET))
				{
					find_secret(y,x);
				}
			}
		}
	}
}


/*
 * Determine if the object can be picked up, and has "=g" in its inscription.
 * If "=g" is followed by a number, pick up if the player has less than this number of similar objects in their inventory.
 */
static bool auto_pickup_okay(const object_type *o_ptr)
{
	cptr s;

	/* It can't be carried */
	if (!inven_carry_okay(o_ptr)) return (FALSE);

	/* No inscription */
	if (!o_ptr->note) return (FALSE);

	/* Find a '=' */
	s = strchr(quark_str(o_ptr->note), '=');

	/* Process inscription */
	while (s)
	{
		/* Auto-pickup on "=g" */
		if (s[1] == 'g')
		{
			if ((s[2] >= '0') && (s[2] <= '9'))
			{
				int i, n;

				n = atoi(s+2);

				if (o_ptr->number > n) return (FALSE);

				for (i = 0; i < INVEN_WIELD; i++)
				{
					if (object_similar(o_ptr, &inventory[i]))
					{
						/* If too many, don't pick up */
						if (inventory[i].number > n) return (FALSE);
					}
				}

				/* Pick up */
				return (TRUE);
			}

			/* No number specified */
			else return (TRUE);

		}

		/* Find another '=' */
		s = strchr(s + 1, '=');
	}

	/* Don't auto pickup */
	return (FALSE);
}


/*
 * Determine if the object has a "=d" in its inscription.
 * If "=d" is followed by a number, don't pick up if the player has this number or more in their inventory.
 *
 * Hack -- treat all monster body parts as having a =d, if the option is set.
 */
static bool auto_pickup_never(const object_type *o_ptr)
{
	cptr s;

	/* Ignore corpses */
	if (easy_corpses)
	{
		switch (o_ptr->tval)
		{
			case TV_BONE:
			case TV_BODY:
			case TV_SKIN:
			case TV_JUNK:
			{
				return (TRUE);
			}
		}
	}

	/* No inscription */
	if (!o_ptr->note) return (FALSE);

	/* Find a '=' */
	s = strchr(quark_str(o_ptr->note), '=');

	/* Process inscription */
	while (s)
	{
		/* Never pickup on "=d" */
		if (s[1] == 'd')
		{
			if ((s[2] >= '0') && (s[2] <= '9'))
			{
				int i, n;

				n = atoi(s+2);

				for (i = 0; i < INVEN_WIELD; i++)
				{
					if (object_similar(o_ptr, &inventory[i]))
					{
						/* If sufficient, don't pickup */
						if (inventory[i].number > n) return (TRUE);
					}
				}

				/* Allow pickup */
				return (FALSE);
			}

			/* No number specified */
			else return (TRUE);

		}

		/* Find another '=' */
		s = strchr(s + 1, '=');
	}

	/* Can pickup */
	return (FALSE);
}


/*
 * Determine if the object can be destroyed, and has "=k" in its inscription.
 * If "=k" is followed by a number, destroy if the player has more than this number of similar objects in their inventory.
 */
static bool auto_destroy_okay(const object_type *o_ptr)
{
	cptr s;

	/* No inscription */
	if (!o_ptr->note) return (FALSE);

	/* Find a '=' */
	s = strchr(quark_str(o_ptr->note), '=');

	/* Process inscription */
	while (s)
	{
		/* Auto-destroy on "=k" */
		if (s[1] == 'k')
		{
			if ((s[2] >= '0') && (s[2] <= '9'))
			{
				int i, n;

				n = atoi(s+2);

				for (i = 0; i < INVEN_WIELD; i++)
				{
					if (object_similar(o_ptr, &inventory[i]))
					{
						/* If too many, allow destroy */
						if (inventory[i].number > n) return (TRUE);
					}
				}

				/* Don't auto destroy */
				return (FALSE);
			}

			/* No number specified */
			else return (TRUE);
		}

		/* Find another '=' */
		s = strchr(s + 1, '=');
	}

	/* Don't auto destroy */
	return (FALSE);
}


/*
 * Determine if the object has "=i" in its inscription.
 * If "=i" is followed by a number, ignore if the player has more than this number of similar objects in their inventory.
 */
bool auto_pickup_ignore(const object_type *o_ptr)
{
	cptr s;

	/* No inscription */
	if (!o_ptr->note) return (FALSE);

	/* Find a '=' */
	s = strchr(quark_str(o_ptr->note), '=');

	/* Process inscription */
	while (s)
	{
		/* Auto-ignore on "=i" */
		if (s[1] == 'i')
		{
			if ((s[2] >= '0') && (s[2] <= '9'))
			{
				int i, n;

				n = atoi(s+2);

				for (i = 0; i < INVEN_WIELD; i++)
				{
					if (object_similar(o_ptr, &inventory[i]))
					{
						/* If too many, allow destroy */
						if (inventory[i].number > n) return (TRUE);
					}
				}

				/* Don't auto-ignore */
				return (FALSE);
			}

			/* No number specified */
			else return (TRUE);
		}

		/* Find another '=' */
		s = strchr(s + 1, '=');
	}

	/* Don't auto ignore */
	return (FALSE);
}


/*
 * Helper routine for py_pickup() and py_pickup_floor().
 *
 * Destroy objects on the ground.
 */
static void py_destroy_aux(int o_idx)
{
	char o_name[80];
	char out_val[160];

	object_type *o_ptr;

	o_ptr = &o_list[o_idx];

	/* Describe the object */
	object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

	/* Verify destruction */
	if (verify_destroy && !auto_pickup_ignore(o_ptr))
	{
		sprintf(out_val, "Really destroy %s? ", o_name);
		if (!get_check(out_val)) return;
	}

	/* Take turn */
	p_ptr->energy_use = 100;

	/* Containers release contents */
	if ((o_ptr->tval == TV_HOLD) && (o_ptr->name3 > 0))
	{
		if (animate_object(-o_idx)) return;

		/* Message */
		msg_format("You cannot destroy %s.", o_name);

		return;
	}

	/* Artifacts cannot be destroyed */
	if (artifact_p(o_ptr))
	{

		/* Message */
		msg_format("You cannot destroy %s.", o_name);

		/* Sense the object if allowed, don't sense ID'ed stuff */
		if (!(o_ptr->feeling)
			&& !(o_ptr->ident & (IDENT_SENSE))
			 && !(object_named_p(o_ptr)))
		{
			o_ptr->discount = INSCRIP_UNBREAKABLE;

			/* The object has been "sensed" */
			o_ptr->ident |= (IDENT_SENSE);

			/* Combine the pack */
			p_ptr->notice |= (PN_COMBINE);

			/* Window stuff */
			p_ptr->window |= (PW_INVEN | PW_EQUIP);
		}

		/* Done */
		return;

	}

	/* Message */
	if (!auto_pickup_ignore(o_ptr))
	{
		/* Message */
		msg_format("You destroy %s.", o_name);
	}

	/* Delete the object directly */
	delete_object_idx(o_idx);

}



/*
 * Helper routine for py_pickup() and py_pickup_floor().
 *
 * Add the given dungeon object to the character's inventory.
 *
 * Delete the object afterwards.
 */
static void py_pickup_aux(int o_idx)
{
	int slot;

	char o_name[80];
	object_type *o_ptr;

	o_ptr = &o_list[o_idx];

	/* Carry the object */
	slot = inven_carry(o_ptr);

	/* Get the object again */
	o_ptr = &inventory[slot];

	/* Describe the object */
	object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

	/* Message */
	msg_format("You have %s (%c).", o_name, index_to_label(slot));

	/* Delete the object */
	if (o_idx >= 0) delete_object_idx(o_idx);
}



/*
 * Make the player carry everything in a grid.
 *
 * If "pickup" is FALSE then only gold will be picked up.
 */
void py_pickup(int pickup)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	s16b this_o_idx, next_o_idx = 0;

	object_type *o_ptr;

	char o_name[80];

#ifdef ALLOW_EASY_FLOOR

	int last_o_idx = 0;

	int can_pickup = 0;
	int not_pickup = 0;

#endif /* ALLOW_EASY_FLOOR */

	/* Are we allowed to pick up anything here? */
	if (!(f_info[cave_feat[py][px]].flags1 & (FF1_DROP))) return;

	/* Scan the pile of objects */
	for (this_o_idx = cave_o_idx[py][px]; this_o_idx; this_o_idx = next_o_idx)
	{
		/* Get the object */
		o_ptr = &o_list[this_o_idx];

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Ignore 'store' items */
		if (o_ptr->ident & (IDENT_STORE)) continue;

		/* Mark the object */
		if (!auto_pickup_ignore(o_ptr))
		{
			o_ptr->ident |= (IDENT_MARKED);

			/* XXX XXX - Mark objects as "seen" (doesn't belong in this function) */
			if (!k_info[o_ptr->k_idx].flavor) k_info[o_ptr->k_idx].aware = TRUE;

			/* XXX XXX - Mark monster objects as "seen" */
			if ((o_ptr->name3 > 0) && !(l_list[o_ptr->name3].sights)) l_list[o_ptr->name3].sights++;

		}

		/* Describe the object */
		object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

		/* Hack -- disturb */
		if (!auto_pickup_never(o_ptr)) disturb(0, 0);

		/* Pick up gold */
		if (o_ptr->tval >= TV_GOLD)
		{
			long value = o_ptr->number * o_ptr->charges - o_ptr->stackc;

			/* Message */
			if (o_ptr->tval == TV_GOLD) msg_format("You have found %ld gold pieces worth of %s.",
			   			value, o_name);
			else msg_format("You have found %s worth %ld gold pieces.", o_name, value);

			/* Collect the gold */
			p_ptr->au += value;

			/* Redraw gold */
			p_ptr->redraw |= (PR_GOLD);

			/* Window stuff */
			p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);

			/* Delete the gold */
			delete_object_idx(this_o_idx);

			/* Check the next object */
			continue;
		}

		/* Test for auto-destroy */
		if (auto_destroy_okay(o_ptr))
		{
			/* Destroy the object */
			py_destroy_aux(this_o_idx);

			/* Check the next object */
			continue;
		}

		/* Test for auto-pickup */
		if (auto_pickup_okay(o_ptr))
		{
			/* Pick up the object */
			py_pickup_aux(this_o_idx);

			/* Check the next object */
			continue;
		}

#ifdef ALLOW_EASY_FLOOR

		/* Easy Floor */
		if (easy_floor)
		{
			/* Pickup if possible */
			if (pickup && inven_carry_okay(o_ptr))
			{
				/* Pick up if allowed */
				if ((!carry_query_flag) && !auto_pickup_never(o_ptr))
				{
					/* Pick up the object */
					py_pickup_aux(this_o_idx);
				}

				/* Else count */
				else
				{
					/* Remember */
					last_o_idx = this_o_idx;

					/* Count */
					++can_pickup;
				}
			}

			/* Else count */
			else
			{
				/* Remember */
				last_o_idx = this_o_idx;

				/* Count */
				++not_pickup;
			}

			/* Check the next object */
			continue;
		}

#endif /* ALLOW_EASY_FLOOR */

		/* Describe the object */
		if ((!pickup) || auto_pickup_never(o_ptr))
		{
			msg_format("You %s %s.", p_ptr->blind || no_lite() ? "feel" : "see", o_name);

			/* Check the next object */
			continue;
		}

		/* Note that the pack is too full */
		else if (!inven_carry_okay(o_ptr))
		{
			msg_format("You have no room for %s.", o_name);

			/* Check the next object */
			continue;
		}

		/* Query before picking up */
		else if (carry_query_flag)
		{
			char out_val[160];
			sprintf(out_val, "Pick up %s? ", o_name);
			if (!get_check(out_val)) continue;
		}

		/* Pick up the object */
		py_pickup_aux(this_o_idx);
	}

#ifdef ALLOW_EASY_FLOOR

	/* Easy floor, objects left */
	if (easy_floor && (can_pickup + not_pickup > 0))
	{
		/* Not picking up */
		if (!pickup)
		{
			/* One object */
			if (not_pickup == 1)
			{
				/* Get the object */
				o_ptr = &o_list[last_o_idx];

				/* Describe the object */
				object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

				/* Message */
				msg_format("You %s %s.", p_ptr->blind || no_lite()? "feel" : "see", o_name);
			}

			/* Multiple objects */
			else
			{
				/* Message */
				msg_format("You %s a pile of %d objects.", p_ptr->blind || no_lite() ? "feel" : "see", not_pickup);
			}

			/* Done */
			return;
		}

		/* No room */
		if (!can_pickup)
		{
			/* One object */
			if (not_pickup == 1)
			{
				/* Get the object */
				o_ptr = &o_list[last_o_idx];

				/* Describe the object */
				object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

				/* Message */
				msg_format("You have no room for %s.", o_name);
			}

			/* Multiple objects */
			else
			{
				/* Message */
				msg_print("You have no room for any of the objects on the floor.");
			}

			/* Done */
			return;
		}

		/* Pick up objects */
		while (1)
		{
			cptr q, s;

			int item;

			/* Restrict the choices */
			item_tester_hook = inven_carry_okay;

			/* Get an object*/
			q = "Get which item? ";
			s = NULL;
			if (!get_item(&item, q, s, (USE_FLOOR | USE_FEATG))) break;

			/* Pick up the object */
			py_pickup_aux(0 - item);
		}
	}

#endif /* ALLOW_EASY_FLOOR */

}


/*
 * Handle player hitting a real trap
 */
void hit_trap(int y, int x)
{
	int dam;

	feature_type *f_ptr;

	cptr name;

	cptr text;

	int feat = cave_feat[y][x];

	/* Get feature */
	f_ptr = &f_info[feat];

	/* Hack --- trapped doors/chests */
	while (f_ptr->flags3 & (FF3_PICK_TRAP))
	{
		/* Get the trap */
		pick_trap(y,x);

		/* Error */
		if (cave_feat[y][x] == feat) break;

		/* Set the trap */
		feat = cave_feat[y][x];

		/* Get feature */
		f_ptr = &f_info[feat];
	}

	/* Use covered or bridged if necessary */
	if ((f_ptr->flags2 & (FF2_COVERED)) || (f_ptr->flags2 & (FF2_BRIDGED)))
	{
		f_ptr = &f_info[f_ptr->mimic];
	}

	/* Hack -- fall onto trap */
	if ((p_ptr->py != y) || (p_ptr->px !=x))
	{
		/* Move player */
		monster_swap(p_ptr->py, p_ptr->px, y, x);
	}

	/* Get the feature name */
	name = (f_name + f_ptr->name);

	/* Get the feature description */
	text = (f_text + f_ptr->text);

	/* Disturb the player */
	disturb(0, 0);

	/* Apply the object */
	if ((cave_o_idx[y][x]) && (f_ptr->flags1 & (FF1_HIT_TRAP)))
	{
		object_type *o_ptr = &o_list[cave_o_idx[y][x]];

		char o_name[80];

		int power = 0;

		switch (o_ptr->tval)
		{
			case TV_BOW:
			{
				object_type *j_ptr;
				u32b f1, f2, f3, f4;

				int i, shots = 1;

				/* Get bow */
				j_ptr = o_ptr;

				/* Get bow flags */
				object_flags(o_ptr,&f1,&f2,&f3,&f4);

				/* Apply extra shots */
				if (f1 & (TR1_SHOTS)) shots += j_ptr->pval;

				/* Test for hit */
				for (i = 0; i < shots; i++)
				{
					if (j_ptr->next_o_idx)
					{
						int ammo = j_ptr->next_o_idx;
						object_type *i_ptr;
						object_type object_type_body;

						/* Use ammo instead of bow */
						o_ptr = &o_list[ammo];

						/* Describe ammo */
						object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 0);

						if ((ammo) && (test_hit_fire((j_ptr->to_h + o_ptr->to_h)* BTH_PLUS_ADJ + f_ptr->power, p_ptr->ac, TRUE)))
						{
							int k, mult;

							switch (j_ptr->sval)
							{
								case SV_SLING:
								case SV_SHORT_BOW:
								mult = 2;
								break;
								case SV_LONG_BOW:
								case SV_LIGHT_XBOW:
									mult = 3;
									break;
								case SV_HEAVY_XBOW:
									mult = 4;
									break;
								default:
									mult = 1;
									break;
							}

							/* Apply extra might */
							if (f1 & (TR1_MIGHT)) mult += j_ptr->pval;

							k = damroll(o_ptr->dd, o_ptr->ds);
							k *= mult;

							k += critical_shot(o_ptr->weight, o_ptr->to_h + j_ptr->to_h, k);
							k += o_ptr->to_d + j_ptr->to_d;

							/* No negative damage */
							if (k < 0) k = 0;

							/* Trap description */
							msg_format("%^s hits you.",o_name);

							/* Damage, check for fear and death */
							take_hit(k, "a player trap");
						}
						else
						{
							/* Trap description */
							msg_format("%^s narrowly misses you.",o_name);
						}

						/* Get local object */
						i_ptr = &object_type_body;

						/* Obtain a local object */
						object_copy(i_ptr, o_ptr);

						/* Modify quantity */
						i_ptr->number = 1;

						/* Apply additional effect from coating or sometimes activate */
						if ((coated_p(i_ptr)) || (auto_activate(i_ptr)))
						{
							/* Make item strike */
							process_item_blow(i_ptr, y, x);

							/* Hack -- Remove coating on original */
							if ((!coated_p(i_ptr)) && (o_ptr->feeling == INSCRIP_COATED)) o_ptr->feeling = 0;
						}

						/* Drop nearby - some chance of breakage */
						drop_near(i_ptr,y,x,breakage_chance(i_ptr));

						/* Decrease the item */
						floor_item_increase(ammo, -1);
						floor_item_optimize(ammo);

						break;
					}
					else
					{
						/* Disarm */
						cave_alter_feat(y,x,FS_DISARM);
					}
				}
			}

			case TV_HAFTED:
			case TV_SWORD:
			case TV_POLEARM:
			{
				object_type *i_ptr;
				object_type object_type_body;

				/* Describe ammo */
				object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 0);

				/* Test for hit */
				if (test_hit_norm(o_ptr->to_h * BTH_PLUS_ADJ + f_ptr->power, p_ptr->ac, TRUE))
				{
					int k;

					k = damroll(o_ptr->dd, o_ptr->ds);

					k += critical_norm(o_ptr->weight, o_ptr->to_h, k);
					k += o_ptr->to_d;

					/* Armour reduces total damage */
					k -= (k * ((p_ptr->ac < 150) ? p_ptr->ac : 150) / 250);

					/* No negative damage */
					if (k < 0) k = 0;

					/* Trap description */
					msg_format("%^s hits you.",o_name);

					/* Damage, check for fear and death */
					take_hit(k, "a player trap");
				}
				else
				{
					/* Trap description */
					msg_format("%^s narrowly misses you.",o_name);					
				}

				/* Get local object */
				i_ptr = &object_type_body;

				/* Obtain a local object */
				object_copy(i_ptr, o_ptr);

				/* Modify quantity */
				i_ptr->number = 1;

				/* Apply additional effect from coating or sometimes activate */
				if ((coated_p(i_ptr)) || (auto_activate(i_ptr)))
				{
					/* Make item strike */
					process_item_blow(i_ptr, y, x);

					/* Hack -- Remove coating on original */
					if ((!coated_p(i_ptr)) && (o_ptr->feeling == INSCRIP_COATED)) o_ptr->feeling = 0;
				}

				/* Drop nearby - some chance of breakage */
				drop_near(i_ptr,y,x,breakage_chance(i_ptr));

				/* Decrease the item */
				floor_item_increase(cave_o_idx[y][x], -1);
				floor_item_optimize(cave_o_idx[y][x]);

				/* Disarm if runs out */
				if (!cave_o_idx[y][x]) cave_alter_feat(y,x,FS_DISARM);

				break;
			}

			case TV_WAND:
			case TV_STAFF:
			{
				if (o_ptr->charges > 0)
				{
					/* Get item effect */
					get_spell(&power, "use", o_ptr, FALSE);

					/* XXX Hack -- new unstacking code */
					o_ptr->stackc++;

					/* No spare charges */	
					if (o_ptr->stackc >= o_ptr->number)
					{
						/* Use a charge off the stack */
						o_ptr->charges--;

						/* Reset the stack count */
						o_ptr->stackc = 0;
					}

					/* XXX Hack -- unstack if necessary */
					if ((o_ptr->number > 1) &&					 
					((!object_charges_p(o_ptr) && (o_ptr->charges == 2) && (o_ptr->stackc > 1)) ||
					  (!object_charges_p(o_ptr) && (rand_int(o_ptr->number) <= o_ptr->stackc) &&
					  (o_ptr->stackc != 1) && (o_ptr->charges > 2))))
					{
						object_type *i_ptr;
						object_type object_type_body;

						/* Get local object */
						i_ptr = &object_type_body;

						/* Obtain a local object */
						object_copy(i_ptr, o_ptr);

						/* Modify quantity */
						i_ptr->number = 1;

						/* Reset stack counter */
						i_ptr->stackc = 0;
 
				 		/* Unstack the used item */
				 		o_ptr->number--;

						/* Reduce the charges on the new item */
						if (o_ptr->stackc > 1)
						{
							i_ptr->charges-=2;
							o_ptr->stackc--;
						}
						else if (!o_ptr->stackc)
						{
							i_ptr->charges--;
							o_ptr->charges++;
							o_ptr->stackc = o_ptr->number-1;
						}

						(void)floor_carry(y,x,i_ptr);
					}
				}
				else
				{
					/* Disarm if runs out */
					cave_alter_feat(y,x,FS_DISARM);
				}

				break;
			}

			case TV_ROD:
			case TV_DRAG_ARMOR:
			{
				if (!((o_ptr->timeout) && ((!o_ptr->stackc) || (o_ptr->stackc >= o_ptr->number))))
				{
					int tmpval;

					/* Store pval */
					tmpval = o_ptr->timeout;

					/* Time rod out */
					o_ptr->timeout = o_ptr->charges;

					/* Get item effect */
					get_spell(&power, "use", o_ptr, FALSE);

					/* Has a power */
					/* Hack -- check if we are stacking rods */
					if ((o_ptr->timeout > 0) && (!(tmpval) || stack_force_times))
					{
						/* Hack -- one more rod charging */
						if (o_ptr->timeout) o_ptr->stackc++;

						/* Reset stack count */
						if (o_ptr->stackc == o_ptr->number) o_ptr->stackc = 0;

						/* Hack -- always use maximum timeout */
						if (tmpval > o_ptr->timeout) o_ptr->timeout = tmpval;

					}

					/* XXX Hack -- unstack if necessary */
					if ((o_ptr->number > 1) && (o_ptr->timeout > 0))
					{
						object_type *i_ptr;
						object_type object_type_body;

						/* Get local object */
						i_ptr = &object_type_body;

						/* Obtain a local object */
						object_copy(i_ptr, o_ptr);

						/* Modify quantity */
						i_ptr->number = 1;

						/* Clear stack counter */
						i_ptr->stackc = 0;

						/* Restore "charge" */
						o_ptr->timeout = tmpval;

						/* Unstack the used item */
						o_ptr->number--;

						/* Reset the stack if required */
						if (o_ptr->stackc == o_ptr->number) o_ptr->stackc = 0;

						(void)floor_carry(y,x,i_ptr);
					}
				}
				break;
			}

			case TV_POTION:
			case TV_SCROLL:
			case TV_FLASK:
			case TV_LITE:
			case TV_FOOD:
			{
				/* Hack -- boring food */
				if ((o_ptr->tval == TV_FOOD) && (o_ptr->sval >= SV_FOOD_MIN_FOOD))
				{
					/* Trap description */
					msg_print("Hmm... there was something under this rock.");					

					/* Disarm */
					cave_alter_feat(y,x,FS_DISARM);
				}
				else
				{
					/* Get item effect */
					get_spell(&power, "use", o_ptr, FALSE);

					/* Decrease the item */
					floor_item_increase(cave_o_idx[y][x], -1);
					floor_item_optimize(cave_o_idx[y][x]);

					/* Disarm if runs out */
					if (!cave_o_idx[y][x]) cave_alter_feat(y,x,FS_DISARM);
				}

				break;
			}

			case TV_RUNESTONE:
			{
				u32b runes = p_ptr->cur_runes;

				int num = 0;
				s16b book[26];

				/* Hack -- use current rune */
				p_ptr->cur_runes = (2 << (o_ptr->sval-1));

				/* Fill the book with spells */
				fill_book(o_ptr,book,&num);

				/* Unhack */
				p_ptr->cur_runes = runes;

				/* Get a power */
				power = book[rand_int(num)];

				/* Decrease the item */
				floor_item_increase(cave_o_idx[y][x], -1);
				floor_item_optimize(cave_o_idx[y][x]);

				/* Disarm if runs out */
				if (!cave_o_idx[y][x]) cave_alter_feat(y,x,FS_DISARM);

				break;
			}

			default:
			{
				/* Trap description */
				msg_print("Hmm... there was something under this rock.");					

				/* Disarm */
				cave_alter_feat(y,x,FS_DISARM);

				break;
			}
		}


		/* Has a power */
		if (power > 0)
		{
			spell_type *s_ptr = &s_info[power];

			int ap_cnt;

			/* Object is used */
			if (k_info[o_ptr->k_idx].used < MAX_SHORT) k_info[o_ptr->k_idx].used++;

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
				if (!d_side)
				{
					d_plus += 25 * d_dice;
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

				(void)project_p(0,y,x,damage, effect);
				(void)project_f(0,y,x,damage, effect);
				(void)project_t(0,y,x,damage, effect);
			}
		}
	}

	/* Regular traps */
	else
	{
		if (strlen(text))
		{
			/* Message */
			msg_format("%s",text);
		}

		if (f_ptr->spell)
		{
      			make_attack_ranged(0,f_ptr->spell,y,x);
		}
		else if (f_ptr->blow.method)
		{
			dam = damroll(f_ptr->blow.d_side,f_ptr->blow.d_dice);
   
			/* Apply the blow */
			project_p(0, p_ptr->py, p_ptr->px, dam, f_ptr->blow.effect);
			project_t(0, p_ptr->py, p_ptr->px, dam, f_ptr->blow.effect);
		}

		/* Get feature */
		f_ptr = &f_info[cave_feat[p_ptr->py][p_ptr->px]];

		if (f_ptr->flags1 & (FF1_HIT_TRAP))
		{
			/* Modify the location hit by the trap */
			cave_alter_feat(y,x,FS_HIT_TRAP);
		}
		else if (f_ptr->flags1 & (FF1_SECRET))
		{
			/* Discover */
			cave_alter_feat(y,x,FS_SECRET);
		}
	}
}


/*
 *   Get style benefits against a monster, using permitted styles.
 */
void mon_style_benefits(const monster_type *m_ptr, u32b style, int *to_hit, int *to_dam, int *to_crit)
{
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	int i;

	/* Reset style benefits */
	*to_hit = 0;
	*to_dam = 0;
	*to_crit = 0;

	/* Check backstab if monster sleeping or fleeing */
	if (((m_ptr->csleep) || (m_ptr->monfear)) && (p_ptr->cur_style & (1L<<WS_SWORD)) &&
		  (p_ptr->pstyle == WS_BACKSTAB) && (inventory[INVEN_WIELD].weight < 100)) style |= (1L <<WS_BACKSTAB);

	/* Check slay orc if monster is an orc */
	if (r_ptr->flags3 & (RF3_ORC)) style |= (1L <<WS_SLAY_ORC);

	/* Check slay troll if monster is a troll */
	if (r_ptr->flags3 & (RF3_TROLL)) style |= (1L <<WS_SLAY_TROLL);

	/* Check slay giant if monster is a giant */
	if (r_ptr->flags3 & (RF3_GIANT)) style |= (1L <<WS_SLAY_GIANT);

	/* Check slay dragon if monster is a dragon */
	if (r_ptr->flags3 & (RF3_DRAGON)) style |= (1L <<WS_SLAY_DRAGON);

	/* Check slay evil if monster is evil */
	if (r_ptr->flags3 & (RF3_EVIL)) style |= (1L <<WS_SLAY_EVIL);

	/* Check slay giant if monster is undead */
	if (r_ptr->flags3 & (RF3_UNDEAD)) style |= (1L <<WS_SLAY_UNDEAD);

	/* Check slay giant if monster is an animal */
	if (r_ptr->flags3 & (RF3_ANIMAL)) style |= (1L <<WS_SLAY_ANIMAL);

	/* Check slay giant if monster is a demon */
	if (r_ptr->flags3 & (RF3_DEMON)) style |= (1L <<WS_SLAY_DEMON);

	/*** Handle styles ***/
	for (i = 0;i< z_info->w_max;i++)
	{
		if (w_info[i].class != p_ptr->pclass) continue;

		if (w_info[i].level > p_ptr->lev) continue;

		/* Check for styles */
		if ((w_info[i].styles==0) || (w_info[i].styles & (style & (1L << p_ptr->pstyle))))
		{
			switch (w_info[i].benefit)
			{
				case WB_HIT:
					*to_hit += (p_ptr->lev - w_info[i].level) /2;
					break;

				case WB_DAM:
					*to_dam += (p_ptr->lev - w_info[i].level) /2;
					break;

				case WB_CRITICAL:
					*to_crit++;
					break;
			}
		}
	}

	/* Only allow criticals against living opponents */
	if ((r_ptr->flags3 & (RF3_NONLIVING)) || (r_ptr->flags2 & (RF2_STUPID)))
	{
		*to_crit = 0;
	}

	/* Only allow criticals against visible opponents */
	if (!(m_ptr->ml)) *to_crit = 0;
}


/*
 * Get the weapon slot based on the melee style and number of blows so far.
 */
static int weapon_slot(u32b melee_style, int blows, bool charging)
{
	int slot = INVEN_WIELD;

	/* Get secondary weapon instead */
	if (!(blows % 2) && (melee_style & (1L << WS_TWO_WEAPON))) slot = INVEN_ARM;

	/* Get the unarmed weapon */
	if (melee_style & (1L << WS_UNARMED))
	{
		/* Use feet while charging */
		if (charging) slot = INVEN_FEET;

		/* Alternate hands and feet */
		else if (!(blows %2)) slot = INVEN_FEET;

		/* Use hands */
		else slot = INVEN_HANDS;
	}

	return(slot);
}





/*
 * Determine if the object has a "=A" in its inscription.
 *
 * This is used to automatically activate the object when attacking, throwing or firing it.
 */
bool auto_activate(const object_type *o_ptr)
{
	u32b f1, f2, f3, f4;

	cptr s;

	/* Check timeout */
	if (o_ptr->timeout) return (FALSE);

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4);
	
	/* Check if can activate */
	if ((f3 & (TR3_ACTIVATE)) == 0) return (FALSE);

	/* No inscription */
	if (!o_ptr->note) return (FALSE);

	/* Find a '=' */
	s = strchr(quark_str(o_ptr->note), '=');

	/* Process inscription */
	while (s)
	{
		/* Automatically activate on "=A" */
		if (s[1] == 'A')
		{
			/* Can activate */
			return (TRUE);
		}

		/* Find another '=' */
		s = strchr(s + 1, '=');
	}

	/* Can activate */
	return (FALSE);
}



/*
 * Attack the monster at the given location
 *
 * If no "weapon" is available, then "punch" the monster one time.
 */
void py_attack(int y, int x, bool charging)
{
	int num = 0, k, bonus, chance;

	/* Style bonuses */
	int style_hit = 0;
	int style_dam = 0;
	int style_crit = 0;
	u32b melee_style;

	int blows = 0; /* Number of blows actually delivered */
	int slot;

	int i;
	int num_blows = 0;
	int do_cuts = 0;
	int do_stun = 0;
	
	monster_type *m_ptr;
	monster_race *r_ptr;
	monster_lore *l_ptr;

	object_type *o_ptr;

	char m_name[80];

	bool fear = FALSE;

	bool do_quake = FALSE;

	bool was_asleep;

	cptr p;

	u32b k1[3], k2[3], k3[3], k4[3];

	u32b n1[3], n2[3], n3[3], n4[3];


	/* Get the monster */
	m_ptr = &m_list[cave_m_idx[y][x]];
	r_ptr = &r_info[m_ptr->r_idx];
	l_ptr = &l_list[m_ptr->r_idx];


	/* Disturb the player */
	disturb(0, 0);

	/* Extract monster name (or "it") */
	monster_desc(m_name, m_ptr, 0);

	/* Auto-Recall if possible and visible */
	if (m_ptr->ml) monster_race_track(m_ptr->r_idx);

	/* Track a new monster */
	if (m_ptr->ml) health_track(cave_m_idx[y][x]);

	/* Some monsters radiate damage when attacked */
	if (r_ptr->flags2 & (RF2_HAS_AURA))
	{
		(void)make_attack_ranged(cave_m_idx[y][x],96+7,p_ptr->py,p_ptr->px);
	}

	/* Handle player fear */
	if (p_ptr->afraid)
	{
		/* Message */
		msg_format("You are too afraid to attack %s!", m_name);

		/* Done */
		return;
	}

	/* Check melee styles only */
	melee_style = p_ptr->cur_style & (WS_WIELD_FLAGS);

	/* Get style benefits */
	mon_style_benefits(m_ptr, melee_style, &style_hit, &style_dam, &style_crit);

	/* Check if monster asleep */
	was_asleep = (m_ptr->csleep == 0);

	/* Disturb the monster */
	m_ptr->csleep = 0;

	/* Mark the monster as attacked by melee */
	m_ptr->mflag |= (MFLAG_HIT_BLOW);

	/* Get number of blows */
	num_blows = p_ptr->num_blow;

	/* Restrict blows if charging */
	if (charging)
	{
		if (melee_style & (1L << WS_TWO_WEAPON)) num_blows = 2;
		else num_blows = 1;
	}

	/* Attack once for each legal blow */
	while (num++ < num_blows)
	{
		/* Deliver a blow */
		blows++;

		/* Get weapon slot */
		slot = weapon_slot(melee_style,blows,charging);

		/* Weapon slot */
		o_ptr = &inventory[slot];

		/* Record the item flags */
		if (o_ptr->k_idx)
		{
			/* Get item first time it is used */
			if ((blows <= 2) && ((blows <= 1)
				|| (melee_style & ((1L << WS_UNARMED) | (1L << WS_TWO_WEAPON))) ))
			{
				k1[blows] = o_ptr->can_flags1;
				k2[blows] = o_ptr->can_flags2;
				k3[blows] = o_ptr->can_flags3;
				k4[blows] = o_ptr->can_flags4;
			}
		}

		/* Some monsters are great at dodging  -EZ- */
		if (mon_evade(m_ptr, m_ptr->stunned || m_ptr->confused? 50 : 80, 100, "your blow")) continue;

		/* Calculate the "attack quality" */
		if (o_ptr->k_idx) bonus = p_ptr->to_h + o_ptr->to_h + style_hit;
		else bonus = p_ptr->to_h + style_hit;

		chance = (p_ptr->skill_thn + (bonus * BTH_PLUS_ADJ));

		/* Test for hit */
		if (!test_hit_norm(chance, calc_monster_ac(m_ptr, FALSE), m_ptr->ml))
		{
			/* Message */
			message_format(MSG_MISS, m_ptr->r_idx, "You miss %s.", m_name);
		}
		/* Test for resistance */
		else if (mon_resist_object(m_ptr, o_ptr))
		{
			/* No need for message */
		}
		/* Test for huge monsters -- they resist non-charging attacks */
		else if ((r_ptr->flags3 & (RF3_HUGE)) && (!charging) && (rand_int(100) < 60))
		{
			/* Message */
			message_format(MSG_MISS, m_ptr->r_idx, "You cannot reach %s.", m_name);

			/* Miss */
		}
		/* Player hits */
		else
		{
			/* Handle normal weapon/gauntlets/boots */
			if (o_ptr->k_idx)
			{
				k = damroll(o_ptr->dd, o_ptr->ds);
				k = tot_dam_aux(o_ptr, k, m_ptr);

				switch (o_ptr->tval)
				{
					case TV_POLEARM:
					{
						int j = rand_int(100);

						/* Hack -- spears do damaging criticals, axes stun or cut */ 
						if (!(strstr(k_name + k_info[o_ptr->k_idx].name, "Axe"))
							&& !(strstr(k_name + k_info[o_ptr->k_idx].name, "Halberd"))
							&& !(strstr(k_name + k_info[o_ptr->k_idx].name, "Scythe")))
							k += critical_norm(o_ptr->weight, o_ptr->to_h + (style_crit * 30), k);
						else if ((j < 50) && !(strstr(k_name + k_info[o_ptr->k_idx].name, "Scythe")))
							do_stun = critical_norm(o_ptr->weight, o_ptr->to_h + (style_crit * 30), k);
						else
							do_cuts = critical_norm(o_ptr->weight, o_ptr->to_h + (style_crit * 30), k);
						break;
					}
					case TV_SWORD:
					{
						do_cuts = critical_norm(o_ptr->weight, o_ptr->to_h + (style_crit * 30), k);
						break;
					}
					default:
					{
						do_stun = critical_norm(o_ptr->weight, o_ptr->to_h + (style_crit * 30), k);
						break;
					}
				}

				/* Add damage bonus */
				k += o_ptr->to_d;

				/* Check usage */
				object_usage(slot);

				/* Note -- must happen after above flags updated */
				if (((p_ptr->cur_flags3 & (TR3_IMPACT)) != 0) && (k > 50))
				{
					do_quake = TRUE;

					/* Always notice */
					equip_can_flags(0x0L,0x0L,TR3_IMPACT,0x0L);
				}
				else if ((p_ptr->cur_flags3 & (TR3_IMPACT)) == 0)
				{
					/* Sometimes notice */
					equip_not_flags(0x0L,0x0L,TR3_IMPACT,0x0L);
				}

			}
			/* Handle bare-hand/bare-foot attack */
			else
			{
				k = 1;
				do_stun = critical_norm(c_info[p_ptr->pclass].min_weight, (style_crit * 30),k);
			}

			/* Adjust for style */
			k += p_ptr->to_d + style_dam;

			/* Adjust for charging */
			if (charging)
			{
				if ((melee_style & (1L << WS_UNARMED)) && (p_ptr->wt > 33)) k *= p_ptr->wt / cp_ptr->chg_weight;
				else if (o_ptr->weight >= 2 * cp_ptr->chg_weight) k *= o_ptr->weight / cp_ptr->chg_weight;
			}

			/* Monster armor reduces total damage */
			k -= (k * ((r_ptr->ac < 150) ? r_ptr->ac : 150) / 250);

			/* No negative damage */
			if (k < 0) k = 0;

			/* Some monsters resist stun */
			if (r_ptr->flags3 & (RF3_NO_STUN)) do_stun = 0;

			/* Some monsters resist cuts */
			if (r_ptr->flags9 & (RF9_NO_CUTS)) do_stun = 0;

			/* Hack --- backstab. Weapon of 10 lbs or less */
			if (melee_style & (1L << WS_BACKSTAB))
			{
				/* Message */
				p = "You backstab %s!";
			}
			else if (do_stun)
			{
				/* Message */
				p = "You batter %s!";
			}
			else if (do_cuts)
			{
				/* Message */
				p = "You wound %s!";
			}
			else if (charging)
			{
				/* Unarmed */
				if (melee_style & (1L << WS_UNARMED)) p = "You flying kick %s!";

				/* Message */
				else p = "You charge %s!";
			}
			else
			{
				/* Unarmed punch */
				if (melee_style & (1L << WS_UNARMED))
				{
					if (blows % 2) p = "You punch %s.";
					else p = "You kick %s.";
				}

				/* Message */
				else p = "You hit %s.";
			}

			/* Message */
			message_format(MSG_HIT, m_ptr->r_idx, p, m_name);

			/* Complex message */
			if (p_ptr->wizard)
			{
				msg_format("You do %d (out of %d) damage.", k, m_ptr->hp);
			}

			/* Apply stun effect */
			if (do_stun)
			{
				/* Avoid overflow */
				if ((do_stun + m_ptr->stunned) / (r_ptr->level / 10 + 1) > 875)
				{
					k+= 4 * ((do_stun + m_ptr->stunned) / (r_ptr->level / 10 + 1) - 100) / 5;
					m_ptr->stunned = 255;
				}

				/* Convert some stun damage to damage */
				else if ((do_stun + m_ptr->stunned) / (r_ptr->level / 10 + 1) > 100)
				{
					k+= 4 * ((do_stun + m_ptr->stunned) / (r_ptr->level / 10 + 1) - 100) / 5;
					m_ptr->stunned = (100 + (do_stun + m_ptr->stunned - 100) / 5) ;
				}
				else m_ptr->stunned += do_stun / (r_ptr->level / 10 + 1);
			}

			/* Apply cuts effect */
			if (do_cuts)
			{
				if ((m_ptr->cut + do_cuts) / (r_ptr->level / 10 + 1) > 255)
				{
					k+= ((m_ptr->cut + do_cuts) / (r_ptr->level / 10 + 1)) - 255;
					m_ptr->cut = 255;
				}
				else m_ptr->cut += do_cuts / (r_ptr->level / 10 + 1);
			}

			/* Damage, check for fear and death */
			if (mon_take_hit(cave_m_idx[y][x], k, &fear, NULL))
			{
				u32b f1, f2, f3, f4;

				/* Extract the flags */
				object_flags(o_ptr, &f1, &f2, &f3, &f4);

				/* We've killed it - Suck blood from the corpse */
				if ((f4 & (TR4_VAMP_HP)) & (r_ptr->flags8 & (RF8_HAS_BLOOD)))
				{
					hp_player(r_ptr->level);

					object_can_flags(o_ptr,0x0L,0x0L,0x0L,TR4_VAMP_HP);
					l_ptr->flags8 |= (RF8_HAS_BLOOD);
				}
				else if (l_ptr->flags8 & (RF8_HAS_BLOOD))
				{
					object_not_flags(o_ptr,0x0L,0x0L,0x0L,TR4_VAMP_HP);
				}

				/* We've killed it - Drain mana from the corpse*/
				if ((f4 & (TR4_VAMP_MANA)) & (r_ptr->mana > 0))
				{
					if (p_ptr->csp < p_ptr->msp)
					{
						p_ptr->csp = p_ptr->csp + r_ptr->mana;
						if (p_ptr->csp >= p_ptr->msp)
						{
							p_ptr->csp = p_ptr->msp;
							p_ptr->csp_frac = 0;
						}
						p_ptr->redraw |= (PR_MANA);
						p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);
					}

					object_can_flags(o_ptr,0x0L,0x0L,0x0L,TR4_VAMP_MANA);
				}
				else if (r_ptr->mana > 0)
				{
					object_not_flags(o_ptr,0x0L,0x0L,0x0L,TR4_VAMP_MANA);
				}

				/* Hack -- cancel wakeup call */
				was_asleep = FALSE;

				break;
			}

			/* Apply additional effect from coating or sometimes activate */
			else if ((coated_p(o_ptr)) || (auto_activate(o_ptr)))
			{
				/* Make item strike */
				process_item_blow(o_ptr, y, x);
			}
		}
	}

	/* Hack -- wake up nearby allies */
	if (was_asleep)
	{
		m_ptr->mflag |= (MFLAG_AGGR | MFLAG_SNEAKED);

		tell_allies_mflag(m_ptr->fy, m_ptr->fx, MFLAG_AGGR, "& has attacked me!");
	}
	else if (fear)
	{
		tell_allies_mflag(m_ptr->fy, m_ptr->fx, MFLAG_AGGR, "& has hurt me badly!");
	}

	/* Hack -- delay fear messages */
	if (fear && m_ptr->ml)
	{
		/* Message */
		message_format(MSG_FLEE, m_ptr->r_idx, "%^s flees in terror!", m_name);
	}

	/* Mega-Hack -- apply earthquake brand */
	if (do_quake) earthquake(p_ptr->py, p_ptr->px, 10);

	/* Charging uses full turn */
	if (charging) p_ptr->energy_use = 100;

	/* Only use required energy to kill monster */
	else p_ptr->energy_use = 100 * blows / p_ptr->num_blow;

	/* Check and display any changed weapon flags */
	for (i = 1; (i <= blows) && (i <= ((melee_style & ((1L << WS_UNARMED) | (1L << WS_TWO_WEAPON))) ? 2 : 1)); i++)
	{
		slot = weapon_slot(melee_style, i, charging);

		o_ptr = &inventory[slot];

		/* Check for new flags */
		n1[i] = (o_ptr->can_flags1 & ~(k1[i])) & (TR1_WEAPON_FLAGS);
		n2[i] = (o_ptr->can_flags2 & ~(k2[i])) & (TR2_WEAPON_FLAGS);
		n3[i] = (o_ptr->can_flags3 & ~(k3[i])) & (TR3_WEAPON_FLAGS);
		n4[i] = (o_ptr->can_flags4 & ~(k4[i])) & (TR4_WEAPON_FLAGS);

		if (n1[i] || n2[i] || n3[i] || n4[i]) update_slot_flags(slot, n1[i], n2[i], n3[i], n4[i]);
	}
}

/*
 * Player is stuck inside terrain.
 *
 * This routine should only be called when energy has been expended.
 *
 */
bool stuck_player(int *dir)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	feature_type *f_ptr = &f_info[cave_feat[py][px]];

	/* Hack -- allowed to move nowhere */
	if ((!*dir) || (*dir == 5)) return (FALSE);

	/* Player can not walk through "walls" */
	if (!(f_ptr->flags1 & (FF1_MOVE))
	&& !(f_ptr->flags3 & (FF3_EASY_CLIMB)))
	{
		/* Disturb the player */
		disturb(0, 0);

		/* Notice unknown obstacles */
		if (!(play_info[py][px] & (PLAY_MARK)))
		{

			/* Get hit by terrain/traps */
			if (((f_ptr->flags1 & (FF1_HIT_TRAP)) && !(f_ptr->flags3 & (FF3_CHEST))) ||
				(f_ptr->spell) || (f_ptr->blow.method))
			{

				/* Hit the trap */
				hit_trap(py, px);
			}

			play_info[py][px] |= (PLAY_MARK);

			lite_spot(py, px);

		}

		/* Always make direction 0 */
		*dir = 0;

		return (TRUE);

	}

	return (FALSE);

}


/*
 * Move player in the given direction, with the given "pickup" flag.
 * 
 * This routine should only be called when energy has been expended.
 *
 * Note that this routine handles monsters in the destination grid,
 * and also handles attempting to move into walls/doors/rubble/etc.
 */
void move_player(int dir, int jumping)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	feature_type *f_ptr;

	int y, x;

	int mimic;

	cptr name;

	/* Move is a climb? -- force boolean */
        bool climb = FALSE;

	/* Find the result of moving */
	y = py + ddy[dir];
	x = px + ddx[dir];

	f_ptr = &f_info[cave_feat[y][x]];

        climb = ((!(f_ptr->flags1 & (FF1_MOVE))
		&& (f_ptr->flags3 & (FF3_EASY_CLIMB)))
		|| (!(f_ptr->flags3 & (FF3_MUST_CLIMB)) 
                && (f_info[cave_feat[py][px]].flags3 & (FF3_MUST_CLIMB)))) != 0;

	/* Hack -- attack monsters --- except hidden ones */
	if ((cave_m_idx[y][x] > 0) && !(m_list[cave_m_idx[y][x]].mflag & (MFLAG_HIDE)))
	{
		bool charging = FALSE;

		/* If moving, you can charge in the direction */
		if ((p_ptr->charging == dir) || (side_dirs[dir][1] == p_ptr->charging)
			|| (side_dirs[dir][2] == p_ptr->charging)) charging = TRUE;

		/* Attack */
		py_attack(y, x, charging);
	}

	else if (stuck_player(&dir))
	{
		/* Do nothing */
	}

#ifdef ALLOW_EASY_ALTER

	/* Optionally alter known traps/doors on (non-jumping) movement */
	else if (easy_alter && !jumping &&
	 (play_info[y][x] & (PLAY_MARK)) &&
		 ( (f_ptr->flags1 & (FF1_DISARM)) ||
		 ( !(f_ptr->flags1 & (FF1_MOVE)) &&
		 !(f_ptr->flags3 & (FF3_EASY_CLIMB)) && 
		 ( (f_ptr->flags1 & (FF1_BASH)) ||       
		   (f_ptr->flags1 & (FF1_OPEN)) ))))
	{
		/* Not already repeating */
		if (!p_ptr->command_rep)
		{
			/* Hack -- auto-repeat */
			if (always_repeat && (p_ptr->command_arg <= 0))
			{
				/* Repeat 99 times */
				p_ptr->command_rep = 99;

				/* Reset the command count */
				p_ptr->command_arg = 0;
			}
		}

		/* Alter */
		do_cmd_alter();
	}

#endif /* ALLOW_EASY_ALTER */

	/* Player can not walk through "walls" */
	/* Also cannot climb over unknown "trees/rubble" */
	else if (!(f_ptr->flags1 & (FF1_MOVE))
	&& (!(f_ptr->flags3 & (FF3_EASY_CLIMB))
	|| !(play_info[y][x] & (PLAY_MARK))))
	{
		if ((verify_safe) && (play_info[p_ptr->py][p_ptr->px] & (PLAY_SAFE)) && !(play_info[y][x] & (PLAY_SAFE)))
		{
			disturb(1,0);
			msg_print("This doesn't feel safe.");

			if (!get_check("Are you sure?")) return;
		}

		/* Disturb the player */
		disturb(0, 0);

		/* Notice unknown obstacles */
		if (!(play_info[y][x] & (PLAY_MARK)))
		{
			/* Get hit by terrain/traps */
			if (((f_ptr->flags1 & (FF1_HIT_TRAP)) && !(f_ptr->flags3 & (FF3_CHEST))) ||
				(f_ptr->spell) || (f_ptr->blow.method))
			{
				/* Hit the trap */
				hit_trap(y, x);
			}

			/* Get the mimiced feature */
			mimic = f_ptr->mimic;

			/* Get the feature name */
			name = (f_name + f_info[mimic].name);

			/* Tell the player */
			msg_format("You feel %s%s blocking your way.",
				((f_ptr->flags2 & (FF2_FILLED)) ? "" :
					(is_a_vowel(name[0]) ? "an " : "a ")),name);


				play_info[y][x] |= (PLAY_MARK);

				lite_spot(y, x);

		}

		/* Mention known obstacles */
		else
		{
			/* Get the mimiced feature */
			mimic = f_ptr->mimic;

			/* Get the feature name */
			name = (f_name + f_info[mimic].name);

			/* Tell the player */
			msg_format("There is %s%s blocking your way.",
				((f_ptr->flags2 & (FF2_FILLED)) ? "" :
				(is_a_vowel(name[0]) ? "an " : "a ")),name);
		}
	}

	/* Petrified */
	else if (p_ptr->petrify)
	{
		/* Disturb the player */
		disturb(0, 0);

		msg_print("You are too petrified to move.");
		return;
	}

	/* Partial movement */
	else if ((climb) && (dir != p_ptr->climbing))
	{
		if ((verify_safe) && (play_info[p_ptr->py][p_ptr->px] & (PLAY_SAFE)) && !(play_info[y][x] & (PLAY_SAFE)))
		{
			disturb(1,0);
			msg_print("This doesn't feel safe.");		

			if (!get_check("Are you sure?")) return;
		}

		/* Get the mimiced feature */
		mimic = f_ptr->mimic;

		/* Use existing feature if not easy climb */
                if (!(f_ptr->flags3 & (FF3_EASY_CLIMB))) mimic = f_info[cave_feat[py][px]].mimic;

		/* Get the feature name */
		name = (f_name + f_info[mimic].name);

		/* Tell the player */
		msg_format("You climb %s%s%s.",
				((f_ptr->flags3 & (FF3_EASY_CLIMB)) ? "" : "out of "),
				((f_ptr->flags2 & (FF2_FILLED)) ? "" : "the "), name);

		p_ptr->climbing = dir;

		/* Automate 2nd movement command */
		p_ptr->command_cmd = 59;
		p_ptr->command_rep = 1;
		p_ptr->command_dir = dir;
  	}

	/* Normal movement */
	else
	{
		if ((verify_safe) && (play_info[p_ptr->py][p_ptr->px] & (PLAY_SAFE)) && !(play_info[y][x] & (PLAY_SAFE)))
		{
			disturb(1,0);
			msg_print("This doesn't feel safe.");		

			if (!get_check("Are you sure?")) return;
		}

		/* Sound XXX XXX XXX */
		/* sound(MSG_WALK); */

		/* Move player */
		monster_swap(py, px, y, x);

		/* New location */
		y = py = p_ptr->py;
		x = px = p_ptr->px;

		/* No longer climbing */
		p_ptr->climbing = 0;

		/* Spontaneous Searching */
		if ((p_ptr->skill_fos >= 50) ||
		    (0 == rand_int(50 - p_ptr->skill_fos)))
		{
			search();
		}

		/* Continuous Searching */
		if (p_ptr->searching)
		{
			/* Catch breath */
			if (!(f_ptr->flags2 & (FF2_FILLED)))
			{
				/* Rest the player */
				set_rest(p_ptr->rest + PY_REST_RATE - p_ptr->tiring);
			}

			search();
		}

		/* Dodging */
		else
		{
			/* Dodging -- reverse direction 180 degrees */
			p_ptr->dodging = 10 - dir;

			/* Hack -- not blocking */
			p_ptr->blocking = 0;

			/* Hack -- update straight away */
			update_stuff();

			/* Hack -- redraw straight away */
			redraw_stuff();
		}

		/* Handle "objects" */
		py_pickup(jumping != always_pickup);

		/* Handle "store doors" */
		if (f_ptr->flags1 & (FF1_ENTER))
		{
			/* Disturb */
			disturb(0, 0);

			/* Hack -- Enter store */
			p_ptr->command_new = '_';

			/* Free turn XXX XXX XXX */
			p_ptr->energy_use = 0;
		}


		/* Get hit by terrain/traps */
		/* Except for chests which must be opened to hit */
		if (((f_ptr->flags1 & (FF1_HIT_TRAP)) && !(f_ptr->flags3 & (FF3_CHEST))) ||
			(f_ptr->spell) || (f_ptr->blow.method))
		{
			/* Disturb */
			disturb(0, 0);

			/* Hit the trap */
			hit_trap(y, x);
		}

		/* Discover secrets */
		/* Except for hidden objects/gold which must be found by searching */
		else if ((f_ptr->flags1 & (FF1_SECRET))
			&& !(f_ptr->flags1 & (FF1_HAS_ITEM | FF1_HAS_GOLD)))
		{
			/* Find the secret */
			find_secret(y,x);

		}

		else if (f_ptr->flags2 & (FF2_KILL_MOVE))
		{
			cptr text;

			/* Get the feature description */
			text = (f_text + f_ptr->text);

			if (strlen(text))
			{
				/* You have found something */
				msg_format("%s",text);
			}


			cave_alter_feat(y,x, FS_KILL_MOVE);
		}

		/* Reveal when you are on shallow, deep or filled terrain */
		if (!(play_info[y][x] & (PLAY_MARK)) &&
		((f_ptr->flags2 & (FF2_SHALLOW)) ||
		(f_ptr->flags2 & (FF2_DEEP)) ||
		(f_ptr->flags2 & (FF2_FILLED)) ))
		{

			/* Get the mimiced feature */
			mimic = f_ptr->mimic;

			/* Get the feature name */
			name = (f_name + f_info[mimic].name);

			/* Tell the player */
			msg_format("You feel you are %s%s.",
				((f_ptr->flags2 & (FF2_FILLED)) ? "" : "in "), name);

			play_info[y][x] |= (PLAY_MARK);

			lite_spot(y, x);
		}
	}
}

/*
 * Hack -- Check for a "known wall" (see below)
 */
static int see_wall(int dir, int y, int x)
{
	/* Get the new location */
	y += ddy[dir];
	x += ddx[dir];

	/* Illegal grids are not known walls XXX XXX XXX */
	if (!in_bounds(y, x)) return (FALSE);

	/* Non-wall grids are not known walls */
	if (!(f_info[f_info[cave_feat[y][x]].mimic].flags1 & (FF1_WALL))) return (FALSE);

	/* Unknown walls are not known walls */
	if (!(play_info[y][x] & (PLAY_MARK))) return (FALSE);

	/* Default */
	return (TRUE);
}

/*
 * Hack -- Check for a "known obstacle" (see below)
 */
static int see_stop(int dir, int y, int x)
{
	/* Get the new location */
	y += ddy[dir];
	x += ddx[dir];

	/* Illegal grids are not known walls XXX XXX XXX */
	if (!in_bounds(y, x)) return (FALSE);

	/* Unknown walls are not known obstacles */
	if (!(play_info[y][x] & (PLAY_MARK))) return (FALSE);

	/* Run-able grids are not known obstacles */
	if (f_info[f_info[cave_feat[y][x]].mimic].flags1 & (FF1_RUN)) return (FALSE);

	/* Default */
	return (TRUE);
}


/*
 * Hack -- Check for an "unknown corner" (see below)
 */
static int see_nothing(int dir, int y, int x)
{
	/* Get the new location */
	y += ddy[dir];
	x += ddx[dir];

	/* Illegal grids are unknown XXX XXX XXX */
	if (!in_bounds(y, x)) return (TRUE);

	/* Memorized grids are always known */
	if (play_info[y][x] & (PLAY_MARK)) return (FALSE);

	/* Default */
	return (TRUE);
}





/*
 * The running algorithm  -CJS-
 *
 * Basically, once you start running, you keep moving until something
 * interesting happens.  In an enclosed space, you run straight, but
 * you follow corners as needed (i.e. hallways).  In an open space,
 * you run straight, but you stop before entering an enclosed space
 * (i.e. a room with a doorway).  In a semi-open space (with walls on
 * one side only), you run straight, but you stop before entering an
 * enclosed space or an open space (i.e. running along side a wall).
 *
 * All discussions below refer to what the player can see, that is,
 * an unknown wall is just like a normal floor.  This means that we
 * must be careful when dealing with "illegal" grids.
 *
 * No assumptions are made about the layout of the dungeon, so this
 * algorithm works in hallways, rooms, town, destroyed areas, etc.
 *
 * In the diagrams below, the player has just arrived in the grid
 * marked as '@', and he has just come from a grid marked as 'o',
 * and he is about to enter the grid marked as 'x'.
 *
 * Running while confused is not allowed, and so running into a wall
 * is only possible when the wall is not seen by the player.  This
 * will take a turn and stop the running.
 *
 * Several conditions are tracked by the running variables.
 *
 *   p_ptr->run_open_area (in the open on at least one side)
 *   p_ptr->run_break_left (wall on the left, stop if it opens)
 *   p_ptr->run_break_right (wall on the right, stop if it opens)
 *
 * When running begins, these conditions are initialized by examining
 * the grids adjacent to the requested destination grid (marked 'x'),
 * two on each side (marked 'L' and 'R').  If either one of the two
 * grids on a given side is a wall, then that side is considered to
 * be "closed".  Both sides enclosed yields a hallway.
 *
 *    LL     @L
 *    @x      (normal)       RxL   (diagonal)
 *    RR      (east)  R    (south-east)
 *
 * In the diagram below, in which the player is running east along a
 * hallway, he will stop as indicated before attempting to enter the
 * intersection (marked 'x').  Starting a new run in any direction
 * will begin a new hallway run.
 *
 * #.#
 * ##.##
 * o@x..
 * ##.##
 * #.#
 *
 * Note that a minor hack is inserted to make the angled corridor
 * entry (with one side blocked near and the other side blocked
 * further away from the runner) work correctly. The runner moves
 * diagonally, but then saves the previous direction as being
 * straight into the gap. Otherwise, the tail end of the other
 * entry would be perceived as an alternative on the next move.
 *
 * In the diagram below, the player is running east down a hallway,
 * and will stop in the grid (marked '1') before the intersection.
 * Continuing the run to the south-east would result in a long run
 * stopping at the end of the hallway (marked '2').
 *
 * ##################
 * o@x       1
 * ########### ######
 * #2  #
 * #############
 *
 * After each step, the surroundings are examined to determine if
 * the running should stop, and to determine if the running should
 * change direction.  We examine the new current player location
 * (at which the runner has just arrived) and the direction from
 * which the runner is considered to have come.
 *
 * Moving one grid in some direction places you adjacent to three
 * or five new grids (for straight and diagonal moves respectively)
 * to which you were not previously adjacent (marked as '!').
 *
 *   ...!      ...
 *   .o@!  (normal)    .o.!  (diagonal)
 *   ...!  (east)      ..@!  (south east)
 *      !!!
 *
 * If any of the newly adjacent grids are "interesting" (monsters,
 * objects, some terrain features) then running stops.
 *
 * If any of the newly adjacent grids seem to be open, and you are
 * looking for a break on that side, then running stops.
 *
 * If any of the newly adjacent grids do not seem to be open, and
 * you are in an open area, and the non-open side was previously
 * entirely open, then running stops.
 *
 * If you are in a hallway, then the algorithm must determine if
 * the running should continue, turn, or stop.  If only one of the
 * newly adjacent grids appears to be open, then running continues
 * in that direction, turning if necessary.  If there are more than
 * two possible choices, then running stops.  If there are exactly
 * two possible choices, separated by a grid which does not seem
 * to be open, then running stops.  Otherwise, as shown below, the
 * player has probably reached a "corner".
 *
 *    ###     o##
 *    o@x  (normal)   #@!   (diagonal)
 *    ##!  (east)     ##x   (south east)
 *
 * In this situation, there will be two newly adjacent open grids,
 * one touching the player on a diagonal, and one directly adjacent.
 * We must consider the two "option" grids further out (marked '?').
 * We assign "option" to the straight-on grid, and "option2" to the
 * diagonal grid.  For some unknown reason, we assign "check_dir" to
 * the grid marked 's', which may be incorrectly labelled.
 *
 *    ###s
 *    o@x?   (may be incorrect diagram!)
 *    ##!?
 *
 * If both "option" grids are closed, then there is no reason to enter
 * the corner, and so we can cut the corner, by moving into the other
 * grid (diagonally).  If we choose not to cut the corner, then we may
 * go straight, but we pretend that we got there by moving diagonally.
 * Below, we avoid the obvious grid (marked 'x') and cut the corner
 * instead (marked 'n').
 *
 *    ###:       o##
 *    o@x#   (normal)    #@n    (maybe?)
 *    ##n#   (east)      ##x#
 *       ####
 *
 * If one of the "option" grids is open, then we may have a choice, so
 * we check to see whether it is a potential corner or an intersection
 * (or room entrance).  If the grid two spaces straight ahead, and the
 * space marked with 's' are both open, then it is a potential corner
 * and we enter it if requested.  Otherwise, we stop, because it is
 * not a corner, and is instead an intersection or a room entrance.
 *
 *    ###
 *    o@x
 *    ##!#
 *
 * I do not think this documentation is correct.
 */




/*
 * Hack -- allow quick "cycling" through the legal directions
 */
static const byte cycle[] =
{ 1, 2, 3, 6, 9, 8, 7, 4, 1, 2, 3, 6, 9, 8, 7, 4, 1 };

/*
 * Hack -- map each direction into the "middle" of the "cycle[]" array
 */
static const byte chome[] =
{ 0, 8, 9, 10, 7, 0, 11, 6, 5, 4 };



/*
 * Initialize the running algorithm for a new direction.
 *
 * Diagonal Corridor -- allow diaginal entry into corridors.
 *
 * Blunt Corridor -- If there is a wall two spaces ahead and
 * we seem to be in a corridor, then force a turn into the side
 * corridor, must be moving straight into a corridor here. (?)
 *
 * Diagonal Corridor    Blunt Corridor (?)
 *       # #  #
 *       #x# @x#
 *       @p.  p
 */
static void run_init(int dir)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int i, row, col;

	bool deepleft, deepright;
	bool shortleft, shortright;


	/* Save the direction */
	p_ptr->run_cur_dir = dir;

	/* Assume running straight */
	p_ptr->run_old_dir = dir;

	/* Assume looking for open area */
	p_ptr->run_open_area = TRUE;

	/* Assume not looking for breaks */
	p_ptr->run_break_right = FALSE;
	p_ptr->run_break_left = FALSE;

	/* Assume no nearby walls */
	deepleft = deepright = FALSE;
	shortright = shortleft = FALSE;

	/* Find the destination grid */
	row = py + ddy[dir];
	col = px + ddx[dir];

	/* Extract cycle index */
	i = chome[dir];

	/* Check for nearby wall */
	if (see_wall(cycle[i+1], py, px))
	{
		p_ptr->run_break_left = TRUE;
		shortleft = TRUE;
	}

	/* Check for distant wall */
	else if (see_wall(cycle[i+1], row, col))
	{
		p_ptr->run_break_left = TRUE;
		deepleft = TRUE;
	}

	/* Check for nearby wall */
	if (see_wall(cycle[i-1], py, px))
	{
		p_ptr->run_break_right = TRUE;
		shortright = TRUE;
	}

	/* Check for distant wall */
	else if (see_wall(cycle[i-1], row, col))
	{
		p_ptr->run_break_right = TRUE;
		deepright = TRUE;
	}

	/* Looking for a break */
	if (p_ptr->run_break_left && p_ptr->run_break_right)
	{
		/* Not looking for open area */
		p_ptr->run_open_area = FALSE;

		/* Hack -- allow angled corridor entry */
		if (dir & 0x01)
		{
			if (deepleft && !deepright)
			{
				p_ptr->run_old_dir = cycle[i - 1];
			}
			else if (deepright && !deepleft)
			{
				p_ptr->run_old_dir = cycle[i + 1];
			}
		}

		/* Hack -- allow blunt corridor entry */
		else if (see_wall(cycle[i], row, col))
		{
			if (shortleft && !shortright)
			{
				p_ptr->run_old_dir = cycle[i - 2];
			}
			else if (shortright && !shortleft)
			{
				p_ptr->run_old_dir = cycle[i + 2];
			}
		}
	}
}


/*
 * Update the current "run" path
 *
 * Return TRUE if the running should be stopped
 */
static bool run_test(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int prev_dir;
	int new_dir;
	int check_dir = 0;

	int row, col;
	int i, max, inv;
	int option, option2;

	int feat;

	/* No options yet */
	option = 0;
	option2 = 0;

	/* Where we came from */
	prev_dir = p_ptr->run_old_dir;


	/* Range of newly adjacent grids */
	max = (prev_dir & 0x01) + 1;


	/* Look at every newly adjacent square. */
	for (i = -max; i <= max; i++)
	{
		s16b this_o_idx, next_o_idx = 0;


		/* New direction */
		new_dir = cycle[chome[prev_dir] + i];

		/* New location */
		row = py + ddy[new_dir];
		col = px + ddx[new_dir];


		/* Visible monsters abort running */
		if (cave_m_idx[row][col] > 0)
		{
			monster_type *m_ptr = &m_list[cave_m_idx[row][col]];

			/* Visible monster */
			if (m_ptr->ml) return (TRUE);
		}

		/* Visible objects abort running */
		for (this_o_idx = cave_o_idx[row][col]; this_o_idx; this_o_idx = next_o_idx)
		{
			object_type *o_ptr;

			/* Get the object */
			o_ptr = &o_list[this_o_idx];

			/* Get the next object */
			next_o_idx = o_ptr->next_o_idx;

			/* Visible object */
			if ((o_ptr->ident & (IDENT_MARKED)) && !(auto_pickup_never(o_ptr))) return (TRUE);
		}


		/* Assume unknown */
		inv = TRUE;

		/* Check memorized grids */
		if (play_info[row][col] & (PLAY_MARK))
		{
			bool notice = TRUE;

			/* Get feature */
			feat = cave_feat[row][col];

			/* Set mimiced feature */
			feat = f_info[feat].mimic;

			/* Notice feature? */
			notice = ((f_info[feat].flags1 & (FF1_NOTICE)) !=0);

			if ((notice) && (f_info[feat].flags1 & (FF1_MOVE)))
			{
				if ((run_ignore_doors) && (f_info[feat].flags1 & (FF1_DOOR)) &&
					(f_info[feat].flags1 & (FF1_CLOSE)))
				{
					/* Option -- ignore */
					notice = FALSE;
				}

				/* Stairs */
				if ((run_ignore_stairs) && (f_info[feat].flags1 & (FF1_STAIRS)))
				{
					/* Option -- ignore */
					notice = FALSE;
				}

				/* Unusual floors */
				if ((run_ignore_floors) && (f_info[feat].flags1 & (FF1_FLOOR)))
				{
					/* Option -- ignore */
					notice = FALSE;
				}

			}

			/* Interesting feature */
			if (notice) return (TRUE);

			/* The grid is "visible" */
			inv = FALSE;
		}

		/* Analyze unknown grids and floors */
		if (inv || cave_floor_bold(row, col))
		{
			/* Looking for open area */
			if (p_ptr->run_open_area)
			{
				/* Nothing */
			}

			/* The first new direction. */
			else if (!option)
			{
				option = new_dir;
			}

			/* Three new directions. Stop running. */
			else if (option2)
			{
				return (TRUE);
			}

			/* Two non-adjacent new directions.  Stop running. */
			else if (option != cycle[chome[prev_dir] + i - 1])
			{
				return (TRUE);
			}

			/* Two new (adjacent) directions (case 1) */
			else if (new_dir & 0x01)
			{
				check_dir = cycle[chome[prev_dir] + i - 2];
				option2 = new_dir;
			}

			/* Two new (adjacent) directions (case 2) */
			else
			{
				check_dir = cycle[chome[prev_dir] + i + 1];
				option2 = option;
				option = new_dir;
			}
		}

		/* Obstacle, while looking for open area */
		else
		{
			if (p_ptr->run_open_area)
			{
				if (i < 0)
				{
					/* Break to the right */
					p_ptr->run_break_right = TRUE;
				}

				else if (i > 0)
				{
					/* Break to the left */
					p_ptr->run_break_left = TRUE;
				}
			}
		}
	}


	/* Looking for open area */
	if (p_ptr->run_open_area)
	{
		/* Hack -- look again */
		for (i = -max; i < 0; i++)
		{
			new_dir = cycle[chome[prev_dir] + i];

			row = py + ddy[new_dir];
			col = px + ddx[new_dir];

			/* Get feature */
			feat = cave_feat[row][col];

			/* Get mimiced feature */
			feat = f_info[feat].mimic;

			/* Unknown grid or non-wall */
			/* Was: cave_floor_bold(row, col) */
			if (!(play_info[row][col] & (PLAY_MARK)) ||
			    (!(f_info[feat].flags1 & (FF1_WALL))) )
			{
				/* Looking to break right */
				if (p_ptr->run_break_right)
				{
					return (TRUE);
				}
			}

			/* Obstacle */
			else
			{
				/* Looking to break left */
				if (p_ptr->run_break_left)
				{
					return (TRUE);
				}
			}
		}

		/* Hack -- look again */
		for (i = max; i > 0; i--)
		{
			new_dir = cycle[chome[prev_dir] + i];

			row = py + ddy[new_dir];
			col = px + ddx[new_dir];

			/* Get feature */
			feat = cave_feat[row][col];

			/* Get mimiced feature */
			feat = f_info[feat].mimic;

			/* Unknown grid or non-wall */
			/* Was: cave_floor_bold(row, col) */
			if (!(play_info[row][col] & (PLAY_MARK)) ||
			    (!(f_info[feat].flags1 & (FF1_WALL))))
			{
				/* Looking to break left */
				if (p_ptr->run_break_left)
				{
					return (TRUE);
				}
			}

			/* Obstacle */
			else
			{
				/* Looking to break right */
				if (p_ptr->run_break_right)
				{
					return (TRUE);
				}
			}
		}
	}


	/* Not looking for open area */
	else
	{
		/* No options */
		if (!option)
		{
			return (TRUE);
		}

		/* One option */
		else if (!option2)
		{
			/* Primary option */
			p_ptr->run_cur_dir = option;

			/* No other options */
			p_ptr->run_old_dir = option;
		}

		/* Two options, examining corners */
		else if (run_use_corners && !run_cut_corners)
		{
			/* Primary option */
			p_ptr->run_cur_dir = option;

			/* Hack -- allow curving */
			p_ptr->run_old_dir = option2;
		}

		/* Two options, pick one */
		else
		{
			/* Get next location */
			row = py + ddy[option];
			col = px + ddx[option];

			/* Don't see that it is closed off. */
			/* This could be a potential corner or an intersection. */
			if (!see_wall(option, row, col) ||
			    !see_wall(check_dir, row, col))
			{
				/* Can not see anything ahead and in the direction we */
				/* are turning, assume that it is a potential corner. */
				if (run_use_corners &&
				    see_nothing(option, row, col) &&
				    see_nothing(option2, row, col))
				{
					p_ptr->run_cur_dir = option;
					p_ptr->run_old_dir = option2;
				}

				/* STOP: we are next to an intersection or a room */
				else
				{
					return (TRUE);
				}
			}

			/* This corner is seen to be enclosed; we cut the corner. */
			else if (run_cut_corners)
			{
				p_ptr->run_cur_dir = option2;
				p_ptr->run_old_dir = option2;
			}

			/* This corner is seen to be enclosed, and we */
			/* deliberately go the long way. */
			else
			{
				p_ptr->run_cur_dir = option;
				p_ptr->run_old_dir = option2;
			}
		}
	}


	/* About to hit a known wall, stop */
	if (see_stop(p_ptr->run_cur_dir, py, px))
	{
		return (TRUE);
	}


	/* Failure */
	return (FALSE);
}



/*
 * Take one step along the current "run" path
 *
 * Called with a real direction to begin a new run, and with zero
 * to continue a run in progress.
 */
void run_step(int dir)
{
	/* Start run */
	if (dir)
	{
		/* Initialize */
		run_init(dir);

		/* Hack -- Set the run counter */
		p_ptr->running = (p_ptr->command_arg ? p_ptr->command_arg : 1000);

		/* Hack -- not running with pathfind */
		p_ptr->running_withpathfind = FALSE;

		/* Calculate torch radius */
		p_ptr->update |= (PU_TORCH);
	}

	/* Continue run */
	else
	{
		if (!p_ptr->running_withpathfind)
		{
			/* Update run */
			if (run_test())
			{
				/* Disturb */
				disturb(0, 0);
	
				/* Done */
				return;
			}
		}
		else
		{
			/* Abort if we have finished */
			if (pf_result_index < 0)
			{
				disturb(0, 0);
				p_ptr->running_withpathfind = FALSE;
				return;
			}
			/* Abort if we would hit a wall */
			else if (pf_result_index == 0)
			{
				int y, x;

				/* Get next step */
				y = p_ptr->py + ddy[pf_result[pf_result_index] - '0'];
				x = p_ptr->px + ddx[pf_result[pf_result_index] - '0'];

				/* Known wall */
				if ((play_info[y][x] & (PLAY_MARK)) && !is_valid_pf(y,x))
				{
					disturb(0,0);
					p_ptr->running_withpathfind = FALSE;
					return;
				}
			}
			/* Hack -- walking stick lookahead.
			 *
			 * If the player has computed a path that is going to end up in a wall,
			 * we notice this and convert to a normal run. This allows us to click
			 * on unknown areas to explore the map.
			 *
			 * We have to look ahead two, otherwise we don't know which is the last
			 * direction moved and don't initialise the run properly.
			 */
			else if (pf_result_index > 0)
			{
				int y, x;

				/* Get next step */
				y = p_ptr->py + ddy[pf_result[pf_result_index] - '0'];
				x = p_ptr->px + ddx[pf_result[pf_result_index] - '0'];

				/* Known wall */
				if ((play_info[y][x] & (PLAY_MARK)) && !is_valid_pf(y,x))
				{
					disturb(0,0);
					p_ptr->running_withpathfind = FALSE;
					return;
				}

				/* Get step after */
				y = y + ddy[pf_result[pf_result_index-1] - '0'];
				x = x + ddx[pf_result[pf_result_index-1] - '0'];

				/* Known wall */
				if ((play_info[y][x] & (PLAY_MARK)) && !is_valid_pf(y,x))
				{
					p_ptr->running_withpathfind = FALSE;

					run_init(pf_result[pf_result_index] - '0');
				}
			}

			p_ptr->run_cur_dir = pf_result[pf_result_index--] - '0';

			/* Hack -- allow easy_alter */
			p_ptr->command_dir = p_ptr->run_cur_dir;
		}
	}

	/* Decrease counter */
	p_ptr->running--;

	/* Take time */
	p_ptr->energy_use = 100;

	/* Rest the player */
	/* XXX Should never be able to run on filled terrain */
	set_rest(p_ptr->rest + PY_REST_RATE - p_ptr->tiring);

	/* Move the player */
	move_player(p_ptr->run_cur_dir, FALSE);

	return;
}
