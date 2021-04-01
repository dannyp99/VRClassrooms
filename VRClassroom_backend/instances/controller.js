const WorldServer   = require('./world_server.js');
const fetch         = require('node-fetch');

const servers = {};
let controller;
class InstancesController {

    addWorldServer(ctx) {

        const ip = ctx.request.headers["x-orig-ip"];
        let s = new WorldServer(ip);
        servers[ip] = s;

        console.log(`Adding Server ${ip}`);

        ctx.response.status = 204;
    }

    async joinServer(ctx) {
        let hub = controller.getHub('Hofstra');
        if(!hub){
            hub = await controller.createHub('Hofstra');
        }
        if(!hub){
            ctx.response.status = 400;
        }else{
            ctx.response.status = 200;
            ctx.response.body = {
                ip: hub.ip,
                port: `${hub.port}`
            };
        }
    }

    async joinMeeting(ctx) {
        const meeting_id = ctx.params.id;
        let meeting = controller.getMeeting(meeting_id);
        if(!meeting) {
            meeting = await controller.createMeeting(meeting_id);
        }
        if(!meeting) {
            ctx.response.status = 400;
        }else {
            ctx.response.status = 200;
            ctx.response.body = {
                ip: meeting.ip,
                port: `${meeting.port}`
            }
        }
    }

    async createHub(hub_name) {
        let s = null;
        for(let key in servers){
            s = servers[key];
        }
        if(!s){
            console.log('No world server');
            return;
        }
        const body = {
            name: "TestServer",
            port: s.newPort
        }
        const response = await fetch(`http://${s.ip}:8181/ue4`, {
            method: 'POST',
            body: JSON.stringify(body),
            headers: {
                'Content-Type': 'application/json'
            }
        });
        if(!response.ok){
            return null;
        }
        return s.createHub(s.ip, body.port, hub_name);
    }

    getHub(hub_name) {
        if(servers.length === 0){
            return null;
        }
        let hub = null;
        for(let key in servers){
            hub = servers[key].getHub(hub_name);
            if(hub){
                break;
            }
        }

        return hub;
    }

    removeHub(ctx){
        const ip = ctx.request.body.ip;
        const port = ctx.request.body.port;

        if(!(ip && port)){
            ctx.throw(400, 'Missing parameters');
        }

        let server = servers[ip];
        if(server){
            if(server.removeHub('Hofstra', port)){
                ctx.status = 204;
            }else{
                ctx.throw(400, 'No instance found');
            }

        }else{
            ctx.throw(400, 'No server found');
        }
    }


    async createMeeting(meeting_id) {
        let s = null;
        for(let key in servers){
            s = servers[key];
        }
        if(!s){
            console.log('No world server');
            return;
        }
        const body = {
            name: "TestClassroom",
            port: s.newPort
        }
        const response = await fetch(`http://${s.ip}:8181/ue4/meeting`, {
            method: 'POST',
            body: JSON.stringify(body),
            headers: {
                'Content-Type': 'application/json'
            }
        });
        if(!response.ok){
            return null;
        }
        return s.createMeeting(s.ip, body.port, meeting_id);
    }

    getMeeting(meeting_id) {
        if(servers.length === 0){
            return null;
        }
        let meeting = null;
        for(let key in servers){
            meeting = servers[key].getMeeting(meeting_id);
            if(meeting){
                break;
            }
        }
        return meeting;
    }

    removeMeeting(ctx){
        const ip = ctx.request.body.ip;
        const port = ctx.request.body.port;

        if(!(ip && port)){
            ctx.throw(400, "Missing parameters");
        }

        let server = servers[ip];
        if(server){
            if(server.removeMeeting(port)){
                ctx.status = 204;
            }else{
                ctx.throw(400, "No instance found");
            }

        }else{
            ctx.throw(400, "No server found");
        }
    }
}

controller = new InstancesController();

module.exports = controller;
