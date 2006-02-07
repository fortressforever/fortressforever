#!/usr/bin/perl
#

use File::Find;

# store stuff here
my @files;
my @dirs;

# start where we were called
$dir = "./";

# recursive dir search
find( \&populatedirs, $dir );

@dirs = reverse(@dirs);

# rename directories that we found
foreach (@dirs)
{
	$oldpath = $newpath = $_;
	$newpath =~ s/^(.*)sdk/\1ff/;
	
	print "DIR: $oldpath -> $newpath\n";
	rename($oldpath, $newpath) or die "Error renaming\n";
}

# recursive dir search
find( \&populatefiles, $dir );

# rename files that we found
foreach (@files)
{
	$oldpath = $newpath = $_;
	$newpath =~ s/sdk/ff/gs;
	
	print "FILE: $oldpath -> $newpath\n";
	rename($oldpath, $newpath) or die "Error renaming\n";
}

# for each dir we find
sub populatedirs()
{
	# store dir name
	if (!-f and /sdk/)
	{
		push(@dirs, $File::Find::name);
	}
}

# for each file we find
sub populatefiles()
{
	# store file name
	if (-f and /sdk/)
	{
		push (@files, $File::Find::name);
	}
}