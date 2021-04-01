const Router = require("@koa/router");
const router = new Router();
const UserController = require("./controller");
const userRole = require('../middleware/userRole');

router.get("/meetings", UserController.getMeetingsById);

router.get("/:id", UserController.getById);

router.get("/", async ctx => {
    if (ctx.query.name) {
        await UserController.searchByName(ctx, ctx.query.name);
    } else {
        await UserController.get(ctx);
    }
});

router.post("/", userRole.can('access admin'), UserController.addUser);

router.put("/",  async ctx => { 
    if (ctx.query.name) {
        await UserController.updateUser(ctx);
    } else {
        await UserController.updateUserPassword(ctx);
    }
});

router.delete("/:id",  userRole.can('access admin'), UserController.deleteUser);

module.exports = router;