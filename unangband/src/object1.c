/* File: object1.c */

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
 * Max sizes of the following arrays.
 */
#define MAX_TITLES     75       /* Used with scrolls (min 65) */
#define MAX_SYLLABLES 158       /* Used with scrolls (see below) */


/*
 * Syllables for scrolls (must be 1-4 letters each).
 */

static cptr syllables[MAX_SYLLABLES] =
{
	"a", "ab", "ag", "aks", "ala", "an", "ankh", "app",
	"arg", "arze", "ash", "aus", "ban", "bar", "bat", "bek",
	"bie", "bin", "bit", "bjor", "blu", "bot", "bu",
	"byt", "comp", "con", "cos", "cre", "dalf", "dan",
	"den", "der", "doe", "dok", "eep", "el", "eng", "er", "ere", "erk",
	"esh", "evs", "fa", "fid", "flit", "for", "fri", "fu", "gan",
	"gar", "glen", "gop", "gre", "ha", "he", "hyd", "i",
	"ing", "ion", "ip", "ish", "it", "ite", "iv", "jo",
	"kho", "kli", "klis", "la", "lech", "man", "mar",
	"me", "mi", "mic", "mik", "mon", "mung", "mur", "nag", "nej",
	"nelg", "nep", "ner", "nes", "nis", "nih", "nin", "o",
	"od", "ood", "org", "orn", "ox", "oxy", "pay", "pet",
	"ple", "plu", "po", "pot", "prok", "re", "rea", "rhov",
	"ri", "ro", "rog", "rok", "rol", "sa", "san", "sat",
	"see", "sef", "seh", "shu", "ski", "sna", "sne", "snik",
	"sno", "so", "sol", "sri", "sta", "sun", "ta", "tab",
	"tem", "ther", "ti", "tox", "trol", "tue", "turs", "u",
	"ulk", "um", "un", "uni", "ur", "val", "viv", "vly",
	"vom", "wah", "wed", "werg", "wex", "whon", "wun", "x",
	"yerg", "yp", "zun", "tri", "blaa"
};


/*
 * Hold the titles of scrolls, 6 to 14 characters each.
 *
 * Also keep an array of scroll colors (always WHITE for now).
 */

static char scroll_adj[MAX_TITLES][16];


static void flavor_assign_fixed(void)
{
	int i, j;

	for (i = 0; i < z_info->x_max; i++)
	{
		flavor_type *flavor_ptr = &x_info[i];

		/* Skip random flavors */
		if (flavor_ptr->sval == SV_UNKNOWN) continue;

		for (j = 0; j < z_info->k_max; j++)
		{
			/* Skip other objects */
			if ((k_info[j].tval == flavor_ptr->tval) &&
			    (k_info[j].sval == flavor_ptr->sval))
			{
				/* Store the flavor index */
				k_info[j].flavor = i;
			}
		}
	}
}


static void flavor_assign_random(byte tval)
{
	int i, j;
	int flavor_count = 0;
	int choice;

	/* Count the random flavors for the given tval */
	for (i = 0; i < z_info->x_max; i++)
	{
		if ((x_info[i].tval == tval) &&
		    (x_info[i].sval == SV_UNKNOWN))
		{
			flavor_count++;
		}
	}

	for (i = 0; i < z_info->k_max; i++)
	{
		/* Skip other object types */
		if (k_info[i].tval != tval) continue;

		/* Skip objects that already are flavored */
		if (k_info[i].flavor != 0) continue;

		/* HACK - Ordinary food is "boring" */
		if ((tval == TV_FOOD) && (k_info[i].sval >= SV_FOOD_MIN_FOOD))
			continue;

		/* HACK - Ordinary lites are "boring" */
		if ((tval == TV_LITE) && (k_info[i].sval >= SV_LITE_MAX_LITE))
			continue;

		if (!flavor_count) quit_fmt("Not enough flavors for tval %d.", tval);

		/* Select a flavor */
		choice = rand_int(flavor_count);
	
		/* Find and store the flavor */
		for (j = 0; j < z_info->x_max; j++)
		{
			/* Skip other tvals */
			if (x_info[j].tval != tval) continue;

			/* Skip assigned svals */
			if (x_info[j].sval != SV_UNKNOWN) continue;

			if (choice == 0)
			{
				/* Store the flavor index */
				k_info[i].flavor = j;

				/* Mark the flavor as used */
				x_info[j].sval = k_info[i].sval;

				/* One less flavor to choose from */
				flavor_count--;

				break;
			}

			choice--;
		}
	}
}


/*
 * Prepare the "variable" part of the "k_info" array.
 *
 * The "color"/"metal"/"type" of an item is its "flavor".
 * For the most part, flavors are assigned randomly each game.
 *
 * Initialize descriptions for the "colored" objects, including:
 * Rings, Amulets, Staffs, Wands, Rods, Food, Potions, Scrolls.
 *
 * The first 4 entries for potions are fixed (Water, Apple Juice,
 * Slime Mold Juice, Unused Potion).
 *
 * Scroll titles are always between 6 and 14 letters long.  This is
 * ensured because every title is composed of whole words, where every
 * word is from 1 to 8 letters long (one or two syllables of 1 to 4
 * letters each), and that no scroll is finished until it attempts to
 * grow beyond 15 letters.  The first time this can happen is when the
 * current title has 6 letters and the new word has 8 letters, which
 * would result in a 6 letter scroll title.
 *
 * Duplicate titles are avoided by requiring that no two scrolls share
 * the same first four letters (not the most efficient method, and not
 * the least efficient method, but it will always work).
 *
 * Hack -- make sure everything stays the same for each saved game
 * This is accomplished by the use of a saved "random seed", as in
 * "town_gen()".  Since no other functions are called while the special
 * seed is in effect, so this function is pretty "safe".
 */
void flavor_init(void)
{
	int i, j;


	/* Hack -- Use the "simple" RNG */
	Rand_quick = TRUE;

	/* Hack -- Induce consistant flavors */
	Rand_value = seed_flavor;


	flavor_assign_fixed();

	flavor_assign_random(TV_RING);
	flavor_assign_random(TV_AMULET);
	flavor_assign_random(TV_STAFF);
	flavor_assign_random(TV_WAND);
	flavor_assign_random(TV_ROD);
	flavor_assign_random(TV_FOOD);
	flavor_assign_random(TV_POTION);
	flavor_assign_random(TV_SCROLL);

	/* Scrolls (random titles, always white) */
	for (i = 0; i < MAX_TITLES; i++)
	{
		/* Get a new title */
		while (TRUE)
		{
			char buf[80];

			bool okay;

			/* Start a new title */
			buf[0] = '\0';

			/* Collect words until done */
			while (1)
			{
				int q, s;

				char tmp[80];

				/* Start a new word */
				tmp[0] = '\0';

				/* Choose one or two syllables */
				s = ((rand_int(100) < 30) ? 1 : 2);

				/* Add a one or two syllable word */
				for (q = 0; q < s; q++)
				{
					/* Add the syllable */
					my_strcat(tmp, syllables[rand_int(MAX_SYLLABLES)], sizeof(tmp));
				}

				/* Stop before getting too long */
				if (strlen(buf) + 1 + strlen(tmp) > 15) break;

				/* Add a space */
				strcat(buf, " ");

				/* Add the word */
				my_strcat(buf, tmp, sizeof(buf));
			}

			/* Save the title */
			my_strcpy(scroll_adj[i], buf+1, sizeof(scroll_adj[0]));

			/* Assume okay */
			okay = TRUE;

			/* Check for "duplicate" scroll titles */
			for (j = 0; j < i; j++)
			{
				cptr hack1 = scroll_adj[j];
				cptr hack2 = scroll_adj[i];

				/* Compare first four characters */
				if (*hack1++ != *hack2++) continue;
				if (*hack1++ != *hack2++) continue;
				if (*hack1++ != *hack2++) continue;
				if (*hack1++ != *hack2++) continue;

				/* Not okay */
				okay = FALSE;

				/* Stop looking */
				break;
			}

			/* Break when done */
			if (okay) break;
		}
	}


	/* Hack -- Use the "complex" RNG */
	Rand_quick = FALSE;

}

#ifdef ALLOW_BORG_GRAPHICS
extern void init_translate_visuals(void);
#endif /* ALLOW_BORG_GRAPHICS */


/*
 * Reset the "visual" lists
 *
 * This involves resetting various things to their "default" state.
 *
 * If the "prefs" flag is TRUE, then we will also load the appropriate
 * "user pref file" based on the current setting of the "use_graphics"
 * flag.  This is useful for switching "graphics" on/off.
 *
 * The features, objects, and monsters, should all be encoded in the
 * relevant "font.pref" and/or "graf.prf" files.  XXX XXX XXX
 *
 * The "prefs" parameter is no longer meaningful.  XXX XXX XXX
 */
void reset_visuals(bool unused)
{
	int i;

	/* Prevent compilation warning */
	(void)unused;

	/* Extract default attr/char code for features */
	for (i = 0; i < z_info->f_max; i++)
	{
		feature_type *f_ptr = &f_info[i];

		/* Assume we will use the underlying values */
		f_ptr->x_attr = f_ptr->d_attr;
		f_ptr->x_char = f_ptr->d_char;
	}

	/* Extract default attr/char code for objects */
	for (i = 0; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Default attr/char */
		k_ptr->x_attr = k_ptr->d_attr;
		k_ptr->x_char = k_ptr->d_char;
	}

	/* Extract default attr/char code for flavors */
	for (i = 0; i < z_info->x_max; i++)
	{
		flavor_type *x_ptr = &x_info[i];

		/* Default attr/char */
		x_ptr->x_attr = x_ptr->d_attr;
		x_ptr->x_char = x_ptr->d_char;
	}


	/* Extract default attr/char code for monsters */
	for (i = 0; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];

		/* Default attr/char */
		r_ptr->x_attr = r_ptr->d_attr;
		r_ptr->x_char = r_ptr->d_char;
	}


	/* Extract attr/chars for inventory objects (by tval) */
	for (i = 0; i < 128; i++)
	{
		/* Default to white */
		tval_to_attr[i] = TERM_WHITE;
	}


	/* Graphic symbols */
	if (use_graphics)
	{
		/* Process "graf.prf" */
		process_pref_file("graf.prf");
	}

	/* Normal symbols */
	else
	{
		/* Process "font.prf" */
		process_pref_file("font.prf");
	}

#ifdef ALLOW_BORG_GRAPHICS
	/* Initialize the translation table for the borg */
	init_translate_visuals();
#endif /* ALLOW_BORG_GRAPHICS */
}

/*
 * Efficient version of '(T) += strfmt((T), "%c", (C))'
 */
#define object_desc_chr_macro(T,C) do { \
 \
	/* Copy the char */ \
	*(T)++ = (C); \
 \
} while (0)



/*
 * Efficient version of '(T) += strfmt((T), "%s", (S))'
 */
#define object_desc_str_macro(T,S) do { \
 \
	cptr s = (S); \
 \
	/* Copy the string */ \
	while (*s) *(T)++ = *s++; \
 \
} while (0)



/*
 * Efficient version of '(T) += strfmt((T), "%u", (N))'
 */
#define object_desc_num_macro(T,N) do { \
 \
	uint n = (N); \
 \
	uint p; \
 \
	/* Find "size" of "n" */ \
	for (p = 1; n >= p * 10; p = p * 10) /* loop */; \
 \
	/* Dump each digit */ \
	while (p >= 1) \
	{ \
		/* Dump the digit */ \
		*(T)++ = I2D(n / p); \
 \
		/* Remove the digit */ \
		n = n % p; \
 \
		/* Process next digit */ \
		p = p / 10; \
	} \
 \
} while (0)



