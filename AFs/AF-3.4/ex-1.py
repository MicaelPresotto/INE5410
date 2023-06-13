from time import sleep
from random import randint
from threading import Thread, Lock, Condition

def produtor():
    global buffer
    for i in range(10):
        sleep(randint(0, 2))  # fica um tempo produzindo...
        item = 'item ' + str(i)
        with lock:
            while len(buffer) == tam_buffer:
                print('>>> Buffer cheio. Produtor irá aguardar.')
                lugar_no_buffer.wait()  # aguarda que haja lugar no buffer
            buffer.append(item)
            print('Produzido %s (há %i itens no buffer)' % (item, len(buffer)))
            if len(buffer) == 1:
                item_no_buffer.notify_all()

def consumidor():
    global buffer
    for i in range(10):
        with lock:
            while len(buffer) == 0:
                print('>>> Buffer vazio. Consumidor irá aguardar.')
                item_no_buffer.wait()  # aguarda que haja um item para consumir
            item = buffer.pop(0)
            print('Consumido %s (há %i itens no buffer)' % (item, len(buffer)))
            if len(buffer) == tam_buffer - 1:
                lugar_no_buffer.notify_all()
        sleep(randint(0, 2))  # fica um tempo consumindo...

buffer = []
tam_buffer = 5
lock = Lock()
lugar_no_buffer = Condition(lock)
item_no_buffer = Condition(lock)
produtores = []
consumidores = []

for i in range(2):
    produtores.append(Thread(target=produtor))
    consumidores.append(Thread(target=consumidor))

for i in range(2):
    produtores[i].start()
    consumidores[i].start()

for i in range(2):
    produtores[i].join()
    consumidores[i].join()