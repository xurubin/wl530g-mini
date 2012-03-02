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
<form method="post" action="upgrade.cgi" enctype="multipart/form-data">
<!-- Table for the conntent page -->	    
<table width="600" border="0" cellpadding="0" cellspacing="0">     	      
    	
<tr>
<td>
<table width="600" border="0" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">System Setup - Upgrading the Firmware</td>
<input type="hidden" name="current_page" value="Basic_Operation_Content.asp">
<input type="hidden" name="next_page" value="Basic_SaveRestart.asp">
<input type="hidden" name="action_mode" value="">
</tr>
<tr class="content_section_header_tr">
<td id="Mode" class="content_section_header_td" colspan="2" width="600"> To upgrade the firmware, follow instructions listed below:</td>
</tr>

<tr>
<td class="content_desc_td" colspan="2" width="614">
              	<ol>
              		<li>
              <p style="margin-top: 7">
                    Check the website for a new version of the firmware.
              </li>
              		<li>
              <p style="margin-top: 7">
                    Download the new version to your local machine.               
              </li>
              		<li>
              <p style="margin-top: 7">
                    Specify the name and the path of the downloaded file in the "New Firmware File" field.
              </li>
              		<li>
              <p style="margin-top: 7">
                    Click "Upload" to upload the file to the WX-54G.  This should take approximately 10 seconds.
              </li>
              <li>
              <p style="margin-top: 7">
                    Once it has uploaded a compatible upgrade file, The WX-54G will automatically start the upgrade process.  Once the process is complete (it may take a few minutes) the system will reboot.
 	      </li>
              </ol>
</td>
</tr>
<tr>
		<td class="content_header_td" width="30">Product ID:</td><td class="content_input_td" width="595"><input type="text" value="<% nvram_get_f("general.log","productid"); %>" readonly="1"></td>
</tr>
<tr>
		<td class="content_header_td" width="30">Firmware Version:</td><td class="content_input_td" width="595"><input type="text" value="<% nvram_get_f("general.log","firmver"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td" width="30">New Firmware File:</td><td class="content_input_td">  
                  <input class="inputSubmit" name="file" size="20" type="file" ></td>
</tr>
<tr>
<td class="content_header_td"></td><td class="content_input_td" width="595">
  <p align="left"><input class="inputSubmit" onClick="onSubmitCtrlOnly(this, 'Upload')" type="submit" value="Upload"> 
  </p>
</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2" width="614">
	             <b>Note:</b>
                     <ol>
                     	<li>Configuration parameter settings will be preserved during the upgrade process, assuming the parameter exists in both the old and new firmware.</li>
                     	<li>If the upgrade process fails, the WX-54G will automatically enters emergency mode, which will be indicated by the LED sequence.  Use the Firmware Restoration utility on the CD to do a system recovery.</li>
                     </ol>
</td>
</tr>
</table>
</td>
</tr>
</table>
</form>
</body>
