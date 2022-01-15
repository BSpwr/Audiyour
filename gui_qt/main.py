from PySide2 import QtCore as Qc
from PySide2 import QtWidgets as Qw
from PySide2 import QtGui as Qg

from equalizer import Equalizer

import sys
import asyncio
import signal
from bluetoothManager import BluetoothManager
from qasync import QEventLoop

class MainWindow(Qw.QMainWindow):
    def __init__(self, app, loop, parent=None):
        super().__init__(parent)

        self.loop = loop
        self.bt_man = BluetoothManager()
        
        desktop_size = Qg.QGuiApplication.primaryScreen().availableGeometry().size()
        self.resize(desktop_size.width() * 0.4, desktop_size.height() * 0.5)
        
        # if len(ports) != 0: self.ser_man.set_port(ports[0])
        
        self.setWindowTitle('Audiyour Control')
        
        self.main_ui = MainUI(parent=self)
        # Set the central widget of the Window.
        self.setCentralWidget(self.main_ui)
        
        self.show()

    def closeEvent(self, event):
        # do stuff
        asyncio.ensure_future(self.bt_man.disconnect())
        event.accept() # let the window close


class MainUI(Qw.QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)

        # self.manager = gatt.DeviceManager(adapter_name='hci0')

        self.bt_man = self.parent().bt_man
        # self.device.connect()

        # self.manager.run()

        # self.device.write_gain_index(0, 0)
    
        self.equalizer = Equalizer(parent=self)
        # self.diff_surround = DiffSurround(parent=self)

        self.devices = {}
                
        self.scan = Qw.QPushButton('Scan')
        self.scan.clicked.connect(lambda: asyncio.ensure_future(self.update_devices()))
        
        self.device_sel = DeviceComboBox(parent=self)
        self.device_sel.activated.connect(lambda port_idx: asyncio.ensure_future(self.update_device(port_idx)))

        self.check_conn_label = Qw.QLabel()
        self.check_conn_label.setAlignment(Qc.Qt.AlignHCenter)

        self.disconnect_btn = Qw.QPushButton('Disconnect')
        self.disconnect_btn.clicked.connect(lambda: asyncio.ensure_future(self.disconnect()))
        
        self.conn_status_layout = Qw.QHBoxLayout()
        self.conn_status_layout.addWidget(self.scan)
        self.conn_status_layout.addWidget(self.device_sel)
        self.conn_status_layout.addWidget(self.check_conn_label)
        self.conn_status_layout.addWidget(self.disconnect_btn)

        self.conn_group = Qw.QGroupBox("Connection")
        self.conn_group.setLayout(self.conn_status_layout)
        
        self.equalizer_layout = Qw.QHBoxLayout()
        self.equalizer_layout.addWidget(self.equalizer)
        
        self.equalizer_group = Qw.QGroupBox("Equalizer")
        self.equalizer_group.setLayout(self.equalizer_layout)
        
        # self.diff_surround_layout = Qw.QHBoxLayout()
        # self.diff_surround_layout.addWidget(self.diff_surround)
        
        # self.diff_surround_group = Qw.QGroupBox("Differential Surround")
        # self.diff_surround_group.setLayout(self.diff_surround_layout)
        
        self.layout = Qw.QVBoxLayout()
        self.layout.addWidget(self.conn_group)
        self.layout.addWidget(self.equalizer_group)
        # self.layout.addWidget(self.diff_surround_group)
        self.setLayout(self.layout)
        
        asyncio.ensure_future(self.update_devices())

        self.update_conn_status()
        
        self.show()

    async def update_devices(self):
        devices = await self.bt_man.discover_availiable_devices()
        for device in devices:
            self.devices[device.name] = device.address
        self.device_sel.updateDevices(devices)
        
    async def update_device(self, port_idx: str):
        self.bt_man.set_address(self.devices[self.device_sel.itemText(port_idx)])
        await self.bt_man.connect()
        self.update_conn_status()
        if self.bt_man.is_connected():
            # # load values from DSP
            asyncio.ensure_future(self.equalizer.load_settings())

    async def disconnect(self):
        await self.bt_man.disconnect()
        self.update_conn_status()
    
    def update_conn_status(self):
        if self.bt_man.is_connected():
            self.check_conn_label.setText("Device Connected ✔️")
        else:
            self.check_conn_label.setText("Device Disconnected ✖️")
        pass

class DeviceComboBox(Qw.QComboBox):
    def __init__(self, parent=None):
        super().__init__(parent)
        # asyncio.ensure_future(self.updateDevices())
            
    def showPopup(self):
        # asyncio.ensure_future(self.updatePorts())
        super().showPopup()

    def updateDevices(self, devices):
        self.clear()
        for device in devices:
            self.addItem(device.name)

# app = Qw.QApplication(sys.argv)  # Create application

# loop = QEventLoop(app)
# asyncio.set_event_loop(loop)  # NEW must set the event loop

def main():
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    
    app = Qw.QApplication(sys.argv)  # Create application
    loop = QEventLoop(app)
    asyncio.set_event_loop(loop) # Allows for spawing async threads from QT code

    app.setPalette(app.style().standardPalette())
    window = MainWindow(app, loop)  # Create main window
    
    # https://xinhuang.github.io/posts/2017-07-31-common-mistakes-using-python3-asyncio.html
    try:
        loop.run_forever()
        tasks = asyncio.all_tasks()
        for t in [t for t in tasks if not (t.done() or t.cancelled())]:
            # give canceled tasks the last chance to run
            loop.run_until_complete(t)
    finally:
        loop.close()

if __name__ == '__main__':
    main()
