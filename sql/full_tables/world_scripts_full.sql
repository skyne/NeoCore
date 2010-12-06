/*  */

/* WORLD BOSS */
UPDATE `creature_template` SET `ScriptName`='boss_ysondre' WHERE `entry`=14887;
UPDATE `creature_template` SET `ScriptName`='boss_emeriss' WHERE `entry`=14889;
UPDATE `creature_template` SET `ScriptName`='boss_taerar' WHERE `entry`=14890;
UPDATE `creature_template` SET `ScriptName`='boss_shade_of_taerar' WHERE `entry`=15302;
UPDATE `creature_template` SET `ScriptName`='boss_kruul' WHERE `entry`=12397;
UPDATE `creature_template` SET `ScriptName`='boss_azuregos' WHERE `entry`=6109;
UPDATE `creature_template` SET `ScriptName`='mob_dementeddruids' WHERE `entry`=15260;

/* GO */
UPDATE `gameobject_template` SET `ScriptName`='go_northern_crystal_pylon' WHERE `entry`=164955;
UPDATE `gameobject_template` SET `ScriptName`='go_western_crystal_pylon' WHERE `entry`=164956;
UPDATE `gameobject_template` SET `ScriptName`='go_eastern_crystal_pylon' WHERE `entry`=164957;
UPDATE `gameobject_template` SET `ScriptName`='go_barov_journal' WHERE `entry`=180794;
UPDATE `gameobject_template` SET `ScriptName`='go_field_repair_bot_74A' where `entry`= 179552;
UPDATE `gameobject_template` SET `ScriptName`='go_orb_of_command' WHERE `entry`=179879;
UPDATE `gameobject_template` SET `ScriptName`='go_tablet_of_madness' WHERE `entry`=180368;
UPDATE `gameobject_template` SET `ScriptName`='go_tablet_of_the_seven' WHERE `entry`=169294;
UPDATE `gameobject_template` set `ScriptName`='go_manticron_cube' where entry = 181713;
UPDATE `gameobject_template` SET `ScriptName`='go_mausoleum_trigger' WHERE `entry` = 104593;
UPDATE `gameobject_template` SET `ScriptName`='go_mausoleum_door' WHERE `entry` = 176594;
UPDATE `gameobject_template` SET `scriptname`='go_crystal_prison' WHERE `entry`=185126;
UPDATE `gameobject_template` SET `ScriptName`='go_jump_a_tron' WHERE `entry` = 183146;
UPDATE `gameobject_template` SET `ScriptName`='go_ethereum_prison' WHERE `entry` = 184421;
UPDATE `gameobject_template` SET `scriptname`='go_sacred_fire_of_life' WHERE `entry` = 175944;
UPDATE `gameobject_template` SET `scriptname`='go_skull_pile' WHERE `entry`=185913;

/* GUARD */
UPDATE `creature_template` SET `ScriptName`='guard_azuremyst' WHERE `entry`=18038;
UPDATE `creature_template` SET `ScriptName`='guard_orgrimmar' WHERE `entry`=3296;
UPDATE `creature_template` SET `ScriptName`='guard_stormwind' WHERE `entry` IN (68,1976);
UPDATE `creature_template` SET `ScriptName`='guard_contested' WHERE `entry` IN (9460,4624,3502,11190,15184);
UPDATE `creature_template` SET `ScriptName`='guard_elwynnforest' WHERE `entry`=1423;
UPDATE `creature_template` SET `ScriptName`='guard_eversong' WHERE `entry`=16221;
UPDATE `creature_template` SET `ScriptName`='guard_darnassus' WHERE `entry`=4262;
UPDATE `creature_template` SET `ScriptName`='guard_teldrassil' WHERE `entry`=3571;
UPDATE `creature_template` SET `ScriptName`='guard_ironforge' WHERE `entry`=5595;
UPDATE `creature_template` SET `ScriptName`='guard_dunmorogh' WHERE `entry` IN (727,13076);
UPDATE `creature_template` SET `ScriptName`='guard_undercity' WHERE `entry`=5624;
UPDATE `creature_template` SET `ScriptName`='guard_bluffwatcher' WHERE `entry`=3084;
UPDATE `creature_template` SET `ScriptName`='guard_durotar' WHERE `entry`=5953;
UPDATE `creature_template` SET `ScriptName`='guard_mulgore' WHERE `entry` IN (3212,3215,3217,3218,3219,3220,3221,3222,3223,3224);
UPDATE `creature_template` SET `ScriptName`='guard_dunmorogh' WHERE `entry` IN (727,13076);
UPDATE `creature_template` SET `ScriptName`='guard_tirisfal' WHERE `entry` IN (1735,1738,2210,1744,1745,5725,1743,2209,1746,1742);
UPDATE `creature_template` SET `ScriptName`='guard_silvermoon' WHERE `entry`=16222;
UPDATE `creature_template` SET `ScriptName`='guard_exodar' WHERE `entry`=16733;
UPDATE `creature_template` SET `ScriptName`='guard_shattrath' WHERE `entry`=19687;
UPDATE `creature_template` SET `ScriptName`='guard_shattrath_aldor' WHERE `entry`=18549;
UPDATE `creature_template` SET `ScriptName`='guard_shattrath_scryer' WHERE `entry`=18568;

