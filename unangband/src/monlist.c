/* File: monlist.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 *
 * UnAngband (c) 2001-2009 Andrew Doull. Modifications to the Angband 2.9.1
 * source code are released under the Gnu Public License. See www.fsf.org
 * for current GPL license details. Addition permission granted to
 * incorporate modifications in all Angband variants as defined in the
 * Angband variants FAQ. See rec.games.roguelike.angband for FAQ.
 */

#include "angband.h"

enum {
	MONLIST_GROUP_BY_SEEN,
	MONLIST_GROUP_BY_NOT_SHOOT,
	MONLIST_GROUP_BY_AWARE,
	MONLIST_GROUP_BY_MAX
};

enum {
	MONLIST_SORT_BY_DISTANCE,
	MONLIST_SORT_BY_PROXIMITY,
	MONLIST_SORT_BY_DEPTH,
	MONLIST_SORT_BY_VENGEANCE,
	MONLIST_SORT_BY_MAX
};

const char *sort_by_name[MONLIST_SORT_BY_MAX]=
{
		"distance",
		"proximity",
		"depth",
		"vengeance",
};


typedef struct monlist_type monlist_type;

enum {
MONLIST_NONE = 0, MONLIST_BLANK, MONLIST_MORE, MONLIST_RACE, MONLIST_KIND,
MONLIST_ARTIFACT, MONLIST_FEATURE,

/* The headings */
MONLIST_HEADER_SEEN_MONSTER, MONLIST_HEADER_NOT_SHOOT_MONSTER,
MONLIST_HEADER_AWARE_MONSTER, MONLIST_HEADER_SEEN_OBJECT,
MONLIST_HEADER_NOT_SHOOT_OBJECT,
MONLIST_HEADER_AWARE_OBJECT, MONLIST_HEADER_SEEN_FEATURE,
MONLIST_HEADER_NOT_SHOOT_FEATURE, MONLIST_HEADER_AWARE_FEATURE,

/* The others */
MONLIST_OTHER_SEEN_MONSTER, MONLIST_OTHER_NOT_SHOOT_MONSTER,
MONLIST_OTHER_AWARE_MONSTER, MONLIST_OTHER_SEEN_OBJECT,
MONLIST_OTHER_NOT_SHOOT_OBJECT,
MONLIST_OTHER_AWARE_OBJECT, MONLIST_OTHER_SEEN_FEATURE,
MONLIST_OTHER_NOT_SHOOT_FEATURE, MONLIST_OTHER_AWARE_FEATURE,

MONLIST_MAX
};

#define MONLIST_DISPLAY_MONSTER	0x01
#define MONLIST_DISPLAY_OBJECT	0x02
#define MONLIST_DISPLAY_FEATURE	0x04

struct monlist_type
{
	int row_type;
	int idx;
	int number;
	int len;

	char *text;
	byte attr;

	/* Graphic for row */
	char x_char;
	byte x_attr;

	int closest_y;
	int closest_x;

	monlist_type *next;
};


/*
 * Free a monster list
 */
void free_monlist(monlist_type *monlist)
{
	while (monlist)
	{
		monlist_type *next_monlist = monlist->next;

		if (monlist->text)
		{
			FREE(monlist->text);
		}

		FREE(monlist);

		monlist = next_monlist;
	}
}


/*
 * Copies the buffer to a screen line
 */
void monlist_copy_buffer_to_screen(char *buf, monlist_type *monlist)
{
	/* Allocate space for the buffer */
	monlist->text = C_ZNEW(strlen(buf) + 2, char);
	monlist->len = strlen(buf);

	/* Copy the buffer */
	my_strcpy(monlist->text, buf, strlen(buf));
}


/*
 * Gets the index into the array.
 */
int monlist_get_monster_index(int idx)
{
	/* Get the monster */
	monster_type *m_ptr = &m_list[idx];

	return (m_ptr->r_idx);
}

/*
 * Gets the index into the array.
 */
