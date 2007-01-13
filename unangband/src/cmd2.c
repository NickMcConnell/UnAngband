/* File: cmd2.c */

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
 * Check if action permissable here.
 */

bool do_cmd_test(int y, int x, int action)
{
	u32b bitzero = 0x01;
	u32b flag;

	cptr act;

	cptr here = (((p_ptr->px == x ) && (p_ptr->py == y))?"here":"there");

	feature_type *f_ptr;
	int feat;

	if ((verify_safe) && (play_info[p_ptr->py][p_ptr->px] & (PLAY_SAFE)) && !(play_info[y][x] & (PLAY_SAFE)))
	{
		disturb(1,0);
		msg_print("This doesn't feel safe.");

		if (!get_check("Are you sure?")) return (FALSE);

		/* Hack -- mark the target safe */
		play_info[y][x] |= (PLAY_SAFE);
	}

	/* Must have knowledge */
	if (!(play_info[y][x] & (PLAY_MARK)))
	{
		/* Message */
		msg_format("You see nothing %s.",here);

		/* Nope */
		return (FALSE);
	}

	/* Get memorised feature */
	feat = f_info[cave_feat[y][x]].mimic;

	f_ptr = &f_info[feat];

	act=NULL;

	switch (action)
	{
		case FS_SECRET: break;
		case FS_OPEN: act=" to open"; break;    
		case FS_CLOSE: act=" to close"; break;
		case FS_BASH: act=" to bash"; break;
		case FS_DISARM: act=" to disarm"; break;
		case FS_SPIKE: act=" to spike"; break;
		case FS_ENTER: act=" to enter"; break;
		case FS_TUNNEL: act=" to tunnel"; break;
		case FS_LESS: act=" to climb up"; break;
		case FS_MORE: act=" to climb down"; break;
		case FS_RUN: act=" to run on"; break;
		case FS_KILL_MOVE: act=" to disturb"; break;
		case FS_FLOOR: act=" to set a trap on"; break;
		default: break;
	}


	if (action < FS_FLAGS2)
	{
		flag = bitzero << (action - FS_FLAGS1);
		if (!(f_ptr->flags1 & flag))
		{
		 msg_format("You see nothing %s%s.",here,act);
		 return (FALSE);
		}
	}

	else if (action < FS_FLAGS3)
	{       
		flag = bitzero << (action - FS_FLAGS2);
		if (!(f_ptr->flags2 & flag))
		{
		 msg_format("You see nothing %s%s.",here,act);
		 return (FALSE);
		}
	}
	
	else if (action < FS_FLAGS_END)
	{       
		flag = bitzero << (action - FS_FLAGS3);
		if (!(f_ptr->flags3 & flag))
		{
		 msg_format("You see nothing %s%s.",here,act);
		 return (FALSE);
		}
	}

	return (TRUE);  

}

/*
 * Check quests caused by travelling from the current level to another destination.
 *
 * XXX This includes travelling to the 'final' location and completely the first quest.
 *
 * Confirm indicates if the player is allowed to cancel this.
 *
 */
static bool check_travel_quest(int dungeon, int level, bool confirm)
{
	int i;
	quest_type *q_ptr;
	quest_event *qe_ptr;

	bool cancel = FALSE;

	/* Check quests for cancellation */
	if (confirm) for (i = 0; i < MAX_Q_IDX; i++)
	{
		q_ptr = &q_list[i];
		qe_ptr = &(q_ptr->event[q_ptr->stage]);

		/* if (q_ptr->stage == QUEST_ACTION) qe_ptr = &(q_ptr->event[QUEST_ACTIVE]); */

		/* Check destination */
		if (q_ptr->stage == QUEST_ACTIVE)
		{
			if ((qe_ptr->dungeon != dungeon) ||
				(qe_ptr->level != level - min_depth(dungeon))) continue;

			/* Allow cancellation */
			if (q_ptr->event[QUEST_FAILED].flags & (EVENT_TRAVEL))
			{
				/* Permit cancellation */
				cancel = TRUE;
			}

			continue;
		}

		if (q_ptr->stage != QUEST_ACTION) continue;

		if ((qe_ptr->dungeon != p_ptr->dungeon) ||
			(qe_ptr->level != p_ptr->depth - min_depth(p_ptr->dungeon))) continue;

		/* Fail quest because we left the level */
		if (q_ptr->event[QUEST_FAILED].flags & (EVENT_LEAVE))
		{
			/* Permit cancellation */
			cancel = TRUE;
		}
	}

	/* Permit cancellation */
	if ((cancel) && (get_check("Really fail the quest? "))) return (FALSE);

	/* Check quests for completion */
	for (i = 0; i < MAX_Q_IDX; i++)
	{
		q_ptr = &q_list[i];
		qe_ptr = &(q_ptr->event[q_ptr->stage]);

		if (q_ptr->stage == QUEST_ACTION) qe_ptr = &(q_ptr->event[QUEST_ACTIVE]);

		/* Check quest allocation */
		if (q_ptr->stage == QUEST_ASSIGN)
		{
			/* Allocate quest travelling to location */
			if ((qe_ptr->flags & (EVENT_TRAVEL)) && 
				(qe_ptr->dungeon == dungeon) &&
				(qe_ptr->level == level - min_depth(dungeon)))
			{
				/* Wipe the structure */
				(void)WIPE(qe_ptr, quest_event);

				qe_ptr->dungeon = dungeon;
				qe_ptr->level = level - min_depth(dungeon);
				qe_ptr->flags |= (EVENT_TRAVEL);

				quest_assign(i);
			}

			/* Allocate quest leaving location */
			else if ((qe_ptr->flags & (EVENT_LEAVE)) && 
				(qe_ptr->dungeon == p_ptr->dungeon) &&
				(qe_ptr->level == p_ptr->depth - min_depth(p_ptr->dungeon)))
			{
				/* Wipe the structure */
				(void)WIPE(qe_ptr, quest_event);

				qe_ptr->dungeon = dungeon;
				qe_ptr->level = level - min_depth(dungeon);
				qe_ptr->flags |= (EVENT_LEAVE);

				quest_assign(i);
			}
		}

		if ((qe_ptr->dungeon != dungeon) ||
			(qe_ptr->level != level - min_depth(dungeon))) continue;

		/* Check destination */
		if (q_ptr->stage == QUEST_ACTIVE)
		{
			/* Update actions */
			qe_ptr = &(q_ptr->event[QUEST_ACTION]);

			/* Fail quest because we travelled to the level */
			if (q_ptr->event[QUEST_FAILED].flags & (EVENT_TRAVEL))
			{
				/* Wipe the structure */
				(void)WIPE(qe_ptr, quest_event);

				qe_ptr->dungeon = dungeon;
				qe_ptr->level = level - min_depth(dungeon);
				qe_ptr->flags |= (EVENT_TRAVEL);

				/* Set quest penalty immedately */
				q_ptr->stage = QUEST_PENALTY;
			}

			/* Get closer to success because we travelled to level */
			else if (q_ptr->event[QUEST_ACTIVE].flags & (EVENT_TRAVEL))
			{
				qe_ptr->dungeon = dungeon;
				qe_ptr->level = level - min_depth(dungeon);
				qe_ptr->flags |= (EVENT_TRAVEL);

				/* Have completed quest? */
				if (qe_ptr->flags == q_ptr->event[QUEST_ACTIVE].flags)
				{
					msg_print("Congratulations. You have succeeded at your quest.");

					/* XXX - Tell player next step */
					q_ptr->stage = QUEST_REWARD;
				}
			}

			continue;
		}

		if (q_ptr->stage != QUEST_ACTION) continue;

		/* Update actions */
		qe_ptr = &(q_ptr->event[QUEST_ACTION]);

		/* Fail quest because we left the level */
		if (q_ptr->event[QUEST_FAILED].flags & (EVENT_LEAVE))
		{
			/* Wipe the structure */
			(void)WIPE(qe_ptr, quest_event);

			qe_ptr->dungeon = p_ptr->dungeon;
			qe_ptr->level = p_ptr->depth - min_depth(p_ptr->dungeon);
			qe_ptr->flags |= (EVENT_LEAVE);

			/* Set quest penalty immediately */
			q_ptr->stage = QUEST_PENALTY;
		}

		/* Get closer to success because we need to leave level */
		else if (q_ptr->event[QUEST_ACTIVE].flags & (EVENT_LEAVE))
		{
			qe_ptr->dungeon = p_ptr->dungeon;
			qe_ptr->level = p_ptr->depth - min_depth(p_ptr->dungeon);
			qe_ptr->flags |= (EVENT_LEAVE);

			/* Have completed quest? */
			if (qe_ptr->flags == qe_ptr->flags)
			{
				msg_print("Congratulations. You have succeeded at your quest.");

				/* XXX - Tell player next step */
				q_ptr->stage = QUEST_REWARD;
			}
		}
	}

	/* Mega-hack */
	if ((adult_campaign) && (p_ptr->dungeon == z_info->t_max -1))
	{

		p_ptr->total_winner = TRUE;

		/* Redraw the "title" */
		p_ptr->redraw |= (PR_TITLE);

		/* Congratulations */
		msg_print("*** CONGRATULATIONS ***");
		msg_print("You have won the game!");
		msg_print("You may retire (commit suicide) when you are ready.");
	}

	return (TRUE);
}



/*
 * Print a list of routes (for travelling).
 */
void print_routes(const s16b *route, int num, int y, int x)
{
	int i, town;

	cptr distance;

	char out_val[160];

	byte line_attr;

	town_type *t_ptr = &t_info[p_ptr->dungeon];
	dungeon_zone *zone = &t_ptr->zone[0];

	/* Title the list */
	prt("", y, x);
	put_str("Location", y, x + 5);
	put_str(" Distance", y, x + 35);
	put_str(" Level", y, x + 45);

	/* Dump the routes */
	for (i = 0; i < num; i++)
	{
		line_attr = TERM_WHITE;

		/* Get the town index */
		town = route[i];

		/* Skip inaccessible towns */
		if (town < 0)
		{
			t_ptr = &t_info[(-1) - town];

			line_attr = TERM_SLATE;

			sprintf(out_val, "  %c) %-30s near to %-16s",
				I2A(i), t_name + t_ptr->name, t_name + t_info[t_ptr->nearby].name);
			c_prt(line_attr, out_val, y + i + 1, x);

			continue;
		}

		/* Get the distance */
		if (t_ptr->nearby == p_ptr->dungeon)
		{
			distance = "nearby";
		}
		else
		{
			distance = "distant";
		}

		/* Get the destination info */
		t_ptr = &t_info[town];

		/* Get the top of the dungeon */
		zone = &(t_ptr->zone[0]);

		/* Dump the spell --(-- */
		sprintf(out_val, "  %c) %-30s %-10s%2d%3s ",
			I2A(i), t_name + t_ptr->name,distance,zone->level,max_depth(town) > min_depth(town) ? format("-%-2d",max_depth(town)) : "");
		c_prt(line_attr, out_val, y + i + 1, x);
	}


	/* Clear the bottom line */
	prt("", y + i + 1, x);
}


/*
 * Set routes
 *
 * Set up the possible routes from this location.
 *
 * Returns number of routes set up.
 */
int set_routes(s16b *routes, int max_num, int from)
{
	town_type *t_ptr = &t_info[p_ptr->dungeon];
	dungeon_zone *zone1 = &t_ptr->zone[0];
	dungeon_zone *zone2 = &t_ptr->zone[0];

	int i, ii, num = 0;

	/* Get the top of the dungeon */
	get_zone(&zone1,from,min_depth(from));

	/* Get the bottom of the dungeon */
	get_zone(&zone2,from,max_depth(from));

	/* Add nearby route */
	if (t_ptr->nearby != from) routes[num++] = t_ptr->nearby;

	/* Add far route if possible */
	if (t_ptr->distant != from)
	{
		if (!(zone2->guard) || (!r_info[zone2->guard].max_num))
		{
			routes[num++] = t_ptr->distant;
		}
	}

	/* Add maps */
	for (i = 0; i < INVEN_WIELD; i++)
	{
		/* Skip non-objects */
		if (!inventory[i].k_idx) continue;

		/* Check for maps */
		if (inventory[i].tval == TV_MAP)
		{
			if (t_info[inventory[i].sval].nearby == from)
			{
				routes[num++] = inventory[i].sval;
			}
			else
			{
				routes[num++] = -inventory[i].sval - 1;
			}
		}

		/* Check for bags for maps */
		else if (inventory[i].tval == TV_BAG)
		{
			/* Scan the bag */
			for (ii = 0; ii < INVEN_BAG_TOTAL; ii++)
			{
				/* Slot holds a map */
				if ((bag_holds[inventory[i].sval][ii][0] == TV_MAP) && (bag_contents[inventory[i].sval][ii]))
				{
					int sval = bag_holds[inventory[i].sval][ii][1];

					if (t_info[sval].nearby == from)
					{
						routes[num++] = sval;
					}
					else
					{
						routes[num++] = -sval - 1;
					}
				}
			}
		}
	}

	/* Add additional locations from any of the above */
	for (i = 0; (i < num) && (num < max_num); i++)
	{
		bool add_nearby =  (t_info[routes[i]].nearby != from);
		bool add_distant =  (t_info[routes[i]].distant != from) && (t_info[routes[i]].distant != t_info[routes[i]].nearby);

		/* Get the bottom of the dungeon */
		get_zone(&zone2, routes[i], max_depth(routes[i]));

		/* Can't travel to distant location */
		if ((zone2->guard) && (r_info[zone2->guard].max_num)) add_distant = FALSE;

		for (ii = 0; (ii < num) && (add_nearby || add_distant); ii++)
		{
			if (t_info[routes[i]].nearby == routes[ii]) add_nearby = FALSE;
			if (t_info[routes[i]].distant == routes[ii]) add_distant = FALSE;
		}

		if (add_nearby)
		{
			routes[num++] = t_info[routes[i]].nearby;
		}

		if (add_distant)
		{
			routes[num++] = t_info[routes[i]].distant;
		}
	}

	/* Return number of routes */
	return(num);
}

