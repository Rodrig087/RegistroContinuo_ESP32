# -*- coding: utf-8 -*-
"""
Created on Wed Apr 22 21:33:17 2020

@author: milto
"""

import matplotlib.pyplot as plt
import numpy as np


#****************************************************************
# Datos de configuracion:
carpetaBin = "C:/Users/milto/Desktop/Procesamiento sismos/Version ESP32/EventosBin"  #Carpeta de entrada que almacena los archivos binarios a convertir
carpetaTxt = "C:/Users/milto/Desktop/Procesamiento sismos/Version ESP32/EventosTxt"  #Carpeta de salida que almacena los archivos de texto convertidos
correccionInercia = 1
#****************************************************************
tramaSize = 2506
banGuardar = 0
banExtraer = 0
contEje = 0
axisValue = 0
contMuestras = 0

axisData = [0 for i in range(4)]
xData = [0 for i in range(4)]
yData = [0 for i in range(4)]
zData = [0 for i in range(4)]

xTrama = []  #Vector para guardar los datos del eje X
yTrama = []  #Vector para guardar los datos del eje Y
zTrama = []  #Vector para guardar los datos del eje Z

numPisoChar = []
numNodoChar = []

#Ingreso de datos:
nombreArchivo = input("Ingrese el nombre del archivo: ")
factorDiezmado = 1
segundosDia = (24 * 3600)



#Abre el archivo binario:
path = carpetaBin + "/" + str(nombreArchivo) + ".dat"
f = open(path, "rb")
tramaDatos = np.fromfile(f, np.int8, 2506)

horaInicioSeg = (3600 * tramaDatos[4]) + (60 * tramaDatos[5]) + (tramaDatos[6])
horaInicio = str('%0.2d' % (int(horaInicioSeg / 3600))) + ":" + str('%0.2d' % (int((horaInicioSeg % 3600) / 60))) + ":" + str('%0.2d' % ((horaInicioSeg % 3600) % 60))
fechaEvento = str('%0.2d' % tramaDatos[1]) + "-" + str('%0.2d' % tramaDatos[2]) + "-" + str('%0.2d' % tramaDatos[3])

fechaInfo = str('%0.2d' % tramaDatos[1]) + str(
    '%0.2d' % tramaDatos[2]) + str(
        '%0.2d' % tramaDatos[3])
horaInfo = str('%0.2d' % (int(horaInicioSeg / 3600))) + str('%0.2d' % (int(
    (horaInicioSeg % 3600) / 60))) + str('%0.2d' %
                                         ((horaInicioSeg % 3600) % 60))

print(fechaEvento+" "+horaInicio)
                                         
#Obtiene y calcula el tiempo de inicio del muestreo
tiempoInicio = int((tramaDatos[tramaSize - 3] * 3600) +
                   (tramaDatos[tramaSize - 2] * 60) +
                   (tramaDatos[tramaSize - 1]))
#print (tiempoInicio)
print("Calculando...")


banExtraer = 1