int monlist_get_object_index(int idx)
{
	/* Get the object */
	object_type *o_ptr = &o_list[idx];

	return (o_ptr->k_idx);
}


/*
 * Checks if a monster should be displayed for this grouping
 */
bool monlist_check_monster_grouping(int idx, int group_by)
{
	/* Get the monster */
	monster_type *m_ptr = &m_list[idx];

	/* Only visible monsters */
	if (!m_ptr->ml) return (FALSE);

	/* Check which type we're collecting */
	if (play_info[m_ptr->fy][m_ptr->fx] & (PLAY_FIRE))
	{
		if (group_by != MONLIST_GROUP_BY_SEEN) return (FALSE);
	}
	else if (play_info[m_ptr->fy][m_ptr->fx] & (PLAY_SEEN))
	{
		if (group_by != MONLIST_GROUP_BY_SEEN) return (FALSE);
	}
	else if (group_by != MONLIST_GROUP_BY_SEEN) return (FALSE);

	return (TRUE);
}


/*
 * Checks if an object should be displayed for this grouping
 *
 * XXX Note we don't care whether we can shoot objects so
 * that grouping will always be empty.
 */
bool monlist_check_object_grouping(int idx, int group_by)
{
	/* Get the monster */
	object_type *o_ptr = &o_list[idx];

	/* Only visible objects */
	if ((o_ptr->ident & (IDENT_MARKED)) == 0) return (FALSE);

	/* Ignore objects carried by monsters */
	if (o_ptr->held_m_idx) return (FALSE);

	/* Check which type we're collecting */
	if (play_info[o_ptr->iy][o_ptr->ix] & (PLAY_SEEN))
	{
		if (group_by != MONLIST_GROUP_BY_SEEN) return (FALSE);
	}
	else if (group_by != MONLIST_GROUP_BY_AWARE) return (FALSE);

	return (TRUE);
}


/*
 * Returns the number that this index contributes
 */
int monlist_get_monster_count(int idx)
{
	(void)idx;

	return (1);
}


/*
 * Returns the number that this index contributes
 */
int monlist_get_object_count(int idx)
{
	/* Get the object */
	object_type *o_ptr = &o_list[idx];

	return (o_ptr->number);
}


/*
 * Checks if a monster should be counted for the secondary index
 */
bool monlist_check_monster_secondary(int idx)
{
	/* Get the monster */
	monster_type *m_ptr = &m_list[idx];

	/* Exclude awake monsters */
	if (!m_ptr->csleep) return (FALSE);

	return (TRUE);
}


/*
 * Checks if an object should be counted for the secondary index
 */
bool monlist_check_object_secondary(int idx)
{
	/* Get the object */
	object_type *o_ptr = &o_list[idx];

	/* Named objects are uninteresting */
	if (object_named_p(o_ptr)) return (FALSE);

	/* Uninteresting objects are uninteresting */
	if (uninteresting_p(o_ptr)) return (FALSE);

	return (TRUE);
}


/*
 * Returns an ordering value for the monster
 */
int monlist_get_monster_order(int idx, int sort_by)
{
	/* Get the monster */
	monster_type *m_ptr = &m_list[idx];

	/* Skip races at the sort distance */
	switch (sort_by)
	{
		case MONLIST_SORT_BY_DISTANCE:
			return (m_ptr->cdis);
		case MONLIST_SORT_BY_PROXIMITY:
			return (100 + r_info[m_ptr->r_idx].aaf - m_ptr->cdis);
		case MONLIST_SORT_BY_VENGEANCE:
			if (l_list[m_ptr->r_idx].deaths) return (l_list[m_ptr->r_idx].deaths + MAX_DEPTH);
			/* Fall through */
		case MONLIST_SORT_BY_DEPTH:
			return (r_info[m_ptr->r_idx].level);
	}

	return (0);
}


/*
 * Returns an ordering value for the object
 */
