static const char CONTENT_WIFI_PORTAL_HTM[] PROGMEM = 
  "<!doctype html> <html lang=en > <meta charset=utf-8 > <title>OpenEVSE</title> <meta name=viewport  content=\"width=device-width, initial-scale=1.0\"> <meta name=viewport  content=\"width=device-width, user-scalable=no\" /> <meta name=description  content=OpenEVSE > <meta name=author  content=OpenEVSE > <meta name=theme-color  content=\"#000000\" /> <link rel=stylesheet  type=\"text/css\" href=style.css > <link rel=icon  type=\"image/png\" href=favicon-32x32.png  sizes=32x32  /> <link rel=icon  type=\"image/png\" href=favicon-16x16.png  sizes=16x16  /> <div id=page > <div class=header > <h1>Open<span>EVSE</span></h1> <h3>WiFi</h3> </div> <div class=container > <div class=box  data-bind=\"visible: wifi.canConfigure() &amp;&amp; !wifi.wifiConnecting()\"> <h2>WiFi Setup</h2> <p><b>Mode:</b> <span id=mode  data-bind=\"text: status.fullMode\"></span></p> <p>Connect to network:</p> <table> <thead> <tr> <th>Select <th>Network <th>RSSI dBm <tbody id=networks  data-bind=\"foreach: scan.results\"> <tr> <td><input class='networkcheckbox' name='network' data-bind=\"checkedValue: bssid(), checked: $root.wifi.bssid\" type='radio'> <td data-bind=\"text: ssid\"> <td data-bind=\"text: rssi\"> <tfoot data-bind=\"visible: 0 == scan.results().length\"> <tr> <td colspan=3 >Scanning... </table> <p data-bind=\"text: config.ssid\"></p> <p> <b>Passkey:</b><br> <input type=text  style=\"width:180px\" autocapitalize=none  autocapitalize=none  data-bind=\"textInput: config.pass\"> <button data-bind=\"click: wifi.saveNetwork, text: (wifi.saveNetworkFetching() ? 'Saving' : (wifi.saveNetworkSuccess() ? 'Saved' : 'Connect')), disable: wifi.saveNetworkFetching\">Connect</button> </p> <p>Connecting to a local WiFi network is not essential. OpenEVSE can be configured and controlled while in standalone WiFi AP (Access Point) mode:</p> <div> <button onclick=\"window.location.href='/home.htm'\">WiFi Standalone Mode</button> </div> <p><i><span class=small-text >Note: remote logging features e.g Emoncms will not work while in AP standalone mode</span></i></p> </div> <div class=box  data-bind=\"visible: wifi.wifiConnecting() || !wifi.canConfigure()\"> <h2>Connecting to <span data-bind=\"text: config.ssid\"></span> ...</h2> <p>Please connect this device to <span data-bind=\"text: config.ssid\"></span> and navigate to the IP address displayed on your OpenEVSE display.</p> <p>Alternatively you can use <a href=\"http://openevse.local/\">http://openevse.local/</a> or <a href=\"http://openevse/\">http://openevse/</a></p> <div id=client-view  data-bind=\"visible: status.isWifiClient\"> <button id=apoff  data-bind=\"click: wifi.turnOffAccessPoint, disable: wifi.turnOffAccessPointFetching\">Continue</button> </div> </div> <!-- <div class=container > <div class=box > <h2>Continue without WiFi</h2> <p>Connecting to a local WiFi network is not essentail.</p> <p>OpenEVSE can be configured and controlled while in AP mode:</p> <div> <button onclick=\"window.location.href='/home.htm'\">Continue</button> </div> <i>Note: remote logging features e.g Emoncms will not work while in AP mode</i> </div> </div> --> <div id=footer > <br><b> Powered by <a href=\"http://www.openevse.com\">OpenEVSE</a> and <a href=\"https://openenergymonitor.org\">OpenEnergyMonitor</a></b> <br> <b>Version: </b>V<span data-bind=\"text: config.version\"></span> </div> </div> <!--end container --> </div> <!--end page --> <script src=lib.js ></script> <script src=wifi_portal.js ></script>\n";
