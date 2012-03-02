<head>
<title>WL530g Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<link rel="stylesheet" type="text/css" href="style.css" media="screen"></link>
<script language="JavaScript" type="text/javascript" src="overlib.js"></script>
<script language="JavaScript" type="text/javascript" src="general.js"></script>
<script type="text/javascript" src="slider/slider.js"></script>
<script type="text/javascript" src="slider/libcrossbrowser.js"></script>
<script type="text/javascript" src="slider/eventhandler.js"></script>
<script language="JavaScript" type="text/javascript">
<!--

function getObj(name)
{
  if (document.getElementById)
  {
  	this.obj = document.getElementById(name);
	this.style = document.getElementById(name).style;
  }
  else if (document.all)
  {
	this.obj = document.all[name];
	this.style = document.all[name].style;
  }
  else if (document.layers)
  {
   	this.obj = document.layers[name];
   	this.style = document.layers[name];
  }
}


function drawSliderBlock(){
	
	x = new getObj('sliderDiv');
	if ((document.all) || (document.getElementById)){
		x.obj.innerHTML = slider_table();
	}
	else{
		x.obj.document.write(slider_table());
		x.obj.document.close();
	}

	initSlider();
}

function loadSlider()
{	
	changePower();
	drawSliderBlock();
}


function slider_table()
{
	var result;
	var assignment_box;
	var disk_num=1;
	var num_disks_in_pane;
	var disk_array = ["A","B"];
	var space_array = ["100","100"];
	syncDisksInPool = true;
	maxSpaceAllowedInSync = 0.0;
	var isJbodPool = 1;
	
	result = "<table border=0 cellspacing=0 cellpadding=0><tr><td>";
	
	result += "<div id='sliderDiv" + disk_num + "'></div>\n";
		// the slider movement and gigabytes textbox value are kept in sync only if there is one disk per pane and it is a mirrored or striped pool

	//alert(stripe_count());
	//alert(mirror_count());
	result += "</td></tr></table>";
	return result;

}


 var max_slider_width = 18; // change it if needed

 function initSlider(){
 
 	var disk_num;
 	
 	sliderArray = new Array();
 	Bs_Objects = new Array(); // Have to initialize the internal array used by slider class as well
 	
 	var colorArray = new Array("red", "red","orange","green","magenta","yellow","turquoise");


	disk_num=1;
	
	//if (document.forms[0].regulation_domain.value == "30DE")
 	//	sliderArray[disk_num.toString(10)] = drawSlider(disk_num.toString(10),"sliderDiv" + disk_num, "red",1, 9, document.forms[0].wl_radio_power_x.value, 1, false); 
 	//else 
 	sliderArray[disk_num.toString(10)] = drawSlider(disk_num.toString(10),"sliderDiv" + disk_num, "red",1, 12, document.forms[0].wl_radio_power_x.value, 1, false); 
	return;
 
 }
 
 function drawSlider(sliderName,divName,color,minVal,maxVal,initVal,interval,isDisabled){
   
   if (arguments[0] != "")
   	mySlider = new Bs_Slider(sliderName);
   else
   	mySlider = new Bs_Slider();
   	
   mySlider.attachOnChange(bsSliderChange);
   mySlider.width         =  ( (maxVal / 2) * max_slider_width );
   //mySlider.width       = 121;
   mySlider.height        = 18;
   mySlider.minVal        = minVal;
   mySlider.maxVal        = maxVal;
   mySlider.valueInterval = interval;
   mySlider.arrowAmount   = interval;
   mySlider.valueDefault  = initVal;
   mySlider.imgDir   = 'slider/';
   //mySlider.setBackgroundImage('background.gif', 'no-repeat');
   mySlider.setBackgroundImage('background.gif', 'repeat');
   mySlider.setBackgroundImageLeft('background_left.gif', 7, 18);
   mySlider.setBackgroundImageRight('background_right.gif', 7, 18);
   mySlider.setSliderIcon('slider.gif', 13, 18);
   mySlider.setArrowIconLeft('arrowLeft.gif', 16, 16);
   mySlider.setArrowIconRight('arrowRight.gif', 16, 16);
   mySlider.useInputField = 0;
   //mySlider.styleValueFieldClass = 'sliderInput';
   mySlider.colorbar = new Object();
   mySlider.colorbar['color']           = color;
   mySlider.colorbar['height']          = 7;
   mySlider.colorbar['widthDifference'] = -14;
   mySlider.colorbar['offsetLeft']      = -3;
   mySlider.colorbar['offsetTop']       = 9;
   mySlider.setDisabled(isDisabled);
   mySlider.drawInto(divName);
   
   return mySlider;

 }
 
 function changePower()
 {
 	var bpower, gpower, power_index;
 	
 	power_index=document.forms[0].wl_radio_power_x.value;
      	
      	if (power_index==12)
      	{	
      		bpower=24;
   		gpower=20;      	
      	}
      	else if(power_index==11)
      	{	
      		bpower=22;
   		gpower=20;
      	}
      	else
      	{
   		bpower=eval(power_index)+10;
   		gpower=eval(power_index)+10;
   	}	
   		 
   	document.forms[0].power_index_1.value = "B: " + bpower + " dbm / G: " + gpower + " dbm";
 }
 
