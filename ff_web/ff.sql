-- phpMyAdmin SQL Dump
-- version 2.7.0-pl2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Apr 05, 2006 at 01:10 PM
-- Server version: 5.0.18
-- PHP Version: 5.1.2
-- 
-- Database: `ff`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `classes`
-- 

CREATE TABLE `classes` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `name` varchar(45) NOT NULL default '',
  `description` text NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=10 ;

-- 
-- Dumping data for table `classes`
-- 

INSERT INTO `classes` VALUES (1, 'Scout', 'Vroom Vroom!');
INSERT INTO `classes` VALUES (2, 'Sniper', 'Pew Pew Lasers!');
INSERT INTO `classes` VALUES (3, 'Soldier', 'BOOM! ROCKETS!!');
INSERT INTO `classes` VALUES (4, 'Demoman', 'Huh huh.. explosions');
INSERT INTO `classes` VALUES (5, 'Medic', 'I heal stuff');
INSERT INTO `classes` VALUES (6, 'HWGuy', 'MMmm chicken');
INSERT INTO `classes` VALUES (7, 'Pyro', 'Fire!');
INSERT INTO `classes` VALUES (8, 'Spy', 'Like a ninja and stuff');
INSERT INTO `classes` VALUES (9, 'Engineer', 'I make toys to do my dirty works yo!');

-- --------------------------------------------------------

-- 
-- Table structure for table `kills`
-- 

