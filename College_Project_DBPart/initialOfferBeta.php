<?php

$vendorNumber =  $_POST['number'] ;

$startP   = '0000-00-00' ;
$endP     = '0000-00-00' ;
$contentP = '' ;


$con = mysql_connect("localhost", "root", "");
mysql_query("SET NAMES 'UTF8'");
$myCon = mysql_select_db("AI", $con);



$sql1 = "select mv_sn from mv_vendors where `mv_sn` = '$vendorNumber' ";
$result1 = mysql_query($sql1);

if ($result1) 
        {
        $sql2 = "UPDATE `mv_vendors` SET  off_content = '$contentP', off_start = '$startP' ,off_end = '$endP'  WHERE `mv_sn` = '$vendorNumber'";
    
        mysql_query($sql2);
        $a = array();
        $a['優惠初始'] = "成功";
        $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
        echo $arr;
        }
    
    
    else 
        {
        $a['初始'] = "fail";
        $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
        echo $arr;   
        exit;
        }