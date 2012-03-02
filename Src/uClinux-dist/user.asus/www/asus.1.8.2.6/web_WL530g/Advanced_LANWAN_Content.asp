<head>
<title>WL530g Web Manager</title>
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
    	
<input type="hidden" name="current_page" value="Advanced_LANWAN_Content.asp"><input type="hidden" name="next_page" value="Advanced_DHCP_Content.asp"><input type="hidden" name="next_host" value=""><input type="hidden" name="sid_list" value="Layer3Forwarding;LANHostConfig;IPConnection;PPPConnection;"><input type="hidden" name="group_id" value=""><input type="hidden" name="modified" value="0"><input type="hidden" name="action_mode" value=""><input type="hidden" name="first_time" value=""><input type="hidden" name="action_script" value="">
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">IP设置 - 广域网 & 局域网（ WAN & LAN ）</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">WL530g可支持数种连接WAN的连接类型。这些类型可从WAN连接类型旁的下拉式选单中选取。设置栏目会根据您是选取那种连接类型而定。
         </td>
</tr>
<tr>
<td class="content_header_td">WAN连接类型：
           </td><td class="content_input_td"><select name="wan_proto" class="content_input_fd" onChange="return change_common(this, 'Layer3Forwarding', 'wan_proto')"><option class="content_input_fd" value="dhcp" <% nvram_match_x("Layer3Forwarding","wan_proto", "dhcp","selected"); %>>Automatic IP</option><option class="content_input_fd" value="static" <% nvram_match_x("Layer3Forwarding","wan_proto", "static","selected"); %>>Static IP</option><option class="content_input_fd" value="pppoe" <% nvram_match_x("Layer3Forwarding","wan_proto", "pppoe","selected"); %>>PPPoE</option><option class="content_input_fd" value="pptp" <% nvram_match_x("Layer3Forwarding","wan_proto", "pptp","selected"); %>>PPTP</option><option class="content_input_fd" value="bigpond" <% nvram_match_x("Layer3Forwarding","wan_proto", "bigpond","selected"); %>>BigPond</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('选择「停用」，会将WAN端口停用并将WL530g设成是一台AP。选择「启用」，会将WL530g设成是一台含有NAT功能的路由器。在开机期间将以太网络线接上WAN端口时，选择「自动」可将WL530g设成是一台路由器。', LEFT);" onMouseOut="return nd();">WAN模式：
           </td><td class="content_input_td"><select name="wan_mode_x" class="content_input_fd" onChange="return change_common(this, 'Layer3Forwarding', 'wan_mode_x')"><option class="content_input_fd" value="0" <% nvram_match_x("Layer3Forwarding","wan_mode_x", "0","selected"); %>>Disabled</option><option class="content_input_fd" value="1" <% nvram_match_x("Layer3Forwarding","wan_mode_x", "1","selected"); %>>Enabled</option><option class="content_input_fd" value="2" <% nvram_match_x("Layer3Forwarding","wan_mode_x", "2","selected"); %>>Auto</option></select></td>
</tr>
</table>
</td>
</tr>
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">广域网 IP设置
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('此WL530g的IP地址与您在远程网络上所见相同。如果您空着不填，WL530g会自动向DHCP服务器取得IP地址。', LEFT);" onMouseOut="return nd();">IP地址：
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="wan_ipaddr" value="<% nvram_get_x("IPConnection","wan_ipaddr"); %>" onBlur="return validate_ipaddr(this, 'wan_ipaddr')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('此一WL530g的IP地址与您在远程网络上所见相同。', LEFT);" onMouseOut="return nd();">子网掩码：
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="wan_netmask" value="<% nvram_get_x("IPConnection","wan_netmask"); %>" onBlur="return validate_ipaddr(this, 'wan_netmask')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('此为预设网关的IP地址，可使WL530g与远程网络或主机之间相互连系。', LEFT);" onMouseOut="return nd();">预设网关：
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="wan_gateway" value="<% nvram_get_x("IPConnection","wan_gateway"); %>" onBlur="return validate_ipaddr(this, 'wan_gateway')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">广域网 DNS设置
            </td>
