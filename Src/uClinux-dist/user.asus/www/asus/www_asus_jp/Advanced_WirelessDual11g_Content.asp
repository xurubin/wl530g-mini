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
    	
<input type="hidden" name="current_page" value="Advanced_Wireless_Content.asp"><input type="hidden" name="next_page" value="Advanced_UniqueKey_Content.asp"><input type="hidden" name="next_host" value=""><input type="hidden" name="sid_list" value="WLANConfig11a;WLANConfig11b;"><input type="hidden" name="group_id" value=""><input type="hidden" name="modified" value="0"><input type="hidden" name="action_mode" value=""><input type="hidden" name="first_time" value=""><input type="hidden" name="action_script" value="">
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">ワイヤレス - インターフェイス</td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このフィールドは、ZVMODELVZがユーザーの国のワイヤレス仕様に従えるようにします。', LEFT);" onMouseOut="return nd();">国コード:
           </td><td class="content_input_td"><select name="" class="content_input_fd">   
<% select_country("WLANConfig11a"); %>
                </select></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">802.11aインターフェイス
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('WLANに対して、32文字以内の証明書文字列を割り当てます。', LEFT);" onMouseOut="return nd();">SSID:
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("WLANConfig11a",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('操作無線チャンネルを選択してください。屋外チャンネルがチェックされていると、一部の特殊チャンネルを選択できます。', LEFT);" onMouseOut="return nd();">チャンネル:
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11a', '')">   
<% select_channel("WLANConfig11a"); %>
                </select><input type="hidden" maxlength="15" size="15" name="x_RegulatoryDomain" value="<% nvram_get_x("Regulatory","x_RegulatoryDomain"); %>" readonly="1"><input type="checkbox" style="margin-left:30" name="_check" value="" onClick="return change_common_radio(this, 'WLANConfig11a', '', '1')">Outdoor Channel</input></td>
</tr>
<input type="hidden" name="" value="<% nvram_get_x("WLANConfig11a",""); %>">
<tr>
<td class="content_header_td" onMouseOver="return overlib('伝送されるデータ速度を選択してください。パフォーマンスを最大にするために、Bestを選択することをお勧めします。ターボモードにチェックしている場合、最大108 Mpbsのデータ速度が得られます。', LEFT);" onMouseOut="return nd();">データ転送速度(Mbps):
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11a', '')"><option class="content_input_fd" value="0" <% nvram_match_x("WLANConfig11a","", "0","selected"); %>>Best</option><option class="content_input_fd" value="1" <% nvram_match_x("WLANConfig11a","", "1","selected"); %>>6</option><option class="content_input_fd" value="2" <% nvram_match_x("WLANConfig11a","", "2","selected"); %>>9</option><option class="content_input_fd" value="3" <% nvram_match_x("WLANConfig11a","", "3","selected"); %>>12</option><option class="content_input_fd" value="4" <% nvram_match_x("WLANConfig11a","", "4","selected"); %>>18</option><option class="content_input_fd" value="5" <% nvram_match_x("WLANConfig11a","", "5","selected"); %>>24</option><option class="content_input_fd" value="6" <% nvram_match_x("WLANConfig11a","", "6","selected"); %>>36</option><option class="content_input_fd" value="7" <% nvram_match_x("WLANConfig11a","", "7","selected"); %>>48</option><option class="content_input_fd" value="8" <% nvram_match_x("WLANConfig11a","", "8","selected"); %>>54</option></select><input type="checkbox" style="margin-left:30" name="_check" value="" onClick="return change_common_radio(this, 'WLANConfig11a', '', '1')">Turbo Mode</input></td>
</tr>
<input type="hidden" name="" value="<% nvram_get_x("WLANConfig11a",""); %>">
<tr>
<td class="content_header_td" onMouseOver="return overlib('このフィールドにより、暗号化が認証段階で要求されているかどうかを設定できます。', LEFT);" onMouseOut="return nd();">認証方式:
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11a', '')"><option class="content_input_fd" value="Open System" <% nvram_match_x("WLANConfig11a","", "Open System","selected"); %>>Open System</option><option class="content_input_fd" value="Shared Key" <% nvram_match_x("WLANConfig11a","", "Shared Key","selected"); %>>Shared Key</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('WEPを有効にすると、データが盗聴者に読み取られないように暗号化できます。', LEFT);" onMouseOut="return nd();">暗号化(WEP):
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_wlweptype(this, 'WLANConfig11a')"><option class="content_input_fd" value="None" <% nvram_match_x("WLANConfig11a","", "None","selected"); %>>None</option><option class="content_input_fd" value="64bits" <% nvram_match_x("WLANConfig11a","", "64bits","selected"); %>>64bits</option><option class="content_input_fd" value="128bits" <% nvram_match_x("WLANConfig11a","", "128bits","selected"); %>>128bits</option><option class="content_input_fd" value="152bits" <% nvram_match_x("WLANConfig11a","", "152bits","selected"); %>>152bits</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('マジックワードを与えるとWEPキーが自動的に生成され、このフィールドをブランクにするとキーを手動で入力できます。', LEFT);" onMouseOut="return nd();">フレーズ:
           </td><td class="content_input_td"><script language="JavaScript" type="text/javascript" src="md5.js"></script><input type="password" maxlength="64" class="content_input_fd" size="32" name="" value="<% nvram_get_x("WLANConfig11a",""); %>" onKeyUp="return is_wlphrase('WLANConfig11a', '', this)" onBlur="return validate_wlphrase('WLANConfig11a', '', this)"></td>
</tr>
<tr>
<td class="content_header_td">WEPキー1 (10または26または32桁の16進数字):
           </td><td class="content_input_td"><input type="pssword" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("WLANConfig11a",""); %>" onBlur="return validate_wlkey(this, 'WLANConfig11a')" onKeyPress="return is_wlkey(this, 'WLANConfig11a')" onKeyUp="return change_wlkey(this, 'WLANConfig11a')"></td>
</tr>
<tr>
<td class="content_header_td">WEPキー2 (10または26または32桁の16進数字):
           </td><td class="content_input_td"><input type="pssword" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("WLANConfig11a",""); %>" onBlur="return validate_wlkey(this, 'WLANConfig11a')" onKeyPress="return is_wlkey(this, 'WLANConfig11a')" onKeyUp="return change_wlkey(this, 'WLANConfig11a')"></td>
</tr>
<tr>
<td class="content_header_td">WEPキー3 (10または26または32桁の16進数字):
           </td><td class="content_input_td"><input type="pssword" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("WLANConfig11a",""); %>" onBlur="return validate_wlkey(this, 'WLANConfig11a')" onKeyPress="return is_wlkey(this, 'WLANConfig11a')" onKeyUp="return change_wlkey(this, 'WLANConfig11a')"></td>
</tr>
<tr>
<td class="content_header_td">WEPキー4 (10または26または32桁の16進数字):
           </td><td class="content_input_td"><input type="pssword" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("WLANConfig11a",""); %>" onBlur="return validate_wlkey(this, 'WLANConfig11a')" onKeyPress="return is_wlkey(this, 'WLANConfig11a')" onKeyUp="return change_wlkey(this, 'WLANConfig11a')"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このフィールドは、デフォルトの伝送WEPキーを示します。', LEFT);" onMouseOut="return nd();">デフォルトキー:
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11a', '')"><option class="content_input_fd" value="Key1" <% nvram_match_x("WLANConfig11a","", "Key1","selected"); %>>Key1</option><option class="content_input_fd" value="Key2" <% nvram_match_x("WLANConfig11a","", "Key2","selected"); %>>Key2</option><option class="content_input_fd" value="Key3" <% nvram_match_x("WLANConfig11a","", "Key3","selected"); %>>Key3</option><option class="content_input_fd" value="Key4" <% nvram_match_x("WLANConfig11a","", "Key4","selected"); %>>Key4</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このフィールドは、AESユニークキー伝送を可能にします。これを有効にすると、各端末はワイヤレスAESキーコントロールページでプリセットされた、ユニークキーで暗号化されたデータを送受信します。', LEFT);" onMouseOut="return nd();">暗号化(AESユニークキー):
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'WLANConfig11a', '', '1')" <% nvram_match_x("WLANConfig11a","", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'WLANConfig11a', '', '0')" <% nvram_match_x("WLANConfig11a","", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このフィールドは、ブロードキャストSSIDを持つプローブメッセージがブロックされるかどうかを示します。', LEFT);" onMouseOut="return nd();">ブロックブロードキャストSSID:
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'WLANConfig11a', '', '1')" <% nvram_match_x("WLANConfig11a","", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'WLANConfig11a', '', '0')" <% nvram_match_x("WLANConfig11a","", "0", "checked"); %>>No</input></td>
</tr>
</table>
</td>
</tr>
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">802.11gインターフェイス
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('32文字以内の文字からなる、識別文字列を適用します。', LEFT);" onMouseOut="return nd();">SSID:
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="wl_ssid" value="<% nvram_get_x("WLANConfig11b","wl_ssid"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('操作している無線ラジオチャンネルを選択する', LEFT);" onMouseOut="return nd();">チャンネル:
           </td><td class="content_input_td"><select name="wl_channel" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11b', 'wl_channel')">   
