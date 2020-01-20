# fer-smart-box
Smart box project aims to create a technical solution that can transform an ordinary office cabinet into a "smart" cabinet, which allows inventory monitoring within it and gives access to inventory only to authorized users.

## Getting started

This project requires the Espressif IoT Development Framework (ESP-IDF) set up and assumes the user already read the [ESP-IDF getting started guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) and set up the framework accordingly.

This project follows the standard [ESP-IDF structure](https://docs.espressif.com/projects/esp-idf/en/latest/contribute/creating-examples.html).

To flash the project on the ESP32 board you need to configure it by executing this command:
 ```
    sudo idf.py menuconfig
 ```
Add configure necessary options:

* WiFi Connection Configuration  ---> WiFi SSID
* WiFi Connection Configuration  ---> WiFi Password

Next run:
 ```
  sudo idf.py -p <path to port> flash monitor
 ```

And the project should be flashed to the board.

## Developing on the project

To start working on the project after cloning create a new branch like this:
 ```
  git checkout master
  git checkout -b task/name_of_task
 ```


## Useful reading materials
* [Kconfig](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/kconfig.html#kconfig-formatting-rules)

* [Networking](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/protocols/index.html)