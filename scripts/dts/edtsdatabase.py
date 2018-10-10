#
# Copyright (c) 2018 Bobby Noelte
# Copyright (c) 2018 Linaro Limited
#
# SPDX-License-Identifier: Apache-2.0
#

from pathlib import Path
from collections.abc import Mapping
import argparse
import json
import pprint

##
# @brief ETDS Database consumer
#
# Methods for ETDS database usage.
#
class EDTSConsumerMixin(object):
    __slots__ = []

    ##
    # @brief Get compatibles
    #
    # @param None
    # @return edts 'compatibles' dict
    def get_compatibles(self):
        return self._edts['compatibles']

    ##
    # @brief Get aliases
    #
    # @param None
    # @return edts 'aliases' dict
    def get_aliases(self):
        return self._edts['aliases']

    ##
    # @brief Get chosen
    #
    # @param None
    # @return edts 'chosen' dict
    def get_chosen(self):
        return self._edts['chosen']

    ##
    # @brief Get device types
    #
    # @param None
    # @return edts device types dict
    def get_device_types(self):
        return self._edts['device-types']

    ##
    # @brief Get controllers
    #
    # @param  None
    # @return compatible generic device type
    def get_controllers(self):
        return self._edts['controllers']

    ##
    # @brief Get device type for a compatible
    #
    # @param compatible
    # @return compatible generic device type
    def get_compatible_device_type(self, compatible):
        for dev_type in list(self._edts['device-types']):
            if compatible in dev_type:
                return list(self._edts['device-types'])
            else:
                return None

    #
    # @param compatible
    # @return dict of compatible devices
    def _generate_device_dict(self, compatibles):
        devices = dict()
        if not isinstance(compatibles, list):
            compatibles = [compatibles, ]
        for compatible in compatibles:
            for device_id in self._edts['compatibles'].get(compatible, []):
                devices[device_id] = self._edts['devices'][device_id]
        return devices

    ##
    # @brief Get all activated compatible devices.
    #
    # @param compatibles compatible(s)
    # @return list of devices that are compatible
    def get_devices_by_compatible(self, compatibles):
        return list(self._generate_device_dict(compatibles).values())

    ##
    # @brief Get device ids of all activated compatible devices.
    #
    # @param compatibles compatible(s)
    # @return list of device ids of activated devices that are compatible
    def get_device_ids_by_compatible(self, compatibles):
        return list(self._generate_device_dict(compatibles).keys())

    ##
    # @brief Get the device dict matching a device_id.
    #
    # @param device_id
    # @return (dict)device
    def get_device_by_device_id(self, device_id):
        return self._edts['devices'][device_id]

    ##
    # @brief Get device id of activated device with given label.
    #
    # @return device id
    def get_device_id_by_label(self, label):
        for device_id, device in self._edts['devices'].items():
            if label == device.get('label', None):
                return device_id
        return None

    ##
    # @brief Get device tree property value for the device of the given device id.
    #
    # @param device_id
    # @param property_path Path of the property to access
    #                      (e.g. 'reg/0', 'interrupts/prio', 'device_id', ...)
    # @return property value
    #
    def get_device_property(self, device_id, property_path, default="<unset>"):
        property_value = self._edts['devices'].get(device_id, None)
        property_path_elems = property_path.strip("'").split('/')
        for elem_index, key in enumerate(property_path_elems):
            if isinstance(property_value, dict):
                property_value = property_value.get(key, None)
            elif isinstance(property_value, list):
                if int(key) < len(property_value):
                    property_value = property_value[int(key)]
                else:
                    property_value = None
            else:
                property_value = None
        if property_value is None:
            if default == "<unset>":
                default = "Device tree property {} not available in {}" \
                                .format(property_path, device_id)
            return default
        return property_value

    def _device_properties_flattened(self, properties, path, flattened, path_prefix):
        if isinstance(properties, dict):
            for prop_name in properties:
                super_path = "{}/{}".format(path, prop_name).strip('/')
                self._device_properties_flattened(properties[prop_name],
                                                  super_path, flattened,
                                                  path_prefix)
        elif isinstance(properties, list):
            for i, prop in enumerate(properties):
                super_path = "{}/{}".format(path, i).strip('/')
                self._device_properties_flattened(prop, super_path, flattened,
                                                  path_prefix)
        else:
            flattened[path_prefix + path] = properties

    ##
    # @brief Get the device tree properties for the device of the given device id.
    #
    # @param device_id
    # @param path_prefix
    # @return dictionary of proerty_path and property_value
    def device_properties_flattened(self, device_id, path_prefix = ""):
        flattened = dict()
        self._device_properties_flattened(self._edts['devices'][device_id],
                                          '', flattened, path_prefix)
        return flattened

    def load(self, file_path):
        with open(file_path, "r") as load_file:
            self._edts = json.load(load_file)

