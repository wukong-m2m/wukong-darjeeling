#include "config.h"
#if defined(SMARTTHINGS)

#include "smartthings_utils.h"

// Simple structure to keep track of the handle, and
// of what needs to be freed later.
typedef struct {
    int socket;
    SSL *sslHandle;
    SSL_CTX *sslContext;
} connection;

// For this example, we'll be testing on openssl.org
#define ST_SERVER  "graph.api.smartthings.com"
#define ST_PORT 443

const char *st_path = "api/smartapps/installations/%s/listAll/?access_token=%s";
// const char *st_http_get_header = "GET /%s HTTP/1.1\r\nConnection: close\r\nHost:graph.api.smartthings.com\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n";
// const char *st_body = "access_token=%s";
const char *st_http_get_header = "GET /%s HTTP/1.1\r\nAccept: */*\r\nHost:graph.api.smartthings.com\r\nConnection: Close\r\n\r\n";

// Establish a regular tcp connection
int tcpConnect ()
{
  int error, handle;
  struct hostent *host;
  struct sockaddr_in server;

  host = gethostbyname (ST_SERVER);
  handle = socket (AF_INET, SOCK_STREAM, 0);
  if (handle == -1)
    {
      // perror ("Socket");
      handle = 0;
    }
  else
    {
      server.sin_family = AF_INET;
      server.sin_port = htons (ST_PORT);
      server.sin_addr = *((struct in_addr *) host->h_addr);
      bzero (&(server.sin_zero), 8);

      error = connect (handle, (struct sockaddr *) &server,
                       sizeof (struct sockaddr));
      if (error == -1)
        {
          // perror ("Connect");
          handle = 0;
        }
    }

  return handle;
}

// Establish a connection using an SSL layer
connection *sslConnect (void)
{
  connection *c;

  c = malloc (sizeof (connection));
  c->sslHandle = NULL;
  c->sslContext = NULL;

  c->socket = tcpConnect ();
  if (c->socket)
    {
      // Register the error strings for libcrypto & libssl
      SSL_load_error_strings ();
      // Register the available ciphers and digests
      SSL_library_init ();

      // New context saying we are a client, and using SSL 2 or 3
      c->sslContext = SSL_CTX_new (SSLv23_client_method ());
      if (c->sslContext == NULL)
        ERR_print_errors_fp (stderr);

      // Create an SSL struct for the connection
      c->sslHandle = SSL_new (c->sslContext);
      if (c->sslHandle == NULL)
        ERR_print_errors_fp (stderr);

      // Connect the SSL struct to our connection
      if (!SSL_set_fd (c->sslHandle, c->socket))
        ERR_print_errors_fp (stderr);

      // Initiate SSL handshake
      if (SSL_connect (c->sslHandle) != 1)
        ERR_print_errors_fp (stderr);
    }
  else
    {
      return NULL;
    }

  return c;
}

// Disconnect & free connection struct
void sslDisconnect (connection *c)
{
  if (c->socket)
    close (c->socket);
  if (c->sslHandle)
    {
      SSL_shutdown (c->sslHandle);
      SSL_free (c->sslHandle);
    }
  if (c->sslContext)
    SSL_CTX_free (c->sslContext);

  free (c);
}

// Read all available text from the connection
char *sslRead (connection *c)
{
  const int readSize = 1024;
  char *rc = NULL;
  int received, count = 0;
  char buffer[1024];

  if (c)
    {
      while (1)
        {
          if (!rc)
            rc = calloc (readSize+1, sizeof (char));
          else
            rc = realloc (rc, (count + 1) *
                          readSize * sizeof (char) + 1);

          received = SSL_read (c->sslHandle, buffer, readSize);
          buffer[received] = '\0';

          if (received > 0)
            strcat (rc, buffer);

          if (received < readSize)
            break;
          count++;
        }
    }

  return rc;
}

// Write text to the connection
void sslWrite (connection *c, char *text)
{
  if (c)
    SSL_write (c->sslHandle, text, strlen (text));
}

// Very basic main: we send GET / and print the response.
int getStatus (char *message, int msg_len, char *app_id, char *access_token,
  char *dev_id, char *status, int status_len)
{
  connection *c;
  char *response;
  c = sslConnect ();
  if (!c) return -1;

  memset(message, 0, msg_len);
  char buf[500] = {0};
  // int a_t_len = strlen(st_body) - 2 + strlen(access_token);
  sprintf(buf, st_path, app_id, access_token);
  // sprintf(message, st_http_get_header, buf, a_t_len);
  // sprintf(buf, st_body, access_token);
  // strcat(message, buf);
  sprintf(message, st_http_get_header, buf);
  // strcpy(message, "GET /ip HTTP/1.1\r\nAccept: */*\r\nHost:httpbin.org\r\nConnection: Keep-Alive\r\n\r\n");

  sslWrite (c, message);
  response = sslRead (c);

  memset(message, 0, msg_len);
  int r_len = strlen(response);
  r_len = r_len>msg_len-1?msg_len-1:r_len;
  strncpy(message, response, r_len);
  sslDisconnect (c);
  free (response);

  message = (strstr(message, "\r\n\r\n"));
  message = (strstr(message, "{"));
  cJSON *root = cJSON_Parse(message);
  if (!root) return -100;
  cJSON *item, *subitem, *devitem;
  item = root->child;
  while (item)
  {
    subitem = item->child;
    while(subitem){
      devitem = cJSON_GetObjectItem(subitem, "id");
      if (devitem && !strcmp(devitem->valuestring, dev_id))
      {
        devitem = cJSON_GetObjectItem(subitem, status);
        if(!devitem) return -101;

        memset(status, 0, status_len);
        switch (devitem->type){
          case cJSON_False:
            sprintf(status, "true");
            return 0;
          case cJSON_True:
            sprintf(status, "false");
            return 0;
          case cJSON_Number:
            sprintf(status, "%d", devitem->valueint);
            return 0;
          case cJSON_String:
            sprintf(status, "%s", devitem->valuestring);
            return 0;
          case cJSON_Array:
            return -102;
          case cJSON_Object:
            return -103;
          case cJSON_NULL:
            return -104;
          default:
            return -105;
        }
      }
      subitem = subitem->next;
    }
    item = item->next;
  }
  memset(status, 65, status_len);
  return -106;
}
#endif