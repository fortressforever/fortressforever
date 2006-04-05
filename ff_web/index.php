<?php

require_once "app_sql.php";

require_once "actions.php";
require_once "servers.php";
require_once "rounds.php";
require_once "parserequest.php";
require_once "library.php";
require_once "player.php";
require_once "maps.php";

run_action($_REQUEST['a'], $_REQUEST);

?>
