#pragma once
#include <LovyanGFX.hpp>
#include <DisplayConfig.h>

#ifndef VSPI_HOST
#define VSPI_HOST 2  // Định nghĩa VSPI_HOST nếu nó không tồn tại
#endif

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI _bus_instance;

public:
  LGFX() {
    // Bus configuration
    {
      auto cfg = _bus_instance.config();
      // Match these with your current pins
      cfg.spi_mode = 0;
      cfg.freq_write = 60000000; // Match your current SPI speed
      cfg.pin_sclk = TFT_SCLK;   // Use your current pin definitions
      cfg.pin_mosi = TFT_MOSI;
      cfg.pin_miso = -1;         // Set to actual pin if MISO is used, otherwise -1
      cfg.pin_dc = TFT_DC;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    // Panel configuration
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = TFT_CS;
      cfg.pin_rst = TFT_RST;
      cfg.pin_busy = -1; 
      
      cfg.panel_width = TFT_WIDTH;
      cfg.panel_height = TFT_HEIGHT;
      
      cfg.memory_width = TFT_WIDTH;
      cfg.memory_height = TFT_HEIGHT;
      
      cfg.offset_x = 0;
      cfg.offset_y = 20;
      cfg.rgb_order = false;
      cfg.invert = true;
      
      _panel_instance.config(cfg);
    }
    
    setPanel(&_panel_instance);
  }
};