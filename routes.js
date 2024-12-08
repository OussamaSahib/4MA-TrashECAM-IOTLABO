let express=require("express");
let router=express.Router();

router.get("/", (req,res)=>
{
    res.render("Home.ejs");
});

// Route vers TrashManager.ejs
router.get("/trashmanager", (req, res) => {
    res.render("TrashManager.ejs");
});

// Route vers Eboueur.ejs
router.get("/trashcollector", (req, res) => {
    res.render("TrashCollector.ejs");
});

//EXPORT DU ROUTER
module.exports= router;