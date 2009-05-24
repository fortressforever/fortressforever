<?

add_action("player", "player_details", array("player"=>"numeric"));

function player_details($params)
{
	if (empty($params['player']) || !is_numeric($params['player'])) {
		throw_error("no player passed");
		return;
	}
	
	$player = sql_query("
		select name, steamid, ffp.value as ffp
		from players
		left join playerstats ffp on players.id = ffp.player and ffp.stat = '$stat_ffpoints'
		where id = '$params[player]'
	");
	
	if (count($player) == 0) {
		throw_error("Unknown player");
		return;
	}
	
	$player = $player[0];

?>
<div class="playerframe">
<div class="playername"><?=$player['name']?></div>
<div class="playerip"><?=$player['steamid']?></div>
<div class="playerffp"><?=sprintf("%d", $player['ffp'])?> FFP Earned</div>
</div>

<h2>Class Breakdown</h2>
<?

	$stat_ffpoints = lookup_stat("ff_points");
	$stat_kills = lookup_stat("kills");
	$stat_deaths = lookup_stat("deaths");
	$stat_played = lookup_stat("time_played");
	
	$classstats = sql_query("
		select classes.name as name, ffp.value as ffp, kills.value as kills, deaths.value as deaths, played.value as time_played
		from classes
		left join playerclassstats ffp on ffp.player = '$params[player]' and classes.id = ffp.class and ffp.stat = '$stat_ffpoints'
		left join playerclassstats kills on kills.player = '$params[player]' and classes.id = kills.class and kills.stat = '$stat_kills'
		left join playerclassstats deaths on deaths.player = '$params[player]' and classes.id = deaths.class and deaths.stat = '$stat_deaths'
		left join playerclassstats played on played.player = '$params[player]' and classes.id = played.class and played.stat = '$stat_played'
		order by classes.id
	");
	
	foreach ($classstats as $class) {
?>
<div class="playerclassframe">
<img src="" class="playerclassimage" />
<div class="playerclassname"><?=$class['name']?></div>
<div class="playerclassffp"><?=sprintf("%d", $class['ffp'])?> FFP Earned</div>
<div class="playerclasskd"><?=sprintf("Kill/Death: %d / %d (%.2f ratio)", $class['kills'], $class['deaths'], $class['deaths']?$class['kills']/$class['deaths']:0)?></div>
<div class="playerclassplayed"><?=h_m_s_format($class['time_played'])?> play time</div>
</div>
<?
	}
}

?>
