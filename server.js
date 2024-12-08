//INITIALISATION EXPRESS +POST+CSS+ROUTES
let express= require("express");
let app= express();
app.use(express.urlencoded({extended:true}));
app.use(express.static("public"));
let router= require("./routes");
app.use("/",router);

//LINK TTN VIA SOCKET.IO
let mqtt= require("mqtt");
const server= require("http").Server(app);
const io= require("socket.io")(server);
const fs= require("fs");


//CONNECTION TO TTN VIA MQTT
const client= mqtt.connect("mqtt://eu1.cloud.thethings.network:1883", {
  username: "garbage-io@ttn",
  password: "NNSXS.NXV3IRRSDKKKL6SJ6FYJ2FPWYJMISAC5VPI5PYY.FMSLYAK7456GN4O23ZEMLYC56MMLXXQI7F5564VKJ2FADXZAIO2A"
});
const deviceIDS= [
  "adafruit-feather-m0",   
  "adafruit-feather-m1"       
];
client.on("connect",()=> {
  console.log("Connected à TTN");
  deviceIDS.forEach(deviceID=> {
    client.subscribe(`v3/garbage-io@ttn/devices/${deviceID}/up`);
  });
});



//RECOVERING DATA FROM TTN RECORDED BY JSON
//JSON: List [], containing several Dict. {}(=each bin)
function SaveData(ID, weight,  quantity, latitude, longitude) {
  //Datas
  const newTrashData= {
    "ID": ID,
    "Weight": weight,
    "Quantity": quantity,
    "Latitude": latitude,
    "Longitude": longitude,
  };
  fs.readFile("./public/trashs.json", "utf8", (err, data)=> {
    let jsonData= [];
    if (!err){
      try{
        jsonData= JSON.parse(data);
        if (!Array.isArray(jsonData)) {
          jsonData= [];
        }
      } catch (parseError){
        console.error("JSON parsing error:", parseError);
        jsonData= [];
      }
    }

    //Update Datas
    const existingIndex= jsonData.findIndex(trash=> trash.ID===ID);
    if (existingIndex>=0){
      jsonData[existingIndex].Weight= weight;
      jsonData[existingIndex].Quantity= quantity;
      jsonData[existingIndex].Latitude= latitude;
      jsonData[existingIndex].Longitude= longitude;
    } else{
      jsonData.push(newTrashData);
    }
    fs.writeFile("./public/trashs.json", JSON.stringify(jsonData, null, 4), (err)=> {
      if (err) throw err;
      console.log("File trashs.json updated!");
    });
  });
}


client.on("message", (topic, message)=> {
  const textDecoder= new TextDecoder("utf-8");
  const jsonString= textDecoder.decode(new Uint8Array(message));
  const jsonData= JSON.parse(jsonString);
  console.log("Datas received from TTN:", jsonData);

  const ID= jsonData.end_device_ids.device_id;
  const Weight= jsonData.uplink_message.decoded_payload.weight;
  const Quantity= jsonData.uplink_message.decoded_payload.quantity;
  const Latitude= jsonData.uplink_message.locations.user.latitude;
  const Longitude= jsonData.uplink_message.locations.user.longitude;

  console.log("ID:", ID);  
  io.emit("event-ID", ID);

  console.log("Weight:", Weight);  
  io.emit("event-weight", Weight);

  console.log("Quantity:", Quantity);  
  io.emit("event-quantity", Quantity);

  console.log("Latitude:", Latitude)
  io.emit("event-latitude", Latitude);

  console.log("Longitude:", Longitude)
  io.emit("event-longitude", Longitude);

  SaveData(ID, Weight, Quantity, Latitude, Longitude);
});




//SEND DATAS ROUTE TO TRASHROUTES.JSON
app.use(express.json()); 
app.post("/saveRoute", (req, res)=> {
  const routeData= req.body;
  const filePath= "./public/trashsroutes.json";


  fs.readFile(filePath, "utf8", (err, data)=> {
      if (err){
          console.error("Error while reading JSON file: ", err);
          res.status(500).send("Error while reading JSON file: ");
          return;
      }

      let jsonData= [];
      try{
          jsonData= JSON.parse(data);
      } 
      catch (parseError){
          console.error("JSON parsing error: ", parseError);
      }

      jsonData.push(routeData);

      fs.writeFile(filePath, JSON.stringify(jsonData, null, 4), (writeErr)=> {
          if (writeErr){
              console.error("Error while writing JSON file: ", writeErr);
              res.status(500).send("Error while writing JSON file: ");
          } 
          else{
              res.status(200).send("Datas successfully saved");
          }
      });
  });
});




//METHODE LISTEN
server.listen(3000,()=> {
  console.log("Serveur en écoute sur le port 3000");
});

