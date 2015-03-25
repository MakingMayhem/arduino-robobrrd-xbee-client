# RoboBrrd XBee Client

This Arduino sketch allows a [RoboBrrd](http://robobrrd.com/kit/) to be 
controlled by a remote ZigBee device. 


## Getting Started

You will need to install the following third-party Arduino libraries:

* [XBee-Arduino Library](https://code.google.com/p/xbee-arduino/)

This sketch works out-of-the-box with the following hardware:

* [Adafruit XBee Adapter Kit](https://www.adafruit.com/products/1430) or [other](https://www.sparkfun.com/products/11373) [boards](http://www.parallax.com/product/32401) that provide 3.3V / 5V shifting
* [Digi XBee ZB](http://www.digi.com/products/wireless-wired-embedded-solutions/zigbee-rf-modules/zigbee-mesh-module/xbee-zb-module), 
  formerly "XBee Series 2"
* [RoboBrrd Kit](http://robobrrd.com/kit/)

The XBee ZB must be configured with:

* `ATAP 2` (API mode with escaping, required by the XBee-Arduino Library)
* `ATAO 0` (Default receive API indicators enabled)

To program the RoboBrrd, you must use: 

* An FTDI cable or adapter, like the [FTDI Friend](http://adafruit.com/products/284) or [FTDI cable](https://www.sparkfun.com/products/9718)
* Arduino 1.6 settings:
    * Board: `Arduino Duemilanove or Diecimila`
    * Processor: `ATmega328`


## Protocol

The sketch expects to a binary payload containing a command stream of 0-66 
bytes. To send more than 66 bytes of commands, send multiple packets. 

### BNF

_To be written._

    0x00   NOOP
    0xFF   DELAY (next 2 octets define uint16_t of duration in milliseconds)
    
    0x01   EYE COLOR (next 3 octets define red, green, blue values)
    0x02   PLAY TONE (next 4 octets define tone and duration: first 2 octets 
           define big endian uint16_t of tone in Hz; last 2 octet define big 
           endian uint16_t of duration in milliseconds)
    0x03   BLINK LED (next 3 octets define number of repetitions and duration 
           LED is on and off: first octet defines uint8_t number of 
           repetitions; last 2 octets define big endian uint16_t of duration of 
           between on and of in milliseconds)
    
    0x10   SET BEAK VALUE (next octet defines value; 0=closed; 255=open)
    0x1C   CENTER LEFT WING
    0X1D   CENTER RIGHT WING
    0x1E   SET LEFT WING VALUE (next octet defines value; 0=down; 255=up)
    0x1F   SET RIGHT WING VALUE (next octet defines value; 0=down; 255=up)
    
    0xe0   FLAP WINGS [move up then down] (next octet defines number of 
           repetitions)

### Examples

To change eyes purple:

    01 ff 00 ff
    
To play a short tune:

    02 01 06 00 fa ff 00 3e 
    02 00 c4 00 7d ff 00 1f 
    02 00 c4 00 7d ff 00 1f 
    02 00 dc 00 fa ff 00 3e 
    02 00 c4 00 fa ff 00 3e 
    02 00 00 00 fa ff 00 3e 
    02 00 f7 00 fa ff 00 3e 
    02 01 06 00 fa ff 00 3e 

To open the beak, flap the wings 3 times then close the beak and re-center both 
wings:

    10 ff e0 03 10 00 1c 1d

To set the left wing up and the right wing down, then switch the up and down 
wings: 

    1e ff 1f 00 ff 00 fa 1e 00 1f ff

### Discussion

The protocol's maximum payload size is 66 bytes. This limit was chosen due to 
the following constraints:

* The XBee-Arduino library supports a maximum _packet_ size of 100 bytes. (This 
  can be changed by editing the library's `.h` file.)
* The [Digi knowledge base](http://www.digi.com/support/kbase/kbaseresultdetl?id=3345) 
  states that the maximum _non-fragmented_ payload size is 66 bytes when using 
  encryption. 


## Control Through Device Cloud

If you have a [Digi XBee Gateway](http://www.digi.com/products/wireless-routers-gateways/gateways/xbee-gateway) 
or [Digi ConnectPort](http://www.digi.com/products/wireless-routers-gateways/gateways/connectportx2gateways), 
you can control the NeoPixels across the Internet using [Device Cloud](http://www.etherios.com/products/devicecloud/) 
by sending [Server Command Interface (SCI)](http://ftp1.digi.com/support/documentation/html/90002008/90002008_R/Default.htm#Programming%20Topics/SCI.htm%3FTocPath%3DDevice%2520Cloud%2520Programming%2520Guide%7CSCI%7C_____0) 
commands. 

An example SCI command to play the short tune found in the Protocol Examples:

    <sci_request version="1.0">
     <send_message>
      <targets>
       <device id="00000000-00000000-004321AB-CDEF0123"/>
      </targets>
      <rci_request version="1.1">
       <do_command target="xbgw">
        <send_serial addr="0013A20012345678" encoding="base64">
         AgEGAPr/AD4CAMQAff8AHwIAxAB9/wAfAgDcAPr/AD4CAMQA+v8APgIAAAD6/wA+AgD3APr/AD4CAQYA+v8APg==
        </send_serial>
       </do_command>
      </rci_request>
     </send_message>
    </sci_request>

You will need to change `device id="…"` to the Device Cloud identifier for your 
XBee Gateway or ConnectPort and `send_serial addr="…"` to the 64bit address of 
your XBee attached to the Arduino.


## License

This software is licensed under the terms of the GPLv3. Its third-party 
software dependencies (i.e., XBee-Arduino library) are also licensed under the 
terms of the GPLv3. See the `LICENSE` file for more information. 
