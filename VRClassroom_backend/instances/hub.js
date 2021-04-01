class Hub {
    ip          = "";
    port        = "";
    name        = "";
    players     = [];

    constructor(ip, port, name){
        this.ip     = ip;
        this.port   = port;
        this.name   = name;
    }
}

module.exports = Hub;