__author__ = 'Barelang 7_DonnyPH'

from pickle import FALSE
from PyQt5 import QtGui
import sys, serial, serial.tools.list_ports, warnings
from PyQt5.QtCore import QObject, pyqtSignal, QThread, pyqtSignal, pyqtSlot
from PyQt5.QtWidgets import QApplication, QMainWindow, qApp
from PyQt5.uic import loadUi

#Port Detection START
ports = [
    p.device
    for p in serial.tools.list_ports.comports()
    if 'ACM' or 'USB' in p.description
]

if not ports:
    raise IOError("There is no device exist on serial port!")

if len(ports) > 0:
    warnings.warn('Connected....')

ser = serial.Serial(ports[0],115200)
#Port Detection END

# MULTI-THREADING
class Worker(QObject):
    finished = pyqtSignal()
    intReady = pyqtSignal(str)

    @pyqtSlot()
    def __init__(self):
        super(Worker, self).__init__()
        self.working = True

    def work(self):
        while self.working:
            line = ser.readline().decode('utf-8')
            print(line)
            self.intReady.emit(line)
        self.finished.emit()

class qt(QMainWindow):

    def __init__(self):

        QMainWindow.__init__(self)
        loadUi('cop.ui', self)

        self.thread = None
        self.worker = None
        self.connectButton.clicked.connect(self.start_loop)
        self.lineEdit_2.setText(ports[0])
        self.lineEdit.setText("115200")
        self.lineEdit.setReadOnly(True)
        self.lineEdit.setReadOnly(True)
        self.image35.setVisible(False)
        self.image40.setVisible(False)
        self.image45.setVisible(False)
        self.Lcenter.setVisible(False)
        self.Rcenter.setVisible(False)
        self.Scenter.setVisible(False)
        self.textnoimage.setVisible(True)

    def loop_finished(self):
        print('Loop Finished')

    def start_loop(self):

        self.worker = Worker()   # a new worker to perform those tasks
        self.thread = QThread()  # a new thread to run our background tasks in
        
        self.worker.moveToThread(self.thread)  # move the worker into the thread, do this first before connecting the signals

        self.thread.started.connect(self.worker.work) # begin our worker object's loop when the thread starts running

        self.worker.intReady.connect(self.onIntReady)        

        self.disconnectButton.clicked.connect(self.stop_loop)      # stop the loop on the stop button click

        self.worker.finished.connect(self.loop_finished)       # do something in the gui when the worker loop ends
        self.worker.finished.connect(self.thread.quit)         # tell the thread it's time to stop running
        self.worker.finished.connect(self.worker.deleteLater)  # have worker mark itself for deletion
        self.thread.finished.connect(self.thread.deleteLater)  # have thread mark itself for deletion

        self.thread.start()

    def stop_loop(self):
        self.worker.working = False

    def onIntReady(self, i):
        self.textEdit.append("{}".format(i))
        print(i)
        self.textEdit.moveCursor(QtGui.QTextCursor.End)

        # Parsing Data from serial port
        serialparse = i.split("/")
        a = serialparse[0]
        b = serialparse[1]
        c = serialparse[2]
        d = serialparse[3]
        e = serialparse[4]
        f = serialparse[5]

        self.textBrowser.append("{}".format(a))
        self.textBrowser_4.append("{}".format(b))
        self.textBrowser_2.append("{}".format(c))
        self.textBrowser_5.append("{}".format(d))
        self.textBrowser_3.append("{}".format(e))
        self.textBrowser_6.append("{}".format(f))
        self.textBrowser.moveCursor(QtGui.QTextCursor.End)
        self.textBrowser_2.moveCursor(QtGui.QTextCursor.End)
        self.textBrowser_3.moveCursor(QtGui.QTextCursor.End)
        self.textBrowser_4.moveCursor(QtGui.QTextCursor.End)
        self.textBrowser_5.moveCursor(QtGui.QTextCursor.End)
        self.textBrowser_6.moveCursor(QtGui.QTextCursor.End)

        self.Lcenter.setVisible(True)
        self.Rcenter.setVisible(True)
        self.Scenter.setVisible(True)

        # Set for moving point based pixel
        move_a = 2*int(a) + 648
        move_b = 420 - 2*int(b)
        move_c1 = 2*int(c) + 938
        move_c2 = 2*int(c) + 948
        move_c3 = 2*int(c) + 958
        move_d = 420 - 2*int (d) 
        move_e = 2*int(e) + 648
        move_f = 420 - 2*int(f)
        
        if self.button35.isChecked():
            self.Rcenter.move(move_c1,move_d)
        if self.button40.isChecked():
            self.Rcenter.move(move_c2,move_d)
        if self.button45.isChecked():
            self.Rcenter.move(move_c3,move_d)

        self.Lcenter.move(move_a,move_b)
        self.Scenter.move(move_e,move_f)
        self.update()

    #Connet to serial port
    def on_disconnectButton_clicked(self):
        self.textEdit_2.setText('Disconnected! Please click CONNECT...')
        self.label_2.setText("DISCONNECTED!")
        self.label_2.setStyleSheet('color: red')
        self.progressBar.setValue(0)
    
    def on_connectButton_clicked(self):
        self.completed = 0
        while self.completed < 100:
            self.completed += 0.005
            self.progressBar.setValue(self.completed)
        
        self.textEdit_2.setText('Data Gathering...')
        self.label_2.setText("CONNECTED!")
        self.label_2.setStyleSheet('color: green')
        self.textnoimage.setVisible(False)

        if self.button35.isChecked():
            self.image35.setVisible(True)
            self.image40.setVisible(False)
            self.image45.setVisible(False)
        if self.button40.isChecked():
            self.image40.setVisible(True)
            self.image35.setVisible(False)
            self.image45.setVisible(False)	
        if self.button45.isChecked():
            self.image45.setVisible(True)
            self.image35.setVisible(False)
            self.image40.setVisible(False)

    def on_pushButton_3_clicked(self):
        #Clear text(str)
        self.textEdit.clear()
        self.textBrowser.clear()
        self.textBrowser_2.clear()
        self.textBrowser_3.clear()
        self.textBrowser_4.clear()
        self.textBrowser_5.clear()
        self.textBrowser_6.clear()

        self.Lcenter.move(660,412)
        self.Rcenter.move(660,412)
        self.Scenter.move(660,412)        

    def on_pushButton_4_clicked(self):
        # Send data from serial port:
        ser.write('t'.encode())
        self.textEdit_2.setText("Tare Complete")
    
    def on_quitbutton_clicked(self):
        # Close app:
        qApp.quit()
        self.hide()

def run():
    app = QApplication(sys.argv)
    widget = qt()
    widget.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    run()