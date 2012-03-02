<head>
<title>ZVMODELVZ Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
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
    	
<input type="hidden" name="current_page" value="Advanced_Wireless_Content.asp"><input type="hidden" name="next_page" value="Advanced_UniqueKey_Content.asp"><input type="hidden" name="next_host" value=""><input type="hidden" name="sid_list" value="WLANConfig11a;WLANConfig11b;"><input type="hidden" name="group_id" value=""><input type="hidden" name="modified" value="0"><input type="hidden" name="action_mode" value=""><input type="hidden" name="first_time" value=""><input type="hidden" name="action_script" value="">
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">无线 - 界面</td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目可让ZVMODELVZ遵循您所在国家的无线通讯规格。', LEFT);" onMouseOut="return nd();">国家代码：
           </td><td class="content_input_td"><select name="" class="content_input_fd">   
<% select_country("WLANConfig11a"); %>
                </select></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">802.11a界面
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('为您的无线局域网（WLAN）指定一组可为32个字符的识别字串。', LEFT);" onMouseOut="return nd();">SSID：
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("WLANConfig11a",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('请选择操作时的无线频道。如果已勾选户外通道（outdoor channel），那么您便可选择一些特殊的频道。', LEFT);" onMouseOut="return nd();">频道：
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11a', '')">   
<% select_channel("WLANConfig11a"); %>
                </select><input type="hidden" maxlength="15" size="15" name="x_RegulatoryDomain" value="<% nvram_get_x("Regulatory","x_RegulatoryDomain"); %>" readonly="1"><input type="checkbox" style="margin-left:30" name="_check" value="" onClick="return change_common_radio(this, 'WLANConfig11a', '', '1')">Outdoor Channel</input></td>
</tr>
<input type="hidden" name="" value="<% nvram_get_x("WLANConfig11a",""); %>">
<tr>
<td class="content_header_td" onMouseOver="return overlib('请选择传输时的传输速率。建议您选择「Auto」使效能达到最大。若是勾选「加速模式」（turbo mode），则可达至108Mpbs的传输速率。', LEFT);" onMouseOut="return nd();">资料传输速率（Mbps）：
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11a', '')"><option class="content_input_fd" value="0" <% nvram_match_x("WLANConfig11a","", "0","selected"); %>>Best</option><option class="content_input_fd" value="1" <% nvram_match_x("WLANConfig11a","", "1","selected"); %>>6</option><option class="content_input_fd" value="2" <% nvram_match_x("WLANConfig11a","", "2","selected"); %>>9</option><option class="content_input_fd" value="3" <% nvram_match_x("WLANConfig11a","", "3","selected"); %>>12</option><option class="content_input_fd" value="4" <% nvram_match_x("WLANConfig11a","", "4","selected"); %>>18</option><option class="content_input_fd" value="5" <% nvram_match_x("WLANConfig11a","", "5","selected"); %>>24</option><option class="content_input_fd" value="6" <% nvram_match_x("WLANConfig11a","", "6","selected"); %>>36</option><option class="content_input_fd" value="7" <% nvram_match_x("WLANConfig11a","", "7","selected"); %>>48</option><option class="content_input_fd" value="8" <% nvram_match_x("WLANConfig11a","", "8","selected"); %>>54</option></select><input type="checkbox" style="margin-left:30" name="_check" value="" onClick="return change_common_radio(this, 'WLANConfig11a', '', '1')">Turbo Mode</input></td>
</tr>
<input type="hidden" name="" value="<% nvram_get_x("WLANConfig11a",""); %>">
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目可让您设置在认证阶段是否需要加密的状态。', LEFT);" onMouseOut="return nd();">认证方式：
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11a', '')"><option class="content_input_fd" value="Open System" <% nvram_match_x("WLANConfig11a","", "Open System","selected"); %>>Open System</option><option class="content_input_fd" value="Shared Key" <% nvram_match_x("WLANConfig11a","", "Shared Key","selected"); %>>Shared Key</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('启用WEP为资料加密，以避免在窃听者（eavesdroppers）的面前公开', LEFT);" onMouseOut="return nd();">加密（WEP）：
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_wlweptype(this, 'WLANConfig11a')"><option class="content_input_fd" value="None" <% nvram_match_x("WLANConfig11a","", "None","selected"); %>>None</option><option class="content_input_fd" value="64bits" <% nvram_match_x("WLANConfig11a","", "64bits","selected"); %>>64bits</option><option class="content_input_fd" value="128bits" <% nvram_match_x("WLANConfig11a","", "128bits","selected"); %>>128bits</option><option class="content_input_fd" value="152bits" <% nvram_match_x("WLANConfig11a","", "152bits","selected"); %>>152bits</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('提供「关键字」（magic word）以自动产生WEP密钥或保持栏目空白并以手动方式输入密钥。', LEFT);" onMouseOut="return nd();">密语：
           </td><td class="content_input_td"><script language="JavaScript" type="text/javascript" src="md5.js"></script><input type="password" maxlength="64" class="content_input_fd" size="32" name="" value="<% nvram_get_x("WLANConfig11a",""); %>" onKeyUp="return is_wlphrase('WLANConfig11a', '', this)" onBlur="return validate_wlphrase('WLANConfig11a', '', this)"></td>
</tr>
<tr>
<td class="content_header_td">WEP密钥1（10或26或32十六进制数字）：
           </td><td class="content_input_td"><input type="pssword" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("WLANConfig11a",""); %>" onBlur="return validate_wlkey(this, 'WLANConfig11a')" onKeyPress="return is_wlkey(this, 'WLANConfig11a')" onKeyUp="return change_wlkey(this, 'WLANConfig11a')"></td>
</tr>
<tr>
<td class="content_header_td">WEP密钥2（10或26或32十六进制数字）：
           </td><td class="content_input_td"><input type="pssword" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("WLANConfig11a",""); %>" onBlur="return validate_wlkey(this, 'WLANConfig11a')" onKeyPress="return is_wlkey(this, 'WLANConfig11a')" onKeyUp="return change_wlkey(this, 'WLANConfig11a')"></td>
</tr>
<tr>
<td class="content_header_td">WEP密钥3（10或26或32十六进制数字）：
           </td><td class="content_input_td"><input type="pssword" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("WLANConfig11a",""); %>" onBlur="return validate_wlkey(this, 'WLANConfig11a')" onKeyPress="return is_wlkey(this, 'WLANConfig11a')" onKeyUp="return change_wlkey(this, 'WLANConfig11a')"></td>
</tr>
<tr>
<td class="content_header_td">WEP密钥4（10或26或32十六进制数字）：
           </td><td class="content_input_td"><input type="pssword" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("WLANConfig11a",""); %>" onBlur="return validate_wlkey(this, 'WLANConfig11a')" onKeyPress="return is_wlkey(this, 'WLANConfig11a')" onKeyUp="return change_wlkey(this, 'WLANConfig11a')"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目指出了预定传送的WEP密钥。', LEFT);" onMouseOut="return nd();">密钥索引：
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11a', '')"><option class="content_input_fd" value="Key1" <% nvram_match_x("WLANConfig11a","", "Key1","selected"); %>>Key1</option><option class="content_input_fd" value="Key2" <% nvram_match_x("WLANConfig11a","", "Key2","selected"); %>>Key2</option><option class="content_input_fd" value="Key3" <% nvram_match_x("WLANConfig11a","", "Key3","selected"); %>>Key3</option><option class="content_input_fd" value="Key4" <% nvram_match_x("WLANConfig11a","", "Key4","selected"); %>>Key4</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目可让您启用AES特有密钥传送功能。如果您启用此项功能，每个工作站则会传送及接收在「无线AES密钥控制」页中预设含有特有密钥加密的资料。', LEFT);" onMouseOut="return nd();">加密（AES特有密钥）：
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'WLANConfig11a', '', '1')" <% nvram_match_x("WLANConfig11a","", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'WLANConfig11a', '', '0')" <% nvram_match_x("WLANConfig11a","", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('选择「Yes」会将一些通过广播封包查询AP的用户端所传来的AP隐藏起来。因此，只有对此AP的SSID有所注意的用户端，才能和其连接。', LEFT);" onMouseOut="return nd();">隐藏网络名称SSID：
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'WLANConfig11a', '', '1')" <% nvram_match_x("WLANConfig11a","", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'WLANConfig11a', '', '0')" <% nvram_match_x("WLANConfig11a","", "0", "checked"); %>>No</input></td>
</tr>
</table>
</td>
</tr>
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">802.11g界面
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('为您的无线局域网（WLAN）指定一组可为32个字符的识别字串。', LEFT);" onMouseOut="return nd();">SSID：
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="wl_ssid" value="<% nvram_get_x("WLANConfig11b","wl_ssid"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('选择操作时的无线频道', LEFT);" onMouseOut="return nd();">频道：
           </td><td class="content_input_td"><select name="wl_channel" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11b', 'wl_channel')">   
