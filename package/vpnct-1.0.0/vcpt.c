/**************** 
Program Name : vcpt - VPN Configuration Puller & Tester
Author : flarifhr
License : None
Plateform : Openwrt
Date : 08/15/2018
*****************/ 
#define _GNU_SOURCE
#include <stdio.h> 
#include <sqlite3.h> 
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include "b64.h"

#define NUMBER 16
#define MAX_STRING 128
#define MAX_STRING_D 15000

#define VPNC_URL "www.vpngate.net/api/iphone/"

//Prototypes
int create_table();
int insert_intotab();
int select_data(char[5]);
int fetch_vpn_configs();
char ** get_field_names();

static int c;

static int cb(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

static int cb_sel(void *data, int argc, char **argv, char **azColName){
   int i;
   fprintf(stderr, "%s", (const char*)data);
   
   FILE * fp;
   
   for(i = 0; i<argc; i++){
	  if (strcmp(azColName[i],"OpenVPN_ConfigData_Base64") == 0) {
			size_t size = 32000;
			unsigned char *decoded;
			decoded = b64_decode(argv[i],size);
			char filename[64] = "";
			c++;
			snprintf( filename, sizeof(filename),"/etc/vpnct/vpn_config_%i.ovpn",c);
			fp = fopen (filename,"w");
			fprintf (fp, "%s",decoded);
			printf("VPN Configuration Save to :%s\n",filename);
	  } else {
		  printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	  }
   }
   
   printf("\n");
   return 0;
}

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}


int main(int argc, char **argv){
	
	
    if (argc < 2)  {
        printf("use correct way of running");
    } 
	
	if (strcmp("gen_3configs", argv[1]) == 0)     {
         printf("Here are 3 Configurations : \n");
		 select_data("none");
    }
	
	if (strcmp("gen_3configs_ccode", argv[1]) == 0)     {
         printf("Here are 3 Configurations for Country Code = %s : \n",argv[2]);
		 select_data(argv[2]);
    }
	
	if (strcmp("connect", argv[1]) == 0)     {
         printf("Here are 3 Configurations....");
		 system("openvpn --config /etc/vpnct/vpn_config_1.ovpn --proto tcp-client --connect-retry 3");
    }
	
	if (strcmp("update", argv[1]) == 0)     {
         printf("Getting VPN Configurations....");
		 fetch_vpn_configs();
		 printf("Updating VPN Configurations database....");
		 //Clean Configuration Files
			system("sed -i 's/,,/,00,/' /etc/vpnct/vcpt.gdr"); 
			system("sed -i '/^*/d' /etc/vpnct/vcpt.gdr");
			system("sed -i 's/#HostName/HostName/' /etc/vpnct/vcpt.gdr");
			create_table();
			insert_intotab();

    }
	
  
   printf("Starting the vcpt..... \n\n"); 

	return 0; 
} 

char ** get_field_names() {
	
	char c[2000];
    FILE *fptr;

    if ((fptr = fopen("/etc/vpnct/vcpt.gdr", "r")) == NULL)
    {
        printf("Error! opening file");
        // Program exits if file pointer returns NULL.
       return NULL;         
    }

    // reads text until newline 
    fscanf(fptr,"%[^\n]", c);

	//initialize the array to return
	int i = 0;
	char **array = malloc (sizeof (char *) * NUMBER);
	if (!array)
		return NULL;
	for (i = 0; i < NUMBER; i++) {
		array[i] = malloc (MAX_STRING + 1);
    if (!array[i]) {
      free (array);
      return NULL;
    }
	}
	//save csv of fieldnames to array
	const char s[2] = ",\r\n";
    char *token;
	int k = 0;
    token = strtok(c, s);
    while( token != NULL ) {
      //printf( " %s\n", token );
	  strncpy(array[k], token, MAX_STRING);
	  k++;
      token = strtok(NULL, s);
    }
	
    fclose(fptr);
    
		
	return array;
}

