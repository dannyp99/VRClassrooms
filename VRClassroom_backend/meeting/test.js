const request = require('supertest');
const server = request.agent('http://localhost:1337');
const log = require('log-to-file');
const fileName = 'meeting-tests.log';
let auth;
let newId;

describe('POST /login', function() {
    it('login', function(done) {
    server
        .post('/login')
        .send({ username: 'sampleUser', password: 'newpassword' })
        .expect(200)
        .end(function(err,res) {
            if (err) return done(err);
            auth = "Bearer " + res.text;
            return done();
        });
    });
});

describe('Meetings Endpoint /meetings', function() {
    it('unauth meetings/', function (done) {
    server
        .get('/meetings')
        .expect(401)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth meetings/', function(done) {
    server
        .get('/meetings')
        .expect(200)
        .set('Authorization', auth)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.body, fileName);
            done();
        });
    });
    it('unauth create meeting', function(done) {
    server
        .post('/meetings')
        .send({
            "map_id": 1,
            "name": "TestCase",
            "password": "Test",
            "repeating": 0,
            "active": 1,
            "creator_id": 1,
            "users":[1],
            "timeSlots": [1]
        })
        .expect(401)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth create meeting', function(done) {
    server
        .post('/meetings')
        .set('Authorization', auth)
        .send({
            "map_id": 1,
            "name": "TestCase",
            "password": "Test",
            "repeating": 0,
            "active": 1,
            "creator_id": 1,
            "users":[1],
            "timeSlots": [1]
        })
        .expect(201)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.body, fileName);
            newId = res.body.id;
            done();
        });
    });
    it('unauth meetings/:id', function (done) {
    server
        .get(`/meetings/${newId}`)
        .expect(401)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth meetings/:id', function(done) {
    server
        .get(`/meetings/${newId}`)
        .expect(200)
        .set('Authorization', auth)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.body, fileName);
            done();
        });
    });
    it('unauth meetings/users/:id', function (done) {
    server
        .get(`/meetings/users/${newId}`)
        .expect(401)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth meetings/users/:id', function(done) {
    server
        .get(`/meetings/users/${newId}`)
        .expect(403)
        .set('Authorization', auth)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('unauth meetings/timeslots/:id', function (done) {
    server
        .get(`/meetings/timeslots/${newId}`)
        .expect(401)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth meetings/timeslots/:id', function(done) {
    server
        .get(`/meetings/timeslots/${newId}`)
        .expect(200)
        .set('Authorization', auth)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('unauth update meetings/:id', function(done) {
    server
        .put(`/meetings/${newId}`)
        .send({
            "map_id": 1,
            "name": "UpdatedTestCase",
            "password": "Test",
            "repeating": 0,
            "active": 1,
            "creator_id": 1,
            "users":[1],
            "timeSlots": [1]
        })
        .expect(401)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth update meetings/:id', function(done) {
        server
        .put(`/meetings/${newId}`)
        .set('Authorization', auth)
        .send({
            "map_id": 1,
            "name": "UpdatedTestCase",
            "password": "UpdatedTest",
            "repeating": 0,
            "active": 1,
            "creator_id": 1,
            "users":[1],
            "timeSlots": [1]
        })
        .expect(200)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.body, fileName);
            done();
        });
    });
    it('unauth delete meetings/:id', function (done) {
    server
        .delete(`/meetings/${newId}`)
        .expect(401)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth delete meetings/:id', function(done) {
    server
        .delete(`/meetings/${newId}`)
        .expect(204)
        .set('Authorization', auth)
        .end(function(err, res) {
            if (err) return done(err);
            log("Successfully Deleted", fileName);
            done();
        });
    });
});
