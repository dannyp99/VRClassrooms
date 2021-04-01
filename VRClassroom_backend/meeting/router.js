const Router = require("@koa/router");
const router = new Router();
const MeetingController = require("./controller");
const userRole = require('../middleware/userRole');

router.get("/", MeetingController.get);

router.get("/:id", MeetingController.getById);

router.get("/users/:id", userRole.can('access attendees'), MeetingController.getUsersById);

router.get("/timeslots/:id", MeetingController.getTimeSlotsById);

router.post("/", MeetingController.addMeeting);

router.put("/:id", MeetingController.updateMeeting);

router.delete("/:id", MeetingController.deleteMeeting);

module.exports = router;
