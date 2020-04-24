# Wireless_Sensor_Net
In the field of Internet of Things, Wireless Sensor Nets are very versatile and can be used in various monitoring systems such as Agricultural, Smart Parking, Smart Security etc.

Wireless Sensor Net(WSN) consist of many Sensor Nodes(SN) and a Gateway Node(GN). SNs collect data and pass it on to GN. The GN will push the data on to a database such Firebase.

For SN, I used ESP32 and for GN I used ESP8266. There is no restriction for this configuration. You can use either of them for both although you may need to include different libraries for Web Server and Firebase in ESP32.

On the GN, I created a HTTP Web Server to get data from SN. From SN, HTTP POST requests are sent to the GN. 

NOTE: I have also uploaded the libraries you would need. I have also included a Firebase library for ESP32 in case you want make your server on ESP32