/*
 * Travel to a different dungeon.
 *
 * This whole thing is a hack -- I haven't decided how elegant it is yet.
 */
static void do_cmd_travel(void)
{
	town_type *t_ptr = &t_info[p_ptr->dungeon];
	dungeon_zone *zone = &t_ptr->zone[0];

	int i, num = 0;

	int journey = 0;

	int by = p_ptr->py / BLOCK_HGT;
	int bx = p_ptr->px / BLOCK_WID;

	bool edge_y = ((by < 2) || (by > ((DUNGEON_HGT/BLOCK_HGT)-3)));
	bool edge_x = ((bx < 2) || (bx > ((DUNGEON_WID/BLOCK_WID)-3)));

	/* Get the top of the dungeon */
	get_zone(&zone,p_ptr->dungeon,min_depth(p_ptr->dungeon));

	if (p_ptr->depth == min_depth(p_ptr->dungeon))
	{
		/* Need to be full to travel for trip */
		if (p_ptr->food < PY_FOOD_FULL)
		{
			msg_print("You'll need a full stomach for the road ahead.");
		}
		else if (p_ptr->blind)
		{
			msg_print("You can't read any maps.");
		}
		else if (p_ptr->confused)
		{
			msg_print("You are too confused.");
		}
		else if (p_ptr->petrify)
		{
			msg_print("You are petrified.");
		}
		else if (p_ptr->afraid)
		{
			msg_print("You are too afraid.");
		}
		else if (p_ptr->image)
		{
			msg_print("The pink mice don't want you to leave.");
		}
		else if ((p_ptr->poisoned) || (p_ptr->cut) || (p_ptr->stun))
		{
			msg_print("You need to recover from any poison, cuts or stun damage.");
		}
		else if (!edge_y && !edge_x && zone->fill)
		{
			msg_format("You need to be close to the edge of %s.",t_name + t_ptr->name);
		}
		else
		{
			int selection = p_ptr->dungeon;

			if (adult_campaign)
			{
				s16b routes[24];
				char out_val[160];

				bool flag, redraw;
				key_event ke;

				/* Routes */
				num = set_routes(routes, 24, p_ptr->dungeon);

				/* Build a prompt (accept all spells) */
				strnfmt(out_val, 78, "(Travel %c-%c, *=List, ESC=exit) Travel where? ",
				I2A(0), I2A(num - 1) );

				/* Nothing chosen yet */
				flag = FALSE;

				/* No redraw yet */
				redraw = FALSE;

				/* Show the list */
				if (auto_display_lists)
				{
					/* Show list */
					redraw = TRUE;

					/* Save screen */
					screen_save();

					/* Display a list of spells */
					print_routes(routes, num, 1, 20);
				}

				/* Get a spell from the user */
				while (!flag && get_com_ex(out_val, &ke))
				{
					char choice;

					if (ke.key == '\xff')
					{
						if (ke.mousebutton)
						{
							if (redraw) ke.key = 'a' + ke.mousey - 2;
							else ke.key = ' ';
						}
						else continue;
					}

					/* Request redraw */
					if ((ke.key == ' ') || (ke.key == '*') || (ke.key == '?'))
					{
						/* Hide the list */
						if (redraw)
						{
							/* Load screen */
							screen_load();

							/* Hide list */
							redraw = FALSE;
						}

						/* Show the list */
						else
						{
							/* Show list */
							redraw = TRUE;

							/* Save screen */
							screen_save();

							/* Display a list of spells */
							print_routes(routes, num, 1, 20);
						}

						/* Ask again */
						continue;

					}

					/* Lowercase 1+ */
					choice = tolower(ke.key);

					/* Extract request */
					i = (islower(choice) ? A2I(choice) : -1);

					/* Totally Illegal */
					if ((i < 0) || (i >= num))
					{
						bell("Illegal destination choice!");
						continue;
					}

					/* Get selection */
					selection = routes[i];

					/* Require "okay" spells */
					if (selection < 0)
					{
						bell("Illegal destination choice!");
						msg_print("You may not travel there from here.");
						continue;
					}

					/* Stop the loop */
					flag = TRUE;
				}

				/* Restore the screen */
				if (redraw)
				{
					/* Load screen */
					screen_load();

					/* Hack -- forget redraw */
					/* redraw = FALSE; */
				}


				/* Abort if needed */
				if (!flag) return;

				/* Set journey time; takes longer at night */
				if (selection == t_ptr->nearby)
				{
					journey = damroll(2 + (level_flag & LF1_DAYLIGHT ? 1 : 0), 4);
				}
				else if (selection == t_ptr->distant)
				{
					journey = damroll(3 + (level_flag & LF1_DAYLIGHT ? 1 : 0), 4);
				}
				else
				{
					journey = damroll(4 + (level_flag & LF1_DAYLIGHT ? 1 : 0), 4);
				}
			}
			else
			{
				cptr q, s;

				int item;
				object_type *o_ptr;

				/* Return to Angband? */
				if (p_ptr->dungeon != 0) selection = 0;

				/* Restrict choices to scrolls */
				item_tester_tval = TV_MAP;

				/* Get an item */
				q = "Follow which map? ";
				s = "You have no maps to guide you.";
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

				/* In a bag? */
				if (o_ptr->tval == TV_BAG)
				{
					/* Get item from bag */
					if (!get_item_from_bag(&item, q, s, o_ptr)) return;

					/* Refer to the item */
					o_ptr = &inventory[item];
				}

				selection = o_ptr->sval;

				/* Did not make selection */
				if (!selection) return;

				/* Returning to Angband */
				if (selection == p_ptr->dungeon) selection = 0;

				/* Journey time */
				journey = damroll(2,4);
			}

			if (journey < 4)
			{
				msg_print("You have a mild and pleasant journey.");
			}
			else if (journey < 7)
			{
				msg_print("Your travels are without incident.");
			}
			else if (journey < 10)
			{
				msg_print("You have a long and arduous trip.");
			}
			else
			{
				msg_print("You get lost in the wilderness!");
				/* XXX Fake a wilderness location? */
			}

			/* Hack -- Get hungry/tired/sore */
			set_food(MAX(500, p_ptr->food-(PY_FOOD_FULL/10*journey)));

			/* Hack -- Time passes (at 4* food use rate) */
			turn += PY_FOOD_FULL/10*journey*4;

			/* XXX Recharges, stop temporary speed etc. */

			/* Check quests due to travelling - cancel if requested */
			if (!check_travel_quest(selection, min_depth(p_ptr->dungeon), TRUE)) return;

			/* Change the dungeon */
			p_ptr->dungeon = selection;

			/* Set the new depth */
			p_ptr->depth = min_depth(p_ptr->dungeon);

#if 0
			/* Reset the recall depth */
			p_ptr->max_depth = min_depth(p_ptr->dungeon);
#endif

			/* Leaving */
			p_ptr->leaving = TRUE;
		}

		return;
	}
}


/*
 * Go up one level, or choose a different dungeon.
 */
void do_cmd_go_up(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	feature_type *f_ptr= &f_info[cave_feat[py][px]];

	/* Verify stairs */
	if (!(f_ptr->flags1 & (FF1_STAIRS)) || !(f_ptr->flags1 & (FF1_LESS)))
	{
		/* Travel if possible */
		if (p_ptr->depth == min_depth(p_ptr->dungeon))
		{
			do_cmd_travel();
			return;
		}

		msg_print("I see no up staircase here.");
		return;
	}

	/* Ironman */
	if ((adult_ironman) && !(adult_campaign))
	{
		msg_print("Nothing happens!");
		return;
	}

	/* Hack -- travel through wilderness */
	if ((adult_campaign) && (p_ptr->depth == max_depth(p_ptr->dungeon)) && (t_info[p_ptr->dungeon].zone[0].tower))
	{
		/* Check quests due to travelling - cancel if requested */
		if (!check_travel_quest(t_info[p_ptr->dungeon].distant, min_depth(p_ptr->dungeon), TRUE)) return;

		/* Success */
		message(MSG_STAIRS_DOWN,0,format("You have found a way through %s.",t_name + t_info[p_ptr->dungeon].name));

		/* Change the dungeon */
		p_ptr->dungeon = t_info[p_ptr->dungeon].distant;

		/* Set the new depth */
		p_ptr->depth = min_depth(p_ptr->dungeon);

		/* Leaving */
		p_ptr->leaving = TRUE;
	}
	else
	{
		/* Check quests due to travelling - cancel if requested */
		if (t_info[p_ptr->dungeon].zone[0].tower)
		{
			if (!check_travel_quest(p_ptr->dungeon, p_ptr->depth + 1, TRUE)) return;
		}
		else
		{
			if (!check_travel_quest(p_ptr->dungeon, p_ptr->depth - 1, TRUE)) return;
		}

		/* Hack -- take a turn */
		p_ptr->energy_use = 100;

		/* Success */
		message(MSG_STAIRS_UP, 0, "You enter a maze of up staircases.");

		/* Create a way back */
		p_ptr->create_stair = feat_state(cave_feat[py][px], FS_LESS);

		/* Hack -- tower level increases depth */
		if (t_info[p_ptr->dungeon].zone[0].tower)
		{
			/* New depth */
			p_ptr->depth++;
		}
		else
		{
			/* New depth */
			p_ptr->depth--;
		}
	}

	/* Leaving */
	p_ptr->leaving = TRUE;
}


/*
 * Go down one level
 */
void do_cmd_go_down(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	feature_type *f_ptr= &f_info[cave_feat[py][px]];

	/* Verify stairs */
	if (!(f_ptr->flags1 & (FF1_STAIRS)) || !(f_ptr->flags1 & (FF1_MORE)))
	{
		msg_print("I see no down staircase here.");
		return;
	}

	/* Hack -- take a turn */
	p_ptr->energy_use = 100;

	/* Hack -- travel through wilderness */
	if ((adult_campaign) && (p_ptr->depth == max_depth(p_ptr->dungeon)) && !(t_info[p_ptr->dungeon].zone[0].tower))
	{
		/* Check quests due to travelling - cancel if requested */
		if (!check_travel_quest(t_info[p_ptr->dungeon].distant, min_depth(p_ptr->dungeon), TRUE)) return;

		/* Success */
		message(MSG_STAIRS_DOWN,0,format("You have found a way through %s.",t_name + t_info[p_ptr->dungeon].name));

		/* Change the dungeon */
		p_ptr->dungeon = t_info[p_ptr->dungeon].distant;

		/* Set the new depth */
		p_ptr->depth = min_depth(p_ptr->dungeon);

	}
	else
	{
		/* Check quests due to travelling - cancel if requested */
		if (t_info[p_ptr->dungeon].zone[0].tower)
		{
			if (!check_travel_quest(p_ptr->dungeon, p_ptr->depth + 1, TRUE)) return;
		}
		else
		{
			if (!check_travel_quest(p_ptr->dungeon, p_ptr->depth - 1, TRUE)) return;
		}

		/* Success */
		message(MSG_STAIRS_DOWN, 0, "You enter a maze of down staircases.");

		/* Create a way back */
		p_ptr->create_stair = feat_state(cave_feat[py][px], FS_MORE);

		/* Hack -- tower level decreases depth */
		if (t_info[p_ptr->dungeon].zone[0].tower)
		{
			/* New depth */
			p_ptr->depth--;
		}
		else
		{
			/* New depth */
			p_ptr->depth++;
		}
	}

	/* Leaving */
	p_ptr->leaving = TRUE;

}



