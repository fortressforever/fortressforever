#!/usr/bin/perl
#

undef $/;

while(<>)
{
	$line = $_;

	$line =~ s/ExcludedFromBuild="TRUE"/ExcludedFromBuild="FALSE"/gs;
	
	print $line;
}
