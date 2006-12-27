

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
 * Hack -- drop permissions
 */
void safe_setuid_drop(void)
{

#ifdef SET_UID

# ifdef SAFE_SETUID

#  ifdef HAVE_SETEGID

	if (setegid(getgid()) != 0)
	{
		quit("setegid(): cannot set permissions correctly!");
	}
 
#  else /* HAVE_SETEGID */

#   ifdef SAFE_SETUID_POSIX

	if (setgid(getgid()) != 0)
	{
		quit("setgid(): cannot set permissions correctly!");
	}
    
#   else /* SAFE_SETUID_POSIX */

	if (setregid(getegid(), getgid()) != 0)
	{
		quit("setregid(): cannot set permissions correctly!");
	}

#   endif /* SAFE_SETUID_POSIX */

#  endif /* HAVE_SETEGID */

# endif /* SAFE_SETUID */

#endif /* SET_UID */

}


/*
 * Hack -- grab permissions
 */
void safe_setuid_grab(void)
{

#ifdef SET_UID

# ifdef SAFE_SETUID

#  ifdef HAVE_SETEGID

	if (setegid(player_egid) != 0)
	{
		quit("setegid(): cannot set permissions correctly!");
	}

#  else /* HAVE_SETEGID */

#   ifdef SAFE_SETUID_POSIX

	if (setgid(player_egid) != 0)
	{
		quit("setgid(): cannot set permissions correctly!");
	}

#   else /* SAFE_SETUID_POSIX */

	if (setregid(getegid(), getgid()) != 0)
	{
		quit("setregid(): cannot set permissions correctly!");
	}

#   endif /* SAFE_SETUID_POSIX */

#  endif /* HAVE_SETEGID */

# endif /* SAFE_SETUID */

#endif /* SET_UID */

}


#if 0

/*
 * Use this (perhaps) for Angband 2.8.4
 *
 * Extract "tokens" from a buffer
 *
 * This function uses "whitespace" as delimiters, and treats any amount of
 * whitespace as a single delimiter.  We will never return any empty tokens.
 * When given an empty buffer, or a buffer containing only "whitespace", we
 * will return no tokens.  We will never extract more than "num" tokens.
 *
 * By running a token through the "text_to_ascii()" function, you can allow
 * that token to include (encoded) whitespace, using "\s" to encode spaces.
 *
 * We save pointers to the tokens in "tokens", and return the number found.
 */
static s16b tokenize_whitespace(char *buf, s16b num, char **tokens)
{
	int k = 0;

	char *s = buf;


	/* Process */
	while (k < num)
	{
		char *t;

		/* Skip leading whitespace */
		for ( ; *s && isspace(*s); ++s) /* loop */;

		/* All done */
		if (!*s) break;

		/* Find next whitespace, if any */
		for (t = s; *t && !isspace(*t); ++t) /* loop */;

		/* Nuke and advance (if necessary) */
		if (*t) *t++ = '\0';

		/* Save the token */
		tokens[k++] = s;

		/* Advance */
		s = t;
	}

	/* Count */
	return (k);
}

#endif


/*
 * Extract the first few "tokens" from a buffer
 *
 * This function uses "colon" and "slash" as the delimeter characters.
 *
 * We never extract more than "num" tokens.  The "last" token may include
 * "delimeter" characters, allowing the buffer to include a "string" token.
 *
 * We save pointers to the tokens in "tokens", and return the number found.
 *
 * Hack -- Attempt to handle the 'c' character formalism
 *
 * Hack -- An empty buffer, or a final delimeter, yields an "empty" token.
 *
 * Hack -- We will always extract at least one token
 */
s16b tokenize(char *buf, s16b num, char **tokens)
{
	int i = 0;

	char *s = buf;


	/* Process */
	while (i < num - 1)
	{
		char *t;

		/* Scan the string */
		for (t = s; *t; t++)
		{
			/* Found a delimiter */
			if ((*t == ':') || (*t == '/')) break;

			/* Handle single quotes */
			if (*t == '\'')
			{
				/* Advance */
				t++;

				/* Handle backslash */
				if (*t == '\\') t++;

				/* Require a character */
				if (!*t) break;

				/* Advance */
				t++;

				/* Hack -- Require a close quote */
				if (*t != '\'') *t = '\'';
			}

			/* Handle back-slash */
			if (*t == '\\') t++;
		}

		/* Nothing left */
		if (!*t) break;

		/* Nuke and advance */
		*t++ = '\0';

		/* Save the token */
		tokens[i++] = s;

		/* Advance */
		s = t;
	}

	/* Save the token */
	tokens[i++] = s;

	/* Number found */
	return (i);
}



/*
 * Parse a sub-file of the "extra info" (format shown below)
 *
 * Each "action" line has an "action symbol" in the first column,
 * followed by a colon, followed by some command specific info,
 * usually in the form of "tokens" separated by colons or slashes.
 *
 * Blank lines, lines starting with white space, and lines starting
 * with pound signs ("#") are ignored (as comments).
 *
 * Note the use of "tokenize()" to allow the use of both colons and
 * slashes as delimeters, while still allowing final tokens which
 * may contain any characters including "delimiters".
 *
 * Note the use of "strtol()" to allow all "integers" to be encoded
 * in decimal, hexidecimal, or octal form.
 *
 * Note that "monster zero" is used for the "player" attr/char, "object
 * zero" will be used for the "stack" attr/char, and "feature zero" is
 * used for the "nothing" attr/char.
 *
 * Specify the attr/char values for "monsters" by race index.
 *   R:<num>:<a>/<c>
 *
 * Specify the attr/char values for "objects" by kind index.
 *   K:<num>:<a>/<c>
 *
 * Specify the attr/char values for "features" by feature index.
 *   F:<num>:<a>/<c>:<flags>
 *
 * Specify the attr/char values for "special" things.
 *   S:<num>:<a>/<c>
 *
 * Specify the attribute values for inventory "objects" by kind tval.
 *   E:<tv>:<a>
 *
 * Define a macro action, given an encoded macro action.
 *   A:<str>
 *
 * Create a macro, given an encoded macro trigger.
 *   P:<str>
 *
 * Create a keymap, given an encoded keymap trigger.
 *   C:<num>:<str>
 *
 * Turn an option off, given its name.
 *   X:<str>
 *
 * Turn an option on, given its name.
 *   Y:<str>
 *
 * Turn a window flag on or off, given a window, flag, and value.
 *   W:<win>:<flag>:<value>
 *
 * Specify visual information, given an index, and some data.
 *   V:<num>:<kv>:<rv>:<gv>:<bv>
 *
 * Specify colors for message-types.
 *   M:<type>:<attr>
 *
 * Specify the attr/char values for "flavors" by flavors index.
 *   L:<num>:<a>/<c>
 *
 * Specify auto-inscriptions for object kinds/ego items/artifacts
 *   I:<type>:<num>:<str>
 *
 */
errr process_pref_file_command(char *buf)
{
	int i, j, n1, n2;

	char *zz[16];


	/* Skip "empty" lines */
	if (!buf[0]) return (0);

	/* Skip "blank" lines */
	if (isspace(buf[0])) return (0);

	/* Skip comments */
	if (buf[0] == '#') return (0);


	/* Paranoia */
	/* if (strlen(buf) >= 1024) return (1); */


	/* Require "?:*" format */
	if (buf[1] != ':') return (1);


	/* Process "R:<num>:<a>/<c>" -- attr/char for monster races */
	if (buf[0] == 'R')
	{
		if (tokenize(buf+2, 3, zz) == 3)
		{
			monster_race *r_ptr;
			i = (huge)strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if ((i < 0) || (i >= z_info->r_max)) return (1);
			r_ptr = &r_info[i];
			if (n1) r_ptr->x_attr = n1;
			if (n2) r_ptr->x_char = n2;
			return (0);
		}
	}


	/* Process "K:<num>:<a>/<c>"  -- attr/char for object kinds */
	else if (buf[0] == 'K')
	{
		if (tokenize(buf+2, 3, zz) == 3)
		{
			object_kind *k_ptr;
			i = (huge)strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if ((i < 0) || (i >= z_info->k_max)) return (1);
			k_ptr = &k_info[i];
			if (n1) k_ptr->x_attr = n1;
			if (n2) k_ptr->x_char = n2;
			return (0);
		}
	}


	/* Process "F:<num>:<a>/<c>:<flags>" -- attr/char for terrain features, plus flags for lighting etc. */
	else if (buf[0] == 'F')
	{
		/* Mega-hack -- feat supports lighting 'yes' or 'no' */
		if (tokenize(buf+2, 4, zz) == 4)
		{
			feature_type *f_ptr;

			i = (huge)strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if ((i < 0) || (i >= z_info->f_max)) return (1);
			f_ptr = &f_info[i];
			if (n1) f_ptr->x_attr = n1;
			if (n2) f_ptr->x_char = n2;

			/* Clear current flags */
			f_ptr->flags3 &= ~(FF3_ATTR_LITE | FF3_ATTR_ITEM | FF3_ATTR_DOOR | FF3_ATTR_WALL);

			if (strstr("A", zz[3])) f_ptr->flags3 |= (FF3_ATTR_LITE);
			else if (strstr("F", zz[3])) f_ptr->flags3 |= (FF3_ATTR_ITEM);
			else if (strstr("D", zz[3])) f_ptr->flags3 |= (FF3_ATTR_DOOR);
			else if (strstr("W", zz[3])) f_ptr->flags3 |= (FF3_ATTR_WALL);

			return (0);
		}
	}


	/* Process "L:<num>:<a>/<c>" -- attr/char for flavors */
	else if (buf[0] == 'L')
	{
		if (tokenize(buf+2, 3, zz) == 3)
		{
			flavor_type *x_ptr;
			i = (huge)strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
                        if ((i < 0) || (i >= z_info->x_max)) return (1);
			x_ptr = &x_info[i];
			if (n1) x_ptr->x_attr = n1;
			if (n2) x_ptr->x_char = n2;
			return (0);
		}
	}


	/* Process "S:<num>:<a>/<c>" -- attr/char for special things */
	else if (buf[0] == 'S')
	{
		if (tokenize(buf+2, 3, zz) == 3)
		{
			j = (byte)strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if ((j < 0) || (j >= 256)) return (1);
			misc_to_attr[j] = n1;
			misc_to_char[j] = n2;
			return (0);
		}
	}


	/* Process "E:<tv>:<a>" -- attribute for inventory objects */
	else if (buf[0] == 'E')
	{
		if (tokenize(buf+2, 2, zz) == 2)
		{
			j = (byte)strtol(zz[0], NULL, 0) % 128;
			n1 = strtol(zz[1], NULL, 0);
			if ((j < 0) || (j >= 128)) return (1);
			if (n1) tval_to_attr[j] = n1;
			return (0);
		}
	}


	/* Process "A:<str>" -- save an "action" for later */
	else if (buf[0] == 'A')
	{
		text_to_ascii(macro_buffer, sizeof(macro_buffer), buf+2);
		return (0);
	}

	/* Process "P:<str>" -- create macro */
	else if (buf[0] == 'P')
	{
		char tmp[1024];
		text_to_ascii(tmp, sizeof(tmp), buf+2);
		macro_add(tmp, macro_buffer);
		return (0);
	}

	/* Process "C:<num>:<str>" -- create keymap */
	else if (buf[0] == 'C')
	{
		int mode;

		char tmp[1024];

		if (tokenize(buf+2, 2, zz) != 2) return (1);

		mode = strtol(zz[0], NULL, 0);
		if ((mode < 0) || (mode >= KEYMAP_MODES)) return (1);

		text_to_ascii(tmp, sizeof(tmp), zz[1]);
		if (!tmp[0] || tmp[1]) return (1);
		i = (byte)(tmp[0]);

		string_free(keymap_act[mode][i]);

		keymap_act[mode][i] = string_make(macro_buffer);

		return (0);
	}


	/* Process "V:<num>:<kv>:<rv>:<gv>:<bv>" -- visual info */
	else if (buf[0] == 'V')
	{
		if (tokenize(buf+2, 5, zz) == 5)
		{
			i = (byte)strtol(zz[0], NULL, 0);
			if ((i < 0) || (i >= 256)) return (1);
			angband_color_table[i][0] = (byte)strtol(zz[1], NULL, 0);
			angband_color_table[i][1] = (byte)strtol(zz[2], NULL, 0);
			angband_color_table[i][2] = (byte)strtol(zz[3], NULL, 0);
			angband_color_table[i][3] = (byte)strtol(zz[4], NULL, 0);
			return (0);
		}
	}


	/* Process "X:<str>" -- turn option off */
	else if (buf[0] == 'X')
	{
		/* Check non-adult options */
		for (i = 0; i < OPT_ADULT; i++)
		{
			if (option_text[i] && streq(option_text[i], buf + 2))
			{
				op_ptr->opt[i] = FALSE;
				return (0);
			}
		}
	}

	/* Process "Y:<str>" -- turn option on */
	else if (buf[0] == 'Y')
	{
		/* Check non-adult options */
		for (i = 0; i < OPT_ADULT; i++)
		{
			if (option_text[i] && streq(option_text[i], buf + 2))
			{
				op_ptr->opt[i] = TRUE;
				return (0);
			}
		}
	}


	/* Process "W:<win>:<flag>:<value>" -- window flags */
	else if (buf[0] == 'W')
	{
		int win, flag, value;

		if (tokenize(buf + 2, 3, zz) == 3)
		{
			win = strtol(zz[0], NULL, 0);
			flag = strtol(zz[1], NULL, 0);
			value = strtol(zz[2], NULL, 0);

			/* Ignore illegal windows */
			/* Hack -- Ignore the main window */
			if ((win <= 0) || (win >= ANGBAND_TERM_MAX)) return (1);

			/* Ignore illegal flags */
			if ((flag < 0) || (flag >= 32)) return (1);

			/* Require a real flag */
			if (window_flag_desc[flag])
			{
				if (value)
				{
					/* Turn flag on */
					op_ptr->window_flag[win] |= (1L << flag);
				}
				else
				{
					/* Turn flag off */
					op_ptr->window_flag[win] &= ~(1L << flag);
				}
			}

			/* Success */
			return (0);
		}
	}


	/* Process "M:<type>:<attr>" -- colors for message-types */
	else if (buf[0] == 'M')
	{
		if (tokenize(buf+2, 2, zz) == 2)
		{
			u16b type = (u16b)strtol(zz[0], NULL, 0);
			int color = color_char_to_attr(zz[1][0]);

			/* Ignore illegal color */
			if (color < 0) return (1);

			/* Store the color */
			return (message_color_define(type, (byte)color));
		}
	}

	/* Process "I:<typ>:<num>:<str>" -- create auto-inscription */
	else if (buf[0] == 'I')
	{
		if (tokenize(buf+2, 3, zz) != 3) return (1);

		i = strtol(zz[1], NULL, 0);

		switch(buf[2])
		{
			case 'E':
			{
				if ((i < 0) || (i >= z_info->e_max)) return (1);

				e_info[i].note = quark_add(zz[2]);

				return (0);
			}
			case 'K':
			{
				if ((i < 0) || (i >= z_info->k_max)) return (1);

				k_info[i].note = quark_add(zz[2]);

				return (0);
			 }
		}
	}



	/* Failure */
	return (1);
}


/*
 * Helper function for "process_pref_file()"
 *
 * Input:
 *   v: output buffer array
 *   f: final character
 *
 * Output:
 *   result
 */
static cptr process_pref_file_expr(char **sp, char *fp)
{
	cptr v;

	char *b;
	char *s;

	char b1 = '[';
	char b2 = ']';

	char f = ' ';

	/* Initial */
	s = (*sp);

	/* Skip spaces */
	while (isspace(*s)) s++;

	/* Save start */
	b = s;

	/* Default */
	v = "?o?o?";

	/* Analyze */
	if (*s == b1)
	{
		const char *p;
		const char *t;

		/* Skip b1 */
		s++;

		/* First */
		t = process_pref_file_expr(&s, &f);

		/* Oops */
		if (!*t)
		{
			/* Nothing */
		}

		/* Function: IOR */
		else if (streq(t, "IOR"))
		{
			v = "0";
			while (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
				if (*t && !streq(t, "0")) v = "1";
			}
		}

		/* Function: AND */
		else if (streq(t, "AND"))
		{
			v = "1";
			while (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
				if (*t && streq(t, "0")) v = "0";
			}
		}

		/* Function: NOT */
		else if (streq(t, "NOT"))
		{
			v = "1";
			while (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
				if (*t && !streq(t, "0")) v = "0";
			}
		}

		/* Function: EQU */
		else if (streq(t, "EQU"))
		{
			v = "1";
			if (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
			}
			while (*s && (f != b2))
			{
				p = t;
				t = process_pref_file_expr(&s, &f);
				if (*t && !streq(p, t)) v = "0";
			}
		}

		/* Function: LEQ */
		else if (streq(t, "LEQ"))
		{
			v = "1";
			if (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
			}
			while (*s && (f != b2))
			{
				p = t;
				t = process_pref_file_expr(&s, &f);
				if (*t && (strcmp(p, t) >= 0)) v = "0";
			}
		}

		/* Function: GEQ */
		else if (streq(t, "GEQ"))
		{
			v = "1";
			if (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
			}
			while (*s && (f != b2))
			{
				p = t;
				t = process_pref_file_expr(&s, &f);
				if (*t && (strcmp(p, t) <= 0)) v = "0";
			}
		}

		/* Oops */
		else
		{
			while (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
			}
		}

		/* Verify ending */
		if (f != b2) v = "?x?x?";

		/* Extract final and Terminate */
		if ((f = *s) != '\0') *s++ = '\0';
	}

	/* Other */
	else
	{
		/* Accept all printables except spaces and brackets */
		while (isprint(*s) && !strchr(" []", *s)) ++s;

		/* Extract final and Terminate */
		if ((f = *s) != '\0') *s++ = '\0';

		/* Variable */
		if (*b == '$')
		{
			/* System */
			if (streq(b+1, "SYS"))
			{
				v = ANGBAND_SYS;
			}

			/* Graphics */
			else if (streq(b+1, "GRAF"))
			{
				v = ANGBAND_GRAF;
			}

			/* Race */
			else if (streq(b+1, "RACE"))
			{
				v = p_name + rp_ptr->name;
			}

			/* Class */
			else if (streq(b+1, "CLASS"))
			{
				v = c_name + cp_ptr->name;
			}

			/* Player */
			else if (streq(b+1, "PLAYER"))
			{
				v = op_ptr->base_name;
			}
		}

		/* Constant */
		else
		{
			v = b;
		}
	}

	/* Save */
	(*fp) = f;

	/* Save */
	(*sp) = s;

	/* Result */
	return (v);
}


/*
 * Open the "user pref file" and parse it.
 */
static errr process_pref_file_aux(cptr name)
{
	FILE *fp;

	char buf[1024];

	char old[1024];

	int line = -1;

	errr err = 0;

	bool bypass = FALSE;


	/* Open the file */
	fp = my_fopen(name, "r");

	/* No such file */
	if (!fp) return (-1);


	/* Process the file */
	while (0 == my_fgets(fp, buf, sizeof(buf)))
	{
		/* Count lines */
		line++;


		/* Skip "empty" lines */
		if (!buf[0]) continue;

		/* Skip "blank" lines */
		if (isspace(buf[0])) continue;

		/* Skip comments */
		if (buf[0] == '#') continue;


		/* Save a copy */
		strcpy(old, buf);


		/* Process "?:<expr>" */
		if ((buf[0] == '?') && (buf[1] == ':'))
		{
			char f;
			cptr v;
			char *s;

			/* Start */
			s = buf + 2;

			/* Parse the expr */
			v = process_pref_file_expr(&s, &f);

			/* Set flag */
			bypass = (streq(v, "0") ? TRUE : FALSE);

			/* Continue */
			continue;
		}

		/* Apply conditionals */
		if (bypass) continue;


		/* Process "%:<file>" */
		if (buf[0] == '%')
		{
			/* Process that file if allowed */
			(void)process_pref_file(buf + 2);

			/* Continue */
			continue;
		}


		/* Process the line */
		err = process_pref_file_command(buf);

		/* Oops */
		if (err) break;
	}


	/* Error */
	if (err)
	{
		/* Print error message */
		/* ToDo: Add better error messages */
		msg_format("Error %d in line %d of file '%s'.", err, line, name);
		msg_format("Parsing '%s'", old);
		message_flush();
	}

	/* Close the file */
	my_fclose(fp);

	/* Result */
	return (err);
}



