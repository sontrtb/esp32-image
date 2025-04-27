#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <LovyanGFX.hpp>
#include "LgfxConfig.h" 

#define BOOT_BUTTON_PIN 9
#define LED_PIN 8

bool apActive = false;
unsigned long lastActiveTime = 0;
const unsigned long ACTIVE_TIMEOUT = 60 * 1000;

const char *ap_ssid = "SFeDev";
const char *ap_password = "12345678";
const char *imagePath = "/uploaded_image";

WebServer server(80);
LGFX tft;

const char *uploadForm = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Image Uploader</title>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 20px;
      text-align: center;
      max-width: 600px;
      margin: 0 auto;
    }
    .container {
      border: 1px solid #ddd;
      padding: 20px;
      border-radius: 10px;
      margin-top: 20px;
    }
    h1 {
      color: #333;
    }
    input[type="file"] {
      margin: 20px 0;
    }
    input[type="submit"] {
      background-color: #4CAF50;
      color: white;
      padding: 12px 20px;
      border: none;
      border-radius: 4px;
      cursor: pointer;
    }
    input[type="submit"]:hover {
      background-color: #45a049;
    }
    #status {
      margin-top: 20px;
      padding: 10px;
    }
    .success {
      color: green;
    }
    .error {
      color: red;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>SFeDev</h1>
    <form method="POST" action="/upload" enctype="multipart/form-data" id="upload_form">
      <input type="file" name="image" accept="image/jpeg,image/png" id="file">
      <br>
      <input type="submit" value="Upload">
    </form>
    <div id="status"></div>
  </div>
  
  <script>
  document.getElementById('upload_form').onsubmit = function (e) {
    e.preventDefault();

    const file = document.getElementById('file').files[0];
    if (!file) {
      document.getElementById('status').innerHTML = '<p class="error">Vui lòng chọn file ảnh!</p>';
      return false;
    }

    const img = new Image();
    const reader = new FileReader();

    reader.onload = function (event) {
      img.src = event.target.result;
    };

    img.onload = function () {
      const targetWidth = 240;
      const targetHeight = 280;

      // Tạo canvas trung gian
      const canvas = document.createElement('canvas');
      canvas.width = targetWidth;
      canvas.height = targetHeight;

      const ctx = canvas.getContext('2d');

      // Tính toán tỉ lệ và cắt ảnh
      const aspectRatio = img.width / img.height;
      const targetRatio = targetWidth / targetHeight;

      let sx, sy, sw, sh;

      if (aspectRatio > targetRatio) {
        // Ảnh quá rộng => cắt chiều ngang
        sh = img.height;
        sw = sh * targetRatio;
        sx = (img.width - sw) / 2;
        sy = 0;
      } else {
        // Ảnh quá cao => cắt chiều dọc
        sw = img.width;
        sh = sw / targetRatio;
        sx = 0;
        sy = (img.height - sh) / 2;
      }

      ctx.drawImage(img, sx, sy, sw, sh, 0, 0, targetWidth, targetHeight);

      // Xác định định dạng file
      const fileType = file.type;
      const isJpeg = fileType === 'image/jpeg';
      const isPng = fileType === 'image/png';
      
      // Convert canvas to blob để upload
      canvas.toBlob(function (blob) {
        const formData = new FormData();
        formData.append('image', blob, file.name);
        formData.append('type', isJpeg ? 'jpg' : (isPng ? 'png' : 'unknown'));

        const xhr = new XMLHttpRequest();
        xhr.open('POST', '/upload', true);

        xhr.onload = function () {
          if (xhr.status === 200) {
            document.getElementById('status').innerHTML = '<p class="success">Upload thành công!</p>';
          } else {
            document.getElementById('status').innerHTML = '<p class="error">Upload thất bại!</p>';
          }
        };

        xhr.send(formData);
        document.getElementById('status').innerHTML = '<p>Đang upload...</p>';
      }, fileType);
    };

    reader.readAsDataURL(file);
  };
</script>

</body>
</html>
)rawliteral";