int monlist_get_object_order(int idx, int sort_by)
{
	/* Get the object */
	object_type *o_ptr = &o_list[idx];

	/* Skip races at the sort distance */
	switch (sort_by)
	{
		case MONLIST_SORT_BY_DISTANCE:
		case MONLIST_SORT_BY_PROXIMITY:
			return (distance(o_ptr->iy, o_ptr->ix, p_ptr->py, p_ptr->px));
		case MONLIST_SORT_BY_DEPTH:
		case MONLIST_SORT_BY_VENGEANCE:
			if (object_named_p(o_ptr)) return (k_info[o_ptr->k_idx].level);
			else return (MAX_DEPTH + o_ptr->tval);
	}

	return (0);
}


/*
 * This creates a new monlist and copies the monster information to it.
 */
monlist_type *monlist_copy_monster_to_screen(int idx, u16b *index_counts, u16b *index2_counts)
{
	/* Get the monster */
	monster_type *m_ptr = &m_list[idx];

	/* Get monster race */
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	/* Get the monster name */
	const char *m_name = r_name + r_ptr->name;

	char buf[80];

	monlist_type *new_monlist;

	/* Get a new monster list */
	new_monlist = C_ZNEW(1, monlist_type);

	/* Display multiple monsters */
	if (index_counts[m_ptr->r_idx] > 1)
	{
		/* Add race count */
		my_strcpy(buf, format("%d %ss", index_counts[m_ptr->r_idx]), sizeof(buf));

		if ((index2_counts[m_ptr->r_idx]) && (index2_counts[m_ptr->r_idx] < index_counts[m_ptr->r_idx]))
		{
			/* Add race count */
			my_strcat(buf, format(" (%d awake)", index_counts[m_ptr->r_idx] - index2_counts[m_ptr->r_idx]), sizeof(buf));
		}
	}
	/* Display single monsters */
	else
	{
		/* Display the entry itself */
		my_strcpy(buf, m_name, sizeof(buf));
	}

	/* Set up the screen line */
	new_monlist->row_type = MONLIST_RACE;
	new_monlist->number = index_counts[m_ptr->r_idx];
	new_monlist->x_attr = r_ptr->x_attr;
	new_monlist->x_char = r_ptr->x_char;

	/* Exactly one - we can target it */
	if (index_counts[m_ptr->r_idx] == 1)
	{
		new_monlist->closest_y = m_ptr->fy;
		new_monlist->closest_x = m_ptr->fx;
	}

	/* Colour text based on wakefulness */
	new_monlist->attr = index2_counts[m_ptr->r_idx] == index_counts[m_ptr->r_idx] ? TERM_SLATE :
		(index2_counts[m_ptr->r_idx] ? TERM_L_WHITE : TERM_WHITE);

	/* Add the buffer */
	monlist_copy_buffer_to_screen(buf, new_monlist);

	/* Return the space taken up */
	return (new_monlist);
}


/*
 * This copies an object to the screen line
 */
monlist_type *monlist_copy_object_to_screen(int idx, u16b *index_counts, u16b *index2_counts)
{
	/* Fake object */
	object_type object_type_body;

	/* Get the fake object */
	object_type *i_ptr = &object_type_body;

	/* Get the object */
	object_type *o_ptr = &o_list[idx];

	char buf[80];

	monlist_type *new_monlist;

	/* Get a new monster list */
	new_monlist = C_ZNEW(1, monlist_type);

	/* Prepare a fake object */
	object_prep(i_ptr, o_ptr->k_idx);

	/* Fake the artifact */
	if (object_named_p(o_ptr) && o_ptr->name1)
	{
		i_ptr->name1 = o_ptr->name1;
	}

	/* Fake the number */
	else
	{
		if (index_counts[o_ptr->k_idx] > 99) i_ptr->number = 99;
		i_ptr->number = index_counts[o_ptr->k_idx];
	}

	/* Describe the object */
	object_desc(buf, sizeof(buf), i_ptr, TRUE, 0);

	/* Add information about how many known */
	if ((index2_counts[o_ptr->k_idx]) && (index2_counts[o_ptr->k_idx] < index_counts[o_ptr->k_idx]))
	{
		/* Add race count */
		my_strcat(buf, format(" (%d unknown)", index2_counts[o_ptr->k_idx]), sizeof(buf));
	}

	/* Set up the screen line */
	new_monlist->row_type = MONLIST_KIND;
	new_monlist->number = index_counts[o_ptr->k_idx];
	new_monlist->x_attr = object_attr(o_ptr);
	new_monlist->x_char = object_char(o_ptr);

	/* Exactly one - we can target it */
	if (index_counts[o_ptr->k_idx] == o_ptr->number)
	{
		new_monlist->closest_y = o_ptr->iy;
		new_monlist->closest_x = o_ptr->ix;
	}

	/* Colour text based on known-ness */
	new_monlist->attr = index2_counts[o_ptr->k_idx] == index_counts[o_ptr->k_idx] ? TERM_WHITE :
		(index2_counts[o_ptr->k_idx] ? TERM_L_WHITE : TERM_SLATE);

	/* Add the buffer */
	monlist_copy_buffer_to_screen(buf, new_monlist);

	/* Return the space taken up */
	return (new_monlist);
}