/* ITEM */
UPDATE `item_template` SET `ScriptName`='item_area_52_special' WHERE `entry`=28132;
UPDATE `item_template` SET `ScriptName`='item_attuned_crystal_cores' WHERE `entry`=34368;
UPDATE `item_template` SET `ScriptName`='item_blackwhelp_net' WHERE `entry`=31129;
UPDATE `item_template` SET `ScriptName`='item_disciplinary_rod' WHERE `entry`=22473;
UPDATE `item_template` SET `ScriptName`='item_draenei_fishing_net' WHERE `entry`=23654;
UPDATE `item_template` SET `ScriptName`='item_flying_machine' WHERE `entry` IN (34060,34061);
UPDATE `item_template` SET `ScriptName`='item_gor_dreks_ointment' WHERE `entry`=30175;
UPDATE `item_template` SET `ScriptName`='item_muiseks_vessel' WHERE `entry` IN (9606,9618,9619,9620,9621);
UPDATE `item_template` SET `ScriptName`='item_nether_wraith_beacon' WHERE `entry`=31742;
UPDATE `item_template` SET `ScriptName`='item_protovoltaic_magneto_collector' WHERE `entry`=30656;
UPDATE `item_template` SET `ScriptName`='item_razorthorn_flayer_gland' WHERE `entry`=34255;
UPDATE `item_template` SET `ScriptName`='item_soul_cannon' WHERE `entry`=32825;
UPDATE `item_template` SET `ScriptName`='item_sparrowhawk_net' WHERE `entry`=32321;
UPDATE `item_template` SET `ScriptName`='item_tainted_core' WHERE `entry`=31088;
UPDATE `item_template` SET `ScriptName`='item_tame_beast_rods' WHERE `entry` IN (15908,15911,15913,15914,15915,15916,15917,15919,15920,15921,15922,15923,23697,23702,23703,23896,23897,23898);
UPDATE `item_template` SET `ScriptName`='item_voodoo_charm' WHERE `entry`=8149;
UPDATE `item_template` SET `ScriptName`='item_vorenthals_presence' WHERE `entry`=30259;
UPDATE `item_template` SET `ScriptName`='item_yehkinyas_bramble' WHERE `entry`=10699;
UPDATE `item_template` SET `ScriptName`='item_zezzaks_shard' WHERE `entry`=31463;
UPDATE `item_template` SET `ScriptName` = "item_only_for_flight" WHERE `entry` IN (34475, 34489, 24538);

/* NPC (usually creatures to be found in more than one specific zone) */
UPDATE `creature_template` SET `ScriptName`='npc_chicken_cluck' WHERE `entry`=620;
UPDATE `creature_template` SET `ScriptName`='npc_dancing_flames' WHERE `entry`=25305;
UPDATE `creature_template` SET `ScriptName`='npc_guardian' WHERE `entry`=5764;
UPDATE `creature_template` SET `ScriptName`='npc_mount_vendor' WHERE `entry` IN (384,1261,1460,2357,3362,3685,4730,4731,4885,7952,7955,16264,17584);
UPDATE `creature_template` SET `ScriptName`='npc_doctor' WHERE `entry` IN (12939,12920);
UPDATE `creature_template` SET `ScriptName`='npc_injured_patient' WHERE `entry` IN (12936,12937,12938,12923,12924,12925);
UPDATE `creature_template` SET `ScriptName`='npc_prof_alchemy' WHERE `entry` IN (17909,19052,22427);
UPDATE `creature_template` SET `ScriptName`='npc_prof_blacksmith' WHERE `entry` IN (5164,11145,11146,11176,11177,11178,11191,11192,11193);
UPDATE `creature_template` SET `ScriptName`='npc_prof_leather' WHERE `entry` IN (7866,7867,7868,7869,7870,7871);
UPDATE `creature_template` SET `ScriptName`='npc_prof_tailor' WHERE `entry` IN (22208,22212,22213);
UPDATE `creature_template` SET `ScriptName`='npc_rogue_trainer' WHERE `entry` IN (918,4163,3328,4583,5165,5167,13283,16684);
UPDATE `creature_template` SET `ScriptName`='npc_sayge' WHERE `entry`=14822;
UPDATE `creature_template` SET `ScriptName`='npc_steam_tonk' WHERE `entry` = 19405;
UPDATE `creature_template` SET `ScriptName`='npc_tonk_mine' WHERE `entry` = 15368;
UPDATE `creature_template` SET `ScriptName`='npc_winter_reveler' WHERE `entry`=15760;
UPDATE `creature_template` SET `ScriptName`='npc_snake_trap_serpents' where entry in (19921, 19833);

/*  */
/* ZONE */
/* */

/* ALTERAC MOUNTAINS */
UPDATE `creature_template` SET `ScriptName`='npc_ravenholdt' WHERE `entry`=13936;

/* ALTERAC VALLEY */


/* ARATHI HIGHLANDS */
UPDATE `creature_template` SET `ScriptName`='npc_professor_phizzlethorpe' where entry=2768;

/* ASHENVALE */
UPDATE `creature_template` SET `ScriptName`='npc_torek' WHERE `entry`=12858;
UPDATE `creature_template` SET `ScriptName`='npc_ruul_snowhoof' WHERE `entry` = 12818;

/* BADLANDS */


/* BARRENS */
UPDATE `creature_template` SET `ScriptName`='npc_beaten_corpse' WHERE `entry`=10668;
UPDATE `creature_template` SET `ScriptName`='npc_sputtervalve' WHERE `entry`=3442;
UPDATE `creature_template` SET `ScriptName`='npc_taskmaster_fizzule' WHERE `entry`=7233;
UPDATE `creature_template` SET `ScriptName`='npc_twiggy_flathead' WHERE `entry` =6248;
UPDATE `creature_template` SET `ScriptName`='npc_wizzlecrank_shredder' where entry=3439;

