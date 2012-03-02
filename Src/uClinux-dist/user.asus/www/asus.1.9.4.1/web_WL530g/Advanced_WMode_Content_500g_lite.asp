<head>
<title>WL530g Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=big5">
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
    	
<input type="hidden" name="current_page" value="Advanced_WMode_Content.asp"><input type="hidden" name="next_page" value="Advanced_ACL_Content.asp"><input type="hidden" name="next_host" value=""><input type="hidden" name="sid_list" value="WLANConfig11a;WLANConfig11b;"><input type="hidden" name="group_id" value=""><input type="hidden" name="modified" value="0"><input type="hidden" name="action_mode" value=""><input type="hidden" name="first_time" value=""><input type="hidden" name="action_script" value="">
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">無線 - 無線銜接</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">無線銜接（亦稱作「無線分散系統」或WDS）功能讓您可透過無線方式連接一或多個AP。
         </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('選擇AP時，僅會取消「無線銜接功能」。選擇WDS時，則僅會取消本地無線基地結合功能。選擇Hybrid時，可啟用無線橋接器功能同時無線基地台便與AP結合。', LEFT);" onMouseOut="return nd();">AP模式：
           </td><td class="content_input_td"><select name="wl_mode_x" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11b', 'wl_mode_x')"><option class="content_input_fd" value="0" <% nvram_match_x("WLANConfig11b","wl_mode_x", "0","selected"); %>>AP Only</option><option class="content_input_fd" value="1" <% nvram_match_x("WLANConfig11b","wl_mode_x", "1","selected"); %>>WDS Only</option><option class="content_input_fd" value="2" <% nvram_match_x("WLANConfig11b","wl_mode_x", "2","selected"); %>>Hybrid</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('選擇作業時的無線頻道', LEFT);" onMouseOut="return nd();">頻道：
           </td><td class="content_input_td"><select name="wl_channel" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11b', 'wl_channel')">   
<% select_channel("WLANConfig11b"); %>
                </select><input type="hidden" maxlength="15" size="15" name="x_RegulatoryDomain" value="<% nvram_get_x("Regulatory","x_RegulatoryDomain"); %>" readonly="1"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('選擇「Yes」時，可將WL530g連接至「遠端銜接點清單」中所列的其他AP。', LEFT);" onMouseOut="return nd();">連接「遠端銜接點清單」中的AP？
           </td><td class="content_input_td"><input type="radio" value="1" name="wl_wdsapply_x" class="content_input_fd" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_wdsapply_x', '1')" <% nvram_match_x("WLANConfig11b","wl_wdsapply_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="wl_wdsapply_x" class="content_input_fd" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_wdsapply_x', '0')" <% nvram_match_x("WLANConfig11b","wl_wdsapply_x", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('選擇「Yes」時，會取消無線銜接限制。任何的無線橋接器均可准予存取。選擇「No」時，會啟用無線橋接限制。僅有「遠端銜接點清單」（ Remote Bridge List）中所列的橋接器才准予存取。', LEFT);" onMouseOut="return nd();">允許匿名？
           </td><td class="content_input_td"><input type="radio" value="1" name="wl_lazywds" class="content_input_fd" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_lazywds', '1')" <% nvram_match_x("WLANConfig11b","wl_lazywds", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="wl_lazywds" class="content_input_fd" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_lazywds', '0')" <% nvram_match_x("WLANConfig11b","wl_lazywds", "0", "checked"); %>>No</input></td>
</tr>
</table>
</td>
</tr>
<tr>
<td>
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_list_header_tr">
<td class="content_list_header_td" width="60%" id="RBRList">遠端銜接點清單
         <input type="hidden" name="wl_wdsnum_x_0" value="<% nvram_get_x("WLANConfig11b", "wl_wdsnum_x"); %>" readonly="1"></td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'RBRList', 16, ' Add ');" name="RBRList" value="新增" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'RBRList', 16, ' Del ');" name="RBRList" value="刪除" size="12">
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
<td></td><td class="content_list_field_header_td" colspan="">MAC位址	                
           	        </td><td></td>
</tr>
<tr>
<td></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="12" class="content_input_list_fd" size="32" name="wl_wdslist_x_0" onKeyPress="return is_hwaddr()"></td>
</tr>
<tr>
<td></td><td colspan="10"><select size="8" name="RBRList_s" multiple="true" style="font-family: 'fixedsys'; font-size: '8pt'">
<% nvram_get_table_x("WLANConfig11b","RBRList"); %>
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
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="B0B0B0">
<tr bgcolor="#CCCCCC"><td colspan="3"><font face="arial" size="2"><b>&nbsp</b></font></td></tr>
<tr bgcolor="#FFFFFF">  
   <td id ="Confirm" height="25" width="34%">  
   <div align="center"><font face="Arial"> <input class=inputSubmit onMouseOut=buttonOut(this) onMouseOver="buttonOver(this)" onClick="onSubmitCtrl(this, ' Restore ')" type="submit" value=" 還原 " name="action"></font></div> 
   </td>  
   <td height="25" width="33%">  
   <div align="center"><font face="Arial"> <input class=inputSubmit onMouseOut=buttonOut(this) onMouseOver="buttonOver(this)" onClick="onSubmitCtrl(this, ' Finish ')" type="submit" value=" 儲存 " name="action"></font></div> 
   </td>
   <td height="25" width="33%">  
   <div align="center"><font face="Arial"> <input class=inputSubmit onMouseOut=buttonOut(this) onMouseOver="buttonOver(this)" onClick="onSubmitCtrl(this, ' Apply ')" type="submit" value=" 套用 " name="action"></font></div> 
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
    <td class="content_header_td_15" align="left">還原: </td>
    <td class="content_input_td_padding" align="left">清除上述設定值並還原有效設定值。</td>
</tr>
<tr bgcolor="#FFFFFF">
    <td class="content_header_td_15" align="left">儲存: </td>
    <td class="content_input_td_padding" align="left">現在確認所有設定值並重新啟動WL530g。</td>
</tr>
<tr bgcolor="#FFFFFF">
    <td class="content_header_td_15" align="left">套用: </td>
    <td class="content_input_td_padding" align="left">確認上述設定值並繼續。</td>
</tr>
</table>
</td>
</tr>

</table>
</form>
</body>
