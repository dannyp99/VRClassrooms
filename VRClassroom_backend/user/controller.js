const userModel = require('./user');
const userMeetingModel = require('../joined_models/userMeeting');
class UserController {
  /* eslint-disable no-param-reassign */

  /**
   * Get current user
   * @param {ctx} Koa Context
   */
  async get(ctx) {
    const currentUser = await userModel.getCurrentUser(ctx.user.id);
    if (currentUser === undefined) {
      ctx.throw(400);
    } else {
      ctx.status = 200;
      ctx.body = currentUser;
    }
  }
  /**
   * Find a user
   * @param {ctx} Koa Context
   */
  async getById(ctx) {
    if (!ctx.params.id) { ctx.throw(400); }
    const user = await userModel.getUser(ctx.params.id);
    if(user) {
      ctx.body = user;
    } else {
      ctx.status = 200;
      ctx.throw(400);
    }
  }
  
  /**
   * Find a user by name
   * @param {ctx} Koa Context
   */
  async searchByName(ctx, name) {
    if(name === undefined) { ctx.throw(400); }
    const user = await userModel.searchUser(name);
    if(user) {
      ctx.body = user;
      ctx.status = 200;
    } else {
      ctx.throw(400);
    }
  }

  /**
   * Get Students by Meeting Id
   * @param {ctx} Koa Context
   */
  async getMeetingsById(ctx) {
    const meetings = await userMeetingModel.getByUserId(ctx.user.id);
    if(meetings === undefined) {
      ctx.throw(400); 
    } else {
      ctx.body = meetings;
      ctx.status = 200;
    }
  }

  /**
   * Add a user
   * @param {ctx} Koa Context
   */
  async addUser(ctx) {
    if (ctx.request.body === undefined) { ctx.throw(400); } 
    const newUser = await userModel.createUser(ctx.request.body);
    if (newUser) {
      ctx.body = newUser;
      ctx.status = 201;
    } else {
      ctx.throw(400);
    }
  }

  /**
   * Update a user
   * @param {ctx} Koa Context
   */
  async updateUser(ctx) {
    if (ctx.query.name === undefined) { ctx.throw(400); }
    const updatedUser = await userModel.updateUser(ctx.user.id, ctx.query.name);
    if (updatedUser) {
      ctx.body = updatedUser;
      ctx.status = 200;
    } else {
      ctx.throw(400);
    }
  }

  /**
   * Update a user password
   * @param {ctx} Koa Context
   */
  async updateUserPassword(ctx) {
      if (ctx.request.body === undefined) { ctx.throw(400); }
      const updatedUser = await userModel.updatedPassword(ctx.user.id, ctx.request.body);
      if (updatedUser) {
          ctx.status = 200;
          ctx.body = updatedUser
      } else {
          ctx.throw(400);
      }
  }

  /**
   * Delete a user
   * @param {ctx} Koa Context
   */
  async deleteUser(ctx) {
    let code = await userModel.deleteUser(ctx.params.id);
    if (code && code < 400) {
      ctx.status = code;
    } else if (code && code >= 400) {
      ctx.throw(code);
    }
  }
  /* eslint-enable no-param-reassign */
}

module.exports = new UserController();