/* BLACKFATHOM DEPTHS */


/* BLACKROCK DEPTHS */
UPDATE `creature_template` SET `ScriptName`='boss_emperor_dagran_thaurissan' WHERE `entry`=9019;
UPDATE `creature_template` SET `ScriptName`='boss_moira_bronzebeard' WHERE `entry`=8929;
UPDATE `creature_template` SET `ScriptName`='boss_ambassador_flamelash' WHERE `entry`=9156;
UPDATE `creature_template` SET `ScriptName`='boss_angerrel' WHERE `entry`=9035;
UPDATE `creature_template` SET `ScriptName`='boss_anubshiah' WHERE `entry`=9031;
UPDATE `creature_template` SET `ScriptName`='boss_doomrel' WHERE `entry`=9039;
UPDATE `creature_template` SET `ScriptName`='boss_doperel' WHERE `entry`=9040;
UPDATE `creature_template` SET `ScriptName`='boss_general_angerforge' WHERE `entry`=9033;
UPDATE `creature_template` SET `ScriptName`='boss_gloomrel' WHERE `entry`=9037;
UPDATE `creature_template` SET `ScriptName`='boss_gorosh_the_dervish' WHERE `entry`=9027;
UPDATE `creature_template` SET `ScriptName`='boss_grizzle' WHERE `entry`=9028;
UPDATE `creature_template` SET `ScriptName`='boss_haterel' WHERE `entry`=9034;
UPDATE `creature_template` SET `ScriptName`='boss_high_interrogator_gerstahn' WHERE `entry`=9018;
UPDATE `creature_template` SET `ScriptName`='boss_magmus' WHERE `entry`=9938;
UPDATE `creature_template` SET `ScriptName`='boss_seethrel' WHERE `entry`=9038;
UPDATE `creature_template` SET `ScriptName`='boss_vilerel' WHERE `entry`=9036;
UPDATE `creature_template` SET `ScriptName`='phalanx' WHERE `entry`=9502;
UPDATE `creature_template` SET `ScriptName`='npc_lokhtos_darkbargainer' WHERE `entry`=12944;
UPDATE `creature_template` SET `ScriptName`='npc_kharan_mighthammer' WHERE `entry`=9021;
UPDATE `creature_template` SET `ScriptName`='npc_grimstone' WHERE `entry`=10096;
UPDATE `creature_template` SET `ScriptName`='npc_rocknot' WHERE `entry`=9503;

/* BLACKROCK SPIRE */
/* BLACKROCK SPIRE Lower bosses */
UPDATE `creature_template` SET `ScriptName`='boss_highlord_omokk' WHERE `entry`=9196;
UPDATE `creature_template` SET `ScriptName`='boss_shadow_hunter_voshgajin' WHERE `entry`=9236;
UPDATE `creature_template` SET `ScriptName`='boss_warmaster_voone' WHERE `entry`=9237;
UPDATE `creature_template` SET `ScriptName`='boss_mother_smolderweb' WHERE `entry`=10596;
UPDATE `creature_template` SET `ScriptName`='quartermaster_zigris' WHERE `entry`=9736;
UPDATE `creature_template` SET `ScriptName`='boss_halycon' WHERE `entry`=10220;
UPDATE `creature_template` SET `ScriptName`='boss_overlord_wyrmthalak' WHERE `entry`=9568;
/* BLACKROCK SPIRE Upper bosses */
UPDATE `creature_template` SET `ScriptName`='boss_the_beast' WHERE `entry`=10430;
UPDATE `creature_template` SET `ScriptName`='boss_drakkisath' WHERE `entry`=10363;
UPDATE `creature_template` SET `ScriptName`='boss_gyth' WHERE `entry`=10339;
UPDATE `creature_template` SET `ScriptName`='boss_rend_blackhand' WHERE `entry`=10429;
UPDATE `creature_template` SET `ScriptName`='boss_pyroguard_emberseer' WHERE `entry`=9816;

/* BLACKWING LAIR */
UPDATE `instance_template` SET `script`='instance_blackwing_lair' WHERE `map`=469;
UPDATE `creature_template` SET `ScriptName`='boss_razorgore' WHERE `entry`=12435;
UPDATE `creature_template` SET `ScriptName`='boss_vaelastrasz' WHERE `entry`=13020;
UPDATE `creature_template` SET `ScriptName`='boss_broodlord' WHERE `entry`=12017;
UPDATE `creature_template` SET `ScriptName`='boss_firemaw' WHERE `entry`=11983;
UPDATE `creature_template` SET `ScriptName`='boss_ebonroc' WHERE `entry`=14601;
UPDATE `creature_template` SET `ScriptName`='boss_flamegor' WHERE `entry`=11981;
UPDATE `creature_template` SET `ScriptName`='boss_chromaggus' WHERE `entry`=14020;
UPDATE `creature_template` SET `ScriptName`='boss_victor_nefarius' WHERE `entry`=10162;
UPDATE `creature_template` SET `ScriptName`='boss_nefarian' WHERE `entry`=11583;

/* BLASTED LANDS */
UPDATE `creature_template` SET `ScriptName`='npc_deathly_usher' WHERE `entry`=8816;
UPDATE `creature_template` SET `ScriptName`='npc_fallen_hero_of_horde' WHERE `entry`=7572;

/* BURNING STEPPES */
UPDATE `creature_template` SET `ScriptName`='npc_ragged_john' WHERE `entry`=9563;


/* DARKSHORE */


