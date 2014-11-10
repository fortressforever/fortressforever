param(
    [string]$new_dll = ''
)

#1>new_dll = d:\fortressforever_myfork\dlls\Debug FF\2005\server.dll

$steam_dir = (Get-ItemProperty HKCU:\Software\Valve\Steam).SteamPath
$ff_dir = "\SteamApps\common\Fortress Forever\FortressForever\bin"

$target_dir = $steam_dir + $ff_dir
if (Test-Path $target_dir)
{
    cp $new_dll $target_dir -Force
}


#pause #for debugging