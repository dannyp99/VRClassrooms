const Router    = require('@koa/router');
const CONFIG    = require('./config.js');
const {spawn}   = require('child_process');
require('dotenv/config')

const router = new Router();

const ServerUser = process.env.SERVER_USERNAME;
const ServerPasswd = process.env.SERVER_PASSWD;

router.post('/', (ctx, next) => {
    const body = ctx.request.body;
    console.log(`Server ${body.name}`);
    const sub = spawn(CONFIG.UE4_PATH, [CONFIG.PROJECT_PATH, `${body.name}?listen?ip=${CONFIG.ip}?port=${body.port}?serverUserName=${ServerUser}?serverPasswd=${ServerPasswd}`, '-server', '-log', '-nosteam', '-messaging', `-port=${body.port}`], {
        detached: true,
        stdio: 'ignore'
    });

    sub.unref();

    ctx.response.status = 204;
});

router.post('/meeting', (ctx, next) => {
    const body = ctx.request.body;
    console.log(`Classroom ${body.name} Id: ${body.meetingId}`);
    const sub = spawn(CONFIG.UE4_PATH, [CONFIG.PROJECT_PATH, `${body.name}?listen?ip=${CONFIG.ip}?port=${body.port}?serverUserName=${ServerUser}?serverPasswd=${ServerPasswd}?meetingId=${body.meetingId}`, '-server', '-log', '-nosteam', '-messaging', `-port=${body.port}`], {
        detached: true,
        stdio: 'ignore'
    });

    sub.unref();

    ctx.response.status = 204;
});

module.exports = router;