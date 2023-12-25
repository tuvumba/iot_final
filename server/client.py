import telnetlib
import time


def listenTN(connection):
    response = connection.read_until(b'\a\b')
    return response[:-2]


def send_and_recieve(text, connection, addAB):
    if addAB == 'Y':
        text += '\a\b'
    text = text.encode()
    print("Sent: ", text)
    connection.write(text)
    response = listenTN(connection)
    print(f"Response: {response.decode()}")
    if response.decode() == "CLOSED":
        return True
    else:
        return False

    # Connect to the server


host = "localhost"
port = 8080  # Default Telnet port
tn = telnetlib.Telnet(host, port)


while True:
    text = input("Enter text to send:")
    add = input("Do you want to add \\a\\b? (Y, N)")
    if send_and_recieve(text, tn, add):
        break


# Close the connection
tn.close()
print("CLOSED, END")
