let express=require("express");
let router=express.Router();

router.get("/", (req,res)=>
{
    res.render("TrashManager.ejs");
});

//EXPORT DU ROUTER
module.exports= router;