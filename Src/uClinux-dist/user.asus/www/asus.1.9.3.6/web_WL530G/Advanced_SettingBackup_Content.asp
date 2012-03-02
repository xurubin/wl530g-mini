<head>
<title>WL530G Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=euc-kr">
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
<td class="content_header_td_title" colspan="2">시스템 설정 - 설정관리</td>
<input type="hidden" name="current_page" value="Basic_Operation_Content.asp">
<input type="hidden" name="next_page" value="Basic_SaveRestart.asp">
</tr>
<tr>
<td class="content_desc_td" colspan="2" width="614">
                  <p style="margin-top: 7">본 기능은 WL530G의 현재 설정을 파일로 저장하거나 파일로 부터 설정을 가져 올 수 있도록 합니다.
</td>
</tr>

<tr class="content_section_header_tr">
<td id="Mode" class="content_section_header_td" colspan="2" width="614">파일로 저장</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2" width="614">
<p style="margin-top: 7"> 
                 커서 이동 <a href="/WL530G.CFG">HERE</a>. 마우스 오른 쪽 버튼을 클릭한 후 <b>"…….로 저장"</b>을 선택하여 WL530G의 현재 설정을 파일로 저장합니다. (참고: 현재 설정을 파일로 저장하는 동안 플래시로도 저장됩니다.
<p style="margin-buttom: 7">                    
</td>
</tr>
<tr class="content_section_header_tr">
<td id="Mode" class="content_section_header_td" colspan="2" width="614">파일로 부터 가져오기</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2" width="614">                           		                                     		
              <p style="margin-top: 7">
              다운로드하는 파일의 경로명과 이름을 아래의 <b>"새 설정 파일"</b>에서 정합니다. 그리고 난 후 <b>"업로드"</b>를 클릭하여 파일을 WL530G에 쓰기합니다. 잠시 후 본 과정이 끝나고 시스템이 재부팅됩니다.            
</td>
</tr>
<tr>
<td class="content_header_td" width="30">새 설정 파일:</td><td class="content_input_td">   
                  <input class="inputSubmit" name="file" size="20" type="file" ></td>
</tr>
<tr>
<td class="content_header_td"></td><td class="content_input_td" width="595">
  <p align="left"><input class="inputSubmit" type="submit" onClick="onSubmitCtrlOnly(this, 'Upload')" value="업로드">
  </p>
</td>
</tr>
</table>
</td>
</tr>
</table>
</form>
</body>
