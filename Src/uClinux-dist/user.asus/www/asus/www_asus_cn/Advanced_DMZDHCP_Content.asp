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
    	
<input type="hidden" name="current_page" value="Advanced_DMZDHCP_Content.asp"><input type="hidden" name="next_page" value="Advanced_DMZDWFilter_Content.asp"><input type="hidden" name="next_host" value=""><input type="hidden" name="sid_list" value="LANHostConfig;"><input type="hidden" name="group_id" value=""><input type="hidden" name="modified" value="0"><input type="hidden" name="action_mode" value=""><input type="hidden" name="first_time" value=""><input type="hidden" name="action_script" value="">
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">无线防火墙 - DHCP服务器</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">ZVMODELVZ在「无线防火墙」功能启用时，可支持多达253个IP地址供您的无线网络使用。一台本地设备的IP地址可由网络管理员手动指定或在DHCP服务器启用时，自动从ZVMODELVZ取得。
         </td>
</tr>
<tr>
<td class="content_header_td">启用DHCP服务器？
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', '', '1')" <% nvram_match_x("LANHostConfig","", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', '', '0')" <% nvram_match_x("LANHostConfig","", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目指出了提供给无线用户端向DHCP服务器要求IP地址的「域名」。', LEFT);" onMouseOut="return nd();">域名：
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get_x("LANHostConfig",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目指定了由您无线网络中的DHCP服务器所指定在地址池（pool）中的第一个地址。', LEFT);" onMouseOut="return nd();">IP地址池首地址：
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="" value="<% nvram_get_x("LANHostConfig",""); %>" onBlur="return validate_ipaddr(this, '')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目指定了由您无线网络中的DHCP服务器所指定在地址池（pool）中的最后一个地址。', LEFT);" onMouseOut="return nd();"> IP地址池末地址：
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="" value="<% nvram_get_x("LANHostConfig",""); %>" onBlur="return validate_ipaddr(this, '')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目指定了允许无线网络用户使用他们目前动态IP地址的全部连接时间。', LEFT);" onMouseOut="return nd();">租约时间：
           </td><td class="content_input_td"><input type="text" maxlength="10" class="content_input_fd" size="10" name="" value="<% nvram_get_x("LANHostConfig",""); %>" onBlur="validate_range(this, 0, 4294967295)" onKeyPress="return is_number(this)"></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">DNS及WINS服务器设置
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本栏目指定了提供无线用户端向DHCP服务器要求IP地址的域名系统（DNS）的IP地址。您可空着不填，那么DNS要求便会交由ZVMODELVZ处理。', LEFT);" onMouseOut="return nd();">DNS服务器1：
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="" value="<% nvram_get_x("LANHostConfig",""); %>" onBlur="return validate_ipaddr(this, '')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td">DNS服务器2：
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd_ro" size="15" name="" value="<% nvram_get_f("FirewallConfig","DmzIP"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('「Windows互联网命名服务」（Windows Internet Naming Service）可管理每台PC与互联网之间的相互联系。若需使用WINS服务器请在此处输入该服务器的IP地址。', LEFT);" onMouseOut="return nd();">WINS服务器：
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="" value="<% nvram_get_x("LANHostConfig",""); %>" onBlur="return validate_ipaddr(this, '')" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">手动指定IP地址
            </td>
</tr>
<tr>
<td class="content_header_td">启用手动指定功能？
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
<td class="content_list_header_td" width="60%" id="DmzManualDHCPList">手动指定的IP列表
         <input type="hidden" name="DmzManualDHCPCount_0" value="<% nvram_get_x("LANHostConfig", "DmzManualDHCPCount"); %>" readonly="1"></td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'DmzManualDHCPList', 8, ' Add ');" name="DmzManualDHCPList" value="增加" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'DmzManualDHCPList', 8, ' Del ');" name="DmzManualDHCPList" value="删除" size="12">
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
<td></td><td class="content_list_field_header_td" colspan="">MAC地址	                
           	        </td><td class="content_list_field_header_td" colspan="">IP地址	                
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
