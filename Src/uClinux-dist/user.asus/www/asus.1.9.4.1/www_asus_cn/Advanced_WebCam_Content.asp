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
    	
<input type="hidden" name="current_page" value="Advanced_WebCam_Content.asp"><input type="hidden" name="next_page" value="SaveRestart.asp"><input type="hidden" name="next_host" value=""><input type="hidden" name="sid_list" value="PrinterStatus;"><input type="hidden" name="group_id" value=""><input type="hidden" name="modified" value="0"><input type="hidden" name="action_mode" value=""><input type="hidden" name="first_time" value=""><input type="hidden" name="action_script" value="">
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">USB应用程序 - 网络相机</td>
</tr>
<tr>
<td class="content_header_td">启用网络相机功能？
           </td><td class="content_input_td"><select name="usb_webenable_x" class="content_input_fd" onChange="return change_common(this, 'PrinterStatus', 'usb_webenable_x')"><option class="content_input_fd" value="0" <% nvram_match_x("PrinterStatus","usb_webenable_x", "0","selected"); %>>Disabled</option><option class="content_input_fd" value="1" <% nvram_match_x("PrinterStatus","usb_webenable_x", "1","selected"); %>>LAN Only</option><option class="content_input_fd" value="2" <% nvram_match_x("PrinterStatus","usb_webenable_x", "2","selected"); %>>LAN and WAN</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目指出了您所使用的网络相机模式。选择「ActiveX Only」可让您在IE平台上执行ActiveX用户端软件以取得最佳的图像品质。选择「ActiveX and Refresh」可让您在IE平台上执行ActiveX用户端软件并在其他浏览器上的检视基本图像。选择「Refresh Only」可让您在所有的浏览器上检视基本图像。', LEFT);" onMouseOut="return nd();">网络相机模式：
           </td><td class="content_input_td"><select name="usb_webmode_x" class="content_input_fd" onChange="return change_common(this, 'PrinterStatus', 'usb_webmode_x')"><option class="content_input_fd" value="0" <% nvram_match_x("PrinterStatus","usb_webmode_x", "0","selected"); %>>ActiveX Only</option><option class="content_input_fd" value="1" <% nvram_match_x("PrinterStatus","usb_webmode_x", "1","selected"); %>>ActiveX and Refresh</option><option class="content_input_fd" value="2" <% nvram_match_x("PrinterStatus","usb_webmode_x", "2","selected"); %>>Refresh Only</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目为您网络相机所使用的驱动程序。', LEFT);" onMouseOut="return nd();">网络相机驱动程序：
           </td><td class="content_input_td"><select name="usb_webdriver_x" class="content_input_fd" onChange="return change_common(this, 'PrinterStatus', 'usb_webdriver_x')"><option class="content_input_fd" value="0" <% nvram_match_x("PrinterStatus","usb_webdriver_x", "0","selected"); %>>PWC 8.8</option><option class="content_input_fd" value="1" <% nvram_match_x("PrinterStatus","usb_webdriver_x", "1","selected"); %>>OV511 2.10</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目为您所希望的图像大小。', LEFT);" onMouseOut="return nd();">图像大小：
           </td><td class="content_input_td"><select name="usb_webimage_x" class="content_input_fd" onChange="return change_common(this, 'PrinterStatus', 'usb_webimage_x')"><option class="content_input_fd" value="0" <% nvram_match_x("PrinterStatus","usb_webimage_x", "0","selected"); %>>640 X 480</option><option class="content_input_fd" value="1" <% nvram_match_x("PrinterStatus","usb_webimage_x", "1","selected"); %>>320 X 240</option><option class="content_input_fd" value="2" <% nvram_match_x("PrinterStatus","usb_webimage_x", "2","selected"); %>>160 X 120</option></select><a href="javascript:openLink('x_WImageSize')" class="content_input_link" name="x_WImageSize_link">预览
             </a></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目指出了图像动作可被侦测的感应度（sense level）。', LEFT);" onMouseOut="return nd();">感应度：
           </td><td class="content_input_td"><select name="usb_websense_x" class="content_input_fd" onChange="return change_common(this, 'PrinterStatus', 'usb_websense_x')"><option class="content_input_fd" value="0" <% nvram_match_x("PrinterStatus","usb_websense_x", "0","selected"); %>>Low</option><option class="content_input_fd" value="1" <% nvram_match_x("PrinterStatus","usb_websense_x", "1","selected"); %>>Medium</option><option class="content_input_fd" value="2" <% nvram_match_x("PrinterStatus","usb_websense_x", "2","selected"); %>>High</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目说明了系统重新载入图像所需的时间间隔。数值的范围是1~65535。', LEFT);" onMouseOut="return nd();">更新时间：
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="usb_webfresh_x" class="content_input_fd" value="<% nvram_get_x("PrinterStatus", "usb_webfresh_x"); %>" onBlur="validate_range(this, 1, 65535)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目指出了在您的Webcam页面上所显示标题的字串。', LEFT);" onMouseOut="return nd();">标题字串：
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="usb_webcaption_x" value="<% nvram_get_x("PrinterStatus","usb_webcaption_x"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目指出了http服务器针对通讯状态进行监听的端口。', LEFT);" onMouseOut="return nd();">网络相机服务器端口：
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="usb_webhttpport_x" class="content_input_fd" value="<% nvram_get_x("PrinterStatus", "usb_webhttpport_x"); %>" onBlur="validate_range(this, 1024, 65535)" onKeyPress="return is_number(this)"><input type="checkbox" style="margin-left:30" name="usb_webhttpport_x_check" value="" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_webhttpport_x', '1')">Password Checking</input></td>
</tr>
<input type="hidden" name="usb_webhttpcheck_x" value="<% nvram_get_x("PrinterStatus","usb_webhttpcheck_x"); %>">
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目指出了服务器监听以ActiveX用户端软件进行通讯的端口。此栏目不能和网络相机服务器端口相同。', LEFT);" onMouseOut="return nd();">ActiveX端口：
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="usb_webactivex_x" class="content_input_fd" value="<% nvram_get_x("PrinterStatus", "usb_webactivex_x"); %>" onBlur="validate_range(this, 1024, 65535)" onKeyPress="return is_number(this)"></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">安全模式设置
            </td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">本项功能可让您通过网络相机来监控您的操作环境。如果测到任何动作，ZVMODELVZ会尝试以电子邮件来警告您。
         </td>
