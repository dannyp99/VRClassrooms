const Router = require("@koa/router");
const router = new Router();
const LoginController = require("./controller");

router.post("/", LoginController.authorize);

module.exports = router;
