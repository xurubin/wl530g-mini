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
    	
<input type="hidden" name="current_page" value="Advanced_DynamicRoute_Content.asp"><input type="hidden" name="next_page" value="SaveRestart.asp"><input type="hidden" name="next_host" value=""><input type="hidden" name="sid_list" value="RouterConfig;"><input type="hidden" name="group_id" value=""><input type="hidden" name="modified" value="0"><input type="hidden" name="action_mode" value=""><input type="hidden" name="first_time" value=""><input type="hidden" name="action_script" value="">
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">라우터 - 유동 라우터</td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('RouterConfig_GWStaticRIP_itemdesc', LEFT);" onMouseOut="return nd();">유동 라우터(RIP)를 활성화하겠습니까?
           </td><td class="content_input_td"><select name="sr_rip_x" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', 'sr_rip_x')"><option class="content_input_fd" value="0" <% nvram_match_x("RouterConfig","sr_rip_x", "0","selected"); %>>Disabled</option><option class="content_input_fd" value="1" <% nvram_match_x("RouterConfig","sr_rip_x", "1","selected"); %>>LAN</option><option class="content_input_fd" value="2" <% nvram_match_x("RouterConfig","sr_rip_x", "2","selected"); %>>WAN</option><option class="content_input_fd" value="3" <% nvram_match_x("RouterConfig","sr_rip_x", "3","selected"); %>>BOTH</option></select></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">WAN
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('RIP 패킷을 보내고 받는 것이 WAN 포트에서 활성화 또는 비활성화 될 수 있습니다.', LEFT);" onMouseOut="return nd();">WAN의 RIP를 활성화하겠습니까?
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'RouterConfig', '', '1')" <% nvram_match_x("RouterConfig","", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'RouterConfig', '', '0')" <% nvram_match_x("RouterConfig","", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 선택한 인터페이스가 RIP 버전 1이나 RIP 버전 2 또는 두 개의 버전 모두로 패킷을 보내고 받을 수 있도록 합니다. 두 버전 모두의 경우에는 패킷은 브로드캐스팅과 멀티캐스팅 모두 됩니다.', LEFT);" onMouseOut="return nd();">RIP 버전:
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="RIP1" <% nvram_match_x("RouterConfig","", "RIP1","selected"); %>>RIP1</option><option class="content_input_fd" value="RIP2" <% nvram_match_x("RouterConfig","", "RIP2","selected"); %>>RIP2</option><option class="content_input_fd" value="Both" <% nvram_match_x("RouterConfig","", "Both","selected"); %>>Both</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('포트에서 분할수평 라우팅의 적용여부를 제어합니다.', LEFT);" onMouseOut="return nd();">분할수평을 활성화하겠습니까?
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'RouterConfig', '', '1')" <% nvram_match_x("RouterConfig","", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'RouterConfig', '', '0')" <% nvram_match_x("RouterConfig","", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('포트에서 적용할 인증방식과 해당키를 선택합니다.', LEFT);" onMouseOut="return nd();">인증방식:
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="Disable" <% nvram_match_x("RouterConfig","", "Disable","selected"); %>>Disable</option><option class="content_input_fd" value="Text" <% nvram_match_x("RouterConfig","", "Text","selected"); %>>Text</option><option class="content_input_fd" value="MD5" <% nvram_match_x("RouterConfig","", "MD5","selected"); %>>MD5</option></select></td>
</tr>
<tr>
<td class="content_header_td">인증키:
           </td><td class="content_input_td"><input type="text" maxlength="16" class="content_input_fd" size="32" name="" value="<% nvram_get_x("RouterConfig",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">LAN
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('RIP 패킷을 보내고 받는 것이 LAN', LEFT);" onMouseOut="return nd();">LAN의 RIP를 활성화하겠습니까?
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'RouterConfig', '', '1')" <% nvram_match_x("RouterConfig","", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'RouterConfig', '', '0')" <% nvram_match_x("RouterConfig","", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('본 필드항목은 선택한 인터페이스가 RIP 버전 1이나 RIP 버전 2 또는 두 개의 버전 모두로 패킷을 보내고 받을 수 있도록 합니다. 두 버전 모두의 경우에는 패킷은 브로드캐스팅과 멀티캐스팅 모두 됩니다.', LEFT);" onMouseOut="return nd();">RIP 버전:
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="RIP1" <% nvram_match_x("RouterConfig","", "RIP1","selected"); %>>RIP1</option><option class="content_input_fd" value="RIP2" <% nvram_match_x("RouterConfig","", "RIP2","selected"); %>>RIP2</option><option class="content_input_fd" value="Both" <% nvram_match_x("RouterConfig","", "Both","selected"); %>>Both</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('포트에서 분할수평 라우팅의 적용여부를 제어합니다.', LEFT);" onMouseOut="return nd();">분할수평을 활성화하겠습니까?
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'RouterConfig', '', '1')" <% nvram_match_x("RouterConfig","", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'RouterConfig', '', '0')" <% nvram_match_x("RouterConfig","", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('포트에서 적용할 인증방식과 해당키를 선택합니다.', LEFT);" onMouseOut="return nd();">인증방식:
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="Disable" <% nvram_match_x("RouterConfig","", "Disable","selected"); %>>Disable</option><option class="content_input_fd" value="Text" <% nvram_match_x("RouterConfig","", "Text","selected"); %>>Text</option><option class="content_input_fd" value="MD5" <% nvram_match_x("RouterConfig","", "MD5","selected"); %>>MD5</option></select></td>
</tr>
<tr>
<td class="content_header_td">인증키:
           </td><td class="content_input_td"><input type="text" maxlength="16" class="content_input_fd" size="32" name="" value="<% nvram_get_x("RouterConfig",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">타이머
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('모든 업데이트 타이머는 초단위이며 RIP 프로세스가 수행되어 이웃한 RIP 라우터로 완전히 능동적인 라우팅의 응답을 보냅니다.', LEFT);" onMouseOut="return nd();">업데이트 시간:
           </td><td class="content_input_td"><input type="text" maxlength="5" class="content_input_fd" size="5" name="" value="<% nvram_get_x("RouterConfig",""); %>" onBlur="validate_range(this, 0, 65535)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('타임아웃 종료 시 라우터는 더 이상 유효하지 않습니다. 그러나 잠시 동안 라우팅 테이블에서 라우터가 유지되어 이웃한 컴퓨터들은 라우터가 드롭 되지 않았음을 통보받게 됩니다.', LEFT);" onMouseOut="return nd();">타임아웃 시간:
           </td><td class="content_input_td"><input type="text" maxlength="5" class="content_input_fd" size="5" name="" value="<% nvram_get_x("RouterConfig",""); %>" onBlur="validate_range(this, 0, 65535)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('쓰레기 수집 타이머가 종료되면 라우터는 라우팅 테이블에서 최종적으로 제거됩니다.', LEFT);" onMouseOut="return nd();">쓰레기 수집시간:
           </td><td class="content_input_td"><input type="text" maxlength="5" class="content_input_fd" size="5" name="" value="<% nvram_get_x("RouterConfig",""); %>" onBlur="validate_range(this, 0, 65535)" onKeyPress="return is_number(this)"></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">라우터 재분배 규칙
            </td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">본 규칙은 RIP 경로를 필터링하기 위하여 사용됩니다. WAN 및 LAN 포트의 발신 또는 수신 데이터를 표시하는 4가지의 기본 라우터 필터 유형을 정의하여 라우터 분배 필터 테이블에서 정한 네트워크만이 처리되거나 처리되지 않도록 설정할 수 있습니다.
         </td>
</tr>
<tr>
<td class="content_header_td">0 유형 라우터는WAN으로 부터 받으며 아래에서 정한 것만 해당함:
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="1" <% nvram_match_x("RouterConfig","", "1","selected"); %>>processed</option><option class="content_input_fd" value="0" <% nvram_match_x("RouterConfig","", "0","selected"); %>>dropped</option></select></td>
</tr>
<tr>
<td class="content_header_td">1 유형 라우터는WAN으로 보내지며 아래에서 정한 것만 해당함:
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="1" <% nvram_match_x("RouterConfig","", "1","selected"); %>>processed</option><option class="content_input_fd" value="0" <% nvram_match_x("RouterConfig","", "0","selected"); %>>dropped</option></select></td>
</tr>
<tr>
<td class="content_header_td">2유형 라우터는LAN으로 부터 받으며 아래에서 정한 것만 해당함:
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="1" <% nvram_match_x("RouterConfig","", "1","selected"); %>>processed</option><option class="content_input_fd" value="0" <% nvram_match_x("RouterConfig","", "0","selected"); %>>dropped</option></select></td>
</tr>
<tr>
<td class="content_header_td">3유형 라우터는LAN으로 보내지며 아래에서 정한 것만 해당함:
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="1" <% nvram_match_x("RouterConfig","", "1","selected"); %>>processed</option><option class="content_input_fd" value="0" <% nvram_match_x("RouterConfig","", "0","selected"); %>>dropped</option></select></td>
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
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_list_header_tr">
<td class="content_list_header_td" width="60%" id="RouteFilter">라우터 분배 필터
         <input type="hidden" name="RipDRouteCount_0" value="<% nvram_get_x("RouterConfig", "RipDRouteCount"); %>" readonly="1"></td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'RouteFilter', 16, ' Add ');" name="RouteFilter" value="추가" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'RouteFilter', 16, ' Del ');" name="RouteFilter" value="삭제" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="button" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return openHelp(this, 'RouterHelp');" name="RouteFilter" value="도움말" size="12">
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
<td></td><td class="content_list_field_header_td" colspan="">라우터 유형	                
           	        </td><td class="content_list_field_header_td" colspan="">네트워크/호스트IP	                
           	        </td><td class="content_list_field_header_td" colspan="">넷마스크 비트	                
           	        </td><td></td>
