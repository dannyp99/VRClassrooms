const timeSlotModel = require('./timeslot');
const meetingTimeSlotModel = require('../joined_models/meetingTimeSlot');
class TimeSlotController {
  /* eslint-disable no-param-reassign */

  /**
   * Get all timeSlots
   * @param {ctx} Koa Context
   */
  async get(ctx) {
    let timeSlots =  await timeSlotModel.getTimeSlots();
    if (timeSlots === undefined) {
      ctx.throw(500);
    } else {
      ctx.status = 200;
      ctx.body = timeSlots;
    }
  }

  /**
   * Find a timeSlot
   * @param {ctx} Koa Context
   */
  async getById(ctx) {
    let timeSlot = await timeSlotModel.getTimeSlot(ctx.params.id);
    if(timeSlot) {
      ctx.status = 200;
      ctx.body = timeSlot;
    } else {
      ctx.throw(400);
    }
  }

  /**
   * Get Meetings by TimeSlot Id
   * @param {ctx} Koa Context
   */
  async getMeetingsById(ctx) {
    let timeSlots = await meetingTimeSlotModel.getByTimeSlotId(ctx.params.id);
    if(timeSlots === undefined) {
      ctx.throw(500); 
    } else {
      ctx.body = timeSlots;
      ctx.status = 200;
    }
  }

  /**
   * Add a timeSlot
   * @param {ctx} Koa Context
   */
  async addTimeSlot(ctx) {
    let newTimeSlot;
    if (ctx.request.body === undefined) { ctx.throw(400); } 
    newTimeSlot = await timeSlotModel.addTimeSlot(ctx.request.body);
    if (newTimeSlot) {
      ctx.body = newTimeSlot;
      ctx.status = 201;
    } else {
      ctx.throw(400);
    }
  }

  /**
   * Update a meeting
   * @param {ctx} Koa Context
   */
  async updateTimeSlot(ctx) {
    let updatedTimeSlot;
    if (ctx.request.body === undefined) { ctx.throw(400); }
    updatedTimeSlot = await timeSlotModel.updateTimeSlot(ctx.params.id, ctx.request.body);
    if (updatedTimeSlot) {
      ctx.body = updatedTimeSlot;
      ctx.status = 200;
    } else {
      ctx.throw(400);
    }
  }

  /**
   * Delete a meeting
   * @param {ctx} Koa Context
   */
  async deleteTimeSlot(ctx) {
    let code = await timeSlotModel.deleteTimeSlot(ctx.params.id);
    if (code && code < 400) {
      ctx.status = code;
    } else if (code && code >= 400) {
      ctx.throw(code);
    }
  }
  /* eslint-enable no-param-reassign */
}

module.exports = new TimeSlotController();