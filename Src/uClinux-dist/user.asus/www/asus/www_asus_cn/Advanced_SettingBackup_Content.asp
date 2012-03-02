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
<body bgcolor="#FFFFFF">
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>    
<form method="post" action="upload.cgi" enctype="multipart/form-data">
<input type="hidden" name="action_mode" value="">
<!-- Table for the conntent page -->	    
<table width="660" border="0" cellpadding="1" cellspacing="0">     	      
    	
<tr>
<td>
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">系统设置 - 设置管理</td>
<input type="hidden" name="current_page" value="Basic_Operation_Content.asp">
<input type="hidden" name="next_page" value="Basic_SaveRestart.asp">
</tr>
<tr>
<td class="content_desc_td" colspan="2" width="614">
                  <p style="margin-top: 7">本项功能可让您在ZVMODELVZ上所做的设置保存成文件，或从文件中载入设置。
</td>
</tr>

<tr class="content_section_header_tr">
<td id="Mode" class="content_section_header_td" colspan="2" width="614">另存为新文件</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2" width="614">
<p style="margin-top: 7"> 
                 请将您的光标移到文件上。 <a href="/ZVMODELVZ.CFG">HERE</a>. 接着点选鼠标右键并选择<b>「另存为....」</b>将目前ZVMODELVZ的设置保存成文件。（注：在您将目前的设置保存成文件时，亦同样会存入闪存内。）
<p style="margin-buttom: 7">                    
</td>
</tr>
<tr class="content_section_header_tr">
<td id="Mode" class="content_section_header_td" colspan="2" width="614">从文件中载入</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2" width="614">                           		                                     		
              <p style="margin-top: 7">
              请将下载文件的路径及名称指定到下方的<b>「新设置档」</b>中。接着，点选<b>「上传」</b> 将文件写入ZVMODELVZ。整个过程需要一些时间来完成，然后系统将会重新开机。            
</td>
</tr>
<tr>
<td class="content_header_td" width="30">新设置文件：</td><td class="content_input_td">   
                  <input class="inputSubmit" name="file" size="20" type="file" ></td>
</tr>
<tr>
<td class="content_header_td"></td><td class="content_input_td" width="595">
  <p align="left"><input class="inputSubmit" type="submit" onClick="onSubmitCtrlOnly(this, 'Upload')" value="上传">
  </p>
</td>
</tr>
</table>
</td>
</tr>
</table>
</form>
</body>
