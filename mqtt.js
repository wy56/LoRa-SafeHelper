var mqtt = require('mqtt');
var client = mqtt.connect('mqtt://student:lora_student@140.115.52.100', 1883);
var ldr = 0;

function get_ldr_volume(data) {
    var find_base64 = data.search('"data"') + 8;
    var base64 = data.substring(find_base64, find_base64 + 8);
    return Buffer(base64, 'base64').toString();
}

client.on('connect', function() {
    console.log('已連接至MQTT伺服器');
    //client.subscribe('application/#'); // global
    client.subscribe('application/2/node/008000000000f313/rx/#') // My LoRa
});

client.on('message', function (topic, msg) {
    //console.log('收到 ' + topic + '主題  訊息：' + msg.toString());
    ldr = get_ldr_volume(msg.toString());
    console.log(ldr);  
}); 

var server,
    ip   = "140.115.59.90",
    port = 1111,
    http = require('http'),
    fs = require("fs"),
    url = require('url'),
    path,
    filePath,
    encode = "utf8";

server = http.createServer(function (req, res) {
    path = url.parse(req.url);

    res.writeHead(200, {'Content-Type': 'text/plain'});
    if(ldr >= 50000) {
        fs.readFile("open.html" , encode, function(err, file) {
            if (err) {
                res.writeHead(404, {
                    'Content-Type': 'text/plain'
                });
                res.end();
                return;
            }
    
            res.writeHead(200, {
                'Content-Type': 'text/html'
            });
            res.write(file);    
            res.end();
        });
    }
    else {
        fs.readFile("close.html" , encode, function(err, file) {
            if (err) {
                res.writeHead(404, {
                    'Content-Type': 'text/plain'
                });
                res.end();
                return;
            }
    
            res.writeHead(200, {
                'Content-Type': 'text/html'
            });
            res.write(file);    
            res.end();
        });
    }
    
    //if(ldr >= 50000) {
      //  res.end('目前保險櫃已開啟'); 
    //}
    //else {
      //  res.end('目前保險會很黑!!!!!!!!!');
    //}
});

server.listen(port, ip);

console.log("Server running at http://" + ip + ":" + port);