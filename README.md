# SUGV-VSLAM
## typical command:
### compile .c file

run following command to compile "MyWheel.c"

```shell
gcc -o MyWheel MyWheel.c -I/home/upsquared/libmodbus/install/include/modbus -L/home/upsquared/libmodbus/install/lib -lmodbus
```

> the library path is "/home/upsquared/libmodbus/install"



### turn on HUAWEI Usb E8372

>  noted that usb_modeswitch can be installed by "apt-get install usb_modeswitch"

- First, open file:

  ```shell
  /etc/usb_modeswitch.d/12d1:1f01
  ```

  add the following configuration:

  ```shell
  ###################################################
  #Huawei E8372
  #Contributed by: ozonejunkie
  
  DefaultVendor=0x12d1
  DefaultProduct=0x1f01
  
  TargetVendor=0x12d1
  TargetProduct=0x14db
  
  MessageContent="55534243123456780000000000000a11062000000000000100000000000000"
  NoDriverLoading=1
  ```

  

- second, open file:

  ```shell
  /lib/udev/rules.d/40-usb_modeswitch.rules
  ```

  add the following configuration:

  ```shell
  # Huawei E8372
  ATTRS{idVendor}=="12d1", ATTRS{idProduct}=="1f01", RUN+="usb_modeswitch '%b/%k'"
  ```

  

- then run:

  ```shell
  sudo usb_modeswitch -c /etc/usb_modeswitch.d/12d1:1f01
  ```

  finished.

