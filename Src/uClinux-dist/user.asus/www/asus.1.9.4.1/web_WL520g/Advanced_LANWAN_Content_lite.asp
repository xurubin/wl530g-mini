<head>
<title>WL520g Web Manager</title>
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
    	
<input type="hidden" name="current_page" value="Advanced_LANWAN_Content.asp"><input type="hidden" name="next_page" value="Advanced_DHCP_Content.asp"><input type="hidden" name="next_host" value=""><input type="hidden" name="sid_list" value="Layer3Forwarding;LANHostConfig;IPConnection;PPPConnection;"><input type="hidden" name="group_id" value=""><input type="hidden" name="modified" value="0"><input type="hidden" name="action_mode" value=""><input type="hidden" name="first_time" value=""><input type="hidden" name="action_script" value="">
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">IP 컨피규레이션 - WAN & LAN</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">WL520g는 여러 방식의 WAN 연결유형을 지원합니다. 이러한 유형은 WAN 연결유형 옆에 있는 드롭다운 메뉴에서 선택할 수 있습니다. 설정 필드항목은 선택한 연결유형에 따라 다릅니다.
         </td>
</tr>
<tr>
<td class="content_header_td">WAN 연결유형:
           </td><td class="content_input_td"><select name="wan_proto" class="content_input_fd" onChange="return change_common(this, 'Layer3Forwarding', 'wan_proto')"><option class="content_input_fd" value="dhcp" <% nvram_match_x("Layer3Forwarding","wan_proto", "dhcp","selected"); %>>Automatic IP</option><option class="content_input_fd" value="static" <% nvram_match_x("Layer3Forwarding","wan_proto", "static","selected"); %>>Static IP</option><option class="content_input_fd" value="pppoe" <% nvram_match_x("Layer3Forwarding","wan_proto", "pppoe","selected"); %>>PPPoE</option><option class="content_input_fd" value="pptp" <% nvram_match_x("Layer3Forwarding","wan_proto", "pptp","selected"); %>>PPTP</option><option class="content_input_fd" value="bigpond" <% nvram_match_x("Layer3Forwarding","wan_proto", "bigpond","selected"); %>>BigPond</option></select></td>
</tr>
</table>
</td>
</tr>
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">WAN IP 설정
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('이것은 원격 네트워크에서 나타나는 바와 같이 WL520g의 IP주소입니다. 공란으로 두면 WL520g가 DHCP 서버로 부터 IP주소를 자동으로 가져옵니다.', LEFT);" onMouseOut="return nd();">IP 주소:
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="wan_ipaddr" value="<% nvram_get_x("IPConnection","wan_ipaddr"); %>" onBlur="return validate_ipaddr(this, 'wan_ipaddr')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('원격 네트워크에서 나타나는 것과 같은 WL520g의 서브넷 마스크입니다', LEFT);" onMouseOut="return nd();">서브넷 마스크:
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="wan_netmask" value="<% nvram_get_x("IPConnection","wan_netmask"); %>" onBlur="return validate_ipaddr(this, 'wan_netmask')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('WL520g와 원격 네트워크 또는 호스트와 연결할 수 있도록 하는 기본설정 게이트웨이의 IP주소입니다.', LEFT);" onMouseOut="return nd();">기본설정 게이트웨이:
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="wan_gateway" value="<% nvram_get_x("IPConnection","wan_gateway"); %>" onBlur="return validate_ipaddr(this, 'wan_gateway')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">WAN DNS 설정
            </td>
