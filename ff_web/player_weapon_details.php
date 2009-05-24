<?php

add_action("playerweapondetails", "player_weapon_details", array("player"=>"numeric", "round"=>"numeric"));

function build_weapon_kills($params)
{
	$ret = array();
	$wkills = sql_query("
		SELECT actioninfo.gameid, count(*) as c
		FROM actions
		LEFT JOIN actioninfo ON actions.action = actioninfo.id
		LEFT JOIN roundplayers ON actions.roundplayer = roundplayers.id
		WHERE roundplayers.player = '$params[player]'
		".(isset($params['round'])?" AND roundplayers.round = '$params[round]'":"")."
		AND actioninfo.type = 'kill'
		GROUP BY roundplayers.player, actioninfo.gameid, actioninfo.name, actioninfo.img
		ORDER BY roundplayers.player, c desc
	");
	
	foreach ($wkills as $w) {
		$ret[$w['gameid']] = $w['c'];
	}
	return $ret;
}

function player_weapon_details($params)
{
	$wkills = build_weapon_kills(array('player'=>$params['player'], 'round'=>$params['round']));
	$lt_wkills = build_weapon_kills(array('player'=>$params['player']));
	
	$weapons = sql_query("SELECT gameid, name, img FROM actioninfo where actioninfo.type = 'kill'");
	
	$total_fired = $total_hit = $total_kills = $total_lt_fired = $total_lt_hit = $total_lifetime_kills = 0;
	foreach ($weapons as $i=>$weapon) {
		$weapons[$i]['hit'] = $hit = get_round_stat($params['round'], $params['player'], "hit_".$weapon['gameid']);
		$weapons[$i]['fired'] = $fired = get_round_stat($params['round'], $params['player'], "fired_".$weapon['gameid']);
		$weapons[$i]['kills'] = $kills = isset($wkills[$weapon['gameid']])?$wkills[$weapon['gameid']]:0;
		$weapons[$i]['lifetime_hit'] = $lifetime_hit = get_player_stat($params['round'], $params['player'], "hit_".$weapon['gameid']);
		$weapons[$i]['lifetime_fired'] = $lifetime_fired = get_player_stat($params['round'], $params['player'], "fired_".$weapon['gameid']);
		$weapons[$i]['lifetime_kills'] = $lifetime_kills = isset($lt_wkills[$weapon['gameid']])?$lt_wkills[$weapon['gameid']]:0;
		if (!$fired) { unset($weapons[$i]); continue; }
		$weapons[$i]['accuracy'] = 100*$hit/$fired;
		$weapons[$i]['lifetime_accuracy'] = 100*$lifetime_hit/$lifetime_fired;
		$total_hit += $hit;
		$total_fired += $fired;
		$total_kills += $kills;
		$total_lifetime_hit += $lifetime_hit;
		$total_lifetime_fired += $lifetime_fired;
		$total_lifetime_kills += $lifetime_kills;
	}
	$total_accuracy = 100 * $total_hit / $total_fired;
	$total_lifetime_accuracy = 100 * $total_lifetime_hit / $total_lifetime_fired;
?>

<h4>Weapon Information</h4>
<div>
<table>
<tr>
	<th rel="col" colspan="2" rowspan="2">Weapon</th>
	<th rel="col" colspan="4">Match</th>
	<th rel="col" colspan="4">Lifetime</th>
</tr>
<tr>
	<th rel="col" colspan="2">Accuracy</th>
	<th rel="col" colspan="2">Kills</th>
	<th rel="col" colspan="2">Accuracy</th>
	<th rel="col" colspan="2">Kills</th>
</tr>
<? foreach ($weapons as $weapon) { ?>
	<tr>
		<td><img src="<?=$weapon['img']?>" alt="<?=$weapon['name']?> icon"/></td>
		<td><?=$weapon['name']?></td>
		<td class="numeric"><?=$weapon['hit']?>/<?=$weapon['fired']?></td>
		<td><?=sprintf("%.2f", $weapon['accuracy'])?>%</td>
		<td class="numeric"><?=$weapon['kills']?></td>
		<td class="numeric"><?=$total_kills&&$weapon['kills']?sprintf("%.2f%%",100*$weapon['kills']/$total_kills):"&nbsp;"?></td>
		<td class="numeric"><?=$weapon['lifetime_hit']?>/<?=$weapon['lifetime_fired']?></td>
		<td><?=sprintf("%.2f", $weapon['lifetime_accuracy'])?>%</td>
		<td class="numeric"><?=$weapon['lifetime_kills']?></td>
		<td class="numeric"><?=$total_lifetime_kills&&$weapon['lifetime_kills']?sprintf("%.2f%%",100*$weapon['lifetime_kills']/$total_lifetime_kills):"&nbsp;"?></td>
	</tr>
<? } ?>
<? if (!count($weapons)) { ?>
	<tr>
		<td colspan="5">No shots fired.</td>
	</tr>
<? } else { ?>
	<tr>
		<th colspan="2" rel="row">Total:</th>
		<td class="numeric"><?=$total_hit?>/<?=$total_fired?></td>
		<td><?=sprintf("%.2f", $total_accuracy)?>%</td>
		<td class="numeric"><?=$total_kills?></td>
		<td class="numeric">100%</td>
		<td class="numeric"><?=$total_lifetime_hit?>/<?=$total_lifetime_fired?></td>
		<td><?=sprintf("%.2f", $total_lifetime_accuracy)?>%</td>
		<td class="numeric"><?=$total_lifetime_kills?></td>
		<td class="numeric">100%</td>
	</tr>
<? } ?>
</table>
</div>

<?
}

?>