/*
 * Process the "user pref file" with the given name
 *
 * See the functions above for a list of legal "commands".
 *
 * We also accept the special "?" and "%" directives, which
 * allow conditional evaluation and filename inclusion.
 */
errr process_pref_file(cptr name)
{
	char buf[1024];

	errr err = 0;


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_PREF, name);

	/* Process the pref file */
	err = process_pref_file_aux(buf);

	/* Stop at parser errors, but not at non-existing file */
	if (err < 1)
	{
		/* Build the filename */
		path_build(buf, 1024, ANGBAND_DIR_USER, name);

		/* Process the pref file */
		err = process_pref_file_aux(buf);
	}

	/* Result */
	return (err);
}



/*
 * Hack -- save a screen dump to a file in html format
 *
 * Note:  Unlike other Angband variants, I'm not worried too about bad attr/char
 * mappings.
 *
 * This may seem like laziness on my part, but the only clean way I can see of
 * doing this code-wise requires that I reset all the visuals anyway. So we do this
 * when we have to (Dumping as a command from the main map) and anywhere else, we
 * try and avoid it (Due to obvious problems with trying to redraw the screen
 * mid-way through displaying a message/animation and so on). That way for someone
 * truely worried about getting everything via an html screenshot, they can reset
 * their pref file and avoid using font preferences that use non-ASCII characters.
 *
 * This also has the additional advantage that we can dump any screen to html,
 * rather than just the main map, which other variants do via duplication of the
 * map info code. We do this by hooking into the currently unused Ctrl-right square
 * bracket in util.c. This is probably dangerous, so we don't use screen_save/screen_load
 * or notify the player in any way that we have committed a screen_dump. Hopefully
 * by the time we call inkey() we don't have any issues with outputting a file.
 *
 */
void dump_html(void)
{
	int y, x;

	byte a = 0, oa = TERM_WHITE;
	char c = ' ';

	FILE *fff;

	char buf[1024];

	/* Click! */
	sound(MSG_SCREENDUMP);

	/* Build the filename */
	/* XXX Support .html vs. .htm file names. We dumb down for DOS */
	path_build(buf, 1024, ANGBAND_DIR_USER, "dump.htm");

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Append to the file */
	fff = my_fopen(buf, "w");

	/* Oops */
	if (!fff) return;


	/* Html preamble */
	fprintf(fff, "<html>\n");
	fprintf(fff, "<head>\n");
	fprintf(fff, "<meta name='generator' content='UnAngband " VERSION_STRING "'>\n");
	fprintf(fff, "<title>" VERSION_NAME " " VERSION_STRING " Screen Dump</title>\n");
	fprintf(fff, "</head>\n");
	fprintf(fff, "<body text='#%02x%02x%02x' bgcolor='#000000'>",
	             angband_color_table[TERM_WHITE][1],
	             angband_color_table[TERM_WHITE][2],
	             angband_color_table[TERM_WHITE][3]);
	fprintf(fff, "<pre><tt>");

	/* Dump the screen */
	for (y = 0; y < 24; y++)
	{
		/* Dump each row */
		for (x = 0; x < 79; x++)
		{
			/* Get the attr/char */
			(void)(Term_what(x, y, &a, &c));

			/* Check for being white (reduce file size) */
			if ((a == oa) && (oa == TERM_WHITE))
			{
				/* Do nothing. */
			}
			else if ((a != oa) && (oa == TERM_WHITE))
			{
				fprintf(fff, "<font color='#%02x%02x%02x'>",
				                  angband_color_table[a][1],
				                  angband_color_table[a][2],
				                  angband_color_table[a][3]);

				oa = a;
			}
			else if ((a != oa) && (a == TERM_WHITE))
			{
				fprintf(fff, "</font>");

				oa = a;
			}
			else if ((a != oa) && (oa != TERM_WHITE))
			{
				fprintf(fff, "</font><font color='#%02x%02x%02x'>",
				                  angband_color_table[a][1],
				                  angband_color_table[a][2],
				                  angband_color_table[a][3]);

				oa = a;
			}

			fprintf(fff, "%c", c);
		}

		/* End the row */
		fprintf(fff, "\n");
	}

	fprintf(fff, "</tt></pre>\n");

	fprintf(fff, "</body>\n");
	fprintf(fff, "</html>\n");

	/* Close it */
	my_fclose(fff);

}



#ifdef CHECK_TIME

/*
 * Operating hours for ANGBAND (defaults to non-work hours)
 */
static char days[7][29] =
{
	"SUN:XXXXXXXXXXXXXXXXXXXXXXXX",
	"MON:XXXXXXXX.........XXXXXXX",
	"TUE:XXXXXXXX.........XXXXXXX",
	"WED:XXXXXXXX.........XXXXXXX",
	"THU:XXXXXXXX.........XXXXXXX",
	"FRI:XXXXXXXX.........XXXXXXX",
	"SAT:XXXXXXXXXXXXXXXXXXXXXXXX"
};

/*
 * Restict usage (defaults to no restrictions)
 */
static bool check_time_flag = FALSE;

#endif


/*
 * Handle CHECK_TIME
 */
errr check_time(void)
{

#ifdef CHECK_TIME

	time_t c;
	struct tm *tp;

	/* No restrictions */
	if (!check_time_flag) return (0);

	/* Check for time violation */
	c = time((time_t *)0);
	tp = localtime(&c);

	/* Violation */
	if (days[tp->tm_wday][tp->tm_hour + 4] != 'X') return (1);

#endif

	/* Success */
	return (0);
}



/*
 * Initialize CHECK_TIME
 */
errr check_time_init(void)
{

#ifdef CHECK_TIME

	FILE *fp;

	char buf[1024];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_FILE, "time.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* No file, no restrictions */
	if (!fp) return (0);

	/* Assume restrictions */
	check_time_flag = TRUE;

	/* Parse the file */
	while (0 == my_fgets(fp, buf, sizeof(buf)))
	{
		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Chop the buffer */
		buf[29] = '\0';

		/* Extract the info */
		if (prefix(buf, "SUN:")) strcpy(days[0], buf);
		if (prefix(buf, "MON:")) strcpy(days[1], buf);
		if (prefix(buf, "TUE:")) strcpy(days[2], buf);
		if (prefix(buf, "WED:")) strcpy(days[3], buf);
		if (prefix(buf, "THU:")) strcpy(days[4], buf);
		if (prefix(buf, "FRI:")) strcpy(days[5], buf);
		if (prefix(buf, "SAT:")) strcpy(days[6], buf);
	}

	/* Close it */
	my_fclose(fp);

#endif /* CHECK_TIME */

	/* Success */
	return (0);
}



#ifdef CHECK_LOAD

#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN  64
#endif

typedef struct statstime statstime;

struct statstime
{
	int cp_time[4];
	int dk_xfer[4];
	unsigned int        v_pgpgin;
	unsigned int        v_pgpgout;
	unsigned int        v_pswpin;
	unsigned int        v_pswpout;
	unsigned int        v_intr;
	int if_ipackets;
	int if_ierrors;
	int if_opackets;
	int if_oerrors;
	int if_collisions;
	unsigned int        v_swtch;
	long avenrun[3];
	struct timeval      boottime;
	struct timeval      curtime;
};

/*
 * Maximal load (if any).
 */
static int check_load_value = 0;

#endif


/*
 * Handle CHECK_LOAD
 */
errr check_load(void)
{

#ifdef CHECK_LOAD

	struct statstime    st;

	/* Success if not checking */
	if (!check_load_value) return (0);

	/* Check the load */
	if (0 == rstat("localhost", &st))
	{
		long val1 = (long)(st.avenrun[2]);
		long val2 = (long)(check_load_value) * FSCALE;

		/* Check for violation */
		if (val1 >= val2) return (1);
	}

#endif

	/* Success */
	return (0);
}


/*
 * Initialize CHECK_LOAD
 */
errr check_load_init(void)
{

#ifdef CHECK_LOAD

	FILE *fp;

	char buf[1024];

	char temphost[MAXHOSTNAMELEN+1];
	char thishost[MAXHOSTNAMELEN+1];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_FILE, "load.txt");

	/* Open the "load" file */
	fp = my_fopen(buf, "r");

	/* No file, no restrictions */
	if (!fp) return (0);

	/* Default load */
	check_load_value = 100;

	/* Get the host name */
	(void)gethostname(thishost, (sizeof thishost) - 1);

	/* Parse it */
	while (0 == my_fgets(fp, buf, sizeof(buf)))
	{
		int value;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Parse, or ignore */
		if (sscanf(buf, "%s%d", temphost, &value) != 2) continue;

		/* Skip other hosts */
		if (!streq(temphost, thishost) &&
		    !streq(temphost, "localhost")) continue;

		/* Use that value */
		check_load_value = value;

		/* Done */
		break;
	}

	/* Close the file */
	my_fclose(fp);

#endif /* CHECK_LOAD */

	/* Success */
	return (0);
}



/*
 * Returns a "rating" of x depending on y, and sets "attr" to the
 * corresponding "attribute".
 */
static cptr likert(int x, int y, byte *attr)
{
	/* Paranoia */
	if (y <= 0) y = 1;

	/* Negative value */
	if (x < 0)
	{
		*attr = TERM_RED;
		return ("Very Bad");
	}

	/* Analyze the value */
	switch ((x / y))
	{
		case 0:
		case 1:
		{
			*attr = TERM_RED;
			return ("Bad");
		}
		case 2:
		{
			*attr = TERM_RED;
			return ("Poor");
		}
		case 3:
		case 4:
		{
			*attr = TERM_YELLOW;
			return ("Fair");
		}
		case 5:
		{
			*attr = TERM_YELLOW;
			return ("Good");
		}
		case 6:
		{
			*attr = TERM_YELLOW;
			return ("Very Good");
		}
		case 7:
		case 8:
		{
			*attr = TERM_L_GREEN;
			return ("Excellent");
		}
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		{
			*attr = TERM_L_GREEN;
			return ("Superb");
		}
		case 14:
		case 15:
		case 16:
		case 17:
		{
			*attr = TERM_L_GREEN;
			return ("Heroic");
		}
		default:
		{
			*attr = TERM_L_GREEN;
			return ("Legendary");
		}
	}
}


/*
 * Prints some "extra" information on the screen.
 */
