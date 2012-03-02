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
    	
<input type="hidden" name="current_page" value="Advanced_DMZDHCP_Content.asp"><input type="hidden" name="next_page" value="Advanced_DMZDWFilter_Content.asp"><input type="hidden" name="next_host" value=""><input type="hidden" name="sid_list" value="LANHostConfig;"><input type="hidden" name="group_id" value=""><input type="hidden" name="modified" value="0"><input type="hidden" name="action_mode" value=""><input type="hidden" name="first_time" value=""><input type="hidden" name="action_script" value="">
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">ワイヤレスファイアウォール - DHCPサーバー</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">ZVMODELVZは、ワイヤレスファイアウォール機能が有効になっていると、ワイヤレスネットワークに対して最大254のIPアドレスをサポートします。ローカルマシンのIPアドレスはネットワーク管理者により手動で割り当てたり、DHCPサーバーが有効になっていると、ZVMODELVZから自動的に取得できます。
         </td>
</tr>
<tr>
<td class="content_header_td">DHCPサーバーを有効にしますか?
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', '', '1')" <% nvram_match_x("LANHostConfig","", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', '', '0')" <% nvram_match_x("LANHostConfig","", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このフィールドはドメイン名を示して、DHCPサーバーからIPアドレスを要求するワイヤレスクライアントに提供します。', LEFT);" onMouseOut="return nd();">ドメイン名:
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("LANHostConfig",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このフィールドは、ワイヤレスネットワークのDHCPサーバーにより割り当てられるプールに、最初のアドレスを指定します。', LEFT);" onMouseOut="return nd();">IPプール開始アドレス:
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="" value="<% nvram_get_x("LANHostConfig",""); %>" onBlur="return validate_ipaddr(this, '')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このフィールドは、ワイヤレスネットワークのDHCPサーバーにより割り当てられるプールに、最後のアドレスを指定します。', LEFT);" onMouseOut="return nd();">IPプール終了アドレス:
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="" value="<% nvram_get_x("LANHostConfig",""); %>" onBlur="return validate_ipaddr(this, '')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このフィールドは、ワイヤレスネットワークユーザーがその現在のダイナミックIPアドレスで許可される接続時間の量を指定します。', LEFT);" onMouseOut="return nd();">リース期間:
           </td><td class="content_input_td"><input type="text" maxlength="10" class="content_input_fd" size="10" name="" value="<% nvram_get_x("LANHostConfig",""); %>" onBlur="validate_range(this, 0, 4294927695)" onKeyPress="return is_number(this)"></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">DNSとWINSサーバー設定
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このフィールドはIPアドレスドメイン名システムを示し、DHCPサーバーからIPアドレスを要求するワイヤレスクライアントに提供します。これをブランクにしておくと、DNS要求はZVMODELVZにより処理されます。', LEFT);" onMouseOut="return nd();">DNSサーバー1:
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="" value="<% nvram_get_x("LANHostConfig",""); %>" onBlur="return validate_ipaddr(this, '')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td">DNSサーバー2:
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd_ro" size="15" name="" value="<% nvram_get_f("FirewallConfig","DmzIP"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('Windowsインターネット命名サービス(WINS)は各PCとインターネットの相互作用を管理します。WINSサーバーを使用する場合、ここにサーバーのIPアドレスを入力してください。', LEFT);" onMouseOut="return nd();">WINSサーバー:
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="" value="<% nvram_get_x("LANHostConfig",""); %>" onBlur="return validate_ipaddr(this, '')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">Assign IP Address Manually
            </td>
</tr>
<tr>
<td class="content_header_td">Enable Manual Assignment?
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', '', '1')" <% nvram_match_x("LANHostConfig","", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', '', '0')" <% nvram_match_x("LANHostConfig","", "0", "checked"); %>>No</input></td>
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
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_list_header_tr">
<td class="content_list_header_td" width="60%" id="DmzManualDHCPList">Manually Assigned IP List
         <input type="hidden" name="DmzManualDHCPCount_0" value="<% nvram_get_x("LANHostConfig", "DmzManualDHCPCount"); %>" readonly="1"></td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'DmzManualDHCPList', 8, ' Add ');" name="DmzManualDHCPList" value="追加" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'DmzManualDHCPList', 8, ' Del ');" name="DmzManualDHCPList" value="削除" size="12">
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
<td></td><td class="content_list_field_header_td" colspan="">MAC Address	                
           	        </td><td class="content_list_field_header_td" colspan="">IP Address	                
           	        </td><td></td>
</tr>
<tr>
<td></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="12" class="content_input_list_fd" size="12" name="_0" onKeyPress="return is_hwaddr()"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="15" class="content_input_list_fd" size="14" name="_0" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr>
<td></td><td colspan="10"><select size="4" name="DmzManualDHCPList_s" multiple="true" style="font-family: 'fixedsys'; font-size: '8pt'">
<% nvram_get_table_x("LANHostConfig","DmzManualDHCPList"); %>
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