function bsSliderChange(sliderObj, val, newPos){       
   // if mirrored or striped and one disk per pane, make sure all disk sizes are made the same
   // update slider value and hidden field value here for the current field alone
   document.forms[0].wl_radio_power_x.value=sliderObj.getValue();
   changePower();   
}


//-->
</script>
</head>  
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>    
<body onLoad="load_body();loadSlider();" onunLoad="return unload_body();">
<form method="GET" name="form" action="apply.cgi">
<!-- Table for the conntent page -->	    
<table width="666" border="0" cellpadding="0" cellspacing="0">     	      
    	
<input type="hidden" name="current_page" value="Advanced_WMMADV_Content.asp"><input type="hidden" name="next_page" value="Advanced_WMMADV_Content.asp"><input type="hidden" name="next_host" value=""><input type="hidden" name="sid_list" value="WLANConfig11a;WLANConfig11b;"><input type="hidden" name="group_id" value=""><input type="hidden" name="modified" value="0"><input type="hidden" name="action_mode" value=""><input type="hidden" name="first_time" value=""><input type="hidden" name="action_script" value="">
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">Wireless - Advanced</td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('The Radio Power should be between 1 to 84. But default value is recommended.', LEFT);" onMouseOut="return nd();">Radio Power:
</td>


<td class="content_input_td">        
    <div id="sliderDiv" >

			<!--	<script>document.write(slider_table());</script> -->
    </div>
    <div id="sliderVal" align="right"><input type="text"  size="20" style="font-size:12px;" name="power_index_1" readonly/></div>
    <input type="hidden" maxlength="5" size="5" name="wl_radio_power_x" class="content_input_fd" value="<% nvram_get_x("WLANConfig11b", "wl_radio_power_x"); %>" onBlur="validate_range(this, 1, 84)" onKeyPress="return is_number(this)"></td>
