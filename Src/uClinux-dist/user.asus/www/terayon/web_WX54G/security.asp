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
<title>Broadcom Home Gateway Reference Design: Security</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<link rel="stylesheet" type="text/css" href="style.css" media="screen">
<script language="JavaScript" type="text/javascript" src="overlib.js"></script>
<script language="JavaScript" type="text/javascript">
<!--
function wl_wep_change()
{
	var mode = document.forms[0].wl_auth_mode[document.forms[0].wl_auth_mode.selectedIndex].value;
	var wep = document.forms[0].wl_wep[document.forms[0].wl_wep.selectedIndex].value;

	/* enable/disable network key 1 to 4 */
	if (wep == "enabled") {
		if (mode == "wpa" || mode == "psk" || mode == "radius") {
			document.forms[0].wl_key1.disabled = 1;
			document.forms[0].wl_key4.disabled = 1;
		}
		else {
			document.forms[0].wl_key1.disabled = 0;
			document.forms[0].wl_key4.disabled = 0;
		}
		document.forms[0].wl_key2.disabled = 0;
		document.forms[0].wl_key3.disabled = 0;
	} else {
		document.forms[0].wl_key1.disabled = 1;
		document.forms[0].wl_key2.disabled = 1;
		document.forms[0].wl_key3.disabled = 1;
		document.forms[0].wl_key4.disabled = 1;
	}

	/* enable/disable key index */
	if (wep == "enabled")
		document.forms[0].wl_key.disabled = 0;
	else
		document.forms[0].wl_key.disabled = 1;

	/* enable/disable gtk rotation interval */
	if (wep == "enabled")
		document.forms[0].wl_wpa_gtk_rekey.disabled = 1;
	else {
		if (mode == "wpa" || mode == "psk")
			document.forms[0].wl_wpa_gtk_rekey.disabled = 0;
		else
			document.forms[0].wl_wpa_gtk_rekey.disabled = 1;
	}
}

function wl_auth_mode_change()
{
	var mode = document.forms[0].wl_auth_mode[document.forms[0].wl_auth_mode.selectedIndex].value;
	var i, cur, algos;

	/* enable/disable radius IP, port, password */
	if (mode == "wpa" || mode == "radius") {
		document.forms[0].wl_radius_ipaddr.disabled = 0;
		document.forms[0].wl_radius_port.disabled = 0;
		document.forms[0].wl_radius_key.disabled = 0;
	} else {
		document.forms[0].wl_radius_ipaddr.disabled = 1;
		document.forms[0].wl_radius_port.disabled = 1;
		document.forms[0].wl_radius_key.disabled = 1;
	}

	/* enable/disable crypto algorithm */
	if (mode == "wpa" || mode == "psk")
		document.forms[0].wl_crypto.disabled = 0;
	else
		document.forms[0].wl_crypto.disabled = 1;

	/* enable/disable psk passphrase */
	if (mode == "psk")
		document.forms[0].wl_wpa_psk.disabled = 0;
	else
		document.forms[0].wl_wpa_psk.disabled = 1;

	/* update wl_crypto */
	if (mode == "wpa" || mode == "psk") {
		/* Save current crypto algorithm */
		for (i = 0; i < document.forms[0].wl_crypto.length; i++) {
			if (document.forms[0].wl_crypto[i].selected) {
				cur = document.forms[0].wl_crypto[i].value;
				break;
			}
		}

		/* Define new crypto algorithms */
		if (<% wl_corerev(); %> >= 3)
			algos = new Array("TKIP", "AES", "TKIP+AES");
		else
			algos = new Array("TKIP");

		/* Reconstruct algorithm array from new crypto algorithms */
		document.forms[0].wl_crypto.length = algos.length;
		for (var i in algos) {
			document.forms[0].wl_crypto[i] = new Option(algos[i], algos[i].toLowerCase());
			document.forms[0].wl_crypto[i].value = algos[i].toLowerCase();
			if (algos[i].toLowerCase() == cur)
				document.forms[0].wl_crypto[i].selected = true;
		}
	}

	/* Save current network key index */
	for (i = 0; i < document.forms[0].wl_key.length; i++) {
		if (document.forms[0].wl_key[i].selected) {
			cur = document.forms[0].wl_key[i].value;
			break;
		}
	}

	/* Define new network key indices */
	if (mode == "wpa" || mode == "psk" || mode == "radius")
		algos = new Array("2", "3");
	else
		algos = new Array("1", "2", "3", "4");

	/* Reconstruct network key indices array from new network key indices */
	document.forms[0].wl_key.length = algos.length;
	for (var i in algos) {
		document.forms[0].wl_key[i] = new Option(algos[i], algos[i]);
		document.forms[0].wl_key[i].value = algos[i];
		if (algos[i] == cur)
			document.forms[0].wl_key[i].selected = true;
	}

	wl_wep_change();
}
//-->
</script>
</head>

<body onLoad="wl_auth_mode_change();">
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
	<span class="title">SECURITY</span><br>
	<span class="subtitle">This page allows you to configure
	security for the wireless LAN interfaces.</span>
    </td>
  </tr>
</table>

