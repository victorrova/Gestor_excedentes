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
	//WSgetPMU();
	//WSgetTime();

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
		
		if (response.id == "WIFILIST")
		{
			WSupdatetWifiScanList(response);
			
			
		}
		else if (response.id == "WIFISTATUS")
		{
			WSupdateWifiConnectStatus(response);
			
		}
		else if (response.id == "WIFIINFO")
		{
			WSupdatetWifiConnectInfo(response);
			
		}
		else if (response.id == "TIME")
		{
			WSupdatetTime(response);
						
		}

		else 
		{
			
			console.log("ID desconocido");
			
		}
}



function initButton() {
	
	document.getElementById('connect_wifi').addEventListener('click', checkCredentials);
	document.getElementById('disconnect_wifi').addEventListener('click', WSdisconnectWIFI);
	document.getElementById('connect_mqtt').addEventListener('click', WSconnectMQTT);
	document.getElementById('disconnect_mqtt').addEventListener('click', WSdisconnectMQTT);
	document.getElementById('scan_wifi').addEventListener('click', WSgetWifiScanList);
}

function WSconnectMQTT() {
	
	var obj = {
			id: "MQTT",
			data:{
				connect: 1,
				broker: document.getElementById("mqtt_uri").value,
				user: document.getElementById("mqtt_user").value,
				pass: document.getElementById("mqtt_pass").value,
			}
	}
	var json = JSON.stringify(obj);
	websocket.send(json);
}

function WSdisconnectMQTT() {
	
	var obj = {
			id: "MQTT",
			data:{
				connect: 0,
			}
	}
	var json = JSON.stringify(obj);
	websocket.send(json);
}

function WSconnectWIFI() {
	
	var obj = {
			id: "wifi",
			data:{
				action: 1,
				ssid: document.getElementById("connect_ssid").value,
				pass: document.getElementById("connect_pass").value,
			}   
	}
	var json = JSON.stringify(obj);
	websocket.send(json);
	//startWifiConnectStatusInterval();
}

function WSdisconnectWIFI() {
	
	var obj = {
			id: "wifi",
			data:{
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
		data:{
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
		data:{
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
		data:{
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

	if(wifiConnectStatus == 3){

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

	else
	{
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
 * Gets PMU data from server.
 */
function WSgetPMU() {
	//Envía la orden de obtener la fecha y la hora del ESP
	var obj = {
		id: "PMU",
		
	}
	var json = JSON.stringify(obj);
	websocket.send(json);
	console.log("WSgetPMU");
}

/**
 * Set PMU data on webpage.
 */
function WSupdatetPMU(response) {

		
		console.log("Recibido WSupdatetPMU");
		document.getElementById('pmu_temp').style.display = 'block';
		document.getElementById('pmu_temp-label').style.display = 'block';
		document.getElementById("pmu_temp").innerText = `${response.data.TEMP} ºC`;

		document.getElementById('pmu-vbus-v').style.display = 'block';
		document.getElementById('pmu-vbus-v-label').style.display = 'block';
		document.getElementById("pmu-vbus-v").innerText = `${response.data.VBUS_VOLTAGE} V`;

		document.getElementById('pmu-vbus-c').style.display = 'block';
		document.getElementById('pmu-vbus-c-label').style.display = 'block';
		document.getElementById("pmu-vbus-c").innerText = `${response.data.VBUS_CURRENT} A`;
		if(response.data.BATTERY_CONNECTED == true){

			document.getElementById('pmu-bat-v').style.display = 'block';
			document.getElementById('pmu-bat-v-label').style.display = 'block';
			document.getElementById("pmu-bat-v").innerText = `${response.data.BATTERY_VOLTAGE} V`;

			document.getElementById('pmu-bat-c').style.display = 'block';
			document.getElementById('pmu-bat-c-label').style.display = 'block';

			if(response.data.BATTERY_CHARGING == true)
			{	
				document.getElementById("pmu-bat-c-label").innerText = `Corriente Carga Batería: `;
				document.getElementById("pmu-bat-c").innerText = `${response.data.CHARGE_CURRENT} A`;
			}
			else
			{	
				document.getElementById("pmu-bat-c-label").innerText = `Corriente Descarga Batería: `;
				document.getElementById("pmu-bat-c").innerText = `${response.data.DISCHARGE_CURRENT} A`;
			}
		}
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