/*
 * Efficient version of '(T) += strfmt((T), "%+d", (I))'
 */
#define object_desc_int_macro(T,I) do { \
 \
	sint i = (I); \
 \
	/* Negative */ \
	if (i < 0) \
	{ \
		/* Take the absolute value */ \
		i = 0 - i; \
 \
		/* Use a "minus" sign */ \
		*(T)++ = '-'; \
	} \
 \
	/* Positive (or zero) */ \
	else \
	{ \
		/* Use a "plus" sign */ \
		*(T)++ = '+'; \
	} \
 \
	/* Dump the number itself */ \
	object_desc_num_macro(T, i); \
 \
} while (0)

/*
 * Creates a description of the item "o_ptr", and stores it in "out_val".
 *
 * One can choose the "verbosity" of the description, including whether
 * or not the "number" of items should be described, and how much detail
 * should be used when describing the item.
 *
 * The given "buf" must be 80 chars long to hold the longest possible
 * description, which can get pretty long, including incriptions, such as:
 * "no more Maces of Disruption (Defender) (+10,+10) [+5] (+3 to stealth)".
 * Note that the inscription will be clipped to keep the total description
 * under 79 chars (plus a terminator).
 *
 * This function uses a big temporary array to create the description,
 * and then copies up to 79 characters from this array into the buffer,
 * which will prevent crashes (but not ugliness) if any object name uses
 * more than 79 characters.
 *
 * Note the use of "object_desc_int_macro()" and "object_desc_num_macro()"
 * and "object_desc_str_macro()" and "object_desc_chr_macro()" as extremely
 * efficient, portable, versions of some common "strfmt()" commands (without
 * the bounds checking or termination writing), which allow a pointer to
 * efficiently move through a buffer while modifying it in various ways.
 *
 * Various improper uses and/or placements of "&" or "~" characters can
 * easily induce out-of-bounds memory accesses.  Some of these could be
 * easily checked for, if efficiency was not a concern.
 *
 * Note that all ego-items (when known) append an "Ego-Item Name", unless
 * the item is also an artifact, which should never happen.
 *
 * Note that all artifacts (when known) append an "Artifact Name", so we
 * have special processing for "Specials" (artifact Lites, Rings, Amulets).
 * The "Specials" never use "modifiers" if they are "known", since they
 * have special "descriptions", such as "The Necklace of the Dwarves".
 *
 * Special Lite's use the "k_info" base-name (Phial, Star, or Arkenstone),
 * plus the artifact name, just like any other artifact, if known.
 *
 * Special Ring's and Amulet's, if not "aware", use the same code as normal
 * rings and amulets, and if "aware", use the "k_info" base-name (Ring or
 * Amulet or Necklace).  They will NEVER "append" the "k_info" name.  But,
 * they will append the artifact name, just like any artifact, if known.
 *
 * None of the Special Rings/Amulets are "EASY_KNOW", though they could be,
 * at least, those which have no "pluses", such as the three artifact lites.
 *
 * Hack -- Display "The One Ring" as "a Plain Gold Ring" until aware.
 *
 * The "pluralization" rules are extremely hackish, in fact, for efficiency,
 * we only handle things like "torch"/"torches" and "cutlass"/"cutlasses",
 * and we would not handle "box"/"boxes", or "knife"/"knives", correctly.
 * Of course, it would be easy to add rules for these forms.
 *
 * If "pref" is true then a "numeric" prefix will be pre-pended, else is is
 * assumed that a string such as "The" or "Your" will be pre-pended later.
 *
 * Modes ("pref" is TRUE):
 *   0 -- Chain Mail of Death
 *   1 -- A Cloak of Death [1,+3]
 *   2 -- An Amulet of Death [1,+3] (+2 to Stealth)
 *   3 -- 5 Rings of Death [1,+3] (+2 to Stealth) {nifty}
 *
 * Modes ("pref" is FALSE):
 *   0 -- Chain Mail of Death
 *   1 -- Cloak of Death [1,+3]
 *   2 -- Amulet of Death [1,+3] (+2 to Stealth)
 *   3 -- Rings of Death [1,+3] (+2 to Stealth) {nifty}
 */
