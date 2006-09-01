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
	run_action("playerweapondetails", $params);
	run_action("playeractiondetails", $params);
	run_action("playerkilldetails", $params);
}


?>