</tr>
<tr>
<td></td><td class="content_list_input_td" colspan=""><select name="_0" class="content_input_list_fd"><option value="0" <% nvram_match_list_x("RouterConfig","", "0","selected", 0); %>>0</option><option value="1" <% nvram_match_list_x("RouterConfig","", "1","selected", 0); %>>1</option><option value="2" <% nvram_match_list_x("RouterConfig","", "2","selected", 0); %>>2</option><option value="3" <% nvram_match_list_x("RouterConfig","", "3","selected", 0); %>>3</option></select></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="15" class="content_input_list_fd" size="15" name="_0" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="5" style="font-family: fixedsys; font-size: 10pt;" size="8" name="_0" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td></td><td colspan="10"><select size="8" name="RouteFilter_s" multiple="true" style="font-family: 'fixedsys'; font-size: '8pt'">
<% nvram_get_table_x("RouterConfig","RouteFilter"); %>
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
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">라우터 메트릭 규칙
            </td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">RIP메트릭은 네트워크 거리입니다. 네트워크 정보를 받을 때에 보통 RIP는 메트릭이 증가됩니다. 재분배된 라우터의 기본설정 메트릭 오프셋은 1로 설정되어 있습니다. 이러한 규칙들은 라우터 메트릭 오프셋 테이블에서 정하였거나 배제된 것만의 메트릭 오프셋을 변경하기 위하여 사용될 수 있습니다. 그러나 다른 네트워크의 메트릭 오프셋은 여전히 1로 설정되어 있습니다.
         </td>
