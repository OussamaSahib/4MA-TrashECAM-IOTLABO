//INITIALISATION EXPRESS +POST+CSS+ROUTES
let express= require("express");
let app= express();
app.use(express.urlencoded({extended:true}));
app.use(express.static("public"));
let router= require("./routes");
app.use("/",router);

//LIEN TTN VIA SOCKET.IO
let mqtt= require("mqtt");
const server= require("http").Server(app);
const io= require("socket.io")(server);
const fs= require("fs");


//CONNEXION À TTN VIA MQTT
const client= mqtt.connect("mqtt://eu1.cloud.thethings.network:1883", {
  username: "garbage-io@ttn",
  password: "NNSXS.NXV3IRRSDKKKL6SJ6FYJ2FPWYJMISAC5VPI5PYY.FMSLYAK7456GN4O23ZEMLYC56MMLXXQI7F5564VKJ2FADXZAIO2A"
});
const deviceIDS= [
  "adafruit-feather-m0",   
  "adafruit-feather-m1"       
];
client.on("connect",()=> {
  console.log("Connecté à TTN");
  deviceIDS.forEach(deviceID=> {
    client.subscribe(`v3/garbage-io@ttn/devices/${deviceID}/up`);
  });
});



//RECUPERATION DATAS DU TTN QU'ON ENREGISTRE DS JSON
//JSON: Liste [], contenant plusieurs Dict. {}(=chaque poubelle)
function SaveData(ID, bytes, latitude, longitude) {
  //Données
  const newTrashData= {
    "ID": ID,
    "Bytes": bytes,
    "Latitude": latitude,
    "Longitude": longitude,
  };
  fs.readFile("./public/trashs.json", "utf8", (err, data)=> {
    let jsonData= [];
    if (!err) {
      try {
        jsonData= JSON.parse(data);
        if (!Array.isArray(jsonData)) {
          jsonData= [];
        }
      } catch (parseError) {
        console.error("Erreur de parsing JSON:", parseError);
        jsonData= [];
      }
    }

    //Mise à jour des données
    const existingIndex= jsonData.findIndex(trash=> trash.ID===ID);
    if (existingIndex>=0) {
      jsonData[existingIndex].Bytes= bytes;
      jsonData[existingIndex].Latitude= latitude;
      jsonData[existingIndex].Longitude= longitude;
    } else {
      jsonData.push(newTrashData);
    }
    fs.writeFile("./public/trashs.json", JSON.stringify(jsonData, null, 4), (err)=> {
      if (err) throw err;
      console.log("Fichier mis à jour!");
    });
  });
}


client.on("message", (topic, message)=> {
  const textDecoder= new TextDecoder("utf-8");
  const jsonString= textDecoder.decode(new Uint8Array(message));
  const jsonData= JSON.parse(jsonString);
  console.log("Données reçues de TTN:", jsonData);

  const ID= jsonData.end_device_ids.device_id;
  const Bytes= jsonData.uplink_message.decoded_payload.bytes;
  const Latitude= jsonData.uplink_message.locations.user.latitude;
  const Longitude= jsonData.uplink_message.locations.user.longitude;

  console.log("ID:", ID);  
  io.emit("event-ID", ID);

  console.log("Bytes:", Bytes);  
  io.emit("event-bytes", Bytes);

  console.log("Latitude:", Latitude)
  io.emit("event-latitude", Latitude);

  console.log("Longitude:", Longitude)
  io.emit("event-longitude", Longitude);

  SaveData(ID, Bytes, Latitude, Longitude);
});


//METHODE LISTEN
server.listen(3000,()=> {
  console.log("Serveur en écoute sur le port 3000");
});