if (banExtraer == 1):

    while (contMuestras < segundosDia):

        try:

            for i in range(7, 2506):  #Recorro las 2501 muestras del vector tramaDatos, omitiendo los 5 ultimos que corresponden a la fecha y hora del modulo gps
                if ((i-7) % (10 * factorDiezmado) == 0):  #Indica el inicio de un nuevo set de muestras
                    banGuardar = 1  #Cambia el estado de la bandera para permitir guardar la muestra
                    j = 0
                    contEje = 0
                    #print("Nuevo")
                    #print(i)
                else:
                    if (banGuardar == 1):
                        if (j < 2):
                            axisData[j] = tramaDatos[i]  #axisData guarda la informacion de los 3 bytes correspondientes a un eje
                            #print("XY")
                            #print(i)
                        else:
                            #print("Z")
                            #print(i)
                            axisData[2] = tramaDatos[i]  #Termina de llenar el vector axisData
                            axisValue = ((axisData[0] << 12) & 0xFF000) + ((axisData[1] << 4) & 0xFF0) + ((axisData[2] >> 4) & 0xF)

                            #Aplica el complemento a 2:
                            if (axisValue >= 0x80000):
                                axisValue = axisValue & 0x7FFFF  #Se descarta el bit 20 que indica el signo (1=negativo)
                                axisValue = -1 * (((~axisValue) + 1) & 0x7FFFF)

                            aceleracion = axisValue * (980 / pow(2, 18))  #Calcula la aceleracion en gales (float)

                            if (contEje == 0):
                                #xTrama.append(aceleracion-offTran)
                                acelY = aceleracion

                            if (contEje == 1):
                                #yTrama.append(aceleracion-offLong)
                                acelX = aceleracion

                            if (contEje == 2):
                                #zTrama.append(aceleracion-offVert)
                                acelZ = aceleracion

                                xTrama.append(acelX)
                                yTrama.append(acelY)
                                zTrama.append(acelZ)
                                
                                banGuardar = 0  #Despues de que termina de guardar la muestra del eje Z limpia la bandera banGuardar

                            j = -1
                            contEje = contEje + 1
                        j = j + 1

            tramaDatos = np.fromfile(f, np.int8, 2506)
            contMuestras = contMuestras + 1

        except:

            #print("Advertencia: Archivo incompleto")
            duracionEvento = str(contMuestras) + ' seg'
            print('Duracion del evento: ' + duracionEvento)
            x_ms = np.linspace(0, contMuestras*1000,(int(250 / factorDiezmado) * contMuestras)-contMuestras)
            contMuestras = segundosDia

    #Cierra el archivo binario:
    f.close()

    #Convierte los vectores X, Y y Z a formato np:
    npX = np.array(xTrama)
    npY = np.array(yTrama)
    npZ = np.array(zTrama)

    #Calcula la media:
    meanX = np.mean(npX)
    meanY = np.mean(npY)
    meanZ = np.mean(npZ)
    #Calcula la mediana:
    medX = np.median(npX)
    medY = np.median(npY)
    medZ = np.median(npZ)
    #Calcula el RMS de los datos de cada eje:
    rmsX = np.sqrt(np.mean(npX**2))
    rmsY = np.sqrt(np.mean(npY**2))
    rmsZ = np.sqrt(np.mean(npZ**2))
    print('')
    print('X:' + ' Media = ' + str(meanX) + ' Mediana = ' + str(medX) + ' RSM = ' + str(rmsX))
    print('Y:' + ' Media = ' + str(meanY) + ' Mediana = ' + str(medY) + ' RSM = ' + str(rmsY))
    print('Z:' + ' Media = ' + str(meanZ) + ' Mediana = ' + str(medZ) + ' RSM = ' + str(rmsZ))
    
    #Corrige los vectores
    #Corrige los valores de las 2 primeras muestras:
    if (correccionInercia==1):
        xTrama[0] = rmsX
        yTrama[0] = rmsY
        zTrama[0] = rmsZ
        xTrama[1] = rmsX
        yTrama[1] = rmsY
        zTrama[1] = rmsZ
    #Realiza el offset de la señales:
    for i in range(0, len(xTrama)):
        xTrama[i] = xTrama[i] - meanX
        yTrama[i] = yTrama[i] - meanY
        zTrama[i] = zTrama[i] - meanZ
    #Convierte los vectores corregidos a formato np:
    npXC = np.array(xTrama)
    npYC = np.array(yTrama)
    npZC = np.array(zTrama)
    
    #Calcula los maximos y minimos de los vectores corregidos:
    minX = np.min(npXC)
    maxX = np.max(npXC)
    minY = np.min(npYC)
    maxY = np.max(npYC)
    minZ = np.min(npZC)
    maxZ = np.max(npZC)
    #Calcula la amplitud Pico-Pico:
    ppX = round(abs(maxX - minX))
    ppY = round(abs(maxY - minY))
    ppZ = round(abs(maxZ - minZ))
    listaPP = [ppX, ppY, ppZ]
    maxPP = max(listaPP)

    #Calcula el RMS de los vectores corregidos:    
    rmsXC = np.sqrt(np.mean(npXC**2))
    rmsYC = np.sqrt(np.mean(npYC**2))
    rmsZC = np.sqrt(np.mean(npZC**2))
    print('')
    print('Long:' + ' RSM = ' + str(rmsXC))
    print('Tran:' + ' RSM = ' + str(rmsYC))
    print('Vert:' + ' RSM = ' + str(rmsZC))
    print('')
    print('xms: '+ str(len(x_ms)));
    print('xTrama: '+ str(len(xTrama)));
    print('yTrama: '+ str(len(yTrama)));
    print('zTrama: '+ str(len(zTrama)));
    print('Diferencia: ' + str(len(x_ms)-len(zTrama)))

    #Grafica las señales
    limY = (maxPP)
    
    fig = plt.figure()
    ax1 = fig.add_subplot(311)
    plt.ylim(-limY, limY)
    plt.plot(x_ms, xTrama)
    plt.setp(ax1.get_xticklabels(), visible=False)
    plt.title('Eje Longitudinal')
    plt.ylabel('Aceleracion [cm/seg2]')

    ax2 = plt.subplot(312, sharex=ax1)
    plt.ylim(-limY, limY)
    plt.plot(x_ms, yTrama)
    plt.setp(ax2.get_xticklabels(), visible=False)
    plt.title('Eje Transversal')
    plt.ylabel('Aceleracion [cm/seg2]')

    ax3 = plt.subplot(313, sharex=ax1)
    plt.ylim(-limY, limY)
    plt.plot(x_ms, zTrama)
    plt.title('Eje Vertical')
    infoTiempo = "Tiempo [ms]\n" "\n" + "Fecha: " + fechaEvento + "   Hora de inicio: " + horaInicio + "   Duracion: " + duracionEvento
    plt.xlabel(infoTiempo)
    plt.ylabel('Aceleracion [cm/seg2]')

    plt.show()

    
#************************************************************************************************************************************************************

