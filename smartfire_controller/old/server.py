#!/usr/bin/python
"""
server.py: REST sever providing access to the Proflame 2 controller
"""
import json
import logging

from flask import Flask, request, jsonify

from fireplace import Fireplace

fp = Fireplace()
app = Flask(__name__)


@app.route("/state", methods=['GET', 'PUT'])
def state():
    """Get or set the whole fireplace state at one time"""
    if request.method == 'GET':
        app.logger.debug("get state")
        return jsonify(fp.state)
    elif request.method == 'PUT':
        value = json.loads(request.data)
        app.logger.debug("put state: {}".format(value))
        fp.set(pilot=value['pilot'], power=value['power'], flame=value['flame'])
        return str(fp.state)


@app.route("/serial", methods=['GET'])
def serial():
    """Return the serial number"""
    app.logger.debug("get serial")
    return str(fp.serial)


@app.route("/pilot", methods=['GET', 'PUT'])
def pilot():
    """Get or set the pilot light and return the status"""
    if request.method == 'GET':
        app.logger.debug("get pilot")
        return str(fp.pilot)
    elif request.method == 'PUT':
        value = request.data == 'True'
        app.logger.debug("put pilot: {}".format(value))
        fp.pilot = value
        return str(fp.pilot)


@app.route("/light", methods=['GET', 'PUT'])
def light():
    """Get or set the light and return the status"""
    if request.method == 'GET':
        app.logger.debug("get light")
        return str(fp.light)
    elif request.method == 'PUT':
        value = int(request.data)
        app.logger.debug("put light: {}".format(value))
        fp.light = value
        return str(fp.light)


@app.route("/thermostat", methods=['GET', 'PUT'])
def thermostat():
    """Get or set the thermostat and return the status"""
    if request.method == 'GET':
        app.logger.debug("get thermostat")
        return str(fp.thermostat)
    elif request.method == 'PUT':
        value = request.data == 'True'
        app.logger.debug("put thermostat: {}".format(value))
        fp.thermostat = value
        return str(fp.thermostat)


@app.route("/power", methods=['GET', 'PUT'])
def power():
    """Get or set the power and return the status"""
    if request.method == 'GET':
        app.logger.debug("get power")
        return str(fp.power)
    elif request.method == 'PUT':
        value = request.data == 'True'
        app.logger.debug("put power: {}".format(value))
        fp.power = value
        return str(fp.power)


@app.route("/front", methods=['GET', 'PUT'])
def front():
    """Get or set the front flame and return the status"""
    if request.method == 'GET':
        app.logger.debug("get front")
        return str(fp.front)
    elif request.method == 'PUT':
        value = request.data == 'True'
        app.logger.debug("put front: {}".format(value))
        fp.front = value
        return str(fp.front)


@app.route("/fan", methods=['GET', 'PUT'])
def fan():
    """Get or set the fan and return the status"""
    if request.method == 'GET':
        app.logger.debug("get fan")
        return str(fp.fan)
    elif request.method == 'PUT':
        value = int(request.data)
        app.logger.debug("put fan: {}".format(value))
        fp.fan = value
        return str(fp.fan)


@app.route("/aux", methods=['GET', 'PUT'])
def aux():
    """Get or set the auxiliary power and return the status"""
    if request.method == 'GET':
        app.logger.debug("get aux")
        return str(fp.aux)
    elif request.method == 'PUT':
        value = request.data == 'True'
        app.logger.debug("put aux: {}".format(value))
        fp.aux = value
        return str(fp.aux)


@app.route("/flame", methods=['GET', 'PUT'])
def flame():
    """Get or set the flame level and return the status"""
    if request.method == 'GET':
        app.logger.debug("get flame")
        return str(fp.flame)
    elif request.method == 'PUT':
        value = int(request.data)
        app.logger.debug("put flame: {}".format(value))
        fp.flame = value
        return str(fp.flame)


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5000, debug=True)  # start the rest server