<% select_channel("WLANConfig11b"); %>
                </select><input type="hidden" maxlength="15" size="15" name="x_RegulatoryDomain" value="<% nvram_get_x("Regulatory","x_RegulatoryDomain"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目指出了802.11g界面所使用的模式。选择「Auto」时，可允许802.11g及802.11b连接至您的装置。选择「54g」，只能使效能达至最大，但802.11b用户端却无法连接您的装置。如果勾选「54g Protection」（54g保护），11g流量的G-Mode保护便会针对11b流量的现状而自动启用。', LEFT);" onMouseOut="return nd();">无线模式：
           </td><td class="content_input_td"><select name="wl_gmode" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11b', 'wl_gmode')"><option class="content_input_fd" value="1" <% nvram_match_x("WLANConfig11b","wl_gmode", "1","selected"); %>>Auto</option><option class="content_input_fd" value="4" <% nvram_match_x("WLANConfig11b","wl_gmode", "4","selected"); %>>54G Only</option><option class="content_input_fd" value="0" <% nvram_match_x("WLANConfig11b","wl_gmode", "0","selected"); %>>802.11B Only</option></select><input type="checkbox" style="margin-left:30" name="wl_gmode_check" value="" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_gmode', '1')">54g Protection</input></td>
</tr>
<input type="hidden" name="wl_gmode_protection_x" value="<% nvram_get_x("WLANConfig11b","wl_gmode_protection_x"); %>">
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目指出了用来对无线用户进行认证的方式。选择不同的认证方式，需要应用不同的加密结构。', LEFT);" onMouseOut="return nd();">认证方式：
           </td><td class="content_input_td"><select name="wl_auth_mode" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11b', 'wl_auth_mode')"><option class="content_input_fd" value="open" <% nvram_match_x("WLANConfig11b","wl_auth_mode", "open","selected"); %>>Open System or Shared Key</option><option class="content_input_fd" value="shared" <% nvram_match_x("WLANConfig11b","wl_auth_mode", "shared","selected"); %>>Shared Key</option><option class="content_input_fd" value="psk" <% nvram_match_x("WLANConfig11b","wl_auth_mode", "psk","selected"); %>>WPA-PSK</option><option class="content_input_fd" value="wpa" <% nvram_match_x("WLANConfig11b","wl_auth_mode", "wpa","selected"); %>>WPA</option><option class="content_input_fd" value="radius" <% nvram_match_x("WLANConfig11b","wl_auth_mode", "radius","selected"); %>>Radius with 802.1x</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目指出了在WPA加密功能启用时，用来将资料加密的加密方式。', LEFT);" onMouseOut="return nd();">WPA加密：
           </td><td class="content_input_td"><select name="wl_crypto" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11b', 'wl_crypto')"><option class="content_input_fd" value="tkip" <% nvram_match_x("WLANConfig11b","wl_crypto", "tkip","selected"); %>>TKIP</option><option class="content_input_fd" value="aes" <% nvram_match_x("WLANConfig11b","wl_crypto", "aes","selected"); %>>AES</option><option class="content_input_fd" value="tkip+aes" <% nvram_match_x("WLANConfig11b","wl_crypto", "tkip+aes","selected"); %>>TKIP+AES</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('WPA密钥(WPA-PSK)应为7个以上，64个以下的字符!!! 如果您将此栏目保留空白，系统将会指定00000000作为您的通行码。', LEFT);" onMouseOut="return nd();">WPA密钥(WPA-PSK)：
           </td><td class="content_input_td"><script language="JavaScript" type="text/javascript" src="md5.js"></script><input type="password" maxlength="64" class="content_input_fd" size="32" name="wl_wpa_psk" value="<% nvram_get_x("WLANConfig11b","wl_wpa_psk"); %>" onKeyUp="return is_wlphrase('WLANConfig11b', 'wl_wpa_psk', this)" onBlur="return validate_wlphrase('WLANConfig11b', 'wl_wpa_psk', this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目指出了当启用WEP加密功能，用来将资料加密的加密方式。', LEFT);" onMouseOut="return nd();">WEP加密：
           </td><td class="content_input_td"><select name="wl_wep_x" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11b', 'wl_wep_x')"><option class="content_input_fd" value="0" <% nvram_match_x("WLANConfig11b","wl_wep_x", "0","selected"); %>>None</option><option class="content_input_fd" value="1" <% nvram_match_x("WLANConfig11b","wl_wep_x", "1","selected"); %>>WEP-64bits</option><option class="content_input_fd" value="2" <% nvram_match_x("WLANConfig11b","wl_wep_x", "2","selected"); %>>WEP-128bits</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('选择WEP-64bits或WEP-128bits加密方式，本栏目可用来自动产生四组的WEP密钥。', LEFT);" onMouseOut="return nd();">通行码：
           </td><td class="content_input_td"><script language="JavaScript" type="text/javascript" src="md5.js"></script><input type="password" maxlength="64" class="content_input_fd" size="32" name="wl_phrase_x" value="<% nvram_get_x("WLANConfig11b","wl_phrase_x"); %>" onKeyUp="return is_wlphrase('WLANConfig11b', 'wl_phrase_x', this)" onBlur="return validate_wlphrase('WLANConfig11b', 'wl_phrase_x', this)"></td>
