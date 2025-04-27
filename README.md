# Hiên thị hình ảnh qua wifi

## 1.Tuỳ chỉnh bo mạch phù hợp trong file platformio.ini
 - Hiện tại mặc định là Esp32c3

## 2.Tuỳ chỉnh kích thước màn hình và chân kết nối
 - Trong file: "DisplayConfig.h"
```
// Display pins
#define TFT_CS 0
#define TFT_DC 1
#define TFT_RST 2
#define TFT_MOSI 3
#define TFT_SCLK 4

// Display dimensions
#define TFT_WIDTH 240
#define TFT_HEIGHT 280
```

## 3.Nếu màn hình bị ngược màu, lệch 
 - Tuỳ chỉnh trong file: "LgfxConfig.h"
```
    cfg.offset_x = 0;   // lệch theo chiều ngang
    cfg.offset_y = 20;  // lệch theo chiều dọc
    cfg.rgb_order = false; 
    cfg.invert = true; // ngược màu
```

## 4.Hướng dẫn sử dụng
 - Khi nhấn nút boot trên bo mạch sẽ vào trang thái thiết lập, esp sẽ phát wifi SFeDev, đèn màu xanh sẽ sáng
 - Sau khi kết nối, truy cập đường dẫn 102.168.4.1 để tải ảnh
 - Wifi sẽ tắt sau 2 phút để giảm tiêu thụ điện
 - Nếu muốn đổi ảnh khác, hãy nhấn nút boot (đèn xanh sẽ sáng) hoặc reset để quay lại Bước 1# esp32-image
