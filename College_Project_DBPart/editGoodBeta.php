<?php

$goodsnP = $_POST['Goodsn'] ;
$itemP  = $_POST['Item'] ;                  //  'koko肉';   
$priceP = $_POST['Price'] ;                  //   '23' ;


$con = mysql_connect("localhost", "root", "");
mysql_query("SET NAMES 'UTF8'");
$myCon = mysql_select_db("AI", $con);


$sql1 = "select goods_sn from goods WHERE `goods_sn` = '$goodsnP'";
$result1 = mysql_query($sql1);


if(mysql_num_rows($result1) ) 
    {
    $sql2 = "UPDATE `goods` SET `goods_title` = '$itemP',`goods_price` = '$priceP' WHERE `goods_sn` = '$goodsnP'";
    
    mysql_query($sql2);
    $a = array();
    $a['編輯'] = "成功";
    $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
    echo $arr;
    }
 else 
    {
    echo "回家吸奶去!!!";   
    }