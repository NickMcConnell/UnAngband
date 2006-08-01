/* File: cmd5.c */

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
 * Allow user to choose a spell/prayer from the given book.
 *
 * If a valid spell is chosen, saves it in '*sn' and returns TRUE
 * If the user hits escape, returns FALSE, and set '*sn' to -1
 * If there are no legal choices, returns FALSE, and sets '*sn' to -2
 *
 * The "prompt" should be "cast", "recite", or "study"
 * The "known" should be TRUE for cast/pray/sing, FALSE for study if
 * a book (eg magic book, prayer book, song book, runestone).
 * For all others, "known" should be TRUE if the player is to be
 * prompted to select an object power, or FALSE if a random power is
 * to be chosen.
 *
 * Now also allows scrolls, potions etc. We do not require mana or
 * ability to read to use any of these.
 */
int get_spell(int *sn, cptr prompt, object_type *o_ptr, bool known)
{
	int i,ii;

	int num = 0;

	int spell = 0;

	int tval;

	s16b book[26];

	bool verify;


	int okay = 0;

	bool flag, redraw;
	key_event ke;

	spell_type *s_ptr;
	spell_cast *sc_ptr = &(s_info[0].cast[0]);

	char out_val[160];

	cptr p;

	bool cast = FALSE;

	/* Get fake tval */
	if (o_ptr->tval == TV_STUDY) tval = o_ptr->sval;
	else tval = o_ptr->tval;

	/* Spell */
	switch (tval)
	{
		case TV_PRAYER_BOOK:
			p="prayer";
			cast = TRUE;
			break;

		case TV_SONG_BOOK:
			p="song";
			cast = TRUE;
			break;

		case TV_MAGIC_BOOK:
			p="spell";
			cast = TRUE;
			break;

		case TV_RUNESTONE:
			p="pattern";
			cast = TRUE;
			break;

		default:
			p="power";
			break;
	}

	/* Cannot cast spells if illiterate */
	if ((cast) &&(c_info[p_ptr->pclass].spell_first > PY_MAX_LEVEL))
	{
		msg_print("You cannot read books or runestones.");

		return(-2);

	}

	/* Hack -- coated objects use coating instead */
	if (coated_p(o_ptr))
	{
		object_type object_type_body;
		object_type *j_ptr = &object_type_body;

		j_ptr->tval = o_ptr->xtra1;
		j_ptr->sval = o_ptr->xtra2;
		j_ptr->k_idx = lookup_kind(o_ptr->xtra1, o_ptr->xtra2);
		o_ptr = j_ptr;
	}

#ifdef ALLOW_REPEAT

	/* Get the spell, if available */
	if (repeat_pull(sn))
	{
		/* Verify the spell */
		if (!(cast) || (spell_okay(*sn, known)))
		{
			/* Success */
			return (TRUE);
		}
	}

#endif /* ALLOW_REPEAT */

	/* Fill the book with spells */
	fill_book(o_ptr,book,&num);

	/* Assume no usable spells */
	okay = 0;

	/* Assume no spells available */
	(*sn) = -2;

	/* Check for "okay" spells */
	if (cast) for (i = 0; i < num; i++)
	{
		/* Look for "okay" spells */
		if (spell_okay(book[i], known)) okay = TRUE;
	}
	/* Get a random spell */
	else if (!known)
	{
		/* Get a random spell */
		*sn = book[rand_int(num)];

#ifdef ALLOW_REPEAT
		repeat_push(*sn);
#endif /* ALLOW_REPEAT */

		/* Something happened */
		return (TRUE);
	}
	else okay = TRUE;

	/* No "okay" spells */
	if (!okay) return (FALSE);

	/* Assume cancelled */
	*sn = (-1);

	/* Nothing chosen yet */
	flag = FALSE;

	/* No redraw yet */
	redraw = FALSE;

	/* Option -- automatically show lists */
	if (auto_display_lists)
	{
		/* Show list */
		redraw = TRUE;

		/* Save screen */
		screen_save();

		/* Display a list of spells */
		print_spells(book, num, 1, 20);
	}

	/* Build a prompt (accept all spells) */
	strnfmt(out_val, 78, "(%^ss %c-%c, *=List, ESC=exit) %^s which %s? ",
	p, I2A(0), I2A(num - 1), prompt, p);

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
			else if (cast)
			{
				/* Show list */
				redraw = TRUE;

				/* Save screen */
				screen_save();

				/* Display a list of spells */
				print_spells(book, num, 1, 20);
			}
			
			/* Show the list */
			else
			{
				/* Show list */
				redraw = TRUE;

				/* Save screen */
				screen_save();

				/* Display a list of spells */
				print_powers(book, num, 1, 20);
			}

			/* Ask again */
			continue;
		}

		choice = ke.key;

		/* Note verify */
		verify = (isupper(choice) ? TRUE : FALSE);

		/* Lowercase 1+*/
		choice = tolower(choice);

		/* Extract request */
		i = (islower(choice) ? A2I(choice) : -1);

		/* Totally Illegal */
		if ((i < 0) || (i >= num))
		{
			bell("Illegal spell choice!");
			continue;
		}

		/* Save the spell index */
		spell = book[i];

		/* Require "okay" spells */
		if ((cast) && (!spell_okay(spell, known)))
		{
			bell("Illegal spell choice!");
			msg_format("You may not %s that %s.", prompt, p);
			continue;
		}


		/* Verify it */
		if (verify)
		{
			char tmp_val[160];

			/* Get the spell */
			s_ptr = &s_info[spell];

			if (cast)
			{
				/* Get the spell details */
				for (ii=0;ii<MAX_SPELL_CASTERS;ii++)
				{
					if (s_ptr->cast[ii].class == p_ptr->pclass)
					{
						sc_ptr=&(s_ptr->cast[ii]);
					}
				}

				/* Prompt */
				strnfmt(tmp_val, 78, "%^s %s (%d mana, %d%% fail)? ",
				prompt, s_name + s_ptr->name,
				sc_ptr->mana, spell_chance(spell));
			}
			else
			{
				/* Prompt */
				strnfmt(tmp_val, 78, "%^s %s)? ",
				prompt, s_name + s_ptr->name);
			}

			/* Belay that order */
			if (!get_check(tmp_val)) continue;
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
	if (!flag) return (FALSE);

	/* Save the choice */
	(*sn) = spell;

#ifdef ALLOW_REPEAT

	repeat_push(*sn);

#endif /* ALLOW_REPEAT */

	/* Success */
	return (TRUE);
}


