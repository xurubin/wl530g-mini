##### hostapd configuration file ##############################################
# Empty lines and lines starting with # are ignored

# Manufacturer specific configuration
manuf_file=/etc/mfg_data

# AP netdevice name (without 'ap' prefix, i.e., wlan0 uses wlan0ap for
# management frames)
interface=wlan0

# hostapd event logger configuration
#
# Two output method: syslog and stdout (only usable if not forking to
# background).
#
# Module bitfield (ORed bitfield of modules that will be logged; -1 = all
# modules):
# bit 0 (1) = IEEE 802.11
# bit 1 (2) = IEEE 802.1X
# bit 2 (4) = RADIUS
#
# Levels (minimum value for logged events):
#  0 = verbose debugging
#  1 = debugging
#  2 = informational messages
#  3 = notification
#  4 = warning
#
logger_syslog=-1
logger_syslog_level=2
logger_stdout=-1
logger_stdout_level=0

# Debugging: 0 = no, 1 = minimal, 2 = verbose, 3 = msg dumps
debug=2

# Dump file for state information (on SIGUSR1)
dump_file=/tmp/hostapd.dump

# Daemonize hostapd process (i.e., fork to background)
daemonize=0


##### Wireless enable/disable ################################

# Wireless enable/disable configuration
# 0 = disable
# 1 = enable
wireless_enable=1


##### AutoLink configuration #################################

# Marvell Auto Link configuration
# 0 = disable
# 1 = enable -- this function will use default settings and wait for MARVELL client to do configuration automatically
# 2 = auto-link done -- used by program, user should not set this value
auto_link=0

# SSID dispatch control:
# 0 = disable -- do not append MAC address after SSID
# 1 = enable -- append MAC address after SSID
ssid_patch=0


##### Speed Booster Mode configuration #######################

# Speed Booster Mode configuration
# user must use Marvell client with speed booster mode ability as the first station to accociate with AP
# to make this mode to be enabled
# 0 = disable
# 1 = enable
speed_booster=0


##### IEEE 802.11 related configuration ######################

# AP Operational Mode
# 0 = 802.11b mode only -- basic and oper rate should include only b rates when you use this mode
# 1 = 802.11g mode only
# 2 = 802.11b/g mixed mode
ap_mode=2

# G Protection Mode
# 0 = disable
# 1 = enable
g_protect=0

# AP Mode of Operation for Linux Wireless Extension
# only mode 3 is working now
# 1 = Single cell network (station)
# 2 = Multi cell network, roaming, ... (IBSS)
# 3 = Synchronisation master or Access Point (BSS)
# 4 = Wireless Repeater (forwarder)
# 5 = Secondary master/repeater (backup)
# 6 = Passive monitor (listen only)
iw_mode=3

# AP Operational Rate Sets configuration
# Value in kbps
oper_rate=1000,2000,5500,11000,6000,9000,12000,18000,24000,36000,48000,54000

# AP Basic Rate Sets configuration
# Value in kbps
basic_rate=1000,2000,5500,11000,6000,9000,12000,18000,24000,36000,48000,54000

# Fixed AP Operational Rate Sets configuration
# 0 = Not fixed
# 1 = Fixed
fixed_tx_data_rate=0

#Fixed B Rate
#Value in kbps
fixed_tx_b_rate=11000

#Fixed G Rate
#Value in kbps
fixed_tx_g_rate=54000

# SSID to be used in IEEE 802.11 management frames
ssid=Y Home

# Beacon Interval
beacon_interval=100

# DTIM Period
dtim_period=3

# RTS Threshold
rts_threshold=2347

# Fragmentation Threshold
fragment_threshold=2346

# Short Retry Limit
short_retry_limit=7

# Long Retry Limit
long_retry_limit=4

# Channel for AP
# Valid ranges from 1 to 14 depending on the region being used
channel=<?wlan_channel?>

# Use of Short Preamble
# 0 = No
# 1 = Yes
short_preamble=1


##### Security related configuration #################

# AP Hidden SSID configuration configuration
hide_ssid=0

# Station MAC address -based authentication
# 0 = accept unless in deny list
# 1 = deny unless in accept list
# 2 = use external RADIUS server (accept/deny lists are searched first)
macaddr_acl=0

# Accept/deny lists are read from separate files (containing list of
# MAC addresses, one per line). Use absolute path name to make sure that the
# files can be read on SIGHUP configuration reloads.
accept_mac_file=/etc/config/hostapd.accept
deny_mac_file=/etc/config/hostapd.deny

# IEEE 802.11 specifies two authentication algorithms. hostapd can be
# configured to allow both of these or only one. Open system authentication
# should be used with IEEE 802.1X.
# Bit fields of allowed authentication algorithms:
# bit 0 = Open System Authentication
# bit 1 = Shared Key Authentication (requires WEP)
# nvram_match("wl_auth_mode","psk")) fprintf(fp, "auth_algs=1\n");

auth_algs=1

# WEP Key lengths for default/broadcast and individual/unicast keys:
# 5 = 40-bit WEP (also known as 64-bit WEP with 40 secret bits)
# 13 = 104-bit WEP (also known as 128-bit WEP with 104 secret bits)
wep_key_len=5

# WEP Key Type
# Value can be any of the two strings defined as follows,
# ascii = ASCII string is used
# hex   = Hex-decimal string is used
wep_key_type=hex

# Default WEP Key Index
# Value ranges from 1 to 4
wep_default_key=1

# Default WEP Key Configuration
wep_default_key1=9876543210
wep_default_key2=1112131415
wep_default_key3=2122232425
wep_default_key4=9192939495

# WEP Encryption Mode
# 0 = Disabled
# 1 = Enabled
privacy_invoked=1

# Weak WEP Encryption Mode
# 0 = Accept both WEP encryption and unencryption frames
# 1 = Accept only WEP encryption frames
exclude_unencrypted=1


##### WPA related configuration ##################

# WPA mode (if WPA mode is enabled, WEP setting will not take effect)
# if both WPA and WPA2 are enabled, it means WPA-WPA2-mixed mode
# 0 = disable
# 1 = WPA-PSK
wpa_mode=1

# WPA Data Encryption
# 2 = TKIP
# 4 = AES-CCMP
wpa_encry=4

# WPA PassPhrase (8..63)
wpa_passphrase=<?wlan_password?>


##### WPA2 related configuration ##################
## NOT WORKING right now
# WPA2 mode (if WPA2 mode is enabled, WEP setting will not take effect)
# if both WPA and WPA2 are enabled, it means WPA-WPA2-mixed mode
# 0 = disable
# 1 = WPA2-PSK
wpa2_mode=0

# WPA2 Data Encryption
# 4 = AES-CCMP
wpa2_encry=4

# WPA2 PassPhrase (8..63)
wpa2_passphrase=12345678


##### WPA, WPA2 common configuration ##############

# Group Rekey Interval (in second)
group_rekey_time=86400


##### Misc configuration #################

# GPIO (0..15) for WLAN LED, if you use the same one, set tx and rx to the same GPIO
wlan_tx_gpio=5
wlan_rx_gpio=5

# antenna seclection
# 1: antenna A
# 2: antenna B
antenna=2

# Watch Dog Timer control
# 0 = disable
# 1 = enable
watchdog_timer=1

# Operation mode (#ifdef WDS)
# 0 = AP
# 1 = P2P
# op_mode=0

# Wireless STA bridge
# 0 = diable, i.e., no packet allowed between wireless stations
# 1 = enable, i.e., packet allowed between wireless stations
sta_bridge=1


minimal_eap=0