<!--
Copyright 2004, Broadcom Corporation
All Rights Reserved.

THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.

$Id$
-->

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html lang="en">
<head>
<title>Broadcom Home Gateway Reference Design: Wireless</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<link rel="stylesheet" type="text/css" href="style.css" media="screen">
<script language="JavaScript" type="text/javascript" src="overlib.js"></script>
<script language="JavaScript" type="text/javascript">
<!--
function wl_basic_rateset_change()
{
	var phytype = document.forms[0].wl_phytype[document.forms[0].wl_phytype.selectedIndex].value;
	var i, cur, defrs;
	
	/* Define new rateset */
	if (phytype == "a")
		defrs = new Array("Default", "All");
	else
		defrs = new Array("Default", "All", "12");

	/* Save current rateset */
	for (i = 0; i < document.forms[0].wl_rateset.length; i++) {
		if (document.forms[0].wl_rateset[i].selected) {
			cur = document.forms[0].wl_rateset[i].value;
			break;
		}
	}

	/* Reconstruct rateset array from new rateset */
	document.forms[0].wl_rateset.length = defrs.length;
	for (var i in defrs) {
		if (defrs[i].toLowerCase() == "12")
			document.forms[0].wl_rateset[i] = new Option("1 & 2 Mbps", defrs[i].toLowerCase());
		else
			document.forms[0].wl_rateset[i] = new Option(defrs[i], defrs[i].toLowerCase());
		document.forms[0].wl_rateset[i].value = defrs[i].toLowerCase();
		if (defrs[i].toLowerCase() == cur)
			document.forms[0].wl_rateset[i].selected = true;
	}
}

function wl_channel_change()
{
	var phytype = document.forms[0].wl_phytype[document.forms[0].wl_phytype.selectedIndex].value;
	var gmode = document.forms[0].wl_gmode[document.forms[0].wl_gmode.selectedIndex].value;
	var country = document.forms[0].wl_country_code[document.forms[0].wl_country_code.selectedIndex].value;
	var channel = document.forms[0].wl_channel[document.forms[0].wl_channel.selectedIndex].value;
	var i, cur, rates, rate;

	/* Save current rate */
	for (i = 0; i < document.forms[0].wl_rate.length; i++) {
		if (document.forms[0].wl_rate[i].selected) {
			cur = document.forms[0].wl_rate[i].value;
			break;
		}
	}

	/* Define new rates */
	if (phytype == "b") {
		if ("<% wl_radioid("b"); %>" != "BCM2050" && country == "Japan" && channel == 14)
			rates = new Array(0, 1, 2);
		else
			rates = new Array(0, 1, 2, 5.5, 11);
	} else if (phytype == "g") {
		/* Japan does not allow OFDM rates on channel 14 */
		if ((country == "Japan" && channel == 14) || gmode == "0")
			rates = new Array(0, 1, 2, 5.5, 11);
		else
			rates = new Array(0, 1, 2, 5.5, 6, 9, 11, 12, 18, 24, 36, 48, 54);
	} else if (phytype == "a") {
		rates = new Array(0, 6, 9, 12, 18, 24, 36, 48, 54);
	} else
		return;

	/* Reconstruct rate array from new rates */
	document.forms[0].wl_rate.length = rates.length;
	for (var i in rates) {
		rate = rates[i] * 1000000;
		if (rates[i] == 0)
			document.forms[0].wl_rate[i] = new Option("Auto", rate);
		else
			document.forms[0].wl_rate[i] = new Option(rates[i] + " Mbps", rate);
		document.forms[0].wl_rate[i].value = rate;
		if (rate == cur)
			document.forms[0].wl_rate[i].selected = true;
	}
}

function wl_country_list_change()
{
	var phytype = document.forms[0].wl_phytype[document.forms[0].wl_phytype.selectedIndex].value;
	var cur = 0;
	var sel = 0;

	/* Save current country */
	for (i = 0; i < document.forms[0].wl_country_code.length; i++) {
		if (document.forms[0].wl_country_code[i].selected) {
			cur = document.forms[0].wl_country_code[i].value;
			break;
		}
	}

	if (phytype == "a") {
<% wl_country_list("a"); %>
	} else {
<% wl_country_list("b"); %>
	}

	/* Reconstruct country_code array from new countries */
	document.forms[0].wl_country_code.length = countries.length;
	for (var i in countries) {
		document.forms[0].wl_country_code[i].value = countries[i];
		if (countries[i] == cur) {
			document.forms[0].wl_country_code[i].selected = true;
			sel = 1;
		}
	}

	if (sel == 0)
		document.forms[0].wl_country_code[0].selected = true;
}