/* Note this routine is simple, but horribly inefficient due 
   to the (1st iteration) design of the data structures */
bool inven_book_okay(const object_type *o_ptr)
{
	int i,ii,iii;

	spell_type *s_ptr;

	if ((o_ptr->tval != TV_MAGIC_BOOK) &&
  	  (o_ptr->tval != TV_PRAYER_BOOK) &&
  	  (o_ptr->tval != TV_RUNESTONE) &&
  	  (o_ptr->tval != TV_SONG_BOOK) &&
	  (o_ptr->tval != TV_STUDY)) return (0);

	
	/* Study notes */
	if (o_ptr->tval == TV_STUDY)
	{
		s_ptr = &s_info[o_ptr->pval];

		for (i=0;i<MAX_SPELL_CASTERS;i++)
		{
			if (s_ptr->cast[i].class == p_ptr->pclass) return (1);
		}
	}

	/* Book / runestone */
	else for (i=0;i<z_info->s_max;i++)
	{
		s_ptr=&s_info[i];

		for (ii=0;ii<MAX_SPELL_APPEARS;ii++)
		{
			if ((s_ptr->appears[ii].tval == o_ptr->tval) &&
			    (s_ptr->appears[ii].sval == o_ptr->sval))
			{
				for (iii=0;iii<MAX_SPELL_CASTERS;iii++)
				{
					if (s_ptr->cast[iii].class == p_ptr->pclass) return(1);
				}
			}
		}

	}			

	return (0);

}