/*
 * Creates a list of sorted indexes
 */
monlist_type *monlist_sort_index(int sort_by, int max, int idx_max, monlist_type *monlist, bool *intro,
		const char* index_name, int header_offset, int monlist_get_index(int idx), int monlist_get_count(int idx),
		bool monlist_check_grouping(int idx, int group_by), bool monlist_check_secondary(int idx),
		int monlist_get_order(int idx, int sort_by),
		monlist_type *monlist_copy_to_screen(int idx, u16b *index_counts, u16b *index2_counts)
)
{
	int idx;

	int i, j;
	int status_count;
	int max_order;

	char buf[80];

	u16b *index_counts;
	u16b *index2_counts;

	monlist_type *first_monlist;
	monlist_type *last_monlist;

	/* Start of list */
	first_monlist = monlist;
	last_monlist = monlist;

	/* Allocate the counter arrays */
	index_counts = C_ZNEW(idx_max, u16b);
	index2_counts = C_ZNEW(idx_max, u16b);

	/* Go through the groupings for the monster list */
	for (i = 0; i < MONLIST_GROUP_BY_MAX; i++)
	{
		/* Reset status count */
		status_count = 0;
		max_order = 0;

		/* Iterate over index list */
		for (idx = 1; idx < max; idx++)
		{
			/* Not in this grouping */
			if (!monlist_check_grouping(idx, i)) continue;

			/* Bump the count for this index */
			index_counts[monlist_get_index(idx)] += monlist_get_count(idx);

			/* Bump the secondary count if qualifies */
			if (monlist_check_secondary(idx)) index2_counts[monlist_get_index(idx)] += monlist_get_count(idx);

			/* We have an index */
			status_count++;

			/* Efficiency - Get maximum sort by */
			max_order = MAX(max_order, monlist_get_order(idx, sort_by));
		}

		/* No visible monsters */
		if (!status_count) continue;

		/* Get a new monlist */
		monlist = C_ZNEW(1, monlist_type);

		/* Set up the buffer */
		my_strcpy(buf, format("You %s%s %d %s%s:%s",
			(i < 2) ? "can see" : "are aware of", (i == 1) ? " but not shoot" : "",
			status_count, index_name, (status_count > 1 ? "s" : ""),
			*intro ? format(" (by %s)", sort_by_name[sort_by]) : ""), sizeof(buf));

		/* Copy the buffer */
		monlist_copy_buffer_to_screen(buf, monlist);

		/* Set up the screen line */
		monlist->row_type = MONLIST_HEADER_SEEN_MONSTER + header_offset * MONLIST_GROUP_BY_MAX + i;
		monlist->number = status_count;
		monlist->attr = TERM_WHITE;

		/* Head of list? */
		if (!first_monlist)
		{
			first_monlist = monlist;
		}
		/* Add as child */
		else
		{
			last_monlist->next = monlist;
		}

		/* Thread it */
		last_monlist = monlist;

		/* Iterate through sort */
		for (j = sort_by ? max_order : 0; (sort_by ? j >= 0 : j <= max_order); sort_by ? j-- : j++)
		{
			/* Iterate over mon_list ( again :-/ ) */
			for (idx = 1; idx < max; idx++)
			{
				/* Do each race only once */
				if (!index_counts[monlist_get_index(idx)]) continue;

				/* Check the monster is valid */
				if (!monlist_check_grouping(idx, j)) continue;

				/* Copy to the screen */
				monlist = monlist_copy_to_screen(idx, index_counts, index2_counts);

				/* Add as child */
				last_monlist->next = monlist;

				/* Thread it */
				last_monlist = monlist;

				/* Don't display again */
				index_counts[monlist_get_index(idx)] = 0;
				index2_counts[monlist_get_index(idx)] = 0;
			}
		}

		/* Get a new monlist */
		monlist = C_ZNEW(1, monlist_type);

		/* Set up the screen line */
		monlist->row_type = MONLIST_BLANK;

		/* Add as child */
		last_monlist->next = monlist;

		/* Thread it */
		last_monlist = monlist;

		/* Introduced */
		*intro = FALSE;
	}

	/* Free the counters */
	FREE(index_counts);
	FREE(index2_counts);

	return (first_monlist);
}