function wl_channel_list_change()
{
	var phytype = document.forms[0].wl_phytype[document.forms[0].wl_phytype.selectedIndex].value;
	var country = document.forms[0].wl_country_code[document.forms[0].wl_country_code.selectedIndex].value;
	var channels = new Array(0);
	var cur = 0;
	var sel = 0;

	/* Save current channel */
	for (i = 0; i < document.forms[0].wl_channel.length; i++) {
		if (document.forms[0].wl_channel[i].selected) {
			cur = document.forms[0].wl_channel[i].value;
			break;
		}
	}

	if (phytype == "a") {
<% wl_channel_list("a"); %>
	} else {
<% wl_channel_list("b"); %>
	}

	/* Reconstruct channel array from new channels */
	document.forms[0].wl_channel.length = channels.length;
	for (var i in channels) {
		if (channels[i] == 0)
			document.forms[0].wl_channel[i] = new Option("Auto", channels[i]);
		else
			document.forms[0].wl_channel[i] = new Option(channels[i], channels[i]);
		document.forms[0].wl_channel[i].value = channels[i];
		if (channels[i] == cur) {
			document.forms[0].wl_channel[i].selected = true;
			sel = 1;
		}
	}

	if (sel == 0)
		document.forms[0].wl_channel[0].selected = true;
}

function wl_phytype_change()
{
	var phytype = document.forms[0].wl_phytype[document.forms[0].wl_phytype.selectedIndex].value;
	var gmode = document.forms[0].wl_gmode[document.forms[0].wl_gmode.selectedIndex].value;

	if (phytype == "g") {
		document.forms[0].wl_gmode.disabled = 0;
		document.forms[0].wl_gmode_protection.disabled = 0;
	} else {
		document.forms[0].wl_gmode.disabled = 1;
		document.forms[0].wl_gmode_protection.disabled = 1;
	}

	if (phytype == "b" || (phytype == "g" && gmode == "0"))
		document.forms[0].wl_plcphdr.disabled = 0;
	else
		document.forms[0].wl_plcphdr.disabled = 1;

	wl_basic_rateset_change();
}

function wl_mode_change()
{
/*
*/


}
function wl_gmode_options()
{
	var gmode = <% nvram_get("wl_gmode"); %>;
	var index;

	if (gmode == "2") {
		index = document.forms[0].wl_gmode.length;
		document.forms[0].wl_gmode[index] = new Option("54g Only", "2");
		document.forms[0].wl_gmode[index].selected = true;
	}
}
function wl_afterburner_options()
{
	var afterburner = <% wl_parse_str("cap", "afterburner"); %>;

	if (afterburner != "1")
		document.forms[0].wl_afterburner.disabled = 1;
}

function wl_recalc()
{
	wl_phytype_change();
	wl_country_list_change();
	wl_channel_list_change();
	wl_mode_change();
	wl_gmode_options();
	wl_afterburner_options()
}

//-->
</script>
</head>

<body onLoad="wl_recalc();">
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>

<table border="0" cellpadding="0" cellspacing="0" width="100%" bgcolor="#cc0000">
  <tr>
    <td><a href="index.asp"><img border="0" src="basic.gif" alt="Basic"></a></td>
    <td><a href="lan.asp"><img border="0" src="lan.gif" alt="LAN"></a></td>
    <td><a href="wan.asp"><img border="0" src="wan.gif" alt="WAN"></a></td>
    <td><a href="status.asp"><img border="0" src="status.gif" alt="Status"></a></td>
    <td><a href="filter.asp"><img border="0" src="filter.gif" alt="Filters"></a></td>
    <td><a href="forward.asp"><img border="0" src="forward.gif" alt="Routing"></a></td>
    <td><a href="wireless.asp"><img border="0" src="wireless.gif" alt="Wireless"></a></td>
    <td><a href="security.asp"><img border="0" src="security.gif" alt="Security"></a></td>
    <td><a href="firmware.asp"><img border="0" src="firmware.gif" alt="Firmware"></a></td>