/*
 * Simple command to "search" for one turn
 */
void do_cmd_search(void)
{

	/* Get the feature */
	feature_type *f_ptr = &f_info[cave_feat[p_ptr->py][p_ptr->px]];

	/* Allow repeated command */
	if (p_ptr->command_arg)
	{
		/* Set repeat count */
		p_ptr->command_rep = p_ptr->command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		p_ptr->command_arg = 0;
	}

	/* Take a turn */
	p_ptr->energy_use = 100;

	/* Catch breath */
	if (!(f_ptr->flags2 & (FF2_FILLED)))
	{
		/* Rest the player */
		set_rest(p_ptr->rest + PY_REST_RATE - p_ptr->tiring);
	}

	/* Search */
	search();
}


/*
 * Hack -- toggle search mode
 */
void do_cmd_toggle_search(void)
{

	/* Hack - Check if we are holding a song */
	if (p_ptr->held_song)
	{
		/* Finish song */
		p_ptr->held_song = 0;

		/* Tell the player */
		msg_print("You finish your song.");
	}

	/* Stop searching */
	if (p_ptr->searching)
	{
		/* Clear the searching flag */
		p_ptr->searching = FALSE;

		/* Clear the last disturb */
		p_ptr->last_disturb = turn;

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);
	}

	/* Start searching */
	else
	{
		/* Set the searching flag */
		p_ptr->searching = TRUE;

		/* Update stuff */
		p_ptr->update |= (PU_BONUS);

		/* Redraw stuff */
		p_ptr->redraw |= (PR_STATE | PR_SPEED);
	}
}



#if defined(ALLOW_EASY_OPEN)

/*
 * Return the number of features around (or under) the character.
 * Usually look for doors and floor traps.
 * ANDY - Counts features that allow action.
 */
static int count_feats(int *y, int *x, int action)
{
	int d, count;

	int feat;

	feature_type *f_ptr;

	u32b flag, bitzero = 0x000000001L;


	/* Count how many matches */
	count = 0;

	/* Check around the character */
	for (d = 0; d < 8; d++)
	{
		/* Extract adjacent (legal) location */
		int yy = p_ptr->py + ddy_ddd[d];
		int xx = p_ptr->px + ddx_ddd[d];

		/* Must have knowledge */
		if (!(play_info[yy][xx] & (PLAY_MARK))) continue;

		/* Get the feature */
		feat = cave_feat[yy][xx];

		/* Get the mimiced feature */
		feat = f_info[feat].mimic;

		f_ptr = &f_info[feat];

		if (action < FS_FLAGS2)
		{
			flag = bitzero << (action - FS_FLAGS1);
			if (!(f_ptr->flags1 & flag)) continue;  
		}

		else if (action < FS_FLAGS_END)
		{       
			flag = bitzero << (action - FS_FLAGS2);
			if (!(f_ptr->flags2 & flag)) continue;  
		}
	
		/* Count it */
		++count;

		/* Remember the location of the last door found */
		*y = yy;
		*x = xx;
	}

	/* All done */
	return count;
}

/*
 * Extract a "direction" which will move one step from the player location
 * towards the given "target" location (or "5" if no motion necessary).
 */
static int coords_to_dir(int y, int x)
{
	return (motion_dir(p_ptr->py, p_ptr->px, y, x));
}

#endif /* ALLOW_EASY_OPEN */


/*
 * Perform the basic "open" command on doors
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_open_aux(int y, int x)
{
	int i, j;

	bool more = FALSE;

	/* Verify legality */
	if (!do_cmd_test(y, x, FS_OPEN)) return (FALSE);

	/* Unknown trapped door */
	if (f_info[cave_feat[y][x]].flags3 & (FF3_PICK_DOOR))
	{
		pick_door(y,x);
	}

	/* Trapped door */
	if (f_info[cave_feat[y][x]].flags1 & (FF1_HIT_TRAP))
	{
		hit_trap(y,x);
	}

	/* Permanent doors */
	else if (f_info[cave_feat[y][x]].flags1 & (FF1_PERMANENT))
	{
		/* Stuck */
		find_secret(y,x);

		return (FALSE);
	}

	/* Locked door */
	else if ((f_info[cave_feat[y][x]].flags1 & (FF1_OPEN)) && (f_info[cave_feat[y][x]].power >0))
	{
		/* Find secrets */
		if (f_info[cave_feat[y][x]].flags1 & (FF1_SECRET))
		{
			find_secret(y,x);

			/* Sanity check */
			if (!(f_info[cave_feat[y][x]].flags1 & (FF1_OPEN))) return (FALSE);
		}

		/* Disarm factor */
		i = p_ptr->skill_dis;

		/* Penalize some conditions */
		if (p_ptr->blind || no_lite()) i = i / 10;
		if (p_ptr->confused || p_ptr->image) i = i / 10;

		/* Extract the lock power */
		j = f_info[cave_feat[y][x]].power;

		/* Extract the difficulty XXX XXX XXX */
		j = i - (j * 4);

		/* Always have a small chance of success */
		if (j < 2) j = 2;

		/* Success */
		if (rand_int(100) < j)
		{
			/* Message */
			message(MSG_OPENDOOR, 0, "You have picked the lock.");

			/* Open the door */
			cave_alter_feat(y, x, FS_OPEN);

			/* Update the visuals */
			p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

			/* Experience */
			gain_exp(1);
		}

		/* Failure */
		else
		{
			/* Failure */
			if (flush_failure) flush();

			/* Message */
			message(MSG_LOCKPICK_FAIL, 0, "You failed to pick the lock.");

			/* We may keep trying */
			more = TRUE;
		}
	}

	/* Closed door */
	else
	{
		/* Find secrets */
		if (f_info[cave_feat[y][x]].flags1 & (FF1_SECRET))
		{
			find_secret(y,x);

			/* Sanity check */
			if (!(f_info[cave_feat[y][x]].flags1 & (FF1_OPEN))) return (FALSE);
		}

		/* Open the door */
		cave_alter_feat(y, x, FS_OPEN);

		/* Update the visuals */
		p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

		/* Sound */
		sound(MSG_OPENDOOR);
	}

	/* Result */
	return (more);
}



/*
 * Open a closed/locked/jammed door.
 *
 * Unlocking a locked door/chest is worth one experience point.
 */
void do_cmd_open(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y, x, dir;

	bool more = FALSE;

#ifdef ALLOW_EASY_OPEN

	/* Easy Open */
	if (easy_open)
	{
		/* Handle a single closed door  */
		if (count_feats(&y, &x, FS_OPEN)  == 1)
		{
			p_ptr->command_dir = coords_to_dir(y, x);
		}
	}

#endif /* ALLOW_EASY_OPEN */

	/* Get a direction (or abort) */
	if (!get_rep_dir(&dir)) return;

	/* Hack -- Apply stuck */
	stuck_player(&dir);

	/* Get location */
	y = py + ddy[dir];
	x = px + ddx[dir];


	/* Verify legality */
	if (!do_cmd_test(y, x,FS_OPEN)) return;


	/* Take a turn */
	p_ptr->energy_use = 100;

	/* Apply confusion */
	if (dir && confuse_dir(&dir))
	{
		/* Get location */
		y = py + ddy[dir];
		x = px + ddx[dir];

	}


	/* Allow repeated command */
	if (p_ptr->command_arg)
	{
		/* Set repeat count */
		p_ptr->command_rep = p_ptr->command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		p_ptr->command_arg = 0;
	}

	/* Monster */
	if (cave_m_idx[y][x] > 0)
	{
		bool charging = FALSE;

		/* Message */
		msg_print("There is a monster in the way!");

		/* If moving, you can charge in the direction */
		if ((p_ptr->charging == dir) || (side_dirs[dir][1] == p_ptr->charging)
			|| (side_dirs[dir][2] == p_ptr->charging)) charging = TRUE;

		/* Attack */
		py_attack(y, x, charging);
	}

	/* Door */
	else
	{
		/* Open the door */
		more = do_cmd_open_aux(y, x);
	}

	/* Cancel repeat unless we may continue */
	if (!more) disturb(0, 0);
}


/*
 * Perform the basic "close" command
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_close_aux(int y, int x)
{
	bool more = FALSE;


	/* Verify legality */
	if (!do_cmd_test(y, x,FS_CLOSE)) return (FALSE);

	/* Trapped door */
	if (f_info[cave_feat[y][x]].flags1 & (FF1_HIT_TRAP))
	{
		hit_trap(y,x);

		/* Update the visuals */
		p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	}

	/* Permanent doors */
	else if	(f_info[cave_feat[y][x]].flags1 & (FF1_PERMANENT))
	{
		/* Stuck */
		find_secret(y,x);

		return (FALSE);
	}

	/* Close door */
	else
	{
		/* Find secrets */
		if (f_info[cave_feat[y][x]].flags1 & (FF1_SECRET))
		{
			find_secret(y,x);

			/* Sanity check */
			if (!(f_info[cave_feat[y][x]].flags1 & (FF1_CLOSE))) return (FALSE);
		}

		/* Close the door */
		cave_alter_feat(y, x, FS_CLOSE);

		/* Update the visuals */
		p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

		/* Sound */
		sound(MSG_SHUTDOOR);
	}

	/* Result */
	return (more);
}


/*
 * Close an open door.
 */
void do_cmd_close(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y, x, dir;

	bool more = FALSE;

#ifdef ALLOW_EASY_OPEN

	/* Easy Close */
	if (easy_open)
	{
		/* Handle a single open door */
		if (count_feats(&y, &x, FS_CLOSE) == 1)
		{
			/* Don't close door player is on */
			if ((y != py) || (x != px))
			{
				p_ptr->command_dir = coords_to_dir(y, x);
			}
		}
	}

#endif /* ALLOW_EASY_OPEN */

	/* Get a direction (or abort) */
	if (!get_rep_dir(&dir)) return;

	/* Hack -- Apply stuck */
	stuck_player(&dir);

	/* Get location */
	y = py + ddy[dir];
	x = px + ddx[dir];

	/* Verify legality */
	if (!do_cmd_test(y, x, FS_CLOSE)) return;

	/* Take a turn */
	p_ptr->energy_use = 50;

	/* Apply stuck / confusion */
	if (dir && confuse_dir(&dir))
	{
		/* Get location */
		y = py + ddy[dir];
		x = px + ddx[dir];

	}

	/* Allow repeated command */
	if (p_ptr->command_arg)
	{
		/* Set repeat count */
		p_ptr->command_rep = p_ptr->command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		p_ptr->command_arg = 0;
	}

	/* Monster */
	if (cave_m_idx[y][x] > 0)
	{
		bool charging = FALSE;

		/* Message */
		msg_print("There is a monster in the way!");

		/* If moving, you can charge in the direction */
		if ((p_ptr->charging == dir) || (side_dirs[dir][1] == p_ptr->charging)
			|| (side_dirs[dir][2] == p_ptr->charging)) charging = TRUE;

		/* Attack */
		py_attack(y, x, charging);
	}

	/* Door */
	else
	{
		/* Close door */
		more = do_cmd_close_aux(y, x);
	}

	/* Cancel repeat unless told not to */
	if (!more) disturb(0, 0);
}






/*
 * Perform the basic "tunnel" command
 *
 * Assumes that no monster is blocking the destination
 *
 * Do not use twall anymore --- ANDY
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_tunnel_aux(int y, int x)
{
	bool more = FALSE;

	int i,j;

	cptr name;

	int feat;
	
	feat = cave_feat[y][x];

	/* Verify legality */
	if (!do_cmd_test(y, x, FS_TUNNEL)) return (FALSE);

	i = p_ptr->skill_dig;

	j = f_info[cave_feat[y][x]].power;      

	/* Hack - bump up power for doors */
	if (f_info[cave_feat[y][x]].flags1 & (FF1_DOOR)) j = 30;

	/* Trapped door */
	if (f_info[cave_feat[y][x]].flags1 & (FF1_HIT_TRAP))
	{
		hit_trap(y,x);

		/* Update the visuals */
		p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	}

	/* Permanent rock */
	else if (f_info[cave_feat[y][x]].flags1 & (FF1_PERMANENT))
	{
		/* Stuck */
		find_secret(y,x);

		return (FALSE);
	}

	/* Dig or tunnel */
	else if (f_info[cave_feat[y][x]].flags2 & (FF2_CAN_DIG))
	{
		/* Dig */
		if (p_ptr->skill_dig > rand_int(20 * j))
		{
			sound(MSG_DIG);

			/* Get mimiced feature */
			feat = f_info[feat].mimic;

			/* Get the name */
			name = (f_name + f_info[feat].name);

			/* Give the message */  
			msg_format("You have removed the %s.",name);
			
			cave_alter_feat(y,x,FS_TUNNEL);

			/* Update the visuals */
			p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
		}

		/* Keep trying */
		else
		{
			/* Get mimiced feature */
			feat = f_info[feat].mimic;

			/* Get the name */
			name = (f_name + f_info[feat].name);

			/* We may continue tunelling */
			msg_format("You dig into the %s.",name);
			more = TRUE;
		}
	}

	else
	{
		/* Tunnel -- much harder */
		if (p_ptr->skill_dig > (j + rand_int(40 * j)))
		{
			sound(MSG_DIG);

			/* Get mimiced feature */
			feat = f_info[feat].mimic;

			/* Get the name */
			name = (f_name + f_info[feat].name);

			/* Give the message */  
			msg_print("You have finished the tunnel.");
			
			cave_alter_feat(y,x,FS_TUNNEL);

			/* Update the visuals */
			p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
		}

		/* Keep trying */
		else
		{

			/* Get mimiced feature */
			feat = f_info[feat].mimic;

			/* Get the name */
			name = (f_name + f_info[feat].name);

			/* We may continue tunelling */
			msg_format("You tunnel into the %s.",name);
			more = TRUE;
		}
	}

	/* Result */
	return (more);
}

