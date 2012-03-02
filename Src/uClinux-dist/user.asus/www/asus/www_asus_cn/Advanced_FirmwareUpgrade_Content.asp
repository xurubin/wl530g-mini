<head>
<title>ZVMODELVZ Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<meta http-equiv="Content-Type" content="text/html">
<link rel="stylesheet" type="text/css" href="style.css" media="screen">
<script language="JavaScript" type="text/javascript" src="overlib.js"></script>
<script language="JavaScript" type="text/javascript" src="general.js"></script>
<script language="JavaScript" type="text/javascript" src="quick.js"></script>
</head>  
<body bgcolor="#FFFFFF" onload="load_upload()">
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>    
<form method="post" action="upgrade.cgi" name="form" enctype="multipart/form-data">
<!-- Table for the conntent page -->	    
<table width="660" border="0" cellpadding="1" cellspacing="0">     	      
    	
<tr>
<td>
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">系统设置 - 固件升级</td>
<input type="hidden" name="current_page" value="Basic_Operation_Content.asp">
<input type="hidden" name="next_page" value="Basic_SaveRestart.asp">
<input type="hidden" name="action_mode" value="">
</tr>
<tr class="content_section_header_tr">
<td id="Mode" class="content_section_header_td" colspan="2" width="614"> 请按照下列说明：</td>
</tr>

<tr>
<td class="content_desc_td" colspan="2" width="614">
              	<ol>
              		<li>
              <p style="margin-top: 7">
                    请到华硕网站查看是否有提供新版的固件。
              </li>
              		<li>
              <p style="margin-top: 7">
                    将正确的版本下载至您个人的设备上。               
              </li>
              		<li>
              <p style="margin-top: 7">
                    请将下载文件的路径及名称指定到「新固件文件」中。
              </li>
              		<li>
              <p style="margin-top: 7">
                    点选「上传」将文件上传至 ZVMODELVZ。整个过程需时约80秒钟。
              </li>
              <li>
              <p style="margin-top: 7">
                    在收到正确的固件文件后，ZVMODELVZ即自动开始升级的过程。整个过程需要一些时间来完成，接着系统将会重新开机。
 	      </li>
              </ol>
</td>
</tr>
<tr>
		<td class="content_header_td" width="30">产品型号：</td><td class="content_input_td" width="595"><input type="text" value="<% nvram_get_f("general.log","productid"); %>" readonly="1"></td>
</tr>
<tr>
		<td class="content_header_td" width="30">固件版本：</td><td class="content_input_td" width="595"><input type="text" value="<% nvram_get_f("general.log","firmver"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td" width="30">新固件文件：</td><td class="content_input_td">  
                  <input class="inputSubmit" name="file" size="20" type="file" ></td>
</tr>
<tr>
<td class="content_header_td"></td><td class="content_input_td" width="595">
  <p align="left"><input class="inputSubmit" name="button" onClick="onSubmitCtrlOnly(this, 'Upload1')" type="button" value="上传"> 
  </p>
</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2" width="614">
	             <b>注意：</b>
                     <ol>
                     	<li>针对同时存在于新旧固件中的设置参数，其设置在整个升级的过程中会一直保留。</li>
                     	<li>当升级过程失败，ZVMODELVZ会自动进入紧急模式。而ZVMODELVZ正面上的LED指示灯会指示此种情况。请使用光盘上的「固件恢复」（Firmware Restoration）公用程序进行系统还原操作。</li>
                     </ol>
</td>
</tr>
</table>
</td>
</tr>
</table>
</form>
</body>
