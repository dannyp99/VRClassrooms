const mariaDB = require('../database.js');
const userMeetingModel = require('../joined_models/userMeeting');
const meetingTimeSlotModel = require('../joined_models/meetingTimeSlot');
class Meeting {
    
    static async getMeetings() {
        const query = 'SELECT * FROM Meeting;';
        let conn;
        try {
            conn = await mariaDB.getConnection();
            const meetings = await conn.query(query);
            return meetings;
          } catch (err) { 
            console.error(err); 
          } finally { 
            if (conn) { conn.end(); }
          }
    }

    static async getMeeting(meetingId) {
        const query = `SELECT * FROM Meeting WHERE id=${mariaDB.escape(meetingId)};`;
        let conn;
        try {
            conn = await mariaDB.getConnection();
            const meeting = await conn.query(query);
            return meeting[0];
        } catch (err) {
            console.error(err);
        } finally {
            if (conn) { conn.end(); }
        }
    }

    static async addMeeting(meeting) {
        if (!meeting) { return; }
        let query = `INSERT INTO Meeting (map_id, name, password, repeating, active, creator_id)
          VALUES (${mariaDB.escape(meeting.map_id)}, ${mariaDB.escape(meeting.name)}, ${mariaDB.escape(meeting.password)}, ${mariaDB.escape(meeting.repeating)}, ${mariaDB.escape(meeting.active)}, ${mariaDB.escape(meeting.creator_id)});
          SELECT * FROM Meeting ORDER BY id DESC LIMIT 1;`;
        let conn;
        try {
            conn = await mariaDB.getConnection();
            await conn.beginTransaction();
            const newData = await conn.query(query);
            const newMeeting = newData[1][0];
            if (!newMeeting && !newMeeting.id) { return; }
            if (meeting.users && meeting.timeSlots) {
                await userMeetingModel.createUserMeeting(newMeeting.id, meeting.users, conn);
                await meetingTimeSlotModel.createMeetingTimeSlot(newMeeting.id, meeting.timeSlots, conn);
            } else {
                console.error("Users or TimeSlots not passed as property of the meeting");
                throw new Error("Users or TimeSlots not passed as property of meeting");
            }
            await conn.commit();
            return newMeeting;
        } catch (err) {
            await conn.rollback();
            console.error(err);
        } finally {
            if (conn) { conn.end(); }
        }
    }

    static async updateMeeting(meetingId, meeting) {
        if (!meeting) { return; }
        const query = `UPDATE Meeting
            SET map_id = ${mariaDB.escape(meeting.map_id)},
            name = ${mariaDB.escape(meeting.name)},
            password = ${mariaDB.escape(meeting.password)},
            repeating = ${mariaDB.escape(meeting.repeating)},
            active = ${mariaDB.escape(meeting.active)},
            creator_id = ${mariaDB.escape(meeting.creator_id)}
            WHERE id = ${mariaDB.escape(meetingId)}
            LIMIT 1;
        SELECT * FROM Meeting WHERE id=${mariaDB.escape(meetingId)};`;
        let conn;
        try {
            conn = await mariaDB.getConnection();
            await conn.beginTransaction();
            const data = await conn.query(query);
            if (meeting.deleteUsers || meeting.insertUsers) {
                await userMeetingModel.updateUserMeeting(meetingId, meeting.deleteUsers, meeting.insertUsers, conn);
            } else {
                console.log("User_Meeting not updated. deleteUsers or insertUsers were undefined");
            }
            if (meeting.deleteTimeSlots || meeting.insertTimeSlots) {
                await meetingTimeSlotModel.updateMeetingTimeSlot(meetingId, meeting.deleteTimeSlots, meeting.insertTimeSlots, conn);
            } else {
                console.log("Meeting_TimeSlots not updated. deleteTimeSlots or insertTimeSlots were undefined");
            }
            await conn.commit();
            const updatedMeeting = data[1][0];
            return updatedMeeting;
        } catch (err) {
            await conn.rollback();
            console.error(err);
        } finally {
            if (conn) { conn.end(); }
        }
    }

    static async deleteMeeting(meetingId) {
        const query = `DELETE FROM Meeting WHERE id = ${mariaDB.escape(meetingId)};`;
        let conn;
        try {
            conn = await mariaDB.getConnection();
            await conn.beginTransaction();

            await meetingTimeSlotModel.deleteMeetingTimeSlot(meetingId, conn);

            await userMeetingModel.deleteUserMeeting(meetingId, conn);

            await conn.query(query);
            
            await conn.commit();
            return 204;
        } catch (err) {
            await conn.rollback();
            console.error(err);
        } finally {
            if (conn) { conn.end(); }
        }
    }
}

module.exports = Meeting;