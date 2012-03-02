<head>
<title>WX54G Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
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
<table width="600" border="0" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">System Setup - Loading or Saving Settings To or From a File</td>
<input type="hidden" name="current_page" value="Basic_Operation_Content.asp">
<input type="hidden" name="next_page" value="Basic_SaveRestart.asp">
</tr>
<tr>
<td class="content_desc_td" colspan="2" width="614">
                  <p style="margin-top: 7">Use this screen to save the current settings of the WX-54G to a file, or to load settings from a file.
</td>
</tr>

<tr class="content_section_header_tr">
<td id="Mode" class="content_section_header_td" colspan="2" width="614">Save As a File</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2" width="614">
<p style="margin-top: 7"> 
                 To save your settings to a file, move your cursor over <a href="/WX54G.CFG">HERE</a>, right-click your mouse and select <b>"Save As..."</b> to save the current settings of the WX-54G to a file. Select a meaningful file name. <b>Note</b>: When you save your current settings to a file, they are saved in flash memory as well.
<p style="margin-buttom: 7">                    
</td>
</tr>
<tr class="content_section_header_tr">
<td id="Mode" class="content_section_header_td" colspan="2" width="614">Load From a File</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2" width="614">                           		                                     		
              <p style="margin-top: 7">
              To load settings from a file, specify the path and name of the downloaded file in the "New Setting File" box..  Then, click <b>Upload</b> to write the file to the WX-54G.   After a few minutes, the system will reboot.            
</td>
</tr>
<tr>
<td class="content_header_td" width="30">New Setting File:</td><td class="content_input_td">   
                  <input class="inputSubmit" name="file" size="20" type="file" ></td>
</tr>
<tr>
<td class="content_header_td"></td><td class="content_input_td" width="595">
  <p align="left"><input class="inputSubmit" type="submit" onClick="onSubmitCtrlOnly(this, 'Upload')" value="Upload">
  </p>
</td>
</tr>
</table>
</td>
</tr>
</table>
</form>
</body>
