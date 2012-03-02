<head>
<title>ZVMODELVZ Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=big5">
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
    	
<input type="hidden" name="current_page" value="Advanced_MultiPPPoE_Content.asp"><input type="hidden" name="next_page" value="Advanced_DHCP_Content.asp"><input type="hidden" name="next_host" value=""><input type="hidden" name="sid_list" value="Layer3Forwarding;LANHostConfig;IPConnection;PPPConnection;"><input type="hidden" name="group_id" value=""><input type="hidden" name="modified" value="0"><input type="hidden" name="action_mode" value=""><input type="hidden" name="first_time" value=""><input type="hidden" name="action_script" value="">
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">IP組態 - PPPoE區段</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">本項功能讓您可根據所要連線的地點採用一個以上的PPPoE區段。
         </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('請選擇一個區段（Session）作為您預設的連線。「區段0」是指在WAN和LAN設定值中所設定的PPPoE區段。', LEFT);" onMouseOut="return nd();">預設的PPPoE區段：
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'PPPConnection', '')"><option class="content_input_fd" value="0" <% nvram_match_x("PPPConnection","", "0","selected"); %>>PPPoE Session 0</option><option class="content_input_fd" value="1" <% nvram_match_x("PPPConnection","", "1","selected"); %>>PPPoE Session 1</option><option class="content_input_fd" value="2" <% nvram_match_x("PPPConnection","", "2","selected"); %>>PPPoE Session 2</option></select></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">PPPoE區段1
            </td>