/*
 * Tunnel through "walls" (including rubble and secret doors)
 *
 * Digging is very difficult without a "digger" weapon, but can be
 * accomplished by strong players using heavy weapons.
 */
void do_cmd_tunnel(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y, x, dir;

	bool more = FALSE;

#ifdef ALLOW_EASY_OPEN

	/* Easy Tunnel */
	if (easy_open)
	{
		/* Handle a single open door */
		if (count_feats(&y, &x, FS_TUNNEL) == 1)
		{
			/* Don't close door player is on */
			if ((y != py) || (x != px))
			{
				p_ptr->command_dir = coords_to_dir(y, x);
			}
		}
	}

#endif /* ALLOW_EASY_OPEN */

	/* Get a direction (or abort) */
	if (!get_rep_dir(&dir)) return;

	/* Hack -- Apply stuck */
	stuck_player(&dir);

	/* Get location */
	y = py + ddy[dir];
	x = px + ddx[dir];


	/* Oops */
	if (!do_cmd_test(y, x, FS_TUNNEL)) return;

	/* Take a turn */
	p_ptr->energy_use = 100;

	/* Apply confusion */
	if (dir && confuse_dir(&dir))
	{
		/* Get location */
		y = py + ddy[dir];
		x = px + ddx[dir];

	}

	/* Allow repeated command */
	if (p_ptr->command_arg)
	{
		/* Set repeat count */
		p_ptr->command_rep = p_ptr->command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		p_ptr->command_arg = 0;
	}

	/* Monster */
	if (cave_m_idx[y][x] > 0)
	{
		bool charging = FALSE;

		/* Message */
		msg_print("There is a monster in the way!");

		/* If moving, you can charge in the direction */
		if ((p_ptr->charging == dir) || (side_dirs[dir][1] == p_ptr->charging)
			|| (side_dirs[dir][2] == p_ptr->charging)) charging = TRUE;

		/* Attack */
		py_attack(y, x, charging);
	}

	/* Walls */
	else
	{
		/* Tunnel through walls */
		more = do_cmd_tunnel_aux(y, x);
	}

	/* Cancel repetition unless we can continue */
	if (!more) disturb(0, 0);
}

/*
 * Perform the basic "disarm" command
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_disarm_aux(int y, int x, bool disarm)
{
	int i, j, power;

	cptr name, act;

	bool more = FALSE;

	/* Arm or disarm */
	if (disarm) act = "disarm";
	else act = "arm";

	/* Verify legality */
	if (!do_cmd_test(y, x, (disarm ? FS_DISARM : FS_TRAP))) return (FALSE);

	/* Get the trap name */
	name = (f_name + f_info[cave_feat[y][x]].name);

	/* Get the "disarm" factor */
	i = p_ptr->skill_dis;

	/* Penalize some conditions */
	if (p_ptr->blind || no_lite()) i = i / 10;
	if (p_ptr->confused || p_ptr->image) i = i / 10;

	/* XXX XXX XXX Variable power? */

	/* Extract trap "power" */
	power = f_info[cave_feat[y][x]].power;

	/* Player trap */
	if (cave_o_idx[y][x])
	{
		/* Use object level instead */
		power = k_info[o_list[cave_o_idx[y][x]].k_idx].level;
	}

	/* Extract the difficulty */
	j = i - power;

	/* Always have a small chance of success */
	if (j < 2) j = 2;

	/* Success */
	if (rand_int(100) < j)
	{
		/* Message */
		msg_format("You have %sed the %s.", act, name);

		/* Reward */
		gain_exp(power);

		/* Remove the trap */
		if (disarm)
		{
			/* Remove the trap */
			cave_alter_feat(y, x, FS_DISARM);

			/* Update the visuals */
			p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
		}
	}

	/* Failure -- Keep trying */
	else if ((i > 5) && (randint(i) > 5))
	{
		/* Failure */
		if (flush_failure) flush();

		/* Message */
		msg_format("You failed to %s the %s.", act, name);

		/* We may keep trying */
		more = TRUE;

		/* Remove the trap */
		if (!disarm)
		{
			/* Remove the trap */
			cave_alter_feat(y, x, FS_DISARM);

			/* Update the visuals */
			p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
		}
	}

	/* Failure -- Set off the trap */
	else
	{
		/* Message */
		msg_format("You set off the %s!", name);

		/* Hit the trap */
		hit_trap(y, x);

		/* Update the visuals */
		p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	}

	/* Result */
	return (more);
}


/*
 * Disarms a trap
 */
void do_cmd_disarm(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y, x, dir;

	bool more = FALSE;

#ifdef ALLOW_EASY_OPEN

	/* Easy Disarm */
	if (easy_open)
	{
		/* Handle a single visible trap or trapped chest */
		if (count_feats(&y, &x, FS_DISARM) == 1)
		{
			p_ptr->command_dir = coords_to_dir(y, x);
		}
	}

#endif /* ALLOW_EASY_OPEN */

	/* Get a direction (or abort) */
	if (!get_rep_dir(&dir)) return;

	/* Hack -- Apply stuck */
	stuck_player(&dir);

	/* Get location */
	y = py + ddy[dir];
	x = px + ddx[dir];


	/* Verify legality */
	if (!do_cmd_test(y, x, FS_DISARM)) return;


	/* Take a turn */
	p_ptr->energy_use = 100;

	/* Apply stuck / confusion */
	if (dir && confuse_dir(&dir))
	{
		/* Get location */
		y = py + ddy[dir];
		x = px + ddx[dir];

	}
	/* Allow repeated command */
	if (p_ptr->command_arg)
	{
		/* Set repeat count */
		p_ptr->command_rep = p_ptr->command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		p_ptr->command_arg = 0;
	}

	/* Monster */
	if (cave_m_idx[y][x] > 0)
	{
		bool charging = FALSE;

		/* Message */
		msg_print("There is a monster in the way!");

		/* If moving, you can charge in the direction */
		if ((p_ptr->charging == dir) || (side_dirs[dir][1] == p_ptr->charging)
			|| (side_dirs[dir][2] == p_ptr->charging)) charging = TRUE;

		/* Attack */
		py_attack(y, x, charging);
	}

	/* Disarm trap */
	else
	{
		/* Disarm the trap */
		more = do_cmd_disarm_aux(y, x, TRUE);
	}

	/* Cancel repeat unless told not to */
	if (!more) disturb(0, 0);
}



/*
 * Perform the basic "bash" command
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_bash_aux(int y, int x, bool charging)
{
	int bash, temp;

	int feat;

	cptr name;

	bool more = FALSE;

	/* Verify legality */
	if (!do_cmd_test(y, x,FS_BASH)) return (FALSE);

	/* Get mimiced feature */
	feat = f_info[cave_feat[y][x]].mimic;

	/* Get the name */
	name = (f_name + f_info[feat].name);

	/* Message */
	msg_format("You smash into the %s!",name);

	/* Trapped door */
	if (f_info[cave_feat[y][x]].flags1 & (FF1_HIT_TRAP))
	{
		hit_trap(y,x);

		/* Update the visuals */
		p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
	}

	/* Permanent doors */
	else if (f_info[cave_feat[y][x]].flags1 & (FF1_PERMANENT))
	{
		/* Stuck */
		find_secret(y,x);

		return (FALSE);
	}

	/* Hack -- Bash power based on strength */
	/* (Ranges from 3 to 20 to 100 to 200) */
	bash = adj_str_blow[p_ptr->stat_ind[A_STR]];

	/* Bonus for charging */
	if (charging) bash *= 2;

	/* Extract door power */
	temp = f_info[cave_feat[y][x]].power;

	/* Compare bash power to door power XXX XXX XXX */
	temp = (bash - (temp * 10));

	/* Hack -- always have a chance */
	if (temp < 1) temp = 1;

	/* Hack -- attempt to bash down the door */
	if (rand_int(100) < temp)
	{
		/* Message */
		msg_format("The %s crashes open!",name);

		/* Find secrets */
		if (f_info[cave_feat[y][x]].flags1 & (FF1_SECRET))
		{
			find_secret(y,x);

			/* Sanity check */
			if (!(f_info[cave_feat[y][x]].flags1 & (FF1_BASH))) return (FALSE);
		}

		/* Break down the door */
		if (rand_int(100) < 50)
		{
			cave_alter_feat(y, x, FS_BASH);
		}

		/* Open the door */
		else
		{
			cave_alter_feat(y, x, FS_OPEN);
		}

		/* Update the visuals */
		p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

		/* Sound */
		sound(MSG_OPENDOOR);

	}

	/* Saving throw against stun */
	else if (rand_int(100) < adj_agi_safe[p_ptr->stat_ind[A_AGI]] +
		 p_ptr->lev)
	{
		/* Message */
		msg_format("The %s holds firm.",name);

		/* Allow repeated bashing */
		more = TRUE;
	}

	/* High agility yields coolness */
	else
	{
		/* Message */
		msg_print("You are off-balance.");

		/* Hack -- Lose balance ala paralysis */
		(void)set_paralyzed(p_ptr->paralyzed + 2 + rand_int(2));
	}

	/* Result */
	return (more);
}


/*
 * Bash open a door, success based on character strength
 *
 * A closed door can be opened - harder if locked. Any door might be
 * bashed open (and thereby broken). Bashing a door is (potentially)
 * faster! You move into the door way. To open a stuck door, it must
 * be bashed. A closed door can be jammed (see do_cmd_spike()).
 *
 * Creatures can also open or bash doors, see elsewhere.
 */
void do_cmd_bash(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y, x, dir;

	bool charging = FALSE;

#ifdef ALLOW_EASY_OPEN

	/* Easy Bash */
	if (easy_open)
	{
		/* Handle a single visible trap */
		if (count_feats(&y, &x, FS_BASH)==1)
		{
			p_ptr->command_dir = coords_to_dir(y, x);
		}
	}

#endif /* ALLOW_EASY_OPEN */

	/* Get a direction (or abort) */
	if (!get_rep_dir(&dir)) return;

	/* Hack -- Apply stuck */
	stuck_player(&dir);

	/* Get location */
	y = py + ddy[dir];
	x = px + ddx[dir];


	/* Verify legality */
	if (!do_cmd_test(y, x, FS_BASH)) return;


	/* Take a turn */
	p_ptr->energy_use = 100;

	/* Apply confusion */
	if (dir && confuse_dir(&dir))
	{
		/* Get location */
		y = py + ddy[dir];
		x = px + ddx[dir];

	}

	/* If moving, you can charge in the direction you move */
	if ((p_ptr->charging == dir) || (side_dirs[dir][1] == p_ptr->charging)
		|| (side_dirs[dir][2] == p_ptr->charging)) charging = TRUE;

	/* Allow repeated command */
	if (p_ptr->command_arg)
	{
		/* Set repeat count */
		p_ptr->command_rep = p_ptr->command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		p_ptr->command_arg = 0;
	}

	/* Monster */
	if (cave_m_idx[y][x] > 0)
	{
		/* Message */
		msg_print("There is a monster in the way!");

		/* Attack */
		py_attack(y, x, charging);
	}

	/* Door */
	else
	{
		/* Bash the door */
		if (!do_cmd_bash_aux(y, x, charging))
		{
			/* Cancel repeat */
			disturb(0, 0);
		}
	}
}



/*
 * Manipulate an adjacent grid in some way
 *
 * Attack monsters, tunnel through walls, disarm traps, open doors.
 *
 * This command must always take energy, to prevent free detection
 * of invisible monsters.
 *
 * The "semantics" of this command must be chosen before the player
 * is confused, and it must be verified against the new grid.
 */
