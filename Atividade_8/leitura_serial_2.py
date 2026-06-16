import serial
import threading
from queue import Queue, Empty
from collections import deque
import time

import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# ==========================
# CONFIGURAÇÕES
# ==========================

PORTA = "COM11"          # Altere para sua porta
BAUDRATE = 115200

N_PONTOS = 500

# ==========================
# BUFFER COMPARTILHADO
# ==========================

fila = Queue()

# ==========================
# THREAD DE LEITURA SERIAL
# ==========================

def ler_serial():
    ser = serial.Serial(PORTA, BAUDRATE, timeout=1)

    print("Serial conectada")
    contador = 0
    inicio = time.time()

    while True:
        try:
            linha = ser.readline().decode("utf-8").strip()

            if not linha:
                continue

            valores = list(map(float, linha.split(",")))

            # Espera X,Y,Z
            if len(valores) >= 3:
                fila.put(valores[:3])

                contador += 1

            if time.time() - inicio >= 1.0:

                    print(f"Taxa de aquisição: {contador} Hz")

                    contador = 0
                    inicio = time.time()

        except Exception as e:
            print("Erro serial:", e)


thread_serial = threading.Thread(
    target=ler_serial,
    daemon=True
)

thread_serial.start()

# ==========================
# DADOS PARA O GRÁFICO
# ==========================

x = deque([0] * N_PONTOS, maxlen=N_PONTOS)
y = deque([0] * N_PONTOS, maxlen=N_PONTOS)
z = deque([0] * N_PONTOS, maxlen=N_PONTOS)

# ==========================
# MATPLOTLIB
# ==========================

fig, ax = plt.subplots(figsize=(12, 6))

linha_x, = ax.plot([], [], label="X")
linha_y, = ax.plot([], [], label="Y")
linha_z, = ax.plot([], [], label="Z")

ax.set_xlim(0, N_PONTOS)
ax.set_ylim(-2, 2)

ax.set_title("Acelerômetro em Tempo Real")
ax.set_xlabel("Amostras")
ax.set_ylabel("Aceleração (g)")
ax.grid(True)
ax.legend()

# ==========================
# ATUALIZAÇÃO DO GRÁFICO
# ==========================

def atualizar(frame):

    recebeu_dado = False

    while True:
        try:
            ax_g, ay_g, az_g = fila.get_nowait()

            x.append(ax_g)
            y.append(ay_g)
            z.append(az_g)

            recebeu_dado = True

        except Empty:
            break

    if recebeu_dado:

        linha_x.set_data(range(len(x)), x)
        linha_y.set_data(range(len(y)), y)
        linha_z.set_data(range(len(z)), z)

        # Ajuste automático do eixo Y
        minimo = min(min(x), min(y), min(z))
        maximo = max(max(x), max(y), max(z))

        margem = 0.2

        ax.set_ylim(
            minimo - margem,
            maximo + margem
        )

    return linha_x, linha_y, linha_z

# Atualiza a cada 20 ms (~50 FPS)
ani = FuncAnimation(
    fig,
    atualizar,
    interval=20,
    blit=False
)

plt.tight_layout()
plt.show()