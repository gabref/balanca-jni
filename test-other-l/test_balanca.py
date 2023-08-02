# ---------------------------------------------------------------------------------------------------------------------------------------------------
# Exemplo: Balança Python
# 15MAI23

# ---------------------------------------------------------------------------------------------------------------------------------------------------
import ctypes
import platform

# ---------------------------------------------------------------------------------------------------------------------------------------------------
if platform.system() == "Windows":
    ffi = ctypes.WinDLL("./E1_Balanca01.dll")
else:
    ffi = ctypes.cdll.LoadLibrary("./libE1_EGR.so")
    

def ConfigurarModeloBalanca(modelo):
    fn = ffi.ConfigurarModeloBalanca
    fn.restype = ctypes.c_int
    fn.argtypes = [ctypes.c_int]

    modelo = ctypes.c_int(modelo)

    return fn(modelo)

def ConfigurarProtocoloComunicacao(protocolo):
    fn = ffi.ConfigurarProtocoloComunicacao
    fn.restype = ctypes.c_int
    fn.argtypes = [ctypes.c_int]

    protocolo = ctypes.c_int(protocolo)

    return fn(protocolo)

def ObterModeloBalanca():
    fn = ffi.ObterModeloBalanca
    fn.restype = ctypes.c_int
    fn.argtypes = []

    return fn()

def AbrirSerial(device, baudrate, length, parity, stopbits):
    fn = ffi.AbrirSerial
    fn.restype = ctypes.c_int
    fn.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.c_int, ctypes.c_char, ctypes.c_int]

    device = ctypes.c_char_p(device)
    baudrate = ctypes.c_int(baudrate)
    length = ctypes.c_int(length)
    parity = ctypes.c_char(bytes(parity, "utf-8"))
    stopbits = ctypes.c_int(stopbits)

    return fn(device, baudrate, length, parity, stopbits)

def Fechar():
    fn = ffi.Fechar
    fn.restype = ctypes.c_int
    fn.argtypes = []

    return fn()

def LerPeso(qtLeituras):
    fn = ffi.LerPeso
    fn.restype = ctypes.c_char_p
    fn.argtypes = [ctypes.c_int]

    qtLeituras = ctypes.c_int(qtLeituras)

    return fn(qtLeituras)

MODELO_BALANCA = 3
PROTOCOLO_COMUNICACAO = 0
BAUDRATE = 2400
LENGTH = 8
PARITY = 'N'
STOPBITS = 1
protocolo_g = [0]

def configureScale(modelo, protocolo, portaSerial):
    if (ObterModeloBalanca() == -1):
        ret = ConfigurarModeloBalanca(modelo)
        if ret != 0:
            return ret
    if (protocolo_g[0] == 0):
        ret = ConfigurarProtocoloComunicacao(protocolo)
        protocolo_g[0] = 1
        if ret != 0:
            return ret
    ret = AbrirSerial(portaSerial, BAUDRATE, LENGTH, PARITY, STOPBITS)
    if ret != 0:
        return ret
    return ret


def ObterNumeroSerie():
    print('\nrealizando nova leitura')
    ret = configureScale(MODELO_BALANCA, PROTOCOLO_COMUNICACAO, b"COM4")
    print('ret configure' + str(ret))

    modelo = ObterModeloBalanca()
    print('modelo: ' + str(modelo))

    # fecha = Fechar()
    # print('fechar: ' + str(fecha))


def lendoPeso():
    print('\nrealizando nova leitura')
    ret = configureScale(MODELO_BALANCA, PROTOCOLO_COMUNICACAO, b"COM4")
    print('ret configure' + str(ret))

    peso = LerPeso(1)
    print('peso: ' + str(peso))

    fecha = Fechar()
    print('fechar: ' + str(fecha) + '\n')

def main():

    modelo = ObterModeloBalanca()
    print('modelo' + str(modelo))


    peso = lendoPeso()
    print('peso: ' + str(peso))

    peso = lendoPeso()
    print('peso: ' + str(peso))

    numero = ObterNumeroSerie()
    print('numero: ' + str(numero))

if __name__ == "__main__":
    main()