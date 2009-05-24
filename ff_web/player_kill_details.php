<?php

add_action("playerkilldetails", "player_kill_details", array("player"=>"numeric", "round"=>"numeric"));

function player_kill_details($params)
{

	if ($params['type'] == "kills") {
		$kills = sql_query("
			select actioninfo.img, actioninfo.name as weaponname, pk.name as name, rp.class as class
			from actions
			left join roundplayers rp on rp.id = actions.roundplayer
			left join roundplayers rk on rk.id = actions.target
			left join players pk on pk.id = rk.player
			left join actioninfo on actioninfo.id = actions.action
			where rp.player = $params[player]
			and rp.round = $params[round]
			and rp.player != rk.player
			and actioninfo.type = 'kill'
			order by actions.target, actions.time
		");
		echo "<h2>Kills Breakdown</h2>";
		if (!count($kills)) {
			echo "No Kills!";
			return;
		}
	} else if ($params['type'] == "deaths") {
		$kills = sql_query("
			select actioninfo.img, actioninfo.name as weaponname, pk.name as name, rp.class as class
			from actions
			left join roundplayers rp on rp.id = actions.roundplayer
			left join roundplayers rk on rk.id = actions.target
			left join players pk on pk.id = rp.player
			left join actioninfo on actioninfo.id = actions.action
			where rk.player = $params[player]
			and rk.round = $params[round]
			and rp.player != rk.player
			and actioninfo.type = 'kill'
			order by actions.target, actions.time
		");		
		echo "<h2>Deaths Breakdown</h2>";
		if (!count($kills)) {
			echo "No Deaths!";
			return;
		}
	} else if ($params['type'] == "suicides") {
		$kills = sql_query("
			select actioninfo.img, actioninfo.name as weaponname, pk.name as name, rp.class as class
			from actions
			left join roundplayers rp on rp.id = actions.roundplayer
			left join roundplayers rk on rk.id = actions.target
			left join players pk on pk.id = rk.player
			left join actioninfo on actioninfo.id = actions.action
			where rp.player = $params[player]
			and rp.round = $params[round]
			and rp.player = rk.player
			and actioninfo.type = 'kill'
			order by actions.target, actions.time
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