void startAccessPointAndServer() {
  if (apActive) return;

  digitalWrite(LED_PIN, LOW);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  
  IPAddress myIP = WiFi.softAPIP();
  Serial.println(myIP);

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_LIGHT_BLUE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Web Server Started");
  tft.setCursor(10, 40);
  tft.print("SSID: ");
  tft.println(ap_ssid);
  tft.setCursor(10, 70);
  tft.print("Password: ");
  tft.println(ap_password);
  tft.setCursor(10, 100);
  tft.print("IP: ");
  tft.println(myIP.toString());

  apActive = true;
  lastActiveTime = millis();
}

void stopAccessPointAndServer() {
  if (!apActive) return;

  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("Đã tắt Access Point để tiết kiệm pin.");

  digitalWrite(LED_PIN, HIGH);
  apActive = false;
}

String getContentType(String filename) {
  if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) return "image/jpeg";
  else if (filename.endsWith(".png")) return "image/png";
  return "text/plain";
}

String fileExtension;

void handleRoot()
{
  server.send(200, "text/html", uploadForm);
}

void displayImage(const char *path)
{
  File imgFile = SPIFFS.open(path, "r");
  if (!imgFile)
  {
    Serial.println("Không mở được file ảnh!");
    return;
  }

  int screenWidth = tft.width();
  int screenHeight = tft.height();

  tft.fillScreen(TFT_BLACK);

  if (String(path).endsWith(".jpg") || String(path).endsWith(".jpeg")) {
    tft.drawJpgFile(SPIFFS, path, 0, 0, screenWidth, screenHeight, 0, 0);
    Serial.println("Đã hiển thị ảnh JPG co giãn vừa màn hình!");
  } 
  else if (String(path).endsWith(".png")) {
    tft.drawPngFile(SPIFFS, path, 0, 0, screenWidth, screenHeight, 0, 0);
    Serial.println("Đã hiển thị ảnh PNG co giãn vừa màn hình!");
  }

  imgFile.close();
}

void handleUpload()
{
  HTTPUpload &upload = server.upload();
  static String finalPath;

  if (upload.status == UPLOAD_FILE_START)
  {
    fileExtension = ".jpg";
    
    String filename = upload.filename;
    if (filename.endsWith(".png")) {
      fileExtension = ".png";
    }
    
    finalPath = String(imagePath) + fileExtension;
    
    if (SPIFFS.exists(finalPath)) {
      SPIFFS.remove(finalPath);
    }

    File file = SPIFFS.open(finalPath, FILE_WRITE);
    if (!file)
    {
      server.send(500, "text/plain", "Không thể ghi file!");
      return;
    }
    file.close();
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    // Ghi dữ liệu vào file
    finalPath = String(imagePath) + fileExtension;
    File file = SPIFFS.open(finalPath, FILE_APPEND);
    if (file)
    {
      file.write(upload.buf, upload.currentSize);
      file.close();
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    finalPath = String(imagePath) + fileExtension;
    
    if (SPIFFS.exists(finalPath)) {
      displayImage(finalPath.c_str());
      server.send(200, "text/plain", "Upload thành công!");
    } else {
      server.send(500, "text/plain", "Lưu file thất bại!");
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); 

  if (!SPIFFS.begin(true)) {
    Serial.println("Lỗi SPIFFS");
    return;
  }

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/upload", HTTP_POST, []() {
    server.send(200, "text/plain", "Upload thành công!");
  }, handleUpload);

  startAccessPointAndServer();
  server.begin();
}

void loop() {
  if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
    delay(200);
    startAccessPointAndServer();
    server.begin();
  }

  if (apActive) {
    server.handleClient();

    if (server.client().connected()) {
      lastActiveTime = millis();
    }

    if (millis() - lastActiveTime > ACTIVE_TIMEOUT) {
      stopAccessPointAndServer();
    }
  }
}