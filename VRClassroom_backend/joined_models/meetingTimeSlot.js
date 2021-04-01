const mariaDB = require('../database');

class MeetingTimeSlot {
    static async getByMeetingId(meetingId) {
        const query = `SELECT ts.* FROM Meeting_TimeSlot AS mt
            LEFT JOIN Time_Slot as ts ON mt.timeslot_id = ts.id
            WHERE mt.meeting_id =${mariaDB.escape(meetingId)};`;
        let conn;
        try {
            conn = await mariaDB.getConnection();
            const timeSlots = await conn.query(query);
            return timeSlots;
        } catch (err) {
            console.error(err);
        } finally {
            if (conn) { conn.end(); }
        }
    }

    static async getByTimeSlotId(timeslotId) {
        const query = `SELECT m.* FROM Meeting_TimeSlot AS mt
            LEFT JOIN Meeting as m ON mt.meeting_id = m.id
            WHERE mt.timeslot_id=${mariaDB.escape(timeslotId)};`;
        let conn;
        try {
            conn = await mariaDB.getConnection();
            let meetings = await conn.query(query);
            return meetings;
        } catch (err) {
            console.error(err);
        } finally {
            if (conn) { conn.end(); }
        }
    }

    static async createMeetingTimeSlot(meetingId, timeSlots, conn) {
        let meetingConn;
        let query = `INSERT INTO Meeting_TimeSlot (meeting_id, timeslot_id)
        VALUES`;
        let meetingTimeSlots;
        if (!timeSlots) { return; }
        try {
            meetingConn = conn ? conn : await mariaDB.getConnection();
            for ( let i = 0; i < timeSlots.length - 1; i++) {
                query += `(${mariaDB.escape(meetingId)}, ${mariaDB.escape(timeSlots[i])}),`;
            }
            query += `(${mariaDB.escape(meetingId)}, ${mariaDB.escape(timeSlots[timeSlots.length - 1])});\n`;
            query += `SELECT * FROM Meeting_TimeSlot WHERE meeting_id=${mariaDB.escape(meetingId)};`;
            meetingTimeSlots = await conn.query(query);
            return meetingTimeSlots;
        } catch (err) {
            await meetingConn.rollback();
            console.error(err);
        } finally {
            if (!conn) { meetingConn.end(); }
        }
    }

    static async updateMeetingTimeSlot(meetingId, delTimeSlots, insertTimeSlots, conn) {
        let meetingConn;
        let updatedMeetingTimeSlots = [];
        try {
            meetingConn = conn ? conn : await mariaDB.getConnection();
            if (delTimeSlots) {
                await this.deleteTimeSlotsFromMeeting(meetingId, delTimeSlots, meetingConn);
            }
            if (insertTimeSlots) {
                updatedMeetingTimeSlots = await this.createMeetingTimeSlot(meetingId,insertTimeSlots, meetingConn);
            }
            return updatedMeetingTimeSlots;
        } catch (err) {
            await meetingConn.rollback();
            console.error(err);
        } finally {
            if (!conn) { meetingConn.end(); }
        }
    }

    static async deleteMeetingTimeSlot(meetingId, conn) {
        let meetingConn;
        const query = `DELETE FROM Meeting_TimeSlot WHERE meeting_id=${mariaDB.escape(meetingId)};`;
        try {
            meetingConn = conn ? conn : await mariaDB.getConnection();
            await meetingConn.query(query);
            return 204;
        } catch (err) {
            await meetingConn.rollback();
            console.error(err);
        } finally {
            if (!conn) { meetingConn.end(); }
        }
    }

    static async deleteTimeSlotsFromMeeting(meetingId, timeSlots, conn) {
        let meetingConn;
        let query = `DELETE FROM Meeting_TimeSlot WHERE meeting_id=${mariaDB.escape(meetingId)} && timeslot_id in(`;
        try {
            meetingConn = conn ? conn : await mariaDB.getConnection();
            for (let i = 0; i < timeSlots.length - 1; i++) {
                query += `${mariaDB.escape(timeSlots[i])},`;
            }
            query += `${mariaDB.escape(timeSlots[timeSlots.length - 1])});`
            await meetingConn.query(query);
            return 204;
        } catch (err) {
            await meetingConn.rollback();
            console.error(err);
        } finally {
            if (!conn) { meetingConn.end(); }
        }
    }
}

module.exports = MeetingTimeSlot;