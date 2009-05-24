<?
/*
	app_debug.php
		This file should provide a bunch of functions for easing in debugging.

	Created by: Kevin Hjelden
	Last Update: 4/28/01
*/

$global_start=microtime();

	$app_debug_str="";

	function debug_add($str) {
		global $app_debug_str;
		$app_debug_str .= $str;
	}

	function showdebug() {
		global $HTTP_GET_VARS, $HTTP_POST_VARS, $HTTP_COOKIE_VARS, $HTTP_ENV_VARS, $app_debug_str, $starttime;

		echo "<pre>";
		echo "$app_debug_str";
		echo "\n<b>URL VARIABLES:</b>\n";
		foreach ($HTTP_GET_VARS as $key => $value) {
			if (is_array($value)) {
				echo htmlspecialchars($key)."=";
				print_r($value);
				echo "\n";
			}
			else
				echo htmlspecialchars($key)."=".htmlspecialchars($value)."\n";
		}
		echo "\n<b>FORM VARIABLES:</b>\n";
		foreach ($HTTP_POST_VARS as $key => $value) {
			if (is_array($value)) {
				echo htmlspecialchars($key)."=";
				print_r($value);
				echo "\n";
			}
			else
				echo htmlspecialchars($key)."=".htmlspecialchars($value)."\n";
		}
		echo "\n<b>COOKIE VARIABLES:</b>\n";
		foreach ($HTTP_COOKIE_VARS as $key => $value) {
			echo "$key=$value\n";
		}
		echo "\n<b>CGI VARIABLES:</b>\n";
		foreach ($HTTP_ENV_VARS as $key => $value) {
			echo "$key=$value\n";
		}
		echo "\n</pre>";
	}
?>
