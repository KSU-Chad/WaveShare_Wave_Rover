#include "battery_ctrl.h"
#include <INA219_WE.h>

INA219_WE ina219 = INA219_WE(INA219_ADDRESS);

void InitINA219(){
  if(!ina219.init()){
    Serial.println("INA219 not connected!");
  }
  ina219.setADCMode(BIT_MODE_9);
  ina219.setPGain(PG_320);
  ina219.setBusRange(BRNG_16);
  ina219.setShuntSizeInOhms(0.01); // used in INA219.
}

void InaDataUpdate(){
  shuntVoltage_mV = ina219.getShuntVoltage_mV();
  busVoltage_V = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getBusPower();
  loadVoltage_V  = busVoltage_V + (shuntVoltage_mV/1000);
  ina219_overflow = ina219.getOverflow();
  // INA219_DATA_V = loadVoltage_V;
  //Serial.println(loadVoltage_V);
}