</tr>
<tr>
<td class="content_header_td">WEP密钥1（10或26十六进制数）：
           </td><td class="content_input_td"><input type="pssword" maxlength="32" class="content_input_fd" size="32" name="wl_key1" value="<% nvram_get_x("WLANConfig11b","wl_key1"); %>" onBlur="return validate_wlkey(this, 'WLANConfig11b')" onKeyPress="return is_wlkey(this, 'WLANConfig11b')" onKeyUp="return change_wlkey(this, 'WLANConfig11b')"></td>
</tr>
<tr>
<td class="content_header_td">WEP密钥2（10或26十六进制数）：
           </td><td class="content_input_td"><input type="pssword" maxlength="32" class="content_input_fd" size="32" name="wl_key2" value="<% nvram_get_x("WLANConfig11b","wl_key2"); %>" onBlur="return validate_wlkey(this, 'WLANConfig11b')" onKeyPress="return is_wlkey(this, 'WLANConfig11b')" onKeyUp="return change_wlkey(this, 'WLANConfig11b')"></td>
</tr>
<tr>
<td class="content_header_td">WEP密钥3（10或26十六进制数）：
           </td><td class="content_input_td"><input type="pssword" maxlength="32" class="content_input_fd" size="32" name="wl_key3" value="<% nvram_get_x("WLANConfig11b","wl_key3"); %>" onBlur="return validate_wlkey(this, 'WLANConfig11b')" onKeyPress="return is_wlkey(this, 'WLANConfig11b')" onKeyUp="return change_wlkey(this, 'WLANConfig11b')"></td>
