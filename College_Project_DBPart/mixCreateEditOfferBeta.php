<?php

$vendorNumber =   $_POST['number'] ;

$startP   = $_POST['startDate'] ;
$endP     = $_POST['endDate'] ;
$contentP = $_POST['content'] ;

$con = mysql_connect("localhost", "root", "");
mysql_query("SET NAMES 'UTF8'");
$myCon = mysql_select_db("AI", $con);

//$sql3 = "UPDATE `mv_vendors` SET  off_content = '$contentP' WHERE `mv_sn` = '$vendorNumber'";
    
  //  mysql_query($sql3);

$sql1 = "select mv_sn from mv_vendors where `mv_sn` = '$vendorNumber' ";
$result1 = mysql_query($sql1);

$sql3 = "select off_content from mv_vendors where mv_sn = '$vendorNumber'";
$result3 = mysql_query($sql3);


 if ($contentP == mysql_result($result3, 0))
       { 
       $a = array();
       $a['offer content'] = "repeat";
       $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
       echo $arr;
        
    // echo '時間設定重複';
       exit;
       }    
else
    {
    if ($result1) 
        {
        $sql2 = "UPDATE `mv_vendors` SET  off_content = '$contentP', off_start = '$startP' ,off_end = '$endP'  WHERE `mv_sn` = '$vendorNumber'";
    
        mysql_query($sql2);
        $a = array();
        $a['優惠設置or修改'] = "成功";
        $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
        echo $arr;
        }
    
    
    else 
        {
        $a['fault'] = "offer not set up";
        $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
        echo $arr;   
        exit;
        }
    }





