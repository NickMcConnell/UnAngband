#!/usr/bin/perl -w
use strict;

use Data::Dumper;
use CGI::Pretty qw/:standard *ul/;
use Carp qw(longmess);

###############################################################################
# Dumps context of txt files into slightly more readable html, includes
# href links, etc.
#
# Usage: Currently, as it's rather a quick hack, it needs to be copied/linked
# into the root "unangband" directory (the one that has a directory "lib" in
# it) then run. It will produce a number of .html files in the current
# directory.
###############################################################################

##########
# Assorted useful functions/globals
##########

# debugging to dump out perl arrays if true
my $perldump = 0;

# generic filename array
my %filenames =
(
	dungeon => {
		txt => "dungeon.txt",
		html => "dungeon.html",
		debug => "dungeon.pl.txt",
		desc => "Dungeons",
	},
	store => {
		txt => "store.txt",
		html => "store.html",
		debug => "store.pl.txt",
		desc => "Stores",
	},
	monster => {
		txt => "monster.txt",
		html => "monster.html",
		debug => "monster.pl.txt",
		desc => "Monsters",
	},
	terrain => {
		txt => "terrain.txt",
		html => "terrain.html",
		debug => "terrain.pl.txt",
		desc => "Terrain",
	},
	vault => {
		txt => "vault.txt",
		html => "vault.html",
		debug => "vault.pl.txt",
		desc => "Vaults",
	},
	object => {
		txt => "object.txt",
		html => "object.html",
		debug => "object.pl.txt",
		desc => "Objects",
	},
	shop_owner => {
		txt => "shop_own.txt",
		html => "shop_owner.html",
		debug => "shop_owner.pl.txt",
		desc => "Shop Owners",
	},
	room => {
		txt => "room.txt",
		html => "room.html",
		debug => "room.pl.txt",
		desc => "Rooms",
	},
	blow => {
		txt => "blows.txt",
		html => "blow.html",
		debug => "blow.pl.txt",
		desc => "Blows",
	},
	effect => {
		txt => "effect.txt",
		html => "effect.html",
		debug => "effect.pl.txt",
		desc => "Effects",
	},
	region => {
		txt => "region.txt",
		html => "region.html",
		debug => "region.pl.txt",
		desc => "Regions",
	},
	spell => {
		txt => "spell.txt",
		html => "spell.html",
		debug => "spell.pl.txt",
		desc => "Spells",
	},
	c => {
		defines => "defines.h",
	},
	defines_TV => {
		html => "defines.html",
	},
);

# a few useful globals for parsing
my $fmid = "([^:]*?)";    # standard param
my $fopt = "(?:$fmid:)*"; # optional param
my $fend = "(.*)";        # last param
my $foptend = "(?:(.+))*"; # last param with optional ending; personally I think this is a bug in the code

my $params1 = "$fend";
my $params2 = "$fmid:$fend";
my $params3 = "$fmid:$fmid:$fend";
my $params4 = "$fmid:$fmid:$fmid:$fend";
my $params5 = "$fmid:$fmid:$fmid:$fmid:$fend";
my $params6 = "$fmid:$fmid:$fmid:$fmid:$fmid:$fend";
my $params7 = "$fmid:$fmid:$fmid:$fmid:$fmid:$fmid:$fend";
my $params8 = "$fmid:$fmid:$fmid:$fmid:$fmid:$fmid:$fmid:$fend";

# some sane styles
my $styles = <<END;
<style type="text/css">
	body { font-family: MS Gothic, Courier New, Verdana; }
	table {
		border-width: 1px;
		//border-spacing: ;
		border-style: outset;
		border-color: gray;
		border-collapse: separate;
		background-color: white;
	}
	table.sample th {
		border-width: 1px;
		padding: 1px;
		border-style: none;
		border-color: gray;
		background-color: white;
	}
	table.sample td {
		border-width: 1px;
		padding: 1px;
		border-style: none;
		border-color: gray;
		background-color: white;
	}
</style>
<!--
********** WARNING *********
Generated via txt2html.pl; do not edit
********** WARNING *********
-->
END

