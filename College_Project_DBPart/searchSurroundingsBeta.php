<?php
$selflatitude = $_POST['latitude'];
$selflongitude = $_POST['longtitude'];

$con = mysql_connect("localhost", "root", "");
mysql_query("SET NAMES 'UTF8'");
$myCon = mysql_select_db("AI", $con);



$sql1 = "select mv_sn from mv_vendors where mv_status = 1";
$sql2 = "select count(mv_status) from mv_vendors where mv_status = 1";
$result1 = mysql_query($sql1) or die(mysql_error());
$result2 = mysql_query($sql2) or die(mysql_error());

$j = mysql_result($result2,0);
if($j == 0)
{
  $a = array();
  $a['mv_vendors'] = "no one on sell.";
  $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
  echo $arr;  
}  
else
{
for($i = 0 ;$i < $j;$i++)
{
    $x = mysql_result($result1,$i);
    $sql3 = "select mv_North,mv_East from mv_vendors where mv_sn = $x";
    $result3 = mysql_query($sql3);
    $latitude = mysql_result($result3,0,'mv_North');
    $longitude = mysql_result($result3,0,'mv_East');
    
    $distance = getdistance($selflatitude,$selflongitude, $latitude, $longitude);
    if($distance < 500)
        {
        $a = array();
        $a['mv_sn'] = $x;
        $a['mv_North'] = $latitude;
        $a['mv_East'] = $longitude;
        $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
        echo $arr;
        }
//echo $latitude,$longitude;
}
}
//$result2 = mysql_query("select count * from mv_vendors where mv_status = '1'")or die(mysql_error());
//echo mysql_result($result2,0);
/*
for($i = 0 ;$i < $count;$i++)
    echo mysql_result($result1,$i);
*/
function getdistance($lat1,$lng1,$lat2,$lng2){
    //将角度转为狐度
    $radLat1=deg2rad($lat1);//deg2rad()函数将角度转换为弧度

    $radLat2=deg2rad($lat2);

    $radLng1=deg2rad($lng1);

    $radLng2=deg2rad($lng2);

    $a=$radLat1-$radLat2;

    $b=$radLng1-$radLng2;

    $s=2*asin(sqrt(pow(sin($a/2),2)+cos($radLat1)*cos($radLat2)*pow(sin($b/2),2)))*6378.137*1000;

    return $s;
}



