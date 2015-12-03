#include "philip_hue_utils.h"


typedef struct point2d { float x; float y; } point2d;

const char *philips_hue_path = "api/newdeveloper/lights/%d";
const char *hue_http_get_header = "GET /%s HTTP/1.1\r\nConnection: close\r\n\r\n";
const char *hue_http_put_header = "PUT /%s HTTP/1.1\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";

void gammaCorrection(float *rgb)
{
  // http://www.babelcolor.com/download/A%20review%20of%20RGB%20color%20spaces.pdf
  // http://www.developers.meethue.com/documentation/color-conversions-rgb-xy
  if (*rgb > 0.04045){
    *rgb = powf((*rgb + 0.055) / (1.0 + 0.055), 2.4);
  } else {
    *rgb = (*rgb / 12.92);
  }
}

float crossProduct(point2d p1, point2d p2){
  return (p1.x * p2.y) - (p1.y * p2.x);
}

void getClosestPointToLine(point2d A, point2d B, point2d P, point2d *ret){
    point2d AP, AB;
    AP.x=(P.x - A.x);AP.y=(P.y - A.y); AB.x=(B.x - A.x); AB.y=(B.y - A.y);
    float ab2 = AB.x * AB.x + AB.y * AB.y, ap_ab = AP.x * AB.x + AP.y * AB.y;
    float t = ap_ab / ab2;
    if (t < 0.0){
      t = 0.0;
    } else if (t > 1.0) {
      t = 1.0;
    }
    ret->x = (A.x + AB.x * t);
    ret->y = (A.y + AB.y * t);
}

float getDistance(point2d a, point2d b)
{
    float dx = a.x - b.x, dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}

void getClosestPointToPoint(point2d gamut_r, point2d gamut_g, point2d gamut_b, float *x, float *y){
    point2d xy, pAB, pAC, pBC;
    xy.x = *x; xy.y = *y;
    getClosestPointToLine(gamut_r, gamut_g, xy, &pAB);
    getClosestPointToLine(gamut_b, gamut_r, xy, &pAC);
    getClosestPointToLine(gamut_g, gamut_b, xy, &pBC);
    float dAB = getDistance(xy, pAB), dAC = getDistance(xy, pAC), dBC = getDistance(xy, pBC);
    point2d closetPoint = pAB;
    float lowest = dAB;
    if (dAC < lowest){
        lowest = dAC;
        closetPoint = pAC;
    }

    if (dBC < lowest){
        lowest = dBC;
        closetPoint = pBC;
    }
    *x = closetPoint.x;
    *y = closetPoint.y;
}

bool checkPointInLampsReach(point2d gamut_r, point2d gamut_g, point2d gamut_b, float x, float y){
  point2d v1, v2, q;
  v1.x = gamut_g.x - gamut_r.x; v1.y = gamut_g.y - gamut_r.y;
  v2.x = gamut_b.x - gamut_r.x; v2.y = gamut_b.y - gamut_r.y;
  q.x = x - gamut_r.x; q.y = y - gamut_r.y;
  float v1Xv2 = crossProduct(v1, v2);
  float s = crossProduct(q, v2) / v1Xv2, t = crossProduct(v1, q) / v1Xv2;
  if ((s >= 0.0) && (t >= 0.0) && (s+t <= 1.0)) return true;
  return false;
}

void getGamut(int8_t gamma, point2d *gamut_r, point2d *gamut_g, point2d *gamut_b){
  //http://www.developers.meethue.com/documentation/supported-lights
  //http://www.developers.meethue.com/documentation/hue-xy-values
  //Find R, G, B in the table of XY-Values not Supported Lights
  switch(gamma){
    case 0:
      // gamut_r->x = 0.704; gamut_r->y = 0.296;
      // gamut_g->x = 0.2151; gamut_g->y = 0.7106;
      // gamut_b->x = 0.138; gamut_b->y = 0.08;
      gamut_r->x = 0.7; gamut_r->y = 0.2986;
      gamut_g->x = 0.214; gamut_g->y = 0.709;
      gamut_b->x = 0.139; gamut_b->y = 0.081;
      break;
    case 1:
      // gamut_r->x = 0.675; gamut_r->y = 0.322;
      // gamut_g->x = 0.409; gamut_g->y = 0.518;
      // gamut_b->x = 0.167; gamut_b->y = 0.04;
      gamut_r->x = 0.674; gamut_r->y = 0.322;
      gamut_g->x = 0.408; gamut_g->y = 0.517;
      gamut_b->x = 0.168; gamut_b->y = 0.041;
      break;
    case 2:
      // gamut_r->x = 0.675; gamut_r->y = 0.322;
      // gamut_g->x = 0.2151; gamut_g->y = 0.7106;
      // gamut_b->x = 0.167; gamut_b->y = 0.04;
      gamut_r->x = 0.692; gamut_r->y = 0.308;
      gamut_g->x = 0.17; gamut_g->y = 0.7;
      gamut_b->x = 0.153; gamut_b->y = 0.048;
      break;
    default:
      break;
  }
}

