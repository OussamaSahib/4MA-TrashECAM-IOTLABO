
# ECAM 4MA Project Trash - Labo IOT
An efficient waste management system plays a major role in our modern urban living. To address this problem, our team developed a prototype as part of our IoT Lab course, given at ECAM in 1st year of master.  
  
The goal of our prototype is to automate waste management in urban areas, using HX711 adc and signal amplifier  on a load cell as well as a HC-SR04 ultrasonic sensor to remotely monitor weight and the fill level  of bins. We aim to optimize waste collection, prevent overflow and improve the efficiency of collection services. The device should be powered by a battery and directly installed inside the bins.  
  
Our prototype uses the LoRaWAN technology with an Adafruit Feather M0. Since it is used to know the fill level of a bin, we called this project “Garbage Filling Notifier”. It will communicate with The Things Network server using the Internet. Then, a user can use an application that will show details on each bins’ filling level.  

The source code for this project can be found in the folder ```Project_INO```, where there is the .ino file. This file is designed to be executed using the Arduino IDE. From now on, we will now detail **how to install and configure the ```server``` for this project**.


## 📋 Prerequisites
Before starting, download Node.js from the official website: [NodeJS](https://nodejs.org/en/)


## ⚙️ Project Installation
### Initialization of the project
In the terminal :
```
npm init
```
The Entry point is set to : ```server.js```

### Installation of dependencies
In the terminal :
```
npm install express --save
npm install nodemon -g --save
npm install ejs --save
npm install
```

## 🚀 Start the Server
To run the server, in the Terminal:
```
nodemon server.js
```
Once the server is running, open the browser and enter the following URL:
```
http://localhost:3000
``` 

**ℹ️Note:**  
If you want to change the port of the server, open the file ```server.js```, and modify the following line by replacing ```3000``` with the desired port number:
```
server.listen(3000,()=> {
  console.log("Server listening on port 3000");
});
```


## 📦Used libraries
- Express
- Socket.IO
- MQTT
- Nodemon
- EJS
- File System (fs)
- Google Maps API (-->via an API key)
