# GLHP Trailer

## Libraries needed:

* OneWire.h: From Arduino Database
* SPI.h: From Arduino Database
* DallasTemperature.h: From Arduino Database or from Miles Burton's website
* AD5422: See DAC references

#### For DS18B20 Calibration
See commit: [5575ebc][DSCAL]


## References

##### AD5422:
Example from Analog Devices for Microblaze processor:[AD Github][ADGIT]

AVR Example: [Github][AVREX]

Other Example: [Github][OTHER] <- See AD5422 .c/.h files

##### DS18B20:   
DallasTemperature.h:[Miles Burton's website][MILES]

[ADGIT]: <https://github.com/analogdevicesinc/no-OS/tree/master/device_drivers/AD5421>
[AVREX]: <https://github.com/d-wsky/DAC-AD5420>
[OTHER]: <https://github.com/cyceron/TML>
[MILES]: <http://www.milesburton.com/?title=Dallas_Temperature_Control_Library>
[DSCAL]: <https://github.com/madhacker3kxl/GLHP_Trailer/commit/5575ebc0df84a17fe0933561ec45dda765844924>