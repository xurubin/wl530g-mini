<head>
<title>ZVMODELVZ Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=big5">
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
<form method="post" action="upgrade.cgi" enctype="multipart/form-data">
<!-- Table for the conntent page -->	    
<table width="660" border="0" cellpadding="1" cellspacing="0">     	      
    	
<tr>
<td>
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">系統設定 - 韌體升級</td>
<input type="hidden" name="current_page" value="Basic_Operation_Content.asp">
<input type="hidden" name="next_page" value="Basic_SaveRestart.asp">
<input type="hidden" name="action_mode" value="">
</tr>
<tr class="content_section_header_tr">
<td id="Mode" class="content_section_header_td" colspan="2" width="614"> 請遵照下列指示：</td>
</tr>

<tr>
<td class="content_desc_td" colspan="2" width="614">
              	<ol>
              		<li>
              <p style="margin-top: 7">
                    請至華碩網站查看是否有提供新版的韌體。
              </li>
              		<li>
              <p style="margin-top: 7">
                    將正確的版本下載至您個人的設備上。               
              </li>
              		<li>
              <p style="margin-top: 7">
                    請將下載檔案的路徑及名稱指定到「新韌體檔案」中。
              </li>
              		<li>
              <p style="margin-top: 7">
                    點選「上傳」將檔案上傳至 ZVMODELVZ。整個過程需時約10秒鐘。
              </li>
              <li>
              <p style="margin-top: 7">
                    在收到正確的韌體檔案後，ZVMODELVZ即自動開始升級的過程。整個過程需要一些時間來完成，接著系統將會重新開機。
 	      </li>
              </ol>
</td>
</tr>
<tr>
		<td class="content_header_td" width="30">產品型號：</td><td class="content_input_td" width="595"><input type="text" value="<% nvram_get_f("general.log","productid"); %>" readonly="1"></td>
</tr>
<tr>
		<td class="content_header_td" width="30">韌體版本：</td><td class="content_input_td" width="595"><input type="text" value="<% nvram_get_f("general.log","firmver"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td" width="30">新韌體檔案：</td><td class="content_input_td">  
                  <input class="inputSubmit" name="file" size="20" type="file" ></td>
</tr>
<tr>
<td class="content_header_td"></td><td class="content_input_td" width="595">
  <p align="left"><input class="inputSubmit" onClick="onSubmitCtrlOnly(this, 'Upload')" type="submit" value="上傳"> 
  </p>
</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2" width="614">
	             <b>註：</b>
                     <ol>
                     	<li>針對同時存在於新舊韌體中的組態參數，其設定值在整個升級的過程中會一直保留。</li>
                     	<li>當升級過程失敗，ZVMODELVZ會自動進入緊急模式。而ZVMODELVZ正面上的LED信號即會指示此種情況。請使用光碟上的「韌體修復」（Firmware Restoration）公用程式進行系統還原作業。</li>
                     </ol>
</td>
</tr>
</table>
</td>
</tr>
</table>
</form>
</body>
