<pre>
<?php

require_once "app_sql.php";

$locations = array("spiral", "redr");

$weapons = array(
	1 => array("ff_grenade_caltrop", "ff_weapon_crowbar", "ff_weapon_shotgun", "ff_weapon_nailgun"),
	2 => array("ff_grenade_frag", "ff_weapon_crowbar", "ff_weapon_sniperrifle", "ff_weapon_autorifle", "ff_weapon_nailgun", "ff_weapon_radiotagrifle"),
	3 => array("ff_grenade_frag", "ff_grenade_nail", "ff_weapon_crowbar", "ff_weapon_shotgun", "ff_weapon_supershotgun", "ff_weapon_rpg"),
	4 => array("ff_grenade_frag", "ff_grenade_mirv", "ff_weapon_crowbar", "ff_weapon_shotgun", "ff_weapon_pipelauncher", "ff_weapon_grenadelauncher", "ff_detpack"),
	5 => array("ff_grenade_frag", "ff_weapon_medkit", "ff_weapon_shotgun", "ff_weapon_supershotgun", "ff_weapon_supernailgun"),
	6 => array("ff_grenade_frag", "ff_grenade_mirv", "ff_weapon_crowbar", "ff_weapon_shotgun", "ff_weapon_supershotgun", "ff_weapon_assaultcannon"),
	7 => array("ff_grenade_frag", "ff_grenade_napalm", "ff_weapon_crowbar", "ff_weapon_shotgun", "ff_weapon_flamethrower", "ff_weapon_ic"),
	8 => array("ff_grenade_frag", "ff_grenade_gas", "ff_weapon_knife", "ff_weapon_tranquiliser", "ff_weapon_supershotgun", "ff_weapon_nailgun"),
	9 => array("ff_grenade_frag", "ff_grenade_emp", "ff_weapon_spanner", "ff_weapon_railgun", "ff_weapon_supershotgun", "ff_dispenser", "ff_sentrygun"),
);

