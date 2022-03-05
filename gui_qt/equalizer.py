from PySide2 import QtCore as Qc
from PySide2 import QtWidgets as Qw
import asyncio

from jumpSlider import JumpSlider


class Equalizer(Qw.QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.freq_bands = [31, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000]
        self.band_widgets = []

        self.set_enable_btn = Qw.QPushButton("Enable Equalizer")
        self.set_enable_btn.setCheckable(True)
        self.set_enable_btn.toggled.connect(self.set_equalizer_enable)

        self.set_defaults_btn = Qw.QPushButton("Reset")
        self.set_defaults_btn.clicked.connect(self.set_defaults)

        self.load_settings_btn = Qw.QPushButton("Load From Device")
        self.load_settings_btn.clicked.connect(lambda: asyncio.ensure_future(self.load_settings()))

        self.toolbar_layout = Qw.QHBoxLayout()
        self.toolbar_layout.addWidget(self.set_enable_btn)
        self.toolbar_layout.addWidget(self.set_defaults_btn)
        self.toolbar_layout.addWidget(self.load_settings_btn)

        self.bands_layout = Qw.QHBoxLayout()

        for idx, b_freq in enumerate(self.freq_bands):
            if b_freq > 999:
                band_w = EqualizerBand(f'{int(b_freq / 1000) if b_freq % 1000 == 0 else (b_freq / 1000)} kHz', parent=self)
            else:
                band_w = EqualizerBand(f'{b_freq} Hz', parent=self)

            band_w.slider.valueChanged.connect(lambda x, idx=idx: asyncio.ensure_future(self.do_band_update(idx, dB=x)))

            self.band_widgets.append(band_w)
            self.bands_layout.addWidget(band_w)

        self.layout = Qw.QVBoxLayout()
        self.layout.addLayout(self.toolbar_layout)
        self.layout.addLayout(self.bands_layout)

        self.setLayout(self.layout)

        self.show()


    async def load_settings(self):
        await self.parent().parent().bt_man.read_eq_gains()
        gains = self.parent().parent().bt_man.eq_gains

        for idx, band_w in enumerate(self.band_widgets):
            band_w.slider.blockSignals(True)
            band_w.slider.setValue(gains[idx])
            band_w.update_db_value(gains[idx])
            band_w.slider.blockSignals(False)
        # self.set_enable_btn.blockSignals(True)
        # self.set_enable_btn.setChecked(status)
        # if self.set_enable_btn.isChecked():
        #     self.set_enable_btn.setText("Disable Equalizer")
        # else:
        #     self.set_enable_btn.setText("Enable Equalizer")
        # self.set_enable_btn.blockSignals(False)

        await self.parent().parent().bt_man.read_eq_enable()
        set_enable = self.parent().parent().bt_man.eq_enable
        self.set_enable_btn.blockSignals(True)
        self.set_enable_btn.setChecked(set_enable)
        self.set_enable_btn.blockSignals(False)


    def set_equalizer_enable(self):
        if self.set_enable_btn.isChecked():
            asyncio.ensure_future(self.parent().parent().bt_man.write_eq_enable(True))
            self.set_enable_btn.setText("Disable Equalizer")
        else:
            asyncio.ensure_future(self.parent().parent().bt_man.write_eq_enable(False))
            self.set_enable_btn.setText("Enable Equalizer")


    def set_defaults(self):
        for band_w in self.band_widgets:
            band_w.slider.blockSignals(True)
            band_w.slider.setValue(0)
            band_w.update_db_value(0)
            band_w.slider.blockSignals(False)

        asyncio.ensure_future(self.parent().parent().bt_man.write_eq_gains([0 for i in range(0, 10)]))


    async def do_band_update(self, band_num, dB):
        await self.parent().parent().bt_man.write_eq_gain_index(band_num, dB)
        self.parent().parent().update_conn_status()


class EqualizerBand(Qw.QWidget):
    def __init__(self, center_freq: str, parent=None):
        super().__init__(parent)
        self.center_freq = center_freq

        self.slider = JumpSlider(Qc.Qt.Vertical, parent=self)
        self.slider.setMinimum(-20)
        self.slider.setMaximum(10)
        self.slider.setSingleStep(1)
        self.slider.setPageStep(1)
        self.slider.setTickInterval(1)
        self.slider.setTickPosition(Qw.QSlider.TicksBothSides)
        self.slider.setSizePolicy(
            Qw.QSizePolicy.Fixed, Qw.QSizePolicy.Expanding)
        self.slider.valueChanged.connect(lambda val: self.update_db_value(val))

        self.db_value_label = Qw.QLabel("0 dB")
        self.db_value_label.setFixedWidth(40)
        self.db_value_label.setAlignment(Qc.Qt.AlignVCenter)
        self.db_value_label.setSizePolicy(
            Qw.QSizePolicy.Fixed, Qw.QSizePolicy.Fixed)

        self.freq_text_box = Qw.QLabel(self.center_freq, parent=self)
        self.freq_text_box.setAlignment(Qc.Qt.AlignVCenter)
        self.freq_text_box.setSizePolicy(
            Qw.QSizePolicy.Fixed, Qw.QSizePolicy.Fixed)

        self.layout = Qw.QVBoxLayout()
        self.layout.addWidget(self.db_value_label)
        self.layout.addWidget(self.slider)
        self.layout.addWidget(self.freq_text_box)
        self.layout.setAlignment(Qc.Qt.AlignVCenter)
        self.setLayout(self.layout)
        self.show()
  

    def update_db_value(self, value: int):
        self.db_value_label.setText(f'{value} dB')
