<?php

add_action("playermostactive", "player_most_active", array("server"=>"numeric"));

function player_most_active($params) {
	if (empty($params['offset']))
		$params['offset'] = 0;
	if (empty($params['count']))
		$params['count'] = 10;
		
	$stat_ffpoints = lookup_stat("ff_points");
	$stat_kills = lookup_stat("kills");
	$stat_deaths = lookup_stat("deaths");
	
	if (isset($params['server'])) {
		$players = sql_query("
			select sum(statprogress.value) as seconds, roundplayers.player as player, players.name as name,
				players.id as id
			from statprogress
			left join statinfo on statinfo.id = statprogress.stat
			left join roundplayers on roundplayers.id = statprogress.player
			left join rounds on rounds.id = roundplayers.round
			left join players on players.id = roundplayers.player
			where statinfo.gameid = 'time_played'
			and rounds.server = '$params[server]'
			group by roundplayers.player
			order by sum(statprogress.value) desc
			limit $params[offset], $params[count]
		");
	} else {
		$players = sql_query("
			select sum(statprogress.value) as seconds, roundplayers.player as player, players.name as name,
				ffp.value as ffp, kills.value as kills, deaths.value as deaths
			from statprogress
			left join statinfo on statinfo.id = statprogress.stat
			left join roundplayers on roundplayers.id = statprogress.player
			left join rounds on rounds.id = roundplayers.round
			left join players on players.id = roundplayers.player
			left join playerstats ffp on players.id = ffp.player and ffp.stat = '$stat_ffpoints'
			left join playerstats kills on players.id = kills.player and kills.stat = '$stat_kills'
			left join playerstats deaths on players.id = deaths.player and deaths.stat = '$stat_deaths'
			where statinfo.gameid = 'time_played'
			group by roundplayers.player
			order by sum(statprogress.value) desc
			limit $params[offset], $params[count]
		");	
	}
?>
<h4>Most Active Players:</h4>
<table>
<tr>
	<th rel="col">Player</th>
	<th rel="col">Time Played</th>
	<th rel="col">FFP</th>
	<th rel="col">Kills</th>
	<th rel="col">Deaths</th>
</tr>
<? foreach ($players as $player) { ?>
	<tr>
		<td><a href="?a=player&player=<?=$player['player']?>"><?=$player['name']?></a></td>
		<td><?=h_m_s_format($player['seconds'])?></td>		
		<td><?=$player['ffp']?></td>
		<td><?=$player['kills']?></td>
		<td><?=$player['deaths']?></td>
	</tr>
<? } ?>
</table>
<?
}

?>
