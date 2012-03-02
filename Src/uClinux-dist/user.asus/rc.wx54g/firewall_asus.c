
#define foreach_x(x)	for(i=0; i<atoi(nvram_safe_get(x)); i++)

char *g_buf;
char g_buf_pool[1024];

void g_buf_init()
{
	g_buf = g_buf_pool;
}

char *g_buf_alloc(char *g_buf_now)
{
	g_buf += strlen(g_buf_now)+1;

	return(g_buf_now);
}

void nvram_unsets(char *name, int count)
{
	char itemname_arr[32];
	int i;

	for(i=0; i<count; i++)
	{
		sprintf(itemname_arr, "%s%d", name, i);
		nvram_unset(itemname_arr);
	}
}

int nvram_cmp(char *name, char *value)
{
		
	if (strcmp(value, nvram_safe_get(name))==0) return 1;
	else return 0;
}

int nvram_set_v(char *name, char *item)
{
	nvram_set(name, nvram_safe_get(item));
}

int nvram_set_v2(char *name, char *item1, char *item2)
{
	char items_arr[128];
	
	sprintf(items_arr, "%s %s", nvram_safe_get(item1), nvram_safe_get(item2));
	
	nvram_set(name, items_arr);
}

char *mac_conv(char *mac_name, int idx)
{
	char *mac;
	char itemname_arr[32];
	int i, j;
	
	sprintf(itemname_arr,"%s%d", mac_name, idx);

	mac = nvram_get(itemname_arr);

	j=0;	
	for(i=0; i<12; i++)
	{		
		if (i!=0&&i%2==0) g_buf[j++] = ':';
		g_buf[j++] = mac[i];
	}
	g_buf[j] = 0;

	return(g_buf_alloc(g_buf));
}
 
char *proto_conv(char *proto_name, int idx)
{	
	char *proto;
	char itemname_arr[32];
	
	sprintf(itemname_arr,"%s%d", proto_name, idx);
	proto=nvram_get(itemname_arr);
		
	if(!strncasecmp(proto, "Both", 3)) strcpy(g_buf, "both");	
	else if(!strncasecmp(proto, "TCP", 3)) strcpy(g_buf, "tcp");
	else if(!strncasecmp(proto, "UDP", 3)) strcpy(g_buf, "udp");
	else strcpy(g_buf,"tcp");
	return (g_buf_alloc(g_buf));
}

char *portrange_conv(char *port_name, int idx)
{
	char *port, *strptr;
	char itemname_arr[32];
	
	sprintf(itemname_arr,"%s%d", port_name, idx);
	port=nvram_get(itemname_arr);
		
	if(!strncmp(port, ">", 1)) {
		sprintf(g_buf, "%d-65535", atoi(port+1) + 1);
	}
	else if(!strncmp(port, "=", 1)) {
		sprintf(g_buf, "%d-%d", atoi(port+1), atoi(port+1));
	}
	else if(!strncmp(port, "<", 1)) {
		sprintf(g_buf, "1-%d", atoi(port+1) - 1);
	}
	else if(strptr=strchr(port, ':'))
	{		
		strcpy(itemname_arr, port);
		strptr = strchr(itemname_arr, ':');
		sprintf(g_buf, "%d-%d", atoi(itemname_arr), atoi(strptr+1));		
	}
	else if(*port)
	{
		sprintf(g_buf, "%d-%d", atoi(port), atoi(port));
	}
	else
	{
		sprintf(g_buf, "");
	}
	
	return(g_buf_alloc(g_buf));
}


char *iprange_conv(char *ip_name, int idx)
{
	char *ip;
	char itemname_arr[32], *convptr;
	char startip[16], endip[16];
	int i, j, k;
	
	sprintf(itemname_arr,"%s%d", ip_name, idx);
	ip=nvram_get(itemname_arr);
			
	// scan all ip string
	i=j=k=0;
	
	while(*(ip+i))
	{
		if (*(ip+i)=='*') 
		{
			startip[j++] = '1';
			endip[k++] = '2';
			endip[k++] = '5';
			endip[k++] = '4';
			// 255 is for broadcast
		}
		else 
		{
			startip[j++] = *(ip+i);
			endip[k++] = *(ip+i);
		}
		i++;
	}	
	
	startip[j++] = 0;
	endip[k++] = 0;
	
	sprintf(g_buf, "%s-%s", startip, endip);	
	return(g_buf_alloc(g_buf));
}


char *ip_conv(char *ip_name, int idx)
{	
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", ip_name, idx);
	return(nvram_get(itemname_arr));
}


