const dgram = require('dgram');
const server = dgram.createSocket('udp4');

/**
 * To play in network with the emulator:
 * telnet 127.0.0.1 5554 (the port may be different)
 * auth PQ6AA3GMjjTMISP8 (read the key from %USERPROFILE%/.emulator_console_auth_token on windows or $home/.emulator_console_auth_token on linux)
 * redir add udp:25600:25600
 * exit
 * node bridge.js
 * join game to port 25601
 *
 * ip address of host is 10.0.2.2
 */

let clients = [];

server.on('error', (err) => {
    console.log(`server error:\n${err.stack}`);
    server.close();
});

server.on('message', (msg, rinfo) => {
    let serverInfo = `${rinfo.address}:${rinfo.port}`;
    let client = clients.find(it => it.serverInfo == serverInfo);
    if (!client) {
        console.log(`Got new client ${rinfo.address}:${rinfo.port}`)

        client = {serverInfo: serverInfo, udp: dgram.createSocket('udp4')};

        client.udp.on('message', (cmsg, crinfo) => {
            //console.log(`client got: ${msg.length} bytes from ${crinfo.address}:${crinfo.port}`);
            server.send(cmsg, rinfo.port, rinfo.address);
        });

        clients.push(client);
    }

    //console.log(`server got: ${msg.length} bytes from ${rinfo.address}:${rinfo.port}`);
    client.udp.send(msg, 25600, 'localhost');

});

server.on('listening', () => {
    const address = server.address();
    console.log(`server listening ${address.address}:${address.port}`);
});

server.bind(25600);
