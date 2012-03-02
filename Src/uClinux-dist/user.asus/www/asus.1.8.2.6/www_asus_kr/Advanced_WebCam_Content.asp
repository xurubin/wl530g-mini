<head>
<title>ZVMODELVZ Web Manager</title>
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
    	
<input type="hidden" name="current_page" value="Advanced_WebCam_Content.asp"><input type="hidden" name="next_page" value="SaveRestart.asp"><input type="hidden" name="next_host" value=""><input type="hidden" name="sid_list" value="PrinterStatus;"><input type="hidden" name="group_id" value=""><input type="hidden" name="modified" value="0"><input type="hidden" name="action_mode" value=""><input type="hidden" name="first_time" value=""><input type="hidden" name="action_script" value="">
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">USB 어플리케이션 - 웹 카메라</td>
</tr>
<tr>
<td class="content_header_td">웹 카메라를 활성화하겠습니까?
           </td><td class="content_input_td"><select name="usb_webenable_x" class="content_input_fd" onChange="return change_common(this, 'PrinterStatus', 'usb_webenable_x')"><option class="content_input_fd" value="0" <% nvram_match_x("PrinterStatus","usb_webenable_x", "0","selected"); %>>Disabled</option><option class="content_input_fd" value="1" <% nvram_match_x("PrinterStatus","usb_webenable_x", "1","selected"); %>>LAN Only</option><option class="content_input_fd" value="2" <% nvram_match_x("PrinterStatus","usb_webenable_x", "2","selected"); %>>LAN and WAN</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 사용하고 있는 웹 카메라의 모드를 표시합니다. ActiveX 만을 선택하면 IE 플랫트홈에서 ActiveX 클라이언트를 실행하여 최고의 품질의 이미지를 볼 수 있습니다. ActiveX와 새로고침을 선택하면 IE 플랫트홈에서 ActiveX 클라이언트를 실행하고 다른 브라우저에서 기본적인 이미지를 볼 수 있습니다. 새로고침만을 선택하면 모든 브라우저에서 기본적인 이미지만을 볼 수 있습니다.', LEFT);" onMouseOut="return nd();">웹 카메라 모드:
           </td><td class="content_input_td"><select name="usb_webmode_x" class="content_input_fd" onChange="return change_common(this, 'PrinterStatus', 'usb_webmode_x')"><option class="content_input_fd" value="0" <% nvram_match_x("PrinterStatus","usb_webmode_x", "0","selected"); %>>ActiveX Only</option><option class="content_input_fd" value="1" <% nvram_match_x("PrinterStatus","usb_webmode_x", "1","selected"); %>>ActiveX and Refresh</option><option class="content_input_fd" value="2" <% nvram_match_x("PrinterStatus","usb_webmode_x", "2","selected"); %>>Refresh Only</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 웹 카메라 드라이버를 표시합니다.', LEFT);" onMouseOut="return nd();">웹 카메라 드라이버:
           </td><td class="content_input_td"><select name="usb_webdriver_x" class="content_input_fd" onChange="return change_common(this, 'PrinterStatus', 'usb_webdriver_x')"><option class="content_input_fd" value="0" <% nvram_match_x("PrinterStatus","usb_webdriver_x", "0","selected"); %>>PWC 8.8</option><option class="content_input_fd" value="1" <% nvram_match_x("PrinterStatus","usb_webdriver_x", "1","selected"); %>>OV511 2.10</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 이미지 크기를 표시합니다.', LEFT);" onMouseOut="return nd();">이미지 크기:
           </td><td class="content_input_td"><select name="usb_webimage_x" class="content_input_fd" onChange="return change_common(this, 'PrinterStatus', 'usb_webimage_x')"><option class="content_input_fd" value="0" <% nvram_match_x("PrinterStatus","usb_webimage_x", "0","selected"); %>>640 X 480</option><option class="content_input_fd" value="1" <% nvram_match_x("PrinterStatus","usb_webimage_x", "1","selected"); %>>320 X 240</option><option class="content_input_fd" value="2" <% nvram_match_x("PrinterStatus","usb_webimage_x", "2","selected"); %>>160 X 120</option></select><a href="javascript:openLink('x_WImageSize')" class="content_input_link" name="x_WImageSize_link">미리보기
             </a></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 이미지 움직임을 탐지할 센스 레벨을 표시합니다.', LEFT);" onMouseOut="return nd();">센스 레벨:
           </td><td class="content_input_td"><select name="usb_websense_x" class="content_input_fd" onChange="return change_common(this, 'PrinterStatus', 'usb_websense_x')"><option class="content_input_fd" value="0" <% nvram_match_x("PrinterStatus","usb_websense_x", "0","selected"); %>>Low</option><option class="content_input_fd" value="1" <% nvram_match_x("PrinterStatus","usb_websense_x", "1","selected"); %>>Medium</option><option class="content_input_fd" value="2" <% nvram_match_x("PrinterStatus","usb_websense_x", "2","selected"); %>>High</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 시스템이 이미지를 로딩 하는 초단위 시간간격을 표시합니다. 값의 범위는 1~65535입니다.', LEFT);" onMouseOut="return nd();">초단위시간 새로고침:
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="usb_webfresh_x" class="content_input_fd" value="<% nvram_get_x("PrinterStatus", "usb_webfresh_x"); %>" onBlur="validate_range(this, 1, 65535)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 웹캠 페이지에 나타날 문자열을 표시합니다.', LEFT);" onMouseOut="return nd();">캡션 문자열:
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="usb_webcaption_x" value="<% nvram_get_x("PrinterStatus","usb_webcaption_x"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 서버가 HTTP 서버가 통신하기 위하여 듣고 있는 포트를 나타냅니다.', LEFT);" onMouseOut="return nd();">HTTP 포트:
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="usb_webhttpport_x" class="content_input_fd" value="<% nvram_get_x("PrinterStatus", "usb_webhttpport_x"); %>" onBlur="validate_range(this, 1024, 65535)" onKeyPress="return is_number(this)"><input type="checkbox" style="margin-left:30" name="usb_webhttpport_x_check" value="" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_webhttpport_x', '1')">Password Checking</input></td>
</tr>
<input type="hidden" name="usb_webhttpcheck_x" value="<% nvram_get_x("PrinterStatus","usb_webhttpcheck_x"); %>">
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 서버가 ActiveX 클라이언트와 통신하기 위하여 듣고 있는 포트를 나타냅니다.', LEFT);" onMouseOut="return nd();">ActiveX 포트:
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="usb_webactivex_x" class="content_input_fd" value="<% nvram_get_x("PrinterStatus", "usb_webactivex_x"); %>" onBlur="validate_range(this, 1024, 65535)" onKeyPress="return is_number(this)"></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">보안모드 설정
            </td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">본 기능은 웹 카메라를 통하여 환경을 모니터링할 수 있도록 합니다. 움직임이 탐지되면 ZVMODELVZ가 이메일로 알려 줍니다.
         </td>
