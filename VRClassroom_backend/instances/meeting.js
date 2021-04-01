class Meeting{
    ip              = "";
    port            = "";
    meeting_id      = "";
    meeting_name    = "";
    players         = [];

    constructor(ip, port, meeting_id, meeting_name){
        this.ip             = ip;
        this.port           = port;
        this.meeting_id     = meeting_id;
        this.meeting_name   = meeting_name;
    }

    addPlayer(player){
        this.players.push(player);
    }
}

module.exports = Meeting;