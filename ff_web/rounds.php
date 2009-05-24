<?php

add_action("recentrounds", "recent_rounds", array("server"=>"numeric", "limit"=>"numeric"));
add_action("rounddetails", "round_details", array("round"=>"numeric"));

function recent_rounds($params)
{
	$limit = isset($params['limit']) ? $params['limit'] : 25;

	$recentrounds = sql_query("
		select rounds.id, maps.id as mapid, maps.name as map, starttime,
		unix_timestamp(endtime)-unix_timestamp(starttime) as duration,
			bluescore, redscore, yellowscore, greenscore, numteams
		from rounds
		left join maps on rounds.map = maps.id
		where 1=1
		".( isset($params['server']) ? " and server = $params[server]" : "" )."
		order by starttime desc
		limit $limit
	");

?>

<h4>Recent Matches</h4>
<div>
<table>
<tr>
	<th rel="col">Time</th>
	<th rel="col">Map</th>
	<th rel="col">Duration</th>
	<th rel="col">Score</th>
	<th rel="col">Detail</th>
</tr>
<? foreach ($recentrounds as $round) { ?>
	<tr>
		<td><?=date("j-M-y g:ia", strtotime($round['starttime']))?></td>
		<td><a href="?a=mapdetails&map=<?=$round['mapid']?>"><?=$round['map']?></a></td>
		<td><?=h_m_s_format($round['duration'])?></td>
		<td>
			<?if ($round['numteams']>=1){?><span class="redteam"><?=$round['redscore']?></span><?}?>
			<?if ($round['numteams']>=2){?><span class="blueteam"><?=$round['bluescore']?></span><?}?>
			<?if ($round['numteams']>=3){?><span class="yellowteam"><?=$round['yellowscore']?></span><?}?>
			<?if ($round['numteams']>=4){?><span class="greenteam"><?=$round['greenscore']?></span><?}?>
		</td>
		<td><a href="?a=rounddetails&round=<?=$round['id']?>">Details</a></td>
	</tr>
<? } ?>
<? if (!count($recentrounds)) { ?>
	<tr>
		<td colspan="5">No recent rounds played</td>
	</tr>
<? } ?>
</table>
</div>

<?
}


function round_details($params)
{
	if (empty($params['round']) || !is_numeric($params['round'])) {
		throw_error("no round passed");
		return;
	}
	
	$round = sql_query("
		select rounds.id, maps.name as mapname, maps.image as mapimage, maps.id as mapid,
			starttime, unix_timestamp(endtime)-unix_timestamp(starttime) as duration,
			bluescore, redscore, yellowscore, greenscore, servers.name as servername, servers.id as serverid
		from rounds
		left join maps on rounds.map = maps.id
		left join servers on rounds.server = servers.id
		where rounds.id = $params[round]
	");
	
	if (count($round) != 1) {
		throw_error("Unknown round");
		return;
	}
	
	$round = $round[0];

?>
<h4>Round Details</h4>
<div>
<table>
<tr>
	<th rel="row">Server</th>
	<td><a href="?a=serverdetails&server=<?=$round['serverid']?>"><?=$round['servername']?></a></td>
</tr>
<tr>
	<th rel="row">Time</th>
	<td><?=date("j-M-y g:ia", strtotime($round['starttime']))?></td>
</tr>
<tr>
	<th rel="row">Duration</th>
	<td><?=h_m_s_format($round['duration'])?></td>
</tr>
<tr>
	<th rel="row">Map</th>
	<td><a href="?a=mapdetails&map=<?=$round['mapid']?>"><img src="<?=$round['mapimage']?>" alt="<?=$round['mapname']?>" /><br /><?=$round['mapname']?></a></td>
</tr>
</table>
</div>

<?

	round_players(array("round"=>$params['round']));
	//round_ribbons(array("round"=>$params['round']));
}


function round_players($params)
{
	if (empty($params['round']) || !is_numeric($params['round'])) {
		throw_error("no round passed");
		return;
	}
	
	$players = sql_query("
		select players.name as playername, classes.name as class, roundplayers.id as id,
			players.id as playerid
		from roundplayers
		left join players on roundplayers.player = players.id
		left join classes on roundplayers.class = classes.id
		where roundplayers.round = $params[round]
	");
	
	$p = array();
	foreach ($players as $player) {
		if (!isset($p[$player['playerid']])) {
			$p[$player['playerid']] = array(
				"id" => $player['playerid'],
				"playername" => $player['playername'],
				"classes" => array(),
			);
		}
		//$p[$player['playerid']]['classes'][] = array('class'=>$player['class'], 'id'=>$player['id']);
		$p[$player['playerid']]['classes'][$player['class']] = true;
	}
	
?>
<h4>Players</h4>
<div>
<table>
<tr>
	<th rel="col">Player</th>
	<th rel="col">Class</th>
	<th rel="col">Kills</th>
	<th rel="col">Deaths</th>
	<th rel="col">Suicides</th>
	<th rel="col">TK</th>
	<th rel="col">Time</th>
</tr>
<? foreach ($p as $player) { ?>
	<tr>
		<td><a href="?a=roundplayer&player=<?=$player['id']?>&round=<?=$params['round']?>"><?=$player['playername']?></a></td>
		<td><? $c = array(); foreach ($player['classes'] as $class=>$value) { $c[] = $class; } echo implode($c, "/"); ?></td>
		<td><?=get_round_stat($params['round'], $player['id'], "kills")?></td>
		<td><?=get_round_stat($params['round'], $player['id'], "deaths")?></td>
		<td><?=get_round_stat($params['round'], $player['id'], "suicides")?></td>
		<td><?=get_round_stat($params['round'], $player['id'], "teamkills")?></td>
		<td><?=h_m_s_format(get_round_stat($params['round'], $player['id'], "time_played"))?></td>
	</tr>
<? } ?>
<? if (!count($players)) { ?>
	<tr>
		<td colspan="5">No players found.</td>
	</tr>
<? } ?>
</table>
</div>

<?

}


function round_ribbons($params)
{
	if (empty($params['round']) || !is_numeric($params['round'])) {
		throw_error("no round passed");
		return;
	}
	
	$ribbons = sql_query("
		select players.name as playername, ribboninfo.name as ribbonname
		from ribbons
		left join players on ribbons.player = players.id
		left join rounds on ribbons.round = rounds.id
		left join ribboninfo on ribbons.ribbon = ribboninfo.id
		where ribbons.round = $params[round]
	");
	
?>
<h4>Ribbons Awarded</h4>
<div>
<table>
<tr>
	<th rel="col">Player</th>
	<th rel="col">Ribbon</th>
</tr>
<? foreach ($ribbons as $ribbon) { ?>
	<tr>
		<td><?=$ribbon['playername']?></td>
		<td><?=$ribbon['ribbonname']?></td>
	</tr>
<? } ?>
<? if (!count($ribbons)) { ?>
	<tr>
		<td colspan="5">No ribbons awarded.</td>
	</tr>
<? } ?>
</table>
</div>

<?

}
?>
