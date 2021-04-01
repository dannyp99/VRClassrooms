const Hub = require('./hub.js');
const Meeting = require('./meeting.js');

class WorldServer {
    ip          = "";
    instances   = 0;
    meetings    = {};
    hubs        = {};
    open_ports  = [];

    constructor(ip){
        this.ip = ip;
    }

    static startingPort = 7778

    get newPort(){

        if(this.open_ports.length > 0){
            return this.open_ports.pop();
        }
        return WorldServer.startingPort + this.instances;
    }

    createHub(ip, port, name){
        // Create new hub object and add it to the list
        let h = new Hub(ip, port, name);
        if(!this.hubs[name]){
            this.hubs[name] = [h];
        }else{
            this.hubs[name].push(h);
        }
        this.instances++;

        return h;
    }

    getHub(name){
        // Return available hub

        if(this.hubs[name]){
            return this.hubs[name][0];
        }
        return null;
    }

    removeHub(name, port){
        let hubs_list = this.hubs[name];
        if(!hubs_list) return;

        for(let i =0; i< hubs_list.length; i++){
            if(hubs_list[i].port.toString() === port){
                hubs_list.splice(i, 1);
                this.open_ports.push(port);
                this.instances--;
                return true;
            }
        }
        return false;
    }

    createMeeting(ip, port, meeting_id){
        // Create new meeting object and add it to the dict
        let m = new Meeting(ip, port, meeting_id, "Physics");
        this.meetings[meeting_id] = m;
        this.instances++;

        return m;
    }

    getMeeting(meeting_id){
        // Get correct meeting

        return this.meetings[meeting_id];
    }

    removeMeeting(port){
        for(let key in this.meetings){
            if(this.meetings[key].port.toString() === port){
                delete this.meetings[key];
                this.open_ports.push(port);
                this.instances--;
                return true;
            }
        }
        return false;
    }
}

module.exports = WorldServer;