</tr>
<tr>
<td class="content_header_td">라우터 메트릭 오프셋:
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="" class="content_input_fd" value="<% nvram_get_x("RouterConfig", ""); %>" onBlur="validate_range(this, 1, 16)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td">수신 라우터에 대한 메트릭 오프셋 추가   RouterConfig_RipOffsetOut_itemname
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="0" <% nvram_match_x("RouterConfig","", "0","selected"); %>>none</option><option class="content_input_fd" value="1" <% nvram_match_x("RouterConfig","", "1","selected"); %>>routes specified below</option><option class="content_input_fd" value="2" <% nvram_match_x("RouterConfig","", "2","selected"); %>>routes not specified below</option></select></td>
</tr>
<tr>
<td class="content_header_td">RouterConfig_RipOffsetOut_itemname
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="0" <% nvram_match_x("RouterConfig","", "0","selected"); %>>none</option><option class="content_input_fd" value="1" <% nvram_match_x("RouterConfig","", "1","selected"); %>>routes specified below</option><option class="content_input_fd" value="2" <% nvram_match_x("RouterConfig","", "2","selected"); %>>routes not specified below</option></select></td>
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
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_list_header_tr">
<td class="content_list_header_td" width="60%" id="RipOffset">라우터 메트릭 오프셋
         <input type="hidden" name="RipOffsetCount_0" value="<% nvram_get_x("RouterConfig", "RipOffsetCount"); %>" readonly="1"></td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'RipOffset', 16, ' Add ');" name="RipOffset" value="추가" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'RipOffset', 16, ' Del ');" name="RipOffset" value="삭제" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="button" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return openHelp(this, 'RouterHelp');" name="RipOffset" value="도움말" size="12">
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
<td></td><td class="content_list_field_header_td" colspan="">방향	                
           	        </td><td class="content_list_field_header_td" colspan="">네트워크/호스트IP	                
           	        </td><td class="content_list_field_header_td" colspan="">넷마스크 비트	                
           	        </td><td></td>
</tr>
<tr>
<td></td><td class="content_list_input_td" colspan=""><select name="_0" class="content_input_list_fd"><option value="IN" <% nvram_match_list_x("RouterConfig","", "IN","selected", 0); %>>IN</option><option value="OUT" <% nvram_match_list_x("RouterConfig","", "OUT","selected", 0); %>>OUT</option></select></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="15" class="content_input_list_fd" size="14" name="_0" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="5" style="font-family: fixedsys; font-size: 10pt;" size="8" name="_0" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td></td><td colspan="10"><select size="4" name="RipOffset_s" multiple="true" style="font-family: 'fixedsys'; font-size: '8pt'">
<% nvram_get_table_x("RouterConfig","RipOffset"); %>
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
