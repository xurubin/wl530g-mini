
/*
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */

#include <stdio.h>
#include "httpd.h"
#include "handlers.h"

struct mime_handler mime_handlers[] = {
	{ "**.htm", "text/html", NULL, NULL, do_file, do_auth },
	{ "**.html", "text/html", NULL, NULL, do_file, do_auth },
	{ "**.gif", "image/gif", NULL, NULL, do_file, NULL },
	{ "**.jpg", "image/jpeg", NULL, NULL, do_file, NULL },
	{ "**.jpeg", "image/gif", NULL, NULL, do_file, NULL },
	{ "**.png", "image/png", NULL, NULL, do_file, NULL },
	{ "**.css", "text/css", NULL, NULL, do_file, NULL },
	{ "**.js", "text/javascript", NULL, NULL, do_file, NULL },

	//{ "**.pac", "application/x-ns-proxy-autoconfig", NULL, NULL, do_file, NULL },
	{ "config/get/*", "application/json", NULL, NULL, do_config_get, do_auth},
	{ "config/set/*", "application/json", NULL, do_parse_postform, do_config_set , do_auth},

	{ "status", "application/json", NULL, NULL, do_status_get, do_auth},

	{ "control/upload_config_template", NULL, NULL, do_parse_fileupload, do_upload_config_template, do_auth},
	{ "control/get_config_template", "application/oct-stream", "Content-Disposition: attachment; filename=template.gz", NULL, do_get_config_template, do_auth},
	{ "control/*", "application/json", NULL, NULL, do_control_get, do_auth},

	{ NULL, NULL, NULL, NULL, NULL, NULL }
};
