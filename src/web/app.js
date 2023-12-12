/**
 * Add globals here
 */
var seconds = null;
var wifiConnectInterval = null;
var wifiConnectStatus = null;



var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);

function onLoad(event) {
	initWebSocket();
	initButton();
	updateSliders();
}

function initWebSocket() {
	console.log('Trying to open a WebSocket connection...');
	websocket = new WebSocket(gateway);
	websocket.onopen = onOpen;
	websocket.onclose = onClose;
	websocket.onmessage = onMessage; // <-- add this line

}

function onOpen(event) {
	console.log('Connection opened');

	//WSgetWifiConnectStatus();

	//setInterval(WSgetTime, 5000); //Update time 




}
function onClose(event) {
	console.log('Connection closed');
	setTimeout(initWebSocket, 2000);
}

// Función que se activa al recibir un mensaje WS
function onMessage(event) {

	// Hay que filtrar los JSON que lleguen
	console.log(event.data);
	var response = JSON.parse(event.data);
	console.log(response);

	if (response.id == "WIFILIST") {
		WSupdatetWifiScanList(response);


	}
	else if (response.id == "WIFISTATUS") {
		WSupdateWifiConnectStatus(response);

	}
	else if (response.id == "WIFIINFO") {
		WSupdatetWifiConnectInfo(response);

	}
	else if (response.id == "TIME") {
		WSupdatetTime(response);

	}

	else {

		console.log("ID desconocido");

	}
}



function initButton() {

	document.getElementById('connect_wifi').addEventListener('click', checkCredentials);
	document.getElementById('disconnect_wifi').addEventListener('click', WSdisconnectWIFI);
	document.getElementById('connect_mqtt').addEventListener('click', WSconnectMQTT);
	document.getElementById('disconnect_mqtt').addEventListener('click', WSdisconnectMQTT);
	document.getElementById('scan_wifi').addEventListener('click', WSgetWifiScanList);
	document.getElementById('pid-send_pid').addEventListener('click', WSsendPID);
	document.getElementById('pid-save_pid').addEventListener('click', WSsavePID);
	document.getElementById('inverter-save_url').addEventListener('click', WSsaveInverterURL);

}

function updateSliders()
{
	var sliderKp = document.getElementById("pid-kp");
	var sliderKi = document.getElementById("pid-ki");
	var sliderKd = document.getElementById("pid-kd");

	var outputKp = document.getElementById("pid-kp-value");
	var outputKi = document.getElementById("pid-ki-value");
	var outputKd = document.getElementById("pid-kd-value");

	outputKp.innerHTML = sliderKp.value; // Display the default slider value
	outputKi.innerHTML = sliderKi.value; // Display the default slider value
	outputKd.innerHTML = sliderKd.value; // Display the default slider value

	// Update the current slider value (each time you drag the slider handle)
	sliderKp.oninput = function () {
		outputKp.innerHTML = this.value;
		WSsendPID();
	}

	sliderKi.oninput = function () {
		outputKi.innerHTML = this.value;
		WSsendPID();
	}

	sliderKd.oninput = function () {
		outputKd.innerHTML = this.value;
		WSsendPID();
	}
}

function WSsendPID() {

	var obj = {
		id: "pid",
		data: {
			action: 0,
			kp: document.getElementById("pid-kp").value,
			ki: document.getElementById("pid-ki").value,
			kd: document.getElementById("pid-kd").value,
			min: document.getElementById("pid-min").value,
			max: document.getElementById("pid-max").value,
		}
	}
	var json = JSON.stringify(obj);
	websocket.send(json);
}

function WSsavePID() {

	var obj = {
		id: "pid",
		data: {
			action: 1,
			kp: document.getElementById("pid-kp").value,
			ki: document.getElementById("pid-ki").value,
			kd: document.getElementById("pid-kd").value,
			min: document.getElementById("pid-min").value,
			max: document.getElementById("pid-max").value,
		}
	}
	var json = JSON.stringify(obj);
	websocket.send(json);
}

function WSsaveInverterURL() {

	var obj = {
		id: "inverter",
		data: {
			action: 1,
			url_inverter: document.getElementById("url_inverter").value,

		}
	}
	var json = JSON.stringify(obj);
	websocket.send(json);
}

function WSconnectMQTT() {

	var obj = {
		id: "mqtt",
		data: {
			action: 1,
			mqtt_host: document.getElementById("mqtt_host").value,
			mqtt_host: document.getElementById("mqtt_port").value,
			mqtt_user: document.getElementById("mqtt_user").value,
			mqtt_pass: document.getElementById("mqtt_pass").value,
		}
	}
	var json = JSON.stringify(obj);
	websocket.send(json);
}

function WSdisconnectMQTT() {

	var obj = {
		id: "mqtt",
		data: {
			action: 0,
		}
	}
	var json = JSON.stringify(obj);
	websocket.send(json);
}

function WSconnectWIFI() {

	var obj = {
		id: "wifi",
		data: {
			action: 1,
			ssid: document.getElementById("connect_ssid").value,
			pass: document.getElementById("connect_pass").value,
		}
	}
	var json = JSON.stringify(obj);
	websocket.send(json);
	//startWifiConnectStatusInterval();

	document.getElementById('disconnect_wifi').style.display = 'block';
}

function WSdisconnectWIFI() {

	var obj = {
		id: "wifi",
		data: {
			action: 0,
		}
	}
	var json = JSON.stringify(obj);
	websocket.send(json);
	//WSgetWifiConnectStatus();
}

/**
 *  WS Scan WiFi AP´s available.
 */
