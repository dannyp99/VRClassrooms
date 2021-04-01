var Koa = require('koa');
require("dotenv/config");

const Router = require('@koa/router');
const publicrouter = new Router();
const protectedrouter = new Router();

const [instancesPublicRouter, instancesProtectedRouter] = require('./instances/router.js');
const meetingsRouter = require('./meeting/router.js');
const timeSlotsRouter = require('./time_slot/router.js');
const userRouter = require('./user/router');
const userModel = require('./user/user');
const mount = require('koa-mount')
const loginRouter = require('./login/router.js');
const bodyParser = require('koa-body');

const userParser = require('./middleware/userParser.js');
const userRoles = require('./middleware/userRole');
var jwt = require('koa-jwt');

var app = new Koa();

var port = 1337;

protectedrouter.use('/timeslots', timeSlotsRouter.routes(), timeSlotsRouter.allowedMethods());
protectedrouter.use('/instances', instancesProtectedRouter.routes(), instancesProtectedRouter.allowedMethods());
protectedrouter.use('/meetings', meetingsRouter.routes(), meetingsRouter.allowedMethods());
protectedrouter.use('/users', userRouter.routes(), userRouter.allowedMethods());
publicrouter.use('/login', loginRouter.routes(), loginRouter.allowedMethods());
publicrouter.use('/instances', instancesPublicRouter.routes(), instancesPublicRouter.allowedMethods());

app
  .use(bodyParser())
  .use(publicrouter.routes())
  .use(publicrouter.allowedMethods())

/**
 * Enforce authentication using JWT
 * @param {ctx} Koa Context
 */
app.use(jwt({ secret: 'outer-possible-ring-meet' }));

app.use(async(ctx, next) => {
  if(!ctx.headers.authorization){
    ctx.throw(400);
  }
  ctx.user = {};
  const id = await userParser.getUser(ctx.headers.authorization);
  ctx.user = await userModel.getUserPermissions(id);

  await next();
})

app
  .use(protectedrouter.routes())
  .use(protectedrouter.allowedMethods())
  .use(userRoles.middleware())

app.listen(port);
console.log("Listening on port %s", port)

module.exports = app;