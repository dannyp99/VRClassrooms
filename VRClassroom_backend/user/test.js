const request = require('supertest');
const server = request.agent('http://localhost:1337');
require("dotenv/config");
const log = require('log-to-file');
const fileName = 'user-tests.log';
let auth;
let superAuth;
let userId;
let newId;

describe('POST /login', function() {
    it('login', function(done) {
    server
        .post('/login')
        .send({ username: 'sampleUser', password: 'newpassword' })
        .expect(200)
        .end(function(err,res) {
            if (err) return done(err);
            log(res.text, fileName);
            auth = "Bearer " + res.text;
            return done();
        });
    });
});

describe('POST /login', function() {
    it('login', function(done) {
    server
        .post('/login')
        .send({ username: process.env.TEST_USER, password: process.env.TEST_PASSWD })
        .expect(200)
        .end(function(err,res) {
            if (err) return done(err);
            log(res.text, fileName);
            superAuth = "Bearer " + res.text;
            return done();
        });
    });
});

describe('User Endpoint /users', function() {
    it('auth users/', function(done) {
    server
        .get('/users')
        .expect(200)
        .set('Authorization', auth)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.body, fileName);
            userId = res.body.id
            done();
        });
    });
    it('unauth users/:id', function (done) {
    server
        .get(`/users/${userId}`)
        .expect(401)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth users/:id', function(done) {
    server
        .get(`/users/${userId}`)
        .expect(200)
        .set('Authorization', auth)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.body, fileName);
            done();
        });
    });
    it('unauth users/meetings/', function (done) {
    server
        .get('/users/meetings/')
        .expect(401)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth users/meetings/', function(done) {
    server
        .get('/users/meetings/')
        .expect(200)
        .set('Authorization', auth)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.body, fileName);
            done();
        });
    });
    it('unauth update users/', function(done) {
    server
        .put(`/users/`)
        .query({name: 'updatedTestCaseUser'})
        .expect(401)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth update users/', function(done) {
    server
        .put(`/users/`)
        .set('Authorization', auth)
        .query({name: 'updatedTestCaseUser'})
        .expect(200)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('unauth update password users/', function(done) {
    server
        .put(`/users/`)
        .send({
            "password": "newpassword",
            "newPasswd": "newpassword"
        })
        .expect(401)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth update password users/', function(done) {
    server
        .put(`/users/`)
        .set('Authorization', auth)
        .send({
            "password": "newpassword",
            "newPasswd": "newpassword"
        })
        .expect(200)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('unauth create users/', function(done) {
    server
        .post('/users/')
        .expect(401)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth not admin create users/', function(done) {
    server
        .post('/users/')
        .set('Authorization', auth)
        .send({
            "role": "Test",
            "username": "testCaseUser",
            "email": "testCaseUser@api.com",
            "password": "password",
            "display_name": "TestCaseUser"
        })
        .expect(403)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth create users/', function(done) {
    server
        .post('/users/')
        .set('Authorization', superAuth)
        .send({
            "role": "Test",
            "username": "testCaseUser",
            "email": "testCaseUser@api.com",
            "password": "password",
            "display_name": "TestCaseUser"
        })
        .expect(201)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.body, fileName);
            newId = res.body.id;
            done();
        });
    });
    it('unauth search users/', function(done) {
    server
        .get('/users')
        .expect(401)
        .end(function (err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth search users/', function(done) {
    server
        .get('/users')
        .query({name: 'testCase'})
        .expect(200)
        .set('Authorization', auth)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.body, fileName);
            done();
        });
    });
    it('unauth delete users/:id', function(done) {
    server
        .delete(`/users/${newId}`)
        .expect(401)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth not admin delete users/:id', function(done) {
    server
        .delete(`/users/${newId}`)
        .expect(403)
        .set('Authorization', auth)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth not admin delete users/:id', function(done) {
    server
        .delete(`/users/${newId}`)
        .expect(204)
        .set('Authorization', superAuth)
        .end(function(err, res) {
            if (err) return done(err);
            log("Successfully Deleted", fileName);
            done();
        });
    });
});