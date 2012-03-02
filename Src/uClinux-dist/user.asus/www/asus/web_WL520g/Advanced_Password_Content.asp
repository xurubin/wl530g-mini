<head>
<title>WL520g Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=euc-kr">
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<link rel="stylesheet" type="text/css" href="style.css" media="screen">
<script language="JavaScript" type="text/javascript" src="overlib.js"></script>
<script language="JavaScript" type="text/javascript" src="general.js"></script>

</head>  
<body bgcolor="#FFFFFF"  onLoad="load_body()">
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>    
<form method="GET" name="form" action="apply.cgi">
<input type="hidden" name="action_mode" value="">
<!-- Table for the conntent page -->	    
<table width="660" border="0" cellpadding="1" cellspacing="0">     	      
    	
<tr>
<td>
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">시스템 설정 - 비밀번호 변경</td>
<input type="hidden" name="current_page" value="Advanced_Password_Content.asp">
<input type="hidden" name="next_page" value="SaveRestart.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="General;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
</tr>
<tr>
		<td class="content_header_td" width="344" colspan="2"></td>
</tr>
<tr>
		<td class="content_header_td" width="34">새로운 비밀번호:</td><td class="content_input_td" width="310"><input type="password" maxlength="16" size="16" name="http_passwd"></td>
</tr>
<tr>
		<td class="content_header_td" width="34">새로운 비밀번호 재입력:</td><td class="content_input_td" width="310"><input type="password" maxlength="16" size="16" name="v_password"></td>
</tr>
<tr>
<td class="content_input_td_less" height="60" colspan="2">
  <p align="right"> 
                  <input class="inputSubmit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" type="submit" value="저장" name="action" onClick="return checkPass(this, document.form.http_passwd, document.form.v_password);">&nbsp;<input class="inputSubmit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" type="reset" value="취소" name="reset">
  </p>
</td>
</tr>
</table>
</td>
</tr>
</table>
</form>
</body>
