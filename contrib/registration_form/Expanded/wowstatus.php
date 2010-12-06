<link rel="stylesheet" type="text/css" href="stylesheet/mall.css">
<title>Status Page</title>
<center><tr>
<td class="alt2"></tr>
<br /><br /><br /><br /><br /><br />
<td class="alt1"><center><b><h2>Characters Online</h2></b></center></td></tr>
<td><table border=1px bordercolor="#FFFFFF" cellspacing=0 cellpadding=3>
<tr>
<?php
$Hostname = "localhost";
$Username = "root";
$Password = "PASSWORD";
$CharacterDatabase = 'characters';
$RealmDatabase = 'realmd';
$WorldDatabase = 'world';
$CharacterDatabaseEncoding = 'utf8'; 

$conn = mysql_connect($Hostname, $Username, $Password) or die('Connection failed: ' . mysql_error());

mysql_select_db($CharacterDatabase, $conn) or die('Select DB failed: ' . mysql_error());

$sql = "SELECT * FROM `characters` WHERE `online` = 1 ORDER BY `name`";
$result = mysql_query($sql, $conn) or die('Query failed: ' . mysql_error());

$count = 0;


echo "<TR><Td class=\"tcat\">Name</td><td class=\"tcat\">Level</td><td class=\"tcat\">Class</td><td class=\"tcat\">Race</td></TR>";

while ($characters = mysql_fetch_array($result)) {
echo "<TR>";

$char_data = explode(' ',$characters['data']);
$char_gender = dechex($char_data[36]);
$char_gender = str_pad($char_gender,8, 0, STR_PAD_LEFT);
$level = $char_data[34];
$gender = $gender{3};
$race=$characters['race'];
$class=$characters['class'];

echo "<TD align=center class=\"tfoot\">";
if ($race==2 || $race==5 || $race==6 || $race==8) echo "<FONT COLOR=\"#FF0000\">";
if ($race==1 || $race==3 || $race==4 || $race==7) echo "<FONT COLOR=\"#0000FF\">";
echo $characters['name'];
echo "</FONT>";
echo "</TD>";

echo "<TD align=center class=\"tfoot\">";
if ($level==60) echo "<b> <font color=\"white\"/b>";
echo $level;
echo "</font></TD>";

echo "<TD align=center class=\"tfoot\">";
if ($class == 1) echo "<center><img src=\"images/wow/1.gif\"></center>";
if ($class == 2) echo "<center><img src=\"images/wow/2.gif\"></center>";
if ($class == 3) echo "<center><img src=\"images/wow/3.gif\"></center>";
if ($class == 4) echo "<center><img src=\"images/wow/4.gif\"></center>";
if ($class == 5) echo "<center><img src=\"images/wow/5.gif\"></center>";
if ($class == 7) echo "<center><img src=\"images/wow/6.gif\"></center>";
if ($class == 8) echo "<center><img src=\"images/wow/8.gif\"></center>";
if ($class == 9) echo "<center><img src=\"images/wow/9.gif\"></center>";
if ($class == 11) echo "<center><img src=\"images/wow/11.gif\"></center>";
echo "</font></TD>";

echo "<TD align=center class=\"tfoot\">";
if ($race == 1) echo "<center><img src=\"images/wow/1-0.gif\"></center>";
if ($race == 2) echo "<center><img src=\"images/wow/2-0.gif\"></center>";
if ($race == 3) echo "<center><img src=\"images/wow/3-0.gif\"></center>";
if ($race == 4) echo "<center><img src=\"images/wow/4-0.gif\"></center>";
if ($race == 5) echo "<center><img src=\"images/wow/5-0.gif\"></center>";
if ($race == 6) echo "<center><img src=\"images/wow/6-0.gif\"></center>";
if ($race == 7) echo "<center><img src=\"images/wow/7-0.gif\"></center>";
if ($race == 8) echo "<center><img src=\"images/wow/8-0.gif\"></center>";
echo "</font></TD>";


echo "</TR>";
$count++;
}

echo "</center>";
?>
</table>

<?php
$realm_db = mysql_connect($Hostname, $Username, $Password);
mysql_select_db($RealmDatabase, $realm_db);
$db_result = mysql_query("SET NAMES $CharacterDatabaseEncoding", $realm_db);

$world_db = mysql_connect($Hostname, $Username, $Password, TRUE);
mysql_select_db($CharacterDatabase, $world_db);
$db_result = mysql_query("SET NAMES $CharacterDatabaseEncoding", $world_db);
 
$uptime_query = mysql_query("SELECT * FROM $WorldDatabase.`uptime` ORDER BY `starttime` DESC LIMIT 1", $world_db)or die(mysql_error()); 
$uptime_results = mysql_fetch_array($uptime_query); 
$maxplayers =  $uptime_results['maxplayers'];
if ($uptime_results['uptime'] > 86400) { //days
    $uptime =  round(($uptime_results['uptime'] / 24 / 60 / 60),2)." Days";
}
elseif($uptime_results['uptime'] > 3600) { //hours
    $uptime =  round(($uptime_results['uptime'] / 60 / 60),2)." Hours";
}
else { //minutes
    $uptime =  round(($uptime_results['uptime'] / 60),2)." Minutes";
}

$player_query = mysql_query("SELECT (SELECT COUNT(guid) FROM $CharacterDatabase.`characters` WHERE race IN(2,5,6,8,10) AND `online`='1') as horde, (SELECT COUNT(guid) FROM $CharacterDatabase.`characters` WHERE race IN(1,3,4,7,11) AND `online`='1') as alliance FROM $CharacterDatabase.`characters`", $world_db)or die(mysql_error()); 
$player_results = mysql_fetch_array($player_query); 
$horde =  $player_results['horde'];
$alliance =  $player_results['alliance'];
$total = $horde + $alliance;

echo "<table width=\"25%\" border=0 cellspacing=0 cellpadding=3 class=\"$bgclass\" align=\"left\">
  <tr>
    <td class=\"tcat\">Uptime:</td>
    <td class=\"tfoot\">" . $uptime . "</td>
  <tr>
    <td class=\"tcat\">Players online:</td>
    <td class=\"tfoot\">" . $total . "</td>
  </tr>
  </tr>
  <tr>
    <td class=\"tcat\">Max online:</td>
    <td class=\"tfoot\">" . $maxplayers . "</td>
  </tr>
  <tr>
    <td class=\"tcat\">Alliance online:</td>
    <td class=\"tfoot\">" . $alliance . "</td>
  </tr>
  <tr>
    <td class=\"tcat\">Horde online:</td>
    <td class=\"tfoot\">" . $horde . "</td>
  </tr>
</table>";
?></td>
</tr>
</td>
