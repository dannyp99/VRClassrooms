require("dotenv/config");
const mariadb = require('mariadb');
var db = mariadb.createPool({
    multipleStatements: true,
    host: process.env.DB_HOST,
    user: process.env.DB_USER,
    password: process.env.DB_PASSWD, 
    database: process.env.DB
});
var dbConn = db.getConnection();
dbConn.then((con) => {
    console.log("Database Connected");
    con.end();
})
.catch(err => {
    console.log(err);
});

module.exports = db;