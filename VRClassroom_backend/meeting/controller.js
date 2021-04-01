const meetingModel = require('./meeting');
const userMeetingModel = require('../joined_models/userMeeting');
const meetingTimeSlotModel = require('../joined_models/meetingTimeSlot');

class MeetingController {
  /* eslint-disable no-param-reassign */

  /**
   * Get all meetings
   * @param {ctx} Koa Context
   */
  async get(ctx) {
    let meetings =  await meetingModel.getMeetings();
    if (meetings === undefined) {
      ctx.throw(500);
    } else {
      ctx.status = 200;
      ctx.body = meetings;
    }
  }
  /**
   * Find a meeting
   * @param {ctx} Koa Context
   */
  async getById(ctx) {
    let meeting = await meetingModel.getMeeting(ctx.params.id);
    if(meeting) {
      ctx.status = 200;
      ctx.body = meeting;
    } else {
      ctx.throw(400);
    }
  }

  /**
   * Get Students by Meeting Id
   * @param {ctx} Koa Context
   */
  async getUsersById(ctx) {
    let users = await userMeetingModel.getByMeetingId(ctx.params.id);
    if(users === undefined) {
      ctx.throw(500); 
    } else {
      ctx.body = users;
      ctx.status = 200;
    }
  }

  /**
   * Get TimeSlots by Meeting Id
   * @param {ctx} Koa Context
   */
  async getTimeSlotsById(ctx) {
    let timeSlots = await meetingTimeSlotModel.getByMeetingId(ctx.params.id);
    if(timeSlots === undefined) {
      ctx.throw(500); 
    } else {
      ctx.body = timeSlots;
      ctx.status = 200;
    }
  }

  /**
   * Add a meeting
   * @param {ctx} Koa Context
   */
  async addMeeting(ctx) {
    let newCourse;
    if (ctx.request.body === undefined) { ctx.throw(400); } 
    newCourse = await meetingModel.addMeeting(ctx.request.body);
    if (newCourse) {
      ctx.body = newCourse;
      ctx.status = 201;
    } else {
      ctx.throw(400);
    }
  }

  /**
   * Update a meeting
   * @param {ctx} Koa Context
   */
  async updateMeeting(ctx) {
    let updatedCourse;
    if (ctx.request.body === undefined) { ctx.throw(400); }
    updatedCourse = await meetingModel.updateMeeting(ctx.params.id, ctx.request.body);
    if (updatedCourse) {
      ctx.body = updatedCourse;
      ctx.status = 200;
    } else {
      ctx.throw(400);
    }
  }

  /**
   * Delete a meeting
   * @param {ctx} Koa Context
   */
  async deleteMeeting(ctx) {
    let code = await meetingModel.deleteMeeting(ctx.params.id);
    if (code && code < 400) {
      ctx.status = code;
    } else if (code && code >= 400) {
      ctx.throw(code);
    }
  }
  /* eslint-enable no-param-reassign */
}

module.exports = new MeetingController();