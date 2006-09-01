<?php

add_action("servers", "display_servers", array("id"=>"numeric"));
add_action("serverdetails", "server_details", array("server"=>"numeric"));
add_action("serverstatistics", "server_statistics", array("server"=>"numeric"));

function display_servers($params)
{
	$query = "
		select id, ip, name, description, website
		from servers
		where 1=1
	";
	if (!empty($params['id']) && is_numeric($params['id']))
		$query .= " and id = ".$params['id'];
	
	$servers = sql_query($query);
?>
<h4>Servers</h4>
<table>
<tr>
	<th rel="col">Name</th>
	<th rel="col">IP</th>
	<th rel="col">Website</th>
	<th rel="col">Details</th>
</tr>
<? foreach ($servers as $server) { ?>
	<tr>
		<td><?=$server['name']?></td>
		<td><?=$server['ip']?></td>
		<td><a href="<?=$server['website']?>"><?=$server['website']?></a></td>
		<td><a href="?a=serverdetails&server=<?=$server['id']?>">Details</a></td>
	</tr>
<? } ?>
</table>
<?
}

function server_details($params)
{
	if (empty($params['server']) || !is_numeric($params['server'])) {
		throw_error("no id passed");
		return;
	}
	
	$server = sql_query("select ip, name, description, website from servers where id = $params[server]");
	
	if (count($server) != 1) {
		throw_error("Unknown server");
		return;
	}
	
	$server = $server[0];

?>
<div class="serverframe">
<div class="servername"><?=$server['name']?></div>
<div class="serverip"><?=$server['ip']?></div>
<div class="serverurl"><a href="<?=$server['website']?>"><?=$server['website']?></a></div>
<div class="serverdescription"><?=$server['description']?></div>
</div>

<?

	server_statistics(array("server"=>$params['server']));

	recent_rounds(array("server"=>$params['server']));
}



function server_statistics($params)
{
	if (empty($params['server']) || !is_numeric($params['server'])) {
		throw_error("no id passed");
		return;
	}
	
	$server = sql_query("select ip, name, description, website from servers where id = $params[server]");
	
	if (count($server) != 1) {
		throw_error("Unknown server");
		return;
	}
	
	$server = $server[0];

?>
<h4>Server Statistics</h4>

<!--<div>Average Players by hour: (show hours 1-24 along bottom and #players)</div>-->

<!--<div>Top maps played:</div>-->

<? run_action("playermostactive", $params); ?>

<?

}


?>