<!--
-->	
    <td width="100%"></td>
  </tr>
</table>

<table border="0" cellpadding="0" cellspacing="0" width="100%">
  <tr>
    <td colspan="2" class="edge"><img border="0" src="blur_new.jpg" alt=""></td>
  </tr>
  <tr>
    <td><img border="0" src="logo_new.gif" alt=""></td>
    <td width="100%" valign="top">
	<br>
	<span class="title">WIRELESS</span><br>
	<span class="subtitle">This page allows you to configure the
	wireless LAN interfaces.</span>
    </td>
  </tr>
</table>

<form method="post" action="wireless.asp">
<input type="hidden" name="page" value="wireless.asp">

<p>
<table border="0" cellpadding="0" cellspacing="0">
  <tr>
    <th width="310"
	onMouseOver="return overlib('Selects which wireless interface to configure.', LEFT);"
	onMouseOut="return nd();">
	Wireless Interface:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_unit" onChange="submit();">
	  <% wl_list(); %>
	</select>
    </td>
    <td>
	<button type="submit" name="action" value="Select">Select</button>
    </td>
  </tr>
</table>

<p>
<table border="0" cellpadding="0" cellspacing="0">
  <tr>
    <th width="310"
	onMouseOver="return overlib('Sets the Network Name (also known as SSID) of this network.', LEFT);"
	onMouseOut="return nd();">
	Network Name (SSID):&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_ssid" value="<% nvram_get("wl_ssid"); %>" size="32" maxlength="32"></td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Selecting <b>Closed</b> hides the network from active scans. Selecting <b>Open</b> reveals the network to active scans.', LEFT);"
	onMouseOut="return nd();">
	Network Type:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_closed">
	  <option value="0" <% nvram_match("wl_closed", "0", "selected"); %>>Open</option>
	  <option value="1" <% nvram_match("wl_closed", "1", "selected"); %>>Closed</option>
	</select>
    </td>
  </tr>    
  <tr>
    <th width="310"
	onMouseOver="return overlib('Selecting <b>On</b> enables AP isolation mode.  When enabled, STAs associated to the AP will not be able to communicate with each other.', LEFT);"
	onMouseOut="return nd();">
	AP Isolation:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_ap_isolate">
	  <option value="0" <% nvram_match("wl_ap_isolate", "0", "selected"); %>>Off</option>
	  <option value="1" <% nvram_match("wl_ap_isolate", "1", "selected"); %>>On</option>
	</select>
    </td>
  </tr>    
  <tr>
    <th width="310"
	onMouseOver="return overlib('Restricts the channel set based on country requirements.', LEFT);"
	onMouseOut="return nd();">
	Country:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_country_code" onChange="wl_recalc();">
	  <option value="<% wl_cur_country(); %>" selected></option>
	</select>
	&nbsp;&nbsp;Current: <% wl_cur_country(); %>
    </td>
    <td></td>
  </tr>
</table>

