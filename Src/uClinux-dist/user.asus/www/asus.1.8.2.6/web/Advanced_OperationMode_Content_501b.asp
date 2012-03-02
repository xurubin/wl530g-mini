<head>
<title>ZVMODELVZ Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<link rel="stylesheet" type="text/css" href="style.css" media="screen">
<script language="JavaScript" type="text/javascript" src="overlib.js"></script>
<script language="JavaScript" type="text/javascript" src="general.js"></script>
<script language="JavaScript" type="text/javascript" src="quick.js"></script>
</head>
<body bgcolor="#FFFFFF" onLoad="loadMode()">
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>
<form method="GET" name="form" action="apply.cgi">
<!-- Table for the conntent page -->
<table width="660" border="0" cellpadding="0" cellspacing="0">
<input type="hidden" name="current_page" value="Advanced_OperationMode_Content.asp">
<input type="hidden" name="next_page" value="Basic_SaveRestart.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="sid_list" value="Layer3Forwarding;IPConnection;">
<input type="hidden" name="Layer3Forwarding_x_ConnectionType" value="<% nvram_get_x("Layer3Forwarding","x_ConnectionType"); %>">
<input type="hidden" name="IPConnection_ConnectionType" value="<% nvram_get_x("IPConnection","ConnectionType"); %>">
<input type="hidden" name="IPConnection_NATEnabled" value="<% nvram_get_x("IPConnection","NATEnabled"); %>">
<tr>
<td>
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">系统设置 - 操作模式</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">
ZVMODELVZ支持三种可满足不同需求的操作模式。请选择符合您现状的模式。
</td>
</tr>
<tr>
<td class="content_header_td_30"><input type="radio" name="x_Mode"> 家用网关</td>
<td class="content_desc_td">
在此种模式中，我们建议您使用ZVMODELVZ通过ADSL或Cable Modem来连接互联网。同时，在您的环境中许多人都可共享同一IP来使用ISP。
<p></p>
从专业术语的角度解释，网关模式是 -启用网络地址转换（Network Address Translation，NAT），允许使用PPPoE、DHCP 用户端或静态IP来进行广域网（WAN）连接操作。另外，也支持一些对家庭用户相当有用的功能，像是即插即用（UPnP）及动态DNS（DDNS）。
</td>
</tr>
<tr>
<td class="content_header_td_30"><input type="radio" name="x_Mode" value="V3"> 接入点（AP）</td>
<td class="content_desc_td">
在「接入点 （AP）」模式中，五组以太网络端口（Ethernet ports）及无线通讯设备全部设置在同一局域网络（LAN）中。在这里不支持与WAN相关的功能。
<p></p>
从专业术语的角度解释，AP接入点模式是 - 关闭网络地址转换（NAT），允许将ZVMODELVZ的一组互联网端口（WAN port）与四组局域网端口（LAN port）桥接在一起。
</td>
</tr>
<tr>
<td class="content_input_td_less" colspan="2" height="60">
  <p align="right">
  <input class="inputSubmit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" type="submit" value="应用" name="action" onClick="saveMode(this)">&nbsp;
  </p>
</td>
</tr></table>
</td>
</tr>
</table>
</form>
</body>