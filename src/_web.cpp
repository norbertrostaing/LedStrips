#include "_web.h"

AsyncWebServer server(80);

const char* PARAM_MESSAGE = "message";

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

String getHTMLPage(String msg);

void setupWebServer() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("trigger")) {
            triggerTriggered(request->getParam("trigger")->value());
            request->redirect("/");
            return;
        }

        String msg ="";
        if (request->hasParam("message")) {
            msg = request->getParam("message")->value();
        }
        request->send(200, "text/html", getHTMLPage(msg));
    });

    // Send a POST request to <IP>/post with a form field message set to <message>
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request){
        for (JsonPair kv : config.as<JsonObject>()) {
            String k = kv.key().c_str();
            if (request->hasParam(k, true) && configTypes[k].as<String>() == "b") {
                writeConfig(k,request->getParam(k, true)->value() == "yes");
            }
            if (request->hasParam(k, true) && configTypes[k].as<String>() == "i") {
                int v = request->getParam(k, true)->value().toInt();
                if (!configOptions[k]["rangeMin"].isNull()) {
                    v = max(v, configOptions[k]["rangeMin"].as<int>());
                    v = min(v, configOptions[k]["rangeMax"].as<int>());
                }
                writeConfig(k,v);
            }
            if (request->hasParam(k, true) && configTypes[k].as<String>() == "f") {
                float v = request->getParam(k, true)->value().toFloat();
                if (!configOptions[k]["rangeMin"].isNull()) {
                    v = max(v, configOptions[k]["rangeMin"].as<float>());
                    v = min(v, configOptions[k]["rangeMax"].as<float>());
                }
                writeConfig(k,v);
            }
            if (request->hasParam(k, true) && configTypes[k].as<String>() == "s") {
                writeConfig(k,request->getParam(k, true)->value());
            }
            if (request->hasParam(k, true) && configTypes[k].as<String>() == "e") {
                String val = request->getParam(k, true)->value();
                bool valid = false;
                for (JsonVariant value : configOptions[k]["options"].as<JsonArray>()) {
                    valid = valid || value.as<String>() == val;
                }
                if (valid) writeConfig(k,val);
            }
        }

    request->redirect("/?message=updated");
    });

    server.onNotFound(notFound);

    server.begin();
}

String getHTMLPage(String msg) {
    String html = 
    "<html>"
    "<head><title>"+chipName+" "+chipId+"</title></head>"
    "<title>"+chipName+" "+chipId+"</title>"
    "<style>"
    "input {width:200px;}"
    "label {width:200px; display:inline-block; text-align:right; padding-right:5px;}"
    "#submit {width:150px;}"
    "#msg {position:absolute; border:1px solid green; background-color:#AAFFAA; display:inline-block; padding:5px 20px; left:100px; top:20px}"
    ".trigger {margin:10px; padding:10px;}"
    "</style>"
    "</head>"
    "<body>"
    "<h1>"+chipName+" "+chipId+"</h1>"
    ;
    if (msg != "") {
        html += "<div id='msg'>"+msg+"</div>";
        html += "<script>setTimeout(()=>{document.getElementById('msg').style.display='none';}, 2000) </script>";
    }
    html += "<a href='/'>refresh</a>";
    html += "<h2>Triggers</h2>";
    for (JsonPair kv : trigger.as<JsonObject>()) {
        String k = kv.key().c_str();
        html += "<a href='/?trigger="+k+"'><input class='trigger' type='button' value='"+k+"'></a><br />";
    }
    html +=
    "<h2>Infos</h2>"
    ;
    for (JsonPair kv : info.as<JsonObject>()) {
        String k = kv.key().c_str();
        String v = info[k].as<String>(); 
        html += "<label for='"+k+"'>"+k+"</label>";
        html += "<input name='"+k+"' id='"+k+"' type='text' value='"+v+"' disabled>";
        html += "<br />";
    }
    html += 
    "<h2>Config</h2>"
    "<form method='post' action='/'>"
    ;
    for (JsonPair kv : config.as<JsonObject>()) {
        String k = kv.key().c_str();
        html += "<label for='"+k+"'>"+k+"</label>";
        if (configTypes[k].as<String>() == "b") {
            bool v = config[k].as<bool>(); 
            String checked = v ? "selected" : "";
            html += "<select name='"+k+"' id='"+k+"'>";
            html += "<option value='no' >No</option>";
            html += "<option value='yes' "+checked+">Yes</option>";
            html += "</select>";
        }
        else if (configTypes[k].as<String>() == "i") {
            int v = config[k].as<int>();
            String mini = configOptions[k]["rangeMin"].isNull() ? "" : "min='"+configOptions[k]["rangeMin"].as<String>()+"' ";
            String maxi = configOptions[k]["rangeMax"].isNull() ? "" : "max='"+configOptions[k]["rangeMax"].as<String>()+"' ";
            html += "<input name='"+k+"' id='"+k+"' type='number' step='1' "+mini+maxi+"value='"+String(v)+"'>";
        }
        else if (configTypes[k].as<String>() == "f") {
            float v = config[k].as<float>(); 
            String mini = configOptions[k]["rangeMin"].isNull() ? "" : "min='"+configOptions[k]["rangeMin"].as<String>()+"' ";
            String maxi = configOptions[k]["rangeMax"].isNull() ? "" : "max='"+configOptions[k]["rangeMax"].as<String>()+"' ";
            html += "<input name='"+k+"' id='"+k+"' type='number' step='0.001' "+mini+maxi+" value='"+String(v)+"'>";
        }
        else if (configTypes[k].as<String>() == "s") {
            String v = config[k].as<String>(); 
            html += "<input name='"+k+"' id='"+k+"' type='text' value='"+v+"'>";
        }
        else if (configTypes[k].as<String>() == "e") {
            String v = config[k].as<String>();
            html += "<select  name='"+k+"' id='"+k+"' >";
            for (JsonVariant optionVal : configOptions[k]["options"].as<JsonArray>()) {
                String opt = optionVal.as<String>();
                String sel = v == opt ? " selected" : ""; 
                html += "<option value='"+opt+"' "+sel+">"+opt+"</option>";
            }
            
            html += "</select>"; 
        }
        html += "<br />";
    }
    html += "<label for='submit'></label><input type='submit' id='submit' value='Save'>";
    html += 
    "</form>"
    "</body>"
    "</html>"
    ;
    return html;
}