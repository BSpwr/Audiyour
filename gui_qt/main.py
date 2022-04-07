from PySide2 import QtCore as Qc
from PySide2 import QtWidgets as Qw
from PySide2 import QtGui as Qg

from equalizer import Equalizer
from mixer import Mixer

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
        self.resize(desktop_size.width() * 0.3, desktop_size.height() * 0.8)

        self.setWindowTitle('Audiyour Control')

        self.main_ui = MainUI(parent=self)
        # Set the central widget of the Window.
        self.setCentralWidget(self.main_ui)

        self.show()

    def closeEvent(self, event):
        # Disconnect from GATT server before closing window
        asyncio.ensure_future(self.bt_man.disconnect())
        event.accept()


class MainUI(Qw.QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)

        self.bt_man = self.parent().bt_man

        self.equalizer = Equalizer(parent=self)
        self.mixer = Mixer(parent=self)

        self.devices = {}

        # Connection status interactables
        self.connection_button = Qw.QPushButton("Connection Settings")
        self.connection_button.clicked.connect(lambda: self.connection_settings_ui.show())

        self.conn_status_label = Qw.QLabel()
        self.conn_status_label.setAlignment(Qc.Qt.AlignHCenter)

        self.conn_status_layout = Qw.QHBoxLayout()
        self.conn_status_layout.addWidget(self.connection_button)
        self.conn_status_layout.addWidget(self.conn_status_label)

        self.NUM_PROFILES = 5
        self.profile_sel = ProfileComboBox(self.NUM_PROFILES, parent=self)
        self.profile_sel.activated.connect(lambda profile_idx: asyncio.ensure_future(self.update_profile(profile_idx)))

        self.profile_load_btn = Qw.QPushButton('Load Profile')
        self.profile_load_btn.clicked.connect(lambda: asyncio.ensure_future(self.load_profile()))

        self.profile_save_btn = Qw.QPushButton('Save Profile')
        self.profile_save_btn.clicked.connect(lambda: asyncio.ensure_future(self.bt_man.write_save_profile()))

        self.load_settings_btn = Qw.QPushButton("Load From Device")
        self.load_settings_btn.clicked.connect(lambda: asyncio.ensure_future(self.load_settings()))

        self.profile_layout = Qw.QHBoxLayout()
        self.profile_layout.addWidget(self.profile_sel)
        self.profile_layout.addWidget(self.profile_load_btn)
        self.profile_layout.addWidget(self.profile_save_btn)
        self.profile_layout.addWidget(self.load_settings_btn)

        self.conn_group = Qw.QGroupBox("Connection")
        self.conn_group.setLayout(self.conn_status_layout)

        self.profile_group = Qw.QGroupBox("Profile")
        self.profile_group.setLayout(self.profile_layout)

        self.equalizer_layout = Qw.QHBoxLayout()
        self.equalizer_layout.addWidget(self.equalizer)

        self.equalizer_group = Qw.QGroupBox("Equalizer")
        self.equalizer_group.setLayout(self.equalizer_layout)

        self.mixer_layout = Qw.QHBoxLayout()
        self.mixer_layout.addWidget(self.mixer)

        self.mixer_group = Qw.QGroupBox("Mixer")
        self.mixer_group.setLayout(self.mixer_layout)

        self.layout = Qw.QVBoxLayout()
        self.layout.addWidget(self.conn_group)
        self.layout.addWidget(self.profile_group)
        self.layout.addWidget(self.equalizer_group)
        self.layout.addWidget(self.mixer_group)
        self.setLayout(self.layout)

        # Inner window
        self.connection_settings_ui = ConnectionSettingsUI(parent=self)
        self.connection_settings_ui.device_connected_successfully.connect(lambda: asyncio.ensure_future(self.handle_device_connected()))
        self.conn_status_label.setText(self.connection_settings_ui.conn_status_label.text())
        self.connection_settings_ui.status_string_updated.connect(lambda x: self.conn_status_label.setText(x))

        asyncio.ensure_future(self.connection_settings_ui.scan_devices())

        self.show()

    async def update_profile(self, profile_idx: int):
        await self.bt_man.write_profile_index(profile_idx)
        if self.bt_man.is_connected():
            await self.equalizer.load_settings()
            await self.mixer.load_settings()


    async def load_profile(self):
        await self.bt_man.write_load_profile()
        while (await self.bt_man.read_load_profile()):
            await asyncio.sleep(0.1)
        if self.bt_man.is_connected():
            await self.equalizer.load_settings()
            await self.mixer.load_settings()


    async def load_settings(self):
        await self.equalizer.load_settings()
        await self.mixer.load_settings()


    async def handle_device_connected(self):
        if self.bt_man.is_connected():
            # # load values from DSP
            await self.equalizer.load_settings()
            await self.mixer.load_settings()
            await self.bt_man.read_profile_index()
            self.profile_sel.setCurrentIndex(self.bt_man.profile_index)