</tr>
<tr>
<td class="content_header_td">啟用PPPoE區段1？
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'PPPConnection', '', '1')" <% nvram_match_x("PPPConnection","", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'PPPConnection', '', '0')" <% nvram_match_x("PPPConnection","", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位僅在您將「WAN連線類型」設為PPPoE時才能使用', LEFT);" onMouseOut="return nd();">用戶名稱：
           </td><td class="content_input_td"><input type="text" maxlength="64" class="content_input_fd" size="32" name="" value="<% nvram_get_x("PPPConnection",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位僅在您將「WAN連線類型」設為PPPoE時才能使用', LEFT);" onMouseOut="return nd();">密碼：
           </td><td class="content_input_td"><input type="password" maxlength="64" class="content_input_fd" size="32" name="" value="<% nvram_get_x("PPPConnection",""); %>" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位讓您可將組態設定為在一段特定的時間後即終止您的ISP連線。數值0為允許無限期的閒置時間。如果勾選「僅針對Tx」，則接收自網際網路的資料將會跳過以便計算閒置時間。', LEFT);" onMouseOut="return nd();">以秒計算的「閒置斷線時間」（選項）：
           </td><td class="content_input_td"><input type="text" maxlength="10" class="content_input_fd" size="10" name="" value="<% nvram_get_x("PPPConnection",""); %>" onBlur="validate_range(this, 0, 4294967295)" onKeyPress="return is_number(this)"><input type="checkbox" style="margin-left:30" name="_check" value="" onClick="return change_common_radio(this, 'PPPConnection', '', '1')">Tx Only</input></td>
</tr>
<input type="hidden" name="" value="<% nvram_get_x("PPPConnection",""); %>">
<tr>
<td class="content_header_td" onMouseOver="return overlib('亦即是指PPPoE封包的「最大傳輸單位」（MTU）。', LEFT);" onMouseOut="return nd();">MTU：
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="" class="content_input_fd" value="<% nvram_get_x("PPPConnection", ""); %>" onBlur="validate_range(this, 576, 1492)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('亦即是指PPPoE封包的「最大接收單位」（MRU）。', LEFT);" onMouseOut="return nd();">MRU：
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="" class="content_input_fd" value="<% nvram_get_x("PPPConnection", ""); %>" onBlur="validate_range(this, 576, 1492)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本項或許某些ISP業者能夠指定。請向您的ISP業者確認並於需要時將它們填入。', LEFT);" onMouseOut="return nd();">網路服務名稱（選項）：
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("PPPConnection",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本項或許某些ISP業者能夠指定。請向您的ISP業者確認並於需要時將它們填入。', LEFT);" onMouseOut="return nd();">存取集訊器名稱（選項）：
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("PPPConnection",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">PPPoE區段2
            </td>
</tr>
<tr>
<td class="content_header_td">啟用PPPoE區段2？
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'PPPConnection', '', '1')" <% nvram_match_x("PPPConnection","", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'PPPConnection', '', '0')" <% nvram_match_x("PPPConnection","", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位僅在您將「WAN連線類型」設為PPPoE時才能使用。', LEFT);" onMouseOut="return nd();">用戶名稱：
           </td><td class="content_input_td"><input type="text" maxlength="64" class="content_input_fd" size="32" name="" value="<% nvram_get_x("PPPConnection",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位僅在您將「WAN連線類型」設為PPPoE時才能使用。', LEFT);" onMouseOut="return nd();">密碼：
           </td><td class="content_input_td"><input type="password" maxlength="64" class="content_input_fd" size="32" name="" value="<% nvram_get_x("PPPConnection",""); %>" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位讓您可將組態設定為在一段特定的時間後即終止您的ISP連線。數值0為允許無限期的閒置時間。如果勾選「僅針對Tx」，則接收自網際網路的資料將會跳過以便計算閒置時間。', LEFT);" onMouseOut="return nd();">以秒計算的「閒置斷線時間」（選項）：
           </td><td class="content_input_td"><input type="text" maxlength="10" class="content_input_fd" size="10" name="" value="<% nvram_get_x("PPPConnection",""); %>" onBlur="validate_range(this, 0, 4294967295)" onKeyPress="return is_number(this)"><input type="checkbox" style="margin-left:30" name="_check" value="" onClick="return change_common_radio(this, 'PPPConnection', '', '1')">Tx Only</input></td>
</tr>
<input type="hidden" name="" value="<% nvram_get_x("PPPConnection",""); %>">
<tr>
<td class="content_header_td" onMouseOver="return overlib('亦即是指PPPoE封包的「最大傳輸單位」（MTU）。', LEFT);" onMouseOut="return nd();">MTU：
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="" class="content_input_fd" value="<% nvram_get_x("PPPConnection", ""); %>" onBlur="validate_range(this, 576, 1492)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('亦即是指PPPoE封包的「最大接收單位」（MRU）。', LEFT);" onMouseOut="return nd();">MRU：
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="" class="content_input_fd" value="<% nvram_get_x("PPPConnection", ""); %>" onBlur="validate_range(this, 576, 1492)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本項或許某些ISP業者能夠指定。請向您的ISP業者確認並於需要時將它們填入。', LEFT);" onMouseOut="return nd();">網路服務名稱（選項）：
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("PPPConnection",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本項或許某些ISP業者能夠指定。請向您的ISP業者確認並於需要時將它們填入。', LEFT);" onMouseOut="return nd();">存取集訊器名稱（選項）：
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("PPPConnection",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
</table>
</td>
</tr>
<tr>
<td>
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_list_header_tr">
<td class="content_list_header_td" width="60%" id="PPPoERouteList">PPPoE路由清單
         <input type="hidden" name="PPPoERouteCount_0" value="<% nvram_get_x("PPPConnection", "PPPoERouteCount"); %>" readonly="1"></td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'PPPoERouteList', 4, ' Add ');" name="PPPoERouteList" value="新增" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'PPPoERouteList', 4, ' Del ');" name="PPPoERouteList" value="刪除" size="12">
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
<td></td><td class="content_list_field_header_td" colspan="">區段	                
           	        </td><td class="content_list_field_header_td" colspan="">IP 類型	                
           	        </td><td class="content_list_field_header_td" colspan="">網路/主機IP	                
           	        </td><td class="content_list_field_header_td" colspan="">網路遮罩位元	                
           	        </td><td></td>
</tr>
<tr>
<td></td><td class="content_list_input_td" colspan=""><select name="_0" class="content_input_list_fd"><option value="0" <% nvram_match_list_x("PPPConnection","", "0","selected", 0); %>>0</option><option value="1" <% nvram_match_list_x("PPPConnection","", "1","selected", 0); %>>1</option><option value="2" <% nvram_match_list_x("PPPConnection","", "2","selected", 0); %>>2</option></select></td><td class="content_list_input_td" colspan=""><select name="_0" class="content_input_list_fd"><option value="Source" <% nvram_match_list_x("PPPConnection","", "Source","selected", 0); %>>Source</option><option value="Destination" <% nvram_match_list_x("PPPConnection","", "Destination","selected", 0); %>>Destination</option></select></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="15" class="content_input_list_fd" size="14" name="_0" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="5" style="font-family: fixedsys; font-size: 10pt;" size="8" name="_0" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td></td><td colspan="10"><select size="4" name="PPPoERouteList_s" multiple="true" style="font-family: 'fixedsys'; font-size: '8pt'">
<% nvram_get_table_x("PPPConnection","PPPoERouteList"); %>
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
   <div align="center"><font face="Arial"> <input class=inputSubmit onMouseOut=buttonOut(this) onMouseOver="buttonOver(this)" onClick="onSubmitCtrl(this, ' Restore ')" type="submit" value=" 還原 " name="action"></font></div> 
   </td>  
   <td height="25" width="33%">  
   <div align="center"><font face="Arial"> <input class=inputSubmit onMouseOut=buttonOut(this) onMouseOver="buttonOver(this)" onClick="onSubmitCtrl(this, ' Finish ')" type="submit" value=" 儲存 " name="action"></font></div> 
   </td>
   <td height="25" width="33%">  
   <div align="center"><font face="Arial"> <input class=inputSubmit onMouseOut=buttonOut(this) onMouseOver="buttonOver(this)" onClick="onSubmitCtrl(this, ' Apply ')" type="submit" value=" 套用 " name="action"></font></div> 
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
    <td class="content_header_td_15" align="left">還原: </td>
    <td class="content_input_td_padding" align="left">清除上述設定值並還原有效設定值。</td>
</tr>
<tr bgcolor="#FFFFFF">
    <td class="content_header_td_15" align="left">儲存: </td>
    <td class="content_input_td_padding" align="left">現在確認所有設定值並重新啟動ZVMODELVZ。</td>
</tr>
<tr bgcolor="#FFFFFF">
    <td class="content_header_td_15" align="left">套用: </td>
    <td class="content_input_td_padding" align="left">確認上述設定值並繼續。</td>
</tr>
</table>
</td>
</tr>

</table>
</form>
</body>