<form method="post" action="security.asp">
<input type="hidden" name="page" value="security.asp">

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
	onMouseOver="return overlib('Sets the network authentication method. <b>802.1X</b> and <b>WPA</b> require that valid RADIUS parameters be set. <b>WPA-PSK</b> requires a valid WPA Pre-Shared Key to be set.', LEFT);"
	onMouseOut="return nd();">
	Network Authentication:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_auth_mode" onChange="wl_auth_mode_change();">
	  <option value="open" <% nvram_match("wl_auth_mode", "open", "selected"); %>>Open</option>
	  <option value="shared" <% nvram_match("wl_auth_mode", "shared", "selected"); %>>Shared</option>
	  <option value="radius" <% nvram_match("wl_auth_mode", "radius", "selected"); %>>802.1X</option>
	  <option value="wpa" <% nvram_match("wl_auth_mode", "wpa", "selected"); %>>WPA</option>
	  <option value="psk" <% nvram_match("wl_auth_mode", "psk", "selected"); %>>WPA-PSK</option>
	</select>
    </td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Selects the WPA data encryption algorithm.', LEFT);"
	onMouseOut="return nd();">
	WPA Encryption:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_crypto">
	  <option value="tkip" <% nvram_match("wl_crypto", "tkip", "selected"); %>>TKIP</option>
	  <option value="aes" <% nvram_match("wl_crypto", "aes", "selected"); %>>AES</option>
	  <option value="tkip+aes" <% nvram_match("wl_crypto", "tkip+aes", "selected"); %>>TKIP+AES</option>
 	</select>
    </td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Enables or disables WEP data encryption. Selecting <b>Enabled</b> enables WEP data encryption and requires that a valid network key be set and selected unless <b>802.1X</b> is enabled.', LEFT);"
	onMouseOut="return nd();">
	WEP Encryption:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_wep" onChange="wl_wep_change();">
	  <option value="disabled" <% nvram_match("wl_wep", "disabled", "selected"); %>>Disabled</option>
	  <option value="enabled" <% nvram_match("wl_wep", "enabled", "selected"); %>>Enabled</option>
 	</select>
    </td>
  </tr>
</table>

<p>
<table border="0" cellpadding="0" cellspacing="0">
  <tr>
    <th width="310"
	onMouseOver="return overlib('Sets the WPA Pre-Shared Key (PSK).', LEFT);"
	onMouseOut="return nd();">
	WPA Pre-Shared Key:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_wpa_psk" value="<% nvram_get("wl_wpa_psk"); %>" type="password"></td>
  </tr>
</table>

<p>
<table border="0" cellpadding="0" cellspacing="0">
  <tr>
    <th width="310"
	onMouseOver="return overlib('Sets the IP address of the RADIUS server to use for authentication and dynamic key derivation.', LEFT);"
	onMouseOut="return nd();">
	RADIUS Server:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_radius_ipaddr" value="<% nvram_get("wl_radius_ipaddr"); %>" size="15" maxlength="15"></td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Sets the UDP port number of the RADIUS server. The port number is usually 1812 or 1645 and depends upon the server.', LEFT);"
	onMouseOut="return nd();">
	RADIUS Port:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_radius_port" value="<% nvram_get("wl_radius_port"); %>" size="5" maxlength="5"></td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Sets the shared secret for the RADIUS connection.', LEFT);"
	onMouseOut="return nd();">
	RADIUS Key:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_radius_key" value="<% nvram_get("wl_radius_key"); %>" type="password"></td>
  </tr>
</table>

<p>
<table border="0" cellpadding="0" cellspacing="0">
  <tr>
    <th width="310"
	onMouseOver="return overlib('Enter 5 ASCII characters or 10 hexadecimal digits for a 64-bit key. Enter 13 ASCII characters or 26 hexadecimal digits for a 128-bit key.', LEFT);"
	onMouseOut="return nd();">
	Network Key 1:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_key1" value="<% nvram_get("wl_key1"); %>" size="26" maxlength="26" type="password"></td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Enter 5 ASCII characters or 10 hexadecimal digits for a 64-bit key. Enter 13 ASCII characters or 26 hexadecimal digits for a 128-bit key.', LEFT);"
	onMouseOut="return nd();">
	Network Key 2:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_key2" value="<% nvram_get("wl_key2"); %>" size="26" maxlength="26" type="password"></td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Enter 5 ASCII characters or 10 hexadecimal digits for a 64-bit key. Enter 13 ASCII characters or 26 hexadecimal digits for a 128-bit key.', LEFT);"
	onMouseOut="return nd();">
	Network Key 3:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_key3" value="<% nvram_get("wl_key3"); %>" size="26" maxlength="26" type="password"></td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Enter 5 ASCII characters or 10 hexadecimal digits for a 64-bit key. Enter 13 ASCII characters or 26 hexadecimal digits for a 128-bit key.', LEFT);"
	onMouseOut="return nd();">
	Network Key 4:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_key4" value="<% nvram_get("wl_key4"); %>" size="26" maxlength="26" type="password"></td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Selects which network key is used for encrypting outbound data and/or authenticating clients.', LEFT);"
	onMouseOut="return nd();">
	Current Network Key:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_key">
	  <option value="1" <% nvram_match("wl_key", "1", "selected"); %>>1</option>
	  <option value="2" <% nvram_match("wl_key", "2", "selected"); %>>2</option>
	  <option value="3" <% nvram_match("wl_key", "3", "selected"); %>>3</option>
	  <option value="4" <% nvram_match("wl_key", "4", "selected"); %>>4</option>
	</select>
    </td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Sets the Network key rotation interval in seconds. Leave blank or set to zero to disable the rotation.', LEFT);"
	onMouseOut="return nd();">
	Network Key Rotation Interval:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td><input name="wl_wpa_gtk_rekey" value="<% nvram_get("wl_wpa_gtk_rekey"); %>" size="10" maxlength="10"></td>
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

<p class="label">&#169;2001-2004 Broadcom Corporation. All rights reserved. 54g is a trademark of Broadcom Corporation.</p>

</body>
</html>
