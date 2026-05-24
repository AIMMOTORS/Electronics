#ifndef BATTERYSTRUCTS_H
#define BATTERYSTRUCTS_H

//EEPROM Addresses
#define NUM_TRIPS 1
//#define NUM_SPEED_IN_TRIP 5
#define EEPROM_DISTANCE 9
#define AVG_SPEED_THIS_T 12
//#define AVG_SPEED_LAST_T 16
//#define NEW_SPEED_T 20
#define VOLTAGE 21
#define MAX_TEMP 22
#define NUM_SWAP_TRIPS 23
#define CURRENT_SOC 27
#define THIS_ST_START_SOC 31
#define LAST_ST_START_SOC 35
#define LAST_ST_FINAL_SOC 39
#define DIST_START_ST 43
//#define KMS_THIS_ST 47
#define KMS_LAST_ST 51
//#define RANGE_LAST_ST 55
//#define AVG_RANGE 59

//DEBUGGING ADDRESSES
#define DEBUG_ADDRESS 65
#define MOTOR_UART_ENABLE 66
#define CAN_ENABLE 67
#define APP_ENABLE 68

//Other params
//#define MAXDEBUGLVL 2
#define MAXDEBUGLVL 3
struct appData{
  uint32_t num_trips;
  uint32_t num_speed_in_trip;
  uint32_t distance;
  uint32_t eeprom_distance;
  uint32_t avg_speed_this_t;
  uint32_t num_swap_trips;
  float current_soc;
  uint32_t this_st_start_soc;
  uint32_t last_st_start_soc;
  uint32_t last_st_final_soc;
  uint32_t dist_start_st;
  uint32_t kms_this_st;
  uint32_t kms_last_st;
  //uint32_t new_speed_t;
   uint32_t bikeSpeed;
  uint32_t Voltage;
  uint32_t max_temp;
  };
  
typedef struct appData appdatastruct;

struct controllerData {
    bool readSuccess = 0;
    int gear;
    bool brake;
    int bikeSpeed;
    
};
 
typedef struct controllerData bikeDataStruct;


//Battery Structs
struct batteryData {
  float Voltage = 0;
  float Current = 0;
  float SOC = 0;
  bool readSuccess=0;
  
};

typedef struct batteryData bmsbasic;

struct detailedBatteryData1 {
  uint16_t max_cell_mv = 0;
  uint8_t no_max_cell_v = 0;
  uint16_t min_cell_mv = 0;
  uint8_t no_min_cell_v = 0;
  uint8_t max_temp = 0;
  uint8_t max_temp_cell_no = 0;
  uint8_t min_temp = 0;
  uint8_t min_temp_cell_no = 0;
  uint8_t state = 0;
  uint8_t charge_mos_state = 0;
  uint8_t discharge_mos_state = 0;
  uint8_t BMS_life = 0;
  uint32_t remaining_capacity_mah = 0;
  bool readSuccess91=0;
  bool readSuccess92=0;
  bool readSuccess93=0;
};

typedef struct detailedBatteryData1 bmsData1;

struct detailedBatteryData2{
  uint8_t no_battery_string=0;
  uint8_t no_temperature=0;
  uint8_t charger_status=0;
  uint8_t load_status=0;
  uint8_t DIO_state=0;
  uint8_t cell_v_frame_no1 = 0;
  uint16_t cell_voltage1f1=0;
  uint16_t cell_voltage2f1=0;
  uint16_t cell_voltage3f1=0;
  uint8_t cell_v_frame_no2 = 0;
  uint16_t cell_voltage1f2=0;
  uint16_t cell_voltage2f2=0;
  uint16_t cell_voltage3f2=0;
  uint8_t cell_v_frame_no3 = 0;
  uint16_t cell_voltage1f3=0;
  uint16_t cell_voltage2f3=0;
  uint16_t cell_voltage3f3=0;
  uint8_t cell_t_frame_no = 0;
  uint64_t cell_temp=0;
  uint64_t cell_bal_state = 0;
  uint64_t bat_fail_status = 0;
  bool readSuccess=0;
};

typedef struct detailedBatteryData2 bmsData2;

#endif // BATTERYSTRUCTS_H
