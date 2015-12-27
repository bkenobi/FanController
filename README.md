# FanController
Arduino based attic fan controller.  This code reads three DHT based thermohygrometers (DHT11/DHT22) to determine if 2 attic fans need to be cycled.  If a fan needs to be operated, a signal is sent via X10.

#Hardware
Arduino Uno (though any variant should work)
DHT11/DHT22 (DHT22 not tested)
X10 controller (PL513/PSC04 one way controller tested though others should work with minimal modification)

#Usage
This code uses 3 DHT11 modules to determine the outside and 2 separate attic temperatures/humidities.  If the inside temperature is too high and outside is coolder or if the attic humidity is too high and the corrected outside air moisture content is lower, the fan will run for the set period.  If a DHT module returns an error, the code will send an ON command on a corresponding X10 address.

Due to accuracy limitations of the DHT11 modules that may or may not be present with the DHT22 or other available thermohygrometers, this code allows for a correction value to be applied to each temperature/humidity value to make it possible to align the 3 modules more accurately.

#Future
A planned update will allow this code to work with ESP8266 controllers.  In this revision, MQTT will be utilized to transmit the temperature/humidity/errors for each DHT11 module as well as fan states.
