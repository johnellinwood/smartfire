# -*- coding: utf-8 -*-

#import boto3
import json
import os
from math import ceil
import urllib2
from utils.alexa_response import AlexaResponse

# aws_dynamodb = boto3.client('dynamodb')


# aws_dynamodb = boto3.client('dynamodb')


def lambda_handler(request, context):
    # Dump the request for logging - check the CloudWatch logs
    print('lambda_handler request  -----')
    print(json.dumps(request))

    if context is not None:
        print('lambda_handler context  -----')
        print(context)

    # Validate we have an Alexa directive
    if 'directive' not in request:
        aer = AlexaResponse(
            name='ErrorResponse',
            payload={'type': 'INVALID_DIRECTIVE',
                     'message': 'Missing key: directive, Is the request a valid Alexa Directive?'})
        return send_response(aer.get())

    # Check the payload version
    payload_version = request['directive']['header']['payloadVersion']
    if payload_version != '3':
        aer = AlexaResponse(
            name='ErrorResponse',
            payload={'type': 'INTERNAL_ERROR',
                     'message': 'This skill only supports Smart Home API version 3'})
        return send_response(aer.get())

    # Crack open the request and see what is being requested
    name = request['directive']['header']['name']
    namespace = request['directive']['header']['namespace']

    # Handle the incoming request from Alexa based on the namespace

    if namespace == 'Alexa.Authorization':
        if name == 'AcceptGrant':
            # Note: This sample accepts any grant request
            # In your implementation you would use the code and token to get and store access tokens
            grant_code = request['directive']['payload']['grant']['code']
            grantee_token = request['directive']['payload']['grantee']['token']
            aar = AlexaResponse(namespace='Alexa.Authorization', name='AcceptGrant.Response')
            return send_response(aar.get())

    if namespace == 'Alexa.Discovery':
        # should split up the devices separately, give them capabilities. then create an aggregate
        # capability should include retrievable ; later use proactivelyReported
        # interfaces should include BrightnessController, PercentageController, PowerController, PowerLevelController
        # may use RangeController to limit 0 to 6 to simulate the remote control better
        # PowerLevelController is 0 to 100
        # PercentageController is 0 to 100
        # maybe need a SceneController as the aggregator
        # CPI is either ToggleController ir PowerController
        # Light is probably BrightnessController
        # Thermostat is either ToggleController or PowerController
        # Power is PowerController
        # Front Flame is either ToggleController or PowerController
        # Fan is probably PercentageController
        # Flame is probably PercentageController
        # displayCategories=[], # FAN, LIGHT, OTHER, SMARTPLUG; later use TEMPERATURE_SENSOR and THERMOSTAT
        # not sure about ACTIVITY_TRIGGER or SCENE_TRIGGER

        if name == 'Discover':
            adr = AlexaResponse(namespace='Alexa.Discovery', name='Discover.Response')

            # Reusable Capabilities
            capability_alexa = adr.create_payload_endpoint_capability()
            capability_powercontroller = adr.create_payload_endpoint_capability(
                interface='Alexa.PowerController',
                supported=[{'name': 'powerState'}],
                retrievable=True
            )
            capability_brightnesscontroller = adr.create_payload_endpoint_capability(
                interface='Alexa.BrightnessController',
                supported=[{'name': 'brightness'}],
                retrievable=True
            )
            capability_powerlevelcontroller = adr.create_payload_endpoint_capability(
                interface='Alexa.PowerLevelController',
                supported=[{'name': 'powerLevel'}],
                retrievable=True
            )

            # Pilot Endpoint
            adr.add_payload_endpoint(
                endpoint_id='fireplace_pilot',
                friendly_name='Fireplace Pilot',
                description='Fireplace Pilot',
                manufacturerName='Jotul',
                display_categories=['SWITCH'],
                capabilities=[capability_alexa, capability_powercontroller])

            # Light Endpoint
            adr.add_payload_endpoint(
                endpoint_id='fireplace_light',
                friendly_name='Fireplace Light',
                description='Fireplace Light',
                manufacturer_name='Jotul',
                display_categories=['LIGHT'],
                capabilities=[capability_alexa, capability_powercontroller, capability_brightnesscontroller])

            # Thermostat Endpoint
            adr.add_payload_endpoint(
                endpoint_id='fireplace_thermostat',
                friendly_name='Fireplace Thermostat',
                description='Fireplace Thermostat',
                manufacturer_name='Jotul',
                display_categories=['SWITCH'],
                capabilities=[capability_alexa, capability_powercontroller])

            # Power Endpoint
            adr.add_payload_endpoint(
                endpoint_id='fireplace_power',
                friendly_name='Fireplace Power',
                description='Fireplace Power',
                manufacturer_name='Jotul',
                display_categories=['SWITCH'],
                capabilities=[capability_alexa, capability_powercontroller])

            # Front Endpoint (Flame Split)
            adr.add_payload_endpoint(
                endpoint_id='fireplace_front',
                friendly_name='Fireplace Front',
                description='Fireplace Front',
                manufacturer_name='Jotul',
                display_categories=['SWITCH'],
                capabilities=[capability_alexa, capability_powercontroller])

            # Fan Endpoint
            adr.add_payload_endpoint(
                endpoint_id='fireplace_fan',
                friendly_name='Fireplace Fan',
                description='Fireplace Fan',
                manufacturer_name='Jotul',
                display_categories=['FAN'],
                capabilities=[capability_alexa, capability_powercontroller, capability_powerlevelcontroller])

            # Aux Endpoint
            adr.add_payload_endpoint(
                endpoint_id='fireplace_aux',
                friendly_name='Fireplace Auxiliary',
                description='Fireplace Auxiliary',
                manufacturer_name='Jotul',
                display_categories=['SMARTPLUG'],
                capabilities=[capability_alexa, capability_powercontroller])

            # Flame Endpoint
            adr.add_payload_endpoint(
                endpoint_id='fireplace_flame',
                friendly_name='Fireplace Flame',
                description='Fireplace Flame',
                manufacturer_name='Jotul',
                display_categories=['FAN'],
                capabilities=[capability_alexa, capability_powercontroller, capability_powerlevelcontroller])

            # Aggregate Fireplace Endpoint
            adr.add_payload_endpoint(
                endpoint_id='fireplace',
                friendly_name='Fireplace',
                description='Fireplace',
                manufacturer_name='Jotul',
                display_categories=['FAN', 'SMARTPLUG', 'LIGHT', 'SWITCH'],
                capabilities=[capability_alexa, capability_powercontroller, capability_powerlevelcontroller])

            return send_response(adr.get())

    if namespace == 'Alexa.PowerController':
        # Note: This sample always returns a success response for either a request to TurnOff or TurnOn
        endpoint_id = request['directive']['endpoint']['endpointId']
        token = request['directive']['endpoint']['scope']['token']

        if endpoint_id == 'fireplace':
            power_state_value = 'OFF' if name == 'TurnOff' else 'ON'
            correlation_token = request['directive']['header']['correlationToken']
            # Check for an error when setting the state
            # state_set = set_device_state(endpoint_id=endpoint_id, state='powerState', value=power_state_value)
            # if not state_set:
            #    return AlexaResponse(
            #        name='ErrorResponse',
            #        payload={'type': 'ENDPOINT_UNREACHABLE', 'message': 'Unable to reach endpoint database.'}).get()
            control_fireplace(endpoint_id=endpoint_id, state='powerState', value=0 if power_state_value == 'OFF' else 1)
            apcr = AlexaResponse(correlation_token=correlation_token, token=token, endpoint_id=endpoint_id)
            apcr.add_context_property(namespace='Alexa.PowerController', name='powerState', value=power_state_value)
            return send_response(apcr.get())

    if namespace == 'Alexa.PowerLevelController':
        endpoint_id = request['directive']['endpoint']['endpointId']
        token = request['directive']['endpoint']['scope']['token']

        if endpoint_id == 'fireplace':
            if name == 'SetPowerLevel':
                # need to handle SetPowerLevel as absolute (0 to 100)
                power_level = request['directive']['payload']['powerLevel']
                correlation_token = request['directive']['header']['correlationToken']
                # add a deferred response here
                control_fireplace(endpoint_id=endpoint_id, state='powerState', value=power_level)
                apcr = AlexaResponse(correlation_token=correlation_token, token=token, endpoint_id=endpoint_id)
                apcr.add_context_property(namespace='Alexa.PowerLevelController', name='powerLevel', value=power_level)
                return send_response(apcr.get())
            elif name == 'AdjustPowerLevel':
                # need to handle AdjustPowerLevel as relative (-100 to 100)
                power_level = request['directive']['payload']['powerLevel']
                correlation_token = request['directive']['header']['correlationToken']
                # add a deferred response here
                control_fireplace(endpoint_id=endpoint_id, state='powerState', value=power_level)
                apcr = AlexaResponse(correlation_token=correlation_token, token=token, endpoint_id=endpoint_id)
                apcr.add_context_property(namespace='Alexa.PowerLevelController', name='powerLevel', value=power_level)
                return send_response(apcr.get())


