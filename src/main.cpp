#include <Arduino.h>
#define MES_PV 34
#define MES_SHUNT 33

// Constants
#define OP_Gain 12.20
#define SHUNT_R 0.53
#define V_offset 0.3

// Prototypes
float calc_current();
float calc_voltage();
float calc_power();
void pno_algorithm();

// Variables
float last_voltage = 0;
float last_power = 0;

void setup() {
  pinMode(MES_PV, INPUT);
  pinMode(MES_SHUNT, INPUT);
  Serial.begin(115200);

  Serial.println("Starting...");
  delay(1000);
  Serial.println("Starting... done");

}

void loop() {
  pno_algorithm();
  delay(1000);
}

float calc_current(){
  int raw = analogRead(MES_SHUNT);
  float voltage_adc = (raw * (3.3 / 4096)) + V_offset;
  float voltage = voltage_adc / OP_Gain;
  float current = (voltage / SHUNT_R);
  return (current);
}

float calc_voltage(){
  int raw = analogRead(MES_PV);
  float voltage = (raw * (3.3 / 4096)) + V_offset;
  return voltage;
}

float calc_power(){
  float voltage = calc_voltage();
  float current = calc_current();
  float power = voltage * current;
  return power;
}

void pno_algorithm(){
  float voltage = calc_voltage();
  float current = calc_current();
  float power = calc_power();

  Serial.print("Voltage: ");
  Serial.print(calc_voltage());
  Serial.print("V");
  Serial.print("  Current: ");
  Serial.print(calc_current()*1000);
  Serial.print("mA");
  Serial.print("  Power: ");
  Serial.print(calc_power()*1000);
  Serial.print("mW");

  

  if ((power - last_power > 0.05) || (power - last_power < -0.05)) {

    Serial.print(" || ");

    if (power > last_power) {
      // Power increased

      if (voltage > last_voltage) {
        // Voltage increased
        Serial.println("Power increased, PV voltage increased. Increase load resistance");
      }
      else {
        // Voltage decreased
        Serial.println("Power increased, PV voltage decreased. Decrease load resistance");
      }

    }
    else {
      // Power decreased
      if (voltage > last_voltage) {
        // Voltage increased
        Serial.println("Power decreased, PV voltage increased. Decrease load resistance");
      }
      else {
        // Voltage decreased
        Serial.println("Power decreased, PV voltage decreased. Increase load resistance");
      }
    }

  }
  else {
    Serial.println(" || ");
  }

  last_voltage = voltage;
  last_power = power;
}