<p>
<table border="0" cellpadding="0" cellspacing="0">
  <tr>
    <th width="310"
	onMouseOver="return overlib('Enables or disables the wireless radio.', LEFT);"
	onMouseOut="return nd();">
	Radio:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_radio">
	  <option value="0" <% nvram_match("wl_radio", "0", "selected"); %>>Disabled</option>
	  <option value="1" <% nvram_match("wl_radio", "1", "selected"); %>>Enabled</option>
	</select>
    </td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Selects the wireless radio band to use.', LEFT);"
	onMouseOut="return nd();">
	Band:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_phytype" onChange="wl_recalc();">
	  <% wl_phytypes(); %>
	</select>
	&nbsp;&nbsp;<% wl_cur_phytype(); %>
    </td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Selects a particular channel on which to operate.', LEFT);"
	onMouseOut="return nd();">
	Channel:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_channel" onChange="wl_channel_change();">
	  <option value="<% nvram_get("wl_channel"); %>" selected></option>
	</select>
	&nbsp;&nbsp;<% wl_cur_channel(); %>
    </td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Set the mode to 54g Auto for the widest compatibility. Set the mode to 54g Performance for the fastest performance among 54g certified equipment. Set the mode to 54g LRS if you are experiencing difficulty with legacy 802.11b equipment.', LEFT);"
	onMouseOut="return nd();">
	54g&#8482; Mode:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_gmode" onChange="wl_phytype_change();">
	  <option value="1" <% nvram_match("wl_gmode", "1", "selected"); %>>54g Auto</option>
	  <option value="4" <% nvram_match("wl_gmode", "4", "selected"); %>>54g Performance</option>
	  <option value="5" <% nvram_match("wl_gmode", "5", "selected"); %>>54g LRS</option>
	  <option value="0" <% nvram_match("wl_gmode", "0", "selected"); %>>802.11b Only</option>
	</select>
    </td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('In <b>Auto</b> mode the AP will use RTS/CTS to improve 802.11g performance in mixed 802.11g/802.11b networks. Turn protection <b>Off</b> to maximize 802.11g throughput under most conditions.', LEFT);"
	onMouseOut="return nd();">
	54g Protection:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_gmode_protection">
	  <option value="off" <% nvram_match("wl_gmode_protection", "off", "selected"); %>>Off</option>
	  <option value="auto" <% nvram_match("wl_gmode_protection", "auto", "selected"); %>>Auto</option>
	</select>
    </td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Forces the transmission rate for the AP to a particular speed.', LEFT);"
	onMouseOut="return nd();">
	Rate:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_rate">
	  <!-- All a/b/g rates -->
	  <option value="0" <% nvram_match("wl_rate", "0", "selected"); %>>Auto</option>
	  <option value="1000000" <% nvram_match("wl_rate", "1000000", "selected"); %>>1 Mbps</option>
	  <option value="2000000" <% nvram_match("wl_rate", "2000000", "selected"); %>>2 Mbps</option>
	  <option value="5500000" <% nvram_match("wl_rate", "5500000", "selected"); %>>5.5 Mbps</option>
	  <option value="6000000" <% nvram_match("wl_rate", "6000000", "selected"); %>>6 Mbps</option>
	  <option value="9000000" <% nvram_match("wl_rate", "9000000", "selected"); %>>9 Mbps</option>
	  <option value="11000000" <% nvram_match("wl_rate", "11000000", "selected"); %>>11 Mbps</option>      
	  <option value="12000000" <% nvram_match("wl_rate", "12000000", "selected"); %>>12 Mbps</option>
	  <option value="18000000" <% nvram_match("wl_rate", "18000000", "selected"); %>>18 Mbps</option>
	  <option value="24000000" <% nvram_match("wl_rate", "24000000", "selected"); %>>24 Mbps</option>
	  <option value="36000000" <% nvram_match("wl_rate", "36000000", "selected"); %>>36 Mbps</option>
	  <option value="48000000" <% nvram_match("wl_rate", "48000000", "selected"); %>>48 Mbps</option>
	  <option value="54000000" <% nvram_match("wl_rate", "54000000", "selected"); %>>54 Mbps</option>
	</select>
    </td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Selects the basic rates that wireless clients must support.', LEFT);"
	onMouseOut="return nd();">
	Basic Rate Set:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_rateset">
	  <option value="default" <% nvram_match("wl_rateset", "default", "selected"); %>>Default</option>
	  <option value="all" <% nvram_match("wl_rateset", "all", "selected"); %>>All</option>
	  <option value="12" <% nvram_match("wl_rateset", "12", "selected"); %>>1 & 2 Mbps</option>
	</select>
    </td>
  </tr>
<!--
-->	
</table>

