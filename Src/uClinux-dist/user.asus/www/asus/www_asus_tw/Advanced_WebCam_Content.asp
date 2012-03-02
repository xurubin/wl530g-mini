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
    	
<input type="hidden" name="current_page" value="Advanced_WebCam_Content.asp"><input type="hidden" name="next_page" value="SaveRestart.asp"><input type="hidden" name="next_host" value=""><input type="hidden" name="sid_list" value="PrinterStatus;"><input type="hidden" name="group_id" value=""><input type="hidden" name="modified" value="0"><input type="hidden" name="action_mode" value=""><input type="hidden" name="first_time" value=""><input type="hidden" name="action_script" value="">
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">USB應用程式 - 網路攝影機</td>
</tr>
<tr>
<td class="content_header_td">啟用網路攝影機功能？
           </td><td class="content_input_td"><select name="usb_webenable_x" class="content_input_fd" onChange="return change_common(this, 'PrinterStatus', 'usb_webenable_x')"><option class="content_input_fd" value="0" <% nvram_match_x("PrinterStatus","usb_webenable_x", "0","selected"); %>>Disabled</option><option class="content_input_fd" value="1" <% nvram_match_x("PrinterStatus","usb_webenable_x", "1","selected"); %>>LAN Only</option><option class="content_input_fd" value="2" <% nvram_match_x("PrinterStatus","usb_webenable_x", "2","selected"); %>>LAN and WAN</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄為指出您所使用的網路攝影機模式。選擇「ActiveX Only」讓您可在IE平台上執行ActiveX客戶端軟體以取得最佳的影像品質。選擇「ActiveX and Refresh」可讓您在IE平台上執行ActiveX客戶端軟體並在其他瀏覽器上的檢視基本影像。選擇「Refresh Only」讓您可在所有的瀏覽器上檢視基本影像。', LEFT);" onMouseOut="return nd();">網路攝影機模式：
           </td><td class="content_input_td"><select name="usb_webmode_x" class="content_input_fd" onChange="return change_common(this, 'PrinterStatus', 'usb_webmode_x')"><option class="content_input_fd" value="0" <% nvram_match_x("PrinterStatus","usb_webmode_x", "0","selected"); %>>ActiveX Only</option><option class="content_input_fd" value="1" <% nvram_match_x("PrinterStatus","usb_webmode_x", "1","selected"); %>>ActiveX and Refresh</option><option class="content_input_fd" value="2" <% nvram_match_x("PrinterStatus","usb_webmode_x", "2","selected"); %>>Refresh Only</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄為您網路攝影機所使用的驅動程式。', LEFT);" onMouseOut="return nd();">網路攝影機驅動程式：
           </td><td class="content_input_td"><select name="usb_webdriver_x" class="content_input_fd" onChange="return change_common(this, 'PrinterStatus', 'usb_webdriver_x')"><option class="content_input_fd" value="0" <% nvram_match_x("PrinterStatus","usb_webdriver_x", "0","selected"); %>>PWC 8.8</option><option class="content_input_fd" value="1" <% nvram_match_x("PrinterStatus","usb_webdriver_x", "1","selected"); %>>OV511 2.10</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位為您所希望的影像大小。', LEFT);" onMouseOut="return nd();">影像大小：
           </td><td class="content_input_td"><select name="usb_webimage_x" class="content_input_fd" onChange="return change_common(this, 'PrinterStatus', 'usb_webimage_x')"><option class="content_input_fd" value="0" <% nvram_match_x("PrinterStatus","usb_webimage_x", "0","selected"); %>>640 X 480</option><option class="content_input_fd" value="1" <% nvram_match_x("PrinterStatus","usb_webimage_x", "1","selected"); %>>320 X 240</option><option class="content_input_fd" value="2" <% nvram_match_x("PrinterStatus","usb_webimage_x", "2","selected"); %>>160 X 120</option></select><a href="javascript:openLink('x_WImageSize')" class="content_input_link" name="x_WImageSize_link">預覽
             </a></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位為指出影像動作可被偵測的感應程度（sense level）。', LEFT);" onMouseOut="return nd();">感應程度：
           </td><td class="content_input_td"><select name="usb_websense_x" class="content_input_fd" onChange="return change_common(this, 'PrinterStatus', 'usb_websense_x')"><option class="content_input_fd" value="0" <% nvram_match_x("PrinterStatus","usb_websense_x", "0","selected"); %>>Low</option><option class="content_input_fd" value="1" <% nvram_match_x("PrinterStatus","usb_websense_x", "1","selected"); %>>Medium</option><option class="content_input_fd" value="2" <% nvram_match_x("PrinterStatus","usb_websense_x", "2","selected"); %>>High</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位為以秒計算指出系統重新載入影像所需的時間間隔。數值的範圍是1~65535。', LEFT);" onMouseOut="return nd();">以秒計算的更新時間：
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="usb_webfresh_x" class="content_input_fd" value="<% nvram_get_x("PrinterStatus", "usb_webfresh_x"); %>" onBlur="validate_range(this, 1, 65535)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位為指出在您的Webcam頁面上所顯示標題的字串。', LEFT);" onMouseOut="return nd();">標題字串：
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="usb_webcaption_x" value="<% nvram_get_x("PrinterStatus","usb_webcaption_x"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位為指出http伺服器針對通訊狀態進行監聽的通訊埠。', LEFT);" onMouseOut="return nd();">網路攝影機伺服器通訊埠：
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="usb_webhttpport_x" class="content_input_fd" value="<% nvram_get_x("PrinterStatus", "usb_webhttpport_x"); %>" onBlur="validate_range(this, 1024, 65535)" onKeyPress="return is_number(this)"><input type="checkbox" style="margin-left:30" name="usb_webhttpport_x_check" value="" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_webhttpport_x', '1')">Password Checking</input></td>
</tr>
<input type="hidden" name="usb_webhttpcheck_x" value="<% nvram_get_x("PrinterStatus","usb_webhttpcheck_x"); %>">
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位為指出伺服器監聽以ActiveX客戶端軟體進行通訊的通訊埠。此欄位不能和網路攝影機伺服器通訊埠相同。', LEFT);" onMouseOut="return nd();">ActiveX通訊埠：
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="usb_webactivex_x" class="content_input_fd" value="<% nvram_get_x("PrinterStatus", "usb_webactivex_x"); %>" onBlur="validate_range(this, 1024, 65535)" onKeyPress="return is_number(this)"></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">安全模式設定
            </td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">本項功能讓您可透過網路攝影機來監控您的作業環境。如果測到任何動作，ZVMODELVZ會嘗試以電子郵件來警告您。
         </td>
