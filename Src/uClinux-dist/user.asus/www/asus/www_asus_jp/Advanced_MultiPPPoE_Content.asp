<head>
<title>ZVMODELVZ Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=Shift_JIS">
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
<td class="content_header_td_title" colspan="2">IP較正 - PPPoE セッション</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">この関数により、接続する場所に従って複数のPPPoEセッションを適用できます。
         </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('既定の接続に対して1つのセッションを選択してください。セッション 0 はWAN & LAN設定で設定されたPPPoEセッションを意味します。', LEFT);" onMouseOut="return nd();">デフォルトPPPoEセッション:
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'PPPConnection', '')"><option class="content_input_fd" value="0" <% nvram_match_x("PPPConnection","", "0","selected"); %>>PPPoE Session 0</option><option class="content_input_fd" value="1" <% nvram_match_x("PPPConnection","", "1","selected"); %>>PPPoE Session 1</option><option class="content_input_fd" value="2" <% nvram_match_x("PPPConnection","", "2","selected"); %>>PPPoE Session 2</option></select></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">PPPoEセッション1
            </td>
</tr>
<tr>
<td class="content_header_td">PPPoEセッション1を有効する?
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'PPPConnection', '', '1')" <% nvram_match_x("PPPConnection","", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'PPPConnection', '', '0')" <% nvram_match_x("PPPConnection","", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('WAN接続タイプをPPPoEとして設定しているとき、このフィールドのみ使用できます。', LEFT);" onMouseOut="return nd();">ユーザーネイム:
           </td><td class="content_input_td"><input type="text" maxlength="64" class="content_input_fd" size="32" name="" value="<% nvram_get_x("PPPConnection",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('WAN接続タイプをPPPoEとして設定しているとき、このフィールドのみ使用できます', LEFT);" onMouseOut="return nd();">パスワード:
           </td><td class="content_input_td"><input type="password" maxlength="64" class="content_input_fd" size="32" name="" value="<% nvram_get_x("PPPConnection",""); %>" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このフィールドにより、指定された時間の後、ISP接続を終了するように構成できます。ゼロの値は活動停止期間を無限にします。Tx のみがチェックされている場合、インターネットのデータはスキップされ活動停止時間をカウントします。', LEFT);" onMouseOut="return nd();">活動停止中の切断時間（秒）(オプション):
           </td><td class="content_input_td"><input type="text" maxlength="10" class="content_input_fd" size="10" name="" value="<% nvram_get_x("PPPConnection",""); %>" onBlur="validate_range(this, 0, 4294967295)" onKeyPress="return is_number(this)"><input type="checkbox" style="margin-left:30" name="_check" value="" onClick="return change_common_radio(this, 'PPPConnection', '', '1')">Tx Only</input></td>
</tr>
<input type="hidden" name="" value="<% nvram_get_x("PPPConnection",""); %>">
<tr>
<td class="content_header_td" onMouseOver="return overlib('PPPoEパケットの最大送信単位(MTU)です。', LEFT);" onMouseOut="return nd();">MTU:
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="" class="content_input_fd" value="<% nvram_get_x("PPPConnection", ""); %>" onBlur="validate_range(this, 576, 1492)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('PPPoEパケットの最大受信単位(MRU)です。', LEFT);" onMouseOut="return nd();">MRU:
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="" class="content_input_fd" value="<% nvram_get_x("PPPConnection", ""); %>" onBlur="validate_range(this, 576, 1492)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このアイテムは、一部のISPにより指定されています。ISPをチェックし、必要に応じて必要事項を記入してください。', LEFT);" onMouseOut="return nd();">サービスネイム(オプション):
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("PPPConnection",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このアイテムは、一部のISPにより指定されています。ISPをチェックし、必要に応じて必要事項を記入してください。', LEFT);" onMouseOut="return nd();">アクセスコンセントレーターネイム(オプション):
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("PPPConnection",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">PPPoE セッション 2
            </td>
</tr>
<tr>
<td class="content_header_td">Enable PPPoE セッション 2?
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'PPPConnection', '', '1')" <% nvram_match_x("PPPConnection","", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'PPPConnection', '', '0')" <% nvram_match_x("PPPConnection","", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('WAN接続タイプをPPPoEとして設定しているとき、このフィールドのみを使用できます', LEFT);" onMouseOut="return nd();">ユーザーネイム:
           </td><td class="content_input_td"><input type="text" maxlength="64" class="content_input_fd" size="32" name="" value="<% nvram_get_x("PPPConnection",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('WAN接続タイプをPPPoEとして設定しているとき、このフィールドのみを使用できます', LEFT);" onMouseOut="return nd();">パスワード:
           </td><td class="content_input_td"><input type="password" maxlength="64" class="content_input_fd" size="32" name="" value="<% nvram_get_x("PPPConnection",""); %>" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このフィールドにより、指定された時間の後、ISP接続を終了するように構成できます。ゼロの値は活動停止期間を無限にします。Tx のみがチェックされている場合、インターネットのデータはスキップされ活動停止時間をカウントします。', LEFT);" onMouseOut="return nd();">活動停止中の切断時間（秒）(オプション):
           </td><td class="content_input_td"><input type="text" maxlength="10" class="content_input_fd" size="10" name="" value="<% nvram_get_x("PPPConnection",""); %>" onBlur="validate_range(this, 0, 4294967295)" onKeyPress="return is_number(this)"><input type="checkbox" style="margin-left:30" name="_check" value="" onClick="return change_common_radio(this, 'PPPConnection', '', '1')">Tx Only</input></td>
</tr>
<input type="hidden" name="" value="<% nvram_get_x("PPPConnection",""); %>">
<tr>
<td class="content_header_td" onMouseOver="return overlib('PPPoEパケットの最大送信単位(MTU)です。', LEFT);" onMouseOut="return nd();">MTU:
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="" class="content_input_fd" value="<% nvram_get_x("PPPConnection", ""); %>" onBlur="validate_range(this, 576, 1492)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('PPPoEパケットの最大受信単位(MRU)です。', LEFT);" onMouseOut="return nd();">MRU:
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="" class="content_input_fd" value="<% nvram_get_x("PPPConnection", ""); %>" onBlur="validate_range(this, 576, 1492)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このアイテムは、一部のISPにより指定されています。ISPをチェックし、必要に応じて必要事項を記入してください。', LEFT);" onMouseOut="return nd();">サービスネイム(オプション):
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("PPPConnection",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このアイテムは、一部のISPにより指定されています。ISPをチェックし、必要に応じて必要事項を記入してください。', LEFT);" onMouseOut="return nd();">アクセスコンセントレーターネイム(オプション):
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("PPPConnection",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
</table>
</td>
</tr>
<tr>
<td>
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_list_header_tr">
<td class="content_list_header_td" width="60%" id="PPPoERouteList">PPPoEルートリスト
         <input type="hidden" name="PPPoERouteCount_0" value="<% nvram_get_x("PPPConnection", "PPPoERouteCount"); %>" readonly="1"></td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'PPPoERouteList', 4, ' Add ');" name="PPPoERouteList" value="追加" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'PPPoERouteList', 4, ' Del ');" name="PPPoERouteList" value="削除" size="12">
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
<td></td><td class="content_list_field_header_td" colspan="">PPPConnection_x_PPPoESess_itemname	                
           	        </td><td class="content_list_field_header_td" colspan="">PPPConnection_x_PPPoEDir_itemname	                
           	        </td><td class="content_list_field_header_td" colspan="">PPPConnection_x_PPPoEIP_itemname	                
           	        </td><td class="content_list_field_header_td" colspan="">PPPConnection_x_PPPoEMask_itemname	                
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
   <div align="center"><font face="Arial"> <input class=inputSubmit onMouseOut=buttonOut(this) onMouseOver="buttonOver(this)" onClick="onSubmitCtrl(this, ' Restore ')" type="submit" value=" 復元 " name="action"></font></div> 
   </td>  
   <td height="25" width="33%">  
   <div align="center"><font face="Arial"> <input class=inputSubmit onMouseOut=buttonOut(this) onMouseOver="buttonOver(this)" onClick="onSubmitCtrl(this, ' Finish ')" type="submit" value=" 終了 " name="action"></font></div> 
   </td>
   <td height="25" width="33%">  
   <div align="center"><font face="Arial"> <input class=inputSubmit onMouseOut=buttonOut(this) onMouseOver="buttonOver(this)" onClick="onSubmitCtrl(this, ' Apply ')" type="submit" value=" 適用 " name="action"></font></div> 
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
    <td class="content_header_td_15" align="left">復元: </td>
    <td class="content_input_td_padding" align="left">上の設定を消去し、有効になっている設定を復元します。</td>
</tr>
<tr bgcolor="#FFFFFF">
    <td class="content_header_td_15" align="left">終了: </td>
    <td class="content_input_td_padding" align="left">すべての設定を確認して、今ZVMODELVZを再起動します。</td>
</tr>
<tr bgcolor="#FFFFFF">
    <td class="content_header_td_15" align="left">適用: </td>
    <td class="content_input_td_padding" align="left">上の設定を確認して、続行します。</td>
</tr>
</table>
</td>
</tr>

</table>
</form>
</body>