#if 0
/* Display the list */
bool display_monlist_rows(monlist_type *monlist, int row, int line, int width, bool force)
{
	int i;

	/* If displaying on the terminal using the '[' command, recenter on the player,
	 * taking away the used up width on the left hand side. Otherwise, recenter when
	 * the player walks next to the display box. */
	if ((Term == angband_term[0]) && ((signed)width < SCREEN_WID) &&
			(((force) && !(center_player)) ||
			((p_ptr->px - p_ptr->wx <= (signed)width + 1) && (p_ptr->py - p_ptr->wy <= (signed)line + 1))))
	{
		screen_load();

		/* Use "modify_panel" */
		if (modify_panel(p_ptr->py - (SCREEN_HGT) / 2, p_ptr->px - (SCREEN_WID + width) / 2))
		{
			/* Force redraw */
			redraw_stuff();
		}

		/* Unable to place the player */
		if ((!force) && (p_ptr->px - p_ptr->wx <= (signed)width + 1) && (p_ptr->py - p_ptr->wy <= (signed)line + 1)) return (FALSE);

		screen_save();
	}

	/* Print the display */
	for (i = row; i < line; i++)
	{
		/* Display the line */
		c_prt(monlist->attr, monlist->text, row + i, 0);

		/* Get the next in the list */
		monlist = monlist->next;
	}

	return (TRUE);
}


/* Interact with the list */
key_event display_monlist_interact(monlist_type *monlist, int row, int line, int width, bool *done, bool force)
{
	/* Get an acceptable keypress. */
	key_event ke = force ? anykey() : inkey_ex();

	while ((ke.key == '\xff') && !(ke.mousebutton))
	{
		int y = KEY_GRID_Y(ke);
		int x = KEY_GRID_X(ke);

		int room = dun_room[p_ptr->py/BLOCK_HGT][p_ptr->px/BLOCK_WID];

		ke = target_set_interactive_aux(y, x, &room, TARGET_PEEK, (use_mouse ? "*,left-click to target, right-click to go to" : "*"));
	}

	/* Tried a command - avoid rest of list */
	if (ke.key != ' ')
	{
		*done = TRUE;
	}

	return (ke);
}


/*
 * Displays a list of sorted indexes
 */
