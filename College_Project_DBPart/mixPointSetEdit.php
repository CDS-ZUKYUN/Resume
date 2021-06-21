<?php

$pathSN = $_POST['pathSN'] ;
$vendorNumber = $_POST['number'] ;
$north =     $_POST['latitude'];
$east  =     $_POST['longitude'];
$preTimeP =  $_POST['preTime'];
//$value = $vendorNumber + ($vendorNumber+1)*4 -4;

$con = mysql_connect("localhost", "root", "");
mysql_query("SET NAMES 'UTF8'");
$myCon = mysql_select_db("AI", $con);
// 3. 先查询, 如果存在就不要在插入了
//$sql1 = "select path_sn from path where path_sn != '$value'";

/*計數路徑點數量，用來限制點個數*/

$result = mysql_query("SELECT count(preTime) from path where path_sn != '$pathSN'and mv_sn ='$vendorNumber'");
//echo mysql_result($result, 0);

/*判斷是否時間設定重複*/

$sql2 = "select preTime from path where  path_sn != '$pathSN' and  mv_sn = '$vendorNumber'";
$result2 = mysql_query($sql2);
//$row = mysql_result($result2,0);
//$replace = "$row[0]";
//print_r($row);
/*判斷是新增還是修改*/
//echo $row;

$j = mysql_result($result, 0);
for($i = 0; $i < $j ; $i++ )
    {
    //echo $i;
    if ($preTimeP == mysql_result($result2, $i))
       { 
       $a = array();
       $a['time'] = "repeat";
       $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
       echo $arr;
        
    // echo '時間設定重複';
       exit;
       }    
    }


if (mysql_result($result, 0) >= 5 ) 
   {
// success 就是key值 对应的value 就是后面的字符串
   $a = array();
   $a['point'] = "overflow";
// $sql2 = "insert into goods values('$value','$vendorNumber','$north', '$east','$preTimeP')";
   $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
   echo $arr;
   }
   
else 
     {
     $sql5 = "select * from path where path_sn ='$pathSN' and mv_sn = '$vendorNumber' ";
     $decision =  mysql_query($sql5);
     if ($decision != NULL)
        {
        $sql4 = "UPDATE `path` SET `pre_North` = '$north' ,`pre_East` = '$east' ,preTime = '$preTimeP' WHERE `path_sn` = '$pathSN' and `mv_sn` = '$vendorNumber'";
    
        mysql_query($sql4);
        
        $a = array();
        $a['edit'] = "set up!"; 
        $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
        echo $arr; 
        
        exit;
        } 
        
        
     else   
        {
        $sql3 = "insert into path values('$pathSN','$vendorNumber','$north', '$east','$preTimeP')";
        $result3 = mysql_query($sql3);
        if ($result3 == 1)
           { // 7. 代表执行成功
           $a = array();
           $a['point'] = "set up.";
        //$a['success'] = "1";
        //$a['good'] = "set ok";
           $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
           echo $arr;
           }    
        else
           {
           $a = array();
           $a['insert'] = "fault";
           $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
           echo $arr;
           exit; 
           }
        
        }   
           
           
           
     }

/*

if (mysql_result($result, 0) > 5 ) 
    {
    // success 就是key值 对应的value 就是后面的字符串
    $a = array();
    $a['point'] = "overflow";
    //$sql2 = "insert into goods values('$value','$vendorNumber','$north', '$east','$preTimeP')";
    $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
    echo $arr;
    }
 else 
    {
    $sql2 = "insert into path values('','$vendorNumber','$north', '$east','$preTimeP')";
    $result2 = mysql_query($sql2);
    //if ($result == 1)
      //  { // 7. 代表执行成功
    $a = array();
    $a['point'] = "set up.";
    //$a['success'] = "1";
    //$a['good'] = "set ok";
    $arr = json_encode($a,JSON_UNESCAPED_UNICODE);
    echo $arr;        
    }
 */