/* DEADMINES */
UPDATE `instance_template` SET `script`='instance_deadmines' WHERE `map`=36;
UPDATE `item_template` SET `ScriptName`='item_defias_gunpowder' WHERE `entry`=5397;
UPDATE `gameobject_template` SET `ScriptName`='go_defias_cannon' WHERE `entry`=16398;
UPDATE `gameobject_template` SET `ScriptName`='go_door_lever_dm' WHERE `entry`=101833;
UPDATE `gameobject_template` SET `ScriptName`='go_main_chambers_access_panel' WHERE `entry` IN (184125,184126);

/* DEADWIND PASS */


/* DESOLACE */


/* DIRE MAUL */


/* DUN MOROGH */
UPDATE `creature_template` SET `ScriptName`='npc_narm_faulk' WHERE `entry`=6177;


/* DUROTAR */


/* DUSKWOOD */


/* DUSTWALLOW MARSH */
UPDATE `creature_template` SET `ScriptName`='mobs_risen_husk_spirit' WHERE `entry` IN (23554,23555);
UPDATE `creature_template` SET `ScriptName`='npc_deserter_agitator' WHERE `entry`=23602;
UPDATE `creature_template` SET `ScriptName`='npc_lady_jaina_proudmoore' WHERE `entry`=4968;
UPDATE `creature_template` SET `ScriptName`='npc_nat_pagle' WHERE `entry`=12919;
UPDATE `creature_template` SET `ScriptName`='npc_restless_apparition' WHERE `entry`=23861;

/* EASTERN PLAGUELANDS */
UPDATE `creature_template` SET `ScriptName`='mobs_ghoul_flayer' WHERE `entry` IN (8530,8531,8532);
UPDATE `creature_template` SET `ScriptName`='npc_augustus_the_touched' WHERE `entry`=12384;
UPDATE `creature_template` SET `ScriptName`='npc_darrowshire_spirit' WHERE `entry`=11064;
UPDATE `creature_template` SET `ScriptName`='npc_tirion_fordring' WHERE `entry`=1855;

/* ELWYNN FOREST */
UPDATE `creature_template` SET `ScriptName`='npc_henze_faulk' WHERE `entry`=6172;

/* FELWOOD */
UPDATE `creature_template` SET `ScriptName`='npcs_riverbreeze_and_silversky' WHERE `entry` IN (9528,9529);

/* FERALAS */
UPDATE `creature_template` SET `ScriptName`='npc_gregan_brewspewer' WHERE `entry`=7775;
UPDATE `creature_template` SET `ScriptName`='npc_screecher_spirit' WHERE `entry`=8612;

/* GHOSTLANDS */
UPDATE `creature_template` SET `ScriptName`='npc_blood_knight_dawnstar' WHERE `entry`=17832;
UPDATE `creature_template` SET `ScriptName`='npc_budd_nedreck' WHERE `entry`=23559;
UPDATE `creature_template` SET `ScriptName`='npc_rathis_tomber' WHERE `entry`=16224;
UPDATE `creature_template` SET `Scriptname`='npc_ranger_lilatha' WHERE entry=16295;
update `gameobject_template` SET `ScriptName`='go_gilded_brazier' where entry = 181956;

/* GNOMEREGAN */

/* HILLSBRAD FOOTHILLS */


/* HINTERLANDS */


/* IRONFORGE */
UPDATE `creature_template` SET `ScriptName`='npc_royal_historian_archesonus' WHERE `entry`=8879;

/* LOCH MODAN */
UPDATE `creature_template` SET `ScriptName`='npc_mountaineer_pebblebitty' WHERE `entry`=3836;

/* MARAUDON */
UPDATE `creature_template` SET `ScriptName`='boss_princess_theradras' WHERE `entry`=12201;
UPDATE `creature_template` SET `ScriptName`='boss_noxxion' WHERE `entry`=13282;
UPDATE `creature_template` SET `ScriptName`='boss_landslide' WHERE `entry`=12203;
UPDATE `creature_template` SET `ScriptName`='celebras_the_cursed' WHERE `entry`=12225;

/* MOLTEN CORE */
UPDATE `instance_template` SET `script`='instance_molten_core' WHERE `map`=409;
UPDATE `creature_template` SET `ScriptName`='boss_lucifron' WHERE `entry`=12118;
UPDATE `creature_template` SET `ScriptName`='boss_magmadar' WHERE `entry`=11982;
UPDATE `creature_template` SET `ScriptName`='boss_gehennas' WHERE `entry`=12259;
UPDATE `creature_template` SET `ScriptName`='boss_garr' WHERE `entry`=12057;
UPDATE `creature_template` SET `ScriptName`='boss_baron_geddon' WHERE `entry`=12056;
UPDATE `creature_template` SET `ScriptName`='boss_shazzrah' WHERE `entry`=12264;
UPDATE `creature_template` SET `ScriptName`='boss_golemagg' WHERE `entry`=11988;
UPDATE `creature_template` SET `ScriptName`='boss_sulfuron' WHERE `entry`=12098;
UPDATE `creature_template` SET `ScriptName`='boss_majordomo' WHERE `entry`=12018;
UPDATE `creature_template` SET `ScriptName`='boss_ragnaros' WHERE `entry`=11502;
UPDATE `creature_template` SET `ScriptName`='mob_ancient_core_hound' WHERE `entry`=11673;
UPDATE `creature_template` SET `ScriptName`='mob_firesworn' WHERE `entry`=12099;
UPDATE `creature_template` SET `ScriptName`='mob_core_rager' WHERE `entry`=11672;
UPDATE `creature_template` SET `ScriptName`='mob_flamewaker_priest' WHERE `entry`=11662;

