#ff-systembot.tcl
# this script updates clients on other client and game statuses in the global sys chan

# todo: metatable or dictionary to keep track of current games easier
# gameInfo = "$hostNick $ircchannel"
set gameID 999
set gameInfo($gameID) "dmh #ffgame-test"

set ffbot(channel) "#ffirc-system"

bind pub -|- !start start_game
bind pub -|- !debug debugproc
bind msg -|- !host parse_hostedgame

# DO NOT MOVE SHIT AROUND IN THIS FUNCTION - it will randomly stop working :(
proc parse_hostedgame {nick host hand text} {
	global ffbot
	global gameID
	global gameInfo
	
	incr gameID
	set newchan "#ffgame-$gameID"
	
	set gameInfo($gameID) "$nick $newchan"
	putquick "PRIVMSG $nick :!join $newchan"
	#putchan $ffbot(channel) "debug: new game created. irc channel = $newchan, hosted by $nick"
	
	# have the bot monitor the new chan for joins/parts and start
	channel add $newchan
	savechannels
}

# #ffgame-1234
# gamenum is $chan 8:11
proc start_game { nick mask handle chan text } {
	global gameInfo
	set gamenum [string range $chan 8 11]
	putquick "PRIVMSG $chan :!connect $gamenum" 
	#putchan $ffbot(channel) "debug: game started! channel=$chan"
	unset gameInfo($gamenum)
	channel remove $chan
	savechannels
}

proc debugproc { nick mask handle chan text } {
	global gameInfo
	global ffbot
	
	#putchan $ffbot(channel) "debug gameInfo#$text=$gameInfo($text)"
	putchan $ffbot(channel) "debug: [array get gameInfo]"
	
	#set testID = [dict keys $gameInfo]
	#dict for { ID host channel } $gameInfo {
	#	putchan $ffbot(channel) "debug gameID=$ID host=$host channel=$channel"
	#}
	#putchan $ffbot(channel) "debug testID= $testID"
	#putchan $ffbot(channel) "debug gameInfo#=$gameInfo"
	#putchan $ffbot(channel) "debug gameID = [dict get $gameInfo id]"
}

putlog "SCRIPT: ff-systembot.tcl loaded"