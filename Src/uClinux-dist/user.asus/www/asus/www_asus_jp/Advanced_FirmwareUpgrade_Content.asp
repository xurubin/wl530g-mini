<head>
<title>ZVMODELVZ Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=Shift_JIS">
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<meta http-equiv="Content-Type" content="text/html">
<link rel="stylesheet" type="text/css" href="style.css" media="screen">
<script language="JavaScript" type="text/javascript" src="overlib.js"></script>
<script language="JavaScript" type="text/javascript" src="general.js"></script>
<script language="JavaScript" type="text/javascript" src="quick.js"></script>
</head>  
<body bgcolor="#FFFFFF" onload="load_upload()">
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>    
<form method="post" action="upgrade.cgi" name="form" enctype="multipart/form-data">
<!-- Table for the conntent page -->	    
<table width="660" border="0" cellpadding="1" cellspacing="0">     	      
    	
<tr>
<td>
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">システムセットアップ - ファームウェアのアップグレード</td>
<input type="hidden" name="current_page" value="Basic_Operation_Content.asp">
<input type="hidden" name="next_page" value="Basic_SaveRestart.asp">
<input type="hidden" name="action_mode" value="">
</tr>
<tr class="content_section_header_tr">
<td id="Mode" class="content_section_header_td" colspan="2" width="614"> 以下にリストアップする指示に従ってください:</td>
</tr>

<tr>
<td class="content_desc_td" colspan="2" width="614">
              	<ol>
              		<li>
              <p style="margin-top: 7">
                    ファームウェアの新しいバージョンがASUSのWebサイトで使用できるかチェックします。
              </li>
              		<li>
              <p style="margin-top: 7">
                    適切なバージョンをローカルマシンにダウンロードします。               
              </li>
              		<li>
              <p style="margin-top: 7">
                    ダウンロードしたファイルのパスと名前を以下の「新しいファームウェアファイル」で指定します。
              </li>
              		<li>
              <p style="margin-top: 7">
                    「アップロード」をクリックして、ファイルをZVMODELVZにアップロードします。これには約80秒かかります。
              </li>
              <li>
              <p style="margin-top: 7">
                    正しいファームウェアファイルを受け取ったら、ZVMODELVZはアップグレードプロセスを自動的に開始します。プロセスを終了してからシステムが再起動するまで、少し時間がかかります。
 	      </li>
              </ol>
</td>
</tr>
<tr>
		<td class="content_header_td" width="30">製品ID:</td><td class="content_input_td" width="595"><input type="text" value="<% nvram_get_f("general.log","productid"); %>" readonly="1"></td>
</tr>
<tr>
		<td class="content_header_td" width="30">ファームウェアバージョン:</td><td class="content_input_td" width="595"><input type="text" value="<% nvram_get_f("general.log","firmver"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td" width="30">新しいファームウェアファイル:</td><td class="content_input_td">  
                  <input class="inputSubmit" name="file" size="20" type="file" ></td>
</tr>
<tr>
<td class="content_header_td"></td><td class="content_input_td" width="595">
  <p align="left"><input class="inputSubmit" name="button" onClick="onSubmitCtrlOnly(this, 'Upload1')" type="button" value="アップロード"> 
  </p>
</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2" width="614">
	             <b>注:</b>
                     <ol>
                     	<li>新旧両方のファームウェアにある設定パラメータにとって、その設定はアップグレードプロセスの間維持されます。</li>
                     	<li>アップグレードプロセスが失敗すると、ZVMODELVZは自動的に緊急モードに入ります。ZVMODELVZ前面にあるLED信号はそのような状況を示します。CDのファームウェア修復ユーティリティを使用して、システム修復を実行してください。</li>
                     </ol>
</td>
</tr>
</table>
</td>
</tr>
</table>
</form>
</body>