/* MOONGLADE */
UPDATE `creature_template` SET `ScriptName`='npc_bunthen_plainswind' WHERE `entry`=11798;
UPDATE `creature_template` SET `ScriptName`='npc_great_bear_spirit' WHERE `entry`=11956;
UPDATE `creature_template` SET `ScriptName`='npc_silva_filnaveth' WHERE `entry`=11800;
UPDATE `creature_template` SET `ScriptName`='npc_clintar_dreamwalker' WHERE `entry`=22834;
UPDATE `creature_template` SET `ScriptName`='npc_clintar_spirit' WHERE `entry` = 22916;

/* MULGORE */
UPDATE `creature_template` SET `ScriptName`='npc_skorn_whitecloud' WHERE `entry`=3052;
UPDATE `creature_template` SET `ScriptName`='npc_kyle_frenzied' WHERE `entry`='23616';
UPDATE `creature_template` SET `ScriptName`='npc_plains_vision' WHERE `entry`='2983';

/* NAXXRAMAS */
UPDATE `instance_template` SET `script`='instance_naxxramas' WHERE `map`=533;
UPDATE `creature_template` SET `ScriptName`='boss_anubrekhan' WHERE `entry`=15956;
UPDATE `creature_template` SET `ScriptName`='boss_faerlina' WHERE `entry`=15953;
UPDATE `creature_template` SET `ScriptName`='boss_maexxna' WHERE `entry`=15952;
UPDATE `creature_template` SET `ScriptName`='boss_noth' WHERE `entry`=15954;
UPDATE `creature_template` SET `ScriptName`='boss_heigan' WHERE `entry`=15936;
UPDATE `creature_template` SET `ScriptName`='boss_loatheb' WHERE `entry`=16011;
UPDATE `creature_template` SET `ScriptName`='boss_razuvious' WHERE `entry`=16061;
UPDATE `creature_template` SET `ScriptName`='boss_gothik' WHERE `entry`=16060;
UPDATE `creature_template` SET `ScriptName`='boss_highlord_mograine' WHERE `entry`=16062;
UPDATE `creature_template` SET `ScriptName`='boss_thane_korthazz' WHERE `entry`=16064;
UPDATE `creature_template` SET `ScriptName`='boss_sir_zeliek' WHERE `entry`=16063;
UPDATE `creature_template` SET `ScriptName`='boss_lady_blaumeux' WHERE `entry`=16065;
UPDATE `creature_template` SET `ScriptName`='boss_patchwerk' WHERE `entry`=16028;
UPDATE `creature_template` SET `ScriptName`='boss_grobbulus' WHERE `entry`=15931;
UPDATE `creature_template` SET `ScriptName`='boss_gluth' WHERE `entry`=15932;
UPDATE `creature_template` SET `ScriptName`='boss_thaddius' WHERE `entry`=15928;
UPDATE `creature_template` SET `ScriptName`='boss_stalagg' WHERE `entry`=15929;
UPDATE `creature_template` SET `ScriptName`='boss_fugen' WHERE `entry`=15930;
UPDATE `creature_template` SET `ScriptName`='boss_sapphiron' WHERE `entry`=15989;
UPDATE `creature_template` SET `ScriptName`='boss_kelthuzad' WHERE `entry`=15990;

/* ONYXIA'S LAIR */
UPDATE `creature_template` SET `ScriptName`='boss_onyxia' WHERE `entry`=10184;

/* ORGRIMMAR */
UPDATE `creature_template` SET `ScriptName`='npc_neeru_fireblade' WHERE `entry`=3216;
UPDATE `creature_template` SET `ScriptName`='npc_shenthul' WHERE `entry`=3401;
UPDATE `creature_template` SET `ScriptName`='npc_thrall_warchief' WHERE `entry`=4949;

/* RAGEFIRE CHASM */

/* RAZORFEN DOWNS */
UPDATE `creature_template` SET `ScriptName`='boss_amnennar_the_coldbringer' WHERE `entry`=7358;

/* RAZORFEN KRAUL */
UPDATE `creature_template` SET `Scriptname`='npc_willix' WHERE entry=4508;

/* REDRIDGE MOUNTAINS */


/* RUINS OF AHN'QIRAJ */
UPDATE `instance_template` SET `script`='instance_ruins_of_ahnqiraj' WHERE `map`=509;

/* SCARLET MONASTERY */
UPDATE `instance_template` SET `script`='instance_scarlet_monastery' WHERE `map`=189;
UPDATE `creature_template` SET `ScriptName`='boss_arcanist_doan' WHERE `entry`=6487;
UPDATE `creature_template` SET `ScriptName`='boss_azshir_the_sleepless' WHERE `entry`=6490;
UPDATE `creature_template` SET `ScriptName`='boss_bloodmage_thalnos' WHERE `entry`=4543;
UPDATE `creature_template` SET `ScriptName`='boss_herod' WHERE `entry`=3975;
UPDATE `creature_template` SET `ScriptName`='boss_high_inquisitor_fairbanks' WHERE `entry`=4542;
UPDATE `creature_template` SET `ScriptName`='boss_high_inquisitor_whitemane' WHERE `entry`=3977;
UPDATE `creature_template` SET `ScriptName`='boss_houndmaster_loksey' WHERE `entry`=3974;
UPDATE `creature_template` SET `ScriptName`='boss_interrogator_vishas' WHERE `entry`=3983;
UPDATE `creature_template` SET `ScriptName`='boss_scarlet_commander_mograine' WHERE `entry`=3976;
UPDATE `creature_template` SET `ScriptName`='boss_headless_horseman' WHERE `entry` = 23682;
UPDATE `creature_template` SET `ScriptName`='mob_head' WHERE `entry` = 23775;
UPDATE `creature_template` SET `ScriptName`='mob_pulsing_pumpkin' WHERE `entry` = 23694;
UPDATE `creature_template` SET `ScriptName`='mob_wisp_invis' WHERE `entry`=23686;
UPDATE `creature_template` SET `ScriptName`='mob_wisp_invis' WHERE `entry`=24034;
UPDATE `creature_template` SET `ScriptName`='mob_scarlet_trainee' WHERE `entry`=6575;
UPDATE `gameobject_template` SET `ScriptName`='go_loosely_turned_soil' WHERE `entry`=186314;

