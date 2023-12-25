import socket
import select
from Crypto.PublicKey import RSA
from Crypto.Cipher import PKCS1_OAEP

#   LOGIN_REQUEST   ->   LOGIN_CONTINUE | LOGIN_FORBIDDEN
#   LOGIN_FORBIDDEN -- close connection, M5 will show login failure
#
#   LOGIN_SUCCESS | LOGIN_FAIL      <-    LOGIN_PASSPHRASE
#   LOGIN_SUCCESS  -- close connection, M5 will show success
#   LOGIN_FAIL -> close connection, fail
#               if login failed, M5 will go to (0)
#               if login failed 3 times, M5 will do an alarm
#
#
#
#

LOGIN_REQUEST = b"LOGIN_REQUEST\a"
LOGIN_CONTINUE = b"LOGIN_CONTINUE\a"
LOGIN_FORBIDDEN = b"LOGIN_FORBIDDEN\a"
LOGIN_SUCCESS = b"LOGIN_SUCCESS\a"
LOGIN_FAIL = b"LOGIN_FAIL\a"
CONNECTION_CLOSED = b"CLOSED\a"
BAD_REQUEST = b"BAD_REQUEST\a"

if __name__ == "__main__":

    listener_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listener_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    isLoginActive = True if input("Is login available? (Y / N) ") == 'Y' else False
    password = input("Enter password for server: ")

    address = "192.168.126.175"
    address_port = (address, 8080)
    listener_socket.bind(address_port)
    listener_socket.listen(1)
    print(f"Server listening @ {address}:8080")

    while True:
        client_socket, client_address = listener_socket.accept()
        client_socket.settimeout(100)
        stage = 0
        while True:
            try:
                client_msg = client_socket.recv(256)
                if client_msg != '':
                    print(f"Client said: |{client_msg.decode()[:-2]}|")
                    if client_msg.decode() == "LOGIN_REQUEST":
                        if isLoginActive:
                            client_socket.sendall(LOGIN_CONTINUE)
                            print("SEND LOGIN_CONTINUE")
                            stage = 1
                            print("STAGE IS NOW 1")
                        else:
                            client_socket.sendall(LOGIN_FORBIDDEN)
                            print("LOGIN_FORBIDDEN, CONNECTION CLOSED")
                            client_socket.sendall(CONNECTION_CLOSED)
                            client_socket.close()
                            break
                    elif stage == 1:
                        m5_password = client_msg.decode()[:-2]
                        print(f"M5 sent password: |{m5_password}|")
                        if m5_password == password:
                            print("PASSWORD CORRECT")
                            client_socket.sendall(LOGIN_SUCCESS)
                            client_socket.sendall(CONNECTION_CLOSED)
                            client_socket.close()
                            break
                        else:
                            print("PASSWORD INCORRECT")
                            client_socket.sendall(LOGIN_FAIL)
                            client_socket.sendall(CONNECTION_CLOSED)
                            client_socket.close()
                            break
                    else:
                        client_socket.sendall(BAD_REQUEST)
                        client_socket.sendall(CONNECTION_CLOSED)
                        client_socket.close()
                        break
            except socket.timeout:
                print("Timeout!")

        try:
            # close the connection
            client_socket.close()
        except OSError:
            # client disconnected first, nothing to do
            pass
