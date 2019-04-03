import json
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib import request


def GetGateWayStatus(ip):
    gateway_status = ip + ' : '
    url = 'http://' + ip + ':8090/'
    try:
        f = request.urlopen(url, None, 1)
        data = f.read()
        f.close()
        gateway_status += data.decode('utf-8')
    except Exception as e:
        print('urlopen Exception: ', e)
        gateway_status += 'no running'

    return gateway_status


class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.end_headers()
        if(self.path != '/'):
            self.wfile.write(b'test')
            return

        gateway_status = ''
        f = open('./config.json', 'r')
        config = json.loads(f.read())  # 反序列化
        for ip in config['ip']:
            gateway_status += GetGateWayStatus(ip)
        self.wfile.write(gateway_status.encode('utf-8'))

        print('request')


def http_server():
    httpd = HTTPServer(('0.0.0.0', 8091), SimpleHTTPRequestHandler)
    httpd.serve_forever()


if __name__ == '__main__':
    http_server()
