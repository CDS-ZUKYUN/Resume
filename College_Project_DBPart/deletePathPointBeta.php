<?php // 1. 获取客户端利用post方式网络请求的body里的字段对应的value (这个字段// 是这里规定的, 前端必须遵守这个name2, pass2等key值)

$pathSN       = $_POST['pathsn'] ; 
$vendorNumber = $_POST['number'] ;

$con = mysql_connect("localhost", "root", "");
mysql_query("SET NAMES 'UTF8'");
$myCon = mysql_select_db("AI", $con);
// 3. 先查询, 如果存在就不要在插入了
$select = "select path_sn ,mv_sn from path where path_sn ='$pathSN' and `mv_sn` = '$vendorNumber' ";
$sql1 = "DELETE FROM path WHERE path_sn = '$pathSN' and `mv_sn` = '$vendorNumber'";

$seleResult = mysql_query($select);

//echo $seleResult;

if (mysql_num_rows($seleResult)) 
    {
    // success 就是key值 对应的value 就是后面的字符串
    mysql_query($sql1);
    $a = array();
    $a['point'] = "delete success";
    $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
    echo $arr;
 
    }
// 5. 如果没注册过, 那么
else 
    {
    // 6. 把数据都插入到mysql数据库中
    $a = array();
    $a['point'] = "not exist";
    $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
    echo $arr;
    exit;
        //}
    /*
    else 
        { // 8. 代表插入失败
        $a = array();
        $a['success'] = "0";
        $a['good'] = "set fault";
        $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
        echo $arr;

        }
    */
    }
  


?>
