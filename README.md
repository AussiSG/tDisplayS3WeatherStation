# Weather Station LilyGo T-Display S3 by AussiSG

I have modified the original code to make it work with the latest ESP32 board library (V3.x) , older library is still supported (v2.x).
Added a feature to change the brightness of the screen with the right button / button 2.
And moved the temperature symbol to the right side of the screen, which feels more logic to me.

# How to install on T-Display S3

1. Clone or download this repository to your local machine into the Arduino directory.

2. Open with Arduino IDE or Visual Studio Code with extensions vscode-arduino extension and vscode-cpptools.

3. Use Lilygo T-Display S3 as board choice.
![image](https://github.com/user-attachments/assets/9c319b57-ca42-438f-93cc-b18de6999281).

5. Setup board information according Lilygo github [LilyGO T-Display S3](https://github.com/Xinyuan-LilyGO/T-Display-S3)
![image](https://github.com/user-attachments/assets/93cffe11-a7d2-4f78-908e-1bde53ccd8d0)

6. Configure TFT_eSPI editing file User_Setup_Select.h, where you need comment “#include <User_Setup.h>” and uncomment line for your device, just one line may be uncommented, for this case “#include <User_Setups/Setup206_LilyGo_T_Display_S3.h>” line. This is somewhere near line 138.

7. Get your free API Key from https://openweathermap.org/.

8. Change info in the .ino file approx. from row 22.
![image](https://github.com/user-attachments/assets/1e47ff4b-fd5c-4c0c-87ad-32cff8ab648a)

10. Download it to your T-Display S3 after download connect to local AP 'S3WeatherStation' with password: 'password'.

11. Connect to your desired WiFi Connection.

12. Enjoy :)

# Credits

Here is the original [TdisplayS3 Weatherstation] (https://github.com/VolosR/tDisplayS3WeatherStation) by VolosR

Brightness adjustment that I saw at [TTGOWeatherStation](https://github.com/JuniorPolegato/TTGOWeatherStation) by JuniorPolegato

# License
This software is licensed with GPL Version 3. https://www.gnu.org/licenses/gpl-3.0.html
