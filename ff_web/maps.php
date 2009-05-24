<?php

add_action("maps", "display_maps", array("id"=>"numeric"));

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

?>