</tr>
<tr>
<td class="content_header_td">啟用安全模式？
           </td><td class="content_input_td"><input type="radio" value="1" name="usb_websecurity_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_websecurity_x', '1')" <% nvram_match_x("PrinterStatus","usb_websecurity_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="usb_websecurity_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_websecurity_x', '0')" <% nvram_match_x("PrinterStatus","usb_websecurity_x", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位為定出「安全模式」將會啟用的日期。', LEFT);" onMouseOut="return nd();">安全模式啟用日期：
           </td><td class="content_input_td"><input type="hidden" maxlength="7" class="content_input_fd" size="7" name="usb_websecurity_date_x" value="<% nvram_get_x("PrinterStatus","usb_websecurity_date_x"); %>">
<p style="word-spacing: 0; margin-top: 0; margin-bottom: 0">
<input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Sun" onChange="return changeDate();">Sun</input><input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Mon" onChange="return changeDate();">Mon</input><input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Tue" onChange="return changeDate();">Tue</input><input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Wed" onChange="return changeDate();">Wed</input>
</p>
<input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Thu" onChange="return changeDate();">Thu</input><input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Fri" onChange="return changeDate();">Fri</input><input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Sat" onChange="return changeDate();">Sat</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位可定出「安全模式」將會啟用的時間間隔。', LEFT);" onMouseOut="return nd();">安全模式啟用時間：
           </td><td class="content_input_td"><input type="hidden" maxlength="11" class="content_input_fd" size="11" name="usb_websecurity_time_x" value="<% nvram_get_x("PrinterStatus","usb_websecurity_time_x"); %>"><input type="text" maxlength="2" class="content_input_fd" size="2" name="usb_websecurity_time_x_starthour" onKeyPress="return is_number(this)" onBlur="return validate_timerange(this, 0)">:
                <input type="text" maxlength="2" class="content_input_fd" size="2" name="usb_websecurity_time_x_startmin" onKeyPress="return is_number(this)" onBlur="return validate_timerange(this, 1)">-
                <input type="text" maxlength="2" class="content_input_fd" size="2" name="usb_websecurity_time_x_endhour" onKeyPress="return is_number(this)" onBlur="return validate_timerange(this, 2)">:
                <input type="text" maxlength="2" class="content_input_fd" size="2" name="usb_websecurity_time_x_endmin" onKeyPress="return is_number(this)" onBlur="return validate_timerange(this, 3)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位為指出您想送往地點的電子郵件帳號。', LEFT);" onMouseOut="return nd();">傳送至：
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="usb_websendto_x" value="<% nvram_get_x("PrinterStatus","usb_websendto_x"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位為指出您想要傳送電子郵件目的電子郵件伺服器。如果您將本欄位保持空白，ZVMODELVZ將會自下方的電子郵件帳號當中去搜尋一「郵件交換主機（Mail Exchanger）。', LEFT);" onMouseOut="return nd();">電子郵件伺服器：
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="usb_webmserver_x" value="<% nvram_get_x("PrinterStatus","usb_webmserver_x"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位讓您可編輯電子郵件的主旨。', LEFT);" onMouseOut="return nd();">主旨：
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="usb_websubject_x" value="<% nvram_get_x("PrinterStatus","usb_websubject_x"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄位讓您可將測到的影像檔附加到電子郵件當中。', LEFT);" onMouseOut="return nd();">附加影像檔？
           </td><td class="content_input_td"><input type="radio" value="1" name="usb_webattach_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_webattach_x', '1')" <% nvram_match_x("PrinterStatus","usb_webattach_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="usb_webattach_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_webattach_x', '0')" <% nvram_match_x("PrinterStatus","usb_webattach_x", "0", "checked"); %>>No</input></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">遠端監控設定
            </td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">本功能讓您可對您LAN網路當中最多六部的網路攝影機進行監控。請輸入與網路攝影機連接的無線路由器的IP位址。如果想要將這網路攝影機公開顯露在網際網路上，請以手動方式為每個HTTP通訊埠及ActiveX通訊埠設置虛擬伺服器。
         </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('選擇「僅適用於LAN」，您便僅能在LAN作業環境內進行監控。如果您想從網際網路上進行監控，請以手動方式對虛擬伺服器清單的http通訊埠及activex進行設定。請考量安全問題。', LEFT);" onMouseOut="return nd();">遠端控制模式：
           </td><td class="content_input_td"><select name="usb_webremote_x" class="content_input_fd" onChange="return change_common(this, 'PrinterStatus', 'usb_webremote_x')"><option class="content_input_fd" value="0" <% nvram_match_x("PrinterStatus","usb_webremote_x", "0","selected"); %>>LAN Only</option></select><a href="javascript:openLink('x_WRemote')" class="content_input_link" name="x_WRemote_link">預覽
             </a></td>
</tr>
<tr>
<td class="content_header_td">遠端位置1：
           </td><td class="content_input_td"><input type="text" maxlength="21" class="content_input_fd" size="15" name="usb_webremote1_x" value="<% nvram_get_x("PrinterStatus","usb_webremote1_x"); %>" onBlur="return validate_ipaddrport(this, 'usb_webremote1_x')" onKeyPress="return is_ipaddrport(this)" onKeyUp="change_ipaddrport(this)"></td>
</tr>
<tr>
<td class="content_header_td">遠端位置2：
           </td><td class="content_input_td"><input type="text" maxlength="21" class="content_input_fd" size="15" name="usb_webremote2_x" value="<% nvram_get_x("PrinterStatus","usb_webremote2_x"); %>" onBlur="return validate_ipaddrport(this, 'usb_webremote2_x')" onKeyPress="return is_ipaddrport(this)" onKeyUp="change_ipaddrport(this)"></td>
</tr>
<tr>
<td class="content_header_td">遠端位置3：
           </td><td class="content_input_td"><input type="text" maxlength="21" class="content_input_fd" size="15" name="usb_webremote3_x" value="<% nvram_get_x("PrinterStatus","usb_webremote3_x"); %>" onBlur="return validate_ipaddrport(this, 'usb_webremote3_x')" onKeyPress="return is_ipaddrport(this)" onKeyUp="change_ipaddrport(this)"></td>
</tr>
<tr>
<td class="content_header_td">遠端位置4：
           </td><td class="content_input_td"><input type="text" maxlength="21" class="content_input_fd" size="15" name="usb_webremote4_x" value="<% nvram_get_x("PrinterStatus","usb_webremote4_x"); %>" onBlur="return validate_ipaddrport(this, 'usb_webremote4_x')" onKeyPress="return is_ipaddrport(this)" onKeyUp="change_ipaddrport(this)"></td>
</tr>
<tr>
<td class="content_header_td">遠端位置5：
           </td><td class="content_input_td"><input type="text" maxlength="21" class="content_input_fd" size="15" name="usb_webremote5_x" value="<% nvram_get_x("PrinterStatus","usb_webremote5_x"); %>" onBlur="return validate_ipaddrport(this, 'usb_webremote5_x')" onKeyPress="return is_ipaddrport(this)" onKeyUp="change_ipaddrport(this)"></td>
</tr>
<tr>
<td class="content_header_td">遠端位置6：
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
