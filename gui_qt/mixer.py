from PySide2 import QtCore as Qc
from PySide2 import QtWidgets as Qw
import asyncio

from jumpSlider import JumpSlider


class Mixer(Qw.QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.tracks = ['3.5mm Jack', 'Bluetooth']
        self.band_widgets = []

        self.jack_enable_btn = Qw.QPushButton("Enable 3.5mm Jack Input")
        self.jack_enable_btn.setCheckable(True)
        self.jack_enable_btn.toggled.connect(self.jack_mixer_enable)

        self.wireless_enable_btn = Qw.QPushButton("Enable Bluetooth Input")
        self.wireless_enable_btn.setCheckable(True)
        self.wireless_enable_btn.toggled.connect(self.wireless_mixer_enable)

        self.set_defaults_btn = Qw.QPushButton("Reset")
        self.set_defaults_btn.clicked.connect(self.set_defaults)

        self.load_settings_btn = Qw.QPushButton("Load From Device")
        self.load_settings_btn.clicked.connect(lambda: asyncio.ensure_future(self.load_settings()))

        self.toolbar_layout = Qw.QHBoxLayout()
        self.toolbar_layout.addWidget(self.jack_enable_btn)
        self.toolbar_layout.addWidget(self.wireless_enable_btn)
        self.toolbar_layout.addWidget(self.set_defaults_btn)
        self.toolbar_layout.addWidget(self.load_settings_btn)

        self.bands_layout = Qw.QHBoxLayout()

        for idx, b_freq in enumerate(self.tracks):
            band_w = MixerBand(b_freq, parent=self)

            band_w.slider.valueChanged.connect(lambda x, idx=idx: asyncio.ensure_future(self.do_band_update(idx, dB=x)))

            self.band_widgets.append(band_w)
            self.bands_layout.addWidget(band_w)

        self.layout = Qw.QVBoxLayout()
        self.layout.addLayout(self.toolbar_layout)
        self.layout.addLayout(self.bands_layout)

        self.setLayout(self.layout)

        self.show()


    async def load_settings(self):
        await self.parent().parent().bt_man.read_mix_gains()
        gains = self.parent().parent().bt_man.mix_gains

        for idx, band_w in enumerate(self.band_widgets):
            band_w.slider.blockSignals(True)
            band_w.slider.setValue(int(gains[idx]))
            band_w.update_db_value(int(gains[idx]))
            band_w.slider.blockSignals(False)

        await self.parent().parent().bt_man.read_mix_line_in_en()
        jack_enable = self.parent().parent().bt_man.mix_line_in_en
        self.jack_enable_btn.blockSignals(True)
        self.jack_enable_btn.setChecked(jack_enable)
        self.jack_enable_btn.blockSignals(False)

        await self.parent().parent().bt_man.read_mix_wireless_in_en()
        wireless_enable = self.parent().parent().bt_man.mix_wireless_in_en
        self.wireless_enable_btn.blockSignals(True)
        self.wireless_enable_btn.setChecked(wireless_enable)
        self.wireless_enable_btn.blockSignals(False)


    def jack_mixer_enable(self):
        if self.jack_enable_btn.isChecked():
            asyncio.ensure_future(self.parent().parent().bt_man.write_mix_line_in_en(True))
            self.jack_enable_btn.setText("Disable 3.5mm Jack Input")
        else:
            asyncio.ensure_future(self.parent().parent().bt_man.write_mix_line_in_en(False))
            self.jack_enable_btn.setText("Enable 3.5mm Jack Input")


    def wireless_mixer_enable(self):
        if self.wireless_enable_btn.isChecked():
            asyncio.ensure_future(self.parent().parent().bt_man.write_mix_wireless_in_en(True))
            self.wireless_enable_btn.setText("Disable Bluetooth Input")
        else:
            asyncio.ensure_future(self.parent().parent().bt_man.write_mix_wireless_in_en(False))
            self.wireless_enable_btn.setText("Enable Bluetooth Input")


    def set_defaults(self):
        for band_w in self.band_widgets:
            band_w.slider.blockSignals(True)
            band_w.slider.setValue(0)
            band_w.update_db_value(0)
            band_w.slider.blockSignals(False)

        asyncio.ensure_future(self.parent().parent().bt_man.write_mix_gains([0 for i in range(0, 2)]))


    async def do_band_update(self, band_num, dB):
        await self.parent().parent().bt_man.write_mix_gain_index(band_num, int(dB))
        self.parent().parent().update_conn_status()


class MixerBand(Qw.QWidget):
    def __init__(self, label: str, parent=None):
        super().__init__(parent)
        self.label = label

        self.slider = JumpSlider(Qc.Qt.Horizontal, parent=self)
        self.slider.setMinimum(-40)
        self.slider.setMaximum(20)
        self.slider.setSingleStep(1)
        self.slider.setPageStep(1)
        self.slider.setTickInterval(1)
        self.slider.setTickPosition(Qw.QSlider.TicksBothSides)
        self.slider.setSizePolicy(
            Qw.QSizePolicy.Expanding, Qw.QSizePolicy.Fixed)
        self.slider.valueChanged.connect(lambda val: self.update_db_value(val))

        self.db_value_label = Qw.QLabel("0 dB")
        self.db_value_label.setFixedWidth(60)
        self.db_value_label.setAlignment(Qc.Qt.AlignVCenter)
        self.db_value_label.setSizePolicy(
            Qw.QSizePolicy.Fixed, Qw.QSizePolicy.Fixed)

        self.label_text_box = Qw.QLabel(self.label, parent=self)
        self.label_text_box.setAlignment(Qc.Qt.AlignVCenter)
        self.label_text_box.setSizePolicy(
            Qw.QSizePolicy.Fixed, Qw.QSizePolicy.Fixed)

        self.layout = Qw.QVBoxLayout()
        self.layout.addWidget(self.db_value_label)
        self.layout.addWidget(self.slider)
        self.layout.addWidget(self.label_text_box)
        self.layout.setAlignment(Qc.Qt.AlignVCenter)
        self.setLayout(self.layout)
        self.show()


    def update_db_value(self, value: int):
        self.db_value_label.setText(f'{value} dB')