static void display_player_xtra_info(void)
{
	int i;
	s32b tmpl;
	int col, row;
	int hit, dam, hit_real;
	int style_hit, style_dam, style_crit;
	u32b style;
	int base, plus;
	int tmp;
	int xthn, xthb, xtht, xfos, xsrh;
	int xdis, xdev, xsav, xstl;
	byte likert_attr;

	object_type *o_ptr;

	cptr desc;

	char name[60];

	char buf[160];

	/* Pretty name */

#ifdef USE_CLASS_PRETTY_NAMES
	lookup_prettyname(name,p_ptr->pclass, p_ptr->pstyle,p_ptr->psval,TRUE,TRUE);
	sprintf(buf,"%s, the %s",op_ptr->full_name,name);
	c_put_str(TERM_L_BLUE, buf, 1, 8);
#else
	c_put_str(TERM_L_BLUE, op_ptr->full_name, 1, 8);
#endif

	/* Current Home Town */
	put_str("Home", 7, 1);
	c_put_str(TERM_L_BLUE, t_name + t_info[p_ptr->town].name, 7, 8);

	/* Current Dungeon */
	put_str("Target", 8, 1);
	c_put_str(TERM_L_BLUE, t_name + t_info[p_ptr->dungeon].name, 8, 8);

	/* Upper middle */
	col = 32;
	row = 2;

	/* Age */
	Term_putstr(col, row, -1, TERM_WHITE, "Age");
	Term_putstr(col+8, row, -1, TERM_L_BLUE, format("%4d", (int)p_ptr->age));

	/* Height */
	Term_putstr(col, row + 1, -1, TERM_WHITE, "Height");
	Term_putstr(col+8, row + 1, -1, TERM_L_BLUE, format("%4d", (int)p_ptr->ht));

	/* Weight */
	Term_putstr(col, row + 2, -1, TERM_WHITE, "Weight");
	Term_putstr(col+8, row + 2, -1, TERM_L_BLUE, format("%4d", (int)p_ptr->wt));

	/* Status */
	Term_putstr(col, row + 3, -1, TERM_WHITE, "Status");
	Term_putstr(col+8, row + 3, -1, TERM_L_BLUE, format("%4d", (int)p_ptr->sc));
	/* Char level */
	Term_putstr(col, row + 4, -1, TERM_WHITE, "Level");
	Term_putstr(col+8, row + 4, -1, ((p_ptr->lev >= p_ptr->max_lev) ? TERM_L_BLUE : TERM_YELLOW),
				format("%4d", p_ptr->lev));

	/* Left */
	col = 1;

	/* Current dungeon level */
	Term_putstr(col, 10, -1, TERM_WHITE, "Depth"); 	 
  	 
	/* Has he actually left the town? */ 	 
	if (p_ptr->max_depth) 	 
	{ 	 
		/*express in feet or level*/ 	 
		if (depth_in_feet) strnfmt(buf, sizeof(buf), "%5d ft", p_ptr->depth * 50); 	 
		else strnfmt(buf, sizeof(buf), " Lev %3d", p_ptr->depth); 	 
	} 	 
	/*hasn't left town*/ 	 
	else strnfmt(buf, sizeof(buf), "    Town"); 	 
	
	Term_putstr(col+10, 10, -1, TERM_L_GREEN, buf);

	/* Max dungeon level */ 	 
	Term_putstr(col, 11, -1, TERM_WHITE, "MaxDepth"); 	 
  	 
	/* Has he actually left the town? */ 	 
	if (p_ptr->max_depth) 	 
	{ 	 
		/*express in feet or level*/ 	 
		if (depth_in_feet) strnfmt(buf, sizeof(buf), "%5d ft", p_ptr->max_depth * 50); 	 
		else strnfmt(buf, sizeof(buf), " Lev %3d", p_ptr->max_depth); 	 
	} 	 
	/*hasn't left town*/ 	 
	else strnfmt(buf, sizeof(buf), "    Town"); 	 
	
	Term_putstr(col+10, 11, -1, TERM_L_GREEN, buf);

	/* Game Turn */
	Term_putstr(col, 12, -1, TERM_WHITE, "Game Turn");
	Term_putstr(col+9, 12, -1, TERM_L_GREEN,
		            format("%9ld", turn));

	/* Player Game Turn */
	Term_putstr(col, 13, -1, TERM_WHITE, "Plr Turn");
	Term_putstr(col+9, 13, -1, TERM_L_GREEN,
		            format("%9ld", player_turn));

	/* Non-Resting Player Game Turn */
	Term_putstr(col, 14, -1, TERM_WHITE, "Act Turn");
	Term_putstr(col+9, 14, -1, TERM_L_GREEN,
		            format("%9ld", player_turn - resting_turn));

	/* Current Experience */
	Term_putstr(col, 15, -1, TERM_WHITE, "Curr Exp");
	if (p_ptr->exp >= p_ptr->max_exp)
	{
		Term_putstr(col+9, 15, -1, TERM_L_GREEN,
		            format("%9ld", p_ptr->exp));
	}
	else
	{
		Term_putstr(col+9, 15, -1, TERM_YELLOW,
		            format("%9ld", p_ptr->exp));
	}

	/* Maximum Experience */
	Term_putstr(col, 16, -1, TERM_WHITE, "Max Exp");
	Term_putstr(col+9, 16, -1, TERM_L_GREEN,
	            format("%9ld", p_ptr->max_exp));

	/* Advance Experience */
	Term_putstr(col, 17, -1, TERM_WHITE, "Adv Exp");
	if (p_ptr->lev < PY_MAX_LEVEL)
	{
		s32b advance = (player_exp[p_ptr->lev - 1] *
		                p_ptr->expfact / 100L);

		/*some players want to see experience needed to gain next level*/
		if (toggle_xp) advance -= p_ptr->exp;

		Term_putstr(col+9, 17, -1, TERM_L_GREEN,
		            format("%9ld", advance));
	}
	else
	{
		Term_putstr(col+9, 17, -1, TERM_L_GREEN,
		            format("%9s", "********"));
	}

	/* Middle left */
	col = 21;

	/* Hit Points */
	if (p_ptr->mhp > 99)
	{
		sprintf(buf, "%d/%d", p_ptr->chp, p_ptr->mhp);
		Term_putstr(col, 10, -1, TERM_WHITE, "HP");
		Term_putstr(col+8, 10, -1, TERM_L_BLUE, format("%9s", buf));
	}
	else 
	{
		sprintf(buf, "%d/%d", p_ptr->chp, p_ptr->mhp);
		Term_putstr(col, 10, -1, TERM_WHITE, "Hit-Points");
		Term_putstr(col+12, 10, -1, TERM_L_BLUE, format("%5s", buf));
	}

	/* Spell Points */
	if (p_ptr->msp > 99)
	{
		sprintf(buf, "%d/%d", p_ptr->csp, p_ptr->msp);
		Term_putstr(col, 11, -1, TERM_WHITE, "SP");
		Term_putstr(col+8, 11, -1, TERM_L_BLUE, format("%9s", buf));
	}
	else 
	{
		sprintf(buf, "%d/%d", p_ptr->csp, p_ptr->msp);
		Term_putstr(col, 11, -1, TERM_WHITE, "Spell-Pts");
		Term_putstr(col+12, 11, -1, TERM_L_BLUE, format("%5s", buf));
	}

	/* Armor */
	base = p_ptr->dis_ac;
	plus = p_ptr->dis_to_a;

	/* Total Armor */
	strnfmt(buf, sizeof(buf), "[%d,%+d]", base, plus);
	Term_putstr(col, 12, -1, TERM_WHITE, "Armor");
	Term_putstr(col+8, 12, -1, TERM_L_BLUE, format("%9s", buf));

	/* Infra */
	strnfmt(buf, sizeof(buf), "%d ft", p_ptr->see_infra * 10);
	Term_putstr(col, 13, -1, TERM_WHITE, "Infravis");
	Term_putstr(col+8, 13, -1, TERM_L_BLUE, format("%9s", buf));

	/* Gold */
	Term_putstr(col, 14, -1, TERM_WHITE, "Gold");
	Term_putstr(col+4, 14, -1, TERM_L_BLUE,
	            format("%13ld", p_ptr->au));

	/* Burden (in pounds) */
	strnfmt(buf, sizeof(buf), "%ld.%ld lbs",
	        p_ptr->total_weight / 10L,
	        p_ptr->total_weight % 10L);
	Term_putstr(col, 15, -1, TERM_WHITE, "Burden");

	/* calculate burden as a % of character's max burden */
	strnfmt(buf, sizeof(buf), format("%6ld lbs", p_ptr->total_weight / 10L, p_ptr->total_weight % 10L));
	Term_putstr(col+7, 15, -1, TERM_L_BLUE, buf);

	/* Now print burden as a percentage of carrying capacity */
	tmpl = ((p_ptr->total_weight * 10L) / adj_str_wgt[p_ptr->stat_ind[A_STR]]) / 10L;
	Term_putstr(col, 16, -1, TERM_WHITE, "% Burden");

	/* output, but leave a space for the % */
	strnfmt(buf, sizeof(buf), format("%8ld", tmpl));
	Term_putstr(col+8, 16, -1, (tmpl < 100L) ? TERM_L_BLUE : TERM_YELLOW, buf);

	/* Hack - add the % at the end */
	sprintf(buf, "%%");
	Term_putstr(col+16, 16, -1, (tmpl < 100L) ? TERM_L_BLUE : TERM_YELLOW, buf);

	/*get the player's speed*/
	i = p_ptr->pspeed;

	/* Hack -- Visually "undo" the Search Mode Slowdown */
	if (p_ptr->searching) i += 10;

	/* Hack -- Visually "undo" temp speed */
	if (p_ptr->fast) i -= 10;

	/* Hack -- Visually "undo" temp slowing */
	if (p_ptr->slow) i += 10;

	/* Fast */
	if (i > 110)
	{
		sprintf(buf, "+%d", (i - 110));
	}

	/* Slow */
	else if (i < 110)
	{
		sprintf(buf, "-%d", (110 - i));
	}

	else
	{
		 sprintf(buf, "Normal");
	}

	/* Speed */
	Term_putstr(col, 17, -1, TERM_WHITE, "Speed");
	Term_putstr(col+8, 17, -1, TERM_L_BLUE, format("%9s", buf));

	/* Middle */
	col = 40;

	/* Base skill */
	hit = p_ptr->dis_to_h;
	dam = p_ptr->dis_to_d;

	/* Basic fighting */
	Term_putstr(col, 10, -1, TERM_WHITE, "Fight");
	strnfmt(buf, sizeof(buf), "(%+d,%+d)", hit, dam);
	Term_putstr(col+5, 10, -1, TERM_L_BLUE, format("%12s", buf));

	/* Melee weapon */
	o_ptr = &inventory[INVEN_WIELD];

	/* Base skill */
	hit = p_ptr->dis_to_h;
	dam = p_ptr->dis_to_d;
	hit_real = p_ptr->to_h;

	/* Check melee styles only */
	style = p_ptr->cur_style & (WS_WIELD_FLAGS);

	/* Get style benefits */
	mon_style_benefits(NULL, style, &style_hit, &style_dam, &style_crit);
	hit += style_hit;
	dam += style_dam;
	hit_real += style_hit;

	/* Apply weapon bonuses */
	if (object_bonus_p(o_ptr)) hit += o_ptr->to_h;
	if (object_bonus_p(o_ptr)) dam += o_ptr->to_d;
	hit_real += o_ptr->to_h;

	/* Fighting Skill (with current weapon) */
	xthn = p_ptr->skill_thn + (hit_real * BTH_PLUS_ADJ);

	/* Melee attacks */
	strnfmt(buf, sizeof(buf), "(%+d,%+d)", hit, dam);
	Term_putstr(col+5, 11, -1, TERM_L_BLUE, format("%12s", buf));

	if (p_ptr->cur_style & (1L << WS_UNARMED))
		Term_putstr(col, 11, -1, TERM_WHITE, "Unarm");
	else if (p_ptr->cur_style & (1L << WS_ONE_HANDED)
			 || (!(p_ptr->pstyle == WS_TWO_HANDED)
				 && p_ptr->cur_style & (1L << WS_TWO_HANDED)))
		Term_putstr(col, 11, -1, TERM_WHITE, "1Hand");
	else if (p_ptr->pstyle == WS_TWO_HANDED
			 && p_ptr->cur_style & (1L << WS_TWO_HANDED))
		Term_putstr(col, 11, -1, TERM_WHITE, "BothH");
	else if (p_ptr->cur_style & (1L << WS_TWO_WEAPON)) 
	{
		Term_putstr(col, 11, -1, TERM_WHITE, "2Weap");

		/* Off-hand melee weapon */
		assert (inventory[INVEN_ARM].k_idx);

		o_ptr = &inventory[INVEN_ARM];

		/* Base skill */
		hit = p_ptr->dis_to_h;
		dam = p_ptr->dis_to_d;

		/* Check melee styles only */
		style = p_ptr->cur_style & (WS_WIELD_FLAGS);

		/* Get style benefits */
		mon_style_benefits(NULL, style, &style_hit, &style_dam, &style_crit);
		hit += style_hit;
		dam += style_dam;

		/* Apply weapon bonuses */
		if (object_bonus_p(o_ptr)) hit += o_ptr->to_h;
		if (object_bonus_p(o_ptr)) dam += o_ptr->to_d;

		Term_putstr(col, 12, -1, TERM_WHITE, "Off-H");
		strnfmt(buf, sizeof(buf), "(%+d,%+d)", hit, dam);
		Term_putstr(col+5, 12, -1, TERM_L_BLUE, format("%12s", buf));
	}
	else if (p_ptr->cur_style & (1L << WS_WEAPON_SHIELD)) 
	{
		Term_putstr(col, 11, -1, TERM_WHITE, "WShld");

		/* Shield stats (if shield bashes are in, display bash value) */
		assert (inventory[INVEN_ARM].k_idx);

		o_ptr = &inventory[INVEN_ARM];

		base = o_ptr->ac;
		plus = 0;
		if (object_bonus_p(o_ptr)) plus += o_ptr->to_a;

		Term_putstr(col, 12, -1, TERM_WHITE, "Bash ");
		strnfmt(buf, sizeof(buf), "[%d,%+d]", base, plus);
		Term_putstr(col+5, 12, -1, TERM_L_BLUE, format("%12s", buf));
	}
	else plog_fmt("Illegal melee style in display_player_xtra_info: %d",
				  p_ptr->cur_style);

	/* Off-hand */
	if (inventory[INVEN_ARM].k_idx)
	{
		o_ptr = &inventory[INVEN_ARM];
	
		if (o_ptr->tval == TV_SHIELD 
			&& !(p_ptr->cur_style & (1L << WS_WEAPON_SHIELD)))
		{
			/* Shield stats (if shield bashes are in, display bash value) */
			base = o_ptr->ac;
			plus = 0;
			if (object_bonus_p(o_ptr)) plus += o_ptr->to_a;

			Term_putstr(col, 12, -1, TERM_WHITE, "Shield");
			strnfmt(buf, sizeof(buf), "[%d,%+d]", base, plus);
			Term_putstr(col+6, 12, -1, TERM_L_BLUE, format("%11s", buf));
		}
		else if (!(o_ptr->tval == TV_SHIELD)
				 && !(p_ptr->cur_style & (1L << WS_TWO_WEAPON)))
		{
			/* Unused weapon, e.g. in off-hand when fighting unarmed */
			Term_putstr(col, 12, -1, TERM_WHITE, "Unused");
		}
	}
	else
	{
		Term_putstr(col, 12, -1, TERM_WHITE, "Free");
	}

	/* Compute charging bonus */
	if (p_ptr->cur_style & (1L << WS_UNARMED)) 
		tmp = MAX(1, p_ptr->wt / cp_ptr->chg_weight);
	else 
		tmp = MAX(1, inventory[INVEN_WIELD].weight / cp_ptr->chg_weight);

	/* Blows */
	strnfmt(buf, sizeof(buf), "%d/turn(x%d)", p_ptr->num_blow, tmp);
	Term_putstr(col, 13, -1, TERM_WHITE, "Blows");
	Term_putstr(col+5, 13, -1, TERM_L_BLUE, format("%12s", buf));

	/* Shooter */
	o_ptr = &inventory[INVEN_BOW];

	/* Base skill */
	hit = p_ptr->dis_to_h;
	dam = 0;
	hit_real = p_ptr->to_h;

	/* Check shooting styles only */
	style = p_ptr->cur_style & WS_LAUNCHER_FLAGS;
	
	/* Get style benefits */
	mon_style_benefits(NULL, style, &style_hit, &style_dam, &style_crit);
	hit += style_hit;
	dam += style_dam;
	hit_real += style_hit;

	/* Apply weapon bonuses */
	if (object_bonus_p(o_ptr)) hit += o_ptr->to_h;
	if (object_bonus_p(o_ptr)) dam += o_ptr->to_d;
	hit_real += o_ptr->to_h;

	/* Shooting Skill (with current bow) */
	xthb = p_ptr->skill_thb + (hit_real * BTH_PLUS_ADJ);

	strnfmt(buf, sizeof(buf), "(%+d,%+d)", hit, dam);
	Term_putstr(col+5, 14, -1, TERM_L_BLUE, format("%12s", buf));

	/* Describe shooting styles */
	if (p_ptr->cur_style & (1L << WS_SLING)) 
		Term_putstr(col, 14, -1, TERM_WHITE, "Sling");
	else if (p_ptr->cur_style & (1L << WS_BOW)) 
		Term_putstr(col, 14, -1, TERM_WHITE, "Bow  ");
	else if (p_ptr->cur_style & (1L << WS_XBOW)) 
		Term_putstr(col, 14, -1, TERM_WHITE, "XBow ");
	else
		Term_putstr(col, 14, -1, TERM_WHITE, "Shoot");

	/* Shots */
	strnfmt(buf, sizeof(buf), "%d/turn(x%d)", p_ptr->num_fire, p_ptr->ammo_mult);
	Term_putstr(col, 15, -1, TERM_WHITE, "Shots");
	Term_putstr(col+5, 15, -1, TERM_L_BLUE, format("%12s", buf));

	/* Throwing attacks */
	hit = p_ptr->dis_to_h;
	dam = 0;
	hit_real = p_ptr->to_h;

	/* Get style benefits */
	mon_style_benefits(NULL, WS_THROWN_FLAGS, &style_hit, &style_dam, &style_crit);
	hit += style_hit;
	dam += style_dam;
	hit_real += style_hit;

	/* Throwing Skill */
	xtht = p_ptr->skill_tht + (hit_real * BTH_PLUS_ADJ);

	Term_putstr(col, 16, -1, TERM_WHITE, "Throw");
	strnfmt(buf, sizeof(buf), "(%+d,%+d)", hit, dam);
	Term_putstr(col+5, 16, -1, TERM_L_BLUE, format("%12s", buf));

	/* Throws */
	strnfmt(buf, sizeof(buf), "%d/turn(x%d)", p_ptr->num_throw, 2);
	Term_putstr(col, 17, -1, TERM_WHITE, "Hurls");
	Term_putstr(col+5, 17, -1, TERM_L_BLUE, format("%12s", buf));

	/* Right */
	col = 59;

	/* Basic abilities */
	xdis = p_ptr->skill_dis;
	xdev = p_ptr->skill_dev;
	xsav = p_ptr->skill_sav;
	xstl = p_ptr->skill_stl;
	xsrh = p_ptr->skill_srh;
	xfos = p_ptr->skill_fos;

	put_str("Fighting", 10, col);
	desc = likert(xthn, 12, &likert_attr);
	c_put_str(likert_attr, format("%9s", desc), 10, col+11);

	put_str("Shooting", 11, col);
	desc = likert(xthb, 12, &likert_attr);
	c_put_str(likert_attr, format("%9s", desc), 11, col+11);

	put_str("Throwing", 12, col);
	desc = likert(xtht, 12, &likert_attr);
	c_put_str(likert_attr, format("%9s", desc), 12, col+11);

	put_str("Stealth", 13, col);
	desc = likert(xstl, 1, &likert_attr);
	c_put_str(likert_attr, format("%9s", desc), 13, col+11);

	put_str("Save Throw", 14, col);
	desc = likert(xsav, 6, &likert_attr);
	c_put_str(likert_attr, format("%9s", desc), 14, col+11);

	put_str("Devices", 15, col);
	desc = likert(xdev, 6, &likert_attr);
	c_put_str(likert_attr, format("%9s", desc), 15, col+11);

	put_str("Disarming", 16, col);
	desc = likert(xdis, 8, &likert_attr);
	c_put_str(likert_attr, format("%9s", desc), 16, col+11);

	put_str("Perception", 17, col);
	desc = likert(xfos, 6, &likert_attr);
	c_put_str(likert_attr, format("%9s", desc), 17, col+11);

	/* Indent output by 1 character, and wrap at column 72 */
	text_out_wrap = 65;
	text_out_indent = 1;

	/* History */
	Term_gotoxy(text_out_indent, 19);
	text_out_to_screen(TERM_WHITE, p_ptr->history);

	/* Reset text_out() vars */
	text_out_wrap = 0;
	text_out_indent = 0;

}



/*
 * Obtain the "flags" for the player as if he was an item
 */
void player_flags(u32b *f1, u32b *f2, u32b *f3, u32b *f4)
{

	int i;

	/* Clear */
	(*f1) = (*f2) = (*f3) = (*f4) = 0L;

	/* Add racial flags */
	(*f1) |= rp_ptr->flags1;
	(*f2) |= rp_ptr->flags2;
	(*f3) |= rp_ptr->flags3;
	(*f4) |= rp_ptr->flags4;

	/*** Handle styles ***/
	for (i = 0;i< z_info->w_max;i++)
	{
		if (w_info[i].class != p_ptr->pclass) continue;

		if (w_info[i].level > p_ptr->lev) continue;

		/* Check for styles */
		if ((w_info[i].styles==0) || (w_info[i].styles & (p_ptr->cur_style & (1L << p_ptr->pstyle))))
		{
			switch (w_info[i].benefit)
			{
				case WB_RES_FEAR:
                                        (*f2) |= TR2_RES_FEAR;
					break;
                        }
		}

	}
}


/*
 * Equippy chars
 */
static void display_player_equippy(int y, int x)
{
	int i;

	byte a;
	char c;

	object_type *o_ptr;

	/* Dump equippy chars */
	for (i = INVEN_WIELD; i < END_EQUIPMENT; ++i)
	{
		/* Object */
		o_ptr = &inventory[i];

		/* Skip empty objects */
		if (!o_ptr->k_idx) continue;

		/* Get attr/char for display */
		a = object_attr(o_ptr);
		c = object_char(o_ptr);

		/* Dump */
		Term_putch(x+i-INVEN_WIELD, y, a, c);
	}
}


/*
 * Equippy chars
 */
static void display_home_equippy(int y, int x)
{
	int i;

	byte a;
	char c;

	object_type *o_ptr;

	store_type *st_ptr = &store[STORE_HOME];

	/* Dump equippy chars */
	for (i = 0; i < MAX_INVENTORY_HOME	; ++i)
	{
		/* Object */
		o_ptr = &st_ptr->stock[i];

		/* Skip empty objects */
		if (!o_ptr->k_idx) continue;

		/* Get attr/char for display */
		a = object_attr(o_ptr);
		c = object_char(o_ptr);

		/* Dump */
		Term_putch(x+i, y, a, c);

	}
}


/*
 * Hack -- see below
 */
static const byte display_player_flag_set[6] =
{
	2,
	2,
	3,
	1,
	4,
	3
};

/*
 * Hack -- see below
 */
static const u32b display_player_flag_head[6] =
{
	TR2_RES_ACID,
	TR2_RES_DARK,
	TR3_SLOW_DIGEST,
	TR1_SAVE,
	TR4_UNDEAD,
	TR3_DRAIN_HP
};

/*
 * Hack -- see below
 */
static cptr display_player_flag_names[6][10] =
{
	{
		" Acid:",	/* TR2_RES_ACID */
		" Elec:",	/* TR2_RES_ELEC */
		" Fire:",	/* TR2_RES_FIRE */
		" Cold:",	/* TR2_RES_COLD */
		" Pois:",	/* TR2_RES_POIS */
		" Fear:",	/* TR2_RES_FEAR */
		" Lite:",	/* TR2_RES_LITE */
		" Dark:",	/* TR2_RES_DARK */
		"Water:",	/* TR4_HURT_WATER */
		""
	},

	{
		"Disea:",	/* TR4_RES_DISEASE */
		"Blind:",	/* TR2_RES_BLIND */
		"Confu:",	/* TR2_RES_CONFU */
		"Sound:",	/* TR2_RES_SOUND */
		"Shard:",	/* TR2_RES_SHARD */
		"Nexus:",	/* TR2_RES_NEXUS */
		"Nethr:",	/* TR2_RES_NETHR */
		"Chaos:",	/* TR2_RES_CHAOS */
		"Disen:",	/* TR2_RES_DISEN */
		""
	},

	{
		" Food:",	/* TR3_SLOW_DIGEST */
		"Feath:",	/* TR3_FEATHER */
		" Glow:",	/* TR3_LITE */
		"Regen:",	/* TR3_REGEN */
		"Telep:",	/* TR3_TELEPATHY */
		"Invis:",	/* TR3_SEE_INVIS */
		"FrAct:",	/* TR3_FREE_ACT */
		"HLife:",	/* TR3_HOLD_LIFE */
		"Activ:",	/* TR3_ACTIVATE */
		"Throw:"	/* TR3_THROWING */
	},

	{
		" Save:",	/* TR1_SAVE */
		"Devic:",	/* TR1_DEVICE */
		"Steal:",	/* TR1_STEALTH */
		"Sear.:",	/* TR1_SEARCH */
		"Infra:",	/* TR1_INFRA */
		"Tunn.:",	/* TR1_TUNNEL */
		"Speed:",	/* TR1_SPEED */
		"Blows:",	/* TR1_BLOWS */
		"Shots:",	/* TR1_SHOTS */
		"Might:"	/* TR1_MIGHT */
	},

	{
		"Undea:",	/* TR4_UNDEAD */
		"Demon:",	/* TR4_DEMON */
		"  Orc:",	/* TR4_ORC */
		"Troll:",	/* TR4_TROLL */
		"Giant:",	/* TR4_GIANT */
		"Dragn:",	/* TR4_DRAGON */
		"  Man:",	/* TR4_MAN */
		"Dwarf:",	/* TR4_DWARF */
		"  Elf:",	/* TR4_ELF */
		"Natur:"	/* TR4_NATURE */
	},

	{
		"Healt:",	/* TR3_DRAIN_HP */
		" Mana:",	/* TR3_DRAIN_MANA */
		"Exper:",	/* TR3_DRAIN_EXP */
		"",		/* xxx */
		"Telep:",	/* TR3_TELEPORT / TR4_ANCHOR */
		"Spell:",	/* TR4_SILENT */
		"",		/* xxx */
		"",		/* xxx */
		" Evil:",	/* TR4_EVIL */
		""		/* xxx */
	}
};



/*
 *
 */