key_event monlist_display_indexes(int sort_by, int max, int idx_max, int *total_count, int row, int *line, int *width,
		const char* index_name, int header_offset, bool *intro, bool *done, bool force, monlist_type *monlist_screen,
		int monlist_get_index(int idx), int monlist_get_count(int idx), bool monlist_check_grouping(int idx, int group_by),
		bool monlist_check_secondary(int idx), int monlist_get_order(int idx, int sort_by),
		int monlist_copy_to_screen(int idx, int line, monlist_type *monlist_screen, u16b *index_counts, u16b *index2_counts)
)
{
	int idx;

	int i, j, k;
	int status_count;
	int max_order;

	int disp_count = 0;

	char buf[80];

	u16b *index_counts;
	u16b *index2_counts;

	key_event ke;

	/* Allocate the counter arrays */
	index_counts = C_ZNEW(idx_max, u16b);
	index2_counts = C_ZNEW(idx_max, u16b);

	/* Go through the groupings for the monster list */
	for (i = 0; !*done && (i < MONLIST_GROUP_BY_MAX); i++)
	{
		/* Reset status count */
		status_count = 0;
		max_order = 0;

		/* Iterate over index list */
		for (idx = 1; idx < max; idx++)
		{
			/* Not in this grouping */
			if (!monlist_check_grouping(idx, i)) continue;

			/* Bump the count for this index */
			index_counts[monlist_get_index(idx)] += monlist_get_count(idx);

			/* Bump the secondary count if qualifies */
			if (monlist_check_secondary(idx)) index2_counts[monlist_get_index(idx)] += monlist_get_count(idx);

			/* Increase the counters */
			*total_count += monlist_get_count(idx);

			/* We have an index */
			status_count++;

			/* Efficiency - Get maximum sort by */
			max_order = MAX(max_order, monlist_get_order(idx, sort_by));
		}

		/* No visible monsters */
		if (!status_count) continue;

		/* Set up the buffer */
		my_strcpy(buf, format("You %s%s %d %s%s:%s",
			(i < 2) ? "can see" : "are aware of", (i == 1) ? " but not shoot" : "",
			status_count, index_name, (status_count > 1 ? "s" : ""),
			*intro ? format(" (by %s)", sort_by_name[sort_by]) : ""), sizeof(buf));

		/* Copy the buffer */
		*width = MAX(*width, monlist_copy_buffer_to_screen(buf, monlist_screen, *line));

		/* Set up the screen line */
		monlist_screen[*line].row_type = MONLIST_HEADER_SEEN_MONSTER + header_offset * MONLIST_GROUP_BY_MAX + i;
		monlist_screen[*line].number = status_count;
		monlist_screen[*line].attr = TERM_WHITE;

		/* Increase line number */
		line++;

		/* Iterate through sort */
		for (j = sort_by ? max_order : 0; !*done && (sort_by ? j >= 0 : j <= max_order); sort_by ? j-- : j++)
		{
			/* Iterate over mon_list ( again :-/ ) */
			for (idx = 1; !*done && idx < max && (*line < max); idx++)
			{
				/* Do each race only once */
				if (!index_counts[monlist_get_index(idx)]) continue;

				/* Check the monster is valid */
				if (!monlist_check_grouping(idx, j)) continue;

				/* Copy to the screen */
				monlist_copy_to_screen(idx, *line, monlist_screen, index_counts, index2_counts);

				/* Increase line number */
				*line++;

				/* Add to monster counter */
				disp_count += index_counts[monlist_get_index(idx)];

				/* Don't display again */
				index_counts[monlist_get_index(idx)] = 0;
				index2_counts[monlist_get_index(idx)] = 0;

				/* Page wrap */
				if ((*line == max) && (disp_count != *total_count))
				{
					/* Format the others */
					my_strcpy(buf, "-- more --", sizeof(buf));

					/* Copy the buffer */
					*width = MAX(*width, monlist_copy_buffer_to_screen(buf, monlist_screen, *line));

					/* Set up the screen line */
					monlist_screen[*line].row_type = MONLIST_MORE;
					monlist_screen[*line].attr = TERM_WHITE;

					/* Increase line number */
					*line++;

					/* Display the list */
					if (display_monlist_rows(monlist_screen, row, *line, *width, force))
					{
						*done = TRUE;
					}

					/* Interact with the list */
					if (Term == angband_term[0])
					{
						ke = display_monlist_interact(monlist_screen, row, *line, *width, done, force);
					}

					/* Free strings - except header */
					for (k = row + *done ? 0 : 1; k < *line; k++)
					{
						FREE(monlist_screen[*line].text);
					}

					/* Reset */
					*line = row + 1;
					width = 0;

					/* Finished - reload the screen */
					if (*done)
					{
						screen_load();
						break;
					}
				}
			}
		}

		/* Others to be displayed */
		if (!*done)
		{
			/* Print "and others" message if we're out of space */
			if (disp_count != *total_count)
			{
				/* Format the others */
				my_strcpy(buf, format("  ...and %d others.", total_count - disp_count), sizeof(buf));

				/* Copy the buffer */
				*width = MAX(*width, monlist_copy_buffer_to_screen(buf, monlist_screen, *line));

				/* Set up the screen line */
				monlist_screen[*line].row_type = MONLIST_OTHER_SEEN_MONSTER + header_offset * MONLIST_GROUP_BY_MAX + i;
				monlist_screen[*line].number = *total_count - disp_count;
				monlist_screen[*line].attr = TERM_WHITE;

				/* We've displayed the others */
				disp_count = *total_count;
			}

			/* Put a shadow */
			else
			{
				monlist_screen[*line].row_type = MONLIST_BLANK;
			}

			/* Increase line number */
			line++;
		}

		/* Introduced? */
		*intro = FALSE;
	}

	/* Free the counters */
	FREE(index_counts);
	FREE(index2_counts);

	return (ke);
}
#endif