function get_new_player($players) {
	while (true) {
		if (rand(0,100) < 20) {
			// find someone that exists already
			$player = sql_query("
				select steamid, name from players
				ORDER BY rand()
				LIMIT 1
			");
			if (!count($player)) continue;
			$player = $player[0];
			foreach ($players as $p) {
				if ($p['steamid'] == $player['steamid'])
					continue;
			}
			unset($player[0]); unset($player[1]);
			return $player;
		} else {
			$pattern = "1234567890abcdefghijklmnopqrstuvwxyz";
			$steamid = "STEAM:" . rand(0,1) . ":" . rand(1000,999999); 
			$name = ""; $r=rand(6,10); for ($i=0; $i<$r; $i++) $name .= $pattern[rand(0,strlen($pattern)-1)];
			//echo "name: $name\n"; 
			return array('steamid'=>$steamid, 'name'=>$name);
		}
	}
}

function get_player($players, $team = -1) {
	$ct = 0;
	while ($ct++ < 100) {
		$r = rand(0,count($players));
		if (!$players[$r]['ingame']) continue;
		if ($team != -1 && $players[$r]['team'] != $team) continue;
		return $r;
	}
	return -1;
}

$players = array();
$stats = array();
$actions = "";
$score = array(0,0);
$flagnames = array("red_flag", "blue_flag");
$flags = array("", "");
$flagdist = array(0, 0);

$r=rand(5,10);
for ($i=0;$i<$r;$i++) {
	$player = get_new_player($players);
	$player['team'] = rand(1,2);
	$player['class'] = rand(1,9);
	$player['ingame'] = true;
	$player['starttime'] = 0;
	$players[] = $player;
}
$time = 0;
while ($time < 30*60) {
	$r = rand(0, 100);
	if ($r<5) {
		// someone changed team
		$rp = get_player($players);
		if ($rp == -1) continue;
		$player = $players[$rp];
		$player['team'] = 3-$player['team']; // switch teams
		$player['starttime'] = $time;
		$players[] = $player;
		$players[$rp]['ingame'] = false;
		$players[$rp]['endtime'] = $time;
	} else if ($r<10) {
		// someone changed class
		$rp = get_player($players);
		if ($rp == -1) continue;
		$player = $players[$rp];
		do { $rc = rand(1,9); } while ($rc == $player['class']);
		$player['class'] = $rc;
		$player['starttime'] = $time;
		$players[] = $player;
		$players[$rp]['ingame'] = false;
		$players[$rp]['endtime'] = $time;
	} else if ($r<15) {
		// someone left server
		$rp = get_player($players);
		if ($rp == -1) continue;
		$players[$rp]['ingame'] = false;
		$players[$rp]['endtime'] = $time;
	} else if ($r<20) {
		// someone joined server
		$player = get_new_player($players);
		$player['team'] = rand(1,2);
		$player['class'] = rand(1,9);
		$player['ingame'] = true;
		$player['starttime'] = 0;
		$players[] = $player;
	} else if ($r<60) {
		// kill
		$rp = get_player($players);
		if ($rp == -1) continue;
		do { $rk = get_player($players); } while ($rp==$rk);
		$hit = rand(5,10); $fired = $hit+rand(5,10);
		$rw = $weapons[$players[$rp]['class']][rand(0, count($weapons[$players[$rp]['class']])-1)];
		$location = $locations[rand(0, count($locations)-1)];
		$coords = rand(-1000,1000).",".rand(-1000,1000).",".rand(-1000,1000);
		$param = "";
		$actions .= "$rp $rk $rw $time $param $coords $location\n";
		$stats[$rp]['kills']++;
		$stats[$rk]['deaths']++;
		$stats[$rp]["fired_".$rw] += $fired;
		$stats[$rp]["hit_".$rw] += $hit;
	} else {
		$coords = rand(-1000,1000).",".rand(-1000,1000).",".rand(-1000,1000);
		$location = $locations[rand(0, count($locations)-1)];
		// do something with the flag
		$f = rand(0,1);
		$fn = $flagnames[$f];
		if ($flags[$f] === "" || $flags[$f] === "dropped") {
			// not picked up. pick up randomly
			if (rand(0,10) < 3) {
				$rp = get_player($players, $f+1);
				if ($rp==-1) continue;
				$actions .= "$rp  ctf_touch_flag $time $fn $coords $location\n";
				$flags[$f] = $rp;
			}
		} else {
			// it's carried by someone.
			if (rand(0,10) < 3) {
				// got killed.
				$rp = get_player($players, 2-$f);
				if ($rp==-1) continue;
				$kp = $flags[$f];
				$actions .= "$rp $kp ctf_kill_flag $time $fn $coords $location\n";
				$flags[$f] = "dropped";
			} else {
				$flagdist[$f]++;
				$rp = $flags[$f];
				if ($flagdist[$f] > 10) {
					$actions .= "$rp  ctf_cap_flag $time $fn $coords $location\n";
					$flagdist[$f] = 0;
					$flags[$f] = "";
					$score[$f] += 10;
				}
			}
		}
	}

	$time += rand(4,20);
}

foreach ($players as $k=>$player) {
	if ($player['ingame']) {
		$players[$k]['ingame'] = false;
		$players[$k]['endtime'] = $time;
	}

	$stats[$k]['time_played'] = $players[$k]['endtime'] - $players[$k]['starttime'];
}

// output the stuffs
?>
login ff-test
auth XXXXXXXX
date <?=date("j-M-Y H:i:s")?> 
duration 1800
map ff_dev_ctf
bluescore <?=$score[0]?> 
redscore <?=$score[1]?> 
yellowscore 0
greenscore 0
numteams 2
<?
echo "players\n";
foreach ($players as $k=>$player) {
	echo "$player[steamid] $player[name] $player[team] $player[class]\n";
}

echo "actions\n";
echo $actions;

echo "stats\n";
ksort($stats);
foreach ($stats as $k=>$stat) {
	foreach ($stat as $statname=>$statvalue) {
		echo "$k $statname $statvalue\n";
	}
}

//print_r($players);
//print_r($stats);

?>
