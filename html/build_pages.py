
def getFileContent(fileName : str):
    file = open(fileName, "r")
    lines = file.readlines()
    file.close()
    return lines

def writeFile(fileName : str, lines):
    file = open(fileName, "w")
    file.writelines(lines)
    file.close()

def createRecord(fileName : str, variableName : str, functionName : str):
    d = dict()
    d["functionName"] = functionName
    d["variable"] = []
    d["variable"].append("const char* {} PROGMEM = R\"rawliteral(\n".format(variableName))
    content = getFileContent(fileName)
    d["variable"].extend(content)
    d["variable"].append(")rawliteral\";\n")
    d["function"] = []
    d["function"].append("const char* {}()\n".format(functionName))
    d["function"].append("{\n")
    d["function"].append("    return {};\n".format(variableName))
    d["function"].append("}\n")
    return d

def recordsToSourceString(records, headerFileName : str):
    lines = []
    lines.append("#include \"{}\"\n".format(headerFileName))
    lines.append("#include <Arduino.h>\n")
    lines.append("\n")
    for i in range(len(records)):
        lines.extend(records[i]["variable"])
        lines.append("\n")

    for i in range(len(records)):
        lines.extend(records[i]["function"])
        lines.append("\n")
    return lines

def recordsToHeaderString(records):
    lines = []
    lines.append("#pragma once\n")
    lines.append("\n")
    for i in range(len(records)):
        lines.append("const char* {}();\n\n".format(records[i]["functionName"]))

    return lines

records = []
records.append(createRecord("default.css", "httpDefaultCss", "getHttpDefaultCss"))
records.append(createRecord("index.html", "httpIndex", "getHttpIndex"))
records.append(createRecord("config_module.html", "httpModuleConfig", "getHttpModuleConfig"))
records.append(createRecord("config_gpio.html", "httpGpioConfig", "getHttpGpioConfig"))
records.append(createRecord("config_timing.html", "httpTimingConfig", "getHttpTimingConfig"))
records.append(createRecord("config_network.html", "httpNetworkConfig", "getHttpNetworkConfig"))
records.append(createRecord("config_mqtt.html", "httpMqttConfig", "getHttpMqttConfig"))
records.append(createRecord("settings.html", "httpSettings", "getHttpSettings"))
records.append(createRecord("network_saved.html", "httpNetworkConfigSaved", "getHttpNetworkConfigSaved"))
records.append(createRecord("config_saved.html", "httpConfigSaved", "getHttpConfigSaved"))
linesSource = recordsToSourceString(records, "http_pages.h")
linesHeader = recordsToHeaderString(records)
writeFile("http_pages.cpp", linesSource)
writeFile("http_pages.h", linesHeader)