void object_desc(char *buf, size_t max, const object_type *o_ptr, int pref, int mode)
{
	cptr basenm;
	cptr modstr;

	int power;

	bool aware;
	bool known;

	int flavor;

	bool append_name;

	bool show_weapon;
	bool show_armour;

	char *b;

	char *t;

	cptr s;

	cptr u;
	cptr v;

	char p1 = '(', p2 = ')';
	char b1 = '[', b2 = ']';
	char c1 = '{', c2 = '}';

	char discount_buf[80];

	char tmp_buf[128];

	u32b f1, f2, f3, f4;

	object_kind *k_ptr = &k_info[o_ptr->k_idx];


	/* Prevent compiler warning */
	(void)max;

	/* Extract some flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4);


	/* See if the object is "aware" */
	aware = (object_aware_p(o_ptr) ? TRUE : FALSE);

	/* See if the object is "known" */
	known = (object_known_p(o_ptr) ? TRUE : FALSE);

	/* See if the object is "flavored" */
	flavor = k_ptr->flavor;

	/* Allow flavors to be hidden when aware */
	if (aware && !show_flavors) flavor = FALSE;

	/* Object is in the inventory of a store */
	if (o_ptr->ident & IDENT_STORE)
	{
		/* Don't show flavors */
		flavor = FALSE;

		/* Pretend known and aware */
		aware = TRUE;
		known = TRUE;
	}

	/* Assume no name appending */
	append_name = FALSE;

	/* Assume no need to show "weapon" bonuses */
	show_weapon = FALSE;

	/* Assume no need to show "armour" bonuses */
	show_armour = FALSE;

	/* Extract default "base" string */
	basenm = (k_name + k_ptr->name);

	/* Assume no "modifier" string */
	modstr = "";


	/* Analyze the object */
	switch (o_ptr->tval)
	{
		/* Some objects are easy to describe */
		case TV_JUNK:
		case TV_SPIKE:
		case TV_FLASK:
		case TV_INSTRUMENT:
		case TV_SPELL:
		case TV_MAP:
		{
			break;
		}

		/* Missiles/Bows/Weapons */
		case TV_SHOT:
		case TV_BOLT:
		case TV_ARROW:
		case TV_BOW:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_DIGGING:
		{
			show_weapon = TRUE;
			break;
		}

		/* Armour */
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CLOAK:
		case TV_CROWN:
		case TV_HELM:
		case TV_SHIELD:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{
			show_armour = TRUE;
			break;
		}

		/* Lites (including a few "Specials") */
		case TV_LITE:
		{
			break;
		}

		/* Amulets (including a few "Specials") */
		case TV_AMULET:
		{
			/* Hack -- Known artifacts */
			if (artifact_p(o_ptr) && aware) break;

			/* Color the object */
			modstr = x_text + x_info[flavor].text;

			if (aware) append_name = TRUE;
			basenm = (flavor ? "& # Amulet~" : "& Amulet~");

			break;
		}

		/* Rings (including a few "Specials") */
		case TV_RING:
		{
			/* Hack -- Known artifacts */
			if (artifact_p(o_ptr) && aware) break;

			/* Color the object */
			modstr = x_text + x_info[flavor].text;

			if (aware) append_name = TRUE;
			basenm = (flavor ? "& # Ring~" : "& Ring~");

			break;
		}

		/* Staffs */
		case TV_STAFF:
		{

			/* Color the object */
			modstr = x_text + x_info[flavor].text;

			if (aware) append_name = TRUE;
			basenm = (flavor ? "& # Staff~" : "& Staff~");
			show_weapon = TRUE;
			break;
		}

		/* Wands */
		case TV_WAND:
		{

			/* Color the object */
			modstr = x_text + x_info[flavor].text;

			if (aware) append_name = TRUE;
			basenm = (flavor ? "& # Wand~" : "& Wand~");

			break;
		}

		/* Rods */
		case TV_ROD:
		{
			/* Color the object */
			modstr = x_text + x_info[flavor].text;

			if (aware) append_name = TRUE;
			basenm = (flavor ? "& # Rod~" : "& Rod~");

			break;
		}

		/* Scrolls */
		case TV_SCROLL:
		{
			/* Color the object */
			modstr = scroll_adj[o_ptr->sval];
			if (aware) append_name = TRUE;
			basenm = (flavor ? "& Scroll~ titled \"#\"" : "& Scroll~");

			break;
		}

		/* Potions */
		case TV_POTION:
		{
			/* Color the object */
			modstr = x_text + x_info[flavor].text;

			if (aware) append_name = TRUE;
			basenm = (flavor ? "& # Potion~" : "& Potion~");

			break;
		}

		/* Food */
		case TV_FOOD:
		{
			/* Ordinary food is "boring" */
			if (o_ptr->sval >= SV_FOOD_MIN_FOOD) break;

			/* Color the object */
			modstr = x_text + x_info[flavor].text;

			if (aware) append_name = TRUE;
			basenm = (flavor ? "& # Mushroom~" : "& Mushroom~");

			break;
		}

		/* Magic Books */
		case TV_MAGIC_BOOK:
		{
			modstr = basenm;
			basenm = "& Magic Book~ #";
			break;
		}


		/* Prayer Books */
		case TV_PRAYER_BOOK:
		{
			modstr = basenm;
			basenm = "& Prayer Book~ #";
			break;
		}

		/* Song Books */
		case TV_SONG_BOOK:
		{
			modstr = basenm;
			basenm = "& Song Book~ #";
			break;
		}

		/* Runestones */
		case TV_RUNESTONE:
		{
			modstr = basenm;
			basenm = "& # Rune stone~";
			break;
		}

		/* Runestones */
		case TV_SERVICE:
		{
			append_name = TRUE;
			basenm = "& Service~";
			break;
		}

		/* Hack -- Gold/Gems */
		case TV_GOLD:
		{
			strcpy(buf, basenm);
			return;
		}

		case TV_HOLD:
		{
			if (o_ptr->name3 > 0) modstr = "sealed";
			else modstr = "empty";
			break;
		}

		/* Hack -- Body Parts/Skeletons/Skins etc. */
		case TV_STATUE:
		case TV_ASSEMBLY:
		case TV_BODY:
		case TV_BONE:
		case TV_EGG:
		case TV_SKIN:
		{
			if (!o_ptr->name3)
			{
				switch (o_ptr->tval)
				{
					case TV_STATUE:
						modstr = "an ancient god";
						break;
					case TV_ASSEMBLY:
						modstr = "mechanism";
						break;
					case TV_SKIN:
						modstr = "dusty";
						break;
					case TV_BODY:
						modstr = "mummified";
						break;
					case TV_EGG:
						if (o_ptr->sval == SV_EGG_SPORE)
						{
							modstr = "dried";
							break;
						}
						/* Drop down */
					default:
						modstr = "broken";
						break;
				}
			}
			else if (r_info[o_ptr->name3].flags1 & (RF1_UNIQUE))
			{
				char tmp_buf[160];

				strcpy(tmp_buf, r_name + r_info[o_ptr->name3].name);
				strcat(tmp_buf, "'s");
				modstr = tmp_buf;

				/* Skip a/an */
				if (basenm[2] == '#') basenm = &basenm[2];
			}
			else
			{
				modstr = (r_name + r_info[o_ptr->name3].name);
			}
			break;
		}

		/* Hack -- Default -- Used in the "inventory" routine */
		default:
		{
			strcpy(buf, "(nothing)");
			return;
		}
	}


	/* Start dumping the result */
	t = b = tmp_buf;

	/* Begin */
	s = basenm;

	/* Handle objects which sometimes use "a" or "an" */
	if (*s == '&')
	{
		/* Paranoia XXX XXX XXX */
		/* ASSERT(s[1] == ' '); */

		/* Skip the ampersand and the following space */
		s += 2;

		/* No prefix */
		if (!pref)
		{
			/* Nothing */
		}

		/* Hack -- None left */
		else if (o_ptr->number <= 0)
		{
			object_desc_str_macro(t, "no more ");
		}

		/* Extract the number */
		else if (o_ptr->number > 1)
		{
			object_desc_num_macro(t, o_ptr->number);
			object_desc_chr_macro(t, ' ');
		}

		/* Hack -- The only one of its kind */
		else if (known && artifact_p(o_ptr))
		{
			object_desc_str_macro(t, "The ");
		}

		/* Hack -- A single one, and next character will be a vowel */
		else if ((*s == '#') ? is_a_vowel(modstr[0]) : is_a_vowel(*s))
		{
			object_desc_str_macro(t, "an ");
		}

		/* A single one, and next character will be a non-vowel */
		else
		{
			object_desc_str_macro(t, "a ");
		}
	}

	/* Handle objects which never use "a" or "an" */
	else
	{
		/* No pref */
		if (!pref)
		{
			/* Nothing */
		}

		/* Hack -- all gone */
		else if (o_ptr->number <= 0)
		{
			object_desc_str_macro(t, "no more ");
		}

		/* Prefix a number if required */
		else if (o_ptr->number > 1)
		{
			object_desc_num_macro(t, o_ptr->number);
			object_desc_chr_macro(t, ' ');
		}

		/* Hack -- The only one of its kind */
		else if (known && artifact_p(o_ptr))
		{
			object_desc_str_macro(t, "The ");
		}

		/* Hack -- A single item, so no prefix needed */
		else
		{
			/* Nothing */
		}
	}

	/* Hack -- display debug stack info in cheat_xtra mode */
	if ((cheat_xtra) && (o_ptr->stackc))
	{
		object_desc_str_macro(t, "(");
		object_desc_num_macro(t,o_ptr->stackc);
		object_desc_str_macro(t, ") ");
	}


	/* Paranoia XXX XXX XXX */
	/* ASSERT(*s != '~'); */

	/* Copy the string */
	for (; *s; s++)
	{
		/* Pluralizer */
		if (*s == '~')
		{
			/* Add a plural if needed */
			if (o_ptr->number != 1)
			{
				char k = t[-1];

				/* Hack -- "Cutlass-es" and "Torch-es" */
				if ((k == 's') || (k == 'h')) *t++ = 'e';

				/* Add an 's' */
				*t++ = 's';
			}
		}

		/* Modifier */
		else if (*s == '#')
		{
			/* Append the modifier */
			object_desc_str_macro(t, modstr);
		}

		/* Normal */
		else
		{
			/* Copy */
			*t++ = *s;
		}
	}


	/* Append the "kind name" to the "base name" */
	if (append_name)
	{
		object_desc_str_macro(t, " of ");
		object_desc_str_macro(t, (k_name + k_ptr->name));
	}


	/* Hack -- Append "Artifact" or "Special" names */
	if (known)
	{
		/* Grab any artifact name */
		if (o_ptr->name1)
		{
			artifact_type *a_ptr = &a_info[o_ptr->name1];

			object_desc_chr_macro(t, ' ');
			object_desc_str_macro(t, (a_name + a_ptr->name));
		}

		/* Grab any ego-item name */
		else if (o_ptr->name2)
		{
			ego_item_type *e_ptr = &e_info[o_ptr->name2];

			object_desc_chr_macro(t, ' ');
			object_desc_str_macro(t, (e_name + e_ptr->name));
		}

		/* Grab any magic-item name */
		else if ((o_ptr->xtra1) && (o_ptr->xtra1 < OBJECT_XTRA_MIN_RUNES) && (o_ptr->discount < INSCRIP_MIN_HIDDEN))
		{
			int i;
			u32b j;

			for (i = 0, j = 0x00000001L; (i< 32) && (j != object_xtra_base[o_ptr->xtra1]);i++, j <<= 1);

			object_desc_chr_macro(t, ' ');
			object_desc_str_macro(t, magic_name[object_xtra_what[o_ptr->xtra1]-1][i + o_ptr->xtra2]);
		}
	}
	/* Hack -- Append guessed names */
	else
	{
		/* Grab any artifact name */
		if (o_ptr->guess1)
		{
			artifact_type *a_ptr = &a_info[o_ptr->guess1];

			object_desc_str_macro(t, " (");
			if (*(a_name + a_ptr->name) == '(')
			{
				object_desc_str_macro(t, (a_name + a_ptr->name) + 1);
				*(t)-- = '\0';
			}
			else
			{
				object_desc_str_macro(t, (a_name + a_ptr->name));
			}
			object_desc_str_macro(t, "?)");
		}

		/* Grab any ego-item name */
		else if (o_ptr->guess2)
		{
			ego_item_type *e_ptr = &e_info[o_ptr->guess2];

			object_desc_str_macro(t, " (");
			if (*(e_name + e_ptr->name) == '(')
			{
				object_desc_str_macro(t, (e_name + e_ptr->name) + 1);
				*(t)-- = '\0';
			}
			else
			{
				object_desc_str_macro(t, (e_name + e_ptr->name));
			}
			object_desc_str_macro(t, "?)");
		}

		/* Grab any kind name */
		else if (k_ptr->guess)
		{
			object_kind *k_ptr = &k_info[lookup_kind(o_ptr->tval,k_ptr->guess-1)];

			object_desc_str_macro(t, " (of ");
			object_desc_str_macro(t, (k_name + k_ptr->name));
			object_desc_str_macro(t, "?)");
		}

		/* Grab any magic name */
		else if (o_ptr->discount < INSCRIP_MIN_HIDDEN)
		{
			int i;
			int x1, x2; /* Fake xtra flags */
			u32b j;

			x1 = 0;
			x2 = 0;

			/* Loop through first flags */
			for (i = 0, j = 0x00000001L; i < 32; i++, j<<=1)
			{
				/* Found a flag */
				if ((j & o_ptr->can_flags1) != 0)
				{
					/* First flag */
					if (!x1) { x1 = 1; x2 = i; }

					/* More than one flag - can't guess magic name */
					else {x1 = -1; }
				}
			}

			/* Loop through second flags */
			for (i = 0, j = 0x00000001L; i < 32; i++, j <<=1)
			{
				/* Found a flag */
				if ((j & o_ptr->can_flags2) != 0)
				{
					/* First flag */
					if (!x1) { x1 = 2; x2 = i; }

					/* More than one flag - can't guess magic name */
					else {x1 = -1; }
				}
			}

			/* Loop through third flags */
			for (i = 0, j = 0x00000001L; i < 32; i++, j <<=1)
			{
				/* Found a flag */
				if ((j & o_ptr->can_flags3) != 0)
				{
					/* First flag */
					if (!x1) { x1 = 3; x2 = i; }

					/* More than one flag - can't guess magic name */
					else {x1 = -1; }
				}
			}

			/* Loop through fourth flags */
			for (i = 0, j = 0x00000001L; i < 32; i++, j <<=1)
			{
				/* Found a flag */
				if ((j & o_ptr->can_flags4) != 0)
				{
					/* First flag */
					if (!x1) { x1 = 4; x2 = i; }

					/* More than one flag - can't guess magic name */
					else {x1 = -1; }
				}
			}

			if (x1 > 0)
			{
				object_desc_str_macro(t, " (");
				if (*(magic_name[x1-1][x2]) == '(')
				{
					object_desc_str_macro(t, (magic_name[x1-1][x2]) + 1);
					*(t)-- = '\0';			
				}
				else
				{
					object_desc_str_macro(t, magic_name[x1-1][x2]);
				}

				object_desc_str_macro(t, "?)");
			}	
		}
	}

	/* Looks like/holds a monster */
	if ((o_ptr->tval == TV_HOLD) && (o_ptr->name3 > 0))
	{
		object_desc_str_macro(t, " containing ");

		if (known)
		{
	
			if (!(r_info[o_ptr->name3].flags1 & (RF1_UNIQUE)))
			{
				cptr name= r_name + r_info[o_ptr->name3].name;
	
				if (is_a_vowel(name[0])) object_desc_str_macro(t, "an ");
				else object_desc_str_macro(t, "a ");
			}
	
			object_desc_str_macro(t, r_name + r_info[o_ptr->name3].name);
		}
		else
		{
			object_desc_str_macro(t, "something");
		}
	}

	/* No more details wanted */
	if (mode < 1) goto object_desc_done;

	/* Display the item like a weapon */
	if (f3 & (TR3_SHOW_MODS)) show_weapon = TRUE;

	/* Display the item like a weapon */
	if (o_ptr->to_h && o_ptr->to_d) show_weapon = TRUE;

	/* Display the item like armour */
	if (o_ptr->ac) show_armour = TRUE;


	/* Dump base weapon info */
	switch (o_ptr->tval)
	{
		/* Missiles */
		case TV_SHOT:
		case TV_BOLT:
		case TV_ARROW:
		{
			/* Fall through */
		}

		/* Weapons */
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_STAFF:
		case TV_DIGGING:
		{
			/* Append a "damage" string */
			object_desc_chr_macro(t, ' ');
			object_desc_chr_macro(t, p1);
			object_desc_num_macro(t, o_ptr->dd);
			object_desc_chr_macro(t, 'd');
			object_desc_num_macro(t, o_ptr->ds);
			object_desc_chr_macro(t, p2);

			/* All done */
			break;
		}

		/* Bows */
		case TV_BOW:
		{
			/* Hack -- Extract the "base power" */
			power = (o_ptr->sval % 10);

			/* Append a "power" string */
			object_desc_chr_macro(t, ' ');
			object_desc_chr_macro(t, p1);
			object_desc_chr_macro(t, 'x');
			object_desc_num_macro(t, power);
			object_desc_chr_macro(t, p2);

			/* All done */
			break;
		}
	}


	/* Add the weapon bonuses */
	if ((known) || (o_ptr->ident & (IDENT_BONUS)))
	{
		/* Show the tohit/todam on request */
		if (show_weapon)
		{
			object_desc_chr_macro(t, ' ');
			object_desc_chr_macro(t, p1);
			object_desc_int_macro(t, o_ptr->to_h);
			object_desc_chr_macro(t, ',');
			object_desc_int_macro(t, o_ptr->to_d);
			object_desc_chr_macro(t, p2);
		}

		/* Show the tohit if needed */
		else if (o_ptr->to_h)
		{
			object_desc_chr_macro(t, ' ');
			object_desc_chr_macro(t, p1);
			object_desc_int_macro(t, o_ptr->to_h);
			object_desc_chr_macro(t, p2);
		}

		/* Show the todam if needed */
		else if (o_ptr->to_d)
		{
			object_desc_chr_macro(t, ' ');
			object_desc_chr_macro(t, p1);
			object_desc_int_macro(t, o_ptr->to_d);
			object_desc_chr_macro(t, p2);
		}
	}


	/* Add the armor bonuses */
	if ((known) || (o_ptr->ident & (IDENT_BONUS)))
	{
		/* Show the armor class info */
		if (show_armour)
		{
			object_desc_chr_macro(t, ' ');
			object_desc_chr_macro(t, b1);
			object_desc_num_macro(t, o_ptr->ac);
			object_desc_chr_macro(t, ',');
			object_desc_int_macro(t, o_ptr->to_a);
			object_desc_chr_macro(t, b2);
		}

		/* No base armor, but does increase armor */
		else if (o_ptr->to_a)
		{
			object_desc_chr_macro(t, ' ');
			object_desc_chr_macro(t, b1);
			object_desc_int_macro(t, o_ptr->to_a);
			object_desc_chr_macro(t, b2);
		}
	}

	/* Hack -- always show base armor */
	else if (show_armour)
	{
		object_desc_chr_macro(t, ' ');
		object_desc_chr_macro(t, b1);
		object_desc_num_macro(t, o_ptr->ac);
		object_desc_chr_macro(t, b2);
	}


	/* No more details wanted */
	if (mode < 2) goto object_desc_done;


	/* Hack -- Wands and Staffs have charges */
	if (((known) || (o_ptr->ident & (IDENT_BONUS))) &&
	    ((o_ptr->tval == TV_STAFF) ||
	     (o_ptr->tval == TV_WAND)))
	{
		/* Dump " (N charges)" */
		object_desc_chr_macro(t, ' ');
		object_desc_chr_macro(t, p1);
		object_desc_num_macro(t, o_ptr->pval);
		object_desc_str_macro(t, " charge");
		if (o_ptr->pval != 1)
		{
			object_desc_chr_macro(t, 's');
		}
		object_desc_chr_macro(t, p2);
	}

	/* Hack -- Process Lanterns/Torches */
	else if ((o_ptr->tval == TV_LITE) && (!artifact_p(o_ptr)))
	{
		/* Hack -- Turns of light for normal lites */
		object_desc_str_macro(t, " (with ");
		object_desc_num_macro(t, o_ptr->pval);
		object_desc_str_macro(t, " turns of light)");
	}


	/* Dump "pval" flags for wearable items */
	if ((known || (o_ptr->ident & (IDENT_BONUS))) && (f1 & (TR1_PVAL_MASK)))
	{
		cptr tail = "";
		cptr tail2 = "";

		/* Start the display */
		object_desc_chr_macro(t, ' ');
		object_desc_chr_macro(t, p1);

		/* Dump the "pval" itself */
		object_desc_int_macro(t, o_ptr->pval);

		/* Do not display the "pval" flags */
		if (f3 & (TR3_HIDE_TYPE))
		{
			/* Nothing */
		}

		/* Do not display the "pval" flags */
		else if (!known)
		{
			/* Nothing */
		}

		/* Stealth */
		else if (f1 & (TR1_STEALTH))
		{
			/* Dump " to stealth" */
			tail = " to stealth";
		}

		/* Searching */
		else if (f1 & (TR1_SEARCH))
		{
			/* Dump " to searching" */
			tail = " to searching";
		}

		/* Infravision */
		else if (f1 & (TR1_INFRA))
		{
			/* Dump " to infravision" */
			tail = " to infravision";
		}

#if 0

		/* Tunneling */
		else if (f1 & (TR1_TUNNEL))
		{
			/* Dump " to digging" */
			tail = " to digging";
		}

#endif

		/* Speed */
		else if (f1 & (TR1_SPEED))
		{
			/* Dump " to speed" */
			tail = " to speed";
		}

		/* Blows */
		else if (f1 & (TR1_BLOWS))
		{
			/* Add " attack" */
			tail = " attack";

			/* Add "attacks" */
			if (ABS(o_ptr->pval) != 1) tail2 = "s";
		}

#if 0

		/* Shots */
		else if (f1 & (TR1_SHOTS))
		{
			/* Nothing */
		}

		/* Might */
		else if (f1 & (TR1_MIGHT))
		{
			/* Nothing */
		}

#endif

		/* Add the descriptor */
		object_desc_str_macro(t, tail);
		object_desc_str_macro(t, tail2);

		/* Finish the display */
		object_desc_chr_macro(t, p2);
	}


	/* Indicate "charging" artifacts/rods */
	if (o_ptr->timeout)
	{
		if ((o_ptr->tval == TV_ROD) || (f3 & (TR3_ACTIVATE)))
		{
			/* Hack -- variant timeout stack */
			if (o_ptr->stackc)
			{
				object_desc_str_macro(t, " (");
				object_desc_num_macro(t, o_ptr->stackc);
				object_desc_str_macro(t, " charging)");
			}
			else
			{
				object_desc_str_macro(t, " (charging)");
			}
		}
	}

	/* No more details wanted */
	if (mode < 3) goto object_desc_done;

	if (o_ptr->xtra1 >= OBJECT_XTRA_MIN_RUNES)
	{
		object_desc_str_macro(t, " <");
		if (o_ptr->xtra2 > 1)
		{
			object_desc_num_macro(t, o_ptr->xtra2);
			object_desc_chr_macro(t,' ');
		}
		object_desc_str_macro(t, y_name+y_info[o_ptr->xtra1 - OBJECT_XTRA_MIN_RUNES].name);
		if (o_ptr->xtra2 > 1) object_desc_chr_macro(t,'s');
		object_desc_chr_macro(t, '>');
	}

	/* Use standard inscription */
	if (o_ptr->note)
	{
		u = quark_str(o_ptr->note);
	}

	/* Use nothing */
	else
	{
		u = NULL;
	}


	/* Use special inscription, if any */
	if (o_ptr->discount >= INSCRIP_NULL)
	{
		v = inscrip_text[o_ptr->discount - INSCRIP_NULL];
		if (strlen(v) == 0) v = NULL;
	}

	/* Use "cursed" if the item is known to be cursed */
	else if (cursed_p(o_ptr) && known)
	{
		v = "cursed";
	}

	/* Use "tried" if the object has been tested unsuccessfully, but not guessed */
	else if (!aware && object_tried_p(o_ptr))
	{
		v = "tried";
	}

	/* Use the discount, if any */
	else if (o_ptr->discount > 0)
	{
		char *q = discount_buf;
		object_desc_num_macro(q, o_ptr->discount);
		object_desc_str_macro(q, "% off");
		*q = '\0';
		v = discount_buf;
	}

	/* Nothing */
	else
	{
		v = NULL;
	}


	/* Inscription */
	if (u || v)
	{
		/* Begin the inscription */
		*t++ = ' ';
		*t++ = c1;

		/* Standard inscription */
		if (u)
		{
			/* Append the inscription */
			while ((t < b + 75) && *u) *t++ = *u++;
		}

		/* Special inscription too */
		if (u && v && (t < b + 75))
		{
			/* Separator */
			*t++ = ',';
			*t++ = ' ';
		}

		/* Special inscription */
		if (v)
		{
			/* Append the inscription */
			while ((t < b + 75) && *v) *t++ = *v++;
		}

		/* Terminate the inscription */
		*t++ = c2;
	}


object_desc_done:

	/* Terminate */
	*t = '\0';

	/* Truncate the string to 80 chars */
	tmp_buf[79] = '\0';

	/* Copy the string over */
	strcpy(buf, tmp_buf);
}

