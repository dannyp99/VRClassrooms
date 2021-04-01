const loginModel = require('./login')
class LoginController{
  /**
   * Authenticate user and issue a JWT token
   * @param {ctx} Koa Context
   */
  async authorize(ctx){
    const { username, password } = ctx.request.body;
    let login = await loginModel.login(username, password);
    if (login === undefined) {
      ctx.throw(400);
    }
    else if (login.length === 0) {
      ctx.throw(401);
    }
    else {
      ctx.headers.authorization = login;
      ctx.body = login;
    }
  }
}
module.exports = new LoginController();
