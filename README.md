# BUILD

```
. setup.sh
idf.py set-target esp32c6
idf.py reconfigure
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
```

# TODO
- Joystick 
  - shows both battery levels
  - robot selection screen (at startup)
  - Gatt server re-connect

- Test ADC voltage measurement?
- Motor direction flip bits (from cmd?)
  - Right/Left motor drive range instead?
  - Joystick mixes?
- UniqueID for robodrive in GattServer

- Command Repeat rate
- Remove debug prints
- State to wait for zero after reset?
- Reconnect from Joystick
- Red LED on drive
