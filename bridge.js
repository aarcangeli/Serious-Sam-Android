const dgram = require('dgram');
const server = dgram.createSocket('udp4');
const client = dgram.createSocket('udp4');

/**
 * To play in network with the emulator:
 * telnet 127.0.0.1 5554 (the port may be different)
 * auth PQ6AA3GMjjTMISP8 (read the key from %USERPROFILE%/.emulator_console_auth_token on windows or $home/.emulator_console_auth_token on linux)
 * redir add udp:25600:25600
 * exit
 * node bridge.js
 * join game to port 25601
 */

let phone;

server.on('error', (err) => {
    console.log(`server error:\n${err.stack}`);
    server.close();
});

server.on('message', (msg, rinfo) => {
    phone = rinfo;
    console.log(`server got: ${msg.length} bytes from ${rinfo.address}:${rinfo.port}`);
    client.send(msg, 25600, 'localhost');
});

client.on('message', (msg, rinfo) => {
    console.log(`client got: ${msg.length} bytes from ${rinfo.address}:${rinfo.port}`);
    server.send(msg, phone.port, phone.address);
});

server.on('listening', () => {
    const address = server.address();
    console.log(`server listening ${address.address}:${address.port}`);
});

server.bind(25601);
