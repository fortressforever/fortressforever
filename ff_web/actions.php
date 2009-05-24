<?php

$actions = array();

function add_action($action, $function, $params)
{
	$GLOBALS['actions'][$action] = array("function"=>$function, "params"=>$params);
}

function run_action($action, $in_params)
{
	global $actions;
	$defaultaction = "servers";

	// load up the action from the list
	if (empty($action))
		{ throw_error("invalid action"); return; }//$action = $defaultaction;
	if (!isset($actions[$action]))
		{ throw_error("invalid action"); return; }//$action = $defaultaction;
	$action = $actions[$action];
	
	// load up the appropriate parameters
	$params = array();
	foreach ($action['params'] as $key => $value)
	{
		$data = $in_params[$key];
		switch ($values)
		{
			case "numeric": $valid = is_numeric($data); break;
			default: $valid = true;
		}
		if ($valid)
			$params[$key] = $data;
	}
	
	// run the action
	$action['function']($params);
}

?>
