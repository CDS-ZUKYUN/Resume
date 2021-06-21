<?php

$vendorNumber  =  $_POST['number'] ;

$b = 0;

$con = mysql_connect("localhost", "root", "");
mysql_query("SET NAMES 'UTF8'");
$myCon = mysql_select_db("AI", $con);

$sql1 = "select path_sn,pre_North,pre_East,preTime from path WHERE `mv_sn` = '$vendorNumber'";
//$result1 = mysql_query($sql1);


//$data=  mysql_fetch_row($result1);
//echo $data[0];



$result = mysql_query($sql1);
//$row = mysql_fetch_row($result);
$a = array();
while ($row = mysql_fetch_array($result)) {
  //$row["欄位名稱"]
  //echo "goods_sn=".$row["goods_sn"]."\n<br>";
  //echo "goods_title=".$row["goods_title"]."\n<br>";
  //echo "goods_price".$row["goods_price"]."\n<br>";
    $a["path_sn$b"] = $row["path_sn"];
    $a["pre_North$b"] = $row["pre_North"];
    $a["pre_East$b"] = $row["pre_East"];
    $a["preTime$b"] = $row["preTime"];
    $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
    $b++;
    
    
}

if($arr)
    {
    echo $arr;
    }
else 
    {
    $a['path'] = "fetch fault";
    $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
    echo $arr;
    
    }