void RGBtoXY(int8_t gamma, uint8_t red, uint8_t green, uint8_t blue, float *x, float *y, float *bri){
  float r = (float)red/255.0, g = (float)green/255.0, b = (float)blue/255.0;
  gammaCorrection(&r); gammaCorrection(&g); gammaCorrection(&b);
  float X, Y, Z, cx, cy;
//      http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
//      sRGB D50
//        X = r * 0.4360747 + g * 0.3850649 + b * 0.0930804;
//        Y = r * 0.2225045 + g * 0.7168786 + b * 0.0406169;
//        Z = r * 0.0139322 + g * 0.0971045 + b * 0.7141733;

//      http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
//      https://en.wikipedia.org/wiki/SRGB
//      sRGB D65
//        X = r * 0.4124564 + g * 0.3575761 + b * 0.1804375;
//        Y = r * 0.2126729 + g * 0.7151522 + b * 0.0721750;
//        Z = r * 0.0193339 + g * 0.1191920 + b * 0.9503041;
    
//      http://www.developers.meethue.com/documentation/hue-xy-values
//      http://www.developers.meethue.com/documentation/color-conversions-rgb-xy
  X = r * 0.664511 + g * 0.154324 + b * 0.162028;
  Y = r * 0.283881 + g * 0.668433 + b * 0.047685;
  Z = r * 0.000088 + g * 0.072310 + b * 0.986039;
  if ((X + Y + Z) == 0.0){
    cx = cy = 0.0;
  } else {
    cx = X / (X + Y + Z);
    cy = Y / (X + Y + Z);
  }
  point2d gamut_r, gamut_g, gamut_b;
  getGamut(gamma, &gamut_r, &gamut_g, &gamut_b);
  if (!checkPointInLampsReach(gamut_r, gamut_g, gamut_b, cx, cy)){
    getClosestPointToPoint(gamut_r, gamut_g, gamut_b, &cx, &cy);
  }
  *x = cx;
  *y = cy;
  *bri = Y;
}

void XYbtoRGB(int8_t gamma, float x, float y, float bri, uint8_t *red, uint8_t *green, uint8_t *blue)
{
  point2d gamut_r, gamut_g, gamut_b;
  getGamut(gamma, &gamut_r, &gamut_g, &gamut_b);
  if (!checkPointInLampsReach(gamut_r, gamut_g, gamut_b, x, y)){
    getClosestPointToPoint(gamut_r, gamut_g, gamut_b, &x, &y);
  }
  float X, Y, Z, z = (1.0-x-y);
  Y = bri;
  X = (Y/y)*x;
  Z = (Y/y)*z;
  float r =  X * 1.656492 - Y * 0.354851 - Z * 0.255038;
  float g = -X * 0.707196 + Y * 1.655397 + Z * 0.036152;
  float b =  X * 0.051713 - Y * 0.121364 + Z * 1.011530;

  // Apply gamma correction
  r = (r <= 0.0031308) ? 12.92 * r : (1.0 + 0.055) * powf(r, (1.0 / 2.4)) - 0.055;
  g = (g <= 0.0031308) ? 12.92 * g : (1.0 + 0.055) * powf(g, (1.0 / 2.4)) - 0.055;
  b = (b <= 0.0031308) ? 12.92 * b : (1.0 + 0.055) * powf(b, (1.0 / 2.4)) - 0.055;

  if (r < 0.0) r = 0.0;
  if (g < 0.0) g = 0.0;
  if (b < 0.0) b = 0.0;

  if (r > b && r > g) {
      // red is biggest
      if (r > 1.0) {
          g = g / r;
          b = b / r;
          r = 1.0;
      }
  }
  else if (g > b && g > r) {
      // green is biggest
      if (g > 1.0) {
          r = r / g;
          b = b / g;
          g = 1.0;
      }
  }
  else if (b > r && b > g) {
      // blue is biggest
      if (b > 1.0) {
          r = r / b;
          g = g / b;
          b = 1.0;
      }
  }
  *red = (uint8_t)(r * 255.0 + 0.5);
  *green = (uint8_t)(g * 255.0 + 0.5);
  *blue = (uint8_t)(b * 255.0 + 0.5);
}

void HSVtoRGB( float *r, float *g, float *b, float h, float s, float v )
{
    int i;
    float f, p, q, t;
    if( s == 0 ) {
        // achromatic (grey)
        *r = *g = *b = v;
        return;
    }
    h /= 60;            // sector 0 to 5
    i = floor( h );
    f = h - i;          // factorial part of h
    p = v * ( 1 - s );
    q = v * ( 1 - s * f );
    t = v * ( 1 - s * ( 1 - f ) );
    switch( i ) {
        case 0:
            *r = v;
            *g = t;
            *b = p;
            break;
        case 1:
            *r = q;
            *g = v;
            *b = p;
            break;
        case 2:
            *r = p;
            *g = v;
            *b = t;
            break;
        case 3:
            *r = p;
            *g = q;
            *b = v;
            break;
        case 4:
            *r = t;
            *g = p;
            *b = v;
            break;
        default:        // case 5:
            *r = v;
            *g = p;
            *b = q;
            break;
    }
}


