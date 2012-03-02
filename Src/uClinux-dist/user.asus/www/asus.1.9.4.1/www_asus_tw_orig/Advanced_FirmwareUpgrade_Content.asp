<head>
<title>ZVMODELVZ Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=euc-kr">
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
<td class="content_header_td_title" colspan="2">시스템 설정 - 펌웨어 업그레이드</td>
<input type="hidden" name="current_page" value="Basic_Operation_Content.asp">
<input type="hidden" name="next_page" value="Basic_SaveRestart.asp">
<input type="hidden" name="action_mode" value="">
</tr>
<tr class="content_section_header_tr">
<td id="Mode" class="content_section_header_td" colspan="2" width="614"> 아래의 설명을 따름:</td>
</tr>

<tr>
<td class="content_desc_td" colspan="2" width="614">
              	<ol>
              		<li>
              <p style="margin-top: 7">
                    새로운 펌웨어 버전이 ASUS 웹사이트에 나와 있는 지를 체크합니다.
              </li>
              		<li>
              <p style="margin-top: 7">
                     로컬 컴퓨터에 적합한 버전을 다운로드합니다.               
              </li>
              		<li>
              <p style="margin-top: 7">
                    "새로운 펌웨어 파일"에서 다운로드하는 파일의 경로와 이름을 지정합니다.
              </li>
              		<li>
              <p style="margin-top: 7">
                     "업로드"를 클릭하여 파일을 ZVMODELVZ로 업로드 합니다. 약 10초 정도 소요됩니다.
              </li>
              <li>
              <p style="margin-top: 7">
                    정확한 펌웨어 파일을 받은 후 ZVMODELVZ 가 업그레이드 과정을 자동으로 시작합니다. 잠시 후 업그레이드 과정이 끝나고 시스템이 재부팅됩니다.
 	      </li>
              </ol>
</td>
</tr>
<tr>
		<td class="content_header_td" width="30">제품 ID:</td><td class="content_input_td" width="595"><input type="text" value="<% nvram_get_f("general.log","productid"); %>" readonly="1"></td>
</tr>
<tr>
		<td class="content_header_td" width="30">펌웨어 버전:</td><td class="content_input_td" width="595"><input type="text" value="<% nvram_get_f("general.log","firmver"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td" width="30">새로운 펌웨어 파일:</td><td class="content_input_td">  
                  <input class="inputSubmit" name="file" size="20" type="file" ></td>
</tr>
<tr>
<td class="content_header_td"></td><td class="content_input_td" width="595">
  <p align="left"><input class="inputSubmit" name="button" onClick="onSubmitCtrlOnly(this, 'Upload1')" type="button" value="업로드"> 
  </p>
</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2" width="614">
	             <b>참고:</b>
                     <ol>
                     	<li>이전 펌웨어와 새로운 펌웨어의 컨피규레이션 파라미터는 업그레이드 과정동안 유지됩니다.</li>
                     	<li>업그레이드 과정이 실패하면 ZVMODELVZ가 자동으로 비상모드 상태가 됩니다. ZVMODELVZ의 앞에 위치한 LED 표시등이 그러한 상황을 알려 줍니다. 시스템 복구를 하려면 펌웨어 복구 유틸리티를 사용합니다.</li>
                     </ol>
</td>
</tr>
</table>
</td>
</tr>
</table>
</form>
</body>
