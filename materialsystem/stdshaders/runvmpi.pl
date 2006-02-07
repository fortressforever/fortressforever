$cwd = Win32::GetFullPathName(".");

$changeToDir = "";
$noMPI = "";
$xboxFlag = "";
$gameFlag = "";

while( 1 )
{
	$argname = shift;

	if( $argname =~ m/-xbox/ )
	{
		# parse it out, to be used later
		$xboxFlag = "-xbox";
	}
	elsif( $argname =~ m/-changetodir/ )
	{
		$changeToDir = shift;
	}
	elsif( $argname =~ m/-game/ )
	{
		$gameFlag = "-game \"" . (shift) . "\"";
	}
	elsif( $argname =~ m/-nompi/ )
	{
		$noMPI = "-nompi";
	}
	else
	{
		last;
	}
}

if( !stat "filelist.txt" || !stat "uniquefilestocopy.txt" )
{
	exit;
}

$shaderpath = $cwd;
$shaderpath =~ s,/,\\,g;
chdir $changeToDir;

$cmdToRun = "shadercompile.exe $noMPI $xboxFlag -shaderpath $shaderpath -mpi_workercount 32 -allowdebug $gameFlag";
system $cmdToRun;

# other options..
#system "shadercompile.exe -verbose -shaderpath $shaderpath -mpi_workercount 32 -allowdebug -mpi_pw apstest -mpi_showappwindow";
