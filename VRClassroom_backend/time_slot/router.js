const Router = require("@koa/router");
const router = new Router();
const TimeSlotController = require("./controller");

router.get("/", TimeSlotController.get);

router.get("/:id", TimeSlotController.getById);

router.get("/meetings/:id", TimeSlotController.getMeetingsById);

router.post("/", TimeSlotController.addTimeSlot);

router.put("/:id", TimeSlotController.updateTimeSlot);

router.delete("/:id", TimeSlotController.deleteTimeSlot);

module.exports = router;
