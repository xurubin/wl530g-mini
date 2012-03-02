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
    	
<input type="hidden" name="current_page" value="Main_RStatus_Content.asp"><input type="hidden" name="next_page" value="default value"><input type="hidden" name="next_host" value=""><input type="hidden" name="sid_list" value="default value"><input type="hidden" name="group_id" value=""><input type="hidden" name="modified" value="0"><input type="hidden" name="action_mode" value=""><input type="hidden" name="first_time" value=""><input type="hidden" name="action_script" value=""><input type="hidden" name="load_script" value="<% load_script("wan.sh"); %>">
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">상태 & 로그 - 상태</td>
</tr>
<tr>
<td class="content_header_td_less" onMouseOver="return overlib('시스템 부팅후 경과시간', LEFT);" onMouseOut="return nd();">시스템 시간:
           </td><td class="content_input_td"><input type="text" size="48" class="content_input_fd" value="<% uptime(); %>" readonly="1"></td>
</tr>
</table>
</td>
</tr>
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">WAN 인터페이스</td>
</tr>
<tr>
<td class="content_header_td_less">WAN 유형:
           </td><td class="content_input_td"><input type="text" size="36" class="content_input_fd" name="wan_proto_t" value="<% nvram_get_f("wan.log","wan_proto_t"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td_less">IP 주소:
           </td><td class="content_input_td"><input type="text" size="36" class="content_input_fd" name="wan_ipaddr_t" value="<% nvram_get_f("wan.log","wan_ipaddr_t"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td_less">서브넷 마스크:
           </td><td class="content_input_td"><input type="text" size="36" class="content_input_fd" name="wan_netmask_t" value="<% nvram_get_f("wan.log","wan_netmask_t"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td_less">게이트웨이:
           </td><td class="content_input_td"><input type="text" size="36" class="content_input_fd" name="wan_gateway_t" value="<% nvram_get_f("wan.log","wan_gateway_t"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td_less">DNS 서버:
           </td><td class="content_input_td"><input type="text" size="36" class="content_input_fd" name="wan_dns_t" value="<% nvram_get_f("wan.log","wan_dns_t"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td_less">링크 상태:
           </td><td class="content_input_td"><input type="text" size="36" class="content_input_fd" name="wan_status_t" value="<% nvram_get_f("wan.log","wan_status_t"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td_less">액션:
           </td><td class="content_input_td"><input type="submit" maxlength="15" class="content_input_fd_ro" onClick="return onSubmitApply('dhcpc_release')" size="12" name="PPPConnection_x_WANAction_button" value="릴리스"><input type="submit" maxlength="15" class="content_input_fd_ro" onClick="return onSubmitApply('dhcpc_renew')" size="12" name="PPPConnection_x_WANAction_button1" value="새로 함"></td>
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
<td class="content_section_header_td" colspan="2">프린터</td>
</tr>
<tr>
<td class="content_header_td_less" onMouseOver="return overlib('ZVMODELVZ와의 프린터 모델 연결', LEFT);" onMouseOut="return nd();">프린터 모델:
           </td><td class="content_input_td"><input type="text" size="36" class="content_input_fd" name="" value="<% nvram_get_f("printer_status.log","printer_model_t"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td_less" onMouseOver="return overlib('프린터의 현재상태', LEFT);" onMouseOut="return nd();">프린터 상태:
           </td><td class="content_input_td"><input type="text" size="36" class="content_input_fd" name="" value="<% nvram_get_f("printer_status.log","printer_status_t"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td_less" onMouseOver="return overlib('본 프린터를 이용하는 사용자의 IP 주소', LEFT);" onMouseOut="return nd();">사용자:
           </td><td class="content_input_td"><input type="text" size="36" class="content_input_fd" name="" value="<% nvram_get_f("printer_status.log","printer_user_t"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td_less">액션:
           </td><td class="content_input_td"><input type="submit" maxlength="15" class="content_input_fd_ro" onClick="return onSubmitApply('lpr_remove')" size="12" name="PrinterStatus_x_PrinterAction_button" value="제거"></td>
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
<td class="content_section_header_td" colspan="2">LAN 인터페이스</td>
</tr>
<tr>
<td class="content_header_td_less">IP 주소:
           </td><td class="content_input_td"><input type="text" size="36" class="content_input_fd" name="lan_ipaddr_t" value="<% nvram_get_f("lan.log","lan_ipaddr_t"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td_less">서브넷 마스크:
           </td><td class="content_input_td"><input type="text" size="36" class="content_input_fd" name="lan_netmask_t" value="<% nvram_get_f("lan.log","lan_netmask_t"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td_less">기본설정 게이트웨이:
           </td><td class="content_input_td"><input type="text" size="36" class="content_input_fd" name="lan_gateway_t" value="<% nvram_get_f("lan.log","lan_gateway_t"); %>" readonly="1"></td>
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
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="B0B0B0">
<tr bgcolor="#CCCCCC"><td colspan="3"><font face="arial" size="2"><b>&nbsp</b></font></td></tr>
<tr bgcolor="#FFFFFF">  
   <td height="25" width="34%">  
   </td>
   <td height="25" width="33%">  
   </td>
   <td height="25" width="33%">  
   <div align="center"><font face="Arial"> <input class=inputSubmit onMouseOut=buttonOut(this) onMouseOver="buttonOver(this)" onClick="onSubmitCtrl(this, ' Refresh ')" type="submit" value="새로고침" name="action"></font></div> 
   </td>
</tr>
</table>
</td>
</tr>

</table>
</form>
</body>
