const mariaDB = require('../database');
const bcrypt = require('bcrypt');
require("dotenv/config");
const saltRounds = Number(process.env.SALT);
class User {

    static async getCurrentUser(userId) {
        return this.getUser(userId);
    }
    
    static async getUser(userId) {
        const query = `SELECT * FROM User WHERE id=${mariaDB.escape(userId)};`;
        let conn;
        try {
            conn = await mariaDB.getConnection();
            const user = await conn.query(query);
            return user;
        } catch (err) {
            console.error(err);
        } finally {
            if (conn) { conn.end(); }
        }
    }
    // In case endpoint to get all data on a user is needed
    // static async getAllUserData(userId) {
    //     const query = `SELECT * FROM User AS usr
    //     LEFT JOIN User_Meeting AS um ON usr.id = um.user_id
    //     LEFT JOIN Meeting AS m ON m.id = um.meeting_id
    //     LEFT JOIN Role AS rle ON usr.role = rle.name
    //     LEFT JOIN Role_Permission AS rp ON rle.id = rp.role_id
    //     LEFT JOIN Permission AS pm ON rp.permission_id = pm.id
    //     WHERE usr.id = ${mariaDB.escape(userId)}
    //     LIMIT 1;`;
    //     let conn;
    //     try {
    //         conn = await mariaDB.getConnection();
    //         const user = await conn.query(query);
    //         return user;
    //     } catch (err) {
    //         console.error(err);
    //     } finally {
    //         if (conn) { conn.end(); }
    //     }
    // }

    // Possibly not going to need to join for permissions
    static async getUserPermissions(userId) {
        const query = `SELECT usr.*, rle.name as role_name, rle.description, rle.permissions, pm.action, pm.target, pm.global
            FROM backend.User AS usr
            LEFT JOIN backend.Role AS rle ON usr.role = rle.name
            LEFT JOIN backend.Role_Permission AS rp ON rle.id = rp.role_id
            LEFT JOIN backend.Permission AS pm ON rp.permission_id = pm.id
            WHERE usr.id = ${mariaDB.escape(userId)}
            LIMIT 1;`;
        let conn;
        try {
            conn = await mariaDB.getConnection();
            const userData = await conn.query(query);
            const user = userData[0];
            return user;
        } catch (err) {
            console.error(err);
        } finally {
            if (conn) { conn.end(); }
        }
    }

    static async searchUser(name) {
        name += '%';
        const query = `SELECT * FROM User WHERE username LIKE ${mariaDB.escape(name)};`;
        let conn;
        try {
            conn = await mariaDB.getConnection();
            const users = await conn.query(query);
            return users;
        } catch (err) {
            console.error(err);
        } finally {
            if (conn) { conn.end(); }
        }
    }

    static async createUser(user) {
        if (!user || !user.password) { return; }
        let conn;
        try {
            conn = await mariaDB.getConnection();
            const hashPass = await bcrypt.hash(user.password, saltRounds);
            await conn.beginTransaction();
            const query = `INSERT INTO User (role, username, email, password_hash, display_name)
                VALUES (${mariaDB.escape(user.role)}, ${mariaDB.escape(user.username)}, ${mariaDB.escape(user.email)}, '${hashPass}', ${mariaDB.escape(user.display_name)});
                SELECT * FROM User ORDER BY id DESC LIMIT 1;`;
            const newData = await conn.query(query);
            const newUser = newData[1][0];
            await conn.commit();
            return newUser;
        } catch (err) {
            conn.rollback();
            console.error(err);
        } finally {
            if (conn) { conn.end(); }
        }
    }

    static async updateUser(userId, displayName) {
        if (!displayName) { return; }
        const query = `UPDATE User
            SET display_name = ${mariaDB.escape(displayName)}
            WHERE id = ${mariaDB.escape(userId)}
            LIMIT 1;
        SELECT * FROM User WHERE id=${mariaDB.escape(userId)};`;
        let conn;
        try {
            conn = await mariaDB.getConnection();
            await conn.beginTransaction();
            const data = await conn.query(query);
            await conn.commit();
            const updatedUser = data[1][0];
            return updatedUser;
        } catch (err) {
            await conn.rollback();
            console.error(err);
        } finally {
            if (conn) { conn.end(); }
        }
    }

    static async updatedPassword(userId, user) {
        if (!user) { return; }
        let query = `SELECT password_hash FROM User WHERE id=${mariaDB.escape(userId)};`;
        let conn;
        try {
            conn = await mariaDB.getConnection();
            const hashedPasswd = await conn.query(query);
            const authUser = await bcrypt.compare(user.password, hashedPasswd[0].password_hash);
            if (authUser) {
                await conn.beginTransaction();
                const newHashedPasswd = await bcrypt.hash(user.newPasswd, saltRounds);
                query = `UPDATE User
                    SET password_hash = '${newHashedPasswd}'
                    WHERE id=${mariaDB.escape(userId)}
                    LIMIT 1;
                SELECT * FROM User WHERE id=${mariaDB.escape(userId)};`;
                const data = await conn.query(query);
                await conn.commit();
                const updatedUser = data[1][0];
                return updatedUser;
            } else {
                console.error('Wrong password User password not updated');
                return;
            }
        } catch (err) {
            await conn.rollback();
            console.error(err);
        } finally {
            if (conn) { conn.end(); }
        }
    }

    static async deleteUser(userId) {
        const query = `DELETE FROM User WHERE id=${mariaDB.escape(userId)};`;
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


module.exports = User;