int socket_send_to(uint32_t ip, char *message, int total, char *response, int res_size)
{
  int portno = 80;
  struct sockaddr_in serv_addr;
  int sockfd, bytes, sent, received;

  /* create the socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
    // error("ERROR opening socket");
    return -1;

  /* fill in the structure */
  memset(&serv_addr,0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(portno);
  serv_addr.sin_addr.s_addr = htonl(ip);

  if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
    // error("ERROR connecting");
    return -2;

  /* send the request */
  sent = 0;
  do {
    bytes = write(sockfd,message+sent,total-sent);
    if (bytes < 0)
          // error("ERROR writing message to socket");
      return -3;
    if (bytes == 0)
      break;
    sent+=bytes;
  } while (sent < total);

  /* receive the response */
  memset(response,0,res_size);
  total = res_size-1;
  received = 0;
  do {
    bytes = read(sockfd,response+received,total-received);
    if (bytes < 0)
        // error("ERROR reading response from socket");
      return -4;
    if (bytes == 0)
      break;
    received+=bytes;
  } while (received < total);

  if (received == total)
    //error("ERROR storing complete response from socket");
    return -5;

  /* close the socket */
  close(sockfd);

  return 0;
}


int get_gamma(uint32_t ip, char *message, int total, int index, float *x, float *y, int *bri, bool *on){
  memset(message, 0, total);
  char path[BUF_SIZE] = {0};
  sprintf(path, philips_hue_path, index);
  sprintf(message, hue_http_get_header, path);
  int ret = socket_send_to(ip, message, strlen(message), message, total);
  if (ret < 0)
    // Socket send error 
    return ret;

  message = (strstr(message, "\r\n\r\n"))+4;
  cJSON * root = cJSON_Parse(message);
  if (!root)
    // Cannot parse
    return -100;
  cJSON* item = cJSON_GetObjectItem(root,"modelid");
  if (!item)
    // No model id available
    return -101;
  char *modelid = item->valuestring;
  if (x != NULL && y != NULL && bri != NULL){
    cJSON* state_item = cJSON_GetObjectItem(root,"state");
    if (!state_item)
      // No state available
      return -102;
    item = cJSON_GetObjectItem(state_item,"xy");
    if (!item)
      // No xy available
      return -103;
    // for (i = 0 ; i < cJSON_GetArraySize(item) ; i++)
    // {
    //    cJSON * subitem = cJSON_GetArrayItem(item, i);
    //    name = cJSON_GetObjectItem(subitem, "name");
    //    index = cJSON_GetObjectItem(subitem, "index");
    //    optional = cJSON_GetObjectItem(subitem, "optional"); 
    // }
    cJSON * subitem = cJSON_GetArrayItem(item, 0);
    if (!subitem)
      // No xy[0] available
      return -104;
    *x = (float)(subitem->valuedouble);
    subitem = cJSON_GetArrayItem(item, 1);
    if (!subitem)
      // No xy[1] available
      return -105;
    *y = (float)(subitem->valuedouble);
    subitem = cJSON_GetObjectItem(state_item,"bri");
    if (!subitem)
      // No bri available
      return -106;
    *bri = subitem->valueint;
    subitem = cJSON_GetObjectItem(state_item,"on");
    if (!subitem)
      // No on available
      return -107;
    *on = (subitem->type)?true:false;
  }

  if (strcmp(modelid, "LCT001") == 0 || 
    strcmp(modelid, "LCT002") == 0 || 
    strcmp(modelid, "LCT003") == 0 || 
    strcmp(modelid, "LCT007") == 0 || 
    strcmp(modelid, "LLM001") == 0)
  {
    ret = 1;
  }
  else if(strcmp(modelid, "LLC006") == 0 || 
    strcmp(modelid, "LLC007") == 0 || 
    strcmp(modelid, "LLC010") == 0 || 
    strcmp(modelid, "LLC011") == 0 ||
    strcmp(modelid, "LLC012") == 0 || 
    strcmp(modelid, "LLC013") == 0 || 
    strcmp(modelid, "LST001") == 0)
  {
    ret = 0;
  } 
  else if(strcmp(modelid, "LLC020") == 0 || 
    strcmp(modelid, "LST002") == 0)
  {
    ret = 2;
  }
  else
  {
    // Unknown Philips hue modelid 
    ret = -102;
  }
  cJSON_Delete(root);

  return ret;
}

int put_command(uint32_t ip, char *message, int total, int index, char *command, int cmd_len){
  memset(message, 0, total);
  char path[BUF_SIZE] = {0};
  sprintf(path, philips_hue_path, index);
  strcat(path, "/state");
  sprintf(message,hue_http_put_header,path, cmd_len);
  strcat(message, command);
  int ret = socket_send_to(ip, message, strlen(message), message, total);
  if (ret < 0)
    // Socket send error 
    return ret;

  // message = (strstr(message, "\r\n\r\n"))+4;
  if (strstr(message, "\"error\""))
    return -100;

  return 0;
}