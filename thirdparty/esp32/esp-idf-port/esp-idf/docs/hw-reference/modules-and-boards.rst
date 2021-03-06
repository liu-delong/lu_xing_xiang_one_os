ESP32 Modules and Boards
========================

Espressif designed and manufactured several development modules and boards to help users evaluate functionality of ESP32 chip. Development boards, depending on intended functionality, have exposed GPIO pins headers, provide USB programming interface, JTAG interface as well as peripherals like touch pads, LCD screen, SD card slot, camera module header, etc.

For details please refer to documentation below, provided together with description of particular boards.


ESP-WROOM-32
------------

The smallest module intended for installation in final products. Can be also used for evaluation after adding extra components like programming interface, boot strapping resistors and break out headers.

.. image:: http://dl.espressif.com/dl/schematics/pictures/esp-wroom-32.jpg
    :align: center
    :width: 40%
    :alt: ESP-WROOM-32 module (front and back)


* `Schematic <http://dl.espressif.com/dl/schematics/ESP-WROOM-32-v3.2_sch.pdf>`__ (PDF)
* `Datasheet <https://espressif.com/sites/default/files/documentation/esp_wroom_32_datasheet_en.pdf>`__ (PDF)
* `ESP32 Module Reference Design <https://espressif.com/sites/default/files/documentation/esp32_module_reference_design.zip>`_ (ZIP) containing OrCAD schematic, PCB layout, gerbers and BOM

   
ESP32 Core Board V2 / ESP32 DevKitC
-----------------------------------

Small and convenient development board with break out pin headers and minimum additional components. Includes USB to serial programming interface, that also provides power supply for the board. Has press buttons to reset the board and put it in upload mode. 

.. image:: http://dl.espressif.com/dl/schematics/pictures/esp32-core-board-v2.jpg
    :align: center
    :width: 50%
    :alt: ESP32 Core Board V2 / ESP32 DevKitC board

* `Schematic <http://dl.espressif.com/dl/schematics/ESP32-Core-Board-V2_sch.pdf>`__ (PDF)
* `ESP32 Development Board Reference Design <https://espressif.com/sites/default/files/documentation/esp32_development_board_reference_design.zip>`_ (ZIP) containing OrCAD schematic, PCB layout, gerbers and BOM
* :doc:`../get-started/get-started-devkitc`
* `CP210x USB to UART Bridge VCP Drivers <http://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers>`_



ESP32 Demo Board V2
-------------------

One of first feature rich evaluation boards that contains several pin headers, dip switches, USB to serial programming interface, reset and boot mode press buttons, power switch, 10 touch pads and separate header to connect LCD screen.

.. image:: http://dl.espressif.com/dl/schematics/pictures/esp32-demo-board-v2.jpg
    :align: center
    :alt: ESP32 Demo Board V2 board

* `Schematic <http://dl.espressif.com/dl/schematics/ESP32-Demo-Board-V2_sch.pdf>`__ (PDF)
* `FTDI Virtual COM Port Drivers`_


ESP32 WROVER KIT V1 / ESP32 DevKitJ V1
--------------------------------------

Development board that has dual port USB to serial converter for programming and JTAG interface for debugging. Power supply is provided by USB interface or from standard 5 mm power supply jack. Power supply selection is done with a jumper and may be put on/off with a separate switch. Has SD card slot, 3.2??? SPI LCD screen and dedicated header to connect a camera. Provides RGB diode for diagnostics. Also, includes 32.768 kHz XTAL for internal RTC to operate it in low power modes.

.. image:: http://dl.espressif.com/dl/schematics/pictures/esp32-devkitj-v1.jpg
    :align: center
    :width: 90%
    :alt: ESP32 WROVER KIT V1 / ESP32 DevKitJ V1 board

* `Schematic <http://dl.espressif.com/dl/schematics/ESP32-DevKitJ-v1_sch.pdf>`__ (PDF)
* `JTAG Debugging for ESP32`_ (PDF)
* `FTDI Virtual COM Port Drivers`_


ESP32 WROVER KIT V2/V3
----------------------

These are updated versions of ESP32 DevKitJ V1 described above with design improvements identified when DevKitJ was in use. All versions of this board are ready to accommodate an ESP-WROOM-32 or ESP32-WROVER module.

.. image:: http://dl.espressif.com/dl/schematics/pictures/esp-wrover-kit-v2.jpg
   :align: center
   :width: 90%
   :alt: ESP32 WROVER KIT V2 board

V2 boards have a shiny black finish and a male camera header, while V3 boards have a matte black finish and a female camera header. Pictured above is a V2 board.

* `Schematic V2 <http://dl.espressif.com/dl/schematics/ESP-WROVER-KIT_SCH-2.pdf>`__ (PDF)
* `Schematic V3 <http://dl.espressif.com/dl/schematics/ESP-WROVER-KIT_SCH-3.pdf>`__ (PDF)
* :doc:`../get-started/get-started-wrover-kit`
* `JTAG Debugging for ESP32`_ (PDF)
* `FTDI Virtual COM Port Drivers`_


.. _JTAG Debugging for ESP32: http://espressif.com/sites/default/files/documentation/jtag_debugging_for_esp32_en.pdf
.. _FTDI Virtual COM Port Drivers: http://www.ftdichip.com/Drivers/D2XX.htm

