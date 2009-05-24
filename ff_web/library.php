<?php

function throw_message($title, $message)
{
?>
<div>
	<div class="t_header"><?=$title?></div>
	<div class="t_body"><?=$message?></div>
</div>
<?
}
function throw_error($error) { return throw_message("Error!", $error); }
function h_m_s_format($duration)
{
	$hours = floor($duration / 3600);
	$minutes = floor(($duration%3600)/60);
	$seconds = $duration%60;
	if ($hours > 0)
		return sprintf("%d:%02d:%02d", $hours, $minutes, $seconds);
	return sprintf("%d:%02d", $minutes, $seconds);
}
function lookup_stat($stat)
{
	static $stats;
	if (!isset($stats)) {
		$stats = array();
		$sq = sql_query("SELECT gameid, id FROM statinfo");
		foreach ($sq as $s) {
			$stats[$s['id']] = $s['gameid'];
			$stats[$s['gameid']] = $s['id'];
		}  
	}
	return $stats[$stat];
}
function lookup_action($action)
{
	static $weapons;
	if (!isset($weapons)) {
		$actions = array();
		$sq = sql_query("SELECT gameid, id FROM actioninfo");
		foreach ($sq as $s) {
			$actions[$s['id']] = $s['gameid'];
			$actions[$s['gameid']] = $s['id'];
		}  
	}
	return $actions[$action];
}

function get_round_stat($round, $player, $stat)
{
	if (!empty($round))
		return get_stat($player, $stat, 'round', array('round'=>$round));
	else
		return get_stat($player, $stat, 'player');
}
function get_player_stat($round, $player, $stat)
{
	return get_stat($player, $stat, 'player');
}

function get_stat($player, $stat, $type, $params=array())
{
	static $stats = array();
	
	if ($type == "round" || $type == "roundplayer")
		$id = "round_".$params['round'];
	else if ($type == "player")
		$id = "player";
	
	if (!isset($stats[$id])) {
		$stats[$id] = array();
		$sq = sql_query("
			select roundplayers.player, stat, sum(value) as value
			from statprogress
			left join roundplayers on roundplayers.id = statprogress.player
			where 1=1 
			".($type=="round"||$type=="roundplayers"?" and roundplayers.round = $params[round]":"")."
			".($type=="player"||$type=="roundplayers"?" and roundplayers.player = $player":"")."
			group by roundplayers.player, stat
		");
		foreach ($sq as $s) {
			$stats[$id][$s['player']][$s['stat']] = $s['value'];
		}
		//print_r($stats);
	}
	if (!is_numeric($stat)) $stat = lookup_stat($stat);
	
	return isset($stats[$id][$player][$stat]) ? $stats[$id][$player][$stat] : 0;
}

function split_by_field($results, $field)
{
	$ret = array();
	foreach ($results as $result)
	{
		if (!isset($last))
			$cur = array();
		else if ($last != $result[$field]) {
			$ret[$last] = $cur;
			$cur = array();
		}
		$cur[] = $result;
		$last = $result[$field];
	}
	if (isset($cur))
		$ret[$last] = $cur;
	return $ret;
}

?>