static void put_flag_char(u32b f[4], int set, u32b flag, int y, int x, int row, int col, int pval)
{
	/* Low resists */
	if (x == 0)
	{
		/* Hack -- Check immunities */
		if ((y < 4) &&
		    (f[2] & ((TR2_IM_ACID) << y)))
		{
			c_put_str(TERM_WHITE, "*", row, col);
		}

		/* MegaHack -- Check immunity to poison */
		else if ((y == 4) &&
			(f[4] & (TR4_IM_POIS)))
		{
			c_put_str(TERM_WHITE, "*", row, col);
		}

		/* Hack -- Check vulnerabilities */
		else if ((y < 4) &&
		    (f[4] & ((TR4_HURT_ACID) << y)))
		{
			c_put_str(TERM_RED, "-", row, col);
		}

		/* MegaHack -- Check vulnerability to poison */
		else if ((y == 4) &&
			(f[4] & (TR4_HURT_POIS)))
		{
			c_put_str(TERM_RED, "-", row, col);
		}

		/* MegaHack -- Check vulnerability to lite */
		else if ((y == 6) &&
			(f[4] & (TR4_HURT_LITE)))
		{
			c_put_str(TERM_RED, "-", row, col);
		}

		/* MegaHack -- Check vulnerability to water */
		else if ((y == 8) &&
			(f[4] & (TR4_HURT_WATER)))
		{
			c_put_str(TERM_RED, "-", row, col);
		}

		/* MegaHack -- Check resistance to water */
		else if ((y == 8) &&
			(f[4] & (TR4_RES_WATER)))
		{
			c_put_str(TERM_WHITE, "+", row, col);
		}

		/* MegaHack -- Skip water */
		else if (y == 8)
		{
			c_put_str(TERM_SLATE, ".", row, col);
		}

		/* Check flags */
		else if (f[set] & flag)
		{
			c_put_str(TERM_WHITE, "+", row, col);
		}

		/* Default */
		else
		{
			c_put_str(TERM_SLATE, ".", row, col);
		}
	}

	/* High resists */
	else if (x == 1)
	{
		/* MegaHack -- Check resistance to disease */
		if ((y == 0) &&
			(f[4] & (TR4_RES_DISEASE)))
		{
			c_put_str(TERM_WHITE, "+", row, col);
		}

		/* MegaHack -- Skip disease */
		else if ((y == 0) &&
			!(f[4] & (TR4_RES_DISEASE)))
		{
			c_put_str(TERM_SLATE, ".", row, col);
		}

		/* Check flags */
		else if (f[set] & flag)
		{
			c_put_str(TERM_WHITE, "+", row, col);
		}

		/* Default */
		else
		{
			c_put_str(TERM_SLATE, ".", row, col);
		}
	}

	/* Abilities */
	else if (x == 2)
	{
		/* MegaHack -- Check hunger */
		if ((y == 4) &&
			(f[3] & (TR3_HUNGER)))
		{
			c_put_str(TERM_RED, "-", row, col);
		}

		/* MegaHack -- Check activation */
		else if ((y == 8) &&
			(f[3] & (TR3_ACTIVATE)))
		{
			c_put_str(TERM_WHITE, "+", row, col);
		}

		/* MegaHack -- Skip activation */
		else if ((y == 8) &&
			!(f[3] & (TR3_ACTIVATE)))
		{
			c_put_str(TERM_SLATE, ".", row, col);
		}

		/* MegaHack -- Check throwing */
		else if ((y == 9) &&
			(f[3] & (TR3_THROWING)))
		{
			c_put_str(TERM_WHITE, "+", row, col);
		}

		/* MegaHack -- Skip throwing */
		else if ((y == 9) &&
			!(f[3] & (TR3_THROWING)))
		{
			c_put_str(TERM_SLATE, ".", row, col);
		}

		/* MegaHack -- Display light pval */
		else if ((y == 2) && (f[set] & flag))
		{
			byte a;

			/* Default */
			char c = '*';

			/* Good */
			if (pval > 0)
			{
				/* Good */
				a = TERM_L_GREEN;

				/* Label boost */
				if (pval < 10) c = I2D(pval);
			}

			/* Bad */
			else if (pval < 0)
			{
				/* Bad */
				a = TERM_RED;

				/* Label boost */
				if (pval > -10) c = I2D(-(pval));
			}

			/* Unknown */
			else
			{
				/* Bad */
				a = TERM_SLATE;
				c = '?';
			}

			/* Display it */
			c_put_str(a, format("%c", c), row, col);
		}

		/* Check flags */
		else if (f[set] & flag)
		{
			c_put_str(TERM_WHITE, "+", row, col);
		}

		/* Default */
		else
		{
			c_put_str(TERM_SLATE, ".", row, col);
		}
	}

	/* Pval dependent abilities */
	else if (x == 3)
	{
		/* MegaHack -- Check static */
		if ((y == 1) &&
			(f[4] & (TR4_STATIC)))
		{
			c_put_str(TERM_RED, "x", row, col);
		}

		/* MegaHack -- Check aggravation */
		else if ((y == 2) &&
			(f[3] & (TR3_AGGRAVATE)))
		{
			c_put_str(TERM_RED, "x", row, col);
		}

		/* MegaHack -- Check windy */
		else if ((y == 9) &&
			(f[4] & (TR4_WINDY)))
		{
			c_put_str(TERM_RED, "x", row, col);
		}

		/* Check flags */
		else if (f[set] & flag)
		{
			byte a;

			/* Default */
			char c = '*';

			/* Good */
			if (pval > 0)
			{
				/* Good */
				a = TERM_L_GREEN;

				/* Label boost */
				if (pval < 10) c = I2D(pval);
			}

			/* Bad */
			else if (pval < 0)
			{
				/* Bad */
				a = TERM_RED;

				/* Label boost */
				if (pval > -10) c = I2D(-(pval));
			}

			/* Unknown */
			else
			{
				/* Bad */
				a = TERM_SLATE;
				c = '?';
			}

			/* Display it */
			c_put_str(a, format("%c", c), row, col);
		}

		/* Default */
		else
		{
			c_put_str(TERM_SLATE, ".", row, col);
		}

	}

	/* Racial sensing / marking */
	else if (x == 4)
	{
		/* MegaHack -- Sense undead */
		if ((y == 0) &&
			(f[3] & (TR3_ESP_UNDEAD)))
		{
			if (f[set] & flag) c_put_str(TERM_WHITE, "*", row, col);
			else c_put_str(TERM_WHITE, "s", row, col);
		}

		/* MegaHack -- Sense demon */
		else if ((y == 1) &&
			(f[3] & (TR3_ESP_DEMON)))
		{
			if (f[set] & flag) c_put_str(TERM_WHITE, "*", row, col);
			else c_put_str(TERM_WHITE, "s", row, col);
		}

		/* MegaHack -- Sense orc */
		else if ((y == 2) &&
			(f[3] & (TR3_ESP_ORC)))
		{
			if (f[set] & flag) c_put_str(TERM_WHITE, "*", row, col);
			else c_put_str(TERM_WHITE, "s", row, col);
		}

		/* MegaHack -- Sense troll */
		else if ((y == 3) &&
			(f[3] & (TR3_ESP_TROLL)))
		{
			if (f[set] & flag) c_put_str(TERM_WHITE, "*", row, col);
			else c_put_str(TERM_WHITE, "s", row, col);
		}

		/* MegaHack -- Sense giant */
		else if ((y == 4) &&
			(f[3] & (TR3_ESP_GIANT)))
		{
			if (f[set] & flag) c_put_str(TERM_WHITE, "*", row, col);
			else c_put_str(TERM_WHITE, "s", row, col);
		}

		/* MegaHack -- Sense dragon */
		else if ((y == 0) &&
			(f[3] & (TR3_ESP_DRAGON)))
		{
			if (f[set] & flag) c_put_str(TERM_WHITE, "*", row, col);
			else c_put_str(TERM_WHITE, "s", row, col);
		}

		/* MegaHack -- Sense nature / mark animal */
		else if (y == 9)
		{
			if ((f[4] & (TR4_ANIMAL)) && (f[3] & (TR3_ESP_NATURE))) c_put_str(TERM_WHITE, "*", row, col);
			else if (f[3] & (TR3_ESP_NATURE)) c_put_str(TERM_WHITE, "s", row, col);
			else if (f[4] & (TR4_ANIMAL)) c_put_str(TERM_WHITE, "+", row, col);
			else c_put_str(TERM_SLATE, ".", row, col);
		}

		/* Check flags */
		else if (f[set] & flag)
		{
			c_put_str(TERM_WHITE, "+", row, col);
		}

		/* Default */
		else
		{
			c_put_str(TERM_SLATE, ".", row, col);
		}

	}

	/* Negative abilities */
	else if (x == 5)
	{
		/* MegaHack -- Check anchor/teleport */
		if (y == 4)
		{
			if (f[4] & (TR4_ANCHOR)) c_put_str(TERM_RED, "x", row, col);
			else if (f[3] & (TR3_TELEPORT)) c_put_str(TERM_WHITE, "+", row, col);
			else c_put_str(TERM_SLATE, ".", row, col);
		}

		/* MegaHack -- Check spellcasting */
		else if (y == 5)
		{
			if (f[4] & (TR4_SILENT)) c_put_str(TERM_RED, "x", row, col);
			else c_put_str(TERM_SLATE, ".", row, col);
		}

		/* MegaHack -- Mark evil */
		else if (y == 8)
		{
			if (f[4] & (TR4_EVIL)) c_put_str(TERM_WHITE, "+", row, col);
			else c_put_str(TERM_SLATE, ".", row, col);
		}

		/* Check flags */
		else if (f[set] & flag)
		{
			c_put_str(TERM_RED, "-", row, col);
		}

		/* MegaHack -- Display health/mana regen pval */
		else if (((y == 0) && (f[3] & TR3_REGEN_HP))
			|| ((y == 1) && (f[3] & TR3_REGEN_MANA)))
		{
			byte a;

			/* Default */
			char c = '*';

			/* Good */
			if (pval > 0)
			{
				/* Good */
				a = TERM_L_GREEN;

				/* Label boost */
				if (pval < 10) c = I2D(pval);
			}

			/* Bad */
			else if (pval < 0)
			{
				/* Bad */
				a = TERM_RED;

				/* Label boost */
				if (pval > -10) c = I2D(-(pval));
			}

			/* Unknown */
			else
			{
				/* Bad */
				a = TERM_SLATE;
				c = '?';
			}

			/* Display it */
			c_put_str(a, format("%c", c), row, col);
		}

		/* Default */
		else
		{
			c_put_str(TERM_SLATE, ".", row, col);
		}
	}
}

/*
 * Special display, part 1
 */
static void display_player_flag_info(int mode)
{
	int x, y, i, n;

	int row, col;

	int set;
	u32b head;
	u32b flag;
	cptr name;

	u32b f[5];

	int xmin = (mode == 1 ? 0 : 4);
	int xmax = (mode == 1 ? 4 : 6);

	/* Four or two columns */
	for (x = xmin; x < xmax; x++)
	{
		/* Reset */
		row = 10;
		col = 20 * (x - xmin);

		/* Extract set */
		set = display_player_flag_set[x];

		/* Extract head */
		head = display_player_flag_head[x];

		/* Header */
		c_put_str(TERM_WHITE, "abcdefghijkl@", row++, col+6);

		/* Ten rows */
		for (y = 0; y < 10; y++)
		{
			/* Extract name */
			name = display_player_flag_names[x][y];

			/* MegaHack -- skip empty rows */
			if (strlen(name) == 0) continue;

			/* Header */
			c_put_str(TERM_WHITE, name, row, col);

			/* Extract flag */
			flag = (head << y);

			/* Check equipment */
			for (n = 6, i = INVEN_WIELD; i < END_EQUIPMENT; ++i, ++n)
			{
				object_type *o_ptr;

				/* Object */
				o_ptr = &inventory[i];

				/* Known flags */
				object_flags_known(o_ptr, &f[1], &f[2], &f[3], &f[4]);

				/* Non-existant objects */
				if (!o_ptr->k_idx)
				{
					c_put_str(TERM_L_DARK, ".", row, col + n);
				}
				else
				{
					/* Put flag in column */
					put_flag_char(f, set, flag, y, x, row, col+n, o_ptr->pval);
				}
			}

			/* Player flags */
			player_flags(&f[1], &f[2], &f[3], &f[4]);

			/* Put flag in column */
			put_flag_char(f, set, flag, y, x, row, col+n, 10);

			/* Advance */
			row++;
		}

		/* Footer */
		c_put_str(TERM_WHITE, "abcdefghijkl@", row++, col+6);

		/* Equippy */
		display_player_equippy(row++, col+6);
	}
}


/*
 * Special display, part 2a
 */
static void display_player_misc_info(void)
{
	cptr p;

	int n;

	char buf[80];

	/* Upper left */

	/* Name */
	put_str("Name", 1, 1);

	c_put_str(TERM_L_BLUE, op_ptr->full_name, 1, 8);

	/* Sex */
	put_str("Sex", 2, 1);
	c_put_str(TERM_L_BLUE, sp_ptr->title, 2, 8);


	/* Race */
	put_str("Race", 3, 1);
	c_put_str(TERM_L_BLUE, p_name + rp_ptr->name, 3, 8);


	/* Class */
	put_str("Class", 4, 1);

	c_put_str(TERM_L_BLUE, c_name + cp_ptr->name, 4, 8);

	/* Style */
	put_str("Style", 5, 1);

	if (p_ptr->psval)
	{
		int t, u;

	        switch (p_ptr->pstyle)
		{
			case WS_MAGIC_BOOK:
                        	t = TV_MAGIC_BOOK;
                        	break;

                	case WS_PRAYER_BOOK:
                        	t = TV_PRAYER_BOOK;
                        	break;

                	case WS_SONG_BOOK:
                        	t = TV_SONG_BOOK;
                        	break;

			default:
				t = TV_RUNESTONE;
				break;
		}

		u = lookup_kind(t, p_ptr->psval);

		p = k_name + k_info[u].name + 3;

        }
	else
		p = w_name_style[p_ptr->pstyle];

	/* Hack -- force string to fit */
	if (strlen(p) >= 24)
	{
		/* Copy first word of name into temporary buffer */
		for (n = 0; (*p) && (*p != ' ') && (n < 24); n++, p++)
		{
			buf[n] = *p;
		}

		buf[n] = '\0';

		/* Point to buffer */
		p = buf;
	}

	/* Dump it */
	c_put_str(TERM_L_BLUE, p, 5, 8);

	/* Title */
	put_str("Title", 6, 1);

	/* Title: Wizard */
	if (p_ptr->wizard)
	{
		strnfmt(buf, sizeof(buf), "[=-WIZARD-=]");
	}

	/* Title: Winner */
	else if (p_ptr->total_winner || (p_ptr->lev > PY_MAX_LEVEL))
	{
		strnfmt(buf, sizeof(buf), "***WINNER***");
	}

	/* Title: Normal */
	else
	{
		strnfmt(buf, sizeof(buf), "%s", c_text + cp_ptr->title[(p_ptr->lev - 1) / 5]);
	}

	c_put_str(TERM_L_BLUE, buf, 6, 8);
}


/*
 * Special display, part 2b
 */
void display_player_stat_info(int row, int col, int min, int max, int attr)
{
	int i;

	char buf[80];

	/* Hack -- display header if displaying first stat only */
	if (min == 0)
	{
		/* Print out the labels for the columns */
		c_put_str(TERM_WHITE, "  Self ", row-1, col+5);
		c_put_str(TERM_WHITE, " EB ", row-1, col+12);
		c_put_str(TERM_WHITE, " TB ", row-1, col+16);
		c_put_str(TERM_WHITE, "  Best ", row-1, col+20);
		c_put_str(TERM_WHITE, "  Curr", row-1, col+27);
	}

	/* Display the stats */
	for (i = min; i < max; i++)
	{
		/* Reduced */
		if (p_ptr->stat_use[i] < p_ptr->stat_top[i])
		{
			/* Use lowercase stat name */
			c_put_str(attr, stat_names_reduced[i], row+i, col);
		}

		/* Normal */
		else
		{
			/* Assume uppercase stat name */
			c_put_str(attr, stat_names[i], row+i, col);
		}

		/* Indicate where increases are never higher than 5 points */
		if (p_ptr->stat_max[i] >= 18+80)
		{
			put_str("!", row+i, col+3);
		}

		/* Internal "natural" maximum value */
		cnv_stat(p_ptr->stat_max[i], buf);
		c_put_str(TERM_L_GREEN, buf, row+i, col+5);

		/* Equipment Bonus */
		sprintf(buf, "%+3d", p_ptr->stat_add[i]);
		c_put_str(TERM_L_BLUE, buf, row+i, col+12);

		/* Temporary Bonus */
		sprintf(buf, "%+3d", 
				(p_ptr->stat_inc_tim[i] ? 5 : 0) 
				- (p_ptr->stat_dec_tim[i] ? 5 : 0));
		c_put_str(TERM_L_BLUE, buf, row+i, col+16);

		/* Resulting "modified" maximum value */
		cnv_stat(p_ptr->stat_top[i], buf);
		c_put_str(TERM_L_GREEN, buf, row+i, col+20);

		/* Only display stat_use if not maximal */
		if (p_ptr->stat_use[i] < p_ptr->stat_top[i])
		{
			cnv_stat(p_ptr->stat_use[i], buf);
			c_put_str(TERM_YELLOW, buf, row+i, col+27);
		}
	}
}


/*
 * Special display, part 2c
 *
 * How to print out the modifications and sustains.
 * Positive mods with no sustain will be light green.
 * Positive mods with a sustain will be dark green.
 * Sustains (with no modification) will be a dark green 's'.
 * Negative mods (from a curse) will be red.
 * Huge mods (>9), like from MICoMorgoth, will be a '*'
 * No mod, no sustain, will be a slate '.'
 */
static void display_player_sust_info(void)
{
	int i, row, col, stat;

	object_type *o_ptr;
	u32b f1, f2, f3, f4;
	u32b ignore_f2, ignore_f3, ignore_f4;

	byte a;
	char c;

	/* Row */
	row = 2;

	/* Column */
	col = 32;

	/* Header */
	c_put_str(TERM_WHITE, "abcdefghijkl@", row-1, col);

	/* Process equipment */
	for (i = INVEN_WIELD; i < END_EQUIPMENT; ++i)
	{
		/* Get the object */
		o_ptr = &inventory[i];

		/* Get the "known" flags */
		object_flags_known(o_ptr, &f1, &f2, &f3, &f4);

		/* Hack -- assume stat modifiers are known */
		object_flags(o_ptr, &f1, &ignore_f2, &ignore_f3, &ignore_f4);

		/* Initialize color based of sign of pval. */
		for (stat = 0; stat < A_MAX - 1 /* AGI coupled with DEX */; stat++)
		{
			/* Default */
			a = TERM_SLATE;
			c = '.';

			/* Boost */
			if (f1 & (1<<stat))
			{
				/* Default */
				c = '*';

				/* Good */
				if (o_ptr->pval > 0)
				{
					/* Good */
					a = TERM_L_GREEN;

					/* Label boost */
					if (o_ptr->pval < 10) c = I2D(o_ptr->pval);
				}

				/* Bad */
				if (o_ptr->pval < 0)
				{
					/* Bad */
					a = TERM_RED;

					/* Label boost */
					if (o_ptr->pval > -10) c = I2D(-(o_ptr->pval));
				}
			}

			/* Sustain */
			if (f2 & (1<<stat))
			{
				/* Dark green */
				a = TERM_GREEN;

				/* Convert '.' to 's' */
				if (c == '.') c = 's';
			}

			/* Dump proper character */
			Term_putch(col, row+stat, a, c);
			if (stat == A_DEX) 
			  Term_putch(col, row+A_AGI, a, c);
		}

		/* Advance */
		col++;
	}

	/* Player flags */
	player_flags(&f1, &f2, &f3, &f4);

	/* Check stats */
	for (stat = 0; stat < A_MAX - 1 /* AGI coupled with DEX */; ++stat)
	{
		/* Default */
		a = TERM_SLATE;
		c = '.';

		/* Sustain */
		if (f2 & (1<<stat))
		{
			/* Dark green "s" */
			a = TERM_GREEN;
			c = 's';
		}

		/* Dump */
		Term_putch(col, row+stat, a, c);
		if (stat == A_DEX) 
		  Term_putch(col, row+A_AGI, a, c);
	}
}


/*
 * Special display, part 3
 *
 * Dump all info for the home equipment
 * Positive mods with no sustain will be light green.
 * Positive mods with a sustain will be dark green.
 * Sustains (with no modification) will be a dark green 's'.
 * Negative mods (from a curse) will be red.
 * Huge mods (>9), like from MICoMorgoth, will be a '*'
 * No mod, no sustain, will be a slate '.'
 * This is followed by the home equipment
 *
 */