/*
 * Peruse the spells/prayers in a Book
 *
 * Note that *all* spells in the book are listed
 *
 * Note that browsing is allowed while confused or blind,
 * and in the dark, primarily to allow browsing in stores.
 */
void do_cmd_browse(void)
{
	int item, sval;

	int num = 0;

	s16b book[26];

	object_type *o_ptr;

	cptr p, q, s;

	int spell=-1;

	int i;

	int tval;

	char choice = 0;

	char out_val[160];


	/* Cannot browse books if illiterate */
	if (c_info[p_ptr->pclass].spell_first > PY_MAX_LEVEL)
	{
		msg_print("You cannot read books or runestones.");

		return;
	}

#if 0

	/* No lite */
	if (p_ptr->blind || no_lite())
	{
		msg_print("You cannot see!");
		return;
	}

	/* Confused */
	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}

	/* Amnesia */
	if (p_ptr->amnesia)
	{
		msg_print("You have forgotten all your spells!");
		return;
	}
#endif

	item_tester_hook = inven_book_okay;

	/* Get an item */
	q = "Browse which book or runestone? ";
	s = "You have no books or runestones that you can read.";
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

	/* Get the item's sval */
	sval = o_ptr->sval;

	/* Track the object kind */
	object_kind_track(o_ptr->k_idx);

	/* Hack -- Handle stuff */
	handle_stuff();

	/* Get fake tval */
	if (o_ptr->tval == TV_STUDY) tval = o_ptr->sval;
	else tval = o_ptr->tval;

	/* Spell */
	switch (tval)
	{
		case TV_PRAYER_BOOK:
			p="prayer";
			break;

		case TV_SONG_BOOK:
			p="song";
			break;

		case TV_MAGIC_BOOK:
			p="spell";
			break;

		case TV_RUNESTONE:
			p="pattern";
			break;

		default:
			p="power";
			break;
	}

	/* Fill book with spells */
	fill_book(o_ptr,book,&num);

	/* Paranoia */
	if (num == 0)
	{
		msg_format("There are no %ss to browse.",p);
		return;
	}

	/* Save screen */
	screen_save();

	/* Display the spells */
	print_spells(book, num, 1, 20);

	/* Build a prompt (accept all spells) */
	strnfmt(out_val, 78, "(%^ss %c-%c, ESC=exit) Browse which %s? ",
		p, I2A(0), I2A(num - 1), p);

	/* Get a spell from the user */
	while ((choice != ESCAPE) && get_com(out_val, &choice))
	{
		/* Lowercase 1+*/
		choice = tolower(choice);

		/* Extract request */
		i = (islower(choice) ? A2I(choice) : -1);

		if ((i >= 0) && (i < num))
		{
			int ii;
			bool legible = FALSE;

			spell_type *s_ptr;

			spell_cast *sc_ptr = &(s_info[0].cast[0]);

			/* Save the spell index */
			spell = book[i];

			/* Load screen */
			screen_load();

			/* Save screen */
			screen_save();

			/* Display the spells */
			print_spells(book, num, 1, 20);

			/* Begin recall */
			Term_gotoxy(0, 1);

			/* Get the spell */
			s_ptr = &s_info[spell];

			/* Get casting information */
			for (ii=0;ii<MAX_SPELL_CASTERS;ii++)
			{
				if (s_ptr->cast[ii].class == p_ptr->pclass)
				{
					legible = TRUE;
					sc_ptr=&(s_ptr->cast[ii]);
				}
			}

			/* Spell is illegible */
			if (!legible)
			{
				msg_format("You cannot read that %s.",p);

				/* Build a prompt (accept all spells) */
				strnfmt(out_val, 78, "(%^ss %c-%c, ESC=exit) Browse which %s? ",
					p, I2A(0), I2A(num - 1), p);

			}
			else
			{
				bool intro = FALSE;

				/* Set text_out hook */
				text_out_hook = text_out_to_screen;

				/* Recall spell */
				spell_desc(&s_info[spell],"When cast, it ",spell_power(spell), TRUE, 1);

				text_out(".\n");

				/* Display pre-requisites */
				for (i = 0; i < MAX_SPELL_PREREQUISITES; i++)
				{
					/* Check if pre-requisite spells */
					if (s_info[spell].preq[i])
					{
						if (!intro) text_out_c(TERM_VIOLET,"You must learn ");
						else text_out_c(TERM_VIOLET, " or ");

						intro = TRUE;

						text_out_c(TERM_VIOLET, s_name + s_info[s_info[spell].preq[i]].name);
					}

				}

				/* Terminate if required */
				if (intro) text_out_c(TERM_VIOLET, format(" before studying this %s.\n",p));

				/* Build a prompt (accept all spells) */
				strnfmt(out_val, 78, "The %s of %s. (%c-%c, ESC) Browse which %s:",
					p, s_name + s_info[spell].name,I2A(0), I2A(num - 1), p);
			}

			continue;
		}

	}

	/* Prompt for a command */
	put_str("(Browsing) Command: ", 0, 0);

	/* Hack -- Get a new command */
	p_ptr->command_new = inkey();

	/* Load screen */
	screen_load();

	/* Hack -- Process "Escape" */
	if (p_ptr->command_new == ESCAPE)
	{
		/* Reset stuff */
		p_ptr->command_new = 0;
	}
}


