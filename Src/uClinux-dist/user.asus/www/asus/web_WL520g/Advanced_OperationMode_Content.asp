<head>
<title>WL520g Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=euc-kr">
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
<td class="content_header_td_title" colspan="2">시스템 설정 - 작동모드</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">
WL520g는 그룹 구성원들의 요구사항을 충족시키기 위하여 3가지의 작동모드를 지원합니다. 상황에 맞는 모드를 선택하십시오.
</td>
</tr>
<tr>
<td class="content_header_td_30"><input type="radio" name="x_Mode"> 홈 게이트웨이</td>
<td class="content_desc_td">
본 모드에서는 ADSL 또는 케이블 모뎀을 통한 인터넷 연결을 위하여 사용자가 WL520g를 사용하고 있고 ISP와 동일한 IP 환경을 공유한 사람들이 많이 있는 것으로 가정합니다.
<p></p>
전문적인 용어로 설명하면 게이트웨이 모드는 NAT가 활성화 되고 PPPoE나 DHCP 클라이언트 또는 고정IP를 사용하여 WAN 연결이 됩니다. 그리고 UPnP와 DDNS와 같이 홈 사용자들에게 유용한 몇 가지의 특징들이 지원됩니다.
</td>
</tr>
<tr>
<td class="content_header_td_30"><input type="radio" name="x_Mode" value=""> 라우터</td>
<td class="content_desc_td">
라우터 모드에서는 회사와의 LAN 연결을 위하여 WL520g를 사용하고 있는 것으로 가정합니다. 따라서 사무실에서 사용자의 요구사항을 충족시킬 수 있는 라우팅 프로토콜을 설정할 수 있습니다.
<p></p>
전문적인 용어로 설명하면 라우터 모드에서는 NAT가 비활성화 되어 있고 고정 라우팅 프로토콜을 설정할 수 있습니다.
</td>
</tr>
<tr>
<td class="content_header_td_30"><input type="radio" name="x_Mode" value="V3"> 액세스 포인트</td>
<td class="content_desc_td">
액세스 포인트 모드에서는 5개의 모든 이더넷 포트와 무선 디바이스는 동일한 로컬 영역 네트워크에 설정할 수 있습니다. WAN 관련 기능은 지원하지 않습니다.
<p></p>
전문적인 용어로 설명하면 액세스 포인트 모드에서는 NAT가 비활성화 되어 있고 WL520g의 1개의 WAN 포트와 4개의 LAN 포트가 서로 연결됩니다.
</td>
</tr>
<tr>
<td class="content_input_td_less" colspan="2" height="60">
  <p align="right">
  <input class="inputSubmit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" type="submit" value="적용" name="action" onClick="saveMode(this)">&nbsp;
  </p>
</td>
</tr></table>
</td>
</tr>
</table>
</form>
</body>