function WSgetWifiScanList() {

	document.getElementById('scan_wifi').disabled = true;
	document.getElementById('scan_wifi').value = "Scanning...";

	document.getElementById('connect_ssid').placeholder = "Scanning...";


	//Envía la orden de escanear redes wifi (connect: 2)
	var obj = {
		id: "wifi",
		data: {
			action: 2,
		}
	}
	var json = JSON.stringify(obj);
	websocket.send(json);

}

/**
 *  WS update WiFi AP´s list available.
 */
function WSupdatetWifiScanList(response) {
	console.log("WSupdatetWifiScanList");

	const list = document.getElementById('wifi_list');
	list.innerHTML = '';
	for (var i = 0; i < response.data.length; i++) {
		console.log(response.data[i].SSID);
		const option = document.createElement('option');
		option.value = response.data[i].SSID;
		list.appendChild(option);
	}
	document.getElementById('scan_wifi').disabled = false;
	document.getElementById('scan_wifi').value = "Scan";
	document.getElementById('connect_ssid').placeholder = "Scan done!";
}

/**
 * Gets the WiFi connection status.
 */
function WSgetWifiConnectStatus() {
	//Envía la orden de obtener STATUS de Wifi
	var obj = {
		id: "wifi",
		data: {
			action: 3,
		}
	}
	var json = JSON.stringify(obj);
	websocket.send(json);

}

/**
 * Set the WiFi connection status.
 */
function WSupdateWifiConnectStatus(response) {

	if (response.data.STATUS == 1) {
		document.getElementById("wifi_connect_status").innerHTML = "Connecting...";
	}
	else if (response.data.STATUS == 2) {
		wifiConnectStatus = 2;
		document.getElementById("wifi_connect_status").innerHTML = "<h4 class='rd'>Failed to Connect. Please check your AP credentials and compatibility</h4>";
		stopWifiConnectStatusInterval();
	}
	else if (response.data.STATUS == 3) {
		wifiConnectStatus = 3;
		document.getElementById("wifi_connect_status").innerHTML = "<h4 class='gr'>Connection Success!</h4>";
		stopWifiConnectStatusInterval();
		//getConnectInfo();
		WSgetWifiConnectInfo()

	}
	else if (response.data.STATUS == 4) {
		wifiConnectStatus = 4;
		document.getElementById("wifi_connect_status").innerHTML = "<h4 class='gr'>Disconnected!</h4>";
		WSgetWifiConnectInfo()

	}

}

/**
 * Gets the WiFi connection info.
 */
function WSgetWifiConnectInfo() {
	//Envía la orden de obtener Info de Wifi
	var obj = {
		id: "wifi",
		data: {
			action: 4,
		}
	}
	var json = JSON.stringify(obj);
	websocket.send(json);
}

/**
 * Set the connection information for displaying on the web page.
 */
function WSupdatetWifiConnectInfo(response) {

	if (wifiConnectStatus == 3) {

		document.getElementById('ConnectInfo').style.display = 'block';

		document.getElementById("connected_ap_label").innerText = "Connected to: ";
		document.getElementById("connected_ap").innerText = response.data.ssid;

		document.getElementById("ip_address_label").innerText = "IP Address: ";
		document.getElementById("wifi_connect_ip").innerText = response.data.ip;

		document.getElementById("netmask_label").innerText = "Netmask: ";
		document.getElementById("wifi_connect_netmask").innerText = response.data.netmask;

		document.getElementById("gateway_label").innerText = "Gateway: ";
		document.getElementById("wifi_connect_gw").innerText = response.data.gw;
		document.getElementById('disconnect_wifi').style.display = 'block';

	}

	else {
		document.getElementById('ConnectInfo').style.display = 'none';
	}
}

/**
 * Gets date and time from server.
 */
function WSgetTime() {
	//Envía la orden de obtener la fecha y la hora del ESP
	var obj = {
		id: "TIME",

	}
	var json = JSON.stringify(obj);
	websocket.send(json);
	console.log("WSgetTime");
}

/**
 * Set date and time.
 */
function WSupdatetTime(response) {

	document.getElementById("timestamp_label").innerText = `${response.data.day}/${response.data.month}/${response.data.year}  ${response.data.hour}:${response.data.min}`;
	console.log("Recibido WSupdateTime");

}

/**
 * Clears the connection status interval.
 */
function stopWifiConnectStatusInterval() {
	if (wifiConnectInterval != null) {
		clearInterval(wifiConnectInterval);
		wifiConnectInterval = null;
	}
}


/**
 * Starts the interval for checking the connection status.
 */
function startWifiConnectStatusInterval() {
	wifiConnectInterval = setInterval(WSgetWifiConnectStatus, 5000);
}


/**
 * Checks credentials on connect_wifi button click.
 */
function checkCredentials() {
	errorList = "";
	credsOk = true;

	/* selectedSSID = $("#connect_ssid").val();
	pwd = $("#connect_pass").val(); */
	selectedSSID = document.getElementById("connect_ssid").value;
	pwd = document.getElementById("connect_pass").value;

	if (selectedSSID == "") {
		errorList += "<h4 class='rd'>SSID cannot be empty!</h4>";
		credsOk = false;
	}
	if (pwd == "") {
		errorList += "<h4 class='rd'>Password cannot be empty!</h4>";
		credsOk = false;
	}

	if (credsOk == false) {
		document.getElementById("wifi_connect_credentials_errors").innerHTML = errorList;
	}
	else {
		document.getElementById("wifi_connect_credentials_errors").innerHTML = "";
		WSconnectWIFI();
	}
}

/**
 * Shows the WiFi password if the box is checked.
 */
function showPassword() {
	var x = document.getElementById("connect_pass");
	if (x.type === "password") {
		x.type = "text";
	}
	else {
		x.type = "password";
	}
}