void do_cmd_alter(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y, x, dir;

	int feat;

	bool more = FALSE;


	/* Get a direction */
	if (!get_rep_dir(&dir)) return;

	/* Hack -- Apply stuck */
	stuck_player(&dir);

	/* Get location */
	y = py + ddy[dir];
	x = px + ddx[dir];


	/* Original feature */
	feat = cave_feat[y][x];

	/* Get mimiced feature */
	feat = f_info[feat].mimic;

	/* Must have knowledge to know feature XXX XXX */
	if (!(play_info[y][x] & (PLAY_MARK))) feat = FEAT_NONE;


	/* Take a turn */
	p_ptr->energy_use = 100;

	/* Apply confusion */
	if (dir && confuse_dir(&dir))
	{
		/* Get location */
		y = py + ddy[dir];
		x = px + ddx[dir];

	}

	/* Allow repeated command */
	if (p_ptr->command_arg)
	{
		/* Set repeat count */
		p_ptr->command_rep = p_ptr->command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		p_ptr->command_arg = 0;
	}

	/* Attack monsters */
	if (cave_m_idx[y][x] > 0)
	{
		bool charging = FALSE;

		/* Message */
		msg_print("There is a monster in the way!");

		/* If moving, you can charge in the direction */
		if ((p_ptr->charging == dir) || (side_dirs[dir][1] == p_ptr->charging)
			|| (side_dirs[dir][2] == p_ptr->charging)) charging = TRUE;

		/* Attack */
		py_attack(y, x, charging);
	}

	/* Disarm traps */
	else if (f_info[feat].flags1 & (FF1_DISARM))
	{
		/* Tunnel */
		more = do_cmd_disarm_aux(y, x, TRUE);
	}

	/* Open closed doors */
	else if (f_info[feat].flags1 & (FF1_OPEN))
	{
		/* Tunnel */
		more = do_cmd_open_aux(y, x);
	}
#if 0
	/* Bash jammed doors */
	else if (f_info[feat].flags1 & (FF1_BASH))
	{
		/* Tunnel */
		more = do_cmd_bash_aux(y, x);
	}
#endif
	/* Tunnel through walls */
	else if (f_info[feat].flags1 & (FF1_TUNNEL))
	{
		/* Tunnel */
		more = do_cmd_tunnel_aux(y, x);
	}

#if 0

	/* Close open doors */
	else if (f_info[feat].flags1 & (FF1_CLOSE))
	{
		/* Close */
		more = do_cmd_close_aux(y, x);
	}

#endif

	/* Oops */
	else
	{
		/* Oops */
		msg_print("You spin around.");
	}

	/* Cancel repetition unless we can continue */
	if (!more) disturb(0, 0);
}


/*
 * Hack -- Set a trap or jam a door closed with a spike.
 *
 * This command may not be repeated.
 *
 * See pick_trap for how traps are chosen, and hit_trap and mon_hit_trap for what
 * player set traps will do.
 */
void do_cmd_set_trap_or_spike(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y, x, dir, item, action;

	object_type *o_ptr;

	cptr q,s;

	/* Get an item */
	q = "Spike/Set trap with which item? ";
	s = "You have nothing to set a trap or spike with.";
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

	/* In a bag? */
	if (o_ptr->tval == TV_BAG)
	{
		/* Get item from bag */
		if (!get_item_from_bag(&item, q, s, o_ptr)) return;

		/* Refer to the item */
		o_ptr = &inventory[item];
	}

	/* Spiking or setting trap? */
	if (o_ptr->tval == TV_SPIKE)
	{
		/* We are spiking */
		action = FS_SPIKE;
	}
	else
	{
		/* Hack -- only set traps on floors (at this stage) XXX */
		action = FS_FLOOR;
	}

#ifdef ALLOW_EASY_OPEN

	/* Easy Bash */
	if (easy_open)
	{
		/* Handle a single visible trap */
		if (count_feats(&y, &x, action)==1)
		{
			p_ptr->command_dir = coords_to_dir(y, x);
		}
	}

#endif /* ALLOW_EASY_OPEN */

	/* Get a direction (or abort) */
	if (!get_rep_dir(&dir)) return;

	/* Hack -- Apply stuck */
	stuck_player(&dir);

	/* Get location */
	y = py + ddy[dir];
	x = px + ddx[dir];


	/* Verify legality */
	if (!do_cmd_test(y, x, action)) return;

	/* Take a turn */
	p_ptr->energy_use = 100;

	/* Apply stuck / confusion */
	if (dir && confuse_dir(&dir))
	{
		/* Get location */
		y = py + ddy[dir];
		x = px + ddx[dir];

	}

	/* Monster */
	if (cave_m_idx[y][x] > 0)
	{
		bool charging = FALSE;

		/* Message */
		msg_print("There is a monster in the way!");

		/* If moving, you can charge in the direction */
		if ((p_ptr->charging == dir) || (side_dirs[dir][1] == p_ptr->charging)
			|| (side_dirs[dir][2] == p_ptr->charging)) charging = TRUE;

		/* Attack */
		py_attack(y, x, charging);
	}

	/* Go for it */
	else
	{
		/* Verify legality */
		if (!do_cmd_test(y, x, action)) return;

#if 0
		/* Trapped door */
		if (f_info[cave_feat[y][x]].flags1 & (FF1_HIT_TRAP))
		{
			hit_trap(y,x);
		}
#endif

		/* Permanent doors */
		/* else */ if (f_info[cave_feat[y][x]].flags1 & (FF1_PERMANENT))
		{
			/* Stuck */
			find_secret(y,x);

			return;
		}

		/* Spike the door */
		else if (action == FS_SPIKE)
		{
			object_type *k_ptr = NULL;

			int feat = cave_feat[y][x];
			int item2 = 0;

			/* Find secrets */
			if (f_info[cave_feat[y][x]].flags1 & (FF1_SECRET))
			{
				find_secret(y,x);

				/* Sanity check */
				if (!(f_info[cave_feat[y][x]].flags1 & (FF1_SPIKE))) return;
			}

			feat = feat_state(feat, FS_SPIKE);

			if (strstr(f_name + f_info[feat].name, "rope"))
			{
				/* Restrict an item */
				item_tester_tval = TV_ROPE;

				/* Get an item */
				q = "Attach which rope? ";
				s = "You have no rope to attach.";
				if (get_item(&item2, q, s, (USE_INVEN | USE_FLOOR)))
				{
					/* Get the object */
					if (item2 >= 0)
					{
						k_ptr = &inventory[item2];
					}
					else
					{
						k_ptr = &o_list[0 - item2];
					}

					/* In a bag? */
					if (k_ptr->tval == TV_BAG)
					{
						/* Get item from bag */
						if (get_item_from_bag(&item2, q, s, k_ptr))
						{
							/* Refer to the item */
							k_ptr = &inventory[item2];
						}
					}
				}
				else
				{
					return;
				}
			}

			cave_alter_feat(y,x,FS_SPIKE);

			/* MegaHack -- handle chain */
			if ((k_ptr) && (k_ptr->sval == SV_ROPE_CHAIN)) cave_set_feat(y, x, cave_feat[y][x] + 1);

			/* Update the visuals */
			p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

			/* Destroy a spike in the pack */
			if (item >= 0)
			{
				if ((o_ptr->number == 1) && (item2 > item)) item2--;

				inven_item_increase(item, -1);
				inven_item_describe(item);
				inven_item_optimize(item);
			}

			/* Destroy a spike on the floor */
			else
			{
				floor_item_increase(0 - item, -1);
				floor_item_describe(0 - item);
				floor_item_optimize(0 - item);
			}

			if (k_ptr)
			{
				/* Reduce inventory -- suppress messages */
				if (item2 >= 0)
				{
					if (k_ptr->number == 1)
					{
						inven_drop_flags(k_ptr);
						k_ptr = NULL;
					}

					inven_item_increase(item2, -1);
					inven_item_optimize(item2);
				}

				/* Reduce and describe floor item */
				else
				{
					if (k_ptr->number == 1) k_ptr = NULL;

					floor_item_increase(0 - item2, -1);
					floor_item_optimize(0 - item2);
				}
			}
		}

		/* Set the trap */
		/* We only let the player set traps when there are no existing
               objects in the grid OR the existing objects in the grid have
               the same tval and sval as the trap being set OR the trap
               being set is TV_BOW and all the objects in the grid can be fired
               by the bow in question */
		else
		{
			int this_o_idx, next_o_idx;

			bool trap_allowed = TRUE;

			object_type object_type_body;

			object_type *j_ptr;

			/* Find secrets */
			if (f_info[cave_feat[y][x]].flags1 & (FF1_SECRET))
			{
				find_secret(y,x);

				/* Sanity check */
				if (!(f_info[cave_feat[y][x]].flags1 & (FF1_FLOOR))) return;
			}

			/* Get object body */
			j_ptr = &object_type_body;

			/* Structure Copy */
			object_copy(j_ptr, o_ptr);

			/* Set one object only */
			j_ptr->number = 1;

			/* Scan all objects in the grid */
			for (this_o_idx = cave_o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
			{
				object_type *i_ptr;

				/* Get the object */
				i_ptr = &o_list[this_o_idx];

				/* Get the next object */
				next_o_idx = i_ptr->next_o_idx;

				/* Check if fired */
				if (j_ptr->tval == TV_BOW)
				{
					switch(j_ptr->sval)
					{
						case SV_LONG_BOW:
						case SV_SHORT_BOW:
						{
							if (i_ptr->tval != TV_ARROW) trap_allowed = FALSE;
							break;
						}
					    case SV_HAND_XBOW:
						case SV_LIGHT_XBOW:
						case SV_HEAVY_XBOW:
						{
							if (i_ptr->tval != TV_BOLT) trap_allowed = FALSE;
							break;
						}
						default:
						{
							if (!is_known_throwing_item(i_ptr)) trap_allowed = FALSE;
							break;
						}
					}
				}
				else if ((j_ptr->tval != i_ptr->tval) || (j_ptr->sval != i_ptr->sval))
				{
					/* Not allowed */
					trap_allowed = FALSE;
				}
			}

			/* Hack -- Dig trap? */
			if (j_ptr->tval == TV_DIGGING)
			{
				/* Hack */
				int tmp = p_ptr->skill_dis;

				/* Hack -- use tunnelling skill to build trap */
				p_ptr->skill_dis = p_ptr->skill_dig;

				/* Hack -- base trap directly on digging skill */
				object_level = MIN(128, p_ptr->skill_dig * 3);

				/* Set the floor trap */
				cave_set_feat(y,x,FEAT_TRAP_ROCK_NONE);

				/* Set the trap */
				pick_trap(y,x);

				/* Reset object level */
				object_level = p_ptr->depth;

				/* Check if we can arm it? */
				do_cmd_disarm_aux(y,x, FALSE);

				/* Reset disarm skill */
				p_ptr->skill_dis = tmp;
			}

			/* Trap allowed? */
			else if ((trap_allowed) && (floor_carry(y,x,j_ptr)))
			{
				/* Hack -- ensure trap is created */
				object_level = 128;

				/* Set the floor trap */
				cave_set_feat(y,x,FEAT_INVIS);

				/* Set the trap */
				pick_trap(y,x);

				/* Reset object level */
				object_level = p_ptr->depth;

				/* Check if we can arm it? */
				do_cmd_disarm_aux(y,x, FALSE);
	
				/* Destroy an item in the pack */
				if (item >= 0)
				{
					inven_item_increase(item, -1);
					inven_item_describe(item);
					inven_item_optimize(item);
				}

				/* Destroy an item on the floor */
				else
				{
					floor_item_increase(0 - item, -1);
					floor_item_describe(0 - item);
					floor_item_optimize(0 - item);
				}
			}
			/* Message */
			else
			{
				msg_print("You can't set this trap here.");
			}
		}
	}
}


static bool do_cmd_walk_test(int y, int x)
{
	int feat;

	cptr name; 

	/* Get feature */
	feat = cave_feat[y][x];

	/* Get mimiced feature */
	feat = f_info[feat].mimic;

	/* Get the name */
	name = (f_name + f_info[feat].name);

	/* Hack -- walking obtains knowledge XXX XXX */
	if (!(play_info[y][x] & (PLAY_MARK))) return (TRUE);

	/* Hack -- walking allows attacking XXX XXX */
	if (cave_m_idx[y][x] >0) return (TRUE);

	/* Hack -- walking allows pickup XXX XXX */
	if (cave_o_idx[y][x] >0) return (TRUE);

	/* Player can not walk through "walls" */
	/* Also cannot climb over unknown "trees/rubble" */
	if (!(f_info[feat].flags1 & (FF1_MOVE))
	&& (!(f_info[feat].flags3 & (FF3_EASY_CLIMB))
	|| !(play_info[y][x] & (PLAY_MARK))))
	{
#ifdef ALLOW_EASY_ALTER

		if (easy_alter)
		{

			if (f_info[feat].flags1 & (FF1_BASH)) return(TRUE);
			if (f_info[feat].flags1 & (FF1_OPEN)) return(TRUE);
		}

#endif /* ALLOW_EASY_ALTER */

		/* Message */
		msg_format("There is %s %s in the way.",
				(is_a_vowel(name[0]) ? "an" : "a"),name);

		/* Nope */
		return (FALSE);

	}

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for the "walk" and "jump" commands.
 */
static void do_cmd_walk_or_jump(int jumping)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y, x, dir;


	/* Get a direction (or abort) */
	if (!get_rep_dir(&dir)) return;

	/* Get location */
	y = py + ddy[dir];
	x = px + ddx[dir];


	/* Verify legality */
	if (!do_cmd_walk_test(y, x)) return;

	/* Take time */
	p_ptr->energy_use = 100;

	/* Hack -- handle stuck players */
	if (stuck_player(&dir))
	{
		/* Get the mimiced feature */
		int mimic = f_info[cave_feat[py][px]].mimic;

		/* Get the feature name */
		cptr name = (f_name + f_info[mimic].name);

		/* Tell the player */
		msg_format("You are stuck %s%s.",
			((f_info[mimic].flags2 & (FF2_FILLED)) ? "" :
				(is_a_vowel(name[0]) ? "inside an " : "inside a ")),name);
	}

	/* Apply confusion */
	else if (confuse_dir(&dir))
	{
		/* Get location */
		y = py + ddy[dir];
		x = px + ddx[dir];

	}


	/* Verify legality */
	if (!do_cmd_walk_test(y, x)) return;


	/* Allow repeated command */
	if (p_ptr->command_arg)
	{
		/* Set repeat count */
		p_ptr->command_rep = p_ptr->command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		p_ptr->command_arg = 0;
	}

	/* Move the player */
	move_player(dir, jumping);
}


/*
 * Walk into a grid.
 */
void do_cmd_walk(void)
{
	/* Move (normal) */
	do_cmd_walk_or_jump(FALSE);
}


/*
 * Jump into a grid.
 */
void do_cmd_jump(void)
{
	/* Move (jump) */
	do_cmd_walk_or_jump(TRUE);
}


/*
 * Start running.
 *
 * Note that running while confused is not allowed.
 */
void do_cmd_run(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y, x, dir;

	/* Hack XXX XXX XXX */
	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}

	/* Get a direction (or abort) */
	if (!get_rep_dir(&dir)) return;

	/* Hack -- handle stuck players */
	if (stuck_player(&dir))
	{
		int mimic = f_info[cave_feat[py][px]].mimic;

		/* Get the feature name */
		cptr name = (f_name + f_info[mimic].name);

		/* Use up energy */
		p_ptr->energy_use = 100;

		/* Tell the player */
		msg_format("You are stuck %s%s.",
			((f_info[mimic].flags2 & (FF2_FILLED)) ? "" :
				(is_a_vowel(name[0]) ? "inside an " : "inside a ")),name);

		return;
	}

	/* Get location */
	y = py + ddy[dir];
	x = px + ddx[dir];

	/* Verify legality */
	if (!do_cmd_walk_test(y, x)) return;

	/* Start run */
	run_step(dir);
}



