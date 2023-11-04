//************************************************************
// this is a simple example that measures battery voltage
// and draws a battery icon
// 
// Make sure to edit TFT_eSI\User_Setup_Select.h
// * comment //#include <User_Setup.h>
// * Uncomment //#include <User_Setups/Setup25_TTGO_T_Display.h>    // Setup file for ESP32 and TTGO T-Display ST7789V SPI bus TFT
//************************************************************
//#include <Arduino.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <driver/adc.h>
#include "esp_adc_cal.h"

TFT_eSPI tft = TFT_eSPI();  // Create an instance of the display

// ADC calibration characteristics
esp_adc_cal_characteristics_t adc_chars;

// Battery measurement parameters
const float batteryMaxVoltage = 4.2; // Maximum voltage of your battery
const float batteryMinVoltage = 3.2; // Minimum voltage of your battery when it's considered empty
const float voltageMultiplier = 5.26; // Voltage divider multiplier

// Function prototype for calculateBatteryPercentage
int calculateBatteryPercentage(float voltage);

void setup() {
  Serial.begin(115200);
  
  // Initialize TFT
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  // Calibrate ADC
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC_ATTEN_DB_2_5, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);

  pinMode(14, OUTPUT);
}


void drawBatteryIcon(int percentage) {
  // Define the size and position of the battery icon
  int x = 170; // X position of the battery icon
  int y = 10; // Y position of the battery icon
  int width = 40; // Width of the battery icon
  int height = 20; // Height of the battery icon
  int gap = 2; // Gap for the battery 'cap'

  // Clear the area where the battery icon will be drawn
  tft.fillRect(x, y - gap, width + gap, height + (2 * gap), TFT_BLACK);

  // Draw the battery 'cap'
  tft.fillRect(x + width, y + (height / 4), gap, height / 2, TFT_WHITE);

  // Draw the battery outline
  tft.drawRect(x, y, width, height, TFT_WHITE);

  // Calculate the width of the inner rectangle based on the percentage
  int innerWidth = (percentage * (width - 2)) / 100;

  // Draw the inner rectangle based on the percentage
  tft.fillRect(x + 1, y + 1, innerWidth, height - 2, percentage > 20 ? TFT_GREEN : TFT_RED);

  // Optionally, you can add text on top of the battery icon to show the exact percentage
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(x + width + 5, y + (height / 2) - 4);
  tft.printf("%d%%", percentage);
}

void loop() {
  digitalWrite(14, HIGH);
  delay(1);
  uint32_t adc_reading = analogRead(34);
  digitalWrite(14, LOW);

   // Convert adc_reading to voltage in mV
  uint32_t voltage_mV = esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars);
  float battery_voltage = (voltage_mV / 1000.0) * voltageMultiplier; // Convert mV to V and apply the voltage divider multiplier

  // TODO: Adjust the voltageMultiplier based on your voltage divider circuit
  // For example, if R1 = 100k and R2 = 100k, multiplier should be (R1 + R2) / R2 = 2
  // If your multimeter reads 3.7V and the ESP reads 5.2V, the multiplier might be too high.


  // Calculate battery percentage
  int battery_percentage = calculateBatteryPercentage(battery_voltage);

  // Display the voltage and percentage on the TFT
  tft.fillScreen(TFT_BLACK); // Clear the screen
  // Draw the battery icon with the current battery percentage
  drawBatteryIcon(battery_percentage);

  delay(5000); // Update every 5 seconds
}

int calculateBatteryPercentage(float voltage) {
  int percentage = (int)((voltage - batteryMinVoltage) / (batteryMaxVoltage - batteryMinVoltage) * 100);
  percentage = constrain(percentage, 0, 100); // Constrain values to between 0 and 100
  return percentage;
}