static void display_home_equipment_info(int mode)
{
	int x, y, n, xmax, xmin;

	int set;
	u32b head;
	u32b flag;
	cptr name;

	u32b f[5];

	int i, row, col, stats;

	object_type *o_ptr;
	u32b f1, f2, f3, f4;
	u32b ignore_f2, ignore_f3, ignore_f4;

	byte a;
	char c;

	store_type *st_ptr = &store[STORE_HOME];

	/* Row */
	row = 2;

	/* Column */
	col = 7;

	/* Equippy */
	display_home_equippy(row-2, col);

	/* Header */
	c_put_str(TERM_WHITE, "abcdefghijklmnopqrstuvwx", row-1, col);

	/* Footer */
	c_put_str(TERM_WHITE, "abcdefghijklmnopqrstuvwx", row+8, col);

	/* Process home stats */
	for (i = 0; i < MAX_INVENTORY_HOME; ++i)
	{
		/* Object */
		o_ptr = &st_ptr->stock[i];

		/* Get the "known" flags */
		object_flags_known(o_ptr, &f1, &f2, &f3, &f4);

		/* Hack -- assume stat modifiers are known */
		object_flags(o_ptr, &f1, &ignore_f2, &ignore_f3, &ignore_f4);

		/* Initialize color based of sign of pval. */
		for (stats = 0; stats < A_MAX - 1 /* AGI coupled with DEX */; stats++)
		{
			/* Assume uppercase stat name */
			c_put_str(TERM_WHITE, stat_names[stats], row+stats, 2);

			/* Default */
			a = TERM_SLATE;
			c = '.';

			/* Boost */
			if (f1 & (1<<stats))
			{
				/* Default */
				c = '*';

				/* Good */
				if (o_ptr->pval > 0)
				{
					/* Good */
					a = TERM_L_GREEN;

					/* Label boost */
					if (o_ptr->pval < 10) c = I2D(o_ptr->pval);
				}

				/* Bad */
				if (o_ptr->pval < 0)
				{
					/* Bad */
					a = TERM_RED;

					/* Label boost */
					if (o_ptr->pval > -10) c = I2D(-(o_ptr->pval));
				}
			}

			/* Sustain */
			if (f2 & (1<<stats))
			{
				/* Dark green */
				a = TERM_GREEN;

				/* Convert '.' to 's' */
				if (c == '.') c = 's';
			}

			/* Dump proper character */
			Term_putch(col, row+stats, a, c);
			if (stats == A_DEX) 
			  Term_putch(col, row+A_AGI, a, c);
		}

		/* Advance */
		col++;
	}

	/*alternate between resists and abilities depending on the modes*/
	if (mode == 3)
	{
		xmin = 0;
		xmax = 2;
	}

	else if (mode == 4)
	{
		xmin = 2;
		xmax = 4;
	}

	else if (mode == 5)
	{
		xmin = 4;
		xmax = 6;
	}

	/*paranoia*/
	else xmax = xmin = 0;

	/* Row */
	row = 11;

	/* Re-Set Column */
	col = 7;

	/* 2nd Header */
	c_put_str(TERM_WHITE, "abcdefghijklmnopqrstuvwx", row-1, col + MAX_INVENTORY_HOME + 8);

	/* Footer */
	c_put_str(TERM_WHITE, "abcdefghijklmnopqrstuvwx", row+10, col);

	/* 2nd Footer */
	c_put_str(TERM_WHITE, "abcdefghijklmnopqrstuvwx", row+10, col + MAX_INVENTORY_HOME + 8);

	/* 3rd Equippy */
	display_home_equippy(row+11, col);

	/* 4th Equippy */
	display_home_equippy(row+11,col+ MAX_INVENTORY_HOME + 8);

	/* Two Rows, alternating depending upon the mode */
	for (x = 0; xmin < xmax; ++xmin, ++x)
	{
		/* Reset */
		col = 32 * x;
		row = 11;

		/* Ten rows */
		for (y = 0; y < 10; y++)
		{
			/* Extract set */
			set = display_player_flag_set[xmin];

			/* Extract head */
			head = display_player_flag_head[xmin];

			/* Extract name */
			name = display_player_flag_names[xmin][y];

			/* MegaHack -- skip empty rows */
			if (strlen(name) == 0) continue;

			/* Extract flag */
			flag = (head << y);

			/* Check equipment */
			for (n = 7, i = 0; i < MAX_INVENTORY_HOME; ++i, ++n)
			{
				object_type *o_ptr;

				/* Object */
				o_ptr = &st_ptr->stock[i];

				/* Known flags */
				object_flags_known(o_ptr, &f[1], &f[2], &f[3], &f[4]);

				/* Non-existant objects */
				if (!o_ptr->k_idx)
				{
					c_put_str(TERM_L_DARK, ".", row, col + n);
				}
				else
				{
					/* Put flag in column */
					put_flag_char(f, set, flag, y, x, row, col+n, o_ptr->pval);
				}
			}

			/* Header */
			c_put_str(TERM_WHITE, name, row, col);

			/* Advance */
			row++;
		}
	}
}


/*
 * Display the character on the screen (in different modes)
 *
 * The top two lines, and the bottom line (or two) are left blank.
 *
 * Mode 0 = standard display with skills/history
 * Mode 1 = special display with equipment flags
 * Mode 2 = special display with equipment flags (columns 4 and 5)
 * Mode 3 = Home equiment Stat Flags and 1st part of Resists
 * Mode 4 = Home equiment Stat Flags and 2nd part of Resists
 * Mode 5 = Home equiment Stat Flags and 3rd part of Resists
 */
void display_player(int mode)
{
	/* Erase screen */
	clear_from(0);

	/* Stat info */
	display_player_stat_info(2, 46, 0, A_MAX, TERM_WHITE);

	/* Stat/Sustain flags */
	if (mode)
		display_player_sust_info();

	if ((mode) < 3)
	{
		/* Misc info */
		display_player_misc_info();

		/* Special */
		if (mode)
		{
			/* Other flags */
			display_player_flag_info(mode);
		}
		else
		{
			/* Extra info */
			display_player_xtra_info();
		}
	}

	else display_home_equipment_info(mode);
}


















static void dump_player_plus_minus(FILE *fff)
{
	int i, stats, modifier;

	u32b f1, f2, f3, f4;

	object_type *o_ptr;

	/* Print it out */
	for (i = INVEN_WIELD; i < END_EQUIPMENT; ++i)
	{

		/* Object */
		o_ptr = &inventory[i];

		/* Get the "known" flags */
		object_flags_known(o_ptr, &f1, &f2, &f3, &f4);

		modifier = FALSE;

		/*check to see if there is an increase or decrease of a stat*/
		for (stats = 0; stats < A_MAX - 1 /* AGI coupled with DEX */; stats++)
		{
			/* Boost */
			if (f1 & (1<<stats)) modifier = TRUE;
		}

		if (modifier)
		{
			/* positive pval */
			if (o_ptr->pval > 0)
			{
				/*Make it a space*/
				fprintf(fff,"+");

			}

			/* negative pval */
			else if (o_ptr->pval < 0)
			{
				/*Make it a space*/
				fprintf(fff,"-");

			}
		}

		/* Just a space */
		else fprintf(fff," ");
	}
}



static void dump_player_stat_info(FILE *fff)
{
	int i, x, y, stats;

	u32b f1, f2, f3, f4, fn;
	u32b ignore_f2, ignore_f3, ignore_f4;

	object_type *o_ptr;

	char c;

	char equippy[20];

	/* Build the equippy */
	for (x = 0,i = INVEN_WIELD; i < END_EQUIPMENT; ++i,++x)
	{

		/* Object */
		o_ptr = &inventory[i];

		/* empty objects */
		if (!o_ptr->k_idx)
		{
			/*Make it a space*/
			equippy[x] = ' ';

		}

		/* Get attr/char for display */
		else equippy[x] = object_char(o_ptr);
	}

	/*finish off the string*/
	equippy[x] = '\0';

	/*Hack - record spaces for the character*/

	fprintf(fff,"(+/-) ");

	dump_player_plus_minus(fff);

	fprintf(fff,"\n      %s               %s\n",equippy, equippy);

	fprintf(fff,"      abcdefghijkl@              abcdefghijkl@\n");

	for (stats = 0; stats < A_MAX; stats++)
	{
		fprintf(fff, "%6s", stat_names_reduced[stats]);

		/* Process equipment, show stat modifiers */
		for (x = 0, y = INVEN_WIELD; y < END_EQUIPMENT; ++y, ++x)
		{
			char c = '.';

			object_type *o_ptr;

			/* Get the object */
			o_ptr = &inventory[y];

			/* Get the "known" flags */
			object_flags_known(o_ptr, &f1, &f2, &f3, &f4);

			/* Hack -- assume stat modifiers are known */
			object_flags(o_ptr, &f1, &ignore_f2, &ignore_f3, &ignore_f4);

			/* Boost */
			if (f1 & (1<<(stats == A_AGI ? A_DEX : stats)))
			{
				/* Default */
				c = '*';

				/* Good */
				if (o_ptr->pval > 0)
				{

					/* Label boost */
					if (o_ptr->pval < 10) c = I2D(o_ptr->pval);

				}

				/* Bad */
				if (o_ptr->pval < 0)
				{
					/* Label boost */
					if (o_ptr->pval > -10) c = I2D(-(o_ptr->pval));

				}
				if (o_ptr->pval == 0) c = '.';

			}
			/*dump the result*/
			fprintf(fff,"%c",c);

		}

		/*a couple spaces, then do the sustains*/
		fprintf(fff, ".      %7s ", stat_names_reduced[stats]);

		/* Process equipment, show stat modifiers */
		for (y = INVEN_WIELD; y < END_EQUIPMENT; ++y)
		{

			object_type *o_ptr;

			/* Get the object */
			o_ptr = &inventory[y];

			/* Get the "known" flags */
			object_flags_known(o_ptr, &f1, &f2, &f3, &f4);

			/* Hack -- assume stat modifiers are known */
			object_flags(o_ptr, &f1, &ignore_f2, &ignore_f3, &ignore_f4);

			/* Sustain */
			if (f2 & (1<<(stats == A_AGI ? A_DEX : stats)))  c = 's';
			else c = '.';

			/*dump the result*/
			fprintf(fff,"%c",c);

		}

		/* Player flags */
		player_flags(&f1, &f2, &f3, &fn);

		/*default*/
		c = '.';

		/* Sustain */
		if (f2 & (1<<(stats == A_AGI ? A_DEX : stats))) c = 's';

		/*dump the result*/
		fprintf(fff,"%c",c);

		fprintf(fff,"\n");

	}

}


static void dump_home_plus_minus(FILE *fff)
{
	int i, stats, modifier;

	u32b f1, f2, f3, f4;

	object_type *o_ptr;
	store_type *st_ptr = &store[STORE_HOME];

	/* Print it out */
	for (i = 0; i < MAX_INVENTORY_HOME; ++i)
	{

		/* Object */
		o_ptr = &st_ptr->stock[i];

		/* Get the "known" flags */
		object_flags_known(o_ptr, &f1, &f2, &f3, &f4);

		modifier = FALSE;

		/*check to see if there is an increase or decrease of a stat*/
		for (stats = 0; stats < A_MAX - 1 /* AGI coupled with DEX */; stats++)
		{
			/* Boost */
			if (f1 & (1<<stats)) modifier = TRUE;
		}

		if (modifier)
		{
			/* positive pval */
			if (o_ptr->pval > 0)
			{
				/*Make it a space*/
				fprintf(fff,"+");

			}

			/* negative pval */
			else if (o_ptr->pval < 0)
			{
				/*Make it a space*/
				fprintf(fff,"-");

			}
		}

		/* Just a space */
		else fprintf(fff," ");
	}
}



static void dump_home_stat_info(FILE *fff)
{
	int i, stats;

	object_type *o_ptr;
	store_type *st_ptr = &store[STORE_HOME];
	u32b f1, f2, f3, f4;
	u32b ignore_f2, ignore_f3, ignore_f4;

	char c;
	char equippy[30];

	/* Print it out */
	for (i = 0; i < MAX_INVENTORY_HOME; ++i)
	{

		/* Object */
		o_ptr = &st_ptr->stock[i];

		/* empty objects */
		if (!o_ptr->k_idx)
		{
			/*Make it a space*/
			equippy[i] = ' ';

		}

		/* Get attr/char for display */
		else equippy[i] = object_char(o_ptr);
	}

	/*finish off the string*/
	equippy[i] = '\0';

	/*Hack - record spaces for the character*/

	fprintf(fff,"(+/-)  ");

	dump_home_plus_minus(fff);

	fprintf(fff,"\n       %s        %s\n", equippy, equippy);

	fprintf(fff,"       abcdefghijklmnopqrstuvwx        abcdefghijklmnopqrstuvwx\n");

	for (stats = 0; stats < A_MAX; stats++)
	{
		fprintf(fff, "%6s ", stat_names_reduced[stats]);

		/* Process home stats */
		for (i = 0; i < MAX_INVENTORY_HOME; ++i)
		{
			/* Object */
			o_ptr = &st_ptr->stock[i];

			/* Get the "known" flags */
			object_flags_known(o_ptr, &f1, &f2, &f3, &f4);

			/* Hack -- assume stat modifiers are known */
			object_flags(o_ptr, &f1, &ignore_f2, &ignore_f3, &ignore_f4);

			c = '.';

			/* Boost */
			if (f1 & (1<<(stats == A_AGI ? A_DEX : stats)))
			{
				/* Default */
				c = '*';

				/* Good */
				if (o_ptr->pval > 0)
				{

					/* Label boost */
					if (o_ptr->pval < 10) c = I2D(o_ptr->pval);

				}

				/* Bad */
				if (o_ptr->pval < 0)
				{
					/* Label boost */
					if (o_ptr->pval > -10) c = I2D(-(o_ptr->pval));

				}
				if (o_ptr->pval == 0) c = '.';

			}
			/*dump the result*/
			fprintf(fff,"%c",c);

		}

		/*a couple spaces, then do the sustains*/
		fprintf(fff, "  %5s ", stat_names_reduced[stats]);

		/* Process equipment, show stat modifiers */
		for (i = 0; i < MAX_INVENTORY_HOME; ++i)
		{

			/* Object */
			o_ptr = &st_ptr->stock[i];

			/* Get the "known" flags */
			object_flags_known(o_ptr, &f1, &f2, &f3, &f4);

			/* Hack -- assume stat modifiers are known */
			object_flags(o_ptr, &f1, &ignore_f2, &ignore_f3, &ignore_f4);

			/* Sustain */
			if (f2 & (1<<(stats == A_AGI ? A_DEX : stats)))  c = 's';
			else c = '.';

			/*dump the result*/
			fprintf(fff,"%c",c);

		}

		fprintf(fff, "\n");

	}

}



/*
 * Hack -- Dump a character description file
 *
 * XXX XXX XXX Allow the "full" flag to dump additional info,
 * and trigger its usage from various places in the code.
 */
errr file_character(cptr name, bool full)
{
	int i, w, x, y, z;

	byte a;
	char c;

	int fd;

	FILE *fff = NULL;

	store_type *st_ptr = &store[STORE_HOME];

	char o_name[80];

	char buf[1024];

	bool no_quests = TRUE;

	char buf2[20];

	int day = bst(DAY, turn);

	/* Format time of the day */
	strnfmt(buf2, 20, get_day(bst(YEAR, turn) + START_YEAR));

	/* Unused parameter */
	(void)full;

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, name);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Check for existing file */
	fd = fd_open(buf, O_RDONLY);

	/* Existing file */
	if (fd >= 0)
	{
		char out_val[160];

		/* Close the file */
		fd_close(fd);

		/* Build query */
		strnfmt(out_val, sizeof(out_val), "Replace existing file %s? ", buf);

		/* Ask */
		if (get_check(out_val)) fd = -1;
	}

	/* Open the non-existing file */
	if (fd < 0) fff = my_fopen(buf, "w");

	/* Invalid file */
	if (!fff) return (-1);

	text_out_hook = text_out_to_file;
	text_out_file = fff;

	/* Begin dump */
	text_out(format("  [%s %s Character Dump]\n\n",
	        VERSION_NAME, VERSION_STRING));

	/* Display player */
	display_player(0);

	/* Dump part of the screen */
	for (y = 1; y < 23; y++)
	{
		/* Dump each row */
		for (x = 0; x < 79; x++)
		{
			/* Get the attr/char */
			(void)(Term_what(x, y, &a, &c));

			/* Dump it */
			buf[x] = c;
		}

		/* Back up over spaces */
		while ((x > 0) && (buf[x-1] == ' ')) --x;

		/* Terminate */
		buf[x] = '\0';

		/* End the row -- hack: use fprintf to avoid line wrap*/
		fprintf(fff, "%s\n", buf);
	}

	/* Display current date in the Elvish calendar */
	text_out(format(" This is %s of the %s year of the third age.  ",
	           get_month_name(day, cheat_xtra, FALSE), buf2));

	/* Display location */
	if (p_ptr->depth == min_depth(p_ptr->dungeon))
	{
		text_out(format("You are in %s.", t_name + t_info[p_ptr->dungeon].name));
	}
	/* Display depth in feet */
	else if (depth_in_feet)
	{
		dungeon_zone *zone=&t_info[0].zone[0];

		/* Get the zone */
		get_zone(&zone,p_ptr->dungeon,p_ptr->depth);

		text_out(format("You are %d ft %s %s.",
			(p_ptr->depth - min_depth(p_ptr->dungeon)) * 50 ,
				zone->tower ? "high above" : "deep in",
					t_name + t_info[p_ptr->dungeon].name));
	}
	/* Display depth */
	else
	{
		text_out(format("You are on level %d of %s.",
			p_ptr->depth - min_depth(p_ptr->dungeon),
				t_name + t_info[p_ptr->dungeon].name));
	}

	/* Skip some lines */
	text_out("\n\n");

	/* If dead, dump last messages -- Prfnoff */
	if (p_ptr->is_dead)
	{
		i = message_num();
		if (i > 15) i = 15;
		text_out("  [Last Messages]\n\n");
		while (i-- > 0)
		{
			text_out(format("> %s\n", message_str((s16b)i)));
		}
		text_out("\n");
	}

	/* Dump the equipment */
	if (p_ptr->equip_cnt)
	{
		text_out("  [Character Equipment]\n\n");

		for (i = INVEN_WIELD; i < END_EQUIPMENT; i++)
		{
			object_desc(o_name, sizeof(o_name), &inventory[i],
				TRUE, 3);

			text_out(format("%c) %s\n", index_to_label(i), o_name));

			/* Describe random object attributes */
			identify_random_gen(&inventory[i]);
		}

		text_out("\n\n");
	}

	/* Dump the quiver */
	if (p_ptr->pack_size_reduce)
	{
		text_out("  [Character Equipment -- Quiver]\n\n");

		for (i = INVEN_QUIVER; i < END_QUIVER; i++)
		{
			/* Ignore empty slots */
			if (!inventory[i].k_idx) continue;

			object_desc(o_name, sizeof(o_name), &inventory[i],
				TRUE, 3);

			text_out(format("%c) %s\n", index_to_label(i), o_name));

			/* Describe random object attributes */
			identify_random_gen(&inventory[i]);
		}

		text_out("\n\n");
	}

	/* Dump the inventory */
	text_out("  [Character Inventory]\n\n");
	for (i = 0; i < INVEN_PACK; i++)
	{
		if (!inventory[i].k_idx) break;

		object_desc(o_name, sizeof(o_name), &inventory[i], TRUE, 3);
		text_out(format("%c) %s\n", index_to_label(i), o_name));

		/* Describe random object attributes */
		identify_random_gen(&inventory[i]);
	}

	text_out("\n");

	/* Dump the Home Flags , then the inventory -- if anything there */
	if (st_ptr->stock_num)
	{
		/* Header */
		text_out("  [Home Inventory]\n\n");

		/* Dump all available items */
		for (i = 0; i < st_ptr->stock_num; i++)
		{
			object_desc(o_name, sizeof(o_name), &st_ptr->stock[i], TRUE, 3);
			text_out(format("%c) %s\n", I2A(i), o_name));

			/* Describe random object attributes */
			identify_random_gen(&st_ptr->stock[i]);
		}

		/* Add an empty line */
		text_out("\n");
	}


	/* Dump dungeons */
	text_out("  [Dungeons Explored]\n\n");

	for (i = 0; i < z_info->t_max; i++)
	{
		if (t_info[i].max_depth)
		{
			text_out("You have reached ");

			/* Express in feet or level*/
			if (depth_in_feet) text_out(format("%d foot depth in ", t_info[i].max_depth * 50));
			else text_out(format("level %d of ",t_info[i].max_depth));

			text_out(t_name + t_info[i].name);
			text_out(".\n");
		}
	}

	text_out("\n");

	/* Dump self-knowledge */
	text_out("  [Self-Knowledge]\n\n");

	/* Do non-spoiler self-knowledge */
	self_knowledge_aux(FALSE, TRUE);

	text_out("\n");

	/* Dump spells learnt */
	if ((c_info[p_ptr->pclass].spell_first <= PY_MAX_LEVEL)
		|| (p_ptr->pstyle == WS_MAGIC_BOOK) || (p_ptr->pstyle == WS_PRAYER_BOOK) || (p_ptr->pstyle == WS_SONG_BOOK))
	{
		/* Dump quests */
		text_out("  [Spells Learnt]\n\n");

		/* Iterate through learnt spells */
		for (i = 0; i < PY_MAX_SPELLS;i++)
		{
			int spell = p_ptr->spell_order[i];

			if (spell)
			{
				if (i % 3 == 2) text_out(format("%s\n",s_name + s_info[spell].name));
				else text_out(format("%-25s",s_name + s_info[spell].name));
			}
		}

		text_out("\n\n");
	}

	/* Dump quests */
	text_out("  [Quests]\n\n");

	/* Warning */
	text_out("Quests will not be enabled until version 0.7.0.  This output is for testing only.\n");

	/* Either print live quests on level, or if nothing live, display current quests */
	if (print_quests(QUEST_ACTIVE , QUEST_REWARD)) no_quests = FALSE;
	else no_quests = !print_quests(QUEST_ASSIGN , QUEST_FINISH);

	/* No quests? */
	if (no_quests) text_out("You currently have no quests.\n");

	text_out("\n");

	text_out("  [Character Equipment Stat Modifiers, Sustains and Flags]\n\n");

	/*dump stat modifiers and sustains*/
	dump_player_stat_info(fff);

	/* Display player */
	display_player(1);

	/* Dump flags, but in two separate rows */
	for (w = 0; w < 2; w ++)
	{
		for (y = 11; y < 21; y++)
		{
			/* Dump each row */
			for (z = 0, x = 0; x < 79; x++, z++)
			{
				/* Get the attr/char */
				(void)(Term_what(x, y, &a, &c));

				/*Go through the whole thing twice, printing
				 *two of the sets of resist flags each time.
				 */
				if ((!w) && (x < 40))
				{
					/*hack - space it out a bit*/
					if (x == 20) text_out("       ");

					/* Dump it */
					text_out(format("%c", c));
				}

				else if ((w) && (x > 39))
				{
					/*hack - space it out a bit*/
					if (x == 60) text_out("       ");

					/* Dump it */
					text_out(format("%c", c));
				}

			}

			/* End the row */
			text_out("\n");
		}
	}

	/* Display player */
	display_player(2);

	/* Display remaining flags */
	for (y = 11; y < 23; y++)
	{
		/* Dump each row */
		for (x = 0; x < 40; x++)
		{
			/* Get the attr/char */
			(void)(Term_what(x, y, &a, &c));

			/*hack - space it out a bit*/
			if (x == 20) text_out("       ");

			/* Dump it */
			text_out(format("%c", c));
		}

		/* End the row */
		text_out("\n");
	}

	/* Create some space */
	text_out("\n\n");

	/* Dump the Home Flags , then the inventory -- if anything there */
	if (st_ptr->stock_num)
	{
		/* Header */
		text_out("  [Home Inventory Stat Modifiers, Sustains and Flags]\n\n");

		/*dump stat modifiers and sustains*/
		dump_home_stat_info(fff);

		/*dump the home resists and abilities display*/
		for (i =3; i <6; ++i)
		{
			/* Display player */
			display_player(i);

			/* Dump part of the screen */
			for (y = 10; y < 20; y++)
			{
				/* Dump each row */
				for (x = 0; x < 79; x++)
				{
					/* Get the attr/char */
					(void)(Term_what(x, y, &a, &c));

					/* Dump it */
					buf[x] = c;
				}

				/* Back up over spaces */
				while ((x > 0) && (buf[x-1] == ' ')) --x;

				/* Terminate */
				buf[x] = '\0';

				/* End the row */
				text_out(format("%s\n", buf));
			}
		}

		/* Display player */
		display_player(0);

		/* End the row */
		text_out("\n");
	}

	else text_out("  [Your Home Is Empty]\n\n");

	/* Dump options */
	text_out("  [Options]\n");

	/* Dump options */
	for (i = 0; i < OPT_MAX; i++)
	{
		/*hack - use game play options*/
		if (i < OPT_GAME_PLAY) continue;
		if ((i >= OPT_EFFICIENCY) && (i < OPT_ADULT)) continue;

		if (option_desc[i])
		{
			text_out(format("%-45s: %s (%s)\n",
			        format("%s%s", (i >= OPT_GAME_PLAY) && (i < OPT_EFFICIENCY) ? "Game: " : "", option_desc[i]),
			        op_ptr->opt[i] ? "yes" : "no ",
			        option_text[i]));
		}
	}

	/* Skip some lines */
	text_out("\n\n");

	/* Close it */
	my_fclose(fff);

	/* Success */
	return (0);
}