# nice little trim function
sub trim {
    return trim2( $_ ) if not @_;
    return map { local $_ = $_; s/^\s+//, s/\s+$//; $_ } @_
        if defined wantarray;
    for ( @_ ) { s/^\s+//, s/\s+$// }
}

# useful shortcuts for html output
sub id_link {
	my $target = shift;
	my $base = shift;

	$base = "" if(not defined $base);

	return a({href=>"$base#$target"}, ">>$target");
}

sub name_link {
	my $target = shift;
	my $name = shift;
	my $base = shift;

	$base = "" if(not defined $base);

	print longmess() if not defined $name;
	
	return a({href=>"$base#$target"}, "$name ($target)");
}

sub terrain_link($\@) {
	my $idx = shift;
	my @terrains = @{ (shift) };
	
	return name_link($idx, $terrains[$idx]->{name}, $filenames{terrain}{html});
}

sub monster_link($\@) {
	my $idx = shift;
	my @monsters = @{ (shift) };
	
	return name_link($idx, $monsters[$idx]->{name}, $filenames{monster}{html});
}

sub vault_link($\@) {
	my $idx = shift;
	my @vaults = @{ (shift) };
	
	return name_link($idx, $vaults[$idx]->{name}, $filenames{vault}{html});
}

sub store_link($\@) {
	my $idx = shift;
	my @stores = @{ (shift) };
	
	return name_link($idx, $stores[$idx]->{name}, $filenames{store}{html});
}

sub blow_map_link($\%) {
	my $name = shift;
	my %blows_map = %{ (shift) };
	
	return name_link($blows_map{$name}->{idx}, $name, $filenames{blow}{html});
}

sub effect_map_link($\%) {
	my $name = shift;
	my %effects_map = %{ (shift) };
	
	#print "Eh?: $name\n" if not defined $effects_map{$name}->{idx};
	return name_link($effects_map{$name}->{idx}, $name, $filenames{effect}{html});
}

# some useful page layout helper funcs
sub generic_header($) {
	my $file = shift;

	return start_html(-title=>$filenames{$file}{desc}),
		h1($filenames{$file}{desc}),
		$styles,
	;
}

sub generic_links(\@) {
	my @collection = @{ (shift) };
	my @return;

	# link collection
	foreach my $item ( @collection ) {
		# array is sparse
		next if not $item;
	
		push @return, id_link($item->{idx})." ";
	}
	return @return;
}

sub generic_footer() {
	return end_html;
}

# and a quick debugging func
sub dump_perl($\@) {
	my $file = shift;
	my @collection = @{ (shift) };

	return if not $perldump;

	open FILETXT, ">$filenames{$file}{debug}";
	print FILETXT Dumper(\@collection);
	close FILETXT;
}

sub generic_item_header($) {
	my $item = shift;

	return a({name=>"$item->{idx}"}, h3("$item->{name} ($item->{idx})"));
}

##########
# Dungeons!
##########

sub parse_dungeons($) {
	my $filein = shift;
	
	open FILEIN, $filein or die "cannot open $filein: $!\n";

	my @dungeons;
	my $idx = -1;
	my $num_zones = 0;
	
	foreach(<FILEIN>) {
		chomp;
		if(/^#/) { }       #discard
		elsif(/^\s*$/) { } #discard
		elsif(/^V:/) { }   #discard
		elsif(/^N:$params2$/) {
			$idx = $1;
			$num_zones = 0;
			$dungeons[$idx]{idx} = $1;
			$dungeons[$idx]{name} = $2;
		}
		elsif(/^G:$params1$/) {
			$dungeons[$idx]{r_char} = $1;
		}
		elsif(/^R:$params1$/) {
			$dungeons[$idx]{r_flag} = $1;
		}
		elsif(/^X:$params4$/) {
			$dungeons[$idx]{nearby}[0] = $1;
			$dungeons[$idx]{nearby}[1] = $2;
			$dungeons[$idx]{nearby}[2] = $3;
			$dungeons[$idx]{nearby}[3] = $4;
		}
		elsif(/^Q:$params2$/) {
			$dungeons[$idx]{quest_opens} = $1;
			$dungeons[$idx]{quest_monster} = $2;
		}
		elsif(/^Y:$params2$/) {
			$dungeons[$idx]{town_lockup_monster} = $1;
			$dungeons[$idx]{town_lockup_ifvisited} = $2;
		}
		elsif(/^W:$params2$/) {
			$dungeons[$idx]{replace_guardian} = $1;
			$dungeons[$idx]{guardian_ifvisited} = $2;
		}
		elsif(/^Z:$params2$/) {
			$dungeons[$idx]{replace_with} = $1;
			$dungeons[$idx]{replace_ifvisited} = $2;
		}
		elsif(/^L:$fmid:$fmid:$fmid:$fmid:$fmid:(?:$fmid:)*$fend$/) {
			#print "$idx $1:$2:$3:$4:$5:$6:$7\n";
			$dungeons[$idx]{zones}[$num_zones]{level} = $1;
			$dungeons[$idx]{zones}[$num_zones]{fill} = $2;
			$dungeons[$idx]{zones}[$num_zones]{big} = $3;
			$dungeons[$idx]{zones}[$num_zones]{small} = $4;
			$dungeons[$idx]{zones}[$num_zones]{guard} = $5;
			if(defined $6) {
				$dungeons[$idx]{zones}[$num_zones]{tower} = $6;
				$dungeons[$idx]{zones}[$num_zones]{name} = $7;
			} else {
				$dungeons[$idx]{zones}[$num_zones]{tower} = $7;
			}
			$num_zones++;
		}
		elsif(/^S:$params8$/) {
			$dungeons[$idx]{store}[0] = $1;
			$dungeons[$idx]{store}[1] = $2;
			$dungeons[$idx]{store}[2] = $3;
			$dungeons[$idx]{store}[3] = $4;
			$dungeons[$idx]{store}[4] = $5;
			$dungeons[$idx]{store}[5] = $6;
			$dungeons[$idx]{store}[6] = $7;
			$dungeons[$idx]{store}[7] = $8;
		}
		elsif(/^D:$params1$/) {
			$dungeons[$idx]{text} .= $1
		}
		else {
			print "Huh?:".$_."\n";
		}
	}
	
	close(FILEIN);

	return @dungeons;
}

sub dump_dungeons_nearby($\@) {
	my $dungeon = shift;
	my @dungeons = @{ (shift) };
	my @string;
	
	foreach my $loc (@{ $dungeon->{nearby} }) {
		if(not $loc eq "0") {
			push @string, a({href=>"#$loc"}, "$dungeons[$loc]->{name} ($loc)")." ";
		}
	}
	
	return li("Nearby Locations: ", @string);
}

sub dump_dungeons_zones($\@\@\@\@) {
	my $dungeon  = shift;
	my @dungeons = @{ (shift) };
	my @monsters = @{ (shift) };
	my @terrains = @{ (shift) };
	my @vaults   = @{ (shift) };
	my @string   ;

	#FIXME: really need to make this make more sense, real english would be good
	foreach my $zone (@{ $dungeon->{zones} }) {
		push @string, li(
			"Zone ".strong($zone->{name} ? "$zone->{name} $dungeon->{name} " : "")
			."level ".strong($zone->{level})
			." filled with ".terrain_link($zone->{fill}, @terrains). ", big:"
			.terrain_link($zone->{big}, @terrains)
			.", small:".terrain_link($zone->{small}, @terrains)
			.(($zone->{guard}!=0) || ($zone->{tower}!=0) ? " containing" : "")
			.($zone->{guard}!=0
				? " quest monster ".monster_link($zone->{guard}, @monsters)
					.($zone->{tower}!=0 ? " and" : "")
				: "")
			.($zone->{tower}!=0
				? " vault(tower) ".vault_link($zone->{tower}, @vaults)
				: "")
		);
	}
	
	return li("Zones: ".ul(@string));
}

sub dump_dungeons_store($\@\@) {
	my $dungeon = shift;
	my @terrains = @{ (shift) };
	my @stores = @{ (shift) };
	my @string;
	
	foreach my $store (@{ $dungeon->{store} }) {
		if(not $store eq "0") {
			push @string,
				# rather hacky and assumes only a single DEFAULT entry in terrain, but should work
				store_link($terrains[$store]->{state}[0]->{power}, @stores)
				." ( ".terrain_link($store, @terrains).") ";
		}
	}
	
	return li("Stores: ", @string) if scalar @string != 0;
	# else
	return "";
}

sub dump_dungeons($\@\@\@\@\@) {
	my $fileout  = shift;
	my @dungeons = @{ (shift) };
	my @monsters = @{ (shift) };
	my @terrains = @{ (shift) };
	my @vaults   = @{ (shift) };
	my @stores   = @{ (shift) };
	
	open FILEOUT, $fileout or die "cannot open $fileout: $!\n";
	
	print FILEOUT
		generic_header("dungeon"),
		generic_links(@dungeons);
	
	# all the data blocks
	foreach my $dungeon ( @dungeons ) {
		# array is still sparse
		next if not $dungeon;
	
		print FILEOUT
			generic_item_header($dungeon),
			start_ul;
	
		print FILEOUT dump_dungeons_nearby($dungeon, @dungeons) if $dungeon->{nearby};
		print FILEOUT li("Wandering Monster Character: ".strong($dungeon->{r_char})) if $dungeon->{r_char};
		print FILEOUT li("Wandering Monster Flags: ".strong($dungeon->{r_flag})) if $dungeon->{r_flag};
		print FILEOUT li("Quest Monster "
			.monster_link($dungeon->{quest_monster}, @monsters)
			." opens entrance to "
			.name_link($dungeon->{quest_opens}, $dungeons[$dungeon->{quest_opens}]->{name})
		) if $dungeon->{quest_monster};
		print FILEOUT li("If "
			.name_link($dungeon->{town_lockup_ifvisited}, $dungeons[$dungeon->{town_lockup_ifvisited}]->{name})
			." is visited lockup town until "
			.strong($dungeon->{town_lockup_monster})
			." is defeated"
		) if $dungeon->{town_lockup_monster};
		print FILEOUT li("If "
			.name_link($dungeon->{guardian_ifvisited}, $dungeons[$dungeon->{guardian_ifvisited}]->{name})
			." is visited replace level guardian with "
			.strong($dungeon->{replace_guardian})
		) if $dungeon->{replace_guardian};
		print FILEOUT li("If "
			.name_link($dungeon->{replace_ifvisited}, $dungeons[$dungeon->{replace_ifvisited}]->{name})
			." is visited replace current level with "
			.name_link($dungeon->{replace_with}, $dungeons[$dungeon->{replace_with}]->{name})
		) if $dungeon->{replace_with};
		print FILEOUT dump_dungeons_zones($dungeon, @dungeons, @monsters, @terrains, @vaults) if $dungeon->{zones};
		print FILEOUT dump_dungeons_store($dungeon, @terrains, @stores) if $dungeon->{store};
	
		print FILEOUT end_ul;

		print FILEOUT p($dungeon->{text}) if $dungeon->{text};
	}
	
	print FILEOUT generic_footer;

	dump_perl("dungeon", @dungeons);
}

##########
# Stores!
##########

sub parse_stores($) {
	my $filein = shift;
	
	open FILEIN, $filein or die "cannot open $filein: $!\n";

	my @stores;
	my $idx = -1;
	my $num_sell = 0;

	foreach(<FILEIN>) {
		chomp;
		if(/^#/) { }       #discard
		elsif(/^\s*$/) { } #discard
		elsif(/^V:/) { }   #discard
		elsif(/^N:$params2$/) {
			$idx = $1;
			$stores[$idx]{idx} = $1;
			$stores[$idx]{name} = $2;
			$num_sell = 0;
		}
		elsif(/^X:$params1$/) {
			$stores[$idx]{base} = $1;
		}
		elsif(/^O:$params3$/) {
			$stores[$idx]{sell}[$num_sell]{tval} = $1;
			$stores[$idx]{sell}[$num_sell]{sval} = $2;
			$stores[$idx]{sell}[$num_sell]{count} = $3;
			$num_sell++;
		}
		# WON'T FIX: Don't handle "B" for buy, since it's not used
		else {
			print "Huh?:".$_."\n";
		}
	}

	close(FILEIN);
	
	return @stores;
}

sub dump_stores_sell($\@) {
	my $store = shift;
	my @TV = @{ (shift) };
	my @string;
	
	foreach my $sell (@{ $store->{sell} }) {
		if (defined @TV[$sell->{tval}]->{item}[$sell->{sval}]) {
			my %tval = %{ @TV[$sell->{tval}] };
			my %sval = %{ @TV[$sell->{tval}]->{item}[$sell->{sval}] };
			push @string, li(
				strong($sell->{count})." "
				.a({-href=>"$filenames{defines_TV}{html}#$tval{idx}_$sval{idx}"}, "$sval{name} ($sval{idx})")
			);
		}
		else {
			my %tval = %{ @TV[$sell->{tval}] };
			die "huh?" if not defined $tval{idx};
			
			push @string, li(
				strong($sell->{count})
				." items of type "
				.a({-href=>"$filenames{defines_TV}{html}#$tval{idx}"}, "$tval{name} ($tval{idx})")
				." subtype ".strong($sell->{sval})
			);
		}
	}
	
	return li("Sell Items: ".ul(@string));
}

sub dump_stores($\@\@) {
	my $fileout = shift;
	my @stores = @{ (shift) };
	my @TV = @{ (shift) };
	
	open FILEOUT, $fileout or die "cannot open $fileout: $!\n";
	
	print FILEOUT
		generic_header("store"),
		generic_links(@stores);
	
	# all the data blocks
	foreach my $store( @stores ) {
		# array is still sparse
		next if not $store;
	
		print FILEOUT
			generic_item_header($store),
			start_ul;
	
		print FILEOUT li("Base buy: ".strong($store->{base})) if $store->{base};
		print FILEOUT dump_stores_sell($store, @TV) if $store->{sell};
		print FILEOUT end_ul;

	}
	print FILEOUT generic_footer;

	dump_perl("store", @stores);
}

##########
# Monsters!
##########

sub parse_monsters($) {
	my $filein = shift;
	
	open FILEIN, $filein or die "cannot open $filein: $!\n";

	my @monsters;
	my $idx = -1;
	my $num_blows = 0;

	foreach(<FILEIN>) {
		chomp;
		if(/^#/) { }       #discard
		elsif(/^\s*$/) { } #discard
		elsif(/^V:/) { }   #discard
		elsif(/^N:$params2$/) {
			$idx = $1;
			$monsters[$idx]{idx} = $1;
			$monsters[$idx]{name} = $2;
			$num_blows = 0;
		}
		elsif(/^D:$params1$/) {
			$monsters[$idx]{text} .= $1;
		}
		elsif(/^G:(.):(.)$/) {
			$monsters[$idx]{d_char} = $1;
			$monsters[$idx]{d_attr} = $2;
		}
		elsif(/^I:$params5$/) {
			$monsters[$idx]{speed} = $1;
			$monsters[$idx]{hp} = $2;
			$monsters[$idx]{aaf} = $3;
			$monsters[$idx]{ac} = $4;
			$monsters[$idx]{"sleep"} = $5;
		}
		elsif(/^W:$params4$/) {
			$monsters[$idx]{level} = $1;
			$monsters[$idx]{rarity} = $2;
			$monsters[$idx]{grp_idx} = $3;
			$monsters[$idx]{mexp} = $4;
		}
		elsif(/^C:$params5$/) {
			$monsters[$idx]{calculated_level} = $1;
			# code discards these two for some reason
			#$monsters[$idx]{rarity} = $2;
			#$monsters[$idx]{grp_idx} = $3;
			$monsters[$idx]{calculated_mexp} = $4;
			$monsters[$idx]{forced_mexp} = $5;
		}
		elsif(/^M:$params4$/) {
			$monsters[$idx]{freq_innate} = $1;
			$monsters[$idx]{freq_spell} = $2;
			$monsters[$idx]{spell_power} = $3;
			$monsters[$idx]{mana} = $4;
		}
		elsif(/^B:(?:$fmid:)*(?:$fmid:)*$fend$/) {
			# game slices and dices the dice value up, but there's no need for us to
			( undef,
				$monsters[$idx]{monster_blow}[$num_blows]{method},
				$monsters[$idx]{monster_blow}[$num_blows]{effect},
				$monsters[$idx]{monster_blow}[$num_blows]{dice}
			) = split(/:/);
			$num_blows++;
		}
		elsif(/^F:$params1$/) {
			$monsters[$idx]{flags} .= $1;
		}
		elsif(/^S:$params1$/) {
			push @{ $monsters[$idx]{spell_flags} }, trim split(/\|/, $1);
		}
		else {
			print "Huh?:".$_."\n";
		}
	}

	close(FILEIN);
	
	return @monsters;
}

sub dump_monsters_blows($\%\%) {
	my $monster = shift;
	my %blows_map = %{ (shift) };
	my %effects_map = %{ (shift) };
	my @string;
	
	foreach my $blow (@{ $monster->{monster_blow} }) {
		push @string, li(
			blow_map_link($blow->{method}, %blows_map)
			.($blow->{effect} ? " causes ".effect_map_link($blow->{effect}, %effects_map) : "")
			.($blow->{dice} ? " and ".strong($blow->{dice})." damage" : "")
		);
	}
	
	return li("Blows: ".ul(@string));
}

sub dump_monsters_spellflags($\%) {
	my @spell_flags = @{ (shift) };
	my %blows_map = %{ (shift) };
	my @string;
	
	foreach my $flag (@spell_flags) {
		push @string, " ".blow_map_link($flag, %blows_map);
	}
	return join "", @string;
}

sub dump_monsters($\@\%\%) {
	my $fileout = shift;
	my @monsters = @{ (shift) };
	my %effects_map = %{ (shift) };
	my %blows_map = %{ (shift) };
	
	open FILEOUT, $fileout or die "cannot open $fileout: $!\n";
	
	print FILEOUT
		generic_header("monster"),
		generic_links(@monsters);
	
	# all the data blocks
	foreach my $monster( @monsters ) {
		# array is still sparse
		next if not $monster;
	
		print FILEOUT
			generic_item_header($monster),
			start_ul;
		
		print FILEOUT
			li("Looks like ".strong($monster->{d_char})." coloured ".strong($monster->{d_attr}))
			.li("Speed: ".strong($monster->{speed}))
			.li("Hit points: ".strong($monster->{hp}))
			.li("Vision/Area of affect radius: ".strong($monster->{aaf}))
			.li("Armour class: ".strong($monster->{ac}))
			.li("Sleep/Alertness: ".strong($monster->{"sleep"}))
			.li("Creature Level/depth found: ".strong($monster->{"level"}))
			.li("Rarity: ".strong($monster->{"rarity"}))
			.li("Group Index: ".strong($monster->{"grp_idx"}))
			.li("Experience for kill: ".strong($monster->{"mexp"}))
		;
		print FILEOUT
			li("Autogenerated calculated creature level/depth: ".strong($monster->{"calculated_level"}))
			.li("Calculated experience for kill at the calculated level/depth: "
				.strong($monster->{"calculated_mexp"}))
			.li("Calculated experience for the kill at the non-autogenerated creature level/depth: ".strong($monster->{"forced_mexp"}))
		if $monster->{"calculated_level"};
		
		print FILEOUT
			li("Monster spell power ".strong($monster->{spell_power})
				.(" and mana ".strong($monster->{mana})))
			.($monster->{freq_innate} != 0 ? li("Innate spell frequencey ".strong($monster->{freq_innate})) : "")
			.($monster->{freq_spell} != 0 ? li("Other spell frequency ".strong($monster->{freq_spell})) : "")
		if $monster->{spell_power};
		
		print FILEOUT dump_monsters_blows($monster, %blows_map, %effects_map) if $monster->{monster_blow};
		print FILEOUT li("Flags: ".strong($monster->{flags})) if $monster->{flags};
		print FILEOUT li("Spell Flags: ".dump_monsters_spellflags($monster->{spell_flags}, %blows_map)) if $monster->{spell_flags};

		print FILEOUT end_ul;

		print FILEOUT p($monster->{text}) if $monster->{text};
	}
	print FILEOUT generic_footer;

	dump_perl("monster", @monsters);
}

##########
# Terrain!
##########

sub parse_terrains($) {
	my $filein = shift;
	
	open FILEIN, $filein or die "cannot open $filein: $!\n";

	my @terrains;
	my $idx = -1;
	my $num_state = 0;

	foreach(<FILEIN>) {
		chomp;
		if(/^#/) { }       #discard
		elsif(/^\s*$/) { } #discard
		elsif(/^V:/) { }   #discard
		elsif(/^N:$params2$/) {
			$idx = $1;
			$terrains[$idx]{idx} = $1;
			$terrains[$idx]{name} = $2;
			$num_state = 0;
		}
		elsif(/^M:$params1$/) {
			$terrains[$idx]{mimic} = $1;
		}
		elsif(/^U:$params1$/) {
			$terrains[$idx]{unseen} = $1;
		}
		elsif(/^O:$params1$/) {
			$terrains[$idx]{k_idx} = $1;
		}
		elsif(/^G:(.):(.)$/) {
			$terrains[$idx]{d_char} = $1;
			$terrains[$idx]{d_attr} = $2;
		}
		elsif(/^F:$params1$/) {
			$terrains[$idx]{flags} .= $1
		}
		elsif(/^W:$params4$/) {
			$terrains[$idx]{level} = $1;
			$terrains[$idx]{rarity} = $2;
			$terrains[$idx]{priority} = $3;
			$terrains[$idx]{edge} = $4;
		}
		elsif(/^K:$params3$/) {
			$terrains[$idx]{state}[$num_state]{action} = $1;
			$terrains[$idx]{state}[$num_state]{result} = $2;
			$terrains[$idx]{state}[$num_state]{power} = $3;
			$num_state++;
		}
		elsif(/^T:(?:$fmid:)*(?:$fmid:)*$fend$/) {
			# game slices and dices the dice value up, but there's no need for us to
			if(not defined $1) {
				#print "$idx $1:$2:$3\n";
				$terrains[$idx]{blow}{method} = $3;
			}
			elsif(not defined $2) {
				#print "$idx $1:$2:$3\n";
				$terrains[$idx]{blow}{method} = $1;
				# we'll assume it's a XdY if it starts with a number
				my $something = $3;
				if($something =~ /^[0-9]/) {
					$terrains[$idx]{blow}{dice} = $something;
				}
				else {
					$terrains[$idx]{blow}{effect} = $something;
				}
			}
			else {
				#print "$idx $1:$2:$3\n";
				$terrains[$idx]{blow}{method} = $1;
				$terrains[$idx]{blow}{effect} = $2;
				$terrains[$idx]{blow}{dice} = $3;
			}
		}
		elsif(/^T:$params3$/) {
			$terrains[$idx]{blow}{method} = $1;
			$terrains[$idx]{blow}{effect} = $2;
			# code splits this, we don't have to
			$terrains[$idx]{blow}{dice} = $3;
		}
		elsif(/^S:$params1$/) {
			$terrains[$idx]{spell} = $1;
		}
		elsif(/^D:$params1$/) {
			$terrains[$idx]{text} .= $1
		}
		else {
			print "Huh?:".$_."\n";
		}
	}

	close(FILEIN);
	
	return @terrains;
}

sub dump_terrains_state($\@) {
	my $terrain = shift;
	my @terrains = @{ (shift) };
	my @string;
	
	foreach my $state (@{ $terrain->{state} }) {
		push @string, li(
			strong($state->{action})
			." resulting in ".terrain_link($state->{result}, @terrains)
			.($state->{power} != 0 ? " at power ".strong($state->{power}) : "")
		);
	}
	
	return li("States: ".ul(@string));
}

sub dump_terrains($\@\%\%) {
	my $fileout = shift;
	my @terrains = @{ (shift) };
	my %blows_map = %{ (shift) };
	my %effects_map = %{ (shift) };
	
	open FILEOUT, $fileout or die "cannot open $fileout: $!\n";
	
	print FILEOUT
		generic_header("terrain"),
		generic_links(@terrains);
	
	# all the data blocks
	foreach my $terrain( @terrains ) {
		# array is still sparse
		next if not $terrain;
	
		print FILEOUT
			generic_item_header($terrain),
			start_ul;

		print FILEOUT li("Looks like ".strong($terrain->{d_char})." coloured ".strong($terrain->{d_attr}));
		print FILEOUT li("Mimic: ".terrain_link($terrain->{mimic}, @terrains)) if $terrain->{mimic};
		print FILEOUT li("Unseen: ".terrain_link($terrain->{unseen}, @terrains)) if $terrain->{unseen};
		print FILEOUT li("Object at this location: ".strong($terrain->{k_idx})) if $terrain->{k_idx};
		print FILEOUT li("Flags: ".strong($terrain->{flags})) if $terrain->{flags};
		print FILEOUT
			li("Level: ".strong($terrain->{level}))
			.li("Rarity: ".strong($terrain->{rarity}))
			.li("Priority: ".strong($terrain->{priority}))
			.($terrain->{edge} ? li("Edge with Feature: ".strong($terrain->{edge})) : "")
		;
		print FILEOUT dump_terrains_state($terrain, @terrains) if $terrain->{state};
		print FILEOUT
			li("Trap: ".strong($terrain->{blow}{method})
			.($terrain->{blow}{effect} ? " causes ".effect_map_link($terrain->{blow}{effect}, %effects_map) : "")
			.($terrain->{blow}{dice} ? " and ".strong($terrain->{blow}{dice})." damage" : "")
		) if $terrain->{blow};

		# the following assumes there is only one spell trap per S: line
		print FILEOUT li("Trap Spell: ".blow_map_link($terrain->{spell}, %blows_map)) if $terrain->{spell};
		
		print FILEOUT end_ul;

		print FILEOUT p($terrain->{text}) if $terrain->{text};
	}
	print FILEOUT generic_footer;

	dump_perl("terrain", @terrains);
}

##########
# Vaults!
##########

sub parse_vaults($) {
	my $filein = shift;
	
	open FILEIN, $filein or die "cannot open $filein: $!\n";

	my @vaults;
	my $idx = -1;

	foreach(<FILEIN>) {
		chomp;
		if(/^#/) { }       #discard
		elsif(/^\s*$/) { } #discard
		elsif(/^V:/) { }   #discard
		elsif(/^N:$params2$/) {
			$idx = $1;
			$vaults[$idx]{idx} = $1;
			$vaults[$idx]{name} = $2;
		}
		elsif(/^D:$params1$/) {
			# descriptions are somewhat special in this one, we need to reinsert eols.
			$vaults[$idx]{text} .= $1."\n"
		}
		elsif(/^X:$params6$/) {
			$vaults[$idx]{typ} = $1;
			$vaults[$idx]{rat} = $2;
			$vaults[$idx]{hgt} = $3;
			$vaults[$idx]{wid} = $4;
			$vaults[$idx]{min_lev} = $5;
			$vaults[$idx]{max_lev} = $6;
		}
		else {
			print "Huh?:".$_."\n";
		}
	}

	close(FILEIN);
	
	return @vaults;
}

sub dump_vaults($\@) {
	my $fileout = shift;
	my @vaults = @{ (shift) };
	
	open FILEOUT, $fileout or die "cannot open $fileout: $!\n";
	
	print FILEOUT
		generic_header("vault"),
		generic_links(@vaults);
	
	# all the data blocks
	foreach my $vault( @vaults ) {
		# array is still sparse
		next if not $vault;
	
		print FILEOUT
			generic_item_header($vault),
			start_ul;

		print FILEOUT
			li("Type: ".strong($vault->{typ}))
			.li("Rating: ".strong($vault->{rat}))
			.li("Height: ".strong($vault->{hgt}))
			.li("Width: ".strong($vault->{wid}))
			.li("Min Level: ".strong($vault->{min_lev}))
			.li("Max Level: ".strong($vault->{max_lev}))
		;

		print FILEOUT end_ul;

		print FILEOUT pre($vault->{text});
	}
	print FILEOUT generic_footer;

	dump_perl("vault", @vaults);
}

##########
# Objects!
##########

sub parse_objects($) {
	my $filein = shift;
	
	open FILEIN, $filein or die "cannot open $filein: $!\n";

	my @objects;
	my $idx = -1;

	foreach(<FILEIN>) {
		chomp;
		if(/^#/) { }       #discard
		elsif(/^\s*$/) { } #discard
		elsif(/^V:/) { }   #discard
		elsif(/^N:$params2$/) {
			$idx = $1;
			$objects[$idx]{idx} = $1;
			$objects[$idx]{name} = $2;
		}
		elsif(/^G:(.):(.)$/) {
			$objects[$idx]{d_char} = $1;
			$objects[$idx]{d_attr} = $2;
		}
		elsif(/^I:$params3$/) {
			$objects[$idx]{tval} = $1;
			$objects[$idx]{sval} = $2;
			$objects[$idx]{pval} = $3;
		}
	}

	close(FILEIN);
	
	return @objects;
}

sub dump_objects($\@) {
	my $fileout = shift;
	my @objects = @{ (shift) };
	
	open FILEOUT, $fileout or die "cannot open $fileout: $!\n";
	
	print FILEOUT
		generic_header("object"),
		generic_links(@objects);
	
	# all the data blocks
	foreach my $object( @objects ) {
		# array is still sparse
		next if not $object;
	
		print FILEOUT
			generic_item_header($object),
			start_ul;
	
		print FILEOUT
			li("Looks like ".strong($object->{d_char})." coloured ".strong($object->{d_attr}))
		;
		print FILEOUT end_ul;

	}
	print FILEOUT generic_footer;

	dump_perl("object", @objects);
}

##########
# Shop Owners!
##########

sub parse_shop_owners($) {
	my $filein = shift;
	
	open FILEIN, $filein or die "cannot open $filein: $!\n";

	my @shop_owners;
	my $idx = -1;

	foreach(<FILEIN>) {
		chomp;
		if(/^#/) { }       #discard
		elsif(/^\s*$/) { } #discard
		elsif(/^V:/) { }   #discard
		elsif(/^N:$params3$/) {
			# WARNING: this breaks if a store has more then 10 idx per store
			$idx = "$1$2";
			$shop_owners[$idx]{store} = $1;
			$shop_owners[$idx]{idx} = $2;
			$shop_owners[$idx]{name} = $3;
		}
		elsif(/^I:$params6$/) {
			$shop_owners[$idx]{owner_race} = $1;
			$shop_owners[$idx]{max_cost} = $2;
			$shop_owners[$idx]{max_inflate} = $3;
			$shop_owners[$idx]{min_inflate} = $4;
			$shop_owners[$idx]{haggle_per} = $5;
			$shop_owners[$idx]{insult_max} = $6;
		}
		else {
			print "Huh?:".$_."\n";
		}
	}

	close(FILEIN);
	
	return @shop_owners;
}

sub dump_shop_owners($\@\@) {
	my $fileout = shift;
	my @shop_owners = @{ (shift) };
	my @stores = @{ (shift) };
	
	open FILEOUT, $fileout or die "cannot open $fileout: $!\n";
	
	print FILEOUT
		generic_header("shop_owner");
	
	# specific link collection since shop_owner is a little special
	foreach my $shop_owner ( @shop_owners ) {
		# array is sparse
		next if not $shop_owner;

		print FILEOUT id_link("$shop_owner->{store}:$shop_owner->{idx}")." ";
	}

	# all the data blocks
	foreach my $shop_owner( @shop_owners ) {
		# array is still sparse
		next if not $shop_owner;
	
		# non-generic item header since this is a little special...
		print FILEOUT
			a({name=>"$shop_owner->{store}:$shop_owner->{idx}"},
				h3("$shop_owner->{name} ($shop_owner->{store}:$shop_owner->{idx})")),
			start_ul;

		print FILEOUT
			li("Store: ".store_link($shop_owner->{store}, @stores))
			.li("Owner Race: ".strong($shop_owner->{owner_race}))
			.li("Maximum Cost/Purse Limit: ".strong($shop_owner->{max_cost}))
			.li("Max Greed/Inflation: ".strong($shop_owner->{max_inflate}))
			.li("Min Greed/Inflation: ".strong($shop_owner->{min_inflate}))
			.li("Haggle Unit/haggleper: ".strong($shop_owner->{haggle_per}))
			.li("Insult limit: ".strong($shop_owner->{insult_max}))
			;
	
		print FILEOUT end_ul;

	}
	print FILEOUT generic_footer;

	dump_perl("shop_owner", @shop_owners);
}

##########
# rooms!
##########

sub parse_rooms($) {
	my $filein = shift;
	
	open FILEIN, $filein or die "cannot open $filein: $!\n";

	my @rooms;
	my $idx = -1;

	foreach(<FILEIN>) {
		chomp;
		if(/^#/) { }       #discard
		elsif(/^\s*$/) { } #discard
		elsif(/^V:/) { }   #discard
		elsif(/^N:$params8$/) {
			$idx++;
			$rooms[$idx]{idx} = $idx;
			$rooms[$idx]{name} = "Room Entry $idx";
			$rooms[$idx]{chart} = $1;
			$rooms[$idx]{next} = $2;
			$rooms[$idx]{branch_on} = $3;
			$rooms[$idx]{branch} = $4;
			$rooms[$idx]{chance} = $5;
			$rooms[$idx]{not_chance} = $6;
			$rooms[$idx]{level_min} = $7;
			$rooms[$idx]{level_max} = $8;
		}
		elsif(/^A:$fend$/) {
			# A: is often "", so if it's null we might as well not add it
			$rooms[$idx]{name1} = $1 if not $1 eq "";
		}
		elsif(/^B:$fend$/) {
			# B: is often "", so if it's null we might as well not add it
			$rooms[$idx]{name2} = $1 if not $1 eq "";
		}
		elsif(/^D:$fend$/) {
			$rooms[$idx]{text} .= $1;
		}
		elsif(/^S:$params1$/) {
			$rooms[$idx]{flags} .= $1;
		}
		elsif(/^P:$params1$/) {
			$rooms[$idx]{p_flag} .= $1;
		}
		elsif(/^L:$params1$/) {
			$rooms[$idx]{l_flag} .= $1;
		}
		elsif(/^G:$params1$/) {
			$rooms[$idx]{r_char} = $1;
		}
		elsif(/^K:$params3$/) {
			$rooms[$idx]{tval} = $1;
			$rooms[$idx]{min_sval} = $2;
			$rooms[$idx]{max_sval} = $3;
		}
		elsif(/^F:$params5$/) {
			$rooms[$idx]{feat} = $1;
			$rooms[$idx]{theme}[0] = $2;
			$rooms[$idx]{theme}[1] = $3;
			$rooms[$idx]{theme}[2] = $4;
			$rooms[$idx]{theme}[3] = $5;
		}
		elsif(/^R:$params1$/) {
			$rooms[$idx]{r_flag} .= $1;
		}
		else {
			print "Huh?:".$_."\n";
		}
	}

	close(FILEIN);
	
	return @rooms;
}

sub dump_rooms($\@\@) {
	my $fileout = shift;
	my @rooms = @{ (shift) };
	my @terrains = @{ (shift) };
	
	open FILEOUT, $fileout or die "cannot open $fileout: $!\n";
	
	print FILEOUT
		generic_header("room"),
		generic_links(@rooms);
	
	# all the data blocks
	foreach my $room( @rooms ) {
		# array is still sparse
		next if not $room;
	
		print FILEOUT
			generic_item_header($room),
			start_ul;
	
		print FILEOUT
			li("Chart Index: ", strong($room->{chart}))
			.li("Next Chart Index: ", strong($room->{next}))
			.li("Branch to chart index: ", strong($room->{branch_on}))
			.li("Branch on chart index: ", strong($room->{branch}))
			.li(" Frequency of this entry: ", strong($room->{chance}))
			.li("Frequency of this entry if conditions not met: ", strong($room->{not_chance}))
			.li("Minimum level: ", strong($room->{level_min}))
			.li("Maximum level: ", strong($room->{level_max}))
		;
		print FILEOUT li("Name1: ", strong($room->{name1})) if $room->{name1};
		print FILEOUT li("Name2: ", strong($room->{name2})) if $room->{name2};
		print FILEOUT li("Room (special) flags: ", strong($room->{flags})) if $room->{flags};
		print FILEOUT li("Description Placement Flags: ", strong($room->{p_flag})) if $room->{p_flag};
		print FILEOUT li("Restrict to these level types: ", strong($room->{l_flag})) if $room->{l_flag};
		print FILEOUT li("Add races of this character: ", strong($room->{r_char})) if $room->{r_char};
		print FILEOUT li("Add items of tval ".strong($room->{tval})." between sval "
			.strong($room->{min_sval})." and ".strong($room->{max_sval})) if $room->{tval};

		print FILEOUT li("Feature: ".terrain_link($room->{feat}, @terrains))
		if defined $room->{feat} and $room->{feat} != 0;
		print FILEOUT li("Tunnel Feature: ".terrain_link($room->{theme}[0], @terrains))
		if defined $room->{theme}[0] and $room->{theme}[0] != 0;
		print FILEOUT li("Doorway Feature ".terrain_link($room->{theme}[1], @terrains))
		if defined $room->{theme}[1] and $room->{theme}[1] != 0;
		print FILEOUT li("Trap Feature ".terrain_link($room->{theme}[2], @terrains))
		if defined $room->{theme}[2] and $room->{theme}[2] != 0;
		print FILEOUT li("Chasm Bridge Feature ".terrain_link($room->{theme}[3], @terrains))
		if defined $room->{theme}[3] and $room->{theme}[3] != 0;
		
		print FILEOUT li("Restrict to levels with these monster types: ", strong($room->{r_flag})) if $room->{r_flag};
		
		print FILEOUT end_ul;
		
		print FILEOUT p($room->{text}) if $room->{text};
	}
	print FILEOUT generic_footer;

	dump_perl("room", @rooms);
}

##########
# Blows
##########

sub parse_blows($) {
	my $filein = shift;
	
	open FILEIN, $filein or die "cannot open $filein: $!\n";

	my @blows;
	my $idx = -1;
	my $num_desc = 0;

	foreach(<FILEIN>) {
		chomp;
		if(/^#/) { }       #discard
		elsif(/^\s*$/) { } #discard
		elsif(/^V:/) { }   #discard
		elsif(/^N:$params2$/) {
			$idx = $1;
			$blows[$idx]{idx} = $1;
			$blows[$idx]{name} = $2;
		}
		elsif(/^F:$params1$/) {
			$blows[$idx]{flags} .= $1;
		}
		elsif(/^T:$params2$/) {
			$blows[$idx]{desc}[$num_desc]{text} = $2;
			($blows[$idx]{desc}[$num_desc]{min}, $blows[$idx]{desc}[$num_desc]{max})
				= ($1 =~ /([0-9]+)-([0-9]+)/);
		}
		elsif(/^D:$params2$/) {
			$blows[$idx]{info1} = $1;
			$blows[$idx]{info2} = $2;
		}
		elsif(/^I:$params3$/) {
			$blows[$idx]{sound} = $1;
			$blows[$idx]{best_range} = $2;
			$blows[$idx]{diameter_of_source} = $3;
		}
		elsif(/^W:$params3$/) {
			$blows[$idx]{mana_cost} = $1;
			$blows[$idx]{ammo_tval} = $2;
			$blows[$idx]{ammo_sval} = $3;
		}
		elsif(/^X:$params4$/) {
			$blows[$idx]{dam_mult} = $1;
			$blows[$idx]{dam_div} = $2;
			$blows[$idx]{dam_var} = $3;
			$blows[$idx]{d_res} = $4;
		}
		elsif(/^C:$params7$/) {
			$blows[$idx]{d_base}  = $1;
			$blows[$idx]{d_summ}  = $2;
			$blows[$idx]{d_hurt}  = $3;
			$blows[$idx]{d_mana}  = $4;
			$blows[$idx]{d_esc}   = $5;
			$blows[$idx]{d_tact}  = $6;
			$blows[$idx]{d_range} = $7;
		}
		elsif(/^A:$params2$/) {
			$blows[$idx]{arc} = $1;
			$blows[$idx]{degree_factor} = $2;
		}
		elsif(/^M:$params1$/) {
			$blows[$idx]{max_range} = $1;
		}
		elsif(/^R:$params1$/) {
			$blows[$idx]{radius} = $1;
		}
		elsif(/^U:$params1$/) {
			$blows[$idx]{number} = $1;
		}
		else {
			print "Huh?:".$_."\n";
		}
	}

	close(FILEIN);
	
	return @blows;
}

sub dump_blows_desc($) {
	my $blow = shift;
	my @string;
	
	foreach my $desc (@{ $blow->{desc} }) {
		push @string, li(
			strong($desc->{min})
			." - ".strong($desc->{max})
			.": ".strong($desc->{text})
		);
	}
	
	return li("Descriptions: ".ul(@string));
}

sub dump_blows($\@\%) {
	my $fileout = shift;
	my @blows = @{ (shift) };
	my %effects_map = %{ (shift) };
	
	open FILEOUT, $fileout or die "cannot open $fileout: $!\n";
	
	print FILEOUT
		generic_header("blow"),
		generic_links(@blows);
	
	# all the data blocks
	foreach my $blow( @blows ) {
		# array is still sparse
		next if not $blow;
	
		print FILEOUT
			generic_item_header($blow),
			start_ul;
		
		print FILEOUT li("Flags: ".strong($blow->{flags})) if defined $blow->{flags};
		print FILEOUT dump_blows_desc($blow) if defined $blow->{desc};
		print FILEOUT li("Info 1: ".strong($blow->{info1})) if $blow->{info1};
		print FILEOUT li("Info 2: ".strong($blow->{info2})) if $blow->{info2};
		print FILEOUT
			li("Sound: ".strong($blow->{sound}))
			.li("Best range: ".strong($blow->{best_range}))
			.li("Diameter of Source: ".strong($blow->{diameter_of_source}))
			if defined $blow->{sound};
		print FILEOUT li("Mana Cost: ".strong($blow->{mana_cost}))
			if defined $blow->{mana_cost} && $blow->{mana_cost}!=0;
		print FILEOUT li("Ammo tval:".strong($blow->{ammo_tval})." sval:".strong($blow->{ammo_sval}))
			if defined $blow->{ammo_tval} && $blow->{ammo_tval}!=0 && $blow->{ammo_sval}!=0;
		print FILEOUT
			li("Monster spell/breath damage:"
				.($blow->{dam_mult}!=0 ? " multipler ".strong($blow->{dam_mult}) : "")
				.($blow->{dam_div}!=0 ? " divisor ".strong($blow->{dam_div}) : "")
				.($blow->{dam_var}!=0 ? " variance ".strong($blow->{dam_var}) : "")
				.($blow->{d_res} eq "" ? "" : " resisted by ".effect_map_link($blow->{d_res}, %effects_map)))
			if defined $blow->{dam_mult};
		print FILEOUT
			li("Monster spell desirability:"
				." d_base=".strong($blow->{d_base})
				." d_summ=".strong($blow->{d_summ})
				." d_hurt=".strong($blow->{d_hurt})
				." d_mana=".strong($blow->{d_mana})
				." d_esc=".strong($blow->{d_esc})
				." d_tact=".strong($blow->{d_tact})
				." d_range=".strong($blow->{d_range}))
			if defined $blow->{d_base};
		print FILEOUT
			li("Arc ".strong($blow->{arc})
				." degree factor ".strong($blow->{degree_factor}))
			if defined $blow->{arc};
		print FILEOUT li("Maximum Range: ".strong($blow->{max_range})) if defined $blow->{max_range};
		print FILEOUT li("Radius: ".strong($blow->{radius})) if defined $blow->{radius};
		print FILEOUT li("Number: ".strong($blow->{number})) if defined $blow->{number};

		print FILEOUT end_ul;
	}
	print FILEOUT generic_footer;

	dump_perl("blow", @blows);
}

##########
# Effects
##########

sub parse_effects($) {
	my $filein = shift;
	
	open FILEIN, $filein or die "cannot open $filein: $!\n";

	my @effects;
	my $idx = -1;

	foreach(<FILEIN>) {
		chomp;
		if(/^#/) { }       #discard
		elsif(/^\s*$/) { } #discard
		elsif(/^V:/) { }   #discard
		elsif(/^N:$params2$/) {
			$idx = $1;
			$effects[$idx]{idx} = $1;
			$effects[$idx]{name} = $2;
		}
		elsif(/^I:$params3$/) {
			$effects[$idx]{power} = $1;
			$effects[$idx]{dam_max} = $2;
			$effects[$idx]{dam_max_powerful} = $2;
		}
		elsif(/^D:$params7$/) {
			$effects[$idx]{info}[0] = $1;
			$effects[$idx]{info}[1] = $2;
			$effects[$idx]{info}[2] = $3;
			$effects[$idx]{info}[3] = $4;
			$effects[$idx]{info}[4] = $5;
			$effects[$idx]{info}[5] = $6;
			$effects[$idx]{info}[6] = $7;
		}
		# F: and T: aren't used either, can't be bothered parsing
		else {
			print "Huh?:".$_."\n";
		}
	}

	close(FILEIN);
	
	return @effects;
}

sub dump_effects($\@) {
	my $fileout = shift;
	my @effects = @{ (shift) };
	
	open FILEOUT, $fileout or die "cannot open $fileout: $!\n";
	
	print FILEOUT
		generic_header("effect"),
		generic_links(@effects);
	
	# all the data blocks
	foreach my $effect( @effects ) {
		# array is still sparse
		next if not $effect;
	
		print FILEOUT
			generic_item_header($effect),
			start_ul;

		# I: is always 0:0:0 so I can't be bothered outputting it

		if (defined $effect->{info}) {
			my @stuff;
			for(my $i=0; $i<7; $i++) {
				push @stuff, li($effect->{info}[$i]);
			}
			print FILEOUT "Descriptions: ".ol({-start=>0}, @stuff);
		}

		print FILEOUT end_ul;
	}
	print FILEOUT generic_footer;

	dump_perl("effect", @effects);
}

##########
# Regions
##########

sub parse_regions($) {
	my $filein = shift;
	
	open FILEIN, $filein or die "cannot open $filein: $!\n";

	my @regions;
	my $idx = -1;

	foreach(<FILEIN>) {
		chomp;
		if(/^#/) { }       #discard
		elsif(/^\s*$/) { } #discard
		elsif(/^V:/) { }   #discard
		elsif(/^N:$params2$/) {
			$idx = $1;
			$regions[$idx]{idx} = $1;
			$regions[$idx]{name} = $2;
		}
		elsif(/^G:(.):(.)$/) {
			$regions[$idx]{d_char} = $1;
			$regions[$idx]{d_attr} = $2;
		}
		elsif(/^I:$params1$/) {
			$regions[$idx]{delay_reset} = $1;
		}
		elsif(/^B:$params1$/) {
			$regions[$idx]{method} = $1;
		}
		elsif(/^F:$params1$/) {
			$regions[$idx]{flags1} .= $1." "
		}
		else {
			print "Huh?:".$_."\n";
		}
	}

	close(FILEIN);
	
	return @regions;
}

sub dump_regions($\@\%) {
	my $fileout = shift;
	my @regions = @{ (shift) };
	my %blows_map = %{ (shift) };
	
	open FILEOUT, $fileout or die "cannot open $fileout: $!\n";
	
	print FILEOUT
		generic_header("region"),
		generic_links(@regions);
	
	# all the data blocks
	foreach my $region( @regions ) {
		# array is still sparse
		next if not $region;
	
		print FILEOUT
			generic_item_header($region),
			start_ul;
	
		print FILEOUT li("Looks like ".strong($region->{d_char})." coloured ".strong($region->{d_attr}))
			if defined $region->{d_char};
		print FILEOUT li("Delay between blows: ".strong($region->{delay_reset}))
			if defined $region->{delay_reset};
		print FILEOUT li("Blow method: ".blow_map_link($region->{method}, %blows_map))
			if defined $region->{method};
		print FILEOUT li("Flags: ".strong($region->{flags1}))
			if defined $region->{flags1};
		
		print FILEOUT end_ul;
	}
	print FILEOUT generic_footer;

	dump_perl("region", @regions);
}

##########
# Spells
##########

sub parse_spells($) {
	my $filein = shift;
	
	open FILEIN, $filein or die "cannot open $filein: $!\n";

	my @spells;
	my $idx = -1;

	foreach(<FILEIN>) {
		chomp;
		if(/^#/) { }       #discard
		elsif(/^\s*$/) { } #discard
		elsif(/^V:/) { }   #discard
		elsif(/^N:$params2$/) {
			$idx = $1;
			$spells[$idx]{idx} = $1;
			$spells[$idx]{name} = $2;
		}
	}

	close(FILEIN);
	
	return @spells;
}

sub dump_spells($\@) {
	my $fileout = shift;
	my @spells = @{ (shift) };
	
	open FILEOUT, $fileout or die "cannot open $fileout: $!\n";
	
	print FILEOUT
		generic_header("spell"),
		generic_links(@spells);
	
	# all the data blocks
	foreach my $spell( @spells ) {
		# array is still sparse
		next if not $spell;
	
		print FILEOUT
			generic_item_header($spell),
			start_ul;
	
		print FILEOUT end_ul;

	}
	print FILEOUT generic_footer;

	dump_perl("spell", @spells);
}

##########
# The defines from the .h file
##########

sub parse_defines_TV($\%) {
	my $filein = shift;
	my $defines = shift; #%{ (shift) };
	
	open FILEIN, $filein or die "cannot open $filein: $!\n";

	foreach(<FILEIN>) {
		chomp;
		if(/#define\s+TV_([A-Z_]+)\s+([0-9]+)\s*(.*)$/) {
			#print "works? $1\n";
			$defines->{TV}[$2]{name} = $1;
			$defines->{TV}[$2]{idx} = $2;
			$defines->{TV}[$2]{comment} = $3 if not $3 eq "";
			$defines->{TV}[$2]{comment} =~ s/\/\*\s+(.*)\s+\*\//$1/ if $defines->{TV}[$2]{comment};
		}
	}

	close(FILEIN);
}

# this is much more painful then the TV one...
sub parse_defines_SV($\%) {
	my $filein = shift;
	my $defines = shift; #%{ (shift) };
	
	open FILEIN, $filein or die "cannot open $filein: $!\n";

	my @section;
	
	foreach(<FILEIN>) {
		chomp;
		if(/\/\*\s*The "sval" (?:codes|values) for (TV_[A-Z\/_]+)\s*\*\//) {
			#my $section_name_long = $1;
			my @section_name = split(/\//, $1);
			@section = ();
			die "something odd with SV" if not defined $section_name[0];
			foreach my $sec ( @section_name ) {
				$sec =~ s/TV_(.*)/$1/;
				my $sec_num = undef;
				foreach my $item ( @{ $defines->{TV} } ) {
					#print $item->{name}."\n" if defined $item;
					$sec_num = $item->{idx} if defined $item and $item->{name} eq $sec;
				}
				die "'TV_$sec' not valid" if not defined $sec_num;
				push @section, $sec_num;
			}
			#print "section is @section\n" if defined $section[0];
		}
		elsif(/#define\s+SV_([A-Z_]+)\s+([0-9]+)\s*(.*)$/) {
			# paranoia
			my $name = $1;
			my $idx = $2;
			my $comment = $3;
			
			#print "works? $name $idx\n";
			if (defined $section[0]) {
				foreach my $sec (@section) {
					#print "works2? $name $idx\n";
					die "duplicate item $name $idx found in section $defines->{TV}[$sec]{name} $sec"
						if defined $defines->{TV}[$sec]{item}[$idx];
					$defines->{TV}[$sec]{item}[$idx]{name} = $name;
					$defines->{TV}[$sec]{item}[$idx]{idx} = $idx;
					$defines->{TV}[$sec]{item}[$idx]{comment} = $comment if not $comment eq "";
					$defines->{TV}[$sec]{item}[$idx]{comment} =~ s/\/\*\s+(.*)\s+\*\//$1/
						if $defines->{TV}[$sec]{item}[$idx]{comment};
				}
			}
		}
		elsif(/^ \* Special "sval" limit --/) {
			# this is the "end" of the real SV_? values and beginning of random max/min stuff
			@section = ();
		}
	}

	close(FILEIN);
}

sub dump_defines($\%) {
	my $fileout = shift;
	my %defines = %{ (shift) };
	
	open FILEOUT, $fileout or die "cannot open $fileout: $!\n";
	
	print FILEOUT
		start_html(-title=>"Random Defines"),
		h1("Random Defines"),
		$styles,
		start_ul;#,
		#generic_links(@spells);

	foreach my $tval ( @{ $defines{TV} } ) {
		# array is sparse
		next if not $tval;
		
		print FILEOUT
			li(a({-href=>"#$tval->{idx}"}, "$tval->{idx} $tval->{name}"));
	}
	print FILEOUT end_ul;


	foreach my $tval ( @{ $defines{TV} } ) {
		# array is still sparse
		next if not $tval;
		
		print FILEOUT
			generic_item_header($tval),
			start_ul;
			#li(a({-href=>"#$tval->{idx}"}, "$tval->{idx} $tval->{name}"));
		foreach my $sval ( @{ $tval->{item} } ) {
			# even THIS array is sparse!?
			next if not $sval;
			
			print FILEOUT
				li(a({name=>"$tval->{idx}_$sval->{idx}"},
					"$sval->{name} ($sval->{idx})"
					.($sval->{comment} ? ": ".strong($sval->{comment}) : "")
				))
			;
		}
		print FILEOUT end_ul;
	}
	# all the data blocks
	#foreach my $spell( @spells ) {
	#	# array is still sparse
	#	next if not $spell;
	
	#	print FILEOUT
	#		generic_item_header($spell),
	#		start_ul;
	
	#	print FILEOUT end_ul;

	#}
	print FILEOUT generic_footer;

	open FILETXT, ">defines.txt";
	print FILETXT Dumper(\%defines);
	close FILETXT;
	
	#dump_perl("spell", @spells);
}

##########
# Some Data::Dumper stuff
##########

sub my_filter {
	my ($hash) = @_;
	return [ (sort keys %$hash) ];
}

$Data::Dumper::Sortkeys = \&my_filter;

##########
# Make CGI::Pretty work better
##########

push @CGI::Pretty::AS_IS,qw(strong a);

##########
# Main...
##########

sub idx2name(\@) {
	my @idxs = @{ (shift) };
	my %names;

	foreach my $idx( @idxs ) {
		$names{$idx->{name}} = $idx if defined $idx;
	}

	return %names;
}

my $indir = "";
my $srcdir = "";
my $outdir = "";

# probe for input directory, only handle the obvious cases
# we're in the lib/edit directory
if ( -e $filenames{dungeon}{txt} ) {
	; # indir is ok
}
# we're another directory in lib/
elsif ( -e "../edit/$filenames{dungeon}{txt}" ) {
	$indir = "../edit/";
}
# we're in the lib directory
elsif ( -e "edit/$filenames{dungeon}{txt}" ) {
	$indir = "edit/";
}
# we're in the lib directory
elsif ( -e "lib/edit/$filenames{dungeon}{txt}" ) {
	$indir = "lib/edit/";
}
# we're possibly in the src directory!?
elsif ( -e "../lib/edit/$filenames{dungeon}{txt}" ) {
	$indir = "../lib/edit/";
}
else {
	die "Could not find $filenames{dungeon}{txt}\n";
}

# set the output dir
$outdir = "../docs" if $indir eq ""; # manual for already being in lib/edit
$outdir = $indir."txt2html/";
$outdir =~ s/edit/docs/;
# create it if it doesn't exist
mkdir $outdir;

if ( $indir eq "" ) {
	$srcdir = "../../src"; # manual for already being in lib/edit
}
elsif ( $indir eq "../edit/") {
	$srcdir = "../../src";
}
else {
	$srcdir = $indir;
	$srcdir =~ s#lib/edit#src#;
}

# Start!
print "Parsing txt from: $indir\n";
print "Parsing src from: $srcdir\n";
print "Writing html to:  $outdir\n";

my %defines;

parse_defines_TV("<$srcdir/defines.h", %defines);
parse_defines_SV("<$srcdir/defines.h", %defines);
dump_defines(">$outdir/defines.html", %defines);

print "Parse Dungeons...\n";
my @dungeons = parse_dungeons("<$indir$filenames{dungeon}{txt}");
print "Parse Stores...\n";
my @stores = parse_stores("<$indir$filenames{store}{txt}");
print "Parse Monsters...\n";
my @monsters = parse_monsters("<$indir$filenames{monster}{txt}");
print "Parse Terrain...\n";
my @terrains = parse_terrains("<$indir$filenames{terrain}{txt}");
print "Parse Vaults...\n";
my @vaults = parse_vaults("<$indir$filenames{vault}{txt}");
print "Parse Objects...\n";
my @objects = parse_objects("<$indir$filenames{object}{txt}");
print "Parse Shop Owners...\n";
my @shop_owners = parse_shop_owners("<$indir$filenames{shop_owner}{txt}");
print "Parse Rooms...\n";
my @rooms = parse_rooms("<$indir$filenames{room}{txt}");
print "Parse Blows...\n";
my @blows = parse_blows("<$indir$filenames{blow}{txt}");
print "Parse Effects...\n";
my @effects = parse_effects("<$indir$filenames{effect}{txt}");
print "Parse Regions...\n";
my @regions = parse_regions("<$indir$filenames{region}{txt}");
print "Parse Spells...\n";
my @spells = parse_spells("<$indir$filenames{spell}{txt}");

my %blows_map = idx2name(@blows);
my %effects_map = idx2name(@effects);

print "Writing Dungeons...\n";
dump_dungeons(">$outdir$filenames{dungeon}{html}", @dungeons, @monsters, @terrains, @vaults, @stores);
print "Writing Stores...\n";
dump_stores(">$outdir$filenames{store}{html}", @stores, @{ $defines{TV} });
print "Writing Monsters...\n";
dump_monsters(">$outdir$filenames{monster}{html}", @monsters, %effects_map, %blows_map);
print "Writing Terrain...\n";
dump_terrains(">$outdir$filenames{terrain}{html}", @terrains, %blows_map, %effects_map);
print "Writing Vaults...\n";
dump_vaults(">$outdir$filenames{vault}{html}", @vaults);
print "Writing Objects...\n";
dump_objects(">$outdir$filenames{object}{html}", @objects);
print "Writing Shop Owners...\n";
dump_shop_owners(">$outdir$filenames{shop_owner}{html}", @shop_owners, @stores);
print "Writing Rooms...\n";
dump_rooms(">$outdir$filenames{room}{html}", @rooms, @terrains);
print "Writing Blows...\n";
dump_blows(">$outdir$filenames{blow}{html}", @blows, %effects_map);
print "Writing Effects...\n";
dump_effects(">$outdir$filenames{effect}{html}", @effects);
print "Writing Regions...\n";
dump_regions(">$outdir$filenames{region}{html}", @regions, %blows_map);
print "Writing Spells...\n";
dump_spells(">$outdir$filenames{spell}{html}", @spells);

#TODO: probably should dump an index file at some point.
