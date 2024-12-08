let express=require("express");
let router=express.Router();


//ROUTE TO "Home.ejs"
router.get("/", (req,res)=>
{
    res.render("Home.ejs");
});

//ROUTE TO "TrashManager.ejs"
router.get("/trashmanager", (req, res) => {
    res.render("TrashManager.ejs");
});

//ROUTE TO "TrashCollector.ejs"
router.get("/trashcollector", (req, res) => {
    res.render("TrashCollector.ejs");
});

//EXPORT OF ROUTER
module.exports= router;