<% select_channel("WLANConfig11b"); %>
                </select><input type="hidden" maxlength="15" size="15" name="x_RegulatoryDomain" value="<% nvram_get_x("Regulatory","x_RegulatoryDomain"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このフィールドは802.11gインターフェイスのモードを示します。Auto（自動）を選択すると、802.11gと802.11bがデバイスに選択できます。54gのみを選択するとパフォーマンスが最大化されますが、802.11bクライアントはデバイスへの接続を許可されません。54g LRSを選択すると、完全な11gレートセットリストを処理できない従来の802.11bクライアントをサポートします。54g保護がチェックされていると、11gトラヒックのGモード保護が11bトラヒックのあるところで自動的に有効になります。', LEFT);" onMouseOut="return nd();">54gモード:
           </td><td class="content_input_td"><select name="wl_gmode" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11b', 'wl_gmode')"><option class="content_input_fd" value="1" <% nvram_match_x("WLANConfig11b","wl_gmode", "1","selected"); %>>Auto</option><option class="content_input_fd" value="4" <% nvram_match_x("WLANConfig11b","wl_gmode", "4","selected"); %>>54G Only</option><option class="content_input_fd" value="0" <% nvram_match_x("WLANConfig11b","wl_gmode", "0","selected"); %>>802.11B Only</option></select><input type="checkbox" style="margin-left:30" name="wl_gmode_check" value="" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_gmode', '1')">54g Protection</input></td>
