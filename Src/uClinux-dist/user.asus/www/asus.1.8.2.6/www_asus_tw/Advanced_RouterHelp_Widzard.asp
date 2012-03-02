<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=big5">
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<meta name="GENERATOR" content="Microsoft FrontPage 4.0">
<meta name="ProgId" content="FrontPage.Editor.Document">
<title>Route Edit Tips</title>
<link rel="stylesheet" type="text/css" href="style.css" media="screen">

<SCRIPT LANGUAGE="JavaScript">

function loadWidzard()
{
   
}

function functionChange(o)
{   
     
}

function optionChange(o)
{      
   
}

function onClose()
{
    window.close();
}

</script>

</head>

<body  onLoad="loadWidzard()">
<form method="POST" name="form" action="--WEBBOT-SELF--" ">  
<!--webbot bot="SaveResults" U-File="fpweb:///_private/form_results.txt"
S-Format="TEXT/CSV" S-Label-Fields="TRUE" -->  
<table border="1" width="542">
  <tr>
    <td class="content_header_td_title"  width="532"><font face="Arial" size="5" color="#FFFFFF">
    <b>路由編輯提示</b>
     </font>
  </td>
  </tr>  
  <tr>
    <td class="content_desc_td" width="532">   
    <p><font size="4"><b>新增/刪除項目</b></font></p>
      <li>
        <b>新增</b> : 請將資料輸入表格中的第一排，接著按下<b>新增</b>將資料插入。如果此時表格已滿或是所需欄位填寫不實，系統將會提醒您。
      </li> 
      <li>
        <b>刪除</b> : 請選擇表格中的一項，或按下<b>Ctrl</b>鍵並在表格中選擇多個項目，然後按下<b>Del</b>鍵將它們刪除。
      </li> 
    <p><font size="4"><b>輸入格式</b></font></p>        
      <p></p>           
      <li>
        <b>網路/主機IP</b> : 其代表的是一路由規則的目的網路或目的主機。因此它可以是一主機位址，如「192.168.123.11」或是一網路位址「192.168.0.0」。
      </li>       
      <li>
        <b>網路遮罩位元</b> : 它可指出網路位址（network ID）及子網路位址（subnet ID）所使用的位元數。例如：假設點十進位（dotted-decimal）網路遮罩是255.255.255.0，那麼它的網路遮罩位元就是24。假設該目的為一主機，那麼它的網路遮罩位元應該就是32。
      </li> 
      <li>
        <b>閘道器</b> : 它代表的是封包發送目的的閘道器的IP位址。此一特定閘道器必須是最先抵達。意即是您必須事前設定好至該閘道器的靜態路由。
      </li>            
    <p align="right" style="margin-right:20"><input type="button" onClick="onClose()" value="關閉" name="Close1">    
        <p></p>
    </td>
  </tr>
</table>
</form>
</body>

</html>