/*
 * Make a string lower case.
 */
static void string_lower(char *buf)
{
	char *s;

	/* Lowercase the string */
	for (s = buf; *s != 0; s++) *s = tolower(*s);
}


/*
 * Recursive file perusal.
 *
 * Return FALSE on "ESCAPE", otherwise TRUE.
 *
 * Process various special text in the input file, including the "menu"
 * structures used by the "help file" system.
 *
 * This function could be made much more efficient with the use of "seek"
 * functionality, especially when moving backwards through a file, or
 * forwards through a file by less than a page at a time.  XXX XXX XXX
 *
 * Consider using a temporary file, in which special lines do not appear,
 * and which could be pre-padded to 80 characters per line, to allow the
 * use of perfect seeking.  XXX XXX XXX
 *
 * Allow the user to "save" the current file.  XXX XXX XXX
 */
bool show_file(cptr name, cptr what, int line, int mode)
{
	int i, k, n;

	key_event ke;

	/* Number of "real" lines passed by */
	int next = 0;

	/* Number of "real" lines in the file */
	int size;

	/* Backup value for "line" */
	int back = 0;

	/* This screen has sub-screens */
	bool menu = FALSE;

	/* Case sensitive search */
	bool case_sensitive = FALSE;

	/* Current help file */
	FILE *fff = NULL;

	/* Find this string (if any) */
	char *find = NULL;

	/* Jump to this tag */
	cptr tag = NULL;

	/* Hold a string to find */
	char finder[80];

	/* Hold a string to show */
	char shower[80];

	/* Filename */
	char filename[1024];

	/* Describe this thing */
	char caption[128];

	/* Path buffer */
	char path[1024];

	/* General buffer */
	char buf[1024];

	/* Lower case version of the buffer, for searching */
	char lc_buf[1024];

	/* Sub-menu information */
	char hook[10][32];

	int wid, hgt;


	/* Wipe finder */
	strcpy(finder, "");

	/* Wipe shower */
	strcpy(shower, "");

	/* Wipe caption */
	strcpy(caption, "");

	/* Wipe the hooks */
	for (i = 0; i < 10; i++) hook[i][0] = '\0';

	/* Get size */
	Term_get_size(&wid, &hgt);

	/* Copy the filename */
	strncpy(filename, name, sizeof(filename));
	filename[sizeof(filename)-1] = '\0';

	n = strlen(filename);

	/* Extract the tag from the filename */
	for (i = 0; i < n; i++)
	{
		if (filename[i] == '#')
		{
			filename[i] = '\0';
			tag = filename + i + 1;
			break;
		}
	}

	/* Redirect the name */
	name = filename;


	/* Hack XXX XXX XXX */
	if (what)
	{
		/* Caption */
		strcpy(caption, what);

		/* Get the filename */
		strcpy(path, name);

		/* Open */
		fff = my_fopen(path, "r");
	}

	/* Look in "help" */
	if (!fff)
	{
		/* Caption */
		sprintf(caption, "Help file '%s'", name);

		/* Build the filename */
		path_build(path, 1024, ANGBAND_DIR_HELP, name);

		/* Open the file */
		fff = my_fopen(path, "r");
	}

	/* Look in "info" */
	if (!fff)
	{
		/* Caption */
		sprintf(caption, "Info file '%s'", name);

		/* Build the filename */
		path_build(path, 1024, ANGBAND_DIR_INFO, name);

		/* Open the file */
		fff = my_fopen(path, "r");
	}

	/* Oops */
	if (!fff)
	{
		/* Message */
		msg_format("Cannot open '%s'.", name);
		message_flush();

		/* Oops */
		return (TRUE);
	}


	/* Pre-Parse the file */
	while (TRUE)
	{
		/* Read a line or stop */
		if (my_fgets(fff, buf, sizeof(buf))) break;

		/* XXX Parse "menu" items */
		if (prefix(buf, "***** "))
		{
			char b1 = '[', b2 = ']';

			/* Notice "menu" requests */
			if ((buf[6] == b1) && isdigit(buf[7]) &&
			    (buf[8] == b2) && (buf[9] == ' '))
			{
				/* This is a menu file */
				menu = TRUE;

				/* Extract the menu item */
				k = D2I(buf[7]);

				/* Extract the menu item */
				strcpy(hook[k], buf + 10);
			}
			/* Notice "tag" requests */
			else if (buf[6] == '<')
			{
				if (tag)
				{
					/* Remove the closing '>' of the tag */
					buf[strlen(buf) - 1] = '\0';

					/* Compare with the requested tag */
					if (streq(buf + 7, tag))
					{
						/* Remember the tagged line */
						line = next;
					}
				}
			}

			/* Skip this */
			continue;
		}

		/* Count the "real" lines */
		next++;
	}

	/* Save the number of "real" lines */
	size = next;



	/* Display the file */
	while (TRUE)
	{
		/* Clear screen */
		Term_clear();


		/* Restart when necessary */
		if (line >= size) line = 0;


		/* Re-open the file if needed */
		if (next > line)
		{
			/* Close it */
			my_fclose(fff);

			/* Hack -- Re-Open the file */
			fff = my_fopen(path, "r");

			/* Oops */
			if (!fff) return (TRUE);

			/* File has been restarted */
			next = 0;
		}


		/* Goto the selected line */
		while (next < line)
		{
			/* Get a line */
			if (my_fgets(fff, buf, sizeof(buf))) break;

			/* Skip tags/links */
			if (prefix(buf, "***** ")) continue;

			/* Count the lines */
			next++;
		}


		/* Dump the next lines of the file */
		for (i = 0; i < hgt - 4; )
		{
			/* Hack -- track the "first" line */
			if (!i) line = next;

			/* Get a line of the file or stop */
			if (my_fgets(fff, buf, sizeof(buf))) break;

			/* Hack -- skip "special" lines */
			if (prefix(buf, "***** ")) continue;

			/* Count the "real" lines */
			next++;

			/* Make a copy of the current line for searching */
			strcpy(lc_buf, buf);

			/* Make the line lower case */
			if (!case_sensitive) string_lower(lc_buf);

			/* Hack -- keep searching */
			if (find && !i && !strstr(lc_buf, find)) continue;

			/* Hack -- stop searching */
			find = NULL;

			/* Dump the line */
			Term_putstr(0, i+2, -1, TERM_WHITE, buf);

			/* Hilite "shower" */
			if (shower[0])
			{
				cptr str = lc_buf;

				/* Display matches */
				while ((str = strstr(str, shower)) != NULL)
				{
					int len = strlen(shower);

					/* Display the match */
					Term_putstr(str-lc_buf, i+2, len, TERM_YELLOW, &buf[str-lc_buf]);

					/* Advance */
					str += len;
				}
			}

			/* Count the printed lines */
			i++;
		}

		/* Hack -- failed search */
		if (find)
		{
			bell("Search string not found!");
			line = back;
			find = NULL;
			continue;
		}


		/* Show a general "title" */
		prt(format("[%s %s, %s, Line %d/%d]", VERSION_NAME,
		           VERSION_STRING, caption, line, size), 0, 0);


		/* Prompt -- menu screen */
		if (menu)
		{
			/* Wait for it */
			prt("[Press a Number, or ESC to exit.]", hgt - 1, 0);
		}

		/* Prompt -- small files */
		else if (size <= hgt - 4)
		{
			/* Wait for it */
			prt("[Press ESC to exit.]", hgt - 1, 0);
		}

		/* Prompt -- large files */
		else
		{
			/* Wait for it */
			prt("[Press Space to advance, or ESC to exit.]", hgt - 1, 0);
		}

		/* Get a keypress */
		ke = anykey();

		/* Return to last screen */
		if (ke.key == '?') break;

		/* Toggle case sensitive on/off */
		if (ke.key == '!')
		{
			case_sensitive = !case_sensitive;
		}

		/* Try showing */
		if (ke.key == '&')
		{
			/* Get "shower" */
			prt("Show: ", hgt - 1, 0);
			(void)askfor_aux(shower, 80);

			/* Make the "shower" lowercase */
			if (!case_sensitive) string_lower(shower);
		}

		/* Try finding */
		if (ke.key == '/')
		{
			/* Get "finder" */
			prt("Find: ", hgt - 1, 0);
			if (askfor_aux(finder, 80))
			{
				/* Find it */
				find = finder;
				back = line;
				line = line + 1;

				/* Make the "finder" lowercase */
				if (!case_sensitive) string_lower(finder);

				/* Show it */
				strcpy(shower, finder);
			}
		}

		/* Go to a specific line */
		if (ke.key == '#')
		{
			char tmp[80];
			prt("Goto Line: ", hgt - 1, 0);
			strcpy(tmp, "0");
			if (askfor_aux(tmp, 80))
			{
				line = atoi(tmp);
			}
		}

		/* Go to a specific file */
		if (ke.key == '%')
		{
			char ftmp[80];
			prt("Goto File: ", hgt - 1, 0);
			strcpy(ftmp, "help.hlp");
			if (askfor_aux(ftmp, 80))
			{
				if (!show_file(ftmp, NULL, 0, mode)) ke.key = ESCAPE;
			}
		}

		/* Back up one line */
		if (ke.key == '=')
		{
			line = line - 1;
			if (line < 0) line = 0;
		}

		/* Back up one half page */
		if (ke.key == '_')
		{
			line = line - ((hgt - 4) / 2);
			if (line < 0) line = 0;
		}

		/* Back up one full page */
		if (ke.key == '-')
		{
			line = line - (hgt - 4);
			if (line < 0) line = 0;
		}

		/* Advance one line */
		if ((ke.key == '\n') || (ke.key == '\r'))
		{
			line = line + 1;
		}

		/* Advance one half page */
		if (ke.key == '+')
		{
			line = line + ((hgt - 4) / 2);
			if (line < 0) line = 0;
		}

		/* Advance one full page */
		if (ke.key == ' ')
		{
			line = line + (hgt - 4);
		}

		/* Scroll forwards or backwards using mouse clicks */
		if (ke.key == '\xff')
		{
			if (ke.mousebutton)
			{
				if (ke.mousey <= hgt / 2)
				{
					/* Back up one full page */
					line = line - (hgt - 4);
					if (line < 0) line = 0;
				}
				else
				{
					/* Advance one full page */
					line = line + (hgt - 4);
				}
			}
		}

		/* Recurse on numbers */
		if (menu && isdigit(ke.key) && hook[D2I(ke.key)][0])
		{
			/* Recurse on that file */
			if (!show_file(hook[D2I(ke.key)], NULL, 0, mode)) ke.key = ESCAPE;
		}

		/* Exit on escape */
		if (ke.key == ESCAPE) break;
	}

	/* Close the file */
	my_fclose(fff);

	/* Done */
	return (ke.key != ESCAPE);
}


/*
 * Peruse the On-Line-Help
 */
void do_cmd_help(void)
{
	/* Save screen */
	screen_save();

	/* Peruse the main help file */
	(void)show_file("help.hlp", NULL, 0, 0);

	/* Load screen */
	screen_load();
}



/*
 * Process the player name and extract a clean "base name".
 *
 * If "sf" is TRUE, then we initialize "savefile" based on player name.
 *
 * Some platforms (Windows, Macintosh, Amiga) leave the "savefile" empty
 * when a new character is created, and then when the character is done
 * being created, they call this function to choose a new savefile name.
 */
void process_player_name(bool sf)
{
	int i;


	/* Cannot be too long */
	if (strlen(op_ptr->full_name) > 15)
	{
		/* Name too long */
		quit_fmt("The name '%s' is too long!", op_ptr->full_name);
	}

	/* Process the player name */
	for (i = 0; op_ptr->full_name[i]; i++)
	{
		char c = op_ptr->full_name[i];

		/* No control characters */
		if (iscntrl(c))
		{
			/* Illegal characters */
			quit_fmt("Illegal control char (0x%02X) in player name", c);
		}

		/* Convert all non-alphanumeric symbols */
		if (!isalpha(c) && !isdigit(c)) c = '_';

		/* Build "base_name" */
		op_ptr->base_name[i] = c;
	}

#if defined(WINDOWS) || defined(MSDOS)

	/* Max length */
	if (i > 8) i = 8;

#endif

	/* Terminate */
	op_ptr->base_name[i] = '\0';

	/* Require a "base" name */
	if (!op_ptr->base_name[0])
	{
		strcpy(op_ptr->base_name, "PLAYER");
	}


	/* Pick savefile name if needed */
	if (sf)
	{
		char temp[128];

#ifdef SAVEFILE_USE_UID
		/* Rename the savefile, using the player_uid and base_name */
		sprintf(temp, "%d.%s", player_uid, op_ptr->base_name);
#else
		/* Rename the savefile, using the base name */
		sprintf(temp, "%s", op_ptr->base_name);
#endif

#ifdef VM
		/* Hack -- support "flat directory" usage on VM/ESA */
		sprintf(temp, "%s.sv", op_ptr->base_name);
#endif /* VM */

		/* Build the filename */
		path_build(savefile, 1024, ANGBAND_DIR_SAVE, temp);
	}
}



/*
 * Use W. Sheldon Simms' random name generator.  Generate a random word using
 * the probability tables we built earlier.  Relies on the ASCII character
 * set.  Relies on European vowels (a, e, i, o, u).  The generated name should
 * be copied/used before calling this function again.
 */
char *make_word(int min_len, int max_len)
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

	/* Paranoia */
	if (min_len > max_len) return "(Bad name)";

	/* Paranoia */
	if (max_len > 90) return "(Bad name)";

	while (1)
	{
	    getletter:
		c_next = 0;
		r = rand_int(n_info->ltotal[c_prev][c_cur]);
		totalfreq = n_info->lprobs[c_prev][c_cur][c_next];

		/*find the letter*/
		while (totalfreq <= r)
		{
			c_next++;
			totalfreq += n_info->lprobs[c_prev][c_cur][c_next];
		}

		if (c_next == E_WORD)
		{
			if ((lnum < min_len) || vow == 0)
			{
				tries++;
				if (tries < 10) goto getletter;
				goto startover;
			}
			*cp = '\0';
			break;
		}

		if (lnum >= max_len) goto startover;

		*cp = I2A(c_next);

		if (is_a_vowel(*cp)) vow++;

		cp++;
		lnum++;
		c_prev = c_cur;
		c_cur = c_next;
	}

	word_buf[0] = toupper((unsigned char)word_buf[0]);

	return (word_buf);
}


/*
 * Gets a name for the character, reacting to name changes.
 *
 * Perhaps we should NOT ask for a name (at "birth()") on
 * Unix machines?  XXX XXX XXX
 *
 * What a horrible name for a global function.  XXX XXX XXX
 */
void get_name(void)
{
	char tmp[16];

	/* Save the player name */
	strcpy(tmp, op_ptr->full_name);

	/* Offer a random name */
	if (!strlen(tmp))
	{
		strcpy(tmp, make_word(7, 15));
	}

	/* Prompt for a new name */
	if (get_string("Enter a name for your character: ", tmp, sizeof(tmp)))
	{
		/* Use the name */
		strcpy(op_ptr->full_name, tmp);

		/* Process the player name */
		process_player_name(FALSE);
	}
}



/*
 * Hack -- commit suicide
 */
void do_cmd_suicide(void)
{
	/* Flush input */
	flush();

	/* Verify Retirement */
	if (p_ptr->total_winner)
	{
		/* Verify */
		if (!get_check("Do you want to retire? ")) return;
	}

	/* Verify Suicide */
	else
	{
		key_event ke;

		/* Verify */
		if (!get_check("Do you really want to quit? ")) return;

		/* Special Verification for suicide */
		prt("Please verify QUITTING by typing the '@' sign: ", 0, 0);
		flush();
		ke = anykey();
		prt("", 0, 0);
		if (ke.key != '@') return;
	}

	/* Commit suicide */
	p_ptr->is_dead = TRUE;

	/* Stop playing */
	p_ptr->playing = FALSE;

	/* Leaving */
	p_ptr->leaving = TRUE;

	/* Cause of death */
	strcpy(p_ptr->died_from, "Quitting");
}



