#ifndef BATTERYSTRUCTS_H
#define BATTERYSTRUCTS_H

//EEPROM Addresses

//DEBUGGING ADDRESSES
#define DEBUG_ADDRESS 65
#define CAN_ENABLE 67
#define WEBPAGE_ENABLE 68

//Other params
#define MAXDEBUGLVL 2

/**
 * @brief Struct created for DALY BMS ID 0x90
 * It will fetch Voltage, current and SoC
 */
struct BatteryData90 {
  unsigned int Voltage = 0;
  unsigned int Current = 0;
  unsigned int SOC = 0;
  bool readSuccess90 = 0;
};

/**
 * @brief Struct created for DALY BMS ID 0x91
 * It will fetch max cell voltage, min cell voltage, no of cells with max voltage and no of cells with min voltage
 */
struct BatteryData91 {
  uint16_t max_cell_mv = 0;  //2 bytes
  uint8_t no_max_cell_v = 0;
  uint16_t min_cell_mv = 0;
  uint8_t no_min_cell_v = 0;
  bool readSuccess91 = 0;
};

/**
 * @brief Struct created for DALY BMS ID 0x92
 * It will fetch max temperature, min temperature, cell no with max temperature and cell no with min temperature
 */
struct BatteryData92 {
  uint8_t max_temp = 0;      //1 byte
  uint8_t max_temp_cell_no = 0;
  uint8_t min_temp = 0;
  uint8_t min_temp_cell_no = 0;
  bool readSuccess92 = 0;
};

/**
 * @brief Struct created for DALY BMS ID 0x93
 * It will fetch charge state of the mosfet, discharge state of the mosfet, BMS_life, BMS life and remaining battery capacity in mah
 */
struct BatteryData93 {
  uint8_t state = 0;
  uint8_t charge_mos_state = 0;
  uint8_t discharge_mos_state = 0;
  uint8_t BMS_life = 0;
  uint32_t remaining_capacity_mah = 0;
  bool readSuccess93 = 0;
};

/**
 * @brief Struct created for DALY BMS ID 0x94
 * It will fetch no of battery string, charger status, load status and DIO state
 */
struct BatteryData94 {
  uint8_t no_battery_string = 0;
  uint8_t no_temperature = 0;
  uint8_t charger_status = 0;
  uint8_t load_status = 0;
  uint8_t DIO_state = 0;
  bool readSuccess94 = 0;
};

/**
 * @brief Struct created for DALY BMS ID 0x95
 * It will fetch individual cell voltages
 */
struct BatteryData95 {
  //uint16_t cellVoltages[20];
  bool readSuccess95 = 0;
};

/**
 * @brief Struct created for DALY BMS ID 0x96
 * It will fetch cell temperature
 */
struct BatteryData96 {
  uint8_t cell_t_frame_no = 0;
  uint8_t cell_temp = 0;
  bool readSuccess96 = 0;
};


struct BatteryData97 {
  uint64_t cell_bal_state = 0;
  bool readSuccess97 = 0;
};

/**
 * @brief Struct created for DALY BMS ID 0x98
 * It will fetch battery fail status
 */
struct BatteryData98 {
  uint64_t bat_fail_status = 0;
  bool readSuccess98 = 0;
};

typedef struct BatteryData90 bmsData90;
typedef struct BatteryData91 bmsData91;
typedef struct BatteryData92 bmsData92;
typedef struct BatteryData93 bmsData93;
typedef struct BatteryData94 bmsData94;
typedef struct BatteryData95 bmsData95;
typedef struct BatteryData96 bmsData96;
typedef struct BatteryData97 bmsData97;
typedef struct BatteryData98 bmsData98;

#endif // BATTERYSTRUCTS_H