int fetch_vpn_configs() {
	
	FILE *fp;
	char* str = "string";
	int x = 10;

    CURL *curl;
    CURLcode res;

  curl = curl_easy_init();
  if(curl) {
    struct string s;
    init_string(&s);

    curl_easy_setopt(curl, CURLOPT_URL, "www.vpngate.net/api/iphone/");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    res = curl_easy_perform(curl);
	
	fp=fopen("/etc/vpnct/vcpt.gdr", "w");
	if(fp == NULL)
    exit(-1);
	fprintf(fp, s.ptr);
	fclose(fp);
    free(s.ptr);
	
	
    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  printf("vpn configs pulled..... \n\n");
  return 0;
	
}
int create_table() {
   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   char *sql;

   /* Open database */
   rc = sqlite3_open("/etc/vpnct/vcpt.db", &db);
   
   if( rc ) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return(0);
   } else {
      fprintf(stdout, "Opened database successfully\n");
   }

   char sql_str[1024] = "";
   char **fn = get_field_names();
   snprintf( sql_str, sizeof(sql_str), "CREATE TABLE IF NOT EXISTS VPN_COFIGS(" \
				"ID INT PRIMARY KEY     NOT NULL," \
				"%s       TEXT    NOT NULL," \
				"%s       TEXT    NOT NULL," \
				"%s       TEXT    NOT NULL," \
				"%s       TEXT    NOT NULL," \
				"%s       TEXT    NOT NULL," \
				"%s       TEXT    NOT NULL," \
				"%s       TEXT    NOT NULL," \
				"%s       TEXT    NOT NULL," \
				"%s       TEXT    NOT NULL," \
				"%s       TEXT    NOT NULL," \
				"%s       TEXT    NOT NULL," \
				"%s       TEXT    NOT NULL," \
				"%s       TEXT    NOT NULL," \
				"%s       TEXT    NOT NULL," \
				"%s       TEXT    NOT NULL" \
				")", fn[0], fn[1], fn[2], fn[3], fn[4], fn[5], fn[6], fn[7], fn[8], fn[9], fn[10], fn[11], fn[12], fn[13], fn[14]);

   
   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql_str, cb, 0, &zErrMsg);
   
   if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   } else {
      fprintf(stdout, "Table created successfully\n");
   }
   sqlite3_close(db);
   return 0;
}

int insert_intotab() {
   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   char *sql;

   /* Open database */
   rc = sqlite3_open("/etc/vpnct/vcpt.db", &db);
   
   if( rc ) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return(0);
   } else {
      fprintf(stderr, "Opened database successfully\n");
   }
   
   //////read file
   FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
	
	char sql_str[15000] = "";
   
	

	int i = 0;
	char **array = malloc (sizeof (char *) * NUMBER);
	if (!array)
		return NULL;
	for (i = 0; i < NUMBER; i++) {
		array[i] = malloc (MAX_STRING_D + 1);
    if (!array[i]) {
      free (array);
      return NULL;
    }
	}
	
	

   fp = fopen("/etc/vpnct/vcpt.gdr", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
	int k = 1;
	
   while ((read = getline(&line, &len, fp)) != -1) {
		const char s[2] = ",\r\n";
		char *token;
		token = strtok(line, s);
		snprintf( sql_str, sizeof(sql_str), " VALUES (%i",k);
		int m = 0;
		while( token != NULL ) {
		  strncpy(array[m], token, MAX_STRING_D);
		  m++;
		  token = strtok(NULL, s);
		}
		 char **fn = get_field_names();
		snprintf( sql_str, sizeof(sql_str), "INSERT OR REPLACE INTO VPN_COFIGS (" \
		"ID,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)" \
		" VALUES (%i,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"" \
		");" , fn[0], fn[1], fn[2], fn[3], fn[4], fn[5], fn[6], fn[7], fn[8], fn[9], fn[10], fn[11], fn[12], fn[13], fn[14], \
		k, array[0], array[1], array[2], array[3],array[4], array[5], array[6], array[7], array[8],array[9] , array[10], array[11], array[12],array[13],array[14]);	
		k++;
		rc = sqlite3_exec(db, sql_str, cb, 0, &zErrMsg);
		if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		printf("%s\n", sql_str);
		sqlite3_free(zErrMsg);
		} else {
		fprintf(stdout, "VPN configuration  added successfully : %i\n",k);
		}
    }

   free(line);
   exit(EXIT_SUCCESS);
   
   sqlite3_close(db);
   return 0;
}

