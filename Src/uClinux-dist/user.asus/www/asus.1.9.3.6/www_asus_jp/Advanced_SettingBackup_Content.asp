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
<body bgcolor="#FFFFFF">
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>    
<form method="post" action="upload.cgi" enctype="multipart/form-data">
<input type="hidden" name="action_mode" value="">
<!-- Table for the conntent page -->	    
<table width="660" border="0" cellpadding="1" cellspacing="0">     	      
    	
<tr>
<td>
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">システムセットアップ - 管理の設定</td>
<input type="hidden" name="current_page" value="Basic_Operation_Content.asp">
<input type="hidden" name="next_page" value="Basic_SaveRestart.asp">
</tr>
<tr>
<td class="content_desc_td" colspan="2" width="614">
                  <p style="margin-top: 7">この機能は、ZVMODELVZの現在の設定をファイルに保存したり、ファイルから設定をロードします。
</td>
</tr>

<tr class="content_section_header_tr">
<td id="Mode" class="content_section_header_td" colspan="2" width="614">ファイルに名前を付けて</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2" width="614">
<p style="margin-top: 7"> 
                 カーソルを移動します <a href="/ZVMODELVZ.CFG">HERE</a>. 次にマウスの右ボタンをクリックし、<b>"Save As..."</b>を選択してZVMODELVZの現在の設定をファイルに保存します。(注: 現在の設定をファイルに保存している間、同様にフラッシュに保存されます。)
<p style="margin-buttom: 7">                    
</td>
</tr>
<tr class="content_section_header_tr">
<td id="Mode" class="content_section_header_td" colspan="2" width="614">ファイルからロード</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2" width="614">                           		                                     		
              <p style="margin-top: 7">
              ダウンロードしたファイルのパスと名前を以下の<b>「新しい設定ファイル」</b>で指定します。次に<b>"アップロード"</b>をクリックしてファイルをZVMODELVZに書き込みます。プロセスを終了してからシステムが再起動するまで、少し時間がかかります。            
</td>
</tr>
<tr>
<td class="content_header_td" width="30">新しい設定ファイル:</td><td class="content_input_td">   
                  <input class="inputSubmit" name="file" size="20" type="file" ></td>
</tr>
<tr>
<td class="content_header_td"></td><td class="content_input_td" width="595">
  <p align="left"><input class="inputSubmit" type="submit" onClick="onSubmitCtrlOnly(this, 'Upload')" value="アップロード">
  </p>
</td>
</tr>
</table>
</td>
</tr>
</table>
</form>
</body>