</tr>
<input type="hidden" name="wl_gmode_protection_x" value="<% nvram_get_x("WLANConfig11b","wl_gmode_protection_x"); %>">
<tr>
<td class="content_header_td" onMouseOver="return overlib('このフィールドは、ワイヤレスユーザーの認証に使用される方式を示します。異なる認証方式を選択すると、異なる暗号化方式が有効になります。', LEFT);" onMouseOut="return nd();">認証方式:
           </td><td class="content_input_td"><select name="wl_auth_mode" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11b', 'wl_auth_mode')"><option class="content_input_fd" value="open" <% nvram_match_x("WLANConfig11b","wl_auth_mode", "open","selected"); %>>Open System or Shared Key</option><option class="content_input_fd" value="shared" <% nvram_match_x("WLANConfig11b","wl_auth_mode", "shared","selected"); %>>Shared Key</option><option class="content_input_fd" value="psk" <% nvram_match_x("WLANConfig11b","wl_auth_mode", "psk","selected"); %>>WPA-PSK</option><option class="content_input_fd" value="wpa" <% nvram_match_x("WLANConfig11b","wl_auth_mode", "wpa","selected"); %>>WPA</option><option class="content_input_fd" value="radius" <% nvram_match_x("WLANConfig11b","wl_auth_mode", "radius","selected"); %>>Radius with 802.1x</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このフィールドは、WPA暗号化が有効になっている間データを暗号化するために使用される暗号化方式を示します。', LEFT);" onMouseOut="return nd();">WPA暗号化:
           </td><td class="content_input_td"><select name="wl_crypto" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11b', 'wl_crypto')"><option class="content_input_fd" value="tkip" <% nvram_match_x("WLANConfig11b","wl_crypto", "tkip","selected"); %>>TKIP</option><option class="content_input_fd" value="aes" <% nvram_match_x("WLANConfig11b","wl_crypto", "aes","selected"); %>>AES</option><option class="content_input_fd" value="tkip+aes" <% nvram_match_x("WLANConfig11b","wl_crypto", "tkip+aes","selected"); %>>TKIP+AES</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('プレ共有キーは7文字以上64文字以下で設定してください!!! このフィールドをブランクにすると、システムはパスフレーズとして00000000を割り当てます。', LEFT);" onMouseOut="return nd();">WPA プレ共有キー:
           </td><td class="content_input_td"><script language="JavaScript" type="text/javascript" src="md5.js"></script><input type="password" maxlength="64" class="content_input_fd" size="32" name="wl_wpa_psk" value="<% nvram_get_x("WLANConfig11b","wl_wpa_psk"); %>" onKeyUp="return is_wlphrase('WLANConfig11b', 'wl_wpa_psk', this)" onBlur="return validate_wlphrase('WLANConfig11b', 'wl_wpa_psk', this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このフィールドは、データを暗号化し盗聴を防ぐために使用される暗号化方式を示します。', LEFT);" onMouseOut="return nd();">暗号化:
           </td><td class="content_input_td"><select name="wl_wep_x" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11b', 'wl_wep_x')"><option class="content_input_fd" value="0" <% nvram_match_x("WLANConfig11b","wl_wep_x", "0","selected"); %>>None</option><option class="content_input_fd" value="1" <% nvram_match_x("WLANConfig11b","wl_wep_x", "1","selected"); %>>WEP-64bits</option><option class="content_input_fd" value="2" <% nvram_match_x("WLANConfig11b","wl_wep_x", "2","selected"); %>>WEP-128bits</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('TKIPまたはAES暗号化方式を選択しているとき、このフィールドは暗号化プロセスを開始するためのパスワードとして使用されます。WEP-64ビットまたはWEP-128ビットを選択しているとき、このフィールドは4つのWEPキーを自動的に生成するために使用されます。', LEFT);" onMouseOut="return nd();">パスフレーズ:
           </td><td class="content_input_td"><script language="JavaScript" type="text/javascript" src="md5.js"></script><input type="password" maxlength="64" class="content_input_fd" size="32" name="wl_phrase_x" value="<% nvram_get_x("WLANConfig11b","wl_phrase_x"); %>" onKeyUp="return is_wlphrase('WLANConfig11b', 'wl_phrase_x', this)" onBlur="return validate_wlphrase('WLANConfig11b', 'wl_phrase_x', this)"></td>
