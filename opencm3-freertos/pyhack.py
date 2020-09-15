from pymodbus.client.sync import ModbusTcpClient as ModbusClient

#from pymodbus.transaction import ModbusSocketFramer as ModbusFramer
from pymodbus.transaction import ModbusRtuFramer as ModbusFramer
#from pymodbus.transaction import ModbusBinaryFramer as ModbusFramer
#from pymodbus.transaction import ModbusAsciiFramer as ModbusFramer

import logging
logging.basicConfig()
log = logging.getLogger()
log.setLevel(logging.DEBUG)

if __name__ == "__main__":
    # ----------------------------------------------------------------------- #
    # Initialize the client
    # ----------------------------------------------------------------------- #
    client = ModbusClient('localhost', port=4321, framer=ModbusFramer)
    client.connect()

    # ----------------------------------------------------------------------- #
    # perform your requests
    # ----------------------------------------------------------------------- #
    rr = client.read_holding_registers(1, 10, unit=0xa)
    print(rr)

    # ----------------------------------------------------------------------- #
    # close the client
    # ---------------------------------------------------------------------- #
    client.close()
