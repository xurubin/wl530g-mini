<head>
<title>WL530G Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=euc-kr">
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<link rel="stylesheet" type="text/css" href="style.css" media="screen"></link>
<script language="JavaScript" type="text/javascript" src="overlib.js"></script>
<script language="JavaScript" type="text/javascript" src="general.js"></script>
</head>  
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>    
<body onLoad="load_body()" onunLoad="return unload_body();">
<form method="GET" name="form" action="apply.cgi">
<!-- Table for the conntent page -->	    
<table width="666" border="0" cellpadding="0" cellspacing="0">     	      
    	
<input type="hidden" name="current_page" value="Advanced_USBStorage_Content.asp"><input type="hidden" name="next_page" value="Advanced_WebCam_Content.asp"><input type="hidden" name="next_host" value=""><input type="hidden" name="sid_list" value="PrinterStatus;"><input type="hidden" name="group_id" value=""><input type="hidden" name="modified" value="0"><input type="hidden" name="action_mode" value=""><input type="hidden" name="first_time" value=""><input type="hidden" name="action_script" value="">
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">USB 어플리케이션 - FTP 서버</td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('USB 디스크를 제거하기 전에 이 버튼을 눌러 캐시 데이터를 디스크에 기록하고 이 페이지가 다시 나타날 때까지 기다리십시오.', LEFT);" onMouseOut="return nd();">USB 디스크 빼냄:
           </td><td class="content_input_td"><input type="hidden" maxlength="15" class="content_input_fd_ro" size="12" name="" value="<% nvram_get_f("ddns.log","DDNSStatus"); %>"><input type="submit" maxlength="15" class="content_input_fd_ro" onClick="return onSubmitApply('eject-usb.sh')" size="12" name="PrinterStatus_x_FEject_button" value="빼냄"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('예를 선택하면 USB 디스크 FTP서버를 구축합니다.', LEFT);" onMouseOut="return nd();">FTP서버를 활성화하겠습니까?
           </td><td class="content_input_td"><input type="radio" value="1" name="usb_ftpenable_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_ftpenable_x', '1')" <% nvram_match_x("PrinterStatus","usb_ftpenable_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="usb_ftpenable_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_ftpenable_x', '0')" <% nvram_match_x("PrinterStatus","usb_ftpenable_x", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('예를 선택하면 비밀번호 없이 사용자의 FTP서버에 연결될 수 있습니다. 익명계정의 홈 디렉터리는 USB 디스크의 루트 디렉터리입니다.', LEFT);" onMouseOut="return nd();">익명 사용자의 로그온을 허락합니까?
           </td><td class="content_input_td"><input type="radio" value="1" name="usb_ftpanonymous_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_ftpanonymous_x', '1')" <% nvram_match_x("PrinterStatus","usb_ftpanonymous_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="usb_ftpanonymous_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_ftpanonymous_x', '0')" <% nvram_match_x("PrinterStatus","usb_ftpanonymous_x", "0", "checked"); %>>No</input><a href="javascript:openLink('x_FIsAnonymous')" class="content_input_link" name="x_FIsAnonymous_link">로그인
             </a></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('예를 선택하면 웹 매니저 로그인에 사용되는 동일한 계정의 FTP서버 연결을 허락하게 됩니다. 상위 사용자 계정의 홈 디렉터리는 USB 디스크의 루트 디렉터리입니다.', LEFT);" onMouseOut="return nd();">상위 사용자의 로그온을 허락합니까?
           </td><td class="content_input_td"><input type="radio" value="1" name="usb_ftpsuper_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_ftpsuper_x', '1')" <% nvram_match_x("PrinterStatus","usb_ftpsuper_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="usb_ftpsuper_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_ftpsuper_x', '0')" <% nvram_match_x("PrinterStatus","usb_ftpsuper_x", "0", "checked"); %>>No</input><a href="javascript:openLink('x_FIsSuperuser')" class="content_input_link" name="x_FIsSuperuser_link">로그인
             </a></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 FTP 프로토콜이 사용하고 있는 포트번호를 표시합니다.', LEFT);" onMouseOut="return nd();">FTP 포트:
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="usb_ftpport_x" class="content_input_fd" value="<% nvram_get_x("PrinterStatus", "usb_ftpport_x"); %>" onBlur="validate_range(this, 1, 65535)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 FTP서버에 동시 로그 인할 수 있는 최대의 사용자 수를 표시합니다.', LEFT);" onMouseOut="return nd();">로그 인 최대 사용자수:
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="usb_ftpmax_x" class="content_input_fd" value="<% nvram_get_x("PrinterStatus", "usb_ftpmax_x"); %>" onBlur="validate_range(this, 1, 12)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 사용자가 특정한 시간동안 로그인을 진행한 사용자의 연결을 종료할 수 있도록 합니다.', LEFT);" onMouseOut="return nd();">초단위 로그인 타임아웃:
           </td><td class="content_input_td"><input type="text" maxlength="5" class="content_input_fd" size="5" name="usb_ftptimeout_x" value="<% nvram_get_x("PrinterStatus","usb_ftptimeout_x"); %>" onBlur="validate_range(this, 0, 65535)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 사용자가 특정한 시간동안 대기상태로 로그인을 진행한 사용자의 연결을 종료할 수 있도록 합니다.', LEFT);" onMouseOut="return nd();">초단위 스테이 타임아웃 :
           </td><td class="content_input_td"><input type="text" maxlength="5" class="content_input_fd" size="5" name="usb_ftpstaytimeout_x" value="<% nvram_get_x("PrinterStatus","usb_ftpstaytimeout_x"); %>" onBlur="validate_range(this, 0, 65535)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('USB 디스크를 넣은 후, zvmodelvz이 USB 디스크의 루트 디렉터리에서 명명된 초기 스크립트를 찾아 이것을 실행할 것입니다. 이것을 빈칸으로 두면 이 기능을 사용하지 않습니다.', LEFT);" onMouseOut="return nd();">초기 스크립트:
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="usb_ftpscript_x" value="<% nvram_get_x("PrinterStatus","usb_ftpscript_x"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
</table>
</td>
</tr>
<tr>
<td>
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_list_header_tr">
<td class="content_list_header_td" width="60%" id="x_FUserList">사용자계정 목록
         <input type="hidden" name="usb_ftpnum_x_0" value="<% nvram_get_x("PrinterStatus", "usb_ftpnum_x"); %>" readonly="1"></td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'x_FUserList', 32, ' Add ');" name="x_FUserList" value="추가" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'x_FUserList', 32, ' Del ');" name="x_FUserList" value="삭제" size="12">
</div>
</td><td width="5%"></td>
</tr>
<table class="content_list_table" width="640" border="0" cellspacing="0" cellpadding="0">
<tr>
<td colspan="3" height="10"></td>
</tr>
<tr>
<td colspan="3">
<div align="center">
<table class="content_list_value_table" border="1" cellspacing="0" cellpadding="0">
<tr>
<td></td><td class="content_list_field_header_td" colspan="">사용자 이름	                
           	        </td><td class="content_list_field_header_td" colspan="">비밀번호	                
           	        </td><td class="content_list_field_header_td" colspan="">최대 로그인	                
           	        </td><td class="content_list_field_header_td" colspan="">권리	                
           	        </td><td></td>
</tr>
<tr>
<td></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="16" size="16" name="usb_ftpusername_x_0" class="content_input_list_fd" onKeyPress="return is_string(this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="16" size="16" name="usb_ftppasswd_x_0" class="content_input_list_fd" onKeyPress="return is_string(this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="5" style="font-family: fixedsys; font-size: 10pt;" size="5" name="usb_ftpmaxuser_x_0" onKeyPress="return is_number(this)"></td><td class="content_list_input_td" colspan=""><select name="usb_ftprights_x_0" class="content_input_list_fd"><option value="Read/Write/Erase" <% nvram_match_list_x("PrinterStatus","usb_ftprights_x", "Read/Write/Erase","selected", 0); %>>Read/Write/Erase</option><option value="Read/Write" <% nvram_match_list_x("PrinterStatus","usb_ftprights_x", "Read/Write","selected", 0); %>>Read/Write</option><option value="Read Only" <% nvram_match_list_x("PrinterStatus","usb_ftprights_x", "Read Only","selected", 0); %>>Read Only</option><option value="View Only" <% nvram_match_list_x("PrinterStatus","usb_ftprights_x", "View Only","selected", 0); %>>View Only</option><option value="Private" <% nvram_match_list_x("PrinterStatus","usb_ftprights_x", "Private","selected", 0); %>>Private</option><option value="Private(WO)" <% nvram_match_list_x("PrinterStatus","usb_ftprights_x", "Private(WO)","selected", 0); %>>Private(WO)</option></select></td>
</tr>
<tr>
<td></td><td colspan="10"><select size="8" name="x_FUserList_s" multiple="true" style="font-family: 'fixedsys'; font-size: '8pt'">
<% nvram_get_table_x("PrinterStatus","x_FUserList"); %>
</select></td>
</tr>
</table>
</div>
</td>
</tr>
<tr>
<td colspan="3" height="10"></td>
</tr>
</table>
</table>
</td>
</tr>
<tr>
<td>
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_list_header_tr">
<td class="content_list_header_td" width="60%" id="x_FBanIPList">금지 IP 목록
         <input type="hidden" name="usb_bannum_x_0" value="<% nvram_get_x("PrinterStatus", "usb_bannum_x"); %>" readonly="1"></td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'x_FBanIPList', 16, ' Add ');" name="x_FBanIPList" value="추가" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'x_FBanIPList', 16, ' Del ');" name="x_FBanIPList" value="삭제" size="12">
</div>
</td><td width="5%"></td>
</tr>
<table class="content_list_table" width="640" border="0" cellspacing="0" cellpadding="0">
<tr>
<td colspan="3" height="10"></td>
</tr>
<tr>
<td colspan="3">
<div align="center">
<table class="content_list_value_table" border="1" cellspacing="0" cellpadding="0">
<tr>
<td></td><td class="content_list_field_header_td" colspan="">IP 주소	                
           	        </td><td></td>
</tr>
<tr>
<td></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="15" class="content_input_list_fd" size="24" name="usb_ftpbanip_x_0" onKeyPress="return is_iprange(this)" onKeyUp="change_iprange(this)"></td>
</tr>
<tr>
<td></td><td colspan="10"><select size="8" name="x_FBanIPList_s" multiple="true" style="font-family: 'fixedsys'; font-size: '8pt'">
<% nvram_get_table_x("PrinterStatus","x_FBanIPList"); %>
</select></td>
</tr>
</table>
</div>
</td>
</tr>
<tr>
<td colspan="3" height="10"></td>
</tr>
</table>
</table>
</td>
</tr>
<tr>
<td>
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0"></table>
</td>
</tr>
<tr>
<td>
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0"></table>
</td>
</tr>
<tr>
<td>		
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="B0B0B0">
<tr bgcolor="#CCCCCC"><td colspan="3"><font face="arial" size="2"><b>&nbsp</b></font></td></tr>
<tr bgcolor="#FFFFFF">  
   <td id ="Confirm" height="25" width="34%">  
   <div align="center"><font face="Arial"> <input class=inputSubmit onMouseOut=buttonOut(this) onMouseOver="buttonOver(this)" onClick="onSubmitCtrl(this, ' Restore ')" type="submit" value=" 복구 " name="action"></font></div> 
   </td>  
   <td height="25" width="33%">  
   <div align="center"><font face="Arial"> <input class=inputSubmit onMouseOut=buttonOut(this) onMouseOver="buttonOver(this)" onClick="onSubmitCtrl(this, ' Finish ')" type="submit" value=" 마침 " name="action"></font></div> 
   </td>
   <td height="25" width="33%">  
   <div align="center"><font face="Arial"> <input class=inputSubmit onMouseOut=buttonOut(this) onMouseOver="buttonOver(this)" onClick="onSubmitCtrl(this, ' Apply ')" type="submit" value=" 적용 " name="action"></font></div> 
   </td>    
</tr>
</table>
</td>
</tr>

<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="B0B0B0">
<tr>
    <td colspan="2" width="616" height="25" bgcolor="#FFBB00"></td> 
</tr>                   
<tr bgcolor="#FFFFFF">
    <td class="content_header_td_15" align="left">복구: </td>
    <td class="content_input_td_padding" align="left">상기 설정을 취소하고 설정복구를 유효화합니다.</td>
</tr>
<tr bgcolor="#FFFFFF">
    <td class="content_header_td_15" align="left">마침: </td>
    <td class="content_input_td_padding" align="left">모든 설정을 확인하고 WL530G를 지금 다시 시작합니다.</td>
</tr>
<tr bgcolor="#FFFFFF">
    <td class="content_header_td_15" align="left">적용: </td>
    <td class="content_input_td_padding" align="left">상기설정을 확인하고 계속합니다.</td>
</tr>
</table>
</td>
</tr>

</table>
</form>
</body>
