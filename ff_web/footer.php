	<!--</div>-->
	</div>

	<div class="footer">

		<a href="fortressforever.rss" title="RSS Feed" target="_blank"><img src="rss.png" style="float: left; border: 0;" alt="RSS Feed"></a>

		#fortressforever @ <a href="http://www.gamesurge.net" target="_blank">gamesurge</a> &amp; <a href="http://www.quakenet.org" target="_blank">quakenet</a> |
		graciously hosted by <a href="http://www.burstfire.net" target="_blank">burstfire.net</a> | 

<?
	global $global_start;
	$global_end=microtime();
	$tmp1=split(" ",$global_start); 
	$tmp2=split(" ",$global_end); 
	$time=ceil((($tmp2[1]-$tmp1[1])+$tmp2[0]-$tmp1[0])*1000);
?>

		<a href="?a=admin" class="hidden"><?=sprintf("%0.3f",$time/1000)?> seconds</a>

	</div>

</div>

</body>
</html>
