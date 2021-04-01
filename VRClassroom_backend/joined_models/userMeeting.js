const mariaDB = require('../database');

class UserMeeting {

    static async getByMeetingId(meetingId) {
        let query = `SELECT u.* FROM User_Meeting as um
            LEFT JOIN User as u ON um.user_id = u.id
            WHERE um.meeting_id=${mariaDB.escape(meetingId)};`;
        let conn;
        try {
            conn = await mariaDB.getConnection();
            let users = await conn.query(query);
            return users;
        } catch (err) {
            console.error(err);
        } finally {
            if (conn) { conn.end(); }
        }
    }

    static async getByUserId(userId) {
        let query = `SELECT m.* FROM User_Meeting as um
            LEFT JOIN Meeting as m ON um.meeting_id = m.id
            WHERE user_id=${mariaDB.escape(userId)};`;
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

    static async createUserMeeting(meetingId, users, conn) {
        let meetingConn;
        let query = `INSERT INTO User_Meeting (user_id, meeting_id)
        VALUES`;
        let userMeeting;
        if (!users) { return; }
        try {
            meetingConn = conn ? conn : await mariaDB.getConnection();
            for (let i = 0; i < users.length - 1; i++) {
                query += `(${mariaDB.escape(users[i])}, ${mariaDB.escape(meetingId)}),`;
            }
            query += `(${mariaDB.escape(users[users.length - 1])}, ${mariaDB.escape(meetingId)});`;
            query += `SELECT * FROM User_Meeting WHERE meeting_id=${mariaDB.escape(meetingId)};`;
            userMeeting = meetingConn.query(query);
            return userMeeting;
        } catch (err) {
            await meetingConn.rollback();
            console.error(err);
        } finally {
            if (!conn) { meetingConn.end(); }
        }
    }

    static async updateUserMeeting(meetingId, delUsers, insertUsers, conn) {
        let meetingConn;
        let updatedUserMeetings = [];
        try {
            meetingConn = conn ? conn : await mariaDB.getConnection();
            if (delUsers) {
                await this.deleteUsersFromMeeting(meetingId, delUsers, meetingConn);
            }
            if (insertUsers) {
                updatedUserMeetings = await this.createUserMeeting(meetingId,insertUsers, meetingConn);
            }
            return updatedUserMeetings;
        } catch (err) {
            await meetingConn.rollback();
            console.error(err);
        } finally {
            if (!conn) { meetingConn.end(); }
        }
    }

    static async deleteUserMeeting(meetingId, conn) {
        let meetingConn;
        const query = `DELETE FROM User_Meeting WHERE meeting_id=${mariaDB.escape(meetingId)};`;
        try {
            meetingConn = conn ? conn : await mariaDB.getConnection();
            await meetingConn.query(query);
            return 204;
        } catch (err) {
            await conn.rollback();
            console.error(err);
        } finally {
            if (!conn) { meetingConn.end(); }
        }
    }

    static async deleteUsersFromMeeting(meetingId, users, conn) {
        let meetingConn;
        let query = `DELETE FROM User_Meeting WHERE meeting_id=${mariaDB.escape(meetingId)} && user_id in (`;
        try {
            meetingConn = conn ? conn : await mariaDB.getConnection();
            for (let i = 0; i < users.length - 1; i++) {
                query += `${mariaDB.escape(users[i])},`;
            }
            query += `${mariaDB.escape(users[users.length - 1])});`
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

module.exports = UserMeeting;