/*
 * Describe an item and pretend the item is fully known and has no flavor.
 */
void object_desc_spoil(char *buf, size_t max, const object_type *o_ptr, int pref, int mode)
{
	object_type object_type_body;
	object_type *i_ptr = &object_type_body;

	/* Make a backup */
	object_copy(i_ptr, o_ptr);

	/* HACK - Pretend the object is in a store inventory */
	i_ptr->ident |= IDENT_STORE;

	/* Describe */
	object_desc(buf, max, i_ptr, pref, mode);
}


/*
 * Convert an inventory index into a one character label.
 *
 * Note that the label does NOT distinguish inven/equip.
 */
char index_to_label(int i)
{
	/* Indexes for "inven" are easy */
	if (i < INVEN_WIELD) return (I2A(i));

	/* Indexes for "equip" are offset */
	return (I2A(i - INVEN_WIELD));
}


/*
 * Convert a label into the index of an item in the "inven".
 *
 * Return "-1" if the label does not indicate a real item.
 */
s16b label_to_inven(int c)
{
	int i;

	/* Convert */
	i = (islower(c) ? A2I(c) : -1);

	/* Verify the index */
	if ((i < 0) || (i > INVEN_PACK)) return (-1);

	/* Empty slots can never be chosen */
	if (!inventory[i].k_idx) return (-1);

	/* Return the index */
	return (i);
}


/*
 * Convert a label into the index of a item in the "equip".
 *
 * Return "-1" if the label does not indicate a real item.
 */
s16b label_to_equip(int c)
{
	int i;

	/* Convert */
	i = (islower(c) ? A2I(c) : -1) + INVEN_WIELD;

	/* Verify the index */
	if ((i < INVEN_WIELD) || (i >= INVEN_TOTAL)) return (-1);

	/* Empty slots can never be chosen */
	if (!inventory[i].k_idx) return (-1);

	/* Return the index */
	return (i);
}



/*
 * Determine which equipment slot (if any) an item likes
 */
s16b wield_slot(const object_type *o_ptr)
{
	/* Slot for equipment */
	switch (o_ptr->tval)
	{
		case TV_SWORD:
			/* Two-weapon combat -- Wielding a light stabbing weapon */
			if ((inventory[INVEN_WIELD].k_idx)
				&& ((inventory[INVEN_WIELD].tval == TV_SWORD)
					|| (inventory[INVEN_WIELD].tval == TV_STAFF)
					|| (inventory[INVEN_WIELD].tval == TV_POLEARM)
						|| (inventory[INVEN_WIELD].tval == TV_HAFTED)))
			{
				if ((inventory[INVEN_WIELD].weight < 200) && (o_ptr->weight < 100)) return (INVEN_ARM);
			}

			/* Fall through */
		case TV_STAFF:
  			/* Two-weapon combat -- all staffs are "identical" */
			if ((inventory[INVEN_WIELD].k_idx)
				&& (inventory[INVEN_WIELD].tval == TV_STAFF))
			{
				return (INVEN_ARM);
			}
			/* Fall through */
		case TV_HAFTED:
		case TV_POLEARM:
  			/* Two-weapon combat -- identical weapons are "balanced" if less than 15 lbs*/
			if ((inventory[INVEN_WIELD].k_idx)
				&& ((inventory[INVEN_WIELD].tval == TV_SWORD)
					|| (inventory[INVEN_WIELD].tval == TV_STAFF)
					|| (inventory[INVEN_WIELD].tval == TV_POLEARM)
						|| (inventory[INVEN_WIELD].tval == TV_HAFTED)))
			{
				object_type *i_ptr = &inventory[INVEN_WIELD];

				if ((i_ptr->tval == o_ptr->tval) && (i_ptr->sval == o_ptr->sval) 
						&& (o_ptr->weight < 150)) return (INVEN_ARM);
			}

			/* Two-weapon combat -- primary weapon */
			if ((inventory[INVEN_ARM].k_idx)
				&& ((inventory[INVEN_ARM].tval == TV_SWORD)
					|| (inventory[INVEN_ARM].tval == TV_STAFF)
					|| (inventory[INVEN_ARM].tval == TV_POLEARM)
						|| (inventory[INVEN_ARM].tval == TV_HAFTED)))
			{
				object_type *i_ptr = &inventory[INVEN_ARM];

				/* Wielding a light stabbing weapon */
				if ((i_ptr->tval == TV_SWORD) && (o_ptr->weight < 200) && (i_ptr->weight < 100)) return (INVEN_WIELD);

				/* Identical weapons are "balanced" if less than 20 lbs */
				if ((i_ptr->tval == o_ptr->tval) && (i_ptr->sval == o_ptr->sval) && (o_ptr->weight < 150)) return (INVEN_WIELD);

				/* Hack -- too unbalanced to wield */
				return (-1);
			}

			/* Wield in primary hand */
			return (INVEN_WIELD);
			break;

		case TV_DIGGING:
		{
			/* Two-weapon combat not allowed */
			if ((inventory[INVEN_ARM].k_idx) && (inventory[INVEN_ARM].tval != TV_SHIELD)) return (-1);

			/* Wield in primary hand */
			return (INVEN_WIELD);
			break;
		}
                case TV_INSTRUMENT:
		case TV_BOW:
		{
			return (INVEN_BOW);
		}

		case TV_RING:
		{
			/* Use the right hand first */
			if (!inventory[INVEN_RIGHT].k_idx) return (INVEN_RIGHT);

			/* Use the left hand for swapping (by default) */
			return (INVEN_LEFT);
		}

		case TV_AMULET:
		{
			return (INVEN_NECK);
		}

		case TV_LITE:
		{
			return (INVEN_LITE);
		}

		case TV_DRAG_ARMOR:
		case TV_HARD_ARMOR:
		case TV_SOFT_ARMOR:
		{
			return (INVEN_BODY);
		}

		case TV_CLOAK:
		{
			return (INVEN_OUTER);
		}

		case TV_SHIELD:
		{
			return (INVEN_ARM);
		}

		case TV_CROWN:
		case TV_HELM:
		{
			return (INVEN_HEAD);
		}

		case TV_GLOVES:
		{
			return (INVEN_HANDS);
		}

		case TV_BOOTS:
		{
			return (INVEN_FEET);
		}

		default:
		{
			break;
		}

	}

	/* No slot available */
	return (-1);
}


