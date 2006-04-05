<?php

add_action("servers", "display_servers", array("id"=>"numeric"));
add_action("serverdetails", "server_details", array("server"=>"numeric"));

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
<h4>Server Details</h4>
<table>
<tr>
	<th rel="row">Name</th>
	<td><?=$server['name']?></td>
</tr>
<tr>
	<th rel="row">IP</th>
	<td><?=$server['ip']?></td>
</tr>
<tr>
	<th rel="row">Website</th>
	<td><a href="<?=$server['website']?>"><?=$server['website']?></a></td>
</tr>
<tr>
	<th rel="row">Description</th>
	<td><?=$server['description']?></td>
</tr>
</table>


<?

	recent_rounds(array("server"=>$params['server']));
}


?>
