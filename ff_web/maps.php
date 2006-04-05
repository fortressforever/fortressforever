<?php

add_action("maps", "display_maps", array("id"=>"numeric"));
add_action("mapdetails", "map_details", array("map"=>"numeric"));

function display_maps($params)
{
	$query = "
		select id, ip, name, description, website
		from servers
		where 1=1
	";
	if (!empty($params['id']) && is_numeric($params['id']))
		$query .= " and id = ".$params['id'];
}

function map_details($params)
{
	if (empty($params['map']) || !is_numeric($params['map'])) {
		throw_error("no map passed");
		return;
	}

	$map = sql_query("
		select name, description, image, pointsvalid
		from maps
		where maps.id = $params[map]
	");
	
	if (count($map) != 1) {
		throw_error("Unknown round");
		return;
	}
	
	$map = $map[0];

	print_r($map);
}

?>