<p>
<table border="0" cellpadding="0" cellspacing="0">
  <tr>
    <th width="310"
	onMouseOver="return overlib('Sets the fragmentation threshold.', LEFT);"
	onMouseOut="return nd();">
	Fragmentation Threshold:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_frag" value="<% nvram_get("wl_frag"); %>" size="4" maxlength="4"></td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Sets the RTS threshold.', LEFT);"
	onMouseOut="return nd();">
	RTS Threshold:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_rts" value="<% nvram_get("wl_rts"); %>" size="4" maxlength="4"></td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Sets the wakeup interval for clients in power-save mode.', LEFT);"
	onMouseOut="return nd();">
	DTIM Interval:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_dtim" value="<% nvram_get("wl_dtim"); %>" size="4" maxlength="4"></td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Sets the beacon interval for the AP.', LEFT);"
	onMouseOut="return nd();">
	Beacon Interval:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_bcn" value="<% nvram_get("wl_bcn"); %>" size="4" maxlength="4"></td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Sets whether short or long preambles are used. Short preambles improve throughput but all clients in the wireless network must support this capability if selected.', LEFT);"
	onMouseOut="return nd();">
	Preamble Type:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_plcphdr">
	  <option value="long" <% nvram_match("wl_plcphdr", "long", "selected"); %>>Long</option>
	  <option value="short" <% nvram_match("wl_plcphdr", "short", "selected"); %>>Short</option>
	</select>
    </td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Enable/Disable XPress mode', LEFT);"
	onMouseOut="return nd();">
	Enable XPress&#8482; Technology:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_frameburst">
	  <option value="off" <% nvram_match("wl_frameburst", "off", "selected"); %>>Off</option>
	  <option value="on" <% nvram_match("wl_frameburst", "on", "selected"); %>>On</option>
	</select>
    </td>
  </tr>
    <tr>
    <th width="310"
	onMouseOver="return overlib('Enable/Disable AfterBurner mode', LEFT);"
	onMouseOut="return nd();">
	Enable AfterBurner Technology:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_afterburner">
	  <option value="off" <% nvram_match("wl_afterburner", "off", "selected"); %>>Off</option>
	  <option value="auto" <% nvram_match("wl_afterburner", "auto", "selected"); %>>On</option>
	</select>
    </td>
  </tr>
</table>

<p>
<table border="0" cellpadding="0" cellspacing="0">
  <tr>
    <th width="310"
	onMouseOver="return overlib('Selecting <b>Wireless Bridge</b> disables access point functionality. Only wireless bridge (also known as Wireless Distribution System or WDS) functionality will be available. Selecting <b>Access Point</b> enables access point functionality. Wireless bridge functionality will still be available and wireless stations will be able to associate to the AP.', LEFT);"
	onMouseOut="return nd();">
	Mode:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_mode" onChange="wl_mode_change();">
	<% wl_router_options();%>
	</select>
    </td>
  </tr>
<!--
-->	
  <tr>
    <th width="310"
	onMouseOver="return overlib('Selecting <b>Disabled</b> disables wireless bridge restriction. Any wireless bridge (including the ones listed in <b>Remote Bridges</b>) will be granted access. Selecting <b>Enabled</b> enables wireless bridge restriction. Only those bridges listed in <b>Remote Bridges</b> will be granted access.', LEFT);"
	onMouseOut="return nd();">
	Bridge Restrict:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_lazywds">
	  <option value="0" <% nvram_match("wl_lazywds", "0", "selected"); %>>Enabled</option>
	  <option value="1" <% nvram_match("wl_lazywds", "1", "selected"); %>>Disabled</option>
	</select>
    </td>
  </tr>
  <tr>	
    <th width="310" valign="top" rowspan="4"
	onMouseOver="return overlib('Enter the wireless MAC addresses of any remote bridges that should be part of the wireless distribution system (WDS).', LEFT);"
	onMouseOut="return nd();">
	<input type="hidden" name="wl_wds" value="4">
	Remote Bridges:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_wds0" value="<% nvram_list("wl_wds", 0); %>" size="17" maxlength="17"></td>
    <td><input name="wl_wds1" value="<% nvram_list("wl_wds", 1); %>" size="17" maxlength="17"></td>
  </tr>
  <tr>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_wds2" value="<% nvram_list("wl_wds", 2); %>" size="17" maxlength="17"></td>
    <td><input name="wl_wds3" value="<% nvram_list("wl_wds", 3); %>" size="17" maxlength="17"></td>
  </tr>
</table>