/*
 * Display visible monsters and/or objects in a window
 *
 * Row is the row to start displaying the list from.
 * Command indicates we return the selected command.
 * Force indicates we're not doing this from the easy_monlist option (should
 * probably rename this).
 *
 * op_ptr->monlist_display defines whether we see monsters, objects or both:
 *
 * 1 	- monsters
 * 2	- objects
 * 3	- both
 *
 * op_ptr->monlist_sort_by shows which way we have sorted this:
 *
 * 0	- distance
 * 1	- depth
 * 2	- number of times your ancestor has been killed
 *
 * Returns the width of the monster and/or object lists, or 0 if no monsters/objects are seen.
 *
 */
void display_monlist(int row, bool command, bool force)
{
	int line = row, max_row;

	monlist_type *monlist = NULL;

	int sort_by = op_ptr->monlist_sort_by;

	bool intro = TRUE;
	bool done = FALSE;

	key_event ke;

	/* Hack -- initialise for the first time */
	if (!op_ptr->monlist_display)
	{
		op_ptr->monlist_display = (MONLIST_DISPLAY_MONSTER) | (MONLIST_DISPLAY_OBJECT);
		sort_by = MONLIST_SORT_BY_VENGEANCE;
	}

	/* Clear the term if in a subwindow, set x otherwise */
	if (Term != angband_term[0])
	{
		clear_from(0);
		max_row = Term->hgt - 1;
	}
	else
	{
		max_row = Term->hgt - 2;

		screen_save();
	}

	/* If hallucinating, we can't see any monsters */
	if (p_ptr->timed[TMD_IMAGE])
	{
		Term_putstr(0, line, 36, TERM_ORANGE, "You're too confused to see straight! ");
		return;
	}

	/*
	 * Iterate multiple times. We put monsters we can project to in the first list, then monsters we can see,
	 * then monsters we are aware of through other means.
	 */
	if (op_ptr->monlist_display % MONLIST_DISPLAY_MONSTER)
	{
		/* Add to the list */
		monlist = monlist_sort_index(sort_by, z_info->m_max, z_info->r_max, monlist, &intro, "monster", 0,
				monlist_get_monster_index, monlist_get_monster_count, monlist_check_monster_grouping,
				monlist_check_monster_secondary, monlist_get_monster_order, monlist_copy_monster_to_screen);

#if 0
		/* Hack -- no ancestor deaths - demote difficulty */
		if ((sort_by == MONLIST_SORT_BY_VENGEANCE) && (max_order <= MAX_DEPTH))
		{
			sort_by = MONLIST_SORT_BY_DEPTH;

			/* We've used the '[' command -- apply this to the character options */
			if (force) op_ptr->monlist_sort_by = MONLIST_SORT_BY_DEPTH;
		}
#endif
	}

	/* Display items */
	if ((!done) && (op_ptr->monlist_display & (MONLIST_DISPLAY_OBJECT)))
	{
		/* Add to the list */
		monlist = monlist_sort_index(sort_by, z_info->o_max, z_info->k_max, monlist, &intro, "object", 0,
				monlist_get_object_index, monlist_get_object_count, monlist_check_object_grouping,
				monlist_check_object_secondary, monlist_get_object_order, monlist_copy_object_to_screen);
	}
#if 0
	/* Display features */
	if ((!done) && (op_ptr->monlist_display & (MONLIST_DISPLAY_FEATURE)))
	{
		/* Display the objects */
		monlist = monlist_sort_index(sort_by, z_info->o_max, z_info->k_max, monlist, &intro, "object", 0,
				monlist_get_object_index, monlist_get_object_count, monlist_check_object_grouping,
				monlist_check_object_secondary, monlist_get_object_order, monlist_copy_object_to_screen);
	}
#endif

#if 0
	/* Display the list */
	if (!done && display_monlist_rows(monlist_screen, row, line, width, force))
	{
		done = FALSE;
	}

	/* Interact with the list */
	if (!done && Term == angband_term[0])
	{
		ke = display_monlist_interact(monlist_screen, row, line, width, &done, force);

		/* Finished - reload the screen */
		if (done)
		{
			screen_load();
		}
	}
#endif
	/* Notice nothing */
	if (!monlist)
	{
		prt(format("You see no %s%s%s%s%s. (by %s)",
				op_ptr->monlist_display & (MONLIST_DISPLAY_MONSTER) ? "monsters" : "",
				(op_ptr->monlist_display & (MONLIST_DISPLAY_OBJECT | MONLIST_DISPLAY_MONSTER)) ==
					(MONLIST_DISPLAY_OBJECT | MONLIST_DISPLAY_MONSTER) ?" or " : "",
				op_ptr->monlist_display & (MONLIST_DISPLAY_OBJECT) ? "objects" : "",
				(op_ptr->monlist_display & (MONLIST_DISPLAY_OBJECT | MONLIST_DISPLAY_FEATURE)) ==
					(MONLIST_DISPLAY_OBJECT | MONLIST_DISPLAY_FEATURE) ?" or " : "",
				op_ptr->monlist_display & (MONLIST_DISPLAY_OBJECT) ? "notable features" : "",
				sort_by_name[sort_by]), row, 0);
	}

	if (Term == angband_term[0])
	{
		/* Get an acceptable keypress. */
		ke = force ? anykey() : inkey_ex();

		while ((ke.key == '\xff') && !(ke.mousebutton))
		{
			int y = KEY_GRID_Y(ke);
			int x = KEY_GRID_X(ke);

			int room = dun_room[p_ptr->py/BLOCK_HGT][p_ptr->px/BLOCK_WID];

			ke = target_set_interactive_aux(y, x, &room, TARGET_PEEK, (use_mouse ? "*,left-click to target, right-click to go to" : "*"));
		}

		/* Reload the screen */
		screen_load();
	}

	/* Display command prompt */
	if (command)
	{
		/*Term_putstr(0, 0, -1, TERM_WHITE, "Command:");*/

		/* Requeue command just pressed */
		p_ptr->command_new = ke;

		/* Hack -- Process "Escape"/"Spacebar"/"Return" */
		if ((p_ptr->command_new.key == ESCAPE) ||
			/*(p_ptr->command_new.key == ' ') ||*/
			(p_ptr->command_new.key == '\r') ||
			(p_ptr->command_new.key == '\n'))
		{
			/* Reset stuff */
			p_ptr->command_new.key = 0;
		}
	}
}