/*
 * Start running with pathfinder.
 *
 * Note that running while confused is not allowed.
 */
void do_cmd_pathfind(int y, int x)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	/* Must not be 0 or 5 */
	int dummy = 1;

	/* Hack XXX XXX XXX */
	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}

	/* Hack -- handle stuck players */
	if (stuck_player(&dummy))
	{
		int mimic = f_info[cave_feat[py][px]].mimic;

		/* Get the feature name */
		cptr name = (f_name + f_info[mimic].name);

		/* Use up energy */
		p_ptr->energy_use = 100;

		/* Tell the player */
		msg_format("You are stuck %s%s.",
			((f_info[mimic].flags2 & (FF2_FILLED)) ? "" :
				(is_a_vowel(name[0]) ? "inside an " : "inside a ")),name);

		return;
	}

	if (findpath(y, x))
	{
		p_ptr->running = 1000;
		/* Calculate torch radius */
		p_ptr->update |= (PU_TORCH);
		p_ptr->running_withpathfind = TRUE;
		run_step(0);
	}
}


/*
 * Stay still.  Search.  Enter stores.
 * Pick up treasure if "pickup" is true.
 */
static void do_cmd_hold_or_stay(int pickup)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	/* Get the feature */
	feature_type *f_ptr = &f_info[cave_feat[p_ptr->py][p_ptr->px]];

	/* Allow repeated command */
	if (p_ptr->command_arg)
	{
		/* Set repeat count */
		p_ptr->command_rep = p_ptr->command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		p_ptr->command_arg = 0;
	}

	/* Take time */
	p_ptr->energy_use = 100;

	/* Catch breath */
	if (!(f_ptr->flags2 & (FF2_FILLED)))
	{
		/* Rest the player */
		set_rest(p_ptr->rest + PY_REST_RATE - p_ptr->tiring);
	}

	/* Spontaneous Searching - doubly effective */
	if ((p_ptr->skill_srh >= 25) || (0 == rand_int(25 - p_ptr->skill_srh)))
	{
		search();
	}

	/* Handle "objects" */
	py_pickup(py, px, pickup);

	/* Hack -- enter a store if we are on one */
	if (f_info[cave_feat[py][px]].flags1 & (FF1_ENTER))
	{
		/* Disturb */
		disturb(0, 0);

		/* Hack -- enter store */
		p_ptr->command_new = '_';

		/* Free turn XXX XXX XXX */
		p_ptr->energy_use = 0;
	}

	/* Blocking -- temporary bonus to ac */
	if (!p_ptr->searching)
	{
		int i;

		/* Slot */
		int slot = INVEN_ARM;

		/* Object  */
		object_type *o_ptr = &inventory[INVEN_ARM];

		/* Style */
		int melee_style = p_ptr->cur_style & (WS_WIELD_FLAGS);

		/* Hack -- not dodging */
		p_ptr->dodging = 0;

		/* Base blocking */
		p_ptr->blocking = p_ptr->to_h;

		/* No shield / secondary weapon */
		if (!o_ptr->k_idx)
		{
			if (inventory[INVEN_WIELD].k_idx)
			{
				slot = INVEN_WIELD;
			}
			else if (inventory[INVEN_HANDS].k_idx)
			{
				slot = INVEN_HANDS;
			}

			o_ptr = &inventory[slot];
		}

		/* Modify by object */
		if (o_ptr->k_idx)
		{
			/* Adjust by ac factor */
			p_ptr->blocking += o_ptr->ac + o_ptr->to_a;

			/* Adjust by to hit factor */
			p_ptr->blocking += o_ptr->to_h;
		}

		/* Modify by style */
		if (!p_ptr->heavy_wield) 
			for (i = 0; i < z_info->w_max; i++)
			{
				if (w_info[i].class != p_ptr->pclass) continue;

				if (w_info[i].level > p_ptr->lev) continue;

			/* Check for styles */
			if (w_info[i].styles==0 
				|| w_info[i].styles & melee_style & (1L << p_ptr->pstyle))
			{
				switch (w_info[i].benefit)
				{
					switch (w_info[i].benefit)
					{
					case WB_HIT:
					case WB_AC:
						p_ptr->blocking += (p_ptr->lev - w_info[i].level) /2;
						break;
					}
				}
			}
			}
	}
}

/*
 * Hold still (usually pickup)
 */
void do_cmd_hold(void)
{
	/* Hold still (usually pickup) */
	do_cmd_hold_or_stay(always_pickup);
}


/*
 * Stay still (usually do not pickup)
 */
void do_cmd_stay(void)
{
	/* Stay still (usually do not pickup) */
	do_cmd_hold_or_stay(!always_pickup);
}


/*
 * Rest (restores hit points and mana and such)
 */
void do_cmd_rest(void)
{
	/* Get the feature */
	feature_type *f_ptr = &f_info[cave_feat[p_ptr->py][p_ptr->px]];

	/* Prompt for time if needed */
	if (p_ptr->command_arg <= 0)
	{
		cptr p = "Rest (0-9999, '*' for HP/SP, '&' as needed, '$' to sleep to recover stats): ";

		char out_val[80];

		/* Default */
		strcpy(out_val, "&");

		/* Ask for duration */
		if (!get_string(p, out_val, 5)) return;

		/* Fall asleep */
		if (out_val[0] == '$')
		{
			p_ptr->command_arg = (-3);
		}

		/* Rest until done */
		else if (out_val[0] == '&')
		{
			p_ptr->command_arg = (-2);
		}

		/* Rest a lot */
		else if (out_val[0] == '*')
		{
			p_ptr->command_arg = (-1);
		}

		/* Rest some */
		else
		{
			p_ptr->command_arg = atoi(out_val);
			if (p_ptr->command_arg <= 0) return;
		}
	}


	/* Paranoia */
	if (p_ptr->command_arg > 9999) p_ptr->command_arg = 9999;

	/* Catch breath */
	if (!(f_ptr->flags2 & (FF2_FILLED)))
	{
		/* Rest the player */
		set_rest(p_ptr->rest + PY_REST_RATE * 2 - p_ptr->tiring);
	}

	/* Take a turn XXX XXX XXX (?) */
	p_ptr->energy_use = 100;

	/* Fall asleep */
	if (p_ptr->command_arg == -3)
	{
		p_ptr->command_arg = PY_SLEEP_ASLEEP;
		set_psleep(1);
	}

	/* Save the rest code */
	p_ptr->resting = p_ptr->command_arg;

	/* Cancel the arg */
	p_ptr->command_arg = 0;

	/* Cancel searching */
	p_ptr->searching = FALSE;

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw the state */
	p_ptr->redraw |= (PR_STATE);

	/* Handle stuff */
	handle_stuff();

	/* Refresh XXX XXX XXX */
	if (fresh_before) Term_fresh();
}






/*
 * Determines the odds of an object breaking when thrown at a monster
 *
 * Note that artifacts never break, see the "drop_near()" function.
 */
int breakage_chance(object_type *o_ptr)
{
	/* Examine the item type */
	switch (o_ptr->tval)
	{
		/* Always break */
		case TV_FLASK:
		case TV_SPELL:
		case TV_POTION:
		case TV_HOLD:
		case TV_FOOD:
		case TV_JUNK:
		case TV_SKIN:
		case TV_EGG:
		{
			return (100);
		}

		/* Often break */
		case TV_LITE:
		case TV_SCROLL:
		case TV_BONE:
		{
			return (50);
		}

		/* Sometimes break */
		case TV_ARROW:
		{
			return (35);
		}

		/* Sometimes break */
		case TV_WAND:
		case TV_SHOT:
		case TV_BOLT:
		case TV_BODY:
		{
			return (25);
		}
	}

	/* Rarely break */
	return (31 - adj_int_break[p_ptr->stat_ind[A_INT]]);
}


/*
 * Hook to determine if an object is rope
 */
static bool item_tester_hook_rope(const object_type *o_ptr)
{
	if ((o_ptr->tval == TV_ROPE) && (o_ptr->sval != SV_ROPE_CHAIN)) return (TRUE);

	/* Assume not */
	return (FALSE);
}


/*
 * Fire or throw an already chosen object.
 *
 * You may only fire items that "match" your missile launcher
 * or a throwing weapon (see do_cmd_throw_selected).
 *
 * You must use slings + pebbles/shots/thrown objects, 
 * bows + arrows, xbows + bolts.
 *
 * See "calc_bonuses()" for more calculations and such.
 *
 * Note: "unseen" monsters are very hard to hit.
 *
 * Objects are more likely to break if they "attempt" to hit a monster.
 *
 * The "extra shot" code works by decreasing the amount of energy
 * required to make each shot, spreading the shots out over time.
 *
 * Note that Bows of "Extra Might" get extra range and an extra bonus
 * for the damage multiplier.
 *
 * Note that Bows of "Extra Shots" give an extra shot.
 */