</tr>
<tr>
<td class="content_header_td"> DNS 서버를 자동 부여합니까?
           </td><td class="content_input_td"><input type="radio" value="1" name="wan_dnsenable_x" class="content_input_fd" onClick="return change_common_radio(this, 'IPConnection', 'wan_dnsenable_x', '1')" <% nvram_match_x("IPConnection","wan_dnsenable_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="wan_dnsenable_x" class="content_input_fd" onClick="return change_common_radio(this, 'IPConnection', 'wan_dnsenable_x', '0')" <% nvram_match_x("IPConnection","wan_dnsenable_x", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('이 필드항목은 WL520g와 연결되는 DNS의 IP주소를 나타냅니다.', LEFT);" onMouseOut="return nd();">DNS 서버1:
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="wan_dns1_x" value="<% nvram_get_x("IPConnection","wan_dns1_x"); %>" onBlur="return validate_ipaddr(this, 'wan_dns1_x')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('이 필드항목은 WL520g와 연결되는 DNS의 IP주소를 나타냅니다.', LEFT);" onMouseOut="return nd();">DNS 서버2:
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="wan_dns2_x" value="<% nvram_get_x("IPConnection","wan_dns2_x"); %>" onBlur="return validate_ipaddr(this, 'wan_dns2_x')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
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
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">PPPoE 및 PPTP 계정
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 WAN 연결유형을 PPPoE로 정한 경우에만 이용 가능합니다.', LEFT);" onMouseOut="return nd();">사용자 이름:
           </td><td class="content_input_td"><input type="text" maxlength="64" class="content_input_fd" size="32" name="wan_pppoe_username" value="<% nvram_get_x("PPPConnection","wan_pppoe_username"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 WAN 연결유형을 PPPoE로 정한 경우에만 이용 가능합니다.', LEFT);" onMouseOut="return nd();">비밀번호:
           </td><td class="content_input_td"><input type="password" maxlength="64" class="content_input_fd" size="32" name="wan_pppoe_passwd" value="<% nvram_get_x("PPPConnection","wan_pppoe_passwd"); %>" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 일정한 시간이 경과한 후 ISP 연결을 종료할 수 있도록 컨피규레이션 할 수 있게 합니다. Tx만이 체크되어 있으면 인터넷으로부터의 데이터가 대기시간을 계산하도록 건너뛰기 합니다.', LEFT);" onMouseOut="return nd();">초단위 대기 연결정지시간(옵션):
           </td><td class="content_input_td"><input type="text" maxlength="10" class="content_input_fd" size="10" name="wan_pppoe_idletime" value="<% nvram_get_x("PPPConnection","wan_pppoe_idletime"); %>" onBlur="validate_range(this, 0, 4294967295)" onKeyPress="return is_number(this)"><input type="checkbox" style="margin-left:30" name="wan_pppoe_idletime_check" value="" onClick="return change_common_radio(this, 'PPPConnection', 'wan_pppoe_idletime', '1')">Tx Only</input></td>
</tr>
<input type="hidden" name="wan_pppoe_txonly_x" value="<% nvram_get_x("PPPConnection","wan_pppoe_txonly_x"); %>">
<tr>
<td class="content_header_td" onMouseOver="return overlib('PPPoE 패킷의 최대전송단위(MTU;Maximum Transmission Unit)입니다.', LEFT);" onMouseOut="return nd();">MTU:
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="wan_pppoe_mtu" class="content_input_fd" value="<% nvram_get_x("PPPConnection", "wan_pppoe_mtu"); %>" onBlur="validate_range(this, 576, 1492)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('packet.PPPoE 패킷의 최대수신단위(MRU;Maximum Receive Unit)입니다.', LEFT);" onMouseOut="return nd();">MRU:
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="wan_pppoe_mru" class="content_input_fd" value="<% nvram_get_x("PPPConnection", "wan_pppoe_mru"); %>" onBlur="validate_range(this, 576, 1492)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 항목은 ISP가 정할 수 있습니다. 필요시 ISP에 연락하여 작성합니다.', LEFT);" onMouseOut="return nd();">서비스 이름(옵션):
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="wan_pppoe_service" value="<% nvram_get_x("PPPConnection","wan_pppoe_service"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 항목은 ISP가 정할 수 있습니다. 필요시 ISP에 연락하여 작성합니다.', LEFT);" onMouseOut="return nd();">액세스 집중장치 이름(옵션):
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="wan_pppoe_ac" value="<% nvram_get_x("PPPConnection","wan_pppoe_ac"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">ISP 특별요구사항
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목에서는 WL520g의 호스트 이름을 부여할 수 있습니다. ISP가 보통 요구합니다.', LEFT);" onMouseOut="return nd();">호스트 이름:
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="wan_hostname" value="<% nvram_get_x("PPPConnection","wan_hostname"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목에서는 인터넷 연결을 위한 WL520g의 MAC 주소를 부여할 수 있습니다. ISP가 보통 요구합니다.', LEFT);" onMouseOut="return nd();">MAC 주소:
           </td><td class="content_input_td"><input type="text" maxlength="12" class="content_input_fd" size="12" name="wan_hwaddr_x" value="<% nvram_get_x("PPPConnection","wan_hwaddr_x"); %>" onBlur="return validate_hwaddr(this)" onKeyPress="return is_hwaddr()"></td>
</tr>
<tr>
<td class="content_header_td">Heart-Beat Server:
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="wan_heartbeat_x" value="<% nvram_get_x("PPPConnection","wan_heartbeat_x"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
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
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">LAN IP 설정
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('로컬 네트워크에서 보여지는 것과 같은 WL520g의 IP주소입니다. 초기설정값은 192.168.1.1입니다.', LEFT);" onMouseOut="return nd();">IP 주소:
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="lan_ipaddr" value="<% nvram_get_x("LANHostConfig","lan_ipaddr"); %>" onBlur="return validate_ipaddr(this, 'lan_ipaddr')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('로컬 네트워크에서 보여지는 것과 같은 WL520g의 서브넷 마스크입니다. 초기설정값은 255.255.255.0입니다.', LEFT);" onMouseOut="return nd();">서브넷 마스크:
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="lan_netmask" value="<% nvram_get_x("LANHostConfig","lan_netmask"); %>" onBlur="return validate_ipaddr(this, 'lan_netmask')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<input type="hidden" name="dhcp_start" value="<% nvram_get_x("LANHostConfig","dhcp_start"); %>"><input type="hidden" name="dhcp_end" value="<% nvram_get_x("LANHostConfig","dhcp_end"); %>">
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
    <td class="content_input_td_padding" align="left">모든 설정을 확인하고 WL520g를 지금 다시 시작합니다.</td>
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
