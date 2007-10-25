/* File: spells1.c */

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
 * Translate player racial or equipment flags into monster smart flags
 */
u32b player_smart_flags(u32b f1, u32b f2, u32b f3, u32b f4)
{
	u32b tmp = 0L;

	(void)f1;

	/* Know immunities */
	if (f2 & (TR2_IM_ACID)) tmp |= (SM_IMM_ACID);
	if (f2 & (TR2_IM_ELEC)) tmp |= (SM_IMM_ELEC);
	if (f2 & (TR2_IM_FIRE)) tmp |= (SM_IMM_FIRE);
	if (f2 & (TR2_IM_COLD)) tmp |= (SM_IMM_COLD);
	if (f4 & (TR4_IM_POIS)) tmp |= (SM_IMM_POIS);

	/* Know protections */
	if (f3 & (TR3_FREE_ACT)) tmp |= (SM_FREE_ACT);

	/* Know resistances */
	if (f2 & (TR2_RES_ACID)) tmp |= (SM_RES_ACID);
	if (f2 & (TR2_RES_ELEC)) tmp |= (SM_RES_ELEC);
	if (f2 & (TR2_RES_FIRE)) tmp |= (SM_RES_FIRE);
	if (f2 & (TR2_RES_COLD)) tmp |= (SM_RES_COLD);
	if (f2 & (TR2_RES_POIS)) tmp |= (SM_RES_POIS);
	if (f2 & (TR2_RES_FEAR)) tmp |= (SM_RES_FEAR);
	if (f2 & (TR2_RES_LITE)) tmp |= (SM_RES_LITE);
	if (f2 & (TR2_RES_DARK)) tmp |= (SM_RES_DARK);
	if (f2 & (TR2_RES_BLIND)) tmp |= (SM_RES_BLIND);
	if (f2 & (TR2_RES_CONFU)) tmp |= (SM_RES_CONFU);
	if (f2 & (TR2_RES_SOUND)) tmp |= (SM_RES_SOUND);
	if (f2 & (TR2_RES_SHARD)) tmp |= (SM_RES_SHARD);
	if (f2 & (TR2_RES_NEXUS)) tmp |= (SM_RES_NEXUS);
	if (f2 & (TR2_RES_NETHR)) tmp |= (SM_RES_NETHR);
	if (f2 & (TR2_RES_CHAOS)) tmp |= (SM_RES_CHAOS);
	if (f2 & (TR2_RES_DISEN)) tmp |= (SM_RES_DISEN);

	return(tmp);
}


/*
 * Translate monster race flags into monster smart flags
 */
u32b monster_smart_flags(int m_idx)
{
	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	
	u32b tmp = 0L;

	/* Know immunities */
	if (r_ptr->flags3 & (RF3_IM_ACID)) tmp |= (SM_IMM_ACID);
	if (r_ptr->flags3 & (RF3_IM_ELEC)) tmp |= (SM_IMM_ELEC);
	if (r_ptr->flags3 & (RF3_IM_FIRE)) tmp |= (SM_IMM_FIRE);
	if (r_ptr->flags3 & (RF3_IM_COLD)) tmp |= (SM_IMM_COLD);
	if (r_ptr->flags3 & (RF3_IM_POIS)) tmp |= (SM_IMM_POIS);

	/* Know protections */
	if (r_ptr->flags9 & (RF9_NO_SLOW)) tmp |= (SM_FREE_ACT);

	/* Known resistances */
	if (m_ptr->oppose_elem) tmp |= (SM_OPP_ACID | SM_OPP_FIRE | SM_OPP_POIS | SM_OPP_ELEC | SM_OPP_COLD);

	/* Know resistances */
	if (r_ptr->flags3 & (RF3_NO_FEAR)) tmp |= (SM_RES_FEAR);
	if (r_ptr->flags9 & (RF9_RES_LITE)) tmp |= (SM_RES_LITE);
	if (r_ptr->flags9 & (RF9_RES_DARK)) tmp |= (SM_RES_DARK);
	if (r_ptr->flags9 & (RF9_RES_BLIND)) tmp |= (SM_RES_BLIND);
	if (r_ptr->flags3 & (RF3_NO_CONF)) tmp |= (SM_RES_CONFU);
	if (r_ptr->flags3 & (RF3_NO_STUN)) tmp |= (SM_RES_SOUND);
	if (r_ptr->flags9 & (RF9_NO_CUTS)) tmp |= (SM_RES_SHARD);
	if (r_ptr->flags3 & (RF3_RES_NEXUS)) tmp |= (SM_RES_NEXUS);
	if (r_ptr->flags3 & (RF3_RES_NETHR)) tmp |= (SM_RES_NETHR);
	if (r_ptr->flags9 & (RF9_RES_CHAOS)) tmp |= (SM_RES_CHAOS);
	if (r_ptr->flags3 & (RF3_RES_DISEN)) tmp |= (SM_RES_DISEN);

	return(tmp);
}


/*
 * Notice that the player has equipment flags.
 * This is similar to equip_can_flags except we allow the casting
 * monster to also notice this ability.
 */
bool player_can_flags(int who, u32b f1, u32b f2, u32b f3, u32b f4)
{
	/* Player notices ability */
	equip_can_flags(f1, f2, f3, f4);

	/* Monster notices ability */
	if (who > SOURCE_MONSTER_START)
	{
	 	monster_type *m_ptr = &m_list[who];
	 	monster_race *r_ptr = &r_info[m_ptr->r_idx];

		u32b smart;

	 	/* Too stupid to learn anything */
 		if (r_ptr->flags2 & (RF2_STUPID)) return (FALSE);

	 	/* Not intelligent, only learn sometimes */
 		if (!(r_ptr->flags2 & (RF2_SMART)) && (rand_int(100) < 50)) return (FALSE);

		/* Get the flags */
		smart = player_smart_flags(f1, f2, f3, f4);

		/* Learn the flags */
		m_ptr->smart |= smart;

		/* Tell the allies */
		return (tell_allies_player_can(m_ptr->fy, m_ptr->fx, smart));
	}
	
	return (FALSE);
}


/*
 * Notice that the player does not have equipment flags.
 * This is similar to equip_not_flags except we allow the casting
 * monster to also notice this ability.
 */
bool player_not_flags(int who, u32b f1, u32b f2, u32b f3, u32b f4)
{
	/* Player notices ability */
	equip_not_flags(f1, f2, f3, f4);

	/* Monster notices ability */
	if (who > SOURCE_MONSTER_START)
	{
	 	monster_type *m_ptr = &m_list[who];
	 	monster_race *r_ptr = &r_info[m_ptr->r_idx];

		u32b smart;

	 	/* Too stupid to learn anything */
 		if (r_ptr->flags2 & (RF2_STUPID)) return (FALSE);

	 	/* Not intelligent, only learn sometimes */
 		if (!(r_ptr->flags2 & (RF2_SMART)) && (rand_int(100) < 50)) return (FALSE);

		/* Learn the flags */
		smart = player_smart_flags(f1, f2, f3, f4);

		/* Learn the flags */
		m_ptr->smart &= ~(smart);

		/* Tell the allies */
		return (tell_allies_player_not(m_ptr->fy, m_ptr->fx, smart));
	}
	
	return (FALSE);
}


/*
 * Update monster's knowledge of the player save.
 */
bool update_smart_save(int who, bool saved)
{
	/* Monster notices ability */
	if (who > SOURCE_MONSTER_START)
	{
		u32b smart = 0L;

	 	monster_type *m_ptr = &m_list[who];
	 	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	 	/* Too stupid to learn anything */
 		if (r_ptr->flags2 & (RF2_STUPID)) return (FALSE);

	 	/* Not intelligent, only learn sometimes */
 		if (!(r_ptr->flags2 & (RF2_SMART)) && (rand_int(100) < 50)) return (FALSE);

		/* Learn saving throw */
		if (p_ptr->skill_sav >= 75) smart |= (SM_GOOD_SAVE);
		if (p_ptr->skill_sav >= 100) smart |= (SM_PERF_SAVE);

		if (smart)
		{
			/* Learn the flags */
			m_ptr->smart |= smart;

			/* Notice lack of perfect save */
			if ((smart & (SM_PERF_SAVE)) == 0)
			{
				m_ptr->smart &= ~(SM_PERF_SAVE);
				if (!saved && tell_allies_player_not(m_ptr->fy, m_ptr->fx, SM_PERF_SAVE)) return (TRUE);
			}

			/* Tell the allies */
			return (saved && tell_allies_player_can(m_ptr->fy, m_ptr->fx, smart));
		}
		else
		{
			/* Learn the flags */
			m_ptr->smart &= ~(SM_GOOD_SAVE | SM_PERF_SAVE);

			/* Tell the allies */
			return (!saved && tell_allies_player_not(m_ptr->fy, m_ptr->fx, SM_GOOD_SAVE | SM_PERF_SAVE));			
		}
	}
	
	return (FALSE);
}


/*
 * Update monster's knowledge of a flag. This is used when it is known a player has the ability,
 * primarily for when a player has a temporary stat.
 */
bool update_smart_learn(int who, u32b flag)
{
	/* Monster notices ability */
	if (who > SOURCE_MONSTER_START)
	{
	 	monster_type *m_ptr = &m_list[who];
	 	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	 	/* Too stupid to learn anything */
 		if (r_ptr->flags2 & (RF2_STUPID)) return (FALSE);

	 	/* Not intelligent, only learn sometimes */
 		if (!(r_ptr->flags2 & (RF2_SMART)) && (rand_int(100) < 50)) return (FALSE);

		/* Learn ability */
		m_ptr->smart |= flag;
		
		/* Tell the allies */
		return (tell_allies_player_can(m_ptr->fy, m_ptr->fx, flag));
	}
	
	return (FALSE);
}


/*
 * Forgot monster's knowledge of a flag. This is used when it is known a player has the lost the ability,
 * primarily for when a player has a temporary stat.
 */
bool update_smart_forget(int who, u32b flag)
{
	/* Monster notices ability */
	if (who > SOURCE_MONSTER_START)
	{
	 	monster_type *m_ptr = &m_list[who];
	 	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	 	/* Too stupid to learn anything */
 		if (r_ptr->flags2 & (RF2_STUPID)) return (FALSE);

	 	/* Not intelligent, only learn sometimes */
 		if (!(r_ptr->flags2 & (RF2_SMART)) && (rand_int(100) < 50)) return (FALSE);

		/* Learn ability */
		m_ptr->smart &= ~(flag);

		/* Tell the allies */
		return (tell_allies_player_not(m_ptr->fy, m_ptr->fx, flag));
	}
	
	return (FALSE);
}


/*
 * Fully update monster's knowledge of the player.
 */
void update_smart_cheat(int m_idx)
{
	monster_type *m_ptr = &m_list[m_idx];

	/* Know weirdness */
	if (!p_ptr->msp) m_ptr->smart |= (SM_IMM_MANA);
	if (p_ptr->skill_sav >= 75) m_ptr->smart |= (SM_GOOD_SAVE);
	if (p_ptr->skill_sav >= 100) m_ptr->smart |= (SM_PERF_SAVE);

	/* Know temporary effects */
	if (p_ptr->oppose_acid) m_ptr->smart |= (SM_OPP_ACID);
	if (p_ptr->oppose_elec) m_ptr->smart |= (SM_OPP_ELEC);
	if (p_ptr->oppose_fire) m_ptr->smart |= (SM_OPP_FIRE);
	if (p_ptr->oppose_cold) m_ptr->smart |= (SM_OPP_COLD);
	if (p_ptr->oppose_pois) m_ptr->smart |= (SM_OPP_POIS);
	if (p_ptr->free_act) m_ptr->smart |= (SM_FREE_ACT);
	if (p_ptr->hero) m_ptr->smart |= (SM_OPP_FEAR);
	if (p_ptr->shero) m_ptr->smart |= (SM_OPP_FEAR);

	/* Get other flags */
	m_ptr->smart |= player_smart_flags(p_ptr->cur_flags1, p_ptr->cur_flags2, p_ptr->cur_flags3, p_ptr->cur_flags4);

	return;
}



/*
 * Fully update monster's knowledge of the player racial abilities.
 * Used by all smart monsters and humanoids of various types.
 */
void update_smart_racial(int m_idx)
{
	monster_type *m_ptr = &m_list[m_idx];

	player_race *shape_ptr = &p_info[p_ptr->pshape];

	/* Get race flags */
	m_ptr->smart |= player_smart_flags(rp_ptr->flags1, rp_ptr->flags2, rp_ptr->flags3, rp_ptr->flags4);

	/* Get shape flags */
	m_ptr->smart |= player_smart_flags(shape_ptr->flags1, shape_ptr->flags2, shape_ptr->flags3, shape_ptr->flags4);

	return;
}


/*
 * Teleport hook: Destination must be projectable from origin
 */
bool teleport_project_hook(const int oy, const int ox, const int ny, const int nx)
{
	/* Must have line of projection between origin and target */
	if (generic_los(oy, ox, ny, nx, CAVE_XLOF)) return (TRUE);
	
	return (FALSE);
}


/*
 * Teleport hook: Destination must be in darkness
 */
bool teleport_darkness_hook(const int oy, const int ox, const int ny, const int nx)
{
	(void)oy;
	(void)ox;

	/* Daylight or permanent light */
	if ((cave_info[ny][nx] & (CAVE_DLIT | CAVE_GLOW)) != 0) return (FALSE);
	
	return (TRUE);
}


/*
 * Teleport hook: Destination must be adjacent to water or living terrain
 */
bool teleport_nature_hook(const int oy, const int ox, const int ny, const int nx)
{
	int y, x;
	
	(void)oy;
	(void)ox;

	for (y = ny - 1; y <= ny + 1; y++)
	{
		for (x = nx - 1; x <= nx + 1; x++)
		{
			/* Check for valid grid */
			if (!in_bounds_fully(y,x)) continue;
					
			/* Water or living feature */
			if (((f_info[cave_feat[y][x]].flags2 & (FF2_WATER)) != 0)
				|| ((f_info[cave_feat[y][x]].flags3 & (FF3_LIVING)) != 0))
			{
				return (TRUE);
			}
		}
	}		

	return (FALSE);
}


/*
 * Teleport hook: Destination must be adjacent to fire or lava
 */
bool teleport_fiery_hook(const int oy, const int ox, const int ny, const int nx)
{
	int y, x;
	
	(void)oy;
	(void)ox;

	for (y = ny - 1; y <= ny + 1; y++)
	{
		for (x = nx - 1; x <= nx + 1; x++)
		{
			/* Check for valid grid */
			if (!in_bounds_fully(y,x)) continue;
					
			/* Lava or burning feature */
			if ((f_info[cave_feat[y][x]].blow.effect == GF_LAVA) ||
				(f_info[cave_feat[y][x]].blow.effect == GF_FIRE) ||
				(f_info[cave_feat[y][x]].blow.effect == GF_SMOKE))				
			{
				return (TRUE);
			}
		}
	}		

	return (FALSE);
}




/*
 * Teleport a monster, normally up to "dis" grids away.
 *
 * Attempt to move the monster at least "dis/2" grids away.
 *
 * But allow variation to prevent infinite loops.
 */
void teleport_away(int m_idx, int dis)
{
	int ny, nx, oy, ox, d, i, min;

	bool look = TRUE;

	monster_type *m_ptr = &m_list[m_idx];

	/* Paranoia */
	if (!m_ptr->r_idx) return;

	/* Save the old location */
	ny = oy = m_ptr->fy;
	nx = ox = m_ptr->fx;

	/* Handle anti-teleport room */
	if (room_has_flag(oy, ox, ROOM_ICKY))
	{
		/* Hack - reduce distance */
		if (dis > 20) dis = 20;
	}

	/* Minimum distance */
	min = dis / 2;

	/* Look until done */
	while (look)
	{
		/* Verify max distance */
		if (dis > 200) dis = 200;

		/* Try several locations */
		for (i = 0; i < 500; i++)
		{
			/* Pick a (possibly illegal) location */
			while (1)
			{
				ny = rand_spread(oy, dis);
				nx = rand_spread(ox, dis);
				d = distance(oy, ox, ny, nx);
				if ((d >= min) && (d <= dis)) break;
			}

			/* Ignore illegal locations */
			if (!in_bounds_fully(ny, nx)) continue;

			/* Require safe passable space for monster */
			if (place_monster_here(ny, nx, m_ptr->r_idx) <= MM_FAIL) continue;

			/* Don't allow teleporting into other monster or player */
			if (cave_m_idx[ny][nx]) continue;

			/* Hack -- no teleport onto glyph of warding */
			if (f_info[cave_feat[ny][nx]].flags1 & (FF1_GLYPH)) continue;

			/* Check if teleportation restricted */
			if (teleport_hook && !(*teleport_hook)(oy, ox, ny, nx)) continue;

			/* Check room */
			if (cave_info[oy][ox] & (CAVE_ROOM))
			{
				/* Don't allow teleporting out of vaults once inside */
				if (room_has_flag(oy, ox, ROOM_ICKY))
				{
					if (!(cave_info[ny][nx] & (CAVE_ROOM))) continue;
					if (dun_room[oy/BLOCK_HGT][ox/BLOCK_WID] != dun_room[ny/BLOCK_HGT][nx/BLOCK_WID]) continue;
				}

				/* Don't allow teleporting into vaults */
				else if (room_has_flag(ny, nx, ROOM_ICKY)) continue;
			}

			/* This grid looks good */
			look = FALSE;

			/* Stop looking */
			break;
		}

		/* Paranoia -- have exhausted looking for space in dungeon */
		if ((dis >= 200) && (min <= 0))
		{
			/* Sound */
			sound(MSG_TPOTHER);
			
 			/* Delete monster instead */
			delete_monster_idx(m_idx);
			
			/* Clear hook */
			teleport_hook = NULL;

			return;
		}

		/* Increase the maximum distance */
		dis = dis * 2;

		/* Decrease the minimum distance */
		min = min / 2;
		
	}

	/* Sound */
	sound(MSG_TPOTHER);

	/* Swap the monsters */
	monster_swap(oy, ox, ny, nx);
	
	/* Clear hook */
	teleport_hook = NULL;
}


/*
 * Teleport the player to a location up to "dis" grids away.
 *
 * If no such spaces are readily available, the distance may increase.
 * Try very hard to move the player at least a quarter that distance.
 */
void teleport_player(int dis)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int d, i, min, y, x;

	bool look = TRUE;

	int tries = 0;

	/* Paranoia */
	if (dis <= 0) return;

	/* Initialize */
	y = py;
	x = px;

	/* Handle anti-teleport room */
	if (room_has_flag(py, px, ROOM_ICKY))
	{
		/* Hack - reduce distance */
		if (dis > 20)
		{
			/* Warn the player */
			msg_print("Oops! It appears this area is teleport proof.");

			dis = 20;
		}
	}

	/* Minimum distance */
	min = dis / 2;

	/* Look until done */
	while (look)
	{
		/* Verify max distance */
		if (dis > 200) dis = 200;

		/* Try several locations */
		for (i = 0; i < 500; i++)
		{
			/* Pick a (possibly illegal) location */
			while (1)
			{
				y = rand_spread(py, dis);
				x = rand_spread(px, dis);
				d = distance(py, px, y, x);
				if ((d >= min) && (d <= dis)) break;
			}

			/* Ignore illegal locations */
			if (!in_bounds_fully(y, x)) continue;
			
			/* Check if teleportation restricted */
			if (teleport_hook && !(*teleport_hook)(py, px, y, x)) continue;

			/* Count tries */
			tries++;

			/* Require "start" floor space if not looking hard */
			if ((!cave_start_bold(y, x)) && (tries < 1000)) continue;

			/* Permit unsafe empty locations if looking hard */
			else if (!(cave_empty_bold(y,x))) continue;

			/* Don't allow teleporting out of vaults once inside */
			if ((cave_info[py][px] & (CAVE_ROOM)) && (room_has_flag(py, px, ROOM_ICKY)) && (tries < 2000))
			{
				if (!(cave_info[y][x] & (CAVE_ROOM))) continue;
				if (dun_room[py/BLOCK_HGT][px/BLOCK_WID] != dun_room[y/BLOCK_HGT][x/BLOCK_WID]) continue;
			}

			/* Don't allow teleporting into vaults if outside */
			else if ((cave_info[y][x] & (CAVE_ROOM)) && (room_has_flag(y, x, ROOM_ICKY)) && (tries < 2000))
			{
				continue;
			}


			/* This grid looks good */
			look = FALSE;

			/* Stop looking */
			break;
		}

		/* Paranoia -- have exhausted looking for space in dungeon */
		if ((dis >= 200) && (min <= 0))
		{
			/* Sound */
			sound(MSG_TPOTHER);
			
			/* Set dodging -- 'random direction' */
			p_ptr->dodging = rand_int(8);
			
			/* Redraw state */
			p_ptr->redraw |= (PR_STATE);

			/* Handle stuff XXX XXX XXX */
			handle_stuff();
			
			/* Clear hook */
			teleport_hook = NULL;

			return;
		}
		
		/* Increase the maximum distance */
		dis = dis * 2;

		/* Decrease the minimum distance */
		min = min / 2;
	}

	/* Sound */
	sound(MSG_TELEPORT);

	/* Move player */
	monster_swap(py, px, y, x);

	/* Set dodging -- 'random direction' */
	p_ptr->dodging = rand_int(8);

	/* Redraw state */
	p_ptr->redraw |= (PR_STATE);

	/* Handle stuff XXX XXX XXX */
	handle_stuff();
	
	/* Clear hook */
	teleport_hook = NULL;
}



/*
 * Teleport player to a grid near the given location
 *
 * This function is slightly obsessive about correctness.
 * This function allows teleporting into vaults (!)
 */
void teleport_player_to(int ny, int nx)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y, x;

	int dis = 0, ctr = 0;

	/* Initialize */
	y = py;
	x = px;

	/* Find a usable location */
	while (1)
	{
		/* Occasionally advance the distance */
		if (ctr++ > (4 * dis * dis + 4 * dis + 1))
		{
			ctr = 0;
			dis++;
		}

		/* Pick a nearby legal location */
		while (1)
		{
			y = rand_spread(ny, dis);
			x = rand_spread(nx, dis);
			if (in_bounds_fully(y, x)) break;
		}

		/* Accept "naked" floor grids */
		if (!cave_naked_bold(y, x)) continue;

		/* Require "start" floor space */
		if (!cave_start_bold(y, x)) continue;

		break;
	}

	/* Sound */
	sound(MSG_TELEPORT);

	/* Move player */
	monster_swap(py, px, y, x);

	/* Handle stuff XXX XXX XXX */
	handle_stuff();
}


/*
 * Teleport monster to a grid near the given location.  This function is
 * used in the monster spell "TELE_SELF_TO", to allow monsters both to
 * suddenly jump near the character, and to make them "dance" around the
 * character.
 *
 * Usually, monster will teleport to a grid that is not more than 4
 * squares away from the given location, and not adjacent to the given
 * location.  These restrictions are relaxed if necessary.
 *
 * This function allows teleporting into vaults.
 */
void teleport_towards(int oy, int ox, int ny, int nx)
{
	int y, x;

	int dist;
	int ctr = 0;
	int min = 2, max = 4;

	/* Find a usable location */
	while (TRUE)
	{
		/* Pick a nearby legal location */
		while (TRUE)
		{
			y = rand_spread(ny, max);
			x = rand_spread(nx, max);
			if (in_bounds_fully(y, x)) break;
		}

		/* Consider all empty grids */
		if (cave_empty_bold(y, x))
		{
			/*Don't allow monster to teleport onto glyphs*/
			if (f_info[cave_feat[y][x]].flags1 & (FF1_GLYPH)) continue;

			/* Calculate distance between target and current grid */
			dist = distance(ny, nx, y, x);

			/* Accept grids that are the right distance away. */
			if ((dist >= min) && (dist <= max)) break;
		}

		/* Occasionally relax the constraints */
		if (++ctr > 15)
		{
			ctr = 0;

			max++;
			if (max > 5) min = 0;
		}
	}

	/* Sound (assumes monster is moving) */
	sound(MSG_TPOTHER);

	/* Move monster */
	monster_swap(oy, ox, y, x);

	/* Handle stuff XXX XXX XXX */
	handle_stuff();
}


/*
 * Creates terrain that will move the player one level up or down 
 * (random when legal)
 */
void teleport_player_level(void)
{
	/* No effect */
	if (adult_ironman)
	{
		msg_print("Nothing happens.");
		return;
	}

	/* Try downstairs */
	if (rand_int(100) < 50)
	{
		/* Crack open the floor */
		place_random_stairs(p_ptr->py, p_ptr->px, FEAT_CRUMBLING_FLOOR);
	}
	else
	{
		/* Crack open the ceiling */
		place_random_stairs(p_ptr->py, p_ptr->px, FEAT_TREMBLING_RUBBLE);
	}

	/* Message depends on actual terrain */
	switch (cave_feat[p_ptr->py][p_ptr->px])
	{
		case FEAT_CRUMBLING_FLOOR:
			message(MSG_TPLEVEL, 0, "The floor beneath you collapses.");
			break;
		case FEAT_TREMBLING_RUBBLE:
			message(MSG_TPLEVEL, 0, "Rubble falls down from the ceiling.");
			break;
	}
}


/*
 * Draw some projections in multi-hued colors.
 * -TY-, -EB-
 */
static byte mh_attr(void)
{
	switch (randint(9))
	{
		case 1:  return (TERM_RED);
		case 2:  return (TERM_GREEN);
		case 3:  return (TERM_BLUE);
		case 4:  return (TERM_YELLOW);
		case 5:  return (TERM_ORANGE);
		case 6:  return (TERM_VIOLET);
		case 7:  return (TERM_L_RED);
		case 8:  return (TERM_L_GREEN);
		case 9:  return (TERM_L_BLUE);
	}

	return (TERM_WHITE);
}

static byte acid_color(void)
{
	switch (rand_int(3))
	{
		case 0: case 1: return (TERM_GREEN);
		case 2: return (TERM_L_GREEN);
	}
	return (TERM_WHITE);
}

static byte elec_color(void)
{
	switch (rand_int(3))
	{
		case 0: case 1: return (TERM_YELLOW);
		case 2: return (TERM_L_BLUE);
	}
	return (TERM_WHITE);
}

static byte fire_color(void)
{
	switch (rand_int(3))
	{
		case 0: case 1: return (TERM_ORANGE);
		case 2: return (TERM_L_DARK);
	}
	return (TERM_WHITE);
}

static byte cold_color(void)
{
	switch (rand_int(3))
	{
		case 0: case 1: return (TERM_WHITE);
		case 2: return (TERM_L_WHITE);
	}
	return (TERM_WHITE);
}

static byte pois_color(void)
{
	switch (rand_int(3))
	{
		case 0: case 1: return (TERM_VIOLET);
		case 2: return (TERM_GREEN);
	}
	return (TERM_WHITE);
}

static byte water_color(void)
{
	switch (rand_int(3))
	{
		case 0: case 1: return (TERM_BLUE);
		case 2: return (TERM_WHITE);
	}
	return (TERM_WHITE);
}

static byte lava_color(void)
{
	switch (rand_int(3))
	{
		case 0: case 1: return (TERM_RED);
		case 2: return (TERM_ORANGE);
	}
	return (TERM_WHITE);
}

static byte plasma_color(void)
{
	switch (rand_int(4))
	{
		case 0: case 1: return (TERM_WHITE);
		case 2: return (TERM_L_RED);
		case 3: return (TERM_YELLOW);
	}

	return (TERM_WHITE);
}

static byte hellfire_color(void)
{
	switch (rand_int(4))
	{
		case 0: case 1: return (TERM_RED);
		case 2: return (TERM_L_RED);
		case 3: return (TERM_L_DARK);
	}

	return (TERM_WHITE);
}

static byte ice_color(void)
{
	switch (rand_int(3))
	{
		case 0: case 1: return (TERM_WHITE);
		case 2: return (TERM_L_BLUE);
	}

	return (TERM_WHITE);
}

static byte lite_color(void)
{
	switch (rand_int(4))
	{
		case 0: case 1: case 2: return (TERM_YELLOW);
		case 3: return (TERM_ORANGE);
	}

	return (TERM_WHITE);
}

static byte confu_color(void)
{
	switch (rand_int(5))
	{
		case 0: case 1: case 2: return (TERM_L_UMBER);
		case 3: return (TERM_UMBER);
		case 4: return (TERM_WHITE);
	}

	return (TERM_WHITE);
}

static byte grav_color(void)
{
	switch (rand_int(4))
	{
		case 0: case 1: return (TERM_DARK);
		case 2: return (TERM_L_DARK);
		case 3: return (TERM_SLATE);
	}

	return (TERM_WHITE);
}

static byte storm_color(void)
{
	switch (rand_int(7))
	{
		case 0: case 1: case 2: return (TERM_SLATE);
		case 3: return (TERM_BLUE);
		case 4: return (TERM_L_BLUE);
		case 5: return (TERM_YELLOW);
		case 6: return (TERM_WHITE);
	}

	return (TERM_WHITE);
}

static byte meteor_color(void)
{
	switch (rand_int(6))
	{
		case 0: case 1: return (TERM_WHITE);
		case 2: return (TERM_L_BLUE);
		case 3: return (TERM_RED);
		case 4: return (TERM_ORANGE);
		case 5: return (TERM_YELLOW);
	}

	return (TERM_WHITE);
}

static byte orb_color(void)
{
	switch (rand_int(4))
	{
		case 0: case 1: case 2: return (TERM_L_DARK);
		case 3: return (TERM_SLATE);
	}

	return (TERM_L_DARK);
}

static byte death_color(void)
{
	switch (rand_int(4))
	{
		case 0: case 1: case 2: return (TERM_L_DARK);
		case 3: return (TERM_VIOLET);
	}

	return (TERM_L_DARK);
}

static byte mana_color(void)
{
	switch (rand_int(4))
	{
		case 0: case 1: return (TERM_VIOLET);
		case 2: return (TERM_BLUE);
		case 3: return (TERM_RED);
	}

	return (TERM_VIOLET);
}


static byte disease_color(void)
{
	switch (rand_int(4))
	{
		case 0: case 1: return (TERM_GREEN);
		case 2: return (TERM_VIOLET);
		case 3: return (TERM_L_RED);
	}

	return (TERM_VIOLET);
}


/*
 * Return a color to use for the bolt/ball spells
 */
byte spell_color(int type)
{
	/* Analyze */
	switch (type)
	{
		case GF_ACID:         return (acid_color());
		case GF_ELEC:         return (elec_color());
		case GF_FIRE:         return (fire_color());
		case GF_COLD:         return (cold_color());
		case GF_POIS:         return (pois_color());

		case GF_PLASMA:       return (plasma_color());
		case GF_HELLFIRE:     return (hellfire_color());
		case GF_ICE:          return (ice_color());

		case GF_LITE_WEAK:    return (TERM_YELLOW);
		case GF_LITE:         return (lite_color());
		case GF_DARK_WEAK:    return (TERM_L_DARK);
		case GF_DARK:         return (TERM_L_DARK);

		case GF_CONFUSION:    return (confu_color());
		case GF_SOUND:        return (TERM_YELLOW);
		case GF_SHARD:        return (TERM_L_UMBER);
		case GF_INERTIA:      return (TERM_L_WHITE);
		case GF_GRAVITY:      return (grav_color());
		case GF_FORCE:        return (TERM_GREEN);

		case GF_WATER_WEAK:	return (water_color());
		case GF_WATER:        return (water_color());

		case GF_WIND:         return (TERM_WHITE);
		case GF_STORM:        return (storm_color());

		case GF_NEXUS:        return (TERM_L_RED);
		case GF_NETHER:       return (TERM_L_GREEN);
		case GF_CHAOS:        return (mh_attr());
		case GF_DISENCHANT:   return (TERM_VIOLET);
		case GF_TIME:         return (TERM_L_BLUE);
		case GF_MANA:         return (mana_color());
		case GF_DRAIN_LIFE:   return (death_color());

		case GF_METEOR:       return (meteor_color());
		case GF_HOLY_ORB:     return (orb_color());

		case GF_EXPLODE:	return (mh_attr());
		case GF_BWATER:	 	return (TERM_VIOLET);
		case GF_BMUD:	   	return (TERM_ORANGE);
		case GF_LAVA:	   	return (lava_color());

		case GF_TERRIFY:	return (TERM_L_WHITE);
		case GF_SALT_WATER:     return (TERM_L_GREEN);
		case GF_STEAM:		return (TERM_L_WHITE);
		case GF_VAPOUR:		return (TERM_L_GREEN);
		case GF_SMOKE:		return (TERM_L_DARK);

		case GF_DISEASE:	return (disease_color());
	}

	/* Standard "color" */
	return (TERM_WHITE);
}



/*
 * Find the attr/char pair to use for a spell effect
 *
 * It is moving (or has moved) from (x,y) to (nx,ny).
 *
 * If the distance is not "one", we (may) return "*".
 */
static u16b bolt_pict(int y, int x, int ny, int nx, int typ)
{
	int base;

	byte k;

	byte a;
	char c;

	/* No motion (*) */
	if ((ny == y) && (nx == x)) base = 0x30;

	/* Vertical (|) */
	else if (nx == x) base = 0x40;

	/* Horizontal (-) */
	else if (ny == y) base = 0x50;

	/* Diagonal (/) */
	else if ((ny-y) == (x-nx)) base = 0x60;

	/* Diagonal (\) */
	else if ((ny-y) == (nx-x)) base = 0x70;

	/* Weird (*) */
	else base = 0x30;

	/* Basic spell color */
	k = spell_color(typ);

	/* Obtain attr/char */
	a = misc_to_attr[base+k];
	c = misc_to_char[base+k];

	/* Create pict */
	return (PICT(a,c));
}




/*
 * Decreases players hit points and sets death flag if necessary
 *
 * Invulnerability needs to be changed into a "shield" XXX XXX XXX
 *
 * Hack -- this function allows the user to save (or quit) the game
 * when he dies, since the "You die." message is shown before setting
 * the player to "dead".
 */
void take_hit(int dam, cptr kb_str)
{
	int old_chp = p_ptr->chp;

	int warning = (p_ptr->mhp * op_ptr->hitpoint_warn / 10);

#ifdef ALLOW_BORG
	/* Mega-Hack -- Borg can't be hurt. */
	if (count_stop) return;
#endif

	/* Paranoia */
	if (p_ptr->is_dead) return;


	/* Disturb */
	disturb(1, 1);

	/* Mega-Hack -- Apply "invulnerability" */
	if (p_ptr->invuln && (dam < 9000)) return;

	/* Hurt the player */
	p_ptr->chp -= dam;

	/* Display the hitpoints */
	p_ptr->redraw |= (PR_HP);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);

	/* Dead player */
	if (p_ptr->chp < 0)
	{
		/* Hack -- Note death */
		message(MSG_DEATH, 0, "You die.");
		msg_print(NULL);

		/* Note cause of death */
		my_strcpy(p_ptr->died_from, kb_str, sizeof(p_ptr->died_from));

		/* No longer a winner */
		p_ptr->total_winner = FALSE;

		/* Note death */
		p_ptr->is_dead = TRUE;

		/* Leaving */
		p_ptr->leaving = TRUE;

		/* Dead */
		return;
	}

	/* Hitpoint warning */
	if (p_ptr->chp < warning)
	{
		/* Hack -- bell on first notice */
		if ((old_chp > warning))
		{
			bell("Low hitpoint warning!");
		}

		/* Message */
		msg_print("*** LOW HITPOINT WARNING! ***");
		msg_print(NULL);
	}
}





/*
 * Does a given class of objects (usually) hate acid?
 * Note that acid can either melt or corrode something.
 */
static bool hates_acid(object_type *o_ptr)
{
	/* Analyze the type */
	switch (o_ptr->tval)
	{
		/* Wearable items */
		case TV_ARROW:
		case TV_BOLT:
		case TV_BOW:
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_DIGGING:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:

		/* Wands are metallic */
		case TV_WAND:

		/* Staffs/Scrolls are wood/paper */
		case TV_STAFF:
		case TV_SCROLL:
		case TV_MAP:
		case TV_ROPE:
		case TV_STATUE:

		/* Junk is useless */
		case TV_BONE:
		case TV_BODY:
		case TV_SKIN:
		case TV_EGG:
		case TV_HOLD:
		case TV_JUNK:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Does a given object (usually) hate fire?
 * Hafted/Polearm weapons have wooden shafts.
 * Arrows/Bows are mostly wooden.
 */
static bool hates_fire(object_type *o_ptr)
{
	/* Analyze the type */
	switch (o_ptr->tval)
	{
		/* Wearable */
		case TV_LITE:
		case TV_ARROW:
		case TV_BOW:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_DIGGING:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:

		/* Books */
		case TV_MAGIC_BOOK:
		case TV_PRAYER_BOOK:
		case TV_SONG_BOOK:
		case TV_INSTRUMENT:

		/* Staffs/Scrolls burn */
		case TV_STAFF:
		case TV_SCROLL:
		case TV_MAP:
		case TV_ROPE:
		case TV_STATUE:

		/* Flasks burn */
		case TV_FLASK:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Does a given object (usually) hate cold?
 */
static bool hates_cold(object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_POTION:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Does a given object (usually) hate water?
 */
static bool hates_water(object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		/* Hack -- immerse vampire skeletons in running water */
		case TV_BONE:
		case TV_SCROLL:
		case TV_MAP:
		case TV_MAGIC_BOOK:
		case TV_PRAYER_BOOK:
		case TV_SONG_BOOK:
		case TV_INSTRUMENT:
		case TV_FOOD:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Does a given object (usually) hate getting warped by wood?
 * Hafted/Polearm weapons have wooden shafts.
 * Arrows/Bows are mostly wooden.
 */
static bool hates_warpwood(object_type *o_ptr)
{
	/* Analyze the type */
	switch (o_ptr->tval)
	{
		/* Wearable */
		case TV_ARROW:
		case TV_BOW:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_DIGGING:
		case TV_INSTRUMENT:

		/* Staffs warp */
		case TV_STAFF:
		{
			return (TRUE);
		}
		
		/* Hack -- wooden statues warp */
		case TV_STATUE:
		{
			if (o_ptr->sval == SV_STATUE_WOOD) return (TRUE);
		}
	}

	return (FALSE);
}

/*
 * Does a given object (usually) hate a terrain type?
 */
bool hates_terrain(object_type *o_ptr, int f_idx)
{
	/* Require non-destructive space */
	switch(f_info[f_idx].blow.effect)
	{
		case GF_NOTHING:
			break;

		case GF_ACID:
		case GF_VAPOUR:
			if (hates_acid(o_ptr)) return(TRUE);
			break;

		case GF_METEOR:
			if (hates_fire(o_ptr)) return(TRUE);
			/* Fall through */
		case GF_ICE:
		case GF_COLD:
			if (hates_cold(o_ptr)) return(TRUE);
			break;

		case GF_STORM:
			if (hates_water(o_ptr)) return(TRUE);
			break;

		case GF_PLASMA:
		case GF_FIRE:
		case GF_SMOKE:
		case GF_HELLFIRE:
			if (hates_fire(o_ptr)) return(TRUE);
			break;

		case GF_WATER:
		case GF_BWATER:
		case GF_BMUD:
		case GF_STEAM:
		case GF_WATER_WEAK:
			if (hates_water(o_ptr)) return(TRUE);
			break;

		case GF_HURT_WOOD:
			if (hates_warpwood(o_ptr)) return (TRUE);
			break;

		default:
			return(TRUE);
	}

	return (FALSE);
}








/*
 * Melt something
 */
static int set_acid_destroy(object_type *o_ptr)
{
	u32b f1, f2, f3, f4;
	if (!hates_acid(o_ptr)) return (FALSE);
	object_flags(o_ptr, &f1, &f2, &f3, &f4);
	if (f2 & (TR2_IGNORE_ACID))
	{
		object_can_flags(o_ptr,0x0L,TR2_IGNORE_ACID,0x0L,0x0L, FALSE);
		return (FALSE);
	}
	return (TRUE);
}


/*
 * Burn something
 */
static int set_fire_destroy(object_type *o_ptr)
{
	u32b f1, f2, f3, f4;
	if (!hates_fire(o_ptr)) return (FALSE);
	object_flags(o_ptr, &f1, &f2, &f3, &f4);
	if (f2 & (TR2_IGNORE_FIRE))
	{
		object_can_flags(o_ptr,0x0L,TR2_IGNORE_FIRE,0x0L,0x0L, FALSE);
		return (FALSE);
	}
	return (TRUE);
}


/*
 * Freeze things
 */
static int set_cold_destroy(object_type *o_ptr)
{
	u32b f1, f2, f3, f4;
	if (!hates_cold(o_ptr)) return (FALSE);
	object_flags(o_ptr, &f1, &f2, &f3, &f4);
	if (f2 & (TR2_IGNORE_COLD))
	{
		object_can_flags(o_ptr,0x0L,TR2_IGNORE_COLD,0x0L,0x0L, FALSE);
		return (FALSE);
	}
	return (TRUE);
}


/*
 * Electrify things.
 *
 * Instead of destroying electrified things, we drain them of charges.
 */
static int set_elec_destroy(object_type *o_ptr)
{
	u32b f1, f2, f3, f4;

	/* Only affect rods, staffs and wands */
	if ((o_ptr->tval != TV_ROD) && (o_ptr->tval != TV_STAFF) && (o_ptr->tval != TV_WAND)) return (FALSE);

	object_flags(o_ptr, &f1, &f2, &f3, &f4);
	if (f2 & (TR2_IGNORE_ELEC))
	{
		object_can_flags(o_ptr,0x0L,TR2_IGNORE_ELEC,0x0L,0x0L, FALSE);
		return (FALSE);
	}
	return (TRUE);
}



/*
 * Soak things through
 */
static int set_water_destroy(object_type *o_ptr)
{
	u32b f1, f2, f3, f4;
	if (!hates_water(o_ptr)) return (FALSE);
	object_flags(o_ptr, &f1, &f2, &f3, &f4);
	if (f2 & (TR2_IGNORE_WATER))
	{
		object_can_flags(o_ptr,0x0L,TR2_IGNORE_WATER,0x0L,0x0L, FALSE);
		return (FALSE);
	}
	return (TRUE);
}


/*
 * Warp wooden things
 */
static int set_warpwood_destroy(object_type *o_ptr)
{
	if (!hates_warpwood(o_ptr)) return (FALSE);

	return (TRUE);
}


/*
 * This seems like a pretty standard "typedef"
 */
typedef int (*inven_func)(object_type *);

/*
 * Destroys a type of item on a given percent chance
 * Note that missiles are no longer necessarily all destroyed
 * Destruction taken from "melee.c" code for "stealing".
 * Returns number of items destroyed.
 */
static int inven_damage(inven_func typ, int perc)
{
	int i, j, k, amt;

	object_type *o_ptr;

	char o_name[80];


	/* Count the casualties */
	k = 0;

	/* Scan through the slots backwards */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		bool damage;

		o_ptr = &inventory[i];

		/* Damage only inventory and quiver */
		if ((i >= INVEN_PACK) && !IS_QUIVER_SLOT(i)) continue;

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Hack -- for now, skip artifacts */
		if (artifact_p(o_ptr)) continue;

		/* Give this item slot a shot at death */
		if ((*typ)(o_ptr))
		{
			damage = FALSE;

			/* Analyze the type to see if we just damage it */
			switch (o_ptr->tval)
			{
				/* Weapons */
				case TV_BOW:
				case TV_SWORD:
				case TV_HAFTED:
				case TV_POLEARM:
				case TV_DIGGING:
				{
					/* Chance to damage it */
					if (rand_int(100) < perc)
					{
						/* Damage the item */
						o_ptr->to_h--;
						o_ptr->to_d--;

						/* Damaged! */
						damage = TRUE;
					}
					else continue;

					break;
				}

				/* Wearable items */
				case TV_HELM:
				case TV_CROWN:
				case TV_SHIELD:
				case TV_BOOTS:
				case TV_GLOVES:
				case TV_CLOAK:
				case TV_SOFT_ARMOR:
				case TV_HARD_ARMOR:
				case TV_DRAG_ARMOR:
				{
					/* Chance to damage it */
					if (rand_int(100) < perc)
					{
						/* Damage the item */
						o_ptr->to_a--;

						/* Damaged! */
						damage = TRUE;
					}
					else continue;

					break;
				}
				/* Rods, staffs and wands */
			        case TV_STAFF:
			        case TV_WAND:
				  {
				    if (rand_int(100) < perc)
				      {
					/* Only drain charges, don't destroy */
					o_ptr->charges = 0;
					o_ptr->stackc = 0;

					/* Damaged! */
					damage = TRUE;
				      }
				    else continue;
				    
				    break;
				  }
			        case TV_ROD:
				  {
				    if (rand_int(100) < perc)
				      {
					/* Discharge */
					int t = randint(999);
					if (o_ptr->timeout < t) o_ptr->timeout = t;
					/* Damaged! */
					damage = TRUE;
				      }
				    else continue;
				    
				    break;
				  }
			}

			/* Damage instead of destroy */
			if (damage)
			{
				/* Hack --- unsense the item */
				o_ptr->ident &= ~(IDENT_SENSE);	

				/* Remove special inscription, if any */
				o_ptr->feeling = 0;

				/* Calculate bonuses */
				p_ptr->update |= (PU_BONUS);

				/* Window stuff */
				p_ptr->window |= (PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);

				/* Casualty count */
				amt = o_ptr->number;
			}

			/* Count the casualties */
			else for (amt = j = 0; j < o_ptr->number; ++j)
			{
				if (rand_int(100) < perc) amt++;
			}

			/* Some casualities */
			if (amt)
			{
				/* Get a description */
				object_desc(o_name, sizeof(o_name), o_ptr, FALSE, 3);

				/* Message */
				msg_format("%sour %s (%c) %s %s!",
					   ((o_ptr->number > 1) ?
					    ((amt == o_ptr->number) ? "All of y" :
					     (amt > 1 ? "Some of y" : "One of y")) : "Y"),
					   o_name, index_to_label(i),
					   (damage ? (amt > 1 ? "are" : "is") : (amt > 1 ? "were" : "was")),
					   (damage ? "damaged" : "destroyed"));

				/* Damage already done? */
				if (damage) continue;

				/* Forget object */
				if (o_ptr->number == amt) inven_drop_flags(o_ptr);

				/* Destroy "amt" items */
				inven_item_increase(i, -amt);
				inven_item_optimize(i);

				/* Count the casualties */
				k += amt;
			}
		}
	}

	/* Return the casualty count */
	return (k);
}


/*
 * Destroys a type of item on a given percent chance carried by monsters.
 * Note that missiles are no longer necessarily all destroyed
 * Destruction taken from "melee.c" code for "stealing".
 * Returns number of items destroyed.
 */
static int mon_inven_damage(int m_idx, inven_func typ, int perc)
{
	monster_type *m_ptr = &m_list[m_idx];

	int j, k, amt;

	object_type *o_ptr;

	s16b this_o_idx, next_o_idx = 0;

	char m_name[80];

	char o_name[80];

	/* Count the casualties */
	k = 0;

	/* Destroy objects being carried */
	for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		/* Get the object */
		o_ptr = &o_list[this_o_idx];

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Hack -- for now, skip artifacts */
		if (artifact_p(o_ptr)) continue;

		/* Give this item slot a shot at death */
		if ((*typ)(o_ptr))
		{
			/* Count the casualties */
			for (amt = j = 0; j < o_ptr->number; ++j)
			{
				if (rand_int(100) < perc) amt++;
			}

			/* Some casualities */
			if (amt)
			{
				/* Get a description */
				object_desc(o_name, sizeof(o_name), o_ptr, FALSE, 3);

				/* Get "possessive" */
				monster_desc(m_name, sizeof(m_name), m_idx, 0x22);

				/* Message */
				msg_format("%^s%s %s %s destroyed!",
					   ((o_ptr->number > 1) ?
					    ((amt == o_ptr->number) ? "All of " :
					     (amt > 1 ? "Some of " : "One of ")) : m_name),
					   (o_ptr->number > 1) ? m_name : "",  o_name, 
					   ((amt > 1) ? "were" : "was"));

				/* Destroy "amt" items */
				if (amt < o_ptr->number) o_ptr->number -= amt;

				/* Delete the object */
				else delete_object_idx(this_o_idx);

				/* Count the casualties */
				k += amt;
			}
		}
	}

	/* Return the casualty count */
	return (k);
}


/*
 * Acid has hit the player, attempt to affect some armor.
 *
 * Note that the "base armor" of an object never changes.
 *
 * If any armor is damaged (or resists), the player takes less damage.
 */
static int minus_ac(int ac)
{
	object_type *o_ptr = NULL;

	u32b f1, f2, f3, f4;

	char o_name[80];

	bool destroy = FALSE;

	int slot = INVEN_BODY;

	/* Pick a (possibly empty) inventory slot */
	switch (randint(6))
	{
		case 1: slot = INVEN_BODY; break;
		case 2: slot = INVEN_ARM; break;
		case 3: slot = INVEN_OUTER; break;
		case 4: slot = INVEN_HANDS; break;
		case 5: slot = INVEN_HEAD; break;
		case 6: slot = INVEN_FEET; break;
	}

	/* Get the object */
	o_ptr = &inventory[slot];

	/* Nothing to damage */
	if (!o_ptr->k_idx) return (FALSE);

	/* No damage left to be done */
	if (o_ptr->ac + o_ptr->to_a - ac <= 0) destroy = TRUE;

	/* Mega Hack -- do not destroy ego items, magic items etc */
	if ((destroy) && (o_ptr->name2 || o_ptr->xtra1 || o_ptr->xtra2 || (o_ptr->tval == TV_DRAG_ARMOR)))
	{
		/* Do not destroy */
		destroy = FALSE;

		/* Already completely damaged? */
		if (o_ptr->ac + o_ptr->to_a <= 0) return (FALSE);

		/* Damage completely */
		ac = o_ptr->ac + o_ptr->to_a;
	}

	/* Describe */
	object_desc(o_name, sizeof(o_name), o_ptr, FALSE, 0);

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4);

	/* Object resists */
	if (f2 & (TR2_IGNORE_ACID))
	{
		/* Always notice */
		object_can_flags(o_ptr,0x0L,TR2_IGNORE_ACID,0x0L,0x0L, FALSE);

		msg_format("Your %s is unaffected!", o_name);

		return (TRUE);
	}

	/* Always notice */
	object_not_flags(o_ptr,0x0L,TR2_IGNORE_ACID,0x0L,0x0L, FALSE);

	/* Message */
	msg_format("Your %s is %s!", o_name, destroy ? "destroyed" : "damaged");

	/* Destroy item */
	if (destroy)
	{
		/* Adjust total weight */
		p_ptr->total_weight -= o_ptr->weight * o_ptr->number;

		/* Clear slot */
		object_wipe(&inventory[slot]);
	}
	/* Damage item */
	else
	{
		/* Damage the item */
		o_ptr->to_a -= ac;

		/* Hack --- unsense the item */
		o_ptr->ident &= ~(IDENT_SENSE);	

		/* Remove special inscription, if any */
		o_ptr->feeling = 0;
	}

	/* Calculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Window stuff */
	p_ptr->window |= (PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);

	/* Item was damaged */
	return (TRUE);
}


/*
 * Hurt the player with Acid
 */
static void acid_dam(int who, int dam, cptr kb_str, bool inven)
{
	int inv = (dam / 15) + 1;

	int res = p_ptr->incr_resist[INCR_RES_ACID];

	/* Vulnerability */
	if ((p_ptr->cur_flags4 & (TR4_HURT_ACID)) != 0)
	{
		/* Always notice */
		player_can_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_ACID);

		/* Immunity reduced to partial protection */
		if ((p_ptr->cur_flags2 & (TR2_IM_ACID)) != 0)
		{
			/* Always notice */
			player_can_flags(who, 0x0L,TR2_IM_ACID,0x0L,0x0L);	

			/* Hack -- always assume 'armor hit' */
			dam = (dam + 5) / 6;

			/* Take damage */
			take_hit(dam, kb_str);

			return;
		}

		/* Increase damage */
		else dam *= 2;
	}

	/* Total Immunity */
	else if ((p_ptr->cur_flags2 & (TR2_IM_ACID)) != 0)
	{
		/* Always notice */
		player_can_flags(who, 0x0L,TR2_IM_ACID,0x0L,0x0L);
		player_not_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_ACID);

		return;
	}

	/* Not vulnerable */
	else if ((p_ptr->cur_flags2 & (TR2_IM_ACID)) != 0)
	{
		/* Always notice */
		player_not_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_ACID);
	}

	/* Resist the damage */
	if ((p_ptr->cur_flags2 & (TR2_RES_ACID)) != 0)
	{
		/* Sometimes notice */
		player_can_flags(who, 0x0L,TR2_RES_ACID,0x0L,0x0L);

		inv /= res;
	}
	else
	{
		/* Sometimes notice */
		player_not_flags(who, 0x0L,TR2_RES_ACID,0x0L,0x0L);
	}

	/* Resist the damage */
	if (p_ptr->oppose_acid)
	{
		/* Monster notices */
		update_smart_learn(who, SM_OPP_ACID);

		/* Increase resistance */
		res += 3;
	}
	else
	{
		/* Monster notices */
		update_smart_forget(who, SM_OPP_ACID);
	}

	/* If any armor gets hit, defend the player */
	if (minus_ac(inv)) dam = (dam + 1) / 2;

	/* Inventory damage */
	if ((inven) && (inv)) inven_damage(set_acid_destroy, inv);

	/* Reduce the damage */
	dam = (dam - res - 1) / res;

	/* No damage */
	if (dam <= 0) return;

	/* Take damage */
	take_hit(dam, kb_str);
}


/*
 * Hurt the player with electricity
 */
static void elec_dam(int who, int dam, cptr kb_str, bool inven)
{
	int inv = (dam / 15) + 1;

	int res = p_ptr->incr_resist[INCR_RES_ELEC];

	/* Vulnerability */
	if ((p_ptr->cur_flags4 & (TR4_HURT_ELEC)) != 0)
	{
		/* Always notice */
		player_can_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_ELEC);

		/* Immunity reduced to partial protection */
		if ((p_ptr->cur_flags2 & (TR2_IM_ELEC)) != 0)
		{
			/* Always notice */
			player_can_flags(who, 0x0L,TR2_IM_ELEC,0x0L,0x0L);	

			/* Reduce effect to basic resistance */
			dam = (dam + 2) / 3;

			/* Take damage */
			take_hit(dam, kb_str);

			return;
		}

		/* Increase damage */
		else dam *= 2;
	}

	/* Total Immunity */
	else if ((p_ptr->cur_flags2 & (TR2_IM_ELEC)) != 0)
	{
		/* Always notice */
		player_can_flags(who, 0x0L,TR2_IM_ELEC,0x0L,0x0L);
		player_not_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_ELEC);

		return;
	}

	/* Not vulnerable */
	else if ((p_ptr->cur_flags2 & (TR2_IM_ELEC)) != 0)
	{
		/* Always notice */
		player_not_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_ELEC);
	}

	/* Resist the damage */
	if ((p_ptr->cur_flags2 & (TR2_RES_ELEC)) != 0)
	{
		/* Sometimes notice */
		player_can_flags(who, 0x0L,TR2_RES_ELEC,0x0L,0x0L);

		inv /= res;
	}
	else
	{
		/* Sometimes notice */
		player_not_flags(who, 0x0L,TR2_RES_ELEC,0x0L,0x0L);
	}

	/* Resist the damage */
	if (p_ptr->oppose_elec)
	{
		/* Monster notices */
		update_smart_learn(who, SM_OPP_ELEC);

		/* Increase resistance */
		res += 3;
	}
	else
	{
		/* Monster notices */
		update_smart_forget(who, SM_OPP_ELEC);
	}

	/* Inventory damage */
	if ((inven) && (inv)) inven_damage(set_elec_destroy, inv);

	/* Reduce the damage */
	dam = (dam - res - 1) / res;

	/* No damage */
	if (dam <= 0) return;

	/* Take damage */
	take_hit(dam, kb_str);
}




/*
 * Hurt the player with Fire
 */
static void fire_dam(int who, int dam, cptr kb_str, bool inven)
{
	int inv = (dam / 15) + 1;

	int res = p_ptr->incr_resist[INCR_RES_FIRE];

	/* Vulnerability */
	if ((p_ptr->cur_flags4 & (TR4_HURT_FIRE)) != 0)
	{
		/* Always notice */
		player_can_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_FIRE);

		/* Immunity reduced to partial protection */
		if ((p_ptr->cur_flags2 & (TR2_IM_FIRE)) != 0)
		{
			/* Always notice */
			player_can_flags(who, 0x0L,TR2_IM_FIRE,0x0L,0x0L);	

			/* Reduce effect to basic resistance */
			dam = (dam + 2) / 3;

			/* Take damage */
			take_hit(dam, kb_str);

			return;
		}

		/* Increase damage */
		else dam *= 3;
	}

	/* Total Immunity */
	else if ((p_ptr->cur_flags2 & (TR2_IM_FIRE)) != 0)
	{
		/* Always notice */
		player_can_flags(who, 0x0L,TR2_IM_FIRE,0x0L,0x0L);
		player_not_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_FIRE);

		return;
	}

	/* Not vulnerable */
	else if ((p_ptr->cur_flags2 & (TR2_IM_FIRE)) != 0)
	{
		/* Always notice */
		player_not_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_FIRE);
	}

	/* Resist the damage */
	if ((p_ptr->cur_flags2 & (TR2_RES_FIRE)) != 0)
	{
		/* Sometimes notice */
		player_can_flags(who, 0x0L,TR2_RES_FIRE,0x0L,0x0L);

		inv /= res;
	}
	else
	{
		/* Sometimes notice */
		player_not_flags(who, 0x0L,TR2_RES_FIRE,0x0L,0x0L);
	}

	/* Resist the damage */
	if (p_ptr->oppose_fire)
	{
		/* Monster notices */
		update_smart_learn(who, SM_OPP_FIRE);

		res += 3;
	}
	else
	{
		/* Monster notices */
		update_smart_forget(who, SM_OPP_FIRE);
	}

	/* Inventory damage */
	if ((inven) && (inv)) inven_damage(set_fire_destroy, inv);

	/* Reduce the damage */
	dam = (dam - res - 1) / res;

	/* No damage */
	if (dam <= 0) return;

	/* Take damage */
	take_hit(dam, kb_str);
}


/*
 * Hurt the player with Cold
 */
static void cold_dam(int who, int dam, cptr kb_str, bool inven)
{
	int inv = (dam / 15) + 1;

	int res = p_ptr->incr_resist[INCR_RES_COLD];

	/* Vulnerability */
	if ((p_ptr->cur_flags4 & (TR4_HURT_COLD)) != 0)
	{
		/* Always notice */
		player_can_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_COLD);

		/* Immunity reduced to partial protection */
		if ((p_ptr->cur_flags2 & (TR2_IM_COLD)) != 0)
		{
			/* Always notice */
			player_can_flags(who, 0x0L,TR2_IM_COLD,0x0L,0x0L);	

			/* Reduce effect to basic resistance */
			dam = (dam + 2) / 3;

			/* Take damage */
			take_hit(dam, kb_str);

			return;
		}

		/* Increase damage */
		else dam *= 3;
	}

	/* Total Immunity */
	else if ((p_ptr->cur_flags2 & (TR2_IM_COLD)) != 0)
	{
		/* Always notice */
		player_can_flags(who, 0x0L,TR2_IM_COLD,0x0L,0x0L);
		player_not_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_COLD);

		return;
	}

	/* Not vulnerable */
	else if ((p_ptr->cur_flags2 & (TR2_IM_COLD)) != 0)
	{
		/* Always notice */
		player_not_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_COLD);
	}

	/* Resist the damage */
	if ((p_ptr->cur_flags2 & (TR2_RES_COLD)) != 0)
	{
		/* Sometimes notice */
		player_can_flags(who, 0x0L,TR2_RES_COLD,0x0L,0x0L);

		inv /= res;
	}
	else
	{
		/* Sometimes notice */
		player_not_flags(who, 0x0L,TR2_RES_COLD,0x0L,0x0L);
	}

	/* Resist the damage */
	if (p_ptr->oppose_cold)
	{
		/* Monster notices */
		update_smart_learn(who, SM_OPP_COLD);

		res += 3;
	}
	else
	{
		/* Monster notices */
		update_smart_forget(who, SM_OPP_COLD);
	}

	/* Inventory damage */
	if ((inven) && (inv)) inven_damage(set_cold_destroy, inv);

	/* Reduce the damage */
	dam = (dam - res - 1) / res;

	/* No damage */
	if (dam <= 0) return;

	/* Take damage */
	take_hit(dam, kb_str);
}

/*
 * Hurt the player with Poison
 */
static void poison_dam(int who, int dam, cptr kb_str, bool inven)
{
	int res = p_ptr->incr_resist[INCR_RES_POIS];

	(void)inven;

	/* Vulnerability */
	if ((p_ptr->cur_flags4 & (TR4_HURT_POIS)) != 0)
	{
		/* Always notice */
		player_can_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_POIS);

		/* Immunity reduced to partial protection */
		if ((p_ptr->cur_flags4 & (TR4_IM_POIS)) != 0)
		{
			/* Always notice */
			player_can_flags(who, 0x0L,0x0L,0x0L,TR4_IM_POIS);

			/* Reduce effect to basic resistance */
			dam = (dam + 2) / 3;

			/* Take damage */
			take_hit(dam, kb_str);

			return;
		}

		/* Increase damage */
		else dam *= 2;
	}

	/* Total Immunity */
	else if ((p_ptr->cur_flags4 & (TR4_IM_POIS)) != 0)
	{
		/* Always notice */
		player_can_flags(who, 0x0L,0x0L,0x0L,TR4_IM_POIS);
		player_not_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_POIS);

		return;
	}

	/* Not vulnerable */
	else if ((p_ptr->cur_flags4 & (TR4_IM_POIS)) != 0)
	{
		/* Always notice */
		player_not_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_POIS);
	}

	/* Resist the damage */
	if ((p_ptr->cur_flags2 & (TR2_RES_POIS)) != 0)
	{
		/* Sometimes notice */
		player_can_flags(who, 0x0L,TR2_RES_POIS,0x0L,0x0L);
	}
	else
	{
		/* Sometimes notice */
		player_not_flags(who, 0x0L,TR2_RES_POIS,0x0L,0x0L);
	}

	/* Resist the damage */
	if (p_ptr->oppose_pois)
	{
		/* Monster notices */
		update_smart_learn(who, SM_OPP_POIS);

		res += 3;
	}
	else
	{
		/* Monster notices */
		update_smart_forget(who, SM_OPP_POIS);
	}

	/* Reduce the damage */
	dam = (dam - res - 1) / res;

	/* No damage */
	if (dam <= 0) return;

	/* Increase poison counter */
	if (!(p_ptr->oppose_pois) && !(p_ptr->cur_flags2 & (TR2_RES_POIS)))
	{
		/* Set poison counter */
		(void)set_poisoned(p_ptr->poisoned + rand_int(dam + 1) + 10);
	}

	/* Take damage */
	take_hit(dam, kb_str);
}



/*
 * Hurt the player with Water
 */
static void water_dam(int who, int dam, cptr kb_str, bool inven)
{
	int inv = (dam / 15) /* + 1 */;

	int res = p_ptr->incr_resist[INCR_RES_WATER];

	/* Check for light being wielded */
	object_type *o_ptr = &inventory[INVEN_LITE];

	(void)who;

	/* Burn some fuel in the current lite */
	if ((inven) && (o_ptr->tval == TV_LITE))
	{
		u32b f1, f2, f3, f4;

		object_flags(o_ptr,&f1,&f2,&f3,&f4);

		/* Hack -- Douse light (except on artifacts and lites unaffected by water)
		 * We no longer reduce the fuel as players found this annoying.
		 */
		if (!artifact_p(o_ptr) && (o_ptr->timeout > 0) && !(f2 & TR2_IGNORE_WATER))
		{
			/* Copy charges */
			o_ptr->charges = o_ptr->timeout;
			
			/* Douse light */
			o_ptr->timeout = 0;

			disturb(0, 0);
			msg_print("Your light has gone out!");
			
			/* Update torch */
			p_ptr->update |= (PU_TORCH);

			/* Fully update the visuals */
			p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);
		}
	}

	/* Resist the damage */
	if ((p_ptr->cur_flags4 & (TR4_RES_WATER)) != 0)
	{
		/* Sometimes notice */
		player_can_flags(who, 0x0L,0x0L,0x0L,TR4_RES_WATER);

		inv /= res;
	}
	else
	{
		/* Sometimes notice */
		player_not_flags(who, 0x0L,0x0L,0x0L,TR4_RES_WATER);
	}

	/* Resist the damage */
	if (p_ptr->oppose_water)
	{
		res += 3;
	}

	/* Inventory damage */
	if (inven) inven_damage(set_water_destroy, inv);

	/* Reduce the damage */
	dam = (dam - res - 1) / res;

	/* No damage */
	if (dam <= 0) return;

	/* Take damage */
	take_hit(dam, kb_str);
}


/*
 * Ignore the effects of terrain under a player.
 */
bool player_ignore_terrain(int f_idx)
{
	feature_type *f_ptr= &f_info[f_idx];

	/* Hack -- boots provide protection from terrain */
	object_type *i_ptr = &inventory[INVEN_FEET];

	u32b f1 = 0x0L;
	u32b f2 = 0x0L;
	u32b f3 = 0x0L;
	u32b f4 = 0x0L;

	bool ignore = FALSE;

	/* Get boot flags -- hack: only if terrain not filled */
	if ((i_ptr->k_idx) && !(f_ptr->flags2 & (FF2_FILLED))) object_flags(i_ptr, &f1, &f2, &f3, &f4);

	/* Check the blow type for terrain */
	if ((f_ptr->blow.method) && !(f_ptr->flags1 & (FF1_HIT_TRAP)))
		switch(f_ptr->blow.effect)
	{
		case GF_FIRE:
		case GF_SMOKE:
		{
			if ((p_ptr->oppose_fire) || (p_ptr->oppose_lava)) ignore = TRUE;
			else if (f2 & (TR2_RES_FIRE))
			{
				object_can_flags(i_ptr, 0x0L, TR2_RES_FIRE, 0x0L, 0x0L, FALSE);
				ignore = TRUE;
			}
			else if (p_ptr->cur_flags2 & (TR2_IM_FIRE))
			{
				equip_can_flags(0x0L, TR2_IM_FIRE, 0x0L, 0x0L);
				ignore = TRUE;
			}
			break;
		}
		case GF_ACID:
		case GF_VAPOUR:
		{
			if (p_ptr->oppose_acid) ignore = TRUE;
			else if (f2 & (TR2_RES_ACID))
			{
				object_can_flags(i_ptr, 0x0L, TR2_RES_ACID, 0x0L, 0x0L, FALSE);
				ignore = TRUE;
			}
			else if (p_ptr->cur_flags2 & (TR2_IM_ACID))
			{
				equip_can_flags(0x0L, TR2_IM_ACID, 0x0L, 0x0L);
				ignore = TRUE;
			}
			break;
		}
		case GF_COLD:
		/* case GF_ICE: */
		{
			if (p_ptr->oppose_cold) ignore = TRUE;
			else if (f2 & (TR2_RES_COLD))
			{
				object_can_flags(i_ptr, 0x0L, TR2_RES_COLD, 0x0L, 0x0L, FALSE);
				ignore = TRUE;
			}
			else if (p_ptr->cur_flags2 & (TR2_IM_COLD))
			{
				equip_can_flags(0x0L, TR2_IM_COLD, 0x0L, 0x0L);
				ignore = TRUE;
			}
			break;
		}
		case GF_WATER:
		case GF_WATER_WEAK:
		case GF_SALT_WATER:
		{
			if (p_ptr->oppose_water) ignore = TRUE;
			else if (f4 & (TR4_RES_WATER))
			{
				object_can_flags(i_ptr, 0x0L, 0x0L, 0x0L, TR4_RES_WATER, FALSE);
				ignore = TRUE;
			}
			break;
		}
		case GF_BMUD:
		case GF_BWATER:
		case GF_STEAM:
		{
			if ((p_ptr->oppose_water) && (p_ptr->oppose_fire)) ignore = TRUE;
			else if ((f4 & (TR4_RES_WATER)) && (p_ptr->oppose_fire))
			{
				object_can_flags(i_ptr, 0x0L, 0x0L, 0x0L, TR4_RES_WATER, FALSE);
				ignore = TRUE;
			}
			else if ((f2 & (TR2_RES_FIRE)) && (p_ptr->oppose_water))
			{
				object_can_flags(i_ptr, 0x0L, TR2_RES_FIRE, 0x0L, 0x0L, FALSE);
				ignore = TRUE;
			}
			else if ((f4 & (TR4_RES_WATER)) && (f2 & (TR2_RES_FIRE)))
			{
				object_can_flags(i_ptr, 0x0L, TR2_RES_FIRE, 0x0L, TR4_RES_WATER, FALSE);
				ignore = TRUE;
			}

			/* Fall through */
		}
		case GF_LAVA:
		{
			if (p_ptr->oppose_lava) ignore = TRUE;
			else if (p_ptr->cur_flags2 & (TR2_IM_FIRE))
			{
				equip_can_flags(0x0L, TR2_IM_FIRE, 0x0L, 0x0L);
				ignore = TRUE;
			}

			break;
		}
	}

	return (ignore);
}


#define rand_roll(n)	(auto_roll ? (auto_roll == 1 ? 1 : n) : randint(n))

/*
 * Calculate the increase of a stat by one randomized level.
 *
 * The "auto_roll" flag selects maximal changes for use 
 * with the auto-roller initialization code
 * or minimal for the point-based character generation.  
 * Otherwise, semi-random changes will occur.
 */
int calc_inc_stat(int value, int auto_roll)
{
	int gain;

	/* Gain one point */
	if (value < 18)
		value++;

	/* Gain 1/10 to 1/5 of distance to 18/100; at last, 3--5 points */
	else if (value < 18+80)
	{
		/* Approximate gain value */
		gain = ((18+100) - value) / 5 + 1;

		/* Roll the bonus */
		value += (rand_roll(gain) + gain) / 2;
	}

	/* Gain 3--5 points at a time */
	else if (value < 18+100)
		value += 2 + rand_roll(3);

	/* Gain 3--4 points at a time */
	else if (value < 18+120)
		value += 2 + rand_roll(2);

	/* Gain 2--3 points at a time */
	else if (value < 18+150)
		value += 1 + rand_roll(2);

	/* Gain 1--2 points at a time */
	else
	{
		value += rand_roll(2);

		/* Maximal value */
		if (value > 18+999) value = 18 + 999;
	}

	/* Return the result */
	return (value);
}


/*
 * Increase a stat by one randomized level.
 * Some code will "restore" a stat before calling this function.
 */
void inc_stat(int stat)
{
	int value = p_ptr->stat_cur[stat];

	/* Calculate the random increase */
	value = calc_inc_stat(value, 0);

	/* Save the new value */
	p_ptr->stat_cur[stat] = value;

	/* Bring up the maximum too */
	if (value > p_ptr->stat_max[stat])
		p_ptr->stat_max[stat] = value;

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);
}


/*
 * Decreases a stat by an amount indended to vary from 0 to 100 percent.
 *
 * Note that "permanent" means that the *given* amount is permanent,
 * not that the new value becomes permanent.  This may not work exactly
 * as expected, due to "weirdness" in the algorithm, but in general,
 * if your stat is already drained, the "max" value will not drop all
 * the way down to the "cur" value.
 */
bool dec_stat(int stat, int amount, int permanent)
{
	int cur, max, loss, same, res = FALSE;


	/* Get the current value */
	cur = p_ptr->stat_cur[stat];
	max = p_ptr->stat_max[stat];

	/* Note when the values are identical */
	same = (cur == max);

	/* Damage "current" value */
	if (cur > 3)
	{
		/* Handle "low" values */
		if (cur <= 18)
		{
			if (amount > 90) cur--;
			if (amount > 50) cur--;
			if (amount > 20) cur--;
			cur--;
		}

		/* Handle "high" values */
		else
		{
			/* Hack -- Decrement by a random amount between one-quarter */
			/* and one-half of the stat bonus times the percentage, with a */
			/* minimum damage of half the percentage. -CWS */
			loss = (((cur-18) / 2 + 1) / 2 + 1);

			/* Paranoia */
			if (loss < 1) loss = 1;

			/* Randomize the loss */
			loss = ((randint(loss) + loss) * amount) / 100;

			/* Maximal loss */
			if (loss < amount/2) loss = amount/2;

			/* Lose some points */
			cur = cur - loss;

			/* Hack -- Only reduce stat to 17 sometimes */
			if (cur < 18) cur = (amount <= 20) ? 18 : 17;
		}

		/* Prevent illegal values */
		if (cur < 3) cur = 3;

		/* Something happened */
		if (cur != p_ptr->stat_cur[stat]) res = TRUE;
	}

	/* Damage "max" value */
	if (permanent && (max > 3))
	{
		/* Handle "low" values */
		if (max <= 18)
		{
			if (amount > 90) max--;
			if (amount > 50) max--;
			if (amount > 20) max--;
			max--;
		}

		/* Handle "high" values */
		else
		{
			/* Hack -- Decrement by a random amount between one-quarter */
			/* and one-half of the stat bonus times the percentage, with a */
			/* minimum damage of half the percentage. -CWS */
			loss = (((max-18) / 2 + 1) / 2 + 1);
			if (loss < 1) loss = 1;
			loss = ((randint(loss) + loss) * amount) / 100;
			if (loss < amount/2) loss = amount/2;

			/* Lose some points */
			max = max - loss;

			/* Hack -- Only reduce stat to 17 sometimes */
			if (max < 18) max = (amount <= 20) ? 18 : 17;
		}

		/* Hack -- keep it clean */
		if (same || (max < cur)) max = cur;

		/* Something happened */
		if (max != p_ptr->stat_max[stat]) res = TRUE;
	}

	/* Apply changes */
	if (res)
	{
		/* Actually set the stat to its new value. */
		p_ptr->stat_cur[stat] = cur;
		p_ptr->stat_max[stat] = max;

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);
	}

	/* Done */
	return (res);
}


/*
 * Restore a stat.  Return TRUE only if this actually makes a difference.
 */
bool res_stat(int stat)
{
	/* Restore if needed */
	if (p_ptr->stat_cur[stat] != p_ptr->stat_max[stat])
	{
		/* Restore */
		p_ptr->stat_cur[stat] = p_ptr->stat_max[stat];

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Success */
		return (TRUE);
	}

	/* Nothing to restore */
	return (FALSE);
}




/*
 * Apply disenchantment to the player's stuff
 *
 * This function is also called from the "melee" code.
 *
 * The "mode" is currently unused.
 *
 * Return "TRUE" if the player notices anything.
 */
bool apply_disenchant(int mode)
{
	int t = 0;

	object_type *o_ptr;

	char o_name[80];

	/* Prevent compiler warning */
	(void)mode;

	/* Pick a random slot */
	switch (randint(8))
	{
		case 1: t = INVEN_WIELD; break;
		case 2: t = INVEN_BOW; break;
		case 3: t = INVEN_BODY; break;
		case 4: t = INVEN_OUTER; break;
		case 5: t = INVEN_ARM; break;
		case 6: t = INVEN_HEAD; break;
		case 7: t = INVEN_HANDS; break;
		case 8: t = INVEN_FEET; break;
	}

	/* Get the item */
	o_ptr = &inventory[t];

	/* No item, nothing happens */
	if (!o_ptr->k_idx) return (FALSE);


	/* Nothing to disenchant */
	if ((o_ptr->to_h <= 0) && (o_ptr->to_d <= 0) && (o_ptr->to_a <= 0))
	{
		/* Nothing to notice */
		return (FALSE);
	}


	/* Describe the object */
	object_desc(o_name, sizeof(o_name), o_ptr, FALSE, 0);


	/* Artifacts have 60% chance to resist */
	if (artifact_p(o_ptr) && (rand_int(100) < 60))
	{
		/* Message */
		msg_format("Your %s (%c) resist%s disenchantment!",
			   o_name, index_to_label(t),
			   ((o_ptr->number != 1) ? "" : "s"));

		/* Always notice */
		object_can_flags(o_ptr,0x0L,TR2_IGNORE_MASK,TR3_INSTA_ART,0x0L, FALSE);

		/* Notice */
		return (TRUE);
	}

	/* Disenchant tohit */
	if (o_ptr->to_h > 0) o_ptr->to_h--;
	if ((o_ptr->to_h > 5) && (rand_int(100) < 20)) o_ptr->to_h--;

	/* Disenchant todam */
	if (o_ptr->to_d > 0) o_ptr->to_d--;
	if ((o_ptr->to_d > 5) && (rand_int(100) < 20)) o_ptr->to_d--;

	/* Disenchant toac */
	if (o_ptr->to_a > 0) o_ptr->to_a--;
	if ((o_ptr->to_a > 5) && (rand_int(100) < 20)) o_ptr->to_a--;

	/* Message */
	msg_format("Your %s (%c) %s disenchanted!",
		   o_name, index_to_label(t),
		   ((o_ptr->number != 1) ? "were" : "was"));

	/* Hack --- unsense the item */
	o_ptr->ident &= ~(IDENT_SENSE);	

	/* Remove special inscription, if any */
	o_ptr->feeling = 0;

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Window stuff */
	p_ptr->window |= (PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);

	/* Notice */
	return (TRUE);
}


/*
 * Apply Nexus
 */
static void apply_nexus(monster_type *m_ptr)
{
	int max1, cur1, max2, cur2, ii, jj;

	switch (randint(7))
	{
		case 1: case 2: case 3:
		{
			teleport_hook = NULL;
			teleport_player(200);
			break;
		}

		case 4: case 5:
		{
			teleport_player_to(m_ptr->fy, m_ptr->fx);
			break;
		}

		case 6:
		{
			if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print("You resist the effects!");
				break;
			}

			/* Teleport Level */
			teleport_player_level();
			break;
		}

		case 7:
		{
			if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print("You resist the effects!");
				break;
			}

			msg_print("Your body starts to scramble...");

			/* Pick a pair of stats */
			ii = rand_int(6);
			for (jj = ii; jj == ii; jj = rand_int(6)) /* loop */;

			max1 = p_ptr->stat_max[ii];
			cur1 = p_ptr->stat_cur[ii];
			max2 = p_ptr->stat_max[jj];
			cur2 = p_ptr->stat_cur[jj];

			p_ptr->stat_max[ii] = max2;
			p_ptr->stat_cur[ii] = cur2;
			p_ptr->stat_max[jj] = max1;
			p_ptr->stat_cur[jj] = cur1;

			p_ptr->update |= (PU_BONUS);

			break;
		}
	}
}


/*
 * Mega-Hack -- track "affected" monsters (see "project()" comments)
 */
static int project_m_n;
static int project_m_x;
static int project_m_y;


/*
 * Temporarily light a grid.
 * 
 * Memorise a monster or terrain if visible.
 *
 * We employ hacks here in order to temporarily make
 * the floor visible.
 *
 */
static bool temp_lite(int y, int x)
{
	int i;

	/* Lite this grid and adjacent grids */
	for (i = 0; i < 8; i++)
	{
		int yy = y + ddy_ddd[i];
		int xx = x + ddx_ddd[i];

		/* Ignore annoying locations */
		if (!(in_bounds_fully(yy, xx))) continue;

		/* Player can see grid, hasn't already seen it, and not blind */
		if ((player_has_los_bold(yy, xx)) && !(play_info[yy][xx] & (PLAY_SEEN))
		    && !(p_ptr->blind))
		{
			/* Temporarily seen */
			play_info[yy][xx] |= (PLAY_SEEN);

			/* Remember? */
			note_spot(yy,xx);

			/* Temporarily seen */
			play_info[yy][xx] &= ~(PLAY_SEEN);

			/* Light? */
			lite_spot(yy,xx);

			/* Player can see grid, not blind and it contains a monster */
			if (cave_m_idx[yy][xx] > 0)
			{
				monster_type *m_ptr = &m_list[cave_m_idx[yy][xx]];
				monster_race *r_ptr = &r_info[m_ptr->r_idx];

				/* Detect all unseen non-invisible non-hidden monsters */
				if (!(m_ptr->ml) &&
					(!(r_ptr->flags2 & (RF2_INVISIBLE)) || !(m_ptr->tim_invis) || (p_ptr->tim_invis) || ((p_ptr->cur_flags3 & (TR3_SEE_INVIS)) != 0))
						&& !(m_ptr->mflag & (MFLAG_HIDE)))
				{
					/* Optimize -- Repair flags */
					repair_mflag_mark = repair_mflag_show = TRUE;
		
					/* Hack -- Detect the monster */
					m_ptr->mflag |= (MFLAG_MARK | MFLAG_SHOW);
		
					/* Update the monster */
					update_mon(cave_m_idx[yy][xx], FALSE);
				}
			}

			/* Something seen */
			return (TRUE);
		}
	}

	return (FALSE);
}




/*
 * We are called from "project()" to "damage" terrain features
 *
 * We are called both for "beam" effects and "ball" effects.
 *
 * The "r" parameter is the "distance from ground zero".
 *
 * Note that we determine if the player can "see" anything that happens
 * by taking into account: blindness, line-of-sight, and illumination.
 *
 * We return "TRUE" if the effect of the projection is "obvious".
 *
 * Hack -- We also "see" grids which are "memorized".
 *
 * XXX Unlike Sangband, we do not have to be careful about ensuring that
 * terrain doesn't get more dangerous and increases damage, because at
 * the moment, the only terrain which affects maximum damage is water
 * and no transitions that create watery terrain also increase damage.
 * If we have transitions that are HURT_COLD or HURT_ELEC and end up
 * with terrain with the WATER flag, then we may have to reconsider this.
 *
 * However, instead we have to check if a transition occurs under a
 * monster, and then later check in project_t if that results in
 * terrain that a monster cannot survive on, and either move or entomb them.
 *
 * Luckily this is very generic and doesn't require we duplicate transition
 * code between here and project_t.
 */
bool project_f(int who, int what, int y, int x, int dam, int typ)
{
	bool obvious = FALSE;
	bool burnout = FALSE;

	cptr f;

	feature_type *f_ptr = &f_info[cave_feat[y][x]];

	int feat;
	
	/* Prevent warning */
	(void)what;

	/* Set feature name */
	f = (f_name + f_ptr->name);

	/* Track changes */
	feat = cave_feat[y][x];
	
	/* Reset the teleport hook */
	teleport_hook = NULL;

	/* Hack -- prevent smoke/vapour etc on floors unless a feature or non-player object causes this */
	if (((who != SOURCE_FEATURE) || (who != SOURCE_OBJECT)) &&
			((f_ptr->flags1 & (FF1_FLOOR)) != 0))
	{
		burnout = TRUE;
	}

	/* Hack -- storm can do several things */
	if (typ == GF_STORM)
	{
		switch(rand_int(3))
		{
			case 0: typ = GF_WIND; break;
			case 1: typ = GF_WATER; break;
			case 2: typ = GF_ELEC; break;
		}
	}

	/* Analyze the type */
	switch (typ)
	{
		case GF_ACID:
		case GF_VAPOUR:
		{
			/* Hack -- halve acid damage in water */
			if (f_ptr->flags2 & (FF2_WATER)) dam /= 2;
		
			/* Destroy hurt by acid */
			if ((f_ptr->flags2 & (FF2_HURT_ACID))  &&
			       (dam > (f_ptr->power*10)))
			{
				/* Check line of sight */
				if ((player_has_los_bold(y, x)) && (f_ptr->flags1 & FF1_NOTICE))
				{
					msg_format("The %s dissolves.",f);
				}

				/* Notice any changes */
				obvious = TRUE;

				/* Destroy the feature */
				cave_alter_feat(y, x, FS_HURT_ACID);
			}
			break;
		}
		case GF_FIRE:
		case GF_SMOKE:
		{
			/* Hack -- halve fire damage in water */
			if (f_ptr->flags2 & (FF2_WATER)) dam /= 2;
		
			/* Drop through */
		}
		case GF_LAVA:
		case GF_PLASMA:
		case GF_HELLFIRE:
		{
			if ((f_ptr->flags2 & (FF2_HURT_FIRE)) &&
			       (dam > (f_ptr->power*10)))
			{
				/* Check line of sight */
				if ((player_has_los_bold(y, x)) && (f_ptr->flags1 & FF1_NOTICE))
				{
					msg_format("The %s burns up.",f);
				}

				/* Notice any changes */
				obvious = TRUE;

				/* Destroy the feature */
				cave_alter_feat(y, x, FS_HURT_FIRE);
			}

			if (temp_lite(y,x)) obvious = TRUE;

			break;
		}
		case GF_COLD:
		{
			/* Hack -- double cold damage in water */
			if (f_ptr->flags2 & (FF2_WATER)) dam *= 2;

			/* Drop through */
		}
		case GF_ICE:
		{
			if ((f_ptr->flags2 & (FF2_HURT_COLD)) &&
			       (dam > (f_ptr->power*10)))
			{
				/* Check line of sight */
				if ((player_has_los_bold(y, x)) && (f_ptr->flags1 & FF1_NOTICE))
				{
					msg_format("The %s freezes.",f);
				}

				/* Notice any changes */
				obvious = TRUE;

				/* Destroy the feature */
				cave_alter_feat(y, x, FS_HURT_COLD);
			}
			break;
		}
		case GF_EXPLODE:
		case GF_METEOR:
		case GF_SHARD:
		case GF_FORCE:
		case GF_SOUND:
		{
			if ((f_ptr->flags2 & (FF2_KILL_HUGE)) &&
			       (dam > (f_ptr->power*10)))
			{
				/* Check line of sight */
				if ((player_has_los_bold(y, x)) && (f_ptr->flags1 & FF1_NOTICE))
				{
					msg_format("The %s shatters.",f);
				}

				/* Notice any changes */
				obvious = TRUE;

				/* Destroy the feature */
				cave_alter_feat(y, x, FS_KILL_HUGE);
			}
			break;
		}
		/* Electricity */
		case GF_ELEC:
		{
			/* Hack -- double electricy damage in water */
			if (f_ptr->flags2 & (FF2_WATER)) dam *= 2;
		
			if ((f_ptr->flags3 & (FF3_HURT_ELEC)) &&
			       (dam > (f_ptr->power*10)))
			{
				/* Check line of sight */
				if ((player_has_los_bold(y, x)) && (f_ptr->flags1 & FF1_NOTICE))
				{
					msg_format("The %s is struck by lightening.",f);
				}

				/* Notice any changes */
				obvious = TRUE;

				/* Destroy the feature */
				cave_alter_feat(y, x, FS_HURT_ELEC);
			}

			if (temp_lite(y,x)) obvious = TRUE;
			break;
		}

		/* Water */
		case GF_WATER_WEAK:
		case GF_SALT_WATER:
		case GF_WATER:
		{
			if ((f_ptr->flags3 & (FF3_HURT_WATER)) &&
			       (dam > (f_ptr->power*10)))
			{
				/* Check line of sight */
				if ((player_has_los_bold(y, x)) && (f_ptr->flags1 & FF1_NOTICE))
				{
					msg_format("The %s floods.",f);
				}

				/* Notice any changes */
				obvious = TRUE;

				/* Destroy the feature */
				cave_alter_feat(y, x, FS_HURT_WATER);
			}
			break;
		}
		
		/* Boiling water */
		case GF_STEAM:
		case GF_BWATER:
		{

			if ((f_ptr->flags3 & (FF3_HURT_BWATER)) &&
			       (dam > (f_ptr->power*10)))
			{
				/* Check line of sight */
				if ((player_has_los_bold(y, x)) && (f_ptr->flags1 & FF1_NOTICE))
				{
					msg_format("The %s evapourates.",f);
				}

				/* Notice any changes */
				obvious = TRUE;

				/* Destroy the feature */
				cave_alter_feat(y, x, FS_HURT_BWATER);
			}
			break;
		}
		case GF_POIS:
		{
			if ((f_ptr->flags3 & (FF3_HURT_POIS)) &&
			       (dam > (f_ptr->power*10)))
			{
				/* Check line of sight */
				if ((player_has_los_bold(y, x)) && (f_ptr->flags1 & FF1_NOTICE))
				{
					msg_format("The %s is poisoned.",f);
				}

				/* Notice any changes */
				obvious = TRUE;

				/* Destroy the feature */
				cave_alter_feat(y, x, FS_HURT_POIS);
			}
			break;
		}

		/* Ignore some effects */
		case GF_HOLY_ORB:
		{
			break;
		}


		/* Destroy Traps (and Locks) */
		case GF_KILL_TRAP:
		{
			/* Reveal secret doors */
			if (cave_feat[y][x] == FEAT_SECRET)
			{
				/* Create closed door */
				cave_set_feat(y, x, FEAT_DOOR_CLOSED);

				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					obvious = TRUE;
				}
			}

			/* Destroy traps */
			if (f_ptr->flags1 & (FF1_TRAP))
			{
				/* Check line of sight */
				if ((player_has_los_bold(y, x)) && (f_ptr->flags1 & FF1_NOTICE))
				{
					msg_print("There is a bright flash of light!");
				}

				/* Notice any changes */
				obvious = TRUE;

				/* Temporarily lite the square */
				temp_lite(y,x);

				/* Destroy the trap */
				cave_alter_feat(y, x, FS_DISARM);
			}

			/* Disarm other traps */
			else if (f_ptr->flags1 & (FF1_DISARM))
			{
				/* Check line of sight */
				if (/*(player_has_los_bold(y, x)) &&*/ (f_ptr->flags1 & FF1_NOTICE))
				{
					msg_print("Click!");
				}

				/* Notice any changes */
				obvious = TRUE;

				/* Destroy the trap */
				cave_alter_feat(y, x, FS_TUNNEL);
			}

			/* Locked doors are unlocked */
			else if (f_ptr->flags1 & (FF1_OPEN | FF1_BASH))
			{
				/* Unlock the door */
				cave_alter_feat(y, x, FS_OPEN);
	
				/* Check line of sound */
				if ((player_has_los_bold(y, x)) && (f_ptr->flags1 & FF1_NOTICE))
				{
					msg_print("Click!");
				}

				/* Notice any changes */
				obvious = TRUE;
			}

			break;
		}

		/* Destroy Doors (and traps) */
		case GF_KILL_DOOR:
		{

			/* Destroy traps */
			if (f_ptr->flags1 & (FF1_DOOR | FF1_TRAP))
			{
				/* Check line of sight */
				if ((player_has_los_bold(y, x)) && (f_ptr->flags1 & FF1_NOTICE))
				{
					msg_print("There is a bright flash of light!");
				}

				/* Notice any changes */
				obvious = TRUE;

				/* Destroy the door */
				cave_alter_feat(y, x, FS_TUNNEL);

				/* Temporarily lite */
				temp_lite(y, x);
			}

			break;
		}
		
		/* Bash doors or jam doors */
		case GF_HURT_WOOD:
		{
			/* Hack -- fail if terrain is not 'wooden' enough */
			if ((f_ptr->x_attr != 'U') && (f_ptr->x_attr != 'u')
				&& ((f_ptr->flags3 & (FF3_LIVING)) == 0))
			{
				break;
			}

			/* Hack -- if we can't spike it, bash it instead */
			if ((((f_ptr->flags1 & (FF1_SPIKE)) == 0)
				|| (feat_state(feat, FS_SPIKE) == feat)) &&
				((f_ptr->flags1 & (FF1_BASH)) != 0))
			{
				/* Check line of sight */
				if ((player_has_los_bold(y, x)) && (f_ptr->flags1 & FF1_NOTICE))
				{
					msg_format("The %s warps.", f);
				}

				/* Notice any changes */
				obvious = TRUE;

				/* Destroy the door */
				cave_alter_feat(y, x, FS_BASH);				
				
				break;
			}

			/* Fall through */
		}
		
		/* Jam Doors */
		case GF_LOCK_DOOR:
		{
			/* Close doors/traps/chests */
			if (f_ptr->flags1 & (FF1_CLOSE))
			{
				/* Check line of sight */
				if ((player_has_los_bold(y, x)) && (f_ptr->flags1 & FF1_NOTICE))
				{
					msg_format("The %s slams shut.", f);
				}

				/* Notice any changes */
				obvious = TRUE;

				/* Destroy the door */
				cave_alter_feat(y, x, FS_CLOSE);
			}

			/* Jam doors */
			while (f_info[cave_feat[y][x]].flags1 & (FF1_SPIKE))
			{
				int feat = cave_feat[y][x];

				/* Jam the door */
				cave_alter_feat(y, x, FS_SPIKE);

				/* Paranoia */
				if (feat == cave_feat[y][x]) break;

				/* Notice any changes */
				obvious = TRUE;
			}

			break;
		}


		/* Destroy walls (and doors) */
		case GF_KILL_WALL:
		{

			/* Destroy walls/doors */
			if (f_ptr->flags2 & (FF2_HURT_ROCK))
			{
				/* Check line of sight */
				if ((player_has_los_bold(y, x)) && (f_ptr->flags1 & FF1_NOTICE))
				{
					msg_format("The %s turns to mud.", f);
				}

				/* Notice any changes */
				obvious = TRUE;

				/* Destroy the trap */
				cave_alter_feat(y, x, FS_HURT_ROCK);
			}

			break;
		}

		/* Make doors */
		case GF_MAKE_DOOR:
		{
			/* Require a "naked" floor grid */
			if (!cave_naked_bold(y, x)) break;

			/* Create closed door */
			cave_set_feat(y, x, FEAT_DOOR_CLOSED);

			/* Observe */
			if (play_info[y][x] & (PLAY_MARK)) obvious = TRUE;

			break;
		}

		/* Make traps */
		case GF_MAKE_TRAP:
		{
			/* Place a trap */
			feat_near(FEAT_INVIS, y, x);

			break;
		}

		/* Make features */
		case GF_FEATURE:
		{
			object_type object_type_body;
			object_type *o_ptr = &object_type_body;
			
			burnout = FALSE;

			/* Require a "floor or ground" grid */
			if (!(f_ptr->flags1 & (FF1_FLOOR))
			 && !(f_ptr->flags3 & (FF3_GROUND))) break;

			/* If caster is player, place a 'counter' or 'timer' */
			if ((who <= SOURCE_PLAYER_START) || ((who > 0) && (m_list[who].mflag & (MFLAG_ALLY))))
			{
				object_prep(o_ptr, lookup_kind(TV_SPELL, SV_SPELL_COUNTER));
				
				/* Hack -- record feature under terrain to allow 'reversion' to old terrain */
				o_ptr->pval = cave_feat[y][x];
				
				/* Traps are given so many charges */
				if (f_info[dam].flags1 & (FF1_HIT_TRAP))
				{
					o_ptr->charges = (p_ptr->lev + 1) / 2;
				}
				/* Other features time out relatively quickly.
				 * Note this must be the same for all terrain in a single casting
				 * otherwise e.g. bridges of stone will time out unusually.
				 */
				else
				{
					o_ptr->timeout = (p_ptr->lev + 1) / 2;
				}
				
				/* Part of the terrain */
				o_ptr->ident |= (IDENT_STORE);
			}

			/* Place a feature */
			if (dam) cave_set_feat(y,x,dam);

			/* Place the counter */			
			if ((who <= SOURCE_PLAYER_START) || ((who > 0) && (m_list[who].mflag & (MFLAG_ALLY))))
			{
				/* Add to the floor */
				floor_carry(y,x,o_ptr);
			}

			/* Notice any changes */
			obvious = TRUE;

			break;
		}		

		/* Make bridge */
		case GF_BRIDGE:
		{
			int old_feat = cave_feat[y][x];
			char name[80];

			if (f_ptr->flags1 & (FF1_SECRET)) cave_alter_feat(y,x,FS_SECRET);

			cave_alter_feat(y,x,FS_BRIDGE);

			my_strcpy(name,f_name+f_ptr->name, sizeof(name));

			if (!(strstr(name,"stone bridge")))
			{
				cave_set_feat(y,x,old_feat);
			}

			/* Notice any changes */
			else obvious = TRUE;

			break;
		}

		/* Raise water */
		case GF_RAISE:
		{
			int feat = cave_feat[y][x];

			if (f_info[feat].flags2 & (FF2_WATER) && !((f_info[feat].flags2 & (FF2_CHASM)) ||
				(f_info[feat].flags1 & (FF1_LESS)) || (f_info[feat].flags2 & (FF2_FILLED))))
			{
				cave_set_feat(y,x,FEAT_WATER_K);

				/* Notice any changes */
				obvious = TRUE;
			}
			break;
		}

		/* Lower water */
		case GF_LOWER:
		{
			int feat = cave_feat[y][x];

			if (prefix(f_name+f_info[feat].name,"stone bridge"))
			{
				/* Hack -- change bridges to prevent exploits */
				cave_set_feat(y, x, FEAT_BRIDGE_CHASM);
			}
			
			else if (f_info[feat].flags2 & (FF2_CAN_SWIM))
			{
				cave_set_feat(y,x,FEAT_EARTH);

				/* Notice any changes */
				obvious = TRUE;
			}
			break;
		}


		/* Lite up the grid */
		case GF_LITE_WEAK:
		case GF_LITE:
		{
			int i;

			/* Turn on the light */
			cave_info[y][x] |= (CAVE_GLOW);

			/* Grid is in line of sight */
			if (player_has_los_bold(y, x))
			{
				/* Observe */
				obvious = TRUE;

				/* Fully update the visuals */
				p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);
			}

			if (!(cave_info[y][x] & (CAVE_XLOS)))
				for (i = 0; i < 8; i++)
			{
				int yy = y + ddy_ddd[i];
				int xx = x + ddx_ddd[i];

				/* Ignore annoying locations */
				if (in_bounds_fully(yy, xx))
				{
					/* Turn on the light */
					cave_info[yy][xx] |= (CAVE_GLOW);
				}

				/* Grid is in line of sight */
				if (player_has_los_bold(yy, xx))
				{
					/* Observe */
					obvious = TRUE;

					/* Fully update the visuals */
					p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);
				}
			}

			break;
		}

		/* Darken the grid */
		case GF_DARK_WEAK:
		case GF_DARK:
		{
			/* Turn off the light */
			cave_info[y][x] &= ~(CAVE_GLOW);

			/* Hack -- Forget "boring" grids */
			if (cave_feat[y][x] <= FEAT_INVIS)
			{
				/* Forget */
				play_info[y][x] &= ~(PLAY_MARK);
			}

			/* Grid is in line of sight */
			if (player_has_los_bold(y, x))
			{
				/* Observe */
				obvious = TRUE;

				/* Fully update the visuals */
				p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);
			}

			/* All done */
			break;
		}

		/* Animate trees */
		case GF_ANIM_TREE:
		{
			if (f_ptr->flags2 & (FF3_TREE)) break;

			summoner = 0;

			if (summon_specific(y, x, who > SOURCE_MONSTER_START ? r_info[who].level - 1 : p_ptr->depth, ANIMATE_TREE,
					FALSE, (MFLAG_MADE) |  (who == SOURCE_PLAYER_CAST ? MFLAG_ALLY : 0L))) cave_set_feat(y,x,FEAT_EARTH);

			break;
		}

		/* Animate elements */
		case GF_ANIM_ELEMENT:
		{
			int i;

			summoner = 0;
			summon_group_type = 0;

			for (i = 0; i < MAX_ELEMENTS; i++)
			{
				if ((element[i].effect == f_ptr->blow.effect) || (f_ptr->flags2 & (element[i].flags2)))
				{
					summon_group_type = element[i].grp_idx;
					break;
				}
			}

			if (summon_group_type)
			{
				if (summon_specific(y, x, who > SOURCE_MONSTER_START ? r_info[who].level - 1 : p_ptr->depth, ANIMATE_ELEMENT,
					FALSE, (MFLAG_MADE) |  (who == SOURCE_PLAYER_CAST ? MFLAG_ALLY : 0L))) cave_set_feat(y,x,FEAT_EARTH);
			}

			break;
		}

		/* Animate objects */
		case GF_ANIM_OBJECT:
		{
			bool change = FALSE;

			summoner = 0;
			summon_attr_type = 0;
			summon_char_type = 0;

			/* Animate chests */
			if (f_ptr->flags3 & (FF3_CHEST))
			{
				summon_char_type = '&';
			}

			/* Animate embedded objects */
			else if (f_ptr->k_idx)
			{
				object_kind *k_ptr = &k_info[f_ptr->k_idx];

				/* XXX Be careful not to animate water as potion mimics */
				if (k_ptr->tval != TV_POTION)
				{
					summon_attr_type = k_ptr->d_attr;
					summon_char_type = k_ptr->d_char;
				}
			}

			/* Animate anything else */
			/* XXX Be careful not to animate rubble / deep terrain as trees / shallow terrain as plants / 
				hidden objects as chest mimics */
			else if ((f_ptr->d_char != ':') && (f_ptr->d_char != ';') && (f_ptr->d_char != '&'))
			{
				summon_attr_type = f_ptr->d_attr;
				summon_char_type = f_ptr->d_char;
			}

			if (summon_attr_type || summon_char_type)
			{
				if (summon_specific(y, x, who > SOURCE_MONSTER_START ? r_info[who].level-1 : p_ptr->depth, ANIMATE_OBJECT,
					FALSE, (MFLAG_MADE) |  (who == SOURCE_PLAYER_CAST ? MFLAG_ALLY : 0L))) change = TRUE;
			}

			if (change)
			{
				if (f_ptr->flags3 & (FF3_GET_FEAT)) cave_alter_feat(y, x, FS_GET_FEAT);
				else if (f_ptr->flags3 & (FF3_OUTSIDE)) cave_set_feat(y,x, FEAT_EARTH);
				else cave_set_feat(y,x,FEAT_FLOOR);

				/* Notice any changes */
				obvious = TRUE;
			}

			break;
		}

		case GF_WEB:
		{
			/* Create webs on floor */
			if (f_ptr->flags1 & (FF1_FLOOR))
			{
				cave_set_feat(y, x, FEAT_FLOOR_WEB);

				/* Notice any changes */
				obvious = TRUE;
			}

			/* Create webs on chasm */
	                else if (f_ptr->flags2 & (FF2_CHASM))
			{
				cave_set_feat(y, x, FEAT_CHASM_WEB);

				/* Notice any changes */
				obvious = TRUE;
			}

			break;
		}

		case GF_BLOOD:
		{
			/* Create blood on floor */
			if (f_ptr->flags1 & (FF1_FLOOR))
			{
				cave_set_feat(y, x, FEAT_FLOOR_BLOOD_T);

				/* Notice any changes */
				obvious = TRUE;
			}
			break;
		}

		case GF_SLIME:
		{
			/* Create slime on floor */
			if (f_ptr->flags1 & (FF1_FLOOR))
			{
				cave_set_feat(y, x, FEAT_FLOOR_SLIME_T);

				/* Notice any changes */
				obvious = TRUE;
			}

			break;
		}

		case GF_WIND:
		{
			/* Blow out flames and other timed effects */
			if (f_ptr->flags3 & (FF3_TIMED))
			{
				/* Check line of sight */
				if ((player_has_los_bold(y, x)) && (f_ptr->flags1 & FF1_NOTICE))
				{
					msg_format("The %s blows out.",f);
				}

				/* Notice any changes */
				obvious = TRUE;

				cave_alter_feat(y,x,FS_TIMED);
			}

			/* Blow out smoke and other spreading effects */
			else if (f_ptr->flags3 & (FF3_SPREAD))
			{
				/* Check line of sight */
				if ((player_has_los_bold(y, x)) && (f_ptr->flags1 & FF1_NOTICE))
				{
					msg_format("The %s blows away.",f);
				}

				/* Notice any changes */
				obvious = TRUE;

				cave_alter_feat(y,x,FS_SPREAD);
			}

			break;
		}

		default:

			dam=0;
			break;
	}

	/* Apply burnout */
	if ((burnout) && (feat != cave_feat[y][x]) &&
		((f_info[cave_feat[y][x]].flags3 & (FF3_SPREAD)) != 0))
	{
		cave_alter_feat(y,x,FS_SPREAD);
	}

	/* Check for monster */
	if (feat != cave_feat[y][x])
	{
		/* Player or monster was affected -- Mark grid for later processing. */
		if (cave_m_idx[y][x] != 0) cave_temp_mark(y, x, FALSE);
	}

	/* Return "Anything seen?" */
	return (obvious);
}



/*
 * We are called from "project()" to "damage" objects
 *
 * We are called both for "beam" effects and "ball" effects.
 *
 * Perhaps we should only SOMETIMES damage things on the ground.
 *
 * The "r" parameter is the "distance from ground zero".
 *
 * Note that we determine if the player can "see" anything that happens
 * by taking into account: blindness, line-of-sight, and illumination.
 *
 * Hack -- We also "see" objects which are "memorized".
 *
 * We return "TRUE" if the effect of the projection is "obvious".
 */
bool project_o(int who, int what, int y, int x, int dam, int typ)
{
	s16b this_o_idx, next_o_idx = 0;

	bool obvious = FALSE;

	u32b f1, f2, f3, f4;
	u32b if1=0;
	u32b if2=0;
	u32b if3=0;
	u32b if4=0;

	char o_name[80];

	/* Prevent warning */
	(void)who;

	/* Scan all objects in the grid */
	for (this_o_idx = cave_o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		bool is_art = FALSE;
		bool ignore = FALSE;
		bool plural = FALSE;
		bool do_kill = FALSE;
		bool do_break = TRUE;
		cptr note_kill = NULL;
		bool do_move = FALSE;

		int ny, nx;


		/* Get the object */
		o_ptr = &o_list[this_o_idx];

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Paranoia - don't break again */
		if (o_ptr->ident & (IDENT_BREAKS)) continue;

		/* Extract the flags */
		object_flags(o_ptr, &f1, &f2, &f3, &f4);

		/* Get the "plural"-ness */
		if (o_ptr->number > 1) plural = TRUE;

		/* Check for artifact */
		if (artifact_p(o_ptr)) is_art = TRUE;

		/* Hack -- storm can do several things */
		if (typ == GF_STORM)
		{
			switch(rand_int(3))
			{
				case 0: typ = GF_WIND; break;
				case 1: typ = GF_WATER; break;
				case 2: typ = GF_ELEC; break;
			}
		}

		/* Analyze the type */
		switch (typ)
		{
			/* Acid -- Lots of things */
			case GF_ACID:
			{
				/* Hack -- halve acid damage in water */
				if (f_info[cave_feat[y][x]].flags2 & (FF2_WATER)) dam /= 2;

				if (hates_acid(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " melt!" : " melts!");
					if (f2 & (TR2_IGNORE_ACID)) ignore = TRUE;
					if2 |= TR2_IGNORE_ACID;
				}
				break;
			}

			/* Elec -- Nothing now */
			/* However, electricity will drain charges completely from wands and staffs on the floor
			   and massively increase the timeout of rods. */
			case GF_ELEC:
			{
				/* Hack -- double electricy damage in water */
				if (f_info[cave_feat[y][x]].flags2 & (FF2_WATER)) dam *= 2;

				/* Quietly drain charged wands/staffs */
				if (((o_ptr->tval == TV_STAFF) ||
				     (o_ptr->tval == TV_WAND)) &&
				    (o_ptr->charges))
				{
					/* Ignore electricity protects */
					if ((f2 & (TR2_IGNORE_ELEC)) == 0)
					{
						/* Uncharge */
						o_ptr->charges = 0;
						o_ptr->stackc = 0;
					}
				}

				/* Quietly drain charged wands/staffs */
				else if (o_ptr->tval == TV_ROD)
				{
					/* Ignore electricity protects */
					if ((f2 & (TR2_IGNORE_ELEC)) == 0)
					{
						int t = randint(999);

						/* Uncharge */
						if (o_ptr->timeout < t) o_ptr->timeout = t;
					}
				}

				break;
			}

			/* Fire -- Flammable objects */
			case GF_FIRE:
			{
				/* Hack -- halve fire damage in water */
				if (f_info[cave_feat[y][x]].flags2 & (FF2_WATER)) dam /= 2;
			
				if (hates_fire(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " burn up!" : " burns up!");
					if (f2 & (TR2_IGNORE_FIRE)) ignore = TRUE;
					if2 |= TR2_IGNORE_FIRE;
				}
				break;
			}

			/* Cold -- potions and flasks */
			case GF_COLD:
			{
				/* Hack -- double cold damage in water */
				if (f_info[cave_feat[y][x]].flags2 & (FF2_WATER)) dam *= 2;

				if (hates_cold(o_ptr))
				{
					note_kill = (plural ? " shatter!" : " shatters!");
					do_kill = TRUE;
					if (f2 & (TR2_IGNORE_COLD)) ignore = TRUE;
					if2 |= TR2_IGNORE_COLD;
				}
				break;
			}

			/* Water -- books, lites, scrolls, food */
			case GF_WATER:
			case GF_BWATER:
			case GF_WATER_WEAK:
			case GF_SALT_WATER:
			{
				if (hates_water(o_ptr))
				{
					note_kill = (plural ? " soak through!" : " soaks through!");
					do_kill = TRUE;
					if (f2 & (TR2_IGNORE_WATER)) ignore = TRUE;
					if2 |= TR2_IGNORE_WATER;
				}
				break;
			}

			/* Fire + Elec */
			case GF_PLASMA:
			{
				if (hates_fire(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " burn up!" : " burns up!");
					if (f2 & (TR2_IGNORE_FIRE)) ignore = TRUE;
					if2 |= TR2_IGNORE_FIRE;
				}
				break;
			}

			/* Fire + Cold */
			case GF_METEOR:
			{
				if (hates_fire(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " burn up!" : " burns up!");
					if (f2 & (TR2_IGNORE_FIRE)) ignore = TRUE;
					if2 |= TR2_IGNORE_FIRE;
				}
				if (hates_cold(o_ptr))
				{
					ignore = FALSE;
					do_kill = TRUE;
					note_kill = (plural ? " shatter!" : " shatters!");
					if (f2 & (TR2_IGNORE_COLD)) ignore = TRUE;
					if2 |= TR2_IGNORE_COLD;
				}
				break;
			}

			/* Hack -- break potions and such */
			case GF_ICE:
			case GF_SHARD:
			case GF_SOUND:
			{
				if (hates_cold(o_ptr))
				{
					note_kill = (plural ? " shatter!" : " shatters!");
					do_kill = TRUE;
				}
				break;
			}

			/* Raise water -- water through if we are raising water */
			case GF_RAISE:
			{
				if ((cave_feat[y][x] == FEAT_WATER) && (hates_water(o_ptr)))
				{
					note_kill = (plural ? " soak through!" : " soaks through!");
					do_kill = TRUE;
					if (f2 & (TR2_IGNORE_WATER)) ignore = TRUE;
					if2 |= TR2_IGNORE_WATER;
				}
				break;
			}

			/* Explosion -- destroys fire + potions */
			case GF_EXPLODE:
			{
				if (hates_fire(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " burn up!" : " burns up!");
					if (f2 & (TR2_IGNORE_FIRE)) ignore = TRUE;
					if2 |= TR2_IGNORE_FIRE;
				}
				if (hates_cold(o_ptr))
				{
					ignore = FALSE;
					do_kill = TRUE;
					note_kill = (plural ? " shatters!" : " shatters!");
					if (f2 & (TR2_IGNORE_COLD)) ignore = TRUE;
					if2 |= TR2_IGNORE_COLD;
				}
				break;
			}

			/* Mana -- very destructive to objects */
			case GF_MANA:
			{
				do_kill = TRUE;
				note_kill = (plural ? " are destroyed!" : " is destroyed!");
				break;
			}

			/* Holy Orb -- destroys cursed non-artifacts */
			case GF_HOLY_ORB:
			{
				if (cursed_p(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " are destroyed!" : " is destroyed!");
				}
				break;
			}

			/* Hellfire -- destroys non-cursed non-artifacts */
			case GF_HELLFIRE:
			{
				if (!cursed_p(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " are destroyed!" : " is destroyed!");
				}
				break;
			}

			/* Force is magnetic */
			case GF_FORCE:
			{
				if ((hates_acid(o_ptr)) && (o_ptr->weight <= dam))
				{
					int dist = 1 + (dam - o_ptr->weight) / 33;

					if (who <= SOURCE_PLAYER_START)
					{
						ny = p_ptr->py;
						nx = p_ptr->px;
					}
					else if (who > SOURCE_MONSTER_START)
					{
						ny = m_list[who].fy;
						nx = m_list[who].fx;
					}

					if ((who > SOURCE_MONSTER_START) || (who <= SOURCE_PLAYER_START))
					{
						int path_n;
						u16b path_g[256];

						/* Calculate the path */
						path_n = project_path(path_g, dist, y, x, &ny, &nx, 0);

						ny = GRID_Y(path_g[dist]);
						nx = GRID_X(path_g[dist]);
					}
					else
					{
						nx = 0;
						ny = 0;

						scatter(&ny, &nx, y, x, dist, 0);
					}

					if (ny != y || nx != x) do_move = TRUE;
				}
				break;
			}

			/* Wind blows things around */
			case GF_WIND:
			{
				if (o_ptr->weight <= dam)
				{
					nx = 0;
					ny = 0;

					scatter(&ny, &nx, y, x, 1 + (dam - o_ptr->weight) / 33, 0);
					if (ny != y || nx != x) do_move = TRUE;
				}
				break;
			}

			/* Animate objects */
			case GF_ANIM_OBJECT:
			{
				if (!is_art)
				{
					bool summons = FALSE;
					int i;
					
					summoner = 0;
					
					if (k_info[o_ptr->k_idx].flavor)
					{
						summon_attr_type = x_info[k_info[o_ptr->k_idx].flavor].d_attr;
						summon_char_type = x_info[k_info[o_ptr->k_idx].flavor].d_char;
					}
					else
					{
						summon_attr_type = k_info[o_ptr->k_idx].d_attr;
						summon_char_type = k_info[o_ptr->k_idx].d_char;						
					}

					/* Hack -- animate statues */
					if (k_info[o_ptr->k_idx].tval == TV_STATUE) summon_char_type = 'g';

					/* Hack -- animate food */
					if ((k_info[o_ptr->k_idx].tval == TV_FOOD) && (k_info[o_ptr->k_idx].sval >= SV_FOOD_MIN_FOOD)) summon_char_type = '!';

					/* Hack -- animate spikes */
					if (k_info[o_ptr->k_idx].tval == TV_SPIKE) summon_char_type = '|';

					/* Hack -- animate assemblies */
					if ((k_info[o_ptr->k_idx].tval == TV_ASSEMBLY) && (o_ptr->name3))
					{
						summon_race_type = o_ptr->name3;
						for (i = 0; i < o_ptr->number; i++)
							summons |= (summon_specific(y, x, 99, SUMMON_FRIEND, FALSE, who == SOURCE_PLAYER_CAST ? MFLAG_ALLY : 0L));
							
						if (summons)
						{
							do_kill = TRUE;
							do_break = FALSE;
							note_kill = (plural ? " animate!" : " animates!");
						}
					}
					else
					{
						for (i = 0; i < o_ptr->number; i++)
							summons |= (summon_specific(y, x, who > SOURCE_MONSTER_START ? r_info[who].level - 1 : p_ptr->depth, ANIMATE_OBJECT,
							FALSE, (MFLAG_MADE) |  (who == SOURCE_PLAYER_CAST ? MFLAG_ALLY : 0L)));
							
						if (summons)
						{
							do_kill = TRUE;
							do_break = FALSE;
							note_kill = (plural ? " animate!" : " animates!");
						}
					}
				}
				else
				{
					/* Hack -- force artifacts to be noticed */
					do_kill = TRUE;
				}
				break;
			}

			/* Animate dead */
			case GF_ANIM_DEAD:
			{
				summoner = 0;
				summon_char_type = 0;
				summon_attr_type = 0;
				summon_flag_type = 0L;

				switch (k_info[o_ptr->k_idx].tval)
				{
					case TV_BONE:
					{
						if (k_info[o_ptr->k_idx].sval == SV_BONE_SKELETON)
						{
							summon_char_type = 's';
						}
						else
						{
							summon_char_type = k_info[o_ptr->k_idx].d_char;
							summon_attr_type = k_info[o_ptr->k_idx].d_attr;
						}
						break;
					}
					case TV_BODY:
					{
						if ((k_info[o_ptr->k_idx].sval == SV_BODY_CORPSE) || 
							(k_info[o_ptr->k_idx].sval == SV_BODY_HEADLESS) ||
							(k_info[o_ptr->k_idx].sval == SV_BODY_BUTCHERED))
						{
							 summon_char_type = 'z';
						}
						/* MegaHack -- animate heads */
						else if (k_info[o_ptr->k_idx].sval == SV_BODY_HEAD)
						{
							summon_char_type = '~';
							summon_attr_type = TERM_WHITE;
						}
						/* Hack -- other body parts */
						else
						{
							summon_char_type = k_info[o_ptr->k_idx].d_char;
							summon_attr_type = k_info[o_ptr->k_idx].d_attr;							
						}
						break;
					}
				}

				/* Skeletons and zombies can be racially flavoured */
				if ((summon_char_type) && !(summon_attr_type) && (o_ptr->name3))
				{
					summon_flag_type |= r_info[o_ptr->name3].flags3 & (RF3_RACE_MASK);
					summon_flag_type |= r_info[o_ptr->name3].flags9 & (RF9_RACE_MASK);

					/* Hack -- non-humanoids animate to undead of their own graphic. Added eyes as a hack. */
					if (((summon_flag_type & (RF3_ANIMAL | RF3_INSECT | RF3_PLANT | RF3_DRAGON)) != 0) || (r_info[o_ptr->name3].d_char == 'e'))
					{
						summon_flag_type = (RF3_UNDEAD);
						summon_attr_type = 0;

						summon_char_type = r_info[o_ptr->name3].d_char;
					}
				}

				if (summon_char_type || summon_attr_type || summon_flag_type)
				{
					bool summons = FALSE;
					int i;
					
					for (i = 0; i < o_ptr->number; i++)
							summons |= (summon_specific(y, x, who > SOURCE_MONSTER_START ? r_info[who].level - 1 : p_ptr->depth, ANIMATE_DEAD,
						FALSE, (MFLAG_MADE) |  (who == SOURCE_PLAYER_CAST ? MFLAG_ALLY : 0L)));
					
					if (summons)
					{
						do_kill = TRUE;
						do_break = FALSE;
						note_kill = (plural ? " animate!" : " animates!");
					}
				}
				break;
			}

			/* Raise dead */
			case GF_RAISE_DEAD:
			{
				bool raise = FALSE;
				bool summons = FALSE;
				int i;
				
				summoner = 0;

				switch (k_info[o_ptr->k_idx].tval)
				{
					case TV_BONE:
					{
						if (k_info[o_ptr->k_idx].sval == SV_BONE_SKELETON) raise = TRUE;
						break;
					}
					case TV_BODY:
					{
						if ((k_info[o_ptr->k_idx].sval == SV_BODY_CORPSE) || 
							(k_info[o_ptr->k_idx].sval == SV_BODY_HEADLESS) ||
							(k_info[o_ptr->k_idx].sval == SV_BODY_BUTCHERED))
							 raise = TRUE;
						break;
					}
				}

				if ((raise) && (o_ptr->name3) && !(summon_race_type))
				{
					summon_race_type = o_ptr->name3;

					for (i = 0; i < o_ptr->number; i++)
							summons |= (summon_specific(y, x, who > SOURCE_MONSTER_START ? r_info[who].level - 1 : p_ptr->depth, RAISE_DEAD,
						FALSE, who == SOURCE_PLAYER_CAST ? MFLAG_ALLY : 0L));
					
					if (summons)
					{
						do_kill = TRUE;
						do_break = FALSE;
						note_kill = (plural ? " come back to life!" : " comes back to life!");
					}
						

					summon_race_type = 0;
				}
				else if ((raise) && (o_ptr->name3 == summon_race_type))
				{
					for (i = 0; i < o_ptr->number; i++)
							summons |= (summon_specific(y, x, 99, RAISE_DEAD, FALSE, (MFLAG_MADE) | (who == SOURCE_PLAYER_CAST ? MFLAG_ALLY : 0L)));
					
					if (summons)
					{
						do_kill = TRUE;
						do_break = FALSE;
						note_kill = (plural ? " come back to life!" : " comes back to life!");
					}
				}
				
				break;
			}
			
			/* Wood warping -- wooden objects */
			case GF_HURT_WOOD:
			{
				if (hates_warpwood(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " warp!" : " warps!");
				}
				break;
			}			
		}


		/* Attempt to destroy the object */
		if (do_kill)
		{
			/* Effect "observed" */
			if (o_ptr->ident & (IDENT_MARKED))
			{
				obvious = TRUE;
				object_desc(o_name, sizeof(o_name), o_ptr, FALSE, 0);
			}

			/* Artifacts, and other objects, get to resist */
			if (is_art || ignore)
			{
				/* Observe the resist */
				if (obvious)
				{
					/* Get known flags */
					u32b k1,n1;
					u32b k2,n2;
					u32b k3,n3;
					u32b k4,n4;

					k1 = o_ptr->can_flags1;
					k2 = o_ptr->can_flags2;
					k3 = o_ptr->can_flags3;
					k4 = o_ptr->can_flags4;

					/* Learn about resistences */
					if (if1 | if2 | if3 | if4)
					{
						object_can_flags(o_ptr,if1,if2,if3,if4, TRUE);
					}

					/* Item is unbreakable */
					else
					{
						if (!o_ptr->feeling && !(o_ptr->ident & (IDENT_SENSE))
							&& object_named_p(o_ptr))
						{
							/* Sense the object */
							o_ptr->feeling = INSCRIP_UNBREAKABLE;

							/* Hack -- for holy orb */
							if (typ == GF_HOLY_ORB) o_ptr->feeling = INSCRIP_TERRIBLE;

							/* The object has been "sensed" */
							o_ptr->ident |= (IDENT_SENSE);
						}

						object_can_flags(o_ptr,0x0L,TR2_IGNORE_MASK,0x0L,0x0L, TRUE);
					}

					/* Check for new flags */
					n1 = o_ptr->can_flags1 & ~(k1);
					n2 = o_ptr->can_flags2 & ~(k2);
					n3 = o_ptr->can_flags3 & ~(k3);
					n4 = o_ptr->can_flags3 & ~(k4);

					if (n1 || n2 || n3 || n4) msg_format("The %s%s unaffected!",
									o_name, 
									plural ? " are" : " is");

				}	
			}

			/* Kill it */
			else
			{
				/* Describe if needed */
				if (((o_ptr->ident & (IDENT_MARKED)) != 0) && note_kill)
				{
					msg_format("The %s%s", o_name, note_kill);
				}

				/* Break open containers */
				if ((o_ptr->tval == TV_HOLD) && (o_ptr->name3 > 0))
				{
					while (o_ptr->number)
					{
						race_near(o_ptr->name3, y, x);

						o_ptr->number--;
					}
				}

				if (do_break)
				{
					/* Delete the object later -- needs to happen before project_f below */
					o_ptr->ident |= (IDENT_BREAKS);

					/* Splash damage on terrain */
					(void)project_f(SOURCE_OBJECT, what, y, x, damroll(1, o_ptr->weight), typ);

					/* And apply effects */
					(void)project_t(SOURCE_OBJECT, what, y, x, damroll(1, o_ptr->weight), typ);
				}
				else
				{
					/* Delete the object */
					delete_object_idx(this_o_idx);
				}

				/* Redraw */
				lite_spot(y, x);
			}
		}

		/* Move the object */
		else if (do_move)
		{
			/* Local object */
			object_type object_type_body;

			/* Get local object */
			object_type *i_ptr = &object_type_body;

			/* Wipe the new object */
			object_wipe(i_ptr);

			/* Make a copy */
			object_copy(i_ptr, o_ptr);

			/* Effect "observed" */
			if (o_ptr->ident & (IDENT_MARKED))
			{
				obvious = TRUE;
			}

			/* Delete the object in its old location */
			delete_object_idx(this_o_idx);

			/* Drop it near the new location */
			drop_near(i_ptr, -1, ny, nx);

			/* Redraw */
			lite_spot(y, x);

			continue;
		}
	}

	/* Return "Anything seen?" */
	return (obvious);
}


/*
 * Check if enough monsters are terrified by the player.
 */
static void check_fear_quest(int m_idx)
{
	int i, j, n;

	/* Check quests for completion */
	for (i = 0; i < MAX_Q_IDX; i++)
	{
		quest_type *q_ptr = &(q_list[i]);
		quest_event *qe_ptr = &(q_ptr->event[q_ptr->stage]);

		/* Local object */
		object_type object_type_body;

		/* Get local object */
		object_type *o_ptr;

		if (q_ptr->stage == QUEST_ACTION) qe_ptr = &(q_ptr->event[QUEST_ACTIVE]);

		if ((qe_ptr->dungeon != p_ptr->dungeon) ||
			(qe_ptr->level != p_ptr->depth - min_depth(p_ptr->dungeon))) continue;

		if (!(qe_ptr->race) || (qe_ptr->race != m_list[m_idx].r_idx)) continue;

		/* Assign quest */
		if (q_ptr->stage == QUEST_ASSIGN)
		{
			/* Wipe the structure */
			(void)WIPE(qe_ptr, quest_event);

			qe_ptr->dungeon = p_ptr->dungeon;
			qe_ptr->level = p_ptr->depth - min_depth(p_ptr->dungeon);
			qe_ptr->race = m_list[m_idx].r_idx;
			qe_ptr->number = 1;
			qe_ptr->flags |= (EVENT_FEAR_RACE);

			quest_assign(i);

			continue;
		}

		if (q_ptr->stage != QUEST_ACTION) continue;

		n = 0;

		for (j = 0; j < z_info->m_max; j++)
		{
			monster_type *m_ptr = &m_list[j];

			if (m_ptr->r_idx != qe_ptr->race) continue;

			if (!m_ptr->monfear) continue;

			n++;
		}

		/* If last monster terrified, drop artifact */
		if ((q_ptr->event[QUEST_ACTION].number + n >= qe_ptr->number) && (qe_ptr->artifact))
		{
			/* Get local object */
			o_ptr = &object_type_body;

			/* Wipe the object */
			object_wipe(o_ptr);

			/* Prepare artifact */
			qe_ptr->kind = lookup_kind(a_info[qe_ptr->artifact].tval, a_info[qe_ptr->artifact].sval);

			/* Prepare object */
			object_prep(o_ptr, qe_ptr->kind);

			/* Prepare artifact */
			o_ptr->name1 = qe_ptr->artifact;

			/* Apply magic */
			apply_magic(o_ptr, object_level, FALSE, FALSE, FALSE);

			/* Drop it in the dungeon */
			drop_near(o_ptr, -1, m_list[m_idx].fy, m_list[m_idx].fx);
		}

		/* Hack -- If last monster terrified, all monsters drop items */
		else if ((qe_ptr->kind) || (qe_ptr->ego_item_type))
		{
			for (j = 0; j < z_info->m_max; j++)
			{
				monster_type *m_ptr = &m_list[j];

				if (m_ptr->r_idx != qe_ptr->race) continue;

				if (!m_ptr->monfear) continue;

				/* Get local object */
				o_ptr = &object_type_body;

				/* Wipe the object */
				object_wipe(o_ptr);

				/* Prepare ego item */
				if ((qe_ptr->ego_item_type) && !(qe_ptr->kind)) qe_ptr->kind =
					lookup_kind(e_info[qe_ptr->ego_item_type].tval[0],
						e_info[qe_ptr->ego_item_type].min_sval[0]);

				/* Prepare object */
				object_prep(o_ptr, qe_ptr->kind);

				/* Prepare ego item */
				o_ptr->name2 = qe_ptr->ego_item_type;

				/* Apply magic */
				apply_magic(o_ptr, object_level, FALSE, FALSE, FALSE);

				/* Drop it in the dungeon */
				drop_near(o_ptr, -1, m_ptr->fy, m_ptr->fx);
			}
		}

		/* Update actions */
		qe_ptr = &(q_ptr->event[QUEST_ACTION]);

		/* Fail quest because we terrified someone */
		if (q_ptr->event[QUEST_FAILED].flags & (EVENT_FEAR_RACE))
		{
			/* Wipe the structure */
			(void)WIPE(qe_ptr, quest_event);

			/* Set action details */
			qe_ptr->dungeon = p_ptr->dungeon;
			qe_ptr->level = p_ptr->depth - min_depth(p_ptr->dungeon);
			qe_ptr->race = m_list[m_idx].r_idx;
			qe_ptr->number = 1;
			qe_ptr->flags |= (EVENT_FEAR_RACE);

			quest_penalty(i);
		}

		/* Get closer to success because we need to terrify someone */
		else if ((qe_ptr->flags & (EVENT_FEAR_RACE)) && (qe_ptr->number + n >= q_ptr->event[QUEST_ACTIVE].number))
		{
			qe_ptr->number = q_ptr->event[QUEST_ACTIVE].number;

			qe_ptr->flags |= (EVENT_FEAR_RACE);

			/* Have completed quest? */
			if ((qe_ptr->flags == q_ptr->event[QUEST_ACTIVE].flags) && (qe_ptr->number <= q_ptr->event[QUEST_ACTIVE].number))
			{
				msg_print("Congratulations. You have succeeded at your quest.");

				quest_reward(i);
			}
			/* Partially completed quest */
			else
			{
				msg_print("You have xxx to go.");
			}
		}
	}
}


/*
 *  Monster saving throw.
 *
 *  Hack -- we manipulate wisdom to indicate the player is nearly affecting the
 *  monster to encourage them to have another go...
 */
static bool monster_save(monster_type *m_ptr, int power, bool *near)
{
	int save;
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	/* Assume monster is too powerful */
	*near = FALSE;

	/* Get monster resistance */
	save = r_ptr->level + 10;

	/* Uniques are tougher */
	if (r_ptr->flags1 & (RF1_UNIQUE)) save += 10;

	/* Are we close? */
	if ((power + 1 < save * 12 / 10) && (power + 40 > save * 8 / 10)) *near = TRUE;

	/* Modify based on current wisdom */
	if (m_ptr->mflag & (MFLAG_NAIVE)) save = save * 8 / 10;
	else if (m_ptr->mflag & (MFLAG_WISE)) save = save * 12 / 10;

	/* Check save */
	if (power + randint(40) < save)
	{
		/* Has a chance */
		if (*near)
		{
			/* Hack -- reduce wisdom */
			if (m_ptr->mflag & (MFLAG_WISE)) m_ptr->mflag &= ~(MFLAG_WISE);
			else m_ptr->mflag |= (MFLAG_NAIVE);
		}

		return (TRUE);
	}

	/* Clear near */
	*near = FALSE;

	/* Monster fails to save */
	return (FALSE);
}

/*
 * Hack -- help decide if a monster race is "okay" to charm.
 * 
 * Accept any insects, except spiders.
 */
static bool charm_insect_hook(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Require insects */
	if ((r_ptr->flags3 & (RF3_INSECT)) == 0) return (FALSE);
	
	/* Reject spiders */
	if (r_ptr->d_char == 'S') return (FALSE);

	return (TRUE);
}


/*
 * Hack -- help decide if a monster race is "okay" to charm.
 * 
 * Accept reptiles.
 */
static bool charm_reptile_hook(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Require animals */
	if ((r_ptr->flags3 & (RF3_ANIMAL)) == 0) return (FALSE);

	/* Explicitly reject undead, demons and dragons, plants and insects */
	if (r_ptr->flags3 & (RF3_UNDEAD | RF3_DEMON | RF3_DRAGON | RF3_PLANT | RF3_INSECT)) return (FALSE);
	
	/* Reject orcs, trolls, giants */
	if (r_ptr->flags3 & (RF3_ORC | RF3_TROLL | RF3_GIANT)) return (FALSE);
	
	/* Reject men, elves, dwarves */
	if (r_ptr->flags9 & (RF9_MAN | RF9_ELF | RF9_DWARF)) return (FALSE);
	
	/* Hack -- Reject hydras */
	if (r_ptr->d_char == 'y') return (FALSE);

	/* Hack -- Reject zephyr hounds */
	if (r_ptr->d_char == 'Z') return (FALSE);
	
	/* Ensure they do not have feathers or fur */
	if ((r_ptr->flags8 & (RF8_HAS_FUR | RF8_HAS_FEATHER)) != 0) return (FALSE);

	return (TRUE);
}


/*
 * Hack -- help decide if a monster race is "okay" to charm.
 * 
 * Accept reptiles.
 */
static bool charm_animal_hook(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Require animals */
	if ((r_ptr->flags3 & (RF3_ANIMAL)) == 0) return (FALSE);

	/* Explicitly reject undead, demons and dragons, plants and insects */
	if (r_ptr->flags3 & (RF3_UNDEAD | RF3_DEMON | RF3_DRAGON | RF3_PLANT | RF3_INSECT)) return (FALSE);
	
	/* Reject orcs, trolls, giants */
	if (r_ptr->flags3 & (RF3_ORC | RF3_TROLL | RF3_GIANT)) return (FALSE);
	
	/* Reject men, elves, dwarves */
	if (r_ptr->flags9 & (RF9_MAN | RF9_ELF | RF9_DWARF)) return (FALSE);
	
	/* Hack -- Reject hydras */
	if (r_ptr->d_char == 'y') return (FALSE);

	/* Hack -- Reject zephyr hounds */
	if (r_ptr->d_char == 'Z') return (FALSE);
	
	/* If they have feathers, ensure not scales or fur */
	if (((r_ptr->flags8 & (RF8_HAS_FEATHER)) != 0) && ((r_ptr->flags8 & (RF8_HAS_FUR | RF8_HAS_SCALE)) != 0)) return (FALSE);
		
	/* If they have fur, ensure not scales or feathers */
	if (((r_ptr->flags8 & (RF8_HAS_FUR)) != 0) && ((r_ptr->flags8 & (RF8_HAS_FEATHER | RF8_HAS_SCALE)) != 0)) return (FALSE);

	/* Reject if they have do not have feathers or fur */
	if ((r_ptr->flags8 & (RF8_HAS_FUR | RF8_HAS_FEATHER)) == 0) return (FALSE);

	return (TRUE);
}



/*
 * Hack -- help decide if a monster race is "okay" to charm.
 * 
 * We define 'persons' as orcs, trolls, giants, elves, dwarves,
 * humans who are not also dragons, demons or undead (currently
 * only undead applies), animals, insects or plants.
 */
static bool charm_person_hook(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Explicitly reject undead, demons and dragons, plants, insects, animals */
	if (r_ptr->flags3 & (RF3_UNDEAD | RF3_DEMON | RF3_DRAGON | RF3_INSECT | RF3_PLANT | RF3_ANIMAL)) return (FALSE);
	
	/* Reject non-orc, non-troll, non-giant, non-elf, non-man, non-dwarf */
	if (((r_ptr->flags3 & (RF3_ORC | RF3_TROLL | RF3_GIANT)) == 0) &&
		((r_ptr->flags9 & (RF9_MAN | RF9_ELF | RF9_DWARF)) == 0)) return (FALSE);
	
	return (TRUE);
}


/*
 * Hack -- help decide if a monster race is "okay" to charm.
 * 
 * We define 'monsters' as creatures that are not persons (see above)
 * who are not also dragons, demons or undead (currently only undead
 * applies). We also exclude animals that are 'straight' animals,
 * and not 'weird' hybrids.
 */
static bool charm_monster_hook(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Explicitly reject undead, demons and dragons */
	if (r_ptr->flags3 & (RF3_UNDEAD | RF3_DEMON | RF3_DRAGON)) return (FALSE);
	
	/* Reject orcs, trolls, giants, that are not plant / animal / insect hybrids */
	if (((r_ptr->flags3 & (RF3_ORC | RF3_TROLL | RF3_GIANT)) != 0) && ((r_ptr->flags3 & (RF3_PLANT | RF3_INSECT | RF3_ANIMAL)) == 0)) return (FALSE);
	
	/* Reject men, elves, dwarves, that are not plant / animal / insect hybrids */
	if (((r_ptr->flags9 & (RF9_MAN | RF9_ELF | RF9_DWARF)) == 0) && ((r_ptr->flags3 & (RF3_PLANT | RF3_INSECT | RF3_ANIMAL)) == 0)) return (FALSE);

	/* Reject 'pure' animals */
	if ((r_ptr->flags3 & (RF3_ANIMAL)) != 0)
	{
		/* If they have feathers, but not scales or fur */
		if (((r_ptr->flags8 & (RF8_HAS_FEATHER)) != 0) && ((r_ptr->flags8 & (RF8_HAS_FUR | RF8_HAS_SCALE)) == 0)) return (FALSE);
		
		/* If they have fur, but not scales or feathers */
		if (((r_ptr->flags8 & (RF8_HAS_FUR)) != 0) && ((r_ptr->flags8 & (RF8_HAS_FEATHER | RF8_HAS_SCALE)) == 0)) return (FALSE);

		/* If they have scales, but not feathers or fur */
		if (((r_ptr->flags8 & (RF8_HAS_SCALE)) != 0) && ((r_ptr->flags8 & (RF8_HAS_FUR | RF8_HAS_FEATHER)) == 0)) return (FALSE);
	}
	
	return (TRUE);
}


/*
 * Hack -- help decide if a monster race is "okay" to charm.
 * 
 * We define simple 'familiar' as not undead, demon, dragon, plant, insect,
 * orc etc., must have the animal flag and not be a hydra, and must not
 * be 'mixed' hybrid animals. Also exclude 'zephyr hounds'.
 * 
 * However, as a nod to D&D, we allow Imps, Quasits and Homouculus'.
 */
static bool charm_familiar_hook(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Hack -- accept some fixed races */
	switch(r_idx)
	{
		case 201: /* Homunculus */
		case 212: /* Quasit */
		case 213: /* Imp */
			return (TRUE);
	}

	/* Require animals */
	if ((r_ptr->flags3 & (RF3_ANIMAL)) == 0) return (FALSE);

	/* Explicitly reject undead, demons and dragons, plants and insects */
	if (r_ptr->flags3 & (RF3_UNDEAD | RF3_DEMON | RF3_DRAGON | RF3_PLANT | RF3_INSECT)) return (FALSE);
	
	/* Reject orcs, trolls, giants */
	if (r_ptr->flags3 & (RF3_ORC | RF3_TROLL | RF3_GIANT)) return (FALSE);
	
	/* Reject men, elves, dwarves */
	if (r_ptr->flags9 & (RF9_MAN | RF9_ELF | RF9_DWARF)) return (FALSE);
	
	/* Hack -- Reject hydras */
	if (r_ptr->d_char == 'y') return (FALSE);

	/* Hack -- Reject zephyr hounds */
	if (r_ptr->d_char == 'Z') return (FALSE);
	
	/* If they have feathers, ensure not scales or fur */
	if (((r_ptr->flags8 & (RF8_HAS_FEATHER)) != 0) && ((r_ptr->flags8 & (RF8_HAS_FUR | RF8_HAS_SCALE)) != 0)) return (FALSE);
		
	/* If they have fur, ensure not scales or feathers */
	if (((r_ptr->flags8 & (RF8_HAS_FUR)) != 0) && ((r_ptr->flags8 & (RF8_HAS_FEATHER | RF8_HAS_SCALE)) != 0)) return (FALSE);

	/* If they have scales, ensure not feathers or fur */
	if (((r_ptr->flags8 & (RF8_HAS_SCALE)) != 0) && ((r_ptr->flags8 & (RF8_HAS_FUR | RF8_HAS_FEATHER)) != 0)) return (FALSE);

	return (TRUE);
}

/*
 * Hack -- help decide if a monster race is "okay" to charm.
 * 
 * Anything dragonnish is fine + plus hydras.
 */
static bool charm_dragon_hook(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Require dragons */
	if (((r_ptr->flags3 & (RF3_DRAGON)) == 0) && (r_ptr->d_char != 'y')) return (FALSE);

	return (TRUE);
}


/*
 * Hack -- help decide if a monster race is "okay" to charm.
 * 
 * Anything demon.
 */
static bool charm_demon_hook(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Require demons */
	if ((r_ptr->flags3 & (RF3_DEMON)) == 0) return (FALSE);

	return (TRUE);
}


/*
 * Hack -- help decide if a monster race is "okay" to charm.
 * 
 * Anything undead.
 */
static bool charm_undead_hook(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Require undead */
	if ((r_ptr->flags3 & (RF3_UNDEAD)) == 0) return (FALSE);

	return (TRUE);
}


/*
 * Helper function for "project()" below.
 *
 * Handle a beam/bolt/ball causing damage to a monster.
 *
 * This routine takes a "source monster" (by index) which is mostly used to
 * determine if the player is causing the damage, and a "radius" (see below),
 * which is used to decrease the power of explosions with distance, and a
 * location, via integers which are modified by certain types of attacks
 * (polymorph and teleport being the obvious ones), a default damage, which
 * is modified as needed based on various properties, and finally a "damage
 * type" (see below).
 *
 * Note that this routine can handle "no damage" attacks (like teleport) by
 * taking a "zero" damage, and can even take "parameters" to attacks (like
 * confuse) by accepting a "damage", using it to calculate the effect, and
 * then setting the damage to zero.  Note that the "damage" parameter is
 * divided by the radius, so monsters not at the "epicenter" will not take
 * as much damage (or whatever)...
 *
 * Note that "polymorph" is dangerous, since a failure in "place_monster()"'
 * may result in a dereference of an invalid pointer.  XXX XXX XXX
 *
 * Various messages are produced, and damage is applied.
 *
 * Just "casting" a substance (i.e. plasma) does not make you immune, you must
 * actually be "made" of that substance, or "breathe" big balls of it.
 *
 * We assume that "Plasma" monsters, and "Plasma" breathers, are immune
 * to plasma.
 *
 * We assume "Nether" is an evil, necromantic force, so it doesn't hurt undead,
 * and hurts evil less.  If can breath nether, then it resists it as well.
 *
 * Damage reductions use the following formulas:
 *   Note that "dam = dam * 6 / (randint(6) + 6);"
 *     gives avg damage of .655, ranging from .858 to .500
 *   Note that "dam = dam * 5 / (randint(6) + 6);"
 *     gives avg damage of .544, ranging from .714 to .417
 *   Note that "dam = dam * 4 / (randint(6) + 6);"
 *     gives avg damage of .444, ranging from .556 to .333
 *   Note that "dam = dam * 3 / (randint(6) + 6);"
 *     gives avg damage of .327, ranging from .427 to .250
 *   Note that "dam = dam * 2 / (randint(6) + 6);"
 *     gives something simple.
 *
 * In this function, "result" messages are postponed until the end, where
 * the "note" string is appended to the monster name, if not NULL.  So,
 * to make a spell have "no effect" just set "note" to NULL.  You should
 * also set "notice" to FALSE, or the player will learn what the spell does.
 *
 * We attempt to return "TRUE" if the player saw anything "useful" happen.
 */
bool project_m(int who, int what, int y, int x, int dam, int typ)
{
	int tmp;

	monster_type *m_ptr;
	monster_race *r_ptr;
	monster_lore *l_ptr;

	cptr name;

	/* Is the monster "seen"? */
	bool seen = FALSE;

	/* Were the effects "obvious" (if seen)? */
	bool obvious = FALSE;

	/* Were the effects "irrelevant"? */
	bool skipped = FALSE;

	/* Were the effects "nearly successful"? */
	bool near = FALSE;

	/* Trapdoor setting (true or false) */
	int do_more = 0;

	/* Polymorph setting (true or false) */
	int do_poly = 0;

	/* Confusion setting (amount to confuse) */
	int do_conf = 0;

	/* Stunning setting (amount to stun) */
	int do_stun = 0;

	/* Cut setting (amount to bleed) */
	int do_cuts = 0;

	/* Poison setting (amount to poison) */
	int do_pois = 0;

	/* Sleep amount (amount to sleep) */
	int do_sleep = 0;

	/* Fear amount (amount to fear) */
	int do_fear = 0;

	/* Heal amount (amount to heal) */
	int do_heal = 0;

	/* Heal amount (amount to haste) */
	int do_haste = 0;

	/* Heal amount (amount to slow) */
	int do_slow = 0;

	/* Heal amount (amount to blind) */
	int do_blind = 0;

	/* Heal amount (amount to enrage) */
	int do_rage = 0;

	/* Heal amount (amount to petrify) */
	int do_petrify = 0;

	/* Destruction of inventory? */
	inven_func do_inven_destroy = NULL;

	/* Affected by charm */
	bool (*charm_hook)(int r_idx) = NULL;

	/* Hold the monster name */
	char m_name[80];

	/* Assume no note */
	cptr note = NULL;

	/* Assume a default death */
	cptr note_dies = " dies.";

	bool was_asleep;

	(void)what;
	
	/* No monster here */
	if (!(cave_m_idx[y][x] > 0)) return (FALSE);

	/* Never affect projector */
	if (cave_m_idx[y][x] == who) return (FALSE);

	/* Obtain monster info */
	m_ptr = &m_list[cave_m_idx[y][x]];

	/* Don't affect hidden monsters */
	if (m_ptr->mflag & (MFLAG_HIDE)) return (FALSE);

	/* Obtain remaining monster info */
	r_ptr = &r_info[m_ptr->r_idx];
	l_ptr = &l_list[m_ptr->r_idx];
	name = (r_name + r_ptr->name);
	if (m_ptr->ml) seen = TRUE;

	/* Check if player attacking -- note skip attacks from allies */
	if (who < SOURCE_PLAYER_START)
	{
		if (m_ptr->cdis > 1) m_ptr->mflag |= (MFLAG_HIT_RANGE);
		else m_ptr->mflag |= (MFLAG_HIT_BLOW);
	}
	/* Check if monsters are fighting out of player LOS.*/
	else if ((who > SOURCE_MONSTER_START) && !(player_has_los_bold(y, x)) && 
			(((m_list[who].mflag & (MFLAG_ALLY)) != 0) != ((m_ptr->mflag & (MFLAG_ALLY)) != 0)))
	{
		/* If so, make them aggressive. */
		m_ptr->mflag |= (MFLAG_AGGR);
	}

	/* Check if monster asleep */
	was_asleep = (m_ptr->csleep == 0);

	/* Walls protect monsters */
	if (!cave_project_bold(y, x)) dam /= 2;


	/* Get the monster name (BEFORE polymorphing) */
	monster_desc(m_name, sizeof(m_name), cave_m_idx[y][x], 0);

	/* Some monsters get "destroyed" */
	if ((r_ptr->flags3 & (RF3_NONLIVING)) ||
	    (r_ptr->flags2 & (RF2_STUPID)))
	{
		/* Special note at death */
		note_dies = " is destroyed.";
	}

	/* Hack -- storm can do several things */
	if (typ == GF_STORM)
	{
		switch(rand_int(3))
		{
			case 0: typ = GF_WIND; break;
			case 1: typ = GF_WATER; break;
			case 2: typ = GF_ELEC; break;
		}
	}

	/* Hack -- disease can do several things */
	if (typ == GF_DISEASE)
	{
		if (seen) obvious = TRUE;
		if (r_ptr->flags3 & (RF3_NONLIVING))
		{
			dam = 0;
			if ((seen) && (l_ptr->flags3 & (RF3_NONLIVING)))
			{
				note = " is unaffected.";
				l_ptr->flags3 |= (RF3_NONLIVING);
			}
		}

		else if (r_ptr->flags4 & (RF4_BRTH_DISEA))
		{
			dam /= 9;
			if ((seen) && (l_ptr->flags4 & (RF4_BRTH_DISEA)))
			{
				note = " is immune to disease.";
				l_ptr->flags4 |= (RF4_BRTH_DISEA);
			}
		}

		else switch(rand_int(16))
		{
			case 0: typ = GF_HALLU; break;
			case 1: typ = GF_BLIND; break;
			case 2: typ = GF_TERRIFY; break;
			case 3: typ = GF_CONFUSION; break;
			case 4: typ = GF_SLOW; break;
			case 5: typ = GF_LOSE_STR; break;
			case 6: typ = GF_LOSE_INT; break;
			case 7: typ = GF_LOSE_WIS; break;
			case 8: typ = GF_LOSE_DEX; break;
			case 9: typ = GF_LOSE_CON; break;
			case 10: typ = GF_HUNGER; break;
			case 11: typ = GF_FORGET; break;
			case 12: typ = GF_CURSE; break;
			case 13: typ = GF_PETRIFY; break;
			case 14: typ = GF_PARALYZE; break;
			case 15: typ = GF_LOSE_MANA; break;
		}
	}
	
	/* Hack -- pre-stage teleportation hooks for efficiency */
	switch(typ)
	{
		/* Teleport from darkness to darkness */
		case GF_AWAY_DARK:
		{
			teleport_hook = teleport_darkness_hook;
			note = "is still in the light.";
			break;
		}
		
		/* Teleport from water/living to water/living */
		case GF_AWAY_NATURE:
		{
			teleport_hook = teleport_nature_hook;
			note = "is apart from nature.";
			break;
		}
		
		/* Teleport from fire/lava to fire/lava */
		case GF_AWAY_FIRE:
		{
			teleport_hook = teleport_fiery_hook;
			note = "is free of smoke and flame.";
			break;
		}
		
		default:
		{
			teleport_hook = NULL;
		}
	}

	/* Hack -- pre-stage charm hooks for efficiency */
	switch(typ)
	{
		/* Teleport from darkness to darkness */
		case GF_CHARM_INSECT:
		{
			charm_hook = charm_insect_hook;
			break;
		}
		
		/* Scaled / skinned animals */
		case GF_CHARM_REPTILE:
		{
			charm_hook = charm_reptile_hook;
			break;
		}
		
		/* Feathered / furry animals */
		case GF_CHARM_ANIMAL:
		{
			charm_hook = charm_animal_hook;
			break;
		}
		
		/* Persons */
		case GF_CHARM_PERSON:
		{
			charm_hook = charm_person_hook;
			break;
		}

		/* Living monsters other than dragons */		
		case GF_CHARM_MONSTER:
		{
			charm_hook = charm_monster_hook;
			break;
		}
		
		/* Any animal */
		case GF_BIND_FAMILIAR:
		{
			charm_hook = charm_familiar_hook;
		}
		
		/* Any dragon */
		case GF_BIND_DRAGON:
		{
			charm_hook = charm_dragon_hook;
		}
		
		/* Any demon */
		case GF_BIND_DEMON:
		{
			charm_hook = charm_demon_hook;
		}

		/* Any undead */
		case GF_BIND_UNDEAD:
		{
			charm_hook = charm_undead_hook;
		}
	}

	/* Analyze the damage type */
	switch (typ)
	{
		/* Hack - for resist magic */
		case GF_RES_MAGIC:
		{
			if (r_ptr->flags9 & (RF9_RES_MAGIC))
			{
				dam = 0;
				if ((seen) && !(l_ptr->flags9 & (RF9_RES_MAGIC)))
				{
					note = " resists magic.";
					l_ptr->flags9 |= (RF9_RES_MAGIC);
				}
			}
		}

		/* Explosion -- destructive -- pure damage */
		case GF_EXPLODE:
		{
			if (seen) obvious = TRUE;
			break;
		}

		/* Acid */
		case GF_ACID:
		{
			/* Hack -- halve acid damage in water */
			if (f_info[cave_feat[y][x]].flags2 & (FF2_WATER)) dam /= 2;

			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_IM_ACID))
			{
				dam /= 9;
				if ((seen) && !(l_ptr->flags3 & (RF3_IM_ACID)))
				{
					note = " is immune to acid.";
					l_ptr->flags3 |= (RF3_IM_ACID);
				}
			}
			else if (m_ptr->oppose_elem)
			{
				dam /= 3;
				note = " is temporarily resistant to acid.";
			}
			else if (r_ptr->flags3 & (RF2_ARMOR))
			{
				dam /= 2;
				if ((seen) && !(l_ptr->flags3 & (RF2_ARMOR)))
				{
					note = " is protected by armour.";
					l_ptr->flags3 |= (RF2_ARMOR);
				}
			}

			/* Do acid damage to inventory */
			do_inven_destroy = set_acid_destroy;

			break;
		}

		/* Electricity */
		case GF_ELEC:
		{
			/* Hack -- double electricy damage in water */
			if (f_info[cave_feat[y][x]].flags2 & (FF2_WATER)) dam *= 2;

			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_IM_ELEC))
			{
				dam /= 9;
				if ((seen) && !(l_ptr->flags3 & (RF3_IM_ELEC)))
				{
					note = " is immune to lightning.";
					l_ptr->flags3 |= (RF3_IM_ELEC);
				}
			}
			else if (m_ptr->oppose_elem)
			{
				dam /= 3;
				note = " is temporarily resistant to lightning.";
			}

			/* Do electricity damage to inventory */
			do_inven_destroy = set_elec_destroy;

			break;
		}

		/* Fire damage */
		case GF_FIRE:
		{
			/* Hack -- halve fire damage in water */
			if (f_info[cave_feat[y][x]].flags2 & (FF2_WATER)) dam /= 2;

			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_IM_FIRE))
			{
				dam /= 9;
				if ((seen) && !(l_ptr->flags3 & (RF3_IM_FIRE)))
				{
					note = " is immune to fire.";
					l_ptr->flags3 |= (RF3_IM_FIRE);
				}
			}
			else if (m_ptr->oppose_elem)
			{
				dam /= 3;
				note = " is temporarily resistant to fire.";
			}

			/* Do fire damage to inventory */
			do_inven_destroy = set_fire_destroy;

			break;
		}

		/* Cold */
		case GF_COLD:
		{
			/* Hack -- double cold damage in water */
			if (f_info[cave_feat[y][x]].flags2 & (FF2_WATER)) dam *= 2;

			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_IM_COLD))
			{
				dam /= 9;
				if ((seen) && !(l_ptr->flags3 & (RF3_IM_COLD)))
				{
					note = " is immune to cold.";
					l_ptr->flags3 |= (RF3_IM_COLD);
				}
			}
			else if (m_ptr->oppose_elem)
			{
				dam /= 3;
				note = " is temporarily resistant to cold.";
			}

			break;
		}

		/* Poison */
		case GF_POIS:
		{
			if (seen) obvious = TRUE;
			do_pois = dam;
			if (r_ptr->flags3 & (RF3_IM_POIS))
			{
				dam /= 9;
				if ((seen) && !(l_ptr->flags3 & (RF3_IM_POIS)))
				{
					note = " is immune to poison.";
					l_ptr->flags3 |= (RF3_IM_POIS);
				}
			}
			break;
		}

		/* Holy Orb -- hurts Evil */
		case GF_HOLY_ORB:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_EVIL))
			{
				dam *= 2;
				if ((seen) && !(l_ptr->flags3 & (RF3_EVIL)))
				{
					note = " is hit hard.";
					l_ptr->flags3 |= (RF3_EVIL);
				}
			}
			else if (r_ptr->flags9 & (RF9_GOOD))
			{
				dam /= 9;
				if ((seen) && !(l_ptr->flags9 & (RF9_GOOD)))
				{
					note = " is immune to holy damage.";
					l_ptr->flags9 |= (RF9_GOOD);
				}
			}
			break;
		}

		/* Hellfire -- fire, plus nastiness to non-evil */
		case GF_HELLFIRE:
		{
			if (seen) obvious = TRUE;

			/* Demons are healed */
			if (r_ptr->flags3 & (RF3_DEMON))
			{

				if ((seen) && !(l_ptr->flags3 & (RF3_DEMON)))
				{
					note = " basks in the flames of Hell!";
					l_ptr->flags3 |= (RF3_DEMON);
				}

				/* Heal */
				do_heal = dam;

				/* No damage */
				dam = 0;
			}

			/* Can resist fire / darkness */
			else if ((r_ptr->flags3 & (RF3_IM_FIRE)) || (r_ptr->flags9 & (RF9_RES_DARK)))
			{
				if ((seen) && (r_ptr->flags3 & (RF3_IM_FIRE)) && !(l_ptr->flags3 & (RF3_IM_FIRE)))
				{
					note = " resists the evil fire.";
					l_ptr->flags3 |= (RF3_IM_FIRE);
				}

				if ((seen) && (r_ptr->flags9 & (RF9_RES_DARK)) && !(l_ptr->flags9 & (RF9_RES_DARK)))
				{
					note = " resists the dark flames.";
					l_ptr->flags9 |= (RF9_RES_DARK);
				}

				dam /= 2;
			}

			/* Evil creatures resist a little */
			else if ((r_ptr->flags3 & (RF3_EVIL)) || (m_ptr->oppose_elem))
			{
				if (m_ptr->oppose_elem)
				{
					note = " is temporarily resistant to fire.";
				}
				if ((seen) && (r_ptr->flags3 & (RF3_EVIL)) && !(l_ptr->flags3 & (RF3_EVIL)))
				{
					note = " resists somewhat.";
					l_ptr->flags3 |= (RF3_EVIL);
				}

				dam = 2 * dam / 3;
			}

			/* If the creature did not resist, it can have nasty done to it */
			else if (randint(dam) > (2 * r_ptr->level / 3 + 30))
			{
				int choice = randint(4);

				/* Effect 1 -- panic */
				if (choice <= 3)
				{
					do_fear = 40;
				}

				/* Effect 2 -- blindess */
				if (choice == 4)
				{
					/* Get blinded later */
					do_blind = 20;
				}
			}

			/* Do fire damage to inventory */
			do_inven_destroy = set_fire_destroy;

			break;
		}

		/* Plasma -- perhaps check ELEC or FIRE XXX */
		case GF_PLASMA:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_RES_PLAS))
			{
				dam *= 3; dam /= (randint(6)+6);
				if ((seen) && !(l_ptr->flags3 & (RF3_RES_PLAS)))
				{
					note = " resists plasma.";
					l_ptr->flags3 |= (RF3_RES_PLAS);
				}
			}

			/* Do fire damage to inventory */
			do_inven_destroy = set_fire_destroy;

			break;
		}

		/* Nether -- see above */
		case GF_NETHER:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				dam = 0;
				if ((seen) && !(l_ptr->flags3 & (RF3_UNDEAD)))
				{
					note = " is immune to nether.";
					l_ptr->flags3 |= (RF3_UNDEAD);
				}
			}
			else if (r_ptr->flags3 & (RF3_RES_NETHR))
			{
				dam *= 3; dam /= (randint(6)+6);
				if ((seen) && !(l_ptr->flags3 & (RF3_RES_NETHR)))
				{
					note = " resists nether.";
					l_ptr->flags3 |= (RF3_RES_NETHR);
				}
			}
			else if (r_ptr->flags3 & (RF3_EVIL))
			{
				dam /= 2;
				if ((seen) && !(l_ptr->flags3 & (RF3_EVIL)))
				{
					note = " somewhat resists nether.";
					l_ptr->flags3 |= (RF3_EVIL);
				}
			}
			break;
		}

		/* Raise/lower water damage -- everything except Water spirits/elementals are immune */
		case GF_RAISE:
		case GF_LOWER:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_HURT_WATER))
			{
				if ((seen) && !(l_ptr->flags3 & (RF3_HURT_WATER)))
				{
					note = " is disrupted.";
					l_ptr->flags3 |= (RF3_HURT_WATER);
				}
			}
			else
			{
				/* No damage */
				dam = 0;
			}
			break;
		}

		/* Water damage -- Water breathers are immune, others resist */
		case GF_WATER:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_RES_WATER))
			{
				dam /= 3;
				if ((seen) && !(l_ptr->flags3 & (RF3_RES_WATER)))
				{
					note = " resists water.";
					l_ptr->flags3 |= (RF3_RES_WATER);
				}
			}
			else if (r_ptr->flags2 & (RF2_CAN_SWIM))
			{
				dam /= 2;
				if ((seen) && !(l_ptr->flags3 & (RF2_CAN_SWIM)))
				{
					note = " swims through the water.";
					l_ptr->flags3 |= (RF2_CAN_SWIM);
				}
			}
			else
			{
				do_stun = 10 + randint(15);

				/* Monster was affected -- Mark grid for later processing. */
				cave_temp_mark(y, x, FALSE);
			}

			/* Do water damage to inventory */
			do_inven_destroy = set_water_destroy;

			break;
		}

		/* Weak water damage -- Heavily stunned/confused take damage */
		/* Hack -- we stun monsters to make them slow down and drown */
		case GF_WATER_WEAK:
		{
			if (seen) obvious = TRUE;
			if (!(r_ptr->flags2 & (RF2_CAN_SWIM)) && ((m_ptr->stunned > 100) || (m_ptr->confused)))
			{
				if (seen) note = " is drowning";
				do_stun = 10 + randint(15);

				/* Monster was affected -- Mark grid for later processing. */
				cave_temp_mark(y, x, FALSE);
			}
                        else if (!(r_ptr->flags2 & (RF2_CAN_SWIM)) && !(r_ptr->flags3 & (RF3_RES_WATER)))
			{
				dam = 0;
				do_stun = 10 + randint(15);
			}

			/* Do water damage to inventory */
			do_inven_destroy = set_water_destroy;

			break;
		}

 		/* Weak water damage -- Heavily stunned/confused take damage */
		/* Hack -- also worms that can't swim take lots of damage */
		/* Hack -- we stun monsters to make them slow down and drown */
		case GF_SALT_WATER:
		{
			if (seen) obvious = TRUE;
			if (!(r_ptr->flags2 & (RF2_CAN_SWIM)) && (r_ptr->d_char == 'w'))
			{
				dam *= 2;
				if (seen) note = " cringes away from the salt water.";
				note_dies = " shrivels away in the salt water.";
				do_stun = 10 + randint(15);

				/* Monster was affected -- Mark grid for later processing. */
				cave_temp_mark(y, x, FALSE);
			}
			else if (!(r_ptr->flags2 & (RF2_CAN_SWIM)) && ((m_ptr->stunned > 100) || (m_ptr->confused)))
			{
				if (seen) note = " is drowning";
				do_stun = 10 + randint(15);

				/* Monster was affected -- Mark grid for later processing. */
				cave_temp_mark(y, x, FALSE);
			}
                        else if (!(r_ptr->flags2 & (RF2_CAN_SWIM)) && !(r_ptr->flags3 & (RF3_RES_WATER)))
			{
				dam = 0;
				do_stun = 10 + randint(15);
			}

			/* Do water damage to inventory */
			do_inven_destroy = set_water_destroy;

			break;
		}


		/* Boiling water damage -- Water spirits/elementals are immune */
		case GF_BWATER:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flags3 & (RF3_IM_FIRE))
			{
				dam *= 3; dam /= (randint(6)+6);
				if ((seen) && !(l_ptr->flags3 & (RF3_IM_FIRE)))
				{
					note = " is immune to fire.";
					l_ptr->flags3 |= (RF3_IM_FIRE);
				}
			}
			else if (m_ptr->oppose_elem)
			{
				dam *= 3; dam /= (randint(3)+3);
				note = " is temporarily resistant to fire.";
			}

			if (r_ptr->flags3 & (RF3_RES_WATER))
			{
				dam /= 3;
				if ((seen) && !(l_ptr->flags3 & (RF3_RES_WATER)))
				{
					note = " resists water.";
					l_ptr->flags3 |= (RF3_RES_WATER);
				}
			}
			else if (!(r_ptr->flags2 & (RF2_CAN_SWIM)) && ((m_ptr->stunned > 100) || (m_ptr->confused)))
			{
				if (seen) note = " is drowning.";
				do_stun = 10 + randint(15);

				/* Monster was affected -- Mark grid for later processing. */
				cave_temp_mark(y, x, FALSE);
			}

			/* Do water damage to inventory */
			do_inven_destroy = set_water_destroy;

			break;
		}


		/* Lava damage -- Magma spirits/elementals are resistant */
		case GF_LAVA:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flags3 & (RF3_IM_FIRE))
			{
				dam *= 3; dam /= (randint(6)+6);
				if ((seen) && !(l_ptr->flags3 & (RF3_IM_FIRE)))
				{
					note = " somewhat resists lava.";
					l_ptr->flags3 |= (RF3_IM_FIRE);
				}
			}
			else if (m_ptr->oppose_elem)
			{
				dam *= 3; dam /= (randint(3)+3);
				note = " is temporarily resistant to fire.";
			}

			if (r_ptr->flags3 & (RF3_RES_LAVA))
			{
				dam /= 3;
				if ((seen) && !(l_ptr->flags3 & (RF3_RES_LAVA)))
				{
					note = " resists lava.";
					l_ptr->flags3 |= (RF3_RES_LAVA);
				}
			}
			else
			{
				do_stun = 10 + randint(15);

				/* Monster was affected -- Mark grid for later processing. */
				cave_temp_mark(y, x, FALSE);
			}

			/* Do fire damage to inventory */
			do_inven_destroy = set_fire_destroy;

			break;
		}

		/* Boiling mud damage -- Ooze spirits/elementals are immune */
		case GF_BMUD:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_IM_FIRE))
			{
				dam *= 3; dam /= (randint(6)+6);
				if ((seen) && !(l_ptr->flags3 & (RF3_IM_FIRE)))
				{
					note = " somewhat resists boiling mud.";
					l_ptr->flags3 |= (RF3_IM_FIRE);
				}
			}
			else if (m_ptr->oppose_elem)
			{
				dam *= 3; dam /= (randint(3)+3);
				note = " is temporarily resistant to fire.";
			}

			if (r_ptr->flags2 & (RF2_CAN_DIG))
			{
				dam /= 3;
				if ((seen) && !(l_ptr->flags2 & (RF2_CAN_DIG)))
				{
					note = " digs through the boiling mud.";
					l_ptr->flags2 |= (RF2_CAN_DIG);
				}
			}
			else if (!(r_ptr->flags2 & (RF2_CAN_DIG)) && ((m_ptr->stunned > 100) || (m_ptr->confused)))
			{
				if (seen) note = " is drowning.";
				do_stun = 10 + randint(15);

				/* Monster was affected -- Mark grid for later processing. */
				cave_temp_mark(y, x, FALSE);
			}

			/* Do water damage to inventory */
			do_inven_destroy = set_water_destroy;

			break;
		}

		/* Suffocation damage for any monster in terrain they can't handle */
		/* eg fish out of water */
		/* Heavily stunned */
		case GF_SUFFOCATE:
		{
			if (seen) obvious = TRUE;
                        if (!(r_ptr->flags3 & (RF3_NONLIVING)) && ((m_ptr->stunned > 100) || (m_ptr->confused)))
			{
				if (seen) note = " is drowning.";
				do_stun = 10 + randint(15);

				/* Monster was affected -- Mark grid for later processing. */
				cave_temp_mark(y, x, FALSE);
			}
                        else if (!(r_ptr->flags3 & (RF3_NONLIVING)))
			{
				dam = 0;
				do_stun = 10 + randint(15);
			}
			break;
		}

		/* Chaos */
		case GF_CHAOS:
		{
			if (seen) obvious = TRUE;
			do_poly = TRUE;
			do_conf = 5 + randint(11);
			if (r_ptr->flags9 & (RF9_RES_CHAOS))
			{
				dam *= 3; dam /= (randint(6)+6);
				do_poly = FALSE;
				if ((seen) && !(l_ptr->flags9 & (RF9_RES_CHAOS)))
				{
					note = " resists chaos.";
					l_ptr->flags9 |= (RF9_RES_CHAOS);
				}
			}
			break;
		}

		/* Hallucination -- Resisted by resist chaos */
		case GF_HALLU:
		{
			if (seen) obvious = TRUE;
			do_conf = 5 + randint(11);
			if (r_ptr->flags9 & (RF9_RES_CHAOS))
			{
				dam *= 3; dam /= (randint(6)+6);
				if ((seen) && !(l_ptr->flags9 & (RF9_RES_CHAOS)))
				{
					note = " resists hallucinations.";
					l_ptr->flags9 |= (RF9_RES_CHAOS);
				}
			}
			break;
		}

		/* Shards -- Shard breathers resist, immune edge resists */
		case GF_SHARD:
		{
			if (seen) obvious = TRUE;
			do_cuts = 10 + randint(15);
			if (r_ptr->flags4 & (RF4_BRTH_SHARD))
			{
				dam *= 3; dam /= (randint(6)+6);
				if ((seen) && !(l_ptr->flags4 & (RF4_BRTH_SHARD)))
				{
					note = " resists shards.";
					l_ptr->flags4 |= (RF4_BRTH_SHARD);
				}
			}
			else if (r_ptr->flags9 & (RF9_IM_EDGED))
			{
				dam *= 3; dam /= (randint(6)+6);
				if ((seen) && !(l_ptr->flags9 & (RF9_IM_EDGED))) 
				{
					note = " resists shards.";
					l_ptr->flags9 |= (RF9_IM_EDGED);
				}
			}
			break;
		}

		/* Sound -- Sound breathers resist, immune blunt resists */
		case GF_SOUND:
		{
			if (seen) obvious = TRUE;
			do_stun = 10 + randint(15);
			if (r_ptr->flags4 & (RF4_BRTH_SOUND))
			{
				dam *= 2; dam /= (randint(6)+6);
				if ((seen) && !(l_ptr->flags4 & (RF4_BRTH_SOUND)))
				{
					note = " resists sound.";
					l_ptr->flags4 |= (RF4_BRTH_SOUND);
				}
			}
			else if (r_ptr->flags9 & (RF9_IM_BLUNT))
			{
				dam *= 3; dam /= (randint(6)+6);
				if ((seen) && !(l_ptr->flags9 & (RF9_IM_BLUNT)))
				{
					note = " resists sound.";
					l_ptr->flags9 |= (RF9_IM_BLUNT);
				}
			}
			break;
		}

		/* Confusion */
		case GF_CONFUSION:
		{
			if (seen) obvious = TRUE;
			do_conf = 10 + randint(15);
			if (r_ptr->flags4 & (RF4_BRTH_CONFU))
			{
				dam *= 2; dam /= (randint(6)+6);
				if ((seen) && !(l_ptr->flags4 & (RF4_BRTH_CONFU)))
				{
					note = " resists confusion.";
					l_ptr->flags4 |= (RF4_BRTH_CONFU);
				}
			}
			break;
		}

		/* Disenchantment */
		case GF_DISENCHANT:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_RES_DISEN))
			{
				dam *= 3; dam /= (randint(6)+6);
				if ((seen) && !(l_ptr->flags3 & (RF3_RES_DISEN)))
				{
					note = " resists disenchantment.";
					l_ptr->flags3 |= (RF3_RES_DISEN);
				}
			}
			break;
		}

		/* Nexus */
		case GF_NEXUS:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_RES_NEXUS))
			{
				dam *= 3; dam /= (randint(6)+6);
				if ((seen) && !(l_ptr->flags3 & (RF3_RES_NEXUS)))
				{
					note = " resists nexus.";
					l_ptr->flags3 |= (RF3_RES_NEXUS);
				}
			}
			else
			{
				/* Monster was affected -- Mark grid for later processing. */
				cave_temp_mark(y, x, FALSE);
			}
			break;
		}

		/* Force */
		case GF_FORCE:
		{
			if (seen) obvious = TRUE;
			do_stun = randint(15);
			if (r_ptr->flags4 & (RF4_BRTH_FORCE))
			{
				dam *= 3; dam /= (randint(6)+6);
				if ((seen) && !(l_ptr->flags4 & (RF4_BRTH_FORCE)))
				{
					note = " resists force.";
					l_ptr->flags4 |= (RF4_BRTH_FORCE);
				}
			}
			else if (r_ptr->flags9 & (RF9_IM_BLUNT))
			{
				dam *= 3; dam /= (randint(6)+6);
				if ((seen) && !(l_ptr->flags9 & (RF9_IM_BLUNT)))
				{
					note = " is unaffected.";
					l_ptr->flags9 |= (RF9_IM_BLUNT);
				}
			}
			else
			{
				/* Monster was affected -- Mark grid for later processing. */
				cave_temp_mark(y, x, FALSE);
			}

			break;
		}

		/* Inertia -- breathers resist */
		case GF_INERTIA:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags4 & (RF4_BRTH_INERT))
			{
				dam *= 3; dam /= (randint(6)+6);
				if ((seen) && !(l_ptr->flags4 & (RF4_BRTH_INERT)))
				{
					note = " resists inertia.";
					l_ptr->flags4 |= (RF4_BRTH_INERT);
				}
			}
			break;
		}

		/* Time -- breathers resist */
		case GF_TIME:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags4 & (RF4_BRTH_TIME))
			{
				dam *= 3; dam /= (randint(6)+6);
				if ((seen) && !(l_ptr->flags4 & (RF4_BRTH_TIME)))
				{
					note = " resists time.";
					l_ptr->flags4 |= (RF4_BRTH_TIME);
				}
			}
			break;
		}

		/* Gravity -- breathers resist */
		case GF_GRAVITY:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flags4 & (RF4_BRTH_GRAV))
			{
				dam *= 3; dam /= (randint(6)+6);
				if ((seen) && !(l_ptr->flags4 & (RF4_BRTH_GRAV)))
				{
					note = " resists gravity.";
					l_ptr->flags4 |= (RF4_BRTH_GRAV);
				}
			}
			else
			{
				/* Monster was affected -- Mark grid for later processing. */
				cave_temp_mark(y, x, FALSE);
			}
			break;
		}

		/* Pure damage -- mana breathers are immune */
		case GF_MANA:
		{
			if (seen) obvious = TRUE;

			/* Mana breathers are immune */
			if (r_ptr->flags4 & (RF4_BRTH_MANA))
			{
				dam /= 9;
				if ((seen) && !(r_ptr->flags4 & (RF4_BRTH_MANA)))
				{
					note = " is immune to mana.";
					l_ptr->flags4 |= (RF4_BRTH_MANA);
				}
			}

			break;
		}

		/* Meteor -- powerful magic missile */
		case GF_METEOR:
		{
			if (seen) obvious = TRUE;
			break;
		}

		/* Ice -- Cold + Cuts + Stun */
		case GF_ICE:
		{
			if (seen) obvious = TRUE;
			do_stun = randint(15);
			do_cuts = randint(15);
			if (r_ptr->flags3 & (RF3_IM_COLD))
			{
				dam /= 3;
				if ((seen) && !(l_ptr->flags3 & (RF3_IM_COLD)))
				{
					note = " resists ice.";
					l_ptr->flags3 |= (RF3_IM_COLD);
				}
			}
			else if (m_ptr->oppose_elem)
			{
				dam /= 2;
				note = " is temporarily resistant to fire.";
			}

			/* Do cold damage to inventory */
			do_inven_destroy = set_cold_destroy;

			break;
		}

		/* Wind -- Stuns + throws monster around */
		case GF_WIND:
		{
			int do_dist;

			if (seen) obvious = TRUE;
			do_stun = randint(15);

			/* Damage-variable throw distance */
			do_dist = 1 + dam / 25;
			if (do_dist > 8) do_dist = 8;

			/* Big, heavy monsters, metallic monsters and ghosts */
			if ((r_ptr->flags3 & (RF3_HUGE)) || (r_ptr->flags9 & (RF9_IM_BLUNT | RF9_IM_EDGED))) do_dist /= 3;
			else if ((r_ptr->flags3 & (RF3_GIANT)) || (r_ptr->flags9 & (RF9_RES_BLUNT | RF9_RES_EDGED))) do_dist /= 2;
			else if (m_ptr->mflag & (MFLAG_OVER)) do_dist = 8;

			/* Scale down damage based on distance -- unless monster flying or climbing */
			dam = dam * do_dist / 8;

			/* Monster was affected -- Mark grid for later processing. */
			cave_temp_mark(y, x, FALSE);

			break;
		}

		/* Drain Life and Vampiric Drain */
		case GF_DRAIN_LIFE:
		case GF_VAMP_DRAIN:
		{
			if (seen) obvious = TRUE;

			/* Hack -- heal undead */
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				do_heal = dam;
				dam = 0;
			}

			else if (r_ptr->flags3 & (RF3_DEMON))
			{
				if ((seen) && !(l_ptr->flags3 & (RF3_DEMON)))
				{
					note = " is unaffected.";
					l_ptr->flags3 |= (RF3_DEMON);
				}

				obvious = FALSE;
				dam = 0;
			}

			else if (r_ptr->flags3 & (RF3_NONLIVING))
			{
				if ((seen) && !(l_ptr->flags3 & (RF3_NONLIVING)))
				{
					note = " is unaffected.";
					l_ptr->flags3 |= (RF3_NONLIVING);
				}

				obvious = FALSE;
				dam = 0;
			}
			else
			{
				note = " is drained of health.";
			}
			
			if (typ == GF_DRAIN_LIFE) break;
			
			/* Do not allow wimpy monsters to yield much profit */
			if (m_ptr->hp + 1 < dam) dam = m_ptr->hp + 1;

			/* Character has cast the spell */
			if (who <= SOURCE_PLAYER_START)
			{
				/* Spell is damaging, and has hit a warm-blooded creature. */
				if (dam > 0)
				{
					/* msg_print("You suck in some life force."); */

					/* Heal caster */
					hp_player(dam);

					/* Feed caster -- protect against bloating */
					if (p_ptr->food + dam * 8 < PY_FOOD_FULL)
						set_food(p_ptr->food + dam * 8);
					else
						set_food(PY_FOOD_FULL);
				}
				/* Oops */
				else if (do_heal)
				{
					take_hit(do_heal, "trying to draw health from an undead opponent");
				}
			}
			break;
		}

		/* Polymorph monster (Use "dam" as "power") */
		case GF_POLY:
		{
			if (seen) obvious = TRUE;

			/* Attempt to polymorph (see below) */
			do_poly = TRUE;

			/* Only polymorph living or once living monsters */
			if (r_ptr->flags3 & (RF3_NONLIVING))
			{
				if ((seen) && !(l_ptr->flags3 & (RF3_NONLIVING)))
				{
					note = " is unaffected.";
					l_ptr->flags3 |= (RF3_NONLIVING);
				}

				obvious = FALSE;
				do_poly = FALSE;
			}

			/* Powerful monsters can resist */
			if (monster_save(m_ptr, dam, &near))
			{
				if ((near) && (seen))
				{
					note = " seems ready to change.";
				}
				else
				{
					if (seen) note = " is unaffected!";

					do_poly = FALSE;
					obvious = FALSE;
				}
			}

			/* No "real" damage */
			dam = 0;

			break;
		}


		/* Clone monsters (Ignore "dam") */
		case GF_CLONE:
		{
			if (seen) obvious = TRUE;

			/* Heal fully */
			do_heal = (m_ptr->maxhp - m_ptr->hp);

			/* Speed up */
			do_haste = 25 + rand_int(25);

			/* Attempt to clone. */
			if (multiply_monster(cave_m_idx[y][x]))
			{
				if (seen) note = " spawns!";
			}

			/* No "real" damage */
			dam = 0;

			break;
		}

		
		/* Heal Monster (use "dam" as amount of healing, except on undead) */
		case GF_HEAL:
		case GF_HEAL_PERC:
		{
			if (seen) obvious = TRUE;

			/* Hack -- hurt undead */
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				if ((seen) && !(l_ptr->flags3 & (RF3_UNDEAD)))
				{
					note = " cringes from the touch of life.";
					l_ptr->flags3 |= (RF3_UNDEAD);
				}
				note_dies = " shrivels away from the touch of life.";
			}
			else
			{
				do_heal = dam;

				/* No "real" damage */
				dam = 0;

				/* Hack -- cure blindness */
				m_ptr->blind = 0;				
			}

			break;
		}


		/* Speed Monster (Ignore "dam") */
		case GF_HASTE:
		{
			if (seen) obvious = TRUE;

			/* Hack -- damage golems */
			if (r_ptr->d_char == 'g')
			{
				if (seen)
				{
					note = " shudders as gears spin uncontrollably.";
				}
				note_dies = " flies apart in a jumble of gears and cogs.";
			}
			else
			{
				/* Speed up */
				do_haste = 50 + rand_int(50);

				/* No "real" damage */
				dam = 0;
			}
			break;
		}

		/* Slow Monster (Use "dam" as "power") */
		case GF_SLOW_WEAK:
		{
			if (seen) obvious = TRUE;

			/* Cannot be slowed */
			if (r_ptr->flags9 & (RF9_NO_SLOW))
			{
				if ((seen) && !(l_ptr->flags9 & (RF9_NO_SLOW)))
				{
					note = " cannot be slowed or paralyzed.";
					l_ptr->flags9 |= (RF9_NO_SLOW);
				}
			}

			/* Powerful monsters can resist */
			else if (monster_save(m_ptr, dam, &near))
			{
				if ((near) && (seen))
				{
					note = " appears sluggish.";
					if (m_ptr->energy > 25) m_ptr->energy -= 25;
					else (m_ptr->energy = 0);
				}
				else
				{
					if (seen) note = " is unaffected!";
					obvious = FALSE;
				}
			}

			/* Normal monsters slow down */
			else
			{
				do_slow = 50 + rand_int(50);
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Sleep (Use "dam" as "power") */
		case GF_SLEEP:
		{
			if (seen) obvious = TRUE;

			/* Cannot be charmed */
			if (r_ptr->flags3 & (RF3_NO_SLEEP))
			{
				if ((seen) && !(l_ptr->flags3 & (RF3_NO_SLEEP)))
				{
					note = " cannot be charmed or slept.";
					l_ptr->flags3 |= (RF3_NO_SLEEP);
				}
			}

			/* Attempt a saving throw */
			else if (monster_save(m_ptr, dam, &near))
			{
				if ((near) && (seen))
				{
					note = " appears drowsy.";
					do_sleep = 2;
				}
				else
				{
					if (seen) note = " is unaffected!";
					obvious = FALSE;
				}
			}
			else
			{
				/* Go to sleep (much) later */
				note = " falls asleep!";
				do_sleep = 500;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Confusion (Use "dam" as "power") */
		case GF_CONF_WEAK:
		{
			if (seen) obvious = TRUE;

			/* Memorize a flag */
			if (r_ptr->flags3 & (RF3_NO_CONF))
			{
				if ((seen) && !(l_ptr->flags3 & (RF3_NO_CONF)))
				{
					note = " cannot be confused.";
					l_ptr->flags3 |= (RF3_NO_CONF);
				}
			}

			/* Attempt a saving throw */
			else if (monster_save(m_ptr, dam, &near))
			{
				if ((near) && (seen))
				{
					note = " appears dizzy.";
					do_conf = 2;
				}
				else
				{
					if (seen) note = " is unaffected!";
					obvious = FALSE;
				}
			}
			else
			{
				/* Get confused later */
				do_conf = damroll(3, (dam / 2)) + 1;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}



		/* Lite, but only hurts susceptible creatures */
		case GF_LITE_WEAK:
		{
			if (seen) obvious = TRUE;

			/* Resist light never blinded -- also don't blind if already afffected */
			if (!(r_ptr->flags9 & (RF9_RES_LITE)) && !m_ptr->blind) do_blind = randint(5);

			/* Hurt by light */
			if (r_ptr->flags3 & (RF3_HURT_LITE))
			{
				if (seen) obvious = TRUE;

				/* Memorize the effects */
				if ((seen) && !(l_ptr->flags3 & (RF3_HURT_LITE)))
				{
					note = " cringes from the light!";
					l_ptr->flags3 |= (RF3_HURT_LITE);
				}

				/* Special effect */
				note_dies = " shrivels away in the light!";
			}

			/* Normally no damage */
			else
			{
				/* No damage */
				dam = 0;
			}

			break;
		}



		/* Lite -- opposite of Dark */
		case GF_LITE:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags9 & (RF9_RES_LITE))
			{
				dam *= 2; dam /= (randint(6)+6);

				/* Memorize the effects */
				if ((seen) && !(l_ptr->flags3 & (RF9_RES_LITE)))
				{
					if (!(l_ptr->flags9 & (RF9_RES_LITE))) note = " resists.";
					l_ptr->flags9 |= (RF9_RES_LITE);
				}

			}
			else
			{
				 if (!m_ptr->blind) do_blind = randint(5);
			}

			if (r_ptr->flags3 & (RF3_HURT_LITE)) 
			{
				/* Memorize the effects */
				if ((seen) && !(l_ptr->flags3 & (RF3_HURT_LITE)))
				{
					note = " cringes from the light!";
					l_ptr->flags3 |= (RF3_HURT_LITE);
				}

				note_dies = " shrivels away in the light!";
				dam *= 2;
			}
			break;
		}


		/* Dark weak, but only blinds non-resistant creatures */
		case GF_DARK_WEAK:
		{
			if (seen) obvious = TRUE;

			/* Resist light never blinded -- also don't blind if already afffected */
			if (!(r_ptr->flags9 & (RF9_RES_DARK)) && !m_ptr->blind) do_blind = randint(5);

			dam = 0;
			break;
		}

		/* Dark -- opposite of Lite */
		case GF_DARK:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags9 & (RF9_RES_DARK))
			{
				dam *= 2; dam /= (randint(6)+6);

				/* Memorize the effects */
				if ((seen) && !(l_ptr->flags9 & (RF9_RES_DARK)))
				{
					note = " resists.";
					l_ptr->flags9 |= (RF9_RES_DARK);
				}

			}
			else
			{
				 if (!m_ptr->blind) do_blind = randint(5);
			}
			break;
		}


		/* Stone to Mud */
		case GF_KILL_WALL:
		{
			/* Hurt by rock remover */
			if (r_ptr->flags3 & (RF3_HURT_ROCK))
			{
				/* Notice effect */
				if (seen) obvious = TRUE;

				/* Memorize the effects */
				if ((seen) && !(l_ptr->flags3 & (RF3_HURT_ROCK)))
				{
					note = " loses some skin!";
					l_ptr->flags3 |= (RF3_HURT_ROCK);
				}

				/* Cute little message */
				note_dies = " dissolves!";
			}

			/* Usually, ignore the effects */
			else
			{
				/* No damage */
				dam = 0;
			}

			break;
		}


		/* Teleport undead (Use "dam" as "power") */
		case GF_AWAY_UNDEAD:
		{
			/* Only affect undead */
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				if (seen) obvious = TRUE;
				if (seen) l_ptr->flags3 |= (RF3_UNDEAD);

				/* Monster was affected -- Mark grid for later processing. */
				cave_temp_mark(y, x, FALSE);
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Teleport evil (Use "dam" as "power") */
		case GF_AWAY_EVIL:
		{
			/* Only affect undead */
			if (r_ptr->flags3 & (RF3_EVIL))
			{
				if (seen) obvious = TRUE;
				if (seen) l_ptr->flags3 |= (RF3_EVIL);

				/* Monster was affected -- Mark grid for later processing. */
				cave_temp_mark(y, x, FALSE);
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Teleport monster (Use "dam" as "power").
		 * Hook should be prepared earlier */
		case GF_AWAY_JUMP:
		case GF_AWAY_ALL:
		case GF_AWAY_DARK:
		case GF_AWAY_NATURE:
		case GF_AWAY_FIRE:
		{
			/* No "real" damage */
			dam = 0;
				
			/* Valid teleport grid? */
			if (!teleport_hook || teleport_hook(y, x, y, x))
			{
				/* Obvious */
				if (seen) obvious = TRUE;

				/* Monster was affected -- Mark grid for later processing. */
				cave_temp_mark(y, x, FALSE);
				
				/* Clear 'failure' note */
				note = NULL;
			}
			
			break;
		}

		/* Turn undead (Use "dam" as "power") */
		case GF_TURN_UNDEAD:
		{
			/* Only affect undead */
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				/* Learn about type */
				if (seen) l_ptr->flags3 |= (RF3_UNDEAD);

				/* Obvious */
				if (seen) obvious = TRUE;

				/* Apply some fear */
				do_fear = damroll(3, (dam / 2)) + 1;

				/* Attempt a saving throw */
				if (monster_save(m_ptr, dam, &near))
				{
					if ((near) && (seen))
					{
						note = " appears to recoil.";
						do_fear = 2;
					}
					else
					{
						if (seen) note = " is unaffected!";

						obvious = FALSE;
						do_fear = 0;
					}
				}
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Turn evil (Use "dam" as "power") */
		case GF_TURN_EVIL:
		{
			/* Only affect evil */
			if (r_ptr->flags3 & (RF3_EVIL))
			{
				/* Learn about type */
				if (seen) l_ptr->flags3 |= (RF3_EVIL);

				/* Obvious */
				if (seen) obvious = TRUE;

				/* Apply some fear */
				do_fear = damroll(3, (dam / 2)) + 1;

				/* Attempt a saving throw */
				if (monster_save(m_ptr, dam, &near))
				{
					if ((near) && (seen))
					{
						note = " appears to recoil.";
						do_fear = 2;
					}
					else
					{
						if (seen) note = " is unaffected!";

						obvious = FALSE;
						do_fear = 0;
					}
				}
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Turn monster (Use "dam" as "power") */
		case GF_FEAR_WEAK:
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Apply some fear */
			do_fear = damroll(3, (dam / 2)) + 1;

			/* Attempt a saving throw */
			if (r_ptr->flags3 & (RF3_NO_FEAR))
			{
				if ((seen) && !(l_ptr->flags3 & (RF3_NO_FEAR)))
				{
					l_ptr->flags3 |= (RF3_NO_FEAR);
					note = " is immune to fear.";

					do_fear = 0;
				}
			}
			else if (monster_save(m_ptr, dam, &near))
			{
				if ((near) && (seen))
				{
					note = " appears to recoil.";
					do_fear = 2;
				}
				else
				{
					if (seen) note = " is unaffected!";
					do_fear = 0;

					obvious = FALSE;
				}
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Dispel undead */
		case GF_DISP_UNDEAD:
		{
			/* Only affect undead */
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				/* Learn about type */
				if ((seen) && !(l_ptr->flags3 & (RF3_UNDEAD)))
				{
					note = " shudders.";
					l_ptr->flags3 |= (RF3_UNDEAD);

					obvious = TRUE;
				}

				/* Obvious */
				if (seen) obvious = TRUE;

				/* Message */
				note_dies = " dissolves!";
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;

				/* No damage */
				dam = 0;
			}

			break;
		}


		/* Dispel evil */
		case GF_DISP_EVIL:
		{
			/* Only affect evil */
			if (r_ptr->flags3 & (RF3_EVIL))
			{
				/* Learn about type */
				if ((seen) && !(l_ptr->flags3 & (RF3_UNDEAD)))
				{
					note = " shudders.";
					l_ptr->flags3 |= (RF3_UNDEAD);

					obvious = TRUE;
				}

				/* Message */
				note_dies = " dissolves!";
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;

				/* No damage */
				dam = 0;
			}

			break;
		}


		/* Dispel monster */
		case GF_DISP_ALL:
		{
			/* Obvious */
			if (seen) obvious = TRUE;

 			/* Message */
			note_dies = " dissolves!";

			break;
		}


		/* Melee attack - blind */
		case GF_BLIND:
		{
			if (seen) obvious = TRUE;

			/* Hurt eyes alot */
			if (r_ptr->d_char == 'e')
			{
				dam *= 2;
				note = " is badly hurt.";
			}

			/* Attempt a saving throw */
			if (r_ptr->flags9 & (RF9_RES_BLIND))
			{
				if ((seen) && !(l_ptr->flags9 & (RF9_RES_BLIND)))
				{
					l_ptr->flags9 |= (RF9_RES_BLIND);
					note = " resists blindness.";
				}
			}
			else if (monster_save(m_ptr, dam, &near))
			{
				if ((near) && (seen))
				{
					note = " appears cross-eyed.";
					do_blind = 2;
				}
				else
				{
					if (seen) note = " is unaffected!";

					obvious = FALSE;
				}
			}
			else
			{
				/* Get blinded later */
				do_blind = damroll(3, (dam / 2)) + 1;
			}
		}

		/* Melee attack - fear */
		case GF_TERRIFY:
		{
			if (seen) obvious = TRUE;
			do_fear = 10 + randint(15);
			if (r_ptr->flags3 & (RF3_NO_FEAR))
			{
				dam = 0;
				do_fear = 0;
				if (seen)
				{
					if (!(l_ptr->flags3 & (RF3_NO_FEAR))) note = " is immune to fear.";
					l_ptr->flags3 |= (RF3_NO_FEAR);
				}
			}
			else if (r_ptr->flags4 & (RF4_BRTH_FEAR))
			{
				dam *= 2; dam /= (randint(6)+6);
				do_fear = 0;
				if (seen)
				{
					if (seen)
					{
						if (!(l_ptr->flags4 & (RF4_BRTH_FEAR))) note = " resists fear.";
						l_ptr->flags4 |= (RF4_BRTH_FEAR);
					}
					l_ptr->flags4 |= (RF4_BRTH_FEAR);
				}
			}
			break;
		}

		/* Melee attack - paralyze */
		case GF_STASTIS:
		case GF_PARALYZE:
		{
			if (seen) obvious = TRUE;

			/* Memorize a flag */
			if (r_ptr->flags9 & (RF9_NO_SLOW))
			{
				if (seen)
				{
					if (!(l_ptr->flags9 & (RF9_NO_SLOW))) note = " cannot be paralyzed or slowed.";
					l_ptr->flags9 |= (RF9_NO_SLOW);
				}
			}
			/* Attempt a saving throw */
			else if (monster_save(m_ptr, dam, &near))
			{
				if ((near) && (seen))
				{
					note = " appears sluggish.";
					if (m_ptr->energy > 25) m_ptr->energy -= 25;
					else (m_ptr->energy = 0);
				}
				else
				{
					if (seen) note = " is unaffected!";

					do_sleep = FALSE;
					obvious = FALSE;
				}
			}
			else
			{
				/* Go to sleep (much) later */
				note = " is paralyzed!";
				do_sleep = 500;
				do_petrify = 10;
			}
			break;
		}


		/* Melee attack - slow */
		case GF_SLOW:
		{
			if (seen) obvious = TRUE;

			/* Memorize a flag */
			if (r_ptr->flags9 & (RF9_NO_SLOW))
			{
				if ((seen) && !(l_ptr->flags9 & (RF9_NO_SLOW)))
				{
					note = " cannot be paralyzed or slowed.";
					l_ptr->flags9 |= (RF9_NO_SLOW);
				}
			}
			/* Attempt a saving throw */
			else if (monster_save(m_ptr, dam, &near))
			{
				if ((near) && (seen))
				{
					note = " appears sluggish.";
					if (m_ptr->energy > 25) m_ptr->energy -= 25;
					else (m_ptr->energy = 0);
				}
				else
				{
					if (seen) note = " is unaffected!";

					obvious = FALSE;
				}
			}
			else
			{
				do_slow = 50 + rand_int(50);
				note = " starts moving slower.";
			}

			break;
		}

		/* Pits */
		case GF_FALL:
		case GF_FALL_SPIKE:
		case GF_FALL_POIS:
		{
			if (seen) obvious = TRUE;

			note = " falls into a pit.";
			break;
		}

		/* Trapdoor */
		case GF_FALL_MORE:
		{
			if (seen) obvious = TRUE;

			/* Hack -- no chasm/trap doors/down stairs on quest levels */
			if (is_quest(p_ptr->depth) ||  (p_ptr->depth == max_depth(p_ptr->dungeon)))
			{
				note = "falls into a chasm.";

				/* Hack -- prevent 'weird' messages */
				if (dam == 0) obvious = FALSE;

				/* Should probably make the monster fall back
				 * into the chasm.
				 */
			}
			else
			{
				note = " falls from sight.";

				do_more = TRUE;
			}

			break;
		}

		/* Melee attack - hurt */
		case GF_BATTER:
		case GF_WOUND:
		case GF_HURT:

		/* Melee attack - unbonus */
		case GF_UN_BONUS:

		/* Melee attack - unpower */
		case GF_UN_POWER:

		/* Melee attack - eat gold */
		case GF_EAT_GOLD:

		/* Melee attack - eat item */
		case GF_EAT_ITEM:

		/* Melee attack - eat food */
		case GF_EAT_FOOD:

		/* Melee attack - eat lite */
		case GF_EAT_LITE:

			break;

		/* Melee attack - lose strength */
		case GF_HUNGER:
		case GF_LOSE_STR:
		{
			if (m_ptr->mflag & (MFLAG_STRONG))
			{
				if (seen) obvious = TRUE;

				m_ptr->mflag &= ~(MFLAG_STRONG);
			}
			else if (!(m_ptr->mflag & (MFLAG_WEAK)))
			{
				if (seen) obvious = TRUE;

				m_ptr->mflag |= (MFLAG_WEAK);
			}

			if (obvious) note = " appears weaker.";
			break;
		}

		/* Melee attack - lose int */
		case GF_LOSE_INT:
		{
			if (m_ptr->mflag & (MFLAG_SMART))
			{
				if (seen) obvious = TRUE;

				m_ptr->mflag &= ~(MFLAG_SMART);
			}
			else if (!(m_ptr->mflag & (MFLAG_STUPID)))
			{
				if (seen) obvious = TRUE;

				m_ptr->mflag |= (MFLAG_STUPID);
			}

			if (obvious) note = " appears stupider.";
			break;
		}

		/* Melee attack - lose wisdom */
		case GF_LOSE_WIS:
		{
			if (m_ptr->mflag & (MFLAG_WISE))
			{
				if (seen) obvious = TRUE;

				m_ptr->mflag &= ~(MFLAG_WISE);
			}
			else if (!(m_ptr->mflag & (MFLAG_NAIVE)))
			{
				if (seen) obvious = TRUE;

				m_ptr->mflag |= (MFLAG_NAIVE);
			}

			if (obvious) note = " appears more naive.";
			break;
		}

		/* Melee attack - lose dex */
		case GF_LOSE_DEX:
		{
			if (m_ptr->mflag & (MFLAG_SKILLFUL))
			{
				if (seen) obvious = TRUE;

				m_ptr->mflag &= ~(MFLAG_SKILLFUL);
			}
			else if (!(m_ptr->mflag & (MFLAG_CLUMSY)))
			{
				if (seen) obvious = TRUE;

				m_ptr->mflag |= (MFLAG_CLUMSY);
			}

			if (obvious) note = " appears clumsier.";
			break;
		}

		/* Melee attack - lose con */
		case GF_LOSE_CON:
		{
			int new_maxhp;

			if (m_ptr->mflag & (MFLAG_HEALTHY))
			{
				if (seen) obvious = TRUE;

				m_ptr->mflag &= ~(MFLAG_HEALTHY);
			}
			else if (!(m_ptr->mflag & (MFLAG_SICK)))
			{
				if (seen) obvious = TRUE;

				m_ptr->mflag |= (MFLAG_SICK);
			}

			if (obvious) note = " appears sicklier.";

			new_maxhp = calc_monster_hp(cave_m_idx[y][x]);

			if (new_maxhp < m_ptr->maxhp)
			{
				note = " appears sicklier.";

				/* Scale down hit points */
				m_ptr->hp = m_ptr->hp * new_maxhp / m_ptr->maxhp;

				/* To a minimum */
				if (m_ptr->hp < 0) m_ptr->hp = 0;

				/* Permanently reduce maximum hp */
				m_ptr->maxhp = new_maxhp;
			}
			break;
		}

		/* Melee attack - lose cha */
		case GF_LOSE_CHR:
		{
			if (seen) obvious = TRUE;
			note = " can't get any uglier.";
			break;
		}

		/* Melee attack - lose all */
		case GF_LOSE_ALL:

		/* Melee attack - exp 10 */
		case GF_EXP_10:

		/* Melee attack - exp 20 */
		case GF_EXP_20:

		/* Melee attack - exp 40 */
		case GF_EXP_40:

		/* Melee attack - exp 80 */
		case GF_EXP_80:
		{
			/* Hack -- just completely ruin monster */
			if (seen) obvious = TRUE;
			note = " is ruined.";
			m_ptr->mflag &= ~(MFLAG_STRONG | MFLAG_SMART | MFLAG_WISE | MFLAG_SKILLFUL | MFLAG_HEALTHY);
			m_ptr->mflag |= (MFLAG_WEAK | MFLAG_STUPID | MFLAG_NAIVE | MFLAG_CLUMSY | MFLAG_SICK);
			break;
		}

		/* Melee attack - shatter */
		case GF_SHATTER:
		{
			earthquake(y, x, 8);			

			break;
		}

		/* Melee attack - lose mana */
		case GF_MANA_DRAIN:
		case GF_LOSE_MANA:
		{
			/* Monster may have mana */
			if (r_ptr->mana)
			{
				/* Drain depends on maximum mana */
				int drain = 2 + rand_int(r_ptr->mana / 10);

				dam = 0;

				/* Monster still has mana */
				if (m_ptr->mana > drain)
				{
					note = " loses mana.";
					m_ptr->mana -= drain;
				}
				/* Monster has no more mana */
				else
				{
					note = " is drained of mana.";
					m_ptr->mana = 0;
				}

				if (seen) obvious = TRUE;
				
				if (typ == GF_LOSE_MANA) break;
				
				/* Convert to player mana */
				drain = damroll(drain,5);
				
				/* Monster notices */
				update_smart_forget(cave_m_idx[y][x], SM_IMM_MANA);

				/* Player gains back all mana */
				if (p_ptr->csp + drain >= p_ptr->msp)
				{
					p_ptr->csp = p_ptr->msp;
					p_ptr->csp_frac = 0;

					msg_print("You feel your head clear.");
				}
				/* Player gains back partial mana */
				else
				{
					p_ptr->csp += drain;

					/*msg_print("You feel your concentration improve.");*/
				}

				/* Update mana */
				p_ptr->update |= (PU_MANA);

				p_ptr->redraw |= (PR_MANA);
				p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1 | PW_PLAYER_2 | PW_PLAYER_3);
				obvious = TRUE;				
			}
			break;
		}

		/* Melee attack - gain mana */
		case GF_GAIN_MANA:
		case GF_GAIN_MANA_PERC:
		{
			/* Monster may have mana */
			if (r_ptr->mana)
			{
				/* Monster gains back all mana */
				if (m_ptr->mana + dam / 5 >= r_ptr->mana)
				{
					note = " restores its mana.";
					m_ptr->mana = r_ptr->mana;
				}
				/* Monster gains back partial mana */
				else
				{
					note = " recovers mana.";
					m_ptr->mana += dam / 5;
				}

				if (seen) obvious = TRUE;
			}

			dam = 0;
			break;
		}

		/* Probe visible monsters */
		case GF_PROBE:
		{
			dam = 0;

			if ((who <= SOURCE_PLAYER_START) && (cave_m_idx[y][x] > 0))
			{
				/* Describe the monster */
				msg_format("%^s has %d hit points.", m_name, m_ptr->hp);

				/* Learn all of the non-spell, non-treasure flags */
				lore_do_probe(cave_m_idx[y][x]);

				/* Probe worked */
				obvious = TRUE;
			}

			break;
		}

		/* Co-exist with a feature */
		case GF_FEATURE:
		{
			if (seen) obvious = TRUE;

			/* Monster was affected -- Mark grid for later processing. */
			cave_temp_mark(y, x, FALSE);

			break;
		}

		/* Steam -- weak boiling water*/
		case GF_STEAM:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_IM_FIRE))
			{
				note = " resists.";
				dam *= 3; dam /= (randint(6)+6);
				if ((seen) && !(l_ptr->flags3 & (RF3_IM_FIRE)))
				{
					note = " is immune to fire.";
					l_ptr->flags3 |= (RF3_IM_FIRE);
				}
			}
			else if (m_ptr->oppose_elem)
			{
				dam *= 3; dam /= (randint(3)+3);
				note = " is temporarily resistant to fire.";
			}

			if (r_ptr->flags3 & (RF3_RES_WATER))
			{
				dam = 0;
				if ((seen) && !(l_ptr->flags3 & (RF3_RES_WATER)))
				{
					note = " resists water.";
					l_ptr->flags3 |= (RF3_RES_WATER);
				}
			}
			break;
		}

		/* Vapour -- weak acid */
		case GF_VAPOUR:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_IM_ACID))
			{
				dam /= 9;
				if ((seen) && !(l_ptr->flags3 & (RF3_IM_ACID)))
				{
					note = " is immune to acid.";
					l_ptr->flags3 |= (RF3_IM_ACID);
				}
			}
			else if (m_ptr->oppose_elem)
			{
				dam /= 3;
				note = " is temporarily resistant to acid.";
			}

			break;
		}

		/* Smoke -- weak fire */
		case GF_SMOKE:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_IM_FIRE))
			{
				dam /= 9;
				if ((seen) && !(l_ptr->flags3 & (RF3_IM_FIRE)))
				{
					note = " is immune to fire.";
					l_ptr->flags3 |= (RF3_IM_FIRE);
				}
			}
			else if (m_ptr->oppose_elem)
			{
				dam /= 3;
				note = " is temporarily resistant to fire.";
			}

			break;
		}

		/* Blind Monster (Use "dam" as "power") */
		case GF_BLIND_WEAK:
		{
			/* Cannot be slowed */
			if (r_ptr->flags9 & (RF9_RES_BLIND))
			{
				if (seen) obvious = TRUE;

				if ((seen) && !(l_ptr->flags9 & (RF9_RES_BLIND)))
				{
					note = " cannot be blinded.";
					l_ptr->flags9 |= (RF9_RES_BLIND);
				}
			}

			/* Powerful monsters can resist */
			else if (monster_save(m_ptr, dam, &near))
			{
				if ((near) && (seen))
				{
					note = " appears cross-eyed.";
					do_blind = 2;
				}
				else
				{
					if (seen) note = " is unaffected!";

					obvious = FALSE;
				}
			}

			/* Normal monsters are blinded */
			else
			{
				/* Blind */
				do_blind = 50 + rand_int(50);
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Enrage Monster (Use "dam" as "power") */
		case GF_RAGE:
		{
			/* Cannot be slowed */
			if (r_ptr->flags3 & (RF3_NONLIVING))
			{
				if ((seen) && !(l_ptr->flags3 & (RF3_NONLIVING)))
				{
					note = " is unaffected.";
					l_ptr->flags3 |= (RF3_NONLIVING);
				}

				obvious = FALSE;
			}

			/* Powerful monsters can resist */
			else if (monster_save(m_ptr, dam, &near))
			{
				if ((near) && (seen))
				{
					note = " loses its temper.";
					do_rage = 2;
				}
				else
				{
					if (seen) note = " is unaffected!";

					obvious = FALSE;
				}
			}

			/* Normal monsters are blinded */
			else
			{
				/* Blind */
				do_rage = 50 + rand_int(50);
			}

			/* No "real" damage */
			dam = 0;
			break;
		}
		
		/* Monster forgets things */
		case GF_FORGET:
		{
			/* Powerful monsters can resist */
			if (monster_save(m_ptr, dam, &near))
			{
				if ((near) && (seen))
				{
					note = " appears absent-minded.";
					m_ptr->ty = 0;
					m_ptr->tx = 0;
				}
				else
				{
					if (seen) note = " is unaffected!";

					obvious = FALSE;
				}
			}

			/* Monster forgets stuff */
			else
			{
				m_ptr->ty = 0;
				m_ptr->tx = 0;
				m_ptr->mflag &= ~(MFLAG_HIT_BLOW | MFLAG_HIT_RANGE | MFLAG_AGGR);
				m_ptr->smart = 0L;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Monster stripped of enchantments */
		{
		case GF_DISPEL:
			/* No "real" damage */
			dam = 0;
		}
		/* Hack -- curse effect - dispel plus damage */
		case GF_CURSE:
		{
			/* Non-living are immune */
			if ((r_ptr->flags3 & (RF3_NONLIVING)) && (typ == GF_CURSE))
			{
				dam = 0;
				if ((seen) && (l_ptr->flags3 & (RF3_NONLIVING)))
				{
					note = " is unaffected.";
					l_ptr->flags3 |= (RF3_NONLIVING);
				}
			}
			/* All others */
			else
			{
				/* Monster abilities removed the next round*/
				if (m_ptr->slowed) m_ptr->slowed = 1;
				if (m_ptr->hasted) m_ptr->hasted = 1;
				if (m_ptr->tim_invis) m_ptr->tim_invis = 1;
				if (m_ptr->tim_passw) m_ptr->tim_passw = 1;
				if (m_ptr->bless) m_ptr->bless = 1;
				if (m_ptr->berserk) m_ptr->berserk = 1;
				if (m_ptr->shield) m_ptr->shield = 1;
				if (m_ptr->oppose_elem) m_ptr->oppose_elem = 1;
			}

			break;
		}

		/* Damage monsters with minds only */
		case GF_MENTAL:
		{
			/* Non-living are immune */
			if (r_ptr->flags2 & (RF2_EMPTY_MIND | RF2_WEIRD_MIND))
			{
				dam = 0;
				if ((seen) && (l_ptr->flags2 & (RF2_WEIRD_MIND | RF2_EMPTY_MIND)))
				{
					note = " is unaffected.";
					if (r_ptr->flags2 & (RF2_EMPTY_MIND)) l_ptr->flags2 |= (RF2_EMPTY_MIND);
					if (r_ptr->flags2 & (RF2_WEIRD_MIND)) l_ptr->flags2 |= (RF2_WEIRD_MIND);					
				}
			}

			break;
		}

		/* Snuff out life if damage exceeds maximum hit points */
		case GF_SNUFF:
		{
			/* Non-living are immune */
			if (r_ptr->flags3 & (RF3_NONLIVING))
			{
				dam = 0;
				if ((seen) && (l_ptr->flags3 & (RF3_NONLIVING)))
				{
					note = " is unaffected.";
					l_ptr->flags3 |= (RF3_NONLIVING);
				}
			}

			/* Monsters with too high hit points are immune */
			else if (m_ptr->maxhp > dam)
			{
				dam = 0;
				note = " is unaffected.";
			}
			break;
		}

		/* Warp wood (Ignore "dam") */
		case GF_HURT_WOOD:
		{
			if (seen) obvious = TRUE;

			/* Hack -- damage trees */
			if (r_ptr->d_char == ':')
			{
				if (seen)
				{
					note = " shudders as it warps.";
				}
				note_dies = " flies apart in shower of splinters.";
			}
			else
			{
				/* No "real" damage */
				dam = 0;
			}
			
			/* Do acid damage to inventory */
			do_inven_destroy = set_warpwood_destroy;
			
			break;
		}

		/* Petrify monster (Use "dam" as "power", and do damage) */
		case GF_PETRIFY:
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Apply some fear */
			do_petrify = damroll(3, (dam / 2)) + 1;

			/* Attempt a saving throw - immune to edged are immune (but not resist) */
			if (r_ptr->flags9 & (RF9_IM_EDGED))
			{
				if ((seen) && !(l_ptr->flags9 & (RF9_IM_EDGED)))
				{
					l_ptr->flags9 |= (RF9_IM_EDGED);
					note = " is immune to petrification.";

					do_petrify = 0;
				}
			}
			else if (monster_save(m_ptr, dam, &near))
			{
				if ((near) && (seen))
				{
					note = " grinds to a halt for a moment.";
					do_petrify = 2;
				}
				else
				{
					if (seen) note = " is unaffected!";
					do_petrify = 0;

					obvious = FALSE;
				}
			}
			break;
		}

		
		/* Entangle monsters in nearby plants (Use "dam" as "power") */
		case GF_TANGLE:
		{
			/* Must be next to plants/water */
			if (!teleport_nature_hook(y, x, y, x)) break;
			
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Apply some petrification */
			do_petrify = damroll(3, (dam / 2)) + 1;

			/* Big, heavy monsters, metallic monsters and ghosts */
			if (r_ptr->flags9 & (RF9_IM_BLUNT)) do_petrify = 0;
			else if ((r_ptr->flags3 & (RF3_HUGE)) || (r_ptr->flags9 & (RF9_IM_BLUNT))) do_petrify /= 5;
			else if ((r_ptr->flags3 & (RF3_GIANT)) || (r_ptr->flags9 & (RF9_RES_BLUNT | RF9_RES_EDGED))) do_petrify /= 3;

			if (do_petrify <= 1)
			{
				note = " breaks free of the plants.";
			}
			else
			{
				note = " is entangled.";
			}
			
			/* No real damage */
			dam = 0;
			
			break;
		}
		
		
		
		/* Charm effects */
		case GF_CHARM_INSECT:
		case GF_CHARM_ANIMAL:
		case GF_CHARM_REPTILE:
		case GF_CHARM_MONSTER:
		case GF_CHARM_PERSON:

		/* Bind effects for the moment */
		case GF_BIND_DRAGON:
		case GF_BIND_FAMILIAR:
		case GF_BIND_UNDEAD:
		case GF_BIND_DEMON:
		{
			if (charm_hook(m_ptr->r_idx))
			{
				if (seen) obvious = TRUE;

				/* Cannot be charmed */
				if (r_ptr->flags3 & (RF3_NO_SLEEP))
				{
					if ((seen) && !(l_ptr->flags3 & (RF3_NO_SLEEP)))
					{
						note = " cannot be charmed or slept.";
						l_ptr->flags3 |= (RF3_NO_SLEEP);
					}
				}

				/* Attempt a saving throw */
				else if (monster_save(m_ptr, dam, &near))
				{
					if ((near) && (seen))
					{
						note = " appears somewhat persuaded.";
						m_ptr->mflag |= (MFLAG_IGNORE);
					}
					else
					{
						if (seen) note = " is unaffected!";
						obvious = FALSE;
					}
				}
				/* Hack -- can't charm multi-headed monsters */
				else if (strstr(r_name + r_ptr->name, "headed"))
				{
					if (seen) note = "'s heads fight amongst themselves.";
					do_conf = dam;
				}
				else
				{
					/* Charm immediately */
					note = " falls under your influence!";
					m_ptr->mflag |= (MFLAG_ALLY);
					m_ptr->mflag &= ~(MFLAG_IGNORE);
					
					/* Clear 'summoned' counter */
					m_ptr->summoned = 0;
				}
			}
			else
			{
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;			
		}

		/* Default */
		default:
		{
			/* Irrelevant */
			skipped = TRUE;

			break;
		}
	}


	/* Absolutely no effect */
	if (skipped)
	{
		/* Skip further if applied to a target known to be unaffected */
		if (who <= SOURCE_PLAYER_NO_TARGET)
			 return (FALSE);

		if (seen) note = " is unaffected!";

		obvious = FALSE;

		/* No damage */
		dam = 0;
	}

	/* "Unique" monsters cannot be polymorphed */
	if (r_ptr->flags1 & (RF1_UNIQUE)) do_poly = FALSE;

	/* "Unique" monsters can only be "killed" by the player */
	if (r_ptr->flags1 & (RF1_UNIQUE))
	{
		/* Uniques may only be killed by the player */
		if ((who > SOURCE_PLAYER_START) && (dam > m_ptr->hp)) dam = m_ptr->hp;
	}

	/* Check healing first */
	if (do_heal)
	{
		/* Obvious */
		if (seen) obvious = TRUE;

		/* Wake up */
		m_ptr->csleep = 0;

		/* Heal */
		m_ptr->hp += dam;

		/* No overflow */
		if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

		/* Redraw (later) if needed */
		if (p_ptr->health_who == cave_m_idx[y][x]) p_ptr->redraw |= (PR_HEALTH);

		/* Message */
		if (r_ptr->flags3 & (RF3_NONLIVING)) note = " repairs.";
		else note = " looks healthier.";

		/* No "real" damage */
		dam = 0;
	}

	/* Check for death */
	else if (dam > m_ptr->hp)
	{
		/* Extract method of death */
		note = note_dies;
	}

	/* Mega-Hack -- Handle "trapdoor"  */
	else if (do_more)
	{
		/* Turn off the damage */
		dam = 0;
	
		/* "Kill" the monster */
		delete_monster_idx(cave_m_idx[y][x]);
	
		/* Paranoia --- Handle rest of monster routine here */

		/* Give detailed messages if destroyed */
		if (note) msg_format("%^s%s", m_name, note);

		/* Redraw the monster grid */
		lite_spot(y, x);

		/* Hack --- no monster left */
		return(obvious);
	}
	
	/* Mega-Hack -- Handle "polymorph" -- monsters get a saving throw */
	else if (do_poly && (randint(90) > r_ptr->level))
	{
		/* Default -- assume no polymorph */
		note = " is unaffected!";

		/* Pick a "new" monster race */
		tmp = poly_r_idx(m_ptr->r_idx, y, x);

		/* Handle polymorh */
		if (tmp != m_ptr->r_idx)
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Monster polymorphs */
			note = " changes!";

			/* Turn off the damage */
			dam = 0;

			/* "Kill" the "old" monster */
			delete_monster_idx(cave_m_idx[y][x]);

			/* Create a new monster (no groups) */
			(void)place_monster_aux(y, x, tmp, FALSE, FALSE, 0L);

			/* Hack -- Assume success XXX XXX XXX */

			/* Hack -- Get new monster */
			m_ptr = &m_list[cave_m_idx[y][x]];

			/* Hack -- Get new race */
			r_ptr = &r_info[m_ptr->r_idx];
		}
	}
	
	/* Still alive -- apply damage secondary effects */
	else
	{
		/*Slowing*/
		if (do_slow > 1)
		{
			/* Increase slowness */
			tmp = m_ptr->slowed + do_slow;

			/* set or add to slow counter */
			set_monster_slow(cave_m_idx[m_ptr->fy][m_ptr->fx],
						tmp, seen);
		}

		/* Hasting */
		if (do_haste > 1)
		{
			/* Increase haste */
			tmp = m_ptr->hasted + do_haste;

			/* set or add to slow counter */
			set_monster_haste(cave_m_idx[m_ptr->fy][m_ptr->fx],
						tmp, seen);
		}

		/* Handle "stun" */
		if ((do_stun > 1) &&
			 !(r_ptr->flags3 & (RF3_NO_STUN)))
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Get confused */
			if (m_ptr->stunned)
			{
				if (!note) note =  " is more dazed.";
				tmp = m_ptr->stunned + (do_stun / (r_ptr->level / 10 + 1));
			}
			else
			{
				if (!note) note =  " is dazed.";
				tmp = do_stun;
			}

			/* Apply stun */
			m_ptr->stunned = MIN(tmp, 200);
		}

		/* Handle "blindness" */
		if ((do_blind > 1) &&
			 !(r_ptr->flags9 & (RF9_RES_BLIND)))
		{
			/* Don't blind already blinded monsters -- but allow cross-eyed to be blinded */
			if (m_ptr->blind <= 1)
			{
				if (seen) obvious = TRUE;
				if (do_blind > 1) if (!note) note =  " is blinded.";

				/* Apply blindness */
				m_ptr->blind = MIN(do_blind, 200);
			}
		}

		/* Handle "rage" */
		if (do_rage > 1)
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Get confused */
			if (m_ptr->stunned)
			{
				if (!note) note =  " is more enraged.";
				tmp = m_ptr->berserk + (do_rage / (r_ptr->level / 10 + 1));
			}
			else
			{
				if (do_rage > 1) if (!note) note =  " is enraged.";
				tmp = do_rage;
			}

			/* Apply stun */
			m_ptr->berserk = MIN(tmp, 200);
			
			/* Target the player */
			if (who <= SOURCE_PLAYER_START)
			{
				m_ptr->ty = p_ptr->py;
				m_ptr->tx = p_ptr->px;
				m_ptr->mflag &= ~(MFLAG_ALLY);
			}
			/* Target a monster */
			else if (who > SOURCE_MONSTER_START)
			{
				m_ptr->ty = m_list[who].fy;
				m_ptr->tx = m_list[who].fy;
				m_ptr->mflag |= (MFLAG_IGNORE);				
			}

		}
		
		/* Handle cuts from player or allies only */
		if ((do_cuts > 1) && ((who <= SOURCE_PLAYER_START) || ((who > 0) && (m_list[who].mflag & (MFLAG_ALLY)))) && 
			 !(r_ptr->flags9 & (RF9_NO_CUTS)))
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Bleed */
			if (m_ptr->cut)
			{
				if (!note) note =  " bleeds more.";
			}
			else
			{
				if (!note) note =  " is bleeding.";
			}

			tmp = do_cuts / (r_ptr->level / 10 + 1);

			/* Apply cuts if player only */
			if (who > SOURCE_MONSTER_START) m_ptr->cut = MIN(255, m_ptr->cut + tmp);
		}

		/* Handle poison from player only */
		if ((do_pois > 1) && ((who <= SOURCE_PLAYER_START) || ((who > 0) && (m_list[who].mflag & (MFLAG_ALLY)))) &&
			 !(r_ptr->flags3 & (RF3_IM_POIS)))
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Get confused */
			if (m_ptr->poisoned)
			{
				if (!note) if (!note) note =  " is more poisoned.";
			}

			else
			{
				if (!note) if (!note) note =  " is poisoned.";
			}

			tmp = do_pois / (r_ptr->level / 10 + 1);

			/* Only apply poison caused by player*/
			m_ptr->poisoned = MIN(255, m_ptr->poisoned + tmp);
		}

		/* Handle confusion */
		if ((do_conf > 1) &&
			 !(r_ptr->flags3 & (RF3_NO_CONF)))
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Already partially confused */
			if (m_ptr->confused)
			{
				if (!note) note = " looks more confused.";
				tmp = m_ptr->confused + (do_conf / (r_ptr->level / 10 + 1));
			}

			/* Was not confused */
			else
			{
				if (!note) note = " looks confused.";
				tmp = do_conf;
			}

			/* Apply confusion */
			m_ptr->confused = (tmp < 200) ? tmp : 200;
		}
		
		/* Fear */
		if (do_fear > 1)
		{
			/* Increase fear */
			tmp = m_ptr->monfear + do_fear;

			/* Hack -- excess fear causes petrification */
			if (tmp > 100)
			{
				do_petrify = tmp - 100;
				tmp = 100;
			}
			
			/* Set monster fear */
			set_monster_fear(m_ptr, tmp, TRUE);

			/* Quest monster */
			if (r_ptr->flags1 & (RF1_QUESTOR)) check_fear_quest(cave_m_idx[y][x]);
		}

		/* Handle petrify. Im_edged resists. */
		if ((do_petrify > 1) &&
			 !(r_ptr->flags9 & (RF9_IM_EDGED)))
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Already partially petrify */
			if (m_ptr->petrify)
			{
				if (!note) note = " looks more petrified.";
				tmp = m_ptr->petrify + (do_conf / (r_ptr->level / 10 + 1));
			}

			/* Was not petrify */
			else
			{
				if (do_conf > 1) if (!note) note = " looks petrified.";
				tmp = do_conf;
			}

			/* Apply confusion */
			m_ptr->petrify = (tmp < 200) ? tmp : 200;
			
			/* As we can't move, need to find new range */
			find_range(cave_m_idx[y][x]);
		}	
	}

	/* If another non-allied monster or trap did the damage, hurt the monster by hand */
	if (who > SOURCE_PLAYER_START)
	{
		/* Redraw (later) if needed */
		if (p_ptr->health_who == cave_m_idx[y][x]) p_ptr->redraw |= (PR_HEALTH);

		/* Wake the monster up */
		m_ptr->csleep = 0;

		/* Hurt the monster */
		m_ptr->hp -= dam;

		/* Dead monster */
		if (m_ptr->hp < 0)
		{
			/* Generate treasure, etc */
			monster_death(cave_m_idx[y][x]);

			/* Delete the monster */
			delete_monster_idx(cave_m_idx[y][x]);

			/* Give detailed messages if destroyed */
			if (note) msg_format("%^s%s", m_name, note);
		}

		/* Damaged monster */
		else
		{
			/* Give detailed messages if visible or destroyed */
			if (note && seen) msg_format("%^s%s", m_name, note);

			/* Hack -- Pain message */
			else if (dam > 0) message_pain(cave_m_idx[y][x], dam);

			/* Hack -- handle sleep */
			if (do_sleep) m_ptr->csleep = do_sleep;

			/* Hack -- handle inventory damage */
			if (do_inven_destroy) mon_inven_damage(cave_m_idx[y][x], do_inven_destroy, typ == GF_HURT_WOOD ? 30 : (dam / 15) + 1);
		}
	}

	/* If the player did it, give him experience, check fear */
	else
	{
		bool fear = FALSE;

		/* Hurt the monster, check for fear and death */
		if (mon_take_hit(cave_m_idx[y][x], dam, &fear, note_dies))
		{
			/* Dead monster */
		}

		/* Damaged monster */
		else
		{
			/* Give detailed messages if visible or destroyed */
			if (note && seen) msg_format("%^s%s", m_name, note);

			/* Hack -- Pain message */
			else if (dam > 0) message_pain(cave_m_idx[y][x], dam);

			/* Take note */
			if ((fear || do_fear) && (m_ptr->ml))
			{
				/* Message */
				message_format(MSG_FLEE, m_ptr->r_idx,
					       "%^s flees in terror!", m_name);
			}

			/* Hack -- handle sleep */
			if (do_sleep) m_ptr->csleep = do_sleep;

			/* Hack -- handle inventory damage */
			if (do_inven_destroy) mon_inven_damage(cave_m_idx[y][x], do_inven_destroy, (dam / 15) + 1);
		}

		/* Hack -- wake up nearby allies */
		if (was_asleep)
		{
			m_ptr->mflag |= (MFLAG_AGGR);

			tell_allies_mflag(m_ptr->fy, m_ptr->fx, MFLAG_AGGR, "& has attacked me!");
		}
		else if (fear)
		{
			tell_allies_mflag(m_ptr->fy, m_ptr->fx, MFLAG_ACTV, "& has hurt me badly!");
		}
	}


	/* Verify this code XXX XXX XXX */

	/* Update the monster */
	update_mon(cave_m_idx[y][x], FALSE);

	/* Redraw the monster grid */
	lite_spot(y, x);


	/* Update monster recall window */
	if (p_ptr->monster_race_idx == m_ptr->r_idx)
	{
		/* Window stuff */
		p_ptr->window |= (PW_MONSTER);
	}


	/* Track it */
	project_m_n++;
	project_m_x = x;
	project_m_y = y;


	/* Return "Anything seen?" */
	return (obvious);
}


/*
 * Helper function for "project()" below.
 *
 * Handle a beam/bolt/ball causing damage to the player.
 *
 * This routine takes a "source monster" (by index), a "distance", a default
 * "damage", and a "damage type".  See "project_m()" above.
 *
 * If "rad" is non-zero, then the blast was centered elsewhere, and the damage
 * is reduced (see "project_m()" above).  This can happen if a monster breathes
 * at the player and hits a wall instead.
 *
 * We return "TRUE" if any "obvious" effects were observed.
 *
 * Actually, for historical reasons, we just assume that the effects were
 * obvious.  XXX XXX XXX
 */
bool project_p(int who, int what, int y, int x, int dam, int typ)
{
	int k = 0;
	int i,j;

	s32b gold;


	/* Hack -- assume obvious */
	bool obvious = TRUE;

	/* Player blind-ness */
	bool blind = (p_ptr->blind ? TRUE : FALSE);

	/* Player needs a "description" (he is blind) */
	bool fuzzy = FALSE;

	/* Source monster */
	monster_type *m_ptr=NULL;

	/* Monster name (for damage) */
	char killer[80];

	/* Target object*/
	object_type *o_ptr;

	/* Object name (for drain) */
	char o_name[80];

	/* Get diseases */
	u32b old_disease = p_ptr->disease;

	/* No player here */
	if (!(cave_m_idx[y][x] < 0)) return (FALSE);

	/* Never affect player in stastis */
	if (p_ptr->stastis) return (FALSE);

	/* Limit maximum damage XXX XXX XXX */
	if (dam > 1600) dam = 1600;

	/* If the player is blind, be more descriptive */
	if (blind) fuzzy = TRUE;

	/* Get the killer */
	if (who > SOURCE_MONSTER_START)
	{
		/* Get the source monster */
		m_ptr = &m_list[who];

		/* Get the monster's real name */
		monster_desc(killer, sizeof(killer), who, 0x88);
		
		/*
		 * XXX Add what?
		 * This will be 1-4 for blows 1-4
		 * and the spell cast for 96 and over.
		 */
	}
	else
	{
		/* Add object what caused the fatal wound */
		if (who <= SOURCE_PLAYER_START)
		{
			my_strcat(killer, cause_of_death[-who][rand_int(4)], sizeof(killer));

			/* Hack -- make it clear it was a trap */
			if (who == SOURCE_PLAYER_TRAP) my_strcat(killer, " trap you set", sizeof(killer));		
		}

		/* Cause of death routine */
		switch(who)
		{
			case SOURCE_PLAYER_ALLY:
			case SOURCE_SELF:
			{
				/* Get the source monster */
				m_ptr = &m_list[what];

				/* Get the monster's real name */
				monster_desc(killer, sizeof(killer), who, 0x88);
				
				break;
			}

			case SOURCE_FEATURE:
			{
				/* Get the source feature */
				feature_type *f_ptr = &f_info[what];
				
				/* Get the feature name */
				my_strcpy(killer, format("%s %s", is_a_vowel((f_name + f_ptr->name)[0]) ? "an" : "a", f_name + f_ptr->name), sizeof(killer));
				
				break;
			}
			
			case SOURCE_SPELL:
			case SOURCE_PLAYER_CAST:
			{
				/* Get the source spell */
				spell_type *s_ptr = &s_info[what];
				
				/* Get the spell name */
				my_strcpy(killer, format("%s %s", is_a_vowel((s_name + s_ptr->name)[0]) ? "an" : "a", s_name + s_ptr->name), sizeof(killer));
				
				break;
			}
			
			case SOURCE_DISEASE:
			{
				/* Get the feature name */
				my_strcpy(killer, disease_name[what], sizeof(killer));
				
				break;
			}
			
			case SOURCE_DAYLIGHT:
			case SOURCE_PLAYER_ATTACK:
			case SOURCE_PLAYER_SHOT:
			case SOURCE_PLAYER_THROW:
			{
				/* Nothing */
				
				break;	
			}

			case SOURCE_BIRTH:
			case SOURCE_PLAYER_EAT_MONSTER:
			case SOURCE_PLAYER_SPORE:
			{
				/* Get the source feature */
				monster_race *r_ptr = &r_info[what];
				
				/* Get the feature name */
				my_strcpy(killer, format("%s %s", is_a_vowel((r_name + r_ptr->name)[0]) ? "an" : "a", r_name + r_ptr->name), sizeof(killer));
				
				break;
			}			

			default:
			{
				/* Fake object */
				object_type object_type_body;

				/* Get an object */
				object_type *o_ptr = &object_type_body;

				/* Prepare the object */
				object_prep(o_ptr, what);

				/* Get the object name */
				object_desc(killer, sizeof(killer), o_ptr, TRUE, 0); 
				
				break;
			}
		}
		
		/* Add a message of destruction */
		if (who > SOURCE_PLAYER_START)
		{
			my_strcpy(killer, cause_of_death[-who][rand_int(4)], sizeof(killer));
		}
	}

	/* Hack -- storm can do several things */
	if (typ == GF_STORM)
	{
		switch(rand_int(3))
		{
			case 0: typ = GF_WIND; break;
			case 1: typ = GF_WATER; break;
			case 2: typ = GF_ELEC; break;
		}
	}

	/* Hack -- pre-stage teleportation hooks for efficiency */
	switch(typ)
	{
		/* Teleport from darkness to darkness */
		case GF_AWAY_DARK:
		{
			teleport_hook = teleport_darkness_hook;
			break;
		}
		
		/* Teleport from water/living to water/living */
		case GF_AWAY_NATURE:
		{
			teleport_hook = teleport_nature_hook;
			break;
		}
		
		/* Teleport from fire/lava to fire/lava */
		case GF_AWAY_FIRE:
		{
			teleport_hook = teleport_fiery_hook;
			break;
		}
		
		default:
		{
			teleport_hook = NULL;
		}
	}

	/* Analyze the damage */
	switch (typ)
	{
		/* Standard damage -- hurts inventory too */
		case GF_ACID:
		{
			/* Hack -- halve acid damage in water */
			if (f_info[cave_feat[y][x]].flags2 & (FF2_WATER)) dam /= 2;

			msg_print ("You are covered in acid!");
			acid_dam(who, dam, killer, TRUE);
			break;
		}

		/* Standard damage -- hurts inventory too */
		case GF_FIRE:
		{
			/* Hack -- halve fire damage in water */
			if (f_info[cave_feat[y][x]].flags2 & (FF2_WATER)) dam /= 2;

			msg_print ("You are enveloped in flames!");
			fire_dam(who, dam, killer, TRUE);
			break;
		}

		/* Standard damage -- hurts inventory too */
		case GF_COLD:
		{
			/* Hack -- double cold damage in water */
			if (f_info[cave_feat[y][x]].flags2 & (FF2_WATER)) dam *= 2;

			msg_print ("You are covered in frost!");
			cold_dam(who, dam, killer, TRUE);
			break;
		}

		/* Standard damage -- hurts inventory too */
		case GF_ELEC:
		{

			/* Hack -- double electricy damage in water */
			if (f_info[cave_feat[y][x]].flags2 & (FF2_WATER)) dam *= 2;

			msg_print("You are struck by electricity!");
			elec_dam(who, dam, killer, TRUE);
			break;
		}

		/* Standard damage -- also poisons player */
		case GF_POIS:
		{
			if (fuzzy) msg_print("You are hit by poison!");
			poison_dam(who, dam, killer, TRUE);
			break;
		}

		/* Standard damage -- also disease player */
		case GF_DISEASE:
		{
			if (fuzzy) msg_print("You are hit by disease!");

			/* Disease resistance */
			if ((p_ptr->cur_flags4 & (TR4_RES_DISEASE)) != 0)
			{
				/* Always notice */
				player_can_flags(who, 0x0L,0x0L,0x0L,TR4_RES_DISEASE);

				/* Reduce damage */
				dam *= 6; dam /= (randint(6) + 6);				
			}
			else
			{
				int rating = dam / (adj_con_fix[p_ptr->stat_ind[A_CON]] + 1);

				/* Always notice */
				player_not_flags(who, 0x0L,0x0L,0x0L,TR4_RES_DISEASE);

				/* Critical disease - multiple effects either quickly, powerfully or heavily */
				if (rating > 45)
				{
					while (rating > 0)
					{
						if (p_ptr->disease & (DISEASE_HEAVY)) rating -= 45;
						else if (p_ptr->disease & (DISEASE_QUICK)) rating -= 30;
						else if (p_ptr->disease & (DISEASE_POWER)) rating -= 15;

						if (rating > 0)
						{
							p_ptr->disease |= (1 << rand_int(DISEASE_TYPES_HEAVY));
							rating -= 15;

							if ((rating > 45) && (rand_int(4)))
							{
								p_ptr->disease |= (DISEASE_HEAVY);
								rating -= 60;
							}
							else if ((rating > 15) && (rand_int(3)))
							{
								p_ptr->disease |= (DISEASE_QUICK);
								rating -= 30;
							}
							else if ((rating > 15) && (rand_int(2)))
							{
								p_ptr->disease |= (DISEASE_POWER);
								rating -= 30;
							}
						}

						p_ptr->disease &= ~(DISEASE_LIGHT);
					}

					if (rating < 0) rating = 0;
				}
				/* Serious disease - multiple mutating disease effects for long time */
				else if (rating > 25)
				{
					if (!p_ptr->disease)
					{
						p_ptr->disease |= (1 << rand_int(DISEASE_TYPES));
						p_ptr->disease |= (DISEASE_DISEASE);											
					}
					else if (!p_ptr->disease & (DISEASE_HEAVY | DISEASE_QUICK | DISEASE_POWER)) p_ptr->disease |= (DISEASE_DISEASE);
					p_ptr->disease &= ~(DISEASE_LIGHT);
				}
				/* Moderate disease - 1 disease effect for a long time, or multiple for a short time */
				else if (rating > 10)
				{
					if ((!p_ptr->disease) && (rand_int(100) < 33))
					{
						p_ptr->disease |= (1 << rand_int(DISEASE_TYPES)) | (DISEASE_LIGHT);
					}

					if ((!p_ptr->disease) || ((p_ptr->disease & (DISEASE_LIGHT)) != 0))
					{
						p_ptr->disease |= (1 << rand_int(DISEASE_TYPES));
					}
				}
				/* Light disease - 1 disease effect for limited time */
				else if (rating > 0)
				{
					if (!p_ptr->disease)
					{
						p_ptr->disease |= (1 << rand_int(DISEASE_TYPES));
						p_ptr->disease |= (DISEASE_LIGHT);
					}
				}
				/* Very light disease - stops recovery of hp for limited time */
				else
				{
					if (!p_ptr->disease)
						p_ptr->disease |= (DISEASE_LIGHT);
				}
			}

			/* Apply damage */
			take_hit(dam, killer);
			
			/* Suffer the disease */
			if (!p_ptr->is_dead) suffer_disease();

			break;
		}

		/* Standard damage */
		case GF_EXPLODE:
		{
			if (fuzzy) msg_print("You are hit by something!");
			take_hit(dam, killer);
			break;
		}

		/* Holy Orb -- Player only takes partial damage unless evil */
		case GF_HOLY_ORB:
		{
			if (fuzzy) msg_print("You are hit by something!");

			/* Disease resistance */
			if ((p_ptr->cur_flags4 & (TR4_EVIL)) != 0)
			{
				/* Always notice */
				player_can_flags(who, 0x0L,0x0L,0x0L,TR4_EVIL);

			}
			else
			{
				/* Always notice */
				player_not_flags(who, 0x0L,0x0L,0x0L,TR4_EVIL);

				/* Reduce damage */
				dam /= 2;
			}

			take_hit(dam, killer);
			break;
		}

		case GF_HELLFIRE:
		{
			if (fuzzy) msg_print("You are hit by hellfire!");
			fire_dam(2 * dam / 3, 0, killer, TRUE);

			if (p_ptr->cur_flags4 & (TR4_EVIL))
			{
				/* Always notice */
				player_can_flags(who, 0x0L,0x0L,0x0L,TR4_EVIL);

				dam = 0;
			}
			else if (p_ptr->cur_flags2 & (TR2_RES_DARK))
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_DARK,0x0L,0x0L);

				dam = dam / 3;
			}
			else
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_DARK,0x0L,0x0L);

				/* Hack -- ignore resist blindness */
				(void)set_blind(p_ptr->blind + rand_range(4, 8));
			}
			take_hit(dam /  3, killer);

			/* Test player's saving throw. */
			if (!(p_ptr->cur_flags4 & (TR4_EVIL)) || (rand_int(100) > p_ptr->skill_sav))
			{
				/* Always notice */
				player_not_flags(who, 0x0L,0x0L,0x0L,TR4_EVIL);

				/* Note lack of resistance */
				if (!(p_ptr->cur_flags2 & (TR2_RES_FEAR | TR2_RES_CHAOS | TR2_RES_CONFU)))
				{
					msg_print("Visions of hell invade your mind!");
				}

				/* Resist fear */
				if (p_ptr->cur_flags2 & (TR2_RES_FEAR))
				{
					/* Sometimes notice */
					if(!(p_ptr->hero || p_ptr->shero)) player_can_flags(who, 0x0L,TR2_RES_FEAR,0x0L,0x0L);
				}
				else
				{
					/* Always notice */
					player_not_flags(who, 0x0L,TR2_RES_FEAR,0x0L,0x0L);

					(void)set_afraid(p_ptr->afraid + rand_range(10, 20));
				}

				/* Resist chaos */
				if (p_ptr->cur_flags2 & (TR2_RES_CHAOS))
				{
					/* Always notice */
					player_can_flags(who, 0x0L,TR2_RES_CHAOS,0x0L,0x0L);
				}
				else
				{
					/* Always notice */
					player_not_flags(who, 0x0L,TR2_RES_CHAOS,0x0L,0x0L);

					(void)set_image(p_ptr->image + rand_range(40, 80));
				}

				/* Resist confusion */
				if (p_ptr->cur_flags2 & (TR2_RES_CONFU))
				{
					/* Sometimes notice */
					player_can_flags(who, 0x0L,TR2_RES_CONFU,0x0L,0x0L);
				}
				else
				{
					/* Always notice */
					player_not_flags(who, 0x0L,TR2_RES_CHAOS,0x0L,0x0L);

					(void)set_confused(p_ptr->confused + rand_range(15, 30));
				}
			}

			break;
		}

		/* Plasma -- No resist XXX */
		case GF_PLASMA:
		{
			if (fuzzy) msg_print("You are hit by something!");
			take_hit(dam, killer);
			if ((p_ptr->cur_flags2 & (TR2_RES_SOUND)) == 0)
			{
				int k = (randint((dam > 40) ? 35 : (dam * 3 / 4 + 5)));

				/* Notice */
				player_not_flags(who, 0x0L,TR2_RES_SOUND,0x0L,0x0L);

				(void)set_stun(p_ptr->stun ? p_ptr->stun + randint(MIN(10,k)) : k);
			}
			else
			{
				/* Notice */
				player_can_flags(who, 0x0L,TR2_RES_SOUND,0x0L,0x0L);
			}
			break;
		}

		/* Nether -- drain experience */
		case GF_NETHER:
		{
			if (fuzzy) msg_print("You are hit by something strange!");
			if ((p_ptr->cur_flags2 & (TR2_RES_NETHR)) != 0)
			{
				/* Notice */
				player_can_flags(who, 0x0L,TR2_RES_NETHR,0x0L,0x0L);

				dam *= 6; dam /= (randint(6) + 6);
			}
			else
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_NETHR,0x0L,0x0L);

				if (((p_ptr->cur_flags3 & (TR3_HOLD_LIFE)) != 0) && (rand_int(100) < 75))
				{
					msg_print("You keep hold of your life force!");

					/* Always notice */
					if (!p_ptr->blessed) player_can_flags(who, 0x0L,0x0L,TR3_HOLD_LIFE,0x0L);
				}
				else if ((p_ptr->cur_flags3 & (TR3_HOLD_LIFE)) != 0)
				{
					msg_print("You feel your life slipping away!");
					lose_exp(200 + (p_ptr->exp/1000) * MON_DRAIN_LIFE);

					/* Always notice */
					if (!p_ptr->blessed) player_can_flags(who, 0x0L,0x0L,TR3_HOLD_LIFE,0x0L);

				}
				else
				{
					msg_print("You feel your life draining away!");
					lose_exp(200 + (p_ptr->exp/100) * MON_DRAIN_LIFE);

					/* Always notice */
					player_not_flags(who, 0x0L,0x0L,TR3_HOLD_LIFE,0x0L);
				}
			}
			take_hit(dam, killer);
			break;
		}

		/* Water -- stun/wet/push around later */
		case GF_WATER:
		{
			if (fuzzy) msg_print("You are hit by something!");
			if ((p_ptr->cur_flags2 & (TR2_RES_SOUND)) == 0)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_SOUND,0x0L,0x0L);

				(void)set_stun(p_ptr->stun ? p_ptr->stun + randint(10) : randint(40));
			}
			else
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_SOUND,0x0L,0x0L);
			}

			if ((p_ptr->cur_flags4 & (TR4_HURT_WATER)) != 0)
			{
				/* Always notice */
				player_can_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_WATER);

				dam *= 2;
			}
			else
			{
				/* Always notice */
				player_not_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_WATER);
			}

			water_dam(who, dam, killer, TRUE);

			/* Mark grid for later processing. */
			cave_temp_mark(y, x, FALSE);

			break;
		}


		/* Weak water -- wet only */
		case GF_SALT_WATER:
		case GF_WATER_WEAK:
		{
			if (fuzzy) msg_print("You are hit by something!");

			if ((p_ptr->cur_flags4 & (TR4_HURT_WATER)) != 0)
			{
				/* Always notice */
				player_can_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_WATER);
			}
			else
			{
				/* Always notice */
				player_not_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_WATER);

				dam = 0;
			}

			water_dam(who, dam, killer, TRUE);

			break;
		}


		/* Chaos -- many effects */
		case GF_CHAOS:
		{
			if (fuzzy) msg_print("You are hit by something strange!");

			if ((p_ptr->cur_flags2 & (TR2_RES_CHAOS)) != 0)
			{
				dam *= 6; dam /= (randint(6) + 6);
			}

			if ((p_ptr->cur_flags2 & (TR2_RES_CONFU)) == 0)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_CONFU,0x0L,0x0L);

				(void)set_confused(p_ptr->confused ? p_ptr->confused + 1: rand_int(20) + 10);
			}
			else
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_CONFU,0x0L,0x0L);
			}

			if ((p_ptr->cur_flags2 & (TR2_RES_CHAOS)) == 0)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_CHAOS,0x0L,0x0L);

				(void)set_image(p_ptr->image ? p_ptr->image + 1 : randint(10));
			}
			else
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_CHAOS,0x0L,0x0L);
			}

			if (((p_ptr->cur_flags2 & (TR2_RES_NETHR)) == 0) && ((p_ptr->cur_flags2 & (TR2_RES_CHAOS)) == 0))
			{
				/* Notice */
				player_not_flags(who, 0x0L,TR2_RES_NETHR,0x0L,0x0L);

				if (((p_ptr->cur_flags3 & (TR3_HOLD_LIFE)) != 0) && (rand_int(100) < 75))
				{
					msg_print("You keep hold of your life force!");

					/* Always notice */
					if (!p_ptr->blessed) player_can_flags(who, 0x0L,0x0L,TR3_HOLD_LIFE,0x0L);
				}
				else if ((p_ptr->cur_flags3 & (TR3_HOLD_LIFE)) != 0)
				{
					msg_print("You feel your life slipping away!");
					lose_exp(500 + (p_ptr->exp/1000) * MON_DRAIN_LIFE);

					/* Always notice */
					if (!p_ptr->blessed) player_can_flags(who, 0x0L,0x0L,TR3_HOLD_LIFE,0x0L);
				}
				else
				{
					msg_print("You feel your life draining away!");
					lose_exp(5000 + (p_ptr->exp/100) * MON_DRAIN_LIFE);

					/* Always notice */
					player_not_flags(who, 0x0L,0x0L,TR3_HOLD_LIFE,0x0L);
				}
			}
			else if ((p_ptr->cur_flags2 & (TR2_RES_NETHR)) != 0)
			{
				/* Notice */
				player_not_flags(who, 0x0L,TR2_RES_NETHR,0x0L,0x0L);
			}

			take_hit(dam, killer);
			break;
		}

		/* Shards -- mostly cutting */
		case GF_SHARD:
		{
			if (fuzzy) msg_print("You are hit by something sharp!");
			if ((p_ptr->cur_flags2 & (TR2_RES_SHARD)) != 0)
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_SHARD,0x0L,0x0L);

				dam *= 6; dam /= (randint(6) + 6);
			}
			else
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_SHARD,0x0L,0x0L);

				/* Inflict disease */
				if ((p_ptr->cut) && (randint(150) < dam))
				{
					/* Serious affliction */
					if (dam > 50)
					{
						if (p_ptr->disease == (DISEASE_CUT | DISEASE_LIGHT)) p_ptr->disease = 0;
						p_ptr->disease |= (DISEASE_CUT);

					}
					else if ((p_ptr->disease == 0) || (p_ptr->disease & (DISEASE_LIGHT)))
					{
						p_ptr->disease |= (DISEASE_CUT | DISEASE_LIGHT);
					}
				}

				/* Inflict cuts */
				(void)set_cut(p_ptr->cut + dam);
			}
			take_hit(dam, killer);
			break;
		}

		/* Sound -- mostly stunning */
		case GF_SOUND:
		{
			if (fuzzy) msg_print("You are hit by something!");
			if ((p_ptr->cur_flags2 & (TR2_RES_SOUND)) != 0)
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_SOUND,0x0L,0x0L);

				dam *= 5; dam /= (randint(6) + 6);
			}
			else
			{
				int k = (randint((dam > 90) ? 35 : (dam / 3 + 5)));

				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_SOUND,0x0L,0x0L);

				/* Inflict disease */
				if ((p_ptr->stun) && (randint(100) < k))
				{
					/* Serious affliction */
					if (k > 20)
					{
						if (p_ptr->disease == (DISEASE_STUN | DISEASE_LIGHT)) p_ptr->disease = 0;
						p_ptr->disease |= (DISEASE_STUN);
					}
					else if ((p_ptr->disease == 0) || (p_ptr->disease & (DISEASE_LIGHT)))
					{
						p_ptr->disease |= (DISEASE_STUN | DISEASE_LIGHT);
					}
				}

				/* Inflict stun */
				(void)set_stun(p_ptr->stun ? p_ptr->stun + randint(MIN(10, k)) : k);
			}
			take_hit(dam, killer);
			break;
		}

		/* Pure confusion */
		case GF_CONFUSION:
		{
			if (fuzzy) msg_print("You are hit by something!");
			if ((p_ptr->cur_flags2 & (TR2_RES_CONFU)) != 0)
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_CONFU,0x0L,0x0L);

				dam *= 5; dam /= (randint(6) + 6);
			}
			else
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_CONFU,0x0L,0x0L);

				/* Inflict disease */
				if ((p_ptr->confused) && (randint(300) < dam))
				{
					/* Serious affliction */
					if (dam > 50)
					{
						if (p_ptr->disease == (DISEASE_CONFUSE | DISEASE_LIGHT)) p_ptr->disease = 0;
						p_ptr->disease |= (DISEASE_CONFUSE);
					}
					else if ((p_ptr->disease == 0) || (p_ptr->disease & (DISEASE_LIGHT)))
					{
						p_ptr->disease |= (DISEASE_CONFUSE | DISEASE_LIGHT);
					}
				}

				/* Inflict confusion */
				(void)set_confused(p_ptr->confused ? p_ptr->confused + 1 : randint(20) + 10);

			}

			take_hit(dam, killer);
			break;
		}

		/* Hallucination */
		case GF_HALLU:
		{
			if (fuzzy) msg_print("You are hit by something!");

			/* Take damage */
			take_hit(dam, killer);

			/* Increase "image" */
			if ((p_ptr->cur_flags2 & (TR2_RES_CHAOS)) == 0)
			{
				int k = 6 + randint(dam / 2);

				/* Inflict disease */
				if ((p_ptr->image) && (randint(100) < k))
				{
					/* Serious affliction */
					if (k > 20)
					{
						if (p_ptr->disease == (DISEASE_HALLUC | DISEASE_LIGHT)) p_ptr->disease = 0;
						p_ptr->disease |= (DISEASE_HALLUC);
					}
					else if ((p_ptr->disease == 0) || (p_ptr->disease & (DISEASE_LIGHT)))
					{
						p_ptr->disease |= (DISEASE_HALLUC | DISEASE_LIGHT);
					}
				}

				/* Inflict hallucination */
				if (set_image(p_ptr->image ? p_ptr->image + 1 : k))
				{
					obvious = TRUE;
				}

				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_CHAOS,0x0L,0x0L);
			}
			else
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_CHAOS,0x0L,0x0L);
			}
			break;
		}

		/* Disenchantment -- see above */
		case GF_DISENCHANT:
		{
			if (fuzzy) msg_print("You are hit by something strange!");
			if ((p_ptr->cur_flags2 & (TR2_RES_DISEN)) != 0)
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_DISEN,0x0L,0x0L);

				dam *= 6; dam /= (randint(6) + 6);
			}
			else
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_DISEN,0x0L,0x0L);

				(void)apply_disenchant(0);
			}
			take_hit(dam, killer);
			break;
		}

		/* Nexus -- see above */
		case GF_NEXUS:
		{
			if (fuzzy) msg_print("You are hit by something strange!");
			if ((p_ptr->cur_flags2 & (TR2_RES_NEXUS)) != 0)
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_NEXUS,0x0L,0x0L);

				dam *= 6; dam /= (randint(6) + 6);
			}
			else if (who > SOURCE_MONSTER_START)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_NEXUS,0x0L,0x0L);

				apply_nexus(m_ptr);
			}
			take_hit(dam, killer);
			break;
		}

		/* Force -- mostly stun / big push */
		case GF_FORCE:
		{
			if (fuzzy) msg_print("You are hit by something!");
			if ((p_ptr->cur_flags2 & (TR2_RES_SOUND)) == 0)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_SOUND,0x0L,0x0L);

				(void)set_stun(p_ptr->stun ? p_ptr->stun + randint(10) : randint(20));
			}
			else
			{
				player_can_flags(who, 0x0L,TR2_RES_SOUND,0x0L,0x0L);
			}
			take_hit(dam, killer);

			/* Mark grid for later processing. */
			cave_temp_mark(y, x, FALSE);

			break;
		}

		/* Inertia -- slowness */
		case GF_INERTIA:
		{
			if (fuzzy) msg_print("You are hit by something strange!");
			(void)set_slow(p_ptr->slow + rand_int(4) + 4);
			take_hit(dam, killer);
			break;
		}

		/* Weak lite -- blinding, plus damage if vulnerable */
		case GF_LITE_WEAK:
		{
			if ((p_ptr->cur_flags4 & (TR4_HURT_LITE)) == 0)
			{
				player_not_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_LITE);
				dam = 0;
			}

			/* Fall through */
		}

		/* Lite -- blinding */
		case GF_LITE:
		{
			if ((fuzzy) && (typ == GF_LITE)) msg_print("You are hit by something!");
			if ((p_ptr->cur_flags4 & (TR4_HURT_LITE)) != 0)
			{
				/* Always notice */
				player_can_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_LITE);

				/* Extra damage */
				if (typ != GF_LITE_WEAK) dam *= 2;
			}
			else player_not_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_LITE);

			if ((p_ptr->cur_flags2 & (TR2_RES_LITE)) != 0)
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_LITE,0x0L,0x0L);

				dam *= 4; dam /= (randint(6) + 6);
			}
			else if (!blind && ((p_ptr->cur_flags2 & (TR2_RES_BLIND)) == 0))
			{
				/* Always notice */
				player_not_flags(who, 0x0L,(TR2_RES_BLIND|TR2_RES_LITE),0x0L,0x0L);

				(void)set_blind(p_ptr->blind + randint(5) + 2);
			}
			else if (!blind)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,(TR2_RES_LITE),0x0L,0x0L);
			}
			take_hit(dam, killer);
			break;
		}

		/* Weak darkness -- blinding only */
		case GF_DARK_WEAK:
		{
			dam = 0;

			/* Fall through */
		}

		/* Dark -- blinding */
		case GF_DARK:
		{
			if ((fuzzy) && (typ == GF_DARK)) msg_print("You are hit by something!");
			if ((p_ptr->cur_flags2 & (TR2_RES_DARK)) != 0)
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_DARK,0x0L,0x0L);

				dam *= 4; dam /= (randint(6) + 6);
			}
			else if (!blind && ((p_ptr->cur_flags2 & (TR2_RES_BLIND)) == 0))
			{
				/* Always notice */
				player_not_flags(who, 0x0L,(TR2_RES_BLIND|TR2_RES_DARK),0x0L,0x0L);

				(void)set_blind(p_ptr->blind + randint(5) + 2);
			}
			else if (!blind)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,(TR2_RES_DARK),0x0L,0x0L);
			}
			take_hit(dam, killer);
			break;
		}

		/* Time -- bolt fewer effects XXX */
		case GF_TIME:
		{
			if (fuzzy) msg_print("You are hit by something strange!");

			switch (randint(10))
			{
				case 1: case 2: case 3: case 4: case 5:
				{
					msg_print("You feel life has clocked back.");
					lose_exp(100 + (p_ptr->exp / 100) * MON_DRAIN_LIFE);
					break;
				}

				case 6: case 7: case 8: case 9:
				{
  				    k = rand_int(A_MAX); /* AGI not coupled with DEX */

					msg_format("You're not as %s as you used to be...", desc_stat_imp_end[k]);

					p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 3) / 4;
					if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
					p_ptr->update |= (PU_BONUS);
					break;
				}

				case 10:
				{
					msg_print("You're not as powerful as you used to be...");

					for (k = 0; k < A_MAX; k++)
					{
						p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 3) / 4;
						if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
					}
					p_ptr->update |= (PU_BONUS);
					break;
				}
			}
			take_hit(dam, killer);
			break;
		}

		/* Gravity -- stun plus slowness plus teleport */
		case GF_GRAVITY:
		{
			if (fuzzy) msg_print("You are hit by something strange!");
			msg_print("Gravity warps around you.");
			(void)set_slow(p_ptr->slow + rand_int(4) + 4);
			if ((p_ptr->cur_flags2 & (TR2_RES_SOUND)) == 0)
			{
				k = (randint((dam > 90) ? 35 : (dam / 3 + 5)));
				(void)set_stun(p_ptr->stun + k);
			}
			take_hit(dam, killer);

			/* Mark grid for later processing. */
			cave_temp_mark(y, x, FALSE);

			break;
		}

		/* Pure damage */
		case GF_MANA:
		{
			if (fuzzy) msg_print("You are hit by something!");
			take_hit(dam, killer);
			break;
		}

		/* Pure damage */
		case GF_METEOR:
		{
			if (fuzzy) msg_print("You are hit by something!");
			take_hit(dam, killer);
			break;
		}

		/* Ice -- cold plus stun plus cuts */
		case GF_ICE:
		{
			if (fuzzy) msg_print("You are hit by something sharp!");

			cold_dam(who, dam, killer, TRUE);

			if ((p_ptr->cur_flags2 & (TR2_RES_SHARD)) == 0)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_SHARD,0x0L,0x0L);

				(void)set_cut(p_ptr->cut + damroll(5, 8));
			}
			else
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_SHARD,0x0L,0x0L);

			}
			if ((p_ptr->cur_flags2 & (TR2_RES_SOUND)) == 0)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_SOUND,0x0L,0x0L);

				(void)set_stun(p_ptr->stun + randint(15));
			}
			else
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_SOUND,0x0L,0x0L);
			}
			break;
		}

		/* Wind -- confusion, but rarely if res_confu */
		case GF_WIND:
		{
			/* Throw distance depends on weight and strength */
			int dist = 20 * dam / p_ptr->wt;
			if (dist > 8) dist = 8;

			/* Feather fall greatly reduces the effect of wind */
			if (p_ptr->cur_flags3 & (TR3_FEATHER))
			{
				/* Always notice */
				player_can_flags(who, 0x0L,0x0L,TR3_FEATHER,0x0L);

				dist = (dist + 2) / 3;
			}
			else
			{
				/* Always notice */
				player_not_flags(who, 0x0L,0x0L,TR3_FEATHER,0x0L);
			}

			/* Scale down damage based on distance */
			dam = dam * dist / 8;

			if (fuzzy) msg_print("You are buffeted by winds!");
			if (dam > randint(200))
			{
				if ((!p_ptr->cur_flags2 & (TR2_RES_CONFU)) || (!rand_int(6)))
				{
					msg_print("You are spun until dizzy!");
					(void)set_confused(p_ptr->confused + rand_range(2, 3));
				}
			}

			/* Take damage */
			take_hit(dam, killer);

			/* Mark grid for later processing. */
			cave_temp_mark(y, x, FALSE);

			break;
		}

		/* Standard damage attacks */
		case GF_BATTER:
		case GF_WOUND:
		case GF_HURT:
		{
			/* Obvious */
			obvious = TRUE;

			/* Take damage */
			take_hit(dam, killer);

			break;
		}

		case GF_UN_BONUS:
		{
			/* Take damage */
			take_hit(dam, killer);

			/* Allow complete resist */
			if ((p_ptr->cur_flags2 & (TR2_RES_DISEN)) == 0)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_DISEN,0x0L,0x0L);

				/* Apply disenchantment */
				if (apply_disenchant(0)) obvious = TRUE;

				/* Monsters get fed */
				if (who > SOURCE_MONSTER_START)
				{
					feed_monster(who);
				}
			}
			else
			{
				/* Sometimes notice */
				player_can_flags(who, 0x0L,TR2_RES_DISEN,0x0L,0x0L);
			}
			break;
		}

		case GF_UN_POWER:
		{
			/* Take damage */
			take_hit(dam, killer);

			/* Find an item */
			for (k = 0; k < 10; k++)
			{
				/* Pick an item */
				i = rand_int(INVEN_PACK);

				/* Obtain the item */
				o_ptr = &inventory[i];

				/* Skip non-objects */
				if (!o_ptr->k_idx) continue;

				/* Drain charged wands/staffs */
				if (((o_ptr->tval == TV_STAFF) ||
				     (o_ptr->tval == TV_WAND)) &&
				    (o_ptr->charges))
				{
					/* Message */
					msg_print("Energy drains from your pack!");

					/* Obvious */
					obvious = TRUE;

					if (who > SOURCE_MONSTER_START)
					{
						/* Heal */
						j = dam/10;
						m_ptr->hp += j * o_ptr->charges * o_ptr->number;
						if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

						/* Redraw (later) if needed */
						if (p_ptr->health_who == who) p_ptr->redraw |= (PR_HEALTH);
					}

					/* Uncharge */
					o_ptr->charges = 0;
					o_ptr->stackc = 0;

					/* Combine / Reorder the pack */
					p_ptr->notice |= (PN_COMBINE | PN_REORDER);

					/* Window stuff */
					p_ptr->window |= (PW_INVEN);

					/* Monsters get fed */
					if (who > SOURCE_MONSTER_START)
					{
						feed_monster(who);
					}

					/* Done */
					break;
				}
			}

			break;
		}

		case GF_EAT_GOLD:
		{
			/* Take damage */
			take_hit(dam, killer);

			/* Obvious */
			obvious = TRUE;

			/* Saving throw (unless paralyzed) based on agi and level */
			if (!p_ptr->paralyzed &&
			    (rand_int(100) < (adj_agi_safe[p_ptr->stat_ind[A_AGI]] +
					      p_ptr->lev)))
			{
				/* Saving throw message */
				msg_print("You quickly protect your money pouch!");

				/* Occasional blink anyway */
				/*if (rand_int(3)) blinked = TRUE;*/
			}

			/* Eat gold */
			else
			{
				gold = (p_ptr->au / 10) + randint(25);
				if (gold < 2) gold = 2;
				if (gold > 5000) gold = (p_ptr->au / 20) + randint(3000);
				if (gold > p_ptr->au) gold = p_ptr->au;
				p_ptr->au -= gold;
				if (gold <= 0)
				{
					msg_print("Nothing was stolen.");
				}
				else if (p_ptr->au)
				{
					msg_print("Your purse feels lighter.");
					msg_format("%ld coins were stolen!", (long)gold);
				}
				else
				{
					msg_print("Your purse feels lighter.");
					msg_print("All of your coins were stolen!");
				}

				/* Redraw gold */
				p_ptr->redraw |= (PR_GOLD);

				/* Window stuff */
				p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);

				/* Run away */
				if (who > SOURCE_MONSTER_START) m_ptr->monfear = 100;
			}

			break;
		}

		case GF_EAT_ITEM:
		{
			/* Take damage */
			take_hit(dam, killer);

			/* Saving throw (unless paralyzed) based on agi and level */
			if (!p_ptr->paralyzed &&
			    (rand_int(100) < (adj_agi_safe[p_ptr->stat_ind[A_AGI]] +
					      p_ptr->lev)))
			{
				/* Saving throw message */
				msg_print("You grab hold of your backpack!");

				/* Occasional "blink" anyway */
				/* blinked = TRUE;*/

				/* Obvious */
				obvious = TRUE;

				/* Done */
				break;
			}

			/* Find an item */
			for (k = 0; k < 10; k++)
			{
				object_type *i_ptr;
				object_type object_type_body;

				u32b f1, f2, f3, f4;

				/* Pick an item */
				i = rand_int(INVEN_PACK);

				/* Obtain the item */
				o_ptr = &inventory[i];

				/* Skip non-objects */
				if (!o_ptr->k_idx) continue;

				/* Clear the flags */
				f1 = f2 = f3 = 0x0L;

				/* Get the flags */
				object_flags(o_ptr,&f1,&f2,&f3,&f4);

				/* Sometimes notice theft-protection */
				if ((rand_int(100)<10) && (f2 & (TR2_IGNORE_THEFT))) object_can_flags(o_ptr,0x0L,TR2_IGNORE_THEFT,0x0L,0x0L, FALSE);

				/* Skip artifacts */
				if (f2 & (TR2_IGNORE_THEFT)) continue;

				/* Get a description */
				object_desc(o_name, sizeof(o_name), o_ptr, FALSE, 3);

				/* Message */
				msg_format("%sour %s (%c) was stolen!",
					   ((o_ptr->number > 1) ? "One of y" : "Y"),
					   o_name, index_to_label(i));

				/* Get local object */
				i_ptr = &object_type_body;

				/* Obtain local object */
				object_copy(i_ptr, o_ptr);

				/* Modify number */
				i_ptr->number = 1;

				/* Reset stack counter */
				i_ptr->stackc = 0;

				/* Sometimes use lower stack object */
				if (!object_charges_p(o_ptr) && (rand_int(o_ptr->number)< o_ptr->stackc))
				{
					if (i_ptr->charges) i_ptr->charges--;

					if (i_ptr->timeout) i_ptr->timeout = 0;

					o_ptr->stackc--;
				}

				/* Forget about it */
				drop_may_flags(i_ptr);

				/* Carry the object */
				if (who > SOURCE_MONSTER_START)
				{
					(void)monster_carry(who, i_ptr);
				}
				else
				{
					/* Hack --- 20% chance of lost forever */
					drop_near(i_ptr,20,p_ptr->py,p_ptr->px);
				}

				/* Forget about item */
				if (o_ptr->number == 1) inven_drop_flags(o_ptr);

				/* Steal the items */
				inven_item_increase(i, -1);
				inven_item_optimize(i);

				/* Obvious */
				obvious = TRUE;

				/* Run away */
				if (who > SOURCE_MONSTER_START) m_ptr->monfear = 100;

				/* Done */
				break;
			}

			break;
		}

		case GF_EAT_FOOD:
		{
			/* Take damage */
			take_hit(dam, killer);

			/* Steal some food */
			for (k = 0; k < 10; k++)
			{
				/* Pick an item from the pack */
				i = rand_int(INVEN_PACK);

				/* Get the item */
				o_ptr = &inventory[i];

				/* Skip non-objects */
				if (!o_ptr->k_idx) continue;

				/* Skip non-food objects */
				if (o_ptr->tval != TV_FOOD) continue;

				/* Get a description */
				object_desc(o_name, sizeof(o_name), o_ptr, FALSE, 0);

				/* Message */
				msg_format("%sour %s (%c) was eaten!",
					   ((o_ptr->number > 1) ? "One of y" : "Y"),
					   o_name, index_to_label(i));

				/* Forget about item */
				if (o_ptr->number == 1) inven_drop_flags(o_ptr);

				/* Steal the items */
				inven_item_increase(i, -1);
				inven_item_optimize(i);

				/* Obvious */
				obvious = TRUE;

				/* Monsters get fed */
				if (who > SOURCE_MONSTER_START)
				{
					feed_monster(who);
				}

				/* Done */
				break;
			}

			break;
		}

		case GF_EAT_LITE:
		{
			/* Take damage */
			take_hit(dam, killer);

			/* Get the lite */
			o_ptr = &inventory[INVEN_LITE];

			/* Drain fuel */
			if ((o_ptr->timeout > 0) && (!artifact_p(o_ptr)))
			{
				/* Reduce fuel */
				o_ptr->timeout -= (250 + randint(250));
				if (o_ptr->timeout < 1) o_ptr->timeout = 1;

				/* Notice */
				if (!p_ptr->blind)
				{
					msg_print("Your light dims.");
					obvious = TRUE;
				}

				/* Window stuff */
				p_ptr->window |= (PW_EQUIP);

				/* Monsters get fed */
				if (who > SOURCE_MONSTER_START)
				{
					feed_monster(who);
				}
			}
			else if (artifact_p(o_ptr))
			{
				if (rand_int(100)<30) object_can_flags(o_ptr,0x0L,0x0L,TR3_INSTA_ART,0x0L, FALSE);
			}
			break;
		}

		case GF_BLIND_WEAK:
		{
			dam = 0;

			/* Fall through */
		}

		case GF_BLIND:
		{
			/* Apply resistance */
			if ((dam) && ((p_ptr->cur_flags2 & (TR2_RES_BLIND)) != 0))
			{
				/* Sometimes notice */
				player_can_flags(who, 0x0L,TR2_RES_BLIND,0x0L,0x0L);

				dam *= 5; dam /= (randint(6) + 6);
			}

			/* Take damage */
			take_hit(dam, killer);

			/* Increase "blind" */
			if ((p_ptr->cur_flags2 & (TR2_RES_BLIND)) == 0)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_BLIND,0x0L,0x0L);

				/* Inflict disease */
				if ((p_ptr->blind) && (randint(400) < dam))
				{
					/* Serious affliction */
					if (dam > 50)
					{
						if (p_ptr->disease == (DISEASE_BLIND | DISEASE_LIGHT)) p_ptr->disease = 0;
						p_ptr->disease |= (DISEASE_BLIND);
					}
					else if ((p_ptr->disease == 0) || (p_ptr->disease & (DISEASE_LIGHT)))
					{
						p_ptr->disease |= (DISEASE_BLIND | DISEASE_LIGHT);
					}
				}

				/* Inflict blindness -- always cumulative */
				if (set_blind(p_ptr->blind + 10 + randint(dam)))
				{
					obvious = TRUE;
				}
			}
			else
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_BLIND,0x0L,0x0L);
			}

			break;
		}

		case GF_FEAR_WEAK:
		{
			dam = 0;

			/* Fall through */
		}

		case GF_TERRIFY:
		{
			/* Apply resistance */
			if ((dam) && ((p_ptr->cur_flags2 & (TR2_RES_FEAR)) != 0))
			{
				/* Sometimes notice */
				if(!(p_ptr->hero || p_ptr->shero)) player_can_flags(who, 0x0L,TR2_RES_FEAR,0x0L,0x0L);

				dam *= 5; dam /= (randint(6) + 6);
			}

			/* Take damage */
			take_hit(dam, killer);

			/* Increase "afraid" */
			if ((p_ptr->cur_flags2 & (TR2_RES_FEAR)) != 0)
			{
				/* Sometimes notice */
				if (!(p_ptr->hero || p_ptr->shero)) player_can_flags(who, 0x0L,TR2_RES_FEAR,0x0L,0x0L);

				msg_print("You stand your ground!");
				obvious = TRUE;
			}
			else if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print("You stand your ground!");
				obvious = TRUE;
			}
			else
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_FEAR,0x0L,0x0L);

				/* Inflict disease */
				if ((p_ptr->afraid) && (randint(400) < dam))
				{
					/* Serious affliction */
					if (dam > 50)
					{
						if (p_ptr->disease == (DISEASE_FEAR | DISEASE_LIGHT)) p_ptr->disease =0;
						p_ptr->disease |= (DISEASE_FEAR);
					}
					else if ((p_ptr->disease == 0) || (p_ptr->disease & (DISEASE_LIGHT)))
					{
						p_ptr->disease |= (DISEASE_FEAR | DISEASE_LIGHT);
					}
				}

				/* Inflict fear - always cumulative */
				if (set_afraid(p_ptr->afraid + 3 + randint(dam)))
				{
					obvious = TRUE;
				}
			}

			break;
		}

		/* Sleep (Use "dam" as "power") */
		case GF_SLEEP:
		{
			/* Increase "paralyzed" */
			if (((p_ptr->cur_flags3 & (TR3_FREE_ACT)) != 0) || (p_ptr->free_act))
			{
				/* Always notice */
				if (!p_ptr->free_act) player_can_flags(who, 0x0L,0x0L,TR3_FREE_ACT,0x0L);
				else update_smart_learn(who, SM_FREE_ACT);

				msg_print("You are unaffected!");
				obvious = TRUE;
			}
			else if (rand_int(100) < p_ptr->skill_sav)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,0x0L,TR3_FREE_ACT,0x0L);

				msg_print("You resist the effects!");
				obvious = TRUE;
			}
			else if (p_ptr->msleep)
			{
				/* Inflict disease */
				if (randint(400) < dam)
				{
					/* Serious affliction */
					if (dam > 150)
					{
						if (p_ptr->disease == (DISEASE_SLEEP | DISEASE_LIGHT)) p_ptr->disease = 0;
						p_ptr->disease |= (DISEASE_SLEEP);
					}
					else if ((p_ptr->disease == 0) || (p_ptr->disease & (DISEASE_LIGHT)))
					{
						p_ptr->disease |= (DISEASE_SLEEP | DISEASE_LIGHT);
					}
				}

				/* Inflict sleepiness */
				if (set_msleep(p_ptr->msleep + dam))
				{
					obvious = TRUE;
				}
			}
			else
			{
				/* Always notice */
				player_not_flags(who, 0x0L,0x0L,TR3_FREE_ACT,0x0L);

				/* Inflict sleepiness */
				if (set_msleep(dam))
				{
					obvious = TRUE;
				}
			}

			break;
		}

		/* Curse */
		case GF_CURSE:
		{
			int k =	10 + randint(15);

			/* Take damage */
			take_hit(dam, killer);

			/* Blessing protects */
			if (p_ptr->blessed)
			{
				msg_print("The gods protect you.");
			}
			else if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print("You resist the effects!");
				obvious = TRUE;
			}
			else if (p_ptr->cursed)
			{
				/* Inflict disease */
				if (randint(400) < dam)
				{
					/* Serious affliction */
					if (dam > 150)
					{
						if (p_ptr->disease == (DISEASE_CURSE | DISEASE_LIGHT)) p_ptr->disease = 0;
						p_ptr->disease |= (DISEASE_CURSE);
					}
					else if ((p_ptr->disease == 0) || (p_ptr->disease & (DISEASE_LIGHT)))
					{
						p_ptr->disease |= (DISEASE_CURSE | DISEASE_LIGHT);
					}
				}

				/* Inflict curse */
				if (set_cursed(p_ptr->cursed + k))
				{
					obvious = TRUE;
				}
			}
			else
			{
				/* Inflict curse */
				if (set_cursed(k))
				{
					obvious = TRUE;
				}
			}

			break;
		}

		/* Make the player temporarily forgetful */
		case GF_FORGET:
		{
			int k =	10 + randint(15);

			/* Take damage */
			take_hit(dam, killer);

			/* Only save protects */
			if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print("You resist the effects!");
				obvious = TRUE;
			}
			else if (p_ptr->amnesia)
			{
				/* Inflict disease */
				if (randint(400) < dam)
				{
					/* Serious affliction */
					if (dam > 150)
					{
						if (p_ptr->disease == (DISEASE_AMNESIA | DISEASE_LIGHT)) p_ptr->disease = 0;
						p_ptr->disease |= (DISEASE_AMNESIA);
					}
					else if ((p_ptr->disease == 0) || (p_ptr->disease & (DISEASE_LIGHT)))
					{
						p_ptr->disease |= (DISEASE_AMNESIA | DISEASE_LIGHT);
					}
				}

				/* Inflict amnesia */
				if (set_amnesia(k))
				{
					obvious = TRUE;
				}
			}
			else
			{
				/* Inflict amnesia */
				if (set_amnesia(k))
				{
					obvious = TRUE;
				}
			}

			break;
		}

		/* Petrification */
		case GF_PETRIFY:
		{
			int k = randint((dam / 3) + 2);

			/* Take damage */
			take_hit(dam, killer);

			/* Increase "petrification" */
			if ((p_ptr->cur_flags2 & (TR2_RES_SHARD)) != 0)
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_SHARD,0x0L,0x0L);

				msg_print("You are unaffected!");
				obvious = TRUE;
			}
			else if (rand_int(100) < p_ptr->skill_sav)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_SHARD,0x0L,0x0L);

				msg_print("You resist the effects!");
				obvious = TRUE;
			}
			else if (p_ptr->petrify)
			{
				/* Inflict disease */
				if (randint(200) < dam)
				{
					/* Serious affliction */
					if (dam > 100)
					{
						if (p_ptr->disease == (DISEASE_PETRIFY | DISEASE_LIGHT)) p_ptr->disease = 0;
						p_ptr->disease |= (DISEASE_PETRIFY);
					}
					else if ((p_ptr->disease == 0) || (p_ptr->disease & (DISEASE_LIGHT)))
					{
						p_ptr->disease |= (DISEASE_PETRIFY | DISEASE_LIGHT);
					}
				}

				/* Inflict petrification */
				if (set_petrify(p_ptr->petrify + k))
				{
					obvious = TRUE;
				}
			}
			else
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_SHARD,0x0L,0x0L);

				/* Inflict petrification */
				if (set_petrify(k))
				{
					obvious = TRUE;
				}
			}

			break;
		}

		/* Paralyze */
		case GF_PARALYZE:
		{
			/* Hack -- Prevent perma-paralysis via damage */
			if (p_ptr->paralyzed && (dam < 1)) dam = 1;

			/* Take damage */
			take_hit(dam, killer);

			/* Increase "paralyzed" */
			if (((p_ptr->cur_flags3 & (TR3_FREE_ACT)) != 0) || (p_ptr->free_act))
			{
				/* Always notice */
				if (!p_ptr->free_act) player_can_flags(who, 0x0L,0x0L,TR3_FREE_ACT,0x0L);
				else update_smart_learn(who, SM_FREE_ACT);

				msg_print("You are unaffected!");
				obvious = TRUE;
			}
			else if (rand_int(100) < p_ptr->skill_sav)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,0x0L,TR3_FREE_ACT,0x0L);

				msg_print("You resist the effects!");
				obvious = TRUE;
			}
			else if (p_ptr->paralyzed)
			{
				/* Inflict disease */
				if (randint(20) < dam)
				{
					/* Serious affliction */
					if (dam > 50)
					{
						if (p_ptr->disease == (DISEASE_PARALYZE | DISEASE_LIGHT)) p_ptr->disease = 0;
						p_ptr->disease |= (DISEASE_PARALYZE);
					}
					else if ((p_ptr->disease == 0) || (p_ptr->disease & (DISEASE_LIGHT)))
					{
						p_ptr->disease |= (DISEASE_PARALYZE | DISEASE_LIGHT);
					}
				}
			}
			else
			{
				int k = randint((dam / 3) + 2);

				/* Always notice */
				player_not_flags(who, 0x0L,0x0L,TR3_FREE_ACT,0x0L);

				/* Inflict paralyzation */
				if (set_paralyzed(k))
				{
					obvious = TRUE;
				}
			}

			break;
		}

		/* Stastis */
		case GF_STASTIS:
		{
			/* Hack -- Prevent perma-paralysis via damage */
			if (p_ptr->stastis && (dam < 1)) dam = 1;

			/* Take damage */
			take_hit(dam, killer);

			/* Only save protects */
			if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print("You resist the effects!");
				obvious = TRUE;
			}
			else if (p_ptr->stastis)
			{
				/* Inflict disease */
				if (randint(400) < dam)
				{
					/* Serious affliction */
					if (dam > 250)
					{
						if (p_ptr->disease == (DISEASE_STASTIS | DISEASE_LIGHT)) p_ptr->disease = 0;
						p_ptr->disease |= (DISEASE_STASTIS);
					}
					else if ((p_ptr->disease == 0) || (p_ptr->disease & (DISEASE_LIGHT)))
					{
						p_ptr->disease |= (DISEASE_STASTIS | DISEASE_LIGHT);
					}
				}
			}
			else
			{
				int k = randint((dam / 3) + 2);

				/* Inflict stastis */
				if (set_stastis(k))
				{
					obvious = TRUE;
				}
			}

			break;
		}

		case GF_SLOW:
		{
			/* Take damage */
			take_hit(dam, killer);

			/* Increase "paralyzed" */
			if (((p_ptr->cur_flags3 & (TR3_FREE_ACT)) != 0) || (p_ptr->free_act))
			{
				/* Always notice */
				if (!p_ptr->free_act) player_can_flags(who, 0x0L,0x0L,TR3_FREE_ACT,0x0L);
				else update_smart_learn(who, SM_FREE_ACT);

				msg_print("You are unaffected!");
				obvious = TRUE;
			}
			else if (rand_int(100) < p_ptr->skill_sav)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,0x0L,TR3_FREE_ACT,0x0L);

				msg_print("You resist the effects!");
				obvious = TRUE;
			}
			else
			{
				/* Always notice */
				player_not_flags(who, 0x0L,0x0L,TR3_FREE_ACT,0x0L);

				/* Inflict disease */
				if ((p_ptr->slow) && (randint(200) < dam))
				{
					/* Serious affliction */
					if (dam > 50)
					{
						if (p_ptr->disease == (DISEASE_SLOW | DISEASE_LIGHT)) p_ptr->disease = 0;
						p_ptr->disease |= (DISEASE_SLOW);
					}
					else if ((p_ptr->disease == 0) || (p_ptr->disease & (DISEASE_LIGHT | DISEASE_DISEASE)))
					{
						p_ptr->disease |= (DISEASE_SLOW | DISEASE_LIGHT);
					}
				}

				/* Inflict slowness */
				if (set_slow(p_ptr->slow ? p_ptr->slow + 3 : randint(25) + 15)) obvious = TRUE;
				{
					obvious = TRUE;
				}
			}
			break;
		}

		case GF_LOSE_STR:
		{
			/* Take damage */
			take_hit(dam, killer);

			/* Damage (stat) */
			if (do_dec_stat(A_STR))
			{
				obvious = TRUE;
				set_stat_dec_tim(p_ptr->stat_dec_tim[A_STR] + rand_int(20) + 20, A_STR);
			}

			/* Damage (stat) */
			if (do_dec_stat(A_SIZ))
			{
				obvious = TRUE;
				set_stat_dec_tim(p_ptr->stat_dec_tim[A_SIZ] + rand_int(20) + 20, A_SIZ);
			}

			break;
		}

		case GF_LOSE_INT:
		{
			/* Take damage */
			take_hit(dam, killer);

			/* Damage (stat) */
			if (do_dec_stat(A_INT))
			{
				obvious = TRUE;
				set_stat_dec_tim(p_ptr->stat_dec_tim[A_INT] + rand_int(20) + 20, A_INT);
			}

			break;
		}

		case GF_LOSE_WIS:
		{
			/* Take damage */
			take_hit(dam, killer);

			/* Damage (stat) */
			if (do_dec_stat(A_WIS))
			{
				obvious = TRUE;
				set_stat_dec_tim(p_ptr->stat_dec_tim[A_WIS] + rand_int(20) + 20, A_WIS);
			}

			break;
		}

		case GF_LOSE_DEX:
		{
			/* Take damage */
			take_hit(dam, killer);

			/* Damage (stat) */
			if (do_dec_stat(A_DEX))
			{
				obvious = TRUE;
				set_stat_dec_tim(p_ptr->stat_dec_tim[A_DEX] + rand_int(20) + 20, A_DEX);
			}
			/* Damage (stat) */
			if (do_dec_stat(A_AGI))
			{
				obvious = TRUE;
				set_stat_dec_tim(p_ptr->stat_dec_tim[A_AGI] + rand_int(20) + 20, A_AGI);
			}

			break;
		}

		case GF_LOSE_CON:
		{
			/* Take damage */
			take_hit(dam, killer);

			/* Damage (stat) */
			if (do_dec_stat(A_CON))
			{
				obvious = TRUE;
				set_stat_dec_tim(p_ptr->stat_dec_tim[A_CON] + rand_int(20) + 20, A_CON);
			}

			break;
		}

		case GF_LOSE_CHR:
		{
			/* Take damage */
			take_hit(dam, killer);

			/* Damage (stat) */
			if (do_dec_stat(A_CHR))
			{
				obvious = TRUE;
				set_stat_dec_tim(p_ptr->stat_dec_tim[A_CHR] + rand_int(20) + 20, A_CHR);
			}

			break;
		}

		case GF_LOSE_ALL:
		{
			/* Take damage */
			take_hit(dam, killer);

			/* Lose all stats */
			for (k = 0; k < A_MAX; k++)
			{
				/* Damage (stats) */
				if (do_dec_stat(k))
				{
					obvious = TRUE;
					set_stat_dec_tim(p_ptr->stat_dec_tim[k] + rand_int(20) + 20, k);
				}
			}

			break;
		}

		case GF_SHATTER:
		{
			/* Obvious */
			obvious = TRUE;

			/* Hack -- Reduce damage based on the player armor class */
			dam -= (dam * ((p_ptr->ac < 150) ? p_ptr->ac : 150) / 250);

			/* Take damage */
			take_hit(dam, killer);

			if (who > SOURCE_MONSTER_START)
			{
				/* Radius 8 earthquake centered at the monster */
				if (dam > 23) earthquake(m_ptr->fy, m_ptr->fx, 8);
			}

			else
			{
				/* Radius 8 earthquake centered at the player */
				if (dam > 23) earthquake(p_ptr->py, p_ptr->px, 8);

			}

			break;
		}

		case GF_EXP_10:
		{
			/* Obvious */
			obvious = TRUE;

			/* Take damage */
			take_hit(dam, killer);

			if (((p_ptr->cur_flags3 & (TR3_HOLD_LIFE)) != 0)  && (rand_int(100) < 95))
			{
				/* Always notice */
				if (!p_ptr->blessed) player_can_flags(who, 0x0L,0x0L,TR3_HOLD_LIFE,0x0L);

				msg_print("You keep hold of your life force!");
			}
			else
			{
				s32b d = damroll(10, 6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
				if ((p_ptr->cur_flags3 & (TR3_HOLD_LIFE)) != 0)
				{
					/* Always notice */
					if (!p_ptr->blessed) player_can_flags(who, 0x0L,0x0L,TR3_HOLD_LIFE,0x0L);

					msg_print("You feel your life slipping away!");
					lose_exp(d/10);
				}
				else
				{
					/* Always notice */
					player_not_flags(who, 0x0L,0x0L,TR3_HOLD_LIFE,0x0L);

					msg_print("You feel your life draining away!");
					lose_exp(d);
				}
			}
			break;
		}

		case GF_EXP_20:
		{
			/* Obvious */
			obvious = TRUE;

			/* Take damage */
			take_hit(dam, killer);

			if (((p_ptr->cur_flags3 & (TR3_HOLD_LIFE)) != 0) && (rand_int(100) < 90))
			{
				/* Always notice */
				if (!p_ptr->blessed) player_can_flags(who, 0x0L,0x0L,TR3_HOLD_LIFE,0x0L);

				msg_print("You keep hold of your life force!");
			}
			else
			{
				s32b d = damroll(20, 6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
				if ((p_ptr->cur_flags3 & (TR3_HOLD_LIFE)) != 0)
				{
					msg_print("You feel your life slipping away!");

					/* Always notice */
					if (!p_ptr->blessed) player_can_flags(who, 0x0L,0x0L,TR3_HOLD_LIFE,0x0L);

					lose_exp(d/10);
				}
				else
				{
					/* Always notice */
					player_not_flags(who, 0x0L,0x0L,TR3_HOLD_LIFE,0x0L);

					msg_print("You feel your life draining away!");
					lose_exp(d);
				}
			}
			break;
		}

		case GF_EXP_40:
		{
			/* Obvious */
			obvious = TRUE;

			/* Take damage */
			take_hit(dam, killer);

			if (((p_ptr->cur_flags3 & (TR3_HOLD_LIFE)) != 0) && (rand_int(100) < 75))
			{
				/* Always notice */
				if (!p_ptr->blessed) player_can_flags(who, 0x0L,0x0L,TR3_HOLD_LIFE,0x0L);

				msg_print("You keep hold of your life force!");
			}
			else
			{
				s32b d = damroll(40, 6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
				if ((p_ptr->cur_flags3 & (TR3_HOLD_LIFE)) != 0)
				{
					/* Always notice */
					if (!p_ptr->blessed) player_can_flags(who, 0x0L,0x0L,TR3_HOLD_LIFE,0x0L);

					msg_print("You feel your life slipping away!");
					lose_exp(d/10);
				}
				else
				{
					/* Always notice */
					player_not_flags(who, 0x0L,0x0L,TR3_HOLD_LIFE,0x0L);

					msg_print("You feel your life draining away!");
					lose_exp(d);
				}
			}
			break;
		}

		case GF_EXP_80:
		{
			/* Obvious */
			obvious = TRUE;

			/* Take damage */
			take_hit(dam, killer);

			if (((p_ptr->cur_flags3 & (TR3_HOLD_LIFE)) != 0) && (rand_int(100) < 50))
			{
				/* Always notice */
				if (!p_ptr->blessed) player_can_flags(who, 0x0L,0x0L,TR3_HOLD_LIFE,0x0L);

				msg_print("You keep hold of your life force!");
			}
			else
			{
				s32b d = damroll(80, 6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
				if ((p_ptr->cur_flags3 & (TR3_HOLD_LIFE)) != 0)
				{
					/* Always notice */
					if (!p_ptr->blessed) player_can_flags(who, 0x0L,0x0L,TR3_HOLD_LIFE,0x0L);

					msg_print("You feel your life slipping away!");
					lose_exp(d/10);
				}
				else
				{
					/* Always notice */
					player_not_flags(who, 0x0L,0x0L,TR3_HOLD_LIFE,0x0L);

					msg_print("You feel your life draining away!");
					lose_exp(d);
				}
			}
			break;
		}

		case GF_FALL:
		{

			if ((p_ptr->cur_flags3 & (TR3_FEATHER)) != 0)
			{
				/* Always notice */
				player_can_flags(who, 0x0L,0x0L,TR3_FEATHER,0x0L);

				msg_print("You float gently down to the bottom of the pit.");
				dam=0;
			}
			else
			{
				/* Always notice */
				player_not_flags(who, 0x0L,0x0L,TR3_FEATHER,0x0L);

				take_hit(dam, killer);
			}

			break;
		}

		case GF_FALL_MORE:
		case GF_FALL_LESS:
		  {
		    if (typ == GF_FALL_MORE && 
			(p_ptr->cur_flags3 & TR3_FEATHER) != 0)
		      {
			/* Always notice */
			player_can_flags(who, 0x0L,0x0L,TR3_FEATHER,0x0L);

			if (typ == GF_FALL_MORE)
			  msg_print("You gently float down to the next level.");
				
			dam = 0;
		      }
		    else
		      {
			if (typ == GF_FALL_MORE)
			  /* Always notice */
			  player_not_flags(who, 0x0L,0x0L,TR3_FEATHER,0x0L);
			  
			take_hit(dam, killer);
		      }

		    /* Hack -- no chasm/trap doors on quest levels */
		    if (!max_depth(p_ptr->dungeon)
			|| (typ == GF_FALL_MORE 
			    && (is_quest(p_ptr->depth) 
				|| p_ptr->depth == max_depth(p_ptr->dungeon)))
			|| (typ == GF_FALL_LESS
			    && p_ptr->depth == min_depth(p_ptr->dungeon)))
		      {
			/* Mark grid for later processing. */
			cave_temp_mark(y, x, FALSE);
		      }
		    /* Hack -- tower level decreases depth */
		    else if ((typ == GF_FALL_MORE 
			      && !t_info[p_ptr->dungeon].zone[0].tower)
			     || (typ == GF_FALL_LESS 
				 && t_info[p_ptr->dungeon].zone[0].tower))
		      {
			/* New depth */
			p_ptr->depth++;

			/* Leaving */
			p_ptr->leaving = TRUE;
		      }
		    else
		      {
			/* New depth */
			p_ptr->depth--;

			/* Leaving */
			p_ptr->leaving = TRUE;
		      }

		    /* Clear stairs */
		    p_ptr->create_stair = 0;

		    break;
		  }

		case GF_FALL_SPIKE:
		{
			if ((p_ptr->cur_flags3 & (TR3_FEATHER)) != 0)
			{
				/* Always notice */
				player_can_flags(who, 0x0L,0x0L,TR3_FEATHER,0x0L);

				msg_print("You float gently to the floor of the pit.");
				msg_print("You carefully avoid touching the spikes.");
			}

			else
			{
				/* Always notice */
				player_not_flags(who, 0x0L,0x0L,TR3_FEATHER,0x0L);

				/* Extra spike damage */
				if (rand_int(100) < 50)
				{
					msg_print("You are impaled!");

					dam = dam * 2;
					(void)set_cut(p_ptr->cut + randint(dam));
				}

				/* Take the damage */
				take_hit(dam, killer);
			}
			break;
		}

		case GF_FALL_POIS:
		{
			if ((p_ptr->cur_flags3 & (TR3_FEATHER)) != 0)
			{
				/* Always notice */
				player_can_flags(who, 0x0L,0x0L,TR3_FEATHER,0x0L);

				msg_print("You float gently to the floor of the pit.");
				msg_print("You carefully avoid touching the spikes.");
			}

			else
			{
				/* Always notice */
				player_not_flags(who, 0x0L,0x0L,TR3_FEATHER,0x0L);

				/* Extra spike damage */
				if (rand_int(100) < 50)
				{
					msg_print("You are impaled on poisonous spikes!");

					/* Cut the player */
					(void)set_cut(p_ptr->cut + randint(dam));

					/* Poison the player */
					poison_dam(who, dam, killer, TRUE);
				}

				/* Take the damage */
				take_hit(dam, killer);
			}

			break;
		}

		/* Lava -- stun/confuse/fire */
		case GF_LAVA:
		{
			msg_print ("You are surrounded by lava!");
			if ((p_ptr->cur_flags2 & (TR2_RES_SOUND)) == 0)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_SOUND,0x0L,0x0L);

				(void)set_stun(p_ptr->stun ? p_ptr->stun + randint(10) : randint(40));
			}
			else
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_SOUND,0x0L,0x0L);
			}
			if ((p_ptr->cur_flags2 & (TR2_RES_CONFU)) == 0)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_CONFU,0x0L,0x0L);

				(void)set_confused(p_ptr->confused ? p_ptr->confused + 1 : randint(5) + 5);
			}
			else
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_CONFU,0x0L,0x0L);
			}

			fire_dam(who, dam, killer, TRUE);
			break;
		}


		case GF_BWATER: /* Fire, water damage */
		{
			msg_print("You are scalded by boiling water.");

			if ((p_ptr->cur_flags2 & (TR2_RES_SOUND)) == 0)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_SOUND,0x0L,0x0L);

				(void)set_stun(p_ptr->stun ? p_ptr->stun + randint(10) : randint(40));
			}
			else
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_SOUND,0x0L,0x0L);
			}
			if ((p_ptr->cur_flags2 & (TR2_RES_CONFU)) == 0)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_CONFU,0x0L,0x0L);

				(void)set_confused(p_ptr->confused ? p_ptr->confused + 1 : randint(5) + 5);
			}
			else
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_CONFU,0x0L,0x0L);
			}

			fire_dam(who, dam/3,killer, FALSE);

			if ((p_ptr->cur_flags4 & (TR4_HURT_WATER)) != 0)
			{
				/* Always notice */
				player_can_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_WATER);

				dam *= 2;
			}
			else
			{
				/* Always notice */
				player_not_flags(who, 0x0L,0x0L,0x0L,TR4_HURT_WATER);
			}

			water_dam(who, (dam*2)/3,killer, TRUE);
			break;
		}
		case GF_BMUD: /* Fire, water damage */
		{
			msg_print("You are splashed with boiling mud.");

			if ((p_ptr->cur_flags2 & (TR2_RES_SOUND)) == 0)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_SOUND,0x0L,0x0L);

				(void)set_stun(p_ptr->stun ? p_ptr->stun + randint(10) : randint(40));
			}
			else
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_SOUND,0x0L,0x0L);
			}
			if ((p_ptr->cur_flags2 & (TR2_RES_CONFU)) == 0)
			{
				/* Always notice */
				player_not_flags(who, 0x0L,TR2_RES_CONFU,0x0L,0x0L);

				(void)set_confused(p_ptr->confused ? p_ptr->confused + 1 : randint(5) + 5);
			}
			else
			{
				/* Always notice */
				player_can_flags(who, 0x0L,TR2_RES_CONFU,0x0L,0x0L);
			}

			water_dam(who, (dam*2)/3,killer, FALSE);
			fire_dam(who, dam/3,killer, FALSE);
			break;
		}

		/* Heal the player a percentage of hitpoints */
		case GF_HEAL_PERC:
		{
			if (p_ptr->mhp > 100) dam = p_ptr->mhp * dam / 100;
		}
		
		/* Heal the player, except if undead */
		case GF_HEAL:
		{
			if (p_ptr->cur_flags4 & (TR4_UNDEAD))
			{
				obvious = TRUE;
				take_hit(dam, killer);

				/* Always notice */
				player_can_flags(who, 0x0L,0x0L,0x0L,TR4_UNDEAD);
			}
			else
			{
				obvious = hp_player(dam);

				/* Always notice */
				if (obvious) player_not_flags(who, 0x0L,0x0L,0x0L,TR4_UNDEAD);

				dam = 0;
			}
			break;
		}

		/* Heal the player if undead, else damage */
		case GF_DRAIN_LIFE:
		{
			if (p_ptr->cur_flags4 & (TR4_UNDEAD))
			{
				obvious = hp_player(dam);

				/* Always notice */
				if (obvious) player_can_flags(who, 0x0L,0x0L,0x0L,TR4_UNDEAD);

				dam = 0;
			}
			else
			{
				obvious = TRUE;
				take_hit(dam, killer);

				/* Always notice */
				player_not_flags(who, 0x0L,0x0L,0x0L,TR4_UNDEAD);
			}
			break;
		}

		/* Teleport the player -- use dam as power*/
		case GF_AWAY_JUMP:
		case GF_AWAY_ALL:
		case GF_AWAY_DARK:
		case GF_AWAY_NATURE:
		case GF_AWAY_FIRE:
		{
			/* Teleport restriction fails */
			if ((teleport_hook && !(teleport_hook(y, x, y, x))) || (p_ptr->cur_flags4 & (TR4_ANCHOR)) || (room_has_flag(p_ptr->py, p_ptr->px, ROOM_ANCHOR)))
			{
				/* Source is player */
				if (who <= SOURCE_PLAYER_START)
				{
					msg_print("You fail to teleport away.");					
				}
				else
				{
					/* Fails to teleport */
					msg_format("%^s fails to teleport you away.", killer);
				}

				/* Learn about anchor */
				if (p_ptr->cur_flags4 & (TR4_ANCHOR)) player_can_flags(who, 0x0L, 0x0L, 0x0L, TR4_ANCHOR);
			}
			else
			{
				player_not_flags(who, 0x0L, 0x0L, 0x0L, TR4_ANCHOR);

				/* Mark grid for later processing. */
				cave_temp_mark(y, x, FALSE);
			}

			dam = 0;
			break;
		}

		case GF_STEAM: /* Fire and water damage */
		{
			msg_print("You are scalded by steam.");

			water_dam(who, (dam*2)/3,killer, FALSE);
			fire_dam(who, dam/3,killer, FALSE);
			break;
		}

		case GF_VAPOUR: /* Acid damage */
		{
			msg_print("You are surrounded by acidic vapour.");

			acid_dam(who, dam, killer, FALSE);
			break;
		}

		case GF_SMOKE: /* Acid damage */
		{
			msg_print("You are surrounded by smoke.");

			fire_dam(who, dam, killer, FALSE);
			break;
		}

		case GF_LOSE_MANA:
		{
			int drain;

			/* Take damage */
			take_hit(dam, killer);

			/* Damage (mana) */
			if (p_ptr->csp)
			{
				/* Drain depends on maximum mana */
				drain = 2 + rand_int(p_ptr->msp / 10);

				/* Drain the mana */
				if (drain > p_ptr->csp)
				{
					p_ptr->csp = 0;
					p_ptr->csp_frac = 0;

					msg_print("Your mana is gone!");

					/* Notice no mana */
					update_smart_learn(who, SM_IMM_MANA);
				}
				else
				{
					p_ptr->csp -= drain;
					msg_print("Your mana drains away.");

					/* Monster notices */
					update_smart_forget(who, SM_IMM_MANA);
				}

				/* Update mana */
				p_ptr->update |= (PU_MANA);				

				/* Redraw mana */
				p_ptr->redraw |= (PR_MANA);

				/* Window stuff */
				p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);
			}
			else
			{
				/* Notice no mana */
				update_smart_learn(who, SM_IMM_MANA);
			}
			break;
		}

		/* Gain a percentage of total mana */
		case GF_GAIN_MANA_PERC:
		{
			if (p_ptr->msp > 100) dam = p_ptr->msp * dam / 100;
		}

		/* Melee attack - gain mana */
		case GF_GAIN_MANA:
		{
			/* Monster may have mana */
			if (p_ptr->msp)
			{
				/* Monster notices */
				update_smart_forget(who, SM_IMM_MANA);

				/* Player gains back all mana */
				if (p_ptr->csp + dam >= p_ptr->msp)
				{
					p_ptr->csp = p_ptr->msp;
					p_ptr->csp_frac = 0;

					msg_print("You feel your head clear.");
				}
				/* Player gains back partial mana */
				else
				{
					p_ptr->csp += dam;

					msg_print("You feel your concentration improve.");
				}

				/* Update mana */
				p_ptr->update |= (PU_MANA);				

				p_ptr->redraw |= (PR_MANA);
				p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1 | PW_PLAYER_2 | PW_PLAYER_3);
				obvious = TRUE;
			}
			else
			{
				/* Notice no mana */
				update_smart_learn(who, SM_IMM_MANA);
			}

			dam = 0;
			break;
		}

		case GF_HUNGER:
		{
			int resist = 2;

			obvious = TRUE;

			/* Take damage */
			take_hit(dam, killer);

			/* We're not dead yet */
			if (!p_ptr->is_dead)
			{
				/* Allow resistance */
				if (rand_int(100) < p_ptr->skill_sav) resist++;

				if ((p_ptr->cur_flags3 & (TR3_SLOW_DIGEST)) != 0)
				{
					resist += 2;
					player_can_flags(who, 0x0L,0x0L,TR3_SLOW_DIGEST,0x0L);
					dam /= 3;
				}
				else
				{
					player_not_flags(who, 0x0L,0x0L,TR3_SLOW_DIGEST,0x0L);
				}

				/* Message -- only if appropriate */
				if ((resist > 2) &&
				    (p_ptr->food > PY_FOOD_ALERT))
				{
					msg_print("You resist the effects!");
				}
				else
				{
					msg_print("You feel hungry...");
				}

				/* Reduce food counter, but not too much. */
				set_food(p_ptr->food -
					MIN(500 + p_ptr->food / 5, p_ptr->food / resist));
			}

			break;
		}

		/* Probe the player */
		case GF_PROBE:
		{
			dam = 0;

			if (who > SOURCE_MONSTER_START)
			{
				msg_print("You have been probed for weaknesses!");

				/* Learn all of the player resistances */
				update_smart_cheat(who);

				/* Probe worked */
				obvious = TRUE;
			}

			break;
		}

		/* Remove enchantments from the player */
		case GF_DISPEL:
		{
			dam = 0;

			msg_print("You have been stripped of enchantments!");

			/* Hack -- give player one round to react */
			if (p_ptr->fast) set_fast(1);
			if (p_ptr->slow) set_slow(1);
			if (p_ptr->stastis) set_stastis(1);

			if (p_ptr->protevil) set_protevil(1);
			if (p_ptr->invuln) set_invuln(1);
			if (p_ptr->hero) set_hero(1);
			if (p_ptr->shero) set_shero(1);
			if (p_ptr->shield) set_shield(1);
			if (p_ptr->blessed) set_blessed(1);
			if (p_ptr->tim_invis) set_tim_invis(1);
			if (p_ptr->tim_infra) set_tim_infra(1);

			if (p_ptr->oppose_acid) set_oppose_acid(1);
			if (p_ptr->oppose_elec) set_oppose_elec(1);
			if (p_ptr->oppose_fire) set_oppose_fire(1);
			if (p_ptr->oppose_cold) set_oppose_cold(1);
			if (p_ptr->oppose_pois) set_oppose_pois(1);

			if (p_ptr->word_recall)
			{
				msg_print("A tension leaves the air around you.");
				p_ptr->word_recall = 0;
			}

			/* Dispel worked */
			obvious = TRUE;

			/* Monster forgets about enchantments */
			update_smart_forget(who, (SM_OPP_ACID | SM_OPP_ELEC | SM_OPP_FIRE | SM_OPP_COLD | SM_OPP_POIS | SM_OPP_FEAR));
			
			break;
		}

		/* Dispel undead */
		case GF_DISP_UNDEAD:
		{
			/* Only affect undead */
			if (p_ptr->cur_flags4 & (TR4_UNDEAD))
			{
				/* Notice */
				player_can_flags(who, 0x0L, 0x0L, 0x0L, TR4_UNDEAD);

				/* Obvious */
				obvious = TRUE;

				/* Message */
				msg_print("You are dispelled!");

				/* Take damage */
				take_hit(dam, killer);
			}

			/* Others ignore */
			else
			{
				/* No damage */
				dam = 0;
			}

			break;
		}


		/* Dispel evil */
		case GF_DISP_EVIL:
		{
			/* Only affect undead */
			if (p_ptr->cur_flags4 & (TR4_EVIL))
			{
				/* Notice */
				player_can_flags(who, 0x0L, 0x0L, 0x0L, TR4_EVIL);

				/* Obvious */
				obvious = TRUE;

				/* Message */
				msg_print("You are dispelled!");

				/* Take damage */
				take_hit(dam, killer);
			}

			/* Others ignore */
			else
			{
				/* No damage */
				dam = 0;
			}

			break;
		}

		/* Warp wood -- hack. Damage if an ent. */
		case GF_HURT_WOOD:
		{
			if (p_ptr->pshape != RACE_ENT)
			{
				dam = 0;
			}
			break;
		}

		/* Default */
		default:
		{
			/* No damage */
			dam = 0;

			break;
		}
	}

	/* Diseases? */
	if (old_disease != p_ptr->disease)
	{
		char output[1024];

		disease_desc(output, sizeof(output), old_disease, p_ptr->disease);

		msg_print(output);

		p_ptr->redraw |= (PR_DISEASE);
	}

	/* Disturb */
	disturb(1, 0);


	/* Return "Anything seen?" */
	return (obvious);
}


/*
 * Helper function for "project()" below.
 *
 * Handle movement of monsters and the player.  Handle the alteration of
 * grids that affect damage.  -LM-
 *
 * This function only checks grids marked with the PLAY_TEMP flag.  To
 * help creatures get out of each other's way, this function processes
 * from outside in.
 *
 * This accomplishes three things:  A creature now cannot be damaged/blinked
 * more than once in a single projection, if all teleport functions also
 * clear the PLAY_TEMP flag.  Also, terrain now affects damage taken, and
 * only then gets altered.  Also, any summoned creatures don't get hurt
 * by the magics that gave them birth.
 *
 * XXX XXX -- Hack -- because the PLAY_TEMP flag may be erased by certain
 * updates, we must be careful not to allow any of the teleport functions
 * called by this function to ask for one.  This work well in practice, but
 * is a definite hack.
 *
 * This function assumes that most messages have already been shown.
 */
bool project_t(int who, int what, int y, int x, int dam, int typ)
{
	monster_type *m_ptr = NULL;
	monster_race *r_ptr = NULL;

	char m_name[80];

	int k;

	bool seen = FALSE;
	bool obvious = FALSE;

	bool affect_player = FALSE;
	bool affect_monster = FALSE;

	int do_dist = 0;

	/* Assume no note */
	cptr note = NULL;

	(void)what;
	
	/* Only process marked grids. */
	if (!(play_info[y][x] & (PLAY_TEMP))) return (FALSE);

	/* Hack -- pre-stage teleportation hooks for efficiency */
	switch(typ)
	{
		/* Teleport to darkness to darkness */
		case GF_AWAY_DARK:
		{
			teleport_hook = teleport_darkness_hook;
			break;
		}
		
		/* Teleport to water/living to water/living */
		case GF_AWAY_NATURE:
		{
			teleport_hook = teleport_nature_hook;
			break;
		}
		
		/* Teleport to fire/lava to fire/lava */
		case GF_AWAY_FIRE:
		{
			teleport_hook = teleport_fiery_hook;
			break;
		}
		
		/* Teleport to 'projectable' locations */
		case GF_AWAY_JUMP:
		case GF_GRAVITY:
		case GF_WIND:
		{
			teleport_hook = teleport_project_hook;
			break;
		}
		
		default:
		{
			teleport_hook = NULL;
		}
	}

	/* Clear the cave_temp flag.  (this is paranoid) */
	play_info[y][x] &= ~(PLAY_TEMP);

	if (cave_m_idx[y][x] < 0)
	{
		affect_player = TRUE;
		obvious = TRUE;
	}

	if (cave_m_idx[y][x] > 0)
	{
		affect_monster = TRUE;

		m_ptr = &m_list[cave_m_idx[y][x]];
		r_ptr = &r_info[m_ptr->r_idx];

		/* Sight check. */
		if (m_ptr->ml) seen = TRUE;

		/* Get the monster name (before teleporting) */
		monster_desc(m_name, sizeof(m_name), cave_m_idx[y][x], 0x40);
	}

	/* Hack -- storm can do several things */
	if (typ == GF_STORM)
	{
		switch(rand_int(3))
		{
			case 0: typ = GF_WIND; break;
			case 1: typ = GF_WATER; break;
			case 2: typ = GF_ELEC; break;
		}
	}

	/* Analyze the type */
	switch (typ)
	{
		/* Gravity -- totally random blink */
		case GF_GRAVITY:
		{
			if (affect_player)
			{
				if (((p_ptr->cur_flags2 & (TR2_RES_NEXUS)) || (p_ptr->cur_flags3 & (TR3_FEATHER))) &&
				    (rand_int(3)))
				{
					msg_print("You barely hold your ground.");
				}
				else
				{
					msg_print("Gravity warps around you.");

					/* Throw the player around unsafely. */
					teleport_player(10);
				}
			}

			if (affect_monster)
			{
				/* Damage-variable throw distance */
				do_dist = 4 + dam / 25;

				/* Resist even when affected */
				if (r_ptr->flags4 & (RF4_BRTH_GRAV)) do_dist = 0;
				else if (r_ptr->flags4 & (RF4_BRTH_NEXUS)) do_dist /= 4;
				else if (r_ptr->flags3 & (RF3_RES_NEXUS)) do_dist /= 2;
				else if (r_ptr->flags9 & (RF9_RES_TPORT))
					do_dist = 2 * do_dist / 3;

				/* Big, heavy monsters, metallic monsters and ghosts */
				if ((r_ptr->flags3 & (RF3_HUGE)) || (r_ptr->flags9 & (RF9_IM_BLUNT | RF9_IM_EDGED))) do_dist /= 3;
				else if ((r_ptr->flags3 & (RF3_GIANT)) || (r_ptr->flags9 & (RF9_RES_BLUNT | RF9_RES_EDGED))) do_dist /= 2;

				if (seen) obvious = TRUE;
			}

			break;
		}

		/* Force -- thrust target away from caster */
		case GF_FORCE:
		{
			if (affect_monster)
			{
				/* Damage-variable throw distance */
				do_dist = 4 + dam / 25;

				/* Force breathers are immune */
				if (r_ptr->flags4 & (RF4_BRTH_FORCE)) break;

				/* Big, heavy monsters, metallic monsters and ghosts */
				if ((r_ptr->flags3 & (RF3_HUGE)) || (r_ptr->flags9 & (RF9_IM_BLUNT | RF9_IM_EDGED))) do_dist /= 3;
				else if ((r_ptr->flags3 & (RF3_GIANT)) || (r_ptr->flags9 & (RF9_RES_BLUNT | RF9_RES_EDGED))) do_dist /= 2;
			}

			if ((affect_monster) || (affect_player))
			{
#if 0
				/* Thrust monster or player away. */
				thrust_away(who, y, x, 1 + dam / 15);
#endif
				/* Hack -- get new location */
				if (affect_monster)
				{
					y = m_ptr->fy;
					x = m_ptr->fx;
				}
			}

			break;
		}

		/* Wind can move monsters and the player about */
		case GF_WIND:
		{
			if (affect_player)
			{
				/* Throw distance depends on weight and strength */
				int dist = 20 * dam / p_ptr->wt;
				if (dist > 8) dist = 8;

				/* Feather fall greatly reduces the effect of wind */
				if (p_ptr->cur_flags3 & (TR3_FEATHER)) dist = (dist + 2) / 3;

				/* Messages */
				if (dist >= 6)
					msg_print("The wind grabs you, and whirls you around!");
				else if (dist >= 1)
					msg_print("The wind buffets you about.");

				/* Throw the player around unsafely. */
				if (dist)
				{
					teleport_player(dist);
				}
			}

			if (affect_monster)
			{
				/* Damage-variable throw distance */
				do_dist = 1 + dam / 25;
				if (do_dist > 8) do_dist = 8;

				/* Big, heavy monsters, metallic monsters and ghosts */
				if ((r_ptr->flags3 & (RF3_HUGE)) || (r_ptr->flags9 & (RF9_IM_BLUNT | RF9_IM_EDGED))) do_dist /= 3;
				else if ((r_ptr->flags3 & (RF3_GIANT)) || (r_ptr->flags9 & (RF9_RES_BLUNT | RF9_RES_EDGED))) do_dist /= 2;
			}

			break;
		}

		/* Many terrain types push the player or monster around. */
		case GF_WATER:
		case GF_WATER_WEAK:
		case GF_SALT_WATER:
		case GF_SUFFOCATE:
		{
			/* XXX To do */
			break;
		}

		/* Nexus - movement */
		case GF_NEXUS:
		{
			if ((affect_player) && (who > SOURCE_MONSTER_START))
			{
				if ((p_ptr->cur_flags2 & (TR2_RES_NEXUS)) == 0)
				{
					/* Get caster */
					monster_type *n_ptr = &m_list[who];

					/* Various effects. */
					apply_nexus(n_ptr);
				}
			}

			if (affect_monster)
			{
				/* Damage-variable throw distance */
				do_dist = 4 + dam / 10;

				/* Resist even when affected */
				if      (r_ptr->flags4 & (RF4_BRTH_NEXUS)) do_dist = 0;
				else if (r_ptr->flags3 & (RF3_RES_NEXUS)) do_dist /= 4;
				else if (r_ptr->flags9 & (RF9_RES_TPORT))
					do_dist = 2 * do_dist / 3;
			}
			break;
		}
		
		/* Teleport away - movement */
		case GF_AWAY_JUMP:
		case GF_AWAY_UNDEAD:
		case GF_AWAY_EVIL:
		case GF_AWAY_ALL:
		case GF_AWAY_NATURE:
		case GF_AWAY_DARK:
		case GF_AWAY_FIRE:
		{
			if (affect_player)
			{
				if (dam) teleport_player(dam);
			}

			if (affect_monster)
			{
				/* Obvious */
				if (seen) obvious = TRUE;

				/* Prepare to teleport */
				do_dist = dam;

				/* Resist even when affected */
				if (r_ptr->flags9 & (RF9_RES_TPORT)) do_dist /= 3;
				else if (r_ptr->flags4 & (RF4_BRTH_NEXUS)) do_dist /= 2;
				else if (r_ptr->flags3 & (RF3_RES_NEXUS)) do_dist /= 2;
			}
			break;
		}
	}

	/* Handle teleportation of monster */
	if (do_dist)
	{
		/* Obvious */
		if (seen) obvious = TRUE;

		/* Teleport */
		teleport_away(cave_m_idx[y][x], do_dist);

		/* No movement */
		if ((y == m_ptr->fy) && (x == m_ptr->fx))
		{
			/* No message */
		}
		/* Visible (after teleport) */
		else if (m_ptr->ml)
		{
			/* No message */
		}
		else
		{
			/* Message */
			note = " disappears!";
		}

		/* Hack -- get new location */
		if (affect_monster)
		{
			y = m_ptr->fy;
			x = m_ptr->fx;
		}
	}

	/* Not teleporting: check if player or monster cannot survive on changed terrain and possibly entomb */
	else
	{
		byte blocked = 0x00;

		/* Block grids marked with temp flags as not to hit monster twice */
		for (k=0;k<8;k++)
		{
			if (play_info[ddy_ddd[k]][ddy_ddd[k]] & (PLAY_TEMP)) blocked |= 1 << k;
		}

		/* Check if monster can move and survive in terrain */
		if ((affect_monster) && (place_monster_here(y, x, m_ptr->r_idx) <= MM_FAIL))
		{
			/* Entomb monster */
			entomb(y, x, blocked);
		}
		/* Check if player can move in terrain */
		else if (affect_player && ((f_info[cave_feat[y][x]].flags1 & (FF1_MOVE)) == 0)
			&& ((f_info[cave_feat[y][x]].flags3 & (FF3_EASY_CLIMB)) == 0))
		{
			/* Entomb player */
			entomb(y, x, blocked);			
		}
	}

	if (affect_monster)
	{
		/* Give detailed messages if visible */
		if (note && seen)
		{
			msg_format("%^s%s", m_name, note);
		}

		/* Update the monster */
		(void)update_mon(cave_m_idx[y][x], FALSE);

		/* Update monster recall window */
		if (p_ptr->monster_race_idx == m_ptr->r_idx)
		{
			/* Window stuff */
			p_ptr->window |= (PW_MONSTER);
		}
	}
	
	/* Clear teleport hook */
	teleport_hook = NULL;

	return (obvious);
}



/*
 * Calculate and store the arcs used to make starbursts.
 */
static void calc_starburst(int height, int width, byte *arc_first,
	byte *arc_dist, int *arc_num)
{
	int i;
	int size, dist, vert_factor;
	int degree_first, center_of_arc;


	/* Note the "size" */
	size = 2 + (width + height/ 22);

	/* Ask for a reasonable number of arcs. */
	*arc_num = 8 + (height * width / 80);
	*arc_num = rand_spread(*arc_num, 3);
	if (*arc_num < 8)  *arc_num = 8;
	if (*arc_num > 45) *arc_num = 45;

	/* Determine the start degrees and expansion distance for each arc. */
	for (degree_first = 0, i = 0; i < *arc_num; i++)
	{
		/* Get the first degree for this arc (using 180-degree circles). */
		arc_first[i] = degree_first;

		/* Get a slightly randomized start degree for the next arc. */
		degree_first += (180 / *arc_num);

		/* Do not entirely leave the usual range */
		if (degree_first < 180 * (i+1) / *arc_num)
		    degree_first = 180 * (i+1) / *arc_num;
		if (degree_first > (180 + *arc_num) * (i+1) / *arc_num)
		    degree_first = (180 + *arc_num) * (i+1) / *arc_num;


		/* Get the center of the arc (convert from 180 to 360 circle). */
		center_of_arc = degree_first + arc_first[i];

		/* Get arc distance from the horizontal (0 and 180 degrees) */
		if      (center_of_arc <=  90) vert_factor = center_of_arc;
		else if (center_of_arc >= 270) vert_factor = ABS(center_of_arc - 360);
		else                           vert_factor = ABS(center_of_arc - 180);

		/*
		 * Usual case -- Calculate distance to expand outwards.  Pay more
		 * attention to width near the horizontal, more attention to height
		 * near the vertical.
		 */
		dist = ((height * vert_factor) + (width * (90 - vert_factor))) / 90;

		/* Randomize distance (should never be greater than radius) */
		arc_dist[i] = rand_range(dist / 4, dist / 2);

		/* Keep variability under control (except in special cases). */
		if ((dist != 0) && (i != 0))
		{
			int diff = arc_dist[i] - arc_dist[i-1];

			if (ABS(diff) > size)
			{
				if (diff > 0)
					arc_dist[i] = arc_dist[i-1] + size;
				else
					arc_dist[i] = arc_dist[i-1] - size;
			}
		}
	}

	/* Neaten up final arc of circle by comparing it to the first. */
	if (TRUE)
	{
		int diff = arc_dist[*arc_num - 1] - arc_dist[0];

		if (ABS(diff) > size)
		{
			if (diff > 0)
				arc_dist[*arc_num - 1] = arc_dist[0] + size;
			else
				arc_dist[*arc_num - 1] = arc_dist[0] - size;
		}
	}
}



/*
 * Generic "beam"/"bolt"/"ball" projection routine.
 *
 * Input:
 *   who: Index of "source" monster (negative for "player")
 *   rad: Radius of explosion (0 = beam/bolt, 1 to 9 = ball)
 *   y,x: Target location (or location to travel "towards")
 *   dam: Base damage roll to apply to affected monsters (or player)
 *   typ: Type of damage to apply to monsters (and objects)
 *   flg: Extra bit flags (see PROJECT_xxxx in "defines.h")
 *   degrees: How wide an arc spell is (in degrees).
 *   source_diameter: how wide the source diameter is.
 *
 * Return:
 *   TRUE if any "effects" of the projection were observed, else FALSE
 *
* At present, there are five major types of projections:
 *
 * Point-effect projection:  (no PROJECT_BEAM flag, radius of zero, and either
 *   jumps directly to target or has a single source and target grid)
 * A point-effect projection has no line of projection, and only affects one
 *   grid.  It is used for most area-effect spells (like dispel evil) and
 *   pinpoint strikes.
 *
 * Bolt:  (no PROJECT_BEAM flag, radius of zero, has to travel from source to
 *   target)
 * A bolt travels from source to target and affects only the final grid in its
 *   projection path.  If given the PROJECT_STOP flag, it is stopped by any
 *   monster or character in its path (at present, all bolts use this flag).
 *
 * Beam:  (PROJECT_BEAM)
 * A beam travels from source to target, affecting all grids passed through
 *   with full damage.  It is never stopped by monsters in its path.  Beams
 *   may never be combined with any other projection type.
 *
 * Ball:  (positive radius, unless the PROJECT_ARC flag is set)
 * A ball travels from source towards the target, and always explodes.  Unless
 *   specified, it does not affect wall grids, but otherwise affects any grids
 *   in LOS from the center of the explosion.
 * If used with a direction, a ball will explode on the first occupied grid in
 *   its path.  If given a target, it will explode on that target.  If a
 *   wall is in the way, it will explode against the wall.  If a ball reaches
 *   MAX_RANGE without hitting anything or reaching its target, it will
 *   explode at that point.
 *
 * Arc:  (positive radius, with the PROJECT_ARC flag set)
 * An arc is a portion of a source-centered ball that explodes outwards
 *   towards the target grid.  Like a ball, it affects all non-wall grids in
 *   LOS of the source in the explosion area.  The width of arc spells is con-
 *   trolled by degrees.
 * An arc is created by rejecting all grids that form the endpoints of lines
 *   whose angular difference (in degrees) from the centerline of the arc is
 *   greater than one-half the input "degrees".  See the table "get_
 *   angle_to_grid" in "util.c" for more information.
 * Note:  An arc with a value for degrees of zero is actually a beam of
 *   defined length.
 *
 * Projections that affect all monsters in LOS are handled through the use
 *   of "project_los()", which applies a single-grid projection to individual
 *   monsters.  Projections that light up rooms or affect all monsters on the
 *   level are more efficiently handled through special functions.
 *
 *
 * Variations:
 *
 * PROJECT_STOP forces a path of projection to stop at the first occupied
 *   grid it hits.  This is used with bolts, and also by ball spells
 *   travelling in a specific direction rather than towards a target.
 *
 * PROJECT_THRU allows a path of projection towards a target to continue
 *   past that target.
 *
 * PROJECT_JUMP allows a projection to immediately set the source of the pro-
 *   jection to the target.  This is used for all area effect spells (like
 *   dispel evil), and can also be used for bombardments.
 *
 * PROJECT_WALL allows a projection, not just to affect one layer of any
 *   passable wall (rubble, trees), but to affect the surface of any wall.
 *   Certain projection types always have this flag.
 *
 * PROJECT_PASS allows projections to ignore walls completely.
 *   Certain projection types always have this flag.
 *
 * PROJECT_HIDE erases all graphical effects, making the projection
 *   invisible.
 *
 * PROJECT_GRID allows projections to affect terrain features.
 *
 * PROJECT_ITEM allows projections to affect objects on the ground.
 *
 * PROJECT_KILL allows projections to affect monsters.
 *
 * PROJECT_PLAY allows projections to affect the player.
 *
 * degrees controls the width of arc spells.  With a value for
 *   degrees of zero, arcs act like beams of defined length.
 *
 * source_diameter controls how quickly explosions lose strength with dis-
 *   tance from the target.  Most ball spells have a source diameter of 10,
 *   which means that they do 1/2 damage at range 1, 1/3 damage at range 2,
 *   and so on.   Caster-centered balls usually have a source diameter of 20,
 *   which allows them to do full damage to all adjacent grids.   Arcs have
 *   source diameters ranging up to 20, which allows the spell designer to
 *   fine-tune how quickly a breath loses strength outwards from the breather.
 *   It is expected, but not required, that wide arcs lose strength more
 *   quickly over distance.
 *
 *
 * The player will only get "experience" for monsters killed by himself
 * Unique monsters can only be destroyed by attacks from the player
 *
 * Implementation notes:
 *
 * If the source grid is not the same as the target, we project along the path
 *   between them.  Bolts stop if they hit anything, beams stop if they hit a
 *   wall, and balls and arcs may exhibit either behavior.  When they reach
 *   the final grid in the path, balls and arcs explode.  We do not allow beams
 *   to be combined with explosions.
 * Balls affect all floor grids in LOS (optionally, also wall grids adjacent
 *   to a grid in LOS) within their radius.  Arcs do the same, but only within
 *   their cone of projection.
 * Because affected grids are only scanned once, and it is really helpful to
 *   have explosions that travel outwards from the source, they are sorted by
 *   distance.  For each distance, an adjusted damage is calculated.
 * In successive passes, the code then displays explosion graphics, erases
 *   these graphics, marks terrain for possible later changes, affects
 *   objects, monsters, the character, and finally changes features and
 *   teleports monsters and characters in marked grids.
 *
 *
 * Usage and graphics notes:
 *
 * If the option "fresh_before" is on, or the delay factor is anything other
 * than zero, bolt and explosion pictures will be momentarily shown on screen.
 *
 * Only 256 grids can be affected per projection, limiting the effective
 * radius of standard ball attacks to nine units (diameter nineteen).  Arcs
 * can have larger radii; an arc capable of going out to range 20 should not
 * be wider than 70 degrees.
 *
 * Balls must explode BEFORE hitting walls, or they would affect monsters on
 * both sides of a wall.
 *
 * Note that for consistency, we pretend that the bolt actually takes time
 * to move from point A to point B, even if the player cannot see part of the
 * projection path.  Note that in general, the player will *always* see part
 * of the path, since it either starts at the player or ends on the player.
 *
 * Hack -- we assume that every "projection" is "self-illuminating".
 *
 * Hack -- when only a single monster is affected, we automatically track
 * (and recall) that monster, unless "PROJECT_JUMP" is used.
 *
 * Note that we must call "handle_stuff()" after affecting terrain features
 * in the blast radius, in case the illumination of the grid was changed,
 * and "update_view()" and "update_monsters()" need to be called.
 */
bool project(int who, int what, int rad, int y0, int x0, int y1, int x1, int dam, int typ,
			 u32b flg, int degrees, byte source_diameter)
{
	int i, j, k;
	int dist = 0;

	u32b dam_temp;
	int centerline = 0;

	int y = y0;
	int x = x0;
	int n1y = 0;
	int n1x = 0;
	int y2, x2;

	int msec = op_ptr->delay_factor * op_ptr->delay_factor;

	/* Assume the player sees nothing */
	bool notice = FALSE;

	/* Assume the player has seen nothing */
	bool visual = FALSE;

	/* Assume the player has seen no blast grids */
	bool drawn = FALSE;

	/* Is the player blind? */
	bool blind = (p_ptr->blind ? TRUE : FALSE);

	/* Number of grids in the "path" */
	int path_n = 0;

	/* Actual grids in the "path" */
	u16b path_g[512];

	/* Number of grids in the "blast area" (including the "beam" path) */
	int grids = 0;

	/* Coordinates of the affected grids */
	byte gx[256], gy[256];

	/* Distance to each of the affected grids. */
	byte gd[256];

	/* Precalculated damage values for each distance. */
	int dam_at_dist[MAX_RANGE+1];

	/*
	 * Starburst projections only --
	 * Holds first degree of arc, maximum effect distance in arc.
	 */
	byte arc_first[45];
	byte arc_dist[45];

	/* Number (max 45) of arcs. */
	int arc_num = 0;

	int degree, max_dist;

	/* Hack -- Flush any pending output */
	handle_stuff();

	/* Make certain that the radius is not too large */
	if (rad > MAX_RANGE) rad = MAX_RANGE;

	/* Some projection types always PROJECT_WALL. */
	if ((typ == GF_KILL_WALL) || (typ == GF_KILL_DOOR))
	{
		flg |= (PROJECT_WALL);
	}

	/* Some projection types always PROJECT_LITE */
	if ((typ == GF_FIRE) || (typ == GF_LITE) ||
		(typ == GF_PLASMA) || (typ == GF_LAVA) ||
		(typ == GF_ELEC))
	{
		flg |= (PROJECT_LITE);
	}

	/* Some projection types always obvious */
	/* Include anything that can be felt by raising / lowering temperature */
	if ((typ == GF_STORM) || (typ == GF_WIND) || (typ == GF_COLD) ||
		(typ == GF_ICE) || (typ == GF_FIRE) || (typ == GF_LAVA) ||
		(typ == GF_BMUD) || (typ == GF_BWATER) || (typ == GF_STEAM) ||
		(typ == GF_SMOKE) || (typ == GF_EXPLODE) || (typ == GF_ELEC) ||
		(typ == GF_PLASMA))
	{
		notice = TRUE;
	}

	/* Some projection types always obvious unless the player is blind */
	/* Include anything that emits something physical */
	if ((typ == GF_ACID) || (typ == GF_VAPOUR) || (typ == GF_SHARD) ||
		(typ == GF_WEB) || (typ == GF_BLOOD) || (typ == GF_SLIME) ||
		(typ == GF_WATER) || (typ == GF_SALT_WATER))
	{
		if (!blind) notice = TRUE;
	}

	/* Hack -- Jump to target, but require a valid target */
	if ((flg & (PROJECT_JUMP)) && (y1) && (x1))
	{
		y0 = y1;
		x0 = x1;

		/* Clear the flag */
		flg &= ~(PROJECT_JUMP);
	}

	/* Lite up source grid */
	if (flg & (PROJECT_LITE))
	{
		if (temp_lite(y, x)) notice = TRUE;
	}

	/* If a single grid is both source and destination, store it. */
	if ((x1 == x0) && (y1 == y0))
	{
		gy[grids] = y0;
		gx[grids] = x0;
		gd[grids++] = 0;
	}

	/* Otherwise, unless an arc or a star, travel along the projection path. */
	else if (!(flg & (PROJECT_ARC | PROJECT_STAR)))
	{
		/* Determine maximum length of projection path */
		if (flg & (PROJECT_BOOM | PROJECT_8WAY)) dist = MAX_RANGE;
		else if (rad <= 0)        dist = MAX_RANGE;
		else                      dist = rad;

		/* Calculate the projection path */
		path_n = project_path(path_g, dist, y0, x0, &y1, &x1, flg);

		/* Project along the path */
		for (i = 0; i < path_n; ++i)
		{
			int oy = y;
			int ox = x;

			int ny = GRID_Y(path_g[i]);
			int nx = GRID_X(path_g[i]);

			/* Hack -- Balls explode before reaching walls. */
			if ((flg & (PROJECT_BOOM | PROJECT_8WAY)) && (!cave_project_bold(ny, nx)))
			{
				break;
			}

			/* Advance */
			y = ny;
			x = nx;

			/* Lite up the path if required */
			if (flg & (PROJECT_LITE))
			{
				notice |= temp_lite(y, x);
			}

			/* If a beam, collect all grids in the path. */
			if (flg & (PROJECT_BEAM))
			{
				gy[grids] = y;
				gx[grids] = x;
				gd[grids++] = 0;
			}

			/* Otherwise, collect only the final grid in the path. */
			else if (i == path_n - 1)
			{
				gy[grids] = y;
				gx[grids] = x;
				gd[grids++] = 0;
			}

			/* Only do visuals if requested */
			if (!blind && !(flg & (PROJECT_HIDE)))
			{
				/* Only do visuals if the player can "see" the projection */
				if (player_has_los_bold(y, x))
				{
					u16b p;

					byte a;
					char c;

					/* Obtain the bolt pict */
					p = bolt_pict(oy, ox, y, x, typ);

					/* Extract attr/char */
					a = PICT_A(p);
					c = PICT_C(p);

					/* Display the visual effects */
					print_rel(c, a, y, x);
					move_cursor_relative(y, x);
					if (op_ptr->delay_factor)
					{
					    Term_fresh();
						if (p_ptr->window) window_stuff();
					}

					/* Delay */
					Term_xtra(TERM_XTRA_DELAY, msec);

					/* Erase the visual effects */
					lite_spot(y, x);
					if (op_ptr->delay_factor) Term_fresh();

					/* Re-display the beam  XXX */
					if (flg & (PROJECT_BEAM))
					{
						/* Obtain the explosion pict */
						p = bolt_pict(y, x, y, x, typ);

						/* Extract attr/char */
						a = PICT_A(p);
						c = PICT_C(p);

						/* Visual effects */
						print_rel(c, a, y, x);
					}

					/* Hack -- Activate delay */
					visual = TRUE;
				}

				/* Hack -- Always delay for consistency */
				else if (visual)
				{
					/* Delay for consistency */
					Term_xtra(TERM_XTRA_DELAY, msec);
				}
			}
		}
	}

	/* Save the "blast epicenter" */
	y2 = y;
	x2 = x;

	/* Beams have already stored all the grids they will affect. */
	if (flg & (PROJECT_BEAM))
	{
		/* No special actions */
	}

	/* Handle 8-way projections */
	else if (flg & (PROJECT_8WAY))
	{
		/* Unblocked directions */
		byte dirs = 0xFF;

		/*
		 * If the center of the explosion hasn't been
		 * saved already, save it now.
		 */
		if (grids == 0)
		{
			gy[grids] = y2;
			gx[grids] = x2;
			gd[grids++] = 0;
		}

		/* Scan outwards from centre */
		for (k = 1; (k <= rad) && (dirs); k++)
		{
			/* In each direction */
			for (i = 0; (i < 8) && (dirs & (1 << i)); i++)
			{
				int y = y2 + ddy_ddd[i] * k;
				int x = x2 + ddx_ddd[i] * k;

				/* This is a non-projectable grid */
				if (!cave_project_bold(y, x))
				{
					/* Spell with PROJECT_PASS ignore these grids */
					if (!(flg & (PROJECT_PASS)))
					{
						/* Remove from available directions */
						dirs &= ~(1 << i);
					}
				}

				/* PROJECT_WALL or PROJECT_PASS is active or terrain is passable */
				if ((flg & (PROJECT_WALL | PROJECT_PASS)) || cave_project_bold(y, x))
				{
					gy[grids] = y;
					gx[grids] = x;
					gd[grids++] = k;
				}
			}
		}
	}

	/* Handle explosions */
	else if (flg & (PROJECT_BOOM))
	{

		/* Pre-calculate some things for starbursts. */
		if (flg & (PROJECT_STAR))
		{
			calc_starburst(1 + rad * 2, 1 + rad * 2, arc_first, arc_dist,
				&arc_num);

			/* Mark the area nearby -- limit range, ignore rooms */
			spread_cave_temp(y0, x0, rad, FALSE);

		}

		/* Pre-calculate some things for arcs. */
		if (flg & (PROJECT_ARC))
		{
			/* The radius of arcs cannot be more than 20 */
			if (rad > 20) rad = 20;

			/* Reorient the grid forming the end of the arc's centerline. */
			n1y = y1 - y0 + 20;
			n1x = x1 - x0 + 20;

			/* Correct overly large or small values */
			if (n1y > 40) n1y = 40;
			if (n1x > 40) n1x = 40;
			if (n1y <  0) n1y =  0;
			if (n1x <  0) n1x =  0;

			/* Get the angle of the arc's centerline */
			centerline = 90 - get_angle_to_grid[n1y][n1x];
		}

		/*
		 * If the center of the explosion hasn't been
		 * saved already, save it now.
		 */
		if (grids == 0)
		{
			gy[grids] = y2;
			gx[grids] = x2;
			gd[grids++] = 0;
		}

		/*
		 * Scan every grid that might possibly
		 * be in the blast radius.
		 */
		for (y = y2 - rad; y <= y2 + rad; y++)
		{
			for (x = x2 - rad; x <= x2 + rad; x++)
			{
				/* Center grid has already been stored. */
				if ((y == y2) && (x == x2)) continue;

				/* Precaution: Stay within area limit. */
				if (grids >= 255) break;

				/* Ignore "illegal" locations */
				if (!in_bounds(y, x)) continue;

				/* This is a non-projectable grid */
				if (!cave_project_bold(y, x))
				{
					/* Spell with PROJECT_PASS ignore these grids */
					if (!(flg & (PROJECT_PASS)))
					{
						/* PROJECT_WALL is active or terrain is passable */
						if ((flg & (PROJECT_WALL)) || cave_project_bold(y, x))
						{
							/* Allow grids next to grids in LOS of explosion center */
							for (i = 0, k = 0; i < 8; i++)
							{
								int yy = y + ddy_ddd[i];
								int xx = x + ddx_ddd[i];

								/* Stay within dungeon */
								if (!in_bounds(yy, xx)) continue;

								if (generic_los(y2, x2, yy, xx, CAVE_XLOF))
								{
									k++;
									break;
								}
							}

							/* Require at least one adjacent grid in LOS */
							if (!k) continue;
						}

						/* We can't affect this non-passable wall */
						else continue;
					}
				}

				/* Must be within maximum distance. */
				dist = (distance(y2, x2, y, x));
				if (dist > rad) continue;


				/* Projection is a starburst */
				if (flg & (PROJECT_STAR))
				{
					/* Grid is within effect range */
					if (play_info[y][x] & (PLAY_TEMP))
					{
						/* Reorient current grid for table access. */
						int ny = y - y2 + 20;
						int nx = x - x2 + 20;

						/* Illegal table access is bad. */
						if ((ny < 0) || (ny > 40) || (nx < 0) || (nx > 40))
							continue;

						/* Get angle to current grid. */
						degree = get_angle_to_grid[ny][nx];

						/* Scan arcs to find the one that applies here. */
						for (i = arc_num - 1; i >= 0; i--)
						{
							if (arc_first[i] <= degree)
							{
								max_dist = arc_dist[i];

								/* Must be within effect range. */
								if (max_dist >= dist)
								{
									gy[grids] = y;
									gx[grids] = x;
									gd[grids] = 0;
									grids++;
								}

								/* Arc found.  End search */
								break;
							}
						}
					}
				}

				/* Use angle comparison to delineate an arc. */
				else if (flg & (PROJECT_ARC))
				{
					int n2y, n2x, tmp, diff;

					/* Reorient current grid for table access. */
					n2y = y - y2 + 20;
					n2x = x - x2 + 20;

					/*
					 * Find the angular difference (/2) between
					 * the lines to the end of the arc's center-
					 * line and to the current grid.
					 */
					tmp = ABS(get_angle_to_grid[n2y][n2x] + centerline) % 180;
					diff = ABS(90 - tmp);

					/*
					 * If difference is not greater then that
					 * allowed, and the grid is in LOS, accept it.
					 */
					if (diff < (degrees + 6) / 4)
					{
						if (generic_los(y2, x2, y, x, CAVE_XLOF))
						{
							gy[grids] = y;
							gx[grids] = x;
							gd[grids] = dist;
							grids++;
						}
					}
				}

				/* Standard ball spell -- accept all grids in LOS. */
				else
				{
					if (flg & (PROJECT_PASS) || generic_los(y2, x2, y, x, CAVE_XLOF))
					{
						gy[grids] = y;
						gx[grids] = x;
						gd[grids] = dist;
						grids++;
					}
				}
			}
		}
	}

	/* Clear the "temp" array  XXX */
	clear_temp_array();

	/* Calculate and store the actual damage at each distance. */
	for (i = 0; i <= MAX_RANGE; i++)
	{
		/* Paranoia -- feature damage does not vary */
		if (typ == GF_FEATURE) dam_temp = dam;

		/* Damage does not vary with range */
		else if (flg & (PROJECT_AREA)) dam_temp = dam;

		/* No damage outside the radius. */
		else if (i > rad) dam_temp = 0;

		/* Standard damage calc. for 10' source diameters, or at origin. */
		else if ((!source_diameter) || (i == 0))
		{
			dam_temp = (dam + i) / (i + 1);
		}

		/* If a particular diameter for the source of the explosion's
		 * energy is given, calculate an adjusted damage.
		 */
		else
		{
			dam_temp = (source_diameter * dam) / ((i + 1) * 10);
			if (dam_temp > (u32b)dam) dam_temp = dam;
		}

		/* Store it. */
		dam_at_dist[i] = dam_temp;
	}

	/* Hack -- prevent arcs and starbursts from hurting the player if they are the source */
	if ( (who < SOURCE_PLAYER_TRAP) && ((flg & (PROJECT_ARC | PROJECT_STAR)) != 0))
	{
		for (i = 0; i < grids; i++)
		{
			if (gd[i] == 0)
			{
				gy[i] = gy[grids-1];
				gx[i] = gx[grids-1];
				gd[i] = gd[grids-1];
				grids--;
			}
		}
	}
	
	/* Sort the blast grids by distance, starting at the origin. */
	for (i = 0, k = 0; i < rad; i++)
	{
		int tmp_y, tmp_x, tmp_d;

		/* Collect all the grids of a given distance together. */
		for (j = k; j < grids; j++)
		{
			if (gd[j] == i)
			{
				tmp_y = gy[k];
				tmp_x = gx[k];
				tmp_d = gd[k];

				gy[k] = gy[j];
				gx[k] = gx[j];
				gd[k] = gd[j];

				gy[j] = tmp_y;
				gx[j] = tmp_x;
				gd[j] = tmp_d;

				/* Write to next slot */
				k++;
			}
		}
	}


	/* Display the "blast area" if allowed */
	if (!blind && !(flg & (PROJECT_HIDE)))
	{
		/* Do the blast from inside out */
		for (i = 0; i < grids; i++)
		{
			/* Extract the location */
			y = gy[i];
			x = gx[i];

			/* Only do visuals if the player can "see" the blast */
			if (player_has_los_bold(y, x))
			{
				u16b p;

				byte a;
				char c;

				drawn = TRUE;

				/* Obtain the explosion pict */
				p = bolt_pict(y, x, y, x, typ);

				/* Extract attr/char */
				a = PICT_A(p);
				c = PICT_C(p);

				/* Visual effects -- Display */
				print_rel(c, a, y, x);
			}

			/* Hack -- center the cursor */
			move_cursor_relative(y2, x2);

			/* New radius is about to be drawn */
			if ((i == grids - 1) || ((i < grids - 1) && (gd[i + 1] > gd[i])))
			{
				/* Flush each radius separately */
				if (op_ptr->delay_factor)
				{
				    Term_fresh();
					if (p_ptr->window) window_stuff();
				}

				/* Flush */
				if (p_ptr->window) window_stuff();

				/* Delay (efficiently) */
				if (visual || drawn)
				{
					Term_xtra(TERM_XTRA_DELAY, msec);
				}
			}
		}

		/* Delay for a while if there are pretty graphics to show */
		if ((grids > 1) && (visual || drawn))
		{
			if (!op_ptr->delay_factor) Term_fresh();
			Term_xtra(TERM_XTRA_DELAY, 50 + msec);
		}

		/* Flush the erasing -- except if we specify lingering graphics */
		if ((drawn) && (!(flg & (PROJECT_NO_REDRAW))))
		{
			/* Erase the explosion drawn above */
			for (i = 0; i < grids; i++)
			{
				/* Extract the location */
				y = gy[i];
				x = gx[i];

				/* Hack -- Erase if needed */
				if (player_has_los_bold(y, x))
				{
					lite_spot(y, x);
				}
			}

			/* Hack -- center the cursor */
			move_cursor_relative(y2, x2);

			/* Flush the explosion */
			if (op_ptr->delay_factor)
			{
			    Term_fresh();
				if (p_ptr->window) window_stuff();
			}
		}
	}

	/* Temporarily lite up grids */
	if (flg & (PROJECT_LITE))
	{
		/* Scan grids */
		for (i = 0; i < grids; i++)
		{
			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Temporarily lite up the grid */
			if (temp_lite(y, x)) notice = TRUE;
		}	
	}

	/* Check features */
	if (flg & (PROJECT_GRID))
	{

		/* Scan for features */
		for (i = 0; i < grids; i++)
		{
			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Affect the feature in that grid */
			if (project_f(who, what, y, x, dam_at_dist[gd[i]], typ))
				notice = TRUE;
		}
	}

	/* Check objects */
	if (flg & (PROJECT_ITEM))
	{

		/* Scan for objects */
		for (i = 0; i < grids; i++)
		{

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Affect the object in the grid */
			if (project_o(who, what, y, x, dam_at_dist[gd[i]], typ)) notice = TRUE;
		}
	}


	/* Check monsters */
	if (flg & (PROJECT_KILL))
	{
		/* Mega-Hack */
		project_m_n = 0;
		project_m_x = 0;
		project_m_y = 0;

		/* Scan for monsters */
		for (i = 0; i < grids; i++)
		{
			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Check for monster */
			if (cave_m_idx[y][x] > 0)

			{
			 	monster_type *m_ptr = &m_list[cave_m_idx[y][x]];

				/* Hack -- handle resist magic. Deeper monsters are more resistant to spells.
				 * All monsters are more resistant to devices and spells from each other. */
				if ((flg & (PROJECT_MAGIC)) && (cave_m_idx[y][x] > 0)
					&& ((r_info[m_list[cave_m_idx[y][x]].r_idx].flags9 & (RF9_RES_MAGIC)) != 0)
					&& (rand_int(100) < 20 + p_ptr->depth / (who != SOURCE_PLAYER_CAST ? 1 : 2)))
				{
					/* Hack -- resist magic */
					if (project_m(who, what, y, x, 0, GF_RES_MAGIC)) notice = TRUE;
				}
				/* Check if monster evades */
				/* Area-effect and jumping spells cannot be dodged */
				else if (!(flg & (PROJECT_ARC | PROJECT_STAR | PROJECT_JUMP |
			             PROJECT_BOOM)) && (cave_m_idx[y][x] > 0) && 
					(mon_evade(cave_m_idx[y][x],((m_ptr->confused || m_ptr->stunned) ? 1 : 3) + gd[i], 5 + gd[i], who <= SOURCE_PLAYER_START ? " your magic" : ""))) continue;

				/* Affect the monster in the grid */
				else if (project_m(who, what, y, x, dam_at_dist[gd[i]], typ))	notice = TRUE;
			}
		}

		/* Player affected one monster (without "jumping") */
		if ((who <= SOURCE_PLAYER_START) && (project_m_n == 1) && !(flg & (PROJECT_JUMP)))
		{
			/* Location */
			x = project_m_x;
			y = project_m_y;

			/* Track if possible */
			if (cave_m_idx[y][x] > SOURCE_MONSTER_START)
			{
				monster_type *m_ptr = &m_list[cave_m_idx[y][x]];

				/* Hack -- auto-recall */
				if (m_ptr->ml) monster_race_track(m_ptr->r_idx);

				/* Hack - auto-track */
				if (m_ptr->ml) health_track(cave_m_idx[y][x]);
			}
		}
	}

	/* Check player */
	if (flg & (PROJECT_PLAY))
	{

		/* Scan for player */
		for (i = 0; i < grids; i++)
		{

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Player is in this grid */
			if (cave_m_idx[y][x] < 0)
			{

				/* Affect the player */
				if (project_p(who, what, y, x, dam_at_dist[gd[i]], typ))
				{
					notice = TRUE;

					/* Only affect the player once */
					break;
				}

			}
		}
	}

	/* Teleport monsters and player around, alter certain features. */
	for (i = 0; i < grids; i++)
	{
		/* Get the grid location */
		y = gy[i];
		x = gx[i];

		/* Grid must be marked. */
		if (!(play_info[y][x] & (PLAY_TEMP))) continue;

		/* Affect marked grid */
		if (project_t(who, what, y, x, dam_at_dist[gd[i]], typ)) notice = TRUE;
	}

	/* Clear the "temp" array  (paranoia is good) */
	clear_temp_array();

	/* Finally break items on floor.  This has to be done here and carefully to avoid infinite recursion. */
	for (i = 0; i < z_info->o_max; i++)
	{
		object_type object_type_body;
		object_type *i_ptr = &object_type_body;
		object_type *o_ptr = &o_list[i];

		/* Only break objects marked for breakage */
		if (!(o_ptr->ident & (IDENT_BREAKS))) continue;

		/* Get a copy of the object */
		object_copy(i_ptr, o_ptr);

		/* Delete the object */
		delete_object_idx(i);

		/* Break the copy of the object. Note that this recursively calls project routines which is why
                   we have to be so careful. */
		break_near(i_ptr, i_ptr->iy, i_ptr->ix);

		/* Redraw */
		lite_spot(i_ptr->iy, i_ptr->ix);
	}

	/* Update stuff if needed */
	if (p_ptr->update) update_stuff();

	/* Return "something was noticed" */
	return (notice);
}
