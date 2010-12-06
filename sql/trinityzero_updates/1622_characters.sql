ALTER TABLE groups DROP COLUMN difficulty;
ALTER TABLE instance DROP COLUMN difficulty;
ALTER TABLE characters DROP COLUMN dungeon_difficulty;

ALTER TABLE auctionhousebot
MODIFY `maxstackgreen` INT(11) DEFAULT '0' COMMENT 'Stack size limits for item qualities - a value of 0 will disable a maximum stack size for that quality, which will allow the bot to create items in stack as large as the item allows.',
MODIFY  `maxstackblue` INT(11) DEFAULT '0' COMMENT 'Stack size limits for item qualities - a value of 0 will disable a maximum stack size for that quality, which will allow the bot to create items in stack as large as the item allows.',
MODIFY  `maxstackpurple` INT(11) DEFAULT '0' COMMENT 'Stack size limits for item qualities - a value of 0 will disable a maximum stack size for that quality, which will allow the bot to create items in stack as large as the item allows.',
MODIFY  `maxstackorange` INT(11) DEFAULT '0' COMMENT 'Stack size limits for item qualities - a value of 0 will disable a maximum stack size for that quality, which will allow the bot to create items in stack as large as the item allows.',
MODIFY  `maxstackyellow` INT(11) DEFAULT '0' COMMENT 'Stack size limits for item qualities - a value of 0 will disable a maximum stack size for that quality, which will allow the bot to create items in stack as large as the item allows.',
MODIFY `buyerpricewhite` INT(11) DEFAULT '3' COMMENT 'Multiplier to vendorprice when buying white items from auctionhouse';

