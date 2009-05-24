<?php

add_action("playeractiondetails", "player_action_details", array("player"=>"numeric", "round"=>"numeric"));

function player_action_details($params)
{
	$actions = sql_query("
		select statinfo.name, sum(value) as value
		from statprogress
		left join roundplayers on roundplayers.id = statprogress.player
		left join statinfo on statinfo.id = statprogress.stat
		where roundplayers.round = $params[round]
		and roundplayers.player = $params[player]
		and statinfo.type = 'action'
  		group by statinfo.name
		order by value desc
	");
?>

<h4>Action Information</h4>
<div>
<table>
<tr>
	<th rel="col">Action</th>
	<th rel="col">Times Performed</th>
</tr>
<? foreach ($actions as $action) { ?>
	<tr>
		<td><?=$action['name']?></td>
		<td class="numeric"><?=$action['value']?></td>
	</tr>
<? } ?>
<? if (!count($actions)) { ?>
	<tr>
		<td colspan="5">No actions performed.</td>
	</tr>
<? } ?>
</table>
</div>

<?	

}

?>
