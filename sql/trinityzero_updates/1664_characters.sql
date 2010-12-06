ALTER TABLE characters MODIFY `honor_rating` FLOAT NOT NULL DEFAULT '0';
-- clean you kill rows from non existant characters
DELETE FROM character_kill WHERE guid NOT IN (SELECT guid FROM characters);