void do_cmd_fire_or_throw_selected(object_type *o_ptr, int item, bool fire)
{
  int item2 = 0;
  int i = 0;
  int j, y, x, tricks, tdis;
  int bow_to_h = 0;
  int bow_to_d = 0;
  int ranged_skill;
  int catch_chance = 0;

  int style_hit, style_dam, style_crit;

  object_type *k_ptr;
  object_type *i_ptr;
  object_type object_type_body;

  bool ammo_can_break = TRUE;
  bool trick_failure = FALSE;
  bool chasm = FALSE;
  int feat;

  byte missile_attr;
  char missile_char;

  char o_name[80];

  int path_n;
  u16b path_g[256];

  int use_old_target_backup = use_old_target;

  bool throwing = is_throwing_item(o_ptr);
  bool trick_throw = !fire && item == INVEN_WIELD && throwing;
  int num_tricks = trick_throw ? p_ptr->num_blow + 1 : 1;

  /* Need a rope? (No rope for trick throws) */
  if (o_ptr->sval == SV_AMMO_GRAPPLE && !trick_throw)
    {
      cptr q, s;

      /* Allow chain for some weapons */
      if (o_ptr->tval == TV_HAFTED || o_ptr->tval == TV_BOLT) 
	item_tester_tval = TV_ROPE;

      /* Require rope */
      else item_tester_hook = item_tester_hook_rope;

      /* Get an item */
      q = "Attach which rope? ";
      s = "You have no rope to attach.";
      if (get_item(&item2, q, s, (USE_INVEN | USE_FLOOR)))
	{
	  /* Get the object */
	  if (item2 >= 0)
	    {
	      k_ptr = &inventory[item2];
	    }
	  else
	    {
	      k_ptr = &o_list[0 - item2];
	    }

	  /* In a bag? */
	  if (k_ptr->tval == TV_BAG)
	    {
	      /* Get item from bag */
	      if (get_item_from_bag(&item2, q, s, k_ptr))
		{
		  /* Refer to the item */
		  k_ptr = &inventory[item2];
		}
	    }
	}
      else
	{
	  k_ptr = &o_list[0 - item2];
	}
    }
  else
    {
      k_ptr = NULL;
    }

  /* Get local object */
  i_ptr = &object_type_body;

  /* Obtain a local object */
  object_copy(i_ptr, o_ptr);

  /* Single object */
  i_ptr->number = 1;

  /* Reset stack counter */
  i_ptr->stackc = 0;

  /* No longer 'stored' */
  i_ptr->ident &= ~(IDENT_STORE);

  /* Describe the object */
  object_desc(o_name, sizeof(o_name), i_ptr, FALSE, 3);

  /* Find the color and symbol for the object */
  missile_attr = object_attr(i_ptr);
  missile_char = object_char(i_ptr);

  /* The first piece of code dependent on fire/throw */
  if (fire)
    {
      /* Get the bow */
      bow_to_h = inventory[INVEN_BOW].to_h;
      bow_to_d = inventory[INVEN_BOW].to_d;

      ranged_skill = p_ptr->skill_thb;

      /* Base range XXX XXX */
      tdis = 6 + 3 * p_ptr->ammo_mult;
    }
  else
    {
      int mul, div;

      ranged_skill = p_ptr->skill_tht;

      /* Badly balanced big weapons waste the throwing skill.
	 Various junk (non-weapons) does not have to_hit, 
	 so don't penalize a second time. */
      if (!throwing 
	  && (i_ptr->tval == TV_STAFF
	      || i_ptr->tval == TV_BOW 
	      || i_ptr->tval == TV_DIGGING 
	      || i_ptr->tval == TV_HAFTED 
	      || i_ptr->tval == TV_POLEARM 
	      || i_ptr->tval == TV_SWORD))
	ranged_skill /= 2;
      
      /* Extract a "distance multiplier" */
      mul = throwing ? 10 : 3;

      /* Enforce a minimum "weight" of one pound */
      div = i_ptr->weight > 10 ? i_ptr->weight : 10;

      /* Hack -- Distance -- Reward strength, penalize weight */
      tdis = (adj_str_blow[p_ptr->stat_ind[A_STR]] + 20) * mul / div;

      /* Max distance of 10 */
      if (tdis > 10) 
	tdis = 10;
    }

  /* Coordinates of the fired/thrown object; start at the player */
  y = p_ptr->py;
  x = p_ptr->px;

  /* Iterate through trick throw targets;
     the last pass if for returning to player;
     if no tricks, just one iteration */
  for (tricks = 0; tricks < num_tricks; tricks++)
    {
      int dir = 0;
      int old_y, old_x; /* Previous weapon location */
      int ty, tx; /* Current target coordinates */

      /* If trick throw failure, stop trick throws */
      if (trick_failure)
	break;

      /* If too far or all tricks used, return to player; no target query */
      if (trick_throw
	  && (!ammo_can_break 
	      || tricks == num_tricks - 1))
	{
	  tx = p_ptr->px;
	  ty = p_ptr->py;
	}
      else
	{

	  /* Get a direction (or cancel) */
	  tx = x;
	  ty = y;
	  while (tx == x && ty == y)
	    {
	      /* Reset the chosen direction */
	      p_ptr->command_dir = 0;

	      if (!get_aim_dir(&dir))
		{
		  /* Canceled */
		  if (tricks > 0)
		    /* If canceled mid-trick-throw, try to return weapon */
		    {
		      tx = p_ptr->px;
		      ty = p_ptr->py;
		      break;
		    }
		  else
		    /* If canceled before first throw, cancel whole move */
		    {
		      return;
		    }
		}
	      else
		{
		  /* No cancel */
		  
		  /* Check for "target request" */
		  if (dir == 5 && target_okay())
		    {
		      tx = p_ptr->target_col;
		      ty = p_ptr->target_row;
		    }
		  else
		    {
		      /* Predict the "target" location */
		      ty = y + 99 * ddy[dir];
		      tx = x + 99 * ddx[dir];
		    }

		  /* Disable auto-target for the rest of trick shots */
		  use_old_target = FALSE;
		}	  
	    }

	  /* Sound */
	  sound(MSG_SHOOT);
	}

      /* If the weapon returns, don't limit the distance */
      if (tx == p_ptr->px && ty == p_ptr->py)
	tdis = 256;

      /* Calculate the path */
      path_n = project_path(path_g, tdis, y, x, &ty, &tx, 0);

      /* Hack -- Handle stuff */
      handle_stuff();

      /* Save the source of the shot/throw */
      old_y = y;
      old_x = x;

      /* Reset ammo_can_break */
      if (o_ptr->tval == TV_FLASK 
	  || o_ptr->tval == TV_POTION 
	  || o_ptr->tval == TV_EGG) 
	/* Hack -- flasks, potions, spores break as if striking a monster */
	ammo_can_break = TRUE;
      else
	/* Otherwise not breaking by default */
	ammo_can_break = FALSE;

      /* Project along the path */
      for (i = 0; i < path_n; ++i)
	{
	  int msec = op_ptr->delay_factor * op_ptr->delay_factor;

	  int ny = GRID_Y(path_g[i]);
	  int nx = GRID_X(path_g[i]);

	  /* Hack -- Stop before hitting walls */
	  if (!cave_project_bold(ny, nx)) 
	    {
	      /* 1st cause of failure: returning weapon hits a wall */
	      trick_failure = tdis == 256;

	      break;
	    }

	  /* Advance */
	  x = nx;
	  y = ny;

	  /* Handle rope over chasm */
	  if (k_ptr)
	    {
	      /* No worry that rope removed from inventory many times */
	      assert (!trick_throw);

	      feat = cave_feat[y][x];

	      if (f_info[feat].flags2 & FF2_CHASM) 
		chasm = TRUE;
	      else 
		chasm = FALSE;

	      feat = feat_state(feat, FS_SPIKE);
	      
	      if (strstr(f_name + f_info[feat].name, "rope"))
		{
		  /* Hack -- remove spike */
		  feat = feat_state(feat, FS_GET_FEAT);

		  /* MegaHack -- handle chain */
		  if (k_ptr->sval == SV_ROPE_CHAIN) 
		    feat++;

		  /* Change the feature */
		  cave_set_feat(y, x, feat);

		  /* Reduce inventory -- suppress messages */
		  if (item2 >= 0)
		    {
		      if (k_ptr->number == 1)
			{
			  inven_drop_flags(k_ptr);
			  k_ptr = NULL;
			}

		      inven_item_increase(item2, -1);
		      inven_item_optimize(item2);
		    }

		  /* Reduce and describe floor item */
		  else
		    {
		      if (k_ptr->number == 1) 
			k_ptr = NULL;

		      floor_item_increase(0 - item2, -1);
		      floor_item_optimize(0 - item2);
		    }
		}
	    }
	  
	  /* Only do visuals if the player can "see" the missile */
	  if (panel_contains(y, x) && player_can_see_bold(y, x))
	    {
	      /* Visual effects */
	      print_rel(missile_char, missile_attr, y, x);
	      move_cursor_relative(y, x);
	      if (fresh_before) 
		Term_fresh();
	      Term_xtra(TERM_XTRA_DELAY, msec);
	      lite_spot(y, x);
	      if (fresh_before) 
		Term_fresh();
	    }

	  /* Delay anyway for consistency */
	  else
	    {
	      /* Pause anyway, for consistancy */
	      Term_xtra(TERM_XTRA_DELAY, msec);
	    }

	  /* Handle monster */
	  if (cave_m_idx[y][x] > 0)
	    {
	      monster_type *m_ptr = &m_list[cave_m_idx[y][x]];
	      monster_race *r_ptr = &r_info[m_ptr->r_idx];

	      int visible = m_ptr->ml;
	      int bonus;
	      int chance;
	      int chance2;
	      int tdam;

	      bool hit_or_near_miss;
	      bool genuine_hit;
	      
	      /* Ignore hidden monsters */
	      if (m_ptr->mflag & (MFLAG_HIDE)) 
		continue;

	      /* If the weapon returns, ignore monsters */
	      if (tdis == 256)
		continue;

	      /* Base damage from the object */
	      tdam = damroll(i_ptr->dd, i_ptr->ds);

	      /* The second fire/throw dependent code piece */
	      if (fire)
		{
		  u32b shoot_style;

		  /* Boost the damage */
		  tdam *= p_ptr->ammo_mult;

		  /* Some monsters are great at dodging  -EZ- */
		  if (mon_evade(cave_m_idx[y][x], 
				m_ptr->cdis + (m_ptr->confused 
					       || m_ptr->stunned ? 1 : 3),
				5 + m_ptr->cdis,
				" your shot"))
		    continue;

		  /* Check shooting styles only */
		  shoot_style = p_ptr->cur_style & WS_LAUNCHER_FLAGS;

		  /* Get style benefits */
		  mon_style_benefits(m_ptr, shoot_style, 
				     &style_hit, &style_dam, &style_crit);
		}
	      else
		{
		  /* Long throws are easier to dodge than long shots */
		  if (mon_evade(cave_m_idx[y][x], 
				2 * m_ptr->cdis + (m_ptr->confused 
						   || m_ptr->stunned ? 1 : 4),
				5 + 2 * m_ptr->cdis,
				" your throw"))
		    continue;

		  if (throwing)
		    mon_style_benefits(m_ptr, WS_THROWN_FLAGS, 
				       &style_hit, &style_dam, &style_crit);
		  else
		    style_hit = style_dam = style_crit = 0;
		}

	      /* Actually "fire" the object */
	      bonus = (p_ptr->to_h + i_ptr->to_h + bow_to_h + style_hit);
	      chance = ranged_skill + bonus * BTH_PLUS_ADJ;
	      chance2 = chance - distance(old_y, old_x, y, x);

	      /* Record for later */
	      catch_chance = chance;

	      /* Test hit fire */
	      hit_or_near_miss = test_hit_fire(chance2, 
					       calc_monster_ac(m_ptr, FALSE), 
					       m_ptr->ml);

	      /* Genuine hit */
	      genuine_hit = test_hit_fire(chance2, 
					  calc_monster_ac(m_ptr, TRUE), 
					  m_ptr->ml);

	      /* Missiles bounce off resistant monsters */
	      if (genuine_hit && mon_resist_object(cave_m_idx[y][x], i_ptr))
		{
		  /* XXX Rewrite remaining path of missile */

		  continue;
		}

	      /* Did we hit it or get close? */
	      if (hit_or_near_miss || genuine_hit)
		{
		  bool fear = FALSE;
		  bool was_asleep = (m_ptr->csleep == 0);

		  /* Assume a default death */
		  cptr note_dies = " dies.";

		  /* Note the collision */
		  ammo_can_break = TRUE;

		  /* 2nd and last cause of failure: bounce off monster */
		  trick_failure = !genuine_hit;

		  /* Disturb the monster */
		  m_ptr->csleep = 0;

		  /* Mark the monster as attacked by the player */
		  m_ptr->mflag |= MFLAG_HIT_RANGE;

		  /* Some monsters get "destroyed" */
		  if (r_ptr->flags3 & RF3_NONLIVING || r_ptr->flags2 & RF2_STUPID)
		    {
		      /* Special note at death */
		      note_dies = " is destroyed.";
		    }

		  /* Apply special damage XXX XXX XXX */
		  tdam = tot_dam_aux(i_ptr, tdam, m_ptr);

		  /* The third piece of fire/throw dependent code */
		  if (fire)
		    /* Apply missile critical damage */
		    tdam += critical_shot(i_ptr->weight, 
					  bonus + style_crit * 30, 
					  tdam);
		  else if (throwing)
		    /* Throws (with specialized throwing weapons) hit harder */
		    tdam += critical_norm(i_ptr->weight, 
					  bonus + style_crit * 30, 
					  tdam);
		  else
		    /* Throwing non-throwing items gives no criticals */
		    tdam += 0;

		  /* Apply launcher, missile and style bonus */
		  tdam += i_ptr->to_d + bow_to_d + style_dam;
		  
		  /* No negative damage */
		  if (tdam < 0) 
		    tdam = 0;

		  /* Handle unseen monster */
		  if (!visible)
		    {
		      /* Invisible monster */
		      msg_format("The %s finds a mark.", o_name);

		      /* Near miss? */
		      if (!genuine_hit) 
			tdam = 0;
		    }

		  /* Handle visible monster */
		  else
		    {
		      char m_name[80];

		      /* Get "the monster" or "it" */
		      monster_desc(m_name, cave_m_idx[y][x], 0);

		      /* Near miss */
		      if (!genuine_hit)
			{
			  /* Missile was stopped */
			  if (r_ptr->flags2 & (RF2_ARMOR) 
			      || m_ptr->shield) 
			    msg_format("%^s blocks the %s with a %sshield.", 
				       m_name, 
				       o_name, 
				       m_ptr->shield ? "mystic " : "");

			  /* No normal damage */
			  tdam = 0;
			}
		      /* Successful hit */
		      else
			{
			  msg_format("The %s hits %s.", o_name, m_name);
			}

		      /* Hack -- Track this monster race */
		      if (m_ptr->ml) 
			monster_race_track(m_ptr->r_idx);

		      /* Hack -- Track this monster */
		      if (m_ptr->ml) 
			health_track(cave_m_idx[y][x]);
		    }

		  /* Complex message */
		  if (p_ptr->wizard)
		    {
		      msg_format("You do %d (out of %d) damage.",
				 tdam, m_ptr->hp);
		    }

		  /* Hit the monster, check for death */
		  if (mon_take_hit(cave_m_idx[y][x], tdam, &fear, note_dies))
		    {
		      /* Dead monster */
		    }

		  /* No death */
		  else
		    {
		      /* Message */
		      message_pain(cave_m_idx[y][x], tdam);

		      /* Alert fellows */
		      if (was_asleep)
			{
			  m_ptr->mflag |= (MFLAG_AGGR | MFLAG_SNEAKED);

			  /* Let allies know */
			  tell_allies_mflag(m_ptr->fy, m_ptr->fx, MFLAG_AGGR, 
					    "& has attacked me!");
			}
		      else if (fear)
			{
			  tell_allies_mflag(m_ptr->fy, m_ptr->fx, MFLAG_AGGR, 
					    "& has hurt me badly!");
			}

		      /* Take note */
		      if (fear && m_ptr->ml)
			{
			  char m_name[80];

			  /* Get the monster name (or "it") */
			  monster_desc(m_name, cave_m_idx[y][x], 0);

			  /* Message */
			  message_format(MSG_FLEE, m_ptr->r_idx,
					 "%^s flees in terror!", m_name);
			}

		  /* Use coating or sometimes activate item */
		      if (coated_p(i_ptr) || auto_activate(i_ptr))
			{
			  /* Make item strike */
			  process_item_blow(i_ptr, y, x);

			  /* Hack -- Remove coating on original --- FIXME: explain why this is needed */
			  if (!coated_p(i_ptr) 
			      && o_ptr->feeling == INSCRIP_COATED) 
			    o_ptr->feeling = 0;
			}
		    }

		  /* Check usage */
		  object_usage(item);

		  /* Stop looking */
		  break;
		}
	    }
	}
    }

  /* Reenable auto-target */
  use_old_target = use_old_target_backup;

  /* Chance of breakage; trick throws always risky */
  j = ((ammo_can_break || trick_throw) ? breakage_chance(i_ptr) : 0);

  /* The fourth and last piece of code dependent on fire/throw */
  if (fire)
    {
      /* Check usage of the bow */
      object_usage(INVEN_BOW);

      /* Take a turn */
      p_ptr->energy_use = 100 / p_ptr->num_fire;
    }
  else
    {
      int throws_per_round;
      
      if (trick_throw)
	throws_per_round = 1;
      else
	/* This does not depend on whether the weapon is a throwing weapon */
	throws_per_round = p_ptr->num_throw;

      /* Take a turn */
      p_ptr->energy_use = 100 / throws_per_round;
    }

  /* Is this a trick throw and has the weapon returned? */
  if (trick_throw && !trick_failure)
    {
      /* Keep it sane */
      catch_chance = MAX(0, MAX(catch_chance, ranged_skill/5));

      /* Randomize */
      catch_chance = randint(catch_chance);

      if (catch_chance <= 2 + catch_chance / 20)
	/* You don't catch the returning weapon; it hits you */
	{
	  /* TODO: this code is taken from hit_trap in cmd1.c --- factor it out */
	  int k;

	  /* Describe */
	  object_desc(o_name, sizeof(o_name), i_ptr, FALSE, 0);

	  k = damroll(i_ptr->dd, i_ptr->ds);
	  k += critical_norm(i_ptr->weight, 2 * i_ptr->to_h, k);
	  k += i_ptr->to_d;

	  /* Armour reduces total damage */
	  k -= (k * ((p_ptr->ac < 150) ? p_ptr->ac : 150) / 250);

	  /* No negative damage */
	  if (k < 0) k = 0;

	  /* Trap description */
	  msg_format("You catch the returning %^s in your belly!", o_name);

	  /* Damage, check for fear and death */
	  take_hit(k, "a returning weapon");

	  /* Apply additional effect from coating or sometimes activate */
	  if (coated_p(i_ptr) || auto_activate(i_ptr))
	    {
	      /* Make item strike */
	      process_item_blow(i_ptr, y, x);
	    }

	  /* Weapon caught */
	}
      else if (catch_chance <= 10 + catch_chance / 10)
	/* You don't catch the returning weapon; it almost hits you */
	{
	  /* Describe */
	  object_desc(o_name, sizeof(o_name), i_ptr, FALSE, 0);

	  msg_format("The returning %^s narrowly misses you.", o_name);

	  /* Weapon not caught */
	  trick_failure = TRUE;
	}
      else
	{
	  /* Weapon caught */
	}
    }

  /* Is this a trick throw and has the weapon returned? */
  if (trick_throw && !trick_failure)
    /* Try to return the weapon to the player */
    {
      /* Perhaps harm the weapon */
      if (rand_int(100) < j && break_near(i_ptr, y, x))
	/* The returned weapon turned out to be totally broken */
	{
	  /* Nothing more to do */
	}
      else
	/* Either intact or only mildly harmed */
	{
	  /* Wear again the (possibly slighly harmed) weapon */
	  object_copy(o_ptr, i_ptr);

	  /* Recalculate bonuses */
	  p_ptr->update |= (PU_BONUS);

	  /* Window stuff --- TODO: are all these needed? */
	  p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);

	  /* Update item list --- TODO: are all these needed? */
	  p_ptr->redraw |= (PR_ITEM_LIST);
	}
    }
  else
    /* Not a successful trick shot; just drop near the last monster/wall */
    {
      /* Sometimes use lower stack object --- FIXME: clarify comment, search and replace similar comments */
      if (!object_charges_p(o_ptr) && rand_int(o_ptr->number) < o_ptr->stackc)
	{
	  if (i_ptr->charges) 
	    i_ptr->charges--;

	  if (i_ptr->timeout) 
	    i_ptr->timeout = 0;
      
	  o_ptr->stackc--;
	}

      /* Reduce and describe inventory */
      if (item >= 0)
	{
	  if (o_ptr->number == 1)
	    {
	      inven_drop_flags(o_ptr);
	      if (item2 > item) 
		item2--;
	    }

	  inven_item_increase(item, -1);
	  inven_item_describe(item);
	  inven_item_optimize(item);
	}

      /* Reduce and describe floor item */
      else
	{
	  bool get_feat = FALSE;
	  
	  floor_item_increase(0 - item, -1);
	  floor_item_optimize(0 - item);
	  
	  /* Get feat */
	  if (o_ptr->ident & (IDENT_STORE)) 
	    get_feat = TRUE;
	  
	  if (get_feat && scan_feat(p_ptr->py, p_ptr->px) < 0) 
	    cave_alter_feat(p_ptr->py, p_ptr->px, FS_GET_FEAT);
	}

      /* Forget information on dropped object --- FIXME: say why it is needed */
      drop_may_flags(i_ptr);
      
      /* Drop (or break) near that location */
      drop_near(i_ptr, j, y, x);
  
      /* Rope doesn't reach other end of chasm */
      if (chasm)
	{
	  /* Project along the path */
	  for ( ; i >= 0; --i)
	    {
	      y = GRID_Y(path_g[i]);
	      x = GRID_X(path_g[i]);
	      
	      feat = cave_feat[y][x];
	      
	      /* Drop rope into chasm */
	      if (strstr(f_name + f_info[feat].name, "rope") 
		  || strstr(f_name + f_info[feat].name, "chain"))
		{
		  /* Hack -- drop into chasm */
		  cave_alter_feat(y, x, FS_TIMED);
		}
	    }
	}	
    }
}


