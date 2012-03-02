<head>
<title>WL530g Web Manager</title>
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
    	
<input type="hidden" name="current_page" value="Advanced_USBStorage_Content.asp"><input type="hidden" name="next_page" value="Advanced_WebCam_Content.asp"><input type="hidden" name="next_host" value=""><input type="hidden" name="sid_list" value="PrinterStatus;"><input type="hidden" name="group_id" value=""><input type="hidden" name="modified" value="0"><input type="hidden" name="action_mode" value=""><input type="hidden" name="first_time" value=""><input type="hidden" name="action_script" value="">
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">USB應用程式 - FTP伺服器</td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('在移除您的USB行動碟之前，請按下此鍵將快取資料寫回到行動碟上並等候本頁面再次顯示。', LEFT);" onMouseOut="return nd();">強制退出USB行動碟：
           </td><td class="content_input_td"><input type="hidden" maxlength="15" class="content_input_fd_ro" size="12" name="" value="<% nvram_get_f("ddns.log","DDNSStatus"); %>"><input type="submit" maxlength="15" class="content_input_fd_ro" onClick="return onSubmitApply('eject-usb.sh')" size="12" name="PrinterStatus_x_FEject_button" value="退出"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('選擇「Yes」，可利用您的USB行動碟建立一處FTP伺服器。', LEFT);" onMouseOut="return nd();">啟用FTP伺服器？
           </td><td class="content_input_td"><input type="radio" value="1" name="usb_ftpenable_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_ftpenable_x', '1')" <% nvram_match_x("PrinterStatus","usb_ftpenable_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="usb_ftpenable_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_ftpenable_x', '0')" <% nvram_match_x("PrinterStatus","usb_ftpenable_x", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('選擇「Yes」，可讓用戶無須密碼即可連上您的FTP伺服器。匿名（Anonymous）帳號的主目錄（home directory）是USB行動碟的根目錄。', LEFT);" onMouseOut="return nd();">允許匿名用戶登入？
           </td><td class="content_input_td"><input type="radio" value="1" name="usb_ftpanonymous_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_ftpanonymous_x', '1')" <% nvram_match_x("PrinterStatus","usb_ftpanonymous_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="usb_ftpanonymous_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_ftpanonymous_x', '0')" <% nvram_match_x("PrinterStatus","usb_ftpanonymous_x", "0", "checked"); %>>No</input><a href="javascript:openLink('x_FIsAnonymous')" class="content_input_link" name="x_FIsAnonymous_link">登入
             </a></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('選擇「Yes」，可讓用戶以相同登入網管軟體（Web Manager）的帳號來連上您的FTP伺服器。超級用戶（Superuser）帳號的主目錄是USB行動碟的根目錄。', LEFT);" onMouseOut="return nd();">允許超級用戶登入？
           </td><td class="content_input_td"><input type="radio" value="1" name="usb_ftpsuper_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_ftpsuper_x', '1')" <% nvram_match_x("PrinterStatus","usb_ftpsuper_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="usb_ftpsuper_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_ftpsuper_x', '0')" <% nvram_match_x("PrinterStatus","usb_ftpsuper_x", "0", "checked"); %>>No</input><a href="javascript:openLink('x_FIsSuperuser')" class="content_input_link" name="x_FIsSuperuser_link">登入
             </a></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位為指出FTP通訊協定所使用的通訊埠編號。', LEFT);" onMouseOut="return nd();">FTP通訊埠：
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="usb_ftpport_x" class="content_input_fd" value="<% nvram_get_x("PrinterStatus", "usb_ftpport_x"); %>" onBlur="validate_range(this, 1, 65535)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位為指出允許能夠同時間登入FTP伺服器的最大用戶數。', LEFT);" onMouseOut="return nd();">允許登入的最大用戶數：
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="usb_ftpmax_x" class="content_input_fd" value="<% nvram_get_x("PrinterStatus", "usb_ftpmax_x"); %>" onBlur="validate_range(this, 1, 12)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位讓您可在用戶輸入登入名稱一段特定的時間後終止用戶連線。', LEFT);" onMouseOut="return nd();">以秒計算的登入逾時時間：
           </td><td class="content_input_td"><input type="text" maxlength="5" class="content_input_fd" size="5" name="usb_ftptimeout_x" value="<% nvram_get_x("PrinterStatus","usb_ftptimeout_x"); %>" onBlur="validate_range(this, 0, 65535)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位讓您可在用戶登入卻維持閒置一段特定的時間後終止用戶連線。', LEFT);" onMouseOut="return nd();">以秒計算的維持逾時時間：
           </td><td class="content_input_td"><input type="text" maxlength="5" class="content_input_fd" size="5" name="usb_ftpstaytimeout_x" value="<% nvram_get_x("PrinterStatus","usb_ftpstaytimeout_x"); %>" onBlur="validate_range(this, 0, 65535)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('當你接上USB硬碟後，WL530g 會在此硬碟的根目錄尋找你指定的檔案並且執行。所以此欄位空白將會自動關閉這一個功能。', LEFT);" onMouseOut="return nd();">自動執行程式:
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="usb_ftpscript_x" value="<% nvram_get_x("PrinterStatus","usb_ftpscript_x"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
</table>
</td>
</tr>
<tr>
<td>
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_list_header_tr">
<td class="content_list_header_td" width="60%" id="x_FUserList">用戶帳號清單
         <input type="hidden" name="usb_ftpnum_x_0" value="<% nvram_get_x("PrinterStatus", "usb_ftpnum_x"); %>" readonly="1"></td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'x_FUserList', 32, ' Add ');" name="x_FUserList" value="新增" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'x_FUserList', 32, ' Del ');" name="x_FUserList" value="刪除" size="12">
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
<td></td><td class="content_list_field_header_td" colspan="">用戶名稱	                
           	        </td><td class="content_list_field_header_td" colspan="">密碼	                
           	        </td><td class="content_list_field_header_td" colspan="">最大登入數	                
           	        </td><td class="content_list_field_header_td" colspan="">用戶權益	                
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
<td class="content_list_header_td" width="60%" id="x_FBanIPList">禁止IP清單
         <input type="hidden" name="usb_bannum_x_0" value="<% nvram_get_x("PrinterStatus", "usb_bannum_x"); %>" readonly="1"></td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'x_FBanIPList', 16, ' Add ');" name="x_FBanIPList" value="新增" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'x_FBanIPList', 16, ' Del ');" name="x_FBanIPList" value="刪除" size="12">
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
<td></td><td class="content_list_field_header_td" colspan="">IP位址	                
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
    <td class="content_input_td_padding" align="left">現在確認所有設定值並重新啟動WL530g。</td>
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
