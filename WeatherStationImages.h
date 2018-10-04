const uint8_t activeSymbole[] = {
  B00000000,
  B00000000,
  B00011000,
  B00100100,
  B01000010,
  B01000010,
  B00100100,
  B00011000
};

const uint8_t inactiveSymbole[] = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00011000,
  B00011000,
  B00000000,
  B00000000
};

// define c0c2ac37ec5e4ca883599460dfb0490e_width 66
// define c0c2ac37ec5e4ca883599460dfb0490e_height 64
static unsigned char garfield[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0xFC, 0x07, 0x00, 0x00,
  0x00, 0x80, 0xFF, 0x03, 0x80, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0xC0, 0xFF,
  0x0F, 0xC0, 0xE1, 0x3F, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0x1D, 0x70, 0xF8,
  0x3F, 0x00, 0x00, 0x00, 0xE0, 0xFF, 0x73, 0x1C, 0x7C, 0x7F, 0x00, 0x00,
  0x00, 0xE0, 0xFB, 0xC7, 0x0F, 0x9F, 0x7F, 0x00, 0x00, 0x00, 0xE0, 0xB7,
  0x8F, 0x83, 0xCF, 0x7D, 0x00, 0x00, 0x00, 0xE0, 0x66, 0x0F, 0xC1, 0x63,
  0x3E, 0x00, 0x00, 0x00, 0xE0, 0x0D, 0x1E, 0x60, 0x00, 0x3F, 0x00, 0x00,
  0x00, 0xE1, 0x01, 0x34, 0x30, 0x00, 0x39, 0x00, 0x00, 0x00, 0xE7, 0x03,
  0x20, 0x18, 0x00, 0x1C, 0x07, 0x00, 0x60, 0xCE, 0x03, 0x60, 0xD8, 0x01,
  0x9E, 0x17, 0x00, 0xE0, 0xCD, 0x80, 0xFF, 0xFC, 0x07, 0xCE, 0x3F, 0x00,
  0xC0, 0x9B, 0xE1, 0xF9, 0x1D, 0x0E, 0xCE, 0x3E, 0x00, 0xB8, 0x97, 0x33,
  0x80, 0x0F, 0x38, 0x66, 0x0F, 0x00, 0xF8, 0xB9, 0x1F, 0x00, 0x03, 0x60,
  0xFF, 0x7B, 0x00, 0xE0, 0xFF, 0x0F, 0x00, 0x03, 0xC0, 0xFF, 0xFE, 0x00,
  0x00, 0xFE, 0x06, 0x00, 0x03, 0x80, 0xFC, 0x7F, 0x00, 0x00, 0x7C, 0x06,
  0x00, 0x03, 0x80, 0xF9, 0x01, 0x00, 0x00, 0x7E, 0x03, 0x00, 0x03, 0x00,
  0xFD, 0x01, 0x00, 0x00, 0x7E, 0x01, 0x00, 0x02, 0x00, 0xFB, 0x03, 0x00,
  0x00, 0xFF, 0x01, 0x00, 0x03, 0x00, 0xF2, 0x07, 0x00, 0x80, 0xBF, 0x01,
  0x00, 0x02, 0x00, 0x66, 0x0F, 0x00, 0xC0, 0x9F, 0x00, 0x00, 0x03, 0x00,
  0x06, 0x1D, 0x00, 0xE0, 0xCD, 0x00, 0x00, 0x01, 0x00, 0x84, 0x1C, 0x00,
  0xF0, 0xC0, 0x00, 0x00, 0x03, 0x00, 0xFC, 0x3F, 0x00, 0x30, 0xFF, 0x00,
  0x00, 0x03, 0x00, 0x1C, 0x3C, 0x00, 0xF8, 0xF3, 0x00, 0x00, 0x01, 0x00,
  0x0C, 0x78, 0x00, 0xF8, 0xE0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0xF8, 0x00,
  0x7C, 0xC0, 0x00, 0x00, 0x01, 0x00, 0x0C, 0xF8, 0x00, 0x3C, 0xC0, 0x00,
  0x00, 0x03, 0x00, 0x0C, 0xF8, 0x01, 0x3E, 0x40, 0x00, 0x00, 0x03, 0x00,
  0xCC, 0xF8, 0x01, 0x3E, 0x44, 0x00, 0x00, 0x03, 0x00, 0xCC, 0xF8, 0x01,
  0x3E, 0x46, 0x00, 0x00, 0x01, 0x00, 0xEC, 0xAE, 0x03, 0x3E, 0xCE, 0x00,
  0x00, 0x03, 0x00, 0xC6, 0x06, 0x03, 0xFE, 0xCC, 0x00, 0x00, 0x03, 0x00,
  0xC6, 0xC6, 0x03, 0xDF, 0x86, 0x17, 0x00, 0x03, 0xFE, 0x87, 0x86, 0x01,
  0xC2, 0x86, 0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0x80, 0x03, 0x8F, 0x82, 0x01,
  0xFF, 0xFF, 0x01, 0x83, 0xE1, 0x03, 0x1E, 0x02, 0x01, 0x7C, 0x3F, 0x00,
  0x83, 0xE1, 0x03, 0x5E, 0x03, 0x03, 0x10, 0x19, 0x80, 0x81, 0xE1, 0x03,
  0x7F, 0x02, 0x06, 0x00, 0x03, 0xC0, 0x80, 0xE1, 0x01, 0xFE, 0x06, 0x0C,
  0xE0, 0x0F, 0x60, 0x80, 0xE1, 0x01, 0x7E, 0x06, 0x38, 0x78, 0x3C, 0x38,
  0xC0, 0xF0, 0x01, 0x3F, 0x06, 0xF0, 0x1F, 0xF0, 0x1F, 0xC0, 0xE0, 0x00,
  0x1E, 0x0C, 0x00, 0x3B, 0xB8, 0x01, 0xC0, 0xC0, 0x00, 0x16, 0x0C, 0x00,
  0xE0, 0x1F, 0x00, 0x60, 0xC0, 0x00, 0x0C, 0x18, 0x00, 0xC0, 0x03, 0x00,
  0x70, 0xF0, 0x00, 0xBC, 0x30, 0x00, 0xC0, 0x07, 0x00, 0x38, 0x70, 0x00,
  0xF8, 0x61, 0x00, 0x60, 0x0E, 0x00, 0x1E, 0x7C, 0x00, 0xF8, 0xC3, 0x01,
  0x38, 0x7C, 0xC0, 0x07, 0x3E, 0x00, 0xF0, 0x03, 0xFF, 0x0F, 0xE0, 0xFF,
  0x01, 0x1F, 0x00, 0xE0, 0x07, 0xFC, 0x07, 0x00, 0x1D, 0x80, 0x0F, 0x00,
  0xC0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x80, 0x0F, 0x00, 0xC0, 0x07, 0x00,
  0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00,
  0xC0, 0x01, 0x00, 0x00, 0x3E, 0x03, 0x00, 0x00, 0x00, 0xF9, 0x00, 0x00,
  0x00, 0xF8, 0x3F, 0x00, 0x00, 0xE0, 0x3F, 0x00, 0x00, 0x00, 0xC0, 0xFF,
  0x07, 0x80, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0xFF, 0x0F,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


