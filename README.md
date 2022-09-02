# Jam Alarm
(Arduino Project) Just ordinary alarm project, nothing special here. jangan lupa bintang kak :)

## Hardware / Module
<ul>
  <li>ESP8266 (Lolin V3)</li>
  <li>RTC DS3231</li>
  <li>DF-Player</li>
</ul>

## Features
<ul>
  <li>Dynamic WiFi configuration</li>
  <li>Customable Alarm Time</li>
  <li>Automatic time calibration</li>
  <li>Reset Setting button</li>
</ul>

## GPIO Connection
<ul>
  <li>D1 ~> SCL</li>
  <li>D3 ~> SDA</li>
  <li>D3 ~> TX DFPlayer</li>
  <li>D4 ~> RX DFPlayer</li>
  <li>D5 ~> Anode LED Indicator</li>
  <li>D6 ~> Reset Button (Pulled down)</li>
</ul>

## Troubleshooting
<ul>
  <li>If RTC cannot work properly try to connect VCC RTC to VIN Lolin V3 or 5V</li>
  <li>If DFPlayer wont play the music check your SD Card</li>
</ul>
