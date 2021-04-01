const db = require('../database.js');
const jwt = require('jsonwebtoken');
const bcrypt = require('bcrypt');

const secret = process.env.JWT_SECRET || 'outer-possible-ring-meet';

class Login {
  /**
   * Verifies raw username and password with the DB
   * @param {ctx} Koa Context
   */
  static async verifyUser(username) {
    let query = `SELECT * FROM backend.User WHERE username=${db.escape(username)};`;
    let conn;
    try {
      conn = await db.getConnection();
      const dbUser = await conn.query(query);
      return dbUser;
    } catch (err) {
        console.log(err);
    } finally {
        if (conn) { conn.end(); }
    }
  }
  /**
   * Passes username, password to verifyUser. Generates JWT if successful
   * @param {ctx} Koa Context
   */
  static async login(username, password) {
        if (!username) return;
        if (!password) return;

        const user = await this.verifyUser(username);
        
        if (user[0] === undefined) 
          {
            return;
          };

        if (await bcrypt.compare(password, user[0].password_hash)) {
          const payload = { user_id: user[0].id};
          const token = jwt.sign(payload, secret, { expiresIn: '1d' });

          return token;
        } else {
          return;
        }
  }
}
module.exports = Login;