/*
 * Print a list of fields (for research).
 */
void print_fields(const s16b *sn, int num, int y, int x)
{
	int i;

	char out_val[160];
#if 0
	/* Title the list */
	prt("", y, x);
#endif
	/* Dump the fields */
	for (i = 0; i < num; i++)
	{
		/* Dump the spell -- skip 'of ' if required */
		sprintf(out_val, "  %c) %-75s ",
			I2A(i), k_name + k_info[sn[i]].name + (k_info[sn[i]].tval == TV_RUNESTONE ? 0 : 3));
		c_prt(TERM_WHITE, out_val, y + i, x);
	}

	/* Clear the bottom line */
	prt("", y + i, x);
}


/*
 * Study a book to gain a new spell/prayer
 */
void do_cmd_study(void)
{
	int i, item;

	int spell = -1;

	cptr p, q, r, s;

	cptr u = " book";

	object_type *o_ptr;

	spell_type *s_ptr;

	int tval;

	int max_spells = PY_MAX_SPELLS;

	object_type object_type_body;

	/* Cannot cast spells if illiterate */
	if (c_info[p_ptr->pclass].spell_first > 50)
	{
		msg_print("You cannot read books or runestones.");

		return;
	}

	if (p_ptr->blind || no_lite())
	{
		msg_print("You cannot see!");
		return;
	}

	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}

	/* Amnesia */
	if (p_ptr->amnesia)
	{
		msg_print("You have forgotten how to read!");
		return;
	}

	if (!(p_ptr->new_spells))
	{
		msg_format("You cannot learn anything new!");
		return;
	}

	/* Restrict choices to "useful" books */
	item_tester_hook = inven_book_okay;

	/* Get an item */
	q = "Study which book or runestone? ";
	s = "You have no books or runestones that you can read.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR | USE_FEATU))) return;

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

	/* Track the object kind */
	object_kind_track(o_ptr->k_idx);

	/* Hack -- Handle stuff */
	handle_stuff();

	/* Get fake tval */
	if (o_ptr->tval == TV_STUDY)
	{
		tval = o_ptr->sval;
		u = "";
	}
	else tval = o_ptr->tval;

	/* Spell */
	switch (tval)
	{
		case TV_PRAYER_BOOK:
			p="prayer";
			r="Pray for which blessing";
			break;

		case TV_SONG_BOOK:
			p="song";
			r="Improvise which melody";
			break;

		case TV_MAGIC_BOOK:
			p="spell";
			r="Research which field";
			break;

		case TV_RUNESTONE:
			p="pattern";
			r="Engrave which pattern";
			u=" combination of runes";
			break;

		default:
			p="power";
			r = "";
			break;
	}

	/* Study materials -- Choose spells in a book related to the current spell */
	if (o_ptr->tval == TV_STUDY)
	{
		s16b field[MAX_SPELL_APPEARS];

		int num = 0;

		int selection = 0;

		/* Get the spell */
		s_ptr = &s_info[o_ptr->pval];

		/* Pick a new spell item */
		for (i = 0; i < MAX_SPELL_APPEARS; i++)
		{
			if (s_ptr->appears[i].tval == tval) field[num++] = lookup_kind(tval, s_ptr->appears[i].sval);
		}

		/* Paranoia */
		if (!num) return;

		/* Display the list and get a selection */
		if (get_list(print_fields, field, num, format("%^ss",p), r, 1, 20, &selection))
		{
			/* Fake the o_ptr */
			o_ptr = &object_type_body;

			/* Set object details required */
			o_ptr->k_idx = selection;
			o_ptr->tval = k_info[selection].tval;
			o_ptr->sval = k_info[selection].sval;
			o_ptr->xtra1 = 0;
		}
		/* Did not choose something */
		else
		{
			return;
		}
	}

	/* Prayer book -- Learn a random prayer */
	if (o_ptr->tval == TV_PRAYER_BOOK)
	{
		int ii;

		int k = 0;

		int gift = -1;

		for (i=0;i<z_info->s_max;i++)
		{
			s_ptr=&s_info[i];

			for (ii=0;ii<MAX_SPELL_APPEARS;ii++)
			{
				if ((s_ptr->appears[ii].tval == o_ptr->tval) &&
			    	(s_ptr->appears[ii].sval == o_ptr->sval) &&
				(spell_okay(i,FALSE)))
				{
					if ((++k == 1) || ((k > 1) &&
						(rand_int(k) ==0)))
					{
						gift = i;
					}
				}
			}
		}

		/* Accept gift */
		spell = gift;
	}

	/* Song book -- Learn a spell in order */
	else if (o_ptr->tval == TV_SONG_BOOK)
	{
		s16b book[26];

		int num = 0;

		int graft = -1;

		/* Fill the book with spells */
		fill_book(o_ptr,book,&num);

		/* Do the hard work */
		for(i=0;i<num;i++)
		{
			if (spell_okay(book[i],FALSE))
			{
				graft = book[i];
				break;
			}

		}

		/* Accept graft */
		spell = graft;
	}

	/* Magic book / runestone -- Learn a selected spell */
	else if ((o_ptr->tval == TV_MAGIC_BOOK) || (o_ptr->tval == TV_RUNESTONE))
	{
		/* Ask for a spell, allow cancel */
		if (!get_spell(&spell, "study", o_ptr, FALSE) && (spell == -1)) return;
	}

	/* Nothing to study */
	if (spell < 0)
	{
		/* Message */
		msg_format("You cannot learn any %ss in that%s.", p, u);

		/* Abort */
		return;
	}


	/* Take a turn */
	p_ptr->energy_use = 100;

	/* Find the next open entry in "spell_order[]" */
	for (i = 0; i < PY_MAX_SPELLS; i++)
	{
		/* Stop at the first empty space */
		if (p_ptr->spell_order[i] == 0) break;
	}

	/* Paranoia */
	if (i >= max_spells)
	{
		/* Message */
		msg_format("You cannot learn any more %ss.", p);

		return;
	}

	/* Add the spell to the known list */
	p_ptr->spell_order[i] = spell;

	/* Learn the spell */
	if (i < 32)
	{
		p_ptr->spell_learned1 |= (1L << i);
	}
	else if (i < 64)
	{
		p_ptr->spell_learned2 |= (1L << (i - 32));
	}
	else if (i < 96)
	{
		p_ptr->spell_learned3 |= (1L << (i - 64));
	}
	else
	{
		p_ptr->spell_learned4 |= (1L << (i - 96));
	}

	/*Set to spell*/
	s_ptr = &(s_info[spell]);

	/* Mention the result */
	message_format(MSG_STUDY, 0, "You have learned the %s of %s.",
	   p, s_name + s_ptr->name);

	/* One less spell available */
	p_ptr->new_spells--;

	/* Message if needed */
	if (p_ptr->new_spells)
	{
		/* Message */
		msg_format("You can learn %d more %s%s.",
		   p_ptr->new_spells, p,
		   (p_ptr->new_spells != 1) ? "s" : "");
	}

	/* Save the new_spells value */
	p_ptr->old_spells = p_ptr->new_spells;

	/* Create a new study object */
	o_ptr = &object_type_body;

	/* Prepare object */
	object_prep(o_ptr,lookup_kind(TV_STUDY, tval));

	/* Set the spell */
	o_ptr->pval = spell;

	/* And carry it */
	item = inven_carry(o_ptr);

	/* Redraw Study Status */
	p_ptr->redraw |= (PR_STUDY);

	/* Redraw object recall */
	p_ptr->window |= (PW_OBJECT);

}




