import time
import serial

def send_hex_command(port_name, hex_command, response_size):
    try:
        # Open the serial port
        serial_port = serial.Serial(port_name, baudrate=2400)
        
        # Convert hex command to bytes
        binary_data = bytes.fromhex(hex_command)
        
        # Write the binary data to the serial port
        serial_port.write(binary_data)
        
        # Read the response
        # response = serial_port.read(response_size)
        response = bytearray()
        while len(response) < response_size:
            if serial_port.in_waiting > 0:
                data = serial_port.read(serial_port.in_waiting)
                response += data
        
        # Close the serial port
        serial_port.close()
        
        return response
    except Exception as e:
        print(f"An error occurred: {e}")
        return None

# Example usage
port_name = "COM4"  # Replace with your actual port name
hex_command = "05"  # Replace with your hex command
response_size = 7  # Replace with the expected response size

# Start timing
start_time = time.time()

response = send_hex_command(port_name, hex_command, response_size)

end_time = time.time()
responsetime = end_time - start_time

if response is not None:
    print(f"Response: {response}")
    print(f"Time taken: {responsetime:.5f} seconds")
else:
    print("no response")

