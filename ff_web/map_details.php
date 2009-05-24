<?php

add_action("mapdetails", "map_details", array("map"=>"numeric"));
function map_details($params)
{
	if (empty($params['map']) || !is_numeric($params['map'])) {
		throw_error("no map passed");
		return;
	}

	$map = sql_query("
		select name, description, image, pointsvalid
		from maps
		where maps.id = $params[map]
	");
	
	if (count($map) != 1) {
		throw_error("Unknown round");
		return;
	}
	
	$map = $map[0];

?>
<h4><?=$map['name']?></h4>
<table>
<tr>
	<th rel="row">Name</th>
	<td><?=$map['name']?></td>
</tr>
<tr>
	<th rel="row">Description</th>
	<td><?=$map['description']?></td>
</tr>
<tr>
	<th rel="row">Image</th>
	<td><img src="<?=$map['image']?>" /></td>
</tr>
<tr>
	<th rel="row">Valid</th>
	<td><?=$map['pointsvalid']?"Points are valid on this map":"<span class=\"warning\">Warning: Points are not valid on this map</span>"?></td>
</tr>
</table>

<?
	// build winners
	$mapwinners = sql_query("
		SELECT count(winner) as wins, winner FROM rounds
		WHERE map = $params[map]
		GROUP by winner
		ORDER BY count(winner)
	");

	// build average scores by team
	$mapscores = sql_query("
		SELECT avg(bluescore) as blueavg, avg(redscore) as redavg,
		      avg(yellowscore) as yellowavg, avg(greenscore) as greenavg,
		      sum(unix_timestamp(endtime)-unix_timestamp(starttime)) timeplayed,
		      max(numteams) as numteams
		FROM rounds
		WHERE map = $params[map]
	");
	$mapscores = $mapscores[0];
	$timeplayed = $mapscores['timeplayed'];
	$highavgscore = max($mapscores['blueavg'], $mapscores['redavg'], $mapscores['yellowavg'], $mapscores['greenavg']); 
	
	// generate teams
	$teams = array_slice(array("blue", "red", "yellow", "green"),0,$mapscores['numteams']);
	$allteams = array();
	
	$totalwins = 0;
	foreach ($mapwinners as $winner) {
		$totalwins += $winner['wins'];
		$allteams[$winner['winner']] = true;
	}
	
	foreach ($teams as $team) {
		if (isset($allteams[$team])) continue;
		$mapwinners[] = array("wins"=>0, "winner"=>$team);
	}
?>
<h3>Wins by team:</h3>
<ul>
<? foreach ($mapwinners as $winner) { ?>
	<li><?=$winner['winner']?>: <?=$winner['wins']?> wins (<?=round(100*$winner['wins']/$totalwins, 2)?>%)</li>
<? } ?>
</ul>

<h3>Score by team:</h3>
<ul>
<? foreach ($teams as $team) { ?>
	<li><?=$team?>: <strong><?=round($mapscores[$team.'avg'], 2)?></strong> average points per game</li>
<? } ?>
</ul>


<?
	
}

?>