<p>
<table border="0" cellpadding="0" cellspacing="0">
  <tr>
    <th width="310"
	onMouseOver="return overlib('Selects whether clients with the specified MAC address are allowed or denied wireless access.', LEFT);"
	onMouseOut="return nd();">
	MAC Restrict Mode:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_macmode">
	  <option value="disabled" <% nvram_match("wl_macmode", "disabled", "selected"); %>>Disabled</option>
	  <option value="allow" <% nvram_match("wl_macmode", "allow", "selected"); %>>Allow</option>
	  <option value="deny" <% nvram_match("wl_macmode", "deny", "selected"); %>>Deny</option>
 	</select>
    </td>
  </tr>
  <tr>	
    <th width="310" valign="top" rowspan="8"
	onMouseOver="return overlib('Allows or denies wireless access to clients with the specified MAC addresses. The MAC address format is XX:XX:XX:XX:XX:XX.', LEFT);"
	onMouseOut="return nd();">
	<input type="hidden" name="wl_maclist" value="16">
	MAC Addresses:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_maclist0" value="<% nvram_list("wl_maclist", 0); %>" size="17" maxlength="17"></td>
    <td><input name="wl_maclist1" value="<% nvram_list("wl_maclist", 1); %>" size="17" maxlength="17"></td>
  </tr>
  <tr>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_maclist2" value="<% nvram_list("wl_maclist", 2); %>" size="17" maxlength="17"></td>
    <td><input name="wl_maclist3" value="<% nvram_list("wl_maclist", 3); %>" size="17" maxlength="17"></td>
  </tr>
  <tr>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_maclist4" value="<% nvram_list("wl_maclist", 4); %>" size="17" maxlength="17"></td>
    <td><input name="wl_maclist5" value="<% nvram_list("wl_maclist", 5); %>" size="17" maxlength="17"></td>
  </tr>
  <tr>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_maclist6" value="<% nvram_list("wl_maclist", 6); %>" size="17" maxlength="17"></td>
    <td><input name="wl_maclist7" value="<% nvram_list("wl_maclist", 7); %>" size="17" maxlength="17"></td>
  </tr>
  <tr>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_maclist8" value="<% nvram_list("wl_maclist", 8); %>" size="17" maxlength="17"></td>
    <td><input name="wl_maclist9" value="<% nvram_list("wl_maclist", 9); %>" size="17" maxlength="17"></td>
  </tr>
  <tr>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_maclist10" value="<% nvram_list("wl_maclist", 10); %>" size="17" maxlength="17"></td>
    <td><input name="wl_maclist11" value="<% nvram_list("wl_maclist", 11); %>" size="17" maxlength="17"></td>
  </tr>
  <tr>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_maclist12" value="<% nvram_list("wl_maclist", 12); %>" size="17" maxlength="17"></td>
    <td><input name="wl_maclist13" value="<% nvram_list("wl_maclist", 13); %>" size="17" maxlength="17"></td>
  </tr>
  <tr>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_maclist14" value="<% nvram_list("wl_maclist", 14); %>" size="17" maxlength="17"></td>
    <td><input name="wl_maclist15" value="<% nvram_list("wl_maclist", 15); %>" size="17" maxlength="17"></td>
  </tr>
</table>

<p>
<table border="0" cellpadding="0" cellspacing="0">
  <tr>
    <th width="310" valign="top"
	onMouseOver="return overlib('Stations currently authenticated to this AP.', LEFT);"
	onMouseOut="return nd();">
	Authenticated Stations:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<table>
	  <tr>
	    <td class="label">MAC Address</td>
	    <td class="label">Associated</td>
	    <td class="label">Authorized</td>
	  </tr>
	  <% wl_auth_list(); %>
	</table>
    </td>
  </tr>
</table>

<p>
<table border="0" cellpadding="0" cellspacing="0">
    <tr>
      <td width="310"></td>
      <td>&nbsp;&nbsp;</td>
      <td>
	  <input type="submit" name="action" value="Apply">
	  <input type="reset" name="action" value="Cancel">
      </td>
    </tr>
</table>

</form>

<p class="label">&#169;2001-2004 Broadcom Corporation. All rights
reserved. 54g and XPress are trademarks of Broadcom Corporation.</p>

</body>
</html>