</tr>
<tr>
<td class="content_header_td">WEPキー1 (10または26桁の16進数字):
           </td><td class="content_input_td"><input type="pssword" maxlength="32" class="content_input_fd" size="32" name="wl_key1" value="<% nvram_get_x("WLANConfig11b","wl_key1"); %>" onBlur="return validate_wlkey(this, 'WLANConfig11b')" onKeyPress="return is_wlkey(this, 'WLANConfig11b')" onKeyUp="return change_wlkey(this, 'WLANConfig11b')"></td>
</tr>
<tr>
<td class="content_header_td">WEPキー2 (10または26桁の16進数字):
           </td><td class="content_input_td"><input type="pssword" maxlength="32" class="content_input_fd" size="32" name="wl_key2" value="<% nvram_get_x("WLANConfig11b","wl_key2"); %>" onBlur="return validate_wlkey(this, 'WLANConfig11b')" onKeyPress="return is_wlkey(this, 'WLANConfig11b')" onKeyUp="return change_wlkey(this, 'WLANConfig11b')"></td>
</tr>
<tr>
<td class="content_header_td">WEPキー3 (10または26桁の16進数字):
           </td><td class="content_input_td"><input type="pssword" maxlength="32" class="content_input_fd" size="32" name="wl_key3" value="<% nvram_get_x("WLANConfig11b","wl_key3"); %>" onBlur="return validate_wlkey(this, 'WLANConfig11b')" onKeyPress="return is_wlkey(this, 'WLANConfig11b')" onKeyUp="return change_wlkey(this, 'WLANConfig11b')"></td>
</tr>
<tr>
<td class="content_header_td">WEPキー4 (10または26桁の16進数字):
           </td><td class="content_input_td"><input type="pssword" maxlength="32" class="content_input_fd" size="32" name="wl_key4" value="<% nvram_get_x("WLANConfig11b","wl_key4"); %>" onBlur="return validate_wlkey(this, 'WLANConfig11b')" onKeyPress="return is_wlkey(this, 'WLANConfig11b')" onKeyUp="return change_wlkey(this, 'WLANConfig11b')"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このフィールドは、デフォルトの伝送WEPキーを示します。', LEFT);" onMouseOut="return nd();">デフォルトキー:
           </td><td class="content_input_td"><select name="wl_key" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11b', 'wl_key')"><option class="content_input_fd" value="1" <% nvram_match_x("WLANConfig11b","wl_key", "1","selected"); %>>Key1</option><option class="content_input_fd" value="2" <% nvram_match_x("WLANConfig11b","wl_key", "2","selected"); %>>Key2</option><option class="content_input_fd" value="3" <% nvram_match_x("WLANConfig11b","wl_key", "3","selected"); %>>Key3</option><option class="content_input_fd" value="4" <% nvram_match_x("WLANConfig11b","wl_key", "4","selected"); %>>Key4</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('このフィールドは、WPAグループキーが変更される時間間隔を秒で示します。0は、周期的なk-変更が必要ないことを意味します。', LEFT);" onMouseOut="return nd();">WPA Re-keyタイマー:
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
