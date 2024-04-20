#ifndef __LSBT_H__
#define __LSBT_H__

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <Adafruit_LSM303.h>
#include <L3G.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

constexpr char *SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
constexpr char *CHARACTERISTIC_UUID = "e68da052-33c2-4814-8793-60112fe6570a";

class LSBT {
 public:
  class LSBTServerCallbacks: public BLEServerCallbacks {
   public: 
    bool device_connected_ = false;
    bool old_device_connected_ = false;
    void onConnect(BLEServer* p_server);
    void onDisconnect(BLEServer* p_server);
  };


  BLEServer* p_server_ = nullptr;
  BLECharacteristic* p_characteristic_ = nullptr;
  LSBTServerCallbacks *p_scallbacks_ = nullptr;
  
  uint8_t data_out_[18];

  void transform_data(lsm303MagData mag, lsm303AccelData acc, L3G::vector<int16_t> g);

  void setup();
  void publish(lsm303MagData mag, lsm303AccelData acc, L3G::vector<int16_t> g);
  
};

#endif //__LSBT_H__