/* SCHOLOMANCE */
UPDATE `instance_template` SET `script`='instance_scholomance' WHERE `map`=289;
UPDATE `creature_template` SET `ScriptName`='boss_darkmaster_gandling' WHERE `entry`=1853;
UPDATE `creature_template` SET `ScriptName`='boss_death_knight_darkreaver' WHERE `entry`=14516;
UPDATE `creature_template` SET `ScriptName`='boss_lord_alexei_barov' WHERE `entry`=10504;
UPDATE `creature_template` SET `ScriptName`='boss_instructor_malicia' WHERE `entry`=10505;
UPDATE `creature_template` SET `ScriptName`='boss_boss_ras_frostwhisper' WHERE `entry`=10508;
UPDATE `creature_template` SET `ScriptName`='boss_the_ravenian' WHERE `entry`=10507;
UPDATE `creature_template` SET `ScriptName`='boss_vectus' WHERE `entry`=10432;
UPDATE `creature_template` SET `ScriptName`='boss_illucia_barov' WHERE `entry`=10502;
UPDATE `creature_template` SET `ScriptName`='boss_doctor_theolen_krastinov' WHERE `entry`=11261;
UPDATE `creature_template` SET `ScriptName`='boss_jandice_barov' WHERE `entry`=10503;
UPDATE `creature_template` SET `ScriptName`='boss_lorekeeper_polkelt' WHERE `entry`=10901;
UPDATE `creature_template` SET `ScriptName`='boss_kormok' WHERE `entry`=16118;
UPDATE `creature_template` SET `ScriptName`='mob_illusionofjandicebarov' WHERE `entry`=11439;

/* SEARING GORGE */
UPDATE `creature_template` SET `ScriptName`='npc_kalaran_windblade' WHERE `entry`=8479;
UPDATE `creature_template` SET `ScriptName`='npc_lothos_riftwaker' WHERE `entry`=14387;
UPDATE `creature_template` SET `ScriptName`='npc_zamael_lunthistle' WHERE `entry`=8436;

/* SHADOWFANG KEEP */
UPDATE `instance_template` SET `script`='instance_shadowfang_keep' WHERE `map`=33;
UPDATE `creature_template` SET `ScriptName`='npc_shadowfang_prisoner' WHERE `entry` IN (3849,3850);

/* SILITHUS */
UPDATE `creature_template` SET `ScriptName`='npcs_rutgar_and_frankal' WHERE `entry` IN (15170,15171);
UPDATE `creature_template` SET `ScriptName`='npc_highlord_demitrian' WHERE `entry`=14347;
UPDATE `gameobject_template` SET `ScriptName`='go_gauntlet_gate' WHERE `entry`=175357;

/* SILVERPINE FOREST */
UPDATE `creature_template` SET `ScriptName`='npc_astor_hadren' WHERE `entry`=6497;
UPDATE `creature_template` SET `ScriptName`='npc_deathstalker_erland' WHERE `entry`=1978;

/* STOCKADES */

/* STONETALON MOUNTAINS */
UPDATE `creature_template` SET `ScriptName`='npc_braug_dimspirit' WHERE `entry`=4489;
UPDATE `creature_template` SET `ScriptName`='npc_kaya_flathoof' WHERE `entry`=11856;

/* STORMWIND CITY */
UPDATE `creature_template` SET `ScriptName`='npc_archmage_malin' WHERE `entry`=2708;
UPDATE `creature_template` SET `ScriptName`='npc_bartleby' WHERE `entry`=6090;
UPDATE `creature_template` SET `ScriptName`='npc_dashel_stonefist' WHERE `entry`=4961;
UPDATE `creature_template` SET `ScriptName`='npc_general_marcus_jonathan' WHERE `entry`=466;
UPDATE `creature_template` SET `ScriptName`='npc_lady_katrana_prestor' WHERE `entry`=1749;

/* STRANGLETHORN VALE */
UPDATE `creature_template` SET `ScriptName`='mob_yenniku' WHERE `entry`=2530;

