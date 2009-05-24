<?php

add_action("parserequest", "parse_request", array("data"=>"string"));
add_action("parserequestform", "parse_request_form", array());

function parse_request_form($params)
{
?>
<form action="index.php" method="post">
<input type="hidden" name="a" value="parserequest" />
<textarea rows="8" cols="60" name="data"></textarea><br />
<input type="submit" value="submit" />
</form>
<?
}

function parse_request($params)
{
	if (empty($params['data'])) {
		throw_error("no data passed");
		return;
	}
	
	// create the structure
	$game = array();
	$mode = "";
	
	foreach (explode("\n", $params['data']) as $line) {
		if (strlen($line) < 4) continue;
		$fields = explode(" ", trim($line));
		if (!count($fields)) continue;
		
		// must be a mode switch if it's 1 field
		if (count($fields) == 1) {
			if ($fields[0] == "players")
				$mode = "players";
			else if ($fields[0] == "actions")
				$mode = "actions";
			else if ($fields[0] == "stats")
				$mode = "stats";
			else
				{ throw_error("unknown mode passed ($fields[0])!"); return; }
			continue;
		}
		
		if ($mode == "") {
			if ($fields[0] == "login" && count($fields)==2) {
				$game['login'] = $fields[1];
				
				// find the server
 				$server = sql_query("select id from servers where loginname = '$game[login]'");
 				if (!count($server))
					{ throw_error("server login not found ($game[login])"); return; }
				$game['serverid'] = $server[0]['id'];
			} else if ($fields[0] == "auth" && count($fields)==2) {
				$game['auth'] = $fields[1];
			} else if ($fields[0] == "map" && count($fields)==2) {
				$game['map'] = $fields[1];
				
			 	// find the map
			 	$map = sql_query("select id from maps where name = '$game[map]'");
			 	if (!count($map))
			 		{ throw_error("map not found ($game[map])"); return; }
			 	$game['mapid'] = $map[0]['id'];
				 	 				
			} else if ($fields[0] == "date") {
				$game['startdate'] = strtotime(implode(" ", array_slice($fields, 1)));
				$game['starttime'] = date("Y-m-d G:i:s", $game['startdate']);				
			} else if ($fields[0] == "duration" && count($fields)==2) {
				$game['duration'] = $fields[1];
				$game['enddate'] = $game['startdate'] + $fields[1];
				$game['endtime'] = date("Y-m-d G:i:s", $game['enddate']);
			} else if (preg_match("/(blue|red|yellow|green)score/", $fields[0], $regs) && count($fields)==2) {
				$game[$fields[0]] = $fields[1];
				if (!isset($game['highscore']) || $fields[1]>$game['highscore']) {
					$game['highscore'] = $fields[1];
					$game['winner'] = $regs[1];
				}						
			} else if ($fields[0] == "numteams" && count($fields)==2) {
				$game['numteams'] = $fields[1];
			}
		} else if ($mode == "players") {
			if (count($fields) != 4) {
				throw_error("invalid players row ($line)!");
				return;
			}

			// look them up in the db			
			$p = sql_query("select id from players where steamid = '$fields[0]'");
			if (count($p)) {
 				$playerid = $p[0]['id'];
	 		} else {
	 			sql_query("
				 	insert into players
				 	(steamid, name)
				 	values
				 	('$fields[0]', '$fields[1]')
				");
				$p = sql_query("select last_insert_id()");
	 			$playerid = $p[0][0];
	 		}			
			
			//STEAM:0:1234 FryGuy 1
			$game['players'][] = array(
				'steamid' => $fields[0],
				'name' => $fields[1],
				'team' => $fields[2],
				'class' => $fields[3],
				'playerid' => $playerid,
			);
		} else if ($mode == "actions") {
			if (count($fields) != 7) {
				throw_error("invalid action row ($line)!");
				return;
			}

			//0 1 ff_weapon_nailgun 14		
			if (!isset($game['players'][$fields[0]]))
				{ throw_error("invalid player on kill ($line)"); return; }
			//if (!isset($game['players'][$fields[1]]))
			//	{ throw_error("invalid killed on kill ($line)"); return; }
			
			$action = lookup_action($fields[2]);
			if (!$action)
				{ throw_error("invalid action ($fields[2])"); return; }
								
			$game['actions'][] = array(
				'player' => $fields[0],
				'target' => $fields[1],
				'action' => $fields[2],
				'actionid' => $action,
				'time' => date("Y-m-d G:i:s", $game['startdate'] + $fields[3]),
				'param' => $fields[4],
				'coords' => $fields[5],
				'location' => $fields[6]
			);
		} else if ($mode == "stats") {
			//0 time_played 97
			if (count($fields) != 3) {
				throw_error("invalid stats row ($line)!");
				return;
			}

			//0 1 ff_weapon_nailgun 14		
			if (!isset($game['players'][$fields[0]]))
				{ throw_error("invalid player on stat ($line)"); return; }

			$statid = lookup_stat($fields[1]);
			if (!$statid)
				{ throw_error("unknown stat ($fields[1])"); return; }

			$game['stats'][] = array(
				'player' => $fields[0],
				'statname' => $fields[1],
				'id' => $statid,
				'value' => $fields[2],
			);			
		}
 	}

 	// check authentication
 	$auth = "XXXXXXXX";
 	if ($game['auth'] != $auth)
 		{ throw_error("invalid authentication"); return; }

	// start the thingy
	sql_query("begin");
 	
 	// insert the game into the db, now that everything's settled
 	sql_query("
		insert into rounds
		(server, map, starttime, endtime, bluescore, redscore, yellowscore, greenscore, numteams, winner)
		values
		('$game[serverid]', '$game[mapid]', '$game[starttime]', '$game[endtime]', '$game[bluescore]', '$game[redscore]', '$game[yellowscore]', '$game[greenscore]', '$game[numteams]', '$game[winner]')
 	");
	$id = sql_query("select last_insert_id()");
	$game['round'] = $id[0][0];
 	
 	// insert the players to roundplayers
	foreach ($game['players'] as $i=>$player) {	
 		sql_query("
 			insert into roundplayers
 			(player, round, class, team)
 			values
 			('$player[playerid]', '$game[round]', '$player[class]', '$player[team]')
 		");
 		$id = sql_query("select last_insert_id()");
 		$game['players'][$i]['id'] = $id[0][0];
 	}
 	
 	// insert the actions
	foreach ($game['actions'] as $i=>$action) {
		$playerid = $game['players'][$action['player']]['id'];
		$targetid = $game['players'][$action['target']]['id'];
 		sql_query("
 			insert into actions
 			(roundplayer, target, action, time, param, coords, location)
 			values
 			('$playerid', '$targetid', '$action[actionid]', '$action[time]', '$action[param]', '$action[coords]', '$action[location]')
 		");
 	}

 	// insert the stats
	foreach ($game['stats'] as $i=>$stat) {
		$playerid = $game['players'][$stat['player']]['id'];
 		sql_query("
 			insert into statprogress
 			(stat, value, player)
 			values
 			('$stat[id]', '$stat[value]', '$playerid')
 		");
 	}
 	
 	sql_query("commit");

 	throw_message("Success!", "Round imported correctly");
 	
 	run_action("parserequestform", array());
}

?>