</tr>
<tr>
<td class="content_header_td">보안모드를 활성화하겠습니까?
           </td><td class="content_input_td"><input type="radio" value="1" name="usb_websecurity_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_websecurity_x', '1')" <% nvram_match_x("PrinterStatus","usb_websecurity_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="usb_websecurity_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_websecurity_x', '0')" <% nvram_match_x("PrinterStatus","usb_websecurity_x", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 보안모드가 활성화 되는 날짜를 표시합니다.', LEFT);" onMouseOut="return nd();">보안모드 활성화 날짜:
           </td><td class="content_input_td"><input type="hidden" maxlength="7" class="content_input_fd" size="7" name="usb_websecurity_date_x" value="<% nvram_get_x("PrinterStatus","usb_websecurity_date_x"); %>">
<p style="word-spacing: 0; margin-top: 0; margin-bottom: 0">
<input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Sun">Sun</input><input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Mon">Mon</input><input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Tue">Tue</input><input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Wed">Wed</input>
</p>
<input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Thu">Thu</input><input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Fri">Fri</input><input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Sat">Sat</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 보안모드가 활성화 되는 시간간격을 표시합니다.', LEFT);" onMouseOut="return nd();">보안모드 활성화 시간:
           </td><td class="content_input_td"><input type="hidden" maxlength="11" class="content_input_fd" size="11" name="usb_websecurity_time_x" value="<% nvram_get_x("PrinterStatus","usb_websecurity_time_x"); %>"><input type="text" maxlength="2" class="content_input_fd" size="2" name="usb_websecurity_time_x_starthour" onKeyPress="return is_number(this)" onBlur="return validate_timerange(this, 0)">:
                <input type="text" maxlength="2" class="content_input_fd" size="2" name="usb_websecurity_time_x_startmin" onKeyPress="return is_number(this)" onBlur="return validate_timerange(this, 1)">-
                <input type="text" maxlength="2" class="content_input_fd" size="2" name="usb_websecurity_time_x_endhour" onKeyPress="return is_number(this)" onBlur="return validate_timerange(this, 2)">:
                <input type="text" maxlength="2" class="content_input_fd" size="2" name="usb_websecurity_time_x_endmin" onKeyPress="return is_number(this)" onBlur="return validate_timerange(this, 3)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 수신인 이메일 주소를 표시합니다.', LEFT);" onMouseOut="return nd();">수신인:
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="usb_websendto_x" value="<% nvram_get_x("PrinterStatus","usb_websendto_x"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 이메일이 수신되는 이메일 서버를 표시합니다. 공란으로 두면 ZVMODELVZ는 아래의 이메일 주소에서 메일교환기를 찾게 됩니다.', LEFT);" onMouseOut="return nd();">이메일 서버:
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="usb_webmserver_x" value="<% nvram_get_x("PrinterStatus","usb_webmserver_x"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목에서 이메일 제목을 편집합니다.', LEFT);" onMouseOut="return nd();">제목:
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="usb_websubject_x" value="<% nvram_get_x("PrinterStatus","usb_websubject_x"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목에서 찾은 이미지파일을 이메일로 첨부합니다.', LEFT);" onMouseOut="return nd();">이미지 파일을 첨부합니까?
           </td><td class="content_input_td"><input type="radio" value="1" name="usb_webattach_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_webattach_x', '1')" <% nvram_match_x("PrinterStatus","usb_webattach_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="usb_webattach_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_webattach_x', '0')" <% nvram_match_x("PrinterStatus","usb_webattach_x", "0", "checked"); %>>No</input></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">원격 모니터 설정
            </td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">본 기능으로 LAN 환경에서 최대 6대 웹 카메라의 모니터링이 가능합니다. 웹 카메라와 연결되고 있는 무선 라우터의 IP주소를 입력하십시오. 인터넷에 웹 카메라를 노출하고자 하면 모든 HTTP 포트의 가상서버와 ActiveX 포트를 수동설정 하십시오.
         </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('LAN만을 선택하면 LAN 환경에서만 모니터링할 수 있습니다. 인터넷에서 모니터링 하고자 하면 가상서버 목록에 있는 HTTP 포트와 ActiveX를 수동설정 하십시오. 보안문제를 고려하십시오.', LEFT);" onMouseOut="return nd();">원격 컨트롤 모드:
           </td><td class="content_input_td"><select name="usb_webremote_x" class="content_input_fd" onChange="return change_common(this, 'PrinterStatus', 'usb_webremote_x')"><option class="content_input_fd" value="0" <% nvram_match_x("PrinterStatus","usb_webremote_x", "0","selected"); %>>LAN Only</option></select><a href="javascript:openLink('x_WRemote')" class="content_input_link" name="x_WRemote_link">미리보기
             </a></td>
</tr>
<tr>
<td class="content_header_td">원격 사이트 1:
           </td><td class="content_input_td"><input type="text" maxlength="21" class="content_input_fd" size="15" name="usb_webremote1_x" value="<% nvram_get_x("PrinterStatus","usb_webremote1_x"); %>" onBlur="return validate_ipaddrport(this, 'usb_webremote1_x')" onKeyPress="return is_ipaddrport(this)" onKeyUp="change_ipaddrport(this)"></td>
</tr>
<tr>
<td class="content_header_td">원격 사이트 2:
           </td><td class="content_input_td"><input type="text" maxlength="21" class="content_input_fd" size="15" name="usb_webremote2_x" value="<% nvram_get_x("PrinterStatus","usb_webremote2_x"); %>" onBlur="return validate_ipaddrport(this, 'usb_webremote2_x')" onKeyPress="return is_ipaddrport(this)" onKeyUp="change_ipaddrport(this)"></td>
</tr>
<tr>
<td class="content_header_td">원격 사이트 3:
           </td><td class="content_input_td"><input type="text" maxlength="21" class="content_input_fd" size="15" name="usb_webremote3_x" value="<% nvram_get_x("PrinterStatus","usb_webremote3_x"); %>" onBlur="return validate_ipaddrport(this, 'usb_webremote3_x')" onKeyPress="return is_ipaddrport(this)" onKeyUp="change_ipaddrport(this)"></td>
</tr>
<tr>
<td class="content_header_td">원격 사이트 4:
           </td><td class="content_input_td"><input type="text" maxlength="21" class="content_input_fd" size="15" name="usb_webremote4_x" value="<% nvram_get_x("PrinterStatus","usb_webremote4_x"); %>" onBlur="return validate_ipaddrport(this, 'usb_webremote4_x')" onKeyPress="return is_ipaddrport(this)" onKeyUp="change_ipaddrport(this)"></td>
</tr>
<tr>
<td class="content_header_td">원격 사이트 5:
           </td><td class="content_input_td"><input type="text" maxlength="21" class="content_input_fd" size="15" name="usb_webremote5_x" value="<% nvram_get_x("PrinterStatus","usb_webremote5_x"); %>" onBlur="return validate_ipaddrport(this, 'usb_webremote5_x')" onKeyPress="return is_ipaddrport(this)" onKeyUp="change_ipaddrport(this)"></td>
</tr>
<tr>
<td class="content_header_td">원격 사이트 6:
           </td><td class="content_input_td"><input type="text" maxlength="21" class="content_input_fd" size="15" name="usb_webremote6_x" value="<% nvram_get_x("PrinterStatus","usb_webremote6_x"); %>" onBlur="return validate_ipaddrport(this, 'usb_webremote6_x')" onKeyPress="return is_ipaddrport(this)" onKeyUp="change_ipaddrport(this)"></td>
</tr>
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
    <td class="content_input_td_padding" align="left">모든 설정을 확인하고 ZVMODELVZ를 지금 다시 시작합니다.</td>
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