/*
 * Return a string mentioning how a given item is carried
 */
cptr mention_use(int i)
{
	cptr p;

	/* Examine the location */
	switch (i)
	{
		case INVEN_WIELD: p = "Wielding"; break;
		case INVEN_BOW:   p = "Shooting"; break;
		case INVEN_LEFT:  p = "On left hand"; break;
		case INVEN_RIGHT: p = "On right hand"; break;
		case INVEN_NECK:  p = "Around neck"; break;
		case INVEN_LITE:  p = "Light source"; break;
		case INVEN_BODY:  p = "On body"; break;
		case INVEN_OUTER: p = "About body"; break;
		case INVEN_ARM:   p = "On arm"; break;
		case INVEN_HEAD:  p = "On head"; break;
		case INVEN_HANDS: p = "On hands"; break;
		case INVEN_FEET:  p = "On feet"; break;
		default:	  p = "In pack"; break;
	}

	/* Hack -- Heavy weapon */
	if ((i == INVEN_WIELD) && (p_ptr->heavy_wield)) p = "Just lifting";

	/* Hack -- Heavy bow */
	if ((i == INVEN_BOW) && (p_ptr->heavy_shoot)) p = "Just holding";

	/* Hack -- Non-shield item */
	if (i == INVEN_WIELD)
	{
		object_type *o_ptr;
		o_ptr = &inventory[i];

		/* Multiple wielded items */
		if (o_ptr->number > 1) p = "Carrying";

		/* Describe use better */
		else switch (o_ptr->tval)
		{
			case TV_SWORD:
			case TV_STAFF:
			case TV_POLEARM:
			case TV_HAFTED:
				break;

			default:
				p = "Using";
				break;
		}
	}
	/* Hack -- Non-shield item */
	else if (i == INVEN_ARM)
	{
		object_type *o_ptr;
		o_ptr = &inventory[i];
		if (o_ptr->tval != TV_SHIELD)
		{
			p = "In off-hand";
		}
	}

	/* Hack -- Instrument */
	else if (i == INVEN_BOW)
	{
		object_type *o_ptr;
		o_ptr = &inventory[i];
		if (o_ptr->tval == TV_INSTRUMENT)
		{
                        p = "Playing";
		}
	}



	/* Return the result */
	return (p);
}


/*
 * Return a string describing how a given item is being worn.
 * Currently, only used for items in the equipment, not inventory.
 */
cptr describe_use(int i)
{
	cptr p;

	switch (i)
	{
		case INVEN_WIELD: p = "attacking monsters with"; break;
		case INVEN_BOW:   p = "shooting missiles with"; break;
		case INVEN_LEFT:  p = "wearing on your left hand"; break;
		case INVEN_RIGHT: p = "wearing on your right hand"; break;
		case INVEN_NECK:  p = "wearing around your neck"; break;
		case INVEN_LITE:  p = "using to light the way"; break;
		case INVEN_BODY:  p = "wearing on your body"; break;
		case INVEN_OUTER: p = "wearing on your back"; break;
		case INVEN_ARM:   p = "wearing on your arm"; break;
		case INVEN_HEAD:  p = "wearing on your head"; break;
		case INVEN_HANDS: p = "wearing on your hands"; break;
		case INVEN_FEET:  p = "wearing on your feet"; break;
		default:	  p = "carrying in your pack"; break;
	}

	if (i<0) p = "using from the ground";

	/* Hack -- Heavy weapon */
	if ((i == INVEN_WIELD) && (p_ptr->heavy_wield)) p = "Just lifting";

	/* Hack -- Heavy bow */
	if ((i == INVEN_BOW) && (p_ptr->heavy_shoot)) p = "Just holding";

	/* Hack -- Non-shield item */
	if (i == INVEN_WIELD)
	{
		object_type *o_ptr;
		o_ptr = &inventory[i];

		/* Multiple wielded items */
		if (o_ptr->number > 1) p = "carrying in your hands";

		/* Describe use better */
		else switch (o_ptr->tval)
		{
			case TV_DIGGING:
				p = "digging with";
				break;

			case TV_SWORD:
			case TV_STAFF:
			case TV_POLEARM:
			case TV_HAFTED:
				break;

			default:
				p = "using in your hands";
				break;
		}
	}

	/* Hack -- Non-shield item */
	else if (i == INVEN_ARM)
	{
		object_type *o_ptr;
		o_ptr = &inventory[i];
		if (o_ptr->tval != TV_SHIELD)
		{
			p = "using in your off-hand";
		}
	}

	/* Hack -- Instrument */
	else if (i == INVEN_BOW)
	{
		object_type *o_ptr;
		o_ptr = &inventory[i];
                if (o_ptr->tval == TV_INSTRUMENT)
		{
                        p = "playing music with";
		}
	}

	/* Return the result */
	return p;
}

/*
 * Check an item against the item tester info
 */
bool item_tester_okay(const object_type *o_ptr)
{
	/* Hack -- allow listing empty slots */
	if (item_tester_full) return (TRUE);

	/* Require an item */
	if (!o_ptr->k_idx) return (FALSE);

	/* Hack -- ignore "gold" */
	if (o_ptr->tval == TV_GOLD) return (FALSE);

	/* Check the tval */
	if (item_tester_tval)
	{
		if (!(item_tester_tval == o_ptr->tval)) return (FALSE);
	}

	/* Check the hook */
	if (item_tester_hook)
	{
		if (!(*item_tester_hook)(o_ptr)) return (FALSE);
	}

	/* Assume okay */
	return (TRUE);
}



/*
 * Get the indexes of objects at a given floor location.
 *
 * Return the number of object indexes acquired.
 *
 * Never acquire more than "size" object indexes, and never return a
 * number bigger than "size", even if more floor objects exist.
 *
 * Valid flags are any combination of the bits:
 *
 *   0x01 -- Verify item tester
 *   0x02 -- Marked items only
 */
sint scan_floor(int *items, int size, int y, int x, int mode)
{
	int this_o_idx, next_o_idx;

	int num = 0;

	/* Sanity */
	if (!in_bounds(y, x)) return (0);

	/* Scan all objects in the grid */
	for (this_o_idx = cave_o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		/* Get the object */
		o_ptr = &o_list[this_o_idx];

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Verify item tester */
		if ((mode & 0x01) && !item_tester_okay(o_ptr)) continue;

		/* Marked items only */
		if ((mode & 0x02) && !o_ptr->marked) continue;

		/* Accept this item */
		items[num++] = this_o_idx;

		/* Enforce size limit */
		if (num >= size) break;
	}

	/* Result */
	return (num);
}


/*
 * Get a random object which is associated with the feature.
 *
 * Return the object index.
 *
 * If no object found, return -1.
 *
 */
sint scan_feat(int y, int x)
{
	int this_o_idx, next_o_idx;

	int num = 0;

	int item = -1;

	/* Sanity */
	if (!in_bounds(y, x)) return (-1);

	/* Scan all objects in the grid */
	for (this_o_idx = cave_o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		/* Get the object */
		o_ptr = &o_list[this_o_idx];

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Marked items only */
		if (!(o_ptr->ident & (IDENT_STORE))) continue;

		/* Enforce size limit */
		if (rand_int(++num)) continue;

		item = this_o_idx;
	}

	/* Result */
	return (item);
}


/*
 * Choice window "shadow" of the "show_inven()" function
 */
void display_inven(void)
{
	register int i, n, z = 0;

	object_type *o_ptr;

	byte attr;

	char tmp_val[80];

	char o_name[80];


	/* Find the "final" slot */
	for (i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Track */
		z = i + 1;
	}

	/* Display the pack */
	for (i = 0; i < z; i++)
	{
		/* Examine the item */
		o_ptr = &inventory[i];

		/* Start with an empty "index" */
		tmp_val[0] = tmp_val[1] = tmp_val[2] = ' ';

		/* Is this item "acceptable"? */
		if (item_tester_okay(o_ptr))
		{
			/* Prepare an "index" */
			tmp_val[0] = index_to_label(i);

			/* Bracket the "index" --(-- */
			tmp_val[1] = ')';
		}

		/* Display the index (or blank space) */
		Term_putstr(0, i, 3, TERM_WHITE, tmp_val);

		/* Obtain an item description */
		object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

		/* Obtain the length of the description */
		n = strlen(o_name);

		/* Get inventory color */
		attr = tval_to_attr[o_ptr->tval & 0x7F];

		/* Display the entry itself */
		Term_putstr(3, i, n, attr, o_name);

		/* Erase the rest of the line */
		Term_erase(3+n, i, 255);

		/* Display the weight if needed */
		if (show_weights && o_ptr->weight)
		{
			int wgt = o_ptr->weight * o_ptr->number;
			sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			Term_putstr(71, i, -1, TERM_WHITE, tmp_val);
		}
	}

	/* Erase the rest of the window */
	for (i = z; i < Term->hgt; i++)
	{
		/* Erase the line */
		Term_erase(0, i, 255);
	}
}



/*
 * Choice window "shadow" of the "show_equip()" function
 */
