const Koa           = require('koa');
const Router        = require("@koa/router");
const bodyParser    = require('koa-body');
const fetch         = require('node-fetch');
const publicIp      = require('public-ip');
const UE_CONFIG     = require("./ue4/config.js");

async function main(){

    const ue4Router = require('./ue4/router.js');

    const app = new Koa();
    const router = new Router();

    router.use('/ue4', ue4Router.routes(), ue4Router.allowedMethods());

    app.use(bodyParser());

    app.use(router.routes()).use(router.allowedMethods());

    const ip = await publicIp.v4();
    UE_CONFIG.ip = ip;

    const api = 'https://api.ev3.me/';
    //const api = 'http://localhost:1337/';

    const response = await fetch(`${api}instances/add-world-server`, {
        headers: {
            'X-Orig-IP': ip
        }
    });
    if(!response.ok){
        console.error('Couldn\'t connect to API');
        return;
    }
    // const data = await response.json();
    // console.log(data);

    app.listen(8181, () => console.log('Server Started'));

    
}

main();