class ConnectionSettingsUI(Qw.QDialog):
    """
    UI to scan for, choose, and rename bluetooth Device
    """

    device_connected_successfully = Qc.Signal()
    status_string_updated = Qc.Signal(str)

    def __init__(self, parent=None):
        super().__init__(parent)

        self.setWindowTitle('Audiyour Connection Settings')
        self.setWindowFlag(Qc.Qt.WindowContextHelpButtonHint, on=False)

        # Holds scanned devices
        self.devices = {}
        self.current_device = None

        self.bt_man = self.parent().bt_man

        # Filter through found devices
        self.filter_devices_box = Qw.QLineEdit()
        self.filter_devices_box.setPlaceholderText("Filter by device name")
        self.filter_devices_box.textChanged.connect(lambda x: self.apply_filter(x))
        # List of devices found
        self.devices_list = Qw.QListWidget()
        self.devices_list.itemPressed.connect(lambda x: asyncio.ensure_future(self.update_device(x.text())))

        # Connection status
        self.string_device_disconnected = "✖️ Not Connected"
        self.string_device_invalid = "✖️ Connected to Invalid Device"
        self.conn_status_label = Qw.QLabel(self.string_device_disconnected)
        self.conn_status_label.setAlignment(Qc.Qt.AlignHCenter)
        # Check connection
        self.check_connection_btn = Qw.QPushButton('Check Connection')
        self.check_connection_btn.setAutoDefault(False)
        self.check_connection_btn.clicked.connect(lambda: asyncio.ensure_future(self.update_conn_status()))
        # Disconnect
        self.disconnect_btn = Qw.QPushButton('Disconnect')
        self.disconnect_btn.setAutoDefault(False)
        self.disconnect_btn.clicked.connect(lambda: asyncio.ensure_future(self.disconnect()))

        # Scanning timeout input
        self.scanning_timeout_label = Qw.QLabel("Scanning timeout (seconds)")
        self.scanning_timeout_box = Qw.QSpinBox()
        self.scanning_timeout_box.setMinimum(0)
        self.scanning_timeout_box.setMaximum(10)
        self.scanning_timeout_box.setSingleStep(1)
        self.scanning_timeout_box.setValue(4) # default 4 second timeout
        self.scanning_timeout_layout = Qw.QHBoxLayout()
        self.scanning_timeout_layout.addWidget(self.scanning_timeout_label)
        self.scanning_timeout_layout.addWidget(self.scanning_timeout_box)
        # Start scan
        self.scan_button = Qw.QPushButton("Start Scan")
        self.scan_button.clicked.connect(lambda: asyncio.ensure_future(self.scan_devices()))
        # Reports scan status
        self.scan_status_label = Qw.QLabel("Please run scan.")

        # Device renaming interactables
        self.device_name_box = Qw.QLineEdit()
        self.device_name_box.setPlaceholderText("Device name cannot be blank")
        self.device_name_box.setMaxLength(32)
        self.device_name_box.setAlignment(Qc.Qt.AlignLeft)
        # Rename device
        self.rename_btn = Qw.QPushButton('Rename Device (Restarts Device)')
        self.rename_btn.clicked.connect(lambda: asyncio.ensure_future(self.rename_device(self.device_name_box.text())))
        self.rename_btn.setDisabled(True)
        # Disable button if name is only whitespace
        self.device_name_box.textChanged.connect(lambda name: self.rename_btn.setDisabled(not (name and not name.isspace())))

        self.scanned_devices = Qw.QVBoxLayout()
        self.scanned_devices.addWidget(self.filter_devices_box)
        self.scanned_devices.addWidget(self.devices_list)
        self.scanned_devices.addWidget(self.check_connection_btn)
        self.scanned_devices.addWidget(self.disconnect_btn)
        self.scanned_devices.addWidget(self.conn_status_label)
        self.scanned_devices_group = Qw.QGroupBox("Connection Settings")
        self.scanned_devices_group.setLayout(self.scanned_devices)

        self.scan_settings = Qw.QVBoxLayout()
        self.scan_settings.addLayout(self.scanning_timeout_layout)
        self.scan_settings.addWidget(self.scan_button)
        self.scan_settings.addWidget(self.scan_status_label)
        self.scan_settings.setAlignment(Qc.Qt.AlignJustify)
        self.scan_settings_group = Qw.QGroupBox("Scanning Settings")
        self.scan_settings_group.setLayout(self.scan_settings)

        self.rename_device_layout = Qw.QVBoxLayout()
        self.rename_device_layout.addWidget(self.device_name_box)
        self.rename_device_layout.addWidget(self.rename_btn)
        self.rename_device_layout.setAlignment(Qc.Qt.AlignJustify)
        self.rename_device_group = Qw.QGroupBox("Rename Device")
        self.rename_device_group.setLayout(self.rename_device_layout)

        self.left_layout = Qw.QVBoxLayout()
        self.left_layout.addWidget(self.scanned_devices_group)

        self.right_layout = Qw.QVBoxLayout()
        self.right_layout.addWidget(self.scan_settings_group)
        self.right_layout.addWidget(self.rename_device_group)

        self.layout = Qw.QHBoxLayout()
        self.layout.addLayout(self.left_layout)
        self.layout.addLayout(self.right_layout)

        self.setLayout(self.layout)

    def apply_filter(self, filter):
        for row in range(self.devices_list.count()):
            self.devices_list.item(row).setHidden(filter not in self.devices_list.item(row).text())
    
    async def scan_devices(self):
        self.scan_status_label.setText("Scanning devices...")

        try:
            devices = await self.bt_man.discover_availiable_devices(self.scanning_timeout_box.value())
            self.devices = {}
            self.devices_list.clear()
            for device in devices:
                self.devices[device.name] = device.address
                self.devices_list.addItem(device.name)
        except:
            self.scan_status_label.setText("Error - please scan again.")
            return
       
        self.scan_status_label.setText("Done scanning.")


    async def rename_device(self, name):
        self.conn_status_label.setText(self.string_device_disconnected)
        self.status_string_updated.emit(self.conn_status_label.text())
        try:
            if name and not name.isspace():
                await self.bt_man.rename_device(name)
        except:
            pass


    async def disconnect(self):
        await self.bt_man.disconnect()
        self.conn_status_label.setText(self.string_device_disconnected)
        self.status_string_updated.emit(self.conn_status_label.text())

    def update_conn_status_basic(self):
        if not self.bt_man.is_connected():
            self.conn_status_label.setText(self.string_device_disconnected)
            self.status_string_updated.emit(self.conn_status_label.text())

    async def update_conn_status(self):
        self.conn_status_label.setText("Checking connection ...")
        is_valid = False
        try:
           if self.bt_man.is_connected():
               is_valid = await self.bt_man.check_valid_connection()
        except:
           pass
        if self.bt_man.is_connected() and is_valid:
            self.conn_status_label.setText("✔️  Connected to [" + self.current_device + "]")
            # Let the main window know that the connection is good
            self.device_connected_successfully.emit()
        elif self.bt_man.is_connected() and not is_valid:
            self.conn_status_label.setText(self.string_device_invalid)
        else:
            self.conn_status_label.setText(self.string_device_disconnected)
        
        self.status_string_updated.emit(self.conn_status_label.text())

    async def update_device(self, device_name: str):
        self.conn_status_label.setText(self.string_device_disconnected)
        self.status_string_updated.emit(self.conn_status_label.text())

        self.current_device = device_name
        self.bt_man.set_address(self.devices[self.current_device])
        await self.bt_man.connect()
        await self.update_conn_status()

class ProfileComboBox(Qw.QComboBox):
    def __init__(self, num_profiles: int, parent=None):
        super().__init__(parent)
        for i in range(1, num_profiles+1):
            self.addItem(str(i))


def main():
    signal.signal(signal.SIGINT, signal.SIG_DFL)

    app = Qw.QApplication(sys.argv)  # Create application
    loop = QEventLoop(app)
    asyncio.set_event_loop(loop)  # Allows for spawing async threads from QT code

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