CREATE TABLE `kills` (
  `player` int(11) default NULL,
  `killed` int(11) default NULL,
  `weapon` int(11) default NULL,
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  KEY `FK_kills_3` (`weapon`),
  KEY `FK_kills_2` (`killed`),
  KEY `FK_kills_1` (`player`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='InnoDB free: 11264 kB; (`attacker`) REFER `ff/roundplayers`(';

-- 
-- Dumping data for table `kills`
-- 

INSERT INTO `kills` VALUES (58, 59, 10, '2006-01-21 16:03:14');
INSERT INTO `kills` VALUES (59, 58, 14, '2006-01-21 16:03:36');
INSERT INTO `kills` VALUES (59, 58, 23, '2006-01-21 16:04:08');
INSERT INTO `kills` VALUES (59, 58, 14, '2006-01-21 16:04:34');
INSERT INTO `kills` VALUES (60, 59, 19, '2006-01-21 16:05:20');

-- --------------------------------------------------------

-- 
-- Table structure for table `maps`
-- 

CREATE TABLE `maps` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(255) default NULL,
  `description` text,
  `image` varchar(255) default NULL,
  `pointsvalid` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=2 ;

-- 
-- Dumping data for table `maps`
-- 

INSERT INTO `maps` VALUES (1, 'ff_dev_ctf', 'Development map for testing out basic capture the flag gameplay', 'maps/ff_dev_ctf.jpg', 0);

-- --------------------------------------------------------

-- 
-- Table structure for table `medalinfo`
-- 

CREATE TABLE `medalinfo` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(255) NOT NULL default '',
  `description` text,
  `img` varchar(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

-- 
-- Dumping data for table `medalinfo`
-- 


-- --------------------------------------------------------

-- 
-- Table structure for table `medals`
-- 

CREATE TABLE `medals` (
  `medal` int(11) NOT NULL default '0',
  `player` int(11) NOT NULL default '0',
  PRIMARY KEY  (`medal`,`player`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- 
-- Dumping data for table `medals`
-- 


-- --------------------------------------------------------

-- 
-- Table structure for table `players`
-- 

CREATE TABLE `players` (
  `id` int(11) NOT NULL auto_increment,
  `steamid` varchar(60) NOT NULL default '',
  `name` varchar(60) NOT NULL default '',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `steamid` (`steamid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=3 ;

-- 
-- Dumping data for table `players`
-- 

INSERT INTO `players` VALUES (1, 'STEAM:0:1234', 'FryGuy');
INSERT INTO `players` VALUES (2, 'STEAM:0:4321', 'mirven');

-- --------------------------------------------------------

-- 
-- Table structure for table `ribboninfo`
-- 

CREATE TABLE `ribboninfo` (
  `id` int(11) NOT NULL auto_increment,
  `gameid` varchar(255) NOT NULL default '',
  `name` varchar(255) NOT NULL default '',
  `description` text,
  `img` varchar(255) default NULL,
  `points` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `gameid` (`gameid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

-- 
-- Dumping data for table `ribboninfo`
-- 


-- --------------------------------------------------------

-- 
-- Table structure for table `ribbons`
-- 

CREATE TABLE `ribbons` (
  `player` int(11) NOT NULL default '0',
  `round` int(11) NOT NULL default '0',
  `ribbon` int(11) NOT NULL default '0',
  KEY `player` (`player`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- 
-- Dumping data for table `ribbons`
-- 


-- --------------------------------------------------------

-- 
-- Table structure for table `roundplayers`
-- 

CREATE TABLE `roundplayers` (
  `player` int(11) NOT NULL default '0',
  `round` int(11) NOT NULL default '0',
  `class` int(11) NOT NULL default '0',
  `id` int(11) NOT NULL auto_increment,
  `team` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `FK_roundplayers_2` (`player`),
  KEY `FK_roundplayers_1` (`round`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='InnoDB free: 11264 kB; (`round`) REFER `ff/rounds`(`id`); (`' AUTO_INCREMENT=61 ;

-- 
-- Dumping data for table `roundplayers`
-- 

INSERT INTO `roundplayers` VALUES (1, 1, 1, 58, 1);
INSERT INTO `roundplayers` VALUES (2, 1, 3, 59, 2);
INSERT INTO `roundplayers` VALUES (1, 1, 5, 60, 1);

-- --------------------------------------------------------

-- 
-- Table structure for table `rounds`
-- 

CREATE TABLE `rounds` (
  `id` int(11) NOT NULL auto_increment,
  `server` int(11) NOT NULL default '0',
  `map` int(11) NOT NULL default '0',
  `starttime` datetime NOT NULL default '0000-00-00 00:00:00',
  `endtime` datetime NOT NULL default '0000-00-00 00:00:00',
  `bluescore` int(11) NOT NULL default '0',
  `redscore` int(11) NOT NULL default '0',
  `yellowscore` int(11) NOT NULL default '0',
  `greenscore` int(11) NOT NULL default '0',
  `numteams` int(11) NOT NULL default '4',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `Index_5` (`server`,`map`,`starttime`),
  KEY `Index_2` (`starttime`),
  KEY `FK_rounds_2` (`map`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=2 ;

-- 
-- Dumping data for table `rounds`
-- 

INSERT INTO `rounds` VALUES (1, 1, 1, '2006-01-21 16:03:00', '2006-01-21 16:33:00', 10, 0, 0, 0, 2);

-- --------------------------------------------------------

-- 
-- Table structure for table `servers`
-- 

CREATE TABLE `servers` (
  `id` int(11) NOT NULL auto_increment,
  `ip` varchar(255) default NULL,
  `name` varchar(255) default NULL,
  `description` text,
  `website` varchar(255) default NULL,
  `loginname` varchar(255) NOT NULL default '',
  `password` varchar(255) NOT NULL default '',
  `sharedsecret` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=2 ;

-- 
-- Dumping data for table `servers`
-- 

INSERT INTO `servers` VALUES (1, '82.136.2.129', 'FF Testing Server', 'Internal testing server for Fortress Forever', 'http://www.fortress-forever.com', 'ff-test', 'test123', 'sharedsecret123');

-- --------------------------------------------------------

-- 
-- Table structure for table `statinfo`
-- 

CREATE TABLE `statinfo` (
  `id` int(11) NOT NULL auto_increment,
  `gameid` varchar(255) NOT NULL default '',
  `name` varchar(255) NOT NULL default '',
  `description` text,
  `img` varchar(255) default NULL,
  `type` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `gameid` (`gameid`),
  KEY `Index_3` (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=70 ;

-- 
-- Dumping data for table `statinfo`
-- 

INSERT INTO `statinfo` VALUES (1, 'kills', 'Number of kills', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (2, 'time_played', 'Time played', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (3, 'ctf_flag_touch', 'Flag touches (CTF)', NULL, NULL, 'action');
INSERT INTO `statinfo` VALUES (4, 'ctf_flag_capture', 'Flag captures (CTF)', NULL, NULL, 'action');
INSERT INTO `statinfo` VALUES (5, 'fired_ff_weapon_assaultcannon', 'Shots fired from Assault Cannon', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (6, 'fired_ff_weapon_autorifle', 'Shots fired from Auto-rifle', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (7, 'fired_ff_weapon_crowbar', 'Shots fired from Crowbar', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (8, 'fired_ff_detpack', 'Shots fired from Detpack', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (9, 'fired_ff_sentrygun', 'Shots fired from Sentry Gun', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (10, 'fired_ff_weapon_flamethrower', 'Shots fired from Flamethrower', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (11, 'fired_ff_weapon_ic', 'Shots fired from Incendiary Cannon', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (12, 'fired_ff_weapon_knife', 'Shots fired from Knife', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (13, 'fired_ff_weapon_medkit', 'Shots fired from Medkit', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (14, 'fired_ff_weapon_nailgun', 'Shots fired from Nail Gun', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (15, 'fired_ff_weapon_pipelauncher', 'Shots fired from Pipe Launcher', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (16, 'fired_ff_weapon_radiotagrifle', 'Shots fired from Radio-tag Rifle', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (17, 'fired_ff_weapon_railgun', 'Shots fired from Railgun', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (18, 'fired_ff_weapon_rpg', 'Shots fired from Rocket Launcher', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (19, 'fired_ff_weapon_shotgun', 'Shots fired from Shotgun', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (20, 'fired_ff_weapon_spanner', 'Shots fired from Spanner', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (21, 'fired_ff_weapon_sniperrifle', 'Shots fired from Sniper Rifle', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (22, 'fired_ff_weapon_supernailgun', 'Shots fired from Super Nail Gun', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (23, 'fired_ff_weapon_supershotgun', 'Shots fired from Super Shotgun', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (24, 'fired_ff_weapon_tranquiliser', 'Shots fired from Tranquiliser', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (25, 'fired_ff_weapon_umbrella', 'Shots fired from Umbrella', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (26, 'fired_ff_weapon_flag', 'Shots fired from Flag', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (27, 'fired_ff_grenade_frag', 'Shots fired from Frag Grenade', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (28, 'fired_ff_grenade_concussion', 'Shots fired from Concussion Grenade', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (29, 'fired_ff_grenade_gas', 'Shots fired from Gas Grenade', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (30, 'fired_ff_grenade_mirv', 'Shots fired from Mirv Grenade', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (31, 'fired_ff_grenade_nail', 'Shots fired from Nail Grenade', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (32, 'fired_ff_grenade_napalm', 'Shots fired from Napalm Grenade', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (33, 'fired_ff_grenade_emp', 'Shots fired from EMP Grenade', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (34, 'fired_ff_grenade_caltrop', 'Shots fired from Caltrop', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (35, 'fired_ff_dispenser', 'Shots fired from Dispenser Detonation', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (36, 'hit_ff_weapon_assaultcannon', 'Shots hit from Assault Cannon', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (37, 'hit_ff_weapon_autorifle', 'Shots hit from Auto-rifle', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (38, 'hit_ff_weapon_crowbar', 'Shots hit from Crowbar', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (39, 'hit_ff_detpack', 'Shots hit from Detpack', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (40, 'hit_ff_sentrygun', 'Shots hit from Sentry Gun', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (41, 'hit_ff_weapon_flamethrower', 'Shots hit from Flamethrower', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (42, 'hit_ff_weapon_ic', 'Shots hit from Incendiary Cannon', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (43, 'hit_ff_weapon_knife', 'Shots hit from Knife', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (44, 'hit_ff_weapon_medkit', 'Shots hit from Medkit', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (45, 'hit_ff_weapon_nailgun', 'Shots hit from Nail Gun', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (46, 'hit_ff_weapon_pipelauncher', 'Shots hit from Pipe Launcher', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (47, 'hit_ff_weapon_radiotagrifle', 'Shots hit from Radio-tag Rifle', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (48, 'hit_ff_weapon_railgun', 'Shots hit from Railgun', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (49, 'hit_ff_weapon_rpg', 'Shots hit from Rocket Launcher', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (50, 'hit_ff_weapon_shotgun', 'Shots hit from Shotgun', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (51, 'hit_ff_weapon_spanner', 'Shots hit from Spanner', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (52, 'hit_ff_weapon_sniperrifle', 'Shots hit from Sniper Rifle', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (53, 'hit_ff_weapon_supernailgun', 'Shots hit from Super Nail Gun', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (54, 'hit_ff_weapon_supershotgun', 'Shots hit from Super Shotgun', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (55, 'hit_ff_weapon_tranquiliser', 'Shots hit from Tranquiliser', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (56, 'hit_ff_weapon_umbrella', 'Shots hit from Umbrella', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (57, 'hit_ff_weapon_flag', 'Shots hit from Flag', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (58, 'hit_ff_grenade_frag', 'Shots hit from Frag Grenade', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (59, 'hit_ff_grenade_concussion', 'Shots hit from Concussion Grenade', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (60, 'hit_ff_grenade_gas', 'Shots hit from Gas Grenade', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (61, 'hit_ff_grenade_mirv', 'Shots hit from Mirv Grenade', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (62, 'hit_ff_grenade_nail', 'Shots hit from Nail Grenade', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (63, 'hit_ff_grenade_napalm', 'Shots hit from Napalm Grenade', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (64, 'hit_ff_grenade_emp', 'Shots hit from EMP Grenade', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (65, 'hit_ff_grenade_caltrop', 'Shots hit from Caltrop', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (66, 'hit_ff_dispenser', 'Shots hit from Dispenser Detonation', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (67, 'deaths', 'Number of deaths', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (68, 'suicides', 'Number of times player killed themself', NULL, NULL, '');
INSERT INTO `statinfo` VALUES (69, 'teamkills', 'Number of teamkills', NULL, NULL, '');

-- --------------------------------------------------------

-- 
-- Table structure for table `statprogress`
-- 

CREATE TABLE `statprogress` (
  `stat` int(11) NOT NULL default '0',
  `value` int(11) NOT NULL default '0',
  `player` int(11) NOT NULL default '0',
  KEY `FK_statprogress_1` (`stat`),
  KEY `FK_statprogress_2` (`player`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='InnoDB free: 11264 kB; (`stat`) REFER `ff/statinfo`(`id`); (';

-- 
-- Dumping data for table `statprogress`
-- 

INSERT INTO `statprogress` VALUES (2, 97, 58);
INSERT INTO `statprogress` VALUES (14, 68, 58);
INSERT INTO `statprogress` VALUES (45, 35, 58);
INSERT INTO `statprogress` VALUES (19, 12, 58);
INSERT INTO `statprogress` VALUES (50, 8, 58);
INSERT INTO `statprogress` VALUES (3, 1, 58);
INSERT INTO `statprogress` VALUES (1, 1, 58);
INSERT INTO `statprogress` VALUES (2, 249, 59);
INSERT INTO `statprogress` VALUES (18, 13, 59);
INSERT INTO `statprogress` VALUES (49, 5, 59);
INSERT INTO `statprogress` VALUES (23, 32, 59);
INSERT INTO `statprogress` VALUES (54, 19, 59);
INSERT INTO `statprogress` VALUES (27, 6, 59);
INSERT INTO `statprogress` VALUES (58, 2, 59);
INSERT INTO `statprogress` VALUES (1, 3, 59);
INSERT INTO `statprogress` VALUES (2, 123, 60);
INSERT INTO `statprogress` VALUES (23, 38, 60);
INSERT INTO `statprogress` VALUES (54, 22, 60);
INSERT INTO `statprogress` VALUES (27, 8, 60);
INSERT INTO `statprogress` VALUES (58, 6, 60);
INSERT INTO `statprogress` VALUES (3, 2, 60);
INSERT INTO `statprogress` VALUES (4, 1, 60);
INSERT INTO `statprogress` VALUES (1, 1, 60);

-- --------------------------------------------------------

-- 
-- Table structure for table `weaponinfo`
-- 

CREATE TABLE `weaponinfo` (
  `id` int(11) NOT NULL auto_increment,
  `gameid` varchar(255) NOT NULL default '',
  `name` varchar(255) NOT NULL default '',
  `description` text,
  `img` varchar(255) default NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `gameid` (`gameid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=32 ;

-- 
-- Dumping data for table `weaponinfo`
-- 

INSERT INTO `weaponinfo` VALUES (1, 'ff_weapon_assaultcannon', 'Assault Cannon', 'Assault Cannon', 'img/weapons/ff_weapon_assaultcannon.jpg');
INSERT INTO `weaponinfo` VALUES (2, 'ff_weapon_autorifle', 'Auto-rifle', 'Auto-rifle', 'img/weapons/ff_weapon_autorifle.jpg');
INSERT INTO `weaponinfo` VALUES (3, 'ff_weapon_crowbar', 'Crowbar', 'Crowbar', 'img/weapons/ff_weapon_crowbar.jpg');
INSERT INTO `weaponinfo` VALUES (4, 'ff_detpack', 'Detpack', 'Detpack', 'img/weapons/ff_detpack.jpg');
INSERT INTO `weaponinfo` VALUES (5, 'ff_sentrygun', 'Sentry Gun', 'Sentry Gun', 'img/weapons/ff_sentrygun.jpg');
INSERT INTO `weaponinfo` VALUES (6, 'ff_weapon_flamethrower', 'Flamethrower', 'Flamethrower', 'img/weapons/ff_weapon_flamethrower.jpg');
INSERT INTO `weaponinfo` VALUES (7, 'ff_weapon_ic', 'Incendiary Cannon', 'Incendiary Cannon', 'img/weapons/ff_weapon_ic.jpg');
INSERT INTO `weaponinfo` VALUES (8, 'ff_weapon_knife', 'Knife', 'Knife', 'img/weapons/ff_weapon_knife.jpg');
INSERT INTO `weaponinfo` VALUES (9, 'ff_weapon_medkit', 'Medkit', 'Medkit', 'img/weapons/ff_weapon_medkit.jpg');
INSERT INTO `weaponinfo` VALUES (10, 'ff_weapon_nailgun', 'Nail Gun', 'Nail Gun', 'img/weapons/ff_weapon_nailgun.jpg');
INSERT INTO `weaponinfo` VALUES (11, 'ff_weapon_pipelauncher', 'Pipe Launcher', 'Pipe Launcher', 'img/weapons/ff_weapon_pipelauncher.jpg');
INSERT INTO `weaponinfo` VALUES (12, 'ff_weapon_radiotagrifle', 'Radio-tag Rifle', 'Radio-tag Rifle', 'img/weapons/ff_weapon_radiotagrifle.jpg');
INSERT INTO `weaponinfo` VALUES (13, 'ff_weapon_railgun', 'Railgun', 'Railgun', 'img/weapons/ff_weapon_railgun.jpg');
INSERT INTO `weaponinfo` VALUES (14, 'ff_weapon_rpg', 'Rocket Launcher', 'Rocket Launcher', 'img/weapons/ff_weapon_rpg.jpg');
INSERT INTO `weaponinfo` VALUES (15, 'ff_weapon_shotgun', 'Shotgun', 'Shotgun', 'img/weapons/ff_weapon_shotgun.jpg');
INSERT INTO `weaponinfo` VALUES (16, 'ff_weapon_spanner', 'Spanner', 'Spanner', 'img/weapons/ff_weapon_spanner.jpg');
INSERT INTO `weaponinfo` VALUES (17, 'ff_weapon_sniperrifle', 'Sniper Rifle', 'Sniper Rifle', 'img/weapons/ff_weapon_sniperrifle.jpg');
INSERT INTO `weaponinfo` VALUES (18, 'ff_weapon_supernailgun', 'Super Nail Gun', 'Super Nail Gun', 'img/weapons/ff_weapon_supernailgun.jpg');
INSERT INTO `weaponinfo` VALUES (19, 'ff_weapon_supershotgun', 'Super Shotgun', 'Super Shotgun', 'img/weapons/ff_weapon_supershotgun.jpg');
INSERT INTO `weaponinfo` VALUES (20, 'ff_weapon_tranquiliser', 'Tranquiliser', 'Tranquiliser', 'img/weapons/ff_weapon_tranquiliser.jpg');
INSERT INTO `weaponinfo` VALUES (21, 'ff_weapon_umbrella', 'Umbrella', 'Umbrella', 'img/weapons/ff_weapon_umbrella.jpg');
INSERT INTO `weaponinfo` VALUES (22, 'ff_weapon_flag', 'Flag', 'Flag', 'img/weapons/ff_weapon_flag.jpg');
INSERT INTO `weaponinfo` VALUES (23, 'ff_grenade_frag', 'Frag Grenade', 'Frag Grenade', 'img/weapons/ff_grenade_frag.jpg');
INSERT INTO `weaponinfo` VALUES (24, 'ff_grenade_concussion', 'Concussion Grenade', 'Concussion Grenade', 'img/weapons/ff_grenade_concussion.jpg');
INSERT INTO `weaponinfo` VALUES (25, 'ff_grenade_gas', 'Gas Grenade', 'Gas Grenade', 'img/weapons/ff_grenade_gas.jpg');
INSERT INTO `weaponinfo` VALUES (26, 'ff_grenade_mirv', 'Mirv Grenade', 'Mirv Grenade', 'img/weapons/ff_grenade_mirv.jpg');
INSERT INTO `weaponinfo` VALUES (27, 'ff_grenade_nail', 'Nail Grenade', 'Nail Grenade', 'img/weapons/ff_grenade_nail.jpg');
INSERT INTO `weaponinfo` VALUES (28, 'ff_grenade_napalm', 'Napalm Grenade', 'Napalm Grenade', 'img/weapons/ff_grenade_napalm.jpg');
INSERT INTO `weaponinfo` VALUES (29, 'ff_grenade_emp', 'EMP Grenade', 'EMP Grenade', 'img/weapons/ff_grenade_emp.jpg');
INSERT INTO `weaponinfo` VALUES (30, 'ff_grenade_caltrop', 'Caltrop', 'Caltrop', 'img/weapons/ff_grenade_caltrop.jpg');
INSERT INTO `weaponinfo` VALUES (31, 'ff_dispenser', 'Dispenser Detonation', 'Dispenser Detonation', 'img/weapons/ff_dispenser.jpg');

-- 
-- Constraints for dumped tables
-- 

-- 
-- Constraints for table `kills`
-- 
ALTER TABLE `kills`
  ADD CONSTRAINT `FK_kills_1` FOREIGN KEY (`player`) REFERENCES `roundplayers` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `FK_kills_2` FOREIGN KEY (`killed`) REFERENCES `roundplayers` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `FK_kills_3` FOREIGN KEY (`weapon`) REFERENCES `weaponinfo` (`id`);

-- 
-- Constraints for table `roundplayers`
-- 
ALTER TABLE `roundplayers`
  ADD CONSTRAINT `FK_roundplayers_1` FOREIGN KEY (`round`) REFERENCES `rounds` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `FK_roundplayers_2` FOREIGN KEY (`player`) REFERENCES `players` (`id`);

-- 
-- Constraints for table `rounds`
-- 
ALTER TABLE `rounds`
  ADD CONSTRAINT `FK_rounds_1` FOREIGN KEY (`server`) REFERENCES `servers` (`id`),
  ADD CONSTRAINT `FK_rounds_2` FOREIGN KEY (`map`) REFERENCES `maps` (`id`);

-- 
-- Constraints for table `statprogress`
-- 
ALTER TABLE `statprogress`
  ADD CONSTRAINT `FK_statprogress_1` FOREIGN KEY (`stat`) REFERENCES `statinfo` (`id`),
  ADD CONSTRAINT `FK_statprogress_2` FOREIGN KEY (`player`) REFERENCES `roundplayers` (`id`) ON DELETE CASCADE ON UPDATE CASCADE;
