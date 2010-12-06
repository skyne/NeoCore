ALTER TABLE `characters` 
DROP COLUMN `arena_pending_points`;

ALTER TABLE `characters` 
ADD COLUMN `honor_highest_rank` int(11) unsigned NOT NULL default '0' AFTER `taxi_path`,
ADD COLUMN `honor_standing` int(11) unsigned NOT NULL default '0' AFTER `honor_highest_rank`,
ADD COLUMN `honor_rating` float unsigned NOT NULL default '0' AFTER `honor_standing`;

