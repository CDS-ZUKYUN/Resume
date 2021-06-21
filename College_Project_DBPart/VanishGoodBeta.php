<?php // 1. 获取客户端利用post方式网络请求的body里的字段对应的value (这个字段// 是这里规定的, 前端必须遵守这个name2, pass2等key值)

$vendorNumber =  $_POST['number'] ;      // '9'    
$itemP  =   $_POST['Item'] ;            // '心' 

$con = mysql_connect("localhost", "root", "");
mysql_query("SET NAMES 'UTF8'");
$myCon = mysql_select_db("AI", $con);
// 3. 先查询, 如果存在就不要在插入了
$select = "select mv_sn,goods_title from goods where mv_sn = '$vendorNumber' And goods_title = '$itemP' ";
$sql1 = "DELETE FROM goods WHERE mv_sn = '$vendorNumber' And goods_title = '$itemP'";

$seleResult = mysql_query($select);

// 4. 如果查到了, 说明已经存在这个用户了, 则返回-1给客户端代表已经注册过了


$sql2 = "select amount from mv_vendors where mv_sn = '$vendorNumber'";
$takeoff = mysql_query($sql2);
$row = mysql_fetch_row($takeoff);
//echo $row[0];
$save = $row[0];

if (mysql_num_rows($seleResult)) 
    {
    // success 就是key值 对应的value 就是后面的字符串
    mysql_query($sql1);
    $a = array();
    $a['good'] = "delete success";
    $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
    echo $arr;
    
    
    
    $save = $save + 1;
    $sql3 = "UPDATE `mv_vendors` SET `amount` = '$save'  WHERE `mv_sn` = '$vendorNumber'";
    mysql_query($sql3);
    }
// 5. 如果没注册过, 那么
else 
    {
        $a = array();
        $a['status'] = "fault";
        $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
        echo $arr;

    }
  


?>