/* STRATHOLME */
UPDATE `instance_template` SET `script`='instance_stratholme' WHERE `map`=329;
UPDATE `creature_template` SET `ScriptName`='boss_dathrohan_balnazzar' WHERE `entry`=10812;
UPDATE `creature_template` SET `ScriptName`='boss_magistrate_barthilas' WHERE `entry`=10435;
UPDATE `creature_template` SET `ScriptName`='boss_maleki_the_pallid' WHERE `entry`=10438;
UPDATE `creature_template` SET `ScriptName`='boss_nerubenkan' WHERE `entry`=10437;
UPDATE `creature_template` SET `ScriptName`='boss_cannon_master_willey' WHERE `entry`=10997;
UPDATE `creature_template` SET `ScriptName`='boss_baroness_anastari' WHERE `entry`=10436;
UPDATE `creature_template` SET `ScriptName`='boss_ramstein_the_gorger' WHERE `entry`=10439;
UPDATE `creature_template` SET `ScriptName`='boss_timmy_the_cruel' WHERE `entry`=10808;
UPDATE `creature_template` SET `ScriptName`='boss_silver_hand_bosses' WHERE `entry` IN (17910,17911,17912,17913,17914);
UPDATE `creature_template` SET `ScriptName`='boss_postmaster_malown' WHERE `entry`=11143;
UPDATE `creature_template` SET `ScriptName`='boss_baron_rivendare' WHERE `entry`=10440;
UPDATE `creature_template` SET `ScriptName`='mob_mindless_skeleton' WHERE `entry`=11197;
UPDATE `creature_template` SET `ScriptName`='mob_thuzadin_acolyte' WHERE `entry`=10399;
UPDATE `creature_template` SET `ScriptName`='mobs_spectral_ghostly_citizen' WHERE `entry` IN (10384,10385);
UPDATE `creature_template` SET `ScriptName`='mob_restless_soul' WHERE `entry`=11122;
UPDATE `creature_template` SET `ScriptName`='mob_freed_soul' WHERE `entry`=11136;

/* SWAMP OF SORROWS */


/* TANARIS */
UPDATE `creature_template` SET `ScriptName`='mob_aquementas' WHERE `entry`=9453;
UPDATE `creature_template` SET `ScriptName`='npc_custodian_of_time' WHERE `entry`=20129;
UPDATE `creature_template` SET `ScriptName`='npc_marin_noggenfogger' WHERE `entry`=7564;
UPDATE `creature_template` SET `ScriptName`='npc_steward_of_time' WHERE `entry`=20142;
UPDATE `creature_template` SET `ScriptName`='npc_stone_watcher_of_norgannon' WHERE `entry`=7918;
UPDATE `creature_template` SET `ScriptName` = 'npc_OOX17' WHERE `entry` = 7784;

/* TELDRASSIL */

/* TEMPLE OF AHN'QIRAJ */
UPDATE `instance_template` SET `script`='instance_temple_of_ahnqiraj' WHERE `map`=531;
UPDATE `creature_template` SET `ScriptName`='boss_cthun' WHERE `entry`=15727;
UPDATE `creature_template` SET `ScriptName`='boss_skeram' WHERE `entry`=15263;
UPDATE `creature_template` SET `ScriptName`='boss_vem' WHERE `entry`=15544;
UPDATE `creature_template` SET `ScriptName`='boss_yauj' WHERE `entry`=15543;
UPDATE `creature_template` SET `ScriptName`='boss_kri' WHERE `entry`=15511;
UPDATE `creature_template` SET `ScriptName`='boss_sartura' WHERE `entry`=15516;
UPDATE `creature_template` SET `ScriptName`='boss_fankriss' WHERE `entry`=15510;
UPDATE `creature_template` SET `ScriptName`='boss_viscidus' WHERE `entry`=15299;
UPDATE `creature_template` SET `ScriptName`='boss_glob_of_viscidus' WHERE `entry`=15667;
UPDATE `creature_template` SET `ScriptName`='boss_huhuran' WHERE `entry`=15509;
UPDATE `creature_template` SET `ScriptName`='boss_veklor' WHERE `entry`=15276;
UPDATE `creature_template` SET `ScriptName`='boss_veknilash' WHERE `entry`=15275;
UPDATE `creature_template` SET `ScriptName`='boss_ouro' WHERE `entry`=15517;
UPDATE `creature_template` SET `ScriptName`='boss_eye_of_cthun' WHERE `entry`=15589;
UPDATE `creature_template` SET `ScriptName`='mob_sartura_royal_guard' WHERE `entry`=15984;
UPDATE `creature_template` SET `ScriptName`='mob_yauj_brood' WHERE `entry`=15621;
UPDATE `creature_template` SET `ScriptName`='mob_claw_tentacle' WHERE `entry`=15725;
UPDATE `creature_template` SET `ScriptName`='mob_eye_tentacle' WHERE `entry`=15726;
UPDATE `creature_template` SET `ScriptName`='mob_giant_claw_tentacle' WHERE `entry`=15728;
UPDATE `creature_template` SET `ScriptName`='mob_giant_eye_tentacle' WHERE `entry`=15334;
UPDATE `creature_template` SET `ScriptName`='mob_giant_flesh_tentacle' WHERE `entry`=15802;
UPDATE `creature_template` SET `ScriptName`='mob_anubisath_sentinel' WHERE `entry`=15264;

/* THOUSAND NEEDLES */
UPDATE `creature_template` SET `ScriptName` = 'npc_swiftmountain' WHERE `entry` = 10427;
UPDATE `creature_template` SET `ScriptName` = 'npc_plucky' WHERE `entry` = '6626';

/* THUNDER BLUFF */
UPDATE `creature_template` SET `ScriptName`='npc_cairne_bloodhoof' WHERE `entry`=3057;

/* TIRISFAL GLADES */
UPDATE `creature_template` SET `ScriptName`='npc_calvin_montague' WHERE `entry`=6784;

