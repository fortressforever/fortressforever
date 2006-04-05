<?

add_action("roundplayer", "round_player_details", array("round"=>"numeric", "player"=>"numeric"));

function round_player_details($params)
{
	if (empty($params['round']) || !is_numeric($params['round'])) {
		throw_error("no round passed");
		return;
	}
	
	if (empty($params['player']) || !is_numeric($params['player'])) {
		throw_error("no player passed");
		return;
	}
	
	$roundplayer = sql_query("
		select id
		from roundplayers
		where roundplayers.round = $params[round]
		and roundplayers.player = $params[player] 
	");
	
	if (count($roundplayer) == 0) {
		throw_error("Unknown round/player");
		return;
	}
	
	$roundplayer = $roundplayer[0];

?>
<h4>Round Details</h4>

<?
	player_weapon_details($params);
	player_action_details($params);
	player_kill_details($params);
}


function build_weapon_kills($params)
{
	$ret = array();
	$wkills = sql_query("
		SELECT weaponinfo.gameid, count(*) as c
		FROM kills
		LEFT JOIN weaponinfo ON kills.weapon = weaponinfo.id
		LEFT JOIN roundplayers ON kills.player = roundplayers.id
		WHERE roundplayers.player = '$params[player]'
		".(isset($params['round'])?" AND roundplayers.round = '$params[round]'":"")."
		GROUP BY roundplayers.player, weaponinfo.gameid, weaponinfo.name, weaponinfo.img
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
	
	$weapons = sql_query("SELECT gameid, name, img FROM weaponinfo");
	
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

function player_action_details($params)
{
	$actions = sql_query("
		select statinfo.name, sum(value) as value
		from statprogress
		left join roundplayers on roundplayers.id = statprogress.player
		left join statinfo on statinfo.id = statprogress.stat
		where roundplayers.round = $params[round]
		and roundplayers.player = $params[player]
		and statinfo.type = 'action'
  		group by statinfo.name
		order by value desc
	");
?>

<h4>Action Information</h4>
<div>
<table>
<tr>
	<th rel="col">Action</th>
	<th rel="col">Times Performed</th>
</tr>
<? foreach ($actions as $action) { ?>
	<tr>
		<td><?=$action['name']?></td>
		<td class="numeric"><?=$action['value']?></td>
	</tr>
<? } ?>
<? if (!count($actions)) { ?>
	<tr>
		<td colspan="5">No actions performed.</td>
	</tr>
<? } ?>
</table>
</div>

<?	

}

function player_kill_details($params)
{

	if ($params['type'] == "kills") {
		$kills = sql_query("
			select weaponinfo.img, weaponinfo.name as weaponname, pk.name as name, rp.class as class
			from kills
			left join roundplayers rp on rp.id = kills.player
			left join roundplayers rk on rk.id = kills.killed
			left join players pk on pk.id = rk.player
			left join weaponinfo on weaponinfo.id = kills.weapon
			where rp.player = $params[player]
			and rp.round = $params[round]
			and rp.player != rk.player
			order by kills.killed, kills.time
		");
		echo "<h2>Kills Breakdown</h2>";
		if (!count($kills)) {
			echo "No Kills!";
			return;
		}
	} else if ($params['type'] == "deaths") {
		$kills = sql_query("
			select weaponinfo.img, weaponinfo.name as weaponname, pp.name as name, rp.class as class
			from kills
			left join roundplayers rp on rp.id = kills.player
			left join roundplayers rk on rk.id = kills.killed
			left join players pp on pp.id = rp.player
			left join weaponinfo on weaponinfo.id = kills.weapon
			where rk.player = $params[player]
			and rk.round = $params[round]
			and rp.player != rk.player
			order by kills.killed, kills.time
		");		
		echo "<h2>Deaths Breakdown</h2>";
		if (!count($kills)) {
			echo "No Deaths!";
			return;
		}
	} else if ($params['type'] == "suicides") {
		$kills = sql_query("
			select weaponinfo.img, weaponinfo.name as weaponname, pp.name as name, rp.class as class
			from kills
			left join roundplayers rp on rp.id = kills.player
			left join roundplayers rk on rk.id = kills.killed
			left join players pp on pp.id = rp.player
			left join weaponinfo on weaponinfo.id = kills.weapon
			where rk.player = $params[player]
			and rk.round = $params[round]
			and rp.player = rk.player
			order by kills.killed, kills.time
		");
		echo "<h2>Suicides</h2>";
		if (!count($kills)) {
			echo "No Suicides!";
			return;
		}
	} else {
		$params['type'] = "kills"; player_kill_details($params);
		$params['type'] = "deaths"; player_kill_details($params);
		$params['type'] = "suicides"; player_kill_details($params);
		return;
	}
	
	$lastkill = "";
	foreach (split_by_field($kills, 'name') as $name=>$playerkills) {
		if ($params['type'] == "kills")
			echo "<div>".count($playerkills)." kills on $name</div>\n";
		if ($params['type'] == "deaths")
			echo "<div>".count($playerkills)." deaths from $name</div>\n";
		else if ($params['type'] == "suicides")
			echo "<div>".count($playerkills)." suicides</div>\n";
		foreach (split_by_field($playerkills, 'class') as $class=>$classkills) {
			echo "<span class=\"class${class}\">";
			foreach ($classkills as $kill) {
				echo "<img src=\"$kill[img]\" alt=\"$kill[weaponname] icon\" />";
			}
			echo "</span>\n";
		}
	}
}
?>
