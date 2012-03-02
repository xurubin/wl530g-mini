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
    	
<input type="hidden" name="current_page" value="Advanced_DynamicRoute_Content.asp"><input type="hidden" name="next_page" value="SaveRestart.asp"><input type="hidden" name="next_host" value=""><input type="hidden" name="sid_list" value="RouterConfig;"><input type="hidden" name="group_id" value=""><input type="hidden" name="modified" value="0"><input type="hidden" name="action_mode" value=""><input type="hidden" name="first_time" value=""><input type="hidden" name="action_script" value="">
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">路由器 - 動態路由</td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('RouterConfig_GWStaticRIP_itemdesc', LEFT);" onMouseOut="return nd();">啟用動態路由（RIP）？
           </td><td class="content_input_td"><select name="sr_rip_x" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', 'sr_rip_x')"><option class="content_input_fd" value="0" <% nvram_match_x("RouterConfig","sr_rip_x", "0","selected"); %>>Disabled</option><option class="content_input_fd" value="1" <% nvram_match_x("RouterConfig","sr_rip_x", "1","selected"); %>>LAN</option><option class="content_input_fd" value="2" <% nvram_match_x("RouterConfig","sr_rip_x", "2","selected"); %>>WAN</option><option class="content_input_fd" value="3" <% nvram_match_x("RouterConfig","sr_rip_x", "3","selected"); %>>BOTH</option></select></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">網際網路（WAN）
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('在WAN通訊埠上，會同時啟用或取消RIP封包的傳送及接收功能。', LEFT);" onMouseOut="return nd();">啟用網際網路（WAN）上的路由資訊協定（RIP）？
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'RouterConfig', '', '1')" <% nvram_match_x("RouterConfig","", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'RouterConfig', '', '0')" <% nvram_match_x("RouterConfig","", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄可啟用所選取的介面來傳送及接收內含RIP Version 1、RIP Version 2或兩者都有的封包。若兩種版本都有，則封包便會以廣播（broadcast）與多點傳播（multicast）同時進行。', LEFT);" onMouseOut="return nd();">RIP版本：
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="RIP1" <% nvram_match_x("RouterConfig","", "RIP1","selected"); %>>RIP1</option><option class="content_input_fd" value="RIP2" <% nvram_match_x("RouterConfig","", "RIP2","selected"); %>>RIP2</option><option class="content_input_fd" value="Both" <% nvram_match_x("RouterConfig","", "Both","selected"); %>>Both</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('可在split-horizon（水平分割）路由機制套用在通訊埠上時進行控制。', LEFT);" onMouseOut="return nd();">啟用Split-horizon？
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'RouterConfig', '', '1')" <% nvram_match_x("RouterConfig","", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'RouterConfig', '', '0')" <% nvram_match_x("RouterConfig","", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('選擇要套用在通訊埠上的驗證機制（authentication mechanism）及對應的金鑰。', LEFT);" onMouseOut="return nd();">驗證方式：
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="Disable" <% nvram_match_x("RouterConfig","", "Disable","selected"); %>>Disable</option><option class="content_input_fd" value="Text" <% nvram_match_x("RouterConfig","", "Text","selected"); %>>Text</option><option class="content_input_fd" value="MD5" <% nvram_match_x("RouterConfig","", "MD5","selected"); %>>MD5</option></select></td>
</tr>
<tr>
<td class="content_header_td">驗證金鑰（Authentication Key）：
           </td><td class="content_input_td"><input type="text" maxlength="16" class="content_input_fd" size="32" name="" value="<% nvram_get_x("RouterConfig",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">區域網路（LAN）
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('在LAN通訊埠上，會同時啟用或取消RIP封包的傳送及接收功能。', LEFT);" onMouseOut="return nd();">啟用在區域網路（LAN）上的路由資訊協定（RIP）？
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'RouterConfig', '', '1')" <% nvram_match_x("RouterConfig","", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'RouterConfig', '', '0')" <% nvram_match_x("RouterConfig","", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('本欄可啟用所選取的介面來傳送及接收內含RIP Version 1、RIP Version 2或兩者都有的封包。若兩種版本都有，則封包便會以廣播（broadcast）與多點傳播（multicast）同時進行。', LEFT);" onMouseOut="return nd();">RIP版本：
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="RIP1" <% nvram_match_x("RouterConfig","", "RIP1","selected"); %>>RIP1</option><option class="content_input_fd" value="RIP2" <% nvram_match_x("RouterConfig","", "RIP2","selected"); %>>RIP2</option><option class="content_input_fd" value="Both" <% nvram_match_x("RouterConfig","", "Both","selected"); %>>Both</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('可在split-horizon（水平分割）路由機制套用在通訊埠上時進行控制。', LEFT);" onMouseOut="return nd();">啟用Split-horizon？
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'RouterConfig', '', '1')" <% nvram_match_x("RouterConfig","", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'RouterConfig', '', '0')" <% nvram_match_x("RouterConfig","", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('選擇要套用在通訊埠上的驗證機制（authentication mechanism）及對應的金鑰。', LEFT);" onMouseOut="return nd();">驗證方式：
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="Disable" <% nvram_match_x("RouterConfig","", "Disable","selected"); %>>Disable</option><option class="content_input_fd" value="Text" <% nvram_match_x("RouterConfig","", "Text","selected"); %>>Text</option><option class="content_input_fd" value="MD5" <% nvram_match_x("RouterConfig","", "MD5","selected"); %>>MD5</option></select></td>
</tr>
<tr>
<td class="content_header_td">驗證金鑰（Authentication Key）：
           </td><td class="content_input_td"><input type="text" maxlength="16" class="content_input_fd" size="32" name="" value="<% nvram_get_x("RouterConfig",""); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">定時裝置
            </td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('每次更新定時秒數一到，RIP程序即會啟動並送出未經請求，內含前往所有相鄰RIP路由器的完整路由表的「回應」（Response）訊息。', LEFT);" onMouseOut="return nd();">更新時間：
           </td><td class="content_input_td"><input type="text" maxlength="5" class="content_input_fd" size="5" name="" value="<% nvram_get_x("RouterConfig",""); %>" onBlur="validate_range(this, 0, 65535)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('一旦已經逾時，該路由便不再具有效力；不過，在短時間內仍會保存在路由表內，讓相鄰的路由器可因此注意到該路由已遭丟棄。', LEFT);" onMouseOut="return nd();">逾時（Timeout）時間：
           </td><td class="content_input_td"><input type="text" maxlength="5" class="content_input_fd" size="5" name="" value="<% nvram_get_x("RouterConfig",""); %>" onBlur="validate_range(this, 0, 65535)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('一旦已過資源回收所定時間，該路由最後便會自路由表中移除。', LEFT);" onMouseOut="return nd();">資源回收（Garbage-collection）時間：
           </td><td class="content_input_td"><input type="text" maxlength="5" class="content_input_fd" size="5" name="" value="<% nvram_get_x("RouterConfig",""); %>" onBlur="validate_range(this, 0, 65535)" onKeyPress="return is_number(this)"></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">路由分配規則（Route Distribution Rules）
            </td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">這些規則可以用來過濾RIP的途徑。我們定出四種可代表WAN及LAN通訊埠的內傳或外傳資料的路由過濾程式。每一種類型，我們僅能設定一些在「路由分配過濾程式表」中指定準備接受處理或丟棄的合適網路。
         </td>
</tr>
<tr>
<td class="content_header_td">針對類型0的路由，僅有以下所指定的一些才會接收自WAN：
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="1" <% nvram_match_x("RouterConfig","", "1","selected"); %>>processed</option><option class="content_input_fd" value="0" <% nvram_match_x("RouterConfig","", "0","selected"); %>>dropped</option></select></td>
</tr>
<tr>
<td class="content_header_td">針對類型1的路由，僅有以下所指定的一些才會傳送至WAN：
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="1" <% nvram_match_x("RouterConfig","", "1","selected"); %>>processed</option><option class="content_input_fd" value="0" <% nvram_match_x("RouterConfig","", "0","selected"); %>>dropped</option></select></td>
</tr>
<tr>
<td class="content_header_td">針對類型2的路由，僅有以下所指定的一些才會接收自LAN：
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="1" <% nvram_match_x("RouterConfig","", "1","selected"); %>>processed</option><option class="content_input_fd" value="0" <% nvram_match_x("RouterConfig","", "0","selected"); %>>dropped</option></select></td>
</tr>
<tr>
<td class="content_header_td">針對類型3的路由，只有以下所指定的一些才會傳送至LAN：
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="1" <% nvram_match_x("RouterConfig","", "1","selected"); %>>processed</option><option class="content_input_fd" value="0" <% nvram_match_x("RouterConfig","", "0","selected"); %>>dropped</option></select></td>
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
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0"></table>
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
<td class="content_list_header_td" width="60%" id="RouteFilter">路由分配過濾
         <input type="hidden" name="RipDRouteCount_0" value="<% nvram_get_x("RouterConfig", "RipDRouteCount"); %>" readonly="1"></td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'RouteFilter', 16, ' Add ');" name="RouteFilter" value="新增" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'RouteFilter', 16, ' Del ');" name="RouteFilter" value="刪除" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="button" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return openHelp(this, 'RouterHelp');" name="RouteFilter" value="說明" size="12">
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
<td></td><td class="content_list_field_header_td" colspan="">路由類型	                
           	        </td><td class="content_list_field_header_td" colspan="">網路/主機IP	                
           	        </td><td class="content_list_field_header_td" colspan="">網路遮罩位元	                
           	        </td><td></td>