bool inven_cast_okay(const object_type *o_ptr)
{
	int i,ii;

	spell_type *s_ptr;

	if ((o_ptr->tval != TV_MAGIC_BOOK) &&
	    (o_ptr->tval != TV_PRAYER_BOOK) &&
	    (o_ptr->tval != TV_RUNESTONE) &&
  	  (o_ptr->tval != TV_SONG_BOOK) &&
	  (o_ptr->tval != TV_STUDY)) return (0);

	/* Research materials */
	if (o_ptr->tval == TV_STUDY)
	{
		for (i=0;i<PY_MAX_SPELLS;i++)
		{
			if (p_ptr->spell_order[i] == o_ptr->pval) return(1);
		}
	}

	/* Book / runestone */
	else for (i=0;i<PY_MAX_SPELLS;i++)
	{
		if (p_ptr->spell_order[i] == 0) continue;

		s_ptr=&s_info[p_ptr->spell_order[i]];

		/* Book / runestone */
		for (ii=0;ii<MAX_SPELL_APPEARS;ii++)
		{
			if ((s_ptr->appears[ii].tval == o_ptr->tval) &&
			    (s_ptr->appears[ii].sval == o_ptr->sval))

			{
				return(1);
			}
		}
	}

	return (0);

}


/*
 * Cast a spell (once chosen)
 */
