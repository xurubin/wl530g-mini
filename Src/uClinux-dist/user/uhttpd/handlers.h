
/* Provides accepted userid/passwd */
extern void do_auth(char *userid, char *passwd, char *realm);

/* Regular file handler */
extern int do_file(char *path, FILE *stream);

/* Parse POST form data. */
extern void do_parse_postform(char *url, FILE *stream, int len, char *boundary);
/* Parse POST file upload. */
extern void do_parse_fileupload(char *url, FILE *stream, int len, char *boundary);

/* Config accessing handler */
extern int do_config_get(char *path, FILE *stream);
extern int do_config_set(char *path, FILE *stream);

/* System status/controlling */
extern int do_status_get(char *path, FILE *stream);
extern int do_upload_config_template(char *path, FILE *stream);
extern int do_get_config_template(char *path, FILE *stream);
extern int do_control_get(char *path, FILE *stream);