</tr>
<tr>
<td></td><td class="content_list_input_td" colspan=""><select name="_0" class="content_input_list_fd"><option value="0" <% nvram_match_list_x("RouterConfig","", "0","selected", 0); %>>0</option><option value="1" <% nvram_match_list_x("RouterConfig","", "1","selected", 0); %>>1</option><option value="2" <% nvram_match_list_x("RouterConfig","", "2","selected", 0); %>>2</option><option value="3" <% nvram_match_list_x("RouterConfig","", "3","selected", 0); %>>3</option></select></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="15" class="content_input_list_fd" size="15" name="_0" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="5" style="font-family: fixedsys; font-size: 10pt;" size="8" name="_0" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td></td><td colspan="10"><select size="8" name="RouteFilter_s" multiple="true" style="font-family: 'fixedsys'; font-size: '8pt'">
<% nvram_get_table_x("RouterConfig","RouteFilter"); %>
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
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">路由順序規則（Route Metric Rules）
            </td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">RIP順序是網路專用的一種距離值。通常在接收到網路資料時，RIP的順序才會增加。而重新分配的路由預設順序補償則設定為1。這些規則僅可用來針對「路由順序補償表」（Route Metric Offset）中所指定或排除的合適網路，變更順序補償。但其他網路的順序補償仍然設定為1。
         </td>
