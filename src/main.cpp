#include <Arduino.h>
// Pins
#define MES_PV 34
#define MES_SHUNT 33
#define LOAD_PWM 26

// Constants
#define OP_Gain 12.20
#define SHUNT_R 0.53
#define V_offset 0.3

#define PWM_FREQ 10000
#define PWM_RES 8

#define RDS_stepsize 1

// Prototypes
float calc_current();
float calc_voltage();
float calc_power();
int duty_cycleinvert(int duty_cycle);
void pno_algorithm();
void set_RDS(int RDS);
void clamp_RDS();

// Variables
float last_voltage = 0;
float last_power = 0;
int RDS = 50;

void setup() {
  pinMode(MES_PV, INPUT);
  pinMode(MES_SHUNT, INPUT);
  pinMode(LOAD_PWM, OUTPUT);

  ledcSetup(2, PWM_FREQ, PWM_RES);
  ledcAttachPin(LOAD_PWM, 2);

  Serial.begin(115200);

  Serial.println("Starting...");
  delay(1000);
  Serial.println("Starting... done");

}

void loop() {
  pno_algorithm();
  delay(250);
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

int duty_cycleinvert(int duty_cycle){
  int inverted_duty_cycle = 255 - duty_cycle;
  return inverted_duty_cycle; 
}

void clamp_RDS(){
  if (RDS > 100) {
    RDS = 100;
  }
  else if (RDS < 0) {
    RDS = 0;
  }
}

void set_RDS(int RDS){
  int duty_cycle = map(RDS, 0, 100, 255, 0);
  ledcWrite(2, duty_cycleinvert(duty_cycle));
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
  Serial.print("  RDS: ");
  Serial.print(RDS);
  Serial.print("  Power: ");
  Serial.print(calc_power()*1000);
  Serial.print("mW");

  

  if ((power - last_power > 0.005) || (power - last_power < -0.005)) {

    Serial.print(" || ");

    if (power > last_power) {
      // Power increased

      if (voltage > last_voltage) {
        // Voltage increased
        Serial.println("Power increased, PV voltage increased. Increase load resistance");
        RDS = RDS + RDS_stepsize;
      }
      else {
        // Voltage decreased
        Serial.println("Power increased, PV voltage decreased. Decrease load resistance");
        RDS = RDS - RDS_stepsize;
      }

    }
    else {
      // Power decreased
      if (voltage > last_voltage) {
        // Voltage increased
        Serial.println("Power decreased, PV voltage increased. Decrease load resistance");
        RDS = RDS - RDS_stepsize;
      }
      else {
        // Voltage decreased
        Serial.println("Power decreased, PV voltage decreased. Increase load resistance");
        RDS = RDS + RDS_stepsize;
      }
    }

  }
  else {
    Serial.print(" || ");
    Serial.println(" No change in power, change resistance ");
    RDS = RDS - RDS_stepsize;
  }

  clamp_RDS();
  set_RDS(RDS);
  last_voltage = voltage;
  last_power = power;
}