/*
 * Save the game
 */
void do_cmd_save_game(void)
{
	/* Disturb the player */
	disturb(1, 0);

	/* Clear messages */
	message_flush();

	/* Handle stuff */
	handle_stuff();

	/* Message */
	prt("Saving game...", 0, 0);

	/* Refresh */
	Term_fresh();

	/* The player is not dead */
	strcpy(p_ptr->died_from, "(saved)");

	/* Forbid suspend */
	signals_ignore_tstp();

	/* Save the player */
	if (save_player())
	{
		prt("Saving game... done.", 0, 0);
	}

	/* Save failed (oops) */
	else
	{
		prt("Saving game... failed!", 0, 0);
	}

	/* Allow suspend again */
	signals_handle_tstp();

	/* Refresh */
	Term_fresh();

	/* Note that the player is not dead */
	strcpy(p_ptr->died_from, "(alive and well)");
}



/*
 * Hack -- Calculates the total number of points earned
 */
long total_points(void)
{
	return (p_ptr->max_exp + (100 * p_ptr->max_depth));
}



/*
 * Centers a string within a 31 character string
 */
static void center_string(char *buf, cptr str)
{
	int i, j;

	/* Total length */
	i = strlen(str);

	/* Necessary border */
	j = 15 - i / 2;

	/* Mega-Hack */
	sprintf(buf, "%*s%s%*s", j, "", str, 31 - i - j, "");
}



#if 0

/*
 * Save a "bones" file for a dead character
 *
 * Note that we will not use these files until a later version, and
 * then we will only use the name and level on which death occured.
 *
 * Should probably attempt some form of locking...
 */
static void make_bones(void)
{
	FILE *fp;

	char str[1024];


	/* Ignore wizards and borgs */
	if (!(p_ptr->noscore & 0x00FF))
	{
		/* Ignore people who die in town */
		if (p_ptr->depth)
		{
			char tmp[128];

			/* XXX XXX XXX "Bones" name */
			sprintf(tmp, "bone.%03d", p_ptr->depth);

			/* Build the filename */
			path_build(str, 1024, ANGBAND_DIR_BONE, tmp);

			/* Attempt to open the bones file */
			fp = my_fopen(str, "r");

			/* Close it right away */
			if (fp) my_fclose(fp);

			/* Do not over-write a previous ghost */
			if (fp) return;

			/* File type is "TEXT" */
			FILE_TYPE(FILE_TYPE_TEXT);

			/* Grab permissions */
			safe_setuid_grab();

			/* Try to write a new "Bones File" */
			fp = my_fopen(str, "w");

			/* Drop permissions */
			safe_setuid_drop();

			/* Not allowed to write it?  Weird. */
			if (!fp) return;

			/* Save the info */
			fprintf(fp, "%s\n", op_ptr->full_name);
			fprintf(fp, "%d\n", p_ptr->mhp);
			fprintf(fp, "%d\n", p_ptr->prace);
			fprintf(fp, "%d\n", p_ptr->pclass);

			/* Close and save the Bones file */
			my_fclose(fp);
		}
	}
}

#endif /* 0 */


/*
 * Hack - save the time of death
 */
static time_t death_time = (time_t)0;


/*
 * Display a "tomb-stone"
 */
static void print_tomb(void)
{
	cptr p;

	char tmp[160];

	char buf[1024];

	FILE *fp;


	/* Clear screen */
	Term_clear();

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_FILE, "dead.txt");

	/* Open the News file */
	fp = my_fopen(buf, "r");

	/* Dump */
	if (fp)
	{
		int i = 0;

		/* Dump the file to the screen */
		while (0 == my_fgets(fp, buf, sizeof(buf)))
		{
			/* Display and advance */
			put_str(buf, i++, 0);
		}

		/* Close */
		my_fclose(fp);
	}


	/* King or Queen */
	if (p_ptr->total_winner || (p_ptr->lev > PY_MAX_LEVEL))
	{
		p = "Magnificent";
	}

	/* Normal */
	else
	{
		p = c_text + cp_ptr->title[(p_ptr->lev - 1) / 5];
	}

	center_string(buf, op_ptr->full_name);
	put_str(buf, 6, 11);

	center_string(buf, "the");
	put_str(buf, 7, 11);

	center_string(buf, p);
	put_str(buf, 8, 11);


	center_string(buf, c_name + cp_ptr->name);
	put_str(buf, 10, 11);

	sprintf(tmp, "Level: %d", (int)p_ptr->lev);
	center_string(buf, tmp);
	put_str(buf, 11, 11);

	sprintf(tmp, "Exp: %ld", (long)p_ptr->exp);
	center_string(buf, tmp);
	put_str(buf, 12, 11);

	sprintf(tmp, "AU: %ld", (long)p_ptr->au);
	center_string(buf, tmp);
	put_str(buf, 13, 11);

	sprintf(tmp, "Killed on Level %d", p_ptr->depth);
	center_string(buf, tmp);
	put_str(buf, 14, 11);

	sprintf(tmp, "by %s.", p_ptr->died_from);
	center_string(buf, tmp);
	put_str(buf, 15, 11);


	sprintf(tmp, "%-.24s", ctime(&death_time));
	center_string(buf, tmp);
	put_str(buf, 17, 11);
}


/*
 * Hack - Know inventory and home items upon death
 */
static void death_knowledge(void)
{
	int i;

	object_type *o_ptr;

	store_type *st_ptr = &store[STORE_HOME];

	/* Hack -- Know everything in the inven/equip */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Aware and Known */
		object_aware(o_ptr);
		object_known(o_ptr);

		/* Fully known */
		o_ptr->ident |= (IDENT_MENTAL);
	}

	/* Hack -- Know everything in the home */
	for (i = 0; i < st_ptr->stock_num; i++)
	{
		o_ptr = &st_ptr->stock[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Aware and Known */
		object_aware(o_ptr);
		object_known(o_ptr);

		/* Fully known */
		o_ptr->ident |= (IDENT_MENTAL);
	}

	/* Hack -- Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff();
}


/*
 * Display some character info
 */
static void show_info(void)
{
	int i, j, k;

	object_type *o_ptr;

	store_type *st_ptr = &store[STORE_HOME];


	/* Display player */
	display_player(0);

	/* Prompt for inventory */
	prt("Hit any key to see more information (ESC to abort): ", 23, 0);

	/* Allow abort at this point */
	if (anykey().key == ESCAPE) return;


	/* Show equipment and inventory */

	/* Equipment -- if any */
	if (p_ptr->equip_cnt)
	{
		Term_clear();
		item_tester_full = TRUE;
		show_equip();
		prt("You are using: -more-", 0, 0);
		if (anykey().key == ESCAPE) return;
	}

	/* Inventory -- if any */
	if (p_ptr->inven_cnt)
	{
		Term_clear();
		item_tester_full = TRUE;
		show_inven();
		prt("You are carrying: -more-", 0, 0);
		if (anykey().key == ESCAPE) return;
	}



	/* Home -- if anything there */
	if (st_ptr->stock_num)
	{
		/* Display contents of the home */
		for (k = 0, i = 0; i < st_ptr->stock_num; k++)
		{
			/* Clear screen */
			Term_clear();

			/* Show 12 items */
			for (j = 0; (j < 12) && (i < st_ptr->stock_num); j++, i++)
			{
				byte attr;

				char o_name[80];
				char tmp_val[80];

				/* Get the object */
				o_ptr = &st_ptr->stock[i];

				/* Print header, clear line */
				sprintf(tmp_val, "%c) ", I2A(j));
				prt(tmp_val, j+2, 4);

				/* Get the object description */
				object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

				/* Get the inventory color */
				attr = tval_to_attr[o_ptr->tval & 0x7F];

				/* Display the object */
				c_put_str(attr, o_name, j+2, 7);
			}

			/* Caption */
			prt(format("Your home contains (page %d): -more-", k+1), 0, 0);

			/* Wait for it */
			if (anykey().key == ESCAPE) return;
		}
	}
}


/*
 * Special version of 'do_cmd_examine'
 */
static void death_examine(void)
{
	int item;

	object_type *o_ptr;

	char o_name[80];

	cptr q, s;


	/* Start out in "display" mode */
	p_ptr->command_see = TRUE;

	/* Get an item */
	q = "Examine which item? ";
	s = "You have nothing to examine.";

	while (TRUE)
	{
		if (!get_item(&item, q, s, (USE_INVEN | USE_EQUIP))) return;

		/* Get the item */
		o_ptr = &inventory[item];

		/* In a bag? */
		if (o_ptr->tval == TV_BAG)
		{
			/* Get item from bag -- hack: cancel to pick the bag itself */
			if (get_item_from_bag(&item, q, s, o_ptr)) o_ptr = &inventory[item];
		}

                /* Known */
                object_known(o_ptr);

		/* Description */
		object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

		/* Describe */
		msg_format("Examining %s...", o_name);

		msg_print("");

		/* Save the screen */
		screen_save();

		/* Describe */
		screen_object(o_ptr);

		(void)anykey();

		/* Load the screen */
		screen_load();
	}
}


/*
 * Seek score 'i' in the highscore file
 */
static int highscore_seek(int i)
{
	/* Seek for the requested record */
	return (fd_seek(highscore_fd, i * sizeof(high_score) + 8));
}


/*
 * Read one score from the highscore file
 */
static errr highscore_read(high_score *score)
{
	/* Read the record, note failure */
	return (fd_read(highscore_fd, (char*)(score), sizeof(high_score)));
}


/*
 * Write one score to the highscore file
 */
static int highscore_write(const high_score *score)
{
	/* Write the record, note failure */
	return (fd_write(highscore_fd, (cptr)(score), sizeof(high_score)));
}




/*
 * Just determine where a new score *would* be placed
 * Return the location (0 is best) or -1 on failure
 */
static int highscore_where(const high_score *score)
{
	int i;

	high_score the_score;

	/* Paranoia -- it may not have opened */
	if (highscore_fd < 0) return (-1);

	/* Go to the start of the highscore file */
	if (highscore_seek(0)) return (-1);

	/* Read until we get to a higher score */
	for (i = 0; i < MAX_HISCORES; i++)
	{
		if (highscore_read(&the_score)) return (i);
		if (strcmp(the_score.pts, score->pts) < 0) return (i);
	}

	/* The "last" entry is always usable */
	return (MAX_HISCORES - 1);
}


/*
 * Actually place an entry into the high score file
 * Return the location (0 is best) or -1 on "failure"
 */
static int highscore_add(const high_score *score)
{
	int i, slot;
	bool done = FALSE;

	high_score the_score, tmpscore;


	/* Paranoia -- it may not have opened */
	if (highscore_fd < 0) return (-1);

	/* Determine where the score should go */
	slot = highscore_where(score);

	/* Hack -- Not on the list */
	if (slot < 0) return (-1);

	/* Hack -- prepare to dump the new score */
	the_score = (*score);

	/* Slide all the scores down one */
	for (i = slot; !done && (i < MAX_HISCORES); i++)
	{
		/* Read the old guy, note errors */
		if (highscore_seek(i)) return (-1);
		if (highscore_read(&tmpscore)) done = TRUE;

		/* Back up and dump the score we were holding */
		if (highscore_seek(i)) return (-1);
		if (highscore_write(&the_score)) return (-1);

		/* Hack -- Save the old score, for the next pass */
		the_score = tmpscore;
	}

	/* Return location used */
	return (slot);
}



/*
 * Display the scores in a given range.
 * Assumes the high score list is already open.
 * Only five entries per line, too much info.
 *
 * Mega-Hack -- allow "fake" entry at the given position.
 */
void display_scores_aux(int from, int to, int note, high_score *score)
{
	key_event ke;

	int j, k, n, place;
	int count;

	high_score the_score;

	char out_val[160];
	char tmp_val[160];

	byte attr;


	/* Paranoia -- it may not have opened */
	if (highscore_fd < 0) return;


	/* Assume we will show the first 10 */
	if (from < 0) from = 0;
	if (to < 0) to = 10;
	if (to > MAX_HISCORES) to = MAX_HISCORES;


	/* Seek to the beginning */
	if (highscore_seek(0)) return;

	/* Hack -- Count the high scores */
	for (count = 0; count < MAX_HISCORES; count++)
	{
		if (highscore_read(&the_score)) break;
	}

	/* Hack -- allow "fake" entry to be last */
	if ((note == count) && score) count++;

	/* Forget about the last entries */
	if (count > to) count = to;


	/* Show 5 per page, until "done" */
	for (k = from, j = from, place = k+1; k < count; k += 5)
	{
		/* Clear screen */
		Term_clear();

		/* Title */
		put_str(format("                %s Hall of Fame", VERSION_NAME),
		        0, 0);

		/* Indicate non-top scores */
		if (k > 0)
		{
			sprintf(tmp_val, "(from position %d)", place);
			put_str(tmp_val, 0, 40);
		}

		/* Dump 5 entries */
		for (n = 0; j < count && n < 5; place++, j++, n++)
		{
			int pr, pc, ps, pp, clev, mlev, cdep, mdep, cdun;

			cptr user, gold, when, aged;

			char name[60];

			/* Hack -- indicate death in yellow */
			attr = (j == note) ? TERM_YELLOW : TERM_WHITE;


			/* Mega-Hack -- insert a "fake" record */
			if ((note == j) && score)
			{
				the_score = (*score);
				attr = TERM_L_GREEN;
				score = NULL;
				note = -1;
				j--;
			}

			/* Read a normal record */
			else
			{
				/* Read the proper record */
				if (highscore_seek(j)) break;
				if (highscore_read(&the_score)) break;
			}

			/* Extract the race/class */
			pr = atoi(the_score.p_r);
			pc = atoi(the_score.p_c);
			ps = atoi(the_score.p_s);
			pp = atoi(the_score.p_p);

			/* Extract the level info */
			clev = atoi(the_score.cur_lev);
			mlev = atoi(the_score.max_lev);
			cdep = atoi(the_score.cur_dep);
			mdep = atoi(the_score.max_dep);
			cdun = atoi(the_score.cur_dun);

			/* Hack -- extract the gold and such */
			for (user = the_score.uid; isspace(*user); user++) /* loop */;
			for (when = the_score.day; isspace(*when); when++) /* loop */;
			for (gold = the_score.gold; isspace(*gold); gold++) /* loop */;
			for (aged = the_score.turns; isspace(*aged); aged++) /* loop */;

			/* Clean up standard encoded form of "when" */
			if ((*when == '@') && strlen(when) == 9)
			{
				sprintf(tmp_val, "%.4s-%.2s-%.2s",
				        when + 1, when + 5, when + 7);
				when = tmp_val;
			}

#ifdef USE_CLASS_PRETTY_NAMES
			/* Get the class name */
			lookup_prettyname(name, pc, ps, pp, FALSE, TRUE);
#else
			/* Get the class name */
			strcpy(name, c_name + c_info[pc].name);
#endif
			/* Dump some info */
			sprintf(out_val, "%3d.%9s  %s the %s %s, Level %d",
			        place, the_score.pts, the_score.who,
			        p_name + p_info[pr].name, name,
			        clev);

			/* Append a "maximum level" */
			if (mlev > clev) strcat(out_val, format(" (Max %d)", mlev));

			/* Dump the first line */
			c_put_str(attr, out_val, n*4 + 2, 0);

			/* Another line of info */
			sprintf(out_val, "               Killed by %s %s %s",
			        the_score.how, ( (cdep == min_depth(cdun)) && (cdep == max_depth(cdun)) ? "in" : 
				( cdep == min_depth(cdun) ? "on the surface of":
				format("on Dungeon Level %d in", cdep) ) ), t_name + t_info[cdun].name);

			/* Append a "maximum level" */
			if (mdep > cdep) strcat(out_val, format(" (Max %d)", mdep));

			/* Dump the info */
			c_put_str(attr, out_val, n*4 + 3, 0);

			/* And still another line of info */
			sprintf(out_val,
			        "               (User %s, Date %s, Gold %s, Turn %s).",
			        user, when, gold, aged);
			c_put_str(attr, out_val, n*4 + 4, 0);
		}


		/* Wait for response */
		prt("[Press ESC to quit, any other key to continue.]", 23, 17);
		ke = anykey();
		prt("", 23, 0);

		/* Hack -- notice Escape */
		if (ke.key == ESCAPE) break;
	}
}


/*
 * Hack -- Display the scores in a given range and quit.
 *
 * This function is only called from "main.c" when the user asks
 * to see the "high scores".
 */
void display_scores(int from, int to)
{
	char buf[1024];

	char version[8];

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_APEX, "scores.raw");

	/* Open the binary high score file, for reading */
	highscore_fd = fd_open(buf, O_RDONLY);

	/* Clear version string */
	version[0] = '\0';

	/* Check highscore version */
	if (!(highscore_fd < 0) && (!fd_read(highscore_fd, (char*)(version), 8)))
	{
		if (!fd_seek(highscore_fd, 0)) return;

		sprintf(version, "%s", VERSION_STRING);

		if (!fd_write(highscore_fd, version, 8)) return;
	}

	/* Compare string */
	if (strcmp(version, "0.6.2"))
	{
		msg_print("Incompatible high score table.");
		msg_print("Please rename lib\\apex\\scores.raw.");
		return;
	}

	/* Clear screen */
	Term_clear();

	/* Title */
	put_str(format("                %s Hall of Fame", VERSION_NAME), 0, 0);

	/* Display the scores */
	display_scores_aux(from, to, -1, NULL);

	/* Shut the high score file */
	fd_close(highscore_fd);

	/* Forget the high score fd */
	highscore_fd = -1;

	/* Wait for response */
	prt("[Press any key to quit.]", 23, 17);
	(void)anykey();
	prt("", 23, 0);

	/* Quit */
	quit(NULL);
}


/*
 * Hack - save index of player's high score
 */
static int score_idx = -1;


/*
 * Enters a players name on a hi-score table, if "legal".
 *
 * Assumes "signals_ignore_tstp()" has been called.
 */
