
# ECAM 4MA Project Trash - Labo IOT

## 📋 Prerequisites
Before starting, download Node.js from the official website: [NodeJS](nodejs.org/en/).


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