void display_equip(void)
{
	register int i, n;
	object_type *o_ptr;
	byte attr;

	char tmp_val[80];

	char o_name[80];

	/* Display the equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		/* Examine the item */
		o_ptr = &inventory[i];

		/* Start with an empty "index" */
		tmp_val[0] = tmp_val[1] = tmp_val[2] = ' ';

		/* Is this item "acceptable"? */
		if (item_tester_okay(o_ptr))
		{
			/* Prepare an "index" */
			tmp_val[0] = index_to_label(i);

			/* Bracket the "index" --(-- */
			tmp_val[1] = ')';
		}

		/* Display the index (or blank space) */
		Term_putstr(0, i - INVEN_WIELD, 3, TERM_WHITE, tmp_val);

		/* Obtain an item description */
		object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

		/* Obtain the length of the description */
		n = strlen(o_name);

		/* Get inventory color */
		attr = tval_to_attr[o_ptr->tval & 0x7F];

		/* Display the entry itself */
		Term_putstr(3, i - INVEN_WIELD, n, attr, o_name);

		/* Erase the rest of the line */
		Term_erase(3+n, i - INVEN_WIELD, 255);

		/* Display the slot description (if needed) */
		if (show_labels)
		{
			Term_putstr(61, i - INVEN_WIELD, -1, TERM_WHITE, "<--");
			Term_putstr(65, i - INVEN_WIELD, -1, TERM_WHITE, mention_use(i));
		}

		/* Display the weight (if needed) */
		if (show_weights && o_ptr->weight)
		{
			int wgt = o_ptr->weight * o_ptr->number;
			int col = (show_labels ? 52 : 71);
			sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			Term_putstr(col, i - INVEN_WIELD, -1, TERM_WHITE, tmp_val);
		}
	}

	/* Erase the rest of the window */
	for (i = INVEN_TOTAL - INVEN_WIELD; i < Term->hgt; i++)
	{
		/* Clear that line */
		Term_erase(0, i, 255);
	}
}



/*
 * Display the inventory.
 *
 * Hack -- do not display "trailing" empty slots
 */
void show_inven(void)
{
	int i, j, k, l, z = 0;
	int col, len, lim;

	object_type *o_ptr;

	char o_name[80];

	char tmp_val[80];

	int out_index[24];
	byte out_color[24];
	char out_desc[24][80];

	/* Default length */
	len = 79 - 50;

	/* Maximum space allowed for descriptions */
	lim = 79 - 3;

	/* Require space for weight (if needed) */
	if (show_weights) lim -= 9;


	/* Find the "final" slot */
	for (i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Track */
		z = i + 1;
	}

	/* Display the inventory */
	for (k = 0, i = 0; i < z; i++)
	{
		o_ptr = &inventory[i];

		/* Is this item acceptable? */
		if (!item_tester_okay(o_ptr)) continue;

		/* Describe the object */
		object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

		/* Hack -- enforce max length */
		o_name[lim] = '\0';

		/* Save the index */
		out_index[k] = i;

		/* Get inventory color */
		out_color[k] = tval_to_attr[o_ptr->tval & 0x7F];

		/* Save the object description */
		strcpy(out_desc[k], o_name);

		/* Find the predicted "line length" */
		l = strlen(out_desc[k]) + 5;

		/* Be sure to account for the weight */
		if (show_weights) l += 9;

		/* Maintain the maximum length */
		if (l > len) len = l;

		/* Advance to next "line" */
		k++;
	}

	/* Find the column to start in */
	col = (len > 76) ? 0 : (79 - len);

	/* Output each entry */
	for (j = 0; j < k; j++)
	{
		/* Get the index */
		i = out_index[j];

		/* Get the item */
		o_ptr = &inventory[i];

		/* Clear the line */
		prt("", j + 1, col ? col - 2 : col);

		/* Prepare an index --(-- */
		sprintf(tmp_val, "%c)", index_to_label(i));

		/* Clear the line with the (possibly indented) index */
		put_str(tmp_val, j + 1, col);

		/* Display the entry itself */
		c_put_str(out_color[j], out_desc[j], j + 1, col + 3);

		/* Display the weight if needed */
		if (show_weights)
		{
			int wgt = o_ptr->weight * o_ptr->number;
			sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			put_str(tmp_val, j + 1, 71);
		}
	}

	/* Make a "shadow" below the list (only if needed) */
	if (j && (j < 23)) prt("", j + 1, col ? col - 2 : col);
}


/*
 * Display the equipment.
 */
void show_equip(void)
{
	int i, j, k, l;
	int col, len, lim;

	object_type *o_ptr;

	char tmp_val[80];

	char o_name[80];

	int out_index[24];
	byte out_color[24];
	char out_desc[24][80];

	/* Default length */
	len = 79 - 50;

	/* Maximum space allowed for descriptions */
	lim = 79 - 3;

	/* Require space for labels (if needed) */
	if (show_labels) lim -= (14 + 2);

	/* Require space for weight (if needed) */
	if (show_weights) lim -= 9;

	/* Scan the equipment list */
	for (k = 0, i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		o_ptr = &inventory[i];

		/* Is this item acceptable? */
		if (!item_tester_okay(o_ptr)) continue;

		/* Description */
		object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

		/* Truncate the description */
		o_name[lim] = 0;

		/* Save the index */
		out_index[k] = i;

		/* Get inventory color */
		out_color[k] = tval_to_attr[o_ptr->tval & 0x7F];

		/* Save the description */
		strcpy(out_desc[k], o_name);

		/* Extract the maximal length (see below) */
		l = strlen(out_desc[k]) + (2 + 3);

		/* Increase length for labels (if needed) */
		if (show_labels) l += (14 + 2);

		/* Increase length for weight (if needed) */
		if (show_weights) l += 9;

		/* Maintain the max-length */
		if (l > len) len = l;

		/* Advance the entry */
		k++;
	}

	/* Hack -- Find a column to start in */
	col = (len > 76) ? 0 : (79 - len);

	/* Output each entry */
	for (j = 0; j < k; j++)
	{
		/* Get the index */
		i = out_index[j];

		/* Get the item */
		o_ptr = &inventory[i];

		/* Clear the line */
		prt("", j + 1, col ? col - 2 : col);

		/* Prepare an index --(-- */
		sprintf(tmp_val, "%c)", index_to_label(i));

		/* Clear the line with the (possibly indented) index */
		put_str(tmp_val, j+1, col);

		/* Use labels */
		if (show_labels)
		{
			/* Mention the use */
			sprintf(tmp_val, "%-14s: ", mention_use(i));
			put_str(tmp_val, j+1, col + 3);

			/* Display the entry itself */
			c_put_str(out_color[j], out_desc[j], j+1, col + 3 + 14 + 2);
		}

		/* No labels */
		else
		{
			/* Display the entry itself */
			c_put_str(out_color[j], out_desc[j], j+1, col + 3);
		}

		/* Display the weight if needed */
		if (show_weights)
		{
			int wgt = o_ptr->weight * o_ptr->number;
			sprintf(tmp_val, "%3d.%d lb", wgt / 10, wgt % 10);
			put_str(tmp_val, j+1, 71);
		}
	}

	/* Make a "shadow" below the list (only if needed) */
	if (j && (j < 23)) prt("", j + 1, col ? col - 2 : col);
}


#ifdef ALLOW_EASY_FLOOR

/*
 * Display a list of the items on the floor at the given location.
 */
void show_floor(const int *floor_list, int floor_num)
{
	int i, j, k, l;
	int col, len, lim;

	object_type *o_ptr;

	char o_name[80];

	char tmp_val[80];

	int out_index[MAX_FLOOR_STACK];
	byte out_color[MAX_FLOOR_STACK];
	char out_desc[MAX_FLOOR_STACK][80];


	/* Default length */
	len = 79 - 50;

	/* Maximum space allowed for descriptions */
	lim = 79 - 3;

	/* Require space for weight (if needed) */
	if (show_weights) lim -= 9;

	/* Display the inventory */
	for (k = 0, i = 0; i < floor_num; i++)
	{
		o_ptr = &o_list[floor_list[i]];

		/* Is this item acceptable? */
		if (!item_tester_okay(o_ptr)) continue;

		/* Describe the object */
		object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

		/* Hack -- enforce max length */
		o_name[lim] = '\0';

		/* Save the index */
		out_index[k] = i;

		/* Get inventory color */
		out_color[k] = tval_to_attr[o_ptr->tval % N_ELEMENTS(tval_to_attr)];

		/* Save the object description */
		my_strcpy(out_desc[k], o_name, sizeof(out_desc[0]));

		/* Find the predicted "line length" */
		l = strlen(out_desc[k]) + 5;

		/* Be sure to account for the weight */
		if (show_weights) l += 9;

		/* Maintain the maximum length */
		if (l > len) len = l;

		/* Advance to next "line" */
		k++;
	}

	/* Find the column to start in */
	col = (len > 76) ? 0 : (79 - len);

	/* Output each entry */
	for (j = 0; j < k; j++)
	{
		/* Get the index */
		i = floor_list[out_index[j]];

		/* Get the item */
		o_ptr = &o_list[i];

		/* Clear the line */
		prt("", j + 1, col ? col - 2 : col);

		/* Prepare an index --(-- */
		sprintf(tmp_val, "%c)", index_to_label(out_index[j]));

		/* Clear the line with the (possibly indented) index */
		put_str(tmp_val, j + 1, col);

		/* Display the entry itself */
		c_put_str(out_color[j], out_desc[j], j + 1, col + 3);

		/* Display the weight if needed */
		if (show_weights)
		{
			int wgt = o_ptr->weight * o_ptr->number;
			sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			put_str(tmp_val, j + 1, 71);
		}
	}

	/* Make a "shadow" below the list (only if needed) */
	if (j && (j < 23)) prt("", j + 1, col ? col - 2 : col);
}

#endif /* ALLOW_EASY_FLOOR */


/*
 * Flip "inven" and "equip" in any sub-windows
 */
void toggle_inven_equip(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < 8; j++)
	{
		/* Unused */
		if (!angband_term[j]) continue;

		/* Flip inven to equip */
		if (op_ptr->window_flag[j] & (PW_INVEN))
		{
			/* Flip flags */
			op_ptr->window_flag[j] &= ~(PW_INVEN);
			op_ptr->window_flag[j] |= (PW_EQUIP);

			/* Window stuff */
			p_ptr->window |= (PW_EQUIP);
		}

		/* Flip inven to equip */
		else if (op_ptr->window_flag[j] & (PW_EQUIP))
		{
			/* Flip flags */
			op_ptr->window_flag[j] &= ~(PW_EQUIP);
			op_ptr->window_flag[j] |= (PW_INVEN);

			/* Window stuff */
			p_ptr->window |= (PW_INVEN);
		}
	}
}




/*
 * Verify the choice of an item.
 *
 * The item can be negative to mean "item on floor".
 */
static bool verify_item(cptr prompt, int item)
{
	char o_name[80];

	char out_val[160];

	object_type *o_ptr;

	/* Inventory */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Floor */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Describe */
	object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

	/* Prompt */
	sprintf(out_val, "%s %s? ", prompt, o_name);

	/* Query */
	return (get_check(out_val));
}


/*
 * Hack -- allow user to "prevent" certain choices.
 *
 * Now can use '<n' to confirm if stack is less than size 'n' or '>n' if stack is more than size 'n'.
 *
 * The item can be negative to mean "item on floor".
 */
