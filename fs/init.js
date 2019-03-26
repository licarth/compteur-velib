load("api_config.js");
load("api_rpc.js");
load("api_timer.js");
load("api_http.js");
load("api_gpio.js");

let D0 = 16;
let D1 = 5;
let D2 = 4;
let D3 = 0;
let D4 = 2;
let D5 = 14;
let D6 = 12;
let D7 = 13;
let D8 = 15;
let D9 = 3;
let D10 = 1;

let SECONDS = 1000;
let METEO_ERROR_BACKOFF = 5 * SECONDS;
let METEO_REFRESH_RATE = 300 * SECONDS;
let VELIB_ERROR_BACKOFF = 5 * SECONDS;
let VELIB_REFRESH_RATE = 30 * SECONDS;

let displayVelib = ffi("void *displayVelib(int, int)");
let displayRain = ffi("void *displayRain(char *)");
let setIntensity = ffi("void *setIntensity(int, int)");
let shutdown = ffi("void *shutdown()");
let wakeup = ffi("void *wakeup()");

let velibTimer;
let meteoTimer;

let meteoSuccess = 0;
let velibSuccess = 0;

let velibRequest = function(callback, error) {
  print("velib request...");
  HTTP.query({
    headers: { "User-Agent": "curl/7.61.0", Accept: "*/*" },
    url:
      "https://www.velib-metropole.fr/webapi/map/details?gpsTopLatitude=48.86233702357265&gpsTopLongitude=2.377841262009241&gpsBotLatitude=48.86192224173112&gpsBotLongitude=2.3764060396395053&zoomLevel=19.22638194438305",
    success: callback,
    error: error,
  });
};

let meteoRequest = function(callback, error) {
  print("meteo request...");
  HTTP.query({
    headers: { "User-Agent": "curl/7.61.0", Accept: "*/*" },
    url: "http://www.meteofrance.com/mf3-rpc-portlet/rest/pluie/751110",
    success: callback,
    error: error,
  });
};

let doMeteo = function() {
  setIntensity(1, 0);
  meteoRequest(
    function(res) {
      print("meteo request done");
      let json = JSON.parse(res);
      let weatherString = "";
      for (let i = 0; i < 8; i++) {
        weatherString =
          weatherString + JSON.stringify(json.dataCadran[i].niveauPluie);
      }
      setIntensity(1, 14);
      displayRain(weatherString);
    },
    function(err) {
      print("meteo request error");
      Timer.set(METEO_ERROR_BACKOFF, false, doMeteo, null);
    },
  );
};

let doVelib = function() {
  setIntensity(0, 0);
  velibRequest(
    function(res) {
      print("velib request done");
      let json = JSON.parse(res);
      print(json);
      displayVelib(json[0].nbBike, json[0].nbEbike);
      setIntensity(0, 14);
    },
    function(err) {
      print("velib request error");
      Timer.set(VELIB_ERROR_BACKOFF, false, doVelib, null);
    },
  );
};

let initTimers = function() {
  Timer.set(10, false, doMeteo, null);
  meteoTimer = Timer.set(METEO_REFRESH_RATE, true, doMeteo, null);
  Timer.set(10, false, doVelib, null);
  doVelib = Timer.set(VELIB_REFRESH_RATE, true, doBus, null);
};

// Monitor network connectivity.
Event.addGroupHandler(
  Net.EVENT_GRP,
  function(ev, evdata, arg) {
    let evs = "???";
    if (ev === Net.STATUS_DISCONNECTED && Cfg.get("wifi.ap.enable") === false) {
      evs = "DISCONNECTED";
    } else if (ev === Net.STATUS_CONNECTING) {
      evs = "CONNECTING";
    } else if (ev === Net.STATUS_CONNECTED) {
      evs = "CONNECTED";
      print("##################################");
      initTimers();
    } else if (ev === Net.STATUS_GOT_IP && Cfg.get("wifi.ap.enable") === true) {
      evs = "GOT_IP";
      print(Cfg.get("wifi.ap.enable"));
    }
    print("== Net event:", ev, evs);
  },
  null,
);
