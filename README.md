# BUILD

```
. setup.sh
idf.py set-target esp32c6
idf.py -B build_revA -DROBODRIVE_BOARD_REV=A build
idf.py -B build_revA -p /dev/ttyACM0 flash monitor
```


```
idf.py -B build_revA -DROBODRIVE_BOARD_REV=A build
idf.py -B build_revB -DROBODRIVE_BOARD_REV=B build
```


The board revision (`A` = DRV8424, `B` = 2x DRV8871) is required and is remembered per
build directory, so keep one build directory per revision (`build_revA`, `build_revB`).
The firmware logs `Robodrive board rev X` at boot.

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