</tr>
<tr>
<td class="content_header_td">路由順序補償：
           </td><td class="content_input_td"><input type="text" maxlength="5" size="5" name="" class="content_input_fd" value="<% nvram_get_x("RouterConfig", ""); %>" onBlur="validate_range(this, 1, 16)" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td class="content_header_td">針對內傳路由，將順序補償設定為
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="0" <% nvram_match_x("RouterConfig","", "0","selected"); %>>none</option><option class="content_input_fd" value="1" <% nvram_match_x("RouterConfig","", "1","selected"); %>>routes specified below</option><option class="content_input_fd" value="2" <% nvram_match_x("RouterConfig","", "2","selected"); %>>routes not specified below</option></select></td>
</tr>
<tr>
<td class="content_header_td">針對外傳路由，將順序補償設定為
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'RouterConfig', '')"><option class="content_input_fd" value="0" <% nvram_match_x("RouterConfig","", "0","selected"); %>>none</option><option class="content_input_fd" value="1" <% nvram_match_x("RouterConfig","", "1","selected"); %>>routes specified below</option><option class="content_input_fd" value="2" <% nvram_match_x("RouterConfig","", "2","selected"); %>>routes not specified below</option></select></td>
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
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0"></table>
</td>
</tr>
<tr>
<td>
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="E0E0E0"></table>
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
<td class="content_list_header_td" width="60%" id="RipOffset">路由順序補償
         <input type="hidden" name="RipOffsetCount_0" value="<% nvram_get_x("RouterConfig", "RipOffsetCount"); %>" readonly="1"></td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'RipOffset', 16, ' Add ');" name="RipOffset" value="新增" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'RipOffset', 16, ' Del ');" name="RipOffset" value="刪除" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="button" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return openHelp(this, 'RouterHelp');" name="RipOffset" value="說明" size="12">
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
<td></td><td class="content_list_field_header_td" colspan="">方向	                
           	        </td><td class="content_list_field_header_td" colspan="">網路/主機IP	                
           	        </td><td class="content_list_field_header_td" colspan="">網路遮罩位元	                
           	        </td><td></td>
</tr>
<tr>
<td></td><td class="content_list_input_td" colspan=""><select name="_0" class="content_input_list_fd"><option value="IN" <% nvram_match_list_x("RouterConfig","", "IN","selected", 0); %>>IN</option><option value="OUT" <% nvram_match_list_x("RouterConfig","", "OUT","selected", 0); %>>OUT</option></select></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="15" class="content_input_list_fd" size="14" name="_0" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="5" style="font-family: fixedsys; font-size: 10pt;" size="8" name="_0" onKeyPress="return is_number(this)"></td>
</tr>
<tr>
<td></td><td colspan="10"><select size="4" name="RipOffset_s" multiple="true" style="font-family: 'fixedsys'; font-size: '8pt'">
<% nvram_get_table_x("RouterConfig","RipOffset"); %>
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