</tr>
<tr>
<td class="content_header_td">自动接上DNS服务器？
           </td><td class="content_input_td"><input type="radio" value="1" name="wan_dnsenable_x" class="content_input_fd" onClick="return change_common_radio(this, 'IPConnection', 'wan_dnsenable_x', '1')" <% nvram_match_x("IPConnection","wan_dnsenable_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="wan_dnsenable_x" class="content_input_fd" onClick="return change_common_radio(this, 'IPConnection', 'wan_dnsenable_x', '0')" <% nvram_match_x("IPConnection","wan_dnsenable_x", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('此一栏目可显示WL530g所连接的DNS的IP地址。', LEFT);" onMouseOut="return nd();">DNS服务器1：
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="wan_dns1_x" value="<% nvram_get_x("IPConnection","wan_dns1_x"); %>" onBlur="return validate_ipaddr(this, 'wan_dns1_x')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('此一栏目可显示WL530g所连接的DNS的IP地址。', LEFT);" onMouseOut="return nd();">DNS服务器2：
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="wan_dns2_x" value="<% nvram_get_x("IPConnection","wan_dns2_x"); %>" onBlur="return validate_ipaddr(this, 'wan_dns2_x')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
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
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">PPPoE或PPTP帐号
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目只在您将广域网连接类型设置为PPPoE时适用', LEFT);" onMouseOut="return nd();">用户名：
           </td><td class="content_input_td"><input type="text" maxlength="64" class="content_input_fd" size="32" name="wan_pppoe_username" value="<% nvram_get_x("PPPConnection","wan_pppoe_username"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目仅在您将广域网连接类型设置为PPPoE时适用', LEFT);" onMouseOut="return nd();">密码：
           </td><td class="content_input_td"><input type="password" maxlength="64" class="content_input_fd" size="32" name="wan_pppoe_passwd" value="<% nvram_get_x("PPPConnection","wan_pppoe_passwd"); %>" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目可让您设置在一特定时间间隔之后终止您ISP连接的状态。数值0可允许无限延长间置时间。如果仅勾选Tx，那么来自互联网的资料将会跳过以继续计算间置的时间。', LEFT);" onMouseOut="return nd();">以秒计算的闲置断线时间（选项）：
           </td><td class="content_input_td"><input type="text" maxlength="10" class="content_input_fd" size="10" name="wan_pppoe_idletime" value="<% nvram_get_x("PPPConnection","wan_pppoe_idletime"); %>" onBlur="validate_range(this, 0, 4294967295)" onKeyPress="return is_number(this)"><input type="checkbox" style="margin-left:30" name="wan_pppoe_idletime_check" value="" onClick="return change_common_radio(this, 'PPPConnection', 'wan_pppoe_idletime', '1')">Tx Only</input></td>
</tr>
<input type="hidden" name="wan_pppoe_txonly_x" value="<% nvram_get_x("PPPConnection","wan_pppoe_txonly_x"); %>">
<tr>
<td class="content_header_td" onMouseOver="return overlib('所指为PPPoE封包的最大传输单位（MTU）。', LEFT);" onMouseOut="return nd();">最大传输单位（MTU）：
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="wan_pppoe_mtu" class="content_input_fd" value="<% nvram_get_x("PPPConnection", "wan_pppoe_mtu"); %>" onBlur="validate_range(this, 576, 1492)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('所指为PPPoE封包的最大接收单位（MRU）。', LEFT);" onMouseOut="return nd();">最大接收单位（MRU）：
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="wan_pppoe_mru" class="content_input_fd" value="<% nvram_get_x("PPPConnection", "wan_pppoe_mru"); %>" onBlur="validate_range(this, 576, 1492)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本项可由某些ISP指定。请向您的ISP确认并在需要时将填入。', LEFT);" onMouseOut="return nd();">网络服务名称（选项）：
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="wan_pppoe_service" value="<% nvram_get_x("PPPConnection","wan_pppoe_service"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本项可由某些ISP指定。请向您的ISP确认并在需要时将填入。', LEFT);" onMouseOut="return nd();">存取集中器（Access Concentrator）名称（选项）：
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="wan_pppoe_ac" value="<% nvram_get_x("PPPConnection","wan_pppoe_ac"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('启用PPPoE Relay可让局域网（LAN）当中的工作站设置可通过网络地址转换（NAT）服务器的个别PPPoE连接。', LEFT);" onMouseOut="return nd();">启用PPPoE Relay（PPPoE中继功能）？
           </td><td class="content_input_td"><input type="radio" value="1" name="wan_pppoe_relay_x" class="content_input_fd" onClick="return change_common_radio(this, 'PPPConnection', 'wan_pppoe_relay_x', '1')" <% nvram_match_x("PPPConnection","wan_pppoe_relay_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="wan_pppoe_relay_x" class="content_input_fd" onClick="return change_common_radio(this, 'PPPConnection', 'wan_pppoe_relay_x', '0')" <% nvram_match_x("PPPConnection","wan_pppoe_relay_x", "0", "checked"); %>>No</input></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">来自ISP的特别需求
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目可让您提供WL530g所使用的主机名称。通常是您ISP要求的名称。', LEFT);" onMouseOut="return nd();">主机名称：
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="wan_hostname" value="<% nvram_get_x("PPPConnection","wan_hostname"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目可让您提供WL530g连接互联网所使用的特有MAC（媒体访问控制）地址。通常是您ISP要求的地址。', LEFT);" onMouseOut="return nd();"> MAC（媒体访问控制）地址：
           </td><td class="content_input_td"><input type="text" maxlength="12" class="content_input_fd" size="12" name="wan_hwaddr_x" value="<% nvram_get_x("PPPConnection","wan_hwaddr_x"); %>" onBlur="return validate_hwaddr(this)" onKeyPress="return is_hwaddr()"></td>
</tr>
<tr>
<td class="content_header_td">认证服务器:
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="wan_heartbeat_x" value="<% nvram_get_x("PPPConnection","wan_heartbeat_x"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
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
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">LAN IP设置
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('此WL530g的IP地址与您在本地网络中所见相同。预设值是192.168.1.1。', LEFT);" onMouseOut="return nd();">IP地址：
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="lan_ipaddr" value="<% nvram_get_x("LANHostConfig","lan_ipaddr"); %>" onBlur="return validate_ipaddr(this, 'lan_ipaddr')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('此WL530g的子网掩码与您在本地网络中所见相同。预设值是255.255.255.0。', LEFT);" onMouseOut="return nd();">子网掩码：
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="lan_netmask" value="<% nvram_get_x("LANHostConfig","lan_netmask"); %>" onBlur="return validate_ipaddr(this, 'lan_netmask')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<input type="hidden" name="dhcp_start" value="<% nvram_get_x("LANHostConfig","dhcp_start"); %>"><input type="hidden" name="dhcp_end" value="<% nvram_get_x("LANHostConfig","dhcp_end"); %>">
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
    <td class="content_input_td_padding" align="left">现在确认所有设置并重新启动WL530g。</td>
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
