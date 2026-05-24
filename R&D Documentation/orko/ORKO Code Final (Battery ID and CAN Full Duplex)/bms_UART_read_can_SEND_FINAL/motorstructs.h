

/**
 * @brief Struct for storing motor controller data
 */
struct controllerData {
    bool readSuccess = 0;
    int gear;
    bool brake;
    int bikeSpeed;
};
 
typedef struct controllerData bikeDataStruct;