static bool get_item_allow(int item)
{
	cptr s;

	object_type *o_ptr;

	int n;

	/* Inventory */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Floor */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* No inscription */
	if (!o_ptr->note) return (TRUE);

	/* Find a '!' */
	s = strchr(quark_str(o_ptr->note), '!');

	/* Process preventions */
	while (s)
	{
		/* Check the "restriction" */
		if ((s[1] == p_ptr->command_cmd) || (s[1] == '*'))
		{
			/* Verify the choice */
			if (!verify_item("Really try", item)) return (FALSE);
		}

		/* Find another '!' */
		s = strchr(s + 1, '!');
	}

	/* Find a '<' */
	s = strchr(quark_str(o_ptr->note), '<');

	/* Process preventions */
	while (s)
	{

		/* Check the "restriction" */
		if ((s[1] >= '0') || (s[1] <= '9'))
		{
			n = atoi(s+1);

			/* Verify the choice */
			if ((o_ptr->number < n) && (!verify_item("Really try", item))) return (FALSE);
		}

		/* Check the "restriction" */
		else if ((s[1] == p_ptr->command_cmd) || (s[1] == '*'))
		{
			/* Check the "restriction" */
			if ((s[2] >= '0') || (s[2] <= '9'))
			{
				n = atoi(s+2);

				/* Verify the choice */
				if ((o_ptr->number < n) && (!verify_item("Really try", item))) return (FALSE);
			}
			
		}

		/* Find another '!' */
		s = strchr(s + 1, '<');
	}


	/* Find a '>' */
	s = strchr(quark_str(o_ptr->note), '>');

	/* Process preventions */
	while (s)
	{

		/* Check the "restriction" */
		if ((s[1] >= '0') || (s[1] <= '9'))
		{
			n = atoi(s+1);

			/* Verify the choice */
			if ((o_ptr->number > n) && (!verify_item("Really try", item))) return (FALSE);
		}

		/* Check the "restriction" */
		else if ((s[1] == p_ptr->command_cmd) || (s[1] == '*'))
		{
			/* Check the "restriction" */
			if ((s[2] >= '0') || (s[2] <= '9'))
			{
				n = atoi(s+2);

				/* Verify the choice */
				if ((o_ptr->number > n) && (!verify_item("Really try", item))) return (FALSE);
			}
			
		}

		/* Find another '!' */
		s = strchr(s + 1, '>');
	}

	/* Allow it */
	return (TRUE);
}


/*
 * Verify the "okayness" of a given item.
 *
 * The item can be negative to mean "item on floor".
 */
static bool get_item_okay(int item)
{
	object_type *o_ptr;

	/* Inventory */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Floor */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Verify the item */
	return (item_tester_okay(o_ptr));
}



/*
 * Find the "first" inventory object with the given "tag".
 *
 * A "tag" is a char "n" appearing as "@n" anywhere in the
 * inscription of an object.
 *
 * Also, the tag "@xn" will work as well, where "n" is a tag-char,
 * and "x" is the "current" p_ptr->command_cmd code.
 *
 */
static int get_tag(int *cp, char tag)
{
	int i;
	cptr s;

	/* Check every object */
	for (i = 0; i < INVEN_TOTAL; ++i)
	{
		object_type *o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Skip empty inscriptions */
		if (!o_ptr->note) continue;

		/* Find a '@' */
		s = strchr(quark_str(o_ptr->note), '@');

		/* Process all tags */
		while (s)
		{
			/* Check the normal tags */
			if (s[1] == tag)
			{
				/* Save the actual inventory ID */
				*cp = i;

				/* Success */
				return (TRUE);
			}

			/* Check the special tags */
			if ((s[1] == p_ptr->command_cmd) && (s[2] == tag))
			{
				/* Save the actual inventory ID */
				*cp = i;

				/* Success */
				return (TRUE);
			}

			/* Find another '@' */
			s = strchr(s + 1, '@');
		}
	}

	/* No such tag */
	return (FALSE);
}



/*
 * Let the user select an item, save its "index"
 *
 * Return TRUE only if an acceptable item was chosen by the user.
 *
 * The selected item must satisfy the "item_tester_hook()" function,
 * if that hook is set, and the "item_tester_tval", if that value is set.
 *
 * All "item_tester" restrictions are cleared before this function returns.
 *
 * The user is allowed to choose acceptable items from the equipment,
 * inventory, or floor, respectively, if the proper flag was given,
 * and there are any acceptable items in that location.
 *
 * The equipment or inventory are displayed (even if no acceptable
 * items are in that location) if the proper flag was given.
 *
 * If there are no acceptable items available anywhere, and "str" is
 * not NULL, then it will be used as the text of a warning message
 * before the function returns.
 *
 * Note that the user must press "-" to specify the item on the floor,
 * and there is no way to "examine" the item on the floor, while the
 * use of "capital" letters will "examine" an inventory/equipment item,
 * and prompt for its use.
 *
 * If a legal item is selected from the inventory, we save it in "cp"
 * directly (0 to 35), and return TRUE.
 *
 * If a legal item is selected from the floor, we save it in "cp" as
 * a negative (-1 to -511), and return TRUE.
 *
 * If no item is available, we do nothing to "cp", and we display a
 * warning message, using "str" if available, and return FALSE.
 *
 * If no item is selected, we do nothing to "cp", and return FALSE.
 *
 * Global "p_ptr->command_new" is used when viewing the inventory or equipment
 * to allow the user to enter a command while viewing those screens, and
 * also to induce "auto-enter" of stores, and other such stuff.
 *
 * Global "p_ptr->command_see" may be set before calling this function to start
 * out in "browse" mode.  It is cleared before this function returns.
 *
 * Global "p_ptr->command_wrk" is used to choose between equip/inven/floor
 * listings.  It is equal to USE_INVEN or USE_EQUIP or USE_FLOOR, except
 * when this function is first called, when it is equal to zero, which will
 * cause it to be set to USE_INVEN.
 *
 * We always erase the prompt when we are done, leaving a blank line,
 * or a warning message, if appropriate, if no items are available.
 *
 * Note that the "easy_floor" option affects this function in several ways.
 *
 * Note that only "acceptable" floor objects get indexes, so between two
 * commands, the indexes of floor objects may change.  XXX XXX XXX
 */
bool get_item(int *cp, cptr pmt, cptr str, int mode)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	char which;

	int i, j, k;

	int i1, i2;
	int e1, e2;
	int f1, f2;

	bool done, item;

	bool oops = FALSE;

	bool use_inven = ((mode & (USE_INVEN)) ? TRUE : FALSE);
	bool use_equip = ((mode & (USE_EQUIP)) ? TRUE : FALSE);
	bool use_floor = ((mode & (USE_FLOOR)) ? TRUE : FALSE);
	bool use_featg = ((mode & (USE_FEATG)) ? TRUE : FALSE);
	bool use_featu = ((mode & (USE_FEATU)) ? TRUE : FALSE);

	bool allow_inven = FALSE;
	bool allow_equip = FALSE;
	bool allow_floor = FALSE;
	bool allow_feats = FALSE;

	bool toggle = FALSE;

	char tmp_val[160];
	char out_val[160];

	int floor_list[MAX_FLOOR_STACK];
	int floor_num;

#ifdef ALLOW_REPEAT

	/* Get the item index */
	if (repeat_pull(cp))
	{
		/* Verify the item */
		if (get_item_okay(*cp))
		{
			/* Forget the item_tester_tval restriction */
			item_tester_tval = 0;

			/* Forget the item_tester_hook restriction */
			item_tester_hook = NULL;

			/* Success */
			return (TRUE);
		}
	}

