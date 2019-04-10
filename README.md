# Azure IoT Connection Guide - TIC2019
This repo contains all the code and details you need to get your MKR NB 1500 board connected to Azure.

You'll be able to connect directly into two Azure IoT Services:
- [Azure IoT Hub](https://azure.microsoft.com/en-au/services/iot-hub/), where you can piece together every detail of your IoT architecture
- [Azure IoT Central](https://azure.microsoft.com/en-au/services/iot-central/), a fully-managed software-as-a-service offering where you can spend less time coding, and more time on your solution

## Deploying Your Cloud Resources
As mentioned above, you have two options for where to send your data. Below are guides for setting up each of the resources.

If you're brand-new to Azure, or even Cloud technologies, check out these guides on [What is Cloud Computing?](https://azure.microsoft.com/en-us/overview/what-is-cloud-computing/) and [What is Azure?](https://azure.microsoft.com/en-us/overview/what-is-azure/).

Before you get started, make sure you've created your [free Azure account](https://azure.microsoft.com/free/?WT.mc_id=A261C142F).

### Azure IoT Hub
IoT Hub is Azure's dedicated IoT gateway. It handles data ingestion, device management, cloud-to-device messaging, security, and much more! All of the Microsoft IoT reference architectures recommend IoT Hub as the best way to ingest data from IoT devices. Even IoT Central uses IoT Hub under the hood!

- Follow this guide for [Creating an IoT Hub](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-arduino-huzzah-esp8266-get-started#create-an-iot-hub)
  - **Note:** You can use the Free Tier of IoT Hub for this walkthrough
- Once you've created your Hub, add a new device and copy the connection string using this guide for [Registering a device](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-arduino-huzzah-esp8266-get-started#register-a-new-device-in-the-iot-hub)

### Azure IoT Central
IoT Central is a fully-managed Software-as-a-Service offering from Microsoft. It's the quickest way to get up-and-running with your IoT use cases and requires almost no coding to get started. The best part? IoT Central is completely free for the first 5 devices!

- You'll need to [Create an IoT Central Application](https://docs.microsoft.com/en-us/azure/iot-central/quick-deploy-iot-central)
- [ ] TODO: Complete IoT Central Guide

### Setting Up Your Arduino Environment
Before you get started with your device, there's a few things you'll need to install to make sure you're ready to program your board

1. Install the [Arduino IDE](https://www.arduino.cc/en/main/software)
2. Open the Arduino IDE and add the following board in the Board Manager:
  - Go to *Tools > Boards > Boards Manager ...* 
  - Install the latest version of *Arduino SAMD Boards (32-bits ARM Cortex-M0+) by Arduino*
3. Next, you'll need to include some libraries:
  - Go to *Sketch > Include Library > Manage Libraries...*
  - Install the latest version of the following libraries:
    - *MKRNB by Arduino*
    - *ArduinoMQTTClient by Arduino* -> **Note:** This library is in Beta
4. Now you're good to go!

### Set Up Arduino Code
- Clone or download the *MKRNB1500-Azure_IoT* folder from this repo
- Open the *MKRNB1500-Azure_IoT.ino* file in the Arduino IDE.
  - You should see several tabs open in the window. These libraries and dependencies for the code.
- Click on the *arduino_secrets.h* tab
  - This file holds sensitive information that you may not want to share (or push to GitHub!). It's easier to hold the seceret values in this file rather than disperse it through your code.
- Copy the device connection string from your chosen service:
  - **IoT Hub** - This can easily be found in the Azure Portal:
    1. Go to the Azure Portal and select the IoT Hub you deployed.
    2. In the side menu, select *IoT Devices* then select the IoT Device you created previously:
    ![Select Created Device](https://github.com/mjksinc/TIC2019/blob/master/ReferenceImages/Hub_Step-1_Device.png)
    3. Copy the *Connection String* field.
    ![Connection String](https://github.com/mjksinc/TIC2019/blob/master/ReferenceImages/Hub_Step-2_ConnectionString.png)
  - **IoT Central** - This requires a few additional steps:
    1. Go to your IoT Central app and select the device you created previously:
    ![Device Explorer](https://github.com/mjksinc/TIC2019/blob/master/ReferenceImages/Central_Step-0_Explorer.png)
    2. Click *Connect* to get the parameters
    ![Device Connect](https://github.com/mjksinc/TIC2019/blob/master/ReferenceImages/Central_Step-1_Device.png)
    3. From this screen, you'll need the *Scope ID*, *Device ID*, and the *Primary Key* for the next step
    ![Device Parameters](https://github.com/mjksinc/TIC2019/blob/master/ReferenceImages/Central_Step-2_Parameters.png)
    4. Open a new tab in your browser and head to [https://dpscstrgen.azurewebsites.net/](https://dpscstrgen.azurewebsites.net/). Copy in the required parameters from your IoT Central page and select *Submit*
    ![Conn String](https://github.com/mjksinc/TIC2019/blob/master/ReferenceImages/Central_Step-3_CnnSrgGen.png)
    5. A new connection string should be generated for your device!
- Paste into the *CONN_STRING* field in your *arduino_secrets.h* file. You should also copy the Hub Name from your connection string into the *SECRET_BROKER* field.
![Populate Arduino values](https://github.com/mjksinc/TIC2019/blob/master/ReferenceImages/PopulateValues.png)
- That's it! You can now connect your device and transmit values to your IoT service.