##
# @brief ETDS Database provider
#
# Methods for ETDS database creation.
#
class EDTSProviderMixin(object):
    __slots__ = []

    def _update_device_compatible(self, device_id, compatible):
        if compatible not in self._edts['compatibles']:
            self._edts['compatibles'][compatible] = list()
        if device_id not in self._edts['compatibles'][compatible]:
            self._edts['compatibles'][compatible].append(device_id)
            self._edts['compatibles'][compatible].sort()

    def _update_device_alias(self, device_id, alias):
        if alias not in self._edts['aliases']:
            self._edts['aliases'][alias] = list()
        if device_id not in self._edts['aliases'][alias]:
            self._edts['aliases'][alias].append(device_id)
            self._edts['aliases'][alias].sort()


    def _inject_cell(self, keys, property_access_point, property_value):

        for i in range(0, len(keys)):
            if i < len(keys) - 1:
                # there are remaining keys
                if keys[i] not in property_access_point:
                    property_access_point[keys[i]] = dict()
                property_access_point = property_access_point[keys[i]]
            else:
                # we have to set the property value
                if keys[i] in property_access_point and \
                   property_access_point[keys[i]] != property_value:
                   # Property is already set and we're overwiting with a new
                   # different value. Tigger an error
                    raise Exception("Overwriting edts cell {} with different value\n \
                                     Initial value: {} \n \
                                     New value: {}".format(property_access_point,
                                     property_access_point[keys[i]],
                                     property_value
                                     ))
                property_access_point[keys[i]] = property_value

    def insert_chosen(self, chosen, device_id):
        self._edts['chosen'][chosen] = device_id

    def _inject_cell(self, keys, property_access_point, property_value):

        for i in range(0, len(keys)):
            if i < len(keys) - 1:
                # there are remaining keys
                if keys[i] not in property_access_point:
                    property_access_point[keys[i]] = dict()
                property_access_point = property_access_point[keys[i]]
            else:
                # we have to set the property value
                if keys[i] in property_access_point and \
                   property_access_point[keys[i]] != property_value:
                   # Property is already set and we're overwiting with a new
                   # different value. Tigger an error
                    raise Exception("Overwriting edts cell {} with different value\n \
                                     Initial value: {} \n \
                                     New value: {}".format(property_access_point,
                                     property_access_point[keys[i]],
                                     property_value
                                     ))
                property_access_point[keys[i]] = property_value


    def insert_device_type(self, compatible, device_type):
        if device_type not in self._edts['device-types']:
            self._edts['device-types'][device_type] = list()
        if compatible not in self._edts['device-types'][device_type]:
            self._edts['device-types'][device_type].append(compatible)
            self._edts['device-types'][device_type].sort()

    def insert_device_controller(self, controller_type, controller, device_id, line):
        if controller_type not in self._edts['controllers']:
            self._edts['controllers'][controller_type] = dict()
        if controller not in self._edts['controllers'][controller_type]:
            self._edts['controllers'][controller_type][controller] = dict()
        if line not in self._edts['controllers'][controller_type][controller]:
            self._edts['controllers'][controller_type][controller]\
                                                                [line] = list()
        if device_id not in self._edts['controllers'][controller_type]\
                                                            [controller][line]:
            self._edts['controllers'][controller_type][controller]\
                                                        [line].append(device_id)
            self._edts['controllers'][controller_type][controller]\
                                                                [line].sort()


    ##
    # @brief Insert property value for the child of a device id.
    #
    # @param device_id
    # @param child_name
    # @param property_path Path of the property to access
    #                      (e.g. 'reg/0', 'interrupts/prio', 'label', ...)
    # @param property_value value
    #
    def insert_child_property(self, device_id, child_name, property_path, property_value):

        # normal property management
        if device_id not in self._edts['devices']:
            self._edts['devices'][device_id] = dict()
            self._edts['devices'][device_id]['device-id'] = device_id

        # Inject prop
        keys = ['children', child_name]
        prop_keys = property_path.strip("'").split('/')
        keys.extend(prop_keys)
        property_access_point = self._edts['devices'][device_id]
        self._inject_cell(keys, property_access_point, property_value)

        # Compute and inject unique device-id
        keys = ['device-id']
        child_id = device_id + '/' + child_name
        property_access_point = self._edts['devices'][device_id]['children']\
                                                                [child_name]
        self._inject_cell(keys, property_access_point, child_id)

        # Update alias struct if needed
        if property_path.startswith('alias'):
            self._update_device_alias(child_id, property_value)

    ##
    # @brief Insert property value for the device of the given device id.
    #
    # @param device_id
    # @param property_path Path of the property to access
    #                      (e.g. 'reg/0', 'interrupts/prio', 'label', ...)
    # @param property_value value
    #
    def insert_device_property(self, device_id, property_path, property_value):

        # special properties
        if property_path.startswith('compatible'):
            self._update_device_compatible(device_id, property_value)
        if property_path.startswith('alias'):
            aliased_device = device_id
            if 'children' in property_path:
                # device_id is the parent_id
                # property_path is children/device_id/alias
                aliased_device = property_path.split('/')[1]
            self._update_device_alias(aliased_device, property_value)

        # normal property management
        if device_id not in self._edts['devices']:
            self._edts['devices'][device_id] = dict()
            self._edts['devices'][device_id]['device-id'] = device_id
        if property_path == 'device-id':
            # already set
            return
        keys = property_path.strip("'").split('/')
        property_access_point = self._edts['devices'][device_id]

        self._inject_cell(keys, property_access_point, property_value)

    ##
    # @brief Write json file
    #
    # @param file_path Path of the file to write
    #
    def save(self, file_path):
        with open(file_path, "w") as save_file:
            json.dump(self._edts, save_file, indent = 4, sort_keys=True)

