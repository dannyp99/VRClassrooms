const mariaDB = require('../database.js');
class TimeSlot {
    static async getTimeSlots() {
        const query = 'SELECT * FROM Time_Slot;';
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

    static async getTimeSlot(timeSlotId) {
        const query = `SELECT * FROM Time_Slot WHERE id=${mariaDB.escape(timeSlotId)};`;
        let conn;
        try {
            conn = await mariaDB.getConnection();
            const timeSlot = await conn.query(query);
            return timeSlot;
        } catch (err) { 
            console.error(err);
        } finally {
            if (conn) { conn.end(); }
        }
    }

    static async addTimeSlot(timeSlot) {
        if (!timeSlot) { return; }
        let query = `INSERT INTO Time_Slot (day, start_time, end_time)
            VALUES(${mariaDB.escape(timeSlot.day)}, ${mariaDB.escape(timeSlot.start_time)}, ${mariaDB.escape(timeSlot.end_time)});
            SELECT * FROM Time_Slot ORDER BY id DESC LIMIT 1;`;
        let conn;
        try {
            conn = await mariaDB.getConnection();
            await conn.beginTransaction();
            const newData = await conn.query(query);
            const newTimeSlot = newData[1][0];
            await conn.commit();
            return newTimeSlot;
        } catch (err) {
            await conn.rollback();
            console.error(err);
        } finally {
            if (conn) { conn.end(); }
        }
    }

    static async updateTimeSlot(timeSlotId, timeSlot) {
        const query = `UPDATE Time_Slot
            SET day = ${mariaDB.escape(timeSlot.day)},
            start_time = ${mariaDB.escape(timeSlot.start_time)},
            end_time = ${mariaDB.escape(timeSlot.end_time)}
            WHERE id=${mariaDB.escape(timeSlotId)}
            LIMIT 1;
        SELECT * FROM Time_Slot WHERE id=${mariaDB.escape(timeSlotId)};`;
        let conn;
        try {
            conn = await mariaDB.getConnection();
            await conn.beginTransaction();
            const data = await conn.query(query);
            await conn.commit();
            const updatedTimeSlot = data[1][0];
            return updatedTimeSlot;
        } catch (err) {
            await conn.rollback();
            console.error(err);
        } finally {
            if (conn) { conn.end(); }
        }
    }

    static async deleteTimeSlot(timeSlotId) {
        const query = `DELETE FROM Time_Slot WHERE id=${mariaDB.escape(timeSlotId)};`;
        let conn;
        try {
            conn = await mariaDB.getConnection();
            await conn.beginTransaction();
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

module.exports = TimeSlot;