def send_response(response):
    # TODO Validate the response
    print('lambda_handler response -----')
    print(json.dumps(response))
    return response


def control_fireplace(endpoint_id, state, value):
    print('control_fireplace({}, {}, {})'.format(endpoint_id, state, value))
    base_url = os.environ['BASE_URL']
    url = base_url + '/state'
    power = 0 if value == 0 else 1
    # level = 0 if value == 0 else int(ceil((value+1)/25)) # 0 if 0, 5 if 100, and between 1 an 4 for everything else
    level = 0;
    if value >= 100:
        level = 6
    elif value >= 75:
        level = 5
    elif value >= 50:
        level = 4
    elif value >= 25:
        level = 2
    elif value > 0:
        level = 1

    body = json.dumps({'pilot': True, 'power': power, 'flame': level})
    print('Sending URL request for fireplace to: {}'.format(body))
    opener = urllib2.build_opener(urllib2.HTTPHandler)
    request = urllib2.Request(url, data=body)
    request.add_header('Content-Type', 'application/json')
    request.get_method = lambda: 'PUT'
    response = opener.open(request)

# def set_device_state(endpoint_id, state, value):
#    attribute_key = state + 'Value'
#    response = aws_dynamodb.update_item(
#        TableName='SampleSmartHome',
#        Key={'ItemId': {'S': endpoint_id}},
#        AttributeUpdates={attribute_key: {'Action': 'PUT', 'Value': {'S': value}}})
#    print(response)
#    if response['ResponseMetadata']['HTTPStatusCode'] == 200:
#        return True
#    else:
#        return False
