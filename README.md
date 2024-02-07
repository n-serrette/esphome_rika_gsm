# esphome_rika_gsm

The ```rika_gsm``` component emulate the gsm module that can be installed on rika stove. It provide a way to send command and retreive the sms the stove want to send.

## Config

### Component

```yaml
external_components:
  - source:
        type: git
        url: https://github.com/n-serrette/esphome_rika_gsm
        ref: main
    components: [ rika_gsm ]

uart:
  id: rika_uart_bus
  tx_pin: GPIO16
  rx_pin: GPIO17
  baud_rate: 38400

rika_gsm:
  id: ria_gsm_component_1
  pin: "1234"
  phone_number: "+33685967412"
```

Configuartion variables:
* **pin** (string): 4 digit string corresponding to the pin code set in the gsm configuration of the stove.
* **id** (Optional, ID): Manually specify the ID used for code generation.
* **phone_number** (Optional string): phone number use to communicate with the stove. No true sms are sent, this is a dummy phone number.

#### Note

Baud rate:
 * Revo (1st gen):  38400
 * TOPO: 38400 (not verified)
 * ROKO: 115200 (not verified)

### Text Sensor

```yaml
text_sensor:
  - platform: rika_gsm
    raw_status:
      name: "Stove raw status"
```

Configuration variables

* **raw_status** (Optional): give the last sms that the stove sent
    * **name** (Required, string): The name for the raw_status text sensor.
    * **id** (Optional, ID): Set the ID of this sensor for use in lambdas.
    * All other options from [Text Sensor](https://esphome.io/components/text_sensor/).


### Binary Sensor

```yaml
binary_sensor:
  - platform: rika_gsm
    gsm_status:
      name: "GSM status"
```

Configuration variables

* **gsm_status** (Optional): indicate if the gsm option is set on the stove
    * **name** (Required, string): The name for the gsm_status binary sensor.
    * **id** (Optional, ID): Set the ID of this sensor for use in lambdas.
    * All other options from [Binary Sensor](https://esphome.io/components/binary_sensor/).

When the gsm option is set the stove try to read if their is a new command every 5s. If their is no stove request after 10s the gsm option is considered deactivated and will be concidered active on the next stove request.

### rika_gsm.send_command Action

Send a command to the stove.

```yaml
on ...:
    then:
        - rika_gsm.send_command:
            command: "ON"
```

Configuration options:
 * **command** (Required, string, templatable): command sent to the stove