static errr enter_score(void)
{
#ifndef SCORE_CHEATERS
	int j;
#endif /* SCORE_CHEATERS */

	high_score the_score;


	/* No score file */
	if (highscore_fd < 0)
	{
		return (0);
	}

#ifndef SCORE_WIZARDS

	/* Wizard-mode pre-empts scoring */
	if (p_ptr->noscore & 0x000F)
	{
		msg_print("Score not registered for wizards.");
		message_flush();
		score_idx = -1;
		return (0);
	}

#endif

#ifndef SCORE_BORGS

	/* Borg-mode pre-empts scoring */
	if (p_ptr->noscore & 0x00F0)
	{
		msg_print("Score not registered for borgs.");
		message_flush();
		score_idx = -1;
		return (0);
	}
#endif /* SCORE_BORGS */

#ifndef SCORE_CHEATERS

	/* Cheaters are not scored */
	for (j = OPT_SCORE; j < OPT_MAX; ++j)
	{
		if (!op_ptr->opt[j]) continue;

		msg_print("Score not registered for cheaters.");
		message_flush();
		score_idx = -1;
		return (0);
	}

#endif /* SCORE_CHEATERS */

	/* Hack -- Interupted */
	if (!p_ptr->total_winner && streq(p_ptr->died_from, "Interrupting"))
	{
		msg_print("Score not registered due to interruption.");
		message_flush();
		score_idx = -1;
		return (0);
	}

	/* Hack -- Quitter */
	if (!p_ptr->total_winner && streq(p_ptr->died_from, "Quitting"))
	{
		msg_print("Score not registered due to quitting.");
		message_flush();
		score_idx = -1;
		return (0);
	}


	/* Clear the record */
	(void)WIPE(&the_score, high_score);

	/* Save the version */
	sprintf(the_score.what, "%s", VERSION_STRING);

	/* Calculate and save the points */
	sprintf(the_score.pts, "%9lu", (long)total_points());
	the_score.pts[9] = '\0';

	/* Save the current gold */
	sprintf(the_score.gold, "%9lu", (long)p_ptr->au);
	the_score.gold[9] = '\0';

	/* Save the current turn */
	sprintf(the_score.turns, "%9lu", (long)turn);
	the_score.turns[9] = '\0';

	/* Save the date in standard encoded form (9 chars) */
	strftime(the_score.day, 10, "@%Y%m%d", localtime(&death_time));

	/* Save the player name (15 chars) */
	sprintf(the_score.who, "%-.15s", op_ptr->full_name);

	/* Save the player info XXX XXX XXX */
	sprintf(the_score.uid, "%7u", player_uid);
	sprintf(the_score.sex, "%c", (p_ptr->psex ? 'm' : 'f'));
	sprintf(the_score.p_r, "%2d", p_ptr->prace);
	sprintf(the_score.p_c, "%2d", p_ptr->pclass);
	sprintf(the_score.p_s, "%2d", p_ptr->pstyle);
	sprintf(the_score.p_p, "%2d", p_ptr->psval);

	/* Save the level and such */
	sprintf(the_score.cur_lev, "%3d", p_ptr->lev);
	sprintf(the_score.cur_dep, "%3d", p_ptr->depth);
	sprintf(the_score.max_lev, "%3d", p_ptr->max_lev);
	sprintf(the_score.max_dep, "%3d", p_ptr->max_depth);
	sprintf(the_score.cur_dun, "%3d", p_ptr->dungeon);

	/* Save the cause of death (31 chars) */
	sprintf(the_score.how, "%-.31s", p_ptr->died_from);

	/* Grab permissions */
	safe_setuid_grab();

	/* Lock (for writing) the highscore file, or fail */
	if (fd_lock(highscore_fd, F_WRLCK)) return (1);

	/* Drop permissions */
	safe_setuid_drop();

	/* Add a new entry to the score list, see where it went */
	score_idx = highscore_add(&the_score);

	/* Grab permissions */
	safe_setuid_grab();

	/* Unlock the highscore file, or fail */
	if (fd_lock(highscore_fd, F_UNLCK)) return (1);

	/* Drop permissions */
	safe_setuid_drop();

	/* Success */
	return (0);
}



/*
 * Enters a players name on a hi-score table, if "legal", and in any
 * case, displays some relevant portion of the high score list.
 *
 * Assumes "signals_ignore_tstp()" has been called.
 */
static void top_twenty(void)
{
	/* Clear screen */
	Term_clear();

	/* No score file */
	if (highscore_fd < 0)
	{
		msg_print("Score file unavailable.");
		message_flush();
		return;
	}

	/* Player's score unavailable */
	if (score_idx == -1)
	{
		display_scores_aux(0, 10, -1, NULL);
		return;
	}

	/* Hack -- Display the top fifteen scores */
	else if (score_idx < 10)
	{
		display_scores_aux(0, 15, score_idx, NULL);
	}

	/* Display the scores surrounding the player */
	else
	{
		display_scores_aux(0, 5, score_idx, NULL);
		display_scores_aux(score_idx - 2, score_idx + 7, score_idx, NULL);
	}


	/* Success */
	return;
}


/*
 * Predict the players location, and display it.
 */
errr predict_score(void)
{
	int j;

	high_score the_score;


	/* No score file */
	if (highscore_fd < 0)
	{
		msg_print("Score file unavailable.");
		message_flush();
		return (0);
	}


	/* Save the version */
	sprintf(the_score.what, "%s", VERSION_STRING);

	/* Calculate and save the points */
	sprintf(the_score.pts, "%9lu", (long)total_points());

	/* Save the current gold */
	sprintf(the_score.gold, "%9lu", (long)p_ptr->au);

	/* Save the current turn */
	sprintf(the_score.turns, "%9lu", (long)turn);

	/* Hack -- no time needed */
	strcpy(the_score.day, "TODAY");

	/* Save the player name (15 chars) */
	sprintf(the_score.who, "%-.15s", op_ptr->full_name);

	/* Save the player info XXX XXX XXX */
	sprintf(the_score.uid, "%7u", player_uid);
	sprintf(the_score.sex, "%c", (p_ptr->psex ? 'm' : 'f'));
	sprintf(the_score.p_r, "%2d", p_ptr->prace);
	sprintf(the_score.p_c, "%2d", p_ptr->pclass);
	sprintf(the_score.p_s, "%2d", p_ptr->pstyle);
	sprintf(the_score.p_p, "%2d", p_ptr->psval);


	/* Save the level and such */
	sprintf(the_score.cur_lev, "%3d", p_ptr->lev);
	sprintf(the_score.cur_dep, "%3d", p_ptr->depth);
	sprintf(the_score.max_lev, "%3d", p_ptr->max_lev);
	sprintf(the_score.max_dep, "%3d", p_ptr->max_depth);
	sprintf(the_score.cur_dun, "%3d", p_ptr->dungeon);

	/* Hack -- no cause of death */
	strcpy(the_score.how, "nobody (yet!)");


	/* See where the entry would be placed */
	j = highscore_where(&the_score);


	/* Hack -- Display the top fifteen scores */
	if (j < 10)
	{
		display_scores_aux(0, 15, j, &the_score);
	}

	/* Display some "useful" scores */
	else
	{
		display_scores_aux(0, 5, -1, NULL);
		display_scores_aux(j - 2, j + 7, j, &the_score);
	}


	/* Success */
	return (0);
}








/*
 * Change the player into a Winner
 */
static void kingly(void)
{
	/* Hack -- retire in town */
	p_ptr->depth = 0;

	/* Fake death */
	strcpy(p_ptr->died_from, "Ripe Old Age");

	/* Restore the experience */
	p_ptr->exp = p_ptr->max_exp;

	/* Restore the level */
	p_ptr->lev = p_ptr->max_lev;

	/* Hack -- Instant Gold */
	p_ptr->au += 10000000L;

	/* Clear screen */
	Term_clear();

	/* Display a crown */
	put_str("#", 1, 34);
	put_str("#####", 2, 32);
	put_str("#", 3, 34);
	put_str(",,,  $$$  ,,,", 4, 28);
	put_str(",,=$   \"$$$$$\"   $=,,", 5, 24);
	put_str(",$$        $$$        $$,", 6, 22);
	put_str("*>         <*>         <*", 7, 22);
	put_str("$$         $$$         $$", 8, 22);
	put_str("\"$$        $$$        $$\"", 9, 22);
	put_str("\"$$       $$$       $$\"", 10, 23);
	put_str("*#########*#########*", 11, 24);
	put_str("*#########*#########*", 12, 24);

	/* Display a message */
	put_str("Veni, Vidi, Vici!", 15, 26);
	put_str("I came, I saw, I conquered!", 16, 21);
	put_str(format("All Hail the Mighty %s!", sp_ptr->winner), 17, 22);

	/* Flush input */
	flush();

	/* Wait for response */
	pause_line(23);
}


/*
 * Handle character death
 */
static void close_game_aux(void)
{
	key_event ke;

	bool wants_to_quit = FALSE;
	cptr p = "[(i)nformation, (m)essages, (f)ile dump, (v)iew scores, e(x)amine item, ESC]";


	/* Handle retirement */
	if (p_ptr->total_winner) kingly();

	/* Save dead player */
	if (!save_player())
	{
		msg_print("death save failed!");
		message_flush();
	}

	/* Get time of death */
	(void)time(&death_time);

	/* You are dead */
	print_tomb();

	/* Hack - Know everything upon death */
	death_knowledge();

	/* Enter player in high score list */
	enter_score();

	/* Flush all input keys */
	flush();

	/* Flush messages */
	message_flush();

	/* Loop */
	while (!wants_to_quit)
	{
		/* Describe options */
		Term_putstr(1, 23, -1, TERM_WHITE, p);

		/* Query */
		ke = anykey();

		switch (ke.key)
		{
			/* Exit */
			case ESCAPE:
			{
				if (get_check("Do you want to quit? "))
					wants_to_quit = TRUE;

				break;
			}

			/* File dump */
			case 'f':
			case 'F':
			{
				char ftmp[80];

				sprintf(ftmp, "%s.txt", op_ptr->base_name);

				if (get_string("File name: ", ftmp, 80))
				{
					if (ftmp[0] && (ftmp[0] != ' '))
					{
						errr err;

						/* Save screen */
						screen_save();

						/* Dump a character file */
						err = file_character(ftmp, FALSE);

						/* Load screen */
						screen_load();

						/* Check result */
						if (err)
						{
							msg_print("Character dump failed!");
						}
						else
						{
							msg_print("Character dump successful.");
						}

						/* Flush messages */
						message_flush();
					}
				}

				break;
			}

			/* Show more info */
			case 'i':
			case 'I':
			{
				/* Save screen */
				screen_save();

				/* Show the character */
				show_info();

				/* Load screen */
				screen_load();

				break;
			}

			/* Show last messages */
			case 'm':
			case 'M':
			{
				/* Save screen */
				screen_save();

				/* Display messages */
				do_cmd_messages();

				/* Load screen */
				screen_load();

				break;
			}

			/* Show top scores */
			case 'v':
			case 'V':
			{
				/* Save screen */
				screen_save();

				/* Show the scores */
				top_twenty();

				/* Load screen */
				screen_load();

				break;
			}

			/* Examine an item */
			case 'x':
			case 'X':
			{
				/* Save screen */
				screen_save();

				/* Clear the screen */
				Term_clear();

				/* Examine items */
				death_examine();

				/* Load screen */
				screen_load();

				break;
			}
		}
	}

#if 0
	/* Dump bones file */
	make_bones();
#endif
}


/*
 * Close up the current game (player may or may not be dead)
 *
 * This function is called only from "main.c" and "signals.c".
 *
 * Note that the savefile is not saved until the tombstone is
 * actually displayed and the player has a chance to examine
 * the inventory and such.  This allows cheating if the game
 * is equipped with a "quit without save" method.  XXX XXX XXX
 */
void close_game(void)
{
	char buf[1024];


	/* Handle stuff */
	handle_stuff();

	/* Flush the messages */
	message_flush();

	/* Flush the input */
	flush();


	/* No suspending now */
	signals_ignore_tstp();


	/* Hack -- Increase "icky" depth */
	character_icky++;


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_APEX, "scores.raw");

	/* Grab permissions */
	safe_setuid_grab();

	/* Open the high score file, for reading/writing */
	highscore_fd = fd_open(buf, O_RDWR);

	/* Drop permissions */
	safe_setuid_drop();

	/* Handle death */
	if (p_ptr->is_dead)
	{
		/* Auxiliary routine */
		close_game_aux();
	}

	/* Still alive */
	else
	{
		/* Save the game */
		do_cmd_save_game();

		/* Prompt for scores XXX XXX XXX */
		prt("Press Return (or Escape).", 0, 40);

		/* Predict score (or ESCAPE) */
		if (anykey().key != ESCAPE) predict_score();
	}


	/* Shut the high score file */
	fd_close(highscore_fd);

	/* Forget the high score fd */
	highscore_fd = -1;


	/* Hack -- Decrease "icky" depth */
	character_icky--;


	/* Allow suspending now */
	signals_handle_tstp();
}


/*
 * Handle abrupt death of the visual system
 *
 * This routine is called only in very rare situations, and only
 * by certain visual systems, when they experience fatal errors.
 *
 * XXX XXX Hack -- clear the death flag when creating a HANGUP
 * save file so that player can see tombstone when restart.
 */
void exit_game_panic(void)
{
	/* If nothing important has happened, just quit */
	if (!character_generated || character_saved) quit("panic");

	/* Mega-Hack -- see "msg_print()" */
	msg_flag = FALSE;

	/* Clear the top line */
	prt("", 0, 0);

	/* Hack -- turn off some things */
	disturb(1, 0);

	/* Hack -- Delay death XXX XXX XXX */
	if (p_ptr->chp < 0) p_ptr->is_dead = FALSE;

	/* Hardcode panic save */
	p_ptr->panic_save = 1;

	/* Forbid suspend */
	signals_ignore_tstp();

	/* Indicate panic save */
	strcpy(p_ptr->died_from, "(panic save)");

	/* Panic save, or get worried */
	if (!save_player()) quit("panic save failed!");

	/* Successful panic save */
	quit("panic save succeeded!");
}



#ifdef HANDLE_SIGNALS


#include <signal.h>


typedef void (*Signal_Handler_t)(int);

/*
 * Wrapper around signal() which it is safe to take the address
 * of, in case signal itself is hidden by some some macro magic.
 */
static Signal_Handler_t wrap_signal(int sig, Signal_Handler_t handler)
{
	return signal(sig, handler);
}

/* Call this instead of calling signal() directly. */  
Signal_Handler_t (*signal_aux)(int, Signal_Handler_t) = wrap_signal;


/*
 * Handle signals -- suspend
 *
 * Actually suspend the game, and then resume cleanly
 */
static void handle_signal_suspend(int sig)
{
	/* Protect errno from library calls in signal handler */
	int save_errno = errno;

	/* Disable handler */
	(void)(*signal_aux)(sig, SIG_IGN);

#ifdef SIGSTOP

	/* Flush output */
	Term_fresh();

	/* Suspend the "Term" */
	Term_xtra(TERM_XTRA_ALIVE, 0);

	/* Suspend ourself */
	(void)kill(0, SIGSTOP);

	/* Resume the "Term" */
	Term_xtra(TERM_XTRA_ALIVE, 1);

	/* Redraw the term */
	Term_redraw();

	/* Flush the term */
	Term_fresh();

#endif

	/* Restore handler */
	(void)(*signal_aux)(sig, handle_signal_suspend);

	/* Restore errno */
	errno = save_errno;
}


/*
 * Handle signals -- simple (interrupt and quit)
 *
 * This function was causing a *huge* number of problems, so it has
 * been simplified greatly.  We keep a global variable which counts
 * the number of times the user attempts to kill the process, and
 * we commit suicide if the user does this a certain number of times.
 *
 * We attempt to give "feedback" to the user as he approaches the
 * suicide thresh-hold, but without penalizing accidental keypresses.
 *
 * To prevent messy accidents, we should reset this global variable
 * whenever the user enters a keypress, or something like that.
 */
static void handle_signal_simple(int sig)
{
	/* Protect errno from library calls in signal handler */
	int save_errno = errno;

	/* Disable handler */
	(void)(*signal_aux)(sig, SIG_IGN);


	/* Nothing to save, just quit */
	if (!character_generated || character_saved) quit(NULL);


	/* Count the signals */
	signal_count++;


	/* Terminate dead characters */
	if (p_ptr->is_dead)
	{
		/* Mark the savefile */
		strcpy(p_ptr->died_from, "Abortion");

		/* Close stuff */
		close_game();

		/* Quit */
		quit("interrupt");
	}

	/* Allow suicide (after 5) */
	else if (signal_count >= 5)
	{
		/* Cause of "death" */
		strcpy(p_ptr->died_from, "Interrupting");

		/* Commit suicide */
		p_ptr->is_dead = TRUE;

		/* Stop playing */
		p_ptr->playing = FALSE;

		/* Leaving */
		p_ptr->leaving = TRUE;

		/* Close stuff */
		close_game();

		/* Quit */
		quit("interrupt");
	}

	/* Give warning (after 4) */
	else if (signal_count >= 4)
	{
		/* Make a noise */
		Term_xtra(TERM_XTRA_NOISE, 0);

		/* Clear the top line */
		Term_erase(0, 0, 255);

		/* Display the cause */
		Term_putstr(0, 0, -1, TERM_WHITE, "Contemplating suicide!");

		/* Flush */
		Term_fresh();
	}

	/* Give warning (after 2) */
	else if (signal_count >= 2)
	{
		/* Make a noise */
		Term_xtra(TERM_XTRA_NOISE, 0);
	}

	/* Restore handler */
	(void)(*signal_aux)(sig, handle_signal_simple);

	/* Restore errno */
	errno = save_errno;
}


/*
 * Handle signal -- abort, kill, etc
 */
static void handle_signal_abort(int sig)
{
	/* Disable handler */
	(void)(*signal_aux)(sig, SIG_IGN);


	/* Nothing to save, just quit */
	if (!character_generated || character_saved) quit(NULL);


	/* Clear the bottom line */
	Term_erase(0, 23, 255);

	/* Give a warning */
	Term_putstr(0, 23, -1, TERM_RED,
	            "A gruesome software bug LEAPS out at you!");

	/* Message */
	Term_putstr(45, 23, -1, TERM_RED, "Panic save...");

	/* Flush output */
	Term_fresh();

	/* Panic Save */
	p_ptr->panic_save = 1;

	/* Panic save */
	strcpy(p_ptr->died_from, "(panic save)");

	/* Forbid suspend */
	signals_ignore_tstp();

	/* Attempt to save */
	if (save_player())
	{
		Term_putstr(45, 23, -1, TERM_RED, "Panic save succeeded!");
	}

	/* Save failed */
	else
	{
		Term_putstr(45, 23, -1, TERM_RED, "Panic save failed!");
	}

	/* Flush output */
	Term_fresh();

	/* Quit */
	quit("software bug");
}




/*
 * Ignore SIGTSTP signals (keyboard suspend)
 */
void signals_ignore_tstp(void)
{

#ifdef SIGTSTP
	(void)(*signal_aux)(SIGTSTP, SIG_IGN);
#endif

}

/*
 * Handle SIGTSTP signals (keyboard suspend)
 */
void signals_handle_tstp(void)
{

#ifdef SIGTSTP
	(void)(*signal_aux)(SIGTSTP, handle_signal_suspend);
#endif

}


/*
 * Prepare to handle the relevant signals
 */
void signals_init(void)
{

#ifdef SIGHUP
	(void)(*signal_aux)(SIGHUP, SIG_IGN);
#endif


#ifdef SIGTSTP
	(void)(*signal_aux)(SIGTSTP, handle_signal_suspend);
#endif


#ifdef SIGINT
	(void)(*signal_aux)(SIGINT, handle_signal_simple);
#endif

#ifdef SIGQUIT
	(void)(*signal_aux)(SIGQUIT, handle_signal_simple);
#endif


#ifdef SIGFPE
	(void)(*signal_aux)(SIGFPE, handle_signal_abort);
#endif

#ifdef SIGILL
	(void)(*signal_aux)(SIGILL, handle_signal_abort);
#endif

#ifdef SIGTRAP
	(void)(*signal_aux)(SIGTRAP, handle_signal_abort);
#endif

#ifdef SIGIOT
	(void)(*signal_aux)(SIGIOT, handle_signal_abort);
#endif

#ifdef SIGKILL
	(void)(*signal_aux)(SIGKILL, handle_signal_abort);
#endif

#ifdef SIGBUS
	(void)(*signal_aux)(SIGBUS, handle_signal_abort);
#endif

#ifdef SIGSEGV
	(void)(*signal_aux)(SIGSEGV, handle_signal_abort);
#endif

#ifdef SIGTERM
	(void)(*signal_aux)(SIGTERM, handle_signal_abort);
#endif

#ifdef SIGPIPE
	(void)(*signal_aux)(SIGPIPE, handle_signal_abort);
#endif

#ifdef SIGEMT
	(void)(*signal_aux)(SIGEMT, handle_signal_abort);
#endif

/*
 * SIGDANGER:
 * This is not a common (POSIX, SYSV, BSD) signal, it is used by AIX(?) to
 * signal that the system will soon be out of memory.
 */
#ifdef SIGDANGER
	(void)(*signal_aux)(SIGDANGER, handle_signal_abort);
#endif

#ifdef SIGSYS
	(void)(*signal_aux)(SIGSYS, handle_signal_abort);
#endif

#ifdef SIGXCPU
	(void)(*signal_aux)(SIGXCPU, handle_signal_abort);
#endif

#ifdef SIGPWR
	(void)(*signal_aux)(SIGPWR, handle_signal_abort);
#endif

}


#else	/* HANDLE_SIGNALS */


/*
 * Do nothing
 */
void signals_ignore_tstp(void)
{
}

/*
 * Do nothing
 */
void signals_handle_tstp(void)
{
}

/*
 * Do nothing
 */
void signals_init(void)
{
}


#endif	/* HANDLE_SIGNALS */