#endif /* ALLOW_REPEAT */


	/* Paranoia XXX XXX XXX */
	msg_print(NULL);


	/* Not done */
	done = FALSE;

	/* No item selected */
	item = FALSE;


	/* Full inventory */
	i1 = 0;
	i2 = INVEN_PACK - 1;

	/* Forbid inventory */
	if (!use_inven) i2 = -1;

	/* Restrict inventory indexes */
	while ((i1 <= i2) && (!get_item_okay(i1))) i1++;
	while ((i1 <= i2) && (!get_item_okay(i2))) i2--;

	/* Accept inventory */
	if (i1 <= i2) allow_inven = TRUE;


	/* Full equipment */
	e1 = INVEN_WIELD;
	e2 = INVEN_TOTAL - 1;

	/* Forbid equipment */
	if (!use_equip) e2 = -1;

	/* Restrict equipment indexes */
	while ((e1 <= e2) && (!get_item_okay(e1))) e1++;
	while ((e1 <= e2) && (!get_item_okay(e2))) e2--;

	/* Accept equipment */
	if (e1 <= e2) allow_equip = TRUE;

	/* Scan all objects in the grid */
	floor_num = scan_floor(floor_list, MAX_FLOOR_STACK, py, px, 0x00);

	/* Full floor */
	f1 = 0;
	f2 = floor_num - 1;

	/* Forbid floor */
	if (!use_floor) f2 = -1;

	/* Restrict floor indexes */
	while ((f1 <= f2) && (!get_item_okay(0 - floor_list[f1]))) f1++;
	while ((f1 <= f2) && (!get_item_okay(0 - floor_list[f2]))) f2--;

	/* Accept floor */
	if (f1 <= f2) allow_floor = TRUE;

	/* Scan the feature */
	if ((use_featg) && (f_info[cave_feat[p_ptr->py][p_ptr->px]].flags3 & (FF3_GET_FEAT)))
	{
		object_type *i_ptr;
		object_type object_type_body;

		i_ptr = &object_type_body;

		if ((make_feat(i_ptr,p_ptr->py,p_ptr->px)) && item_tester_okay(i_ptr)) allow_feats = TRUE;
	}

	/* Rescan the feature */
	if ((use_featu) && (f_info[cave_feat[p_ptr->py][p_ptr->px]].flags3 & (FF3_USE_FEAT)))
	{
		object_type *i_ptr;
		object_type object_type_body;

		i_ptr = &object_type_body;

		if ((make_feat(i_ptr,p_ptr->py,p_ptr->px)) && item_tester_okay(i_ptr)) allow_feats = TRUE;
	}

	/* Require at least one legal choice */
	if (!allow_inven && !allow_equip && !allow_floor && !allow_feats)
	{
		/* Cancel p_ptr->command_see */
		p_ptr->command_see = FALSE;

		/* Oops */
		oops = TRUE;

		/* Done */
		done = TRUE;
	}

	/* Analyze choices */
	else
	{
		/* Hack -- Start on equipment if requested */
		if (p_ptr->command_see &&
		    (p_ptr->command_wrk == (USE_EQUIP)) &&
		    use_equip)
		{
			p_ptr->command_wrk = (USE_EQUIP);
		}

		/* Use inventory if allowed */
		else if (use_inven)
		{
			p_ptr->command_wrk = (USE_INVEN);
		}

		/* Use equipment if allowed */
		else if (use_equip)
		{
			p_ptr->command_wrk = (USE_EQUIP);
		}

#ifdef ALLOW_EASY_FLOOR

		/* Use floor if allowed */
		else if (easy_floor)
		{
			p_ptr->command_wrk = (USE_FLOOR);
		}

#endif /* ALLOW_EASY_FLOOR */

		/* Hack -- Use (empty) inventory */
		else
		{
			p_ptr->command_wrk = (USE_INVEN);
		}
	}

	/* Look up the tag */
	if (get_tag(&k, '*'))
	{
		/*Hack -- Validate the item */
		if ((k < INVEN_WIELD) ? !allow_inven : !allow_equip)
		{
			bell("Illegal object choice (tag)!");
		}

		/* Validate the item */
		else if (!get_item_okay(k))
		{
			bell("Illegal object choice (tag)!");
		}

		/* Allow player to "refuse" certain actions */
		else if (!get_item_allow(k))
		{
			/* Nothing */
		}

		else
		{
			/* Accept that choice */
			(*cp) = k;
			item = TRUE;
			done = TRUE;
			p_ptr->command_see = FALSE;
		}
	}

	/* Start out in "display" mode */
	if (p_ptr->command_see)
	{
		/* Save screen */
		screen_save();
	}

	/* Repeat until done */
	while (!done)
	{
		/* Show choices */
		if (show_choices)
		{
			int ni = 0;
			int ne = 0;

			/* Scan windows */
			for (j = 0; j < 8; j++)
			{
				/* Unused */
				if (!angband_term[j]) continue;

				/* Count windows displaying inven */
				if (op_ptr->window_flag[j] & (PW_INVEN)) ni++;

				/* Count windows displaying equip */
				if (op_ptr->window_flag[j] & (PW_EQUIP)) ne++;
			}

			/* Toggle if needed */
			if (((p_ptr->command_wrk == (USE_EQUIP)) && ni && !ne) ||
			    ((p_ptr->command_wrk == (USE_INVEN)) && !ni && ne))
			{
				/* Toggle */
				toggle_inven_equip();

				/* Track toggles */
				toggle = !toggle;
			}

			/* Update */
			p_ptr->window |= (PW_INVEN | PW_EQUIP);

			/* Redraw windows */
			window_stuff();
		}

		/* Viewing inventory */
		if (p_ptr->command_wrk == (USE_INVEN))
		{
			/* Redraw if needed */
			if (p_ptr->command_see) show_inven();

			/* Begin the prompt */
			sprintf(out_val, "Inven:");

			/* List choices */
			if (i1 <= i2)
			{
				/* Build the prompt */
				sprintf(tmp_val, " %c-%c,",
					index_to_label(i1), index_to_label(i2));

				/* Append */
				strcat(out_val, tmp_val);
			}

			/* Indicate ability to "view" */
			if (!p_ptr->command_see) strcat(out_val, " * to see,");

			/* Indicate legality of "toggle" */
			if (use_equip) strcat(out_val, " / for Equip,");

			/* Indicate legality of the "floor" */
			if (allow_floor) strcat(out_val, " - for floor,");

			/* Indicate legality of the "feature" */
			if (allow_feats)
			{
				strcat(out_val, " . for ");
				strcat(out_val,f_name + f_info[cave_feat[p_ptr->py][p_ptr->px]].name);
				strcat(out_val,",");
			}

		}

		/* Viewing equipment */
		else if (p_ptr->command_wrk == (USE_EQUIP))
		{
			/* Redraw if needed */
			if (p_ptr->command_see) show_equip();

			/* Begin the prompt */
			sprintf(out_val, "Equip:");

			/* List choices */
			if (e1 <= e2)
			{
				/* Build the prompt */
				sprintf(tmp_val, " %c-%c,",
					index_to_label(e1), index_to_label(e2));

				/* Append */
				strcat(out_val, tmp_val);
			}

			/* Indicate ability to "view" */
			if (!p_ptr->command_see) strcat(out_val, " * to see,");

			/* Indicate legality of "toggle" */
			if (use_inven) strcat(out_val, " / for Inven,");

			/* Indicate legality of the "floor" */
			if (allow_floor) strcat(out_val, " - for floor,");

			/* Indicate legality of the "feature" */
			if (allow_feats)
			{
				strcat(out_val, " . for ");
				strcat(out_val,f_name + f_info[cave_feat[p_ptr->py][p_ptr->px]].name);
				strcat(out_val,",");
			}
		}

#ifdef ALLOW_EASY_FLOOR

		/* Viewing floor */
		else
		{
			/* Redraw if needed */
			if (p_ptr->command_see) show_floor(floor_list, floor_num);

			/* Begin the prompt */
			sprintf(out_val, "Floor:");

			/* List choices */
			if (f1 <= f2)
			{
				/* Build the prompt */
				sprintf(tmp_val, " %c-%c,", I2A(f1), I2A(f2));

				/* Append */
				strcat(out_val, tmp_val);
			}

			/* Indicate ability to "view" */
			if (!p_ptr->command_see) strcat(out_val, " * to see,");

			/* Append */
			if (use_inven) strcat(out_val, " / for Inven,");

			/* Append */
			else if (use_equip) strcat(out_val, " / for Equip,");

			/* Indicate legality of the "feature" */
			if (allow_feats)
			{
				strcat(out_val, " . for ");
				strcat(out_val,f_name + f_info[cave_feat[p_ptr->py][p_ptr->px]].name);
				strcat(out_val,",");
			}
		}

#endif /* ALLOW_EASY_FLOOR */

		/* Finish the prompt */
		strcat(out_val, " ESC");

		/* Build the prompt */
		sprintf(tmp_val, "(%s) %s", out_val, pmt);

		/* Show the prompt */
		prt(tmp_val, 0, 0);


		/* Get a key */
		which = inkey();

		/* Parse it */
		switch (which)
		{
			case ESCAPE:
			{
				done = TRUE;
				break;
			}

			case '*':
			case '?':
			case ' ':
			{
				/* Hide the list */
				if (p_ptr->command_see)
				{
					/* Flip flag */
					p_ptr->command_see = FALSE;

					/* Load screen */
					screen_load();
				}

				/* Show the list */
				else
				{
					/* Save screen */
					screen_save();

					/* Flip flag */
					p_ptr->command_see = TRUE;
				}

				break;
			}

			case '/':
			{
				/* Toggle to inventory */
				if (use_inven && (p_ptr->command_wrk != (USE_INVEN)))
				{
					p_ptr->command_wrk = (USE_INVEN);
				}

				/* Toggle to equipment */
				else if (use_equip && (p_ptr->command_wrk != (USE_EQUIP)))
				{
					p_ptr->command_wrk = (USE_EQUIP);
				}

				/* No toggle allowed */
				else
				{
					bell("Cannot switch item selector!");
					break;
				}

				/* Hack -- Fix screen */
				if (p_ptr->command_see)
				{
					/* Load screen */
					screen_load();

					/* Save screen */
					screen_save();
				}

				/* Need to redraw */
				break;
			}

			case '-':
			{
				/* Paranoia */
				if (!allow_floor)
				{
					bell("Cannot select floor!");
					break;
				}

#ifdef ALLOW_EASY_FLOOR

				if (easy_floor)
				{
					/* There is only one item */
					if (floor_num == 1)
					{
						/* Hack -- Auto-Select */
						if ((p_ptr->command_wrk == (USE_FLOOR)) ||
						    (!floor_query_flag))
						{
							/* Special index */
							k = 0 - floor_list[0];

							/* Allow player to "refuse" certain actions */
							if (!get_item_allow(k))
							{
								done = TRUE;
								break;
							}

							/* Accept that choice */
							(*cp) = k;
							item = TRUE;
							done = TRUE;

							break;
						}
					}

					/* Hack -- Fix screen */
					if (p_ptr->command_see)
					{
						/* Load screen */
						screen_load();

						/* Save screen */
						screen_save();
					}

					p_ptr->command_wrk = (USE_FLOOR);

					break;
				}

#endif /* ALLOW_EASY_FLOOR */

				/* Check each legal object */
				for (i = 0; i < floor_num; ++i)
				{
					/* Special index */
					k = 0 - floor_list[i];

					/* Skip non-okay objects */
					if (!get_item_okay(k)) continue;

					/* Verify the item (if required) */
					if (floor_query_flag && !verify_item("Try", k)) continue;

					/* Allow player to "refuse" certain actions */
					if (!get_item_allow(k)) continue;

					/* Accept that choice */
					(*cp) = k;
					item = TRUE;
					done = TRUE;
					break;
				}

				break;
			}

			case '0':
			case '1': case '2': case '3':
			case '4': case '5': case '6':
			case '7': case '8': case '9':
			{
				/* Look up the tag */
				if (!get_tag(&k, which))
				{
					bell("Illegal object choice (tag)!");
					break;
				}

				/* Hack -- Validate the item */
				if ((k < INVEN_WIELD) ? !allow_inven : !allow_equip)
				{
					bell("Illegal object choice (tag)!");
					break;
				}

				/* Validate the item */
				if (!get_item_okay(k))
				{
					bell("Illegal object choice (tag)!");
					break;
				}

				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(k))
				{
					done = TRUE;
					break;
				}

				/* Accept that choice */
				(*cp) = k;
				item = TRUE;
				done = TRUE;
				break;
			}

			case '\n':
			case '\r':
			{
				/* Choose "default" inventory item */
				if (p_ptr->command_wrk == (USE_INVEN))
				{
					if (i1 != i2)
					{
						bell("Illegal object choice (default)!");
						break;
					}

					k = i1;
				}

				/* Choose "default" equipment item */
				else if (p_ptr->command_wrk == (USE_EQUIP))
				{
					if (e1 != e2)
					{
						bell("Illegal object choice (default)!");
						break;
					}

					k = e1;
				}

#ifdef ALLOW_EASY_FLOOR

				/* Choose "default" floor item */
				else
				{
					if (f1 != f2)
					{
						bell("Illegal object choice (default)!");
						break;
					}

					k = 0 - floor_list[f1];
				}

#endif /* ALLOW_EASY_FLOOR */

				/* Validate the item */
				if (!get_item_okay(k))
				{
					bell("Illegal object choice (default)!");
					break;
				}

				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(k))
				{
					done = TRUE;
					break;
				}

				/* Accept that choice */
				(*cp) = k;
				item = TRUE;
				done = TRUE;
				break;
			}

			case '.':

				/* Paranoia */
				if (!allow_feats)
				{
					bell("Cannot select feature!");
					break;
				}

				/* Hack -- pick previous feature - don't validate or allow */
				k = scan_feat(p_ptr->py,p_ptr->px);

				/* Paranoia */
				if (k<0)
				{
					bell("Too many objects for feature!");
					break;
				}

				/* Accept that choice */
				(*cp) = 0 - k;
				item = TRUE;
				done = TRUE;

				break;

			default:
			{
				bool verify;

				/* Note verify */
				verify = (isupper(which) ? TRUE : FALSE);

				/* Lowercase */
				which = tolower(which);

				/* Convert letter to inventory index */
				if (p_ptr->command_wrk == (USE_INVEN))
				{
					k = label_to_inven(which);

					if (k < 0)
					{
						bell("Illegal object choice (inven)!");
						break;
					}
				}

				/* Convert letter to equipment index */
				else if (p_ptr->command_wrk == (USE_EQUIP))
				{
					k = label_to_equip(which);

					if (k < 0)
					{
						bell("Illegal object choice (equip)!");
						break;
					}
				}

#ifdef ALLOW_EASY_FLOOR

				/* Convert letter to floor index */
				else
				{
					k = (islower(which) ? A2I(which) : -1);

					if (k < 0 || k >= floor_num)
					{
						bell("Illegal object choice (floor)!");
						break;
					}

					/* Special index */
					k = 0 - floor_list[k];
				}

#endif /* ALLOW_EASY_FLOOR */

				/* Validate the item */
				if (!get_item_okay(k))
				{
					bell("Illegal object choice (normal)!");
					break;
				}

				/* Verify the item */
				if (verify && !verify_item("Try", k))
				{
					done = TRUE;
					break;
				}

				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(k))
				{
					done = TRUE;
					break;
				}

				/* Accept that choice */
				(*cp) = k;
				item = TRUE;
				done = TRUE;
				break;
			}
		}
	}


	/* Fix the screen if necessary */
	if (p_ptr->command_see)
	{
		/* Load screen */
		screen_load();

		/* Hack -- Cancel "display" */
		p_ptr->command_see = FALSE;
	}


	/* Forget the item_tester_tval restriction */
	item_tester_tval = 0;

	/* Forget the item_tester_hook restriction */
	item_tester_hook = NULL;


	/* Clean up */
	if (show_choices)
	{
		/* Toggle again if needed */
		if (toggle) toggle_inven_equip();

		/* Update */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);

		/* Window stuff */
		window_stuff();
	}


	/* Clear the prompt line */
	prt("", 0, 0);

	/* Warning if needed */
	if (oops && str) msg_print(str);

#ifdef ALLOW_REPEAT

	/* Save item if available */
	if (item) repeat_push(*cp);

#endif /* ALLOW_REPEAT */

	/* Result */
	return (item);
}


