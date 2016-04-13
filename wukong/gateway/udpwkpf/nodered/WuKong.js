/**
 * Copyright 2015 WuKong
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/


module.exports = function(RED) {
    "use strict";
    var reconnectTime = RED.settings.socketReconnectTime||10000;
    var socketTimeout = RED.settings.socketTimeout||null;
    var net = require('net');

    function WuKongSubscriber(n) {
        RED.nodes.createNode(this,n);
        this.server = n.server;
        this.port = Number(n.port);
        this.out = n.out;
        this.splitc = n.splitc;
        this.topic = n.topic;

        if (this.out != "char") { this.splitc = Number(this.splitc); }
        else { this.splitc = this.splitc.replace("\\n",0x0A).replace("\\r",0x0D).replace("\\t",0x09).replace("\\e",0x1B).replace("\\f",0x0C).replace("\\0",0x00); } // jshint ignore:line

        var buf;
        if (this.out == "count") { buf = new Buffer(this.splitc); }
        else { buf = new Buffer(65536); } // set it to 64k... hopefully big enough for most TCP packets.... but only hopefully

        this.connected = false;
        var node = this;
        var client;
        
        this.on("input", function(msg) {
            var i = 0;
            if ((!Buffer.isBuffer(msg.payload)) && (typeof msg.payload !== "string")) {
                msg.payload = msg.payload.toString();
            }
            if (!node.connected) {
                client = net.Socket();
                if (socketTimeout) { client.setTimeout(socketTimeout); }
                //node.status({});
                var host = node.server || msg.host;
                var port = node.port || msg.port;
                var m;

                if (host && port) {
                    client.connect(port, host, function() {
                        //node.log(RED._("tcpin.errors.client-connected"));
                        node.status({fill:"green",shape:"dot",text:"common.status.connected"});
                        node.connected = true;
                        /*
                         * Send the subscription message
                         */
                        var message = node.generateSubscriptionMessage()
                        if (message){
                            client.write(message)
                        }
                        //
                        //client.write(msg.payload);
                    });
                }
                else {
                    node.warn(RED._("tcpin.errors.no-host"));
                }

                client.on('data', function(data) {
                    /*
                     * decode the message from WuKong
                     */
                    var boundary = '||'
                    var chunks = data.toString().split(boundary)
                    var residue = []
                    var messages = []
                    for (var i=0;i<chunks.length;i++){
                        var chunk=chunks[i].trim();
                        if (!chunk) continue
                        try{
                            messages.push(JSON.parse(chunk))
                        }
                        catch(e){
                            residue.push(chunk)
                        }
                    }
                    for (var i=0;i<messages.length;i++){
                        var data = messages[i].value
                        if (node.out == "sit") { // if we are staying connected just send the buffer
                            node.send({"payload": data});
                        }
                        else if (node.splitc === 0) {
                            node.send({"payload": data});
                        }
                        else {
                            for (var j = 0; j < data.length; j++ ) {
                                if (node.out === "time")  {
                                    // do the timer thing
                                    if (node.tout) {
                                        i += 1;
                                        buf[i] = data[j];
                                    }
                                    else {
                                        node.tout = setTimeout(function () {
                                            node.tout = null;
                                            m = new Buffer(i+1);
                                            buf.copy(m,0,0,i+1);
                                            node.send({"payload":m});
                                            //if (client) { client.end(); }
                                        }, node.splitc);
                                        i = 0;
                                        buf[0] = data[j];
                                    }
                                }
                                // count bytes into a buffer...
                                else if (node.out == "count") {
                                    buf[i] = data[j];
                                    i += 1;
                                    if ( i >= node.splitc) {
                                        m = new Buffer(i);
                                        buf.copy(m,0,0,i);
                                        node.send({"payload":m});
                                        //if (client) { client.end(); }
                                        i = 0;
                                    }
                                }
                                // look for a char
                                else {
                                    buf[i] = data[j];
                                    i += 1;
                                    if (data[j] == node.splitc) {
                                        m = new Buffer(i);
                                        buf.copy(m,0,0,i);
                                        node.send({"payload":m});
                                        //if (client) { client.end(); }
                                        i = 0;
                                    }
                                }
                            }
                        }
                        
                    }
                });

                client.on('end', function() {
                    node.connected = false;
                    node.status({});
                    node.send({"payload":m});
                    client = null;
                });

                client.on('close', function() {
                    node.status({});
                    if (node.done) { node.done(); }
                });

                client.on('error', function() {
                    node.error(RED._("tcpin.errors.connect-fail"),msg);
                    node.status({fill:"red",shape:"ring",text:"common.status.error"});
                    if (client) { client.end(); }
                });

                client.on('timeout',function() {
                    node.warn(RED._("tcpin.errors.connect-timeout"));
                    if (client) {
                        client.end();
                        setTimeout(function() {
                            client.connect(port, host, function() {
                                node.connected = true;
                                node.status({fill:"green",shape:"dot",text:"common.status.connected"});
                                client.write(msg.payload);
                            });
                        },reconnectTime);
                    }
                });
            }
            else { 
                /*
                 * Send the subscription message
                 */
                var message = node.generateSubscriptionMessage()
                if (message){
                    client.write(message)
                }
                //client.write(msg.payload);
            }
        });

        this.on("close", function(done) {
            node.done = done;
            if (client) {
                buf = null;
                client.destroy();
            }
            if (!node.connected) { done(); }
        });
        /*
         *
         */
        this.generateSubscriptionMessage = function(){
            if ((!node.topic) || (node.topic == '0')) return null;
            var boundary = '||'
            return boundary+JSON.stringify({
                topic:parseInt(node.topic),
                type:'*',
                value:'*',
                subscribe:1,
            })+boundary;
        }
    }
    RED.nodes.registerType("WuKong Subscriber",WuKongSubscriber);


    function WuKongPublisher(n) {
        RED.nodes.createNode(this,n);
        this.host = n.host;
        this.port = n.port * 1;
        this.base64 = n.base64;
        this.doend = n.end || false;
        this.beserver = n.beserver;
        this.name = n.name;
        this.closing = false;
        this.connected = false;
        this.topic = n.topic;
        this.publishing_property = n.publishing_property;
        var node = this;

        if (!node.beserver||node.beserver=="client") {
            var reconnectTimeout;
            var client = null;
            var end = false;

            var setupTcpClient = function() {
                node.log(RED._("tcpin.status.connecting",{host:node.host,port:node.port}));
                node.status({fill:"grey",shape:"dot",text:"common.status.connecting"});
                client = net.connect(node.port, node.host, function() {
                    node.connected = true;
                    node.log(RED._("tcpin.status.connected",{host:node.host,port:node.port}));
                    node.status({fill:"green",shape:"dot",text:"common.status.connected"});
                });
                client.on('error', function (err) {
                    node.log(RED._("tcpin.errors.error",{error:err.toString()}));
                });
                client.on('end', function (err) {
                });
                client.on('close', function() {
                    node.status({fill:"red",shape:"ring",text:"common.status.disconnected"});
                    node.connected = false;
                    client.destroy();
                    if (!node.closing) {
                        if (end) {
                            end = false;
                            reconnectTimeout = setTimeout(setupTcpClient,20);
                        }
                        else {
                            node.log(RED._("tcpin.errors.connection-lost",{host:node.host,port:node.port}));
                            reconnectTimeout = setTimeout(setupTcpClient,reconnectTime);
                        }
                    } else {
                        if (node.done) { node.done(); }
                    }
                });
            }
            setupTcpClient();

            node.on("input", function(msg) {
                if (node.connected && msg.payload != null) {
                    /*
                    if (Buffer.isBuffer(msg.payload)) {
                        client.write(msg.payload);
                    } else if (typeof msg.payload === "string" && node.base64) {
                        client.write(new Buffer(msg.payload,'base64'));
                    } else {
                        client.write(new Buffer(""+msg.payload));
                    }
                    */
                    var boundary = '||';
                    var message = {
                        topic:parseInt(node.topic),
                        type:'',
                        value:'',
                    }
                    var prop = node.publishing_property.trim() || 'payload'
                    var data = msg[prop]
                    if (Buffer.isBuffer(data)) {
                        message.value = data.toString()
                        message.type = 'string'
                    } else if (typeof data === "string" && node.base64) {
                        message.value = new Buffer(data,'base64').toString()
                        message.type = 'string'
                    } else {
                        message.value = data
                        message.type = typeof(data)
                        if (message.type =='number') message.type = 'short'
                    }
                    client.write(new Buffer(boundary+JSON.stringify(message)+boundary+'\n'))
                    if (node.doend === true) {
                        end = true;
                        if (client) { client.end(); }
                    }
                }
            });

            node.on("close", function(done) {
                node.done = done;
                this.closing = true;
                if (client) { client.end(); }
                clearTimeout(reconnectTimeout);
                if (!node.connected) { done(); }
            });

        } else if (node.beserver == "reply") {
            node.on("input",function(msg) {
                if (msg._session && msg._session.type == "tcp") {
                    var client = connectionPool[msg._session.id];
                    if (client) {
                        if (Buffer.isBuffer(msg.payload)) {
                            client.write(msg.payload);
                        } else if (typeof msg.payload === "string" && node.base64) {
                            client.write(new Buffer(msg.payload,'base64'));
                        } else {
                            client.write(new Buffer(""+msg.payload));
                        }
                    }
                }
            });
        } else {
            var connectedSockets = [];
            node.status({text:RED._("tcpin.status.connections",{count:0})});
            var server = net.createServer(function (socket) {
                socket.setKeepAlive(true,120000);
                if (socketTimeout !== null) { socket.setTimeout(socketTimeout); }
                var remoteDetails = socket.remoteAddress+":"+socket.remotePort;
                node.log(RED._("tcpin.status.connection-from",{host:socket.remoteAddress, port:socket.remotePort}));
                connectedSockets.push(socket);
                node.status({text:RED._("tcpin.status.connections",{count:connectedSockets.length})});
                socket.on('timeout', function() {
                    node.log(RED._("tcpin.errors.timeout",{port:node.port}));
                    socket.end();
                });
                socket.on('close',function() {
                    node.log(RED._("tcpin.status.connection-closed",{host:socket.remoteAddress, port:socket.remotePort}));
                    connectedSockets.splice(connectedSockets.indexOf(socket),1);
                    node.status({text:RED._("tcpin.status.connections",{count:connectedSockets.length})});
                });
                socket.on('error',function() {
                    node.log(RED._("tcpin.errors.socket-error",{host:socket.remoteAddress, port:socket.remotePort}));
                    connectedSockets.splice(connectedSockets.indexOf(socket),1);
                    node.status({text:RED._("tcpin.status.connections",{count:connectedSockets.length})});
                });
            });

            node.on("input", function(msg) {
                if (msg.payload != null) {
                    var buffer;
                    if (Buffer.isBuffer(msg.payload)) {
                        buffer = msg.payload;
                    } else if (typeof msg.payload === "string" && node.base64) {
                        buffer = new Buffer(msg.payload,'base64');
                    } else {
                        buffer = new Buffer(""+msg.payload);
                    }
                    for (var i = 0; i < connectedSockets.length; i += 1) {
                        if (node.doend === true) { connectedSockets[i].end(buffer); }
                        else { connectedSockets[i].write(buffer); }
                    }
                }
            });

            server.on('error', function(err) {
                if (err) {
                    node.error(RED._("tcpin.errors.cannot-listen",{port:node.port,error:err.toString()}));
                }
            });

            server.listen(node.port, function(err) {
                if (err) {
                    node.error(RED._("tcpin.errors.cannot-listen",{port:node.port,error:err.toString()}));
                } else {
                    node.log(RED._("tcpin.status.listening-port",{port:node.port}));
                    node.on('close', function() {
                        for (var c in connectedSockets) {
                            if (connectedSockets.hasOwnProperty(c)) {
                                connectedSockets[c].end();
                                connectedSockets[c].unref();
                            }
                        }
                        server.close();
                        node.log(RED._("tcpin.status.stopped-listening",{port:node.port}));
                    });
                }
            });
        }
    }
    RED.nodes.registerType("WuKong Publisher",WuKongPublisher);
}