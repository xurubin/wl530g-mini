<head>
<title>ZVMODELVZ Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=big5">
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
<input type="hidden" name="wan_proto" value="<% nvram_get_x("Layer3Forwarding","wan_proto"); %>">
<input type="hidden" name="wan_route_x" value="<% nvram_get_x("IPConnection","wan_route_x"); %>">
<input type="hidden" name="wan_nat_x" value="<% nvram_get_x("IPConnection","wan_nat_x"); %>">

<tr>
<td>
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">系統設定 - 操作模式</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">
ZVMODELVZ支援二種可滿足不同需求的操作模式。請選擇能符合您現況的模式。
</td>
</tr>

<tr>
<td class="content_header_td_30"><input type="radio" name="x_Mode" value="V3"> 無線網路橋接器</td>
<td class="content_desc_td">
在「無線網路橋接器」模式中，乙太網路通訊埠及無線通訊裝置均裝設在同一區域網域（LAN）之中。在這裏不支援與WAN相關的功能。
<p></p>
從專業術語的角度解釋，無線網路橋接器模式是 - 取網路位址轉換（NAT），允許將ZVMODELVZ的一組區域網路通訊埠及無線通訊埠橋接在一起。
</td>
</tr>
<tr>
<td class="content_header_td_30"><input type="radio" name="x_Mode"> 家用閘道器</td>
<td class="content_desc_td">
在此種模式中，我們建議您僅可使用ZVMODELVZ的乙太網路通訊埠透過ADSL或Cable Modem來連結網際網路。同時，在您的環境中許多人都可分享同一IP來使用ISP。
<p></p>
從專業術語的角度解釋，閘道器模式是 - 啟用網路位址轉換（NAT），允許使用PPPoE、DHCP client或靜態IP來進行網際網路（WAN）連線作業。另外，也支援一些對家庭用戶相當有用的功能，像是通用隨插即用（UPnP）及動態DNS（DDNS）。
</td>
</tr>
<tr>
<td class="content_input_td_less" colspan="2" height="60">
  <p align="right">
  <input class="inputSubmit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" type="submit" value="套用" name="action" onClick="saveMode(this)">&nbsp;
  </p>
</td>
</tr></table>
</td>
</tr>
</table>
</form>
</body>