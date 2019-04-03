package main

import (
	"encoding/json"
	"io/ioutil"
	"net/http"
	"os"
)

type IpList struct {
	IP []string
}

func main() {
	http.HandleFunc("/", helloServer)
	http.ListenAndServe("0.0.0.0:8091", nil)
}

func helloServer(w http.ResponseWriter, req *http.Request) {
	result := getGateWayInfo()
	w.Write([]byte(result))
}

func getGateWayInfo() string {
	var ipList IpList
	getIpList(&ipList)

	var info string
	for _, ip := range ipList.IP {
		info += ip + " : "
		url := "http://" + ip + ":8090/"
		resp, err := http.Get(url)
		if err != nil {
			info += ":not running" + "\r\n"
			continue
		}
		defer resp.Body.Close()

		body, _ := ioutil.ReadAll(resp.Body)

		info += string(body) + "\r\n"
	}

	return info
}

func getIpList(ip_list *IpList) {
	file, err := os.OpenFile("config.json", os.O_RDWR, 0)
	if err != nil {
		return
	}
	defer file.Close()

	denc := json.NewDecoder(file)
	err = denc.Decode(&ip_list)
}
