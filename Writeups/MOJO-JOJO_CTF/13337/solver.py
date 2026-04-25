import socket
import struct

def run_solver():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    try:
        server.bind(('127.0.0.1', 13337))
    except OSError:
        print("[!] Port 13337 already in use")
        return
        
    server.listen(5)
    
    print("listening on 13337...")
    targets = [
        0x85183DB0, 0x5066E5B5, 0xCBF9F35F, 0x2B4627A3,
        0xF732E92C, 0xF002F1E0, 0xD399FD13, 0x6CF9E00F
    ]

    while True:
        client, addr = server.accept()
        print(f"[*] Intercepted connection from {addr}")
        
        try:
            magic_client = client.recv(4)
            if magic_client != b'\x0D\xF7\xB6\xFA':
                print(f"[-] Invalid magic client: {magic_client.hex()}")
                client.close()
                continue
            print("[*] Sending handshake...")
            handshake = b'\x8A\xE1\xAF\xF5' + struct.pack("<II", 32, 0)
            client.send(handshake)
            
           
            session_data = client.recv(4)
            if not session_data:
                client.close()
                continue
            
            session_id = struct.unpack("<I", session_data)[0]
            print(f"[*] Captured Session ID: 0x{session_id:08X}")
            
            
            response = b""
            for t in targets:
                response += struct.pack("<I", t ^ session_id)
            
            print("[*] Dispatching Synchronized Data Frame...")
            client.send(response)
            
        except Exception as e:
            print(f"[-] Error: {e}")
        finally:
            client.close()

if __name__ == "__main__":
    run_solver()
