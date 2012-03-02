<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
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
    <b>路径编辑提示</b>
     </font>
  </td>
  </tr>  
  <tr>
    <td class="content_desc_td" width="532">   
    <p><font size="4"><b>增加/删除项目</b></font></p>
      <li>
        <b>增加</b> : 请将资料输入表格中的第一排，接着按下<b>增加</b>将资料插入。如果此时表格已满或是所需栏目填写不实，系统将会提醒您。
      </li> 
      <li>
        <b>删除</b> : 请选择表格中的一项，或按下<b>Ctrl</b>键并在表格中选择多个项目，然后按下<b>Del</b>键将它们删除。
      </li> 
    <p><font size="4"><b>输入格式</b></font></p>        
      <p>路径规则的项目列在下方：</p>           
      <li>
        <b>网络/主机IP</b> : 其代表的是路由规则的目的网络或目的主机。因此它可以是一主机地址，如「192.168.123.11」或是一网络地址「192.168.0.0」。
      </li>       
      <li>
        <b>网络掩码比特</b> : 它可指出网络地址（network ID）及子网地址（subnet ID）所使用的字节数。例如：假设点十进制（dotted-decimal）网络掩码是255.255.255.0，那么它的网络掩码比特就是24。假设该目的为一主机，那么它的网络掩码比特应该就是32。
      </li> 
      <li>
        <b>网关</b> : 它代表的是封包发送目的地网关的IP地址。此一特定网关必须是最先抵达。这意味着您必须事先设置好至该网关的静态路由。
      </li>            
    <p align="right" style="margin-right:20"><input type="button" onClick="onClose()" value="关闭" name="Close1">    
        <p></p>
    </td>
  </tr>
</table>
</form>
</body>

</html>
