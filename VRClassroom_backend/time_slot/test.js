const request = require('supertest');
const server = request.agent('http://localhost:1337');
const log = require('log-to-file');
const fileName = 'timeslot-tests.log';
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

describe('TimeSlots Endpoint /timeslots', function() {
    it('unauth timeslots/', function (done) {
    server
        .get('/timeslots')
        .expect(401)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth timeslots/', function(done) {
    server
        .get('/timeslots')
        .set('Authorization', auth)
        .expect(200)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.body, fileName);
            done();
        });
    });
    it('unauth create timeslots/', function(done) {
    server
        .post('/timeslots')
        .send({
            "day": 1,
            "start_time": "1",
            "end_time": "2"
        })
        .expect(401)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth create timeslots/', function(done) {
    server
        .post('/timeslots')
        .set('Authorization', auth)
        .send({
            "day": 1,
            "start_time": "1",
            "end_time": "2"
        })
        .expect(201)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.body, fileName);
            newId = res.body.id;
            done();
        });
    });
    it('unauth timeslots/:id', function (done) {
    server
        .get(`/timeslots/${newId}`)
        .expect(401)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth timeslots/:id', function(done) {
    server
        .get(`/timeslots/${newId}`)
        .set('Authorization', auth)
        .expect(200)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.body, fileName);
            done();
        });
    });
    it('unauth timeslots/meetings/:id', function(done) {
    server
        .get(`/timeslots/meetings/${newId}`)
        .expect(401)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth timeslots/meetings/:id', function(done) {
    server
        .get(`/timeslots/meetings/${newId}`)
        .expect(200)
        .set('Authorization', auth)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('unauth update timeslots/:id', function(done) {
    server
        .put(`/timeslots/${newId}`)
        .send({
            "day": 1,
            "start_time": "4",
            "end_time": "5"
        })
        .expect(401)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth update timeslots/:id', function(done) {
        server
        .put(`/timeslots/${newId}`)
        .set('Authorization', auth)
        .send({
            "day": 1,
            "start_time": "4",
            "end_time": "5"
        })
        .expect(200)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.body, fileName);
            done();
        });
    });
    it('unauth delete timeslots/:id', function (done) {
    server
        .delete(`/timeslots/${newId}`)
        .expect(401)
        .end(function(err, res) {
            if (err) return done(err);
            log(res.text, fileName);
            done();
        });
    });
    it('auth delete timeslots/:id', function(done) {
    server
        .delete(`/timeslots/${newId}`)
        .expect(204)
        .set('Authorization', auth)
        .end(function(err, res) {
            if (err) return done(err);
            log("Successfully Deleted", fileName);
            done();
        });
    });
});
