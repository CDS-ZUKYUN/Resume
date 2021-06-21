<?php

$vendorNumber =  $_POST['number'];

$con = mysql_connect("localhost", "root", "");
mysql_query("SET NAMES 'UTF8'");
$myCon = mysql_select_db("AI", $con);


$sql1 = "select off_content , off_start , off_end from mv_vendors  WHERE  mv_sn = '$vendorNumber'";
$result1 = mysql_query($sql1);


$a = array();
while ($row = mysql_fetch_array($result1)) {
    //$row["欄位名稱"]
    //echo "goods_sn=".$row["goods_sn"]."\n<br>";
    //echo "goods_title=".$row["goods_title"]."\n<br>";
    //echo "goods_price".$row["goods_price"]."\n<br>";
    $a["off_content"] = $row["off_content"];
    $a["off_start"] = $row["off_start"];
    $a["off_end"] = $row["off_end"];
    $arr = json_encode($a, JSON_UNESCAPED_UNICODE);
}


if($arr)
    {
    echo $arr;
    }
else 
    {
    echo "offer info get fail.";
    }
