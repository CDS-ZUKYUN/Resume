<?php


$searchName =  $_POST['goodName'] ;

$b = 0;
$arr='';

$con = mysql_connect("localhost", "root", "");
mysql_query("SET NAMES 'UTF8'");
$myCon = mysql_select_db("AI", $con);


$sql1 = "select mv_sn from goods  WHERE `goods_title` = '$searchName'";


$result = mysql_query($sql1);
//$row = mysql_fetch_row($result);
$a = array();
while ($row = mysql_fetch_array($result)) {
  //$row["欄位名稱"]
  //echo "goods_sn=".$row["goods_sn"]."\n<br>";
  //echo "goods_title=".$row["goods_title"]."\n<br>";
  //echo "goods_price".$row["goods_price"]."\n<br>";
    $a["$b.th"] = $row["mv_sn"];

    $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
    $b++;
    
    
}


if($arr)
    {
    echo $arr;
    }
else 
    {
    echo "not any vendor have.";
    }