void do_cmd_cast_aux(int spell, int plev, cptr p, cptr t)
{
	int i;
	int chance;

	spell_type *s_ptr;
	spell_cast *sc_ptr = &(s_info[0].cast[0]);


	/* Get the spell */
	s_ptr = &s_info[spell];

	/* Get the spell details */
	for (i=0;i<MAX_SPELL_CASTERS;i++)
	{
		if (s_ptr->cast[i].class == p_ptr->pclass)
		{
			sc_ptr=&(s_ptr->cast[i]);
		}


	}

	/* Verify "dangerous" spells */
	if (sc_ptr->mana > p_ptr->csp)
	{
		/* Warning */
		msg_format("You do not have enough mana to %s this %s.",p,t);

		/* Verify */
		if (!get_check("Attempt it anyway? "))
		{
			if (p_ptr->held_song)
			{
				/* Redraw the state */
				p_ptr->redraw |= (PR_STATE);			

				p_ptr->held_song = 0;
			}

			return;
		}
	}

	/* Verify "warning" spells */
	else if ((verify_mana) &&
	 ((p_ptr->csp - sc_ptr->mana) < (p_ptr->msp * op_ptr->hitpoint_warn) / 10))
	{
		/* Warning */
		msg_format("You have limited mana to %s this %s.",p,t);

		/* Verify */
		if (!get_check("Attempt it anyway? "))
		{
			if (p_ptr->held_song)
			{
				/* Redraw the state */
				p_ptr->redraw |= (PR_STATE);			

				p_ptr->held_song = 0;
			}
			return;
		}
	}

	/* Spell failure chance */
	chance = spell_chance(spell);

	/* Some items and some rooms silence the player */
	if ((p_ptr->cur_flags4 & (TR4_SILENT)) || (room_has_flag(p_ptr->py, p_ptr->px, ROOM_SILENT)))
	{
		/* Some items silence the player */
		chance = 99;

		/* Warn the player */
		msg_print("You are engulfed in magical silence.");

		/* Get the room */
		if (!(room_has_flag(p_ptr->py, p_ptr->px, ROOM_SILENT)))
		{
			/* Always notice */
			equip_can_flags(0x0L,0x0L,0x0L,TR4_SILENT);
		}
	}
	else
	{
		/* Always notice */
		equip_not_flags(0x0L,0x0L,0x0L,TR4_SILENT);
	}

	/* Failed spell */
	if (rand_int(100) < chance)
	{
		if (flush_failure) flush();
		msg_format("You failed to %s the %s!",p,t);

		if (p_ptr->held_song)
		{
			/* Redraw the state */
			p_ptr->redraw |= (PR_STATE);			

			p_ptr->held_song = 0;
		}
	}

	/* Process spell */
	else
	{
		/* Must be true to let us abort */
		bool abort = TRUE;

		/* Always true */
		bool known = TRUE;

		/* Apply the spell effect */
		process_spell(spell,plev,&abort,&known);

		/* Did we cancel? */
		if (abort) return;

		for (i=0;i<PY_MAX_SPELLS;i++)
		{

			if (p_ptr->spell_order[i] == spell) break;

		}


		/* Paranoia */
		if (i==PY_MAX_SPELLS) ;

		/* A spell was cast */ 
		else if (!((i < 32) ?
		      (p_ptr->spell_worked1 & (1L << i)) :
		      ((i < 64) ? (p_ptr->spell_worked2 & (1L << (i - 32))) :
		      ((i < 96) ? (p_ptr->spell_worked3 & (1L << (i - 64))) :
		      (p_ptr->spell_worked4 & (1L << (i - 96)))))))
		{
			int e = sc_ptr->level;

			/* The spell worked */
			if (i < 32)
			{
				p_ptr->spell_worked1 |= (1L << i);
			}
			else if (i < 64)
			{
				p_ptr->spell_worked2 |= (1L << (i - 32));
			}
			else if (i < 96)
			{
				p_ptr->spell_worked3 |= (1L << (i - 64));
			}
			else if (i < 128)
			{
				p_ptr->spell_worked2 |= (1L << (i - 96));
			}

			/* Gain experience */
			gain_exp(e * sc_ptr->level);

			/* Redraw object recall */
			p_ptr->window |= (PW_OBJECT);
		}
	}

	/* Sufficient mana */
	if (sc_ptr->mana <= p_ptr->csp)
	{
		/* Use some mana */
		p_ptr->csp -= sc_ptr->mana;
	}

	/* Over-exert the player */
	else
	{
		int oops = sc_ptr->mana - p_ptr->csp;

		/* No mana left */
		p_ptr->csp = 0;
		p_ptr->csp_frac = 0;

		/* Message */
		msg_print("You faint from the effort!");

		/* Hack -- Bypass free action */
		(void)set_paralyzed(p_ptr->paralyzed + randint(5 * oops + 1));

		/* Damage CON (possibly permanently) */
		if (rand_int(100) < 50)
		{
			bool perm = (rand_int(100) < 25);

			/* Message */
			msg_print("You have damaged your health!");

			/* Reduce constitution */
			(void)dec_stat(A_CON, 15 + randint(10), perm);
		}
	}

	/* Redraw mana */
	p_ptr->redraw |= (PR_MANA);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);

}


