#include <arduino.h>
#include <wifi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"

const char* ssid = "SSID";
const char* password = "PASS";

string servername = "XXX.XXX.XXX.XXX";

string serverpath = "/upload/upload-single-image";

const int serverport = 4000;

wificlient client;

// camera_model_ai_thinker
#define pwdn_gpio_num     32
#define reset_gpio_num    -1
#define xclk_gpio_num      0
#define siod_gpio_num     26
#define sioc_gpio_num     27

#define y9_gpio_num       35
#define y8_gpio_num       34
#define y7_gpio_num       39
#define y6_gpio_num       36
#define y5_gpio_num       21
#define y4_gpio_num       19
#define y3_gpio_num       18
#define y2_gpio_num        5
#define vsync_gpio_num    25
#define href_gpio_num     23
#define pclk_gpio_num     22

const int timerinterval = 10000;    // time between each http post image
unsigned long previousmillis = 0;   // last time image was sent

void setup() {
  write_peri_reg(rtc_cntl_brown_out_reg, 0);
  serial.begin(115200);

  wifi.mode(wifi_sta);
  serial.println();
  serial.print("connecting to ");
  serial.println(ssid);
  wifi.begin(ssid, password);  
  while (wifi.status() != wl_connected) {
    serial.print(".");
    delay(500);
  }
  serial.println();
  serial.print("esp32-cam ip address: ");
  serial.println(wifi.localip());

  camera_config_t config;
  config.ledc_channel = ledc_channel_0;
  config.ledc_timer = ledc_timer_0;
  config.pin_d0 = y2_gpio_num;
  config.pin_d1 = y3_gpio_num;
  config.pin_d2 = y4_gpio_num;
  config.pin_d3 = y5_gpio_num;
  config.pin_d4 = y6_gpio_num;
  config.pin_d5 = y7_gpio_num;
  config.pin_d6 = y8_gpio_num;
  config.pin_d7 = y9_gpio_num;
  config.pin_xclk = xclk_gpio_num;
  config.pin_pclk = pclk_gpio_num;
  config.pin_vsync = vsync_gpio_num;
  config.pin_href = href_gpio_num;
  config.pin_sscb_sda = siod_gpio_num;
  config.pin_sscb_scl = sioc_gpio_num;
  config.pin_pwdn = pwdn_gpio_num;
  config.pin_reset = reset_gpio_num;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = pixformat_jpeg;

  // init with high specs to pre-allocate larger buffers
  if(psramfound()){
    config.frame_size = framesize_svga;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = framesize_cif;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != esp_ok) {
    serial.printf("camera init failed with error 0x%x", err);
    delay(1000);
    esp.restart();
  }

  sendphoto(); 
}

void loop() {
  unsigned long currentmillis = millis();
  if (currentmillis - previousmillis >= timerinterval) {
    sendphoto();
    previousmillis = currentmillis;
  }
}

string sendphoto() {
  string getall;
  string getbody;

  camera_fb_t * fb = null;
  fb = esp_camera_fb_get();
  if(!fb) {
    serial.println("camera capture failed");
    delay(1000);
    esp.restart();
  }
  
  serial.println("connecting to server: " + servername);

  if (client.connect(servername.c_str(), serverport)) {
    serial.println("connection successful!");    
    string head = "--customboundary\r\ncontent-disposition: form-data; name=\"imagefile\"; filename=\"esp32-cam.jpg\"\r\ncontent-type: image/jpeg\r\n\r\n";
    string tail = "\r\n--customboundary--\r\n";

    uint32_t imagelen = fb->len;
    uint32_t extralen = head.length() + tail.length();
    uint32_t totallen = imagelen + extralen;
  
    client.println("post " + serverpath + " http/1.1");
    client.println("host: " + servername + ":" + serverport);
    client.println("content-length: " + string(totallen));
    client.println("content-type: multipart/form-data; boundary=customboundary");
    client.println();
    client.print(head);
  
    uint8_t *fbbuf = fb->buf;
    size_t fblen = fb->len;
    for (size_t n = 0; n < fblen; n = n + 1024) {
      if (n + 1024 < fblen) {
        client.write(fbbuf, 1024);
        fbbuf += 1024;
      }
      else if (fblen%1024>0) {
        size_t remainder = fblen%1024;
        client.write(fbbuf, remainder);
      }
    }   
    client.print(tail);
    
    esp_camera_fb_return(fb);
    
    int timouttimer = 10000;
    long starttimer = millis();
    boolean state = false;
    
    while ((starttimer + timouttimer) > millis()) {
      serial.print(".");
      delay(100);      
      while (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (getall.length()==0) { state=true; }
          getall = "";
        }
        else if (c != '\r') { getall += string(c); }
        if (state==true) { getbody += string(c); }
        starttimer = millis();
      }
      if (getbody.length()>0) { break; }
    }
    serial.println();
    client.stop();
    serial.println(getbody);
  }
  else {
    getbody = "connection to " + servername +  " failed.";
    serial.println(getbody);
  }
  return getbody;
}
