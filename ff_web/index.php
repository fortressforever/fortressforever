<?php

// library
require_once "app_sql.php";
require_once "library.php";
require_once "actions.php";

//admin
require_once "parserequest.php";

// servers
require_once "servers.php";

// rounds
require_once "rounds.php";

// player
require_once "player.php";
require_once "player_round.php";
require_once "player_action_details.php";
require_once "player_most_active.php";
require_once "player_weapon_details.php";
require_once "player_kill_details.php";

// maps
require_once "maps.php";
require_once "map_details.php";


add_action("mainpage", "main_page", array());
function main_page($params)
{
	run_action("servers", $params);
	run_action("playermostactive", $params);
}


include "header.php";
run_action(isset($_REQUEST['a'])?$_REQUEST['a']:"mainpage", $_REQUEST);
include "footer.php";

?>