/*
 * Cast a spell
 */
void do_cmd_cast(void)
{
	int item, spell, tval;

	object_type *o_ptr;

	cptr p, t;

	cptr q, s;

	cptr u = " book";

	/* Check if we are holding a song */
	if (p_ptr->held_song)
	{
		/* Verify */
		if (!get_check(format("Continue singing %s?", s_name + s_info[p_ptr->held_song].name)))
		{
			/* Redraw the state */
			p_ptr->redraw |= (PR_STATE);			

			p_ptr->held_song = 0;
		}
	}

	/* Cannot cast spells if illiterate */
	if (c_info[p_ptr->pclass].spell_first > 50)
	{
		msg_print("You cannot read books or runestones.");
		return;
	}


	/* Require lite */
	if (p_ptr->blind || no_lite())
	{
		msg_print("You cannot see!");
		return;
	}

	/* Not when confused */
	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}

	/* Amnesia */
	if (p_ptr->amnesia)
	{
		msg_print("You have forgotten how to read!");
		return;
	}

	/* Restrict choices to spells we can cast */
	item_tester_hook = inven_cast_okay;

	/* Get an item */
	q = "Use which book? ";
	s = "You have nothing you have studied!";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR | USE_FEATU))) return;

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

	/* Track the object kind */
	object_kind_track(o_ptr->k_idx);

	/* Hack -- Handle stuff */
	handle_stuff();

	/* Get fake tval */
	if (o_ptr->tval == TV_STUDY)
	{
		tval = o_ptr->sval;
		u = "";
	}
	else tval = o_ptr->tval;

	/* Cast, recite, sing or play */
	switch (tval)
	{
		case TV_MAGIC_BOOK:
		{
			p="cast";
			t="spell";
			
			break;
		}
		case TV_RUNESTONE:
		{
			p="draw";
			t="pattern";
			u=" combination of runes";
			
			break;
		}
		case TV_PRAYER_BOOK:
		{
       			p="recite";
			t="prayer";
			
			break;
		}
		case TV_SONG_BOOK:
		{
			if (p_ptr->pstyle == WS_INSTRUMENT)
			{
				p="play";
			}
			else
			{
				p="sing";
			}

			t = "song";
			break;
		}

		default:
		{
			p="use";
			t="power";
			break;		
		}
	}

	/* Ask for a spell */
	if (!get_spell(&spell, p, o_ptr, TRUE))
	{
		if (spell == -2) msg_format("You don't know any %ss in that%s.",t,u);
		return;
	}

	/* Take a turn */
	p_ptr->energy_use = 100;

	/* Hold a song if possible */
	if (s_info[spell].flags3 & (SF3_HOLD_SONG))
	{
		int i;

		for (i = 0;i< z_info->w_max;i++)
		{
			if (w_info[i].class != p_ptr->pclass) continue;

			if (w_info[i].level > p_ptr->lev) continue;

			if (w_info[i].benefit != WB_HOLD_SONG) continue;

			/* Check for styles */       
			if ((w_info[i].styles==0) || (w_info[i].styles & (p_ptr->cur_style & (1L << p_ptr->pstyle))))
			{
				/* Verify */
				if (get_check(format("Continue singing %s?", s_name + s_info[spell].name))) p_ptr->held_song = spell;
			}

			/* Hack - Cancel searching */
			/* Stop searching */
			if (p_ptr->searching)
			{
				/* Clear the searching flag */
				p_ptr->searching = FALSE;

				/* Clear the last disturb */
				p_ptr->last_disturb = turn;
			}

			/* Recalculate bonuses */
			p_ptr->update |= (PU_BONUS);

			/* Redraw the state */
			p_ptr->redraw |= (PR_STATE);

		}
	}

	/* Cast the spell - held songs get cast later*/
	if (p_ptr->held_song != spell) do_cmd_cast_aux(spell,spell_power(spell),p,t);
}

