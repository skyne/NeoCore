CREATE TABLE `character_kill` (
  `guid` int(11) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `creature_template` int(11) unsigned NOT NULL default '0' COMMENT 'Creature Identifier',
  `honor` float NOT NULL default '0',
  `date` int(11) unsigned NOT NULL default '0',
  `type` tinyint(3) unsigned NOT NULL default '0',
  KEY `idx_guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';
