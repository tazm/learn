let http = require("http")
let fs = require("fs")

let buffer = fs.readFileSync('./config.json', 'utf-8')
let config = JSON.parse(buffer)

//普通方法
/*
function GetGateWayStatus(inResult) {
    let gatewayStatus = ''
    for (let index in config.ip) {
        let url = 'http://' + config.ip[index] + ':8090/'
        let client = http.get(url, res => {
            let data = ''
            res.setEncoding('utf-8')  // 设置编码后读取的数据就是字符串,而不是buffer二进制;
            res.on('data', function (chunk) {
                data += chunk
            })
            res.on('end', function () {
                gatewayStatus += config.ip[index] + ' : ' + data
                if (++index == config.ip.length) {
                    inResult.end(gatewayStatus)
                }
            })
        });

        client.on('error', error => {
            gatewayStatus += config.ip[index] + ' : no running'
            if (++index == config.ip.length) {
                inResult.end(gatewayStatus)
            }
        })

        client.end()
    }
}
*/

function GetStatus(ip) {
    return new Promise(resolve => {
        let gatewayStatus = ''
        let url = 'http://' + ip + ':8090/'
        let client = http.get(url, res => {
            let data = ''
            res.setEncoding('utf-8')  // 设置编码后读取的数据就是字符串,而不是buffer二进制;
            res.on('data', function (chunk) {
                data += chunk
            })
            res.on('end', function () {
                gatewayStatus = ip + ' : ' + data
                resolve(gatewayStatus)
            })
        });

        client.on('error', error => {
            gatewayStatus = ip + ' : no running'
            resolve(gatewayStatus)
        })

        client.end()
    })
}

//Promise
/* function GetGateWayStatus(inResponse) {
    let requests = config.ip.map(ip => GetStatus(ip))
    Promise.all(requests).then(result => {
        inResponse.end(result.join('-'))
    })
} */

// async await
async function GetGateWayStatus(inResponse) {
    let requests = config.ip.map(ip => GetStatus(ip))
    let results = await Promise.all(requests)
    inResponse.end(results.join('-'))
}

let server = http.createServer(function (req, res) {
    if (req.url == '/') {
        res.writeHead(200, { 'Content-Type': 'application/json' })
        console.log('recv request')
        GetGateWayStatus(res)
    }
})
server.listen(8091, "0.0.0.0")
console.log('start')
