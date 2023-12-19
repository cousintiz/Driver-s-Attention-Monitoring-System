import socket
import cv2, sys
from ultralytics import YOLO
from deepface import DeepFace
from time import sleep

# models
model = YOLO("./models/yolov8m.pt") 
head = YOLO("./models/head.pt")
eyes = YOLO("./models/eyes.pt")

#fonte da camera
cap = cv2.VideoCapture(2) 

#class names in the model
names = model.names
eye_names = eyes.names
head_names = head.names
predThresHold = 1.5

#socket parameters
HOST = '***********'
PORT = 1234

server= socket.socket()
alert = "Driver is Drowsy"

def setup_socket():
    #set serever and client as global so that they can visibile
    global server, client
        
    server.bind((HOST, PORT))		
    print("socket binded to %s" %(PORT))

    server.listen(5)	
    print ("socket is listening")	

    # Estabelecer connection with client.	
    client, addr = server.accept()	
    print ('Got connection from', addr)


def scan_driver(frame) -> bool:
    """
    return: 
    """
    _class = None
    try:
        results = model(source=frame, conf = 0.6, verbose=False)
        for result in results:
            for aux in result.boxes.cls:
                _class = names[int(aux)]
                if _class not in ['person', 'Person']:
                    return False
    except Exception as e:
        print(f"Error:{e}")
    # retorna a classe e a frame em analise
    print(f"scan_driver:{_class}")
    return True


def scan_eyes(frame) -> str:
    """
    return: class predicted 
    """
    _class = None
    try:
        results = eyes(source=frame, conf = 0.6, verbose=False)
        for result in results:
            for aux in result.boxes.cls:
                _class = eye_names[int(aux)]
    except Exception as e:
        print(f"Error:{e}")
    # retorna a classe e a frame em analise
    print(f"scan_eyes:{_class}")
    return _class


def scan_head(frame) -> str:
    """
    return: class predicted 
    """
    _class = None
    try:
        results = head(source=frame, conf = 0.6, verbose=False)
        for result in results:
            for aux in result.boxes.cls:
                _class = head_names[int(aux)]
    except Exception as e:
        print(f"Error:{e}")
    # retorna a classe e a frame em analise
    print(f"scan_head:{_class}")
    return _class


def see_face(frame) -> str:
    #funcao para detecao de pessoas
    try:
        #deepface method for identify person face
        result = DeepFace.extract_faces(frame, enforce_detection = False)
        result = result[0]["confidence"]
    except Exception as e:
        print(f"Error:{e}")
    # retorna a confidence, e a frame em analise
    return result


def trigger() -> bool:
    flag = 0
    time = 10

    while (time > 0) and  (flag < 6 ):
        _, frame = cap.read()
        if scan_head(frame) != "awake":
            flag += 1
        else:
            flag -= 1
        if see_face(frame) < predThresHold:
            flag += 1

        time -= 1


    if flag >= 6:
        _, frame = cap.read()
        if scan_driver(frame):
            return True
    else:
        return False


def analyze()-> bool:

    flag = 0
    time = 20

    while time > 0 and  (flag < 13 ):
        _, frame = cap.read()
        if scan_head(frame) != "awake" :
            flag += 1
        else:
            flag -= 1
        if see_face(frame) < predThresHold:
            flag += 1
        if scan_eyes(frame) != "Non Drowsy":
            flag += 1
        else: 
            flag -=1
        time -= 1

    if flag > 10:
        _, frame = cap.read()
        if scan_driver(frame):
            return True
        else:
            return False
    else:
        return False


def cleanUp():
    server.close()
    cap.release()
    sys.exit()


def run():
    try:
        while True:
            try:
                _, frame = cap.read()
                if scan_driver(frame) is True:
                    print("Quickly see his attention")
                    if trigger() is True:
                        print("Driver needs to be checked")
                        if analyze() is True:
                            print("Send alert to system 2...")
                            # manda alerta ao sistema 2
                            client.send(alert.encode()) 
                            # recebe mensagem do sistema 2
                            feedback = client.recv(1024).decode() 
                            print(feedback)
                            sleep(2) 
                    else:
                        print("Driver is okay...")
                else:
                    print(f"No driver was detected")
            except KeyboardInterrupt:
                print("a fazer marcha-a trás")
                sleep(5)
                print("a tirar marcha-a trás")
                run() #voltar a correr
    except KeyboardInterrupt:          
        cleanUp()


if __name__ == "__main__":  
    setup_socket() # set up connection  
    run()