/*
 * Fire an object from the pack or floor.
 * See do_cmd_fire_selected.
 */
void do_cmd_fire(void)
{
  int item;

  object_type *o_ptr;

  cptr q, s;

  /* Berserk */
  if (p_ptr->shero)
    {
      msg_print("You are too enraged!");
      return;
    }

  /* Some items and some rooms blow missiles around */
  if (p_ptr->cur_flags4 & (TR4_WINDY) 
      || room_has_flag(p_ptr->py, p_ptr->px, ROOM_WINDY))
    {
      msg_print("Its too windy around you!");
      return;
    }

  /* Require proper missile */
  item_tester_tval = p_ptr->ammo_tval;

  /* Require throwing weapon */
  if (!item_tester_tval) 
    item_tester_hook = is_known_throwing_item;

  /* Get an item */
  q = "Fire which item? ";
  s = "You have nothing to fire.";
  if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR | USE_FEATG)))
    return;

  /* Get the object */
  if (item >= 0)
    {
      o_ptr = &inventory[item];

      /* A cursed quiver disables the use of non-cursed ammo */
      if (IS_QUIVER_SLOT(item) && p_ptr->cursed_quiver && !cursed_p(o_ptr))
	{
	  msg_print("Your quiver is cursed!");
	  return;
	}
    }
  else
    {
      o_ptr = &o_list[0 - item];
    }

  /* In a bag? */
  if (o_ptr->tval == TV_BAG)
    {
      /* Get item from bag */
      if (!get_item_from_bag(&item, q, s, o_ptr)) 
	return;

      /* Refer to the item */
      o_ptr = &inventory[item];
    }

  /* Check for launcher */
  if (p_ptr->num_fire)
    /* Launcher wielded, so fire */
    do_cmd_fire_or_throw_selected(o_ptr, item, TRUE);
  else 
    /* No launcher, so throw */
    do_cmd_fire_or_throw_selected(o_ptr, item, FALSE);
}


/*
 * Throw an object from the pack or floor.
 * See do_cmd_throw_selected.
 */
void do_cmd_throw(void)
{
  int item;

  object_type *o_ptr;

  cptr q, s;

  /* Berserk */
  if (p_ptr->shero)
    {
      msg_print("You are too enraged!");
      return;
    }

  /* Some items and some rooms blow missiles around */
  if (p_ptr->cur_flags4 & (TR4_WINDY) 
      || room_has_flag(p_ptr->py, p_ptr->px, ROOM_WINDY))
    {
      msg_print("Its too windy around you!");
      return;
    }

  /* Get an item */
  q = "Throw which item? ";
  s = "You have nothing to throw.";
  if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR | USE_FEATG))) 
    return;

  /* Get the object */
  if (item >= 0)
    {
      o_ptr = &inventory[item];

      /* A cursed quiver disables the use of non-cursed ammo */
      if (IS_QUIVER_SLOT(item) && p_ptr->cursed_quiver && !cursed_p(o_ptr))
	{
	  msg_print("Your quiver is cursed!");
	  return;
	}
    }
  else
    {
      o_ptr = &o_list[0 - item];
    }

  /* In a bag? */
  if (o_ptr->tval == TV_BAG)
    {
      /* Get item from bag */
      if (!get_item_from_bag(&item, q, s, o_ptr)) 
	return;

      /* Refer to the item */
      o_ptr = &inventory[item];
    }

  do_cmd_fire_or_throw_selected(o_ptr, item, FALSE);
}