##
# @brief Extended DTS database
#
# Database schema:
#
# _edts dict(
#   'aliases': dict(alias) : sorted list(device-id)),
#   'chosen': dict(chosen),
#   'devices':  dict(device-id :  device-struct),
#   'compatibles':  dict(compatible : sorted list(device-id)),
#   'device-types': dict(device-type : sorted list(compatible)),
#   ...
# )
#
# device-struct dict(
#   'device-id' : device-id,
#   'compatible' : list(compatible) or compatible,
#   'label' : label,
#   property-name : property-value ...
# )
#
# Database types:
#
# device-id: opaque id for a device (do not use for other purposes),
# compatible: any of ['st,stm32-spi-fifo', ...] - 'compatibe' from <binding>.yaml
# label: any of ['UART_0', 'SPI_11', ...] - label directive from DTS
#
class EDTSDatabase(EDTSConsumerMixin, EDTSProviderMixin, Mapping):

    def __init__(self, *args, **kw):
        self._edts = dict(*args, **kw)
        # setup basic database schema
        for edts_key in ('devices', 'compatibles', 'device-types', \
                          'controllers', 'aliases', 'chosen'):
            if not edts_key in self._edts:
                self._edts[edts_key] = dict()

    def __getitem__(self, key):
        return self._edts[key]

    def __iter__(self):
        return iter(self._edts)

    def __len__(self):
        return len(self._edts)

    def parse_arguments(self):
        rdh = argparse.RawDescriptionHelpFormatter
        parser = argparse.ArgumentParser(description=__doc__, formatter_class=rdh)

        parser.add_argument("-e", "--edts", nargs=1, required=True,
                            help="edts json file")

        return parser.parse_args()

    def main(self):
        args = self.parse_arguments()
        edts_file = Path(args.edts[0])
        if not edts_file.is_file():
            raise self._get_error_exception(
                "Generated extended device tree database file '{}' not found/ no access.".
                format(edts_file), 2)
        self._edts = EDTSDatabase()
        self._edts.load(str(edts_file))

        pprint.pprint(dict(self._edts))

        return 0


if __name__ == '__main__':
    EDTSDatabase().main()