</tr>
<tr>
<td class="content_header_td">WEP密钥4（10或26十六进制数）：
           </td><td class="content_input_td"><input type="pssword" maxlength="32" class="content_input_fd" size="32" name="wl_key4" value="<% nvram_get_x("WLANConfig11b","wl_key4"); %>" onBlur="return validate_wlkey(this, 'WLANConfig11b')" onKeyPress="return is_wlkey(this, 'WLANConfig11b')" onKeyUp="return change_wlkey(this, 'WLANConfig11b')"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目指出了预定传送的WEP密钥。', LEFT);" onMouseOut="return nd();">密钥索引：
           </td><td class="content_input_td"><select name="wl_key" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11b', 'wl_key')"><option class="content_input_fd" value="1" <% nvram_match_x("WLANConfig11b","wl_key", "1","selected"); %>>Key1</option><option class="content_input_fd" value="2" <% nvram_match_x("WLANConfig11b","wl_key", "2","selected"); %>>Key2</option><option class="content_input_fd" value="3" <% nvram_match_x("WLANConfig11b","wl_key", "3","selected"); %>>Key3</option><option class="content_input_fd" value="4" <% nvram_match_x("WLANConfig11b","wl_key", "4","selected"); %>>Key4</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目指定了WPA群组密钥变更，以秒计算的时间间隔。0表示无须周期性密钥变更。', LEFT);" onMouseOut="return nd();">网络密钥转动间隔：
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="wl_wpa_gtk_rekey" class="content_input_fd" value="<% nvram_get_x("WLANConfig11b", "wl_wpa_gtk_rekey"); %>" onBlur="validate_range(this, 0, 86400)" onKeyPress="return is_number(this)"></td>
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
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="B0B0B0">
<tr bgcolor="#CCCCCC"><td colspan="3"><font face="arial" size="2"><b>&nbsp</b></font></td></tr>
<tr bgcolor="#FFFFFF">  
   <td id ="Confirm" height="25" width="34%">  
   <div align="center"><font face="Arial"> <input class=inputSubmit onMouseOut=buttonOut(this) onMouseOver="buttonOver(this)" onClick="onSubmitCtrl(this, ' Restore ')" type="submit" value=" 还原 " name="action"></font></div> 
   </td>  
   <td height="25" width="33%">  
   <div align="center"><font face="Arial"> <input class=inputSubmit onMouseOut=buttonOut(this) onMouseOver="buttonOver(this)" onClick="onSubmitCtrl(this, ' Finish ')" type="submit" value=" 保存 " name="action"></font></div> 
   </td>
   <td height="25" width="33%">  
   <div align="center"><font face="Arial"> <input class=inputSubmit onMouseOut=buttonOut(this) onMouseOver="buttonOver(this)" onClick="onSubmitCtrl(this, ' Apply ')" type="submit" value=" 应用 " name="action"></font></div> 
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
    <td class="content_header_td_15" align="left">还原: </td>
    <td class="content_input_td_padding" align="left">清除上述设置并还原有效设置。</td>
</tr>
<tr bgcolor="#FFFFFF">
    <td class="content_header_td_15" align="left">保存: </td>
    <td class="content_input_td_padding" align="left">现在确认所有设置并重新启动ZVMODELVZ。</td>
</tr>
<tr bgcolor="#FFFFFF">
    <td class="content_header_td_15" align="left">应用: </td>
    <td class="content_input_td_padding" align="left">确认上述设置并继续。</td>
</tr>
</table>
</td>
</tr>

</table>
</form>
</body>
