from flask import Flask
from flask import request

from rflib import *
from bitstring import *

import subprocess

app = Flask(__name__)

@app.route("/ON", methods=['GET'])
def turnOn():
    print "Turning on"
    subprocess.call(['python', '/home/pi/fireplace/on.py'])
    return ""

@app.route("/OFF", methods=['GET'])
def turnOff  ():
    print "Turning off"
    subprocess.call(['python', '/home/pi/fireplace/off.py'])
    return ""

if __name__ == "__main__":
    app.run(host='0.0.0.0', debug=True)