int select_data(char criteria[5]) {
	
   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   //char *sql;
   char sql_str[1024] = "";
   const char* data = "VPN Configuration Pulled \n";

   /* Open database */
   rc = sqlite3_open("/etc/vpnct/vcpt.db", &db);
   
   if( rc ) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return(0);
   } else {
      fprintf(stderr, "Opened database successfully\n");
   }
   
   if (strcmp(criteria,"none") == 0) {
   snprintf(sql_str, sizeof(sql_str),"SELECT * FROM VPN_COFIGS WHERE ID IN (SELECT ID FROM VPN_COFIGS ORDER BY RANDOM() LIMIT 3)");
   } else {
	   snprintf(sql_str, sizeof(sql_str),"SELECT * FROM VPN_COFIGS WHERE ID IN (SELECT ID FROM VPN_COFIGS WHERE CountryShort = \"%s\" ORDER BY RANDOM() LIMIT 3)", criteria);
   }
   
   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql_str, cb_sel, (void*)data, &zErrMsg);
   
   if( rc != SQLITE_OK ) {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   } else {
      fprintf(stdout, "Operation done successfully\n");
   }
   sqlite3_close(db);
   return 0;
}

#ifdef b64_USE_CUSTOM_MALLOC
extern void* b64_malloc(size_t);
#endif

#ifdef b64_USE_CUSTOM_REALLOC
extern void* b64_realloc(void*, size_t);
#endif

unsigned char *
b64_decode (const char *src, size_t len) {
  return b64_decode_ex(src, len, NULL);
}

unsigned char *
b64_decode_ex (const char *src, size_t len, size_t *decsize) {
  int i = 0;
  int j = 0;
  int l = 0;
  size_t size = 0;
  unsigned char *dec = NULL;
  unsigned char buf[3];
  unsigned char tmp[4];

  // alloc
  dec = (unsigned char *) b64_malloc(1);
  if (NULL == dec) { return NULL; }

  // parse until end of source
  while (len--) {
    // break if char is `=' or not base64 char
    if ('=' == src[j]) { break; }
    if (!(isalnum(src[j]) || '+' == src[j] || '/' == src[j])) { break; }

    // read up to 4 bytes at a time into `tmp'
    tmp[i++] = src[j++];

    // if 4 bytes read then decode into `buf'
    if (4 == i) {
      // translate values in `tmp' from table
      for (i = 0; i < 4; ++i) {
        // find translation char in `b64_table'
        for (l = 0; l < 64; ++l) {
          if (tmp[i] == b64_table[l]) {
            tmp[i] = l;
            break;
          }
        }
      }

      // decode
      buf[0] = (tmp[0] << 2) + ((tmp[1] & 0x30) >> 4);
      buf[1] = ((tmp[1] & 0xf) << 4) + ((tmp[2] & 0x3c) >> 2);
      buf[2] = ((tmp[2] & 0x3) << 6) + tmp[3];

      // write decoded buffer to `dec'
      dec = (unsigned char *) b64_realloc(dec, size + 3);
      if (dec != NULL){
        for (i = 0; i < 3; ++i) {
          dec[size++] = buf[i];
        }
      } else {
        return NULL;
      }

      // reset
      i = 0;
    }
  }

  // remainder
  if (i > 0) {
    // fill `tmp' with `\0' at most 4 times
    for (j = i; j < 4; ++j) {
      tmp[j] = '\0';
    }

    // translate remainder
    for (j = 0; j < 4; ++j) {
        // find translation char in `b64_table'
        for (l = 0; l < 64; ++l) {
          if (tmp[j] == b64_table[l]) {
            tmp[j] = l;
            break;
          }
        }
    }

    // decode remainder
    buf[0] = (tmp[0] << 2) + ((tmp[1] & 0x30) >> 4);
    buf[1] = ((tmp[1] & 0xf) << 4) + ((tmp[2] & 0x3c) >> 2);
    buf[2] = ((tmp[2] & 0x3) << 6) + tmp[3];

    // write remainer decoded buffer to `dec'
    dec = (unsigned char *) b64_realloc(dec, size + (i - 1));
    if (dec != NULL){
      for (j = 0; (j < i - 1); ++j) {
        dec[size++] = buf[j];
      }
    } else {
      return NULL;
    }
  }

  // Make sure we have enough space to add '\0' character at end.
  dec = (unsigned char *) b64_realloc(dec, size + 1);
  if (dec != NULL){
    dec[size] = '\0';
  } else {
    return NULL;
  }

  // Return back the size of decoded string if demanded.
  if (decsize != NULL) {
    *decsize = size;
  }

  return dec;
}

