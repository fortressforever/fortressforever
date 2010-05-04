#ff-systembot.tcl
# this script updates clients on other client and game statuses in the global sys chan

#dict set active_games ID channel players
# todo: metatable or dictionary to keep track of current games easier
set ffbot(channel) "#ffirc-system"
set gameid 1000
set gamecount 0

bind pub -|- !start start_game
bind pub -|- !debug debugproc
bind msg -|- !host parse_hostedgame

# todo  - check gamenum is unique
# DO NOT MOVE SHIT AROUND IN THIS FUNCTION - it will randomly stop working :(
proc parse_hostedgame {nick host hand text} {
	global gameid
	global ffbot
	global gamecount
	
	#set newchan "#ffgame-$text"
	set newchan "#ffgame-$gameid"
	
	putquick "PRIVMSG $nick :!join $newchan"
	putchan $ffbot(channel) "debug: new game created. irc channel = $newchan, hosted by $nick"
	
	# have the bot monitor the new chan for joins/parts and start
	channel add $newchan
	
	incr gamecount
	incr gameid
	#set gamecount [incr gamecount]
	#set gameid [incr gameid]

	savechannels
}

# #ffgame-1234
# gamenum is $chan 8:11
proc start_game { nick mask handle chan text } {
	global gamecount
	global gameid
	set gamenum [string range $chan 8 11]
	
	putquick "PRIVMSG $chan :!connect $gamenum" 
	incr gamecount -1
	incr gameid -1 

	putchan $ffbot(channel) "debug: game started! channel=$chan"
	
	channel remove $chan
}

proc debugproc { nick mask handle chan text } {
	global gamecount
	global gameid
	putchan $chan "Debug: gamecount = $gamecount, cur gameid=$gameid"
}

putlog "SCRIPT: ff-systembot.tcl loaded"