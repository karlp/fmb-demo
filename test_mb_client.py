#!/usr/bin/env python3
"""
Host test code for communicating with any of the modbus devices in
this repository.  Hexadecimal is accepted for all integer arguments
with a leading 0x
"""

import argparse

import logging
logging.basicConfig()
log = logging.getLogger()
log.setLevel(logging.DEBUG)

import pymodbus.transaction
import pymodbus.client.sync

def type_modbus_address(val):
    """Yes, all, even private addresses..."""
    a = int(val, 0)
    if a in range(0,255):
        return a
    raise argparse.ArgumentTypeError("Modbus addresses must be integers in range: 0..255")


def get_parser():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    group = parser.add_argument_group("Connection options")
    group.add_argument("-d", "--device", help="Serial device to open")
    group.add_argument("-p", "--port", help="TCP port to connect to", default=1502)
    group.add_argument("-H", "--host", help="TCP host to connect to", default="localhost")
    group.add_argument("-R", "--rtu-framing", help="Force use of RTU framing", action="store_true")
    group.add_argument("-b", "--baudrate",  help="Serial port baudrate", default=19200)
    group.add_argument("-y", "--parity", help="Serial port parity", choices="EON", default="E")
    group.add_argument("--renode-raw", help="don't try any termios setting on port open", action="store_true")
    # tcp framing is invalid on serial, so no real use for this option.
    #group.add_argument("-T", "--tcp-framing", help="Use TCP framing", action="store_true")
    # so, app -d /dev/blah -T should be tcp framing,
    # app -d /dev/blah should be rtu framing
    # and app by itself should do tcp client
    group = parser.add_argument_group("Test options")
    group.add_argument("-a", "--reg-address", help="Modbus register base address to request", default=0x2000, type=int)
    group.add_argument("-n", "--count", help="Count of registers to read", default=10, type=int)
    group.add_argument("-u", "--unit-address", help="Modbus unit id to address, integer in range 0..255",
                       default=0xa, type=type_modbus_address)
    return parser




if __name__ == "__main__":
    p = get_parser()
    opts = p.parse_args()
    print("opts are ", opts)
    client = None
    if opts.device:
        if opts.renode_raw:
            client = pymodbus.client.sync.ModbusSerialClient(method="rtu", port=opts.device)
        else:
            client = pymodbus.client.sync.ModbusSerialClient(method="rtu", port=opts.device, baudrate=opts.baudrate, parity=opts.parity)
    else:
        framer = pymodbus.framer.socket_framer.ModbusSocketFramer
        if opts.rtu_framing:
            framer = pymodbus.transaction.ModbusRtuFramer
        client = pymodbus.client.sync.ModbusTcpClient(host=opts.host, port=opts.port, framer=framer)

    client.connect()

    rr = client.read_holding_registers(opts.reg_address, opts.count, unit=opts.unit_address)
    print(rr)
    for i,r in enumerate(rr.registers):
        print(f"Reg addr {opts.reg_address+i:#04x}: {r}\t   ({r:#04x})")

    client.close()
