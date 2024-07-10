// <<<<<<<<<<=========INA219: 0x42===========>>>>>>>>>>

#define INA219_ADDRESS 0x42


float shuntVoltage_mV = 0.0;
float loadVoltage_V = 0.0;
float busVoltage_V = 0.0;
float current_mA = 0.0;
float power_mW = 0.0; 
bool ina219_overflow = false;

void InitINA219();

void InaDataUpdate();