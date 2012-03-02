<head>
<title>WL520g Web Manager</title>
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
    	
<input type="hidden" name="current_page" value="Advanced_DDNS_Content.asp"><input type="hidden" name="next_page" value="SaveRestart.asp"><input type="hidden" name="next_host" value=""><input type="hidden" name="sid_list" value="LANHostConfig;"><input type="hidden" name="group_id" value=""><input type="hidden" name="modified" value="0"><input type="hidden" name="action_mode" value=""><input type="hidden" name="first_time" value=""><input type="hidden" name="action_script" value="">
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">IP设置 - 其他</td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('如果您启用了UPnP，WL520g将会由系统自动发现，例如Windows XP。同时，UPnP可让这些系统自动设置WL520g状态以适用各种的互联网应用程序，例如线上游戏及视讯会议。', LEFT);" onMouseOut="return nd();">启用UPnP？
           </td><td class="content_input_td"><input type="radio" value="1" name="upnp_enable" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'upnp_enable', '1')" <% nvram_match_x("LANHostConfig","upnp_enable", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="upnp_enable" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'upnp_enable', '0')" <% nvram_match_x("LANHostConfig","upnp_enable", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本项功能可让您指定远端服务器来记录与WL520g有关的记录信息。如果您空着不填，系统仅会在WL520g上记载最多为1024道的信息。', LEFT);" onMouseOut="return nd();">远端记录服务器（Remote Log Server）：
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="log_ipaddr" value="<% nvram_get_x("LANHostConfig","log_ipaddr"); %>" onBlur="return validate_ipaddr(this, 'log_ipaddr')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目可指出您所在位置的时区。', LEFT);" onMouseOut="return nd();">时区：
           </td><td class="content_input_td"><select name="time_zone" class="content_input_fd" onChange="return change_common(this, 'LANHostConfig', 'time_zone')"><option class="content_input_fd" value="UCT11" <% nvram_match_x("LANHostConfig","time_zone", "UCT11","selected"); %>>(GMT-11:00) Midway Island, Samoa</option><option class="content_input_fd" value="UCT10" <% nvram_match_x("LANHostConfig","time_zone", "UCT10","selected"); %>>(GMT-10:00) Hawaii</option><option class="content_input_fd" value="NAST9NADT" <% nvram_match_x("LANHostConfig","time_zone", "NAST9NADT","selected"); %>>(GMT-09:00) Alaska</option><option class="content_input_fd" value="PST8PDT" <% nvram_match_x("LANHostConfig","time_zone", "PST8PDT","selected"); %>>(GMT-08:00) Pacific Time</option><option class="content_input_fd" value="MST7MDT" <% nvram_match_x("LANHostConfig","time_zone", "MST7MDT","selected"); %>>(GMT-07:00) Mountain Time</option><option class="content_input_fd" value="MST7" <% nvram_match_x("LANHostConfig","time_zone", "MST7","selected"); %>>(GMT-07:00) Arizona</option><option class="content_input_fd" value="CST6CDT" <% nvram_match_x("LANHostConfig","time_zone", "CST6CDT","selected"); %>>(GMT-06:00) Central Time</option><option class="content_input_fd" value="UCT6" <% nvram_match_x("LANHostConfig","time_zone", "UCT6","selected"); %>>(GMT-06:00) Middle America</option><option class="content_input_fd" value="UCT5" <% nvram_match_x("LANHostConfig","time_zone", "UCT5","selected"); %>>(GMT-05:00) Indiana East, Colombia</option><option class="content_input_fd" value="EST5EDT" <% nvram_match_x("LANHostConfig","time_zone", "EST5EDT","selected"); %>>(GMT-05:00) Eastern Time</option><option class="content_input_fd" value="AST4ADT" <% nvram_match_x("LANHostConfig","time_zone", "AST4ADT","selected"); %>>(GMT-04:00) Atlantic Time, Brazil West</option><option class="content_input_fd" value="UCT4" <% nvram_match_x("LANHostConfig","time_zone", "UCT4","selected"); %>>(GMT-04:00) Bolivia, Venezuela</option><option class="content_input_fd" value="UCT3" <% nvram_match_x("LANHostConfig","time_zone", "UCT3","selected"); %>>(GMT-03:00) Guyana</option><option class="content_input_fd" value="EBST3EBDT" <% nvram_match_x("LANHostConfig","time_zone", "EBST3EBDT","selected"); %>>(GMT-03:00) Brazil East, Greenland</option><option class="content_input_fd" value="NORO2" <% nvram_match_x("LANHostConfig","time_zone", "NORO2","selected"); %>>(GMT-02:00) Mid-Atlantic</option><option class="content_input_fd" value="UCT0" <% nvram_match_x("LANHostConfig","time_zone", "UCT0","selected"); %>>(GMT) Gambia, Liberia, Morocco</option><option class="content_input_fd" value="GMT0BST" <% nvram_match_x("LANHostConfig","time_zone", "GMT0BST","selected"); %>>(GMT) England</option><option class="content_input_fd" value="MET-1METDST" <% nvram_match_x("LANHostConfig","time_zone", "MET-1METDST","selected"); %>>(GMT+01:00) Netherland, France, Italy</option><option class="content_input_fd" value="MEZ-1MESZ" <% nvram_match_x("LANHostConfig","time_zone", "MEZ-1MESZ","selected"); %>>(GMT+01:00) Germany</option><option class="content_input_fd" value="UCT-1" <% nvram_match_x("LANHostConfig","time_zone", "UCT-1","selected"); %>>(GMT+01:00) Tunisia</option><option class="content_input_fd" value="EET-2EETDST" <% nvram_match_x("LANHostConfig","time_zone", "EET-2EETDST","selected"); %>>(GMT+02:00) Greece, Ukraine, Turkey</option><option class="content_input_fd" value="SAST-2" <% nvram_match_x("LANHostConfig","time_zone", "SAST-2","selected"); %>>(GMT+02:00) South Africa</option><option class="content_input_fd" value="IST-3IDT" <% nvram_match_x("LANHostConfig","time_zone", "IST-3IDT","selected"); %>>(GMT+03:00) Iraq, Jordan, Kuwait</option><option class="content_input_fd" value="MSK-3MSD" <% nvram_match_x("LANHostConfig","time_zone", "MSK-3MSD","selected"); %>>(GMT+03:00) Moscow Winter Time</option><option class="content_input_fd" value="UCT-4" <% nvram_match_x("LANHostConfig","time_zone", "UCT-4","selected"); %>>(GMT+04:00) Armenia</option><option class="content_input_fd" value="UCT-5" <% nvram_match_x("LANHostConfig","time_zone", "UCT-5","selected"); %>>(GMT+05:00) Pakistan, Russia</option><option class="content_input_fd" value="UCT-6" <% nvram_match_x("LANHostConfig","time_zone", "UCT-6","selected"); %>>(GMT+06:00) Bangladesh, Russia</option><option class="content_input_fd" value="UCT-7" <% nvram_match_x("LANHostConfig","time_zone", "UCT-7","selected"); %>>(GMT+07:00) Thailand, Russia</option><option class="content_input_fd" value="CST-8" <% nvram_match_x("LANHostConfig","time_zone", "CST-8","selected"); %>>(GMT+08:00) China Coast, Hong Kong</option><option class="content_input_fd" value="CCT-8" <% nvram_match_x("LANHostConfig","time_zone", "CCT-8","selected"); %>>(GMT+08:00) Taipei</option><option class="content_input_fd" value="SST-8" <% nvram_match_x("LANHostConfig","time_zone", "SST-8","selected"); %>>(GMT+08:00) Singapore</option><option class="content_input_fd" value="WAS-8WAD" <% nvram_match_x("LANHostConfig","time_zone", "WAS-8WAD","selected"); %>>(GMT+08:00) Australia West</option><option class="content_input_fd" value="JST-9" <% nvram_match_x("LANHostConfig","time_zone", "JST-9","selected"); %>>(GMT+09:00) Japan, Korea</option><option class="content_input_fd" value="KST-9KDT" <% nvram_match_x("LANHostConfig","time_zone", "KST-9KDT","selected"); %>>(GMT+09:00) Korean</option><option class="content_input_fd" value="UCT-10" <% nvram_match_x("LANHostConfig","time_zone", "UCT-10","selected"); %>>(GMT+10:00) Guam, Russia</option><option class="content_input_fd" value="EST-10EDT" <% nvram_match_x("LANHostConfig","time_zone", "EST-10EDT","selected"); %>>(GMT+10:00) Australia</option><option class="content_input_fd" value="UCT-11" <% nvram_match_x("LANHostConfig","time_zone", "UCT-11","selected"); %>>(GMT+11:00) Solomon Islands</option><option class="content_input_fd" value="UCT-12" <% nvram_match_x("LANHostConfig","time_zone", "UCT-12","selected"); %>>(GMT+12:00) Fiji</option><option class="content_input_fd" value="NZST-12NZDT" <% nvram_match_x("LANHostConfig","time_zone", "NZST-12NZDT","selected"); %>>(GMT+12:00) New Zealand</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('系统时间同步用的NTP服务器。', LEFT);" onMouseOut="return nd();">NTP服务器
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="ntp_server0" value="<% nvram_get_x("LANHostConfig","ntp_server0"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"><a href="javascript:openLink('x_NTPServer1')" class="content_input_link" name="x_NTPServer1_link">NTP连接
             </a></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">DDNS设置：
            </td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">动态DNS（DDNS）让您即使在没有静态IP地址的情况下，仍可将服务器连同特有名称一并汇出至互联网上。目前，在WL520g之中已有数台嵌入的DDNS用户端。您可点选下方的「免费试用」，来启用一个免费试用帐号。
         </td>
</tr>
<tr>
<td class="content_header_td">启用DDNS 用户端？
           </td><td class="content_input_td"><input type="radio" value="1" name="ddns_enable_x" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'ddns_enable_x', '1')" <% nvram_match_x("LANHostConfig","ddns_enable_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="ddns_enable_x" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'ddns_enable_x', '0')" <% nvram_match_x("LANHostConfig","ddns_enable_x", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td">服务器：
           </td><td class="content_input_td"><select name="ddns_server_x" class="content_input_fd" onChange="return change_common(this, 'LANHostConfig', 'ddns_server_x')"><option class="content_input_fd" value="WWW.DYNDNS.ORG" <% nvram_match_x("LANHostConfig","ddns_server_x", "WWW.DYNDNS.ORG","selected"); %>>WWW.DYNDNS.ORG</option><option class="content_input_fd" value="WWW.DYNDNS.ORG(CUSTOM)" <% nvram_match_x("LANHostConfig","ddns_server_x", "WWW.DYNDNS.ORG(CUSTOM)","selected"); %>>WWW.DYNDNS.ORG(CUSTOM)</option><option class="content_input_fd" value="WWW.DYNDNS.ORG(STATIC)" <% nvram_match_x("LANHostConfig","ddns_server_x", "WWW.DYNDNS.ORG(STATIC)","selected"); %>>WWW.DYNDNS.ORG(STATIC)</option><option class="content_input_fd" value="WWW.TZO.COM" <% nvram_match_x("LANHostConfig","ddns_server_x", "WWW.TZO.COM","selected"); %>>WWW.TZO.COM</option><option class="content_input_fd" value="WWW.ZONEEDIT.COM" <% nvram_match_x("LANHostConfig","ddns_server_x", "WWW.ZONEEDIT.COM","selected"); %>>WWW.ZONEEDIT.COM</option></select><a href="javascript:openLink('x_DDNSServer')" class="content_input_link" name="x_DDNSServer_link">免费试用
             </a></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目可用来当作登入动态DNS（Dynamic-DNS）服务的身份识别。', LEFT);" onMouseOut="return nd();">用户名称或E-mail帐号：
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="ddns_username_x" value="<% nvram_get_x("LANHostConfig","ddns_username_x"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目可用来作为登入动态DNS服务的密码。', LEFT);" onMouseOut="return nd();">密码或DDNS密钥：
           </td><td class="content_input_td"><input type="password" maxlength="64" class="content_input_fd" size="32" name="ddns_passwd_x" value="<% nvram_get_x("LANHostConfig","ddns_passwd_x"); %>" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目代表了您在动态DNS服务中所登记并期待与世界共享的主机名称。', LEFT);" onMouseOut="return nd();">主机名称：
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="ddns_hostname_x" value="<% nvram_get_x("LANHostConfig","ddns_hostname_x"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目可决定含有通配符的域名是否可改成IP地址。', LEFT);" onMouseOut="return nd();">启用通配符（wildcard）？
           </td><td class="content_input_td"><input type="radio" value="1" name="ddns_wildcard_x" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'ddns_wildcard_x', '1')" <% nvram_match_x("LANHostConfig","ddns_wildcard_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="ddns_wildcard_x" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'ddns_wildcard_x', '0')" <% nvram_match_x("LANHostConfig","ddns_wildcard_x", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('此键可让您以手动方式更新DDNS数据库。但仅在DDNS自动更新失败时才能使用。您可以自「系统记录」（System Log）中取得目前DDNS更新的状态。', LEFT);" onMouseOut="return nd();">手动更新：
           </td><td class="content_input_td"><input type="hidden" maxlength="15" class="content_input_fd_ro" size="12" name="" value="<% nvram_get_f("ddns.log","DDNSStatus"); %>"><input type="submit" maxlength="15" class="content_input_fd_ro" onClick="return onSubmitApply('ddnsclient')" size="12" name="LANHostConfig_x_DDNSStatus_button" value="更新"></td>
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
    <td class="content_input_td_padding" align="left">现在确认所有设置并重新启动WL520g。</td>
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