/* ULDAMAN */
UPDATE `instance_template` SET `script`='instance_uldaman' WHERE `map`=70;
UPDATE `creature_template` SET `ScriptName`='boss_ironaya' WHERE `entry`=7228;
UPDATE `creature_template` SET `ScriptName`='mob_jadespine_basilisk' WHERE `entry`=4863;
UPDATE `creature_template` SET `ScriptName`='npc_lore_keeper_of_norgannon' WHERE `entry`=7172;
UPDATE `creature_template` SET `ScriptName`='boss_archaedas' WHERE `entry`=2748;
UPDATE `creature_template` SET `ScriptName`='mob_archaedas_minions' WHERE `entry` IN ('7309', '7077', '7076', '10120');
UPDATE `creature_template` SET `ScriptName`='mob_stonekeepers' WHERE `entry`=4857;
UPDATE `gameobject_template` SET `ScriptName`='go_altar_of_the_keepers' WHERE `entry`=130511;
UPDATE `gameobject_template` SET `ScriptName`='go_altar_of_archaedas' WHERE `entry`=133234;

/* UN'GORO CRATER */
UPDATE `creature_template` SET `ScriptName` = 'npc_ame' WHERE `entry` = 9623;

/* UNDERCITY */
UPDATE `creature_template` SET `ScriptName`='npc_lady_sylvanas_windrunner' WHERE `entry`=10181;
UPDATE `creature_template` SET `ScriptName`='npc_highborne_lamenter' WHERE `entry`=21628;
UPDATE `creature_template` SET `ScriptName`='npc_parqual_fintallas' WHERE `entry`=4488;

/* WAILING CAVERNS */


/* WESTERN PLAGUELANDS */
UPDATE `creature_template` SET `ScriptName`='npcs_dithers_and_arbington' WHERE `entry` IN (11056,11057);
UPDATE `creature_template` SET `ScriptName`='npc_the_scourge_cauldron' WHERE `entry`=11152;

/* WESTFALL */
UPDATE `creature_template` SET `ScriptName`='npc_defias_traitor' WHERE `entry`='467';

/* WETLANDS */


/* WINTERSPRING */
UPDATE `creature_template` SET `ScriptName`='npc_lorax' WHERE `entry`=10918;
UPDATE `creature_template` SET `ScriptName`='npc_rivern_frostwind' WHERE `entry`=10618;
UPDATE `creature_template` SET `ScriptName`='npc_witch_doctor_mauari' WHERE `entry`=10307;


/* ZUL'FARRAK */
UPDATE `creature_template` SET `ScriptName`='npc_sergeant_bly' WHERE `entry`=7604;
UPDATE `creature_template` SET `ScriptName`='npc_weegli_blastfuse' WHERE `entry`=7607;

/* ZUL'GURUB */
UPDATE `instance_template` SET `script`='instance_zulgurub' WHERE `map`=309;
UPDATE `creature_template` SET `ScriptName`='boss_jeklik' WHERE `entry`=14517;
UPDATE `creature_template` SET `ScriptName`='boss_venoxis' WHERE `entry`=14507;
UPDATE `creature_template` SET `ScriptName`='boss_marli' WHERE `entry`=14510;
UPDATE `creature_template` SET `ScriptName`='boss_mandokir' WHERE `entry`=11382;
UPDATE `creature_template` SET `ScriptName`='boss_gahzranka' WHERE `entry`=15114;
UPDATE `creature_template` SET `ScriptName`='boss_jindo' WHERE `entry`=11380;
UPDATE `creature_template` SET `ScriptName`='boss_hakkar' WHERE `entry`=14834;
UPDATE `creature_template` SET `ScriptName`='boss_thekal' WHERE `entry`=14509;
UPDATE `creature_template` SET `ScriptName`='boss_arlokk' WHERE `entry`=14515;
UPDATE `creature_template` SET `ScriptName`='boss_grilek' WHERE `entry`=15082;
UPDATE `creature_template` SET `ScriptName`='boss_hazzarah' WHERE `entry`=15083;
UPDATE `creature_template` SET `ScriptName`='boss_renataki' WHERE `entry`=15084;
UPDATE `creature_template` SET `ScriptName`='boss_wushoolay' WHERE `entry`=15085;
UPDATE `creature_template` SET `ScriptName`='mob_zealot_lorkhan' WHERE `entry`=11347;
UPDATE `creature_template` SET `ScriptName`='mob_zealot_zath' WHERE `entry`=11348;
UPDATE `creature_template` SET `ScriptName`='mob_healing_ward' WHERE `entry`=14987;
UPDATE `creature_template` SET `ScriptName`='mob_spawn_of_marli' WHERE `entry`=15041;
UPDATE `creature_template` SET `ScriptName`='mob_batrider' WHERE `entry`=14965;
UPDATE `creature_template` SET `ScriptName`='mob_shade_of_jindo' WHERE `entry`=14986;

/* EOF */

/* to do: find correct section */
UPDATE `instance_template` SET `script` = 'instance_blackrock_depths' WHERE `map` = 230;
UPDATE `creature_template` SET `ScriptName` = 'mob_voidtraveler' WHERE `entry` = 19226;
UPDATE `creature_template` SET `ScriptName` = 'mob_shadowy_construct' WHERE `entry` = 23111;
UPDATE `creature_template` SET `ScriptName` = 'mob_phalanx' WHERE `entry` = 9502;
UPDATE `creature_template` SET `ScriptName` = 'npc_draenei_survivor' WHERE `entry` = 16483;
UPDATE `creature_template` SET `ScriptName` = 'mob_inner_demon' WHERE entry = 21857;
UPDATE `creature_template` SET `ScriptName` = 'mob_webwrap' WHERE `entry` = 16486;
UPDATE `creature_template` SET `ScriptName` = 'mob_dragonmaw_peon' WHERE `entry` = 22252;
UPDATE `creature_template` SET `ScriptName` = 'npc_karynaku' WHERE `entry` = 22112;
UPDATE `creature_template` SET `ScriptName` = 'mob_ohgan' WHERE `entry` = 14988;