</tr>
<tr>
<td class="content_header_td">启用安全模式？
           </td><td class="content_input_td"><input type="radio" value="1" name="usb_websecurity_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_websecurity_x', '1')" <% nvram_match_x("PrinterStatus","usb_websecurity_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="usb_websecurity_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_websecurity_x', '0')" <% nvram_match_x("PrinterStatus","usb_websecurity_x", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目可指定「安全模式」将会启用的日期。', LEFT);" onMouseOut="return nd();">安全模式启用日期：
           </td><td class="content_input_td"><input type="hidden" maxlength="7" class="content_input_fd" size="7" name="usb_websecurity_date_x" value="<% nvram_get_x("PrinterStatus","usb_websecurity_date_x"); %>">
<p style="word-spacing: 0; margin-top: 0; margin-bottom: 0">
<input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Sun">Sun</input><input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Mon">Mon</input><input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Tue">Tue</input><input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Wed">Wed</input>
</p>
<input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Thu">Thu</input><input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Fri">Fri</input><input type="checkbox" class="content_input_fd" name="usb_websecurity_date_x_Sat">Sat</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目可指定「安全模式」将会启用的时间间隔。', LEFT);" onMouseOut="return nd();">安全模式启用时间：
           </td><td class="content_input_td"><input type="hidden" maxlength="11" class="content_input_fd" size="11" name="usb_websecurity_time_x" value="<% nvram_get_x("PrinterStatus","usb_websecurity_time_x"); %>"><input type="text" maxlength="2" class="content_input_fd" size="2" name="usb_websecurity_time_x_starthour" onKeyPress="return is_number(this)" onBlur="return validate_timerange(this, 0)">:
                <input type="text" maxlength="2" class="content_input_fd" size="2" name="usb_websecurity_time_x_startmin" onKeyPress="return is_number(this)" onBlur="return validate_timerange(this, 1)">-
                <input type="text" maxlength="2" class="content_input_fd" size="2" name="usb_websecurity_time_x_endhour" onKeyPress="return is_number(this)" onBlur="return validate_timerange(this, 2)">:
                <input type="text" maxlength="2" class="content_input_fd" size="2" name="usb_websecurity_time_x_endmin" onKeyPress="return is_number(this)" onBlur="return validate_timerange(this, 3)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目指出了您欲送达地点的电子邮件帐号。', LEFT);" onMouseOut="return nd();">传送至：
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="usb_websendto_x" value="<% nvram_get_x("PrinterStatus","usb_websendto_x"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目指出了您想要传送电子邮件目的电子邮件服务器。如果您将本栏目保持空白，ZVMODELVZ将会从下方的电子邮件帐号当中去搜索「邮件交换主机（Mail Exchanger）。', LEFT);" onMouseOut="return nd();">电子邮件服务器：
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="usb_webmserver_x" value="<% nvram_get_x("PrinterStatus","usb_webmserver_x"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目可让您编辑电子邮件的主题。', LEFT);" onMouseOut="return nd();">主题：
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="usb_websubject_x" value="<% nvram_get_x("PrinterStatus","usb_websubject_x"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目可让您将测到的图像文件附加到电子邮件当中。', LEFT);" onMouseOut="return nd();">附加图像文件？
           </td><td class="content_input_td"><input type="radio" value="1" name="usb_webattach_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_webattach_x', '1')" <% nvram_match_x("PrinterStatus","usb_webattach_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="usb_webattach_x" class="content_input_fd" onClick="return change_common_radio(this, 'PrinterStatus', 'usb_webattach_x', '0')" <% nvram_match_x("PrinterStatus","usb_webattach_x", "0", "checked"); %>>No</input></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">远端监控设置
            </td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">本功能可让您对您LAN网络当中最多六部的网络相机进行监控。请输入与网络相机连接的无线路由器的IP地址。如果想要将这网络相机在互联网上公开，请以手动方式为每个HTTP端口及ActiveX端口设置虚拟服务器。
         </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('选择「仅适用于LAN」，您只能在LAN操作环境内进行监控。如果您想从互联网上进行监控，请以手动方式对虚拟服务器列表的http端口及activex进行设置。请考虑安全问题。', LEFT);" onMouseOut="return nd();">远程监视模式：
           </td><td class="content_input_td"><select name="usb_webremote_x" class="content_input_fd" onChange="return change_common(this, 'PrinterStatus', 'usb_webremote_x')"><option class="content_input_fd" value="0" <% nvram_match_x("PrinterStatus","usb_webremote_x", "0","selected"); %>>LAN Only</option></select><a href="javascript:openLink('x_WRemote')" class="content_input_link" name="x_WRemote_link">预览
             </a></td>
</tr>
<tr>
<td class="content_header_td">远端位置1：
           </td><td class="content_input_td"><input type="text" maxlength="21" class="content_input_fd" size="15" name="usb_webremote1_x" value="<% nvram_get_x("PrinterStatus","usb_webremote1_x"); %>" onBlur="return validate_ipaddrport(this, 'usb_webremote1_x')" onKeyPress="return is_ipaddrport(this)" onKeyUp="change_ipaddrport(this)"></td>
</tr>
<tr>
<td class="content_header_td">远端位置2：
           </td><td class="content_input_td"><input type="text" maxlength="21" class="content_input_fd" size="15" name="usb_webremote2_x" value="<% nvram_get_x("PrinterStatus","usb_webremote2_x"); %>" onBlur="return validate_ipaddrport(this, 'usb_webremote2_x')" onKeyPress="return is_ipaddrport(this)" onKeyUp="change_ipaddrport(this)"></td>
</tr>
<tr>
<td class="content_header_td">远端位置3：
           </td><td class="content_input_td"><input type="text" maxlength="21" class="content_input_fd" size="15" name="usb_webremote3_x" value="<% nvram_get_x("PrinterStatus","usb_webremote3_x"); %>" onBlur="return validate_ipaddrport(this, 'usb_webremote3_x')" onKeyPress="return is_ipaddrport(this)" onKeyUp="change_ipaddrport(this)"></td>
</tr>
<tr>
<td class="content_header_td">远端位置4：
           </td><td class="content_input_td"><input type="text" maxlength="21" class="content_input_fd" size="15" name="usb_webremote4_x" value="<% nvram_get_x("PrinterStatus","usb_webremote4_x"); %>" onBlur="return validate_ipaddrport(this, 'usb_webremote4_x')" onKeyPress="return is_ipaddrport(this)" onKeyUp="change_ipaddrport(this)"></td>
</tr>
<tr>
<td class="content_header_td">远端位置5：
           </td><td class="content_input_td"><input type="text" maxlength="21" class="content_input_fd" size="15" name="usb_webremote5_x" value="<% nvram_get_x("PrinterStatus","usb_webremote5_x"); %>" onBlur="return validate_ipaddrport(this, 'usb_webremote5_x')" onKeyPress="return is_ipaddrport(this)" onKeyUp="change_ipaddrport(this)"></td>
</tr>
<tr>
<td class="content_header_td">远端位置6：
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
