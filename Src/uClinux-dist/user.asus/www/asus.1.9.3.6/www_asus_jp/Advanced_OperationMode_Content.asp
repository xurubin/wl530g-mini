<head>
<title>ZVMODELVZ Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=Shift_JIS">
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
<td class="content_header_td_title" colspan="2">システムセットアップ - 操作モード</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">
ZVMODELVZは3つの操作モードをサポートして、異なるグループのユーザーから異なる要件を満たします。状況に一致するモードを選択してください。
</td>
</tr>
<tr>
<td class="content_header_td_30"><input type="radio" name="x_Mode"> ホームゲートウェイ</td>
<td class="content_desc_td">
このモードで、ユーザーはZVMODELVZを使用し、ADSLまたはケーブルモデムを通してインターネットに接続するものと、また、お使いの環境にはIPとISPを共有する多くのユーザーがいるものと仮定します。
<p></p>
専門用語の説明をすると、ゲートウェイモードはNATが有効、WAN接続がPPPoE、またはDHCPクライアント、またはスタティックIPを使用して許可されています。さらに、UPnPやDDNSなどの家庭ユーザーに役に立ついくつかの機能が、サポートされています。
</td>
</tr>
<tr>
<td class="content_header_td_30"><input type="radio" name="x_Mode" value=""> ルータ</td>
<td class="content_desc_td">
ルータモードで、ユーザーはZVMODELVZを使用して企業でLANに接続しているものと仮定します。従って、経路指定プロトコルを設定してオフィスの要求を満たすことができます。
<p></p>
専門用語の説明をすると、ルータモードは、NATが無効、スタティックおよびダイナミック経路指定プロトコルが設定を許可され、WAN接続がスタティックIPを使用してのみ許可されています。
</td>
</tr>
<tr>
<td class="content_header_td_30"><input type="radio" name="x_Mode" value="V3"> アクセスポイント</td>
<td class="content_desc_td">
アクセスポイントモードで、5つのすべてのイーサネットポートとワイヤレスデバイスは同じローカルエリアネットワークに配置するように設定されます。機能に関連するこれらのWANはここではサポートされていません。
<p></p>
専門用語の説明をすると、アクセスポイントモードは、NATが無効、ZVMODELVZの1つのWANポートと4つのLANポートが互いにブリッジされています。
</td>
</tr>
<tr>
<td class="content_input_td_less" colspan="2" height="60">
  <p align="right">
  <input class="inputSubmit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" type="submit" value="適用" name="action" onClick="saveMode(this)">&nbsp;
  </p>
</td>
</tr></table>
</td>
</tr>
</table>
</form>
</body>