</td>
    	  
    	  
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('Please input string as CWmin CWmax AIFSN TXOP(b) TXOP(a,g) Admin Forced.', LEFT);" onMouseOut="return nd();">WMM Background Parameters for AP:
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="wl_wme_ap_bk" value="<% nvram_get_x("WLANConfig11b","wl_wme_ap_bk"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('Please input string as CWmin CWmax AIFSN TXOP(b) TXOP(a,g) Admin Forced.', LEFT);" onMouseOut="return nd();">WMM Best-Effect Parameters for AP:
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="wl_wme_ap_be" value="<% nvram_get_x("WLANConfig11b","wl_wme_ap_be"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('Please input string as CWmin CWmax AIFSN TXOP(b) TXOP(a,g) Admin Forced.', LEFT);" onMouseOut="return nd();">WMM Video Parameters for AP:
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="wl_wme_ap_vi" value="<% nvram_get_x("WLANConfig11b","wl_wme_ap_vi"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('Please input string as CWmin CWmax AIFSN TXOP(b) TXOP(a,g) Admin Forced.', LEFT);" onMouseOut="return nd();">WMM Voice Parameters for AP:
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="wl_wme_ap_vo" value="<% nvram_get_x("WLANConfig11b","wl_wme_ap_vo"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('Please input string as CWmin CWmax AIFSN TXOP(b) TXOP(a,g) Admin Forced.', LEFT);" onMouseOut="return nd();">WMM Background Parameters for STA:
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="wl_wme_sta_bk" value="<% nvram_get_x("WLANConfig11b","wl_wme_sta_bk"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('Please input string as CWmin CWmax AIFSN TXOP(b) TXOP(a,g) Admin Forced.', LEFT);" onMouseOut="return nd();">WMM Best-Effect Parameters for STA:
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="wl_wme_sta_be" value="<% nvram_get_x("WLANConfig11b","wl_wme_sta_be"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('Please input string as CWmin CWmax AIFSN TXOP(b) TXOP(a,g) Admin Forced.', LEFT);" onMouseOut="return nd();">WMM Video Parameters for STA:
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="wl_wme_sta_vi" value="<% nvram_get_x("WLANConfig11b","wl_wme_sta_vi"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('Please input string as CWmin CWmax AIFSN TXOP(b) TXOP(a,g) Admin Forced.', LEFT);" onMouseOut="return nd();">WMM Voice Parameters for STA:
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="wl_wme_sta_vo" value="<% nvram_get_x("WLANConfig11b","wl_wme_sta_vo"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td">Enable WPA2 Preauthentication?
           </td><td class="content_input_td"><input type="radio" value="1" name="wl_preauth" class="content_input_fd" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_preauth', '1')" <% nvram_match_x("WLANConfig11b","wl_preauth", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="wl_preauth" class="content_input_fd" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_preauth', '0')" <% nvram_match_x("WLANConfig11b","wl_preauth", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td">Network Re-authentication Innterval:
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="wl_net_reauth" value="<% nvram_get_x("WLANConfig11b","wl_net_reauth"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
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
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="B0B0B0">
<tr bgcolor="#CCCCCC"><td colspan="3"><font face="arial" size="2"><b>&nbsp</b></font></td></tr>
<tr bgcolor="#FFFFFF">  
   <td id ="Confirm" height="25" width="34%">  
   <div align="center"><font face="Arial"> <input class=inputSubmit onMouseOut=buttonOut(this) onMouseOver="buttonOver(this)" onClick="onSubmitCtrl(this, ' Restore ')" type="submit" value=" Restore " name="action"></font></div> 
   </td>  
   <td height="25" width="33%">  
   <div align="center"><font face="Arial"> <input class=inputSubmit onMouseOut=buttonOut(this) onMouseOver="buttonOver(this)" onClick="onSubmitCtrl(this, ' Finish ')" type="submit" value=" Finish " name="action"></font></div> 
   </td>
   <td height="25" width="33%">  
   <div align="center"><font face="Arial"> <input class=inputSubmit onMouseOut=buttonOut(this) onMouseOver="buttonOver(this)" onClick="onSubmitCtrl(this, ' Apply ')" type="submit" value=" Apply " name="action"></font></div> 
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
    <td class="content_header_td_15" align="left">Restore: </td>
    <td class="content_input_td_padding" align="left">Clear the above settings and restore the settings in effect.</td>
</tr>
<tr bgcolor="#FFFFFF">
    <td class="content_header_td_15" align="left">Finish: </td>
    <td class="content_input_td_padding" align="left">Confirm all settings and restart WL530g now.</td>
</tr>
<tr bgcolor="#FFFFFF">
    <td class="content_header_td_15" align="left">Apply: </td>
    <td class="content_input_td_padding" align="left">Confirm above settings and continue.</td>
</tr>
</table>
</td>
</tr>

</table>
</form>
</body>
